/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#if ! defined(INCLUDED_COMPHELPER_OPTIONAL_HXX)
#define INCLUDED_COMPHELPER_OPTIONAL_HXX

#if ! defined(_COM_SUN_STAR_BEANS_OPTIONAL_HPP_)
#include "com/sun/star/beans/Optional.hpp"
#endif
#include "boost/optional.hpp"

namespace comphelper {

/// Object generators for boost::optional<T>, beans::Optional<T>:

template <typename T>
inline ::boost::optional<T> make_optional( T const& v )
{
    return ::boost::optional<T>(v);
}

template <typename T>
inline ::boost::optional<T> make_optional(
    ::com::sun::star::beans::Optional<T> const& o )
{
    if (o.IsPresent)
        return ::boost::optional<T>(o.Value);
    else
        return ::boost::optional<T>();
}

template <typename T>
inline ::com::sun::star::beans::Optional<T> makeOptional( T const& v )
{
//     CPPU_IS_CPP_MAPPING_OF_NON_VOID_UNO_TYPE(T);
    return ::com::sun::star::beans::Optional<T>(true, v);
}

template <typename T>
inline ::com::sun::star::beans::Optional<T> makeOptional(
    ::boost::optional<T> const& o )
{
//     CPPU_IS_CPP_MAPPING_OF_NON_VOID_UNO_TYPE(T);
    if (o)
        return ::com::sun::star::beans::Optional<T>(true, *o);
    else
        return ::com::sun::star::beans::Optional<T>();
}

inline ::com::sun::star::beans::Optional<sal_Bool> makeOptional(
    ::boost::optional<bool> const& o )
{
    if (o)
        return ::com::sun::star::beans::Optional<sal_Bool>(true, *o);
    else
        return ::com::sun::star::beans::Optional<sal_Bool>();
}

inline ::com::sun::star::beans::Optional<sal_Bool> makeOptional( bool v )
{
    return ::com::sun::star::beans::Optional<sal_Bool>(true, v);
}

} // namespace comphelper

#endif // ! defined(INCLUDED_COMPHELPER_OPTIONAL_HXX)

