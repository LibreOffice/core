/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataSourceHelper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 17:33:21 $
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

#include "DataSourceHelper.hxx"
#include "macros.hxx"
#include "ChartModelHelper.hxx"
#include "DiagramHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "DataSource.hxx"
#include "ContainerHelper.hxx"
#include "ControllerLockGuard.hxx"
#include "PropertyHelper.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>

#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace
{
void lcl_addRanges( ::std::vector< ::rtl::OUString > & rOutResult,
                    const uno::Reference< data::XLabeledDataSequence > & xLabeledSeq )
{
    if( ! xLabeledSeq.is())
        return;
    uno::Reference< data::XDataSequence > xSeq( xLabeledSeq->getLabel());
    if( xSeq.is())
        rOutResult.push_back( xSeq->getSourceRangeRepresentation());
    xSeq.set( xLabeledSeq->getValues());
    if( xSeq.is())
        rOutResult.push_back( xSeq->getSourceRangeRepresentation());
}

void lcl_addDataSourceRanges(
    ::std::vector< ::rtl::OUString > & rOutResult,
    const uno::Reference< data::XDataSource > & xDataSource )
{
    if( xDataSource.is() )
    {
        uno::Sequence< uno::Reference< data::XLabeledDataSequence > > aDataSequences( xDataSource->getDataSequences() );
        for( sal_Int32 i=0; i<aDataSequences.getLength(); ++i)
            lcl_addRanges( rOutResult, aDataSequences[i] );
    }
}

void lcl_addErrorBarRanges(
    ::std::vector< ::rtl::OUString > & rOutResult,
    const uno::Reference< XDataSeries > & xDataSeries )
{
    uno::Reference< beans::XPropertySet > xSeriesProp( xDataSeries, uno::UNO_QUERY );
    if( !xSeriesProp.is())
        return;

    try
    {
        uno::Reference< beans::XPropertySet > xErrorBarProp;
        if( ( xSeriesProp->getPropertyValue( C2U("ErrorBarY")) >>= xErrorBarProp ) &&
            xErrorBarProp.is())
        {
            sal_Int32 eStyle = ::com::sun::star::chart::ErrorBarStyle::NONE;
            if( ( xErrorBarProp->getPropertyValue( C2U("ErrorBarStyle")) >>= eStyle ) &&
                eStyle == ::com::sun::star::chart::ErrorBarStyle::FROM_DATA )
            {
                uno::Reference< data::XDataSource > xErrorBarDataSource( xErrorBarProp, uno::UNO_QUERY );
                if( xErrorBarDataSource.is() )
                    lcl_addDataSourceRanges( rOutResult, xErrorBarDataSource );
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

struct lcl_migrateData : public ::std::unary_function<
        Reference< chart2::data::XLabeledDataSequence >, void >
{
    lcl_migrateData( const Reference< chart2::data::XDataProvider > & xDataProvider ) :
            m_xDataProvider( xDataProvider )
    {}

    Reference< chart2::data::XDataSequence > migrateData(
        const Reference< chart2::data::XDataSequence > & xOldData )
    {
        if( ! xOldData.is())
            return xOldData;

        OSL_ASSERT( m_xDataProvider.is());
        Reference< chart2::data::XDataSequence > xNewData(
            m_xDataProvider->createDataSequenceByRangeRepresentation(
                xOldData->getSourceRangeRepresentation()));
        PropertyHelper::copyProperties(
            Reference< beans::XPropertySet >( xOldData, uno::UNO_QUERY ),
            Reference< beans::XPropertySet >( xNewData, uno::UNO_QUERY ));
        return xNewData;
    }

    void operator()( const Reference< chart2::data::XLabeledDataSequence > & xLSeq )
    {
        if( m_xDataProvider.is() && xLSeq.is())
        {
            xLSeq->setValues( migrateData( xLSeq->getValues()));
            xLSeq->setLabel( migrateData( xLSeq->getLabel()));
        }
    }

private:
    Reference< chart2::data::XDataProvider > m_xDataProvider;
};


} // anonymous namespace

uno::Sequence< beans::PropertyValue > DataSourceHelper::createArguments(
                                            bool bUseColumns, bool bFirstCellAsLabel, bool bHasCategories )
{
    ::com::sun::star::chart::ChartDataRowSource eRowSource = ::com::sun::star::chart::ChartDataRowSource_ROWS;
    if( bUseColumns )
        eRowSource = ::com::sun::star::chart::ChartDataRowSource_COLUMNS;

    uno::Sequence< beans::PropertyValue > aArguments(3);
    aArguments[0] = beans::PropertyValue( C2U("DataRowSource")
        , -1, uno::makeAny( eRowSource )
        , beans::PropertyState_DIRECT_VALUE );
    aArguments[1] = beans::PropertyValue( C2U("FirstCellAsLabel")
        , -1, uno::makeAny( bFirstCellAsLabel )
        , beans::PropertyState_DIRECT_VALUE );
    aArguments[2] = beans::PropertyValue( C2U("HasCategories")
        , -1, uno::makeAny( bHasCategories )
        , beans::PropertyState_DIRECT_VALUE );

    return aArguments;
}

uno::Sequence< beans::PropertyValue > DataSourceHelper::createArguments(
                                            const ::rtl::OUString & rRangeRepresentation,
                                            const uno::Sequence< sal_Int32 >& rSequenceMapping,
                                            bool bUseColumns, bool bFirstCellAsLabel, bool bHasCategories )
{
    uno::Sequence< beans::PropertyValue > aArguments( createArguments( bUseColumns, bFirstCellAsLabel, bHasCategories ));
    aArguments.realloc( aArguments.getLength() + 1 );
    aArguments[aArguments.getLength() - 1] =
        beans::PropertyValue( C2U("CellRangeRepresentation")
                              , -1, uno::makeAny( rRangeRepresentation )
                              , beans::PropertyState_DIRECT_VALUE );
    if( rSequenceMapping.getLength() )
    {
        aArguments.realloc( aArguments.getLength() + 1 );
        aArguments[aArguments.getLength() - 1] =
            beans::PropertyValue( C2U("SequenceMapping")
                                , -1, uno::makeAny( rSequenceMapping )
                                , beans::PropertyState_DIRECT_VALUE );
    }
    return aArguments;
}

void DataSourceHelper::readArguments( const uno::Sequence< beans::PropertyValue >& rArguments
                                     , ::rtl::OUString & rRangeRepresentation, uno::Sequence< sal_Int32 >& rSequenceMapping
            , bool& bUseColumns, bool& bFirstCellAsLabel, bool& bHasCategories )
{
    const beans::PropertyValue* pArguments = rArguments.getConstArray();
    for(sal_Int32 i=0; i<rArguments.getLength(); ++i, ++pArguments)
    {
        const beans::PropertyValue& aProperty = *pArguments;
        if( aProperty.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "DataRowSource" ) ))
        {
            ::com::sun::star::chart::ChartDataRowSource eRowSource;
            if( aProperty.Value >>= eRowSource )
                bUseColumns = (eRowSource==::com::sun::star::chart::ChartDataRowSource_COLUMNS);
        }
        else if( aProperty.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FirstCellAsLabel" ) ))
        {
            aProperty.Value >>= bFirstCellAsLabel;
        }
        else if( aProperty.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "HasCategories" ) ))
        {
            aProperty.Value >>= bHasCategories;
        }
        else if( aProperty.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "CellRangeRepresentation" ) ))
        {
            aProperty.Value >>= rRangeRepresentation;
        }
        else if( aProperty.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "SequenceMapping" ) ))
        {
            aProperty.Value >>= rSequenceMapping;
        }
    }
}

