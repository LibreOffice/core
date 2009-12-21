/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: InternalDataProvider.cxx,v $
 * $Revision: 1.7 $
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
#include <rtl/math.hxx>

#include <valarray>

#include "InternalDataProvider.hxx"
#include "LabeledDataSequence.hxx"
#include "DataSource.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "XMLRangeHelper.hxx"
#include "ContainerHelper.hxx"
#include "CommonConverters.hxx"
#include "CommonFunctors.hxx"
#include "UncachedDataSequence.hxx"
#include "DataSourceHelper.hxx"
#include "ChartModelHelper.hxx"
#include "DiagramHelper.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <rtl/ustrbuf.hxx>
#include <unotools/charclass.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include <vector>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::std;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace chart
{

struct OUStringBufferAppend : public unary_function< OUString, void >
{
    OUStringBufferAppend( OUStringBuffer & rBuffer, const OUString & rSeparator ) :
            m_rBuffer( rBuffer ),
            m_aSep( rSeparator )
    {}
    void operator() ( const OUString & rStr )
    {
        m_rBuffer.append( m_aSep );
        m_rBuffer.append( rStr );
    }
private:
    OUStringBuffer m_rBuffer;
    OUString       m_aSep;
};

OUString FlattenStringSequence( const Sequence< OUString > & aSeq )
{
    if( aSeq.getLength() == 0 )
        return OUString();
    OUStringBuffer aBuf( aSeq[0] );
    for_each( aSeq.getConstArray() + 1, aSeq.getConstArray() + aSeq.getLength(),
              OUStringBufferAppend( aBuf, OUString(RTL_CONSTASCII_USTRINGPARAM(" "))));
    return aBuf.makeStringAndClear();
}

// ================================================================================

namespace
{

// note: in xmloff this name is used to indicate usage of own data
static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.InternalDataProvider" ));

static const ::rtl::OUString lcl_aCategoriesRangeName(
    RTL_CONSTASCII_USTRINGPARAM( "categories" ));
static const ::rtl::OUString lcl_aCategoriesRoleName(
    RTL_CONSTASCII_USTRINGPARAM( "categories" ));
static const ::rtl::OUString lcl_aLabelRangePrefix(
    RTL_CONSTASCII_USTRINGPARAM( "label " ));
static const ::rtl::OUString lcl_aCompleteRange(
    RTL_CONSTASCII_USTRINGPARAM( "all" ));


struct lcl_DataProviderRangeCreator : public unary_function< OUString, Reference< chart2::data::XLabeledDataSequence > >
{
    lcl_DataProviderRangeCreator( const Reference< chart2::data::XDataProvider > & xDataProvider ) :
            m_xDataProvider( xDataProvider )
    {}

    Reference< chart2::data::XLabeledDataSequence > operator() ( const OUString & rRange )
    {
        Reference< chart2::data::XLabeledDataSequence > xResult;
        if( m_xDataProvider.is())
            try
            {
                xResult.set( new ::chart::LabeledDataSequence(
                                 m_xDataProvider->createDataSequenceByRangeRepresentation( rRange )));
            }
            catch( const lang::IllegalArgumentException & ex )
            {
                // data provider cannot create single data sequences, but then
                // detectArguments should work also with an empty data source
                (void)(ex);
            }
            catch( const uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        return xResult;
    }

private:
    Reference< chart2::data::XDataProvider > m_xDataProvider;
};

typedef ::std::multimap< OUString, uno::WeakReference< chart2::data::XDataSequence > >
    lcl_tSequenceMap;

struct lcl_modifySeqMapValue : public ::std::unary_function< lcl_tSequenceMap, void >
{
    void operator() ( const lcl_tSequenceMap::value_type & rMapEntry )
    {
        // convert weak reference to reference
        Reference< chart2::data::XDataSequence > xSeq( rMapEntry.second );
        if( xSeq.is())
        {
            Reference< util::XModifiable > xMod( xSeq, uno::UNO_QUERY );
            if( xMod.is())
                xMod->setModified( sal_True );
        }
    }
};

Sequence< Reference< chart2::data::XLabeledDataSequence > >
    lcl_internalizeData(
        const Sequence< Reference< chart2::data::XLabeledDataSequence > > & rDataSeq,
        InternalData & rInternalData,
        InternalDataProvider & rProvider )
{
    Sequence< Reference< chart2::data::XLabeledDataSequence > > aResult( rDataSeq.getLength());
    for( sal_Int32 i=0; i<rDataSeq.getLength(); ++i )
    {
        sal_Int32 nNewIndex( rInternalData.appendColumn());
        OUString aIdentifier( OUString::valueOf( nNewIndex ));
        //@todo: deal also with genericXDataSequence
        Reference< chart2::data::XNumericalDataSequence > xValues( rDataSeq[i]->getValues(), uno::UNO_QUERY );
        Reference< chart2::data::XTextualDataSequence > xLabel( rDataSeq[i]->getLabel(), uno::UNO_QUERY );
        Reference< chart2::data::XDataSequence > xNewValues;

        if( xValues.is())
        {
            ::std::vector< double > aValues( ContainerHelper::SequenceToVector( xValues->getNumericalData()));
            rInternalData.enlargeData( nNewIndex + 1, aValues.size());
            rInternalData.setColumnValues( nNewIndex, aValues );
            xNewValues.set( rProvider.createDataSequenceByRangeRepresentation( aIdentifier ));
            comphelper::copyProperties(
                Reference< beans::XPropertySet >( xValues, uno::UNO_QUERY ),
                Reference< beans::XPropertySet >( xNewValues, uno::UNO_QUERY ));
        }

        if( xLabel.is())
        {
            ::std::vector< OUString > aLabels( rInternalData.getColumnLabels());
            OSL_ASSERT( static_cast< size_t >( nNewIndex ) < aLabels.size());
            if( aLabels.size() <= static_cast< size_t >( nNewIndex ) )
                aLabels.resize( nNewIndex+1 );
            aLabels[nNewIndex] = FlattenStringSequence( xLabel->getTextualData());
            rInternalData.setColumnLabels( aLabels );
            Reference< chart2::data::XDataSequence > xNewLabel(
                rProvider.createDataSequenceByRangeRepresentation( lcl_aLabelRangePrefix + aIdentifier ));
            comphelper::copyProperties(
                Reference< beans::XPropertySet >( xLabel, uno::UNO_QUERY ),
                Reference< beans::XPropertySet >( xNewLabel, uno::UNO_QUERY ));
            aResult[i] =
                Reference< chart2::data::XLabeledDataSequence >(
                    new LabeledDataSequence( xNewValues, xNewLabel ));
        }
        else
        {
            aResult[i] =
                Reference< chart2::data::XLabeledDataSequence >(
                    new LabeledDataSequence( xNewValues ));
        }
    }
    return aResult;
}

struct lcl_internalizeSeries : public ::std::unary_function< Reference< chart2::XDataSeries >, void >
{
    lcl_internalizeSeries( InternalData & rInternalData,
                           InternalDataProvider & rProvider ) :
            m_rInternalData( rInternalData ),
            m_rProvider( rProvider )
    {}
    void operator() ( const Reference< chart2::XDataSeries > & xSeries )
    {
        Reference< chart2::data::XDataSource > xSource( xSeries, uno::UNO_QUERY );
        Reference< chart2::data::XDataSink >   xSink(   xSeries, uno::UNO_QUERY );
        if( xSource.is() && xSink.is())
            xSink->setData( lcl_internalizeData( xSource->getDataSequences(), m_rInternalData, m_rProvider ));
     }

private:
    InternalData &          m_rInternalData;
    InternalDataProvider &  m_rProvider;
};

} // anonymous namespace
InternalDataProvider::InternalDataProvider(const Reference< uno::XComponentContext > & /*_xContext*/) :
        m_bDataInColumns( true )
{}

// ================================================================================

InternalDataProvider::InternalDataProvider() :
        m_bDataInColumns( true )
{}

InternalDataProvider::InternalDataProvider(
    const Reference< ::com::sun::star::chart::XChartDataArray > & xDataToCopy ) :
        m_bDataInColumns( true )
{
    if( xDataToCopy.is())
    {
        setData( xDataToCopy->getData() );
        setColumnDescriptions( xDataToCopy->getColumnDescriptions() );
        setRowDescriptions( xDataToCopy->getRowDescriptions() );
    }
}

InternalDataProvider::InternalDataProvider(
    const Reference< chart2::XChartDocument > & xChartDoc ) :
        m_bDataInColumns( true )
{
    try
    {
        Reference< chart2::XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartDoc ) );
        if( xDiagram.is())
        {
            InternalData & rData( getInternalData() );
            // categories
            Reference< chart2::data::XLabeledDataSequence > xCategories( DiagramHelper::getCategoriesFromDiagram( xDiagram ));
            if( xCategories.is())
            {
                // @todo: be able to deal with XDataSequence, too
                Reference< chart2::data::XTextualDataSequence > xSeq( xCategories->getValues(), uno::UNO_QUERY );
                if( xSeq.is())
                    rData.setRowLabels( ContainerHelper::SequenceToVector( xSeq->getTextualData()));
                DiagramHelper::setCategoriesToDiagram(
                    new LabeledDataSequence(
                        createDataSequenceByRangeRepresentation( lcl_aCategoriesRangeName )),
                    xDiagram );
            }

            // data series
            ::std::vector< Reference< chart2::XDataSeries > > aSeriesVector( ChartModelHelper::getDataSeries( xChartDoc ));
            ::std::for_each( aSeriesVector.begin(), aSeriesVector.end(),
                             lcl_internalizeSeries( rData, *this ));
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// copy-CTOR
InternalDataProvider::InternalDataProvider( const InternalDataProvider & rOther ) :
        impl::InternalDataProvider_Base(),
        m_aSequenceMap( rOther.m_aSequenceMap ),
        m_apData( new InternalData( rOther.getInternalData() ) ),
        m_bDataInColumns( rOther.m_bDataInColumns )
{}

InternalDataProvider::~InternalDataProvider()
{}

void InternalDataProvider::addDataSequenceToMap(
    const OUString & rRangeRepresentation,
    const Reference< chart2::data::XDataSequence > & xSequence )
{
    m_aSequenceMap.insert(
        tSequenceMap::value_type(
            rRangeRepresentation,
            uno::WeakReference< chart2::data::XDataSequence >( xSequence )));
}

void InternalDataProvider::deleteMapReferences( const OUString & rRangeRepresentation )
{
    // set sequence to deleted by setting its range to an empty string
    tSequenceMapRange aRange( m_aSequenceMap.equal_range( rRangeRepresentation ));
    for( tSequenceMap::iterator aIt( aRange.first ); aIt != aRange.second; ++aIt )
    {
        Reference< chart2::data::XDataSequence > xSeq( aIt->second );
        if( xSeq.is())
        {
            Reference< container::XNamed > xNamed( xSeq, uno::UNO_QUERY );
            if( xNamed.is())
                xNamed->setName( OUString());
        }
    }
    // remove from map
    m_aSequenceMap.erase( aRange.first, aRange.second );
}

void InternalDataProvider::adaptMapReferences(
    const OUString & rOldRangeRepresentation,
    const OUString & rNewRangeRepresentation )
{
    tSequenceMapRange aRange( m_aSequenceMap.equal_range( rOldRangeRepresentation ));
    tSequenceMap aNewElements;
    for( tSequenceMap::iterator aIt( aRange.first ); aIt != aRange.second; ++aIt )
    {
        Reference< chart2::data::XDataSequence > xSeq( aIt->second );
        if( xSeq.is())
        {
            Reference< container::XNamed > xNamed( xSeq, uno::UNO_QUERY );
            if( xNamed.is())
                xNamed->setName( rNewRangeRepresentation );
        }
        aNewElements.insert( tSequenceMap::value_type( rNewRangeRepresentation, aIt->second ));
    }
    // erase map values for old index
    m_aSequenceMap.erase( aRange.first, aRange.second );
    // add new entries for values with new index
    ::std::copy( aNewElements.begin(), aNewElements.end(),
                 ::std::inserter( m_aSequenceMap,
                                  m_aSequenceMap.upper_bound( rNewRangeRepresentation )));
}

void InternalDataProvider::increaseMapReferences(
    sal_Int32 nBegin, sal_Int32 nEnd )
{
    for( sal_Int32 nIndex = nEnd - 1; nIndex >= nBegin; --nIndex )
    {
        adaptMapReferences( OUString::valueOf( nIndex ),
                            OUString::valueOf( nIndex + 1 ));
        adaptMapReferences( lcl_aLabelRangePrefix + OUString::valueOf( nIndex ),
                            lcl_aLabelRangePrefix + OUString::valueOf( nIndex + 1 ));
    }
}

void InternalDataProvider::decreaseMapReferences(
    sal_Int32 nBegin, sal_Int32 nEnd )
{
    for( sal_Int32 nIndex = nBegin; nIndex < nEnd; ++nIndex )
    {
        adaptMapReferences( OUString::valueOf( nIndex ),
                            OUString::valueOf( nIndex - 1 ));
        adaptMapReferences( lcl_aLabelRangePrefix + OUString::valueOf( nIndex ),
                            lcl_aLabelRangePrefix + OUString::valueOf( nIndex - 1 ));
    }
}

Reference< chart2::data::XDataSequence > InternalDataProvider::createDataSequenceAndAddToMap(
    const OUString & rRangeRepresentation )
{
    Reference< chart2::data::XDataSequence > xSeq(
        new UncachedDataSequence( this, rRangeRepresentation ));
    addDataSequenceToMap( rRangeRepresentation, xSeq );
    return xSeq;
}

Reference< chart2::data::XDataSequence > InternalDataProvider::createDataSequenceAndAddToMap(
    const OUString & rRangeRepresentation,
    const OUString & rRole )
{
    Reference< chart2::data::XDataSequence > xSeq(
        new UncachedDataSequence( this, rRangeRepresentation, rRole ));
    addDataSequenceToMap( rRangeRepresentation, xSeq );
    return xSeq;
}

const InternalData & InternalDataProvider::getInternalData() const
{
    if( m_apData.get())
        return *(m_apData.get());

    m_apData.reset( new InternalData() );
    return *(m_apData.get());
}

InternalData & InternalDataProvider::getInternalData()
{
    if( m_apData.get())
        return *(m_apData.get());

    m_apData.reset( new InternalData());
    return *(m_apData.get());
}

void InternalDataProvider::createDefaultData()
{
    getInternalData().createDefaultData();
}

// ____ XDataProvider ____
::sal_Bool SAL_CALL InternalDataProvider::createDataSourcePossible( const Sequence< beans::PropertyValue >& /* aArguments */ )
    throw (uno::RuntimeException)
{
    return true;
}

Reference< chart2::data::XDataSource > SAL_CALL InternalDataProvider::createDataSource(
    const Sequence< beans::PropertyValue >& aArguments )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    OUString aRangeRepresentation;
    bool bUseColumns = true;
    bool bFirstCellAsLabel = true;
    bool bHasCategories = true;
    uno::Sequence< sal_Int32 > aSequenceMapping;
    DataSourceHelper::readArguments( aArguments, aRangeRepresentation, aSequenceMapping, bUseColumns, bFirstCellAsLabel, bHasCategories );

    OSL_ASSERT( aRangeRepresentation.equals( lcl_aCompleteRange ));

    ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aResultLSeqVec;
    InternalData & rData( getInternalData());

    // categories
    if ( bHasCategories )
        aResultLSeqVec.push_back(
            new LabeledDataSequence( createDataSequenceAndAddToMap( lcl_aCategoriesRangeName, lcl_aCategoriesRoleName )));

    // data with labels
    ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aDataVec;
    const sal_Int32 nCount = (bUseColumns ? rData.getColumnCount() : rData.getRowCount());
    for( sal_Int32 nIdx=0; nIdx<nCount; ++nIdx )
    {
        aDataVec.push_back(
            new LabeledDataSequence(
                createDataSequenceAndAddToMap( OUString::valueOf( nIdx )),
                createDataSequenceAndAddToMap( lcl_aLabelRangePrefix + OUString::valueOf( nIdx ))));
    }

    // attention: this data provider has the limitation that it stores
    // internally if data comes from columns or rows. It is intended for
    // creating only one used data source.
    // @todo: add this information in the range representation strings
    m_bDataInColumns = bUseColumns;

    //reorder labeled sequences according to aSequenceMapping; ignore categories
    for( sal_Int32 nNewIndex = 0; nNewIndex < aSequenceMapping.getLength(); nNewIndex++ )
    {
        std::vector< LabeledDataSequence* >::size_type nOldIndex = aSequenceMapping[nNewIndex];
        if( nOldIndex < aDataVec.size() )
        {
            if( aDataVec[nOldIndex].is() )
            {
                aResultLSeqVec.push_back( aDataVec[nOldIndex] );
                aDataVec[nOldIndex] = 0;
            }
        }
    }

    //add left over data sequences to result
    ::std::vector< Reference< chart2::data::XLabeledDataSequence > >::iterator aIt(aDataVec.begin());
    const ::std::vector< Reference< chart2::data::XLabeledDataSequence > >::const_iterator aEndIt(aDataVec.end());
    for( ;aIt!=aEndIt; ++aIt)
    {
        if( aIt->is() )
            aResultLSeqVec.push_back( *aIt );
    }

    return new DataSource( ContainerHelper::ContainerToSequence(aResultLSeqVec) );
}

Sequence< beans::PropertyValue > SAL_CALL InternalDataProvider::detectArguments(
    const Reference< chart2::data::XDataSource >& /* xDataSource */ )
    throw (uno::RuntimeException)
{
    Sequence< beans::PropertyValue > aArguments( 4 );
    aArguments[0] = beans::PropertyValue(
        C2U("CellRangeRepresentation"), -1, uno::makeAny( lcl_aCompleteRange ),
        beans::PropertyState_DIRECT_VALUE );
    aArguments[1] = beans::PropertyValue(
        C2U("DataRowSource"), -1, uno::makeAny(
            m_bDataInColumns
            ? ::com::sun::star::chart::ChartDataRowSource_COLUMNS
            : ::com::sun::star::chart::ChartDataRowSource_ROWS ),
        beans::PropertyState_DIRECT_VALUE );
    // internal data always contains labels and categories
    aArguments[2] = beans::PropertyValue(
        C2U("FirstCellAsLabel"), -1, uno::makeAny( true ), beans::PropertyState_DIRECT_VALUE );
    aArguments[3] = beans::PropertyValue(
        C2U("HasCategories"), -1, uno::makeAny( true ), beans::PropertyState_DIRECT_VALUE );

    // #i85913# Sequence Mapping is not needed for internal data, as it is
    // applied to the data when the data source is created.

    return aArguments;
}

::sal_Bool SAL_CALL InternalDataProvider::createDataSequenceByRangeRepresentationPossible( const OUString& /* aRangeRepresentation */ )
    throw (uno::RuntimeException)
{
    return true;
}

Reference< chart2::data::XDataSequence > SAL_CALL InternalDataProvider::createDataSequenceByRangeRepresentation(
    const OUString& aRangeRepresentation )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    if( aRangeRepresentation.equals( lcl_aCategoriesRangeName ))
    {
        // categories
        return createDataSequenceAndAddToMap( lcl_aCategoriesRangeName, lcl_aCategoriesRoleName );
    }
    else if( aRangeRepresentation.match( lcl_aLabelRangePrefix ))
    {
        // label
        sal_Int32 nIndex = aRangeRepresentation.copy( lcl_aLabelRangePrefix.getLength()).toInt32();
        return createDataSequenceAndAddToMap( lcl_aLabelRangePrefix + OUString::valueOf( nIndex ));
    }
    else if( aRangeRepresentation.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "last" )))
    {
        sal_Int32 nIndex = (m_bDataInColumns
                            ? getInternalData().getColumnCount()
                            : getInternalData().getRowCount()) - 1;
        return createDataSequenceAndAddToMap( OUString::valueOf( nIndex ));
    }
    else if( aRangeRepresentation.getLength())
    {
        // data
        sal_Int32 nIndex = aRangeRepresentation.toInt32();
        return createDataSequenceAndAddToMap( OUString::valueOf( nIndex ));
    }

    return Reference< chart2::data::XDataSequence >();
}

