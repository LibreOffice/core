/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

// SOActiveX.cpp : Implementation of CSOActiveX

#include "stdafx2.h"
#include "so_activex.h"
#include "SOActiveX.h"
#include "SOComWindowPeer.h"

#define STAROFFICE_WINDOWCLASS "SOParentWindow"

#define BARS_NUMBER  3
#define BARS_TO_SHOW 2

OLECHAR* pSlotUrl[BARS_NUMBER] =
                        {L"slot:5910" // SID_TOGGLEFUNCTIONBAR
                        ,L"slot:5920" // SID_TOGGLESTATUSBAR
                        ,L"slot:6661" // SID_TOGGLE_MENUBAR
                        };

OLECHAR* pSlotName[BARS_NUMBER] =
                        {L"FunctionBarVisible"      // SID_TOGGLEFUNCTIONBAR
                        ,L"StatusBarVisible"        // SID_TOGGLESTATUSBAR
                        ,L"MenuBarVisible"          // SID_TOGGLE_MENUBAR
                        };





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

    // DEBUG
    EXCEPINFO myInfo;
    return idispUnoObject->Invoke( id, IID_NULL,LOCALE_USER_DEFAULT, DISPATCH_METHOD,
                    &dispparams, pResult, &myInfo, 0);
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
        return hr;

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


// CSOActiveX

CSOActiveX::CSOActiveX()
: mCookie(0)
, mCurFileUrl( L"private:factory/swriter" )
, mbLoad( FALSE )
, mParentWin( NULL )
, mOffWin( NULL )
, mbViewOnly( FALSE )
{
    CLSID clsFactory = {0x82154420,0x0FBF,0x11d4,{0x83, 0x13,0x00,0x50,0x04,0x52,0x6A,0xB4}};
    HRESULT hr = CoCreateInstance( clsFactory, NULL, CLSCTX_ALL, __uuidof(IDispatch), (void**)&mpDispFactory);

    mPWinClass.style            = CS_HREDRAW|CS_VREDRAW;
    mPWinClass.lpfnWndProc      = ::DefWindowProc;
    mPWinClass.cbClsExtra       = 0;
    mPWinClass.cbWndExtra       = 0;
    mPWinClass.hInstance        = (HINSTANCE) GetModuleHandle(NULL); //myInstance;
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
    if( mpDispFrame && mbViewOnly )
    {
        ShowSomeBars();
        mbViewOnly = FALSE;
    }

    if( mpDispFrame )
    {
        // mpDispFrame->dispose();
        CComVariant dummyResult;
        ExecuteFunc( mpDispFrame, L"dispose", NULL, 0, &dummyResult );
        mpDispFrame = CComPtr< IDispatch >();
    }

    if( ::IsWindow( mOffWin ) )
        ::DestroyWindow( mOffWin );

    return S_OK;
}


STDMETHODIMP CSOActiveX::InitNew ()
{
    mbLoad = TRUE;
    return S_OK;
}

STDMETHODIMP CSOActiveX::Load ( LPSTREAM pStm )
{
    mbLoad = TRUE;

    // may be later?
    // for now just ignore

    return S_OK;
}

STDMETHODIMP CSOActiveX::Load( LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog )
{
    IPropertyBag2* pPropBag2;
    HRESULT hr = pPropBag->QueryInterface( IID_IPropertyBag2, (void**)&pPropBag2 );
    ATLASSERT( hr >= 0 );

    if( !SUCCEEDED( hr ) )
        return hr;

    unsigned long aNum;
    hr = pPropBag2->CountProperties( &aNum );
    ATLASSERT( hr >= 0 );
    if( !SUCCEEDED( hr ) )
        return hr;

    PROPBAG2* aPropNames = new PROPBAG2[aNum];
    unsigned long aReaded;

    hr = pPropBag2->GetPropertyInfo( 0,
                                     aNum,
                                     aPropNames,
                                     &aReaded );
    ATLASSERT( hr >= 0 );
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
    ATLASSERT( hr >= 0 );
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
        // all information from the 'object' tag is in strings
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
                // the default value
                mbViewOnly = FALSE;
            }
        }
    }

    delete[] hvs;
    delete[] aVal;
    delete[] aPropNames;

    if( !mpDispFactory )
        return hr;

    mbLoad = TRUE;

    Invalidate();
    UpdateWindow();

    return hr;
}

HRESULT CSOActiveX::GetUnoStruct( OLECHAR* sStructName, CComPtr<IDispatch>& pdispResult )
{
    return GetIDispByFunc( mpDispFactory, L"Bridge_GetStruct", &CComVariant( sStructName ), 1, pdispResult );
}

