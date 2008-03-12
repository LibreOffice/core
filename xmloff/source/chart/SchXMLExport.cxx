/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SchXMLExport.cxx,v $
 *
 *  $Revision: 1.97 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:25:49 $
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
#include "precompiled_xmloff.hxx"

#include <xmloff/xmlprmap.hxx>

#include "SchXMLExport.hxx"
#include "XMLChartPropertySetMapper.hxx"
#include "SchXMLSeriesHelper.hxx"
#include "ColorPropertySet.hxx"
#include "SchXMLTools.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _GLOBNAME_HXX
#include <tools/globname.hxx>
#endif
#ifndef _SOT_CLSIDS_HXX
#include <sot/clsids.hxx>
#endif

#ifndef _SVTOOLS_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include <xmloff/families.hxx>
#endif
#ifndef _XMLOFF_XMLASTPLP_HXX
#include <xmloff/xmlaustp.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_XMLMETAE_HXX
#include <xmloff/xmlmetae.hxx>
#endif
#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
// header for any2enum
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#include <list>
#include <typeinfo>

#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XRefreshable.hpp>

#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/ChartLegendPosition.hpp>
#include <com/sun/star/chart/XTwoAxisXSupplier.hpp>
#include <com/sun/star/chart/XTwoAxisYSupplier.hpp>
#include <com/sun/star/chart/XAxisZSupplier.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/ChartAxisAssign.hpp>
#include <com/sun/star/chart/ChartSeriesAddress.hpp>
#include <com/sun/star/chart/X3DDisplay.hpp>
#include <com/sun/star/chart/XStatisticDisplay.hpp>

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>

#include <com/sun/star/util/XStringMapping.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/chart/XSecondAxisTitleSupplier.hpp>

#include "MultiPropertySetHandler.hxx"
#include "PropertyMap.hxx"

using namespace com::sun::star;
using namespace ::xmloff::token;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::rtl::OUStringToOString;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;

namespace
{
Reference< uno::XComponentContext > lcl_getComponentContext()
{
    Reference< uno::XComponentContext > xContext;
    try
    {
        Reference< beans::XPropertySet > xFactProp( comphelper::getProcessServiceFactory(), uno::UNO_QUERY );
        if( xFactProp.is())
            xFactProp->getPropertyValue(OUString::createFromAscii("DefaultContext")) >>= xContext;
    }
    catch( uno::Exception& )
    {}

    return xContext;
}

class lcl_MatchesRole : public ::std::unary_function< Reference< chart2::data::XLabeledDataSequence >, bool >
{
public:
    explicit lcl_MatchesRole( const OUString & aRole ) :
            m_aRole( aRole )
    {}

    bool operator () ( const Reference< chart2::data::XLabeledDataSequence > & xSeq ) const
    {
        if( !xSeq.is() )
            return  false;
        Reference< beans::XPropertySet > xProp( xSeq->getValues(), uno::UNO_QUERY );
        OUString aRole;

        return ( xProp.is() &&
                 (xProp->getPropertyValue(
                     OUString( RTL_CONSTASCII_USTRINGPARAM( "Role" )) ) >>= aRole ) &&
                 m_aRole.equals( aRole ));
    }

private:
    OUString m_aRole;
};

template< typename T >
    void lcl_SequenceToVectorAppend( const Sequence< T > & rSource, ::std::vector< T > & rDestination )
{
    rDestination.reserve( rDestination.size() + rSource.getLength());
    ::std::copy( rSource.getConstArray(), rSource.getConstArray() + rSource.getLength(),
                 ::std::back_inserter( rDestination ));
}

Reference< chart2::data::XLabeledDataSequence > lcl_getCategories( const Reference< chart2::XDiagram > & xDiagram )
{
    Reference< chart2::data::XLabeledDataSequence >  xResult;
    try
    {
        Reference< chart2::XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW );
        Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 i=0; i<aCooSysSeq.getLength(); ++i )
        {
            Reference< chart2::XCoordinateSystem > xCooSys( aCooSysSeq[i] );
            OSL_ASSERT( xCooSys.is());
            for( sal_Int32 nN = xCooSys->getDimension(); nN--; )
            {
                const sal_Int32 nMaxAxisIndex = xCooSys->getMaximumAxisIndexByDimension(nN);
                for(sal_Int32 nI=0; nI<=nMaxAxisIndex; ++nI)
                {
                    Reference< chart2::XAxis > xAxis = xCooSys->getAxisByDimension( nN, nI );
                    OSL_ASSERT( xAxis.is());
                    if( xAxis.is())
                    {
                        chart2::ScaleData aScaleData = xAxis->getScaleData();
                        if( aScaleData.Categories.is())
                        {
                            xResult.set( aScaleData.Categories );
                            break;
                        }
                    }
                }
            }
        }
    }
    catch( uno::Exception & ex )
    {
        (void)ex; // avoid warning for pro build
        OSL_ENSURE( false, OUStringToOString(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "Exception caught. Type: " )) +
                        OUString::createFromAscii( typeid( ex ).name()) +
                        OUString( RTL_CONSTASCII_USTRINGPARAM( ", Message: " )) +
                        ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
    }

    /*
    //unused ranges are very problematic as they bear the risk to damage the rectangular structure completly
    if(!xResult.is())
    {
        Sequence< Reference< chart2::data::XLabeledDataSequence > > aUnusedSequences( xDiagram->getUnusedData() );

        lcl_MatchesRole aHasCategories( OUString::createFromAscii("categories" ) );
        for( sal_Int32 nN=0; nN<aUnusedSequences.getLength(); nN++ )
        {
            if( aHasCategories( aUnusedSequences[nN] ) )
            {
                xResult.set( aUnusedSequences[nN] );
                break;
            }
        }
    }
    */

    return xResult;
}

Reference< chart2::data::XDataSource > lcl_createDataSource(
    const Sequence< Reference< chart2::data::XLabeledDataSequence > > & aData )
{
    Reference< chart2::data::XDataSink > xSink;
    Reference< uno::XComponentContext > xContext( lcl_getComponentContext());
    if( xContext.is() )
        xSink.set(
            xContext->getServiceManager()->createInstanceWithContext(
                OUString::createFromAscii("com.sun.star.chart2.data.DataSource"),
                xContext ), uno::UNO_QUERY_THROW );
    if( xSink.is())
        xSink->setData( aData );

    return Reference< chart2::data::XDataSource >( xSink, uno::UNO_QUERY );
}

Sequence< Reference< chart2::data::XLabeledDataSequence > > lcl_getAllSeriesSequences( const Reference< chart2::XChartDocument >& xChartDoc )
{
    ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aContainer;
    if( xChartDoc.is() )
    {
        Reference< chart2::XDiagram > xDiagram( xChartDoc->getFirstDiagram());
        ::std::vector< Reference< chart2::XDataSeries > > aSeriesVector( SchXMLSeriesHelper::getDataSeriesFromDiagram( xDiagram ));
        for( ::std::vector< Reference< chart2::XDataSeries > >::const_iterator aSeriesIt( aSeriesVector.begin() )
            ; aSeriesIt != aSeriesVector.end(); ++aSeriesIt )
        {
            Reference< chart2::data::XDataSource > xDataSource( *aSeriesIt, uno::UNO_QUERY );
            if( !xDataSource.is() )
                continue;
            uno::Sequence< Reference< chart2::data::XLabeledDataSequence > > aDataSequences( xDataSource->getDataSequences() );
            lcl_SequenceToVectorAppend( aDataSequences, aContainer );
        }
    }

    Sequence< Reference< chart2::data::XLabeledDataSequence > > aRet( aContainer.size());
    ::std::copy( aContainer.begin(), aContainer.end(), aRet.getArray());

    return aRet;
}

Reference< chart2::data::XLabeledDataSequence >
    lcl_getDataSequenceByRole(
        const Sequence< Reference< chart2::data::XLabeledDataSequence > > & aLabeledSeq,
        const OUString & rRole )
{
    Reference< chart2::data::XLabeledDataSequence > aNoResult;

    const Reference< chart2::data::XLabeledDataSequence > * pBegin = aLabeledSeq.getConstArray();
    const Reference< chart2::data::XLabeledDataSequence > * pEnd = pBegin + aLabeledSeq.getLength();
    const Reference< chart2::data::XLabeledDataSequence > * pMatch =
        ::std::find_if( pBegin, pEnd, lcl_MatchesRole( rRole ));

    if( pMatch != pEnd )
        return *pMatch;

    return aNoResult;
}

Reference< chart2::data::XDataSource > lcl_pressUsedDataIntoRectangularFormat( const Reference< chart2::XChartDocument >& xChartDoc, sal_Bool& rOutSourceHasCategoryLabels )
{
    ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aLabeledSeqVector;

    //categories are always the first sequence
    Reference< chart2::XDiagram > xDiagram( xChartDoc->getFirstDiagram());
    Reference< chart2::data::XLabeledDataSequence > xCategories( lcl_getCategories( xDiagram ) );
    if( xCategories.is() )
        aLabeledSeqVector.push_back( xCategories );
    rOutSourceHasCategoryLabels = sal_Bool(xCategories.is());

    Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeriesSeqVector(
            lcl_getAllSeriesSequences( xChartDoc ) );

    //the first x-values is always the next sequence //todo ... other x-values get lost for old format
    Reference< chart2::data::XLabeledDataSequence > xXValues(
        lcl_getDataSequenceByRole( aSeriesSeqVector, OUString::createFromAscii("values-x" ) ) );
    if( xXValues.is() )
        aLabeledSeqVector.push_back( xXValues );

    //add all other sequences now without x-values
    lcl_MatchesRole aHasXValues( OUString::createFromAscii("values-x" ) );
    for( sal_Int32 nN=0; nN<aSeriesSeqVector.getLength(); nN++ )
    {
        if( !aHasXValues( aSeriesSeqVector[nN] ) )
            aLabeledSeqVector.push_back( aSeriesSeqVector[nN] );
    }

    Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeq( aLabeledSeqVector.size() );
    ::std::copy( aLabeledSeqVector.begin(), aLabeledSeqVector.end(), aSeq.getArray() );

    return lcl_createDataSource( aSeq );
}

bool lcl_isSeriesAttachedToFirstAxis(
    const Reference< chart2::XDataSeries > & xDataSeries )
{
    bool bResult=true;

    try
    {
        sal_Int32 nAxisIndex = 0;
        Reference< beans::XPropertySet > xProp( xDataSeries, uno::UNO_QUERY_THROW );
        if( xProp.is() )
            xProp->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("AttachedAxisIndex") ) ) >>= nAxisIndex;
        bResult = (0==nAxisIndex);
    }
    catch( uno::Exception & ex )
    {
        (void)ex; // avoid warning for pro build
        OSL_ENSURE( false, OUStringToOString(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "Exception caught. Type: " )) +
                        OUString::createFromAscii( typeid( ex ).name()) +
                        OUString( RTL_CONSTASCII_USTRINGPARAM( ", Message: " )) +
                        ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
    }

    return bResult;
}

OUString lcl_ConvertRange( const ::rtl::OUString & rRange, const Reference< chart2::XChartDocument > & xDoc )
{
    OUString aResult = rRange;
    Reference< chart2::data::XRangeXMLConversion > xConversion(
        SchXMLExportHelper::GetDataProvider( xDoc ), uno::UNO_QUERY );
    if( xConversion.is())
        aResult = xConversion->convertRangeToXML( rRange );
    return aResult;
}

typedef ::std::pair< OUString, OUString > tLabelAndValueRange;

tLabelAndValueRange lcl_getLabelAndValueRangeByRole(
    const Sequence< Reference< chart2::data::XLabeledDataSequence > > & aSeqCnt,
    const OUString & rRole,
    const Reference< chart2::XChartDocument > & xDoc,
    SchXMLExportHelper::tDataSequenceCont & rOutSequencesToExport )
{
    tLabelAndValueRange aResult;

    Reference< chart2::data::XLabeledDataSequence > xLabeledSeq(
        lcl_getDataSequenceByRole( aSeqCnt, rRole ));
    if( xLabeledSeq.is())
    {
        Reference< chart2::data::XDataSequence > xLabelSeq( xLabeledSeq->getLabel());
        if( xLabelSeq.is())
            aResult.first = lcl_ConvertRange( xLabelSeq->getSourceRangeRepresentation(), xDoc );

        Reference< chart2::data::XDataSequence > xValueSeq( xLabeledSeq->getValues());
        if( xValueSeq.is())
            aResult.second = lcl_ConvertRange( xValueSeq->getSourceRangeRepresentation(), xDoc );

        if( xLabelSeq.is() || xValueSeq.is())
            rOutSequencesToExport.push_back( SchXMLExportHelper::tLabelValuesDataPair( xLabelSeq, xValueSeq ));
    }

    return aResult;
}

sal_Int32 lcl_getSequenceLengthByRole(
    const Sequence< Reference< chart2::data::XLabeledDataSequence > > & aSeqCnt,
    const OUString & rRole )
{
    Reference< chart2::data::XLabeledDataSequence > xLabeledSeq(
        lcl_getDataSequenceByRole( aSeqCnt, rRole ));
    if( xLabeledSeq.is())
    {
        Reference< chart2::data::XDataSequence > xSeq( xLabeledSeq->getValues());
        return xSeq->getData().getLength();
    }
    return 0;
}

bool lcl_hasChartType( const Reference< chart2::XDiagram > & xDiagram, const OUString & rChartType )
{
    try
    {
        Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xDiagram, uno::UNO_QUERY_THROW );
        Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq( xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 nCooSysIdx=0; nCooSysIdx<aCooSysSeq.getLength(); ++nCooSysIdx )
        {
            Reference< chart2::XChartTypeContainer > xCTCnt( aCooSysSeq[nCooSysIdx], uno::UNO_QUERY_THROW );
            Sequence< Reference< chart2::XChartType > > aChartTypes( xCTCnt->getChartTypes());
            for( sal_Int32 nCTIdx=0; nCTIdx<aChartTypes.getLength(); ++nCTIdx )
            {
                if( aChartTypes[nCTIdx]->getChartType().equals( rChartType ))
                    return true;
            }
        }
    }
    catch( uno::Exception & )
    {
        DBG_ERROR( "Exception while searching for chart type in diagram" );
    }
    return false;
}

OUString lcl_flattenStringSequence( const Sequence< OUString > & rSequence )
{
    OUStringBuffer aResult;
    bool bPrecedeWithSpace = false;
    for( sal_Int32 nIndex=0; nIndex<rSequence.getLength(); ++nIndex )
    {
        if( rSequence[nIndex].getLength())
        {
            if( bPrecedeWithSpace )
                aResult.append( static_cast< sal_Unicode >( ' ' ));
            aResult.append( rSequence[nIndex] );
            bPrecedeWithSpace = true;
        }
    }
    return aResult.makeStringAndClear();
}

OUString lcl_getLabelString( const Reference< chart2::data::XDataSequence > & xLabelSeq )
{
    Sequence< OUString > aLabels;

    uno::Reference< chart2::data::XTextualDataSequence > xTextualDataSequence( xLabelSeq, uno::UNO_QUERY );
    if( xTextualDataSequence.is())
    {
        aLabels = xTextualDataSequence->getTextualData();
    }
    else if( xLabelSeq.is())
    {
        Sequence< uno::Any > aAnies( xLabelSeq->getData());
        aLabels.realloc( aAnies.getLength());
        for( sal_Int32 i=0; i<aAnies.getLength(); ++i )
            aAnies[i] >>= aLabels[i];
    }

    return lcl_flattenStringSequence( aLabels );
}

sal_Int32 lcl_getMaxSequenceLength(
    const SchXMLExportHelper::tDataSequenceCont & rContainer )
{
    sal_Int32 nResult = 0;
    for( SchXMLExportHelper::tDataSequenceCont::const_iterator aIt( rContainer.begin());
         aIt != rContainer.end(); ++aIt )
    {
        if( aIt->first.is())
        {
            sal_Int32 nSeqLength = aIt->first->getData().getLength();
            if( nSeqLength > nResult )
                nResult = nSeqLength;
        }
        if( aIt->second.is())
        {
            sal_Int32 nSeqLength = aIt->second->getData().getLength();
            if( nSeqLength > nResult )
                nResult = nSeqLength;
        }
    }
    return nResult;
}

void lcl_fillCategoriesIntoStringVector(
    const Reference< chart2::data::XDataSequence > & xCategories,
    ::std::vector< OUString > & rOutCategories )
{
    OSL_ASSERT( xCategories.is());
    if( !xCategories.is())
        return;
    Reference< chart2::data::XTextualDataSequence > xTextualDataSequence( xCategories, uno::UNO_QUERY );
    if( xTextualDataSequence.is())
    {
        rOutCategories.clear();
        Sequence< OUString > aTextData( xTextualDataSequence->getTextualData());
        ::std::copy( aTextData.getConstArray(), aTextData.getConstArray() + aTextData.getLength(),
                     ::std::back_inserter( rOutCategories ));
    }
    else
    {
        Sequence< uno::Any > aAnies( xCategories->getData());
        rOutCategories.resize( aAnies.getLength());
        for( sal_Int32 i=0; i<aAnies.getLength(); ++i )
            aAnies[i] >>= rOutCategories[i];
    }
}