Reference< sheet::XRangeSelection > SAL_CALL InternalDataProvider::getRangeSelection()
    throw (uno::RuntimeException)
{
    // there is no range selection component
    return Reference< sheet::XRangeSelection >();
}

// ____ XInternalDataProvider ____
::sal_Bool SAL_CALL InternalDataProvider::hasDataByRangeRepresentation( const OUString& aRange )
    throw (uno::RuntimeException)
{
    sal_Bool bResult = false;
    const InternalData & rData( getInternalData());

    if( aRange.equals( lcl_aCategoriesRangeName ))
    {
        bResult = true;
    }
    else if( aRange.match( lcl_aLabelRangePrefix ))
    {
        sal_Int32 nIndex = aRange.copy( lcl_aLabelRangePrefix.getLength()).toInt32();
        bResult = (nIndex < (m_bDataInColumns ? rData.getColumnCount(): rData.getRowCount()));
    }
    else
    {
        sal_Int32 nIndex = aRange.toInt32();
        bResult = (nIndex < (m_bDataInColumns ? rData.getColumnCount(): rData.getRowCount()));
    }

    return bResult;
}

Sequence< uno::Any > SAL_CALL InternalDataProvider::getDataByRangeRepresentation( const OUString& aRange )
    throw (uno::RuntimeException)
{
    Sequence< uno::Any > aResult;
    const InternalData & rData( getInternalData());

    if( aRange.equals( lcl_aCategoriesRangeName ))
    {
        vector< OUString > aCategories( m_bDataInColumns ? rData.getRowLabels() : rData.getColumnLabels());
        aResult.realloc( aCategories.size());
        transform( aCategories.begin(), aCategories.end(),
                   aResult.getArray(), CommonFunctors::makeAny< OUString >());
    }
    else if( aRange.match( lcl_aLabelRangePrefix ))
    {
        sal_Int32 nIndex = aRange.copy( lcl_aLabelRangePrefix.getLength()).toInt32();
        vector< OUString > aLabels( m_bDataInColumns ? rData.getColumnLabels() : rData.getRowLabels());
        if( nIndex < static_cast< sal_Int32 >( aLabels.size()))
        {
            aResult.realloc( 1 );
            aResult[0] = uno::makeAny( aLabels[ nIndex ] );
        }
    }
    else
    {
        sal_Int32 nIndex = aRange.toInt32();
        if( nIndex >= 0 )
        {
            Sequence< double > aData;
            if( m_bDataInColumns )
                aData = rData.getColumnValues(nIndex);
            else
                aData = rData.getRowValues(nIndex);
            if( aData.getLength() )
            {
                aResult.realloc( aData.getLength());
                transform( aData.getConstArray(), aData.getConstArray() + aData.getLength(),
                           aResult.getArray(), CommonFunctors::makeAny< double >());
            }
        }
    }

    return aResult;
}

