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
#include "ChartModelHelper.hxx"
#include "ControllerLockGuard.hxx"
#include "DataSourceHelper.hxx"

#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>

#include <comphelper/property.hxx>
#include <tools/diagnose_ex.h>

namespace chart
{

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
    using ::com::sun::star::chart2::XChartDocument;
    using ::com::sun::star::chart2::XInternalDataProvider;
    using ::com::sun::star::chart2::XAnyDescriptionAccess;
    using ::com::sun::star::view::XSelectionSupplier;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::chart2::XTitled;
    using ::com::sun::star::util::XModifiable;
    using ::com::sun::star::chart2::data::XDataSource;
    using ::com::sun::star::chart2::data::XLabeledDataSequence;

    // = helper
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

    // = ChartModelClone
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

    ChartModelClone::~ChartModelClone()
    {
        if ( !impl_isDisposed() )
            dispose();
    }

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

    ModelFacet ChartModelClone::getFacet() const
    {
        if ( m_aSelection.hasValue() )
            return E_MODEL_WITH_SELECTION;
        if ( m_xDataClone.is() )
            return E_MODEL_WITH_DATA;
        return E_MODEL;
    }

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
                Reference< XDataSource > xUsedData( DataSourceHelper::getUsedData( i_model ) );
                if ( xUsedData.is() && xNewDataProvider.is() )
                {
                    Sequence< Reference< XLabeledDataSequence > > aData( xUsedData->getDataSequences() );
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

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
