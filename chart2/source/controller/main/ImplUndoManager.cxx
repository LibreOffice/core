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

#include "ImplUndoManager.hxx"
#include "DisposeHelper.hxx"
#include "CommonFunctors.hxx"
#include "ControllerLockGuard.hxx"
#include "PropertyHelper.hxx"
#include "DataSourceHelper.hxx"
#include "ChartModelHelper.hxx"

#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <boost/bind.hpp>
#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::com::sun::star::chart2::XAnyDescriptionAccess;

namespace chart
{
namespace impl
{

void ImplApplyDataToModel(
    Reference< frame::XModel > & xInOutModelToChange,
    const Reference< chart2::XInternalDataProvider > & xData )
{
    Reference< chart2::XChartDocument > xDoc( xInOutModelToChange, uno::UNO_QUERY );
    OSL_ASSERT( xDoc.is() && xDoc->hasInternalDataProvider());

    // copy data from stored internal data provider
    if( xDoc.is() && xDoc->hasInternalDataProvider())
    {
        Reference< XAnyDescriptionAccess > xCurrentData( xDoc->getDataProvider(), uno::UNO_QUERY );
        Reference< XAnyDescriptionAccess > xSavedData( xData, uno::UNO_QUERY );
        if( xCurrentData.is() && xSavedData.is())
        {
            xCurrentData->setData( xSavedData->getData());
            xCurrentData->setAnyRowDescriptions( xSavedData->getAnyRowDescriptions());
            xCurrentData->setAnyColumnDescriptions( xSavedData->getAnyColumnDescriptions());
        }
    }
}

// ----------------------------------------

UndoElement::UndoElement(
    const OUString & rActionString,
    const Reference< frame::XModel > & xModel ) :
        m_aActionString( rActionString )
{
    initialize( xModel );
}

UndoElement::UndoElement(
    const Reference< frame::XModel > & xModel )
{
    initialize( xModel );
}

UndoElement::UndoElement( const UndoElement & rOther ) :
        m_aActionString( rOther.m_aActionString )
{
    initialize( rOther.m_xModel );
}

UndoElement::~UndoElement()
{}

void UndoElement::initialize( const Reference< frame::XModel > & xModel )
{
    if ( xModel.is() )
    {
        m_xModel.set( UndoElement::cloneModel( xModel ) );
    }
}

void UndoElement::dispose()
{
    Reference< lang::XComponent > xComp( m_xModel, uno::UNO_QUERY );
    if( xComp.is())
        xComp->dispose();
    m_xModel.set( 0 );
}

void UndoElement::applyToModel(
    Reference< frame::XModel > & xInOutModelToChange )
{
    UndoElement::applyModelContentToModel( xInOutModelToChange, m_xModel );
}

UndoElement * UndoElement::createFromModel(
    const Reference< frame::XModel > & xModel )
{
    return new UndoElement( getActionString(), xModel );
}

void UndoElement::setActionString( const ::rtl::OUString & rActionString )
{
    m_aActionString = rActionString;
}

OUString UndoElement::getActionString() const
{
    return m_aActionString;
}

// static
Reference< frame::XModel > UndoElement::cloneModel( const Reference< frame::XModel > & xModel )
{
    Reference< frame::XModel > xResult;
    uno::Reference< util::XCloneable > xCloneable( xModel, uno::UNO_QUERY );
    OSL_ENSURE( xCloneable.is(), "Cannot clone model" );
    if( xCloneable.is())
        xResult.set( xCloneable->createClone(), uno::UNO_QUERY );

    return xResult;
}

// static
void UndoElement::applyModelContentToModel(
    Reference< frame::XModel > & xInOutModelToChange,
    const Reference< frame::XModel > & xModelToCopyFrom,
    const Reference< chart2::XInternalDataProvider > & xData /* = 0 */ )
{

    if( xModelToCopyFrom.is() && xInOutModelToChange.is())
    {
        try
        {
            // /-- loccked controllers of destination
            ControllerLockGuard aLockedControllers( xInOutModelToChange );
            Reference< chart2::XChartDocument > xSource( xModelToCopyFrom, uno::UNO_QUERY_THROW );
            Reference< chart2::XChartDocument > xDestination( xInOutModelToChange, uno::UNO_QUERY_THROW );

            // propagate the correct flag for plotting of hidden values to the data provider and all used sequences
            ChartModelHelper::setIncludeHiddenCells( ChartModelHelper::isIncludeHiddenCells( xModelToCopyFrom ) , xInOutModelToChange );

            // diagram
            xDestination->setFirstDiagram( xSource->getFirstDiagram());

            // main title
            Reference< chart2::XTitled > xDestinationTitled( xDestination, uno::UNO_QUERY_THROW );
            Reference< chart2::XTitled > xSourceTitled( xSource, uno::UNO_QUERY_THROW );
            xDestinationTitled->setTitleObject( xSourceTitled->getTitleObject());

            // page background
            comphelper::copyProperties(
                xSource->getPageBackground(),
                xDestination->getPageBackground() );

            // apply data (not applied in standard Undo)
            if( xData.is())
                ImplApplyDataToModel( xInOutModelToChange, xData );

            // register all sequences at the internal data provider to get adapted
            // indexes when columns are added/removed
            if( xDestination->hasInternalDataProvider())
            {
                Reference< chart2::XInternalDataProvider > xNewDataProvider( xDestination->getDataProvider(), uno::UNO_QUERY );
                Reference< chart2::data::XDataSource > xUsedData( DataSourceHelper::getUsedData( xInOutModelToChange ));
                if( xUsedData.is() && xNewDataProvider.is())
                {
                    Sequence< Reference< chart2::data::XLabeledDataSequence > > aData( xUsedData->getDataSequences());
                    for( sal_Int32 i=0; i<aData.getLength(); ++i )
                    {
                        xNewDataProvider->registerDataSequenceForChanges( aData[i]->getValues());
                        xNewDataProvider->registerDataSequenceForChanges( aData[i]->getLabel());
                    }
                }
            }

            // restore modify status
            Reference< util::XModifiable > xSourceMod( xSource, uno::UNO_QUERY );
            Reference< util::XModifiable > xDestMod( xDestination, uno::UNO_QUERY );
            if( xSourceMod.is() && xDestMod.is() &&
                ! xSourceMod->isModified() )
            {
                xDestMod->setModified( sal_False );
            }
            // \-- loccked controllers of destination
        }
        catch( uno::Exception & )
        {
        }
    }
}

// ----------------------------------------

UndoElementWithData::UndoElementWithData(
    const OUString & rActionString,
    const Reference< frame::XModel > & xModel ) :
        UndoElement( rActionString, xModel )
{
    initializeData();
}

UndoElementWithData::UndoElementWithData(
    const Reference< frame::XModel > & xModel ) :
        UndoElement( xModel )
{
    initializeData();
}


UndoElementWithData::UndoElementWithData(
    const UndoElementWithData & rOther ) :
        UndoElement( rOther )
{
    initializeData();
}

UndoElementWithData::~UndoElementWithData()
{}

void UndoElementWithData::initializeData()
{
    try
    {
        Reference< chart2::XChartDocument > xChartDoc( m_xModel, uno::UNO_QUERY_THROW );
        OSL_ASSERT( xChartDoc->hasInternalDataProvider());
        if( xChartDoc->hasInternalDataProvider())
        {
            Reference< util::XCloneable > xCloneable( xChartDoc->getDataProvider(), uno::UNO_QUERY );
            OSL_ENSURE( xCloneable.is(), "Cannot clone data" );
            if( xCloneable.is())
                m_xData.set( xCloneable->createClone(), uno::UNO_QUERY );
        }
    }
    catch( uno::Exception & )
    {
    }
}

void UndoElementWithData::dispose()
{
    UndoElement::dispose();
    m_xData.set( 0 );
}

void UndoElementWithData::applyToModel(
    Reference< frame::XModel > & xInOutModelToChange )
{
    UndoElement::applyModelContentToModel( xInOutModelToChange, m_xModel, m_xData );
}

UndoElement * UndoElementWithData::createFromModel(
    const Reference< frame::XModel > & xModel )
{
    return new UndoElementWithData( getActionString(), xModel );
}

// ========================================

// ----------------------------------------

UndoElementWithSelection::UndoElementWithSelection(
    const OUString & rActionString,
    const Reference< frame::XModel > & xModel ) :
        UndoElement( rActionString, xModel )
{
    initialize( xModel );
}

UndoElementWithSelection::UndoElementWithSelection(
    const Reference< frame::XModel > & xModel ) :
        UndoElement( xModel )
{
    initialize( xModel );
}

UndoElementWithSelection::UndoElementWithSelection(
    const UndoElementWithSelection & rOther ) :
        UndoElement( rOther )
{
    initialize( rOther.m_xModel );
}

UndoElementWithSelection::~UndoElementWithSelection()
{}

void UndoElementWithSelection::initialize( const Reference< frame::XModel > & xModel )
{
    try
    {
        uno::Reference< view::XSelectionSupplier > xSelSupp( xModel->getCurrentController(), uno::UNO_QUERY );
        OSL_ASSERT( xSelSupp.is() );

        if( xSelSupp.is() )
            m_aSelection = xSelSupp->getSelection();
    }
    catch( const uno::Exception & )
    {
    }
}

void UndoElementWithSelection::dispose()
{
    UndoElement::dispose();
    m_aSelection.clear();
}

void UndoElementWithSelection::applyToModel(
    Reference< frame::XModel > & xInOutModelToChange )
{
    UndoElement::applyModelContentToModel( xInOutModelToChange, m_xModel );
    Reference< view::XSelectionSupplier > xCurrentSelectionSuppl( xInOutModelToChange->getCurrentController(), uno::UNO_QUERY );
    OSL_ASSERT( xCurrentSelectionSuppl.is() );

    if( xCurrentSelectionSuppl.is())
        xCurrentSelectionSuppl->select( m_aSelection );
}

UndoElement * UndoElementWithSelection::createFromModel(
        const Reference< frame::XModel > & xModel )
{
    return new UndoElementWithSelection( getActionString(), xModel );
}

// ----------------------------------------

ShapeUndoElement::ShapeUndoElement( const OUString& rActionString, SdrUndoAction* pAction )
    :UndoElement( rActionString, Reference< frame::XModel >() )
    ,m_pAction( pAction )
{
}

ShapeUndoElement::ShapeUndoElement( const ShapeUndoElement& rOther )
    :UndoElement( rOther )
    ,m_pAction( rOther.m_pAction )
{
}

ShapeUndoElement::~ShapeUndoElement()
{
}

SdrUndoAction* ShapeUndoElement::getSdrUndoAction()
{
    return m_pAction;
}

// ========================================

UndoStack::UndoStack() :
        m_nSizeLimit( 1000 )
{
}

UndoStack::~UndoStack()
{
    disposeAndClear();
}

void UndoStack::pop()
{
    if( ! empty())
    {
        top()->dispose();
        delete top();
        m_aStack.pop_back();
    }
}

void UndoStack::push( UndoElement * pElement )
{
    m_aStack.push_back( pElement );
    applyLimitation();
}

UndoElement * UndoStack::top() const
{
    return m_aStack.back();
}

OUString UndoStack::topUndoString() const
{
    if( ! empty())
        return top()->getActionString();
    return OUString();
}

Sequence< OUString > UndoStack::getUndoStrings() const
{
    sal_Int32 nSize( static_cast< sal_Int32 >( m_aStack.size()));
    Sequence< OUString > aResult( nSize );
    for( sal_Int32 i=0; i<nSize; ++i )
        aResult[i] = m_aStack[i]->getActionString();
    return aResult;
}

bool UndoStack::empty() const
{
    return m_aStack.empty();
}

void UndoStack::disposeAndClear()
{
    ::std::for_each( m_aStack.begin(), m_aStack.end(), ::boost::mem_fn( & UndoElement::dispose ));
    ::std::for_each( m_aStack.begin(), m_aStack.end(), CommonFunctors::DeletePtr< UndoElement >() );
    m_aStack.clear();
}

void UndoStack::limitSize( sal_Int32 nMaxSize )
{
    m_nSizeLimit = nMaxSize;
    applyLimitation();
}

void UndoStack::applyLimitation()
{
    if( m_aStack.size() > static_cast< sal_uInt32 >( m_nSizeLimit ))
    {
        tUndoStackType::iterator aBegin( m_aStack.begin());
        tUndoStackType::iterator aEnd( aBegin + (m_aStack.size() - m_nSizeLimit));
        // dispose and remove all undo elements that are over the limit
        ::std::for_each( aBegin, aEnd, ::boost::mem_fn( & UndoElement::dispose ));
        ::std::for_each( aBegin, aEnd, CommonFunctors::DeletePtr< UndoElement >() );
        m_aStack.erase( aBegin, aEnd );
    }
}

// ================================================================================

namespace
{
static const OUString aUndoStepsPropName( RTL_CONSTASCII_USTRINGPARAM("Steps"));
} // anonymous namespace

UndoStepsConfigItem::UndoStepsConfigItem( ConfigItemListener & rListener ) :
        ::utl::ConfigItem( OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Common/Undo"))),
    m_rListener( rListener )
{
    EnableNotification( Sequence< OUString >( & aUndoStepsPropName, 1 ));
}

UndoStepsConfigItem::~UndoStepsConfigItem()
{
}

void UndoStepsConfigItem::Notify( const Sequence< OUString > & aPropertyNames )
{
    for( sal_Int32 nIdx=0; nIdx<aPropertyNames.getLength(); ++nIdx )
    {
        if( aPropertyNames[nIdx].equals( aUndoStepsPropName ))
            m_rListener.notify( aPropertyNames[nIdx] );
    }
}

void UndoStepsConfigItem::Commit()
{
}

// mtehod is not const, because GetProperties is not const
sal_Int32 UndoStepsConfigItem::getUndoSteps()
{
    sal_Int32 nSteps = -1;
    Sequence< uno::Any > aValues(
        GetProperties( Sequence< OUString >( & aUndoStepsPropName, 1 )));
    if( aValues.getLength())
        aValues[0] >>= nSteps;
    return nSteps;
}

} // namespace impl
} //  namespace chart