void SAL_CALL InternalDataProvider::setDataByRangeRepresentation(
    const OUString& aRange, const Sequence< uno::Any >& aNewData )
    throw (uno::RuntimeException)
{
    InternalData & rData( getInternalData());

    if( aRange.equals( lcl_aCategoriesRangeName ))
    {
        vector< OUString > aCategories;
        transform( aNewData.getConstArray(), aNewData.getConstArray() + aNewData.getLength(),
                   back_inserter( aCategories ), CommonFunctors::AnyToString());

        if( m_bDataInColumns )
            rData.setRowLabels( aCategories );
        else
            rData.setColumnLabels( aCategories );
    }
    else if( aRange.match( lcl_aLabelRangePrefix ))
    {
        sal_uInt32 nIndex = aRange.copy( lcl_aLabelRangePrefix.getLength()).toInt32();
        OUString aNewLabel;
        if( aNewData.getLength() &&
            (aNewData[0] >>= aNewLabel))
        {
            if( m_bDataInColumns )
            {
                vector< OUString > aLabels( rData.getColumnLabels());
                if ( aLabels.size() <= nIndex )
                    aLabels.push_back(aNewLabel);
                else
                    aLabels[ nIndex ] = aNewLabel;
                rData.setColumnLabels( aLabels );
            }
            else
            {
                vector< OUString > aLabels( rData.getRowLabels());
                if ( aLabels.size() <= nIndex )
                    aLabels.push_back(aNewLabel);
                else
                    aLabels[ nIndex ] = aNewLabel;
                rData.setRowLabels( aLabels );
            }
        }
    }
    else
    {
        sal_Int32 nIndex = aRange.toInt32();
        if( nIndex>=0 )
        {
            vector< double > aNewDataVec;
            transform( aNewData.getConstArray(), aNewData.getConstArray() + aNewData.getLength(),
                       back_inserter( aNewDataVec ), CommonFunctors::AnyToDouble());

            // ensure that the data is large enough
            if( m_bDataInColumns )
            {
                rData.enlargeData( nIndex, 0 );
                rData.setColumnValues( nIndex, aNewDataVec );
            }
            else
            {
                rData.enlargeData( 0, nIndex );
                rData.setRowValues( nIndex, aNewDataVec );
            }
        }
    }
}