double lcl_getValueFromSequence( const Reference< chart2::data::XDataSequence > & xSeq, sal_Int32 nIndex )
{
    double fResult = 0.0;
    ::rtl::math::setNan( &fResult );
    Reference< chart2::data::XNumericalDataSequence > xNumSeq( xSeq, uno::UNO_QUERY );
    if( xNumSeq.is())
    {
        Sequence< double > aValues( xNumSeq->getNumericalData());
        if( nIndex < aValues.getLength() )
            fResult = aValues[nIndex];
    }
    else
    {
        Sequence< uno::Any > aAnies( xSeq->getData());
        if( nIndex < aAnies.getLength() )
            aAnies[nIndex] >>= fResult;
    }
    return fResult;
}

::std::vector< double > lcl_getAllValuesFromSequence( const Reference< chart2::data::XDataSequence > & xSeq )
{
    double fNan = 0.0;
    ::rtl::math::setNan( &fNan );
    ::std::vector< double > aResult;

    Reference< chart2::data::XNumericalDataSequence > xNumSeq( xSeq, uno::UNO_QUERY );
    if( xNumSeq.is())
    {
        Sequence< double > aValues( xNumSeq->getNumericalData());
        ::std::copy( aValues.getConstArray(), aValues.getConstArray() + aValues.getLength(),
                     ::std::back_inserter( aResult ));
    }
    else if( xSeq.is())
    {
        Sequence< uno::Any > aAnies( xSeq->getData());
        aResult.resize( aAnies.getLength());
        for( sal_Int32 i=0; i<aAnies.getLength(); ++i )
            aAnies[i] >>= aResult[i];
    }
    return aResult;
}

struct lcl_TableData
{
    typedef ::std::vector< OUString > tStringContainer;
    typedef ::std::vector< ::std::vector< double > > tTwoDimNumberContainer;

    tTwoDimNumberContainer aDataInRows;
    tStringContainer       aDataRangeRepresentations;
    tStringContainer       aFirstRowStrings;
    tStringContainer       aFirstRowRangeRepresentations;
    tStringContainer       aFirstColumnStrings;
    tStringContainer       aFirstColumnRangeRepresentations;
};

// ::std::bind2nd( ::std::mem_fun_ref( &T::resize ), nSize ) does not work
template< class T >
    struct lcl_resize
    {
        lcl_resize( typename T::size_type nSize ) : m_nSize( nSize ) {}
        void operator()( T & t )
        { t.resize( m_nSize ); }
    private:
        typename T::size_type m_nSize;
    };


typedef ::std::map< sal_Int32, SchXMLExportHelper::tLabelValuesDataPair >
    lcl_DataSequenceMap;

struct lcl_SequenceToMapElement :
    public ::std::unary_function< lcl_DataSequenceMap::mapped_type, lcl_DataSequenceMap::value_type >
{
    lcl_SequenceToMapElement( sal_Int32 nOffset = 0 ) :
            m_nOffset( nOffset )
    {}
    result_type operator() ( const argument_type & rContent )
    {
        sal_Int32 nIndex = -1;
        if( rContent.second.is())
        {
            OUString aRangeRep( rContent.second->getSourceRangeRepresentation());
            if( aRangeRep.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("categories")))
            {
                OSL_ASSERT( m_nOffset > 0 );
                nIndex = 0;
            }
            else
                nIndex = aRangeRep.toInt32() + m_nOffset;
        }
        else if( rContent.first.is())
            nIndex = rContent.first->getSourceRangeRepresentation().copy( sizeof("label ")).toInt32() + m_nOffset;
        return result_type( nIndex, rContent );
    }
private:
    sal_Int32 m_nOffset;
};

void lcl_PrepareInternalSequencesForTableExport(
    SchXMLExportHelper::tDataSequenceCont & rInOutSequences, bool bHasCategories )
{
    lcl_DataSequenceMap aIndexSequenceMap;
    const sal_Int32 nOffset = bHasCategories ? 1 : 0;
    ::std::transform( rInOutSequences.begin(), rInOutSequences.end(),
                      ::std::inserter( aIndexSequenceMap, aIndexSequenceMap.begin()),
                      lcl_SequenceToMapElement( nOffset ));

    rInOutSequences.clear();
    sal_Int32 nIndex = 0;
    for( lcl_DataSequenceMap::const_iterator aIt = aIndexSequenceMap.begin();
         aIt != aIndexSequenceMap.end(); ++aIt, ++nIndex )
    {
        if( aIt->first < 0 )
            continue;
        // fill empty columns
        for( ; nIndex < aIt->first; ++nIndex )
            rInOutSequences.push_back(
                SchXMLExportHelper::tDataSequenceCont::value_type( 0, 0 ));
        OSL_ASSERT( nIndex == aIt->first );
        rInOutSequences.push_back( aIt->second );
    }
}


lcl_TableData lcl_getDataForLocalTable(
    const SchXMLExportHelper::tDataSequenceCont & aPassedSequences, bool bHasCategoryLabels,
    bool bSwap,
    bool bHasOwnData,
    const Reference< chart2::data::XRangeXMLConversion > & xRangeConversion )
{
    lcl_TableData aResult;

    SchXMLExportHelper::tDataSequenceCont aSequencesToExport( aPassedSequences );
    if( bHasOwnData )
        lcl_PrepareInternalSequencesForTableExport( aSequencesToExport, bHasCategoryLabels );

    SchXMLExportHelper::tDataSequenceCont::size_type nNumSequences = aSequencesToExport.size();
    SchXMLExportHelper::tDataSequenceCont::const_iterator aBegin( aSequencesToExport.begin());
    SchXMLExportHelper::tDataSequenceCont::const_iterator aEnd( aSequencesToExport.end());
    SchXMLExportHelper::tDataSequenceCont::const_iterator aIt( aBegin );

    if( bHasCategoryLabels )
    {
        if( nNumSequences>=1 ) //#i83537#
            --nNumSequences;
        else
            bHasCategoryLabels=false;
    }
    size_t nMaxSequenceLength( lcl_getMaxSequenceLength( aSequencesToExport ));
    size_t nNumColumns( bSwap ? nMaxSequenceLength : nNumSequences );
    size_t nNumRows( bSwap ? nNumSequences : nMaxSequenceLength );

    // resize data
    aResult.aDataInRows.resize( nNumRows );
    double fNan = 0.0;
    ::rtl::math::setNan( &fNan );
    ::std::for_each( aResult.aDataInRows.begin(), aResult.aDataInRows.end(),
                     lcl_resize< lcl_TableData::tTwoDimNumberContainer::value_type >( nNumColumns ));
    aResult.aFirstRowStrings.resize( nNumColumns );
    aResult.aFirstColumnStrings.resize( nNumRows );

    lcl_TableData::tStringContainer & rCategories =
        (bSwap ? aResult.aFirstRowStrings : aResult.aFirstColumnStrings );
    lcl_TableData::tStringContainer & rLabels =
        (bSwap ? aResult.aFirstColumnStrings : aResult.aFirstRowStrings );

    if( aIt != aEnd )
    {
        if( bHasCategoryLabels )
        {
            lcl_fillCategoriesIntoStringVector( aIt->second, rCategories );
            if( aIt->second.is())
            {
                OUString aRange( aIt->second->getSourceRangeRepresentation());
                if( xRangeConversion.is())
                    aRange = xRangeConversion->convertRangeToXML( aRange );
                if( bSwap )
                    aResult.aFirstRowRangeRepresentations.push_back( aRange );
                else
                    aResult.aFirstColumnRangeRepresentations.push_back( aRange );
            }
            ++aIt;
        }
        else
        {
            // autogenerated categories
            rCategories.clear();
            lcl_SequenceToVectorAppend( aIt->second->generateLabel( chart2::data::LabelOrigin_LONG_SIDE ), rCategories );
        }
    }

    // iterate over all sequences
    size_t nSeqIdx = 0;
    for( ; aIt != aEnd; ++aIt, ++nSeqIdx )
    {
        OUString aRange;
        if( aIt->first.is())
        {
            rLabels[nSeqIdx] = lcl_getLabelString( aIt->first );
            aRange = aIt->first->getSourceRangeRepresentation();
            if( xRangeConversion.is())
                aRange = xRangeConversion->convertRangeToXML( aRange );
        }
        else if( aIt->second.is())
            rLabels[nSeqIdx] = lcl_flattenStringSequence(
                aIt->second->generateLabel( chart2::data::LabelOrigin_SHORT_SIDE ));
        if( bSwap )
            aResult.aFirstColumnRangeRepresentations.push_back( aRange );
        else
            aResult.aFirstRowRangeRepresentations.push_back( aRange );

        ::std::vector< double > aNumbers( lcl_getAllValuesFromSequence( aIt->second ));
        if( bSwap )
            aResult.aDataInRows[nSeqIdx] = aNumbers;
        else
        {
            const sal_Int32 nSize( static_cast< sal_Int32 >( aNumbers.size()));
            for( sal_Int32 nIdx=0; nIdx<nSize; ++nIdx )
                aResult.aDataInRows[nIdx][nSeqIdx] = aNumbers[nIdx];
        }

        if( aIt->second.is())
        {
            aRange =  aIt->second->getSourceRangeRepresentation();
            if( xRangeConversion.is())
                aRange = xRangeConversion->convertRangeToXML( aRange );
        }
        aResult.aDataRangeRepresentations.push_back( aRange );
    }

    return aResult;
}

void lcl_exportNumberFormat( const OUString& rPropertyName, const Reference< beans::XPropertySet >& xPropSet,
                                        SvXMLExport& rExport )
{
    if( xPropSet.is())
    {
        sal_Int32 nNumberFormat = 0;
        Any aNumAny = xPropSet->getPropertyValue( rPropertyName );
        if( (aNumAny >>= nNumberFormat) && (nNumberFormat != -1) )
            rExport.addDataStyle( nNumberFormat );
    }
}

::std::vector< Reference< chart2::data::XDataSequence > >
    lcl_getErrorBarSequences( const Reference< beans::XPropertySet > & xErrorBarProp )
{
    ::std::vector< Reference< chart2::data::XDataSequence > > aResult;
    Reference< chart2::data::XDataSource > xErrorBarDataSource( xErrorBarProp, uno::UNO_QUERY );
    if( !xErrorBarDataSource.is())
        return aResult;

    const OUString aRolePrefix( RTL_CONSTASCII_USTRINGPARAM( "error-bars-" ));
//     const OUString aXRolePrefix( aRolePrefix + OUString( RTL_CONSTASCII_USTRINGPARAM( "x-" )));
//     const OUString aYRolePrefix( aRolePrefix + OUString( RTL_CONSTASCII_USTRINGPARAM( "y-" )));
//     const OUString aPositivePostfix( RTL_CONSTASCII_USTRINGPARAM( "positive" ));
//     const OUString aNegativePostfix( RTL_CONSTASCII_USTRINGPARAM( "negative" ));

    Sequence< Reference< chart2::data::XLabeledDataSequence > > aSequences(
        xErrorBarDataSource->getDataSequences());
    for( sal_Int32 nI=0; nI< aSequences.getLength(); ++nI )
    {
        try
        {
            if( aSequences[nI].is())
            {
                Reference< chart2::data::XDataSequence > xSequence( aSequences[nI]->getValues());
                Reference< beans::XPropertySet > xSeqProp( xSequence, uno::UNO_QUERY_THROW );
                OUString aRole;
                if( ( xSeqProp->getPropertyValue(
                          OUString( RTL_CONSTASCII_USTRINGPARAM( "Role" ))) >>= aRole ) &&
                    aRole.match( aRolePrefix ))
                {
                    aResult.push_back( xSequence );
                }
            }
        }
        catch( uno::Exception & rEx )
        {
#ifdef DBG_UTIL
            String aStr( rEx.Message );
            ByteString aBStr( aStr, RTL_TEXTENCODING_ASCII_US );
            DBG_ERROR1( "chart:exporting error bar ranges: %s", aBStr.GetBuffer());
#else
            (void)rEx; // avoid warning
#endif
        }
    }

    return aResult;
}

} // anonymous namespace

struct SchXMLDataPointStruct
{
    OUString   maStyleName;
    sal_Int32  mnRepeat;

    SchXMLDataPointStruct() : mnRepeat( 1 ) {}
};

// ========================================
// class SchXMLExportHelper
// ========================================

SchXMLExportHelper::SchXMLExportHelper(
    SvXMLExport& rExport,
    SvXMLAutoStylePoolP& rASPool ) :
        mrExport( rExport ),
        mrAutoStylePool( rASPool ),
        mbHasSeriesLabels( sal_False ),
        mbHasCategoryLabels( sal_False ),
        mbRowSourceColumns( sal_True )
        // #110680#
        // this id depends on the ServiceManager used due to the binary filter stripping.
        // ,msCLSID( rtl::OUString( SvGlobalName( SO3_SCH_CLASSID ).GetHexName()))
{
    // #110680#
    // changed initialisation for msCLSID. Compare the ServiceInfo name with
    // the known name of the LegacyServiceManager.
    Reference<lang::XServiceInfo> xServiceInfo( mrExport.getServiceFactory(), uno::UNO_QUERY );
    DBG_ASSERT( xServiceInfo.is(), "XMultiServiceFactory without xServiceInfo (!)" );
    OUString rdbURL = xServiceInfo->getImplementationName();
    OUString implLegacyServiceManagerName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.office.LegacyServiceManager" ) );

    if( rdbURL.equals( implLegacyServiceManagerName ))
    {
        msCLSID = OUString( SvGlobalName( BF_SO3_SCH_CLASSID ).GetHexName());
    }
    else
    {
        msCLSID = OUString( SvGlobalName( SO3_SCH_CLASSID ).GetHexName());
    }

    msTableName = OUString::createFromAscii( "local-table" );

    // create factory
    mxPropertyHandlerFactory = new XMLChartPropHdlFactory;

    if( mxPropertyHandlerFactory.is() )
    {
        // create property set mapper
        mxPropertySetMapper = new XMLChartPropertySetMapper;
    }

    mxExpPropMapper = new XMLChartExportPropertyMapper( mxPropertySetMapper, rExport );

    // register chart auto-style family
    mrAutoStylePool.AddFamily(
        XML_STYLE_FAMILY_SCH_CHART_ID,
        OUString::createFromAscii( XML_STYLE_FAMILY_SCH_CHART_NAME ),
        mxExpPropMapper.get(),
        OUString::createFromAscii( XML_STYLE_FAMILY_SCH_CHART_PREFIX ));

    // register shape family
    mrAutoStylePool.AddFamily(
        XML_STYLE_FAMILY_SD_GRAPHICS_ID,
        OUString::createFromAscii( XML_STYLE_FAMILY_SD_GRAPHICS_NAME ),
        mxExpPropMapper.get(),
        OUString::createFromAscii( XML_STYLE_FAMILY_SD_GRAPHICS_PREFIX ));
    // register paragraph family also for shapes
    mrAutoStylePool.AddFamily(
        XML_STYLE_FAMILY_TEXT_PARAGRAPH,
        GetXMLToken( XML_PARAGRAPH ),
        mxExpPropMapper.get(),
        String( 'P' ));
    // register text family also for shapes
    mrAutoStylePool.AddFamily(
        XML_STYLE_FAMILY_TEXT_TEXT,
        GetXMLToken( XML_TEXT ),
        mxExpPropMapper.get(),
        String( 'T' ));
}

SchXMLExportHelper::~SchXMLExportHelper() {}

const OUString& SchXMLExportHelper::getChartCLSID()
{
    return msCLSID;
}

void SchXMLExportHelper::exportAutoStyles()
{
    if( mxExpPropMapper.is())
    {
        //ToDo: when embedded in calc/writer this is not necessary because the
        // numberformatter is shared between both documents
        mrExport.exportAutoDataStyles();

        // export chart auto styles
        mrAutoStylePool.exportXML(
            XML_STYLE_FAMILY_SCH_CHART_ID
            , mrExport.GetDocHandler(),
            mrExport.GetMM100UnitConverter(),
            mrExport.GetNamespaceMap()
            );

        // export auto styles for additional shapes
        mrExport.GetShapeExport()->exportAutoStyles();
        // and for text in additional shapes
        mrExport.GetTextParagraphExport()->exportTextAutoStyles();
    }
}

void SchXMLExportHelper::collectAutoStyles( Reference< chart::XChartDocument > rChartDoc )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogContext, "xmloff", "bm", "::SchXMLExportHelper::collectAutoStyles" );

    parseDocument( rChartDoc, sal_False );
}

void SchXMLExportHelper::exportChart( Reference< chart::XChartDocument > rChartDoc,
                                      sal_Bool bIncludeTable )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogContext, "xmloff", "bm", "::SchXMLExportHelper::exportChart" );

    parseDocument( rChartDoc, sal_True, bIncludeTable );
    DBG_ASSERT( maAutoStyleNameQueue.empty(), "There are still remaining autostyle names in the queue" );
}


// private methods
// ---------------

