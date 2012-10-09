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

#ifndef _SFX_SFXBASECONTROLLER_HXX_
#define _SFX_SFXBASECONTROLLER_HXX_

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
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
#include <cppuhelper/implbase9.hxx>
#include <cppuhelper/basemutex.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#include <com/sun/star/ui/XContextMenuInterception.hpp>
#include <com/sun/star/ui/XContextMenuInterceptor.hpp>
#include <com/sun/star/awt/XMouseClickHandler.hpp>
#include <com/sun/star/awt/XKeyHandler.hpp>
#include <com/sun/star/awt/XUserInputInterception.hpp>

#include <sfx2/viewsh.hxx>
#include <sfx2/sfxuno.hxx>

// Some defines to write better code :-)
#define REFERENCE               ::com::sun::star::uno::Reference
#define ANY                     ::com::sun::star::uno::Any
#define XDISPATCH               ::com::sun::star::frame::XDispatch
#define DISPATCHDESCRIPTOR      ::com::sun::star::frame::DispatchDescriptor
#define XMODEL                  ::com::sun::star::frame::XModel
#define XFRAME                  ::com::sun::star::frame::XFrame
#define XEVENTLISTENER          ::com::sun::star::lang::XEventListener
#define MUTEX                   ::osl::Mutex
#define RUNTIMEEXCEPTION        ::com::sun::star::uno::RuntimeException
#define UNOTYPE                 ::com::sun::star::uno::Type
#define UNOURL                  ::com::sun::star::util::URL
#define XCONTEXTMENUINTERCEPTOR ::com::sun::star::ui::XContextMenuInterceptor

//________________________________________________________________________________________________________
//  forwards
//________________________________________________________________________________________________________

struct  IMPL_SfxBaseController_DataContainer    ;   // impl. struct to hold member of class SfxBaseController

class SfxViewFrame;

sal_Int16 MapGroupIDToCommandGroup( sal_Int16 nGroupID );

//________________________________________________________________________________________________________
//  class declarations
//________________________________________________________________________________________________________

typedef ::cppu::WeakImplHelper9 <   ::com::sun::star::frame::XController2
                                ,   ::com::sun::star::frame::XControllerBorder
                                ,   ::com::sun::star::frame::XDispatchProvider
                                ,   ::com::sun::star::task::XStatusIndicatorSupplier
                                ,   ::com::sun::star::ui::XContextMenuInterception
                                ,   ::com::sun::star::awt::XUserInputInterception
                                ,   ::com::sun::star::frame::XDispatchInformationProvider
                                ,   ::com::sun::star::frame::XTitle
                                ,   ::com::sun::star::frame::XTitleChangeBroadcaster
                                >   SfxBaseController_Base;

class SFX2_DLLPUBLIC SfxBaseController  :public SfxBaseController_Base
                                        ,public ::cppu::BaseMutex
{
//________________________________________________________________________________________________________
//  public methods
//________________________________________________________________________________________________________

public:

    //____________________________________________________________________________________________________
    //  constructor/destructor
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    SfxBaseController( SfxViewShell* pView ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    ~SfxBaseController() ;

    SAL_DLLPRIVATE void ReleaseShell_Impl();
    SAL_DLLPRIVATE void BorderWidthsChanged_Impl();

    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > SAL_CALL getStatusIndicator(  ) throw (::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XController2
    //____________________________________________________________________________________________________
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL getComponentWindow() throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getViewControllerName() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getCreationArguments() throw (::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XController
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL attachFrame( const REFERENCE< XFRAME >& xFrame ) throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL attachModel( const REFERENCE< XMODEL >& xModel ) throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual sal_Bool SAL_CALL suspend( sal_Bool bSuspend ) throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    ANY SAL_CALL getViewData() throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    void SAL_CALL restoreViewData( const ANY& aValue ) throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    REFERENCE< XFRAME > SAL_CALL getFrame() throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    REFERENCE< XMODEL > SAL_CALL getModel() throw( RUNTIMEEXCEPTION ) ;

    //____________________________________________________________________________________________________
    //  XDispatchProvider
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual REFERENCE< XDISPATCH > SAL_CALL queryDispatch(  const   UNOURL &            aURL            ,
                                                            const   ::rtl::OUString &   sTargetFrameName,
                                                                    FrameSearchFlags    eSearchFlags    ) throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual ::com::sun::star::uno::Sequence< REFERENCE< XDISPATCH > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< DISPATCHDESCRIPTOR >& seqDescriptor ) throw( RUNTIMEEXCEPTION ) ;

    //____________________________________________________________________________________________________
    //  XControllerBorder
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::frame::BorderWidths SAL_CALL getBorder() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addBorderResizeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XBorderResizeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeBorderResizeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XBorderResizeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL queryBorderedArea( const ::com::sun::star::awt::Rectangle& aPreliminaryRectangle ) throw (::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XComponent
    //____________________________________________________________________________________________________

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL dispose() throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL addEventListener( const REFERENCE< XEVENTLISTENER >& aListener ) throw( RUNTIMEEXCEPTION ) ;

    /**___________________________________________________________________________________________________
        @short      -
        @descr      -

        @seealso    -

        @param      -

        @return     -

        @onerror    -
    */

    virtual void SAL_CALL removeEventListener( const REFERENCE< XEVENTLISTENER >& aListener ) throw( RUNTIMEEXCEPTION ) ;
    virtual void SAL_CALL registerContextMenuInterceptor( const REFERENCE< XCONTEXTMENUINTERCEPTOR >& xInterceptor ) throw( RUNTIMEEXCEPTION );
    virtual void SAL_CALL releaseContextMenuInterceptor( const REFERENCE< XCONTEXTMENUINTERCEPTOR >& xInterceptor ) throw( RUNTIMEEXCEPTION );

    virtual void SAL_CALL addKeyHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeKeyHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addMouseClickHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseClickHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeMouseClickHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseClickHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XDispatchInformationProvider
    //____________________________________________________________________________________________________
    virtual ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL getSupportedCommandGroups() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchInformation > SAL_CALL getConfigurableDispatchInformation( sal_Int16 nCommandGroup ) throw (::com::sun::star::uno::RuntimeException);

    // css::frame::XTitle
    virtual ::rtl::OUString SAL_CALL getTitle(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTitle( const ::rtl::OUString& sTitle ) throw (::com::sun::star::uno::RuntimeException);

    // css::frame::XTitleChangeBroadcaster
    virtual void SAL_CALL addTitleChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTitleChangeListener >& xListener )     throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTitleChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTitleChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    // FIXME: TL needs this in sw/source/ui/uno/unotxdoc.cxx now;
    // either the _Impl name should vanish or there should be an "official" API
    SfxViewShell* GetViewShell_Impl() const;
    SAL_DLLPRIVATE sal_Bool HandleEvent_Impl( NotifyEvent& rEvent );
    SAL_DLLPRIVATE sal_Bool HasKeyListeners_Impl();
    SAL_DLLPRIVATE sal_Bool HasMouseClickListeners_Impl();
    SAL_DLLPRIVATE void SetCreationArguments_Impl( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& i_rCreationArgs );
    SAL_DLLPRIVATE ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTitle > impl_getTitleHelper ();
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

//________________________________________________________________________________________________________
//  private variables
//________________________________________________________________________________________________________

private:

    IMPL_SfxBaseController_DataContainer*   m_pData ;

} ; // class SfxBaseController

#endif  // _SFX_SFXBASECONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
