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


#include "uielement/generictoolbarcontroller.hxx"

#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/status/ItemState.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>

#include <comphelper/processfactory.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <tools/urlobj.hxx>
#include <classes/resource.hrc>
#include <classes/fwkresid.hxx>
#include <framework/menuconfiguration.hxx>
#include <uielement/menubarmanager.hxx>

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;

namespace framework
{

static sal_Bool isEnumCommand( const OUString& rCommand )
{
    INetURLObject aURL( rCommand );

    if (( aURL.GetProtocol() == INET_PROT_UNO ) &&
        ( aURL.GetURLPath().indexOf( '.' ) != -1))
        return sal_True;

    return sal_False;
}

static OUString getEnumCommand( const OUString& rCommand )
{
    INetURLObject aURL( rCommand );

    OUString   aEnumCommand;
    String          aURLPath = aURL.GetURLPath();
    xub_StrLen      nIndex   = aURLPath.Search( '.' );
    if (( nIndex > 0 ) && ( nIndex < aURLPath.Len() ))
        aEnumCommand = aURLPath.Copy( nIndex+1 );

    return aEnumCommand;
}

static OUString getMasterCommand( const OUString& rCommand )
{
    OUString aMasterCommand( rCommand );
    INetURLObject aURL( rCommand );
    if ( aURL.GetProtocol() == INET_PROT_UNO )
    {
        sal_Int32 nIndex = aURL.GetURLPath().indexOf( '.' );
        if ( nIndex )
        {
            aURL.SetURLPath( aURL.GetURLPath().copy( 0, nIndex ) );
            aMasterCommand = aURL.GetMainURL( INetURLObject::NO_DECODE );
        }
    }
    return aMasterCommand;
}

struct ExecuteInfo
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >     xDispatch;
    ::com::sun::star::util::URL                                                aTargetURL;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >  aArgs;
};

GenericToolbarController::GenericToolbarController( const Reference< XComponentContext >&    rxContext,
                                                    const Reference< XFrame >&               rFrame,
                                                    ToolBox*                                 pToolbar,
                                                    sal_uInt16                               nID,
                                                    const OUString&                          aCommand ) :
    svt::ToolboxController( rxContext, rFrame, aCommand )
    ,   m_pToolbar( pToolbar )
    ,   m_nID( nID )
    ,   m_bEnumCommand( isEnumCommand( aCommand ))
    ,   m_bMadeInvisible( sal_False )
    ,   m_aEnumCommand( getEnumCommand( aCommand ))
{
    if ( m_bEnumCommand )
        addStatusListener( getMasterCommand( aCommand ) );
}

GenericToolbarController::~GenericToolbarController()
{
}

void SAL_CALL GenericToolbarController::dispose()
throw ( RuntimeException )
{
    SolarMutexGuard aSolarMutexGuard;

    svt::ToolboxController::dispose();

    m_pToolbar = 0;
    m_nID = 0;
}

void SAL_CALL GenericToolbarController::execute( sal_Int16 KeyModifier )
throw ( RuntimeException )
{
    Reference< XDispatch >       xDispatch;
    Reference< XURLTransformer > xURLTransformer;
    OUString                     aCommandURL;

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( m_bDisposed )
            throw DisposedException();

        if ( m_bInitialized &&
             m_xFrame.is() &&
             !m_aCommandURL.isEmpty() )
        {
            xURLTransformer = URLTransformer::create(m_xContext);

            aCommandURL = m_aCommandURL;
            URLToDispatchMap::iterator pIter = m_aListenerMap.find( m_aCommandURL );
            if ( pIter != m_aListenerMap.end() )
                xDispatch = pIter->second;
        }
    }

    if ( xDispatch.is() && xURLTransformer.is() )
    {
        com::sun::star::util::URL aTargetURL;
        Sequence<PropertyValue>   aArgs( 1 );

        // Add key modifier to argument list
        aArgs[0].Name  = OUString( "KeyModifier" );
        aArgs[0].Value <<= KeyModifier;

        aTargetURL.Complete = aCommandURL;
        xURLTransformer->parseStrict( aTargetURL );

        // Execute dispatch asynchronously
        ExecuteInfo* pExecuteInfo = new ExecuteInfo;
        pExecuteInfo->xDispatch     = xDispatch;
        pExecuteInfo->aTargetURL    = aTargetURL;
        pExecuteInfo->aArgs         = aArgs;
        Application::PostUserEvent( STATIC_LINK(0, GenericToolbarController , ExecuteHdl_Impl), pExecuteInfo );
    }
}