uno::Reference< chart2::data::XDataSource > DataSourceHelper::pressUsedDataIntoRectangularFormat(
        const uno::Reference< chart2::XChartDocument >& xChartDoc, bool bWithCategories )
{
    ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aResultVector;

    //categories are always the first sequence
    Reference< chart2::XDiagram > xDiagram( xChartDoc->getFirstDiagram());

    if( bWithCategories )
    {
        Reference< chart2::data::XLabeledDataSequence > xCategories( DiagramHelper::getCategoriesFromDiagram( xDiagram ) );
        if( xCategories.is() )
            aResultVector.push_back( xCategories );
    }

    ::std::vector< Reference< chart2::XDataSeries > > xSeriesVector( DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );
    uno::Reference< chart2::data::XDataSource > xSeriesSource(
        DataSeriesHelper::getDataSource( ContainerHelper::ContainerToSequence(xSeriesVector) ) );
    Sequence< Reference< chart2::data::XLabeledDataSequence > > aDataSeqences( xSeriesSource->getDataSequences() );

    //the first x-values is always the next sequence //todo ... other x-values get lost for old format
    Reference< chart2::data::XLabeledDataSequence > xXValues(
        DataSeriesHelper::getDataSequenceByRole( xSeriesSource, C2U("values-x") ) );
    if( xXValues.is() )
        aResultVector.push_back( xXValues );

    //add all other sequences now without x-values
    for( sal_Int32 nN=0; nN<aDataSeqences.getLength(); nN++ )
    {
        OUString aRole( DataSeriesHelper::GetRole( aDataSeqences[nN] ) );
        if( !aRole.equals(C2U("values-x")) )
            aResultVector.push_back( aDataSeqences[nN] );
    }

    Sequence< Reference< chart2::data::XLabeledDataSequence > > aResultSequence( aResultVector.size() );
    ::std::copy( aResultVector.begin(), aResultVector.end(), aResultSequence.getArray() );

    return new DataSource( aResultSequence );
}

