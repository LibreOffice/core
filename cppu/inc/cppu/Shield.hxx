/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Shield.hxx,v $
 * $Revision: 1.3 $
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

#ifndef INCLUDED_cppu_Shield_hxx
#define INCLUDED_cppu_Shield_hxx

#include <cppu/Map.hxx>


namespace cssu = com::sun::star::uno;

namespace cppu
{
    /** Helpers for mapping objects relative to the thread-safe and current environments.
        (http://wiki.services.openoffice.org/wiki/Uno/Cpp/Spec/Shield_Helpers)
    */


    /** Maps an object from the current to the thread-safe Environment, returns mapped object.

        @param  pT        the object to be mapped
        @return           the mapped object
        @since UDK 3.2.7
     */
    template<class T> inline T * shield(T * pT)
    {
        return mapOut(pT, cssu::Environment(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV)))));
    }

    /** Maps an object from the thread-safe Environment to the current one, returns mapped object.

        @param  pT        the object to be mapped
        @return           the mapped object
        @since UDK 3.2.7
     */
    template<class T> inline T * unshield(T * pT)
    {
        return mapIn(pT, cssu::Environment(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV)))));
    }


    /** Maps an any from the current to the thread-safe Environment, fills the passed any.

        @param  any  the any to be mapped
        @param       the target any
        @since UDK 3.2.7
     */
    inline void shieldAny(cssu::Any const & any, cssu::Any * res)
    {
        mapOutAny(any, res, cssu::Environment(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV)))));
    }


    /** Maps an any from the thread-safe Environment to the current one, fills the passed any.

        @param  any  the any to be mapped
        @param       the target any
        @since UDK 3.2.7
     */
    inline void unshieldAny(cssu::Any const & any, cssu::Any * res)
    {
        mapInAny(any, res, cssu::Environment(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV)))));
    }
}

#endif
