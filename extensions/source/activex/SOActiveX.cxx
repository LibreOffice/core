/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */



#pragma warning (disable:4505)


#pragma warning (push,1)
#pragma warning (disable:4265)

#include "stdafx2.h"
#include "so_activex.h"
#include "SOActiveX.h"
#include "SOComWindowPeer.h"
#include "SODispatchInterceptor.h"
#include "SOActionsApproval.h"

#pragma warning (pop)

#define STAROFFICE_WINDOWCLASS "SOParentWindow"




void OutputError_Impl( HWND hw, HRESULT ErrorCode )
{
    void* sMessage;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        ErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
        (LPTSTR) &sMessage,
        0,
        NULL
    );
    ::MessageBoxA( hw, (LPCTSTR)sMessage, NULL, MB_OK | MB_ICONINFORMATION );
    LocalFree( sMessage );
}

HRESULT ExecuteFunc( IDispatch* idispUnoObject,
                     OLECHAR* sFuncName,
                     CComVariant* params,
                     unsigned int count,
                     CComVariant* pResult )
{
    if( !idispUnoObject )
        return E_FAIL;

    DISPID id;
    HRESULT hr = idispUnoObject->GetIDsOfNames( IID_NULL, &sFuncName, 1, LOCALE_USER_DEFAULT, &id);
    if( !SUCCEEDED( hr ) ) return hr;

    DISPPARAMS dispparams= { params, 0, count, 0};

    
    EXCEPINFO myInfo;
    hr = idispUnoObject->Invoke( id, IID_NULL,LOCALE_USER_DEFAULT, DISPATCH_METHOD,
                    &dispparams, pResult, &myInfo, 0);

    
    
    
    

    return hr;
}

HRESULT GetIDispByFunc( IDispatch* idispUnoObject,
                          OLECHAR* sFuncName,
                          CComVariant* params,
                          unsigned int count,
                          CComPtr<IDispatch>& pdispResult )
{
    if( !idispUnoObject )
        return E_FAIL;

    CComVariant result;
    HRESULT hr = ExecuteFunc( idispUnoObject, sFuncName, params, count, &result );
    if( !SUCCEEDED( hr ) ) return hr;

    if( result.vt != VT_DISPATCH || result.pdispVal == NULL )
        return E_FAIL;

    pdispResult = CComPtr<IDispatch>( result.pdispVal );

    return S_OK;
}

HRESULT PutPropertiesToIDisp( IDispatch* pdispObject,
                              OLECHAR** sMemberNames,
                              CComVariant* pVariant,
                              unsigned int count )
{
    for( unsigned int ind = 0; ind < count; ind++ )
    {
        DISPID id;
        HRESULT hr = pdispObject->GetIDsOfNames( IID_NULL, &sMemberNames[ind], 1, LOCALE_USER_DEFAULT, &id );
        if( !SUCCEEDED( hr ) ) return hr;

        hr = CComDispatchDriver::PutProperty( pdispObject, id, &pVariant[ind] );
        if( !SUCCEEDED( hr ) ) return hr;
    }

    return S_OK;
}

HRESULT GetPropertiesFromIDisp( IDispatch* pdispObject,
                                OLECHAR** sMemberNames,
                                CComVariant* pVariant,
                                unsigned int count )
{
    for( unsigned int ind = 0; ind < count; ind++ )
    {
        DISPID id;
        HRESULT hr = pdispObject->GetIDsOfNames( IID_NULL, &sMemberNames[ind], 1, LOCALE_USER_DEFAULT, &id );
        if( !SUCCEEDED( hr ) ) return hr;

        hr = CComDispatchDriver::GetProperty( pdispObject, id, &pVariant[ind] );
        if( !SUCCEEDED( hr ) ) return hr;
    }

    return S_OK;
}



