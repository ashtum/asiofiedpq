#pragma once

#include <psql/detail/oid_map.hpp>
#include <psql/detail/type_traits.hpp>

#include <string_view>

namespace psql
{
namespace detail
{
template<typename T>
struct type_name_of_impl
{
  static void apply(std::vector<std::string_view>&, const detail::oid_map&)
  {
  }
};

template<typename T>
void type_name_of(std::vector<std::string_view>& vec, const detail::oid_map& omp)
{
  return type_name_of_impl<std::decay_t<T>>::apply(vec, omp);
}

template<typename T>
  requires is_array<T>::value
struct type_name_of_impl<T>
{
  static void apply(std::vector<std::string_view>& vec, const detail::oid_map& omp)
  {
    type_name_of<typename T::value_type>(vec, omp);
  }
};

template<typename T>
struct type_tag
{
};

template<typename T>
  requires is_composite<T>::value
struct type_name_of_impl<T>
{
  static void apply(std::vector<std::string_view>& vec, const detail::oid_map& omp)
    requires is_user_defined<T>::value
  {
    if (!omp.contains(user_defined<T>::name))
      vec.push_back(user_defined<T>::name);

    std::apply([&](auto&&... mems) { (type_name_of<decltype(T{}.*mems)>(vec, omp), ...); }, user_defined<T>::members);
  }

  static void apply(std::vector<std::string_view>& vec, const detail::oid_map& omp)
    requires is_tuple<T>::value
  {
    [&]<typename... Ts>(type_tag<std::tuple<Ts...>>) { (type_name_of<Ts>(vec, omp), ...); }(type_tag<T>{});
  }
};

} // namespace detail
} // namespace psql
