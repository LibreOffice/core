/*************************************************************************
 *
 *  $RCSfile: optional.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-03-30 08:17:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#if ! defined(INCLUDED_COMPHELPER_OPTIONAL_HXX)
#define INCLUDED_COMPHELPER_OPTIONAL_HXX

// #if ! defined(INCLUDED_CPPUHELPER_BOOSTUNO_HXX)
// #include "cppuhelper/boostuno.hxx"
// #endif
#if ! defined(_COM_SUN_STAR_BEANS_OPTIONAL_HPP_)
#include "com/sun/star/beans/Optional.hpp"
#endif
#include "boost/optional.hpp"

namespace comphelper {

/// Object generators for boost::optional<T>, beans::Optional<T>:

template <typename T>
inline ::boost::optional<T> make_optional( T const & v )
{
    return ::boost::optional<T>(v);
}

template <typename T>
inline ::boost::optional<T> make_optional(
    ::com::sun::star::beans::Optional<T> const & o )
{
    if (o.IsPresent)
        return ::boost::optional<T>(o.Value);
    else
        return ::boost::optional<T>();
}

template <typename T>
inline ::com::sun::star::beans::Optional<T> makeOptional( T const & v )
{
//     CPPU_IS_CPP_MAPPING_OF_NON_VOID_UNO_TYPE(T);
    return ::com::sun::star::beans::Optional<T>(true, v);
}

template <typename T>
inline ::com::sun::star::beans::Optional<T> makeOptional(
    ::boost::optional<T> const & o )
{
//     CPPU_IS_CPP_MAPPING_OF_NON_VOID_UNO_TYPE(T);
    if (o)
        return ::com::sun::star::beans::Optional<T>(true, *o);
    else
        return ::com::sun::star::beans::Optional<T>();
}

inline ::com::sun::star::beans::Optional<sal_Bool> makeOptional(
    ::boost::optional<bool> const & o )
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

