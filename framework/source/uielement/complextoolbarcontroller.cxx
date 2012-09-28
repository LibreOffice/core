/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include "uielement/complextoolbarcontroller.hxx"

#include "uielement/toolbar.hxx"

#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/status/ItemState.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>
#include <com/sun/star/frame/XControlNotificationListener.hpp>

#include <comphelper/processfactory.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>

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
    ToolBox*                                 pToolbar,
    sal_uInt16                                   nID,
    const ::rtl::OUString&                          aCommand ) :
    svt::ToolboxController( rServiceManager, rFrame, aCommand )
    ,   m_pToolbar( pToolbar )
    ,   m_nID( nID )
    ,   m_bMadeInvisible( sal_False )
{
    m_xURLTransformer.set( URLTransformer::create(::comphelper::getComponentContext(m_xServiceManager)) );
}

// ------------------------------------------------------------------

ComplexToolbarController::~ComplexToolbarController()
{
}

// ------------------------------------------------------------------

void SAL_CALL ComplexToolbarController::dispose()
throw ( RuntimeException )
{
    SolarMutexGuard aSolarMutexGuard;

    m_pToolbar->SetItemWindow( m_nID, 0 );
    svt::ToolboxController::dispose();

    m_xURLTransformer.clear();
    m_pToolbar = 0;
    m_nID = 0;
}

// ------------------------------------------------------------------
Sequence<PropertyValue> ComplexToolbarController::getExecuteArgs(sal_Int16 KeyModifier) const
{
    Sequence<PropertyValue> aArgs( 1 );

    // Add key modifier to argument list
    aArgs[0].Name = rtl::OUString( "KeyModifier" );
    aArgs[0].Value <<= KeyModifier;
    return aArgs;
}
// -----------------------------------------------------------------------------
void SAL_CALL ComplexToolbarController::execute( sal_Int16 KeyModifier )
throw ( RuntimeException )
{
    Reference< XDispatch >       xDispatch;
    Reference< XURLTransformer > xURLTransformer;
    ::rtl::OUString                     aCommandURL;
    ::com::sun::star::util::URL  aTargetURL;
    Sequence<PropertyValue> aArgs;

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( m_bDisposed )
            throw DisposedException();

        if ( m_bInitialized &&
             m_xFrame.is() &&
             m_xServiceManager.is() &&
             !m_aCommandURL.isEmpty() )
        {
            xURLTransformer = m_xURLTransformer;
            xDispatch = getDispatchFromCommand( m_aCommandURL );
            aCommandURL = m_aCommandURL;
            aTargetURL = getInitializedURL();
            aArgs = getExecuteArgs(KeyModifier);
        }
    }

    if ( xDispatch.is() && !aTargetURL.Complete.isEmpty() )
    {
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
    SolarMutexGuard aSolarMutexGuard;

    if ( m_bDisposed )
        return;

    if ( m_pToolbar )
    {
        m_pToolbar->EnableItem( m_nID, Event.IsEnabled );

        sal_uInt16 nItemBits = m_pToolbar->GetItemBits( m_nID );
        nItemBits &= ~TIB_CHECKABLE;
        TriState eTri = STATE_NOCHECK;

        sal_Bool        bValue = sal_Bool();
        rtl::OUString   aStrValue;
        ItemStatus      aItemState;
        Visibility      aItemVisibility;
        ControlCommand  aControlCommand;

        if ( Event.State >>= bValue )
        {
            // Boolean, treat it as checked/unchecked
            if ( m_bMadeInvisible )
                m_pToolbar->ShowItem( m_nID, sal_True );
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
                m_pToolbar->ShowItem( m_nID, sal_True );
        }
        else if ( Event.State >>= aItemState )
        {
            eTri = STATE_DONTKNOW;
            nItemBits |= TIB_CHECKABLE;
            if ( m_bMadeInvisible )
                m_pToolbar->ShowItem( m_nID, sal_True );
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
                m_pToolbar->ShowItem( m_nID, sal_True );
        }

        else if ( m_bMadeInvisible )
            m_pToolbar->ShowItem( m_nID, sal_True );

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
   catch ( const Exception& )
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
   catch ( const Exception& )
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

        // Add frame as source to the information sequence
        sal_Int32 nCount = rInfo.getLength();
        uno::Sequence< beans::NamedValue > aInfoSeq( rInfo );
        aInfoSeq.realloc( nCount+1 );
        aInfoSeq[nCount].Name  = ::rtl::OUString( "Source" );
        aInfoSeq[nCount].Value = uno::makeAny( getFrameInterface() );
        pNotifyInfo->aInfoSeq  = aInfoSeq;

        Application::PostUserEvent( STATIC_LINK(0, ComplexToolbarController, Notify_Impl), pNotifyInfo );
    }
}

// --------------------------------------------------------
sal_Int32 ComplexToolbarController::getFontSizePixel( const Window* pWindow )
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    const Font&          rFont     = rSettings.GetAppFont();

    // Calculate height of the application font used by window
    sal_Int32 nHeight = sal_Int32( rFont.GetHeight() );
    ::Size aPixelSize = pWindow->LogicToPixel( ::Size( 0, nHeight ), MAP_APPFONT );
    return aPixelSize.Height();
}

// --------------------------------------------------------

uno::Reference< frame::XDispatch > ComplexToolbarController::getDispatchFromCommand( const rtl::OUString& aCommand ) const
{
    uno::Reference< frame::XDispatch > xDispatch;

    if ( m_bInitialized && m_xFrame.is() && m_xServiceManager.is() && !aCommand.isEmpty() )
    {
        URLToDispatchMap::const_iterator pIter = m_aListenerMap.find( aCommand );
        if ( pIter != m_aListenerMap.end() )
            xDispatch = pIter->second;
    }

    return xDispatch;
}

// --------------------------------------------------------

const ::com::sun::star::util::URL& ComplexToolbarController::getInitializedURL()
{
    if ( m_aURL.Complete.isEmpty() )
    {
        m_aURL.Complete = m_aCommandURL;
        m_xURLTransformer->parseStrict( m_aURL );
    }
    return m_aURL;
}

void ComplexToolbarController::notifyFocusGet()
{
    // send focus get notification
    uno::Sequence< beans::NamedValue > aInfo;
    addNotifyInfo( rtl::OUString( "FocusSet" ),
                    getDispatchFromCommand( m_aCommandURL ),
                    aInfo );
}

void ComplexToolbarController::notifyFocusLost()
{
    // send focus lost notification
    uno::Sequence< beans::NamedValue > aInfo;
    addNotifyInfo( rtl::OUString( "FocusLost" ),
                    getDispatchFromCommand( m_aCommandURL ),
                    aInfo );
}

void ComplexToolbarController::notifyTextChanged( const ::rtl::OUString& aText )
{
    // send text changed notification
    uno::Sequence< beans::NamedValue > aInfo( 1 );
    aInfo[0].Name  = rtl::OUString( "Text" );
    aInfo[0].Value <<= aText;
    addNotifyInfo( rtl::OUString( "TextChanged" ),
                   getDispatchFromCommand( m_aCommandURL ),
                   aInfo );
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