::rtl::OUString lcl_GetStringFromNumberSequence( const ::com::sun::star::uno::Sequence< sal_Int32 >& rSequenceMapping, bool bRemoveOneFromEachIndex /*should be true if having categories*/ )
{
    const sal_Int32* pArray = rSequenceMapping.getConstArray();
    const sal_Int32 nSize = rSequenceMapping.getLength();
    sal_Int32 i = 0;
    OUStringBuffer aBuf;
    bool bHasPredecessor = false;
    for( i = 0; i < nSize; ++i )
    {
        sal_Int32 nIndex = pArray[ i ];
        if( bRemoveOneFromEachIndex )
            --nIndex;
        if(nIndex>=0)
        {
            if(bHasPredecessor)
                aBuf.append( static_cast< sal_Unicode >( ' ' ));
            aBuf.append( nIndex, 10 );
            bHasPredecessor = true;
        }
    }
    return aBuf.makeStringAndClear();
}

/// if bExportContent is false the auto-styles are collected
void SchXMLExportHelper::parseDocument( Reference< chart::XChartDocument >& rChartDoc,
                                        sal_Bool bExportContent,
                                        sal_Bool bIncludeTable )
{
    Reference< chart2::XChartDocument > xNewDoc( rChartDoc, uno::UNO_QUERY );
    if( !rChartDoc.is() || !xNewDoc.is() )
    {
        DBG_ERROR( "No XChartDocument was given for export." );
        return;
    }

    awt::Size aPageSize( getPageSize( xNewDoc ));
    if( bExportContent )
        addSize( aPageSize );
    Reference< chart::XDiagram > xDiagram = rChartDoc->getDiagram();
    Reference< chart2::XDiagram > xNewDiagram;
    if( xNewDoc.is())
        xNewDiagram.set( xNewDoc->getFirstDiagram());

    //todo remove if model changes are notified and view is updated automatically
    if( bExportContent )
    {
        Reference< util::XRefreshable > xRefreshable( xNewDoc, uno::UNO_QUERY );
        if( xRefreshable.is() )
            xRefreshable->refresh();
    }

    // get Properties of ChartDocument
    sal_Bool bHasMainTitle = sal_False;
    sal_Bool bHasSubTitle = sal_False;
    sal_Bool bHasLegend = sal_False;

    std::vector< XMLPropertyState > aPropertyStates;

    Reference< beans::XPropertySet > xDocPropSet( rChartDoc, uno::UNO_QUERY );
    if( xDocPropSet.is())
    {
        try
        {
            Any aAny( xDocPropSet->getPropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "HasMainTitle" ))));
            aAny >>= bHasMainTitle;
            aAny = xDocPropSet->getPropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "HasSubTitle" )));
            aAny >>= bHasSubTitle;
            aAny = xDocPropSet->getPropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "HasLegend" )));
            aAny >>= bHasLegend;
        }
        catch( beans::UnknownPropertyException & )
        {
            DBG_WARNING( "Required property not found in ChartDocument" );
        }
    }

    // chart element
    // -------------

    SvXMLElementExport* pElChart = 0;
    // get property states for autostyles
    if( mxExpPropMapper.is())
    {
        Reference< beans::XPropertySet > xPropSet( rChartDoc->getArea(), uno::UNO_QUERY );
        if( xPropSet.is())
            aPropertyStates = mxExpPropMapper->Filter( xPropSet );
    }

    if( bExportContent )
    {
        OUString sChartType( xDiagram->getDiagramType() );

        // attributes
        // determine class
        if( sChartType.getLength())
        {
            enum XMLTokenEnum eXMLChartType = SchXMLTools::getTokenByChartType( sChartType, true /* bUseOldNames */ );

            DBG_ASSERT( eXMLChartType != XML_TOKEN_INVALID, "invalid chart class" );
            if( eXMLChartType == XML_TOKEN_INVALID )
                eXMLChartType = XML_BAR;

            if( eXMLChartType == XML_ADD_IN )
            {
                // sChartType is the servie-name of the add-in
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS,
                                       mrExport.GetNamespaceMap().GetQNameByKey(
                                           XML_NAMESPACE_OOO, sChartType) );
            }
            else if( eXMLChartType != XML_TOKEN_INVALID )
            {
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS,
                        mrExport.GetNamespaceMap().GetQNameByKey(
                            XML_NAMESPACE_CHART, GetXMLToken(eXMLChartType )) );
            }

            //column-mapping or row-mapping
            if( maSequenceMapping.getLength() )
            {
                enum XMLTokenEnum eTransToken = ::xmloff::token::XML_ROW_MAPPING;
                if( mbRowSourceColumns )
                    eTransToken = ::xmloff::token::XML_COLUMN_MAPPING;
                ::rtl::OUString aSequenceMappingStr( lcl_GetStringFromNumberSequence(
                    maSequenceMapping, mbHasCategoryLabels && !xNewDoc->hasInternalDataProvider() ) );

                mrExport.AddAttribute( XML_NAMESPACE_CHART,
                                        ::xmloff::token::GetXMLToken( eTransToken ),
                                        aSequenceMappingStr );
            }
        }
        // write style name
        AddAutoStyleAttribute( aPropertyStates );

        //element
        pElChart = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_CHART, sal_True, sal_True );
    }
    else    // autostyles
    {
        CollectAutoStyle( aPropertyStates );
    }
    // remove property states for autostyles
    aPropertyStates.clear();

    // title element
    // -------------

    if( bHasMainTitle )
    {
        // get property states for autostyles
        if( mxExpPropMapper.is())
        {
            Reference< beans::XPropertySet > xPropSet( rChartDoc->getTitle(), uno::UNO_QUERY );
            if( xPropSet.is())
                aPropertyStates = mxExpPropMapper->Filter( xPropSet );
        }
        if( bExportContent )
        {
            Reference< drawing::XShape > xShape = rChartDoc->getTitle();
            if( xShape.is())    // && "hasTitleBeenMoved"
                addPosition( xShape );

            // write style name
            AddAutoStyleAttribute( aPropertyStates );

            // element
            SvXMLElementExport aElTitle( mrExport, XML_NAMESPACE_CHART, XML_TITLE, sal_True, sal_True );

            // content (text:p)
            Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
            if( xPropSet.is())
            {
                Any aAny( xPropSet->getPropertyValue(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "String" ))));
                OUString aText;
                aAny >>= aText;
                exportText( aText );
            }
        }
        else    // autostyles
        {
            CollectAutoStyle( aPropertyStates );
        }
        // remove property states for autostyles
        aPropertyStates.clear();
    }

    // subtitle element
    // ----------------

    if( bHasSubTitle )
    {
        // get property states for autostyles
        if( mxExpPropMapper.is())
        {
            Reference< beans::XPropertySet > xPropSet( rChartDoc->getSubTitle(), uno::UNO_QUERY );
            if( xPropSet.is())
                aPropertyStates = mxExpPropMapper->Filter( xPropSet );
        }

        if( bExportContent )
        {
            Reference< drawing::XShape > xShape = rChartDoc->getSubTitle();
            if( xShape.is())
                addPosition( xShape );

            // write style name
            AddAutoStyleAttribute( aPropertyStates );

            // element (has no subelements)
            SvXMLElementExport aElSubTitle( mrExport, XML_NAMESPACE_CHART, XML_SUBTITLE, sal_True, sal_True );

            // content (text:p)
            Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
            if( xPropSet.is())
            {
                Any aAny( xPropSet->getPropertyValue(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "String" ))));
                OUString aText;
                aAny >>= aText;
                exportText( aText );
            }
        }
        else    // autostyles
        {
            CollectAutoStyle( aPropertyStates );
        }
        // remove property states for autostyles
        aPropertyStates.clear();
    }

    // legend element
    // --------------
    if( bHasLegend )
    {
        // get property states for autostyles
        if( mxExpPropMapper.is())
        {
            Reference< beans::XPropertySet > xPropSet( rChartDoc->getLegend(), uno::UNO_QUERY );
            if( xPropSet.is())
                aPropertyStates = mxExpPropMapper->Filter( xPropSet );
        }

        if( bExportContent )
        {
            Reference< beans::XPropertySet > xProp( rChartDoc->getLegend(), uno::UNO_QUERY );
            if( xProp.is())
            {
                chart::ChartLegendPosition aLegendPos = chart::ChartLegendPosition_NONE;
                try
                {
                    Any aAny( xProp->getPropertyValue(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "Alignment" ))));
                    aAny >>= aLegendPos;
                }
                catch( beans::UnknownPropertyException & )
                {
                    DBG_WARNING( "Property Align not found in ChartLegend" );
                }

                switch( aLegendPos )
                {
                    case chart::ChartLegendPosition_LEFT:
//                      msString = GetXMLToken(XML_LEFT);
                        // #i35421# change left->start (not clear why this was done)
                        msString = GetXMLToken(XML_START);
                        break;
                    case chart::ChartLegendPosition_RIGHT:
//                      msString = GetXMLToken(XML_RIGHT);
                        // #i35421# change right->end (not clear why this was done)
                        msString = GetXMLToken(XML_END);
                        break;
                    case chart::ChartLegendPosition_TOP:
                        msString = GetXMLToken(XML_TOP);
                        break;
                    case chart::ChartLegendPosition_BOTTOM:
                        msString = GetXMLToken(XML_BOTTOM);
                        break;
                    case chart::ChartLegendPosition_NONE:
                    case chart::ChartLegendPosition_MAKE_FIXED_SIZE:
                        // nothing
                        break;
                }

                // export anchor position
                if( msString.getLength())
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LEGEND_POSITION, msString );

                // export absolute position
                msString = OUString();
                Reference< drawing::XShape > xShape( xProp, uno::UNO_QUERY );
                if( xShape.is())
                    addPosition( xShape );
            }

            // write style name
            AddAutoStyleAttribute( aPropertyStates );

            // element
            SvXMLElementExport aLegend( mrExport, XML_NAMESPACE_CHART, XML_LEGEND, sal_True, sal_True );
        }
        else    // autostyles
        {
            CollectAutoStyle( aPropertyStates );
        }
        // remove property states for autostyles
        aPropertyStates.clear();
    }

    // plot-area element
    // -----------------
    if( xDiagram.is())
        exportPlotArea( xDiagram, xNewDiagram, aPageSize, bExportContent, bIncludeTable );

    // export additional shapes
    // ------------------------
    if( xDocPropSet.is() )
    {
        if( bExportContent )
        {
            if( mxAdditionalShapes.is())
            {
                // can't call exportShapes with all shapes because the
                // initialisation happend with the complete draw page and not
                // the XShapes object used here. Thus the shapes have to be
                // exported one by one
                UniReference< XMLShapeExport > rShapeExport = mrExport.GetShapeExport();
                Reference< drawing::XShape > xShape;
                const sal_Int32 nShapeCount( mxAdditionalShapes->getCount());
                for( sal_Int32 nShapeId = 0; nShapeId < nShapeCount; nShapeId++ )
                {
                    mxAdditionalShapes->getByIndex( nShapeId ) >>= xShape;
                    DBG_ASSERT( xShape.is(), "Shape without an XShape?" );
                    if( ! xShape.is())
                        continue;

                    rShapeExport->exportShape( xShape );
                }
                // this would be the easier way if it worked:
                //mrExport.GetShapeExport()->exportShapes( mxAdditionalShapes );
            }
        }
        else
        {
            // get a sequence of non-chart shapes (inserted via clipboard)
            try
            {
                Any aShapesAny = xDocPropSet->getPropertyValue( OUString::createFromAscii( "AdditionalShapes" ));
                aShapesAny >>= mxAdditionalShapes;
            }
            catch( uno::Exception & rEx )
            {
                (void)rEx; // avoid warning for pro build
                OSL_TRACE(
                    OUStringToOString(
                        OUString( RTL_CONSTASCII_USTRINGPARAM(
                            "AdditionalShapes not found: " )) +
                        rEx.Message,
                        RTL_TEXTENCODING_ASCII_US ).getStr());
            }

            if( mxAdditionalShapes.is())
            {
                // seek shapes has to be called for the whole page because in
                // the shape export the vector of shapes is accessed via the
                // ZOrder which might be (actually is) larger than the number of
                // shapes in mxAdditionalShapes
                Reference< drawing::XDrawPageSupplier > xSupplier( rChartDoc, uno::UNO_QUERY );
                DBG_ASSERT( xSupplier.is(), "Cannot retrieve draw page to initialize shape export" );
                if( xSupplier.is() )
                {
                    Reference< drawing::XShapes > xDrawPage( xSupplier->getDrawPage(), uno::UNO_QUERY );
                    DBG_ASSERT( xDrawPage.is(), "Invalid draw page for initializing shape export" );
                    if( xDrawPage.is())
                        mrExport.GetShapeExport()->seekShapes( xDrawPage );
                }

                // can't call collectShapesAutoStyles with all shapes because
                // the initialisation happend with the complete draw page and
                // not the XShapes object used here. Thus the shapes have to be
                // exported one by one
                UniReference< XMLShapeExport > rShapeExport = mrExport.GetShapeExport();
                Reference< drawing::XShape > xShape;
                const sal_Int32 nShapeCount( mxAdditionalShapes->getCount());
                for( sal_Int32 nShapeId = 0; nShapeId < nShapeCount; nShapeId++ )
                {
                    mxAdditionalShapes->getByIndex( nShapeId ) >>= xShape;
                    DBG_ASSERT( xShape.is(), "Shape without an XShape?" );
                    if( ! xShape.is())
                        continue;

                    rShapeExport->collectShapeAutoStyles( xShape );
                }
                // this would be the easier way if it worked:
                // mrExport.GetShapeExport()->collectShapesAutoStyles( mxAdditionalShapes );
            }
        }
    }

    // table element
    // (is included as subelement of chart)
    // ------------------------------------
    if( bExportContent )
    {
        // #85929# always export table, otherwise clipboard may loose data
        exportTable();
    }

    // close <chart:chart> element
    if( pElChart )
        delete pElChart;
}

void SchXMLExportHelper::exportTable()
{
    // table element
    // -------------
    mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_NAME, msTableName );
    SvXMLElementExport aTable( mrExport, XML_NAMESPACE_TABLE, XML_TABLE, sal_True, sal_True );

    bool bHasOwnData = false;
    Reference< chart2::XChartDocument > xNewDoc( mrExport.GetModel(), uno::UNO_QUERY );
    Reference< chart2::data::XRangeXMLConversion > xRangeConversion;
    if( xNewDoc.is())
    {
        bHasOwnData = xNewDoc->hasInternalDataProvider();
        xRangeConversion.set( xNewDoc->getDataProvider(), uno::UNO_QUERY );
    }

    lcl_TableData aData( lcl_getDataForLocalTable(
                             m_aDataSequencesToExport, mbHasCategoryLabels, !mbRowSourceColumns, bHasOwnData, xRangeConversion ));

    lcl_TableData::tStringContainer::const_iterator aDataRangeIter( aData.aDataRangeRepresentations.begin());
    const lcl_TableData::tStringContainer::const_iterator aDataRangeEndIter( aData.aDataRangeRepresentations.end());
    lcl_TableData::tStringContainer::const_iterator aFirstColumnRangeIter( aData.aFirstColumnRangeRepresentations.begin());
    const lcl_TableData::tStringContainer::const_iterator aFirstColumnRangeEndIter( aData.aFirstColumnRangeRepresentations.end());

    // declare columns
    {
        SvXMLElementExport aHeaderColumns( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_HEADER_COLUMNS, sal_True, sal_True );
        SvXMLElementExport aHeaderColumn( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, sal_True, sal_True );
    }
    {
        SvXMLElementExport aColumns( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMNS, sal_True, sal_True );
        mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED,
                               OUString::valueOf( static_cast< sal_Int64 >( aData.aFirstRowStrings.size())));
        SvXMLElementExport aColumn( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, sal_True, sal_True );
    }

    // export rows with content
    {
        SvXMLElementExport aHeaderRows( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_HEADER_ROWS, sal_True, sal_True );
        SvXMLElementExport aRow( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_ROW, sal_True, sal_True );
        {
            SvXMLElementExport aEmptyCell( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True );
            SvXMLElementExport aEmptyParagraph( mrExport, XML_NAMESPACE_TEXT, XML_P, sal_True, sal_True );
        }

        lcl_TableData::tStringContainer::const_iterator aFirstRowRangeIter( aData.aFirstRowRangeRepresentations.begin());
        const lcl_TableData::tStringContainer::const_iterator aFirstRowRangeEndIter( aData.aFirstRowRangeRepresentations.end());
        for( lcl_TableData::tStringContainer::const_iterator aIt( aData.aFirstRowStrings.begin());
             aIt != aData.aFirstRowStrings.end(); ++aIt )
        {
            mrExport.AddAttribute( XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_STRING );
            SvXMLElementExport aCell( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True );
            // write the original range name as id into the local table
            // to allow a correct re-association when copying via clipboard
            if( !bHasOwnData && aFirstRowRangeIter != aFirstRowRangeEndIter )
            {
                if( (*aFirstRowRangeIter).getLength())
                    mrExport.AddAttribute( XML_NAMESPACE_TEXT, XML_ID, *aFirstRowRangeIter );
                ++aFirstRowRangeIter;
            }
            exportText( *aIt );
        }
        OSL_ASSERT( bHasOwnData || aFirstRowRangeIter == aFirstRowRangeEndIter );
    } // closing row and header-rows elements

    // value rows
    {
        SvXMLElementExport aRows( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_ROWS, sal_True, sal_True );
        lcl_TableData::tStringContainer::const_iterator aFirstColIt( aData.aFirstColumnStrings.begin());
        for( lcl_TableData::tTwoDimNumberContainer::const_iterator aColIt( aData.aDataInRows.begin());
             aColIt != aData.aDataInRows.end(); ++aColIt )
        {
            SvXMLElementExport aRow( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_ROW, sal_True, sal_True );
            {
                mrExport.AddAttribute( XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_STRING );
                SvXMLElementExport aCell( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True );
                if( aFirstColIt != aData.aFirstColumnStrings.end())
                {
                    // write the original range name as id into the local table
                    // to allow a correct re-association when copying via clipboard
                    if( !bHasOwnData && aFirstColumnRangeIter != aFirstColumnRangeEndIter )
                        mrExport.AddAttribute( XML_NAMESPACE_TEXT, XML_ID, *aFirstColumnRangeIter++ );
                    exportText( *aFirstColIt );
                    ++aFirstColIt;
                }
            }
            for( lcl_TableData::tTwoDimNumberContainer::value_type::const_iterator aInnerIt( aColIt->begin());
                 aInnerIt != aColIt->end(); ++aInnerIt )
            {
                SvXMLUnitConverter::convertDouble( msStringBuffer, *aInnerIt );
                msString = msStringBuffer.makeStringAndClear();
                mrExport.AddAttribute( XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_FLOAT );
                mrExport.AddAttribute( XML_NAMESPACE_OFFICE, XML_VALUE, msString );
                SvXMLElementExport aCell( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True );
                // write the original range name as id into the local table to
                // allow a correct re-association when copying via clipboard
                if( ( !bHasOwnData && aDataRangeIter != aDataRangeEndIter ) &&
                    ( mbRowSourceColumns || (aInnerIt == aColIt->begin())) )
                {
                    if( (*aDataRangeIter).getLength())
                        mrExport.AddAttribute( XML_NAMESPACE_TEXT, XML_ID, *aDataRangeIter );
                    ++aDataRangeIter;
                }
                exportText( msString, false ); // do not convert tabs and lfs
            }
        }
    }

    // if range iterator was used it should have reached its end
    OSL_ASSERT( bHasOwnData || (aDataRangeIter == aDataRangeEndIter) );
    OSL_ASSERT( bHasOwnData || (aFirstColumnRangeIter == aFirstColumnRangeEndIter) );
}

