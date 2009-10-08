/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: smartpointer.hxx,v $
 *
 *  $Revision: 1.1.8.2 $
 *
 *  last change: $Author: mav $ $Date: 2008/10/30 11:59:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _INPROCSERV_SMARTPOINTER_HXX_
#define _INPROCSERV_SMARTPOINTER_HXX_

namespace inprocserv{

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

