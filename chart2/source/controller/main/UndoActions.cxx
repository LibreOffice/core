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

#include "UndoActions.hxx"
#include "DisposeHelper.hxx"
#include "CommonFunctors.hxx"
#include "PropertyHelper.hxx"
#include "ChartModelClone.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <tools/diagnose_ex.h>
#include <svx/svdundo.hxx>

#include <boost/bind.hpp>
#include <algorithm>

using namespace ::com::sun::star;

using ::rtl::OUString;

namespace chart
{
namespace impl
{

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::util::XCloneable;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::view::XSelectionSupplier;
    using ::com::sun::star::chart2::XChartDocument;
    using ::com::sun::star::document::UndoFailedException;
    /** === end UNO using === **/

// ---------------------------------------------------------------------------------------------------------------------
UndoElement::UndoElement( const OUString& i_actionString, const Reference< XModel >& i_documentModel, const ::boost::shared_ptr< ChartModelClone >& i_modelClone )
    :UndoElement_MBase()
    ,UndoElement_TBase( m_aMutex )
    ,m_sActionString( i_actionString )
    ,m_xDocumentModel( i_documentModel )
    ,m_pModelClone( i_modelClone )
{
}

// ---------------------------------------------------------------------------------------------------------------------
UndoElement::~UndoElement()
{
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL UndoElement::disposing()
{
    if ( !!m_pModelClone )
        m_pModelClone->dispose();
    m_pModelClone.reset();
    m_xDocumentModel.clear();
}

// ---------------------------------------------------------------------------------------------------------------------
::rtl::OUString SAL_CALL UndoElement::getTitle() throw (RuntimeException)
{
    return m_sActionString;
}

// ---------------------------------------------------------------------------------------------------------------------
void UndoElement::impl_toggleModelState()
{
    // get a snapshot of the current state of our model
    ::boost::shared_ptr< ChartModelClone > pNewClone( new ChartModelClone( m_xDocumentModel, m_pModelClone->getFacet() ) );
    // apply the previous snapshot to our model
    m_pModelClone->applyToModel( m_xDocumentModel );
    // remember the new snapshot, for the next toggle
    m_pModelClone = pNewClone;
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL UndoElement::undo(  ) throw (UndoFailedException, RuntimeException)
{
    impl_toggleModelState();
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL UndoElement::redo(  ) throw (UndoFailedException, RuntimeException)
{
    impl_toggleModelState();
}

// =====================================================================================================================
// = ShapeUndoElement
// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------
ShapeUndoElement::ShapeUndoElement( SdrUndoAction& i_sdrUndoAction )
    :ShapeUndoElement_MBase()
    ,ShapeUndoElement_TBase( m_aMutex )
    ,m_pAction( &i_sdrUndoAction )
{
}

// ---------------------------------------------------------------------------------------------------------------------
ShapeUndoElement::~ShapeUndoElement()
{
}

// ---------------------------------------------------------------------------------------------------------------------
::rtl::OUString SAL_CALL ShapeUndoElement::getTitle() throw (RuntimeException)
{
    if ( !m_pAction )
        throw DisposedException( ::rtl::OUString(), *this );
    return m_pAction->GetComment();
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL ShapeUndoElement::undo(  ) throw (UndoFailedException, RuntimeException)
{
    if ( !m_pAction )
        throw DisposedException( ::rtl::OUString(), *this );
    m_pAction->Undo();
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL ShapeUndoElement::redo(  ) throw (UndoFailedException, RuntimeException)
{
    if ( !m_pAction )
        throw DisposedException( ::rtl::OUString(), *this );
    m_pAction->Redo();
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL ShapeUndoElement::disposing()
{
}

} // namespace impl
} //  namespace chart