void SchXMLExportHelper::exportPlotArea(
    Reference< chart::XDiagram > xDiagram,
    Reference< chart2::XDiagram > xNewDiagram,
    const awt::Size & rPageSize,
    sal_Bool bExportContent,
    sal_Bool bIncludeTable )
{
    DBG_ASSERT( xDiagram.is(), "Invalid XDiagram as parameter" );
    if( ! xDiagram.is())
        return;

    // variables for autostyles
    Reference< beans::XPropertySet > xPropSet;
    std::vector< XMLPropertyState > aPropertyStates;

    OUString aASName;
    sal_Bool bHasTwoYAxes = sal_False;
    sal_Bool bIs3DChart = sal_False;
    drawing::HomogenMatrix aTransMatrix;

    msStringBuffer.setLength( 0 );

    // plot-area element
    // -----------------

    SvXMLElementExport* pElPlotArea = 0;
    // get property states for autostyles
    xPropSet = Reference< beans::XPropertySet >( xDiagram, uno::UNO_QUERY );
    if( xPropSet.is())
    {
        if( mxExpPropMapper.is())
            aPropertyStates = mxExpPropMapper->Filter( xPropSet );
    }
    if( bExportContent )
    {
        UniReference< XMLShapeExport > rShapeExport;

        // write style name
        AddAutoStyleAttribute( aPropertyStates );

        if( msChartAddress.getLength() )
        {
            if( !bIncludeTable )
                mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, msChartAddress );

            Reference< chart::XChartDocument > xDoc( mrExport.GetModel(), uno::UNO_QUERY );
            if( xDoc.is() )
            {
                Reference< beans::XPropertySet > xDocProp( xDoc, uno::UNO_QUERY );
                if( xDocProp.is())
                {
                    Any aAny;
                    sal_Bool bFirstCol = false, bFirstRow = false;

                    try
                    {
                        aAny = xDocProp->getPropertyValue(
                            OUString( RTL_CONSTASCII_USTRINGPARAM( "DataSourceLabelsInFirstColumn" )));
                        aAny >>= bFirstCol;
                        aAny = xDocProp->getPropertyValue(
                            OUString( RTL_CONSTASCII_USTRINGPARAM( "DataSourceLabelsInFirstRow" )));
                        aAny >>= bFirstRow;

                        if( bFirstCol || bFirstRow )
                        {
                            mrExport.AddAttribute( XML_NAMESPACE_CHART,
                                                   ::xmloff::token::GetXMLToken( ::xmloff::token::XML_DATA_SOURCE_HAS_LABELS ),
                                                   ( bFirstCol
                                                     ? ( bFirstRow
                                                         ?  ::xmloff::token::GetXMLToken( ::xmloff::token::XML_BOTH )
                                                         :  ::xmloff::token::GetXMLToken( ::xmloff::token::XML_COLUMN ))
                                                     : ::xmloff::token::GetXMLToken( ::xmloff::token::XML_ROW )));
                        }
                    }
                    catch( beans::UnknownPropertyException & )
                    {
                        DBG_ERRORFILE( "Properties missing" );
                    }
                }
            }
        }

        // #i72973#, #144135# only export table-number-list in OOo format (also for binary)
        Reference< beans::XPropertySet > xExportInfo( mrExport.getExportInfo());
        if( msTableNumberList.getLength() &&
            xExportInfo.is())
        {
            try
            {
                OUString sExportTableNumListPropName( RTL_CONSTASCII_USTRINGPARAM("ExportTableNumberList"));
                Reference< beans::XPropertySetInfo > xInfo( xExportInfo->getPropertySetInfo());
                bool bExportTableNumberList = false;
                if( xInfo.is() && xInfo->hasPropertyByName( sExportTableNumListPropName ) &&
                    (xExportInfo->getPropertyValue( sExportTableNumListPropName ) >>= bExportTableNumberList) &&
                    bExportTableNumberList )
                {
                    // this attribute is for charts embedded in calc documents only.
                    // With this you are able to store a file again in 5.0 binary format
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_TABLE_NUMBER_LIST, msTableNumberList );
                }
            }
            catch( uno::Exception & rEx )
            {
#ifdef DBG_UTIL
                String aStr( rEx.Message );
                ByteString aBStr( aStr, RTL_TEXTENCODING_ASCII_US );
                DBG_ERROR1( "chart:TableNumberList property caught: %s", aBStr.GetBuffer());
#else
                (void)rEx; // avoid warning
#endif
            }
        }

        // attributes
        Reference< drawing::XShape > xShape ( xDiagram, uno::UNO_QUERY );
        if( xShape.is())
        {
            addPosition( xShape );
            addSize( xShape );
        }

        if( xPropSet.is())
        {
            Any aAny;
            try
            {
                aAny = xPropSet->getPropertyValue(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "HasSecondaryYAxis" )));
                aAny >>= bHasTwoYAxes;
            }
            catch( beans::UnknownPropertyException & )
            {
                DBG_ERROR( "Property HasSecondaryYAxis not found in Diagram" );
            }

            // 3d attributes
            try
            {
                aAny = xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Dim3D" )));
                aAny >>= bIs3DChart;

                if( bIs3DChart )
                {
                    rShapeExport = mrExport.GetShapeExport();
                    if( rShapeExport.is())
                        rShapeExport->export3DSceneAttributes( xPropSet );
                }
            }
            catch( uno::Exception & rEx )
            {
#ifdef DBG_UTIL
                String aStr( rEx.Message );
                ByteString aBStr( aStr, RTL_TEXTENCODING_ASCII_US );
                DBG_ERROR1( "chart:exportPlotAreaException caught: %s", aBStr.GetBuffer());
#else
                (void)rEx; // avoid warning
#endif
            }
        }

        // element
        pElPlotArea = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_PLOT_AREA, sal_True, sal_True );

        // light sources (inside plot area element)
        if( bIs3DChart &&
            rShapeExport.is())
            rShapeExport->export3DLamps( xPropSet );
    }
    else    // autostyles
    {
        CollectAutoStyle( aPropertyStates );
    }
    // remove property states for autostyles
    aPropertyStates.clear();

    // axis elements
    // -------------
    exportAxes( xDiagram, xNewDiagram, bExportContent );

    // series elements
    // ---------------
    exportSeries( xNewDiagram, rPageSize, bExportContent, bHasTwoYAxes );

    // stock-chart elements
    OUString sChartType ( xDiagram->getDiagramType());
    if( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.StockDiagram" )))
    {
        Reference< chart::XStatisticDisplay > xStockPropProvider( xDiagram, uno::UNO_QUERY );
        if( xStockPropProvider.is())
        {
            // stock-gain-marker
            Reference< beans::XPropertySet > xStockPropSet = xStockPropProvider->getUpBar();
            if( xStockPropSet.is())
            {
                aPropertyStates.clear();
                aPropertyStates = mxExpPropMapper->Filter( xStockPropSet );

                if( aPropertyStates.size() > 0 )
                {
                    if( bExportContent )
                    {
                        AddAutoStyleAttribute( aPropertyStates );

                        SvXMLElementExport aGain( mrExport, XML_NAMESPACE_CHART, XML_STOCK_GAIN_MARKER, sal_True, sal_True );
                    }
                    else
                    {
                        CollectAutoStyle( aPropertyStates );
                    }
                }
            }

            // stock-loss-marker
            xStockPropSet = xStockPropProvider->getDownBar();
            if( xStockPropSet.is())
            {
                aPropertyStates.clear();
                aPropertyStates = mxExpPropMapper->Filter( xStockPropSet );

                if( aPropertyStates.size() > 0 )
                {
                    if( bExportContent )
                    {
                        AddAutoStyleAttribute( aPropertyStates );

                        SvXMLElementExport aGain( mrExport, XML_NAMESPACE_CHART, XML_STOCK_LOSS_MARKER, sal_True, sal_True );
                    }
                    else
                    {
                        CollectAutoStyle( aPropertyStates );
                    }
                }
            }

            // stock-range-line
            xStockPropSet = xStockPropProvider->getMinMaxLine();
            if( xStockPropSet.is())
            {
                aPropertyStates.clear();
                aPropertyStates = mxExpPropMapper->Filter( xStockPropSet );

                if( aPropertyStates.size() > 0 )
                {
                    if( bExportContent )
                    {
                        AddAutoStyleAttribute( aPropertyStates );

                        SvXMLElementExport aGain( mrExport, XML_NAMESPACE_CHART, XML_STOCK_RANGE_LINE, sal_True, sal_True );
                    }
                    else
                    {
                        CollectAutoStyle( aPropertyStates );
                    }
                }
            }
        }
    }

    // wall and floor element
    // ----------------------

    Reference< chart::X3DDisplay > xWallFloorSupplier( xDiagram, uno::UNO_QUERY );
    if( mxExpPropMapper.is() &&
        xWallFloorSupplier.is())
    {
        // remove property states for autostyles
        aPropertyStates.clear();

        Reference< beans::XPropertySet > xWallPropSet( xWallFloorSupplier->getWall(), uno::UNO_QUERY );
        if( xWallPropSet.is())
        {
            aPropertyStates = mxExpPropMapper->Filter( xWallPropSet );

            if( aPropertyStates.size() > 0 )
            {
                // write element
                if( bExportContent )
                {
                    // add style name attribute
                    AddAutoStyleAttribute( aPropertyStates );

                    SvXMLElementExport aWall( mrExport, XML_NAMESPACE_CHART, XML_WALL, sal_True, sal_True );
                }
                else    // autostyles
                {
                    CollectAutoStyle( aPropertyStates );
                }
            }
        }

        // floor element
        // -------------

        // remove property states for autostyles
        aPropertyStates.clear();

        Reference< beans::XPropertySet > xFloorPropSet( xWallFloorSupplier->getFloor(), uno::UNO_QUERY );
        if( xFloorPropSet.is())
        {
            aPropertyStates = mxExpPropMapper->Filter( xFloorPropSet );

            if( aPropertyStates.size() > 0 )
            {
                // write element
                if( bExportContent )
                {
                    // add style name attribute
                    AddAutoStyleAttribute( aPropertyStates );

                    SvXMLElementExport aFloor( mrExport, XML_NAMESPACE_CHART, XML_FLOOR, sal_True, sal_True );
                }
                else    // autostyles
                {
                    CollectAutoStyle( aPropertyStates );
                }
            }
        }
    }

    if( pElPlotArea )
        delete pElPlotArea;
}

