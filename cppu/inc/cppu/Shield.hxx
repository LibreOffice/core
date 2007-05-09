/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Shield.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:33:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
