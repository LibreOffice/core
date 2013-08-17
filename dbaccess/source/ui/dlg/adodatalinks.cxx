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



#if defined(WNT)
#if defined _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable: 4917)
#endif
// LO/windows.h conflict
#undef WB_LEFT
#undef WB_RIGHT
#include "msdasc.h"        // OLE DB Service Component header
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include "stdio.h"

#include <initguid.h>  // Include only once in your application
#include <adoid.h>     // needed for CLSID_CADOConnection
#include <adoint.h>    // needed for ADOConnection

#include "adodatalinks.hxx"

#ifdef __MINGW32__
const IID IID_IDataSourceLocator = { 0x2206CCB2, 0x19C1, 0x11D1, { 0x89, 0xE0, 0x00, 0xC0, 0x4F, 0xD7, 0xA8, 0x29 } };
const CLSID CLSID_DataLinks = { 0x2206CDB2, 0x19C1, 0x11D1, { 0x89, 0xE0, 0x00, 0xC0, 0x4F, 0xD7, 0xA8, 0x29 } };
#endif


BSTR PromptEdit(long hWnd,BSTR connstr);
BSTR PromptNew(long hWnd);

OUString getAdoDatalink(long hWnd,OUString& oldLink)
{
    OUString dataLink;
    if (!oldLink.isEmpty())
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
