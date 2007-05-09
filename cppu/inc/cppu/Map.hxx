/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Map.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:33:15 $
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

#ifndef INCLUDED_cppu_Map_hxx
#define INCLUDED_cppu_Map_hxx

#include <uno/mapping.hxx>


namespace cssu = com::sun::star::uno;

namespace cppu
{
    /** Helpers for mapping objects relative to the current environment.
        (http://wiki.services.openoffice.org/wiki/Uno/Cpp/Spec/Map_Helpers)
    */

    /** Maps an object from the current to an outer Environment, returns mapped object.

        @param  pT        the object to be mapped
        @param  outerEnv  the target environment
        @return           the mapped object
        @since UDK 3.2.7
     */
    template<class T> inline T * mapOut(T * pT, cssu::Environment const & outerEnv)
    {
        cssu::Mapping curr2outer(cssu::Environment::getCurrent(), outerEnv);

        return reinterpret_cast<T *>(curr2outer.mapInterface(pT, getCppuType((cssu::Reference<T> *)NULL)));
    }


    /** Maps an object from an outer Environment to the current, returns mapped object.

        @param  pT        the object to be mapped
        @param  outerEnv  the source environment
        @return           the mapped object
        @since UDK 3.2.7
     */
    template<class T> inline T * mapIn(T * pT, cssu::Environment const & outerEnv)
    {
        cssu::Mapping outer2curr(outerEnv, cssu::Environment::getCurrent());

        return reinterpret_cast<T *>(outer2curr.mapInterface(pT, getCppuType((cssu::Reference<T> *)NULL)));
    }


    /** Maps an any from the current to an outer Environment, fills passed any.

        @param  any       the any to be mapped
        @param  res       the target any
        @param  outerEnv  the target environment
        @since UDK 3.2.7
     */
    // Problem: any gets assigned to something, acquire/releases may be called in wrong env.
    inline void mapOutAny(cssu::Any const & any, cssu::Any * res, cssu::Environment const & outerEnv)
    {
        cssu::Mapping curr2outer(cssu::Environment::getCurrent(), outerEnv);

        uno_any_destruct(res, (uno_ReleaseFunc)cssu::cpp_release);
        uno_type_any_constructAndConvert(
            res,
            const_cast<void *>(any.getValue()),
            any.getValueTypeRef(),
            curr2outer.get());
    }


    /** Maps an any from an outer Environment to the current, fills passed any.

        @param  any       the any to be mapped
        @param  res       the target any
        @param  outerEnv  the source environment
        @since UDK 3.2.7
     */
    inline void mapInAny(cssu::Any const & any, cssu::Any * res, cssu::Environment const & outerEnv)
    {
        cssu::Mapping outer2curr(outerEnv, cssu::Environment::getCurrent());

        uno_any_destruct(res, (uno_ReleaseFunc)cssu::cpp_release);
        uno_type_any_constructAndConvert(
            res,
            const_cast<void *>(any.getValue()),
            any.getValueTypeRef(),
            outer2curr.get());
    }
}

#endif
