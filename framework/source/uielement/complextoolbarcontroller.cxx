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

#include <uielement/complextoolbarcontroller.hxx>

#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>
#include <com/sun/star/frame/XControlNotificationListener.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <svtools/toolboxcontroller.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/settings.hxx>

using namespace ::com::sun::star;
using namespace css::awt;
using namespace css::uno;
using namespace css::beans;
using namespace css::lang;
using namespace css::frame;
using namespace css::frame::status;
using namespace css::util;

namespace framework
{

ComplexToolbarController::ComplexToolbarController(
    const Reference< XComponentContext >& rxContext,
    const Reference< XFrame >&            rFrame,
    ToolBox*                              pToolbar,
    sal_uInt16                            nID,
    const OUString&                       aCommand ) :
    svt::ToolboxController( rxContext, rFrame, aCommand )
    ,   m_pToolbar( pToolbar )
    ,   m_nID( nID )
    ,   m_bMadeInvisible( false )
{
    m_xURLTransformer.set( URLTransformer::create(m_xContext) );
}

ComplexToolbarController::~ComplexToolbarController()
{
}

void SAL_CALL ComplexToolbarController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    m_pToolbar->SetItemWindow( m_nID, nullptr );
    svt::ToolboxController::dispose();

    m_xURLTransformer.clear();
    m_pToolbar.clear();
    m_nID = 0;
}

Sequence<PropertyValue> ComplexToolbarController::getExecuteArgs(sal_Int16 KeyModifier) const
{
    Sequence<PropertyValue> aArgs( 1 );

    // Add key modifier to argument list
    aArgs[0].Name = "KeyModifier";
    aArgs[0].Value <<= KeyModifier;
    return aArgs;
}

void SAL_CALL ComplexToolbarController::execute( sal_Int16 KeyModifier )
{
    Reference< XDispatch >       xDispatch;
    Reference< XURLTransformer > xURLTransformer;
    css::util::URL  aTargetURL;
    Sequence<PropertyValue> aArgs;

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( m_bDisposed )
            throw DisposedException();

        if ( m_bInitialized &&
             m_xFrame.is() &&
             !m_aCommandURL.isEmpty() )
        {
            xURLTransformer = m_xURLTransformer;
            xDispatch = getDispatchFromCommand( m_aCommandURL );
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
        Application::PostUserEvent( LINK(nullptr, ComplexToolbarController , ExecuteHdl_Impl), pExecuteInfo );
    }
}

void ComplexToolbarController::statusChanged( const FeatureStateEvent& Event )
{
    SolarMutexGuard aSolarMutexGuard;

    if ( m_bDisposed )
        return;

    if ( m_pToolbar )
    {
        m_pToolbar->EnableItem( m_nID, Event.IsEnabled );

        ToolBoxItemBits nItemBits = m_pToolbar->GetItemBits( m_nID );
        nItemBits &= ~ToolBoxItemBits::CHECKABLE;
        TriState eTri = TRISTATE_FALSE;

        bool            bValue;
        OUString        aStrValue;
        ItemStatus      aItemState;
        Visibility      aItemVisibility;
        ControlCommand  aControlCommand;

        if ( Event.State >>= bValue )
        {
            // Boolean, treat it as checked/unchecked
            if ( m_bMadeInvisible )
                m_pToolbar->ShowItem( m_nID );
            m_pToolbar->CheckItem( m_nID, bValue );
            if ( bValue )
                eTri = TRISTATE_TRUE;
            nItemBits |= ToolBoxItemBits::CHECKABLE;
        }
        else if ( Event.State >>= aStrValue )
        {
            OUString aText( MnemonicGenerator::EraseAllMnemonicChars( aStrValue ) );
            m_pToolbar->SetItemText( m_nID, aText );
            m_pToolbar->SetQuickHelpText( m_nID, aText );

            if ( m_bMadeInvisible )
                m_pToolbar->ShowItem( m_nID );
        }
        else if ( Event.State >>= aItemState )
        {
            eTri = TRISTATE_INDET;
            nItemBits |= ToolBoxItemBits::CHECKABLE;
            if ( m_bMadeInvisible )
                m_pToolbar->ShowItem( m_nID );
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
                m_pToolbar->ShowItem( m_nID );
        }

        else if ( m_bMadeInvisible )
            m_pToolbar->ShowItem( m_nID );

        m_pToolbar->SetItemState( m_nID, eTri );
        m_pToolbar->SetItemBits( m_nID, nItemBits );
    }
}

IMPL_STATIC_LINK( ComplexToolbarController, ExecuteHdl_Impl, void*, p, void )
{
   ExecuteInfo* pExecuteInfo = static_cast<ExecuteInfo*>(p);
   SolarMutexReleaser aReleaser;
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

   delete pExecuteInfo;
}

IMPL_STATIC_LINK( ComplexToolbarController, Notify_Impl, void*, p, void )
{
   NotifyInfo* pNotifyInfo = static_cast<NotifyInfo*>(p);
   SolarMutexReleaser aReleaser;
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

   delete pNotifyInfo;
}

void ComplexToolbarController::addNotifyInfo(
    const OUString&                      aEventName,
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
        aInfoSeq[nCount].Name  = "Source";
        aInfoSeq[nCount].Value <<= getFrameInterface();
        pNotifyInfo->aInfoSeq  = aInfoSeq;

        Application::PostUserEvent( LINK(nullptr, ComplexToolbarController, Notify_Impl), pNotifyInfo );
    }
}

sal_Int32 ComplexToolbarController::getFontSizePixel( const vcl::Window* pWindow )
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    const vcl::Font&     rFont     = rSettings.GetAppFont();

    // Calculate height of the application font used by window
    sal_Int32 nHeight = sal_Int32( rFont.GetFontHeight() );
    ::Size aPixelSize = pWindow->LogicToPixel(::Size(0, nHeight), MapMode(MapUnit::MapAppFont));
    return aPixelSize.Height();
}

uno::Reference< frame::XDispatch > ComplexToolbarController::getDispatchFromCommand( const OUString& aCommand ) const
{
    uno::Reference< frame::XDispatch > xDispatch;

    if ( m_bInitialized && m_xFrame.is() && !aCommand.isEmpty() )
    {
        URLToDispatchMap::const_iterator pIter = m_aListenerMap.find( aCommand );
        if ( pIter != m_aListenerMap.end() )
            xDispatch = pIter->second;
    }

    return xDispatch;
}

const css::util::URL& ComplexToolbarController::getInitializedURL()
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
    addNotifyInfo( "FocusSet",
                    getDispatchFromCommand( m_aCommandURL ),
                    aInfo );
}

void ComplexToolbarController::notifyFocusLost()
{
    // send focus lost notification
    uno::Sequence< beans::NamedValue > aInfo;
    addNotifyInfo( "FocusLost",
                    getDispatchFromCommand( m_aCommandURL ),
                    aInfo );
}

void ComplexToolbarController::notifyTextChanged( const OUString& aText )
{
    // send text changed notification
    uno::Sequence< beans::NamedValue > aInfo { { "Text", css::uno::makeAny(aText) } };
    addNotifyInfo( "TextChanged",
                   getDispatchFromCommand( m_aCommandURL ),
                   aInfo );
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
