/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _INPROCSERV_SMARTPOINTER_HXX_
#define _INPROCSERV_SMARTPOINTER_HXX_

// #define OWNDEBUG

#ifdef OWNDEBUG
#define WRITEDEBUGINFOINTERN( x ) WriteDebugInfo( (DWORD)this, x, sizeof( x ) )
#define WRITEDEBUGINFO( x ) WRITEDEBUGINFOINTERN( x ":" MY_STRING_LINE "\n" )
#define TO_STRING( x ) #x
#define MACRO_VALUE_TO_STRING( x ) TO_STRING( x )
#define MY_STRING_LINE MACRO_VALUE_TO_STRING( __LINE__ )
#else
#define WRITEDEBUGINFO( x ) void()
#define MY_STRING_LINE
#endif


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