void SchXMLExportHelper::exportAxes(
    const Reference< chart::XDiagram > & xDiagram,
    const Reference< chart2::XDiagram > & xNewDiagram,
    sal_Bool bExportContent )
{
    DBG_ASSERT( xDiagram.is(), "Invalid XDiagram as parameter" );
    if( ! xDiagram.is())
        return;

    // variables for autostyles
    const OUString sNumFormat( OUString::createFromAscii( "NumberFormat" ));
    Reference< beans::XPropertySet > xPropSet;
    std::vector< XMLPropertyState > aPropertyStates;

    OUString aASName;

    // get some properties from document first
    sal_Bool bHasXAxis = sal_False,
        bHasYAxis = sal_False,
        bHasZAxis = sal_False,
        bHasSecondaryXAxis = sal_False,
        bHasSecondaryYAxis = sal_False;
    sal_Bool bHasXAxisTitle = sal_False,
        bHasYAxisTitle = sal_False,
        bHasZAxisTitle = sal_False,
        bHasSecondaryXAxisTitle = sal_False,
        bHasSecondaryYAxisTitle = sal_False;
    sal_Bool bHasXAxisMajorGrid = sal_False,
        bHasXAxisMinorGrid = sal_False,
        bHasYAxisMajorGrid = sal_False,
        bHasYAxisMinorGrid = sal_False,
        bHasZAxisMajorGrid = sal_False,
        bHasZAxisMinorGrid = sal_False;
    sal_Bool bIs3DChart = sal_False;

    // get multiple properties using XMultiPropertySet
    MultiPropertySetHandler aDiagramProperties (xDiagram);

    //  Check for supported services and then the properties provided by this service.
    Reference<lang::XServiceInfo> xServiceInfo (xDiagram, uno::UNO_QUERY);
    if (xServiceInfo.is())
    {
        if (xServiceInfo->supportsService(
            OUString::createFromAscii ("com.sun.star.chart.ChartAxisXSupplier")))
        {
            aDiagramProperties.Add (
                OUString(RTL_CONSTASCII_USTRINGPARAM("HasXAxis")), bHasXAxis);
        }
        if (xServiceInfo->supportsService(
            OUString::createFromAscii ("com.sun.star.chart.ChartAxisYSupplier")))
        {
            aDiagramProperties.Add (
                OUString(RTL_CONSTASCII_USTRINGPARAM("HasYAxis")), bHasYAxis);
        }
        if (xServiceInfo->supportsService(
            OUString::createFromAscii ("com.sun.star.chart.ChartAxisZSupplier")))
        {
            aDiagramProperties.Add (
                OUString(RTL_CONSTASCII_USTRINGPARAM("HasZAxis")), bHasZAxis);
        }
        if (xServiceInfo->supportsService(
            OUString::createFromAscii ("com.sun.star.chart.ChartTwoAxisXSupplier")))
        {
            aDiagramProperties.Add (
                OUString(RTL_CONSTASCII_USTRINGPARAM("HasSecondaryXAxis")), bHasSecondaryXAxis);
        }
        if (xServiceInfo->supportsService(
            OUString::createFromAscii ("com.sun.star.chart.ChartTwoAxisYSupplier")))
        {
            aDiagramProperties.Add (
                OUString(RTL_CONSTASCII_USTRINGPARAM("HasSecondaryYAxis")), bHasSecondaryYAxis);
        }
    }

    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasXAxisTitle")), bHasXAxisTitle);
    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasYAxisTitle")), bHasYAxisTitle);
    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasZAxisTitle")), bHasZAxisTitle);
    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasSecondaryXAxisTitle")), bHasSecondaryXAxisTitle);
    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasSecondaryYAxisTitle")), bHasSecondaryYAxisTitle);

    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasXAxisGrid")), bHasXAxisMajorGrid);
    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasYAxisGrid")), bHasYAxisMajorGrid);
    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasZAxisGrid")), bHasZAxisMajorGrid);

    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasXAxisHelpGrid")), bHasXAxisMinorGrid);
    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasYAxisHelpGrid")), bHasYAxisMinorGrid);
    aDiagramProperties.Add (
        OUString (RTL_CONSTASCII_USTRINGPARAM ("HasZAxisHelpGrid")), bHasZAxisMinorGrid);

    aDiagramProperties.Add(
        OUString (RTL_CONSTASCII_USTRINGPARAM ("Dim3D")), bIs3DChart);

    if ( ! aDiagramProperties.GetProperties ())
    {
        DBG_WARNING ("Required properties not found in Chart diagram");
    }

    SvXMLElementExport* pAxis = NULL;

    // x axis
    // -------

    // write axis element also if the axis itself is not visible, but a grid or
    // title
    Reference< chart::XAxisXSupplier > xAxisXSupp( xDiagram, uno::UNO_QUERY );
    if( xAxisXSupp.is())
    {
        bool bHasAxisProperties = false;
        // get property states for autostyles
        if( mxExpPropMapper.is())
        {
            xPropSet = xAxisXSupp->getXAxis();
            if( xPropSet.is())
            {
                bHasAxisProperties = true;
                lcl_exportNumberFormat( sNumFormat, xPropSet, mrExport );
                aPropertyStates = mxExpPropMapper->Filter( xPropSet );
            }
        }

        if( bHasXAxis ||
            bHasXAxisTitle || bHasXAxisMajorGrid || bHasXAxisMinorGrid ||
            mbHasCategoryLabels || bHasAxisProperties )
        {
            if( bExportContent )
            {
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_DIMENSION, XML_X );
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_NAME, XML_PRIMARY_X );

                // write style name
                AddAutoStyleAttribute( aPropertyStates );

                // element
                pAxis = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_AXIS, sal_True, sal_True );
            }
            else    // autostyles
            {
                CollectAutoStyle( aPropertyStates );
            }
            aPropertyStates.clear();

            // axis-title
            if( bHasXAxisTitle )
            {
                Reference< beans::XPropertySet > xTitleProp( xAxisXSupp->getXAxisTitle(), uno::UNO_QUERY );
                if( xTitleProp.is())
                {
                    aPropertyStates = mxExpPropMapper->Filter( xTitleProp );
                    if( bExportContent )
                    {
                        OUString aText;
                        Any aAny( xTitleProp->getPropertyValue(
                            OUString( RTL_CONSTASCII_USTRINGPARAM( "String" ))));
                        aAny >>= aText;

                        Reference< drawing::XShape > xShape( xTitleProp, uno::UNO_QUERY );
                        if( xShape.is())
                            addPosition( xShape );

                        AddAutoStyleAttribute( aPropertyStates );
                        SvXMLElementExport aTitle( mrExport, XML_NAMESPACE_CHART, XML_TITLE, sal_True, sal_True );

                        // paragraph containing title
                        exportText( aText );
                    }
                    else
                    {
                        CollectAutoStyle( aPropertyStates );
                    }
                    aPropertyStates.clear();
                }
            }

            // categories if we have a categories chart
            if( bExportContent &&
                mbHasCategoryLabels )
            {
                OUString aCategoriesRange;
                // fill msString with cell-range-address of categories
                // export own table references
                if( xNewDiagram.is())
                {
                    Reference< chart2::data::XLabeledDataSequence > xCategories( lcl_getCategories( xNewDiagram ) );
                    if( xCategories.is() )
                    {
                        Reference< chart2::data::XDataSequence > xValues( xCategories->getValues() );
                        if( xValues.is())
                        {
                            Reference< chart2::XChartDocument > xNewDoc( mrExport.GetModel(), uno::UNO_QUERY );
                            aCategoriesRange = lcl_ConvertRange( xValues->getSourceRangeRepresentation(), xNewDoc );
                            m_aDataSequencesToExport.push_back( tLabelValuesDataPair( 0, xValues ));
                        }
                    }
                }

                if( aCategoriesRange.getLength())
                    mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, aCategoriesRange );
                SvXMLElementExport aCategories( mrExport, XML_NAMESPACE_CHART, XML_CATEGORIES, sal_True, sal_True );
            }

            // grid
            Reference< beans::XPropertySet > xMajorGrid( xAxisXSupp->getXMainGrid(), uno::UNO_QUERY );
            if( bHasXAxisMajorGrid && xMajorGrid.is())
            {
                aPropertyStates = mxExpPropMapper->Filter( xMajorGrid );
                if( bExportContent )
                {
                    AddAutoStyleAttribute( aPropertyStates );
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, XML_MAJOR );
                    SvXMLElementExport aGrid( mrExport, XML_NAMESPACE_CHART, XML_GRID, sal_True, sal_True );
                }
                else
                {
                    CollectAutoStyle( aPropertyStates );
                }
                aPropertyStates.clear();
            }
            Reference< beans::XPropertySet > xMinorGrid( xAxisXSupp->getXHelpGrid(), uno::UNO_QUERY );
            if( bHasXAxisMinorGrid && xMinorGrid.is())
            {
                aPropertyStates = mxExpPropMapper->Filter( xMinorGrid );
                if( bExportContent )
                {
                    AddAutoStyleAttribute( aPropertyStates );
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, XML_MINOR );
                    SvXMLElementExport aGrid( mrExport, XML_NAMESPACE_CHART, XML_GRID, sal_True, sal_True );
                }
                else
                {
                    CollectAutoStyle( aPropertyStates );
                }
                aPropertyStates.clear();
            }
            if( pAxis )
            {
                delete pAxis;
                pAxis = NULL;
            }
        }
    }

    // secondary x axis
    if( bHasSecondaryXAxis || bHasSecondaryXAxisTitle )
    {
        Reference< chart::XTwoAxisXSupplier > xAxisTwoXSupp( xDiagram, uno::UNO_QUERY );
        if( xAxisTwoXSupp.is())
        {
            // get property states for autostyles
            if( mxExpPropMapper.is())
            {
                xPropSet = xAxisTwoXSupp->getSecondaryXAxis();
                lcl_exportNumberFormat( sNumFormat, xPropSet, mrExport );
                if( xPropSet.is())
                    aPropertyStates = mxExpPropMapper->Filter( xPropSet );
            }
            if( bExportContent )
            {
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_DIMENSION, XML_X );
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_NAME, XML_SECONDARY_X );
                AddAutoStyleAttribute( aPropertyStates );
                pAxis = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_AXIS, sal_True, sal_True );
            }
            else    // autostyles
            {
                CollectAutoStyle( aPropertyStates );
            }
            aPropertyStates.clear();

            if( bHasSecondaryXAxisTitle )
            {
                Reference< chart::XSecondAxisTitleSupplier > xAxisSupp( xDiagram, uno::UNO_QUERY );
                Reference< beans::XPropertySet > xTitleProp( xAxisSupp->getSecondXAxisTitle(), uno::UNO_QUERY );
                if( xTitleProp.is())
                {
                    aPropertyStates = mxExpPropMapper->Filter( xTitleProp );
                    if( bExportContent )
                    {
                        OUString aText;
                        Any aAny( xTitleProp->getPropertyValue(
                            OUString( RTL_CONSTASCII_USTRINGPARAM( "String" ))));
                        aAny >>= aText;

                        Reference< drawing::XShape > xShape( xTitleProp, uno::UNO_QUERY );
                        if( xShape.is())
                            addPosition( xShape );

                        AddAutoStyleAttribute( aPropertyStates );
                        SvXMLElementExport aTitle( mrExport, XML_NAMESPACE_CHART, XML_TITLE, sal_True, sal_True );

                        exportText( aText );
                    }
                    else
                    {
                        CollectAutoStyle( aPropertyStates );
                    }
                    aPropertyStates.clear();
                }
            }

            if( pAxis )
            {
                delete pAxis;
                pAxis = NULL;
            }
        }
    }

    // y axis
    // -------

    // write axis element also if the axis itself is not visible, but a grid or
    // title
    Reference< chart::XAxisYSupplier > xAxisYSupp( xDiagram, uno::UNO_QUERY );
    if( xAxisYSupp.is())
    {
        bool bHasAxisProperties = false;
        // get property states for autostyles
        if( mxExpPropMapper.is())
        {
            xPropSet = xAxisYSupp->getYAxis();
            if( xPropSet.is())
            {
                bHasAxisProperties = true;
                lcl_exportNumberFormat( sNumFormat, xPropSet, mrExport );
                aPropertyStates = mxExpPropMapper->Filter( xPropSet );
            }
        }

        if( bHasYAxis ||
            bHasYAxisTitle || bHasYAxisMajorGrid || bHasYAxisMinorGrid || bHasAxisProperties )
        {
            if( bExportContent )
            {
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_DIMENSION, XML_Y );
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_NAME, XML_PRIMARY_Y );
                AddAutoStyleAttribute( aPropertyStates );
                pAxis = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_AXIS, sal_True, sal_True );
            }
            else
            {
                CollectAutoStyle( aPropertyStates );
            }
            aPropertyStates.clear();

            // axis-title
            if( bHasYAxisTitle )
            {
                Reference< beans::XPropertySet > xTitleProp( xAxisYSupp->getYAxisTitle(), uno::UNO_QUERY );
                if( xTitleProp.is())
                {
                    aPropertyStates = mxExpPropMapper->Filter( xTitleProp );
                    if( bExportContent )
                    {
                        OUString aText;
                        Any aAny( xTitleProp->getPropertyValue(
                            OUString( RTL_CONSTASCII_USTRINGPARAM( "String" ))));
                        aAny >>= aText;

                        Reference< drawing::XShape > xShape( xTitleProp, uno::UNO_QUERY );
                        if( xShape.is())
                            addPosition( xShape );

                        AddAutoStyleAttribute( aPropertyStates );
                        SvXMLElementExport aTitle( mrExport, XML_NAMESPACE_CHART, XML_TITLE, sal_True, sal_True );

                        // paragraph containing title
                        exportText( aText );
                    }
                    else
                    {
                        CollectAutoStyle( aPropertyStates );
                    }
                    aPropertyStates.clear();
                }
            }

            // grid
            Reference< beans::XPropertySet > xMajorGrid( xAxisYSupp->getYMainGrid(), uno::UNO_QUERY );
            if( bHasYAxisMajorGrid && xMajorGrid.is())
            {
                aPropertyStates = mxExpPropMapper->Filter( xMajorGrid );

                if( bExportContent )
                {
                    AddAutoStyleAttribute( aPropertyStates );
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, XML_MAJOR );
                    SvXMLElementExport aGrid( mrExport, XML_NAMESPACE_CHART, XML_GRID, sal_True, sal_True );
                }
                else
                {
                    CollectAutoStyle( aPropertyStates );
                }
                aPropertyStates.clear();
            }
            // minor grid
            Reference< beans::XPropertySet > xMinorGrid( xAxisYSupp->getYHelpGrid(), uno::UNO_QUERY );
            if( bHasYAxisMinorGrid && xMinorGrid.is())
            {
                aPropertyStates = mxExpPropMapper->Filter( xMinorGrid );

                if( bExportContent )
                {
                    AddAutoStyleAttribute( aPropertyStates );
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, XML_MINOR );
                    SvXMLElementExport aGrid( mrExport, XML_NAMESPACE_CHART, XML_GRID, sal_True, sal_True );
                }
                else
                {
                    CollectAutoStyle( aPropertyStates );
                }
                aPropertyStates.clear();
            }
            if( pAxis )
            {
                delete pAxis;
                pAxis = NULL;
            }
        }
    }

    if( bHasSecondaryYAxis || bHasSecondaryYAxisTitle )
    {
        Reference< chart::XTwoAxisYSupplier > xAxisTwoYSupp( xDiagram, uno::UNO_QUERY );
        if( xAxisTwoYSupp.is())
        {
            // get property states for autostyles
            if( mxExpPropMapper.is())
            {
                xPropSet = xAxisTwoYSupp->getSecondaryYAxis();
                lcl_exportNumberFormat( sNumFormat, xPropSet, mrExport );
                if( xPropSet.is())
                    aPropertyStates = mxExpPropMapper->Filter( xPropSet );
            }
            if( bExportContent )
            {
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_DIMENSION, XML_Y );
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_NAME, XML_SECONDARY_Y );
                AddAutoStyleAttribute( aPropertyStates );
                pAxis = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_AXIS, sal_True, sal_True );
            }
            else    // autostyles
            {
                CollectAutoStyle( aPropertyStates );
            }
            aPropertyStates.clear();
            if( bHasSecondaryYAxisTitle )
            {
                Reference< chart::XSecondAxisTitleSupplier > xAxisSupp( xDiagram, uno::UNO_QUERY );
                Reference< beans::XPropertySet > xTitleProp( xAxisSupp->getSecondYAxisTitle(), uno::UNO_QUERY );
                if( xTitleProp.is())
                {
                    aPropertyStates = mxExpPropMapper->Filter( xTitleProp );
                    if( bExportContent )
                    {
                        OUString aText;
                        Any aAny( xTitleProp->getPropertyValue(
                            OUString( RTL_CONSTASCII_USTRINGPARAM( "String" ))));
                        aAny >>= aText;

                        Reference< drawing::XShape > xShape( xTitleProp, uno::UNO_QUERY );
                        if( xShape.is())
                            addPosition( xShape );

                        AddAutoStyleAttribute( aPropertyStates );
                        SvXMLElementExport aTitle( mrExport, XML_NAMESPACE_CHART, XML_TITLE, sal_True, sal_True );

                        exportText( aText );
                    }
                    else
                    {
                        CollectAutoStyle( aPropertyStates );
                    }
                    aPropertyStates.clear();
                }
            }

            if( pAxis )
            {
                delete pAxis;
                pAxis = NULL;
            }
        }
    }

    // z axis
    // -------

    if( bHasZAxis &&
        bIs3DChart )
    {
        Reference< chart::XAxisZSupplier > xAxisZSupp( xDiagram, uno::UNO_QUERY );
        if( xAxisZSupp.is())
        {
            // get property states for autostyles
            if( mxExpPropMapper.is())
            {
                xPropSet = xAxisZSupp->getZAxis();
                lcl_exportNumberFormat( sNumFormat, xPropSet, mrExport );
                if( xPropSet.is())
                    aPropertyStates = mxExpPropMapper->Filter( xPropSet );
            }
            if( bExportContent )
            {
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_DIMENSION, XML_Z );
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_NAME, XML_PRIMARY_Z );

                AddAutoStyleAttribute( aPropertyStates );
                pAxis = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_AXIS, sal_True, sal_True );
            }
            else
            {
                CollectAutoStyle( aPropertyStates );
            }
            aPropertyStates.clear();

            // axis-title
            if( bHasZAxisTitle )
            {
                Reference< beans::XPropertySet > xTitleProp( xAxisZSupp->getZAxisTitle(), uno::UNO_QUERY );
                if( xTitleProp.is())
                {
                    aPropertyStates = mxExpPropMapper->Filter( xTitleProp );
                    if( bExportContent )
                    {
                        OUString aText;
                        Any aAny( xTitleProp->getPropertyValue(
                            OUString( RTL_CONSTASCII_USTRINGPARAM( "String" ))));
                        aAny >>= aText;

                        Reference< drawing::XShape > xShape( xTitleProp, uno::UNO_QUERY );
                        if( xShape.is())
                            addPosition( xShape );

                        AddAutoStyleAttribute( aPropertyStates );
                        SvXMLElementExport aTitle( mrExport, XML_NAMESPACE_CHART, XML_TITLE, sal_True, sal_True );

                        // paragraph containing title
                        exportText( aText );
                    }
                    else
                    {
                        CollectAutoStyle( aPropertyStates );
                    }
                    aPropertyStates.clear();
                }
            }

            // grid
            Reference< beans::XPropertySet > xMajorGrid( xAxisZSupp->getZMainGrid(), uno::UNO_QUERY );
            if( bHasZAxisMajorGrid && xMajorGrid.is())
            {
                aPropertyStates = mxExpPropMapper->Filter( xMajorGrid );

                if( bExportContent )
                {
                    AddAutoStyleAttribute( aPropertyStates );
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, XML_MAJOR );
                    SvXMLElementExport aGrid( mrExport, XML_NAMESPACE_CHART, XML_GRID, sal_True, sal_True );
                }
                else
                {
                    CollectAutoStyle( aPropertyStates );
                }
                aPropertyStates.clear();
            }
            // minor grid
            Reference< beans::XPropertySet > xMinorGrid( xAxisZSupp->getZHelpGrid(), uno::UNO_QUERY );
            if( bHasZAxisMinorGrid && xMinorGrid.is())
            {
                aPropertyStates = mxExpPropMapper->Filter( xMinorGrid );

                if( bExportContent )
                {
                    AddAutoStyleAttribute( aPropertyStates );
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, XML_MINOR );
                    SvXMLElementExport aGrid( mrExport, XML_NAMESPACE_CHART, XML_GRID, sal_True, sal_True );
                }
                else
                {
                    CollectAutoStyle( aPropertyStates );
                }
                aPropertyStates.clear();
            }
        }
        if( pAxis )
        {
            delete pAxis;
            pAxis = NULL;
        }
    }
}

