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

#ifndef INCLUDED_COMPHELPER_FLAGGUARD_HXX
#define INCLUDED_COMPHELPER_FLAGGUARD_HXX

#include <comphelper/scopeguard.hxx>
#include <utility>

namespace comphelper
{

    //= ValueRestorationGuard

    // note: can't store the originalValue in a ValueRestorationGuard member,
    // because it will be used from base class dtor
    template <typename T> struct ValueRestorationGuard_Impl
    {
        T& rVal;
        T const originalValue;
        ValueRestorationGuard_Impl(T& i_valRef)
            : rVal(i_valRef), originalValue(i_valRef) {}
        void operator()()
        {
            rVal = originalValue;
        }
    };

    template <typename T>
    class ValueRestorationGuard : public ScopeGuard<ValueRestorationGuard_Impl<T>>
    {
    public:
        ValueRestorationGuard(T& i_valRef)
            : ScopeGuard<ValueRestorationGuard_Impl<T>>(ValueRestorationGuard_Impl(i_valRef))
        {}

        template <typename T1>
        ValueRestorationGuard(T& i_valRef, T1&& i_temporaryValue)
            : ScopeGuard<ValueRestorationGuard_Impl<T>>(ValueRestorationGuard_Impl(i_valRef))
        {
            i_valRef = std::forward<T1>(i_temporaryValue);
        }
    };

    typedef ValueRestorationGuard<bool> FlagRestorationGuard;

    //= FlagGuard

    // Guarantees that the flag is true within the scope of the guard, and is set to false after
    // its destruction, regardless of initial flag value
    class FlagGuard : public FlagRestorationGuard
    {
    public:
        // Set flag to false before passing its reference to base class ctor, so that it would be
        // reset back to false in base class dtor
        explicit FlagGuard(bool& i_flagRef)
            : FlagRestorationGuard((i_flagRef = false), true)
        {
        }
    };


} // namespace comphelper


#endif // INCLUDED_COMPHELPER_FLAGGUARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
