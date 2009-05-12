/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: getfilenamewrapper.cxx,v $
 * $Revision: 1.10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <stdio.h>
#include <osl/diagnose.h>
#include "getfilenamewrapper.hxx"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <objbase.h>
#include <process.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

namespace /* private */
{

    //-----------------------------------------------
    // This class prevents changing of the working
    // directory.
    //-----------------------------------------------
    class CurDirGuard
    {
        BOOL m_bValid;
        wchar_t* m_pBuffer;
        DWORD m_nBufLen;

    public:
        CurDirGuard()
        : m_bValid( FALSE )
        , m_pBuffer( NULL )
        , m_nBufLen( 0 )
        {
            m_nBufLen = GetCurrentDirectoryW( 0, NULL );
            if ( m_nBufLen )
            {
                m_pBuffer = new wchar_t[m_nBufLen];
                m_bValid = ( GetCurrentDirectoryW( m_nBufLen, m_pBuffer ) == ( m_nBufLen - 1 ) );
            }
        }

        ~CurDirGuard()
        {
            BOOL bDirSet = FALSE;

            if ( m_pBuffer )
            {
                if ( m_bValid )
                {
                    if ( m_nBufLen - 1 > MAX_PATH )
                    {
                        if ( (LONG32)GetVersion() < 0 )
                        {
                            // this is Win 98/ME branch, such a long path can not be set
                            // use the system path as fallback later
                        }
                        else
                        {
                            DWORD nNewLen = m_nBufLen + 8;
                            wchar_t* pNewBuffer = new wchar_t[nNewLen];
                            if ( m_nBufLen > 3 && m_pBuffer[0] == (wchar_t)'\\' && m_pBuffer[1] == (wchar_t)'\\' )
                            {
                                if ( m_pBuffer[2] == (wchar_t)'?' )
                                    _snwprintf( pNewBuffer, nNewLen, L"%s", m_pBuffer );
                                else
                                    _snwprintf( pNewBuffer, nNewLen, L"\\\\?\\UNC\\%s", m_pBuffer+2 );
                            }
                            else
                                _snwprintf( pNewBuffer, nNewLen, L"\\\\?\\%s", m_pBuffer );
                            bDirSet = SetCurrentDirectoryW( pNewBuffer );

                            delete [] pNewBuffer;
                        }
                    }
                    else
                        bDirSet = SetCurrentDirectoryW( m_pBuffer );
                }

                delete [] m_pBuffer;
                m_pBuffer = NULL;
            }

            if ( !bDirSet )
            {
                // the fallback solution
                wchar_t pPath[MAX_PATH+1];
                if ( GetWindowsDirectoryW( pPath, MAX_PATH+1 ) <= MAX_PATH )
                {
                    SetCurrentDirectoryW( pPath );
                }
                else
                {
                    // the system path is also too long?!!
                }
            }
        }
    };

    //-----------------------------------------------
    //
    //-----------------------------------------------

    struct GetFileNameParam
    {
        GetFileNameParam(bool bOpen, LPOPENFILENAME lpofn) :
            m_bOpen(bOpen),
            m_lpofn(lpofn),
            m_bRet(false),
            m_ExtErr(0)
        {}

        bool            m_bOpen;
        LPOPENFILENAME  m_lpofn;
        bool            m_bRet;
        int             m_ExtErr;
    };

    //-----------------------------------------------
    //
    //-----------------------------------------------

    unsigned __stdcall ThreadProc(void* pParam)
    {
        CurDirGuard aGuard;

        GetFileNameParam* lpgfnp =
            reinterpret_cast<GetFileNameParam*>(pParam);

        HRESULT hr = OleInitialize( NULL );

        if (lpgfnp->m_bOpen)
            lpgfnp->m_bRet = GetOpenFileName(lpgfnp->m_lpofn);
        else
            lpgfnp->m_bRet = GetSaveFileName(lpgfnp->m_lpofn);

        lpgfnp->m_ExtErr = CommDlgExtendedError();

        if ( SUCCEEDED( hr ) )
            OleUninitialize();

        return 0;
    }

    //-----------------------------------------------
    // exceutes GetOpenFileName/GetSaveFileName in
    // a separat thread
    //-----------------------------------------------

    bool ThreadExecGetFileName(LPOPENFILENAME lpofn, bool bOpen, /*out*/ int& ExtErr)
    {
        GetFileNameParam gfnp(bOpen,lpofn);
        unsigned         id;

        HANDLE hThread = reinterpret_cast<HANDLE>(
            _beginthreadex(0, 0, ThreadProc, &gfnp, 0, &id));

        OSL_POSTCOND(hThread, "could not create STA thread");

        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);

        ExtErr = gfnp.m_ExtErr;

        return gfnp.m_bRet;
    }

    //-----------------------------------------------
    // This function returns true if the calling
    // thread belongs to a Multithreaded Appartment
    // (MTA)
    //-----------------------------------------------

    bool IsMTA()
    {
        HRESULT hr = CoInitialize(NULL);

        if (RPC_E_CHANGED_MODE == hr)
            return true;

        if(SUCCEEDED(hr))
            CoUninitialize();

        return false;
    }

} // namespace private


//-----------------------------------------------
//
//-----------------------------------------------

CGetFileNameWrapper::CGetFileNameWrapper() :
    m_ExtendedDialogError(0)
{
}

//-----------------------------------------------
//
//-----------------------------------------------

bool CGetFileNameWrapper::getOpenFileName(LPOPENFILENAME lpofn)
{
    OSL_PRECOND(lpofn,"invalid parameter");

    bool bRet = false;

    if (IsMTA())
    {
        bRet = ThreadExecGetFileName(
            lpofn, true, m_ExtendedDialogError);
    }
    else
    {
        CurDirGuard aGuard;

        HRESULT hr = OleInitialize( NULL );

        bRet = GetOpenFileName(lpofn);
        m_ExtendedDialogError = CommDlgExtendedError();

        if ( SUCCEEDED( hr ) )
            OleUninitialize();
    }

    return bRet;
}

//-----------------------------------------------
//
//-----------------------------------------------

bool CGetFileNameWrapper::getSaveFileName(LPOPENFILENAME lpofn)
{
    OSL_PRECOND(lpofn,"invalid parameter");

    bool bRet = false;

    if (IsMTA())
    {
        bRet = ThreadExecGetFileName(
            lpofn, false, m_ExtendedDialogError);
    }
    else
    {
        CurDirGuard aGuard;

        bRet = GetSaveFileName(lpofn);
        m_ExtendedDialogError = CommDlgExtendedError();
    }

    return bRet;
}

//-----------------------------------------------
//
//-----------------------------------------------

int CGetFileNameWrapper::commDlgExtendedError( )
{
    return m_ExtendedDialogError;
}