void SchXMLExportHelper::exportSeries(
    const Reference< chart2::XDiagram > & xNewDiagram,
    const awt::Size & rPageSize,
    sal_Bool bExportContent,
    sal_Bool bHasTwoYAxes )
{
    Reference< chart2::XCoordinateSystemContainer > xBCooSysCnt( xNewDiagram, uno::UNO_QUERY );
    if( ! xBCooSysCnt.is())
        return;
    Reference< chart2::XChartDocument > xNewDoc( mrExport.GetModel(), uno::UNO_QUERY );

    OUString aFirstXRange;

    std::vector< XMLPropertyState > aPropertyStates;

    const OUString sNumFormat( OUString::createFromAscii( "NumberFormat" ));
    const OUString sPercentageNumFormat( OUString::createFromAscii( "PercentageNumberFormat" ));

    Sequence< Reference< chart2::XCoordinateSystem > >
        aCooSysSeq( xBCooSysCnt->getCoordinateSystems());
    for( sal_Int32 nCSIdx=0; nCSIdx<aCooSysSeq.getLength(); ++nCSIdx )
    {
        Reference< chart2::XChartTypeContainer > xCTCnt( aCooSysSeq[nCSIdx], uno::UNO_QUERY );
        if( ! xCTCnt.is())
            continue;
        Sequence< Reference< chart2::XChartType > > aCTSeq( xCTCnt->getChartTypes());
        for( sal_Int32 nCTIdx=0; nCTIdx<aCTSeq.getLength(); ++nCTIdx )
        {
            Reference< chart2::XDataSeriesContainer > xDSCnt( aCTSeq[nCTIdx], uno::UNO_QUERY );
            if( ! xDSCnt.is())
                continue;
            // note: if xDSCnt.is() then also aCTSeq[nCTIdx]
            OUString aChartType( aCTSeq[nCTIdx]->getChartType());
            OUString aLabelRole = aCTSeq[nCTIdx]->getRoleOfSequenceForSeriesLabel();

            // special export for stock charts
            if( aChartType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart2.CandleStickChartType")))
            {
                sal_Bool bJapaneseCandleSticks = sal_False;
                Reference< beans::XPropertySet > xCTProp( aCTSeq[nCTIdx], uno::UNO_QUERY );
                if( xCTProp.is())
                    xCTProp->getPropertyValue( OUString::createFromAscii("Japanese")) >>= bJapaneseCandleSticks;
                exportCandleStickSeries(
                    xDSCnt->getDataSeries(), xNewDiagram, bJapaneseCandleSticks, bExportContent );
                continue;
            }

            // export dataseries for current chart-type
            Sequence< Reference< chart2::XDataSeries > > aSeriesSeq( xDSCnt->getDataSeries());
            for( sal_Int32 nSeriesIdx=0; nSeriesIdx<aSeriesSeq.getLength(); ++nSeriesIdx )
            {
                // export series
                Reference< chart2::data::XDataSource > xSource( aSeriesSeq[nSeriesIdx], uno::UNO_QUERY );
                if( xSource.is())
                {
                    SvXMLElementExport* pSeries = NULL;
                    Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeqCnt(
                        xSource->getDataSequences());
                    sal_Int32 nMainSequenceIndex = -1;
                    sal_Int32 nSeriesLength = 0;
                    sal_Int32 nSeqIdx=0;
                    sal_Int32 nAttachedAxis = chart::ChartAxisAssign::PRIMARY_Y;
                    sal_Bool bHasMeanValueLine = false;
                    chart::ChartRegressionCurveType eRegressionType( chart::ChartRegressionCurveType_NONE );
                    chart::ChartErrorIndicatorType eErrorType( chart::ChartErrorIndicatorType_NONE );
                    sal_Int32 nErrorBarStyle( chart::ErrorBarStyle::NONE );
                    Reference< beans::XPropertySet > xPropSet;
                    tLabelValuesDataPair aSeriesLabelValuesPair;

                    // search for main sequence and create a series element
                    {
                        Reference< chart2::data::XDataSequence > xValuesSeq;
                        Reference< chart2::data::XDataSequence > xLabelSeq;
                        for( ;nMainSequenceIndex==-1 && nSeqIdx<aSeqCnt.getLength();
                             ++nSeqIdx )
                        {
                            OUString aRole;
                            Reference< chart2::data::XDataSequence > xTempValueSeq( aSeqCnt[nSeqIdx]->getValues() );
                            Reference< beans::XPropertySet > xSeqProp( xTempValueSeq, uno::UNO_QUERY );
                            if( xSeqProp.is())
                                xSeqProp->getPropertyValue(OUString::createFromAscii("Role")) >>= aRole;
                            // "main" sequence
                            if( aRole.equals( aLabelRole ))
                            {
                                xValuesSeq.set( xTempValueSeq );
                                xLabelSeq.set( aSeqCnt[nSeqIdx]->getLabel());
                                // "main" sequence attributes exported, now go on with domains
                                nMainSequenceIndex = nSeqIdx;
                                break;
                            }
                        }

                        // have found the main sequence, then xValuesSeq and
                        // xLabelSeq contain those.  Otherwise both are empty
                        {
                            nSeriesLength = (xValuesSeq.is()? xValuesSeq->getData().getLength() : sal_Int32(0));
                            // get property states for autostyles
                            try
                            {
                                xPropSet = SchXMLSeriesHelper::createOldAPISeriesPropertySet(
                                    aSeriesSeq[nSeriesIdx], mrExport.GetModel() );
                            }
                            catch( uno::Exception & rEx )
                            {
                                (void)rEx; // avoid warning for pro build
                                OSL_TRACE(
                                    OUStringToOString(
                                        OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                      "Series not found or no XPropertySet: " )) +
                                        rEx.Message,
                                        RTL_TEXTENCODING_ASCII_US ).getStr());
                                continue;
                            }
                            if( xPropSet.is())
                            {
                                // determine attached axis
                                try
                                {
                                    Any aAny( xPropSet->getPropertyValue(
                                                  OUString( RTL_CONSTASCII_USTRINGPARAM( "Axis" ))));
                                    aAny >>= nAttachedAxis;

                                    aAny = xPropSet->getPropertyValue(
                                        OUString( RTL_CONSTASCII_USTRINGPARAM ( "MeanValue" )));
                                    aAny >>= bHasMeanValueLine;

                                    aAny = xPropSet->getPropertyValue(
                                        OUString( RTL_CONSTASCII_USTRINGPARAM( "RegressionCurves" )));
                                    aAny >>= eRegressionType;

                                    aAny = xPropSet->getPropertyValue(
                                        OUString( RTL_CONSTASCII_USTRINGPARAM( "ErrorIndicator" )));
                                    aAny >>= eErrorType;

                                    aAny = xPropSet->getPropertyValue(
                                        OUString( RTL_CONSTASCII_USTRINGPARAM( "ErrorBarStyle" )));
                                    aAny >>= nErrorBarStyle;
                                }
                                catch( beans::UnknownPropertyException & rEx )
                                {
                                    (void)rEx; // avoid warning for pro build
                                    OSL_TRACE(
                                        OUStringToOString(
                                            OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                          "Required property not found in DataRowProperties: " )) +
                                            rEx.Message,
                                            RTL_TEXTENCODING_ASCII_US ).getStr());
                                }

                                lcl_exportNumberFormat( sNumFormat, xPropSet, mrExport );
                                lcl_exportNumberFormat( sPercentageNumFormat, xPropSet, mrExport );

                                if( mxExpPropMapper.is())
                                    aPropertyStates = mxExpPropMapper->Filter( xPropSet );
                            }

                            if( bExportContent )
                            {
                                if( bHasTwoYAxes )
                                {
                                    if( nAttachedAxis == chart::ChartAxisAssign::SECONDARY_Y )
                                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_SECONDARY_Y );
                                    else
                                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_PRIMARY_Y );
                                }

                                // write style name
                                AddAutoStyleAttribute( aPropertyStates );

                                if( xValuesSeq.is())
                                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_VALUES_CELL_RANGE_ADDRESS,
                                                           lcl_ConvertRange(
                                                               xValuesSeq->getSourceRangeRepresentation(),
                                                               xNewDoc ));
                                else
                                    // #i75297# allow empty series, export empty range to have all ranges on import
                                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_VALUES_CELL_RANGE_ADDRESS, OUString());

                                if( xLabelSeq.is())
                                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LABEL_CELL_ADDRESS,
                                                           lcl_ConvertRange(
                                                               xLabelSeq->getSourceRangeRepresentation(),
                                                               xNewDoc ));
                                if( xLabelSeq.is() || xValuesSeq.is() )
                                    aSeriesLabelValuesPair = tLabelValuesDataPair( xLabelSeq, xValuesSeq );

                                // chart-type for mixed types
                                enum XMLTokenEnum eCTToken(
                                    SchXMLTools::getTokenByChartType( aChartType, false /* bUseOldNames */ ));
                                //@todo: get token for current charttype
                                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS,
                                                       mrExport.GetNamespaceMap().GetQNameByKey(
                                                           XML_NAMESPACE_CHART, GetXMLToken( eCTToken )));

                                // open series element until end of for loop
                                pSeries = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_SERIES, sal_True, sal_True );
                            }
                            else    // autostyles
                            {
                                CollectAutoStyle( aPropertyStates );
                            }
                            // remove property states for autostyles
                            aPropertyStates.clear();
                        }
                    }

                    // export domain elements if we have a series parent element
                    if( nMainSequenceIndex>-1 )
                    {
                        // domain elements
                        if( bExportContent )
                        {
                            for( nSeqIdx=0; nSeqIdx<aSeqCnt.getLength(); ++nSeqIdx )
                            {
                                if( nSeqIdx != nMainSequenceIndex )
                                {
                                    bool bExportDomain = false;
                                    //@todo: roles should be exported
                                    Reference< chart2::data::XDataSequence > xSeq( aSeqCnt[nSeqIdx]->getValues() );
                                    if( xSeq.is())
                                    {
                                        OUString aXRange( lcl_ConvertRange(
                                                    xSeq->getSourceRangeRepresentation(), xNewDoc ) );
                                        //work around error in OOo 2.0 (problems with multiple series having a domain element)
                                        if( !aFirstXRange.getLength() || !aXRange.equals(aFirstXRange) )
                                        {
                                            bExportDomain = true;
                                            mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, aXRange);
                                            m_aDataSequencesToExport.push_back( tLabelValuesDataPair( 0, xSeq ));
                                        }

                                        if( !aFirstXRange.getLength() )
                                            aFirstXRange = aXRange;
                                    }
//                                     xSeq.set( aSeqCnt[nSeqIdx]->getLabel());
//                                     if( xSeq.is())
//                                         mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LABEL_CELL_ADDRESS,
//                                                                lcl_ConvertRange(
//                                                                    xSeq->getSourceRangeRepresentation(),
//                                                                    xNewDoc ));
                                    if( bExportDomain )
                                    {
                                        SvXMLElementExport aDomain( mrExport, XML_NAMESPACE_CHART,
                                                                    XML_DOMAIN, sal_True, sal_True );
                                    }
                                }
                            }
                        }
                    }

                    // add sequences for main sequence after domain sequences,
                    // so that the export of the local table has the correct order
                    if( bExportContent &&
                        (aSeriesLabelValuesPair.first.is() || aSeriesLabelValuesPair.second.is()))
                        m_aDataSequencesToExport.push_back( aSeriesLabelValuesPair );

                    // statistical objects:
                    // regression curves and mean value lines
                    if( bHasMeanValueLine &&
                        xPropSet.is() &&
                        mxExpPropMapper.is() )
                    {
                        Reference< beans::XPropertySet > xStatProp;
                        try
                        {
                            Any aPropAny( xPropSet->getPropertyValue(
                                            OUString( RTL_CONSTASCII_USTRINGPARAM( "DataMeanValueProperties" ))));
                            aPropAny >>= xStatProp;
                        }
                        catch( uno::Exception & rEx )
                        {
                            (void)rEx; // avoid warning for pro build
                            DBG_ERROR1( "Exception caught during Export of series - optional DataMeanValueProperties not available: %s",
                                        OUStringToOString( rEx.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
                        }

                        if( xStatProp.is() )
                        {
                            aPropertyStates = mxExpPropMapper->Filter( xStatProp );

                            if( aPropertyStates.size() > 0 )
                            {
                                // write element
                                if( bExportContent )
                                {
                                    // add style name attribute
                                    AddAutoStyleAttribute( aPropertyStates );

                                    SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_MEAN_VALUE, sal_True, sal_True );
                                }
                                else    // autostyles
                                {
                                    CollectAutoStyle( aPropertyStates );
                                }
                            }
                        }
                    }

                    if( eRegressionType != chart::ChartRegressionCurveType_NONE &&
                        xPropSet.is() &&
                        mxExpPropMapper.is() )
                    {
                        exportRegressionCurve( aSeriesSeq[nSeriesIdx], xPropSet, rPageSize, bExportContent );
                    }

                    if( nErrorBarStyle != chart::ErrorBarStyle::NONE &&
                        eErrorType != chart::ChartErrorIndicatorType_NONE &&
                        xPropSet.is() &&
                        mxExpPropMapper.is() )
                    {
                        Reference< beans::XPropertySet > xStatProp;
                        try
                        {
                            Any aPropAny( xPropSet->getPropertyValue(
                                            OUString( RTL_CONSTASCII_USTRINGPARAM( "DataErrorProperties" ))));
                            aPropAny >>= xStatProp;
                        }
                        catch( uno::Exception & rEx )
                        {
                            (void)rEx; // avoid warning for pro build
                            DBG_ERROR1( "Exception caught during Export of series - optional DataErrorProperties not available: %s",
                                        OUStringToOString( rEx.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
                        }

                        if( xStatProp.is() )
                        {
                            if( bExportContent &&
                                nErrorBarStyle == chart::ErrorBarStyle::FROM_DATA )
                            {
                                // register data ranges for error bars for export in local table
                                ::std::vector< Reference< chart2::data::XDataSequence > > aErrorBarSequences(
                                    lcl_getErrorBarSequences( xStatProp ));
                                for( ::std::vector< Reference< chart2::data::XDataSequence > >::const_iterator aIt(
                                         aErrorBarSequences.begin()); aIt != aErrorBarSequences.end(); ++aIt )
                                {
                                    m_aDataSequencesToExport.push_back( tLabelValuesDataPair( 0, *aIt ));
                                }
                            }

                            aPropertyStates = mxExpPropMapper->Filter( xStatProp );

                            if( aPropertyStates.size() > 0 )
                            {
                                // write element
                                if( bExportContent )
                                {
                                    // add style name attribute
                                    AddAutoStyleAttribute( aPropertyStates );
                                    SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_ERROR_INDICATOR, sal_True, sal_True );
                                }
                                else    // autostyles
                                {
                                    CollectAutoStyle( aPropertyStates );
                                }
                            }
                        }
                    }

                    exportDataPoints(
                        uno::Reference< beans::XPropertySet >( aSeriesSeq[nSeriesIdx], uno::UNO_QUERY ),
                        nSeriesLength, xNewDiagram, bExportContent );

                    // close series element
                    if( pSeries )
                        delete pSeries;
                }
            }
            aPropertyStates.clear();
        }
    }
}

void SchXMLExportHelper::exportRegressionCurve(
    const Reference< chart2::XDataSeries > & xSeries,
    const Reference< beans::XPropertySet > & xSeriesProp,
    const awt::Size & rPageSize,
    sal_Bool bExportContent )
{
    OSL_ASSERT( mxExpPropMapper.is());

    std::vector< XMLPropertyState > aPropertyStates;
    std::vector< XMLPropertyState > aEquationPropertyStates;
    Reference< beans::XPropertySet > xStatProp;
    try
    {
        Any aPropAny( xSeriesProp->getPropertyValue(
                          OUString( RTL_CONSTASCII_USTRINGPARAM( "DataRegressionProperties" ))));
        aPropAny >>= xStatProp;
    }
    catch( uno::Exception & rEx )
    {
        (void)rEx; // avoid warning for pro build
        DBG_ERROR1( "Exception caught during Export of series - optional DataRegressionProperties not available: %s",
                    OUStringToOString( rEx.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
    }

    if( xStatProp.is() )
    {
        Reference< chart2::XRegressionCurve > xRegCurve( SchXMLTools::getRegressionCurve( xSeries ));
        Reference< beans::XPropertySet > xEquationProperties;
        if( xRegCurve.is())
            xEquationProperties.set( xRegCurve->getEquationProperties());

        bool bShowEquation = false;
        bool bShowRSquared = false;
        bool bExportEquation = false;
        aPropertyStates = mxExpPropMapper->Filter( xStatProp );
        if( xEquationProperties.is())
        {
            xEquationProperties->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "ShowEquation" )))
                >>= bShowEquation;
            xEquationProperties->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "ShowCorrelationCoefficient" )))
                >>= bShowRSquared;
            bExportEquation = ( bShowEquation || bShowRSquared );
            if( bExportEquation )
            {
                // number format
                sal_Int32 nNumberFormat = 0;
                if( ( xEquationProperties->getPropertyValue(
                          OUString( RTL_CONSTASCII_USTRINGPARAM( "NumberFormat" ))) >>= nNumberFormat ) &&
                    nNumberFormat != -1 )
                {
                    mrExport.addDataStyle( nNumberFormat );
                }
                aEquationPropertyStates = mxExpPropMapper->Filter( xEquationProperties );
            }
        }

        if( !aPropertyStates.empty() || bExportEquation )
        {
            // write element
            if( bExportContent )
            {
                // add style name attribute
                if( !aPropertyStates.empty())
                    AddAutoStyleAttribute( aPropertyStates );
                SvXMLElementExport aRegressionExport( mrExport, XML_NAMESPACE_CHART, XML_REGRESSION_CURVE, sal_True, sal_True );
                if( bExportEquation )
                {
                    // default is true
                    if( !bShowEquation )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_DISPLAY_EQUATION, XML_FALSE );
                    // default is false
                    if( bShowRSquared )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_DISPLAY_R_SQUARE, XML_TRUE );

                    // export position
                    chart2::RelativePosition aRelativePosition;
                    if( xEquationProperties->getPropertyValue(
                            OUString( RTL_CONSTASCII_USTRINGPARAM("RelativePosition"))) >>= aRelativePosition )
                    {
                        double fX = aRelativePosition.Primary * rPageSize.Width;
                        double fY = aRelativePosition.Secondary * rPageSize.Height;
                        awt::Point aPos;
                        aPos.X = static_cast< sal_Int32 >( ::rtl::math::round( fX ));
                        aPos.Y = static_cast< sal_Int32 >( ::rtl::math::round( fY ));
                        addPosition( aPos );
                    }

                    if( !aEquationPropertyStates.empty())
                        AddAutoStyleAttribute( aEquationPropertyStates );

                    SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_EQUATION, sal_True, sal_True );
                }
            }
            else    // autostyles
            {
                if( !aPropertyStates.empty())
                    CollectAutoStyle( aPropertyStates );
                if( bExportEquation && !aEquationPropertyStates.empty())
                    CollectAutoStyle( aEquationPropertyStates );
            }
        }
    }
}

