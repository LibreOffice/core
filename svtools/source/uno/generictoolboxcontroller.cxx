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

#include <svtools/generictoolboxcontroller.hxx>

#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/XDispatch.hpp>

#include <vcl/svapp.hxx>

using namespace css::awt;
using namespace css::uno;
using namespace css::beans;
using namespace css::lang;
using namespace css::frame;
using namespace css::frame::status;
using namespace css::util;

namespace svt
{

struct ExecuteInfo
{
    css::uno::Reference< css::frame::XDispatch >     xDispatch;
    css::util::URL                                                aTargetURL;
    css::uno::Sequence< css::beans::PropertyValue >  aArgs;
};

GenericToolboxController::GenericToolboxController( const Reference< XComponentContext >& rxContext,
                                                    const Reference< XFrame >&            rFrame,
                                                    ToolBox*                              pToolbox,
                                                    sal_uInt16                            nID,
                                                    const OUString&                       aCommand ) :
    svt::ToolboxController( rxContext, rFrame, aCommand )
    ,   m_pToolbox( pToolbox )
    ,   m_nID( nID )
{
    // Initialization is done through ctor
    m_bInitialized = true;

    // insert main command to our listener map
    if ( !m_aCommandURL.isEmpty() )
        m_aListenerMap.emplace( aCommand, Reference< XDispatch >() );
}

GenericToolboxController::~GenericToolboxController()
{
}

void SAL_CALL GenericToolboxController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;
    m_pToolbox.clear();
    m_nID = 0;
    svt::ToolboxController::dispose();
}

void SAL_CALL GenericToolboxController::execute( sal_Int16 /*KeyModifier*/ )
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
    Sequence<PropertyValue>   aArgs;

    aTargetURL.Complete = aCommandURL;
    if ( m_xUrlTransformer.is() )
        m_xUrlTransformer->parseStrict( aTargetURL );

    // Execute dispatch asynchronously
    ExecuteInfo* pExecuteInfo = new ExecuteInfo;
    pExecuteInfo->xDispatch     = xDispatch;
    pExecuteInfo->aTargetURL    = aTargetURL;
    pExecuteInfo->aArgs         = aArgs;
    Application::PostUserEvent( LINK(nullptr, GenericToolboxController , ExecuteHdl_Impl), pExecuteInfo );
}

void GenericToolboxController::statusChanged( const FeatureStateEvent& Event )
{
    SolarMutexGuard aSolarMutexGuard;

    if ( m_bDisposed )
        return;

    if ( !m_pToolbox )
        return;

    m_pToolbox->EnableItem( m_nID, Event.IsEnabled );

    ToolBoxItemBits nItemBits = m_pToolbox->GetItemBits( m_nID );
    nItemBits &= ~ToolBoxItemBits::CHECKABLE;
    TriState eTri = TRISTATE_FALSE;

    bool        bValue;
    OUString    aStrValue;
    ItemStatus  aItemState;

    if ( Event.State >>= bValue )
    {
        // Boolean, treat it as checked/unchecked
        m_pToolbox->SetItemBits( m_nID, nItemBits );
        m_pToolbox->CheckItem( m_nID, bValue );
        if ( bValue )
            eTri = TRISTATE_TRUE;
        nItemBits |= ToolBoxItemBits::CHECKABLE;
    }
    else if ( Event.State >>= aStrValue )
    {
        m_pToolbox->SetItemText( m_nID, aStrValue );
    }
    else if ( Event.State >>= aItemState )
    {
        eTri = TRISTATE_INDET;
        nItemBits |= ToolBoxItemBits::CHECKABLE;
    }

    m_pToolbox->SetItemState( m_nID, eTri );
    m_pToolbox->SetItemBits( m_nID, nItemBits );
}

IMPL_STATIC_LINK( GenericToolboxController, ExecuteHdl_Impl, void*, p, void )
{
   ExecuteInfo* pExecuteInfo = static_cast<ExecuteInfo*>(p);
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
   delete pExecuteInfo;
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
