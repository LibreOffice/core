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

// SODispatchInterceptor.cpp : Implementation of CHelpApp and DLL registration.

#include "stdio.h"
#include "stdafx2.h"
#include "so_activex.h"
#include "SOActiveX.h"
#include "SODispatchInterceptor.h"
#include "com_uno_helper.h"
#include <sal/macros.h>




STDMETHODIMP SODispatchInterceptor::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] =
    {
        &IID_ISODispatchInterceptor,
    };

    for (int i=0;i<SAL_N_ELEMENTS(arr);i++)
    {
#ifdef _MSC_VER
        if (InlineIsEqualGUID(*arr[i],riid))
#else
        if (::ATL::InlineIsEqualGUID(*arr[i],riid))
#endif
            return S_OK;
    }
    return S_FALSE;
}

STDMETHODIMP SODispatchInterceptor::queryDispatch( IDispatch FAR* aURL,
                                                   BSTR aTargetFrameName,
                                                   long nSearchFlags,
                                                   IDispatch FAR* FAR* retVal )
{
    if ( !aURL || !retVal ) return E_FAIL;

    CComVariant aTargetUrl;
    OLECHAR* sURLMemberName = L"Complete";
    DISPID nURLID;
    HRESULT hr = aURL->GetIDsOfNames( IID_NULL, &sURLMemberName, 1, LOCALE_USER_DEFAULT, &nURLID );
    if( !SUCCEEDED( hr ) ) return hr;

    hr = CComDispatchDriver::GetProperty( aURL, nURLID, &aTargetUrl );
    if( !SUCCEEDED( hr ) ) return hr;

    if( aTargetUrl.vt != VT_BSTR  ) return E_FAIL;

    USES_CONVERSION;
    if( !strncmp( OLE2T( aTargetUrl.bstrVal ), ".uno:OpenHyperlink", 18 ) )
    {
        CComQIPtr< IDispatch, &IID_IDispatch > pIDisp( this );
        if( pIDisp )
        {
            this->AddRef();
            *retVal = pIDisp;
        }
    }
    else
    {
        if( !m_xSlave )
        {
            *retVal = NULL;
            return S_OK;
        }

        CComVariant aResult;
        CComVariant aArgs[3];
        aArgs[0] = CComVariant( nSearchFlags );
        aArgs[1] = CComVariant( aTargetFrameName );
        aArgs[2] = CComVariant( aURL );

        hr = ExecuteFunc( m_xSlave, L"queryDispatch", aArgs, 3, &aResult );
        if( !SUCCEEDED( hr ) || aResult.vt != VT_DISPATCH || aResult.pdispVal == NULL )
        {
            *retVal = NULL;
            return S_OK;
        }

        *retVal = aResult.pdispVal;

        CComQIPtr< IUnknown, &IID_IUnknown > pIUnk( *retVal );
        if( pIUnk )
            (*retVal)->AddRef();
    }

    return S_OK;
}

STDMETHODIMP SODispatchInterceptor::queryDispatches( SAFEARRAY FAR* aDescripts, SAFEARRAY FAR* FAR* retVal)
{
    if ( !aDescripts || !retVal || SafeArrayGetDim( aDescripts ) != 1 )
        return E_FAIL;

    long nLB, nUB;

    HRESULT hr = SafeArrayGetLBound( aDescripts, 1, &nLB );
    if( !SUCCEEDED( hr ) ) return hr;

    hr = SafeArrayGetUBound( aDescripts, 1, &nUB );
    if( !SUCCEEDED( hr ) ) return hr;
    if( nUB < nLB ) return E_FAIL;

    *retVal = SafeArrayCreateVector( VT_DISPATCH, 0, nUB - nLB );

    for ( long ind = nLB; ind <= nUB; ind ++ )
    {
        CComPtr<IDispatch> pElem;
        SafeArrayGetElement( aDescripts, &ind, pElem );
        if( pElem )
        {
            OLECHAR* pMemberNames[3] = { L"FeatureURL", L"FrameName", L"SearchFlags" };
            CComVariant pValues[3];
            hr = GetPropertiesFromIDisp( pElem, pMemberNames, pValues, 3 );
            if( !SUCCEEDED( hr ) ) return hr;
            if( pValues[0].vt != VT_DISPATCH || pValues[0].pdispVal == NULL
             || pValues[1].vt != VT_BSTR || pValues[2].vt != VT_I4 )
                return E_FAIL;

            CComPtr<IDispatch> aRes;
            hr = queryDispatch( pValues[0].pdispVal, pValues[1].bstrVal, pValues[2].lVal, &aRes );
            SafeArrayPutElement( *retVal, &ind, aRes );
        }
    }

    return S_OK;
}


