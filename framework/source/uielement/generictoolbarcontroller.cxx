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

#include <uielement/generictoolbarcontroller.hxx>

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>
#include <com/sun/star/frame/ControlCommand.hpp>

#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <tools/urlobj.hxx>
#include <strings.hrc>
#include <classes/fwkresid.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;

namespace framework
{

static bool isEnumCommand( const OUString& rCommand )
{
    INetURLObject aURL( rCommand );

    return ( aURL.GetProtocol() == INetProtocol::Uno ) &&
           ( aURL.GetURLPath().indexOf( '.' ) != -1);
}

static OUString getEnumCommand( const OUString& rCommand )
{
    INetURLObject aURL( rCommand );

    OUString   aEnumCommand;
    OUString   aURLPath = aURL.GetURLPath();
    sal_Int32  nIndex   = aURLPath.indexOf( '.' );
    if (( nIndex > 0 ) && ( nIndex < aURLPath.getLength() ))
        aEnumCommand = aURLPath.copy( nIndex+1 );

    return aEnumCommand;
}

static OUString getMasterCommand( const OUString& rCommand )
{
    OUString aMasterCommand( rCommand );
    INetURLObject aURL( rCommand );
    if ( aURL.GetProtocol() == INetProtocol::Uno )
    {
        sal_Int32 nIndex = aURL.GetURLPath().indexOf( '.' );
        if ( nIndex )
        {
            aURL.SetURLPath( aURL.GetURLPath().copy( 0, nIndex ) );
            aMasterCommand = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        }
    }
    return aMasterCommand;
}

GenericToolbarController::GenericToolbarController( const Reference< XComponentContext >&    rxContext,
                                                    const Reference< XFrame >&               rFrame,
                                                    ToolBox*                                 pToolbar,
                                                    sal_uInt16                               nID,
                                                    const OUString&                          aCommand ) :
    svt::ToolboxController( rxContext, rFrame, aCommand )
    ,   m_xToolbar( pToolbar )
    ,   m_nID( nID )
    ,   m_bEnumCommand( isEnumCommand( aCommand ))
    ,   m_bMadeInvisible( false )
    ,   m_aEnumCommand( getEnumCommand( aCommand ))
{
    if ( m_bEnumCommand )
        addStatusListener( getMasterCommand( aCommand ) );
}

GenericToolbarController::~GenericToolbarController()
{
}

void SAL_CALL GenericToolbarController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    svt::ToolboxController::dispose();

    m_xToolbar.clear();
    m_nID = 0;
}

void SAL_CALL GenericToolbarController::execute( sal_Int16 KeyModifier )
{
    Reference< XDispatch >       xDispatch;
    OUString                     aCommandURL;

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( m_bDisposed )
            throw DisposedException();

        if ( m_bInitialized &&
             m_xFrame.is() &&
             !m_aCommandURL.isEmpty() )
        {
            aCommandURL = m_aCommandURL;
            URLToDispatchMap::iterator pIter = m_aListenerMap.find( m_aCommandURL );
            if ( pIter != m_aListenerMap.end() )
                xDispatch = pIter->second;
        }
    }

    if ( !xDispatch.is() )
        return;

    css::util::URL aTargetURL;
    Sequence<PropertyValue>   aArgs( 1 );

    // Add key modifier to argument list
    aArgs[0].Name  = "KeyModifier";
    aArgs[0].Value <<= KeyModifier;

    aTargetURL.Complete = aCommandURL;
    if ( m_xUrlTransformer.is() )
        m_xUrlTransformer->parseStrict( aTargetURL );

    // Execute dispatch asynchronously
    ExecuteInfo* pExecuteInfo = new ExecuteInfo;
    pExecuteInfo->xDispatch     = xDispatch;
    pExecuteInfo->aTargetURL    = aTargetURL;
    pExecuteInfo->aArgs         = aArgs;
    Application::PostUserEvent( LINK(nullptr, GenericToolbarController , ExecuteHdl_Impl), pExecuteInfo );
}

