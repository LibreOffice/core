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
#include "ImplDocumentActions.hxx"

#include <com/sun/star/container/XChild.hpp>

#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

//-----------------------------------------------------------------------------
namespace
{
    uno::Reference< uno::XInterface > lcl_getParent( const uno::Reference< uno::XInterface >& i_component )
    {
        const uno::Reference< container::XChild > xAsChild( i_component, uno::UNO_QUERY_THROW );
        return xAsChild->getParent();
    }
}

//-----------------------------------------------------------------------------

UndoGuard_Base::UndoGuard_Base( const OUString& i_undoString, const uno::Reference< document::XUndoManager > & i_undoManager )
        :m_xChartModel( lcl_getParent( i_undoManager ), uno::UNO_QUERY_THROW )
        ,m_xUndoManager( i_undoManager )
        ,m_aUndoString( i_undoString )
        ,m_bActionPosted( false )
{
}

//-----------------------------------------------------------------------------

UndoGuard_Base::~UndoGuard_Base()
{
    if ( !!m_pDocumentSnapshot )
        discardSnapshot();
}

//-----------------------------------------------------------------------------

void UndoGuard_Base::commit()
{
    if ( !m_bActionPosted && !!m_pDocumentSnapshot && m_xUndoManager.is() )
    {
        const Reference< document::XUndoAction > xAction( new impl::UndoElement( m_aUndoString, m_xChartModel, m_pDocumentSnapshot ) );
        m_pDocumentSnapshot.reset();    // don't dispose, it's data went over to the UndoElement
        m_xUndoManager->addUndoAction( xAction );
    }
    m_bActionPosted = true;
}

//-----------------------------------------------------------------------------

void UndoGuard_Base::rollback()
{
    ENSURE_OR_RETURN_VOID( !!m_pDocumentSnapshot, "no snapshot!" );
    m_pDocumentSnapshot->applyToModel( m_xChartModel );
    discardSnapshot();
}

//-----------------------------------------------------------------------------

void UndoGuard_Base::takeSnapshot( bool i_withData, bool i_withSelection )
{
    impl::ModelFacet eModelFacet( impl::E_MODEL );
    if ( i_withData )
        eModelFacet = impl::E_MODEL_WITH_DATA;
    else if ( i_withSelection )
        eModelFacet = impl::E_MODEL_WITH_SELECTION;
    m_pDocumentSnapshot.reset( new impl::ChartModelClone( m_xChartModel, eModelFacet ) );
}

//-----------------------------------------------------------------------------
void UndoGuard_Base::discardSnapshot()
{
    ENSURE_OR_RETURN_VOID( !!m_pDocumentSnapshot, "no snapshot!" );
    m_pDocumentSnapshot->dispose();
    m_pDocumentSnapshot.reset();
}

//-----------------------------------------------------------------------------

UndoGuard::UndoGuard( const OUString& i_undoString, const uno::Reference< document::XUndoManager >& i_undoManager )
    :UndoGuard_Base( i_undoString, i_undoManager )
{
    takeSnapshot( false, false );
}

UndoGuard::~UndoGuard()
{
    // nothing to do ... TODO: can this class be removed?
}

//-----------------------------------------------------------------------------

UndoLiveUpdateGuard::UndoLiveUpdateGuard( const OUString& i_undoString, const uno::Reference< document::XUndoManager >& i_undoManager )
    :UndoGuard_Base( i_undoString, i_undoManager )
{
    takeSnapshot( false, false );
}

UndoLiveUpdateGuard::~UndoLiveUpdateGuard()
{
    if ( !isActionPosted() )
        rollback();
}

//-----------------------------------------------------------------------------

UndoLiveUpdateGuardWithData::UndoLiveUpdateGuardWithData(
        const OUString& i_undoString, const uno::Reference< document::XUndoManager >& i_undoManager )
    :UndoGuard_Base( i_undoString, i_undoManager )
{
    takeSnapshot( true, false );
}

UndoLiveUpdateGuardWithData::~UndoLiveUpdateGuardWithData()
{
    if ( !isActionPosted() )
        rollback();
}

//-----------------------------------------------------------------------------

UndoGuardWithSelection::UndoGuardWithSelection(
        const OUString& i_undoString, const uno::Reference< document::XUndoManager >& i_undoManager )
    :UndoGuard_Base( i_undoString, i_undoManager )
{
    takeSnapshot( false, true );
}

UndoGuardWithSelection::~UndoGuardWithSelection()
{
    if ( !isActionPosted() )
        rollback();
}

} //  namespace chart
