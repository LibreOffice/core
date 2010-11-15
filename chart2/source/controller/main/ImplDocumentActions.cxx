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

#include "ImplDocumentActions.hxx"
#include "DisposeHelper.hxx"
#include "CommonFunctors.hxx"
#include "ControllerLockGuard.hxx"
#include "PropertyHelper.hxx"
#include "DataSourceHelper.hxx"
#include "ChartModelHelper.hxx"

#include <com/sun/star/chart/XComplexDescriptionAccess.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifiable.hpp>
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
    using ::com::sun::star::chart::XComplexDescriptionAccess;
    using ::com::sun::star::chart2::XTitled;
    using ::com::sun::star::chart2::XInternalDataProvider;
    using ::com::sun::star::util::XModifiable;
    using ::com::sun::star::document::UndoFailedException;
    /** === end UNO using === **/

// =====================================================================================================================
// = helper
// =====================================================================================================================
namespace
{
    Reference< XModel > lcl_cloneModel( const Reference< XModel > & xModel )
    {
        Reference< XModel > xResult;
        try
        {
            const Reference< XCloneable > xCloneable( xModel, UNO_QUERY_THROW );
            xResult.set( xCloneable->createClone(), UNO_QUERY_THROW );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return xResult;
    }

}

// =====================================================================================================================
// = ChartModelClone
// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------
ChartModelClone::ChartModelClone( const Reference< XModel >& i_model, const ModelFacet i_facet )
{
    m_xModelClone.set( lcl_cloneModel( i_model ) );

    try
    {
        if ( i_facet == E_MODEL_WITH_DATA )
        {
            const Reference< XChartDocument > xChartDoc( m_xModelClone, UNO_QUERY_THROW );
            ENSURE_OR_THROW( xChartDoc->hasInternalDataProvider(), "invalid chart model" );

            const Reference< XCloneable > xCloneable( xChartDoc->getDataProvider(), UNO_QUERY_THROW );
            m_xDataClone.set( xCloneable->createClone(), UNO_QUERY_THROW );
        }

        if ( i_facet == E_MODEL_WITH_SELECTION )
        {
            const Reference< XSelectionSupplier > xSelSupp( m_xModelClone->getCurrentController(), UNO_QUERY_THROW );
            m_aSelection = xSelSupp->getSelection();
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}


// ---------------------------------------------------------------------------------------------------------------------
ChartModelClone::~ChartModelClone()
{
    if ( !impl_isDisposed() )
        dispose();
}

// ---------------------------------------------------------------------------------------------------------------------
void ChartModelClone::dispose()
{
    if ( impl_isDisposed() )
        return;

    try
    {
        Reference< XComponent > xComp( m_xModelClone, UNO_QUERY_THROW );
        xComp->dispose();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    m_xModelClone.clear();
    m_xDataClone.clear();
    m_aSelection.clear();
}

// ---------------------------------------------------------------------------------------------------------------------
ModelFacet ChartModelClone::getFacet() const
{
    if ( m_aSelection.hasValue() )
        return E_MODEL_WITH_SELECTION;
    if ( m_xDataClone.is() )
        return E_MODEL_WITH_DATA;
    return E_MODEL;
}

// ---------------------------------------------------------------------------------------------------------------------
void ChartModelClone::applyToModel( const Reference< XModel >& i_model ) const
{
    applyModelContentToModel( i_model, m_xModelClone, m_xDataClone );

    if ( m_aSelection.hasValue() )
    {
        try
        {
            Reference< XSelectionSupplier > xCurrentSelectionSuppl( i_model->getCurrentController(), UNO_QUERY_THROW );
            xCurrentSelectionSuppl->select( m_aSelection );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
namespace
{
    void ImplApplyDataToModel( const Reference< XModel >& i_model, const Reference< XInternalDataProvider > & i_data )
    {
        Reference< XChartDocument > xDoc( i_model, UNO_QUERY );
        OSL_ASSERT( xDoc.is() && xDoc->hasInternalDataProvider() );

        // copy data from stored internal data provider
        if( xDoc.is() && xDoc->hasInternalDataProvider())
        {
            Reference< XComplexDescriptionAccess > xCurrentData( xDoc->getDataProvider(), UNO_QUERY );
            Reference< XComplexDescriptionAccess > xSavedData( i_data, UNO_QUERY );
            if ( xCurrentData.is() && xSavedData.is() )
            {
                xCurrentData->setData( xSavedData->getData() );
                xCurrentData->setComplexRowDescriptions( xSavedData->getComplexRowDescriptions() );
                xCurrentData->setComplexColumnDescriptions( xSavedData->getComplexColumnDescriptions() );
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ChartModelClone::applyModelContentToModel( const Reference< XModel >& i_model,
    const Reference< XModel >& i_modelToCopyFrom, const Reference< XInternalDataProvider >& i_data )
{
    ENSURE_OR_RETURN_VOID( i_model.is(), "ChartModelElement::applyModelContentToModel: invalid source model!" );
    ENSURE_OR_RETURN_VOID( i_modelToCopyFrom.is(), "ChartModelElement::applyModelContentToModel: invalid source model!" );
    try
    {
        // /-- loccked controllers of destination
        ControllerLockGuard aLockedControllers( i_model );
        Reference< XChartDocument > xSource( i_modelToCopyFrom, UNO_QUERY_THROW );
        Reference< XChartDocument > xDestination( i_model, UNO_QUERY_THROW );

        // propagate the correct flag for plotting of hidden values to the data provider and all used sequences
        ChartModelHelper::setIncludeHiddenCells( ChartModelHelper::isIncludeHiddenCells( i_modelToCopyFrom ) , i_model );

        // diagram
        xDestination->setFirstDiagram( xSource->getFirstDiagram() );

        // main title
        Reference< XTitled > xDestinationTitled( xDestination, UNO_QUERY_THROW );
        Reference< XTitled > xSourceTitled( xSource, UNO_QUERY_THROW );
        xDestinationTitled->setTitleObject( xSourceTitled->getTitleObject() );

        // page background
        ::comphelper::copyProperties(
            xSource->getPageBackground(),
            xDestination->getPageBackground() );

        // apply data (not applied in standard Undo)
        if ( i_data.is() )
            ImplApplyDataToModel( i_model, i_data );

        // register all sequences at the internal data provider to get adapted
        // indexes when columns are added/removed
        if ( xDestination->hasInternalDataProvider() )
        {
            Reference< XInternalDataProvider > xNewDataProvider( xDestination->getDataProvider(), UNO_QUERY );
            Reference< chart2::data::XDataSource > xUsedData( DataSourceHelper::getUsedData( i_model ) );
            if ( xUsedData.is() && xNewDataProvider.is() )
            {
                Sequence< Reference< chart2::data::XLabeledDataSequence > > aData( xUsedData->getDataSequences() );
                for( sal_Int32 i=0; i<aData.getLength(); ++i )
                {
                    xNewDataProvider->registerDataSequenceForChanges( aData[i]->getValues() );
                    xNewDataProvider->registerDataSequenceForChanges( aData[i]->getLabel() );
                }
            }
        }

        // restore modify status
        Reference< XModifiable > xSourceMod( xSource, UNO_QUERY );
        Reference< XModifiable > xDestMod( xDestination, UNO_QUERY );
        if ( xSourceMod.is() && xDestMod.is() && !xSourceMod->isModified() )
        {
            xDestMod->setModified( sal_False );
        }
        // \-- loccked controllers of destination
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

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
