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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"


#if defined(WNT)
#if defined _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable: 4917)
#endif
#include "msdasc.h"        // OLE DB Service Component header
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include "stdio.h"

#include <initguid.h>  // Include only once in your application
#include <adoid.h>     // needed for CLSID_CADOConnection
#include <adoint.h>    // needed for ADOConnection

#include "adodatalinks.hxx"

BSTR PromptEdit(long hWnd,BSTR connstr);
BSTR PromptNew(long hWnd);

::rtl::OUString getAdoDatalink(long hWnd,::rtl::OUString& oldLink)
{
    ::rtl::OUString dataLink;
    if (oldLink.getLength())
    {
        dataLink=reinterpret_cast<sal_Unicode *>(PromptEdit(hWnd,(BSTR)oldLink.getStr()));
    }
    else
        dataLink=reinterpret_cast<sal_Unicode *>(PromptNew(hWnd));
    return dataLink;
}
BSTR PromptNew(long hWnd)
{
    BSTR connstr=NULL;
    HRESULT hr;
    IDataSourceLocator* dlPrompt = NULL;
    ADOConnection* piTmpConnection = NULL;
    BSTR _result=NULL;

     // Initialize COM
     ::CoInitialize( NULL );

    // Instantiate DataLinks object.
      hr = CoCreateInstance(
                    CLSID_DataLinks,                //clsid -- Data Links UI
                    NULL,                           //pUnkOuter
                    CLSCTX_INPROC_SERVER,           //dwClsContext
                    IID_IDataSourceLocator,     //riid
                    (void**)&dlPrompt   //ppvObj
                    );
    if( FAILED( hr ) )
    {
        piTmpConnection->Release( );
        dlPrompt->Release( );
        return connstr;
    }

    dlPrompt->put_hWnd(hWnd);
    if( FAILED( hr ) )
    {
        piTmpConnection->Release( );
        dlPrompt->Release( );
        return connstr;
    }

    // Prompt for connection information.
    hr = dlPrompt->PromptNew((IDispatch **)&piTmpConnection);

    if( FAILED( hr ) || !piTmpConnection )
    {
        dlPrompt->Release( );
        return connstr;
    }

    hr = piTmpConnection->get_ConnectionString(&_result);
    if( FAILED( hr ) )
    {
        piTmpConnection->Release( );
        dlPrompt->Release( );
        return connstr;
    }

    piTmpConnection->Release( );
    dlPrompt->Release( );
    CoUninitialize();
    return _result;
}

BSTR PromptEdit(long hWnd,BSTR connstr)
{
    HRESULT hr;
    IDataSourceLocator* dlPrompt = NULL;
    ADOConnection* piTmpConnection = NULL;
    BSTR _result=NULL;

     // Initialize COM
     ::CoInitialize( NULL );

     hr = CoCreateInstance(CLSID_CADOConnection,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IADOConnection,
                (LPVOID *)&piTmpConnection);
    if( FAILED( hr ) )
    {
        piTmpConnection->Release( );
        return connstr;
    }


    hr = piTmpConnection->put_ConnectionString(connstr);
    if( FAILED( hr ) )
    {
        piTmpConnection->Release( );
        return connstr;
    }

    // Instantiate DataLinks object.
      hr = CoCreateInstance(
                    CLSID_DataLinks,                //clsid -- Data Links UI
                    NULL,                           //pUnkOuter
                    CLSCTX_INPROC_SERVER,           //dwClsContext
                    IID_IDataSourceLocator,     //riid
                    (void**)&dlPrompt   //ppvObj
                    );
    if( FAILED( hr ) )
    {
        piTmpConnection->Release( );
        dlPrompt->Release( );
        return connstr;
    }

    dlPrompt->put_hWnd(hWnd);
    if( FAILED( hr ) )
    {
        piTmpConnection->Release( );
        dlPrompt->Release( );
        return connstr;
    }

    VARIANT_BOOL pbSuccess;

    // Prompt for connection information.
    hr = dlPrompt->PromptEdit((IDispatch **)&piTmpConnection,&pbSuccess);
    if( SUCCEEDED( hr ) && sal_False == pbSuccess ) //if user press cancel then sal_False == pbSuccess
    {
        piTmpConnection->Release( );
        dlPrompt->Release( );
        return connstr;
    }

    if( FAILED( hr ) )
    {
        // Prompt for new connection information.
        piTmpConnection->Release( );
        piTmpConnection = NULL;
        hr = dlPrompt->PromptNew((IDispatch **)&piTmpConnection);
        if(  FAILED( hr ) || !piTmpConnection )
        {
            dlPrompt->Release( );
            return connstr;
        }
    }

    hr = piTmpConnection->get_ConnectionString(&_result);
    if( FAILED( hr ) )
    {
        piTmpConnection->Release( );
        dlPrompt->Release( );
        return connstr;
    }

    piTmpConnection->Release( );
    dlPrompt->Release( );
    CoUninitialize();
    return _result;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
