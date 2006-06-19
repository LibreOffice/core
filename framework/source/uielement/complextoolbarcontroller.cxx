/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: complextoolbarcontroller.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 11:36:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef __FRAMEWORK_UIELEMENT_COMPLEXTOOLBARCONTROLLER_HXX
#include "uielement/complextoolbarcontroller.hxx"
#endif

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_TOOLBAR_HXX_
#include "uielement/toolbar.hxx"
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_STATUS_ITEMSTATUS_HPP_
#include <com/sun/star/frame/status/ItemStatus.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_STATUS_ITEMSTATE_HPP_
#include <com/sun/star/frame/status/ItemState.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_STATUS_VISIBILITY_HPP_
#include <com/sun/star/frame/status/Visibility.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLNOTIFICATIONLISTENER_HPP_
#include <com/sun/star/frame/XControlNotificationListener.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _SVTOOLS_TOOLBOXCONTROLLER_HXX
#include <svtools/toolboxcontroller.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VCL_MNEMONIC_HXX_
#include <vcl/mnemonic.hxx>
#endif
#include <tools/urlobj.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;
using namespace ::com::sun::star::util;

namespace framework
{

// ------------------------------------------------------------------

ComplexToolbarController::ComplexToolbarController(
    const Reference< XMultiServiceFactory >& rServiceManager,
    const Reference< XFrame >&               rFrame,
    ToolBar*                                 pToolbar,
    USHORT                                   nID,
    const OUString&                          aCommand ) :
    svt::ToolboxController( rServiceManager, rFrame, aCommand )
    ,   m_pToolbar( pToolbar )
    ,   m_nID( nID )
    ,   m_bMadeInvisible( sal_False )
{
    m_xURLTransformer = Reference< XURLTransformer >( m_xServiceManager->createInstance(
                                                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                      UNO_QUERY_THROW );
}

// ------------------------------------------------------------------

ComplexToolbarController::~ComplexToolbarController()
{
}

// ------------------------------------------------------------------

void SAL_CALL ComplexToolbarController::dispose()
throw ( RuntimeException )
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    m_pToolbar->SetItemWindow( m_nID, 0 );
    svt::ToolboxController::dispose();

    m_pToolbar = 0;
    m_nID = 0;
}

// ------------------------------------------------------------------

void SAL_CALL ComplexToolbarController::execute( sal_Int16 KeyModifier )
throw ( RuntimeException )
{
    Reference< XDispatch >       xDispatch;
    Reference< XURLTransformer > xURLTransformer;
    OUString                     aCommandURL;
    ::com::sun::star::util::URL  aTargetURL;

    {
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        if ( m_bDisposed )
            throw DisposedException();

        if ( m_bInitialized &&
             m_xFrame.is() &&
             m_xServiceManager.is() &&
             m_aCommandURL.getLength() )
        {
            xURLTransformer = m_xURLTransformer;
            xDispatch = getDispatchFromCommand( m_aCommandURL );
            aCommandURL = m_aCommandURL;
            aTargetURL = getInitializedURL();
        }
    }

    if ( xDispatch.is() && aTargetURL.Complete.getLength() > 0 )
    {
        Sequence<PropertyValue> aArgs( 1 );

        // Add key modifier to argument list
        aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "KeyModifier" ));
        aArgs[0].Value <<= KeyModifier;

        // Execute dispatch asynchronously
        ExecuteInfo* pExecuteInfo = new ExecuteInfo;
        pExecuteInfo->xDispatch     = xDispatch;
        pExecuteInfo->aTargetURL    = aTargetURL;
        pExecuteInfo->aArgs         = aArgs;
        Application::PostUserEvent( STATIC_LINK(0, ComplexToolbarController , ExecuteHdl_Impl), pExecuteInfo );
    }
}

// ------------------------------------------------------------------

