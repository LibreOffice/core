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

// SODispatchInterceptor.h: Definition of the SODispatchInterceptor class

#pragma once

#ifdef _MSC_VER
#pragma once
#endif

#include "resource.h"
#include <ExDispID.h>
#include <ExDisp.h>
#include <shlguid.h>

#include <atlctl.h>

#include <so_activex.h>

class CSOActiveX;


// SODispatchInterceptor

class SODispatchInterceptor :
    public IDispatchImpl<ISODispatchInterceptor, &IID_ISODispatchInterceptor, &LIBID_SO_ACTIVEXLib>,
    public ISupportErrorInfo,
    public CComObjectRoot,
    public CComCoClass<SODispatchInterceptor,&CLSID_SODispatchInterceptor>
{
    CComPtr<IDispatch>  m_xMaster;
    CComPtr<IDispatch>  m_xSlave;
    CSOActiveX*         m_xParentControl;
    CRITICAL_SECTION    mMutex;
public:
    SODispatchInterceptor() : m_xParentControl( nullptr ) { InitializeCriticalSection(&mMutex); }
    virtual ~SODispatchInterceptor() { ATLASSERT( !m_xParentControl ); DeleteCriticalSection(&mMutex); }

BEGIN_COM_MAP(SODispatchInterceptor)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISODispatchInterceptor)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
END_COM_MAP()
#if defined __clang__
#pragma clang diagnostic pop
#endif
DECLARE_NOT_AGGREGATABLE(SODispatchInterceptor)
// Remove the comment from the line above if you don't want your object to
// support aggregation.

DECLARE_REGISTRY_RESOURCEID(IDR_SODISPATCHINTERCEPTOR)

    void SetParent( CSOActiveX* pParent )
    {
        ATLASSERT( !m_xParentControl );
        EnterCriticalSection( &mMutex );
        m_xParentControl = pParent;
        LeaveCriticalSection( &mMutex );
    }

    void ClearParent()
    {
        EnterCriticalSection( &mMutex );
        m_xParentControl = nullptr;
        LeaveCriticalSection( &mMutex );
    }

// ISupportsErrorInfo
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid) override;

// ISODispatchInterceptor

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getSlaveDispatchProvider(
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *retVal) override
        {
            *retVal = m_xSlave;
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE setSlaveDispatchProvider(
            /* [in] */ IDispatch __RPC_FAR *xNewDispatchProvider) override
        {
            m_xSlave = xNewDispatchProvider;
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getMasterDispatchProvider(
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *retVal) override
        {
            *retVal = m_xMaster;
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE setMasterDispatchProvider(
            /* [in] */ IDispatch __RPC_FAR *xNewSupplier) override
        {
            m_xMaster = xNewSupplier;
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE queryDispatch(
            /* [in] */ IDispatch __RPC_FAR *aURL,
            /* [in] */ BSTR aTargetFrameName,
            /* [in] */ long nSearchFlags,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *retVal) override;

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE queryDispatches(
            /* [in] */ SAFEARRAY __RPC_FAR * aDescripts,
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *retVal) override;

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE dispatch(
            /* [in] */ IDispatch __RPC_FAR *aURL,
            /* [in] */ SAFEARRAY __RPC_FAR * aArgs) override;

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE addStatusListener(
            /* [in] */ IDispatch __RPC_FAR *xControl,
            /* [in] */ IDispatch __RPC_FAR *aURL) override;

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE removeStatusListener(
            /* [in] */ IDispatch __RPC_FAR *xControl,
            /* [in] */ IDispatch __RPC_FAR *aURL) override;

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getInterceptedURLs(
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pVal) override;

        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Bridge_implementedInterfaces(
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pVal) override
        {
            *pVal = SafeArrayCreateVector( VT_BSTR, 0, 4 );

            if( !*pVal )
                return E_FAIL;

            LONG ix = 0;
            CComBSTR aInterface( OLESTR( "com.sun.star.frame.XDispatchProviderInterceptor" ) );
            SafeArrayPutElement( *pVal, &ix, aInterface );

            ix = 1;
            aInterface = CComBSTR( OLESTR( "com.sun.star.frame.XDispatchProvider" ) );
            SafeArrayPutElement( *pVal, &ix, aInterface );

            ix = 2;
            aInterface = CComBSTR( OLESTR( "com.sun.star.frame.XDispatch" ) );
            SafeArrayPutElement( *pVal, &ix, aInterface );

            ix = 3;
            aInterface = CComBSTR( OLESTR( "com.sun.star.frame.XInterceptorInfo" ) );
            SafeArrayPutElement( *pVal, &ix, aInterface );

            return S_OK;
        }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
