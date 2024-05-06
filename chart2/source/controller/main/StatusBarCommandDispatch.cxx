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
#include <ObjectNameProvider.hxx>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <ChartModel.hxx>
#include <utility>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

StatusBarCommandDispatch::StatusBarCommandDispatch(
    const Reference< uno::XComponentContext > & xContext,
    rtl::Reference<::chart::ChartModel> xModel,
    const Reference< view::XSelectionSupplier > & xSelSupp ) :
        impl::StatusBarCommandDispatch_Base( xContext ),
        m_xChartModel(std::move( xModel  )),
        m_xSelectionSupplier( xSelSupp ),
        m_bIsModified( false )
{}

StatusBarCommandDispatch::~StatusBarCommandDispatch()
{}

void StatusBarCommandDispatch::initialize()
{
    if( m_xChartModel.is())
    {
        m_xChartModel->addModifyListener( this );
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

    if( bFireContext && m_xChartModel.is())
    {
        uno::Any aArg;
        aArg <<= ObjectNameProvider::getSelectedObjectText( m_aSelectedOID.getObjectCID(), m_xChartModel );
        fireStatusEventForURL( u".uno:Context"_ustr, aArg, true, xSingleListener );
    }
    if( bFireModified )
    {
        uno::Any aArg;
        if( m_bIsModified )
            aArg <<= u"*"_ustr;
        fireStatusEventForURL( u".uno:ModifiedStatus"_ustr, aArg, true, xSingleListener );
    }
}

// ____ XDispatch ____
void SAL_CALL StatusBarCommandDispatch::dispatch(
    const util::URL& /* URL */,
    const Sequence< beans::PropertyValue >& /* Arguments */ )
{
    // nothing to do here
}

// ____ WeakComponentImplHelperBase ____
/// is called when this is disposed
void StatusBarCommandDispatch::disposing(std::unique_lock<std::mutex>& /*rGuard*/)
{
    m_xChartModel.clear();
    m_xSelectionSupplier.clear();
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL StatusBarCommandDispatch::disposing( const lang::EventObject& /* Source */ )
{
    m_xChartModel.clear();
    m_xSelectionSupplier.clear();
}

// ____ XModifyListener ____
void SAL_CALL StatusBarCommandDispatch::modified( const lang::EventObject& aEvent )
{
    if( m_xChartModel.is())
        m_bIsModified = m_xChartModel->isModified();

    CommandDispatch::modified( aEvent );
}

// ____ XSelectionChangeListener ____
void SAL_CALL StatusBarCommandDispatch::selectionChanged( const lang::EventObject& /* aEvent */ )
{
    if( m_xSelectionSupplier.is())
        m_aSelectedOID = ObjectIdentifier( m_xSelectionSupplier->getSelection() );
    else
        m_aSelectedOID = ObjectIdentifier();
    fireAllStatusEvents( nullptr );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