void SAL_CALL InternalDataProvider::insertSequence( ::sal_Int32 nAfterIndex )
    throw (uno::RuntimeException)
{
    if( m_bDataInColumns )
    {
        increaseMapReferences( nAfterIndex + 1, getInternalData().getColumnCount());
        getInternalData().insertColumn( nAfterIndex );
    }
    else
    {
        increaseMapReferences( nAfterIndex + 1, getInternalData().getRowCount());
        getInternalData().insertRow( nAfterIndex );
    }
}

void SAL_CALL InternalDataProvider::deleteSequence( ::sal_Int32 nAtIndex )
    throw (uno::RuntimeException)
{
    deleteMapReferences( OUString::valueOf( nAtIndex ));
    deleteMapReferences( lcl_aLabelRangePrefix + OUString::valueOf( nAtIndex ));
    if( m_bDataInColumns )
    {
        decreaseMapReferences( nAtIndex + 1, getInternalData().getColumnCount());
        getInternalData().deleteColumn( nAtIndex );
    }
    else
    {
        decreaseMapReferences( nAtIndex + 1, getInternalData().getRowCount());
        getInternalData().deleteRow( nAtIndex );
    }
}

void SAL_CALL InternalDataProvider::appendSequence()
    throw (uno::RuntimeException)
{
    if( m_bDataInColumns )
        getInternalData().appendColumn();
    else
        getInternalData().appendRow();
}

