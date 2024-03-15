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

#include "ChartModelClone.hxx"
#include <ChartModel.hxx>
#include <ChartModelHelper.hxx>
#include <ControllerLockGuard.hxx>
#include <DataSource.hxx>
#include <DataSourceHelper.hxx>

#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>

#include <comphelper/property.hxx>
#include <comphelper/diagnose_ex.hxx>

namespace chart
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::util::XCloneable;
    using ::com::sun::star::chart2::XChartDocument;
    using ::com::sun::star::chart2::XInternalDataProvider;
    using ::com::sun::star::chart2::XAnyDescriptionAccess;
    using ::com::sun::star::view::XSelectionSupplier;
    using ::com::sun::star::chart2::data::XLabeledDataSequence;

    // = helper
    namespace
    {
        rtl::Reference<::chart::ChartModel> lcl_cloneModel( const rtl::Reference<::chart::ChartModel> & xModel )
        {
            try
            {
                return new ChartModel(*xModel);
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
            return nullptr;
        }

    }

    // = ChartModelClone
    ChartModelClone::ChartModelClone( const rtl::Reference<::chart::ChartModel>& i_model, const ModelFacet i_facet )
    {
        m_xModelClone = lcl_cloneModel( i_model );

        try
        {
            if ( i_facet == E_MODEL_WITH_DATA )
            {
                ENSURE_OR_THROW( m_xModelClone && m_xModelClone->hasInternalDataProvider(), "invalid chart model" );

                const Reference< XCloneable > xCloneable( m_xModelClone->getDataProvider(), UNO_QUERY_THROW );
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
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    ChartModelClone::~ChartModelClone()
    {
        if ( !impl_isDisposed() )
            dispose();
    }

    void ChartModelClone::dispose()
    {
        if ( impl_isDisposed() )
            return;

        m_xModelClone.clear();
        m_xDataClone.clear();
        m_aSelection.clear();
    }

    ModelFacet ChartModelClone::getFacet() const
    {
        if ( m_aSelection.hasValue() )
            return E_MODEL_WITH_SELECTION;
        if ( m_xDataClone.is() )
            return E_MODEL_WITH_DATA;
        return E_MODEL;
    }

    void ChartModelClone::applyToModel( const rtl::Reference<::chart::ChartModel>& i_model ) const
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
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
        }
    }

    namespace
    {
        void ImplApplyDataToModel( const Reference< XModel >& i_model, const Reference< XInternalDataProvider > & i_data )
        {
            Reference< XChartDocument > xDoc( i_model, UNO_QUERY );
            OSL_ASSERT( xDoc.is() && xDoc->hasInternalDataProvider() );

            // copy data from stored internal data provider
            if( xDoc.is() && xDoc->hasInternalDataProvider())
            {
                Reference< XAnyDescriptionAccess > xCurrentData( xDoc->getDataProvider(), UNO_QUERY );
                Reference< XAnyDescriptionAccess > xSavedData( i_data, UNO_QUERY );
                if ( xCurrentData.is() && xSavedData.is() )
                {
                    xCurrentData->setData( xSavedData->getData() );
                    xCurrentData->setAnyRowDescriptions( xSavedData->getAnyRowDescriptions());
                    xCurrentData->setAnyColumnDescriptions( xSavedData->getAnyColumnDescriptions());
                }
            }
        }
    }

    void ChartModelClone::applyModelContentToModel( const rtl::Reference<::chart::ChartModel>& i_model,
        const rtl::Reference<::chart::ChartModel>& i_modelToCopyFrom,
        const Reference< XInternalDataProvider >& i_data )
    {
        ENSURE_OR_RETURN_VOID( i_model.is(), "ChartModelElement::applyModelContentToModel: invalid source model!" );
        ENSURE_OR_RETURN_VOID( i_modelToCopyFrom.is(), "ChartModelElement::applyModelContentToModel: invalid source model!" );
        try
        {
            // locked controllers of destination
            ControllerLockGuardUNO aLockedControllers( i_model );

            // propagate the correct flag for plotting of hidden values to the data provider and all used sequences
            ChartModelHelper::setIncludeHiddenCells(ChartModelHelper::isIncludeHiddenCells( i_modelToCopyFrom ), *i_model);

            // diagram
            i_model->setFirstDiagram( i_modelToCopyFrom->getFirstDiagram() );

            // main title
            i_model->setTitleObject( i_modelToCopyFrom->getTitleObject() );

            // page background
            ::comphelper::copyProperties(
                i_modelToCopyFrom->getPageBackground(),
                i_model->getPageBackground() );

            // apply data (not applied in standard Undo)
            if ( i_data.is() )
                ImplApplyDataToModel( i_model, i_data );

            // register all sequences at the internal data provider to get adapted
            // indexes when columns are added/removed
            if ( i_model->hasInternalDataProvider() )
            {
                Reference< XInternalDataProvider > xNewDataProvider( i_model->getDataProvider(), UNO_QUERY );
                rtl::Reference< DataSource > xUsedData = DataSourceHelper::getUsedData( *i_model );
                if ( xUsedData.is() && xNewDataProvider.is() )
                {
                    const Sequence< Reference< XLabeledDataSequence > > aData( xUsedData->getDataSequences() );
                    for( Reference< XLabeledDataSequence > const & labeledDataSeq : aData )
                    {
                        xNewDataProvider->registerDataSequenceForChanges( labeledDataSeq->getValues() );
                        xNewDataProvider->registerDataSequenceForChanges( labeledDataSeq->getLabel() );
                    }
                }
            }

            // restore modify status
            if ( !i_modelToCopyFrom->isModified() )
            {
                i_model->setModified( false );
            }
            // \-- locked controllers of destination
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
