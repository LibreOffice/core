/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: docholder.hxx,v $
 * $Revision: 1.21 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DOCHOLDER_HXX_
#define _DOCHOLDER_HXX_

#include "common.h"
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <cppuhelper/implbase4.hxx>

#include <rtl/ref.hxx>

#include "embeddocaccess.hxx"

class EmbedDocument_Impl;
class Interceptor;
class CIIAObj;

namespace winwrap {
    class CHatchWin;
}


class DocumentHolder :
    public ::cppu::WeakImplHelper4<
                        ::com::sun::star::util::XCloseListener,
                          ::com::sun::star::frame::XTerminateListener,
                        ::com::sun::star::util::XModifyListener,
                        ::com::sun::star::ui::XDockingAreaAcceptor>
{
private:
    ::osl::Mutex                m_aMutex;

    BOOL                        m_bAllowInPlace;
    LPOLEINPLACESITE            m_pIOleIPSite;
    LPOLEINPLACEFRAME           m_pIOleIPFrame;
    LPOLEINPLACEUIWINDOW        m_pIOleIPUIWindow;
    winwrap::CHatchWin*         m_pCHatchWin;

    ::rtl::Reference< EmbeddedDocumentInstanceAccess_Impl > m_xOleAccess;

    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XDispatchProviderInterceptor > m_xInterceptorLocker;
    Interceptor*                m_pInterceptor;

    ::com::sun::star::uno::Reference<
    ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    RECT                          m_aBorder;

    // contains top level system window data
    bool                          m_bOnDeactivate;
    HWND                          m_hWndxWinParent;
    HWND                          m_hWndxWinCont;
    HMENU                         m_nMenuHandle;
    HMENU                         m_nMenuShared;
    HOLEMENU                      m_nOLEMenu;
    com::sun::star::uno::Reference<
        com::sun::star::awt::XWindow> m_xEditWindow;

    com::sun::star::uno::Reference<
        com::sun::star::awt::XWindow> m_xContainerWindow;

    ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XModel > m_xDocument;
    sal_Int16                    m_nMacroExecMode;

    ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XLayoutManager> m_xLayoutManager;


    ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XFrame > m_xFrame;

    ::rtl::OUString m_aContainerName,m_aDocumentNamePart,m_aFilterName;

    CComPtr< IDispatch > m_pIDispatch;

    sal_Bool m_bLink;


    ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XFrame > DocumentFrame();


    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >
        CreateNewInterceptor();

    void ClearInterceptorInternally();

    void LoadDocInFrame( sal_Bool bPluginMode );
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
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
        const ::rtl::Reference< EmbeddedDocumentInstanceAccess_Impl >& xOleAccess );

    ~DocumentHolder();

    // Methods for inplace activation


    BOOL isActive() const;
    void DisableInplaceActivation(BOOL);
    HRESULT InPlaceActivate(LPOLECLIENTSITE,BOOL);
    void InPlaceDeactivate(void);
    HRESULT UIActivate();
    void UIDeactivate();
    BOOL InPlaceMenuCreate(void);
    BOOL InPlaceMenuDestroy(void);

    void OpenIntoWindow(void);
    BOOL Undo(void);

    // further methods

    void SetDocument(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XModel >& xDoc,
        sal_Bool bLink = sal_False
    );

    sal_Bool ExecuteSuspendCloseFrame();

    void DisconnectFrameDocument();
    void CloseDocument();
    void CloseFrame();
    void ClearInterceptor();
    void FreeOffice();

    void resizeWin( const SIZEL& rNewSize );

    void setTitle(const rtl::OUString& aDocumentName);
    rtl::OUString getTitle() const { return m_aDocumentNamePart; }

    void setContainerName(const rtl::OUString& aContainerName);
    rtl::OUString getContainerName() const { return m_aContainerName; }
    void OnPosRectChanged(LPRECT lpRect) const;
    void show();


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

    ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XModel >
    GetDocument() const
    {
        return m_xDocument;
    }

    // XEventListener
    virtual void SAL_CALL
    disposing( const com::sun::star::lang::EventObject& aSource )
        throw( ::com::sun::star::uno::RuntimeException );

    // XCloseListener
    virtual void SAL_CALL
    queryClosing(
        const com::sun::star::lang::EventObject& aSource,
        sal_Bool bGetsOwnership
    )
        throw(
            ::com::sun::star::util::CloseVetoException
        );

    virtual void SAL_CALL
    notifyClosing(
        const com::sun::star::lang::EventObject& aSource
    )
        throw( ::com::sun::star::uno::RuntimeException );

    // XTerminateListener
    virtual void SAL_CALL
    queryTermination(
        const com::sun::star::lang::EventObject& aSource
    )
        throw(
            ::com::sun::star::frame::TerminationVetoException
        );

    virtual void SAL_CALL
    notifyTermination(
        const com::sun::star::lang::EventObject& aSource
    )
        throw( ::com::sun::star::uno::RuntimeException );


    // XModifyListener
    virtual void SAL_CALL
    modified(
        const ::com::sun::star::lang::EventObject& aEvent
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    // XDockingAreaAcceptor

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XWindow> SAL_CALL
    getContainerWindow(
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual sal_Bool SAL_CALL
    requestDockingAreaSpace(
        const ::com::sun::star::awt::Rectangle& RequestedSpace
    )
        throw(
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    setDockingAreaSpace(
        const ::com::sun::star::awt::Rectangle& BorderSpace
    )
        throw (
            ::com::sun::star::uno::RuntimeException
        );
};

#endif

