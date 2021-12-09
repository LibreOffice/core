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

// SOActiveX.cpp : Implementation of CSOActiveX

#include "StdAfx2.h"
#include "SOActiveX.h"
#include "SOComWindowPeer.h"
#include "SODispatchInterceptor.h"
#include "SOActionsApproval.h"
#include "com_uno_helper.h"

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif
#include <so_activex.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif

#define STAROFFICE_WINDOWCLASS L"SOParentWindow"


static void OutputError_Impl( HWND hw, HRESULT ErrorCode )
{
    LPWSTR sMessage = nullptr;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        ErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        reinterpret_cast<LPWSTR>(&sMessage),
        0,
        nullptr
    );
    MessageBoxW( hw, sMessage, nullptr, MB_OK | MB_ICONINFORMATION );
    HeapFree( GetProcessHeap(), 0, sMessage );
}

HRESULT ExecuteFunc( IDispatch* idispUnoObject,
                     OLECHAR const * sFuncName,
                     CComVariant* params,
                     unsigned int count,
                     CComVariant* pResult )
{
    if( !idispUnoObject )
        return E_FAIL;

    DISPID id;
    HRESULT hr = idispUnoObject->GetIDsOfNames( IID_NULL, const_cast<OLECHAR **>(&sFuncName), 1, LOCALE_USER_DEFAULT, &id);
    if( !SUCCEEDED( hr ) ) return hr;

    DISPPARAMS dispparams= { params, nullptr, count, 0};

    // DEBUG
    EXCEPINFO myInfo;
    hr = idispUnoObject->Invoke( id, IID_NULL,LOCALE_USER_DEFAULT, DISPATCH_METHOD,
                    &dispparams, pResult, &myInfo, nullptr);

    // for debugging purposes
    // USES_CONVERSION;
    // if ( !SUCCEEDED( hr ) )
    //  ::MessageBox( NULL, OLE2A( myInfo.bstrDescription ), OLE2A( myInfo.bstrSource ), MB_OK | MB_ICONINFORMATION );

    return hr;
}

static HRESULT GetIDispByFunc( IDispatch* idispUnoObject,
                          OLECHAR const * sFuncName,
                          CComVariant* params,
                          unsigned int count,
                          CComPtr<IDispatch>& pdispResult )
{
    if( !idispUnoObject )
        return E_FAIL;

    CComVariant result;
    HRESULT hr = ExecuteFunc( idispUnoObject, sFuncName, params, count, &result );
    if( !SUCCEEDED( hr ) ) return hr;

    if( result.vt != VT_DISPATCH || result.pdispVal == nullptr )
        return E_FAIL;

    pdispResult = CComPtr<IDispatch>( result.pdispVal );

    return S_OK;
}

static HRESULT PutPropertiesToIDisp( IDispatch* pdispObject,
                              OLECHAR const ** sMemberNames,
                              CComVariant* pVariant,
                              unsigned int count )
{
    for( unsigned int ind = 0; ind < count; ind++ )
    {
        DISPID id;
        HRESULT hr = pdispObject->GetIDsOfNames( IID_NULL, const_cast<OLECHAR **>(&sMemberNames[ind]), 1, LOCALE_USER_DEFAULT, &id );
        if( !SUCCEEDED( hr ) ) return hr;

        hr = CComDispatchDriver::PutProperty( pdispObject, id, &pVariant[ind] );
        if( !SUCCEEDED( hr ) ) return hr;
    }

    return S_OK;
}

HRESULT GetPropertiesFromIDisp( IDispatch* pdispObject,
                                OLECHAR const ** sMemberNames,
                                CComVariant* pVariant,
                                unsigned int count )
{
    for( unsigned int ind = 0; ind < count; ind++ )
    {
        DISPID id;
        HRESULT hr = pdispObject->GetIDsOfNames( IID_NULL, const_cast<OLECHAR **>(&sMemberNames[ind]), 1, LOCALE_USER_DEFAULT, &id );
        if( !SUCCEEDED( hr ) ) return hr;

        hr = CComDispatchDriver::GetProperty( pdispObject, id, &pVariant[ind] );
        if( !SUCCEEDED( hr ) ) return hr;
    }

    return S_OK;
}

// CSOActiveX

CSOActiveX::CSOActiveX()
: mCookie(0)
, mCurFileUrl( L"private:factory/swriter" )
, mbLoad( FALSE )
, mbViewOnly( TRUE )
, mParentWin( nullptr )
, mOffWin( nullptr )
, mpDispatchInterceptor( nullptr )
, mnVersion( SO_NOT_DETECTED )
, mbReadyForActivation( FALSE )
, mbDrawLocked( false )
{
    CLSID const clsFactory = {0x82154420,0x0FBF,0x11d4,{0x83, 0x13,0x00,0x50,0x04,0x52,0x6A,0xB4}};
    HRESULT hr = CoCreateInstance( clsFactory, nullptr, CLSCTX_ALL, __uuidof(IDispatch), reinterpret_cast<void**>(&mpDispFactory));
    if( !SUCCEEDED( hr ) )
        OutputError_Impl( nullptr, hr );

    mPWinClass.style            = CS_HREDRAW|CS_VREDRAW;
    mPWinClass.lpfnWndProc      = DefWindowProcW;
    mPWinClass.cbClsExtra       = 0;
    mPWinClass.cbWndExtra       = 0;
    mPWinClass.hInstance        = GetModuleHandleW(nullptr); //myInstance;
    mPWinClass.hIcon            = nullptr;
    mPWinClass.hCursor          = nullptr;
    mPWinClass.hbrBackground    = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND);
    mPWinClass.lpszMenuName     = nullptr;
    mPWinClass.lpszClassName    = STAROFFICE_WINDOWCLASS;

    RegisterClassW(&mPWinClass);
}

