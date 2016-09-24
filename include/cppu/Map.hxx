/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_CPPU_MAP_HXX
#define INCLUDED_CPPU_MAP_HXX

#include <uno/mapping.hxx>


namespace cppu
{
    /** Helpers for mapping objects relative to the current environment.
        (http://wiki.openoffice.org/wiki/Uno/Cpp/Spec/Map_Helpers)
    */

    /** Maps an object from the current to an outer Environment, returns mapped object.

        @param  pT        the object to be mapped
        @param  outerEnv  the target environment
        @return           the mapped object
        @since UDK 3.2.7
     */
    template<class T> inline T * mapOut(T * pT, css::uno::Environment const & outerEnv)
    {
        css::uno::Mapping curr2outer(css::uno::Environment::getCurrent(), outerEnv);

        return reinterpret_cast<T *>(curr2outer.mapInterface(pT, cppu::UnoType<T>::get()));
    }


    /** Maps an object from an outer Environment to the current, returns mapped object.

        @param  pT        the object to be mapped
        @param  outerEnv  the source environment
        @return           the mapped object
        @since UDK 3.2.7
     */
    template<class T> inline T * mapIn(T * pT, css::uno::Environment const & outerEnv)
    {
        css::uno::Mapping outer2curr(outerEnv, css::uno::Environment::getCurrent());

        return reinterpret_cast<T *>(outer2curr.mapInterface(pT, cppu::UnoType<T>::get()));
    }


    /** Maps an any from the current to an outer Environment, fills passed any.

        @param  any       the any to be mapped
        @param  res       the target any
        @param  outerEnv  the target environment
        @since UDK 3.2.7
     */
    // Problem: any gets assigned to something, acquire/releases may be called in wrong env.
    inline void mapOutAny(css::uno::Any const & any, css::uno::Any * res, css::uno::Environment const & outerEnv)
    {
        css::uno::Mapping curr2outer(css::uno::Environment::getCurrent(), outerEnv);

        uno_any_destruct(res, css::uno::cpp_release);
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
    inline void mapInAny(css::uno::Any const & any, css::uno::Any * res, css::uno::Environment const & outerEnv)
    {
        css::uno::Mapping outer2curr(outerEnv, css::uno::Environment::getCurrent());

        uno_any_destruct(res, css::uno::cpp_release);
        uno_type_any_constructAndConvert(
            res,
            const_cast<void *>(any.getValue()),
            any.getValueTypeRef(),
            outer2curr.get());
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
