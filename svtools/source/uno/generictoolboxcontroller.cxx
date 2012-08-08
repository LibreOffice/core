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

#include <svtools/generictoolboxcontroller.hxx>

#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/status/ItemState.hpp>

#include <comphelper/componentcontext.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;
using namespace ::com::sun::star::util;

namespace svt
{

struct ExecuteInfo
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >     xDispatch;
    ::com::sun::star::util::URL                                                aTargetURL;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >  aArgs;
};

GenericToolboxController::GenericToolboxController( const Reference< XMultiServiceFactory >& rServiceManager,
                                                    const Reference< XFrame >&               rFrame,
                                                    ToolBox*                                 pToolbox,
                                                    sal_uInt16                                   nID,
                                                    const ::rtl::OUString&                          aCommand ) :
    svt::ToolboxController( rServiceManager, rFrame, aCommand )
    ,   m_pToolbox( pToolbox )
    ,   m_nID( nID )
{
    // Initialization is done through ctor
    m_bInitialized = sal_True;

    // insert main command to our listener map
    if ( !m_aCommandURL.isEmpty() )
        m_aListenerMap.insert( URLToDispatchMap::value_type( aCommand, Reference< XDispatch >() ));
}

GenericToolboxController::~GenericToolboxController()
{
}

void SAL_CALL GenericToolboxController::dispose()
throw ( RuntimeException )
{
    SolarMutexGuard aSolarMutexGuard;

    svt::ToolboxController::dispose();

    m_pToolbox = 0;
    m_nID = 0;
}

void SAL_CALL GenericToolboxController::execute( sal_Int16 /*KeyModifier*/ )
throw ( RuntimeException )
{
    Reference< XDispatch >       xDispatch;
    Reference< XURLTransformer > xURLTransformer;
    ::rtl::OUString                     aCommandURL;

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( m_bDisposed )
            throw DisposedException();

        if ( m_bInitialized &&
             m_xFrame.is() &&
             m_xServiceManager.is() &&
             !m_aCommandURL.isEmpty() )
        {
            xURLTransformer = com::sun::star::util::URLTransformer::create( ::comphelper::ComponentContext(m_xServiceManager).getUNOContext() );

            aCommandURL = m_aCommandURL;
            URLToDispatchMap::iterator pIter = m_aListenerMap.find( m_aCommandURL );
            if ( pIter != m_aListenerMap.end() )
                xDispatch = pIter->second;
        }
    }

    if ( xDispatch.is() && xURLTransformer.is() )
    {
        com::sun::star::util::URL aTargetURL;
        Sequence<PropertyValue>   aArgs;

        aTargetURL.Complete = aCommandURL;
        xURLTransformer->parseStrict( aTargetURL );

        // Execute dispatch asynchronously
        ExecuteInfo* pExecuteInfo = new ExecuteInfo;
        pExecuteInfo->xDispatch     = xDispatch;
        pExecuteInfo->aTargetURL    = aTargetURL;
        pExecuteInfo->aArgs         = aArgs;
        Application::PostUserEvent( STATIC_LINK(0, GenericToolboxController , ExecuteHdl_Impl), pExecuteInfo );
    }
}

void GenericToolboxController::statusChanged( const FeatureStateEvent& Event )
throw ( RuntimeException )
{
    SolarMutexGuard aSolarMutexGuard;

    if ( m_bDisposed )
        return;

    if ( m_pToolbox )
    {
        m_pToolbox->EnableItem( m_nID, Event.IsEnabled );

        sal_uInt16 nItemBits = m_pToolbox->GetItemBits( m_nID );
        nItemBits &= ~TIB_CHECKABLE;
        TriState eTri = STATE_NOCHECK;

        sal_Bool        bValue = sal_Bool();
        rtl::OUString   aStrValue;
        ItemStatus      aItemState;

        if ( Event.State >>= bValue )
        {
            // Boolean, treat it as checked/unchecked
            m_pToolbox->SetItemBits( m_nID, nItemBits );
            m_pToolbox->CheckItem( m_nID, bValue );
            if ( bValue )
                eTri = STATE_CHECK;
            nItemBits |= TIB_CHECKABLE;
        }
        else if ( Event.State >>= aStrValue )
        {
            m_pToolbox->SetItemText( m_nID, aStrValue );
        }
        else if ( Event.State >>= aItemState )
        {
            eTri = STATE_DONTKNOW;
            nItemBits |= TIB_CHECKABLE;
        }

        m_pToolbox->SetItemState( m_nID, eTri );
        m_pToolbox->SetItemBits( m_nID, nItemBits );
    }
}

IMPL_STATIC_LINK_NOINSTANCE( GenericToolboxController, ExecuteHdl_Impl, ExecuteInfo*, pExecuteInfo )
{
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
   return 0;
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