CSOActiveX::~CSOActiveX()
{
    Cleanup();

}

HRESULT CSOActiveX::Cleanup()
{
    CComVariant dummyResult;

    if( mpDispatchInterceptor )
    {
        if( mpDispFrame )
        {
            // remove dispatch interceptor
            CComQIPtr< IDispatch, &IID_IDispatch > pIDispDispInter( mpDispatchInterceptor );
            CComVariant aVariant( pIDispDispInter );
            ExecuteFunc( mpDispFrame,
                         L"releaseDispatchProviderInterceptor",
                         &aVariant,
                         1,
                         &dummyResult );
        }

        mpDispatchInterceptor->ClearParent();
        mpDispatchInterceptor->Release();
        mpDispatchInterceptor = nullptr;
    }

    mpDispTempFile = CComPtr< IDispatch >();
    mbReadyForActivation = FALSE;

    if( mpInstanceLocker )
    {
        ExecuteFunc( mpInstanceLocker, L"dispose", nullptr, 0, &dummyResult );
        mpInstanceLocker = CComPtr< IDispatch >();
    }

    if( mpDispFrame )
    {
        bool bCloserActivated = false;

        CComPtr<IDispatch> pDispDocumentCloser;
        CComVariant aDocCloser( L"com.sun.star.embed.DocumentCloser" );
        HRESULT hr = GetIDispByFunc( mpDispFactory,
                                     L"createInstance",
                                     &aDocCloser,
                                     1,
                                     pDispDocumentCloser );
        if ( SUCCEEDED( hr ) && pDispDocumentCloser )
        {
            SAFEARRAY* pInitFrame = SafeArrayCreateVector(VT_VARIANT, 0, 1);
            LONG nInitInd = 0;
            CComVariant pFrameVariant( mpDispFrame );
            SafeArrayPutElement( pInitFrame, &nInitInd, &pFrameVariant );
            CComVariant aVarInitFrame;
            aVarInitFrame.vt = VT_ARRAY | VT_VARIANT; aVarInitFrame.parray = pInitFrame;
            hr = ExecuteFunc( pDispDocumentCloser, L"initialize", &aVarInitFrame, 1, &dummyResult );
            if( SUCCEEDED( hr ) )
            {
                // the following call will let the closing happen
                hr = ExecuteFunc( pDispDocumentCloser, L"dispose", nullptr, 0, &dummyResult );
                bCloserActivated = SUCCEEDED( hr );
            }
        }

        if ( !bCloserActivated )
        {
            CComVariant aPropVar;
            aPropVar.vt = VT_BOOL; aPropVar.boolVal = VARIANT_TRUE;
            if ( !SUCCEEDED( ExecuteFunc( mpDispFrame, L"close", &aPropVar, 1, &dummyResult ) ) )
                ExecuteFunc( mpDispFrame, L"dispose", nullptr, 0, &dummyResult );
        }

        mpDispFrame = CComPtr< IDispatch >();
    }

    if( ::IsWindow( mOffWin ) )
        ::DestroyWindow( mOffWin );

    TerminateOffice();

    return S_OK;
}

