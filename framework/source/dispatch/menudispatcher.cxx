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

#include <dispatch/menudispatcher.hxx>
#include <general.h>
#include <framework/menuconfiguration.hxx>
#include <framework/addonmenu.hxx>
#include <services.h>

#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <vcl/window.hxx>
#include <vcl/syswin.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <tools/rcid.h>
#include <osl/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <ucbhelper/content.hxx>

namespace framework{

using namespace ::com::sun::star                ;
using namespace ::com::sun::star::awt           ;
using namespace ::com::sun::star::beans         ;
using namespace ::com::sun::star::container     ;
using namespace ::com::sun::star::frame         ;
using namespace ::com::sun::star::lang          ;
using namespace ::com::sun::star::uno           ;
using namespace ::com::sun::star::util          ;
using namespace ::cppu                          ;

const sal_uInt16 SLOTID_MDIWINDOWLIST = 5610;

//-------------------------------------------------------------------------------------------------------------
//  debug methods
//  (should be private everyway!)
//-------------------------------------------------------------------------------------------------------------

/*-****************************************************************************************************//**
    @short      debug-method to check incoming parameter of some other mehods of this class
    @descr      The following methods are used to check parameters for other methods
                of this class. The return value is used directly for an ASSERT(...).

    @seealso    ASSERTs in implementation!

    @param      css::uno::References to checking variables
    @return     sal_False on invalid parameter<BR>
                sal_True  otherway

    @onerror    -
*//*-*****************************************************************************************************/

#ifdef ENABLE_ASSERTIONS

static sal_Bool impldbg_checkParameter_MenuDispatcher      (   const   css::uno::Reference< css::uno::XComponentContext >& xContext        ,
                                                               const   css::uno::Reference< css::frame::XFrame >&              xOwner          );
static sal_Bool impldbg_checkParameter_addStatusListener    (   const   css::uno::Reference< css::frame::XStatusListener >&     xControl        ,
                                                                const   css::util::URL&                                         aURL            );
static sal_Bool impldbg_checkParameter_removeStatusListener (   const   css::uno::Reference< css::frame::XStatusListener >&     xControl        ,
                                                                const   css::util::URL&                                         aURL            );
#endif  // #ifdef ENABLE_ASSERTIONS

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
MenuDispatcher::MenuDispatcher(   const   uno::Reference< XComponentContext >&  xContext    ,
                                  const   uno::Reference< XFrame >&             xOwner      )
        //  Init baseclasses first
        :   ThreadHelpBase          ( &Application::GetSolarMutex()  )
        // Init member
        ,   m_xOwnerWeak            ( xOwner                         )
        ,   m_xContext              ( xContext                       )
        ,   m_aListenerContainer    ( m_aLock.getShareableOslMutex() )
        ,   m_bAlreadyDisposed      ( sal_False                      )
        ,   m_bActivateListener     ( sal_False                      )
        ,   m_pMenuManager          ( NULL                           )
{
    // Safe impossible cases
    // We need valid information about our owner for work.
    LOG_ASSERT( impldbg_checkParameter_MenuDispatcher( xContext, xOwner ), "MenuDispatcher::MenuDispatcher()\nInvalid parameter detected!\n" )

    m_bActivateListener = sal_True;
    xOwner->addFrameActionListener( uno::Reference< XFrameActionListener >( (OWeakObject *)this, UNO_QUERY ));
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
MenuDispatcher::~MenuDispatcher()
{
    // Warn programmer if he forgot to dispose this instance.
    // We must release all our references ...
    // and a dtor isn't the best place to do that!
}

//*****************************************************************************************************************
//  XDispatch
//*****************************************************************************************************************
void SAL_CALL MenuDispatcher::dispatch(    const   URL&                        /*aURL*/            ,
                                            const   Sequence< PropertyValue >&  /*seqProperties*/   ) throw( RuntimeException )
{
}

//*****************************************************************************************************************
//  XDispatch
//*****************************************************************************************************************
void SAL_CALL MenuDispatcher::addStatusListener(   const   uno::Reference< XStatusListener >&   xControl,
                                                    const   URL&                            aURL    ) throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );
    // Safe impossible cases
    // Method not defined for all incoming parameter
    LOG_ASSERT( impldbg_checkParameter_addStatusListener( xControl, aURL ), "MenuDispatcher::addStatusListener()\nInvalid parameter detected.\n" )
    // Add listener to container.
    m_aListenerContainer.addInterface( aURL.Complete, xControl );
}

