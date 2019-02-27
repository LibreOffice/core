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

#ifndef INCLUDED_SFX2_SFXBASECONTROLLER_HXX
#define INCLUDED_SFX2_SFXBASECONTROLLER_HXX

#include <memory>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#include <com/sun/star/frame/XController2.hpp>
#include <com/sun/star/frame/XControllerBorder.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/XTitleChangeBroadcaster.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#include <com/sun/star/ui/XContextMenuInterception.hpp>
#include <com/sun/star/ui/XContextMenuInterceptor.hpp>
#include <com/sun/star/awt/XMouseClickHandler.hpp>
#include <com/sun/star/awt/XKeyHandler.hpp>
#include <com/sun/star/awt/XUserInputInterception.hpp>
#include <tools/link.hxx>
#include <vcl/button.hxx>

#include <com/sun/star/ui/XSidebarProvider.hpp>

#include <sfx2/viewsh.hxx>
#include <sfx2/sfxuno.hxx>
#include <sfx2/groupid.hxx>

struct  IMPL_SfxBaseController_DataContainer    ;   // impl. struct to hold member of class SfxBaseController

class SfxViewFrame;

sal_Int16 MapGroupIDToCommandGroup( SfxGroupId nGroupID );


//  class declarations


typedef ::cppu::WeakImplHelper  <   css::frame::XController2
                                ,   css::frame::XControllerBorder
                                ,   css::frame::XDispatchProvider
                                ,   css::task::XStatusIndicatorSupplier
                                ,   css::ui::XContextMenuInterception
                                ,   css::awt::XUserInputInterception
                                ,   css::frame::XDispatchInformationProvider
                                ,   css::frame::XTitle
                                ,   css::frame::XTitleChangeBroadcaster
                                ,   css::lang::XInitialization
                                >   SfxBaseController_Base;

class SFX2_DLLPUBLIC SfxBaseController  :public SfxBaseController_Base
                                        ,public ::cppu::BaseMutex
{

//  public methods


public:


    //  constructor/destructor


    SfxBaseController( SfxViewShell* pView ) ;

    virtual ~SfxBaseController() override ;

    SAL_DLLPRIVATE void ReleaseShell_Impl();
    SAL_DLLPRIVATE void BorderWidthsChanged_Impl();

    css::uno::Reference< css::task::XStatusIndicator > SAL_CALL getStatusIndicator(  ) override;


    //  XController2
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL getComponentWindow() override;
    virtual OUString SAL_CALL getViewControllerName() override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCreationArguments() override;

    virtual css::uno::Reference< css::ui::XSidebarProvider > SAL_CALL getSidebar() override;


    //  XController


    virtual void SAL_CALL attachFrame( const css::uno::Reference< css::frame::XFrame >& xFrame ) override ;

    virtual sal_Bool SAL_CALL attachModel( const css::uno::Reference< css::frame::XModel >& xModel ) override ;

    virtual sal_Bool SAL_CALL suspend( sal_Bool bSuspend ) override ;

    css::uno::Any SAL_CALL getViewData() override ;

    void SAL_CALL restoreViewData( const css::uno::Any& aValue ) override ;

    css::uno::Reference< css::frame::XFrame > SAL_CALL getFrame() override ;

    css::uno::Reference< css::frame::XModel > SAL_CALL getModel() override ;


    //  XDispatchProvider


    virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch(  const   css::util::URL &    aURL            ,
                                                                                  const   OUString &   sTargetFrameName,
                                                                                  sal_Int32            eSearchFlags    ) override ;

    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& seqDescriptor ) override ;


    //  XControllerBorder


    virtual css::frame::BorderWidths SAL_CALL getBorder() override;
    virtual void SAL_CALL addBorderResizeListener( const css::uno::Reference< css::frame::XBorderResizeListener >& xListener ) override;
    virtual void SAL_CALL removeBorderResizeListener( const css::uno::Reference< css::frame::XBorderResizeListener >& xListener ) override;
    virtual css::awt::Rectangle SAL_CALL queryBorderedArea( const css::awt::Rectangle& aPreliminaryRectangle ) override;


    //  XComponent


    virtual void SAL_CALL dispose() override ;

    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override ;

    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override ;
    virtual void SAL_CALL registerContextMenuInterceptor( const css::uno::Reference< css::ui::XContextMenuInterceptor >& xInterceptor ) override;
    virtual void SAL_CALL releaseContextMenuInterceptor( const css::uno::Reference< css::ui::XContextMenuInterceptor >& xInterceptor ) override;

    virtual void SAL_CALL addKeyHandler( const css::uno::Reference< css::awt::XKeyHandler >& xHandler ) override;
    virtual void SAL_CALL removeKeyHandler( const css::uno::Reference< css::awt::XKeyHandler >& xHandler ) override;
    virtual void SAL_CALL addMouseClickHandler( const css::uno::Reference< css::awt::XMouseClickHandler >& xHandler ) override;
    virtual void SAL_CALL removeMouseClickHandler( const css::uno::Reference< css::awt::XMouseClickHandler >& xHandler ) override;


    //  XDispatchInformationProvider
    virtual css::uno::Sequence< sal_Int16 > SAL_CALL getSupportedCommandGroups() override;
    virtual css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation( sal_Int16 nCommandGroup ) override;

    // css::frame::XTitle
    virtual OUString SAL_CALL getTitle(  ) override;
    virtual void SAL_CALL setTitle( const OUString& sTitle ) override;

    // css::frame::XTitleChangeBroadcaster
    virtual void SAL_CALL addTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener ) override;
    virtual void SAL_CALL removeTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener ) override;

    // css::lang::XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // FIXME: TL needs this in sw/source/ui/uno/unotxdoc.cxx now;
    // either the _Impl name should vanish or there should be an "official" API
    SfxViewShell* GetViewShell_Impl() const;
    SAL_DLLPRIVATE bool HandleEvent_Impl( NotifyEvent const & rEvent );
    SAL_DLLPRIVATE bool HasKeyListeners_Impl();
    SAL_DLLPRIVATE bool HasMouseClickListeners_Impl();
    SAL_DLLPRIVATE void SetCreationArguments_Impl( const css::uno::Sequence< css::beans::PropertyValue >& i_rCreationArgs );
    SAL_DLLPRIVATE css::uno::Reference< css::frame::XTitle > impl_getTitleHelper ();
private:
    enum ConnectSfxFrame
    {
        E_CONNECT,
        E_DISCONNECT,
        E_RECONNECT
    };
    SAL_DLLPRIVATE void ConnectSfxFrame_Impl( const ConnectSfxFrame i_eConnect );
    SAL_DLLPRIVATE SfxViewFrame& GetViewFrame_Impl() const;
    SAL_DLLPRIVATE void ShowInfoBars( );

    DECL_LINK( CheckOutHandler, Button*, void );

    std::unique_ptr<IMPL_SfxBaseController_DataContainer>   m_pData ;

} ; // class SfxBaseController

#endif  // _SFX_SFXBASECONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