HRESULT CSOActiveX::GetUrlStruct( OLECHAR* sUrl, CComPtr<IDispatch>& pdispUrl )
{
    HRESULT hr = GetUnoStruct( L"com.sun.star.util.URL", pdispUrl );
    if( !SUCCEEDED( hr ) ) return hr;

    OLECHAR* sURLMemberName = L"Complete";
    DISPID nURLID;
    hr = pdispUrl->GetIDsOfNames( IID_NULL, &sURLMemberName, 1, LOCALE_USER_DEFAULT, &nURLID );
    if( !SUCCEEDED( hr ) ) return hr;
    hr = CComDispatchDriver::PutProperty( pdispUrl, nURLID, &CComVariant( sUrl ) );
    if( !SUCCEEDED( hr ) ) return hr;

    CComPtr<IDispatch> pdispTransformer;
    hr = GetIDispByFunc( mpDispFactory,
                         L"createInstance",
                         &CComVariant( L"com.sun.star.util.URLTransformer" ),
                         1,
                         pdispTransformer );
    if( !SUCCEEDED( hr ) ) return hr;

    CComVariant dummyResult;
    CComVariant aInOutParam;
    aInOutParam.ppdispVal = &pdispUrl;
    aInOutParam.vt = VT_DISPATCH | VT_BYREF;
    hr = ExecuteFunc( pdispTransformer, L"parseStrict", &aInOutParam, 1, &dummyResult );
    if( !SUCCEEDED( hr ) || dummyResult.vt != VT_BOOL || !dummyResult.boolVal ) return hr;

    return S_OK;
}


HRESULT CSOActiveX::CreateFrameOldWay( HWND hwnd, int width, int height )
{
    if( !mpDispFactory )
        return E_FAIL;

    // create window handle holder
    CComPtr< CComObject< SOComWindowPeer > > pPeerToSend = new CComObject<SOComWindowPeer>( hwnd );
    pPeerToSend->SetHWNDInternally( hwnd );
    CComQIPtr< IDispatch, &IID_IDispatch > pIDispToSend( pPeerToSend );

    // create rectangle structure
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

    // create WindowDescriptor structure
    CComPtr<IDispatch> pdispWinDescr;
    hr = GetUnoStruct( L"com.sun.star.awt.WindowDescriptor", pdispWinDescr );
    if( !SUCCEEDED( hr ) ) return hr;

    // fill in descriptor with info
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

    // create XToolkit instance
    CComPtr<IDispatch> pdispToolkit;
    hr = GetIDispByFunc( mpDispFactory, L"createInstance", &CComVariant( L"com.sun.star.awt.Toolkit" ), 1, pdispToolkit );
    if( !SUCCEEDED( hr ) ) return hr;

    // create window with toolkit
    hr = GetIDispByFunc( pdispToolkit, L"createWindow", &CComVariant( pdispWinDescr ), 1, mpDispWin );
    if( !SUCCEEDED( hr ) ) return hr;

    // create frame
    hr = GetIDispByFunc( mpDispFactory, L"createInstance", &CComVariant( L"com.sun.star.frame.Task" ), 1, mpDispFrame );
    if( !SUCCEEDED( hr ) || !mpDispFrame )
    {
        // the interface com.sun.star.frame.Task is removed in 6.1
        // but the interface com.sun.star.frame.Frame has some bugs in 6.0
        hr = GetIDispByFunc( mpDispFactory, L"createInstance", &CComVariant( L"com.sun.star.frame.Frame" ), 1, mpDispFrame );
        if( !SUCCEEDED( hr ) ) return hr;
    }

    // initialize frame
    CComVariant dummyResult;
    hr = ExecuteFunc( mpDispFrame, L"initialize", &CComVariant( mpDispWin ), 1, &dummyResult );
    if( !SUCCEEDED( hr ) ) return hr;

    // create desktop
    CComPtr<IDispatch> pdispDesktop;
    hr = GetIDispByFunc( mpDispFactory, L"createInstance", &CComVariant( L"com.sun.star.frame.Desktop" ), 1, pdispDesktop );
    if( !SUCCEEDED( hr ) ) return hr;

    // create tree of frames
    CComPtr<IDispatch> pdispChildren;
    hr = GetIDispByFunc( pdispDesktop, L"getFrames", NULL, 0, pdispChildren );
    if( !SUCCEEDED( hr ) ) return hr;

    // insert new frame into desktop hierarchy
    hr = ExecuteFunc( pdispChildren, L"append", &CComVariant( mpDispFrame ), 1, &dummyResult );
    if( !SUCCEEDED( hr ) ) return hr;

    // initialize window
    hr = ExecuteFunc( mpDispWin, L"setBackground", &CComVariant( (long)0xFFFFFFFF ), 1, &dummyResult );
    if( !SUCCEEDED( hr ) ) return hr;

    hr = ExecuteFunc( mpDispWin, L"setVisible", &CComVariant( TRUE ), 1, &dummyResult );
    if( !SUCCEEDED( hr ) ) return hr;

    CComVariant aPosArgs[5];
    aPosArgs[4] = CComVariant( 0 );
    aPosArgs[3] = CComVariant( 0 );
    aPosArgs[2] = CComVariant( width );
    aPosArgs[1] = CComVariant( height );
    aPosArgs[0] = CComVariant( 12 );
    hr = ExecuteFunc( mpDispWin, L"setPosSize", aPosArgs, 5, &dummyResult );
    if( !SUCCEEDED( hr ) ) return hr;


    return S_OK;
}

