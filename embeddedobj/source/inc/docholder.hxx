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

#ifndef INCLUDED_EMBEDDEDOBJ_SOURCE_INC_DOCHOLDER_HXX
#define INCLUDED_EMBEDDEDOBJ_SOURCE_INC_DOCHOLDER_HXX

#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XBorderResizeListener.hpp>
#include <com/sun/star/frame/BorderWidths.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/embed/XHatchWindowController.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>

class OCommonEmbeddedObject;
class Interceptor;

class DocumentHolder :
    public ::cppu::WeakImplHelper<
                        ::com::sun::star::util::XCloseListener,
                          ::com::sun::star::frame::XTerminateListener,
                        ::com::sun::star::util::XModifyListener,
                        ::com::sun::star::document::XEventListener,
                        ::com::sun::star::frame::XBorderResizeListener,
                        ::com::sun::star::embed::XHatchWindowController >
{
private:

    OCommonEmbeddedObject* m_pEmbedObj;

    Interceptor*        m_pInterceptor;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor > m_xOutplaceInterceptor;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable > m_xComponent;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > m_xOwnWindow; // set for inplace objects
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > m_xHatchWindow; // set for inplace objects

    ::com::sun::star::awt::Rectangle m_aObjRect;
    ::com::sun::star::frame::BorderWidths m_aBorderWidths;

    OUString m_aContainerName;
    OUString m_aDocumentNamePart;

    bool m_bReadOnly;

    bool m_bWaitForClose;
    bool m_bAllowClosing;
    bool m_bDesktopTerminated;

    sal_Int32 m_nNoBorderResizeReact;
    sal_Int32 m_nNoResizeReact;

    ::com::sun::star::uno::Reference< ::com::sun::star::ui::XDockingAreaAcceptor > m_xCachedDocAreaAcc;

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > m_aOutplaceFrameProps;


    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > GetDocFrame();
    bool LoadDocToFrame( bool );

    ::com::sun::star::awt::Rectangle CalculateBorderedArea( const ::com::sun::star::awt::Rectangle& aRect );
    ::com::sun::star::awt::Rectangle AddBorderToArea( const ::com::sun::star::awt::Rectangle& aRect );

    void ResizeWindows_Impl( const ::com::sun::star::awt::Rectangle& aHatchRect );

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > RetrieveOwnMenu_Impl();
    bool MergeMenus_Impl(
                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >& xOwnLM,
                   const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >& xContLM,
                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xContDisp,
                const OUString& aContModuleName );

public:

    static void FindConnectPoints(
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xMenu,
        sal_Int32 nConnectPoints[2] )
            throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > MergeMenusForInplace(
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xContMenu,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xContDisp,
        const OUString& aContModuleName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xOwnMenu,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xOwnDisp )
            throw ( ::com::sun::star::uno::Exception );


    DocumentHolder( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext,
                    OCommonEmbeddedObject* pEmbObj );
    virtual ~DocumentHolder();

    OCommonEmbeddedObject* GetEmbedObject() { return m_pEmbedObj; }

    void SetComponent( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable >& xDoc, bool bReadOnly );
    void ResizeHatchWindow();
    void FreeOffice();

    void CloseDocument( bool bDeliverOwnership, bool bWaitForClose );
    void CloseFrame();

    OUString GetTitle() const
    {
        return m_aContainerName + " - " + m_aDocumentNamePart;
    }

    void SetOutplaceFrameProperties( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aProps )
        { m_aOutplaceFrameProps = aProps; }

    void PlaceFrame( const ::com::sun::star::awt::Rectangle& aNewRect );

    static bool SetFrameLMVisibility( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                                    bool bVisible );

    bool ShowInplace( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParent,
                      const ::com::sun::star::awt::Rectangle& aRectangleToShow,
                      const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xContainerDP );

    bool ShowUI(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >& xContainerLM,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xContainerDP,
        const OUString& aContModuleName );
    bool HideUI(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >& xContainerLM );

    void Show();

    bool SetExtent( sal_Int64 nAspect, const ::com::sun::star::awt::Size& aSize );
    bool GetExtent( sal_Int64 nAspect, ::com::sun::star::awt::Size *pSize );

    sal_Int32 GetMapUnit( sal_Int64 nAspect );

    void SetOutplaceDispatchInterceptor(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >&
                                                                                            xOutplaceInterceptor )
    {
        m_xOutplaceInterceptor = xOutplaceInterceptor;
    }

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable > GetComponent() { return m_xComponent; }

// XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// XCloseListener
    virtual void SAL_CALL queryClosing( const ::com::sun::star::lang::EventObject& Source, sal_Bool GetsOwnership ) throw (::com::sun::star::util::CloseVetoException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL notifyClosing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// XTerminateListener
    virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

// XEventListener
    virtual void SAL_CALL notifyEvent( const ::com::sun::star::document::EventObject& Event ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

// XBorderResizeListener
    virtual void SAL_CALL borderWidthsChanged( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& aObject, const ::com::sun::star::frame::BorderWidths& aNewSize ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// XHatchWindowController
    virtual void SAL_CALL requestPositioning( const ::com::sun::star::awt::Rectangle& aRect ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Rectangle SAL_CALL calcAdjustedRectangle( const ::com::sun::star::awt::Rectangle& aRect ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL activated(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL deactivated(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