void SchXMLExportHelper::exportCandleStickSeries(
    const Sequence< Reference< chart2::XDataSeries > > & aSeriesSeq,
    const Reference< chart2::XDiagram > & xDiagram,
    sal_Bool bJapaneseCandleSticks,
    sal_Bool bExportContent )
{
//  std::vector< XMLPropertyState > aPropertyStates;

    for( sal_Int32 nSeriesIdx=0; nSeriesIdx<aSeriesSeq.getLength(); ++nSeriesIdx )
    {
        Reference< chart2::XDataSeries > xSeries( aSeriesSeq[nSeriesIdx] );
        sal_Int32 nAttachedAxis = lcl_isSeriesAttachedToFirstAxis( xSeries )
            ? chart::ChartAxisAssign::PRIMARY_Y
            : chart::ChartAxisAssign::SECONDARY_Y;

        Reference< chart2::data::XDataSource > xSource( xSeries, uno::UNO_QUERY );
        if( xSource.is())
        {
            // export series in correct order (as we don't store roles)
            // with japanese candlesticks: open, low, high, close
            // otherwise: low, high, close
            Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeqCnt(
                xSource->getDataSequences());

            sal_Int32 nSeriesLength =
                lcl_getSequenceLengthByRole( aSeqCnt, OUString::createFromAscii("values-last"));

            if( bExportContent )
            {
                Reference< chart2::XChartDocument > xNewDoc( mrExport.GetModel(), uno::UNO_QUERY );
                //@todo: export data points

                // open
                if( bJapaneseCandleSticks )
                {
                    tLabelAndValueRange aRanges( lcl_getLabelAndValueRangeByRole(
                        aSeqCnt, OUString::createFromAscii("values-first"),  xNewDoc, m_aDataSequencesToExport ));
                    if( aRanges.second.getLength())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_VALUES_CELL_RANGE_ADDRESS, aRanges.second );
                    if( aRanges.first.getLength())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LABEL_CELL_ADDRESS, aRanges.first );
                    if( nAttachedAxis == chart::ChartAxisAssign::SECONDARY_Y )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_SECONDARY_Y );
                    else
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_PRIMARY_Y );
                    SvXMLElementExport aOpenSeries( mrExport, XML_NAMESPACE_CHART, XML_SERIES, sal_True, sal_True );
                    // export empty data points
                    exportDataPoints( 0, nSeriesLength, xDiagram, bExportContent );
                }

                // low
                {
                    tLabelAndValueRange aRanges( lcl_getLabelAndValueRangeByRole(
                        aSeqCnt, OUString::createFromAscii("values-min"),  xNewDoc, m_aDataSequencesToExport ));
                    if( aRanges.second.getLength())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_VALUES_CELL_RANGE_ADDRESS, aRanges.second );
                    if( aRanges.first.getLength())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LABEL_CELL_ADDRESS, aRanges.first );
                    if( nAttachedAxis == chart::ChartAxisAssign::SECONDARY_Y )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_SECONDARY_Y );
                    else
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_PRIMARY_Y );
                    SvXMLElementExport aLowSeries( mrExport, XML_NAMESPACE_CHART, XML_SERIES, sal_True, sal_True );
                    // export empty data points
                    exportDataPoints( 0, nSeriesLength, xDiagram, bExportContent );
                }

                // high
                {
                    tLabelAndValueRange aRanges( lcl_getLabelAndValueRangeByRole(
                        aSeqCnt, OUString::createFromAscii("values-max"),  xNewDoc, m_aDataSequencesToExport ));
                    if( aRanges.second.getLength())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_VALUES_CELL_RANGE_ADDRESS, aRanges.second );
                    if( aRanges.first.getLength())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LABEL_CELL_ADDRESS, aRanges.first );
                    if( nAttachedAxis == chart::ChartAxisAssign::SECONDARY_Y )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_SECONDARY_Y );
                    else
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_PRIMARY_Y );
                    SvXMLElementExport aHighSeries( mrExport, XML_NAMESPACE_CHART, XML_SERIES, sal_True, sal_True );
                    // export empty data points
                    exportDataPoints( 0, nSeriesLength, xDiagram, bExportContent );
                }

                // close
                {
                    tLabelAndValueRange aRanges( lcl_getLabelAndValueRangeByRole(
                        aSeqCnt, OUString::createFromAscii("values-last"),  xNewDoc, m_aDataSequencesToExport ));
                    if( aRanges.second.getLength())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_VALUES_CELL_RANGE_ADDRESS, aRanges.second );
                    if( aRanges.first.getLength())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LABEL_CELL_ADDRESS, aRanges.first );
                    if( nAttachedAxis == chart::ChartAxisAssign::SECONDARY_Y )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_SECONDARY_Y );
                    else
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_PRIMARY_Y );
                    // write style name
//                     AddAutoStyleAttribute( aPropertyStates );
                    // chart type
//                     mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS,
//                                            mrExport.GetNamespaceMap().GetQNameByKey(
//                                                XML_NAMESPACE_CHART, GetXMLToken( XML_STOCK )));
                    SvXMLElementExport aCloseSeries( mrExport, XML_NAMESPACE_CHART, XML_SERIES, sal_True, sal_True );
                    // export empty data points
                    exportDataPoints( 0, nSeriesLength, xDiagram, bExportContent );
                }
            }
            else    // autostyles
            {
                // for close series
//                 CollectAutoStyle( aPropertyStates );
            }
            // remove property states for autostyles
//             aPropertyStates.clear();
        }
    }
}

void SchXMLExportHelper::exportDataPoints(
    const uno::Reference< beans::XPropertySet > & xSeriesProperties,
    sal_Int32 nSeriesLength,
    const uno::Reference< chart2::XDiagram > & xDiagram,
    sal_Bool bExportContent )
{
    // data-points
    // -----------
    // write data-points only if they contain autostyles
    // objects with equal autostyles are grouped using the attribute
    // repeat="number"

    // Note: if only the nth data-point has autostyles there is an element
    // without style and repeat="n-1" attribute written in advance.

    // the sequence aDataPointSeq contains indices of data-points that
    // do have own attributes.  This increases the performance substantially.

    // more performant version for #93600#
    if( mxExpPropMapper.is())
    {
        uno::Reference< chart2::XDataSeries > xSeries( xSeriesProperties, uno::UNO_QUERY );

        std::vector< XMLPropertyState > aPropertyStates;

        const OUString sNumFormat( OUString::createFromAscii( "NumberFormat" ));
        const OUString sPercentageNumFormat( OUString::createFromAscii( "PercentageNumberFormat" ));

        bool bVaryColorsByPoint = false;
        Sequence< sal_Int32 > aDataPointSeq;
        if( xSeriesProperties.is())
        {
            Any aAny = xSeriesProperties->getPropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "AttributedDataPoints" )));
            aAny >>= aDataPointSeq;
            xSeriesProperties->getPropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "VaryColorsByPoint" ))) >>= bVaryColorsByPoint;
        }


        sal_Int32 nSize = aDataPointSeq.getLength();
        DBG_ASSERT( nSize <= nSeriesLength, "Too many point attributes" );

        const sal_Int32 * pPoints = aDataPointSeq.getConstArray();
        sal_Int32 nElement;
        sal_Int32 nRepeat;
        Reference< chart2::XColorScheme > xColorScheme;
        if( xDiagram.is())
            xColorScheme.set( xDiagram->getDefaultColorScheme());

        ::std::list< SchXMLDataPointStruct > aDataPointList;

        sal_Int32 nLastIndex = -1;
        sal_Int32 nCurrIndex = 0;

        // collect elements
        if( bVaryColorsByPoint && xColorScheme.is() )
        {
            ::std::set< sal_Int32 > aAttrPointSet;
            ::std::copy( pPoints, pPoints + aDataPointSeq.getLength(),
                            ::std::inserter( aAttrPointSet, aAttrPointSet.begin()));
            const ::std::set< sal_Int32 >::const_iterator aEndIt( aAttrPointSet.end());
            for( nElement = 0; nElement < nSeriesLength; ++nElement )
            {
                aPropertyStates.clear();
                uno::Reference< beans::XPropertySet > xPropSet;
                if( aAttrPointSet.find( nElement ) != aEndIt )
                {
                    try
                    {
                        xPropSet = SchXMLSeriesHelper::createOldAPIDataPointPropertySet(
                                    xSeries, nElement, mrExport.GetModel() );
                    }
                    catch( uno::Exception & rEx )
                    {
                        (void)rEx; // avoid warning for pro build
                        DBG_ERROR1( "Exception caught during Export of data point: %s",
                                    OUStringToOString( rEx.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
                    }
                }
                else
                {
                    // property set only containing the color
                    xPropSet.set( new ::xmloff::chart::ColorPropertySet(
                                        xColorScheme->getColorByIndex( nElement )));
                }
                DBG_ASSERT( xPropSet.is(), "Pie Segments should have properties" );
                if( xPropSet.is())
                {
                    lcl_exportNumberFormat( sNumFormat, xPropSet, mrExport );
                    lcl_exportNumberFormat( sPercentageNumFormat, xPropSet, mrExport );

                    aPropertyStates = mxExpPropMapper->Filter( xPropSet );
                    if( aPropertyStates.size() > 0 )
                    {
                        if( bExportContent )
                        {
                            // write data-point with style
                            DBG_ASSERT( ! maAutoStyleNameQueue.empty(), "Autostyle queue empty!" );

                            SchXMLDataPointStruct aPoint;
                            aPoint.maStyleName = maAutoStyleNameQueue.front();
                            maAutoStyleNameQueue.pop();
                            aDataPointList.push_back( aPoint );
                        }
                        else
                        {
                            CollectAutoStyle( aPropertyStates );
                        }
                    }
                }
            }
            DBG_ASSERT( static_cast<sal_Int32>(aDataPointList.size()) == nSeriesLength, "not enough data points" );
        }
        else
        {
            for( nElement = 0; nElement < nSize; ++nElement )
            {
                aPropertyStates.clear();
                nCurrIndex = pPoints[ nElement ];
                //assuming sorted indices in pPoints

                if( nCurrIndex<0 || nCurrIndex>=nSeriesLength )
                    break;

                // write leading empty data points
                if( nCurrIndex - nLastIndex > 1 )
                {
                    SchXMLDataPointStruct aPoint;
                    aPoint.mnRepeat = nCurrIndex - nLastIndex - 1;
                    aDataPointList.push_back( aPoint );
                }

                uno::Reference< beans::XPropertySet > xPropSet;
                // get property states
                try
                {
                    xPropSet = SchXMLSeriesHelper::createOldAPIDataPointPropertySet(
                                    xSeries, nCurrIndex, mrExport.GetModel() );
                }
                catch( uno::Exception & rEx )
                {
                    (void)rEx; // avoid warning for pro build
                    DBG_ERROR1( "Exception caught during Export of data point: %s",
                                OUStringToOString( rEx.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
                }
                if( xPropSet.is())
                {
                    lcl_exportNumberFormat( sNumFormat, xPropSet, mrExport );
                    lcl_exportNumberFormat( sPercentageNumFormat, xPropSet, mrExport );

                    aPropertyStates = mxExpPropMapper->Filter( xPropSet );
                    if( aPropertyStates.size() > 0 )
                    {
                        if( bExportContent )
                        {
                            // write data-point with style
                            DBG_ASSERT( ! maAutoStyleNameQueue.empty(), "Autostyle queue empty!" );
                            SchXMLDataPointStruct aPoint;
                            aPoint.maStyleName = maAutoStyleNameQueue.front();
                            maAutoStyleNameQueue.pop();

                            aDataPointList.push_back( aPoint );
                            nLastIndex = nCurrIndex;
                        }
                        else
                        {
                            CollectAutoStyle( aPropertyStates );
                        }
                        continue;
                    }
                }

                // if we get here the property states are empty
                SchXMLDataPointStruct aPoint;
                aDataPointList.push_back( aPoint );

                nLastIndex = nCurrIndex;
            }
            // final empty elements
            nRepeat = nSeriesLength - nLastIndex - 1;
            if( nRepeat > 0 )
            {
                SchXMLDataPointStruct aPoint;
                aPoint.mnRepeat = nRepeat;
                aDataPointList.push_back( aPoint );
            }
        }

        if( bExportContent )
        {
            // write elements (merge equal ones)
            ::std::list< SchXMLDataPointStruct >::iterator aIter = aDataPointList.begin();
            SchXMLDataPointStruct aPoint;
            SchXMLDataPointStruct aLastPoint;

            // initialize so that it doesn't matter if
            // the element is counted in the first iteration
            aLastPoint.mnRepeat = 0;

            for( ; aIter != aDataPointList.end(); ++aIter )
            {
                aPoint = (*aIter);

                if( aPoint.maStyleName == aLastPoint.maStyleName )
                    aPoint.mnRepeat += aLastPoint.mnRepeat;
                else if( aLastPoint.mnRepeat > 0 )
                {
                    // write last element
                    if( aLastPoint.maStyleName.getLength() )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_STYLE_NAME, aLastPoint.maStyleName );

                    if( aLastPoint.mnRepeat > 1 )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_REPEATED,
                                            OUString::valueOf( (sal_Int64)( aLastPoint.mnRepeat ) ));

                    SvXMLElementExport aPointElem( mrExport, XML_NAMESPACE_CHART, XML_DATA_POINT, sal_True, sal_True );
                }
                aLastPoint = aPoint;
            }
            // write last element if it hasn't been written in last iteration
            if( aPoint.maStyleName == aLastPoint.maStyleName )
            {
                if( aLastPoint.maStyleName.getLength() )
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_STYLE_NAME, aLastPoint.maStyleName );

                if( aLastPoint.mnRepeat > 1 )
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_REPEATED,
                                        OUString::valueOf( (sal_Int64)( aLastPoint.mnRepeat ) ));

                SvXMLElementExport aPointElem( mrExport, XML_NAMESPACE_CHART, XML_DATA_POINT, sal_True, sal_True );
            }
        }
    }
}


void SchXMLExportHelper::getCellAddress( sal_Int32 nCol, sal_Int32 nRow )
{
    msStringBuffer.append( (sal_Unicode)'.' );
    if( nCol < 26 )
        msStringBuffer.append( (sal_Unicode)('A' + nCol) );
    else if( nCol < 702 )
    {
        msStringBuffer.append( (sal_Unicode)('A' + nCol / 26 - 1 ));
        msStringBuffer.append( (sal_Unicode)('A' + nCol % 26) );
    }
    else
    {
        msStringBuffer.append( (sal_Unicode)('A' + nCol / 702 - 1 ));
        msStringBuffer.append( (sal_Unicode)('A' + (nCol % 702) / 26 ));
        msStringBuffer.append( (sal_Unicode)('A' + nCol % 26) );
    }

    msStringBuffer.append( nRow + (sal_Int32)1 );
}

void SchXMLExportHelper::addPosition( const awt::Point & rPosition )
{
    mrExport.GetMM100UnitConverter().convertMeasure( msStringBuffer, rPosition.X );
    msString = msStringBuffer.makeStringAndClear();
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_X, msString );

    mrExport.GetMM100UnitConverter().convertMeasure( msStringBuffer, rPosition.Y );
    msString = msStringBuffer.makeStringAndClear();
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_Y, msString );
}

void SchXMLExportHelper::addPosition( Reference< drawing::XShape > xShape )
{
    if( xShape.is())
        addPosition( xShape->getPosition());
}

