/*************************************************************************
 *
 *  $RCSfile: getfilenamewrapper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:04:59 $
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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _GETFILENAMEWRAPPER_HXX_
#include "getfilenamewrapper.hxx"
#endif

#include <objbase.h>
#include <process.h>


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

        if (lpgfnp->m_bOpen)
            lpgfnp->m_bRet = GetOpenFileName(lpgfnp->m_lpofn);
        else
            lpgfnp->m_bRet = GetSaveFileName(lpgfnp->m_lpofn);

        lpgfnp->m_ExtErr = CommDlgExtendedError();

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
        bRet = GetOpenFileName(lpofn);
        m_ExtendedDialogError = CommDlgExtendedError();
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