HRESULT CSOActiveX::TerminateOffice()
{
    // create desktop
    CComPtr<IDispatch> pdispDesktop;
    CComVariant aDesktopServiceName( L"com.sun.star.frame.Desktop" );

    HRESULT hr = GetIDispByFunc( mpDispFactory, L"createInstance", &aDesktopServiceName, 1, pdispDesktop );
    if( !pdispDesktop || !SUCCEEDED( hr ) ) return hr;

    // create tree of frames
    CComPtr<IDispatch> pdispChildren;
    hr = GetIDispByFunc( pdispDesktop, L"getFrames", nullptr, 0, pdispChildren );
    if( !pdispChildren || !SUCCEEDED( hr ) ) return hr;

    CComVariant aFrames;
    CComVariant nFlag( 4 );
    hr = ExecuteFunc( pdispChildren, L"queryFrames", &nFlag, 1, &aFrames );
    if ( SUCCEEDED( hr ) )
    {
        if ( ( aFrames.vt == ( VT_ARRAY | VT_DISPATCH ) || aFrames.vt == ( VT_ARRAY | VT_VARIANT ) )
          && ( !aFrames.parray || (aFrames.parray->cDims == 1 && aFrames.parray->rgsabound[0].cElements == 0) ) )
        {
            // there is no frames open
            // TODO: check whether the frames are hidden if they are open?
            CComVariant dummyResult;
            hr = ExecuteFunc( pdispDesktop, L"terminate", nullptr, 0, &dummyResult );
        }
    }

    return hr;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CSOActiveX::InitNew ()
{
    mnVersion = GetVersionConnected();
    mbLoad = TRUE;
    return S_OK;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CSOActiveX::Load ( LPSTREAM /*pStm*/ )
{
    mnVersion = GetVersionConnected();
    mbLoad = TRUE;

    // may be later?
    // for now just ignore

    return S_OK;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CSOActiveX::Load( LPPROPERTYBAG pPropBag, LPERRORLOG /*pErrorLog*/ )
{
    mnVersion = GetVersionConnected();

    IPropertyBag2* pPropBag2;
    HRESULT hr = pPropBag->QueryInterface( IID_IPropertyBag2, reinterpret_cast<void**>(&pPropBag2) );
    //ATLASSERT( hr >= 0 );

    if( !SUCCEEDED( hr ) )
        return hr;

    unsigned long aNum;
    hr = pPropBag2->CountProperties( &aNum );
    //ATLASSERT( hr >= 0 );
    if( !SUCCEEDED( hr ) )
        return hr;

    PROPBAG2* aPropNames = new PROPBAG2[aNum];
    unsigned long aReaded;

    hr = pPropBag2->GetPropertyInfo( 0,
                                     aNum,
                                     aPropNames,
                                     &aReaded );
    //ATLASSERT( hr >= 0 );
    if( !SUCCEEDED( hr ) )
    {
        delete[] aPropNames;
        return hr;
    }

    CComVariant* aVal = new CComVariant[aNum];
    HRESULT*     hvs = new HRESULT[aNum];
    hr = pPropBag2->Read( aNum,
                          aPropNames,
                          nullptr,
                          aVal,
                          hvs );
    //ATLASSERT( hr >= 0 );
    if( !SUCCEEDED( hr ) )
    {
        delete[] hvs;
        delete[] aVal;
        delete[] aPropNames;
        return hr;
    }

    for( unsigned long ind = 0; ind < aNum; ind++ )
    {
        // all information from the 'object' tag is in strings
        if (aVal[ind].vt == VT_BSTR && !wcscmp(aPropNames[ind].pstrName, L"src"))
        {
            mCurFileUrl = wcsdup( aVal[ind].bstrVal );
        }
        else if( aVal[ind].vt == VT_BSTR
                && !wcscmp(aPropNames[ind].pstrName, L"readonly"))
        {
            if (!wcscmp(aVal[ind].bstrVal, L"true"))
            {
                // the default value
                mbViewOnly = TRUE;
            }
            else
            {
                mbViewOnly = FALSE;
            }
        }
    }

    delete[] hvs;
    delete[] aVal;
    delete[] aPropNames;

    if( !mpDispFactory )
        return hr;

    mbReadyForActivation = FALSE;
    if (BSTR bStrUrl = SysAllocString(mCurFileUrl))
    {
        hr = CBindStatusCallback<CSOActiveX>::Download(
            this, &CSOActiveX::CallbackCreateXInputStream, bStrUrl, m_spClientSite, FALSE);
        SysFreeString(bStrUrl);
        if (hr == MK_S_ASYNCHRONOUS)
            hr = S_OK;
    }
    else
        hr = E_OUTOFMEMORY;

    if ( !SUCCEEDED( hr ) )
    {
        // trigger initialization without stream
        mbLoad = TRUE;

        Invalidate();
        UpdateWindow();
    }

    return hr;
}

HRESULT CSOActiveX::GetUnoStruct( OLECHAR const * sStructName, CComPtr<IDispatch>& pdispResult )
{
    CComVariant aComStruct( sStructName );
    return GetIDispByFunc( mpDispFactory, L"Bridge_GetStruct", &aComStruct, 1, pdispResult );
}

HRESULT CSOActiveX::GetUrlStruct( OLECHAR const * sUrl, CComPtr<IDispatch>& pdispUrl )
{
    HRESULT hr = GetUnoStruct( L"com.sun.star.util.URL", pdispUrl );
    if( !SUCCEEDED( hr ) ) return hr;

    OLECHAR const * sURLMemberName = L"Complete";
    DISPID nURLID;
    hr = pdispUrl->GetIDsOfNames( IID_NULL, const_cast<OLECHAR **>(&sURLMemberName), 1, LOCALE_USER_DEFAULT, &nURLID );
    if( !SUCCEEDED( hr ) ) return hr;
    CComVariant aComUrl( sUrl );
    hr = CComDispatchDriver::PutProperty( pdispUrl, nURLID, &aComUrl );
    if( !SUCCEEDED( hr ) ) return hr;

    CComPtr<IDispatch> pdispTransformer;
    CComVariant aServiceName( L"com.sun.star.util.URLTransformer" );
    hr = GetIDispByFunc( mpDispFactory,
                         L"createInstance",
                         &aServiceName,
                         1,
                         pdispTransformer );
    if( !SUCCEEDED( hr ) ) return hr;

    CComVariant dummyResult;
    CComVariant aParam[2];
    aParam[1].ppdispVal = &pdispUrl;
    aParam[1].vt = VT_DISPATCH | VT_BYREF;
    aParam[0] = CComVariant( L"file:///" );

    hr = ExecuteFunc( pdispTransformer, L"parseSmart", aParam, 2, &dummyResult );
    if( !SUCCEEDED( hr ) || dummyResult.vt != VT_BOOL || !dummyResult.boolVal ) return hr;

    return S_OK;
}

HRESULT CSOActiveX::SetLayoutManagerProps()
{
    if ( !mpDispFrame )
        return E_FAIL;

    CComVariant pVarLayoutMgr;
    OLECHAR const * sLMPropName = L"LayoutManager";
    HRESULT hr = GetPropertiesFromIDisp( mpDispFrame, &sLMPropName, &pVarLayoutMgr, 1 );
    if( pVarLayoutMgr.vt != VT_DISPATCH || pVarLayoutMgr.pdispVal == nullptr )
        return E_FAIL;

    CComPtr<IDispatch> pdispLM( pVarLayoutMgr.pdispVal );


    if( !SUCCEEDED( hr ) || !pdispLM )
        return E_FAIL;

    OLECHAR const * sATName = L"AutomaticToolbars";
    CComVariant pATProp;
    pATProp.vt = VT_BOOL; pATProp.boolVal = VARIANT_FALSE ;
    hr = PutPropertiesToIDisp( pdispLM, &sATName, &pATProp, 1 );

    return hr;
}

HRESULT CSOActiveX::CreateFrameOldWay( HWND hwnd, int width, int height )
{
    if( !mpDispFactory )
        return E_FAIL;

    // create window handle holder
    CComPtr< CComObject< SOComWindowPeer > > pPeerToSend = new CComObject<SOComWindowPeer>();
    pPeerToSend->SetHWNDInternally( hwnd );
    CComQIPtr< IDispatch, &IID_IDispatch > pIDispToSend( pPeerToSend );

    // create rectangle structure
    CComPtr<IDispatch> pdispRectangle;
    HRESULT hr = GetUnoStruct( L"com.sun.star.awt.Rectangle", pdispRectangle );
    if( !SUCCEEDED( hr ) ) return hr;

    OLECHAR const * sRectMemberNames[4] = { L"X",
                                      L"Y",
                                      L"Width",
                                      L"Height" };
    CComVariant pRectVariant[4];
    pRectVariant[0] = pRectVariant[1] = pRectVariant[2] = pRectVariant[3] = CComVariant( 0 );

    hr = PutPropertiesToIDisp( pdispRectangle, sRectMemberNames, pRectVariant, 4 );
    if( !SUCCEEDED( hr ) ) return hr;

    // create WindowDescriptor structure
    CComPtr<IDispatch> pdispWinDescr;
    hr = GetUnoStruct( L"com.sun.star.awt.WindowDescriptor", pdispWinDescr );
    if( !SUCCEEDED( hr ) ) return hr;

    // fill in descriptor with info
    OLECHAR const * sDescriptorMemberNames[6] = { L"Type",
                                 L"WindowServiceName",
                                 L"ParentIndex",
                                 L"Parent",
                                 L"Bounds",
                                 L"WindowAttributes" };
    CComVariant pDescriptorVar[6];
    pDescriptorVar[0] = CComVariant( 0 );
    pDescriptorVar[1] = CComVariant( L"workwindow" );
    pDescriptorVar[2] = CComVariant( 1 );
    pDescriptorVar[3] = CComVariant( pIDispToSend );
    pDescriptorVar[4] = CComVariant( pdispRectangle );
    pDescriptorVar[5] = CComVariant( 33 );
    hr = PutPropertiesToIDisp( pdispWinDescr, sDescriptorMemberNames, pDescriptorVar, 6 );
    if( !SUCCEEDED( hr ) ) return hr;

    // create XToolkit instance
    CComPtr<IDispatch> pdispToolkit;
    CComVariant aServiceName( L"com.sun.star.awt.Toolkit" );
    hr = GetIDispByFunc( mpDispFactory, L"createInstance", &aServiceName, 1, pdispToolkit );
    if( !SUCCEEDED( hr ) ) return hr;

    // create window with toolkit
    CComVariant aWinDescr( pdispWinDescr );
    hr = GetIDispByFunc( pdispToolkit, L"createWindow", &aWinDescr, 1, mpDispWin );
    if( !SUCCEEDED( hr ) ) return hr;

    // create frame
    aServiceName = CComVariant( L"com.sun.star.frame.Task" );
    hr = GetIDispByFunc( mpDispFactory, L"createInstance", &aServiceName, 1, mpDispFrame );
    if( !SUCCEEDED( hr ) || !mpDispFrame )
    {
        // the interface com.sun.star.frame.Task is removed in 6.1
        // but the interface com.sun.star.frame.Frame has some bugs in 6.0
        aServiceName = CComVariant( L"com.sun.star.frame.Frame" );
        hr = GetIDispByFunc( mpDispFactory, L"createInstance", &aServiceName, 1, mpDispFrame );
        if( !SUCCEEDED( hr ) ) return hr;
    }

    // initialize frame
    CComVariant dummyResult;
    CComVariant aDispWin( mpDispWin );
    hr = ExecuteFunc( mpDispFrame, L"initialize", &aDispWin, 1, &dummyResult );
    if( !SUCCEEDED( hr ) ) return hr;

    // set some properties to the layout manager, ignore errors for now
    SetLayoutManagerProps();

    // create desktop
    CComPtr<IDispatch> pdispDesktop;
    aServiceName = CComVariant( L"com.sun.star.frame.Desktop" );
    hr = GetIDispByFunc( mpDispFactory, L"createInstance", &aServiceName, 1, pdispDesktop );
    if( !SUCCEEDED( hr ) ) return hr;

    // create tree of frames
    CComPtr<IDispatch> pdispChildren;
    hr = GetIDispByFunc( pdispDesktop, L"getFrames", nullptr, 0, pdispChildren );
    if( !SUCCEEDED( hr ) ) return hr;

    // insert new frame into desktop hierarchy
    CComVariant aDispFrame( mpDispFrame );
    hr = ExecuteFunc( pdispChildren, L"append", &aDispFrame, 1, &dummyResult );
    if( !SUCCEEDED( hr ) ) return hr;

    // initialize window
    CComVariant aTransparent( long(0xFFFFFFFF) );
    hr = ExecuteFunc( mpDispWin, L"setBackground", &aTransparent, 1, &dummyResult );
    if( !SUCCEEDED( hr ) ) return hr;

    CComVariant aTrue( TRUE );
    hr = ExecuteFunc( mpDispWin, L"setVisible", &aTrue, 1, &dummyResult );
    if( !SUCCEEDED( hr ) ) return hr;

    CComVariant aPosArgs[5];
    aPosArgs[4] = CComVariant( 0 );
    aPosArgs[3] = CComVariant( 0 );
    aPosArgs[2] = CComVariant( width );
    aPosArgs[1] = CComVariant( height );
    aPosArgs[0] = CComVariant( 12 );
    hr = ExecuteFunc( mpDispWin, L"setPosSize", aPosArgs, 5, &dummyResult );
    if( !SUCCEEDED( hr ) ) return hr;

    // create frame locker if there is such service
    aServiceName = CComVariant( L"com.sun.star.embed.InstanceLocker" );
    hr = GetIDispByFunc( mpDispFactory, L"createInstance", &aServiceName, 1, mpInstanceLocker );
    if( SUCCEEDED( hr ) && mpInstanceLocker )
    {
        SAFEARRAY* pInitVals = SafeArrayCreateVector(VT_VARIANT, 0, 3);

        // the first sequence element
        LONG nInitInd = 0;
        CComVariant pFrameVariant( mpDispFrame );
        SafeArrayPutElement( pInitVals, &nInitInd, &pFrameVariant );

        // the second sequence element
        nInitInd = 1;
        CComVariant pStrArr( 1 );
        SafeArrayPutElement( pInitVals, &nInitInd, &pStrArr );

        // the third sequence element
        nInitInd = 2;
        CComPtr<IDispatch> pdispValueObj;
        hr = GetIDispByFunc( mpDispFactory, L"Bridge_GetValueObject", nullptr, 0, pdispValueObj );
        if( !SUCCEEDED( hr ) || !pdispValueObj ) return hr;

        CComVariant aValueArgs[2];
        aValueArgs[1] = CComVariant( L"com.sun.star.embed.XActionsApproval" );
        CComPtr< CComObject< SOActionsApproval > > pApproval( new CComObject<SOActionsApproval>() );
        aValueArgs[0] = CComVariant ( pApproval );

        hr = ExecuteFunc( pdispValueObj, L"Set", aValueArgs, 2, &dummyResult );
        if( !SUCCEEDED( hr ) ) return hr;

        CComVariant aValueObj( pdispValueObj );
        SafeArrayPutElement( pInitVals, &nInitInd, &aValueObj );

        // execute initialize()
        CComVariant aVarInitVals;
        aVarInitVals.vt = VT_ARRAY | VT_VARIANT; aVarInitVals.parray = pInitVals;
        hr = ExecuteFunc( mpInstanceLocker, L"initialize", &aVarInitVals, 1, &dummyResult );
        if( !SUCCEEDED( hr ) ) return hr;
    }

    return S_OK;
}

HRESULT CSOActiveX::CallLoadComponentFromURL1PBool( OLECHAR const * sUrl, OLECHAR const * sArgName, BOOL sArgVal )
{
    SAFEARRAY* pPropVals = SafeArrayCreateVector(VT_DISPATCH, 0, 1);
    LONG ix = 0;
    CComPtr<IDispatch> pdispPropVal;
    HRESULT hr = GetUnoStruct( L"com.sun.star.beans.PropertyValue", pdispPropVal );
    if( !SUCCEEDED( hr ) ) return hr;

    OLECHAR const * sPropMemberNames[2] = { L"Name", L"Value" };
    CComVariant pPropVar[2];
    pPropVar[0] = CComVariant( sArgName );
    pPropVar[1].vt = VT_BOOL; pPropVar[1].boolVal = sArgVal ? VARIANT_TRUE : VARIANT_FALSE ;
    hr = PutPropertiesToIDisp( pdispPropVal, sPropMemberNames, pPropVar, 2 );
    if( !SUCCEEDED( hr ) ) return hr;

    SafeArrayPutElement( pPropVals, &ix, pdispPropVal );

    CComVariant aDispArgs[4];
    aDispArgs[3] = CComVariant( sUrl );
    aDispArgs[2] = CComVariant( L"_self" );
    aDispArgs[1] = CComVariant( 0 );
    // aDispArgs[0] = CComVariant( pPropVals ); such constructor is not defined ??!
    aDispArgs[0].vt = VT_ARRAY | VT_DISPATCH; aDispArgs[0].parray = pPropVals;

    CComVariant dummyResult;
    hr = ExecuteFunc( mpDispFrame, L"loadComponentFromURL", aDispArgs, 4, &dummyResult );
    if( !SUCCEEDED( hr ) ) return hr;

    return S_OK;
}

HRESULT CSOActiveX::CallDispatchMethod( OLECHAR const * sUrl,
                                        CComVariant* aArgNames,
                                        CComVariant* aArgVals,
                                        unsigned int count )
{
    CComPtr<IDispatch> pdispURL;
    HRESULT hr = GetUrlStruct( sUrl, pdispURL );
    if( !SUCCEEDED( hr ) ) return hr;

    CComPtr<IDispatch> pdispXDispatch;
    CComVariant aArgs[3];
    aArgs[2] = CComVariant( pdispURL );
    aArgs[1] = CComVariant( L"" );
    aArgs[0] = CComVariant( int(0) );
    hr = GetIDispByFunc( mpDispFrame,
                         L"queryDispatch",
                         aArgs,
                         3,
                         pdispXDispatch );
    if( !SUCCEEDED( hr ) ) return hr;

    SAFEARRAY* pPropVals = SafeArrayCreateVector(VT_DISPATCH, 0, count);
    for( LONG ix = 0; ix < static_cast<LONG>(count); ix ++ )
    {
        CComPtr<IDispatch> pdispPropVal;
        hr = GetUnoStruct( L"com.sun.star.beans.PropertyValue", pdispPropVal );
        if( !SUCCEEDED( hr ) ) return hr;

        OLECHAR const * sPropMemberNames[2] = { L"Name", L"Value" };
        CComVariant pPropVar[2];
        pPropVar[0] = aArgNames[ix];
        pPropVar[1] = aArgVals[ix];
        hr = PutPropertiesToIDisp( pdispPropVal, sPropMemberNames, pPropVar, 2 );
        if( !SUCCEEDED( hr ) ) return hr;

        SafeArrayPutElement( pPropVals, &ix, pdispPropVal );
    }

    CComVariant aDispArgs[2];
    aDispArgs[1] = CComVariant( pdispURL );
    // aDispArgs[0] = CComVariant( pPropVals ); such constructor is not defined ??!
    aDispArgs[0].vt = VT_ARRAY | VT_DISPATCH; aDispArgs[0].parray = pPropVals;

    CComVariant dummyResult;
    hr = ExecuteFunc( pdispXDispatch, L"dispatch", aDispArgs, 2, &dummyResult );
    if( !SUCCEEDED( hr ) ) return hr;

    return S_OK;
}

void CSOActiveX::CallbackCreateXInputStream( CBindStatusCallback<CSOActiveX>* /*pbsc*/, BYTE* pBytes, DWORD dwSize )
{
    if ( mbReadyForActivation )
        return;

    bool bSuccess = false;
    bool bFinishDownload = false;
    if ( !pBytes )
    {
        // means the download is finished, dwSize contains hresult
        bFinishDownload = true;
        if ( SUCCEEDED( dwSize ) )
            bSuccess = true;
    }
    else
    {
        HRESULT hr = S_OK;

        if ( !mpDispTempFile )
        {
            CComVariant aServiceName( L"com.sun.star.io.TempFile" );
            hr = GetIDispByFunc( mpDispFactory,
                                 L"createInstance",
                                 &aServiceName,
                                 1,
                                 mpDispTempFile );
        }

        if( SUCCEEDED( hr ) && mpDispTempFile )
        {
            SAFEARRAY* pDataArray = SafeArrayCreateVector(VT_I1, 0, dwSize);

            if ( pDataArray )
            {
                hr = SafeArrayLock( pDataArray );
                if ( SUCCEEDED( hr ) )
                {
                    for( DWORD ix = 0; ix < dwSize; ix++ )
                        static_cast<BYTE*>(pDataArray->pvData)[ix] = pBytes[ix];
                    hr = SafeArrayUnlock( pDataArray );
                    if ( SUCCEEDED( hr ) )
                    {
                        CComVariant aArgs[1];
                        aArgs[0].vt = VT_ARRAY | VT_I1; aArgs[0].parray = pDataArray;
                        CComVariant dummyResult;
                        hr = ExecuteFunc( mpDispTempFile, L"writeBytes", aArgs, 1, &dummyResult );
                        if( SUCCEEDED( hr ) )
                            bSuccess = true;
                    }
                }
            }
        }
    }

    if ( !bSuccess )
    {
        // the download failed, let StarOffice download
        bFinishDownload = true;
        mpDispTempFile = CComPtr< IDispatch >();
    }

    if ( bFinishDownload )
    {
        // trigger the loading now
        mbLoad = TRUE;
        mbReadyForActivation = TRUE;

        Invalidate();
        UpdateWindow();
    }
}

HRESULT CSOActiveX::LoadURLToFrame( )
{
    CComVariant aArgNames[4] = { L"ReadOnly", L"ViewOnly", L"AsTemplate", L"InputStream" };
    CComVariant aArgVals[4];
    unsigned int nCount = 3; // the 4-th argument is used only if the stream can be retrieved

    aArgVals[0].vt = VT_BOOL; aArgVals[0].boolVal = mbViewOnly ? VARIANT_TRUE : VARIANT_FALSE;
    aArgVals[1].vt = VT_BOOL; aArgVals[1].boolVal = mbViewOnly ? VARIANT_TRUE : VARIANT_FALSE;
    aArgVals[2].vt = VT_BOOL; aArgVals[2].boolVal = VARIANT_FALSE;

    if ( mpDispTempFile )
    {
        aArgVals[3] = CComVariant( mpDispTempFile );
        nCount = 4;
    }

    HRESULT hr = CallDispatchMethod( mCurFileUrl, aArgNames, aArgVals, nCount );
    if( !SUCCEEDED( hr ) ) return hr;

    // try to get the model and set the presentation specific property, the setting will fail for other document formats
    CComPtr<IDispatch> pdispController;
    hr = GetIDispByFunc( mpDispFrame, L"getController", nullptr, 0, pdispController );
    if ( SUCCEEDED( hr ) && pdispController )
    {
        CComPtr<IDispatch> pdispModel;
        hr = GetIDispByFunc( pdispController, L"getModel", nullptr, 0, pdispModel );
        if ( SUCCEEDED( hr ) && pdispModel )
        {
            CComPtr<IDispatch> pdispPres;
            hr = GetIDispByFunc( pdispModel, L"getPresentation", nullptr, 0, pdispPres );
            if ( SUCCEEDED( hr ) && pdispPres )
            {
                // this is a presentation
                // let the slide show be shown in the document window
                OLECHAR const * pPropName = L"IsFullScreen";
                CComVariant pPresProp;
                pPresProp.vt = VT_BOOL; pPresProp.boolVal = VARIANT_FALSE ;
                hr = PutPropertiesToIDisp( pdispPres, &pPropName, &pPresProp, 1 );

                // start the slide show
                if ( SUCCEEDED( hr ) )
                {
                    CComVariant dummyResult;
                    ExecuteFunc( pdispPres, L"Start", nullptr, 0, &dummyResult );
                }
            }
        }
    }

    // create dispatch interceptor
    mpDispatchInterceptor = new CComObject< SODispatchInterceptor >();
    mpDispatchInterceptor->AddRef();
    mpDispatchInterceptor->SetParent( this );
    CComQIPtr< IDispatch, &IID_IDispatch > pIDispDispInter( mpDispatchInterceptor );

    // register dispatch interceptor in the frame
    CComVariant aDispVariant( pIDispDispInter );
    CComVariant dummyResult;
    hr = ExecuteFunc( mpDispFrame,
                      L"registerDispatchProviderInterceptor",
                                          &aDispVariant,
                      1,
                      &dummyResult );

    if( !SUCCEEDED( hr ) ) return hr;

    return S_OK;
}

SOVersion CSOActiveX::GetVersionConnected()
{
    SOVersion bResult = SO_NOT_DETECTED;
    if( mpDispFactory )
    {
        // create ConfigurationProvider instance
        CComPtr<IDispatch> pdispConfProv;
        CComVariant aServiceName( L"com.sun.star.configuration.ConfigurationProvider" );
        HRESULT hr = GetIDispByFunc( mpDispFactory,
                             L"createInstance",
                             &aServiceName,
                             1,
                             pdispConfProv );

        if( SUCCEEDED( hr ) && pdispConfProv )
        {
            CComPtr<IDispatch> pdispConfAccess;

            SAFEARRAY* pInitParams = SafeArrayCreateVector( VT_VARIANT, 0, 1 );

            if( pInitParams )
            {
                LONG ix = 0;
                CComVariant aConfPath( L"org.openoffice.Setup" );
                SafeArrayPutElement( pInitParams, &ix, &aConfPath );

                CComVariant aArgs[2];
                aArgs[1] = CComVariant( L"com.sun.star.configuration.ConfigurationAccess" );
                aArgs[0].vt = VT_ARRAY | VT_VARIANT; aArgs[0].parray = pInitParams;

                hr = GetIDispByFunc( pdispConfProv,
                                     L"createInstanceWithArguments",
                                    aArgs,
                                     2,
                                     pdispConfAccess );

                if( SUCCEEDED( hr ) && pdispConfAccess )
                {
                    CComVariant aOfficeName;

                    CComVariant aProductName( L"Product/ooName" );
                    hr = ExecuteFunc( pdispConfAccess,
                                        L"getByHierarchicalName",
                                        &aProductName,
                                        1,
                                        &aOfficeName );

                    if( SUCCEEDED( hr ) && aOfficeName.vt == VT_BSTR )
                    {
                        CComVariant aOfficeVersion;

                        CComVariant aProductVersion( L"Product/ooSetupVersion" );
                        hr = ExecuteFunc( pdispConfAccess,
                                            L"getByHierarchicalName",
                                            &aProductVersion,
                                            1,
                                            &aOfficeVersion );

                        if( SUCCEEDED( hr ) && aOfficeVersion.vt == VT_BSTR )
                        {
                            if (!wcscmp(aOfficeName.bstrVal, L"StarOffice"))
                            {
                                if (!wcsncmp(aOfficeVersion.bstrVal, L"6.1", 3))
                                    bResult = SO_61;
                                else if (!wcsncmp(aOfficeVersion.bstrVal, L"6.0", 3))
                                    bResult = SO_60;
                                else if (!wcsncmp(aOfficeVersion.bstrVal, L"5.2", 3))
                                    bResult = SO_52;
                                else
                                    bResult = SO_UNKNOWN;
                            }
                            else // OpenOffice
                            {
                                if (!wcsncmp(aOfficeVersion.bstrVal, L"1.1", 3))
                                    bResult = OO_11;
                                else if (!wcsncmp(aOfficeVersion.bstrVal, L"1.0", 3))
                                    bResult = OO_10;
                                else
                                    bResult = OO_UNKNOWN;
                            }
                        }
                    }
                }
            }
        }
    }

    return bResult;
}

namespace {

class LockingGuard
{
    bool& mbLocked;
public:
    explicit LockingGuard( bool& bLocked )
    : mbLocked( bLocked )
    {
        mbLocked = true;
    }

    ~LockingGuard()
    {
        mbLocked = false;
    }
};

}

HRESULT CSOActiveX::OnDrawAdvanced( ATL_DRAWINFO& di )
{
    // This method is called only in main thread, no need to lock it

    // Get read of reentrance problems
    if ( mbDrawLocked )
        return S_OK;
    LockingGuard aGuard( mbDrawLocked );

    if( m_spInPlaceSite && mCurFileUrl && mbReadyForActivation )
    {
        HWND hwnd;
        HRESULT hr = m_spInPlaceSite->GetWindow( &hwnd );
        if( !SUCCEEDED( hr ) ) return hr;

        if( mParentWin != hwnd || !mOffWin )
        {
            if( mpDispFrame )
            {
                CComVariant dummyResult;
                CComVariant aPropVar;
                aPropVar.vt = VT_BOOL; aPropVar.boolVal = VARIANT_FALSE;
                (void) ExecuteFunc( mpDispFrame, L"close", &aPropVar, 1, &dummyResult );
                mpDispFrame = CComPtr<IDispatch>();
            }

            mParentWin = hwnd;
            mOffWin = CreateWindowW(
                                STAROFFICE_WINDOWCLASS,
                                L"OfficeContainer",
                                WS_CHILD | WS_CLIPCHILDREN | WS_BORDER,
                                di.prcBounds->left,
                                di.prcBounds->top,
                                di.prcBounds->right - di.prcBounds->left,
                                di.prcBounds->bottom - di.prcBounds->top,
                                mParentWin,
                                nullptr,
                                nullptr,
                                nullptr );

            ::ShowWindow( mOffWin, SW_SHOW );
        }
        else
        {
            RECT aRect;
            ::GetWindowRect( mOffWin, &aRect );

            if( aRect.left !=  di.prcBounds->left || aRect.top != di.prcBounds->top
             || aRect.right != di.prcBounds->right || aRect.bottom != di.prcBounds->bottom )
            {
                // on this state the office window should exist already
                ::SetWindowPos( mOffWin,
                              HWND_TOP,
                              di.prcBounds->left,
                              di.prcBounds->top,
                              di.prcBounds->right - di.prcBounds->left,
                              di.prcBounds->bottom - di.prcBounds->top,
                              SWP_NOZORDER );

                CComVariant aPosArgs[5];
                aPosArgs[4] = CComVariant( 0 );
                aPosArgs[3] = CComVariant( 0 );
                aPosArgs[2] = CComVariant( int(di.prcBounds->right - di.prcBounds->left) );
                aPosArgs[1] = CComVariant( int(di.prcBounds->bottom - di.prcBounds->top) );
                aPosArgs[0] = CComVariant( 12 );
                CComVariant dummyResult;
                hr = ExecuteFunc( mpDispWin, L"setPosSize", aPosArgs, 5, &dummyResult );
                if( !SUCCEEDED( hr ) ) return hr;
            }
        }

        if (mnVersion == SO_NOT_DETECTED)
        {
            OutputError_Impl( mOffWin, CS_E_INVALID_VERSION );
            return E_FAIL;
        }

        if( ! mpDispFrame )
        {
            hr = CreateFrameOldWay( mOffWin,
                            di.prcBounds->right - di.prcBounds->left,
                            di.prcBounds->bottom - di.prcBounds->top );

            if( !SUCCEEDED( hr ) )
            {
                // if the frame can not be opened do not try any more
                mbReadyForActivation = FALSE;
                OutputError_Impl( mOffWin, STG_E_ABNORMALAPIEXIT );
                return hr;
            }
        }

        if( mbLoad )
        {
            hr = LoadURLToFrame();
            mbLoad = FALSE;

            if( !SUCCEEDED( hr ) )
            {
                // if the document can not be opened do not try any more
                mbReadyForActivation = FALSE;

                OutputError_Impl( mOffWin, STG_E_ABNORMALAPIEXIT );

                return hr;
            }
        }
    }
    else
    {
        // activate the fallback
        CComControl<CSOActiveX>::OnDrawAdvanced( di );
    }

    return S_OK;
}

HRESULT CSOActiveX::OnDraw( ATL_DRAWINFO& di )
{
    // fallback that is activated by the parent class
    if ( di.hdcDraw )
        FillRect( di.hdcDraw, reinterpret_cast<RECT const *>(di.prcBounds), reinterpret_cast<HBRUSH>(COLOR_BACKGROUND) );

    return S_OK;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CSOActiveX::SetClientSite( IOleClientSite* aClientSite )
{
    HRESULT hr = IOleObjectImpl<CSOActiveX>::SetClientSite( aClientSite );

    if( !aClientSite )
    {
        //ATLASSERT( mWebBrowser2 );
        if( mWebBrowser2 )
            AtlUnadvise( mWebBrowser2, DIID_DWebBrowserEvents2, mCookie );
        return hr;
    }

    CComPtr<IOleContainer> aContainer;
    m_spClientSite->GetContainer( &aContainer );
//  ATLASSERT( aContainer );

    if( SUCCEEDED( hr )  && aContainer )
    {
        CComQIPtr<IServiceProvider, &IID_IServiceProvider> aServiceProvider( aContainer );
        //ATLASSERT( aServiceProvider );

        if( aServiceProvider )
        {
            aServiceProvider->QueryService( SID_SInternetExplorer,
                                            IID_IWebBrowser,
                                            reinterpret_cast<void**>(&mWebBrowser2) );
//          ATLASSERT( mWebBrowser2 );
            if( mWebBrowser2 )
                AtlAdvise( mWebBrowser2, GetUnknown(), DIID_DWebBrowserEvents2, &mCookie );
        }
    }

    return hr;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CSOActiveX::Invoke(DISPID dispidMember,
                                REFIID riid,
                                LCID lcid,
                                WORD wFlags,
                                DISPPARAMS* pDispParams,
                                VARIANT* pvarResult,
                                EXCEPINFO* pExcepInfo,
                                UINT* puArgErr)
{
    if (riid != IID_NULL)
        return DISP_E_UNKNOWNINTERFACE;

    if (!pDispParams)
        return DISP_E_PARAMNOTOPTIONAL;

    if ( dispidMember == DISPID_ONQUIT )
        Cleanup();

    IDispatchImpl<ISOActiveX, &IID_ISOActiveX,
                  &LIBID_SO_ACTIVEXLib>::Invoke(
             dispidMember, riid, lcid, wFlags, pDispParams,
             pvarResult, pExcepInfo, puArgErr);

    return S_OK;
}

HRESULT CSOActiveX::GetURL( const OLECHAR* url,
                              const OLECHAR* target )
{
    CComVariant aEmpty1, aEmpty2, aEmpty3;
    CComVariant aUrl( url );
    CComVariant aTarget;
    if ( target )
        aTarget = CComVariant( target );

    return mWebBrowser2->Navigate2( &aUrl,
                                  &aEmpty1,
                                  &aTarget,
                                  &aEmpty2,
                                  &aEmpty3 );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
