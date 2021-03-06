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

// SOActiveX.h : Declaration of the CSOActiveX

#pragma once

#include "resource.h"

#include <ExDispID.h>
#include <ExDisp.h>
#include <shlguid.h>

#include <atlctl.h>

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif
#include <so_activex.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif

class SODispatchInterceptor;

enum SOVersion {
    SO_NOT_DETECTED = 0,
    SO_52,
    SO_60,
    SO_61,
    SO_UNKNOWN,
    OO_10,
    OO_11,
    OO_UNKNOWN
};


// CSOActiveX
class ATL_NO_VTABLE CSOActiveX :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<ISOActiveX, &IID_ISOActiveX, &LIBID_SO_ACTIVEXLib>,
    public CComControl<CSOActiveX>,
    public IPersistStreamInitImpl<CSOActiveX>,
    public IOleControlImpl<CSOActiveX>,
    public IOleObjectImpl<CSOActiveX>,
    public IOleInPlaceActiveObjectImpl<CSOActiveX>,
    public IViewObjectExImpl<CSOActiveX>,
    public IOleInPlaceObjectWindowlessImpl<CSOActiveX>,
//  public IConnectionPointContainerImpl<CSOActiveX>,
    public CComCoClass<CSOActiveX, &CLSID_SOActiveX>,
//  public CProxy_ItryPluginEvents< CSOActiveX >,
    public IPersistPropertyBagImpl< CSOActiveX >,
    public IProvideClassInfo2Impl<  &CLSID_SOActiveX,
                                    &DIID__ISOActiveXEvents,
                                    &LIBID_SO_ACTIVEXLib >,
    public IObjectSafetyImpl< CSOActiveX,
                              INTERFACESAFE_FOR_UNTRUSTED_DATA >
{
protected:
    CComPtr<IWebBrowser2>   mWebBrowser2;
    DWORD                   mCookie;

    CComPtr<IDispatch>      mpDispFactory;
    CComPtr<IDispatch>      mpDispFrame;
    CComPtr<IDispatch>      mpInstanceLocker;
    CComPtr<IDispatch>      mpDispWin;
    OLECHAR const *         mCurFileUrl;
    BOOL                    mbLoad;
    BOOL                    mbViewOnly;
    WNDCLASSW               mPWinClass;
    HWND                    mParentWin;
    HWND                    mOffWin;

    SODispatchInterceptor*  mpDispatchInterceptor;
    SOVersion               mnVersion;

    BOOL                    mbReadyForActivation;
    CComPtr<IDispatch>      mpDispTempFile;

    bool                    mbDrawLocked;

public:
    CSOActiveX();
    ~CSOActiveX() override;

DECLARE_REGISTRY_RESOURCEID(IDR_SOACTIVEX)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSOActiveX)
    COM_INTERFACE_ENTRY(ISOActiveX)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IViewObjectEx)
    COM_INTERFACE_ENTRY(IViewObject2)
    COM_INTERFACE_ENTRY(IViewObject)
    COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceObject)
    COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY(IOleControl)
    COM_INTERFACE_ENTRY(IOleObject)
    COM_INTERFACE_ENTRY(IPersistStreamInit)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
//  COM_INTERFACE_ENTRY(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY(IPersistPropertyBag)
    COM_INTERFACE_ENTRY(IObjectSafety)
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
END_COM_MAP()
#if defined __clang__
#pragma clang diagnostic pop
#endif

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
    // offset of on non-standard-layout type '_PropMapClass' (aka 'CSOActiveX'),
    // expanded from macro 'PROP_DATA_ENTRY'
#endif
BEGIN_PROP_MAP(CSOActiveX)
    PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
    PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
    // Example entries
    // PROP_ENTRY("Property Description", dispid, clsid)
    // PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()
#if defined __clang__
#pragma clang diagnostic pop
#endif

BEGIN_CONNECTION_POINT_MAP(CSOActiveX)
END_CONNECTION_POINT_MAP()

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
BEGIN_MSG_MAP(CSOActiveX)
#if defined __clang__
#pragma clang diagnostic pop
#endif
    CHAIN_MSG_MAP(CComControl<CSOActiveX>)
    DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);



// IViewObjectEx
    static DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

// ISOActiveX
public:

    STDMETHOD(SetClientSite)( IOleClientSite* aClientSite ) override;
    STDMETHOD(Invoke)(  DISPID dispidMember,
                        REFIID riid,
                        LCID lcid,
                        WORD wFlags,
                        DISPPARAMS* pDispParams,
                        VARIANT* pvarResult,
                        EXCEPINFO* pExcepInfo,
                        UINT* puArgErr) override;
    STDMETHOD(Load) ( LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog ) override;
    STDMETHOD(Load) ( LPSTREAM pStm ) override;
    STDMETHOD(InitNew) () override;
    HRESULT OnDrawAdvanced(ATL_DRAWINFO& di) override;
    HRESULT OnDraw(ATL_DRAWINFO& di) override;

    HRESULT SetLayoutManagerProps();
    HRESULT CreateFrameOldWay( HWND hwnd, int width, int height );
    HRESULT GetUnoStruct( OLECHAR const * sStructName, CComPtr<IDispatch>& pdispResult );
    HRESULT LoadURLToFrame();
    HRESULT CallDispatchMethod( OLECHAR const * sUrl, CComVariant* sArgNames, CComVariant* sArgVal, unsigned int count );
    HRESULT CallLoadComponentFromURL1PBool( OLECHAR const * sUrl, OLECHAR const * sArgName, BOOL sArgVal );
    HRESULT GetUrlStruct( OLECHAR const * sUrl, CComPtr<IDispatch>& pdispUrl );
    HRESULT Cleanup();
    HRESULT TerminateOffice();
    HRESULT GetURL( const OLECHAR* url,
                                const OLECHAR* target );

    void CallbackCreateXInputStream( CBindStatusCallback<CSOActiveX>* pbsc, BYTE* pBytes, DWORD dwSize );


    SOVersion GetVersionConnected();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