void ComplexToolbarController::statusChanged( const FeatureStateEvent& Event )
throw ( RuntimeException )
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    if ( m_bDisposed )
        return;

    if ( m_pToolbar )
    {
        m_pToolbar->EnableItem( m_nID, Event.IsEnabled );

        USHORT nItemBits = m_pToolbar->GetItemBits( m_nID );
        nItemBits &= ~TIB_CHECKABLE;
        TriState eTri = STATE_NOCHECK;

        sal_Bool        bValue;
        rtl::OUString   aStrValue;
        ItemStatus      aItemState;
        Visibility      aItemVisibility;
        ControlCommand  aControlCommand;

        if ( Event.State >>= bValue )
        {
            // Boolean, treat it as checked/unchecked
            if ( m_bMadeInvisible )
                m_pToolbar->ShowItem( m_nID, TRUE );
            m_pToolbar->CheckItem( m_nID, bValue );
            if ( bValue )
                eTri = STATE_CHECK;
            nItemBits |= TIB_CHECKABLE;
        }
        else if ( Event.State >>= aStrValue )
        {
            ::rtl::OUString aText( MnemonicGenerator::EraseAllMnemonicChars( aStrValue ) );
            m_pToolbar->SetItemText( m_nID, aText );
            m_pToolbar->SetQuickHelpText( m_nID, aText );

            if ( m_bMadeInvisible )
                m_pToolbar->ShowItem( m_nID, TRUE );
        }
        else if ( Event.State >>= aItemState )
        {
            eTri = STATE_DONTKNOW;
            nItemBits |= TIB_CHECKABLE;
            if ( m_bMadeInvisible )
                m_pToolbar->ShowItem( m_nID, TRUE );
        }
        else if ( Event.State >>= aItemVisibility )
        {
            m_pToolbar->ShowItem( m_nID, aItemVisibility.bVisible );
            m_bMadeInvisible = !aItemVisibility.bVisible;
        }
        else if ( Event.State >>= aControlCommand )
        {
            executeControlCommand( aControlCommand );
            if ( m_bMadeInvisible )
                m_pToolbar->ShowItem( m_nID, TRUE );
        }

        else if ( m_bMadeInvisible )
            m_pToolbar->ShowItem( m_nID, TRUE );

        m_pToolbar->SetItemState( m_nID, eTri );
        m_pToolbar->SetItemBits( m_nID, nItemBits );
    }
}

// ------------------------------------------------------------------

IMPL_STATIC_LINK_NOINSTANCE( ComplexToolbarController, ExecuteHdl_Impl, ExecuteInfo*, pExecuteInfo )
{
   const sal_uInt32 nRef = Application::ReleaseSolarMutex();
   try
   {
       // Asynchronous execution as this can lead to our own destruction!
       // Framework can recycle our current frame and the layout manager disposes all user interface
       // elements if a component gets detached from its frame!
       pExecuteInfo->xDispatch->dispatch( pExecuteInfo->aTargetURL, pExecuteInfo->aArgs );
   }
   catch ( Exception& )
   {
   }

   Application::AcquireSolarMutex( nRef );
   delete pExecuteInfo;
   return 0;
}

// ------------------------------------------------------------------

IMPL_STATIC_LINK_NOINSTANCE( ComplexToolbarController, Notify_Impl, NotifyInfo*, pNotifyInfo )
{
   const sal_uInt32 nRef = Application::ReleaseSolarMutex();
   try
   {
       // Asynchronous execution: As this can lead to our own destruction!
       // Framework can recycle our current frame and the layout manager disposes all user interface
       // elements if a component gets detached from its frame!
       frame::ControlEvent aEvent;
       aEvent.aURL  = pNotifyInfo->aSourceURL;
       aEvent.Event = pNotifyInfo->aEventName;
       aEvent.aInformation = pNotifyInfo->aInfoSeq;
       pNotifyInfo->xNotifyListener->controlEvent( aEvent );
   }
   catch ( Exception& )
   {
   }

   Application::AcquireSolarMutex( nRef );
   delete pNotifyInfo;
   return 0;
}

// ------------------------------------------------------------------

void ComplexToolbarController::addNotifyInfo(
    const rtl::OUString&                      aEventName,
    const uno::Reference< frame::XDispatch >& xDispatch,
    const uno::Sequence< beans::NamedValue >& rInfo )
{
    uno::Reference< frame::XControlNotificationListener > xControlNotify( xDispatch, uno::UNO_QUERY );

    if ( xControlNotify.is() )
    {
        // Execute notification asynchronously
        NotifyInfo* pNotifyInfo = new NotifyInfo;

        pNotifyInfo->aEventName      = aEventName;
        pNotifyInfo->xNotifyListener = xControlNotify;
        pNotifyInfo->aSourceURL      = getInitializedURL();
        pNotifyInfo->aInfoSeq        = rInfo;

        Application::PostUserEvent( STATIC_LINK(0, ComplexToolbarController, Notify_Impl), pNotifyInfo );
    }
}

// --------------------------------------------------------

uno::Reference< frame::XDispatch > ComplexToolbarController::getDispatchFromCommand( const rtl::OUString& aCommand ) const
{
    uno::Reference< frame::XDispatch > xDispatch;

    if ( m_bInitialized && m_xFrame.is() && m_xServiceManager.is() && aCommand.getLength() )
    {
        URLToDispatchMap::const_iterator pIter = m_aListenerMap.find( aCommand );
        if ( pIter != m_aListenerMap.end() )
            xDispatch = pIter->second;
    }

    return xDispatch;
}

// --------------------------------------------------------

void ComplexToolbarController::executeControlCommand( const ::com::sun::star::frame::ControlCommand& )
{
    // must be implemented by sub class!
}

const ::com::sun::star::util::URL& ComplexToolbarController::getInitializedURL()
{
    if ( m_aURL.Complete.getLength() == 0 )
    {
        m_aURL.Complete = m_aCommandURL;
        m_xURLTransformer->parseStrict( m_aURL );
    }
    return m_aURL;
}

} // namespace
