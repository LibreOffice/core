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

#include "StatusBarCommandDispatch.hxx"
#include "ObjectNameProvider.hxx"
#include "macros.hxx"
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include "ResId.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

StatusBarCommandDispatch::StatusBarCommandDispatch(
    const Reference< uno::XComponentContext > & xContext,
    const Reference< frame::XModel > & xModel,
    const Reference< view::XSelectionSupplier > & xSelSupp ) :
        impl::StatusBarCommandDispatch_Base( xContext ),
        m_xModifiable( xModel, uno::UNO_QUERY  ),
        m_xSelectionSupplier( xSelSupp ),
        m_bIsModified( false )
{}

StatusBarCommandDispatch::~StatusBarCommandDispatch()
{}

void StatusBarCommandDispatch::initialize()
{
    if( m_xModifiable.is())
    {
        Reference< util::XModifyBroadcaster > xModifyBroadcaster( m_xModifiable, uno::UNO_QUERY );
        OSL_ASSERT( xModifyBroadcaster.is());
        if( xModifyBroadcaster.is())
            xModifyBroadcaster->addModifyListener( this );
    }

    if( m_xSelectionSupplier.is())
    {
        m_xSelectionSupplier->addSelectionChangeListener( this );
    }
}

void StatusBarCommandDispatch::fireStatusEvent(
    const OUString & rURL,
    const Reference< frame::XStatusListener > & xSingleListener /* = 0 */ )
{
    bool bFireAll( rURL.isEmpty() );
    bool bFireContext(  bFireAll || rURL == ".uno:Context" );
    bool bFireModified( bFireAll || rURL == ".uno:ModifiedStatus" );

    if( bFireContext )
    {
        uno::Any aArg;
        Reference< chart2::XChartDocument > xDoc( m_xModifiable, uno::UNO_QUERY );
        aArg <<= ObjectNameProvider::getSelectedObjectText( m_aSelectedOID.getObjectCID(), xDoc );
        fireStatusEventForURL( ".uno:Context", aArg, true, xSingleListener );
    }
    if( bFireModified )
    {
        uno::Any aArg;
        if( m_bIsModified )
            aArg <<= OUString("*");
        fireStatusEventForURL( ".uno:ModifiedStatus", aArg, true, xSingleListener );
    }
}

// ____ XDispatch ____
void SAL_CALL StatusBarCommandDispatch::dispatch(
    const util::URL& /* URL */,
    const Sequence< beans::PropertyValue >& /* Arguments */ )
    throw (uno::RuntimeException)
{
    // nothing to do here
}

// ____ WeakComponentImplHelperBase ____
/// is called when this is disposed
void SAL_CALL StatusBarCommandDispatch::disposing()
{
    m_xModifiable.clear();
    m_xSelectionSupplier.clear();
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL StatusBarCommandDispatch::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
    m_xModifiable.clear();
    m_xSelectionSupplier.clear();
}

// ____ XModifyListener ____
void SAL_CALL StatusBarCommandDispatch::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    if( m_xModifiable.is())
        m_bIsModified = m_xModifiable->isModified();

    CommandDispatch::modified( aEvent );
}

// ____ XSelectionChangeListener ____
void SAL_CALL StatusBarCommandDispatch::selectionChanged( const lang::EventObject& /* aEvent */ )
    throw (uno::RuntimeException)
{
    if( m_xSelectionSupplier.is())
        m_aSelectedOID = ObjectIdentifier( m_xSelectionSupplier->getSelection() );
    else
        m_aSelectedOID = ObjectIdentifier();
    fireAllStatusEvents( 0 );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
