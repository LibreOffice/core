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

// SOComWindowPeer.h: Definition of the SOComWindowPeer class

#ifndef INCLUDED_EXAMPLES_ACTIVEX_SOCOMWINDOWPEER_H
#define INCLUDED_EXAMPLES_ACTIVEX_SOCOMWINDOWPEER_H

#include "resource.h"
#include <ExDispID.h>
#include <ExDisp.h>
#include <shlguid.h>
#include <atlctl.h>

#include "so_activex.h"

class SOComWindowPeer :
    public IDispatchImpl<ISOComWindowPeer, &IID_ISOComWindowPeer, &LIBID_SO_ACTIVEXLib>,
    public ISupportErrorInfo,
    public CComObjectRoot,
    public CComCoClass<SOComWindowPeer,&CLSID_SOComWindowPeer>
{
    HWND m_hwnd;
public:
    SOComWindowPeer() : m_hwnd( NULL ) {}

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
            /* [in] */ SAFEARRAY __RPC_FAR * procId,
            /* [in] */ short s,
            /* [retval][out] */ long __RPC_FAR *ret)
        {
            *ret = (long) m_hwnd;
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE getToolkit(
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *retVal)
        {
            *retVal = NULL;
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE setPointer(
            /* [in] */ IDispatch __RPC_FAR *xPointer)
        {
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE setBackground(
            /* [in] */ int nColor)
        {
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE invalidate(
            /* [in] */ short __MIDL_0015)
        {
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE invalidateRect(
            /* [in] */ IDispatch __RPC_FAR *aRect,
            /* [in] */ short nFlags)
        {
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE dispose( void)
        {
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE addEventListener(
            /* [in] */ IDispatch __RPC_FAR *xListener)
        {
            return S_OK;
        }

        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE removeEventListener(
            /* [in] */ IDispatch __RPC_FAR *xListener)
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

#endif // INCLUDED_EXAMPLES_ACTIVEX_SOCOMWINDOWPEER_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