void GenericToolbarController::statusChanged( const FeatureStateEvent& Event )
{
    SolarMutexGuard aSolarMutexGuard;

    if ( m_bDisposed )
        return;

    if ( !m_xToolbar )
        return;

    m_xToolbar->EnableItem( m_nID, Event.IsEnabled );

    ToolBoxItemBits nItemBits = m_xToolbar->GetItemBits( m_nID );
    nItemBits &= ~ToolBoxItemBits::CHECKABLE;
    TriState eTri = TRISTATE_FALSE;

    bool            bValue;
    OUString        aStrValue;
    ItemStatus      aItemState;
    Visibility      aItemVisibility;
    ControlCommand  aControlCommand;

    if (( Event.State >>= bValue ) && !m_bEnumCommand )
    {
        // Boolean, treat it as checked/unchecked
        if ( m_bMadeInvisible )
            m_xToolbar->ShowItem( m_nID );
        m_xToolbar->CheckItem( m_nID, bValue );
        if ( bValue )
            eTri = TRISTATE_TRUE;
        nItemBits |= ToolBoxItemBits::CHECKABLE;
    }
    else if ( Event.State >>= aStrValue )
    {
        if ( m_bEnumCommand )
        {
            bValue = aStrValue == m_aEnumCommand;

            m_xToolbar->CheckItem( m_nID, bValue );
            if ( bValue )
                eTri = TRISTATE_TRUE;
            nItemBits |= ToolBoxItemBits::CHECKABLE;
        }
        else
        {
            // Replacement for place holders
            if ( aStrValue.startsWith("($1)") )
            {
                aStrValue = FwkResId(STR_UPDATEDOC) + " " + aStrValue.copy( 4 );
            }
            else if ( aStrValue.startsWith("($2)") )
            {
                aStrValue = FwkResId(STR_CLOSEDOC_ANDRETURN) + aStrValue.copy( 4 );
            }
            else if ( aStrValue.startsWith("($3)") )
            {
                aStrValue = FwkResId(STR_SAVECOPYDOC) + aStrValue.copy( 4 );
            }
            m_xToolbar->SetItemText( m_nID, aStrValue );
            // tdf#124267 strip mnemonic from tooltip
            m_xToolbar->SetQuickHelpText(m_nID, aStrValue.replaceFirst("~", ""));
        }

        if ( m_bMadeInvisible )
            m_xToolbar->ShowItem( m_nID );
    }
    else if (( Event.State >>= aItemState ) && !m_bEnumCommand )
    {
        eTri = TRISTATE_INDET;
        nItemBits |= ToolBoxItemBits::CHECKABLE;
        if ( m_bMadeInvisible )
            m_xToolbar->ShowItem( m_nID );
    }
    else if ( Event.State >>= aItemVisibility )
    {
        m_xToolbar->ShowItem( m_nID, aItemVisibility.bVisible );
        m_bMadeInvisible = !aItemVisibility.bVisible;
    }
    else if ( Event.State >>= aControlCommand )
    {
        if (aControlCommand.Command == "SetQuickHelpText")
        {
            for ( NamedValue const & rArg : std::as_const(aControlCommand.Arguments) )
            {
                if (rArg.Name == "HelpText")
                {
                    OUString aHelpText;
                    rArg.Value >>= aHelpText;
                    m_xToolbar->SetQuickHelpText(m_nID, aHelpText);
                    break;
                }
            }
        }
        if ( m_bMadeInvisible )
            m_xToolbar->ShowItem( m_nID );
    }
    else if ( m_bMadeInvisible )
        m_xToolbar->ShowItem( m_nID );

    m_xToolbar->SetItemState( m_nID, eTri );
    m_xToolbar->SetItemBits( m_nID, nItemBits );
}

IMPL_STATIC_LINK( GenericToolbarController, ExecuteHdl_Impl, void*, p, void )
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

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
