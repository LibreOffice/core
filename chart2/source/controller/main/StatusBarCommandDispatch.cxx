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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "StatusBarCommandDispatch.hxx"
#include "ObjectNameProvider.hxx"
#include "macros.hxx"
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include "ResId.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

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
    bool bFireAll(rURL.getLength() == 0);
    bool bFireContext(  bFireAll || rURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(".uno:Context")));
    bool bFireModified( bFireAll || rURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(".uno:ModifiedStatus")));

    if( bFireContext )
    {
        uno::Any aArg;
        Reference< chart2::XChartDocument > xDoc( m_xModifiable, uno::UNO_QUERY );
        aArg <<= ObjectNameProvider::getSelectedObjectText( m_aSelectedOID.getObjectCID(), xDoc );
        fireStatusEventForURL( C2U(".uno:Context"), aArg, true, xSingleListener );
    }
    if( bFireModified )
    {
        uno::Any aArg;
        if( m_bIsModified )
            aArg <<= C2U("*");
        fireStatusEventForURL( C2U(".uno:ModifiedStatus"), aArg, true, xSingleListener );
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