//*****************************************************************************************************************
//  XDispatch
//*****************************************************************************************************************
void SAL_CALL MenuDispatcher::removeStatusListener(    const   uno::Reference< XStatusListener >&   xControl,
                                                        const   URL&                            aURL    ) throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );
    // Safe impossible cases
    // Method not defined for all incoming parameter
    LOG_ASSERT( impldbg_checkParameter_removeStatusListener( xControl, aURL ), "MenuDispatcher::removeStatusListener()\nInvalid parameter detected.\n" )
    // Add listener to container.
    m_aListenerContainer.removeInterface( aURL.Complete, xControl );
}

//*****************************************************************************************************************
//   XFrameActionListener
//*****************************************************************************************************************

void SAL_CALL MenuDispatcher::frameAction( const FrameActionEvent& aEvent ) throw ( RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_pMenuManager && aEvent.Action == FrameAction_FRAME_UI_ACTIVATED )
    {
        MenuBar* pMenuBar = (MenuBar *)m_pMenuManager->GetMenu();
        uno::Reference< XFrame > xFrame( m_xOwnerWeak.get(), UNO_QUERY );
        aGuard.unlock();

        if ( xFrame.is() && pMenuBar )
        {
            uno::Reference< ::com::sun::star::awt::XWindow >xContainerWindow = xFrame->getContainerWindow();

            SolarMutexGuard aSolarGuard;
            {
                Window* pWindow = VCLUnoHelper::GetWindow( xContainerWindow );
                while ( pWindow && !pWindow->IsSystemWindow() )
                    pWindow = pWindow->GetParent();

                if ( pWindow )
                {
                    SystemWindow* pSysWindow = (SystemWindow *)pWindow;
                    pSysWindow->SetMenuBar( pMenuBar );
                }
            }
        }
    }
    else if ( m_pMenuManager && aEvent.Action == css::frame::FrameAction_COMPONENT_DETACHING )
    {
        if ( m_pMenuManager )
            impl_setMenuBar( NULL );
    }
}

