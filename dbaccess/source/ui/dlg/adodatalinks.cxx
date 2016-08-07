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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"


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

#ifndef _DBAUI_ADO_DATALINK_HXX_
#include "adodatalinks.hxx"
#endif

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
