/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ChartDebugTrace.cxx,v $
 * $Revision: 1.4.44.1 $
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

#include "ChartDebugTrace.hxx"
#include "macros.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>
#include <rtl/math.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

#if OSL_DEBUG_LEVEL >= CHART_TRACE_OSL_DEBUG_LEVEL

namespace
{
/*
const char lcl_aSpace=' ';

void lcl_IndentedTrace( int nIndent, char* pStr )
{
    if( nIndent > 0 )
    {
        OSL_TRACE( "%*c%s", nIndent, lcl_aSpace, pStr );
    }
    else
    {
        OSL_TRACE( pStr );
    }
}

void lcl_TraceException( const uno::Exception & aEx )
{
    OSL_TRACE(
        U2C( C2U( "*** Exception caught during trace. Type: " ) +
             OUString::createFromAscii( typeid( aEx ).name()) +
             C2U( ", Message: " ) +
             aEx.Message ));
}

void lcl_TraceCategories( const Reference< data::XLabeledDataSequence > & xCat, int nIndent )
{
    if( ! xCat.is())
        return;
    try
    {
        Reference< data::XDataSequence > xValues( xCat->getValues());
        if( xValues.is())
        {
            OSL_TRACE( "%*ccategories: source: %s", nIndent, lcl_aSpace,
                       U2C( xValues->getSourceRangeRepresentation()));
        }
        Reference< data::XDataSequence > xLabel( xCat->getLabel());
        if( xLabel.is())
        {
            OSL_TRACE( "%*ccategories' label: source: %s", nIndent, lcl_aSpace,
                       U2C( xLabel->getSourceRangeRepresentation()));
        }
    }
    catch( uno::Exception & ex )
    {
        lcl_TraceException( ex );
    }
}

void lcl_TraceDataSeriesSeq( const Sequence< Reference< XDataSeries > > & aSeries, int nIndent )
{
    for( sal_Int32 j = 0; j < aSeries.getLength(); ++j )
    {
        Reference< beans::XPropertySet > xProp( aSeries[j], uno::UNO_QUERY );
        OUString aId;

        OSL_TRACE( "%*cindex %ld", nIndent, lcl_aSpace, j );

        StackingDirection aStDir;
        if( xProp.is() &&
            ( xProp->getPropertyValue( C2U( "StackingDirection" )) >>= aStDir ) &&
            aStDir != StackingDirection_NO_STACKING )
        {
            OSL_TRACE( "%*cstacking in %s", nIndent + 2, lcl_aSpace,
                       (aStDir == StackingDirection_Y_STACKING)
                       ? "y-direction" : "z-direction" );
        }

        Reference< data::XDataSource > xSource( aSeries[j], uno::UNO_QUERY );
        if( xSource.is())
        {
            Sequence< Reference< data::XLabeledDataSequence > > aSequences( xSource->getDataSequences());
            const sal_Int32 nMax = aSequences.getLength();
            for( sal_Int32 k = 0; k < nMax; ++k )
            {
                if( aSequences[k].is())
                {
                    OUString aSourceId(C2U("<none>"));
                    if( aSequences[k]->getValues().is())
                        aSourceId = aSequences[k]->getValues()->getSourceRangeRepresentation();
                    xProp.set( aSequences[k]->getValues(), uno::UNO_QUERY );
                    if( xProp.is() &&
                        ( xProp->getPropertyValue( C2U( "Role" )) >>= aId ))
                    {
                        OSL_TRACE( "%*cdata sequence %d: role: %s, source: %s",
                                   nIndent + 2, lcl_aSpace, k, U2C( aId ), U2C( aSourceId ));
                    }
                    else
                    {
                        OSL_TRACE( "%*cdata sequence %d, unknown role, source: %s",
                                   nIndent + 2, lcl_aSpace, k, U2C( aSourceId ) );
                    }

                    aSourceId = C2U("<none>");
                    if( aSequences[k]->getLabel().is())
                        aSourceId = OUString( aSequences[k]->getLabel()->getSourceRangeRepresentation());
                    xProp.set( aSequences[k]->getLabel(), uno::UNO_QUERY );
                    if( xProp.is() &&
                        ( xProp->getPropertyValue( C2U( "Role" )) >>= aId ))
                    {
                        OSL_TRACE( "%*cdata sequence label %d: role: %s, source: %s",
                                   nIndent + 2, lcl_aSpace, k, U2C( aId ), U2C( aSourceId ));
                    }
                    else
                    {
                        OSL_TRACE( "%*cdata sequence label %d: unknown role, source: %s",
                                   nIndent + 2, lcl_aSpace, k, U2C( aSourceId ) );
                    }
                }
            }
        }
    }
}

void lcl_TraceChartType( const Reference< XChartType > & xChartType, int nIndent )
{
    if( xChartType.is())
    {
        OSL_TRACE( "%*c* type: %s", nIndent, lcl_aSpace, U2C( xChartType->getChartType()) );

        lcl_IndentedTrace( nIndent + 2, "Supported Roles" );
        sal_Int32 i=0;
        Sequence< OUString > aMandRoles( xChartType->getSupportedMandatoryRoles());
        if( aMandRoles.getLength() > 0 )
        {
            lcl_IndentedTrace( nIndent + 4, "mandatory" );
            for( i=0; i<aMandRoles.getLength(); ++i )
            {
                OSL_TRACE( "%*c%s", nIndent + 6, lcl_aSpace, U2C( aMandRoles[i] ));
            }
        }
        Sequence< OUString > aOptRoles( xChartType->getSupportedOptionalRoles());
        if( aOptRoles.getLength() > 0 )
        {
            lcl_IndentedTrace( nIndent + 4, "optional" );
            for( i=0; i<aOptRoles.getLength(); ++i )
            {
                OSL_TRACE( "%*c%s", nIndent + 6, lcl_aSpace, U2C( aOptRoles[i] ));
            }
        }
        OSL_TRACE( "%*crole of sequence for label: %s", nIndent + 2, lcl_aSpace,
                   U2C( xChartType->getRoleOfSequenceForSeriesLabel()));

        Reference< XDataSeriesContainer > xDSCnt( xChartType, uno::UNO_QUERY );
        if( xDSCnt.is())
        {
            lcl_IndentedTrace( nIndent + 2, "Data Series" );
            lcl_TraceDataSeriesSeq( xDSCnt->getDataSeries(), nIndent + 4 );
        }
    }
}

void lcl_TraceCoordinateSystem( const Reference< XCoordinateSystem > & xCooSys, int nIndent )
{
    if( xCooSys.is()) try
    {
        sal_Int32 nDim = xCooSys->getDimension();
        OSL_TRACE( "%*c* dim: %ld, type: %s", nIndent, lcl_aSpace,
                   nDim, U2C( xCooSys->getCoordinateSystemType() ));
        nIndent += 2;
        OSL_TRACE( "%*cview service-name: %s", nIndent, lcl_aSpace,
                   U2C( xCooSys->getViewServiceName() ));

        Reference< beans::XPropertySet > xProp( xCooSys, uno::UNO_QUERY );
        if( xProp.is())
        {
            Reference< beans::XPropertySetInfo > xInfo( xProp->getPropertySetInfo(), uno::UNO_QUERY );
            sal_Bool bSwap;
            if( xInfo.is() &&
                xInfo->hasPropertyByName( C2U("SwapXAndYAxis")) &&
                (xProp->getPropertyValue( C2U("SwapXAndYAxis")) >>= bSwap) &&
                bSwap )
            {
                lcl_IndentedTrace( nIndent, "swap x-axis and y-axis" );
            }
        }

        if( nDim >= 2 )
        {
            const sal_Int32 nMaxIndex = xCooSys->getMaximumAxisIndexByDimension(1);
            for(sal_Int32 nI=0; nI<=nMaxIndex; ++nI)
            {
                Reference< XScale > xScale( xCooSys->getAxisByDimension( 1, nI ));
                if( xScale.is())
                {
                    ScaleData aData( xScale->getScaleData());
                    if( aData.AxisType==AxisType::PERCENT )
                        lcl_IndentedTrace( nIndent, "percent stacking at y-scale" );
                }
            }
        }

        Sequence< uno::Any > aOrigin( xCooSys->getOrigin());
        double x, y, z;
        ::rtl::math::setNan( &x ), ::rtl::math::setNan( &y ), ::rtl::math::setNan( &z );
        if( aOrigin.getLength() > 0 &&
            aOrigin[0].hasValue() )
            aOrigin[0] >>= x;
        if( aOrigin.getLength() > 1 &&
            aOrigin[1].hasValue() )
            aOrigin[1] >>= y;
        if( aOrigin.getLength() > 2 &&
            aOrigin[2].hasValue() )
            aOrigin[2] >>= z;
        OSL_TRACE( "%*corigin: (%f, %f, %f)", nIndent, lcl_aSpace, x, y, z );

        Reference< XChartTypeContainer > xCTCnt( xCooSys, uno::UNO_QUERY );
        if( xCTCnt.is())
        {
            Sequence< Reference< XChartType > > aChartTypes( xCTCnt->getChartTypes());
            if( aChartTypes.getLength() > 0 )
            {
                lcl_IndentedTrace( nIndent, "Chart Types" );
                for( sal_Int32 i=0; i<aChartTypes.getLength(); ++i )
                {
                    lcl_TraceChartType( aChartTypes[i], nIndent + 2 );
                }
            }
        }
    }
    catch( uno::Exception & ex )
    {
        lcl_TraceException( ex );
    }
}

void lcl_TraceMeter(
    const Reference< XMeter > & xMeter,
    const Sequence< Reference< XCoordinateSystem > > & aCooSys,
    bool bWithCategories,
    int nIndent )
{
    try
    {
        Reference< XCoordinateSystem > xCooSys( xMeter->getCoordinateSystem());
        for( sal_Int32 i=0; i<aCooSys.getLength(); ++i )
            if( aCooSys[i] == xCooSys )
            {
                OSL_TRACE( "%*cbelongs to Coordinate System %ld.", nIndent + 2, lcl_aSpace, i );
            }
        OSL_TRACE( "%*crepresents  Dimension %ld.", nIndent + 2, lcl_aSpace, xMeter->getRepresentedDimension());
        if( bWithCategories )
        {
            Reference< XScale > xScale( xCooSys->getAxisByDimension( xMeter->getRepresentedDimension(), xMeter->getIndex() ));
            if( xScale.is())
            {
                ScaleData aData = xScale->getScaleData();
                if( aData.Categories.is())
                {
                    lcl_TraceCategories( aData.Categories, nIndent + 2 );
                }
            }
        }
    }
    catch( uno::Exception & ex )
    {
        lcl_TraceException( ex );
    }
}
*/
} // anonymous namespace
#endif