void SAL_CALL InternalDataProvider::insertDataPointForAllSequences( ::sal_Int32 nAfterIndex )
    throw (uno::RuntimeException)
{
    sal_Int32 nMaxRep = 0;
    if( m_bDataInColumns )
    {
        getInternalData().insertRow( nAfterIndex );
        nMaxRep = getInternalData().getColumnCount();
    }
    else
    {
        getInternalData().insertColumn( nAfterIndex );
        nMaxRep = getInternalData().getRowCount();
    }

    // notify change to all affected ranges
    tSequenceMap::const_iterator aBegin( m_aSequenceMap.lower_bound( C2U("0")));
    tSequenceMap::const_iterator aEnd( m_aSequenceMap.upper_bound( OUString::valueOf( nMaxRep )));
    ::std::for_each( aBegin, aEnd, lcl_modifySeqMapValue());

    tSequenceMapRange aRange( m_aSequenceMap.equal_range( lcl_aCategoriesRangeName ));
    ::std::for_each( aRange.first, aRange.second, lcl_modifySeqMapValue());
}

void SAL_CALL InternalDataProvider::deleteDataPointForAllSequences( ::sal_Int32 nAtIndex )
    throw (uno::RuntimeException)
{
    sal_Int32 nMaxRep = 0;
    if( m_bDataInColumns )
    {
        getInternalData().deleteRow( nAtIndex );
        nMaxRep = getInternalData().getColumnCount();
    }
    else
    {
        getInternalData().deleteColumn( nAtIndex );
        nMaxRep = getInternalData().getRowCount();
    }

    // notify change to all affected ranges
    tSequenceMap::const_iterator aBegin( m_aSequenceMap.lower_bound( C2U("0")));
    tSequenceMap::const_iterator aEnd( m_aSequenceMap.upper_bound( OUString::valueOf( nMaxRep )));
    ::std::for_each( aBegin, aEnd, lcl_modifySeqMapValue());

    tSequenceMapRange aRange( m_aSequenceMap.equal_range( lcl_aCategoriesRangeName ));
    ::std::for_each( aRange.first, aRange.second, lcl_modifySeqMapValue());
}

