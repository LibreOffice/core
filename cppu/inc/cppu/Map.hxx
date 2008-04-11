/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Map.hxx,v $
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
