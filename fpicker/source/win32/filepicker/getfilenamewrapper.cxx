/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: getfilenamewrapper.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 10:53:22 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _GETFILENAMEWRAPPER_HXX_
#include "getfilenamewrapper.hxx"
#endif

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

