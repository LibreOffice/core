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

#ifndef _INPROCSERV_SMARTPOINTER_HXX_
#define _INPROCSERV_SMARTPOINTER_HXX_

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

} // namespace inprocserv

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
