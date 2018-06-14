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

#ifndef INCLUDED_COMPHELPER_VALUEGUARD_HXX
#define INCLUDED_COMPHELPER_VALUEGUARD_HXX

#include <comphelper/scopeguard.hxx>


namespace comphelper
{
    class COMPHELPER_DLLPUBLIC ValueRestorationGuard : public ScopeGuard
    {
    public:
        template<class T>
        ValueRestorationGuard(T& i_valRef)
            : ScopeGuard(RestoreValue(i_valRef))
        {}

        template <class T>
        ValueRestorationGuard(T& i_valRef, const T& i_temporaryValue)
            : ValueRestorationGuard(i_valRef)
        {
            i_valRef = i_temporaryValue;
        }

        ~ValueRestorationGuard();

    private:
        // note: can't store the originalValue in a ValueRestorationGuard member,
        // because it will be used from base class dtor
        template<class T>
        struct RestoreValue
        {
            T & rValue;
            T originalValue;
            RestoreValue(T& i_valRef)
                : rValue(i_valRef), originalValue(i_valRef) {}
            void operator()()
            {
                rValue = originalValue;
            }
        };
    };
} // namespace comphelper


#endif // INCLUDED_COMPHELPER_VALUEGUARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