uno::Sequence< ::rtl::OUString > DataSourceHelper::getUsedDataRanges(
    const uno::Reference< chart2::XDiagram > & xDiagram )
{
    ::std::vector< ::rtl::OUString > aResult;

    if( xDiagram.is())
    {
        uno::Reference< data::XLabeledDataSequence > xCategories( DiagramHelper::getCategoriesFromDiagram( xDiagram ) );
        if( xCategories.is() )
            lcl_addRanges( aResult, xCategories );

        ::std::vector< uno::Reference< XDataSeries > > aSeriesVector( DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );
        for( ::std::vector< uno::Reference< XDataSeries > >::const_iterator aSeriesIt( aSeriesVector.begin() )
                 ; aSeriesIt != aSeriesVector.end(); ++aSeriesIt )
        {
            uno::Reference< data::XDataSource > xDataSource( *aSeriesIt, uno::UNO_QUERY );
            lcl_addDataSourceRanges( aResult, xDataSource );
            lcl_addErrorBarRanges( aResult, *aSeriesIt );
        }
    }

    return ContainerHelper::ContainerToSequence( aResult );
}

uno::Sequence< ::rtl::OUString > DataSourceHelper::getUsedDataRanges( const uno::Reference< frame::XModel > & xChartModel )
{
    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    return getUsedDataRanges( xDiagram );
}

uno::Reference< chart2::data::XDataSource > DataSourceHelper::getUsedData(
    const uno::Reference< chart2::XChartDocument >& xChartDoc,
    bool bIncludeUnusedData /* = false */ )
{
    if( bIncludeUnusedData )
        return getUsedData( uno::Reference< frame::XModel >( xChartDoc, uno::UNO_QUERY ), bIncludeUnusedData );
    else
         return pressUsedDataIntoRectangularFormat( xChartDoc );
}

uno::Reference< chart2::data::XDataSource > DataSourceHelper::getUsedData(
    const uno::Reference< frame::XModel >& xChartModel,
    bool bIncludeUnusedData /* = false */ )
{
    ::std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aResult;

    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    uno::Reference< data::XLabeledDataSequence > xCategories( DiagramHelper::getCategoriesFromDiagram( xDiagram ) );
    if( xCategories.is() )
        aResult.push_back( xCategories );

    ::std::vector< uno::Reference< XDataSeries > > aSeriesVector( ChartModelHelper::getDataSeries( xChartModel ) );
    for( ::std::vector< uno::Reference< XDataSeries > >::const_iterator aSeriesIt( aSeriesVector.begin() )
        ; aSeriesIt != aSeriesVector.end(); ++aSeriesIt )
    {
        uno::Reference< data::XDataSource > xDataSource( *aSeriesIt, uno::UNO_QUERY );
        if( !xDataSource.is() )
            continue;
        uno::Sequence< uno::Reference< data::XLabeledDataSequence > > aDataSequences( xDataSource->getDataSequences() );
        ::std::copy( aDataSequences.getConstArray(), aDataSequences.getConstArray() + aDataSequences.getLength(),
                     ::std::back_inserter( aResult ));
    }

    if( bIncludeUnusedData && xDiagram.is())
    {
        uno::Sequence< uno::Reference< data::XLabeledDataSequence > > aUnusedData( xDiagram->getUnusedData());
        ::std::copy( aUnusedData.getConstArray(),
                     aUnusedData.getConstArray() + aUnusedData.getLength(),
                     ::std::back_inserter( aResult ));
    }

    return uno::Reference< chart2::data::XDataSource >(
        new DataSource( ContainerHelper::ContainerToSequence( aResult )));
}