namespace chart
{
namespace debug
{

#if OSL_DEBUG_LEVEL >= CHART_TRACE_OSL_DEBUG_LEVEL

void ChartDebugTraceDocument(
    const Reference< XChartDocument > & /*xDoc*/,
    int /*nIndent*/ )
{
    /*
#if OSL_DEBUG_LEVEL >= CHART_TRACE_OSL_DEBUG_LEVEL
    try
    {
        OSL_TRACE( "%*cas %sternal data", nIndent, 'h',
                   xDoc->hasInternalDataProvider() ? "in": "ex" );

        Reference< lang::XMultiServiceFactory > xCTManager( xDoc->getChartTypeManager(), uno::UNO_QUERY );
        if( xCTManager.is())
        {
            Sequence< OUString > aServiceNames( xCTManager->getAvailableServiceNames());
            OSL_TRACE( "%*c ChartTypeManager has %ld entries", nIndent, '*', aServiceNames.getLength());
# if OSL_DEBUG_LEVEL >= (CHART_TRACE_OSL_DEBUG_LEVEL + 1)
            for( sal_Int32 i=0; i<aServiceNames.getLength(); ++i )
            {
                OSL_TRACE( "%*c%s", nIndent + 2, lcl_aSpace, U2C( aServiceNames[i] ));
            }
# endif
        }
        Reference< XDiagram > xDiagram( xDoc->getFirstDiagram());
        lcl_IndentedTrace( nIndent, "* Diagram" );
        ChartDebugTraceDiagram( xDiagram, nIndent + 2 );
    }
    catch( uno::Exception & ex )
    {
        lcl_TraceException( ex );
    }
#endif
    */
}

void ChartDebugTraceDiagram(
    const Reference< XDiagram > & /*xDiagram*/,
    int /*nIndent*/ )
{
    /*
#if OSL_DEBUG_LEVEL >= CHART_TRACE_OSL_DEBUG_LEVEL
    try
    {
        Reference< XCoordinateSystemContainer > xCooSysCnt( xDiagram, uno::UNO_QUERY_THROW );
        Sequence< Reference< XCoordinateSystem > > aCooSys( xCooSysCnt->getCoordinateSystems() );
        if( aCooSys.getLength() > 0 )
        {
            lcl_IndentedTrace( nIndent, "CoordinateSystems" );
            for( sal_Int32 i=0; i<aCooSys.getLength(); ++i )
                lcl_TraceCoordinateSystem( aCooSys[i], nIndent + 2 );
        }
        else
        {
            lcl_IndentedTrace( nIndent, "<No Coordinate Systems>" );
        }

        Reference< XAxisContainer > xAxisCnt( xDiagram, uno::UNO_QUERY_THROW );
        Sequence< Reference< XAxis > > aAxes( xAxisCnt->getAxes() );
        if( aAxes.getLength() > 0 )
        {
            lcl_IndentedTrace( nIndent, "Axes" );
            for( sal_Int32 i=0; i<aAxes.getLength(); ++i )
                lcl_TraceMeter( Reference< XMeter >( aAxes[i], uno::UNO_QUERY ), aCooSys, true, nIndent + 2 );
        }
        else
        {
            lcl_IndentedTrace( nIndent, "<No Axes>" );
        }

        Reference< XGridContainer > xGridCnt( xDiagram, uno::UNO_QUERY_THROW );
        Sequence< Reference< XGrid > > aGrids( xGridCnt->getGrids() );
        if( aGrids.getLength() > 0 )
        {
            lcl_IndentedTrace( nIndent, "Grids" );
            for( sal_Int32 i=0; i<aGrids.getLength(); ++i )
                lcl_TraceMeter( Reference< XMeter >( aGrids[i], uno::UNO_QUERY ), aCooSys, false, nIndent + 2 );
        }
        else
        {
            lcl_IndentedTrace( nIndent, "<No Grids>" );
        }
    }
    catch( uno::Exception & ex )
    {
        lcl_TraceException( ex );
    }

#endif

*/
}
#endif

} // namespace debug
} //  namespace chart
