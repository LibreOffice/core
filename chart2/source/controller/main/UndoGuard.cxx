/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
