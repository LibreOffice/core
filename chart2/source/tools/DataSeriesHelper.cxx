/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataSeriesHelper.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 11:00:49 $
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
#include "DataSeriesHelper.hxx"
#include "DiagramHelper.hxx"
#include "DataSource.hxx"
#include "macros.hxx"
#include "ContainerHelper.hxx"

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XTEXTUALDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_STACKINGDIRECTION_HPP_
#include <com/sun/star/chart2/StackingDirection.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_LABELORIGIN_HPP_
#include <com/sun/star/chart2/data/LabelOrigin.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_AXISTYPE_HPP_
#include <com/sun/star/chart2/AxisType.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART2_SYMBOLSTYLE_HPP_
#include <com/sun/star/chart2/SymbolStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_SYMBOL_HPP_
#include <com/sun/star/chart2/Symbol.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif


#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <functional>
#include <algorithm>
#include <iterator>
#include <vector>
#include <set>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ----------------------------------------
namespace
{

class lcl_MatchesRole : public ::std::unary_function< Reference< chart2::data::XLabeledDataSequence >, bool >
{
public:
    explicit lcl_MatchesRole( const OUString & aRole, bool bMatchPrefix ) :
            m_aRole( aRole ),
            m_bMatchPrefix( bMatchPrefix )
    {}

    bool operator () ( const Reference< chart2::data::XLabeledDataSequence > & xSeq ) const
    {
        if(!xSeq.is())
            return false;
        Reference< beans::XPropertySet > xProp( xSeq->getValues(), uno::UNO_QUERY );
        OUString aRole;

        if( m_bMatchPrefix )
            return ( xProp.is() &&
                     (xProp->getPropertyValue(
                         OUString( RTL_CONSTASCII_USTRINGPARAM( "Role" )) ) >>= aRole ) &&
                     aRole.match( m_aRole ));

        return ( xProp.is() &&
                 (xProp->getPropertyValue(
                     OUString( RTL_CONSTASCII_USTRINGPARAM( "Role" )) ) >>= aRole ) &&
                 m_aRole.equals( aRole ));
    }

private:
    OUString m_aRole;
    bool     m_bMatchPrefix;
};

Reference< chart2::data::XLabeledDataSequence > lcl_findLSequenceWithOnlyLabel(
    const Reference< chart2::data::XDataSource > & xDataSource )
{
    Reference< chart2::data::XLabeledDataSequence > xResult;
    Sequence< Reference< chart2::data::XLabeledDataSequence > > aSequences( xDataSource->getDataSequences());

    for( sal_Int32 i=0; i<aSequences.getLength(); ++i )
    {
        OSL_ENSURE( aSequences[i].is(), "empty LabeledDataSequence" );
        // no values are set but a label exists
        if( aSequences[i].is() &&
            ( ! aSequences[i]->getValues().is() &&
              aSequences[i]->getLabel().is()))
        {
            xResult.set( aSequences[i] );
            break;
        }
    }

    return xResult;
}

void lcl_getCooSysAndChartTypeOfSeries(
    const Reference< chart2::XDataSeries > & xSeries,
    const Reference< chart2::XDiagram > & xDiagram,
    Reference< chart2::XCoordinateSystem > & xOutCooSys,
    Reference< chart2::XChartType > & xOutChartType )
{
    Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xDiagram, uno::UNO_QUERY );
    if( xCooSysCnt.is())
    {
        Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq( xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 nCooSysIdx=0; nCooSysIdx<aCooSysSeq.getLength(); ++nCooSysIdx )
        {
            Reference< chart2::XChartTypeContainer > xCTCnt( aCooSysSeq[nCooSysIdx], uno::UNO_QUERY_THROW );
            Sequence< Reference< chart2::XChartType > > aChartTypes( xCTCnt->getChartTypes());
            for( sal_Int32 nCTIdx=0; nCTIdx<aChartTypes.getLength(); ++nCTIdx )
            {
                Reference< chart2::XDataSeriesContainer > xSeriesCnt( aChartTypes[nCTIdx], uno::UNO_QUERY );
                if( xSeriesCnt.is())
                {
                    Sequence< Reference< chart2::XDataSeries > > aSeries( xSeriesCnt->getDataSeries());
                    for( sal_Int32 nSeriesIdx=0; nSeriesIdx<aSeries.getLength(); ++nSeriesIdx )
                    {
                        if( aSeries[nSeriesIdx] == xSeries )
                        {
                            xOutCooSys.set( aCooSysSeq[nCooSysIdx] );
                            xOutChartType.set( aChartTypes[nCTIdx] );
                        }
                    }
                }
            }
        }
    }
}

} // anonymous namespace
// ----------------------------------------

