/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: optional.hxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

