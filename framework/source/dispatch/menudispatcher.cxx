/*************************************************************************
 *
 *  $RCSfile: menudispatcher.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-24 13:33:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_DISPATCH_MENUDISPATCHER_HXX_
#include <dispatch/menudispatcher.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_ARGUMENTANALYZER_HXX_
#include <classes/argumentanalyzer.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef __FRAMEWORK_CLASSES_MENUCONFIGURATION_HXX_
#include <classes/menuconfiguration.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_ADDONMENU_HXX_
#include <classes/addonmenu.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XTOOLKIT_HPP_
#include <com/sun/star/awt/XToolkit.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_WINDOWATTRIBUTE_HPP_
#include <com/sun/star/awt/WindowAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_WINDOWDESCRIPTOR_HPP_
#include <com/sun/star/awt/WindowDescriptor.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOWPEER_HPP_
#include <com/sun/star/awt/XWindowPeer.hpp>
#endif

#ifndef _COM_SUN_STAR_MOZILLA_XPLUGININSTANCE_HPP_
#include <com/sun/star/mozilla/XPluginInstance.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_UNKNOWNPROPERTYEXCEPTION_HPP_
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetException.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#include <vcl/window.hxx>
#include <vcl/syswin.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/resmgr.hxx>
#include <tools/rcid.h>
#include <vos/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#include <ucbhelper/content.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

using namespace ::com::sun::star::awt           ;
using namespace ::com::sun::star::beans         ;
using namespace ::com::sun::star::container     ;
using namespace ::com::sun::star::frame         ;
using namespace ::com::sun::star::lang          ;
using namespace ::com::sun::star::mozilla       ;
using namespace ::com::sun::star::uno           ;
using namespace ::com::sun::star::util          ;
using namespace ::cppu                          ;
using namespace ::osl                           ;
using namespace ::rtl                           ;
using namespace ::vos                           ;

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

const USHORT SLOTID_MDIWINDOWLIST = 5610;

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
MenuDispatcher::MenuDispatcher(   const   Reference< XMultiServiceFactory >&  xFactory    ,
                                    const   Reference< XFrame >&                xOwner      )
        //  Init baseclasses first
        :   ThreadHelpBase          ( &Application::GetSolarMutex()  )
        ,   OWeakObject             (                                )
        // Init member
        ,   m_xOwnerWeak            ( xOwner                         )
        ,   m_xFactory              ( xFactory                       )
        ,   m_aListenerContainer    ( m_aLock.getShareableOslMutex() )
        ,   m_pMenuManager          ( NULL                           )
        ,   m_bAlreadyDisposed      ( sal_False                      )
        ,   m_bActivateListener     ( sal_False                      )
{
    // Safe impossible cases
    // We need valid informations about ouer ownerfor work.
    LOG_ASSERT( impldbg_checkParameter_MenuDispatcher( xFactory, xOwner ), "MenuDispatcher::MenuDispatcher()\nInvalid parameter detected!\n" )

    m_bActivateListener = sal_True;
    xOwner->addFrameActionListener( Reference< XFrameActionListener >( (OWeakObject *)this, UNO_QUERY ));
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
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_4     (   MenuDispatcher                     ,
                            OWeakObject                         ,
                            DIRECT_INTERFACE(   XTypeProvider   ),
                            DIRECT_INTERFACE(   XDispatch       ),
                            DIRECT_INTERFACE(   XEventListener  ),
                            DERIVED_INTERFACE(  XFrameActionListener, XEventListener )
                        )

DEFINE_XTYPEPROVIDER_4  (   MenuDispatcher     ,
                            XTypeProvider       ,
                            XDispatch           ,
                            XEventListener      ,
                            XFrameActionListener
                        )


//*****************************************************************************************************************
//  XDispatch
//*****************************************************************************************************************
void SAL_CALL MenuDispatcher::dispatch(    const   URL&                        aURL            ,
                                            const   Sequence< PropertyValue >&  seqProperties   ) throw( RuntimeException )
{
    const char RESOURCE_URL[] = "private:resource/";

    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );
    // Safe impossible cases
    // Method not defined for all incoming parameter
    LOG_ASSERT( impldbg_checkParameter_dispatch( aURL, seqProperties ), "MenuDispatcher::dispatch()\nInvalid parameter detected.\n" )

    Reference< XFrame > xFrame( m_xOwnerWeak.get(), UNO_QUERY );
    LOG_ASSERT( !(xFrame.is()==sal_False), "MenuDispatcher::dispatch()\nDispatch failed ... can't get reference to owner!\n" )
    if( xFrame.is() == sal_True )
    {
        OUString aResourceURLCommand( RTL_CONSTASCII_USTRINGPARAM( RESOURCE_URL ));

        if ( aURL.Complete.compareTo( aResourceURLCommand, aResourceURLCommand.getLength() ) == 0 )
        {
            // check for the right url syntax
            OUString aResourceString = aURL.Complete.copy( aResourceURLCommand.getLength() );

            int nResIdIndex = aResourceString.indexOf( '/' );
            int         nResId      = 0;
            MenuBar*    pMenuBar    = NULL;
            ResMgr* pResManager = NULL;

            aGuard.unlock();
            OGuard aSolarGuard( Application::GetSolarMutex() );
            {
                // execute not thread safe VCL code
                if ( nResIdIndex >= 0 )
                {
                    OUString aResourceFileName( aResourceString.copy( 0, nResIdIndex ) );
                    pResManager = new ResMgr( aResourceFileName );
                }

                if (!pResManager)
                {
                    LOG_WARNING("MenuDispatcher::dispatch()", "no res manager!")
                    return;
                }

                nResId = aResourceString.copy( nResIdIndex+1 ).toInt32();
                ResId aMenuBarResId( nResId, pResManager );
                aMenuBarResId.SetRT( RSC_MENU );

                if ( pResManager->IsAvailable(aMenuBarResId ) )
                {
                    pMenuBar = new MenuBar( aMenuBarResId );
                    pMenuBar->SetCloserHdl( LINK( this, MenuDispatcher, Close_Impl ) );
                }

                delete pResManager;
            }

            if ( pMenuBar )
            {
                // set new menu bar if there is an old one delete it before!
                if ( !impl_setMenuBar( pMenuBar, sal_True ))
                {
                    OGuard aSolarGuard( Application::GetSolarMutex() );
                    delete pMenuBar;
                }
            }
        }
        else if ( aURL.Complete.compareToAscii( ".uno:load" ) == 0 )
        {
            aGuard.unlock();

            // load menu from xml stream
            Reference< ::com::sun::star::io::XInputStream > xInputStream;
            int nPropertyCount = seqProperties.getLength();

            for( int nProperty = 0; nProperty < nPropertyCount; ++nProperty )
            {
                if( seqProperties[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("InputStream")) )
                    seqProperties[nProperty].Value >>= xInputStream;
            }

            MenuBar* pNewMenuBar = NULL;

            if ( xInputStream.is() )
            {
                MenuConfiguration aMenuConfiguration( m_xFactory );

                try
                {
                    OGuard aSolarGuard( Application::GetSolarMutex() );
                    {
                        pNewMenuBar = aMenuConfiguration.CreateMenuBarFromConfiguration( xInputStream );
                    }

                    if ( pNewMenuBar )
                        impl_setMenuBar( pNewMenuBar );
                }
                catch ( WrappedTargetException& )
                {
                }
            }

            impl_sendStatusEvent( xFrame, aURL.Complete, ( pNewMenuBar != 0 ));
        }
        else if ( aURL.Complete.compareToAscii( ".uno:save" ) == 0 )
        {
            // save menu to xml stream
            int         nPropertyCount      = seqProperties.getLength();
            sal_Bool    bStoreSuccessfull   = sal_False;
            Reference< ::com::sun::star::io::XOutputStream > xOutputStream;

            for( int nProperty = 0; nProperty < nPropertyCount; ++nProperty )
            {
                if( seqProperties[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("OutputStream")) )
                    seqProperties[nProperty].Value >>= xOutputStream;
            }

            MenuConfiguration aMenuConfiguration( m_xFactory );
            MenuBar* pMenuBar = (MenuBar *)m_pMenuManager->GetMenu();
            aGuard.unlock();

            if ( xOutputStream.is() && m_pMenuManager )
            {
                OGuard aSolarGuard( Application::GetSolarMutex() );
                {
                    try
                    {
                        aMenuConfiguration.StoreMenuBar( pMenuBar, xOutputStream );
                        bStoreSuccessfull = sal_True;
                        xOutputStream->flush();
                    }
                    catch ( WrappedTargetException& )
                    {
                    }
                }
            }

            impl_sendStatusEvent( xFrame, aURL.Complete, bStoreSuccessfull );
        }
        else if ( aURL.Complete.compareToAscii( ".uno:close" ) == 0 )
        {
            // close menu
            impl_setMenuBar( NULL );
            aGuard.unlock();
        }
    }
}

//*****************************************************************************************************************
//  XDispatch
//*****************************************************************************************************************
void SAL_CALL MenuDispatcher::addStatusListener(   const   Reference< XStatusListener >&   xControl,
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
void SAL_CALL MenuDispatcher::removeStatusListener(    const   Reference< XStatusListener >&   xControl,
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
        Reference< XFrame > xFrame( m_xOwnerWeak.get(), UNO_QUERY );
        aGuard.unlock();

        if ( xFrame.is() && pMenuBar )
        {
            Reference< ::com::sun::star::awt::XWindow >xContainerWindow = xFrame->getContainerWindow();

            OGuard aSolarGuard( Application::GetSolarMutex() );
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
void SAL_CALL MenuDispatcher::disposing( const EventObject& aEvent ) throw( RuntimeException )
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
            Reference< XFrame > xFrame( m_xOwnerWeak.get(), UNO_QUERY );
            if ( xFrame.is() )
            {
                xFrame->removeFrameActionListener( Reference< XFrameActionListener >( (OWeakObject *)this, UNO_QUERY ));
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
        m_xFactory = Reference< XMultiServiceFactory >();

        // Remove our menu from system window if it is still there!
        if ( m_pMenuManager )
            impl_setMenuBar( NULL );
    }
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
void MenuDispatcher::impl_sendStatusEvent( const   Reference< XFrame >&    xEventSource    ,
                                            const   OUString&               sURL            ,
                                            sal_Bool                        bLoadState      )
{
    // Get listener for given URL!
    OInterfaceContainerHelper* pListenerForURL = m_aListenerContainer.getContainer( sURL );
    // Send messages to all listener.
    // Do nothing, if there no listener or "getContainer()" works not correct!
    if( pListenerForURL != NULL )
    {
        // Build event for send to listener.
        FeatureStateEvent aEvent;
        aEvent.FeatureURL.Complete  =   sURL                        ;
        aEvent.FeatureDescriptor    =   FEATUREDESCRIPTOR_LOADSTATE ;
        aEvent.IsEnabled            =   bLoadState                  ;
        aEvent.Requery              =   sal_False                   ;
        aEvent.State              <<=   xEventSource                ;

        // Send message to all listener on this URL.
        OInterfaceIteratorHelper aIterator(*pListenerForURL);
        while( aIterator.hasMoreElements() )
        {
            try
            {
                ((XStatusListener*)aIterator.next())->statusChanged( aEvent );
            }
            catch( RuntimeException& )
            {
                aIterator.remove();
            }
        }
    }
}

//*****************************************************************************************************************
//  private method
//
//
//*****************************************************************************************************************
sal_Bool MenuDispatcher::impl_setMenuBar( MenuBar* pMenuBar, sal_Bool bMenuFromResource )
{
    Reference< XFrame > xFrame( m_xOwnerWeak.get(), UNO_QUERY );
    if ( xFrame.is() )
    {
        Reference< ::com::sun::star::awt::XWindow >xContainerWindow = xFrame->getContainerWindow();
        Window* pWindow = NULL;

        // Use SolarMutex for threadsafe code too!
        OGuard aSolarGuard( Application::GetSolarMutex() );
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
                OGuard aSolarGuard( Application::GetSolarMutex() );
                {
                    // remove old menu from our system window if it was set before
                     if ( m_pMenuManager->GetMenu() == (Menu *)pSysWindow->GetMenuBar() )
                        pSysWindow->SetMenuBar( NULL );

                    // remove listener before we destruct ourself, so we cannot be called back afterwards
                    m_pMenuManager->RemoveListener();

                    SAL_STATIC_CAST( ::com::sun::star::uno::XInterface*, (OWeakObject*)m_pMenuManager )->release();
                }

                m_pMenuManager = 0;
            }

            if ( pMenuBar != NULL )
            {
                OGuard aSolarGuard( Application::GetSolarMutex() );
                {
                    USHORT nPos = pMenuBar->GetItemPos( SLOTID_MDIWINDOWLIST );
                    if ( nPos != MENU_ITEM_NOTFOUND )
                    {
                        OUString aNoContext;

                        Reference< XModel >         xModel;
                        Reference< XController >    xController( xFrame->getController(), UNO_QUERY );

                        if ( xController.is() )
                            xModel = Reference< XModel >( xController->getModel(), UNO_QUERY );

                        // retrieve addon popup menus and add them to our menu bar
                        AddonMenuManager::MergeAddonPopupMenus( xFrame, xModel, nPos, pMenuBar );

                        // retrieve addon help menu items and add them to our help menu
                        AddonMenuManager::MergeAddonHelpMenu( xFrame, pMenuBar );
                    }

                    // set new menu on our system window and create new menu manager
                    if ( bMenuFromResource )
                        m_pMenuManager = new MenuManager( xFrame, pMenuBar, sal_True, sal_False );
                    else
                        m_pMenuManager = new MenuManager( xFrame, pMenuBar, sal_True, sal_True );
                    pSysWindow->SetMenuBar( pMenuBar );
                }
            }

            return sal_True;
        }
    }

    return sal_False;
}

IMPL_LINK( MenuDispatcher, Close_Impl, void*, pVoid )
{
    css::uno::Reference < css::frame::XFrame > xFrame( m_xOwnerWeak.get(), css::uno::UNO_QUERY );
    if ( !xFrame.is() )
        return 0;

    css::util::URL aURL;
    aURL.Complete = ::rtl::OUString::createFromAscii(".uno:CloseWin");
    css::uno::Reference< css::util::XURLTransformer >  xTrans ( m_xFactory->createInstance(
                        ::rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer") ), css::uno::UNO_QUERY );
    if( xTrans.is() )
    {
        // Datei laden
        xTrans->parseStrict( aURL );
        Reference< XDispatchProvider > xProv( xFrame, UNO_QUERY );
        if ( xProv.is() )
        {
            css::uno::Reference < css::frame::XDispatch > aDisp = xProv->queryDispatch( aURL, ::rtl::OUString(), 0 );
            if ( aDisp.is() )
                aDisp->dispatch( aURL, css::uno::Sequence < css::beans::PropertyValue>() );
        }
    }

    return 0;
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
sal_Bool MenuDispatcher::impldbg_checkParameter_MenuDispatcher(   const   Reference< XMultiServiceFactory >&  xFactory    ,
                                                                        const   Reference< XFrame >&                xOwner      )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xFactory     ==  NULL        )   ||
            ( &xOwner       ==  NULL        )   ||
            ( xFactory.is() ==  sal_False   )   ||
            ( xOwner.is()   ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// We don't know anything about right values of aURL and seqArguments!
// Check valid references only.
sal_Bool MenuDispatcher::impldbg_checkParameter_dispatch(  const   URL&                        aURL        ,
                                                                const   Sequence< PropertyValue >&  seqArguments)
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &aURL         ==  NULL    )   ||
            ( &seqArguments ==  NULL    )
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
sal_Bool MenuDispatcher::impldbg_checkParameter_addStatusListener( const   Reference< XStatusListener >&   xControl,
                                                                        const   URL&                            aURL    )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xControl                 ==  NULL    )   ||
            ( &aURL                     ==  NULL    )   ||
            ( aURL.Complete.getLength() <   1       )
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
sal_Bool MenuDispatcher::impldbg_checkParameter_removeStatusListener(  const   Reference< XStatusListener >&   xControl,
                                                                            const   URL&                            aURL    )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xControl                 ==  NULL    )   ||
            ( &aURL                     ==  NULL    )   ||
            ( aURL.Complete.getLength() <   1       )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

#endif  //  #ifdef ENABLE_ASSERTIONS

}       //  namespace framework
