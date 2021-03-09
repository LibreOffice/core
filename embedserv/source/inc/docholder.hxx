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

#ifndef INCLUDED_EMBEDSERV_SOURCE_INC_DOCHOLDER_HXX
#define INCLUDED_EMBEDSERV_SOURCE_INC_DOCHOLDER_HXX

#include "common.h"
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/frame/XFrame2.hpp>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>

#include <rtl/ref.hxx>

#include "embeddocaccess.hxx"

class EmbedDocument_Impl;
class Interceptor;
class CIIAObj;

namespace winwrap {
    class CHatchWin;
}


class DocumentHolder :
    public ::cppu::WeakImplHelper<
                        css::util::XCloseListener,
                        css::frame::XTerminateListener,
                        css::util::XModifyListener,
                        css::ui::XDockingAreaAcceptor>
{
private:
    ::osl::Mutex                m_aMutex;

    bool                        m_bAllowInPlace;
    LPOLEINPLACESITE            m_pIOleIPSite;
    LPOLEINPLACEFRAME           m_pIOleIPFrame;
    LPOLEINPLACEUIWINDOW        m_pIOleIPUIWindow;
    winwrap::CHatchWin*         m_pCHatchWin;

    ::rtl::Reference< EmbeddedDocumentInstanceAccess_Impl > m_xOleAccess;

    css::uno::WeakReference< css::frame::XDispatchProviderInterceptor > m_xInterceptorLocker;
    rtl::Reference<Interceptor> m_pInterceptor;

    css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;

    RECT                          m_aBorder;

    // contains top level system window data
    bool                          m_bOnDeactivate;
    HWND                          m_hWndxWinParent;
    HWND                          m_hWndxWinCont;
    HMENU                         m_nMenuHandle;
    HMENU                         m_nMenuShared;
    HOLEMENU                      m_nOLEMenu;
    css::uno::Reference< css::awt::XWindow> m_xEditWindow;

    css::uno::Reference< css::awt::XWindow> m_xContainerWindow;

    css::uno::Reference< css::frame::XModel > m_xDocument;
    sal_Int16                    m_nMacroExecMode;

    css::uno::Reference< css::frame::XLayoutManager> m_xLayoutManager;


    css::uno::Reference< css::frame::XFrame2 > m_xFrame;

    OUString m_aContainerName,m_aDocumentNamePart,m_aFilterName;

    CComPtr< IDispatch > m_pIDispatch;

    bool m_bLink;


    css::uno::Reference< css::frame::XFrame2 > DocumentFrame();


    css::uno::Reference< css::frame::XDispatchProviderInterceptor >
        CreateNewInterceptor();

    void ClearInterceptorInternally();

    void LoadDocInFrame( bool bPluginMode );
public:


    // the instance to which we belong
    static HINSTANCE m_hInstance;

    HWND GetEditWindowParentHandle() const
    {
        return m_hWndxWinParent;
    }

    void SetContainerWindowHandle(HWND hWndxWinCont)
    {
        m_hWndxWinCont = hWndxWinCont;
    }

    DocumentHolder(
        const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory,
        const ::rtl::Reference< EmbeddedDocumentInstanceAccess_Impl >& xOleAccess );

    ~DocumentHolder() override;

    // Methods for inplace activation


    BOOL isActive() const;
    void DisableInplaceActivation(BOOL);
    HRESULT InPlaceActivate(LPOLECLIENTSITE,BOOL);
    void InPlaceDeactivate();
    HRESULT UIActivate();
    void UIDeactivate();
    BOOL InPlaceMenuCreate();
    BOOL InPlaceMenuDestroy();

    static void OpenIntoWindow();
    static BOOL Undo();

    // further methods

    void SetDocument(
        const css::uno::Reference< css::frame::XModel >& xDoc,
        bool bLink = false
    );

    bool ExecuteSuspendCloseFrame();

    void DisconnectFrameDocument( bool bComplete = false );
    void CloseDocument();
    void CloseFrame();
    void ClearInterceptor();
    void FreeOffice();

    void resizeWin( const SIZEL& rNewSize );

    void setTitle(const OUString& aDocumentName);
    OUString getTitle() const { return m_aDocumentNamePart; }

    void setContainerName(const OUString& aContainerName);
    OUString getContainerName() const { return m_aContainerName; }
    void OnPosRectChanged(LPRECT lpRect) const;
    void show();

    bool HasFrame() { return m_xFrame.is(); }
    bool IsLink() { return m_bLink; }

    /** hides the document window, even in case of an external container
     *  side managed window.
     */

    void hide();

    IDispatch* GetIDispatch();

    HRESULT GetDocumentBorder( RECT *pRect );
    // HRESULT SetVisArea( const RECTL *pRect );
    // HRESULT GetVisArea( RECTL *pRect );
    HRESULT SetExtent( const SIZEL *pSize );
    HRESULT GetExtent( SIZEL *pSize );
    // sets extension on the hatchwindow
    HRESULT SetContRects(LPCRECT pRect);
    HRESULT SetObjectRects(LPCRECT aRect, LPCRECT aClip);

    HWND GetTopMostWinHandle() const
    {
        return m_hWndxWinParent;
     }

    css::uno::Reference< css::frame::XModel >
    GetDocument() const
    {
        return m_xDocument;
    }

    // XEventListener
    virtual void SAL_CALL
    disposing( const css::lang::EventObject& aSource ) override;

    // XCloseListener
    virtual void SAL_CALL
    queryClosing(
        const css::lang::EventObject& aSource,
        sal_Bool bGetsOwnership
    ) override;

    virtual void SAL_CALL
    notifyClosing(
        const css::lang::EventObject& aSource
    ) override;

    // XTerminateListener
    virtual void SAL_CALL
    queryTermination(
        const css::lang::EventObject& aSource
    ) override;

    virtual void SAL_CALL
    notifyTermination(
        const css::lang::EventObject& aSource
    ) override;


    // XModifyListener
    virtual void SAL_CALL
    modified(
        const css::lang::EventObject& aEvent
    ) override;

    // XDockingAreaAcceptor

    virtual css::uno::Reference<
        css::awt::XWindow> SAL_CALL
    getContainerWindow(
    ) override;

    virtual sal_Bool SAL_CALL
    requestDockingAreaSpace(
        const css::awt::Rectangle& RequestedSpace
    ) override;

    virtual void SAL_CALL
    setDockingAreaSpace(
        const css::awt::Rectangle& BorderSpace
    ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