//*****************************************************************************************************************
//   XEventListener
//*****************************************************************************************************************
void SAL_CALL MenuDispatcher::disposing( const EventObject& ) throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );
    // Safe impossible cases
    LOG_ASSERT( !(m_bAlreadyDisposed==sal_True), "MenuDispatcher::disposing()\nObject already disposed .. don't call it again!\n" )

    if( m_bAlreadyDisposed == sal_False )
    {
        m_bAlreadyDisposed = sal_True;

        if ( m_bActivateListener )
        {
            uno::Reference< XFrame > xFrame( m_xOwnerWeak.get(), UNO_QUERY );
            if ( xFrame.is() )
            {
                xFrame->removeFrameActionListener( uno::Reference< XFrameActionListener >( (OWeakObject *)this, UNO_QUERY ));
                m_bActivateListener = sal_False;
                if ( m_pMenuManager )
                {
                    EventObject aEventObj;
                    aEventObj.Source = xFrame;
                    m_pMenuManager->disposing( aEventObj );
                }
            }
        }

        // Forget our factory.
        m_xContext = uno::Reference< XComponentContext >();

        // Remove our menu from system window if it is still there!
        if ( m_pMenuManager )
            impl_setMenuBar( NULL );
    }
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
void MenuDispatcher::impl_setAccelerators( Menu* pMenu, const Accelerator& aAccel )
{
    for ( sal_uInt16 nPos = 0; nPos < pMenu->GetItemCount(); ++nPos )
    {
        sal_uInt16     nId    = pMenu->GetItemId(nPos);
        ::PopupMenu* pPopup = pMenu->GetPopupMenu(nId);
        if ( pPopup )
            impl_setAccelerators( (Menu *)pPopup, aAccel );
        else if ( nId && !pMenu->GetPopupMenu(nId))
        {
            KeyCode aCode = aAccel.GetKeyCode( nId );
            if ( aCode.GetCode() )
                pMenu->SetAccelKey( nId, aCode );
        }
    }
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
sal_Bool MenuDispatcher::impl_setMenuBar( MenuBar* pMenuBar, sal_Bool bMenuFromResource )
{
    uno::Reference< XFrame > xFrame( m_xOwnerWeak.get(), UNO_QUERY );
    if ( xFrame.is() )
    {
        uno::Reference< ::com::sun::star::awt::XWindow >xContainerWindow = xFrame->getContainerWindow();
        Window* pWindow = NULL;

        // Use SolarMutex for threadsafe code too!
        SolarMutexGuard aSolarGuard;
        {
            pWindow = VCLUnoHelper::GetWindow( xContainerWindow );
            while ( pWindow && !pWindow->IsSystemWindow() )
                pWindow = pWindow->GetParent();
        }

        if ( pWindow )
        {
            // Ready for multithreading
            ResetableGuard aGuard( m_aLock );

            SystemWindow* pSysWindow = (SystemWindow *)pWindow;

            if ( m_pMenuManager )
            {
                // remove old menu from our system window if it was set before
                if ( m_pMenuManager->GetMenu() == (Menu *)pSysWindow->GetMenuBar() )
                    pSysWindow->SetMenuBar( NULL );

                // remove listener before we destruct ourself, so we cannot be called back afterwards
                m_pMenuManager->RemoveListener();

                (static_cast< ::com::sun::star::uno::XInterface* >((OWeakObject*)m_pMenuManager))->release();

                m_pMenuManager = 0;
            }

            if ( pMenuBar != NULL )
            {
                sal_uInt16 nPos = pMenuBar->GetItemPos( SLOTID_MDIWINDOWLIST );
                if ( nPos != MENU_ITEM_NOTFOUND )
                {
                    uno::Reference< XModel >            xModel;
                    uno::Reference< XController >   xController( xFrame->getController(), UNO_QUERY );

                    if ( xController.is() )
                        xModel = uno::Reference< XModel >( xController->getModel(), UNO_QUERY );

                    // retrieve addon popup menus and add them to our menu bar
                    AddonMenuManager::MergeAddonPopupMenus( xFrame, xModel, nPos, pMenuBar );

                    // retrieve addon help menu items and add them to our help menu
                    AddonMenuManager::MergeAddonHelpMenu( xFrame, pMenuBar );
                }

                // set new menu on our system window and create new menu manager
                if ( bMenuFromResource )
                {
                    m_pMenuManager = new MenuManager( m_xContext, xFrame, pMenuBar, sal_True, sal_False );
                }
                else
                {
                    m_pMenuManager = new MenuManager( m_xContext, xFrame, pMenuBar, sal_True, sal_True );
                }

                pSysWindow->SetMenuBar( pMenuBar );
            }

            return sal_True;
        }
    }

    return sal_False;
}

//_________________________________________________________________________________________________________________
//  debug methods
//_________________________________________________________________________________________________________________

/*-----------------------------------------------------------------------------------------------------------------
    The follow methods checks the parameter for other functions. If a parameter or his value is non valid,
    we return "sal_False". (else sal_True) This mechanism is used to throw an ASSERT!

    ATTENTION

        If you miss a test for one of this parameters, contact the autor or add it himself !(?)
        But ... look for right testing! See using of this methods!
-----------------------------------------------------------------------------------------------------------------*/

#ifdef ENABLE_ASSERTIONS

//*****************************************************************************************************************
static sal_Bool impldbg_checkParameter_MenuDispatcher(   const   uno::Reference< XComponentContext >&  xContext    ,
                                                                  const   uno::Reference< XFrame >&             xOwner      )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xContext     ==  NULL        )   ||
            ( &xOwner       ==  NULL        )   ||
            ( xContext.is() ==  sal_False   )   ||
            ( xOwner.is()   ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// We need a valid URL. What is meaning with "register for nothing"?!
// xControl must correct to - nobody can advised otherwise!
static sal_Bool impldbg_checkParameter_addStatusListener( const   uno::Reference< XStatusListener >&   xControl,
                                                          const   URL&                                 aURL    )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xControl                 ==  NULL    )   ||
            ( &aURL                     ==  NULL    )   ||
            ( aURL.Complete.isEmpty()               )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// The same goes for these case! We have added valid listener for correct URL only.
// We can't remove invalid listener for nothing!
static sal_Bool impldbg_checkParameter_removeStatusListener(  const   uno::Reference< XStatusListener >&   xControl,
                                                              const   URL&                                 aURL    )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xControl                 ==  NULL    )   ||
            ( &aURL                     ==  NULL    )   ||
            ( aURL.Complete.isEmpty()               )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

#endif  //  #ifdef ENABLE_ASSERTIONS

}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
