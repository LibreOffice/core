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


#if defined(_WIN32)
// LO/windows.h conflict
#undef WB_LEFT
#undef WB_RIGHT
#include <msdasc.h>

#include <o3tl/char16_t2wchar_t.hxx>

#include <initguid.h>
#include <adoid.h>
#include <adoint.h>

#include "adodatalinks.hxx"

namespace {

OUString PromptNew(long hWnd)
{
    HRESULT hr;
    IDataSourceLocator* dlPrompt = nullptr;
    ADOConnection* piTmpConnection = nullptr;
    BSTR _result=nullptr;

    // Initialize COM
    ::CoInitialize( nullptr );

    // Instantiate DataLinks object.
    hr = CoCreateInstance(
                    CLSID_DataLinks,                //clsid -- Data Links UI
                    nullptr,                        //pUnkOuter
                    CLSCTX_INPROC_SERVER,           //dwClsContext
                    IID_IDataSourceLocator,     //riid
                    reinterpret_cast<void**>(&dlPrompt)   //ppvObj
                    );
    if( FAILED( hr ) )
    {
        return OUString();
    }

    dlPrompt->put_hWnd(hWnd);
    if( FAILED( hr ) )
    {
        dlPrompt->Release( );
        return OUString();
    }

    // Prompt for connection information.
    hr = dlPrompt->PromptNew(reinterpret_cast<IDispatch **>(&piTmpConnection));

    if( FAILED( hr ) || !piTmpConnection )
    {
        dlPrompt->Release( );
        return OUString();
    }

    hr = piTmpConnection->get_ConnectionString(&_result);
    if( FAILED( hr ) )
    {
        piTmpConnection->Release( );
        dlPrompt->Release( );
        return OUString();
    }

    piTmpConnection->Release( );
    dlPrompt->Release( );
    CoUninitialize();
    // Don't we need SysFreeString(_result)?
    return o3tl::toU(_result);
}

OUString PromptEdit(long hWnd, OUString const & connstr)
{
    HRESULT hr;
    IDataSourceLocator* dlPrompt = nullptr;
    ADOConnection* piTmpConnection = nullptr;
    BSTR _result=nullptr;

    // Initialize COM
    ::CoInitialize( nullptr );

    hr = CoCreateInstance(CLSID_CADOConnection,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_IADOConnection,
                reinterpret_cast<LPVOID *>(&piTmpConnection));
    if( FAILED( hr ) )
    {
        piTmpConnection->Release( );
        return connstr;
    }


    hr = piTmpConnection->put_ConnectionString(
        const_cast<BSTR>(o3tl::toW(connstr.getStr())));
    if( FAILED( hr ) )
    {
        piTmpConnection->Release( );
        return connstr;
    }

    // Instantiate DataLinks object.
    hr = CoCreateInstance(
                    CLSID_DataLinks,                //clsid -- Data Links UI
                    nullptr,                        //pUnkOuter
                    CLSCTX_INPROC_SERVER,           //dwClsContext
                    IID_IDataSourceLocator,     //riid
                    reinterpret_cast<void**>(&dlPrompt) //ppvObj
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
    hr = dlPrompt->PromptEdit(reinterpret_cast<IDispatch **>(&piTmpConnection),&pbSuccess);
    if( SUCCEEDED( hr ) && !pbSuccess ) //if user press cancel then sal_False == pbSuccess
    {
        piTmpConnection->Release( );
        dlPrompt->Release( );
        return connstr;
    }

    if( FAILED( hr ) )
    {
        // Prompt for new connection information.
        piTmpConnection->Release( );
        piTmpConnection = nullptr;
        hr = dlPrompt->PromptNew(reinterpret_cast<IDispatch **>(&piTmpConnection));
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
    // Don't we need SysFreeString(_result)?
    return o3tl::toU(_result);
}

}

OUString getAdoDatalink(long hWnd,OUString const & oldLink)
{
    OUString dataLink;
    if (!oldLink.isEmpty())
    {
        dataLink=PromptEdit(hWnd,oldLink);
    }
    else
        dataLink=PromptNew(hWnd);
    return dataLink;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
