/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef __FRAMEWORK_HELPER_SHAREABLEMUTEX_HXX_
#define __FRAMEWORK_HELPER_SHAREABLEMUTEX_HXX_

#include <osl/interlck.h>
#include <osl/mutex.hxx>
#include <fwidllapi.h>

namespace framework
{

class FWI_DLLPUBLIC ShareableMutex
{
    public:
        ShareableMutex();
        ShareableMutex( const ShareableMutex& rShareableMutex );
        const ShareableMutex& operator=( const ShareableMutex& rShareableMutex );

        ~ShareableMutex();

        void acquire();
        void release();

    private:
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

        MutexRef* pMutexRef;
};

class ShareGuard
{
    public:
        ShareGuard( ShareableMutex& rShareMutex ) :
            m_rShareMutex( rShareMutex )
        {
            m_rShareMutex.acquire();
        }

        ~ShareGuard()
        {
            m_rShareMutex.release();
        }

    private:
        ShareGuard();
        ShareGuard& operator=( const ShareGuard& );

        ShareableMutex& m_rShareMutex;
};

}

#endif // #ifndef __FRAMEWORK_HELPER_SHAREABLEMUTEX_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
