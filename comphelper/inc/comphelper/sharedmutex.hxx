/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#ifndef COMPHELPER_SHAREDMUTEX_HXX
#define COMPHELPER_SHAREDMUTEX_HXX

#include "comphelper/comphelperdllapi.h"

#include <osl/mutex.hxx>

#include <boost/shared_ptr.hpp>

//........................................................................
namespace comphelper
{
//........................................................................

    //============================================================
    //= SharedMutex
    //============================================================
    class COMPHELPER_DLLPUBLIC SharedMutex
    {
    public:
        SharedMutex();
        SharedMutex( const SharedMutex& );
        SharedMutex& operator=( const SharedMutex& );
        ~SharedMutex()
        {
        }

        inline ::osl::Mutex& getMutex() { return *m_pMutexImpl; }
        inline operator ::osl::Mutex& () { return *m_pMutexImpl; }

    private:
        ::boost::shared_ptr< ::osl::Mutex >  m_pMutexImpl;
    };

    //============================================================
    //= SharedMutexBase
    //============================================================
    /** sometimes, it's necessary to have an initialized ::osl::Mutex to pass
        to some ctor call of your base class. In this case, you can't hold the
        SharedMutex as member, but you need to move it into another base class,
        which is initialized before the mutex-requiring class is.
    */
    class COMPHELPER_DLLPUBLIC SharedMutexBase
    {
    protected:
        SharedMutexBase()
        {
        }
        ~SharedMutexBase()
        {
        }

    protected:
        ::osl::Mutex&   getMutex() const { return m_aMutex; }
        SharedMutex&    getSharedMutex() const { return m_aMutex; }

    private:
        mutable SharedMutex m_aMutex;
    };

//........................................................................
} // namespace comphelper
//........................................................................

#endif // COMPHELPER_SHAREDMUTEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