void SAL_CALL InternalDataProvider::swapDataPointWithNextOneForAllSequences( ::sal_Int32 nAtIndex )
    throw (uno::RuntimeException)
{
    if( m_bDataInColumns )
        getInternalData().swapRowWithNext( nAtIndex );
    else
        getInternalData().swapColumnWithNext( nAtIndex );
    sal_Int32 nMaxRep = (m_bDataInColumns
                         ? getInternalData().getColumnCount()
                         : getInternalData().getRowCount());

    // notify change to all affected ranges
    tSequenceMap::const_iterator aBegin( m_aSequenceMap.lower_bound( C2U("0")));
    tSequenceMap::const_iterator aEnd( m_aSequenceMap.upper_bound( OUString::valueOf( nMaxRep )));
    ::std::for_each( aBegin, aEnd, lcl_modifySeqMapValue());

    tSequenceMapRange aRange( m_aSequenceMap.equal_range( lcl_aCategoriesRangeName ));
    ::std::for_each( aRange.first, aRange.second, lcl_modifySeqMapValue());
}

void SAL_CALL InternalDataProvider::registerDataSequenceForChanges( const Reference< chart2::data::XDataSequence >& xSeq )
    throw (uno::RuntimeException)
{
    if( xSeq.is())
        addDataSequenceToMap( xSeq->getSourceRangeRepresentation(), xSeq );
}