STDMETHODIMP SODispatchInterceptor::dispatch( IDispatch FAR* aURL, SAFEARRAY FAR* aArgs)
{
    // get url from aURL
    OLECHAR* pUrlName = L"Complete";
    CComVariant pValue;
    HRESULT hr = GetPropertiesFromIDisp( aURL, &pUrlName, &pValue, 1 );
    if( !SUCCEEDED( hr ) ) return hr;
    if( pValue.vt != VT_BSTR || pValue.bstrVal == NULL )
        return E_FAIL;

    USES_CONVERSION;
    if( !strncmp( OLE2T( pValue.bstrVal ), ".uno:OpenHyperlink", 18 ) )
    {
        long nLB = 0, nUB = 0;
        // long nDim = SafeArrayGetDim( aArgs );

        hr = SafeArrayGetLBound( aArgs, 1, &nLB );
        if( !SUCCEEDED( hr ) ) return hr;

        hr = SafeArrayGetUBound( aArgs, 1, &nUB );
        if( !SUCCEEDED( hr ) ) return hr;
        if( nUB < nLB ) return E_FAIL;

        for ( long ind = nLB; ind <= nUB; ind ++ )
        {
            CComVariant pVarElem;
            SafeArrayGetElement( aArgs, &ind, &pVarElem );
            if( pVarElem.vt == VT_DISPATCH && pVarElem.pdispVal != NULL )
            {
                OLECHAR* pMemberNames[2] = { L"Name", L"Value" };
                CComVariant pValues[2];
                hr = GetPropertiesFromIDisp( pVarElem.pdispVal, pMemberNames, pValues, 2 );
                if( !SUCCEEDED( hr ) ) return hr;

                if( pValues[0].vt == VT_BSTR && pValues[1].vt == VT_BSTR )
                {
                    USES_CONVERSION;
                    if( !strncmp( OLE2T( pValues[0].bstrVal ), "URL", 3 ) )
                    {
                        EnterCriticalSection( &mMutex );
                        if( m_xParentControl )
                        {
                            // call GetUrl to the browser instance
                            m_xParentControl->GetURL( pValues[1].bstrVal, L"_self" );
                        }
                        LeaveCriticalSection( &mMutex );

                        break;
                    }
                }
            }
        }
    }

    return S_OK;
}

STDMETHODIMP SODispatchInterceptor::addStatusListener( IDispatch FAR* /*xControl*/, IDispatch FAR* /*aURL*/)
{
    // not implemented
    return S_OK;
}

STDMETHODIMP SODispatchInterceptor::removeStatusListener( IDispatch FAR* /*xControl*/, IDispatch FAR* /*aURL*/)
{
    // not implemented
    return S_OK;
}

STDMETHODIMP SODispatchInterceptor::getInterceptedURLs( SAFEARRAY FAR* FAR* pVal )
{
    *pVal = SafeArrayCreateVector( VT_BSTR, 0, 3 );

    if( !*pVal )
        return E_FAIL;

    long ix = 0;
    CComBSTR aPattern( OLESTR( "ftp://*" ) );
    SafeArrayPutElement( *pVal, &ix, aPattern );

    ix = 1;
    aPattern = CComBSTR( OLESTR( "http://*" ) );
    SafeArrayPutElement( *pVal, &ix, aPattern );

    ix = 2;
    aPattern = CComBSTR( OLESTR( "file://*" ) );
    SafeArrayPutElement( *pVal, &ix, aPattern );

    return S_OK;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