bool DataSourceHelper::detectRangeSegmentation(
    const uno::Reference<
        frame::XModel >& xChartModel
    , ::rtl::OUString& rOutRangeString
    , ::com::sun::star::uno::Sequence< sal_Int32 >& rSequenceMapping
    , bool& rOutUseColumns
    , bool& rOutFirstCellAsLabel
    , bool& rOutHasCategories
    , bool bIncludeUnusedData /* = false */)
{
    bool bSomethingDetected = false;

    uno::Reference< XChartDocument > xChartDocument( xChartModel, uno::UNO_QUERY );
    if( !xChartDocument.is() )
        return bSomethingDetected;
    uno::Reference< data::XDataProvider >  xDataProvider( xChartDocument->getDataProvider() );
    if( !xDataProvider.is() )
        return bSomethingDetected;

    OSL_ASSERT( !bIncludeUnusedData ); //bIncludeUnusedData is not supported currently
    (void)(bIncludeUnusedData); // avoid warning in non-debug build

    try
    {
        DataSourceHelper::readArguments(
            xDataProvider->detectArguments( pressUsedDataIntoRectangularFormat( xChartDocument ) ),
            rOutRangeString, rSequenceMapping, rOutUseColumns, rOutFirstCellAsLabel, rOutHasCategories );
        bSomethingDetected = (rOutRangeString.getLength() > 0);

        uno::Reference< chart2::data::XLabeledDataSequence > xCategories(
                    DiagramHelper::getCategoriesFromDiagram( xChartDocument->getFirstDiagram() ));
        rOutHasCategories = xCategories.is();
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return bSomethingDetected;
}

bool DataSourceHelper::allArgumentsForRectRangeDetected(
    const uno::Reference< chart2::XChartDocument >& xChartDocument )
{
    bool bHasDataRowSource = false;
    bool bHasFirstCellAsLabel = false;
//     bool bHasHasCategories = false;
    bool bHasCellRangeRepresentation = false;
//     bool bHasSequenceMapping = false;

    uno::Reference< data::XDataProvider > xDataProvider( xChartDocument->getDataProvider() );
    if( !xDataProvider.is() )
        return false;

    try
    {
        const uno::Sequence< beans::PropertyValue > aArguments(
            xDataProvider->detectArguments( pressUsedDataIntoRectangularFormat( xChartDocument )));
        const beans::PropertyValue* pArguments = aArguments.getConstArray();
        for(sal_Int32 i=0; i<aArguments.getLength(); ++i, ++pArguments)
        {
            const beans::PropertyValue& aProperty = *pArguments;
            if( aProperty.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "DataRowSource" ) ))
            {
                bHasDataRowSource =
                    (aProperty.Value.hasValue() && aProperty.Value.isExtractableTo(
                        ::getCppuType( reinterpret_cast<
                                       const ::com::sun::star::chart::ChartDataRowSource * >(0))));
            }
            else if( aProperty.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "FirstCellAsLabel" ) ))
            {
                bHasFirstCellAsLabel =
                    (aProperty.Value.hasValue() && aProperty.Value.isExtractableTo(::getBooleanCppuType()));
            }
//             else if( aProperty.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "HasCategories" ) ))
//             {
//                 bHasHasCategories =
//                     (aProperty.Value.hasValue() && aProperty.Value.isExtractableTo(::getBooleanCppuType()));
//             }
            else if( aProperty.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "CellRangeRepresentation" ) ))
            {
                ::rtl::OUString aRange;
                bHasCellRangeRepresentation =
                    (aProperty.Value.hasValue() && (aProperty.Value >>= aRange) && aRange.getLength() > 0);
            }
//         else if( aProperty.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "SequenceMapping" ) ))
//         {
//             bHasSequenceMapping =
//                 (aProperty.Value.hasValue() && aProperty.Value.isExtractableTo(
//                     ::getCppuType( reinterpret_cast<
//                                    const uno::Sequence< sal_Int32 > * >(0))));
//         }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return (bHasCellRangeRepresentation && bHasDataRowSource && bHasFirstCellAsLabel);
}

