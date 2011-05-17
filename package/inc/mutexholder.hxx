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

#ifndef __MUTEXHOLDER_HXX_
#define __MUTEXHOLDER_HXX_

#include <osl/mutex.hxx>

class SotMutexHolder
{
    ::osl::Mutex m_aMutex;
    sal_Int32    m_nRefCount;

    public:
    SotMutexHolder() : m_nRefCount( 0 ) {}

    void AddRef()
    {
        m_nRefCount++;
    }

    void ReleaseRef()
    {
        if ( !--m_nRefCount )
            delete this;
    }

    ::osl::Mutex& GetMutex() { return m_aMutex; }
};

class SotMutexHolderRef
{
    SotMutexHolder* m_pHolder;

public:
    SotMutexHolderRef()
    : m_pHolder( NULL )
    {}

    SotMutexHolderRef( SotMutexHolder* pHolder )
    : m_pHolder( pHolder )
    {
        if ( m_pHolder )
            m_pHolder->AddRef();
    }

    SotMutexHolderRef( const SotMutexHolderRef& rRef )
    : m_pHolder( rRef.m_pHolder )
    {
        if ( m_pHolder )
            m_pHolder->AddRef();
    }

    ~SotMutexHolderRef()
    {
        if ( m_pHolder )
            m_pHolder->ReleaseRef();
    }

    SotMutexHolderRef& operator =( const SotMutexHolderRef& rRef )
    {
        if ( m_pHolder )
            m_pHolder->ReleaseRef();

        m_pHolder = rRef.m_pHolder;

        if ( m_pHolder )
            m_pHolder->AddRef();

        return *this;
    }

    SotMutexHolderRef& operator =( SotMutexHolder* pHolder )
    {
        if ( m_pHolder )
            m_pHolder->ReleaseRef();

        m_pHolder = pHolder;

        if ( m_pHolder )
            m_pHolder->AddRef();
        return *this;
    }

    SotMutexHolder* operator ->() const
    {
        return m_pHolder;
    }

    SotMutexHolder& operator *() const
    {
        return *m_pHolder;
    }

    operator SotMutexHolder*() const
    {
        return m_pHolder;
    }

    sal_Bool Is() const
    {
        return m_pHolder != NULL;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
