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

#include "UndoGuard.hxx"
#include "ChartModelClone.hxx"
#include "UndoActions.hxx"

#include <com/sun/star/container/XChild.hpp>

#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

//-----------------------------------------------------------------------------

UndoGuard::UndoGuard( const OUString& i_undoString, const uno::Reference< document::XUndoManager > & i_undoManager,
                               const ModelFacet i_facet )
    :m_xChartModel( i_undoManager->getParent(), uno::UNO_QUERY_THROW )
    ,m_xUndoManager( i_undoManager )
    ,m_pDocumentSnapshot()
    ,m_aUndoString( i_undoString )
    ,m_bActionPosted( false )
{
    m_pDocumentSnapshot.reset( new ChartModelClone( m_xChartModel, i_facet ) );
}

//-----------------------------------------------------------------------------

UndoGuard::~UndoGuard()
{
    if ( !!m_pDocumentSnapshot )
        discardSnapshot();
}

//-----------------------------------------------------------------------------

void UndoGuard::commit()
{
    if ( !m_bActionPosted && !!m_pDocumentSnapshot )
    {
        try
        {
            const Reference< document::XUndoAction > xAction( new impl::UndoElement( m_aUndoString, m_xChartModel, m_pDocumentSnapshot ) );
            m_pDocumentSnapshot.reset();    // don't dispose, it's data went over to the UndoElement
            m_xUndoManager->addUndoAction( xAction );
        }
        catch( const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    m_bActionPosted = true;
}

//-----------------------------------------------------------------------------

void UndoGuard::rollback()
{
    ENSURE_OR_RETURN_VOID( !!m_pDocumentSnapshot, "no snapshot!" );
    m_pDocumentSnapshot->applyToModel( m_xChartModel );
    discardSnapshot();
}

//-----------------------------------------------------------------------------
void UndoGuard::discardSnapshot()
{
    ENSURE_OR_RETURN_VOID( !!m_pDocumentSnapshot, "no snapshot!" );
    m_pDocumentSnapshot->dispose();
    m_pDocumentSnapshot.reset();
}

//-----------------------------------------------------------------------------

UndoLiveUpdateGuard::UndoLiveUpdateGuard( const OUString& i_undoString, const uno::Reference< document::XUndoManager >& i_undoManager )
    :UndoGuard( i_undoString, i_undoManager, E_MODEL )
{
}

UndoLiveUpdateGuard::~UndoLiveUpdateGuard()
{
    if ( !isActionPosted() )
        rollback();
}

//-----------------------------------------------------------------------------

UndoLiveUpdateGuardWithData::UndoLiveUpdateGuardWithData(
        const OUString& i_undoString, const uno::Reference< document::XUndoManager >& i_undoManager )
    :UndoGuard( i_undoString, i_undoManager, E_MODEL_WITH_DATA )
{
}

UndoLiveUpdateGuardWithData::~UndoLiveUpdateGuardWithData()
{
    if ( !isActionPosted() )
        rollback();
}

//-----------------------------------------------------------------------------

UndoGuardWithSelection::UndoGuardWithSelection(
        const OUString& i_undoString, const uno::Reference< document::XUndoManager >& i_undoManager )
    :UndoGuard( i_undoString, i_undoManager, E_MODEL_WITH_SELECTION )
{
}

//-----------------------------------------------------------------------------

UndoGuardWithSelection::~UndoGuardWithSelection()
{
    if ( !isActionPosted() )
        rollback();
}

//-----------------------------------------------------------------------------

UndoContext::UndoContext( const Reference< document::XUndoManager > & i_undoManager, const ::rtl::OUString& i_undoTitle )
    :m_xUndoManager( i_undoManager )
{
    ENSURE_OR_THROW( m_xUndoManager.is(), "invalid undo manager!" );
    m_xUndoManager->enterUndoContext( i_undoTitle );
}

//-----------------------------------------------------------------------------

UndoContext::~UndoContext()
{
    m_xUndoManager->leaveUndoContext();
}

//-----------------------------------------------------------------------------

HiddenUndoContext::HiddenUndoContext( const Reference< document::XUndoManager > & i_undoManager )
    :m_xUndoManager( i_undoManager )
{
    ENSURE_OR_THROW( m_xUndoManager.is(), "invalid undo manager!" );
    try
    {
        m_xUndoManager->enterHiddenUndoContext();
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
        m_xUndoManager.clear();
            // prevents the leaveUndoContext in the dtor
    }
}

//-----------------------------------------------------------------------------

HiddenUndoContext::~HiddenUndoContext()
{
    try
    {
        if ( m_xUndoManager.is() )
            m_xUndoManager->leaveUndoContext();
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
