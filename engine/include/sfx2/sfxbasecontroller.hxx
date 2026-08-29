/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#include <com/sun/star/frame/XController2.hpp>
#include <com/sun/star/frame/XControllerBorder.hpp>
#include <com/sun/star/frame/XInfobarProvider.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/XTitleChangeBroadcaster.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#include <com/sun/star/ui/XContextMenuInterception.hpp>
#include <com/sun/star/awt/XUserInputInterception.hpp>
#include <tools/link.hxx>

#include <sfx2/groupid.hxx>

namespace weld { class Button; }

struct  IMPL_SfxBaseController_DataContainer    ;   // impl. struct to hold member of class SfxBaseController

class NotifyEvent;
class SfxViewFrame;
class SfxViewShell;

sal_Int16 MapGroupIDToCommandGroup( SfxGroupId nGroupID );




typedef ::cppu::WeakImplHelper  <   css::frame::XController2
                                ,   css::frame::XControllerBorder
                                ,   css::frame::XDispatchProvider
                                ,   css::task::XStatusIndicatorSupplier
                                ,   css::ui::XContextMenuInterception
                                ,   css::awt::XUserInputInterception
                                ,   css::frame::XDispatchInformationProvider
                                ,   css::frame::XInfobarProvider
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

    css::uno::Reference< css::task::XStatusIndicator > getStatusIndicator(  ) override;


    //  XController2
    virtual css::uno::Reference< css::awt::XWindow > getComponentWindow() override;
    virtual OUString getViewControllerName() override;
    virtual cpo::uno::Sequence< css::beans::PropertyValue > getCreationArguments() override;

    virtual css::uno::Reference< css::ui::XSidebarProvider > getSidebar() override;


    //  XController


    virtual void attachFrame( const css::uno::Reference< css::frame::XFrame >& xFrame ) override ;

    virtual bool attachModel( const css::uno::Reference< css::frame::XModel >& xModel ) override ;

    virtual bool suspend( bool bSuspend ) override ;

    cpo::uno::Any getViewData() override ;

    void restoreViewData( const cpo::uno::Any& aValue ) override ;

    css::uno::Reference< css::frame::XFrame > getFrame() override ;

    css::uno::Reference< css::frame::XModel > getModel() override ;


    //  XDispatchProvider


    virtual css::uno::Reference< css::frame::XDispatch > queryDispatch(  const   css::util::URL &    aURL            ,
                                                                                  const   OUString &   sTargetFrameName,
                                                                                  sal_Int32            eSearchFlags    ) override ;

    virtual cpo::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > queryDispatches( const cpo::uno::Sequence< css::frame::DispatchDescriptor >& seqDescriptor ) override ;


    //  XControllerBorder


    virtual css::frame::BorderWidths getBorder() override;
    virtual void addBorderResizeListener( const css::uno::Reference< css::frame::XBorderResizeListener >& xListener ) override;
    virtual void removeBorderResizeListener( const css::uno::Reference< css::frame::XBorderResizeListener >& xListener ) override;
    virtual css::awt::Rectangle queryBorderedArea( const css::awt::Rectangle& aPreliminaryRectangle ) override;


    //  XComponent


    virtual void dispose() override ;

    virtual void addEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override ;

    virtual void removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override ;
    virtual void registerContextMenuInterceptor( const css::uno::Reference< css::ui::XContextMenuInterceptor >& xInterceptor ) override;
    virtual void releaseContextMenuInterceptor( const css::uno::Reference< css::ui::XContextMenuInterceptor >& xInterceptor ) override;

    virtual void addKeyHandler( const css::uno::Reference< css::awt::XKeyHandler >& xHandler ) override;
    virtual void removeKeyHandler( const css::uno::Reference< css::awt::XKeyHandler >& xHandler ) override;
    virtual void addMouseClickHandler( const css::uno::Reference< css::awt::XMouseClickHandler >& xHandler ) override;
    virtual void removeMouseClickHandler( const css::uno::Reference< css::awt::XMouseClickHandler >& xHandler ) override;


    //  XDispatchInformationProvider
    virtual cpo::uno::Sequence< sal_Int16 > getSupportedCommandGroups() override;
    virtual cpo::uno::Sequence< css::frame::DispatchInformation > getConfigurableDispatchInformation( sal_Int16 nCommandGroup ) override;

    // css::frame::XTitle
    virtual OUString getTitle(  ) override;
    virtual void setTitle( const OUString& sTitle ) override;

    // css::frame::XTitleChangeBroadcaster
    virtual void addTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener ) override;
    virtual void removeTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener ) override;

    // css::lang::XInitialization
    virtual void initialize( const cpo::uno::Sequence< cpo::uno::Any >& aArguments ) override;

    // XInfobarProvider
    virtual void
    appendInfobar(const OUString& sId, const OUString& sPrimaryMessage,
                  const OUString& sSecondaryMessage, sal_Int32 aInfobarType,
                  const cpo::uno::Sequence<css::beans::StringPair>& actionButtons,
                  bool bShowCloseButton) override;
    virtual void updateInfobar(const OUString& sId, const OUString& sPrimaryMessage,
                                        const OUString& sSecondaryMessage,
                                        sal_Int32 aInfobarType) override;
    virtual void removeInfobar(const OUString& sId) override;
    virtual bool hasInfobar(const OUString& sId) override;

    // FIXME: TL needs this in sw/source/ui/uno/unotxdoc.cxx now;
    // either the _Impl name should vanish or there should be an "official" API
    SfxViewShell* GetViewShell_Impl() const;
    SAL_DLLPRIVATE bool HandleEvent_Impl( NotifyEvent const & rEvent );
    SAL_DLLPRIVATE bool HasKeyListeners_Impl() const;
    SAL_DLLPRIVATE bool HasMouseClickListeners_Impl() const;
    SAL_DLLPRIVATE void SetCreationArguments_Impl( const cpo::uno::Sequence< css::beans::PropertyValue >& i_rCreationArgs );
    SAL_DLLPRIVATE css::uno::Reference< css::frame::XTitle > impl_getTitleHelper ();

protected:
    void CopyKitViewCallbackFromFrameCreator();

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

    DECL_DLLPRIVATE_LINK( CheckOutHandler, weld::Button&, void );

    std::unique_ptr<IMPL_SfxBaseController_DataContainer>   m_pData ;

} ; // class SfxBaseController

#endif  // _SFX_SFXBASECONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
