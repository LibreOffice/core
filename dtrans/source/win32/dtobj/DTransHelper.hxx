/*************************************************************************
 *
 *  $RCSfile: DTransHelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: tra $ $Date: 2001-03-05 06:35:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _DTRANSHELPER_HXX_
#define _DTRANSHELPER_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <windows.h>

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define AUTO_INIT                 TRUE
#define NO_AUTO_INIT              FALSE
#define MEM_DESTROY_ON_RELEASE    TRUE
#define NO_MEM_DESTROY_ON_RELEASE FALSE

//------------------------------------------------------------------------
// deklarations
//------------------------------------------------------------------------


//-------------------------------------------------------------------------
// a helper class to manage a global memory area, the clients can write
// into the global memory area and extract the handle to the global mem
// note: not thread-safe
//-------------------------------------------------------------------------

class CStgTransferHelper
{
public:
    // will be thrown in case of failures
    class CStgTransferException
    {
    public:
        HRESULT m_hr;
        CStgTransferException( HRESULT hr ) : m_hr( hr ) {};
    };

public:
    CStgTransferHelper(
        sal_Bool bAutoInit = sal_False,
        HGLOBAL  hGlob = NULL,
        sal_Bool bDelStgOnRelease = sal_False,
        sal_Bool bReleaseStreamOnDestr = sal_True );

    ~CStgTransferHelper( );

    void SAL_CALL write( const void* lpData, ULONG cb, ULONG* cbWritten = NULL );
    void SAL_CALL read( LPVOID pv, ULONG cb, ULONG* pcbRead = NULL );

    HGLOBAL SAL_CALL getHGlobal( ) const;
    void    SAL_CALL getIStream( LPSTREAM* ppStream );

    void SAL_CALL init(
        SIZE_T newSize,
        sal_uInt32 uiFlags = GMEM_MOVEABLE | GMEM_ZEROINIT,
        sal_Bool bDelStgOnRelease = sal_False,
        sal_Bool bReleasStreamOnDestr = sal_True );

    void SAL_CALL init(
        HGLOBAL hGlob,
        sal_Bool bDelStgOnRelease = sal_False,
        sal_Bool bReleaseStreamOnDestr = sal_True );

    // returns the size of the managed memory
    sal_uInt32 SAL_CALL memSize( ) const;

    // free the global memory and necessary
    // release the internal stream pointer
    void SAL_CALL cleanup( );

private:
    LPSTREAM m_lpStream;
    sal_Bool m_bDelStgOnRelease;
    sal_Bool m_bReleaseStreamOnDestr;

private:
    CStgTransferHelper( const CStgTransferHelper& );
    CStgTransferHelper& operator=( const CStgTransferHelper& );
};

//-------------------------------------------------------------------------
// something like an auto-pointer - allows access to the memory belonging
// to a HGLOBAL and automatically unlocks a global memory at destruction
// time
//-------------------------------------------------------------------------

class CRawHGlobalPtr
{
public:

    //---------------------------------------------
    // ctor
    //---------------------------------------------

    CRawHGlobalPtr( HGLOBAL hGlob ) :
        m_hGlob( hGlob ),
        m_bIsLocked( FALSE ),
        m_pGlobMem( NULL )
    {
    }


    //---------------------------------------------
    // ctor
    //---------------------------------------------

    CRawHGlobalPtr( const CStgTransferHelper& theHGlobalHelper ) :
        m_hGlob( theHGlobalHelper.getHGlobal( ) ),
        m_bIsLocked( FALSE ),
        m_pGlobMem( NULL )
    {
    }

    //---------------------------------------------
    // dtor
    //---------------------------------------------

    ~CRawHGlobalPtr( )
    {
        if ( m_bIsLocked )
            GlobalUnlock( m_hGlob );
    }

    //---------------------------------------------
    // lock the global memory (initializes a
    // pointer to this memory)
    //---------------------------------------------

    BOOL Lock( )
    {
        if ( !m_bIsLocked && ( NULL != m_hGlob ) )
        {
            m_pGlobMem = GlobalLock( m_hGlob );
            m_bIsLocked = ( NULL != m_pGlobMem );
        }

        return m_bIsLocked;
    }

    //---------------------------------------------
    // unlock the global memory (invalidates the
    // pointer to this memory)
    //---------------------------------------------

    BOOL Unlock( )
    {
        GlobalUnlock( m_hGlob );
        m_bIsLocked = FALSE;
        m_pGlobMem = NULL;

        return ( NO_ERROR == GetLastError( ) );
    }

    //---------------------------------------------
    // locks the global memory and returns a
    // pointer to this memory
    //---------------------------------------------

    LPVOID GetMemPtr( )
    {
        Lock( );
        return m_pGlobMem;
    }

    //---------------------------------------------
    // size of mem we point to
    //---------------------------------------------

    int MemSize( ) const
    {
        return GlobalSize( m_hGlob );
    }

private:
    HGLOBAL m_hGlob;
    BOOL    m_bIsLocked;
    LPVOID  m_pGlobMem;
};

#endif