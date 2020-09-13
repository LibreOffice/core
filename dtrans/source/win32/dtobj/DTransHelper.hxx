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

#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <objidl.h>
#include <WinClip.hxx>

#define AUTO_INIT                 true

// a helper class to manage a global memory area, the clients can write
// into the global memory area and extract the handle to the global mem
// note: not thread-safe

class CStgTransferHelper
{
public:
    // will be thrown in case of failures
    class CStgTransferException
    {
    public:
        HRESULT m_hr;
        explicit CStgTransferException( HRESULT hr ) : m_hr( hr ) {};
    };

public:
    CStgTransferHelper(
        bool bAutoInit = false,
        HGLOBAL  hGlob = nullptr,
        bool bDelStgOnRelease = false );

    ~CStgTransferHelper( );

    void write( const void* lpData, ULONG cb, ULONG* cbWritten = nullptr );
    void read( LPVOID pv, ULONG cb, ULONG* pcbRead = nullptr );

    HGLOBAL getHGlobal( ) const;
    void    getIStream( LPSTREAM* ppStream );

    void init(
        SIZE_T newSize,
        sal_uInt32 uiFlags = GHND,
        bool bDelStgOnRelease = false );

    void init(
        HGLOBAL hGlob,
        bool bDelStgOnRelease = false );

    // returns the size of the managed memory
    sal_uInt32 memSize( CLIPFORMAT cf = CF_INVALID ) const;

    // free the global memory and necessary
    // release the internal stream pointer
    void cleanup( );

private:
    LPSTREAM m_lpStream;
    bool m_bDelStgOnRelease;

private:
    CStgTransferHelper( const CStgTransferHelper& );
    CStgTransferHelper& operator=( const CStgTransferHelper& );
};

// something like an auto-pointer - allows access to the memory belonging
// to a HGLOBAL and automatically unlocks a global memory at destruction
// time

class CRawHGlobalPtr
{
public:

    // ctor

    explicit CRawHGlobalPtr( HGLOBAL hGlob ) :
        m_hGlob( hGlob ),
        m_bIsLocked( false ),
        m_pGlobMem( nullptr )
    {
    }

    // ctor

    explicit CRawHGlobalPtr( const CStgTransferHelper& theHGlobalHelper ) :
        m_hGlob( theHGlobalHelper.getHGlobal( ) ),
        m_bIsLocked( false ),
        m_pGlobMem( nullptr )
    {
    }

    // dtor

    ~CRawHGlobalPtr( )
    {
        if ( m_bIsLocked )
            GlobalUnlock( m_hGlob );
    }

    // lock the global memory (initializes a
    // pointer to this memory)

    BOOL Lock( )
    {
        if ( !m_bIsLocked && ( nullptr != m_hGlob ) )
        {
            m_pGlobMem = GlobalLock( m_hGlob );
            m_bIsLocked = ( nullptr != m_pGlobMem );
        }

        return m_bIsLocked;
    }

    // unlock the global memory (invalidates the
    // pointer to this memory)

    BOOL Unlock( )
    {
        GlobalUnlock( m_hGlob );
        m_bIsLocked = false;
        m_pGlobMem = nullptr;

        return ( NO_ERROR == GetLastError( ) );
    }

    // locks the global memory and returns a
    // pointer to this memory

    LPVOID GetMemPtr( )
    {
        Lock( );
        return m_pGlobMem;
    }

    // size of mem we point to

    int MemSize( ) const
    {
        return GlobalSize( m_hGlob );
    }

private:
    HGLOBAL m_hGlob;
    bool    m_bIsLocked;
    LPVOID  m_pGlobMem;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
