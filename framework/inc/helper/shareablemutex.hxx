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

#include <osl/interlck.h>
#include <osl/mutex.hxx>

namespace framework
{

/**
 * This acts like a rtl::Reference<osl::Mutex>
 */
class ShareableMutex
{
    public:
        ShareableMutex();
        ShareableMutex( const ShareableMutex& rShareableMutex );
        ShareableMutex& operator=( const ShareableMutex& rShareableMutex );

        ~ShareableMutex() { m_pMutexRef->release(); }

        /** acquire the shared mutex */
        void acquire();
        /** release the shared mutex */
        void release();

    private:
        /* ShareableMutex::MutexRef will destroy itself when the last ShareableMutex pointing to it is destroyed */
        struct MutexRef
        {
            MutexRef() : m_refCount(0) {}
            void acquire()
            {
                osl_atomic_increment( &m_refCount );
            }

            void release()
            {
                if ( osl_atomic_decrement( &m_refCount ) == 0 )
                    delete this;
            }

            oslInterlockedCount m_refCount;
            osl::Mutex          m_oslMutex;
        };

        MutexRef* m_pMutexRef;
};

class ShareGuard
{
    public:
        explicit ShareGuard( ShareableMutex& rShareMutex ) :
            m_rShareMutex( rShareMutex )
        {
            m_rShareMutex.acquire();
        }

        ~ShareGuard()
        {
            m_rShareMutex.release();
        }

    private:
        ShareGuard& operator=( const ShareGuard& ) = delete;

        ShareableMutex& m_rShareMutex;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
