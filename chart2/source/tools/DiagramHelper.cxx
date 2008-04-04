/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DiagramHelper.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 11:01:13 $
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
#include "DiagramHelper.hxx"
#include "LegendHelper.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "DataSeriesHelper.hxx"
#include "AxisHelper.hxx"
#include "ContainerHelper.hxx"
#include "ChartTypeHelper.hxx"
#include "CommonConverters.hxx"
#include "servicenames_charttypes.hxx"

#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XChartTypeTemplate.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/InterpretedData.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>

#include <rtl/math.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::std;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace
{
bool lcl_ChartTypeIsMemberOfCooSys(
    const Reference< chart2::XCoordinateSystem > & xCooSys,
    const Reference< chart2::XChartType > & xChartType )
{
    Reference< chart2::XChartTypeContainer > xCTCnt( xCooSys, uno::UNO_QUERY );
    if( xCTCnt.is())
    {
        Sequence< Reference< chart2::XChartType > > aChartTypes( xCTCnt->getChartTypes());
        for( sal_Int32 i=0; i<aChartTypes.getLength(); ++i )
            if( aChartTypes[i] == xChartType )
                return true;
    }

    return false;
}

} // anonymous namespace

namespace chart
{

// static
DiagramHelper::tTemplateWithServiceName
    DiagramHelper::getTemplateForDiagram(
        const Reference< XDiagram > & xDiagram,
        const Reference< lang::XMultiServiceFactory > & xChartTypeManager,
        const OUString & rPreferredTemplateName )
{
    DiagramHelper::tTemplateWithServiceName aResult;

    if( ! (xChartTypeManager.is() && xDiagram.is()))
        return aResult;

    Sequence< OUString > aServiceNames( xChartTypeManager->getAvailableServiceNames());
    const sal_Int32 nLength = aServiceNames.getLength();

    bool bHasPreferredTemplate = (rPreferredTemplateName.getLength() > 0);
    bool bTemplateFound = false;

    if( bHasPreferredTemplate )
    {
        Reference< XChartTypeTemplate > xTempl(
            xChartTypeManager->createInstance( rPreferredTemplateName ), uno::UNO_QUERY );

        if( xTempl.is() &&
            xTempl->matchesTemplate( xDiagram, sal_True ))
        {
            aResult.first = xTempl;
            aResult.second = rPreferredTemplateName;
            bTemplateFound = true;
        }
    }

    for( sal_Int32 i = 0; ! bTemplateFound && i < nLength; ++i )
    {
        try
        {
            if( ! bHasPreferredTemplate ||
                ! rPreferredTemplateName.equals( aServiceNames[ i ] ))
            {
                Reference< XChartTypeTemplate > xTempl(
                    xChartTypeManager->createInstance( aServiceNames[ i ] ), uno::UNO_QUERY_THROW );

                if( xTempl->matchesTemplate( xDiagram, sal_True ))
                {
                    aResult.first = xTempl;
                    aResult.second = aServiceNames[ i ];
                    bTemplateFound = true;
                }
            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return aResult;
}

// static
void DiagramHelper::setVertical(
    const Reference< XDiagram > & xDiagram,
    bool bVertical /* = true */ )
{
    try
    {
        Reference< XCoordinateSystemContainer > xCnt( xDiagram, uno::UNO_QUERY );
        if( xCnt.is())
        {
            Sequence< Reference< XCoordinateSystem > > aCooSys(
                xCnt->getCoordinateSystems());
            uno::Any aValue;
            aValue <<= bVertical;
            for( sal_Int32 i=0; i<aCooSys.getLength(); ++i )
            {
                uno::Reference< XCoordinateSystem > xCooSys( aCooSys[i] );
                Reference< beans::XPropertySet > xProp( xCooSys, uno::UNO_QUERY );
                bool bChanged = false;
                if( xProp.is() )
                {
                    bool bOldSwap = sal_False;
                    if( !(xProp->getPropertyValue( C2U("SwapXAndYAxis") ) >>= bOldSwap)
                        || bVertical != bOldSwap )
                        bChanged = true;

                    if( bChanged )
                        xProp->setPropertyValue( C2U("SwapXAndYAxis"), aValue );
                }
                if( xCooSys.is() )
                {
                    const sal_Int32 nDimensionCount( xCooSys->getDimension() );
                    sal_Int32 nDimIndex = 0;
                    for(nDimIndex=0; nDimIndex<nDimensionCount; ++nDimIndex)
                    {
                        const sal_Int32 nMaximumScaleIndex = xCooSys->getMaximumAxisIndexByDimension(nDimIndex);
                        for(sal_Int32 nI=0; nI<=nMaximumScaleIndex; ++nI)
                        {
                            Reference< chart2::XAxis > xAxis( xCooSys->getAxisByDimension( nDimIndex,nI ));
                            if( xAxis.is() )
                            {
                                if( nDimensionCount == 2 && nDimIndex == 0 )
                                {
                                    //adapt scale orientation for 2D bar charts
                                    chart2::ScaleData aScaleData = xAxis->getScaleData();
                                    if( !bVertical != (AxisOrientation_MATHEMATICAL==aScaleData.Orientation) )
                                    {
                                        aScaleData.Orientation = bVertical ? AxisOrientation_REVERSE : AxisOrientation_MATHEMATICAL;
                                        xAxis->setScaleData( aScaleData );
                                    }
                                }

                                //adapt title rotation only when axis swapping has changed
                                if( bChanged )
                                {
                                    Reference< XTitled > xTitled( xAxis, uno::UNO_QUERY );
                                    if( xTitled.is())
                                    {
                                        Reference< beans::XPropertySet > xTitleProps( xTitled->getTitleObject(), uno::UNO_QUERY );
                                        if( !xTitleProps.is() )
                                            continue;
                                        double fAngleDegree = 0.0;
                                        xTitleProps->getPropertyValue( C2U( "TextRotation" ) ) >>= fAngleDegree;
                                        if( !::rtl::math::approxEqual( fAngleDegree, 0.0 )
                                            && !::rtl::math::approxEqual( fAngleDegree, 90.0 ) )
                                            continue;

                                        double fNewAngleDegree = 0.0;
                                        if( !bVertical && nDimIndex == 1 )
                                            fNewAngleDegree = 90.0;
                                        else if( bVertical && nDimIndex == 0 )
                                            fNewAngleDegree = 90.0;

                                        xTitleProps->setPropertyValue( C2U( "TextRotation" ), uno::makeAny( fNewAngleDegree ));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

//static
bool DiagramHelper::getVertical( const uno::Reference< chart2::XDiagram > & xDiagram,
                             bool& rbFound, bool& rbAmbiguous )
{
    bool bValue = false;
    rbFound = false;
    rbAmbiguous = false;

    Reference< XCoordinateSystemContainer > xCnt( xDiagram, uno::UNO_QUERY );
    if( xCnt.is())
    {
        Sequence< Reference< XCoordinateSystem > > aCooSys(
            xCnt->getCoordinateSystems());
        for( sal_Int32 i=0; i<aCooSys.getLength(); ++i )
        {
            Reference< beans::XPropertySet > xProp( aCooSys[i], uno::UNO_QUERY );
            if( xProp.is())
            {
                bool bCurrent = false;
                if( xProp->getPropertyValue( C2U("SwapXAndYAxis") ) >>= bCurrent )
                {
                    if( !rbFound )
                    {
                        bValue = bCurrent;
                        rbFound = true;
                    }
                    else if( bCurrent != bValue )
                    {
                        // ambiguous -> choose always first found
                        rbAmbiguous = true;
                    }
                }
            }
        }
    }
    return bValue;
}

//static
void DiagramHelper::setStackMode(
    const Reference< XDiagram > & xDiagram,
    StackMode eStackMode,
    bool bOnlyAtFirstChartType /* = false */
)
{
    try
    {
        if( eStackMode == StackMode_AMBIGUOUS )
            return;

        bool bValueFound = false;
        bool bIsAmbiguous = false;
        StackMode eOldStackMode = DiagramHelper::getStackMode( xDiagram, bValueFound, bIsAmbiguous );

        if( eStackMode == eOldStackMode && !bIsAmbiguous )
            return;

        StackingDirection eNewDirection = StackingDirection_NO_STACKING;
        if( eStackMode == StackMode_Y_STACKED || eStackMode == StackMode_Y_STACKED_PERCENT )
            eNewDirection = StackingDirection_Y_STACKING;
        else if( eStackMode == StackMode_Z_STACKED )
            eNewDirection = StackingDirection_Z_STACKING;

        uno::Any aNewDirection( uno::makeAny(eNewDirection) );

        sal_Bool bPercent = sal_False;
        if( eStackMode == StackMode_Y_STACKED_PERCENT )
            bPercent = sal_True;

        //iterate through all coordinate systems
        uno::Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
        if( !xCooSysContainer.is() )
            return;
        uno::Sequence< uno::Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
        for( sal_Int32 nCS = 0; nCS < aCooSysList.getLength(); ++nCS )
        {
            uno::Reference< XCoordinateSystem > xCooSys( aCooSysList[nCS] );
            //set correct percent stacking
            const sal_Int32 nMaximumScaleIndex = xCooSys->getMaximumAxisIndexByDimension(1);
            for(sal_Int32 nI=0; nI<=nMaximumScaleIndex; ++nI)
            {
                Reference< chart2::XAxis > xAxis( xCooSys->getAxisByDimension( 1,nI ));
                if( xAxis.is())
                {
                    chart2::ScaleData aScaleData = xAxis->getScaleData();
                    if( (aScaleData.AxisType==AxisType::PERCENT) != bPercent )
                    {
                        if( bPercent )
                            aScaleData.AxisType = AxisType::PERCENT;
                        else
                            aScaleData.AxisType = AxisType::REALNUMBER;
                        xAxis->setScaleData( aScaleData );
                    }
                }
            }
            //iterate through all chart types in the current coordinate system
            uno::Reference< XChartTypeContainer > xChartTypeContainer( xCooSys, uno::UNO_QUERY );
            if( !xChartTypeContainer.is() )
                continue;
            uno::Sequence< uno::Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
            sal_Int32 nMax = aChartTypeList.getLength();
            if( bOnlyAtFirstChartType
                && nMax >= 1 )
                nMax = 1;
            for( sal_Int32 nT = 0; nT < nMax; ++nT )
            {
                uno::Reference< XChartType > xChartType( aChartTypeList[nT] );

                //iterate through all series in this chart type
                uno::Reference< XDataSeriesContainer > xDataSeriesContainer( xChartType, uno::UNO_QUERY );
                OSL_ASSERT( xDataSeriesContainer.is());
                if( !xDataSeriesContainer.is() )
                    continue;

                uno::Sequence< uno::Reference< XDataSeries > > aSeriesList( xDataSeriesContainer->getDataSeries() );
                for( sal_Int32 nS = 0; nS < aSeriesList.getLength(); ++nS )
                {
                    Reference< beans::XPropertySet > xProp( aSeriesList[nS], uno::UNO_QUERY );
                    if(xProp.is())
                        xProp->setPropertyValue( C2U( "StackingDirection" ), aNewDirection );
                }
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

//static

StackMode DiagramHelper::getStackMode( const Reference< XDiagram > & xDiagram, bool& rbFound, bool& rbAmbiguous )
{
    rbFound=false;
    rbAmbiguous=false;

    StackMode eGlobalStackMode = StackMode_NONE;

    //iterate through all coordinate systems
    uno::Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( !xCooSysContainer.is() )
        return eGlobalStackMode;
    uno::Sequence< uno::Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
    for( sal_Int32 nCS = 0; nCS < aCooSysList.getLength(); ++nCS )
    {
        uno::Reference< XCoordinateSystem > xCooSys( aCooSysList[nCS] );

        //iterate through all chart types in the current coordinate system
        uno::Reference< XChartTypeContainer > xChartTypeContainer( xCooSys, uno::UNO_QUERY );
        if( !xChartTypeContainer.is() )
            continue;
        uno::Sequence< uno::Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
        for( sal_Int32 nT = 0; nT < aChartTypeList.getLength(); ++nT )
        {
            uno::Reference< XChartType > xChartType( aChartTypeList[nT] );

            StackMode eLocalStackMode = DiagramHelper::getStackModeFromChartType(
                xChartType, rbFound, rbAmbiguous, xCooSys );

            if( rbFound && eLocalStackMode != eGlobalStackMode && nT>0 )
            {
                rbAmbiguous = true;
                return eGlobalStackMode;
            }

            eGlobalStackMode = eLocalStackMode;
        }
    }

    return eGlobalStackMode;
}

// static
StackMode DiagramHelper::getStackModeFromChartType(
    const Reference< XChartType > & xChartType,
    bool& rbFound, bool& rbAmbiguous,
    const Reference< XCoordinateSystem > & xCorrespondingCoordinateSystem )
{
    OSL_ASSERT( !xCorrespondingCoordinateSystem.is() ||
                lcl_ChartTypeIsMemberOfCooSys(
                    xCorrespondingCoordinateSystem,
                    xChartType ));

    StackMode eStackMode = StackMode_NONE;
    rbFound = false;
    rbAmbiguous = false;

    try
    {
        Reference< XDataSeriesContainer > xDSCnt( xChartType, uno::UNO_QUERY_THROW );
        Sequence< Reference< chart2::XDataSeries > > aSeries( xDSCnt->getDataSeries());

        chart2::StackingDirection eCommonDirection = chart2::StackingDirection_NO_STACKING;
        bool bDirectionInitialized = false;

        // first series is irrelvant for stacking, start with second, unless
        // there is only one series
        const sal_Int32 nSeriesCount = aSeries.getLength();
        sal_Int32 i = (nSeriesCount == 1) ? 0: 1;
        for( ; i<nSeriesCount; ++i )
        {
            rbFound = true;
            Reference< beans::XPropertySet > xProp( aSeries[i], uno::UNO_QUERY_THROW );
            chart2::StackingDirection eCurrentDirection = eCommonDirection;
            // property is not MAYBEVOID
            bool bSuccess = ( xProp->getPropertyValue( C2U("StackingDirection") ) >>= eCurrentDirection );
            OSL_ASSERT( bSuccess );
            (void)(bSuccess);  // avoid warning in non-debug builds
            if( ! bDirectionInitialized )
            {
                eCommonDirection = eCurrentDirection;
                bDirectionInitialized = true;
            }
            else
            {
                if( eCommonDirection != eCurrentDirection )
                {
                    rbAmbiguous = true;
                    break;
                }
            }
        }

        if( rbFound )
        {
            if( eCommonDirection == chart2::StackingDirection_Z_STACKING )
                eStackMode = StackMode_Z_STACKED;
            else if( eCommonDirection == chart2::StackingDirection_Y_STACKING )
            {
                eStackMode = StackMode_Y_STACKED;

                // percent stacking
                if( xCorrespondingCoordinateSystem.is() )
                {
                    if( 1 < xCorrespondingCoordinateSystem->getDimension() )
                    {
                        sal_Int32 nAxisIndex = 0;
                        if( nSeriesCount )
                            nAxisIndex = DataSeriesHelper::getAttachedAxisIndex(aSeries[0]);

                        Reference< chart2::XAxis > xAxis(
                            xCorrespondingCoordinateSystem->getAxisByDimension( 1,nAxisIndex ));
                        if( xAxis.is())
                        {
                            chart2::ScaleData aScaleData = xAxis->getScaleData();
                            if( aScaleData.AxisType==chart2::AxisType::PERCENT )
                                eStackMode = StackMode_Y_STACKED_PERCENT;
                        }
                    }
                }
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return eStackMode;
}

// static
sal_Int32 DiagramHelper::getDimension( const Reference< XDiagram > & xDiagram )
{
    // -1: not yet set
    sal_Int32 nResult = -1;

    try
    {
        Reference< XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW );
        Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());

        for( sal_Int32 i=0; i<aCooSysSeq.getLength(); ++i )
        {
            Reference< XCoordinateSystem > xCooSys( aCooSysSeq[i] );
            if(xCooSys.is())
            {
                nResult = xCooSys->getDimension();
                break;
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return nResult;
}

// static
void DiagramHelper::setDimension(
    const Reference< XDiagram > & xDiagram,
    sal_Int32 nNewDimensionCount )
{
    if( ! xDiagram.is())
        return;

    if( DiagramHelper::getDimension( xDiagram ) == nNewDimensionCount )
        return;

    try
    {
        //change all coordinate systems:

        Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY_THROW );
        Sequence< Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
        for( sal_Int32 nCS = 0; nCS < aCooSysList.getLength(); ++nCS )
        {
            Reference< XCoordinateSystem > xOldCooSys( aCooSysList[nCS], uno::UNO_QUERY );
            Reference< XCoordinateSystem > xNewCooSys;

            Reference< XChartTypeContainer > xChartTypeContainer( xOldCooSys, uno::UNO_QUERY );
            if( !xChartTypeContainer.is() )
                continue;

            Sequence< Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
            for( sal_Int32 nT = 0; nT < aChartTypeList.getLength(); ++nT )
            {
                Reference< XChartType > xChartType( aChartTypeList[nT], uno::UNO_QUERY );
                if(!xNewCooSys.is())
                {
                    xNewCooSys = xChartType->createCoordinateSystem( nNewDimensionCount );
                    break;
                }
                //@todo make sure that all following charttypes are also capable of the new dimension
                //otherwise separate them in a different group
                //BM: might be done in replaceCoordinateSystem()
            }

            // replace the old coordinate system at all places where it was used
            DiagramHelper::replaceCoordinateSystem( xDiagram, xOldCooSys, xNewCooSys );
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// static
void DiagramHelper::replaceCoordinateSystem(
    const Reference< XDiagram > & xDiagram,
    const Reference< XCoordinateSystem > & xCooSysToReplace,
    const Reference< XCoordinateSystem > & xReplacement )
{
    OSL_ASSERT( xDiagram.is());
    if( ! xDiagram.is())
        return;

    // update the coordinate-system container
    Reference< XCoordinateSystemContainer > xCont( xDiagram, uno::UNO_QUERY );
    if( xCont.is())
    {
        try
        {
            // move chart types of xCooSysToReplace to xReplacement
            Reference< XChartTypeContainer > xCTCntCooSys( xCooSysToReplace, uno::UNO_QUERY_THROW );
            Reference< XChartTypeContainer > xCTCntReplacement( xReplacement, uno::UNO_QUERY_THROW );
            xCTCntReplacement->setChartTypes( xCTCntCooSys->getChartTypes());

            xCont->removeCoordinateSystem( xCooSysToReplace );
            xCont->addCoordinateSystem( xReplacement );
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

//static
bool DiagramHelper::isSeriesAttachedToMainAxis(
                          const uno::Reference< chart2::XDataSeries >& xDataSeries )
{
    sal_Int32 nAxisIndex = DataSeriesHelper::getAttachedAxisIndex(xDataSeries);
    return (nAxisIndex==0);
}

//static
bool DiagramHelper::attachSeriesToAxis( bool bAttachToMainAxis
                        , const uno::Reference< chart2::XDataSeries >& xDataSeries
                        , const uno::Reference< chart2::XDiagram >& xDiagram
                        , const uno::Reference< uno::XComponentContext > & xContext
                        )
{
    bool bChanged = false;

    //set property at axis
    Reference< beans::XPropertySet > xProp( xDataSeries, uno::UNO_QUERY_THROW );
    if( !xProp.is() )
        return bChanged;

    sal_Int32 nNewAxisIndex = bAttachToMainAxis ? 0 : 1;
    sal_Int32 nOldAxisIndex = DataSeriesHelper::getAttachedAxisIndex(xDataSeries);

    if( nOldAxisIndex != nNewAxisIndex )
    {
        try
        {
            xProp->setPropertyValue( C2U("AttachedAxisIndex"), uno::makeAny( nNewAxisIndex ) );
            bChanged = true;
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    if( bChanged && xDiagram.is() )
    {
        uno::Reference< XAxis > xAxis( AxisHelper::getAxis( 1, bAttachToMainAxis, xDiagram ) );
        if(!xAxis.is()) //create an axis if necessary
            xAxis = AxisHelper::createAxis( 1, bAttachToMainAxis, xDiagram, xContext );
    }

    return bChanged;
}

//static
uno::Reference< XAxis > DiagramHelper::getAttachedAxis(
        const uno::Reference< XDataSeries >& xSeries,
        const uno::Reference< XDiagram >& xDiagram )
{
    return AxisHelper::getAxis( 1, DiagramHelper::isSeriesAttachedToMainAxis( xSeries ), xDiagram );
}

//static
uno::Reference< XChartType > DiagramHelper::getChartTypeOfSeries(
                                const uno::Reference< chart2::XDiagram >&   xDiagram
                              , const uno::Reference< XDataSeries >&        xGivenDataSeries )
{
    if( !xGivenDataSeries.is() )
        return 0;
    if(!xDiagram.is())
        return 0;

    //iterate through the model to find the given xSeries
    //the found parent indicates the charttype

    //iterate through all coordinate systems
    uno::Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( !xCooSysContainer.is())
        return 0;

    uno::Sequence< uno::Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
    for( sal_Int32 nCS = 0; nCS < aCooSysList.getLength(); ++nCS )
    {
        uno::Reference< XCoordinateSystem > xCooSys( aCooSysList[nCS] );

        //iterate through all chart types in the current coordinate system
        uno::Reference< XChartTypeContainer > xChartTypeContainer( xCooSys, uno::UNO_QUERY );
        OSL_ASSERT( xChartTypeContainer.is());
        if( !xChartTypeContainer.is() )
            continue;
        uno::Sequence< uno::Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
        for( sal_Int32 nT = 0; nT < aChartTypeList.getLength(); ++nT )
        {
            uno::Reference< XChartType > xChartType( aChartTypeList[nT] );

            //iterate through all series in this chart type
            uno::Reference< XDataSeriesContainer > xDataSeriesContainer( xChartType, uno::UNO_QUERY );
            OSL_ASSERT( xDataSeriesContainer.is());
            if( !xDataSeriesContainer.is() )
                continue;

            uno::Sequence< uno::Reference< XDataSeries > > aSeriesList( xDataSeriesContainer->getDataSeries() );
            for( sal_Int32 nS = 0; nS < aSeriesList.getLength(); ++nS )
            {
                if( xGivenDataSeries==aSeriesList[nS] )
                    return xChartType;
            }
        }
    }
    return 0;
}

//static
uno::Reference< XCoordinateSystem > DiagramHelper::getCoordinateSystemOfChartType(
              const uno::Reference< chart2::XDiagram >& xDiagram
            , const uno::Reference< XChartType >& xGivenChartType )
{
    if( !xGivenChartType.is() )
        return 0;

    //iterate through the model to find the given xChartType
    //the found parent indicates the coordinate system

    //iterate through all coordinate systems
    uno::Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( !xCooSysContainer.is())
        return 0;

    uno::Sequence< uno::Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
    for( sal_Int32 nCS = 0; nCS < aCooSysList.getLength(); ++nCS )
    {
        uno::Reference< XCoordinateSystem > xCooSys( aCooSysList[nCS] );

        //iterate through all chart types in the current coordinate system
        uno::Reference< XChartTypeContainer > xChartTypeContainer( xCooSys, uno::UNO_QUERY );
        OSL_ASSERT( xChartTypeContainer.is());
        if( !xChartTypeContainer.is() )
            continue;
        uno::Sequence< uno::Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
        for( sal_Int32 nT = 0; nT < aChartTypeList.getLength(); ++nT )
        {
            uno::Reference< XChartType > xChartType( aChartTypeList[nT] );

            if( xGivenChartType==xChartType )
                return xCooSys;
        }
    }
    return 0;
}

// static
::std::vector< Reference< XDataSeries > >
    DiagramHelper::getDataSeriesFromDiagram(
        const Reference< XDiagram > & xDiagram )
{
    ::std::vector< Reference< XDataSeries > > aResult;

    try
    {
        Reference< XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW );
        Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 i=0; i<aCooSysSeq.getLength(); ++i )
        {
            Reference< XChartTypeContainer > xCTCnt( aCooSysSeq[i], uno::UNO_QUERY_THROW );
            Sequence< Reference< XChartType > > aChartTypeSeq( xCTCnt->getChartTypes());
            for( sal_Int32 j=0; j<aChartTypeSeq.getLength(); ++j )
            {
                Reference< XDataSeriesContainer > xDSCnt( aChartTypeSeq[j], uno::UNO_QUERY_THROW );
                Sequence< Reference< XDataSeries > > aSeriesSeq( xDSCnt->getDataSeries() );
                ::std::copy( aSeriesSeq.getConstArray(), aSeriesSeq.getConstArray() + aSeriesSeq.getLength(),
                             ::std::back_inserter( aResult ));
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return aResult;
}

Sequence< Sequence< Reference< XDataSeries > > >
        DiagramHelper::getDataSeriesGroups( const Reference< XDiagram > & xDiagram )
{
    vector< Sequence< Reference< XDataSeries > > > aResult;

    //iterate through all coordinate systems
    Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( xCooSysContainer.is() )
    {
        Sequence< Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
        for( sal_Int32 nCS = 0; nCS < aCooSysList.getLength(); ++nCS )
        {
            //iterate through all chart types in the current coordinate system
            Reference< XChartTypeContainer > xChartTypeContainer( aCooSysList[nCS], uno::UNO_QUERY );
            if( !xChartTypeContainer.is() )
                continue;
            Sequence< Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
            for( sal_Int32 nT = 0; nT < aChartTypeList.getLength(); ++nT )
            {
                Reference< XDataSeriesContainer > xDataSeriesContainer( aChartTypeList[nT], uno::UNO_QUERY );
                if( !xDataSeriesContainer.is() )
                    continue;
                aResult.push_back( xDataSeriesContainer->getDataSeries() );
            }
        }
    }
    return ContainerHelper::ContainerToSequence( aResult );
}

Reference< XChartType >
    DiagramHelper::getChartTypeByIndex( const Reference< XDiagram >& xDiagram, sal_Int32 nIndex )
{
    Reference< XChartType > xChartType;

    //iterate through all coordinate systems
    Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( ! xCooSysContainer.is())
        return xChartType;

    Sequence< Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
    sal_Int32 nTypesSoFar = 0;
    for( sal_Int32 nCS = 0; nCS < aCooSysList.getLength(); ++nCS )
    {
        Reference< XChartTypeContainer > xChartTypeContainer( aCooSysList[nCS], uno::UNO_QUERY );
        if( !xChartTypeContainer.is() )
            continue;
        Sequence< Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
        if( nIndex >= 0 && nIndex < (nTypesSoFar + aChartTypeList.getLength()) )
        {
            xChartType.set( aChartTypeList[nIndex - nTypesSoFar] );
            break;
        }
        nTypesSoFar += aChartTypeList.getLength();
    }

    return xChartType;
}

namespace
{

std::vector< Reference< XAxis > > lcl_getAxisHoldingCategoriesFromDiagram(
    const Reference< XDiagram > & xDiagram )
{
    std::vector< Reference< XAxis > > aRet;

    Reference< XAxis > xResult;
    // return first x-axis as fall-back
    Reference< XAxis > xFallBack;
    try
    {
        Reference< XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW );
        Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 i=0; i<aCooSysSeq.getLength(); ++i )
        {
            Reference< XCoordinateSystem > xCooSys( aCooSysSeq[i] );
            OSL_ASSERT( xCooSys.is());
            for( sal_Int32 nN = xCooSys->getDimension(); nN--; )
            {
                const sal_Int32 nMaximumScaleIndex = xCooSys->getMaximumAxisIndexByDimension(nN);
                for(sal_Int32 nI=0; nI<=nMaximumScaleIndex; ++nI)
                {
                    Reference< XAxis > xAxis = xCooSys->getAxisByDimension( nN,nI );
                    OSL_ASSERT( xAxis.is());
                    if( xAxis.is())
                    {
                        ScaleData aScaleData = xAxis->getScaleData();
                        if( aScaleData.Categories.is() || (aScaleData.AxisType == AxisType::CATEGORY) )
                        {
                            aRet.push_back(xAxis);
                        }
                        if( (nN == 0) && !xFallBack.is())
                            xFallBack.set( xAxis );
                    }
                }
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    if( aRet.empty() )
        aRet.push_back(xFallBack);

    return aRet;
}

} // anonymous namespace

//static
bool DiagramHelper::isCategoryDiagram(
            const Reference< XDiagram >& xDiagram )
{
    try
    {
        Reference< XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW );
        Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 i=0; i<aCooSysSeq.getLength(); ++i )
        {
            Reference< XCoordinateSystem > xCooSys( aCooSysSeq[i] );
            OSL_ASSERT( xCooSys.is());
            for( sal_Int32 nN = xCooSys->getDimension(); nN--; )
            {
                const sal_Int32 nMaximumScaleIndex = xCooSys->getMaximumAxisIndexByDimension(nN);
                for(sal_Int32 nI=0; nI<=nMaximumScaleIndex; ++nI)
                {
                    Reference< XAxis > xAxis = xCooSys->getAxisByDimension( nN,nI );
                    OSL_ASSERT( xAxis.is());
                    if( xAxis.is())
                    {
                        ScaleData aScaleData = xAxis->getScaleData();
                        if( aScaleData.AxisType == AxisType::CATEGORY )
                            return true;
                    }
                }
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return false;
}

// static
void DiagramHelper::setCategoriesToDiagram(
    const Reference< chart2::data::XLabeledDataSequence >& xCategories,
    const Reference< XDiagram >& xDiagram,
    bool bSetAxisType  /* = false */,
    bool bCategoryAxis /* = true */ )
{
    std::vector< Reference< chart2::XAxis > > aCatAxes(
        lcl_getAxisHoldingCategoriesFromDiagram( xDiagram ));

    std::vector< Reference< chart2::XAxis > >::iterator aIt( aCatAxes.begin() );
    std::vector< Reference< chart2::XAxis > >::const_iterator aEnd( aCatAxes.end() );

    for( aIt = aCatAxes.begin(); aIt != aEnd; ++aIt )
    {
        Reference< chart2::XAxis > xCatAxis(*aIt);
        if( xCatAxis.is())
        {
            ScaleData aScaleData( xCatAxis->getScaleData());
            aScaleData.Categories = xCategories;
            if( bSetAxisType )
            {
                if( bCategoryAxis )
                    aScaleData.AxisType = AxisType::CATEGORY;
                else if( aScaleData.AxisType == AxisType::CATEGORY )
                    aScaleData.AxisType = AxisType::REALNUMBER;
            }
            xCatAxis->setScaleData( aScaleData );
        }
    }
}

// static
Reference< data::XLabeledDataSequence >
    DiagramHelper::getCategoriesFromDiagram(
        const Reference< XDiagram > & xDiagram )
{
    Reference< data::XLabeledDataSequence > xResult;

    try
    {
        std::vector< Reference< chart2::XAxis > > aCatAxes(
            lcl_getAxisHoldingCategoriesFromDiagram( xDiagram ));
        std::vector< Reference< chart2::XAxis > >::iterator aIt( aCatAxes.begin() );
        std::vector< Reference< chart2::XAxis > >::const_iterator aEnd( aCatAxes.end() );
        //search for first categories
        if( aIt != aEnd )
        {
            Reference< chart2::XAxis > xCatAxis(*aIt);
            if( xCatAxis.is())
            {
                ScaleData aScaleData( xCatAxis->getScaleData());
                if( aScaleData.Categories.is() )
                {
                    xResult.set( aScaleData.Categories );
                    uno::Reference<beans::XPropertySet> xProp(aScaleData.Categories->getValues(), uno::UNO_QUERY );
                    if( xProp.is() )
                    {
                        try
                        {
                            xProp->setPropertyValue( C2U( "Role" ), uno::makeAny( C2U("categories") ) );
                        }
                        catch( uno::Exception & ex )
                        {
                            ASSERT_EXCEPTION( ex );
                        }
                    }
                }
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

//static
void DiagramHelper::generateAutomaticCategoriesFromChartType(
            Sequence< rtl::OUString >& rRet,
            const Reference< XChartType >& xChartType )
{
    if(!xChartType.is())
        return;
    rtl::OUString aMainSeq( xChartType->getRoleOfSequenceForSeriesLabel() );
        Reference< XDataSeriesContainer > xSeriesCnt( xChartType, uno::UNO_QUERY );
    if( xSeriesCnt.is() )
    {
        Sequence< Reference< XDataSeries > > aSeriesSeq( xSeriesCnt->getDataSeries() );
        for( sal_Int32 nS = 0; nS < aSeriesSeq.getLength(); nS++ )
        {
            Reference< data::XDataSource > xDataSource( aSeriesSeq[nS], uno::UNO_QUERY );
            if( !xDataSource.is() )
                continue;
            Reference< chart2::data::XLabeledDataSequence > xLabeledSeq(
                ::chart::DataSeriesHelper::getDataSequenceByRole( xDataSource, aMainSeq ));
            if( !xLabeledSeq.is() )
                continue;
            Reference< chart2::data::XDataSequence > xValueSeq( xLabeledSeq->getValues() );
            if( !xValueSeq.is() )
                continue;
            rRet = xValueSeq->generateLabel( chart2::data::LabelOrigin_LONG_SIDE );
            if( rRet.getLength() )
                return;
        }
    }
}

//static
Sequence< rtl::OUString > DiagramHelper::generateAutomaticCategories(
            const Reference< XChartDocument >& xChartDoc )
{
    Sequence< rtl::OUString > aRet;
    if(xChartDoc.is())
    {
        uno::Reference< chart2::XDiagram > xDia( xChartDoc->getFirstDiagram() );
        if(xDia.is())
        {
            Reference< data::XLabeledDataSequence > xCategories( DiagramHelper::getCategoriesFromDiagram( xDia ) );
            if( xCategories.is() )
                aRet = DataSequenceToStringSequence(xCategories->getValues());
            if( !aRet.getLength() )
            {
                /*
                //unused ranges are very problematic as they bear the risk to damage the rectangular structure completly
                if( bUseUnusedDataAlso )
                {
                    Sequence< Reference< chart2::data::XLabeledDataSequence > > aUnusedSequences( xDia->getUnusedData() );
                    ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aUnusedCategoryVector(
                        DataSeriesHelper::getAllDataSequencesByRole( aUnusedSequences, C2U("categories") ) );
                    if( aUnusedCategoryVector.size() && aUnusedCategoryVector[0].is() )
                        aRet = DataSequenceToStringSequence(aUnusedCategoryVector[0]->getValues());
                }
                */
                if( !aRet.getLength() )
                {
                    Reference< XCoordinateSystemContainer > xCooSysCnt( xDia, uno::UNO_QUERY );
                    if( xCooSysCnt.is() )
                    {
                        Sequence< Reference< XCoordinateSystem > > aCooSysSeq( xCooSysCnt->getCoordinateSystems() );
                        if( aCooSysSeq.getLength() )
                            aRet = DiagramHelper::generateAutomaticCategories( aCooSysSeq[0] );
                    }
                }
            }
        }
    }
    return aRet;
}

//static
Sequence< rtl::OUString > DiagramHelper::generateAutomaticCategories(
            const Reference< XCoordinateSystem > & xCooSys )
{
    Sequence< rtl::OUString > aRet;

    Reference< XChartTypeContainer > xTypeCntr( xCooSys, uno::UNO_QUERY );
    if( xTypeCntr.is() )
    {
        Sequence< Reference< XChartType > > aChartTypes( xTypeCntr->getChartTypes() );
        for( sal_Int32 nN=0; nN<aChartTypes.getLength(); nN++ )
        {
            DiagramHelper::generateAutomaticCategoriesFromChartType( aRet, aChartTypes[nN] );
            if( aRet.getLength() )
                return aRet;
        }
    }
    return aRet;
}

// static
Sequence< Reference< XChartType > >
    DiagramHelper::getChartTypesFromDiagram(
        const Reference< XDiagram > & xDiagram )
{
    ::std::vector< Reference< XChartType > > aResult;

    if(xDiagram.is())
    try
    {
        Reference< XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW );
        Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 i=0; i<aCooSysSeq.getLength(); ++i )
        {
            Reference< XChartTypeContainer > xCTCnt( aCooSysSeq[i], uno::UNO_QUERY_THROW );
            Sequence< Reference< XChartType > > aChartTypeSeq( xCTCnt->getChartTypes());
            ::std::copy( aChartTypeSeq.getConstArray(), aChartTypeSeq.getConstArray() + aChartTypeSeq.getLength(),
                         ::std::back_inserter( aResult ));
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return ContainerHelper::ContainerToSequence( aResult );
}

//static
bool DiagramHelper::areChartTypesCompatible( const Reference< ::chart2::XChartType >& xFirstType,
                const Reference< ::chart2::XChartType >& xSecondType )
{
    if( !xFirstType.is() || !xSecondType.is() )
        return false;

    ::std::vector< ::rtl::OUString > aFirstRoles( ContainerHelper::SequenceToVector( xFirstType->getSupportedMandatoryRoles() ) );
    ::std::vector< ::rtl::OUString > aSecondRoles( ContainerHelper::SequenceToVector( xSecondType->getSupportedMandatoryRoles() ) );
    ::std::sort( aFirstRoles.begin(), aFirstRoles.end() );
    ::std::sort( aSecondRoles.begin(), aSecondRoles.end() );
    return ( aFirstRoles == aSecondRoles );
}

namespace
{
     /**
     * This method implements the logic of checking if a series can be moved
     * forward/backward. Depending on the "bDoMove" parameter the series will
     * be moved (bDoMove = true) or the function just will test if the
     * series can be moved without doing the move (bDoMove = false).
     *
     * @param xDiagram
     *  Reference to the diagram that contains the series.
     *
     * @param xGivenDataSeries
     *  Reference to the series that should moved or tested for moving.
     *
     * @param bForward
     *  Direction in which the series should be moved or tested for moving.
     *
     * @param bDoMove
     *  Should this function really move the series (true) or just test if it is
     *  possible (false).
     *
     *
     * @returns
     *  in case of bDoMove == true
     *      - True : if the move was done
     *      - False : the move failed
     *  in case of bDoMove == false
     *      - True : the series can be moved
     *      - False : the series can not be moved
     *
     */

bool lcl_moveSeriesOrCheckIfMoveIsAllowed(
    const Reference< XDiagram >& xDiagram,
    const Reference< XDataSeries >& xGivenDataSeries,
    bool bForward,
    bool bDoMove )
{
    bool bMovedOrMoveAllowed = false;

    try
    {
        uno::Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );

        //find position of series.
        bool bFound = false;

        if( xGivenDataSeries.is() && xCooSysContainer.is() )
        {
            uno::Sequence< uno::Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );

            for( sal_Int32 nCS = 0; !bFound && nCS < aCooSysList.getLength(); ++nCS )
            {
                uno::Reference< XCoordinateSystem > xCooSys( aCooSysList[nCS] );

                //iterate through all chart types in the current coordinate system
                uno::Reference< XChartTypeContainer > xChartTypeContainer( xCooSys, uno::UNO_QUERY );
                OSL_ASSERT( xChartTypeContainer.is());
                if( !xChartTypeContainer.is() )
                    continue;
                uno::Sequence< uno::Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
                uno::Reference< XChartType > xFormerChartType;

                for( sal_Int32 nT = 0; !bFound && nT < aChartTypeList.getLength(); ++nT )
                {
                    uno::Reference< XChartType > xCurrentChartType( aChartTypeList[nT] );

                    //iterate through all series in this chart type
                    uno::Reference< XDataSeriesContainer > xDataSeriesContainer( xCurrentChartType, uno::UNO_QUERY );
                    OSL_ASSERT( xDataSeriesContainer.is());
                    if( !xDataSeriesContainer.is() )
                        continue;

                    uno::Sequence< uno::Reference< XDataSeries > > aSeriesList( xDataSeriesContainer->getDataSeries() );

                    for( sal_Int32 nS = 0; !bFound && nS < aSeriesList.getLength(); ++nS )
                    {

                        // We found the series we are interrested in !
                        if( xGivenDataSeries==aSeriesList[nS] )
                        {
                            sal_Int32 nOldSeriesIndex = nS;
                            bFound = true;

                            try
                            {
                                sal_Int32 nNewSeriesIndex = nS;

                                if( bForward )
                                    nNewSeriesIndex--;
                                else
                                    nNewSeriesIndex++;


                                if( nNewSeriesIndex >= 0 && nNewSeriesIndex < aSeriesList.getLength() )
                                {
                                    //move series in the same charttype
                                    bMovedOrMoveAllowed = true;
                                    if( bDoMove )
                                    {
                                        aSeriesList[ nOldSeriesIndex ] = aSeriesList[ nNewSeriesIndex ];
                                        aSeriesList[ nNewSeriesIndex ] = xGivenDataSeries;
                                        xDataSeriesContainer->setDataSeries( aSeriesList );
                                    }
                                }
                                else if( nNewSeriesIndex<0 )
                                {
                                    //exchange series with former charttype
                                    if( xFormerChartType.is() && DiagramHelper::areChartTypesCompatible( xFormerChartType, xCurrentChartType ) )
                                    {
                                        bMovedOrMoveAllowed = true;
                                        if( bDoMove )
                                        {
                                            uno::Reference< XDataSeriesContainer > xOtherDataSeriesContainer( xFormerChartType, uno::UNO_QUERY );
                                            if( xOtherDataSeriesContainer.is() )
                                            {
                                                uno::Sequence< uno::Reference< XDataSeries > > aOtherSeriesList( xOtherDataSeriesContainer->getDataSeries() );
                                                sal_Int32 nOtherSeriesIndex = aOtherSeriesList.getLength()-1;
                                                if( nOtherSeriesIndex >= 0 && nOtherSeriesIndex < aOtherSeriesList.getLength() )
                                                {
                                                    uno::Reference< XDataSeries > xExchangeSeries( aOtherSeriesList[nOtherSeriesIndex] );
                                                    aOtherSeriesList[nOtherSeriesIndex] = xGivenDataSeries;
                                                    xOtherDataSeriesContainer->setDataSeries(aOtherSeriesList);

                                                    aSeriesList[nOldSeriesIndex]=xExchangeSeries;
                                                    xDataSeriesContainer->setDataSeries(aSeriesList);
                                                }
                                            }
                                        }
                                    }
                                }
                                else if( nT+1 < aChartTypeList.getLength() )
                                {
                                    //exchange series with next charttype
                                    uno::Reference< XChartType > xOtherChartType( aChartTypeList[nT+1] );
                                    if( xOtherChartType.is() && DiagramHelper::areChartTypesCompatible( xOtherChartType, xCurrentChartType ) )
                                    {
                                        bMovedOrMoveAllowed = true;
                                        if( bDoMove )
                                        {
                                            uno::Reference< XDataSeriesContainer > xOtherDataSeriesContainer( xOtherChartType, uno::UNO_QUERY );
                                            if( xOtherDataSeriesContainer.is() )
                                            {
                                                uno::Sequence< uno::Reference< XDataSeries > > aOtherSeriesList( xOtherDataSeriesContainer->getDataSeries() );
                                                sal_Int32 nOtherSeriesIndex = 0;
                                                if( nOtherSeriesIndex >= 0 && nOtherSeriesIndex < aOtherSeriesList.getLength() )
                                                {
                                                    uno::Reference< XDataSeries > xExchangeSeries( aOtherSeriesList[nOtherSeriesIndex] );
                                                    aOtherSeriesList[nOtherSeriesIndex] = xGivenDataSeries;
                                                    xOtherDataSeriesContainer->setDataSeries(aOtherSeriesList);

                                                    aSeriesList[nOldSeriesIndex]=xExchangeSeries;
                                                    xDataSeriesContainer->setDataSeries(aSeriesList);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            catch( util::CloseVetoException& )
                            {
                            }
                            catch( uno::RuntimeException& )
                            {
                            }
                        }
                    }
                    xFormerChartType = xCurrentChartType;
                }
            }
        }
    }
    catch( util::CloseVetoException& )
    {
    }
    catch( uno::RuntimeException& )
    {
    }
    return bMovedOrMoveAllowed;
}
} // anonymous namespace


bool DiagramHelper::isSeriesMoveable(
    const Reference< XDiagram >& xDiagram,
    const Reference< XDataSeries >& xGivenDataSeries,
    bool bForward )
{
    bool bIsMoveable = false;
    const bool bDoMove = false;

    bIsMoveable = lcl_moveSeriesOrCheckIfMoveIsAllowed(
        xDiagram, xGivenDataSeries, bForward, bDoMove );

    return bIsMoveable;
}


bool DiagramHelper::moveSeries( const Reference< XDiagram >& xDiagram, const Reference< XDataSeries >& xGivenDataSeries, bool bForward )
{
    bool bMoved = false;
    const bool bDoMove = true;

    bMoved = lcl_moveSeriesOrCheckIfMoveIsAllowed(
        xDiagram, xGivenDataSeries, bForward, bDoMove );

    return bMoved;
}

sal_Int32 DiagramHelper::getIndexOfSeriesWithinChartType(
                const Reference< XDataSeries >& xDataSeries,
               const Reference< XChartType >& xChartType )
{
    sal_Int32 nRet = -1;

    uno::Reference< XDataSeriesContainer > xDataSeriesContainer( xChartType, uno::UNO_QUERY );
    if( xDataSeriesContainer.is() )
    {
        uno::Sequence< uno::Reference< XDataSeries > > aSeriesList( xDataSeriesContainer->getDataSeries() );
        for( sal_Int32 nS = 0; nS < aSeriesList.getLength(); ++nS )
        {
            if( xDataSeries==aSeriesList[nS] )
            {
                nRet = nS;
                break;
            }
        }
    }

    return nRet;
}

bool DiagramHelper::isSupportingFloorAndWall( const Reference<
                chart2::XDiagram >& xDiagram )
{
    //pies and donuts currently do not support this because of wrong files from older versions
    //todo: allow this in future again, if fileversion are available for ole objects (metastream)
    //thus the wrong bottom can be removed on import

    Sequence< Reference< chart2::XChartType > > aTypes(
            ::chart::DiagramHelper::getChartTypesFromDiagram( xDiagram ) );
    for( sal_Int32 nN = 0; nN < aTypes.getLength(); nN++ )
    {
        Reference< chart2::XChartType > xType( aTypes[nN] );
        if( xType.is() && xType->getChartType().match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            return false;
        if( xType.is() && xType->getChartType().match(CHART2_SERVICE_NAME_CHARTTYPE_NET) )
            return false;
    }
    return true;
}

bool DiagramHelper::isPieOrDonutChart( const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDiagram >& xDiagram )
{
    uno::Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeByIndex(
        xDiagram, 0 ) );

    if( xChartType .is() )
    {
        rtl::OUString aChartType = xChartType->getChartType();
        if( aChartType.equals(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            return true;
    }
    return false;
}

// static
sal_Int32 DiagramHelper::getGeometry3D(
    const uno::Reference< chart2::XDiagram > & xDiagram,
    bool& rbFound, bool& rbAmbiguous )
{
    sal_Int32 nCommonGeom( DataPointGeometry3D::CUBOID );
    rbFound = false;
    rbAmbiguous = false;

    ::std::vector< Reference< chart2::XDataSeries > > aSeriesVec(
        DiagramHelper::getDataSeriesFromDiagram( xDiagram ));

    if( aSeriesVec.empty())
        rbAmbiguous = true;

    for( ::std::vector< Reference< chart2::XDataSeries > >::const_iterator aIt =
             aSeriesVec.begin(); aIt != aSeriesVec.end(); ++aIt )
    {
        try
        {
            sal_Int32 nGeom = 0;
            Reference< beans::XPropertySet > xProp( *aIt, uno::UNO_QUERY_THROW );
            if( xProp->getPropertyValue( C2U( "Geometry3D" )) >>= nGeom )
            {
                if( ! rbFound )
                {
                    // first series
                    nCommonGeom = nGeom;
                    rbFound = true;
                }
                // further series: compare for uniqueness
                else if( nCommonGeom != nGeom )
                {
                    rbAmbiguous = true;
                    break;
                }
            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return nCommonGeom;
}

// static
void DiagramHelper::setGeometry3D(
    const Reference< chart2::XDiagram > & xDiagram,
    sal_Int32 nNewGeometry )
{
    ::std::vector< Reference< chart2::XDataSeries > > aSeriesVec(
        DiagramHelper::getDataSeriesFromDiagram( xDiagram ));

    for( ::std::vector< Reference< chart2::XDataSeries > >::const_iterator aIt =
             aSeriesVec.begin(); aIt != aSeriesVec.end(); ++aIt )
    {
        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints(
            *aIt, C2U( "Geometry3D" ), uno::makeAny( nNewGeometry ));
    }
}

} //  namespace chart
