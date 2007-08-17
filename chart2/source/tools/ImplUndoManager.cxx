/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ImplUndoManager.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-08-17 12:15:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifiable.hpp>

#include <boost/bind.hpp>
#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::com::sun::star::chart::XChartDataArray;

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
        Reference< XChartDataArray > xCurrentData( xDoc->getDataProvider(), uno::UNO_QUERY );
        Reference< XChartDataArray > xSavedData( xData, uno::UNO_QUERY );
        if( xCurrentData.is() && xSavedData.is())
        {
            xCurrentData->setData( xSavedData->getData());
            xCurrentData->setRowDescriptions( xSavedData->getRowDescriptions());
            xCurrentData->setColumnDescriptions( xSavedData->getColumnDescriptions());
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
    m_xModel.set( UndoElement::cloneModel( xModel ));
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

            // diagram
            xDestination->setFirstDiagram( xSource->getFirstDiagram());

            // main title
            Reference< chart2::XTitled > xDestinationTitled( xDestination, uno::UNO_QUERY_THROW );
            Reference< chart2::XTitled > xSourceTitled( xSource, uno::UNO_QUERY_THROW );
            xDestinationTitled->setTitleObject( xSourceTitled->getTitleObject());

            // page background
            PropertyHelper::copyProperties(
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
    catch( const uno::Exception & )
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

sal_Int32 UndoStack::getCurrentLimitation() const
{
    return m_nSizeLimit;
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