namespace chart
{

namespace DataSeriesHelper
{

OUString GetRole( const uno::Reference< chart2::data::XLabeledDataSequence >& xLabeledDataSequence )
{
    OUString aRet;
    if( xLabeledDataSequence.is() )
    {
        Reference< beans::XPropertySet > xProp( xLabeledDataSequence->getValues(), uno::UNO_QUERY );
        if( xProp.is() )
            xProp->getPropertyValue( C2U("Role") ) >>= aRet;
    }
    return aRet;
}

void SetRole( const uno::Reference< chart2::data::XLabeledDataSequence >& xLabeledDataSequence,
                const ::rtl::OUString& rRole )
{
    if( xLabeledDataSequence.is() )
    {
        Reference< beans::XPropertySet > xProp( xLabeledDataSequence->getValues(), uno::UNO_QUERY );
        if( xProp.is() )
            xProp->setPropertyValue( C2U("Role"), uno::makeAny(rRole) ) ;
    }
}

Reference< chart2::data::XLabeledDataSequence >
    getDataSequenceByRole(
        const Reference< chart2::data::XDataSource > & xSource, OUString aRole,
        bool bMatchPrefix /* = false */ )
{
    Reference< chart2::data::XLabeledDataSequence > aNoResult;
    if( ! xSource.is())
        return aNoResult;
    Sequence< Reference< chart2::data::XLabeledDataSequence > > aLabeledSeq( xSource->getDataSequences());

    const Reference< chart2::data::XLabeledDataSequence > * pBegin = aLabeledSeq.getConstArray();
    const Reference< chart2::data::XLabeledDataSequence > * pEnd = pBegin + aLabeledSeq.getLength();
    const Reference< chart2::data::XLabeledDataSequence > * pMatch =
        ::std::find_if( pBegin, pEnd, lcl_MatchesRole( aRole, bMatchPrefix ));

    if( pMatch != pEnd )
        return *pMatch;

    return aNoResult;
}

::std::vector< Reference< chart2::data::XLabeledDataSequence > >
    getAllDataSequencesByRole( const Sequence< Reference< chart2::data::XLabeledDataSequence > > & aDataSequences,
                               OUString aRole, bool bMatchPrefix /* = false */ )
{
    ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aResultVec;
    ::std::remove_copy_if( aDataSequences.getConstArray(), aDataSequences.getConstArray() + aDataSequences.getLength(),
                           ::std::back_inserter( aResultVec ),
                           ::std::not1( lcl_MatchesRole( aRole, bMatchPrefix )));
    return aResultVec;
}

Reference< chart2::data::XDataSource >
    getDataSource( const Sequence< Reference< chart2::XDataSeries > > & aSeries )
{
    ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aSeqVec;

    for( sal_Int32 i = 0; i < aSeries.getLength(); ++i )
    {
        Reference< chart2::data::XDataSource > xSource( aSeries[ i ], uno::UNO_QUERY );
        if( xSource.is())
        {
            Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeq( xSource->getDataSequences());
            ::std::copy( aSeq.getConstArray(), aSeq.getConstArray() + aSeq.getLength(),
                         ::std::back_inserter( aSeqVec ));
        }
    }

    return Reference< chart2::data::XDataSource >(
        new DataSource( ContainerHelper::ContainerToSequence( aSeqVec )));
}

namespace
{
OUString lcl_getDataSequenceLabel( const Reference< chart2::data::XDataSequence > & xSequence )
{
    OUString aResult;

    Reference< chart2::data::XTextualDataSequence > xTextSeq( xSequence, uno::UNO_QUERY );
    if( xTextSeq.is())
    {
        Sequence< OUString > aSeq( xTextSeq->getTextualData());

        const sal_Int32 nMax = aSeq.getLength() - 1;
        OUString aVal;
        OUStringBuffer aBuf;

        for( sal_Int32 i = 0; i <= nMax; ++i )
        {
            aBuf.append( aSeq[i] );
            if( i < nMax )
                aBuf.append( sal_Unicode( ' ' ));
        }
        aResult = aBuf.makeStringAndClear();
    }
    else if( xSequence.is())
    {
        Sequence< uno::Any > aSeq( xSequence->getData());

        const sal_Int32 nMax = aSeq.getLength() - 1;
        OUString aVal;
        OUStringBuffer aBuf;
        double fNum = 0;

        for( sal_Int32 i = 0; i <= nMax; ++i )
        {
            if( aSeq[i] >>= aVal )
            {
                aBuf.append( aVal );
                if( i < nMax )
                    aBuf.append( sal_Unicode( ' ' ));
            }
            else if( aSeq[ i ] >>= fNum )
            {
                aBuf.append( fNum );
                if( i < nMax )
                    aBuf.append( sal_Unicode( ' ' ));
            }
        }
        aResult = aBuf.makeStringAndClear();
    }

    return aResult;
}
}

OUString getLabelForLabeledDataSequence(
    const Reference< chart2::data::XLabeledDataSequence > & xLabeledSeq )
{
    OUString aResult;
    if( xLabeledSeq.is())
    {
        Reference< chart2::data::XDataSequence > xSeq( xLabeledSeq->getLabel());
        if( xSeq.is() )
            aResult = lcl_getDataSequenceLabel( xSeq );
        if( !xSeq.is() || !aResult.getLength() )
        {
            // no label set or label content is empty -> use auto-generated one
            Reference< chart2::data::XDataSequence > xValueSeq( xLabeledSeq->getValues() );
            if( xValueSeq.is() )
            {
                Sequence< OUString > aLabels( xValueSeq->generateLabel(
                    chart2::data::LabelOrigin_SHORT_SIDE ) );
                // no labels returned is interpreted as: auto-generation not
                // supported by sequence
                if( aLabels.getLength() )
                    aResult=aLabels[0];
                else
                {
                    //todo?: maybe use the index of the series as name
                    //but as the index may change it would be better to have such a name persistent
                    //what is not possible at the moment
                    //--> maybe use the identifier as part of the name ...
                    aResult = lcl_getDataSequenceLabel( xValueSeq );
                }
            }
        }
    }
    return aResult;
}

OUString getDataSeriesLabel(
    const Reference< chart2::XDataSeries > & xSeries,
    const OUString & rLabelSequenceRole )
{
    OUString aResult;

    Reference< chart2::data::XDataSource > xSource( xSeries, uno::UNO_QUERY );
    if( xSource.is())
    {
        Reference< chart2::data::XLabeledDataSequence > xLabeledSeq(
            ::chart::DataSeriesHelper::getDataSequenceByRole( xSource, rLabelSequenceRole ));
        if( xLabeledSeq.is())
            aResult = getLabelForLabeledDataSequence( xLabeledSeq );
        else
        {
            // special case: labeled data series with only a label and no values may
            // serve as label
            xLabeledSeq.set( lcl_findLSequenceWithOnlyLabel( xSource ));
            if( xLabeledSeq.is())
            {
                Reference< chart2::data::XDataSequence > xSeq( xLabeledSeq->getLabel());
                if( xSeq.is())
                    aResult = lcl_getDataSequenceLabel( xSeq );
            }
        }

    }

    return aResult;
}

void setStackModeAtSeries(
    const Sequence< Reference< chart2::XDataSeries > > & aSeries,
    const Reference< chart2::XCoordinateSystem > & xCorrespondingCoordinateSystem,
    StackMode eStackMode )
{
    if( eStackMode == StackMode_AMBIGUOUS )
        return;

    const OUString aPropName( RTL_CONSTASCII_USTRINGPARAM( "StackingDirection" ));
    const uno::Any aPropValue = uno::makeAny(
        ( (eStackMode == StackMode_Y_STACKED) ||
          (eStackMode == StackMode_Y_STACKED_PERCENT) )
        ? chart2::StackingDirection_Y_STACKING
        : (eStackMode == StackMode_Z_STACKED )
        ? chart2::StackingDirection_Z_STACKING
        : chart2::StackingDirection_NO_STACKING );

    std::set< sal_Int32 > aAxisIndexSet;
    for( sal_Int32 i=0; i<aSeries.getLength(); ++i )
    {
        try
        {
            Reference< beans::XPropertySet > xProp( aSeries[i], uno::UNO_QUERY );
            if( xProp.is() )
            {
                xProp->setPropertyValue( aPropName, aPropValue );

                sal_Int32 nAxisIndex;
                xProp->getPropertyValue( C2U("AttachedAxisIndex") ) >>= nAxisIndex;
                aAxisIndexSet.insert(nAxisIndex);
            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    if( xCorrespondingCoordinateSystem.is() &&
        1 < xCorrespondingCoordinateSystem->getDimension() )
    {
        sal_Int32 nAxisIndexCount = aAxisIndexSet.size();
        if( !nAxisIndexCount )
        {
            aAxisIndexSet.insert(0);
            nAxisIndexCount = aAxisIndexSet.size();
        }

        for( ::std::set< sal_Int32 >::const_iterator aIt = aAxisIndexSet.begin();
            aIt != aAxisIndexSet.end(); ++aIt )
        {
            sal_Int32 nAxisIndex = *aIt;
            Reference< chart2::XAxis > xAxis(
                xCorrespondingCoordinateSystem->getAxisByDimension( 1, nAxisIndex ));
            if( xAxis.is())
            {
                sal_Bool bPercent = (eStackMode == StackMode_Y_STACKED_PERCENT);
                chart2::ScaleData aScaleData = xAxis->getScaleData();

                if( bPercent != (aScaleData.AxisType==chart2::AxisType::PERCENT) )
                {
                    if( bPercent )
                        aScaleData.AxisType = chart2::AxisType::PERCENT;
                    else
                        aScaleData.AxisType = chart2::AxisType::REALNUMBER;
                    xAxis->setScaleData( aScaleData );
                }
            }
        }
    }
}

sal_Int32 getAttachedAxisIndex( const Reference< chart2::XDataSeries > & xSeries )
{
    sal_Int32 nRet = 0;
    try
    {
        Reference< beans::XPropertySet > xProp( xSeries, uno::UNO_QUERY );
        if( xProp.is() )
        {
            xProp->getPropertyValue( C2U("AttachedAxisIndex") ) >>= nRet;
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return nRet;
}

sal_Int32 getNumberFormatKeyFromAxis(
    const Reference< chart2::XDataSeries > & xSeries,
    const Reference< chart2::XCoordinateSystem > & xCorrespondingCoordinateSystem,
    sal_Int32 nDimensionIndex,
    sal_Int32 nAxisIndex /* = -1 */ )
{
    sal_Int32 nResult =  0;
    if( nAxisIndex == -1 )
        nAxisIndex = getAttachedAxisIndex( xSeries );
    try
    {
        Reference< beans::XPropertySet > xAxisProp(
            xCorrespondingCoordinateSystem->getAxisByDimension( nDimensionIndex, nAxisIndex ), uno::UNO_QUERY );
        if( xAxisProp.is())
            xAxisProp->getPropertyValue( C2U("NumberFormat")) >>= nResult;
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return nResult;
}

Reference< chart2::XCoordinateSystem > getCoordinateSystemOfSeries(
    const Reference< chart2::XDataSeries > & xSeries,
    const Reference< chart2::XDiagram > & xDiagram )
{
    Reference< chart2::XCoordinateSystem > xResult;
    Reference< chart2::XChartType > xDummy;
    lcl_getCooSysAndChartTypeOfSeries( xSeries, xDiagram, xResult, xDummy );

    return xResult;
}

Reference< chart2::XChartType > getChartTypeOfSeries(
    const Reference< chart2::XDataSeries > & xSeries,
    const Reference< chart2::XDiagram > & xDiagram )
{
    Reference< chart2::XChartType > xResult;
    Reference< chart2::XCoordinateSystem > xDummy;
    lcl_getCooSysAndChartTypeOfSeries( xSeries, xDiagram, xDummy, xResult );

    return xResult;
}

void deleteSeries(
    const Reference< chart2::XDataSeries > & xSeries,
    const Reference< chart2::XChartType > & xChartType )
{
    try
    {
        Reference< chart2::XDataSeriesContainer > xSeriesCnt( xChartType, uno::UNO_QUERY_THROW );
        ::std::vector< Reference< chart2::XDataSeries > > aSeries(
            ContainerHelper::SequenceToVector( xSeriesCnt->getDataSeries()));
        ::std::vector< Reference< chart2::XDataSeries > >::iterator aIt =
              ::std::find( aSeries.begin(), aSeries.end(), xSeries );
        if( aIt != aSeries.end())
        {
            aSeries.erase( aIt );
            xSeriesCnt->setDataSeries( ContainerHelper::ContainerToSequence( aSeries ));
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void switchSymbolsOnOrOff( const Reference< beans::XPropertySet > & xSeriesProperties,
                    bool bSymbolsOn, sal_Int32 nSeriesIndex )
{
    if( !xSeriesProperties.is() )
        return;

    chart2::Symbol aSymbProp;
    if( (xSeriesProperties->getPropertyValue( C2U( "Symbol" )) >>= aSymbProp ) )
    {
        if( !bSymbolsOn )
            aSymbProp.Style = chart2::SymbolStyle_NONE;
        else if( aSymbProp.Style == chart2::SymbolStyle_NONE )
        {
            aSymbProp.Style = chart2::SymbolStyle_STANDARD;
            aSymbProp.StandardSymbol = nSeriesIndex;
        }
        xSeriesProperties->setPropertyValue( C2U( "Symbol" ), uno::makeAny( aSymbProp ));
    }
    //todo: check attributed data points
}

void switchLinesOnOrOff( const Reference< beans::XPropertySet > & xSeriesProperties, bool bLinesOn )
{
    if( !xSeriesProperties.is() )
        return;

    if( bLinesOn )
    {
        // keep line-styles that are not NONE
        drawing::LineStyle eLineStyle;
        if( (xSeriesProperties->getPropertyValue( C2U( "LineStyle" )) >>= eLineStyle ) &&
            eLineStyle == drawing::LineStyle_NONE )
        {
            xSeriesProperties->setPropertyValue( C2U( "LineStyle" ), uno::makeAny( drawing::LineStyle_SOLID ) );
        }
    }
    else
        xSeriesProperties->setPropertyValue( C2U( "LineStyle" ), uno::makeAny( drawing::LineStyle_NONE ) );
}

void setPropertyAlsoToAllAttributedDataPoints( const Reference< chart2::XDataSeries >& xSeries,
                                              const OUString& rPropertyName, const uno::Any& rPropertyValue )
{
    Reference< beans::XPropertySet > xSeriesProperties( xSeries, uno::UNO_QUERY );
    if( !xSeriesProperties.is() )
        return;

    xSeriesProperties->setPropertyValue( rPropertyName, rPropertyValue );
    uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
    if( xSeriesProperties->getPropertyValue( C2U( "AttributedDataPoints" ) ) >>= aAttributedDataPointIndexList )
    {
        for(sal_Int32 nN=aAttributedDataPointIndexList.getLength();nN--;)
        {
            Reference< beans::XPropertySet > xPointProp( xSeries->getDataPointByIndex(aAttributedDataPointIndexList[nN]) );
            if(!xPointProp.is())
                continue;
            xPointProp->setPropertyValue( rPropertyName, rPropertyValue );
        }
    }
}

bool hasAttributedDataPointDifferentValue( const Reference< chart2::XDataSeries >& xSeries,
                                              const OUString& rPropertyName, const uno::Any& rPropertyValue )
{
    Reference< beans::XPropertySet > xSeriesProperties( xSeries, uno::UNO_QUERY );
    if( !xSeriesProperties.is() )
        return false;

    uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
    if( xSeriesProperties->getPropertyValue( C2U( "AttributedDataPoints" ) ) >>= aAttributedDataPointIndexList )
    {
        for(sal_Int32 nN=aAttributedDataPointIndexList.getLength();nN--;)
        {
            Reference< beans::XPropertySet > xPointProp( xSeries->getDataPointByIndex(aAttributedDataPointIndexList[nN]) );
            if(!xPointProp.is())
                continue;
            uno::Any aPointValue( xPointProp->getPropertyValue( rPropertyName ) );
            if( !( rPropertyValue==aPointValue ) )
                return true;
        }
    }
    return false;
}

} //  namespace DataSeriesHelper
} //  namespace chart
