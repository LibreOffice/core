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

// SOComWindowPeer.h: Definition of the SOComWindowPeer class

#ifndef __SOCOMWINDOWPEER_H_
#define __SOCOMWINDOWPEER_H_

#ifdef _MSC_VER
#pragma once
#endif

#include "resource.h"       // main symbols
#include <ExDispID.h>
#include <ExDisp.h>
#include <shlguid.h>

#include <atlctl.h>

#include "so_activex.h"

/////////////////////////////////////////////////////////////////////////////
// SOComWindowPeer

class SOComWindowPeer :
    public IDispatchImpl<ISOComWindowPeer, &IID_ISOComWindowPeer, &LIBID_SO_ACTIVEXLib>,
    public ISupportErrorInfo,
    public CComObjectRoot,
    public CComCoClass<SOComWindowPeer,&CLSID_SOComWindowPeer>
{
    HWND m_hwnd;
public:
    SOComWindowPeer() : m_hwnd( NULL ) {}
    virtual ~SOComWindowPeer() { }

BEGIN_COM_MAP(SOComWindowPeer)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISOComWindowPeer)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
DECLARE_NOT_AGGREGATABLE(SOComWindowPeer)
// Remove the comment from the line above if you don't want your object to
// support aggregation.

DECLARE_REGISTRY_RESOURCEID(IDR_SOCOMWINDOWPEER)

// ISupportsErrorInfo
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// ISOComWindowPeer
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getWindowHandle(
            /* [in] */ SAFEARRAY __RPC_FAR * /*procId*/,
            /* [in] */ short /*s*/,
            /* [retval][out] */ long __RPC_FAR *ret)
        {
            *ret = HandleToLong( m_hwnd );
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getToolkit(
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *retVal)
        {
            *retVal = NULL;
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE setPointer(
            /* [in] */ IDispatch __RPC_FAR* /*xPointer*/)
        {
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE setBackground(
            /* [in] */ int /*nColor*/)
        {
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE invalidate(
            /* [in] */ short /*__MIDL_0015*/)
        {
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE invalidateRect(
            /* [in] */ IDispatch __RPC_FAR* /*aRect*/,
            /* [in] */ short /*nFlags*/)
        {
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE dispose( void)
        {
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE addEventListener(
            /* [in] */ IDispatch __RPC_FAR* /*xListener*/)
        {
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE removeEventListener(
            /* [in] */ IDispatch __RPC_FAR* /*xListener*/)
        {
            return S_OK;
        }

        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Bridge_implementedInterfaces(
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pVal)
        {
            *pVal = SafeArrayCreateVector( VT_BSTR, 0, 2 );

            if( !*pVal )
                return E_FAIL;

            long ix = 0;
            CComBSTR aInterface( OLESTR( "com.sun.star.awt.XSystemDependentWindowPeer" ) );
            SafeArrayPutElement( *pVal, &ix, aInterface );

            ix = 1;
            aInterface = CComBSTR( OLESTR( "com.sun.star.awt.XWindowPeer" ) );
            SafeArrayPutElement( *pVal, &ix, aInterface );

            return S_OK;
        }

        void SetHWNDInternally( HWND hwnd ) { m_hwnd = hwnd; }
};

#endif // __SOCOMWINDOWPEER_H_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
