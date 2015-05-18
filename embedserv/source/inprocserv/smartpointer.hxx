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

#ifndef INCLUDED_EMBEDSERV_SOURCE_INPROCSERV_SMARTPOINTER_HXX
#define INCLUDED_EMBEDSERV_SOURCE_INPROCSERV_SMARTPOINTER_HXX

namespace inprocserv{

void WriteDebugInfo( DWORD pThis, char* pString, DWORD nToWrite );

template< class T > class ComSmart
{
    T* m_pInterface;

    void OwnRelease()
    {
        if ( m_pInterface )
        {
            T* pInterface = m_pInterface;
            m_pInterface = NULL;
            pInterface->Release();
        }
    }

public:
    ComSmart()
    : m_pInterface( NULL )
    {}

    ComSmart( const ComSmart<T>& rObj )
    : m_pInterface( rObj.m_pInterface )
    {
        if ( m_pInterface != NULL )
            m_pInterface->AddRef();
    }

    ComSmart( T* pInterface )
    : m_pInterface( pInterface )
    {
         if ( m_pInterface != NULL )
            m_pInterface->AddRef();
    }

    ~ComSmart()
    {
        OwnRelease();
    }

    ComSmart& operator=( const ComSmart<T>& rObj )
    {
        OwnRelease();

        m_pInterface = rObj.m_pInterface;

        if ( m_pInterface != NULL )
            m_pInterface->AddRef();

        return *this;
    }

    ComSmart<T>& operator=( T* pInterface )
    {
        OwnRelease();

        m_pInterface = pInterface;

        if ( m_pInterface != NULL )
            m_pInterface->AddRef();

        return *this;
    }

    operator T*() const
    {
        return m_pInterface;
    }

    T& operator*() const
    {
        return *m_pInterface;
    }

    T** operator&()
    {
        OwnRelease();

        m_pInterface = NULL;

        return &m_pInterface;
    }

    T* operator->() const
    {
        return m_pInterface;
    }

    BOOL operator==( const ComSmart<T>& rObj ) const
    {
        return ( m_pInterface == rObj.m_pInterface );
    }

    BOOL operator!=( const ComSmart<T>& rObj ) const
    {
        return ( m_pInterface != rObj.m_pInterface );
    }

    BOOL operator==( const T* pInterface ) const
    {
        return ( m_pInterface == pInterface );
    }

    BOOL operator!=( const T* pInterface ) const
    {
        return ( m_pInterface != pInterface );
    }
};

class CSGuard
{
    CRITICAL_SECTION* m_pCriticalSection;

public:
    CSGuard( CRITICAL_SECTION* pCS )
    : m_pCriticalSection( pCS )
    {
        if ( m_pCriticalSection )
            EnterCriticalSection( m_pCriticalSection );
    }

    ~CSGuard()
    {
        if ( m_pCriticalSection )
            LeaveCriticalSection( m_pCriticalSection );
    }
};

class ULONGGuard
{
    ULONG* m_pValue;

public:
    ULONGGuard( ULONG* pValue )
    : m_pValue( pValue )
    {
        if ( m_pValue )
            (*m_pValue)++;
    }

    ~ULONGGuard()
    {
        if ( m_pValue )
            (*m_pValue)--;
    }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