HRESULT CSOActiveX::CallDispatch1PBool( OLECHAR* sUrl, OLECHAR* sArgName, BOOL sArgVal )
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

    SAFEARRAY FAR* pPropVals = SafeArrayCreateVector( VT_DISPATCH, 0, 1 );
    long ix = 0;
    CComPtr<IDispatch> pdispPropVal;
    hr = GetUnoStruct( L"com.sun.star.beans.PropertyValue", pdispPropVal );
    if( !SUCCEEDED( hr ) ) return hr;

    OLECHAR*    sPropMemberNames[2] = { L"Name", L"Value" };
    CComVariant pPropVar[2];
    pPropVar[0] = CComVariant( sArgName );
    pPropVar[1] = CComVariant(); pPropVar[1].vt = VT_BOOL; pPropVar[1].boolVal = sArgVal ? VARIANT_TRUE : VARIANT_FALSE ;
    hr = PutPropertiesToIDisp( pdispPropVal, sPropMemberNames, pPropVar, 2 );
    if( !SUCCEEDED( hr ) ) return hr;

    SafeArrayPutElement( pPropVals, &ix, pdispPropVal );

    CComVariant aDispArgs[2];
    aDispArgs[1] = CComVariant( pdispURL );
    // aDispArgs[0] = CComVariant( pPropVals ); such constructor is not defined ??!
    aDispArgs[0] = CComVariant(); aDispArgs[0].vt = VT_ARRAY | VT_DISPATCH; aDispArgs[0].parray = pPropVals;

    CComVariant dummyResult;
    hr = ExecuteFunc( pdispXDispatch, L"dispatch", aDispArgs, 2, &dummyResult );
    if( !SUCCEEDED( hr ) ) return hr;

    return S_OK;
}

HRESULT CSOActiveX::ShowSomeBars()
{
    // show FunctionBar and StatusBar
    for( int ind = 0; ind < BARS_TO_SHOW; ind ++ )
    {
        HRESULT hr = CallDispatch1PBool( pSlotUrl[ind], pSlotName[ind], TRUE );
        if( !SUCCEEDED( hr ) ) return hr;
    }

    return S_OK;
}

HRESULT CSOActiveX::HideAllBars()
{
    for( int ind = 0; ind < BARS_NUMBER; ind ++ )
    {
        HRESULT hr = CallDispatch1PBool( pSlotUrl[ind], pSlotName[ind], FALSE );
        if( !SUCCEEDED( hr ) ) return hr;
    }

    return S_OK;
}

HRESULT CSOActiveX::LoadURLToFrame( )
{
    HRESULT hr = CallDispatch1PBool( mCurFileUrl, L"ReadOnly", mbViewOnly );
    if( !SUCCEEDED( hr ) ) return hr;

    if( mbViewOnly )
        HideAllBars();

    return S_OK;
}

HRESULT CSOActiveX::OnDrawAdvanced( ATL_DRAWINFO& di )
{
    if( m_spInPlaceSite && mCurFileUrl )
    {
        HWND hwnd;
        HRESULT hr = m_spInPlaceSite->GetWindow( &hwnd );
        if( !SUCCEEDED( hr ) ) return hr;

        if( mParentWin != hwnd || !mOffWin )
        {
            if( mpDispFrame )
            {
                CComVariant dummyResult;
                ExecuteFunc( mpDispFrame, L"dispose", NULL, 0, &dummyResult );
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

        if( ! mpDispFrame )
        {
            hr = CreateFrameOldWay( mOffWin,
                            di.prcBounds->right - di.prcBounds->left,
                            di.prcBounds->bottom - di.prcBounds->top );
            if( !SUCCEEDED( hr ) ) return hr;
        }

        if( mbLoad )
        {
            hr = LoadURLToFrame();
            if( !SUCCEEDED( hr ) ) return hr;
            mbLoad = FALSE;
        }
    }

    return S_OK;
}


STDMETHODIMP CSOActiveX::SetClientSite( IOleClientSite* aClientSite )
{
    HRESULT hr = IOleObjectImpl<CSOActiveX>::SetClientSite( aClientSite );

    if( !aClientSite )
    {
        ATLASSERT( mWebBrowser2 );
        if( mWebBrowser2 )
            AtlUnadvise( mWebBrowser2, DIID_DWebBrowserEvents2, mCookie );
        return hr;
    }

    CComPtr<IOleContainer> aContainer;
    m_spClientSite->GetContainer( &aContainer );
    ATLASSERT( aContainer );

    if( SUCCEEDED( hr )  && aContainer )
    {
        CComQIPtr<IServiceProvider, &IID_IServiceProvider> aServiceProvider( aContainer );
        ATLASSERT( aServiceProvider );

        if( aServiceProvider )
        {
            aServiceProvider->QueryService( SID_SInternetExplorer,
                                            IID_IWebBrowser,
                                            (void**)&mWebBrowser2 );
            ATLASSERT( mWebBrowser2 );
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