CSOActiveX::CSOActiveX()
: mCookie(0)
, mCurFileUrl( L"private:factory/swriter" )
, mbLoad( FALSE )
, mParentWin( NULL )
, mOffWin( NULL )
, mbViewOnly( TRUE )
, mpDispatchInterceptor( NULL )
, mnVersion( SO_NOT_DETECTED )
, mbReadyForActivation( FALSE )
, mbDrawLocked( FALSE )
{
    CLSID clsFactory = {0x82154420,0x0FBF,0x11d4,{0x83, 0x13,0x00,0x50,0x04,0x52,0x6A,0xB4}};
    HRESULT hr = CoCreateInstance( clsFactory, NULL, CLSCTX_ALL, __uuidof(IDispatch), (void**)&mpDispFactory);
    if( !SUCCEEDED( hr ) )
        OutputError_Impl( NULL, hr );

    mPWinClass.style            = CS_HREDRAW|CS_VREDRAW;
    mPWinClass.lpfnWndProc      = ::DefWindowProc;
    mPWinClass.cbClsExtra       = 0;
    mPWinClass.cbWndExtra       = 0;
    mPWinClass.hInstance        = (HINSTANCE) GetModuleHandle(NULL); 
    mPWinClass.hIcon            = NULL;
    mPWinClass.hCursor          = NULL;
    mPWinClass.hbrBackground    = (HBRUSH) COLOR_BACKGROUND;
    mPWinClass.lpszMenuName     = NULL;
    mPWinClass.lpszClassName    = STAROFFICE_WINDOWCLASS;

    RegisterClass(&mPWinClass);
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
        mpDispatchInterceptor = NULL;
    }

    mpDispTempFile = CComPtr< IDispatch >();
    mbReadyForActivation = FALSE;

    if( mpInstanceLocker )
    {
        ExecuteFunc( mpInstanceLocker, L"dispose", NULL, 0, &dummyResult );
        mpInstanceLocker = CComPtr< IDispatch >();
    }

    if( mpDispFrame )
    {
        BOOL bCloserActivated = FALSE;

        CComPtr<IDispatch> pDispDocumentCloser;
        CComVariant aDocCloser( L"com.sun.star.embed.DocumentCloser" );
        HRESULT hr = GetIDispByFunc( mpDispFactory,
                                     L"createInstance",
                                     &aDocCloser,
                                     1,
                                     pDispDocumentCloser );
        if ( SUCCEEDED( hr ) && pDispDocumentCloser )
        {
            SAFEARRAY FAR* pInitFrame = SafeArrayCreateVector( VT_VARIANT, 0, 1 );
            long nInitInd = 0;
            CComVariant pFrameVariant( mpDispFrame );
            SafeArrayPutElement( pInitFrame, &nInitInd, &pFrameVariant );
            CComVariant aVarInitFrame;
            aVarInitFrame.vt = VT_ARRAY | VT_VARIANT; aVarInitFrame.parray = pInitFrame;
            hr = ExecuteFunc( pDispDocumentCloser, L"initialize", &aVarInitFrame, 1, &dummyResult );
            if( SUCCEEDED( hr ) )
            {
                
                hr = ExecuteFunc( pDispDocumentCloser, L"dispose", NULL, 0, &dummyResult );
                bCloserActivated = SUCCEEDED( hr );
            }
        }

        if ( !bCloserActivated )
        {
            CComVariant aPropVar;
            aPropVar.vt = VT_BOOL; aPropVar.boolVal = VARIANT_TRUE;
            if ( !SUCCEEDED( ExecuteFunc( mpDispFrame, L"close", &aPropVar, 1, &dummyResult ) ) )
                ExecuteFunc( mpDispFrame, L"dispose", NULL, 0, &dummyResult );
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
    
    CComPtr<IDispatch> pdispDesktop;
    CComVariant aDesktopServiceName( L"com.sun.star.frame.Desktop" );

    HRESULT hr = GetIDispByFunc( mpDispFactory, L"createInstance", &aDesktopServiceName, 1, pdispDesktop );
    if( !pdispDesktop || !SUCCEEDED( hr ) ) return hr;

    
    CComPtr<IDispatch> pdispChildren;
    hr = GetIDispByFunc( pdispDesktop, L"getFrames", NULL, 0, pdispChildren );
    if( !pdispChildren || !SUCCEEDED( hr ) ) return hr;

    CComVariant aFrames;
    CComVariant nFlag( 4 );
    hr = ExecuteFunc( pdispChildren, L"queryFrames", &nFlag, 1, &aFrames );
    if ( SUCCEEDED( hr ) )
    {
        if ( ( aFrames.vt == ( VT_ARRAY | VT_DISPATCH ) || aFrames.vt == ( VT_ARRAY | VT_VARIANT ) )
          && ( !aFrames.parray || aFrames.parray->cDims == 1 && aFrames.parray->rgsabound[0].cElements == 0 ) )
        {
            
            
            CComVariant dummyResult;
            hr = ExecuteFunc( pdispDesktop, L"terminate", NULL, 0, &dummyResult );
        }
    }

    return hr;
}

STDMETHODIMP CSOActiveX::InitNew ()
{
    mnVersion = GetVersionConnected();
    mbLoad = TRUE;
    return S_OK;
}

STDMETHODIMP CSOActiveX::Load ( LPSTREAM /*pStm*/ )
{
    mnVersion = GetVersionConnected();
    mbLoad = TRUE;

    
    

    return S_OK;
}

STDMETHODIMP CSOActiveX::Load( LPPROPERTYBAG pPropBag, LPERRORLOG /*pErrorLog*/ )
{
    mnVersion = GetVersionConnected();

    IPropertyBag2* pPropBag2;
    HRESULT hr = pPropBag->QueryInterface( IID_IPropertyBag2, (void**)&pPropBag2 );
    

    if( !SUCCEEDED( hr ) )
        return hr;

    unsigned long aNum;
    hr = pPropBag2->CountProperties( &aNum );
    
    if( !SUCCEEDED( hr ) )
        return hr;

    PROPBAG2* aPropNames = new PROPBAG2[aNum];
    unsigned long aReaded;

    hr = pPropBag2->GetPropertyInfo( 0,
                                     aNum,
                                     aPropNames,
                                     &aReaded );
    
    if( !SUCCEEDED( hr ) )
    {
        delete[] aPropNames;
        return hr;
    }

    CComVariant* aVal = new CComVariant[aNum];
    HRESULT*     hvs = new HRESULT[aNum];
    hr = pPropBag2->Read( aNum,
                          aPropNames,
                          NULL,
                          aVal,
                          hvs );
    
    if( !SUCCEEDED( hr ) )
    {
        delete[] hvs;
        delete[] aVal;
        delete[] aPropNames;
        return hr;
    }

    USES_CONVERSION;
    for( unsigned long ind = 0; ind < aNum; ind++ )
    {
        
        if( aVal[ind].vt == VT_BSTR && !strcmp( OLE2T( aPropNames[ind].pstrName ), "src" ) )
        {
            mCurFileUrl = wcsdup( aVal[ind].bstrVal );
        }
        else if( aVal[ind].vt == VT_BSTR
                && !strcmp( OLE2T( aPropNames[ind].pstrName ), "readonly" ) )
        {
            if( !strcmp( OLE2T( aVal[ind].bstrVal ), "true" ) )
            {
                
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
    hr = CBindStatusCallback<CSOActiveX>::Download( this, &CSOActiveX::CallbackCreateXInputStream, mCurFileUrl, m_spClientSite, FALSE );
    if ( hr == MK_S_ASYNCHRONOUS )
        hr = S_OK;

    if ( !SUCCEEDED( hr ) )
    {
        
        mbLoad = TRUE;

        Invalidate();
        UpdateWindow();
    }

    return hr;
}

HRESULT CSOActiveX::GetUnoStruct( OLECHAR* sStructName, CComPtr<IDispatch>& pdispResult )
{
    CComVariant aComStruct( sStructName );
    return GetIDispByFunc( mpDispFactory, L"Bridge_GetStruct", &aComStruct, 1, pdispResult );
}

HRESULT CSOActiveX::GetUrlStruct( OLECHAR* sUrl, CComPtr<IDispatch>& pdispUrl )
{
    HRESULT hr = GetUnoStruct( L"com.sun.star.util.URL", pdispUrl );
    if( !SUCCEEDED( hr ) ) return hr;

    OLECHAR* sURLMemberName = L"Complete";
    DISPID nURLID;
    hr = pdispUrl->GetIDsOfNames( IID_NULL, &sURLMemberName, 1, LOCALE_USER_DEFAULT, &nURLID );
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
    aParam[0] = CComVariant( L"file:

    hr = ExecuteFunc( pdispTransformer, L"parseSmart", aParam, 2, &dummyResult );
    if( !SUCCEEDED( hr ) || dummyResult.vt != VT_BOOL || !dummyResult.boolVal ) return hr;

    return S_OK;
}

HRESULT CSOActiveX::SetLayoutManagerProps()
{
    if ( !mpDispFrame )
        return E_FAIL;

    CComVariant pVarLayoutMgr;
    OLECHAR* sLMPropName = L"LayoutManager";
    HRESULT hr = GetPropertiesFromIDisp( mpDispFrame, &sLMPropName, &pVarLayoutMgr, 1 );
    if( pVarLayoutMgr.vt != VT_DISPATCH || pVarLayoutMgr.pdispVal == NULL )
        return E_FAIL;

      CComPtr<IDispatch> pdispLM( pVarLayoutMgr.pdispVal );


    if( !SUCCEEDED( hr ) || !pdispLM )
        return E_FAIL;

    OLECHAR* sATName = L"AutomaticToolbars";
    CComVariant pATProp;
    pATProp.vt = VT_BOOL; pATProp.boolVal = VARIANT_FALSE ;
    hr = PutPropertiesToIDisp( pdispLM, &sATName, &pATProp, 1 );

    return hr;
}

HRESULT CSOActiveX::CreateFrameOldWay( HWND hwnd, int width, int height )
{
    if( !mpDispFactory )
        return E_FAIL;

    
    CComPtr< CComObject< SOComWindowPeer > > pPeerToSend = new CComObject<SOComWindowPeer>();
    pPeerToSend->SetHWNDInternally( hwnd );
    CComQIPtr< IDispatch, &IID_IDispatch > pIDispToSend( pPeerToSend );

    
    CComPtr<IDispatch> pdispRectangle;
    HRESULT hr = GetUnoStruct( L"com.sun.star.awt.Rectangle", pdispRectangle );
    if( !SUCCEEDED( hr ) ) return hr;

    OLECHAR* sRectMemberNames[4] = { L"X",
                                      L"Y",
                                      L"Width",
                                      L"Height" };
    CComVariant pRectVariant[4];
    pRectVariant[0] = pRectVariant[1] = pRectVariant[2] = pRectVariant[3] = CComVariant( 0 );

    hr = PutPropertiesToIDisp( pdispRectangle, sRectMemberNames, pRectVariant, 4 );
    if( !SUCCEEDED( hr ) ) return hr;

    
    CComPtr<IDispatch> pdispWinDescr;
    hr = GetUnoStruct( L"com.sun.star.awt.WindowDescriptor", pdispWinDescr );
    if( !SUCCEEDED( hr ) ) return hr;

    
    OLECHAR* sDescriptorMemberNames[6] = { L"Type",
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

    
    CComPtr<IDispatch> pdispToolkit;
    CComVariant aServiceName( L"com.sun.star.awt.Toolkit" );
    hr = GetIDispByFunc( mpDispFactory, L"createInstance", &aServiceName, 1, pdispToolkit );
    if( !SUCCEEDED( hr ) ) return hr;

    
    CComVariant aWinDescr( pdispWinDescr );
    hr = GetIDispByFunc( pdispToolkit, L"createWindow", &aWinDescr, 1, mpDispWin );
    if( !SUCCEEDED( hr ) ) return hr;

    
    aServiceName = CComVariant( L"com.sun.star.frame.Task" );
    hr = GetIDispByFunc( mpDispFactory, L"createInstance", &aServiceName, 1, mpDispFrame );
    if( !SUCCEEDED( hr ) || !mpDispFrame )
    {
        
        
        aServiceName = CComVariant( L"com.sun.star.frame.Frame" );
        hr = GetIDispByFunc( mpDispFactory, L"createInstance", &aServiceName, 1, mpDispFrame );
        if( !SUCCEEDED( hr ) ) return hr;
    }

    
    CComVariant dummyResult;
    CComVariant aDispWin( mpDispWin );
    hr = ExecuteFunc( mpDispFrame, L"initialize", &aDispWin, 1, &dummyResult );
    if( !SUCCEEDED( hr ) ) return hr;

    
    SetLayoutManagerProps();

    
    CComPtr<IDispatch> pdispDesktop;
    aServiceName = CComVariant( L"com.sun.star.frame.Desktop" );
    hr = GetIDispByFunc( mpDispFactory, L"createInstance", &aServiceName, 1, pdispDesktop );
    if( !SUCCEEDED( hr ) ) return hr;

    
    CComPtr<IDispatch> pdispChildren;
    hr = GetIDispByFunc( pdispDesktop, L"getFrames", NULL, 0, pdispChildren );
    if( !SUCCEEDED( hr ) ) return hr;

    
    CComVariant aDispFrame( mpDispFrame );
    hr = ExecuteFunc( pdispChildren, L"append", &aDispFrame, 1, &dummyResult );
    if( !SUCCEEDED( hr ) ) return hr;

    
    CComVariant aTransparent( (long)0xFFFFFFFF );
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

    
    aServiceName = CComVariant( L"com.sun.star.embed.InstanceLocker" );
    hr = GetIDispByFunc( mpDispFactory, L"createInstance", &aServiceName, 1, mpInstanceLocker );
    if( SUCCEEDED( hr ) && mpInstanceLocker )
    {
        SAFEARRAY FAR* pInitVals = SafeArrayCreateVector( VT_VARIANT, 0, 3 );

        
        long nInitInd = 0;
        CComVariant pFrameVariant( mpDispFrame );
        SafeArrayPutElement( pInitVals, &nInitInd, &pFrameVariant );

        
        nInitInd = 1;
        CComVariant pStrArr( 1L );
        SafeArrayPutElement( pInitVals, &nInitInd, &pStrArr );

        
        nInitInd = 2;
        CComPtr<IDispatch> pdispValueObj;
        hr = GetIDispByFunc( mpDispFactory, L"Bridge_GetValueObject", NULL, 0, pdispValueObj );
        if( !SUCCEEDED( hr ) || !pdispValueObj ) return hr;

        CComVariant aValueArgs[2];
        aValueArgs[1] = CComVariant( L"com.sun.star.embed.XActionsApproval" );
        CComPtr< CComObject< SOActionsApproval > > pApproval( new CComObject<SOActionsApproval>() );
        aValueArgs[0] = CComVariant ( pApproval );

        hr = ExecuteFunc( pdispValueObj, L"Set", aValueArgs, 2, &dummyResult );
        if( !SUCCEEDED( hr ) ) return hr;

        CComVariant aValueObj( pdispValueObj );
        SafeArrayPutElement( pInitVals, &nInitInd, &aValueObj );

        
        CComVariant aVarInitVals;
        aVarInitVals.vt = VT_ARRAY | VT_VARIANT; aVarInitVals.parray = pInitVals;
        hr = ExecuteFunc( mpInstanceLocker, L"initialize", &aVarInitVals, 1, &dummyResult );
        if( !SUCCEEDED( hr ) ) return hr;
    }

    return S_OK;
}

HRESULT CSOActiveX::CallLoadComponentFromURL1PBool( OLECHAR* sUrl, OLECHAR* sArgName, BOOL sArgVal )
{
    SAFEARRAY FAR* pPropVals = SafeArrayCreateVector( VT_DISPATCH, 0, 1 );
    long ix = 0;
    CComPtr<IDispatch> pdispPropVal;
    HRESULT hr = GetUnoStruct( L"com.sun.star.beans.PropertyValue", pdispPropVal );
    if( !SUCCEEDED( hr ) ) return hr;

    OLECHAR*    sPropMemberNames[2] = { L"Name", L"Value" };
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
    
    aDispArgs[0].vt = VT_ARRAY | VT_DISPATCH; aDispArgs[0].parray = pPropVals;

    CComVariant dummyResult;
    hr = ExecuteFunc( mpDispFrame, L"loadComponentFromURL", aDispArgs, 4, &dummyResult );
    if( !SUCCEEDED( hr ) ) return hr;

    return S_OK;
}

HRESULT CSOActiveX::CallDispatchMethod( OLECHAR* sUrl,
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
    aArgs[0] = CComVariant( (int)0 );
    hr = GetIDispByFunc( mpDispFrame,
                         L"queryDispatch",
                         aArgs,
                         3,
                         pdispXDispatch );
    if( !SUCCEEDED( hr ) ) return hr;

    SAFEARRAY FAR* pPropVals = SafeArrayCreateVector( VT_DISPATCH, 0, count );
    for( long ix = 0; ix < (long)count; ix ++ )
    {
        CComPtr<IDispatch> pdispPropVal;
        hr = GetUnoStruct( L"com.sun.star.beans.PropertyValue", pdispPropVal );
        if( !SUCCEEDED( hr ) ) return hr;

        OLECHAR*    sPropMemberNames[2] = { L"Name", L"Value" };
        CComVariant pPropVar[2];
        pPropVar[0] = aArgNames[ix];
        pPropVar[1] = aArgVals[ix];
        hr = PutPropertiesToIDisp( pdispPropVal, sPropMemberNames, pPropVar, 2 );
        if( !SUCCEEDED( hr ) ) return hr;

        SafeArrayPutElement( pPropVals, &ix, pdispPropVal );
    }

    CComVariant aDispArgs[2];
    aDispArgs[1] = CComVariant( pdispURL );
    
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

    BOOL bSuccess = FALSE;
    BOOL bFinishDownload = FALSE;
    if ( !pBytes )
    {
        
        bFinishDownload = TRUE;
        if ( SUCCEEDED( dwSize ) )
            bSuccess = TRUE;
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
            SAFEARRAY FAR* pDataArray = SafeArrayCreateVector( VT_I1, 0, dwSize );

            if ( pDataArray )
            {
                hr = SafeArrayLock( pDataArray );
                if ( SUCCEEDED( hr ) )
                {
                    for( DWORD ix = 0; ix < dwSize; ix++ )
                        ((BYTE*)(pDataArray->pvData))[ix] = pBytes[ix];
                    hr = SafeArrayUnlock( pDataArray );
                    if ( SUCCEEDED( hr ) )
                    {
                        CComVariant aArgs[1];
                        aArgs[0].vt = VT_ARRAY | VT_I1; aArgs[0].parray = pDataArray;
                        CComVariant dummyResult;
                        hr = ExecuteFunc( mpDispTempFile, L"writeBytes", aArgs, 1, &dummyResult );
                        if( SUCCEEDED( hr ) )
                            bSuccess = TRUE;
                    }
                }
            }
        }
    }

    if ( !bSuccess )
    {
        
        bFinishDownload = TRUE;
        mpDispTempFile = CComPtr< IDispatch >();
    }

    if ( bFinishDownload )
    {
        
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
    unsigned int nCount = 3; 

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

    CComVariant aBarName( L"MenuBarVisible" );
    CComVariant aBarVis;
    aBarVis.vt = VT_BOOL; aBarVis.boolVal = VARIANT_FALSE;
    hr = CallDispatchMethod( L"slot:6661", &aBarName, &aBarVis, 1 );
    
    

    
    CComPtr<IDispatch> pdispController;
    hr = GetIDispByFunc( mpDispFrame, L"getController", NULL, 0, pdispController );
    if ( SUCCEEDED( hr ) && pdispController )
    {
        CComPtr<IDispatch> pdispModel;
        hr = GetIDispByFunc( pdispController, L"getModel", NULL, 0, pdispModel );
        if ( SUCCEEDED( hr ) && pdispModel )
        {
            CComPtr<IDispatch> pdispPres;
            hr = GetIDispByFunc( pdispModel, L"getPresentation", NULL, 0, pdispPres );
            if ( SUCCEEDED( hr ) && pdispPres )
            {
                
                
                OLECHAR* pPropName = L"IsFullScreen";
                CComVariant pPresProp;
                pPresProp.vt = VT_BOOL; pPresProp.boolVal = VARIANT_FALSE ;
                hr = PutPropertiesToIDisp( pdispPres, &pPropName, &pPresProp, 1 );

                
                if ( SUCCEEDED( hr ) )
                {
                    CComVariant dummyResult;
                    ExecuteFunc( pdispPres, L"Start", NULL, 0, &dummyResult );
                }
            }
        }
    }

    
    mpDispatchInterceptor = new CComObject< SODispatchInterceptor >();
    mpDispatchInterceptor->AddRef();
    mpDispatchInterceptor->SetParent( this );
    CComQIPtr< IDispatch, &IID_IDispatch > pIDispDispInter( mpDispatchInterceptor );

    
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
                long ix = 0;
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
                            USES_CONVERSION;
                            if( !strcmp( OLE2T( aOfficeName.bstrVal ), "StarOffice" ) )
                            {
                                if( !strncmp( OLE2T( aOfficeVersion.bstrVal ), "6.1", 3 ) )
                                    bResult = SO_61;
                                else if( !strncmp( OLE2T( aOfficeVersion.bstrVal ), "6.0", 3 ) )
                                    bResult = SO_60;
                                else if( !strncmp( OLE2T( aOfficeVersion.bstrVal ), "5.2", 3 ) )
                                    bResult = SO_52;
                                else
                                    bResult = SO_UNKNOWN;
                            }
                            else 
                            {
                                if( !strncmp( OLE2T( aOfficeVersion.bstrVal ), "1.1", 3 ) )
                                    bResult = OO_11;
                                else if( !strncmp( OLE2T( aOfficeVersion.bstrVal ), "1.0", 3 ) )
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

class LockingGuard
{
    BOOL& mbLocked;
public:
    LockingGuard( BOOL& bLocked )
    : mbLocked( bLocked )
    {
        mbLocked = TRUE;
    }

    ~LockingGuard()
    {
        mbLocked = FALSE;
    }
};

HRESULT CSOActiveX::OnDrawAdvanced( ATL_DRAWINFO& di )
{
    

    
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
                HRESULT hr = ExecuteFunc( mpDispFrame, L"close", &aPropVar, 1, &dummyResult );
                (void)hr;
                mpDispFrame = CComPtr<IDispatch>();
            }

            mParentWin = hwnd;
            mOffWin = CreateWindow(
                                STAROFFICE_WINDOWCLASS,
                                "OfficeContainer",
                                WS_CHILD | WS_CLIPCHILDREN | WS_BORDER,
                                di.prcBounds->left,
                                di.prcBounds->top,
                                di.prcBounds->right - di.prcBounds->left,
                                di.prcBounds->bottom - di.prcBounds->top,
                                mParentWin,
                                NULL,
                                NULL,
                                NULL );

            ::ShowWindow( mOffWin, SW_SHOW );
        }
        else
        {
            RECT aRect;
            ::GetWindowRect( mOffWin, &aRect );

            if( aRect.left !=  di.prcBounds->left || aRect.top != di.prcBounds->top
             || aRect.right != di.prcBounds->right || aRect.bottom != di.prcBounds->bottom )
            {
                
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

        if( !mnVersion )
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
                
                mbReadyForActivation = FALSE;

                OutputError_Impl( mOffWin, STG_E_ABNORMALAPIEXIT );

                return hr;
            }
        }
    }
    else
    {
        
        CComControl<CSOActiveX>::OnDrawAdvanced( di );
    }

    return S_OK;
}

HRESULT CSOActiveX::OnDraw( ATL_DRAWINFO& di )
{
    
    if ( di.hdcDraw )
        FillRect( di.hdcDraw, (RECT*)di.prcBounds, (HBRUSH)COLOR_BACKGROUND );

    return S_OK;
}

STDMETHODIMP CSOActiveX::SetClientSite( IOleClientSite* aClientSite )
{
    HRESULT hr = IOleObjectImpl<CSOActiveX>::SetClientSite( aClientSite );

    if( !aClientSite )
    {
        
        if( mWebBrowser2 )
            AtlUnadvise( mWebBrowser2, DIID_DWebBrowserEvents2, mCookie );
        return hr;
    }

    CComPtr<IOleContainer> aContainer;
    m_spClientSite->GetContainer( &aContainer );


    if( SUCCEEDED( hr )  && aContainer )
    {
        CComQIPtr<IServiceProvider, &IID_IServiceProvider> aServiceProvider( aContainer );
        

        if( aServiceProvider )
        {
            aServiceProvider->QueryService( SID_SInternetExplorer,
                                            IID_IWebBrowser,
                                            (void**)&mWebBrowser2 );

            if( mWebBrowser2 )
                AtlAdvise( mWebBrowser2, GetUnknown(), DIID_DWebBrowserEvents2, &mCookie );
        }
    }

    return hr;
}

STDMETHODIMP CSOActiveX::Invoke(DISPID dispidMember,
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