void GenericToolbarController::statusChanged( const FeatureStateEvent& Event )
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
        OUString   aStrValue;
        ItemStatus      aItemState;
        Visibility      aItemVisibility;

        if (( Event.State >>= bValue ) && !m_bEnumCommand )
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
            if ( m_bEnumCommand )
            {
                if ( aStrValue == m_aEnumCommand )
                    bValue = sal_True;
                else
                    bValue = sal_False;

                m_pToolbar->CheckItem( m_nID, bValue );
                if ( bValue )
                    eTri = STATE_CHECK;
                nItemBits |= TIB_CHECKABLE;
            }
            else
            {
                // Replacement for place holders
                if ( aStrValue.matchAsciiL( "($1)", 4 ))
                {
                    String aResStr = String( FwkResId( STR_UPDATEDOC ));
                    OUString aTmp( aResStr );
                    aTmp += OUString( " " );
                    aTmp += aStrValue.copy( 4 );
                    aStrValue = aTmp;
                }
                else if ( aStrValue.matchAsciiL( "($2)", 4 ))
                {
                    String aResStr = String( FwkResId( STR_CLOSEDOC_ANDRETURN ));
                    OUString aTmp( aResStr );
                    aTmp += aStrValue.copy( 4 );
                    aStrValue = aTmp;
                }
                else if ( aStrValue.matchAsciiL( "($3)", 4 ))
                {
                    String aResStr = String( FwkResId( STR_SAVECOPYDOC ));
                    OUString aTmp( aResStr );
                    aTmp += aStrValue.copy( 4 );
                    aStrValue = aTmp;
                }
                OUString aText( MnemonicGenerator::EraseAllMnemonicChars( aStrValue ) );
                m_pToolbar->SetItemText( m_nID, aText );
                m_pToolbar->SetQuickHelpText( m_nID, aText );
            }

            if ( m_bMadeInvisible )
                m_pToolbar->ShowItem( m_nID, sal_True );
        }
        else if (( Event.State >>= aItemState ) && !m_bEnumCommand )
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
        else if ( m_bMadeInvisible )
            m_pToolbar->ShowItem( m_nID, sal_True );

        m_pToolbar->SetItemState( m_nID, eTri );
        m_pToolbar->SetItemBits( m_nID, nItemBits );
    }
}

IMPL_STATIC_LINK_NOINSTANCE( GenericToolbarController, ExecuteHdl_Impl, ExecuteInfo*, pExecuteInfo )
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

MenuToolbarController::MenuToolbarController( const Reference< XComponentContext >& rxContext,
                                              const Reference< XFrame >& rFrame,
                                              ToolBox* pToolBar,
                                              sal_uInt16   nID,
                                              const OUString& aCommand,
                                              const OUString& aModuleIdentifier,
                                              const Reference< XIndexAccess >& xMenuDesc )
    : GenericToolbarController( rxContext, rFrame, pToolBar, nID, aCommand ),
      m_xMenuDesc( xMenuDesc ),
      pMenu( NULL ),
      m_aModuleIdentifier( aModuleIdentifier )
{
}

MenuToolbarController::~MenuToolbarController()
{
    try
    {
        if ( m_xMenuManager.is() )
            m_xMenuManager->dispose();
    }
    catch( const Exception& ) {}
    if ( pMenu )
    {
        delete pMenu;
        pMenu = NULL;
    }

}

class Toolbarmenu : public ::PopupMenu
{
    public:
    Toolbarmenu();
    ~Toolbarmenu();
};

Toolbarmenu::Toolbarmenu()
{
    OSL_TRACE("**** contstructing Toolbarmenu 0x%x", this );
}

Toolbarmenu::~Toolbarmenu()
{
    OSL_TRACE("**** destructing Toolbarmenu 0x%x", this );
}

void SAL_CALL MenuToolbarController::click() throw (RuntimeException)
{
    createPopupWindow();
}

Reference< XWindow > SAL_CALL
MenuToolbarController::createPopupWindow() throw (::com::sun::star::uno::RuntimeException)
{
    if ( !pMenu )
    {
        Reference< XDispatchProvider > xDispatch;
        Reference< XURLTransformer > xURLTransformer = URLTransformer::create( m_xContext );
        pMenu = new Toolbarmenu();
        m_xMenuManager.set( new MenuBarManager( m_xContext, m_xFrame, xURLTransformer, xDispatch, m_aModuleIdentifier, pMenu, sal_True, sal_True ) );
        if ( m_xMenuManager.is() )
        {
            MenuBarManager* pMgr = dynamic_cast< MenuBarManager* >( m_xMenuManager.get() );
            pMgr->SetItemContainer( m_xMenuDesc );
        }
    }

    if ( !pMenu || !m_pToolbar )
        return NULL;

    OSL_ENSURE ( pMenu->GetItemCount(), "Empty PopupMenu!" );

    ::Rectangle aRect( m_pToolbar->GetItemRect( m_nID ) );
    pMenu->Execute( m_pToolbar, aRect, POPUPMENU_EXECUTE_DOWN );

    return NULL;
}
} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
