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

#ifndef _DOCHOLDER_HXX_
#define _DOCHOLDER_HXX_

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
#include <cppuhelper/implbase6.hxx>

class OCommonEmbeddedObject;
class Interceptor;

class DocumentHolder :
    public ::cppu::WeakImplHelper6<
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

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable > m_xComponent;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > m_xOwnWindow; // set for inplace objects
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > m_xHatchWindow; // set for inplace objects

    ::com::sun::star::awt::Rectangle m_aObjRect;
    ::com::sun::star::frame::BorderWidths m_aBorderWidths;

    ::rtl::OUString m_aContainerName;
    ::rtl::OUString m_aDocumentNamePart;

    sal_Bool m_bReadOnly;

    sal_Bool m_bWaitForClose;
    sal_Bool m_bAllowClosing;
    sal_Bool m_bDesktopTerminated;

    sal_Int32 m_nNoBorderResizeReact;
    sal_Int32 m_nNoResizeReact;

    ::com::sun::star::uno::Reference< ::com::sun::star::ui::XDockingAreaAcceptor > m_xCachedDocAreaAcc;

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > m_aOutplaceFrameProps;


    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > GetDocFrame();
    sal_Bool LoadDocToFrame( sal_Bool );

    ::com::sun::star::awt::Rectangle CalculateBorderedArea( const ::com::sun::star::awt::Rectangle& aRect );
    ::com::sun::star::awt::Rectangle AddBorderToArea( const ::com::sun::star::awt::Rectangle& aRect );

    void ResizeWindows_Impl( const ::com::sun::star::awt::Rectangle& aHatchRect );

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > RetrieveOwnMenu_Impl();
    sal_Bool MergeMenues_Impl(
                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >& xOwnLM,
                   const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >& xContLM,
                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xContDisp,
                const ::rtl::OUString& aContModuleName );

public:

    static void FindConnectPoints(
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xMenu,
        sal_Int32 nConnectPoints[2] )
            throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > MergeMenuesForInplace(
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xContMenu,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xContDisp,
        const ::rtl::OUString& aContModuleName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xOwnMenu,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xOwnDisp )
            throw ( ::com::sun::star::uno::Exception );


    DocumentHolder( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
                    OCommonEmbeddedObject* pEmbObj );
    ~DocumentHolder();

    OCommonEmbeddedObject* GetEmbedObject() { return m_pEmbedObj; }

    void SetComponent( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable >& xDoc, sal_Bool bReadOnly );
    void ResizeHatchWindow();
    void LockOffice();
    void FreeOffice();

    void CloseDocument( sal_Bool bDeliverOwnership, sal_Bool bWaitForClose );
    void CloseFrame();

    rtl::OUString GetTitle() const
    {
        return m_aContainerName + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( " - " )) + m_aDocumentNamePart;
    }

    rtl::OUString GetContainerName() const { return m_aContainerName; }

    void SetOutplaceFrameProperties( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aProps )
        { m_aOutplaceFrameProps = aProps; }

    void PlaceFrame( const ::com::sun::star::awt::Rectangle& aNewRect );

    sal_Bool SetFrameLMVisibility( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                                    sal_Bool bVisible );

    sal_Bool ShowInplace( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParent,
                      const ::com::sun::star::awt::Rectangle& aRectangleToShow,
                      const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xContainerDP );

    sal_Bool ShowUI(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >& xContainerLM,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xContainerDP,
        const ::rtl::OUString& aContModuleName );
    sal_Bool HideUI(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >& xContainerLM );

    void Show();

    sal_Bool SetExtent( sal_Int64 nAspect, const ::com::sun::star::awt::Size& aSize );
    sal_Bool GetExtent( sal_Int64 nAspect, ::com::sun::star::awt::Size *pSize );

    sal_Int32 GetMapUnit( sal_Int64 nAspect );

    void SetOutplaceDispatchInterceptor(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >&
                                                                                            xOutplaceInterceptor )
    {
        m_xOutplaceInterceptor = xOutplaceInterceptor;
    }

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable > GetComponent() { return m_xComponent; }

// XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

// XCloseListener
    virtual void SAL_CALL queryClosing( const ::com::sun::star::lang::EventObject& Source, sal_Bool GetsOwnership ) throw (::com::sun::star::util::CloseVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyClosing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

// XTerminateListener
    virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException);

// XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw ( ::com::sun::star::uno::RuntimeException );

// XEventListener
    virtual void SAL_CALL notifyEvent( const ::com::sun::star::document::EventObject& Event ) throw ( ::com::sun::star::uno::RuntimeException );

// XBorderResizeListener
    virtual void SAL_CALL borderWidthsChanged( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& aObject, const ::com::sun::star::frame::BorderWidths& aNewSize ) throw (::com::sun::star::uno::RuntimeException);

// XHatchWindowController
    virtual void SAL_CALL requestPositioning( const ::com::sun::star::awt::Rectangle& aRect ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL calcAdjustedRectangle( const ::com::sun::star::awt::Rectangle& aRect ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL activated(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL deactivated(  ) throw (::com::sun::star::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
