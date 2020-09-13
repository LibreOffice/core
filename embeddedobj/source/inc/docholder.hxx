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

#pragma once

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
#include <rtl/ref.hxx>

class OCommonEmbeddedObject;
class Interceptor;

class DocumentHolder :
    public ::cppu::WeakImplHelper<
                        css::util::XCloseListener,
                        css::frame::XTerminateListener,
                        css::util::XModifyListener,
                        css::document::XEventListener,
                        css::frame::XBorderResizeListener,
                        css::embed::XHatchWindowController >
{
private:

    OCommonEmbeddedObject* m_pEmbedObj;

    rtl::Reference<Interceptor>        m_xInterceptor;
    css::uno::Reference< css::frame::XDispatchProviderInterceptor > m_xOutplaceInterceptor;

    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    css::uno::Reference< css::util::XCloseable > m_xComponent;

    css::uno::Reference< css::frame::XFrame > m_xFrame;
    css::uno::Reference< css::awt::XWindow > m_xOwnWindow; // set for inplace objects
    css::uno::Reference< css::awt::XWindow > m_xHatchWindow; // set for inplace objects

    css::awt::Rectangle m_aObjRect;
    css::frame::BorderWidths m_aBorderWidths;

    OUString m_aContainerName;
    OUString m_aDocumentNamePart;

    bool m_bReadOnly;

    bool m_bWaitForClose;
    bool m_bAllowClosing;
    bool m_bDesktopTerminated;

    sal_Int32 m_nNoBorderResizeReact;
    sal_Int32 m_nNoResizeReact;

    css::uno::Sequence< css::uno::Any > m_aOutplaceFrameProps;


    css::uno::Reference< css::frame::XFrame > const & GetDocFrame();
    bool LoadDocToFrame( bool );

    css::awt::Rectangle CalculateBorderedArea( const css::awt::Rectangle& aRect );
    css::awt::Rectangle AddBorderToArea( const css::awt::Rectangle& aRect );

    void ResizeWindows_Impl( const css::awt::Rectangle& aHatchRect );

    css::uno::Reference< css::container::XIndexAccess > RetrieveOwnMenu_Impl();
    bool MergeMenus_Impl(
                const css::uno::Reference< css::frame::XLayoutManager >& xOwnLM,
                const css::uno::Reference< css::frame::XLayoutManager >& xContLM,
                const css::uno::Reference< css::frame::XDispatchProvider >& xContDisp,
                const OUString& aContModuleName );

public:
    /// @throws css::uno::Exception
    static void FindConnectPoints(
        const css::uno::Reference< css::container::XIndexAccess >& xMenu,
        sal_Int32 nConnectPoints[2] );

    /// @throws css::uno::Exception
    static css::uno::Reference< css::container::XIndexAccess > MergeMenusForInplace(
        const css::uno::Reference< css::container::XIndexAccess >& xContMenu,
        const css::uno::Reference< css::frame::XDispatchProvider >& xContDisp,
        const OUString& aContModuleName,
        const css::uno::Reference< css::container::XIndexAccess >& xOwnMenu,
        const css::uno::Reference< css::frame::XDispatchProvider >& xOwnDisp );


    DocumentHolder( const css::uno::Reference< css::uno::XComponentContext >& xContext,
                    OCommonEmbeddedObject* pEmbObj );
    virtual ~DocumentHolder() override;

    OCommonEmbeddedObject* GetEmbedObject() { return m_pEmbedObj; }

    void SetComponent( const css::uno::Reference< css::util::XCloseable >& xDoc, bool bReadOnly );
    void ResizeHatchWindow();
    void FreeOffice();

    void CloseDocument( bool bDeliverOwnership, bool bWaitForClose );
    void CloseFrame();

    OUString GetTitle() const
    {
        return m_aContainerName + ( m_aDocumentNamePart.isEmpty() ? OUString() : ( " - " + m_aDocumentNamePart ) );
    }

    OUString const & GetContainerName() const
    {
        return m_aContainerName;
    }

    void SetOutplaceFrameProperties( const css::uno::Sequence< css::uno::Any >& aProps )
        { m_aOutplaceFrameProps = aProps; }

    void PlaceFrame( const css::awt::Rectangle& aNewRect );

    static bool SetFrameLMVisibility( const css::uno::Reference< css::frame::XFrame >& xFrame,
                                    bool bVisible );

    bool ShowInplace( const css::uno::Reference< css::awt::XWindowPeer >& xParent,
                      const css::awt::Rectangle& aRectangleToShow,
                      const css::uno::Reference< css::frame::XDispatchProvider >& xContainerDP );

    bool ShowUI(
        const css::uno::Reference< css::frame::XLayoutManager >& xContainerLM,
        const css::uno::Reference< css::frame::XDispatchProvider >& xContainerDP,
        const OUString& aContModuleName );
    bool HideUI(
        const css::uno::Reference< css::frame::XLayoutManager >& xContainerLM );

    void Show();

    bool SetExtent( sal_Int64 nAspect, const css::awt::Size& aSize );
    bool GetExtent( sal_Int64 nAspect, css::awt::Size *pSize );

    sal_Int32 GetMapUnit( sal_Int64 nAspect );

    void SetOutplaceDispatchInterceptor(
        const css::uno::Reference< css::frame::XDispatchProviderInterceptor >&
                                                                                            xOutplaceInterceptor )
    {
        m_xOutplaceInterceptor = xOutplaceInterceptor;
    }

    const css::uno::Reference< css::util::XCloseable >& GetComponent() const { return m_xComponent; }

// XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

// XCloseListener
    virtual void SAL_CALL queryClosing( const css::lang::EventObject& Source, sal_Bool GetsOwnership ) override;
    virtual void SAL_CALL notifyClosing( const css::lang::EventObject& Source ) override;

// XTerminateListener
    virtual void SAL_CALL queryTermination( const css::lang::EventObject& Event ) override;
    virtual void SAL_CALL notifyTermination( const css::lang::EventObject& Event ) override;

// XModifyListener
    virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) override;

// XEventListener
    virtual void SAL_CALL notifyEvent( const css::document::EventObject& Event ) override;

// XBorderResizeListener
    virtual void SAL_CALL borderWidthsChanged( const css::uno::Reference< css::uno::XInterface >& aObject, const css::frame::BorderWidths& aNewSize ) override;

// XHatchWindowController
    virtual void SAL_CALL requestPositioning( const css::awt::Rectangle& aRect ) override;
    virtual css::awt::Rectangle SAL_CALL calcAdjustedRectangle( const css::awt::Rectangle& aRect ) override;
    virtual void SAL_CALL activated(  ) override;
    virtual void SAL_CALL deactivated(  ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