void SchXMLExportHelper::addSize( const awt::Size & rSize )
{
    mrExport.GetMM100UnitConverter().convertMeasure( msStringBuffer, rSize.Width );
    msString = msStringBuffer.makeStringAndClear();
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_WIDTH,  msString );

    mrExport.GetMM100UnitConverter().convertMeasure( msStringBuffer, rSize.Height );
    msString = msStringBuffer.makeStringAndClear();
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_HEIGHT, msString );
}

void SchXMLExportHelper::addSize( Reference< drawing::XShape > xShape )
{
    if( xShape.is())
        addSize( xShape->getSize() );
}

awt::Size SchXMLExportHelper::getPageSize( const Reference< chart2::XChartDocument > & xChartDoc ) const
{
    awt::Size aSize( 8000, 7000 );
    uno::Reference< embed::XVisualObject > xVisualObject( xChartDoc, uno::UNO_QUERY );
    DBG_ASSERT( xVisualObject.is(),"need XVisualObject for page size" );
    if( xVisualObject.is() )
        aSize = xVisualObject->getVisualAreaSize( embed::Aspects::MSOLE_CONTENT );

    return aSize;
}

void SchXMLExportHelper::swapDataArray( Sequence< Sequence< double > >& rSequence )
{
    sal_Int32 nOuterSize = rSequence.getLength();
    sal_Int32 nInnerSize = rSequence[0].getLength();    // assume that all subsequences have same length
    sal_Int32 i, o;

    Sequence< Sequence< double > > aResult( nInnerSize );
    Sequence< double >* pArray = aResult.getArray();
    for( i = 0; i < nInnerSize; i++ )
    {
        pArray[ i ].realloc( nOuterSize );
        for( o = 0 ; o < nOuterSize ; o++ )
            aResult[ i ][ o ] = rSequence[ o ][ i ];
    }

    rSequence = aResult;
}

void SchXMLExportHelper::CollectAutoStyle( const std::vector< XMLPropertyState >& aStates )
{
    if( aStates.size())
        maAutoStyleNameQueue.push( GetAutoStylePoolP().Add( XML_STYLE_FAMILY_SCH_CHART_ID, aStates ));
}

void SchXMLExportHelper::AddAutoStyleAttribute( const std::vector< XMLPropertyState >& aStates )
{
    if( aStates.size())
    {
        DBG_ASSERT( ! maAutoStyleNameQueue.empty(), "Autostyle queue empty!" );

        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_STYLE_NAME,  maAutoStyleNameQueue.front() );
        maAutoStyleNameQueue.pop();
    }
}

void SchXMLExportHelper::exportText( const OUString& rText, bool bConvertTabsLFs )
{
    SchXMLTools::exportText( mrExport, rText, bConvertTabsLFs );
}

// ========================================
// class SchXMLExport
// ========================================

// #110680#
SchXMLExport::SchXMLExport(
    const Reference< lang::XMultiServiceFactory >& xServiceFactory,
    sal_uInt16 nExportFlags )
:   SvXMLExport( xServiceFactory, MAP_CM, ::xmloff::token::XML_CHART, nExportFlags ),
    maAutoStylePool( *this ),
    maExportHelper( *this, maAutoStylePool )
{
}


SchXMLExport::~SchXMLExport()
{
    // stop progress view
    if( mxStatusIndicator.is())
    {
        mxStatusIndicator->end();
        mxStatusIndicator->reset();
    }
}

sal_uInt32 SchXMLExport::exportDoc( enum ::xmloff::token::XMLTokenEnum eClass )
{
    Reference< chart2::XChartDocument > xChartDoc( GetModel(), uno::UNO_QUERY );
    maExportHelper.InitRangeSegmentationProperties( xChartDoc );
    return SvXMLExport::exportDoc( eClass );
}

void SchXMLExport::_ExportStyles( sal_Bool bUsed )
{
    SvXMLExport::_ExportStyles( bUsed );
}

void SchXMLExport::_ExportMasterStyles()
{
    // not available in chart
    DBG_WARNING( "Master Style Export requested. Not available for Chart" );
}

void SchXMLExport::_ExportAutoStyles()
{
    // there are no styles that require their own autostyles
    if( getExportFlags() & EXPORT_CONTENT )
    {
        Reference< chart::XChartDocument > xChartDoc( GetModel(), uno::UNO_QUERY );
        if( xChartDoc.is())
        {
            maExportHelper.collectAutoStyles( xChartDoc );
            maExportHelper.exportAutoStyles();
        }
        else
        {
            DBG_ERROR( "Couldn't export chart due to wrong XModel (must be XChartDocument)" );
        }
    }
}

void SchXMLExport::_ExportContent()
{
    Reference< chart::XChartDocument > xChartDoc( GetModel(), uno::UNO_QUERY );
    if( xChartDoc.is())
    {
        // determine if data comes from the outside
        sal_Bool bIncludeTable = sal_True;

        Reference< chart2::XChartDocument > xNewDoc( xChartDoc, uno::UNO_QUERY );
        if( xNewDoc.is())
        {
            // check if we have own data.  If so we must not export the complete
            // range string, as this is our only indicator for having own or
            // external data. @todo: fix this in the file format!
            Reference< lang::XServiceInfo > xDPServiceInfo( maExportHelper.GetDataProvider( xNewDoc ), uno::UNO_QUERY );
            if( ! (xDPServiceInfo.is() &&
                   xDPServiceInfo->getImplementationName().equalsAsciiL(
                       RTL_CONSTASCII_STRINGPARAM( "com.sun.star.comp.chart.InternalDataProvider" ))))
            {
                bIncludeTable = sal_False;
            }
        }
        else
        {
            Reference< lang::XServiceInfo > xServ( xChartDoc, uno::UNO_QUERY );
            if( xServ.is())
            {
                if( xServ->supportsService(
                        OUString::createFromAscii( "com.sun.star.chart.ChartTableAddressSupplier" )))
                {
                    Reference< beans::XPropertySet > xProp( xServ, uno::UNO_QUERY );
                    if( xProp.is())
                    {
                        Any aAny;
                        try
                        {
                            OUString sChartAddress;
                            aAny = xProp->getPropertyValue(
                                OUString::createFromAscii( "ChartRangeAddress" ));
                            aAny >>= sChartAddress;
                            maExportHelper.SetChartRangeAddress( sChartAddress );

                            OUString sTableNumberList;
                            aAny = xProp->getPropertyValue(
                                OUString::createFromAscii( "TableNumberList" ));
                            aAny >>= sTableNumberList;
                            maExportHelper.SetTableNumberList( sTableNumberList );

                            // do not include own table if there are external addresses
                            bIncludeTable = (sChartAddress.getLength() == 0);
                        }
                        catch( beans::UnknownPropertyException & )
                        {
                            DBG_ERROR( "Property ChartRangeAddress not supported by ChartDocument" );
                        }
                    }
                }
            }
        }
        maExportHelper.exportChart( xChartDoc, bIncludeTable );
    }
    else
    {
        DBG_ERROR( "Couldn't export chart due to wrong XModel" );
    }
}

void SchXMLExport::SetProgress( sal_Int32 nPercentage )
{
    // set progress view
    if( mxStatusIndicator.is())
        mxStatusIndicator->setValue( nPercentage );
}

// static
Reference< chart2::data::XDataProvider > SchXMLExportHelper::GetDataProvider(
    const Reference< chart2::XChartDocument > & xDoc )
{
    Reference< chart2::data::XDataProvider > xResult;
    if( xDoc.is())
    {
        xResult.set( xDoc->getDataProvider());
        // allowed to attach a new data provider in export?
//         if( ! xResult.is())
//         {
//             Reference< container::XChild > xChild( xDoc, uno::UNO_QUERY );
//             if( xChild.is())
//             {
//                 Reference< lang::XMultiServiceFactory > xFact( xChild->getParent(), uno::UNO_QUERY );
//                 if( xFact.is())
//                 {
//                     xResult.set(
//                         xFact->createInstance( OUString::createFromAscii("com.sun.star.chart2.data.DataProvider")),
//                         uno::UNO_QUERY );
//                     if( xResult.is())
//                     {
//                         Reference< chart2::data::XDataReceiver > xReceiver( xDoc, uno::UNO_QUERY );
//                         if( xReceiver.is())
//                             xReceiver->attachDataProvider( xResult );
//                     }
//                 }
//             }
//         }
    }
    return xResult;
}

void SchXMLExportHelper::InitRangeSegmentationProperties( const Reference< chart2::XChartDocument > & xChartDoc )
{
    if( xChartDoc.is())
        try
        {
            Reference< chart2::data::XDataProvider > xDataProvider( GetDataProvider( xChartDoc ));
            OSL_ENSURE( xDataProvider.is(), "No DataProvider" );
            if( xDataProvider.is())
            {
                Reference< chart2::data::XDataSource > xDataSource( lcl_pressUsedDataIntoRectangularFormat( xChartDoc, mbHasCategoryLabels ));
                Sequence< beans::PropertyValue > aArgs( xDataProvider->detectArguments( xDataSource ));
                ::rtl::OUString sCellRange, sBrokenRange;
                bool bBrokenRangeAvailable = false;
                for( sal_Int32 i=0; i<aArgs.getLength(); ++i )
                {
                    if( aArgs[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("CellRangeRepresentation")))
                        aArgs[i].Value >>= sCellRange;
                    else if( aArgs[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("BrokenCellRangeForExport")))
                    {
                        if( aArgs[i].Value >>= sBrokenRange )
                            bBrokenRangeAvailable = true;
                    }
                    else if( aArgs[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("DataRowSource")))
                    {
                        chart::ChartDataRowSource eRowSource;
                        aArgs[i].Value >>= eRowSource;
                        mbRowSourceColumns = ( eRowSource == chart::ChartDataRowSource_COLUMNS );
                    }
                    else if( aArgs[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("FirstCellAsLabel")))
                        aArgs[i].Value >>= mbHasSeriesLabels;
                    else if( aArgs[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("SequenceMapping")))
                        aArgs[i].Value >>= maSequenceMapping;
                    else if( aArgs[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("TableNumberList")))
                        aArgs[i].Value >>= msTableNumberList;
                }

                // #i79009# For Writer we have to export a broken version of the
                // range, where every row number is noe too large, so that older
                // version can correctly read those files.
                msChartAddress = (bBrokenRangeAvailable ? sBrokenRange : sCellRange);
                if( msChartAddress.getLength() > 0 )
                {
                    // convert format to XML-conform one
                    Reference< chart2::data::XRangeXMLConversion > xConversion( xDataProvider, uno::UNO_QUERY );
                    if( xConversion.is())
                        msChartAddress = xConversion->convertRangeToXML( msChartAddress );
                }
            }
        }
        catch( uno::Exception & ex )
        {
            (void)ex; // avoid warning for pro build
            OSL_ENSURE( false, OUStringToOString(
                            OUString( RTL_CONSTASCII_USTRINGPARAM( "Exception caught. Type: " )) +
                            OUString::createFromAscii( typeid( ex ).name()) +
                            OUString( RTL_CONSTASCII_USTRINGPARAM( ", Message: " )) +
                            ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
        }
}

// export components ========================================

// first version: everything goes in one storage

Sequence< OUString > SAL_CALL SchXMLExport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Chart.XMLExporter" ) );
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLExport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SchXMLExport.Compact" ) );
}

Reference< uno::XInterface > SAL_CALL SchXMLExport_createInstance(const Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    // #110680#
    // #103997# removed some flags from EXPORT_ALL
    // return (cppu::OWeakObject*)new SchXMLExport( EXPORT_ALL ^ ( EXPORT_SETTINGS | EXPORT_MASTERSTYLES | EXPORT_SCRIPTS ));
    return (cppu::OWeakObject*)new SchXMLExport( rSMgr, EXPORT_ALL ^ ( EXPORT_SETTINGS | EXPORT_MASTERSTYLES | EXPORT_SCRIPTS ));
}

// Oasis format
Sequence< OUString > SAL_CALL SchXMLExport_Oasis_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Chart.XMLOasisExporter" ) );
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLExport_Oasis_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SchXMLExport.Oasis.Compact" ) );
}

Reference< uno::XInterface > SAL_CALL SchXMLExport_Oasis_createInstance(const Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    // #103997# removed some flags from EXPORT_ALL
    return (cppu::OWeakObject*)new SchXMLExport( rSMgr,
        (EXPORT_ALL ^ ( EXPORT_SETTINGS | EXPORT_MASTERSTYLES | EXPORT_SCRIPTS )) | EXPORT_OASIS );
}

// ============================================================

// multiple storage version: one for content / styles / meta

Sequence< OUString > SAL_CALL SchXMLExport_Styles_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Chart.XMLStylesExporter" ));
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLExport_Styles_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SchXMLExport.Styles" ));
}

Reference< uno::XInterface > SAL_CALL SchXMLExport_Styles_createInstance(const Reference< lang::XMultiServiceFactory >& rSMgr) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new SchXMLExport( EXPORT_STYLES );
    return (cppu::OWeakObject*)new SchXMLExport( rSMgr, EXPORT_STYLES );
}

// Oasis format
Sequence< OUString > SAL_CALL SchXMLExport_Oasis_Styles_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Chart.XMLOasisStylesExporter" ));
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLExport_Oasis_Styles_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SchXMLExport.Oasis.Styles" ));
}

Reference< uno::XInterface > SAL_CALL SchXMLExport_Oasis_Styles_createInstance(const Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SchXMLExport( rSMgr, EXPORT_STYLES | EXPORT_OASIS );
}

// ------------------------------------------------------------

Sequence< OUString > SAL_CALL SchXMLExport_Content_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Chart.XMLContentExporter" ));
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLExport_Content_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SchXMLExport.Content" ));
}

Reference< uno::XInterface > SAL_CALL SchXMLExport_Content_createInstance(const Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new SchXMLExport( EXPORT_AUTOSTYLES | EXPORT_CONTENT | EXPORT_FONTDECLS );
    return (cppu::OWeakObject*)new SchXMLExport( rSMgr, EXPORT_AUTOSTYLES | EXPORT_CONTENT | EXPORT_FONTDECLS );
}

// Oasis format
Sequence< OUString > SAL_CALL SchXMLExport_Oasis_Content_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Chart.XMLOasisContentExporter" ));
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLExport_Oasis_Content_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SchXMLExport.Oasis.Content" ));
}

Reference< uno::XInterface > SAL_CALL SchXMLExport_Oasis_Content_createInstance(const Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SchXMLExport( rSMgr, EXPORT_AUTOSTYLES | EXPORT_CONTENT | EXPORT_FONTDECLS | EXPORT_OASIS );
}

// ------------------------------------------------------------

// Sequence< OUString > SAL_CALL SchXMLExport_Meta_getSupportedServiceNames() throw()
// {
//  const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Chart.XMLMetaExporter" ));
//  const Sequence< OUString > aSeq( &aServiceName, 1 );
//  return aSeq;
// }

// OUString SAL_CALL SchXMLExport_Meta_getImplementationName() throw()
// {
//  return OUString( RTL_CONSTASCII_USTRINGPARAM( "SchXMLExport.Meta" ));
// }

// Reference< uno::XInterface > SAL_CALL SchXMLExport_Meta_createInstance(const Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
// {
//  return (cppu::OWeakObject*)new SchXMLExport( EXPORT_META );
// }

// Oasis format
Sequence< OUString > SAL_CALL SchXMLExport_Oasis_Meta_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Chart.XMLOasisMetaExporter" ));
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLExport_Oasis_Meta_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SchXMLExport.Oasis.Meta" ));
}

Reference< uno::XInterface > SAL_CALL SchXMLExport_Oasis_Meta_createInstance(const Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SchXMLExport( rSMgr, EXPORT_META | EXPORT_OASIS  );
}


// XServiceInfo
OUString SAL_CALL SchXMLExport::getImplementationName() throw( uno::RuntimeException )
{
    switch( getExportFlags())
    {
        case EXPORT_ALL:
            return SchXMLExport_getImplementationName();
        case EXPORT_STYLES:
            return SchXMLExport_Styles_getImplementationName();
        case ( EXPORT_AUTOSTYLES | EXPORT_CONTENT | EXPORT_FONTDECLS ):
            return SchXMLExport_Content_getImplementationName();
//         case EXPORT_META:
//             return SchXMLExport_Meta_getImplementationName();

        // Oasis format
        case ( EXPORT_ALL | EXPORT_OASIS ):
            return SchXMLExport_Oasis_getImplementationName();
        case ( EXPORT_STYLES | EXPORT_OASIS ):
            return SchXMLExport_Oasis_Styles_getImplementationName();
        case ( EXPORT_AUTOSTYLES | EXPORT_CONTENT | EXPORT_FONTDECLS | EXPORT_OASIS  ):
            return SchXMLExport_Oasis_Content_getImplementationName();
        case ( EXPORT_META | EXPORT_OASIS ):
            return SchXMLExport_Oasis_Meta_getImplementationName();

        case EXPORT_SETTINGS:
        // there is no settings component in chart
        default:
            return OUString::createFromAscii( "SchXMLExport" );
    }
}
