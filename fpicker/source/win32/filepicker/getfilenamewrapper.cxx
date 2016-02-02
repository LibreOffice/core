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

#include <stdio.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <sal/log.hxx>
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


    // This class prevents changing of the working
    // directory.

    class CurDirGuard
    {
        sal_Bool m_bValid;
        wchar_t* m_pBuffer;
        DWORD m_nBufLen;

    public:
        CurDirGuard()
        : m_bValid( sal_False )
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
            bool bDirSet = false;

            if ( m_pBuffer )
            {
                if ( m_bValid )
                {
                    if ( m_nBufLen - 1 > MAX_PATH )
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


    unsigned __stdcall ThreadProc(void* pParam)
    {
        osl_setThreadName("fpicker GetOpenFileName");

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


    // executes GetOpenFileName/GetSaveFileName in
    // a separate thread


    bool ThreadExecGetFileName(LPOPENFILENAME lpofn, bool bOpen, /*out*/ int& ExtErr)
    {
        GetFileNameParam gfnp(bOpen,lpofn);
        unsigned         id;

        HANDLE hThread = reinterpret_cast<HANDLE>(
            _beginthreadex(0, 0, ThreadProc, &gfnp, 0, &id));

        SAL_WARN_IF( !hThread, "fpicker", "could not create STA thread");

        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);

        ExtErr = gfnp.m_ExtErr;

        return gfnp.m_bRet;
    }


    // This function returns true if the calling
    // thread belongs to a Multithreaded Apartment
    // (MTA)


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


CGetFileNameWrapper::CGetFileNameWrapper() :
    m_ExtendedDialogError(0)
{
}


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


int CGetFileNameWrapper::commDlgExtendedError( )
{
    return m_ExtendedDialogError;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