// ____ XRangeXMLConversion ____
OUString SAL_CALL InternalDataProvider::convertRangeToXML( const OUString& aRangeRepresentation )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    XMLRangeHelper::CellRange aRange;
    aRange.aTableName = OUString(RTL_CONSTASCII_USTRINGPARAM("local-table"));
    InternalData & rData( getInternalData());

    // attention: this data provider has the limitation that it stores
    // internally if data comes from columns or rows. It is intended for
    // creating only one used data source.
    // @todo: add this information in the range representation strings
    if( aRangeRepresentation.equals( lcl_aCategoriesRangeName ))
    {
        aRange.aUpperLeft.bIsEmpty = false;
        if( m_bDataInColumns )
        {
            aRange.aUpperLeft.nColumn = 0;
            aRange.aUpperLeft.nRow = 1;
            aRange.aLowerRight = aRange.aUpperLeft;
            aRange.aLowerRight.nRow = rData.getRowCount();
        }
        else
        {
            aRange.aUpperLeft.nColumn = 1;
            aRange.aUpperLeft.nRow = 0;
            aRange.aLowerRight = aRange.aUpperLeft;
            aRange.aLowerRight.nColumn = rData.getColumnCount();
        }
    }
    else if( aRangeRepresentation.match( lcl_aLabelRangePrefix ))
    {
        sal_Int32 nIndex = aRangeRepresentation.copy( lcl_aLabelRangePrefix.getLength()).toInt32();
        aRange.aUpperLeft.bIsEmpty = false;
        aRange.aLowerRight.bIsEmpty = true;
        if( m_bDataInColumns )
        {
            aRange.aUpperLeft.nColumn = nIndex + 1;
            aRange.aUpperLeft.nRow = 0;
        }
        else
        {
            aRange.aUpperLeft.nColumn = 0;
            aRange.aUpperLeft.nRow = nIndex + 1;
        }
    }
    else if( aRangeRepresentation.equals( lcl_aCompleteRange ))
    {
        aRange.aUpperLeft.bIsEmpty = false;
        aRange.aLowerRight.bIsEmpty = false;
        aRange.aUpperLeft.nColumn = 0;
        aRange.aUpperLeft.nRow = 0;
        aRange.aLowerRight.nColumn = rData.getColumnCount();
        aRange.aLowerRight.nRow = rData.getRowCount();
    }
    else
    {
        sal_Int32 nIndex = aRangeRepresentation.toInt32();
        aRange.aUpperLeft.bIsEmpty = false;
        if( m_bDataInColumns )
        {
            aRange.aUpperLeft.nColumn = nIndex + 1;
            aRange.aUpperLeft.nRow = 1;
            aRange.aLowerRight = aRange.aUpperLeft;
            aRange.aLowerRight.nRow = rData.getRowCount();
        }
        else
        {
            aRange.aUpperLeft.nColumn = 1;
            aRange.aUpperLeft.nRow = nIndex + 1;
            aRange.aLowerRight = aRange.aUpperLeft;
            aRange.aLowerRight.nColumn = rData.getColumnCount();
        }
    }

    return XMLRangeHelper::getXMLStringFromCellRange( aRange );
}

