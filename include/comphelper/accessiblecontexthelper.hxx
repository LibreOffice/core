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

#pragma once

#include <comphelper/accessiblecomponenthelper.hxx>
#include <comphelper/solarmutex.hxx>


namespace comphelper
{


    //= OContextEntryGuard

    /** helper class for guarding the entry into OAccessibleComponentHelper methods.

        <p>The class has two responsibilities:
        <ul><li>it locks the mutex of an OAccessibleComponentHelper instance, as long as the guard lives</li>
            <li>it checks if a given OAccessibleComponentHelper instance is alive, else an exception is thrown
                our of the constructor of the guard</li>
        </ul>
        <br/>
        This makes it your first choice (hopefully :) for guarding any interface method implementations of
        you derived class.
        </p>
    */
    class OContextEntryGuard : public ::osl::ClearableMutexGuard
    {
    public:
        /** constructs the guard

            <p>The given context (it's mutex, respectively) is locked, and an exception is thrown if the context
            is not alive anymore. In the latter case, of course, the mutex is freed, again.</p>

        @param _pContext
            the context which shall be guarded
        @precond <arg>_pContext</arg> != NULL
        */
        inline OContextEntryGuard(OAccessibleComponentHelper* _pContext);
    };


    inline OContextEntryGuard::OContextEntryGuard(OAccessibleComponentHelper* _pContext)
        : ::osl::ClearableMutexGuard( _pContext->GetMutex() )
    {
        _pContext->ensureAlive();
    }


    //= OExternalLockGuard

    class OExternalLockGuard
            :public osl::Guard<SolarMutex>
            ,public OContextEntryGuard
    {
    public:
        inline OExternalLockGuard(OAccessibleComponentHelper* _pContext);
    };


    inline OExternalLockGuard::OExternalLockGuard(OAccessibleComponentHelper* _pContext)
        :osl::Guard<SolarMutex>( SolarMutex::get() )
        ,OContextEntryGuard( _pContext )
    {
        // Only lock the external mutex,
        // release the ::osl::Mutex of the OAccessibleComponentHelper instance.
        // If you call into another UNO object with locked ::osl::Mutex,
        // this may lead to dead locks.
        clear();
    }


}   // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
