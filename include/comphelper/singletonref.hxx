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

#include <sal/config.h>
#include <sal/log.hxx>
#include <mutex>

namespace comphelper
{
/** @short  Template for implementing singleton classes.
            This is a replacement for salhelper::SingletonRef, but which uses std::mutex instead of osl::Mutex.

            Such classes can be instantiated every time they
            are needed. But the internal wrapped object will
            be created one times only. Of course it's used
            resources are referenced one times only too.
            This template hold it alive till the last
            reference is gone. Further all operations
            on this reference are threadsafe. Only
            calls directly to the internal object (which modify
            its state) must be made threadsafe by the object itself
            or from outside.

    @attention  To prevent the code against race conditions, it's not
                allowed to start operations inside the ctor
                of the internal wrapped object - especially operations
                which needs a reference to the same singleton too.

                The only chance to suppress such strange constellations
                is a lazy-init mechanism.

                <ul>
                    <li>a) The singleton class can provide a special init()
                           method, which must be called as first after creation.</li>
                    <li>b) The singleton class can call a special impl_init()
                           method implicit for every called interface method.</li>
                </ul>

                Note further that this singleton pattern can work only, if
                all user of such singleton are located inside the same library!
                Because static values can't be exported - e.g. from windows libraries.
 */
template <class SingletonClass> class SingletonRef
{
    // member

private:
    /** @short  pointer to the internal wrapped singleton. */
    static SingletonClass* m_pInstance;

    /** @short  ref count, which regulate creation and removing of m_pInstance. */
    static sal_Int32 m_nRef;

    // interface

public:
    /** @short  standard ctor.

                    The internal wrapped object is created only,
                    if its ref count was 0. Otherwise this method
                    does nothing ... except increasing of the internal
                    ref count!
         */
    SingletonRef()
    {
        // GLOBAL SAFE ->
        std::unique_lock aLock(SingletonRef::ownStaticLock());

        // must be increased before(!) the check is done.
        // Otherwise this check can fail inside the same thread ...
        ++m_nRef;
        if (m_nRef == 1)
            m_pInstance = new SingletonClass();

        SAL_WARN_IF(!(m_nRef > 0 && m_pInstance), "comphelper",
                    "Race? Ref count of singleton >0, but instance is NULL!");
        // <- GLOBAL SAFE
    }

    /** @short  standard dtor.

                    The internal wrapped object is removed only,
                    if its ref count will be 0. Otherwise this method
                    does nothing ... except decreasing of the internal
                    ref count!
         */
    ~SingletonRef()
    {
        // GLOBAL SAFE ->
        std::unique_lock aLock(SingletonRef::ownStaticLock());

        // must be decreased before(!) the check is done.
        // Otherwise this check can fail inside the same thread ...
        --m_nRef;
        if (m_nRef == 0)
        {
            delete m_pInstance;
            m_pInstance = nullptr;
        }
        // <- GLOBAL SAFE
    }

    SingletonRef& operator=(SingletonRef const&) = default;

    /** @short  Allows rSingle->someBodyOp().
         */
    SingletonClass* operator->() const
    {
        // GLOBAL SAFE ->
        return m_pInstance;
        // <- GLOBAL SAFE
    }

    /** @short  Allows (*rSingle).someBodyOp().
         */
    SingletonClass& operator*() const
    {
        // GLOBAL SAFE ->
        return *m_pInstance;
        // <- GLOBAL SAFE
    }

    // helper

private:
    SingletonRef(SingletonRef&) = delete;

    static std::mutex& ownStaticLock()
    {
        static std::mutex aInstance;
        return aInstance;
    }
};

template <class SingletonClass> SingletonClass* SingletonRef<SingletonClass>::m_pInstance = nullptr;

template <class SingletonClass> sal_Int32 SingletonRef<SingletonClass>::m_nRef = 0;

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
