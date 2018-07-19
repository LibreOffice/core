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


namespace comphelper
{

    //= FlagRestorationGuard

    class COMPHELPER_DLLPUBLIC FlagRestorationGuard : public ScopeGuard
    {
    public:
        FlagRestorationGuard( bool& i_flagRef, bool i_temporaryValue )
            : ScopeGuard(RestoreFlag(i_flagRef))
        {
            i_flagRef = i_temporaryValue;
        }

        ~FlagRestorationGuard();

    private:
        // note: can't store the originalValue in a FlagRestorationGuard member,
        // because it will be used from base class dtor
        struct RestoreFlag
        {
            bool & rFlag;
            bool const originalValue;
            RestoreFlag(bool & i_flagRef)
                : rFlag(i_flagRef), originalValue(i_flagRef) {}
            void operator()()
            {
                rFlag = originalValue;
            }
        };
    };


    //= FlagGuard

    class COMPHELPER_DLLPUBLIC FlagGuard : public ScopeGuard
    {
    public:
        explicit FlagGuard( bool& i_flagRef )
            : ScopeGuard( [&i_flagRef] () { i_flagRef = false; } )
        {
            i_flagRef = true;
        }

        ~FlagGuard();
    };


} // namespace comphelper


#endif // INCLUDED_COMPHELPER_FLAGGUARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