OUString SAL_CALL InternalDataProvider::convertRangeFromXML( const OUString& aXMLRange )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    XMLRangeHelper::CellRange aRange( XMLRangeHelper::getCellRangeFromXMLString( aXMLRange ));
    if( aRange.aUpperLeft.bIsEmpty )
    {
        OSL_ENSURE( aRange.aLowerRight.bIsEmpty, "Weird Range" );
        return OUString();
    }

    // "all"
    if( !aRange.aLowerRight.bIsEmpty &&
        ( aRange.aUpperLeft.nColumn != aRange.aLowerRight.nColumn ) &&
        ( aRange.aUpperLeft.nRow != aRange.aLowerRight.nRow ) )
        return lcl_aCompleteRange;

    // attention: this data provider has the limitation that it stores
    // internally if data comes from columns or rows. It is intended for
    // creating only one used data source.
    // @todo: add this information in the range representation strings

    // data in columns
    if( m_bDataInColumns )
    {
        if( aRange.aUpperLeft.nColumn == 0 )
            return lcl_aCategoriesRangeName;
        if( aRange.aUpperLeft.nRow == 0 )
            return lcl_aLabelRangePrefix + OUString::valueOf( aRange.aUpperLeft.nColumn - 1 );

        return OUString::valueOf( aRange.aUpperLeft.nColumn - 1 );
    }

    // data in rows
    if( aRange.aUpperLeft.nRow == 0 )
        return lcl_aCategoriesRangeName;
    if( aRange.aUpperLeft.nColumn == 0 )
        return lcl_aLabelRangePrefix + OUString::valueOf( aRange.aUpperLeft.nRow - 1 );

    return OUString::valueOf( aRange.aUpperLeft.nRow - 1 );
}

// ____ XChartDataArray ____
// note: do not use m_bDataInColumns for all XChartDataArray-specific code
// the chart-API assumes data is always in rows
Sequence< Sequence< double > > SAL_CALL InternalDataProvider::getData()
    throw (uno::RuntimeException)
{
    return getInternalData().getData();
}

void SAL_CALL InternalDataProvider::setData( const Sequence< Sequence< double > >& rDataInRows )
    throw (uno::RuntimeException)
{
    return getInternalData().setData( rDataInRows );
}

Sequence< OUString > SAL_CALL InternalDataProvider::getRowDescriptions()
    throw (uno::RuntimeException)
{
    return ContainerHelper::ContainerToSequence( getInternalData().getRowLabels());
}

void SAL_CALL InternalDataProvider::setRowDescriptions( const Sequence< OUString >& aRowDescriptions )
    throw (uno::RuntimeException)
{
    getInternalData().setRowLabels( ContainerHelper::SequenceToVector( aRowDescriptions ));
}

Sequence< OUString > SAL_CALL InternalDataProvider::getColumnDescriptions()
    throw (uno::RuntimeException)
{
    return ContainerHelper::ContainerToSequence( getInternalData().getColumnLabels());
}

void SAL_CALL InternalDataProvider::setColumnDescriptions( const Sequence< OUString >& aColumnDescriptions )
    throw (uno::RuntimeException)
{
    getInternalData().setColumnLabels( ContainerHelper::SequenceToVector( aColumnDescriptions ));
}


// ____ XChartData (base of XChartDataArray) ____
void SAL_CALL InternalDataProvider::addChartDataChangeEventListener(
    const Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& )
    throw (uno::RuntimeException)
{
}

void SAL_CALL InternalDataProvider::removeChartDataChangeEventListener(
    const Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& )
    throw (uno::RuntimeException)
{
}

double SAL_CALL InternalDataProvider::getNotANumber()
    throw (uno::RuntimeException)
{
    double fNan;
    ::rtl::math::setNan( & fNan );
    return fNan;
}

::sal_Bool SAL_CALL InternalDataProvider::isNotANumber( double nNumber )
    throw (uno::RuntimeException)
{
    return ::rtl::math::isNan( nNumber )
        || ::rtl::math::isInf( nNumber );
}
// lang::XInitialization:
void SAL_CALL InternalDataProvider::initialize(const uno::Sequence< uno::Any > & _aArguments) throw (uno::RuntimeException, uno::Exception)
{
    comphelper::SequenceAsHashMap aArgs(_aArguments);
    if ( aArgs.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CreateDefaultData")),sal_False) )
        createDefaultData();
}
// ____ XCloneable ____
Reference< util::XCloneable > SAL_CALL InternalDataProvider::createClone()
    throw (uno::RuntimeException)
{
    return Reference< util::XCloneable >( new InternalDataProvider( *this ));
}


// ================================================================================

Sequence< OUString > InternalDataProvider::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 1 );
    aServices[ 0 ] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.data.DataProvider" ));
    return aServices;
}

// ================================================================================

APPHELPER_XSERVICEINFO_IMPL( InternalDataProvider, lcl_aServiceName );

} //  namespace chart