void DataSourceHelper::setRangeSegmentation(
            const uno::Reference< frame::XModel >& xChartModel
            , const ::com::sun::star::uno::Sequence< sal_Int32 >& rSequenceMapping
            , bool bUseColumns , bool bFirstCellAsLabel, bool bUseCategories )
{
    uno::Reference< XChartDocument > xChartDocument( xChartModel, uno::UNO_QUERY );
    if( !xChartDocument.is() )
        return;
    uno::Reference< data::XDataProvider > xDataProvider( xChartDocument->getDataProvider() );
    if( !xDataProvider.is() )
        return;
    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    if( !xDiagram.is() )
        return;
    uno::Reference< chart2::XChartTypeManager > xChartTypeManager( xChartDocument->getChartTypeManager() );
    if( !xChartTypeManager.is() )
        return;
    uno::Reference< lang::XMultiServiceFactory > xTemplateFactory( xChartTypeManager, uno::UNO_QUERY );
    if( !xTemplateFactory.is() )
        return;

    ::rtl::OUString aRangeString;
    bool bDummy;
    uno::Sequence< sal_Int32 > aDummy;
    readArguments( xDataProvider->detectArguments( pressUsedDataIntoRectangularFormat( xChartDocument )),
                   aRangeString, aDummy, bDummy, bDummy, bDummy );

    uno::Sequence< beans::PropertyValue > aArguments(
        createArguments( aRangeString, rSequenceMapping, bUseColumns, bFirstCellAsLabel, bUseCategories ) );

    uno::Reference< chart2::data::XDataSource > xDataSource( xDataProvider->createDataSource(
                                                                 aArguments ) );
    if( !xDataSource.is() )
        return;

    DiagramHelper::tTemplateWithServiceName aTemplateAndService =
        DiagramHelper::getTemplateForDiagram( xDiagram, xTemplateFactory );

    rtl::OUString aServiceName( aTemplateAndService.second );
    uno::Reference< chart2::XChartTypeTemplate > xTemplate = aTemplateAndService.first;

    if( !xTemplate.is() )
    {
        if( aServiceName.getLength() == 0 )
            aServiceName = C2U("com.sun.star.chart2.template.Column");
        xTemplate.set( xTemplateFactory->createInstance( aServiceName ), uno::UNO_QUERY );
    }
    if( !xTemplate.is() )
        return;

    // /-- locked controllers
    ControllerLockGuard aCtrlLockGuard( xChartModel );
    xTemplate->changeDiagramData( xDiagram, xDataSource, aArguments );
    // \-- locked controllers
}

Sequence< OUString > DataSourceHelper::getRangesFromLabeledDataSequence(
    const Reference< data::XLabeledDataSequence > & xLSeq )
{
    Sequence< OUString > aResult;
    if( xLSeq.is())
    {
        Reference< data::XDataSequence > xLabel( xLSeq->getLabel());
        Reference< data::XDataSequence > xValues( xLSeq->getValues());

        if( xLabel.is())
        {
            if( xValues.is())
            {
                aResult.realloc( 2 );
                aResult[0] = xLabel->getSourceRangeRepresentation();
                aResult[1] = xValues->getSourceRangeRepresentation();
            }
            else
            {
                aResult.realloc( 1 );
                aResult[0] = xLabel->getSourceRangeRepresentation();
            }
        }
        else if( xValues.is())
        {
            aResult.realloc( 1 );
            aResult[0] = xValues->getSourceRangeRepresentation();
        }
    }
    return aResult;
}

Sequence< OUString > DataSourceHelper::getRangesFromDataSource( const Reference< data::XDataSource > & xSource )
{
    ::std::vector< OUString > aResult;
    if( xSource.is())
    {
        Sequence< Reference< data::XLabeledDataSequence > > aLSeqSeq( xSource->getDataSequences());
        for( sal_Int32 i=0; i<aLSeqSeq.getLength(); ++i )
        {
            Reference< data::XDataSequence > xLabel( aLSeqSeq[i]->getLabel());
            Reference< data::XDataSequence > xValues( aLSeqSeq[i]->getValues());

            if( xLabel.is())
                aResult.push_back( xLabel->getSourceRangeRepresentation());
            if( xValues.is())
                aResult.push_back( xValues->getSourceRangeRepresentation());
        }
    }
    return ContainerHelper::ContainerToSequence( aResult );
}

bool DataSourceHelper::migrateData(
    const Reference< chart2::XChartDocument > & xChartDoc,
    const Reference< chart2::data::XDataProvider > & xNewDataProvider )
{
    if( ! ( xChartDoc.is() && xNewDataProvider.is()))
        return false;

    bool bCouldMigrate = true;
    Reference< chart2::data::XDataSource > xDataSource(
        DataSourceHelper::getUsedData( xChartDoc, true /* bIncludeUnusedData */ ));
    Sequence< Reference< chart2::data::XLabeledDataSequence > > aLSeq(
        xDataSource->getDataSequences());
    ::std::for_each( aLSeq.getArray(), aLSeq.getArray() + aLSeq.getLength(),
                     lcl_migrateData( xNewDataProvider ));

    return bCouldMigrate;
}


//.............................................................................
} //namespace chart
//.............................................................................
