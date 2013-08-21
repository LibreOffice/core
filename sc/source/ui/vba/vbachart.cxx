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
#include "vbachart.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/chart/XAxisXSupplier.hpp>
#include <com/sun/star/chart/XAxisYSupplier.hpp>
#include <com/sun/star/chart/XAxisZSupplier.hpp>
#include <com/sun/star/chart/XTwoAxisXSupplier.hpp>
#include <com/sun/star/chart/XTwoAxisYSupplier.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/chart/ChartSymbolType.hpp>
#include <com/sun/star/chart/ChartSolidType.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/ChartDataCaption.hpp>
#include <ooo/vba/excel/XlChartType.hpp>
#include <ooo/vba/excel/XlRowCol.hpp>
#include <ooo/vba/excel/XlAxisType.hpp>
#include <ooo/vba/excel/XlAxisGroup.hpp>

#include <basic/sberrors.hxx>
#include "vbachartobject.hxx"
#include "vbarange.hxx"
#include "vbacharttitle.hxx"
#include "vbaaxes.hxx"

using namespace ::com::sun::star;
using namespace ::ooo::vba;
using namespace ::ooo::vba::excel::XlChartType;
using namespace ::ooo::vba::excel::XlRowCol;
using namespace ::ooo::vba::excel::XlAxisType;
using namespace ::ooo::vba::excel::XlAxisGroup;

const OUString CHART_NAME("Name");
// #TODO move this constant to vbaseries.[ch]xx ( when it exists )
const OUString DEFAULTSERIESPREFIX("Series");
const OUString DATAROWSOURCE("DataRowSource");
const OUString UPDOWN("UpDown");
const OUString VOLUME("Volume");
const OUString LINES("Lines");
const OUString SPLINETYPE("SplineType");
const OUString SYMBOLTYPE("SymbolType");
const OUString DEEP("Deep");
const OUString SOLIDTYPE("SolidType");
const OUString VERTICAL("Vertical");
const OUString PERCENT("Percent");
const OUString STACKED("Stacked");
const OUString DIM3D("Dim3D");
const OUString HASMAINTITLE("HasMainTitle");
const OUString HASLEGEND("HasLegend");

ScVbaChart::ScVbaChart( const css::uno::Reference< ov::XHelperInterface >& _xParent, const css::uno::Reference< css::uno::XComponentContext >& _xContext, const css::uno::Reference< css::lang::XComponent >& _xChartComponent, const css::uno::Reference< css::table::XTableChart >& _xTableChart ) : ChartImpl_BASE( _xParent, _xContext ), mxTableChart( _xTableChart )
{
    mxChartDocument.set( _xChartComponent, uno::UNO_QUERY_THROW ) ;
    // #TODO is is possible that the XPropertySet interface is not set
    // code in setPlotBy seems to indicate that this is possible? but
    // additionally there is no check in most of the places where it is used
    // ( and therefore could possibly be NULL )
    // I'm going to let it throw for the moment ( npower )
    mxDiagramPropertySet.set( mxChartDocument->getDiagram(), uno::UNO_QUERY_THROW );
    mxChartPropertySet.set( _xChartComponent, uno::UNO_QUERY_THROW ) ;
}

OUString SAL_CALL
ScVbaChart::getName() throw (css::uno::RuntimeException)
{
    OUString sName;
    uno::Reference< beans::XPropertySet > xProps( mxChartDocument, uno::UNO_QUERY_THROW );
    try
    {
        xProps->getPropertyValue( CHART_NAME ) >>= sName;
    }
    catch( const uno::Exception & ) // swallow exceptions
    {
    }
    return sName;
}

uno::Any  SAL_CALL
ScVbaChart::SeriesCollection(const uno::Any&) throw (uno::RuntimeException)
{
    return uno::Any();
}

::sal_Int32 SAL_CALL
ScVbaChart::getChartType() throw ( uno::RuntimeException, script::BasicErrorException)
{
    sal_Int32 nChartType = -1;
    try
    {
        OUString sDiagramType = mxChartDocument->getDiagram()->getDiagramType();
        if ( sDiagramType == "com.sun.star.chart.AreaDiagram" )
        {
            if (is3D())
            {
                nChartType = getStackedType(xl3DAreaStacked, xl3DAreaStacked100, xl3DArea);
            }
            else
            {
                nChartType = getStackedType(xlAreaStacked, xlAreaStacked100, xlArea);
            }
        }
        else if ( sDiagramType == "com.sun.star.chart.PieDiagram" )
        {
            if (is3D())
                nChartType = xl3DPie;
            else
                nChartType = xlPie;                 /*TODO XlChartType  xlPieExploded, XlChartType xlPieOfPie */
        }
        else if ( sDiagramType == "com.sun.star.chart.BarDiagram" )
        {
            sal_Int32 nSolidType = chart::ChartSolidType::RECTANGULAR_SOLID;
            if (mxDiagramPropertySet->getPropertySetInfo()->hasPropertyByName(SOLIDTYPE))
            {       //in 2D diagrams 'SolidType' may not be set
                if (is3D())
                    mxDiagramPropertySet->getPropertyValue(SOLIDTYPE) >>= nSolidType;
            }
            switch (nSolidType)
            {
                case chart::ChartSolidType::CONE:
                    nChartType = getSolidType(xlConeCol, xlConeColStacked, xlConeColStacked100, xlConeColClustered, xlConeBarStacked, xlConeBarStacked100, xlConeBarClustered);
                    break;
                case chart::ChartSolidType::CYLINDER:
                    nChartType = getSolidType(xlCylinderCol, xlCylinderColStacked, xlCylinderColStacked100, xlCylinderColClustered, xlCylinderBarStacked, xlCylinderBarStacked100, xlCylinderBarClustered);
                    break;
                case chart::ChartSolidType::PYRAMID:
                    nChartType = getSolidType(xlPyramidCol, xlPyramidColStacked, xlPyramidColStacked100, xlPyramidColClustered, xlPyramidBarStacked, xlPyramidBarStacked100, xlPyramidBarClustered);
                    break;
                default: // RECTANGULAR_SOLID
                    if (is3D())
                    {
                        nChartType = getSolidType(xl3DColumn, xl3DColumnStacked, xl3DColumnStacked100, xl3DColumnClustered, xl3DBarStacked, xl3DBarStacked100, xl3DBarClustered);
                    }
                    else
                    {
                        nChartType = getSolidType(xlColumnClustered,  xlColumnStacked, xlColumnStacked100, xlColumnClustered, xlBarStacked, xlBarStacked100, xlBarClustered);
                    }
                    break;
                }
            }
        else if ( sDiagramType == "com.sun.star.chart.StockDiagram" )
        {
            sal_Bool bVolume = false;
            mxDiagramPropertySet->getPropertyValue(VOLUME) >>= bVolume;
            if (bVolume)
            {
                nChartType = getStockUpDownValue(xlStockVOHLC, xlStockVHLC);
            }
            else
            {
                nChartType = getStockUpDownValue(xlStockOHLC, xlStockHLC);
            }
        }
        else if ( sDiagramType == "com.sun.star.chart.XYDiagram" )
        {
            sal_Bool bHasLines = false;
            mxDiagramPropertySet->getPropertyValue(LINES) >>= bHasLines;
            sal_Int32 nSplineType = 0;
            mxDiagramPropertySet->getPropertyValue(SPLINETYPE) >>= nSplineType;
            if (nSplineType == 1)
            {
                nChartType = getMarkerType(xlXYScatterSmooth, xlXYScatterSmoothNoMarkers);
            }
            else if (bHasLines)
            {
                nChartType = getMarkerType(xlXYScatterLines, xlXYScatterLinesNoMarkers);
            }
            else
            {
                nChartType = xlXYScatter;
            }
        }
        else if ( sDiagramType == "com.sun.star.chart.LineDiagram" )
        {
            if (is3D())
            {
                nChartType = xl3DLine;
            }
            else if (hasMarkers())
            {
                nChartType = getStackedType(xlLineMarkersStacked, xlLineMarkersStacked100, xlLineMarkers);
            }
            else
            {
                nChartType = getStackedType(xlLineStacked, xlLineStacked100, xlLine);
            }
        }
        else if ( sDiagramType == "com.sun.star.chart.DonutDiagram" )
        {
            nChartType = xlDoughnut;                    // TODO DoughnutExploded ??
        }
        else if ( sDiagramType == "com.sun.star.chart.NetDiagram" )
        {
            nChartType = getMarkerType(xlRadarMarkers, xlRadar);
        }
    }
    catch ( const uno::Exception& )
    {
        throw script::BasicErrorException( OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, OUString() );
    }
    return nChartType;
}

void SAL_CALL
ScVbaChart::setChartType( ::sal_Int32 _nChartType ) throw ( uno::RuntimeException, script::BasicErrorException)
{
try
{
    switch (_nChartType)
    {
        case xlColumnClustered:
        case xlColumnStacked:
        case xlColumnStacked100:
        case xl3DColumnClustered:
        case xl3DColumnStacked:
        case xl3DColumnStacked100:
        case xl3DColumn:
        case xlBarClustered:
        case xlBarStacked:
        case xlBarStacked100:
        case xl3DBarClustered:
        case xl3DBarStacked:
        case xl3DBarStacked100:
        case xlConeColClustered:
        case xlConeColStacked:
        case xlConeColStacked100:
        case xlConeBarClustered:
        case xlConeBarStacked:
        case xlConeBarStacked100:
        case xlConeCol:
        case xlPyramidColClustered:
        case xlPyramidColStacked:
        case xlPyramidColStacked100:
        case xlPyramidBarClustered:
        case xlPyramidBarStacked:
        case xlPyramidBarStacked100:
        case xlPyramidCol:
        case xlCylinderColClustered:
        case xlCylinderColStacked:
        case xlCylinderColStacked100:
        case xlCylinderBarClustered:
        case xlCylinderBarStacked:
        case xlCylinderBarStacked100:
        case xlCylinderCol:
        case xlSurface: // not possible
        case xlSurfaceWireframe:
        case xlSurfaceTopView:
        case xlSurfaceTopViewWireframe:
            setDiagram( OUString("com.sun.star.chart.BarDiagram"));
            break;
        case xlLine:
        case xl3DLine:
        case xlLineStacked:
        case xlLineStacked100:
        case xlLineMarkers:
        case xlLineMarkersStacked:
        case xlLineMarkersStacked100:
            setDiagram( OUString("com.sun.star.chart.LineDiagram"));
            break;
        case xl3DArea:
        case xlArea:
        case xlAreaStacked:
        case xlAreaStacked100:
        case xl3DAreaStacked:
        case xl3DAreaStacked100:
            setDiagram( OUString("com.sun.star.chart.AreaDiagram") );
            break;
        case xlDoughnut:
        case xlDoughnutExploded:
            setDiagram( OUString("com.sun.star.chart.DonutDiagram") );
            break;
        case xlStockHLC:
        case xlStockOHLC:
        case xlStockVHLC:
        case xlStockVOHLC:
            setDiagram( OUString("com.sun.star.chart.StockDiagram"));
            mxDiagramPropertySet->setPropertyValue( UPDOWN, uno::makeAny(sal_Bool((_nChartType == xlStockOHLC) || (_nChartType == xlStockVOHLC))));
            mxDiagramPropertySet->setPropertyValue(VOLUME, uno::makeAny(sal_Bool((_nChartType == xlStockVHLC) || (_nChartType == xlStockVOHLC))));
            break;

        case xlPieOfPie:                            // not possible
        case xlPieExploded: // SegmentOffset an ChartDataPointProperties ->am XDiagram abholen //wie macht Excel das?
        case xl3DPieExploded:
        case xl3DPie:
        case xlPie:
        case xlBarOfPie:                            // not possible (Zoom pie)
            setDiagram( OUString("com.sun.star.chart.PieDiagram"));
            break;

        case xlRadar:
        case xlRadarMarkers:
        case xlRadarFilled:
            setDiagram( OUString("com.sun.star.chart.NetDiagram"));
            break;
        case xlXYScatter:
        case xlBubble:                      // not possible
        case xlBubble3DEffect:              // not possible
        case xlXYScatterLines:
        case xlXYScatterLinesNoMarkers:
        case xlXYScatterSmooth:
        case xlXYScatterSmoothNoMarkers:
            setDiagram( OUString("com.sun.star.chart.XYDiagram"));
            switch(_nChartType)
            {
                case xlXYScatter:
                case xlBubble:                      // not possible
                case xlBubble3DEffect:              // not possible
                    mxDiagramPropertySet->setPropertyValue(LINES, uno::makeAny( sal_False ));
                    break;
                case xlXYScatterLines:
                case xlXYScatterLinesNoMarkers:
                    mxDiagramPropertySet->setPropertyValue(LINES, uno::makeAny( sal_True ));
                    break;
                case xlXYScatterSmooth:
                case xlXYScatterSmoothNoMarkers:
                    mxDiagramPropertySet->setPropertyValue(SPLINETYPE, uno::makeAny( sal_Int32(1)));
                    break;
                default:
                    break;
            }
            break;
        default:
            throw script::BasicErrorException( OUString(), uno::Reference< uno::XInterface >(), SbERR_CONVERSION, OUString() );
    }

    switch (_nChartType)
    {
        case xlLineMarkers:
        case xlLineMarkersStacked:
        case xlLineMarkersStacked100:
        case xlRadarMarkers:
        case xlXYScatterLines:
        case xlXYScatterSmooth:
        case xlXYScatter:
        case xlBubble:                      // not possible
        case xlBubble3DEffect:              // not possible
            mxDiagramPropertySet->setPropertyValue(SYMBOLTYPE, uno::makeAny( chart::ChartSymbolType::AUTO));
            break;
        default:
            if (mxDiagramPropertySet->getPropertySetInfo()->hasPropertyByName(SYMBOLTYPE))
            {
                mxDiagramPropertySet->setPropertyValue(SYMBOLTYPE, uno::makeAny(chart::ChartSymbolType::NONE));
            }
            break;
    }

    switch (_nChartType)
    {
        case xlConeCol:
        case xlPyramidCol:
        case xlCylinderCol:
        case xl3DColumn:
        case xlSurface:                         // not possible
        case xlSurfaceWireframe:
        case xlSurfaceTopView:
        case xlSurfaceTopViewWireframe:
            mxDiagramPropertySet->setPropertyValue(DEEP,uno::makeAny( sal_True ));
            break;
        default:
                if (mxDiagramPropertySet->getPropertySetInfo()->hasPropertyByName(DEEP))
                {
                    mxDiagramPropertySet->setPropertyValue(DEEP, uno::makeAny( sal_False));
                }
                break;
        }


        switch (_nChartType)
        {
                case xlConeColClustered:
                case xlConeColStacked:
                case xlConeColStacked100:
                case xlConeBarClustered:
                case xlConeBarStacked:
                case xlConeBarStacked100:
                case xlConeCol:
                        mxDiagramPropertySet->setPropertyValue(SOLIDTYPE, uno::makeAny(chart::ChartSolidType::CONE));
                        break;
                case xlPyramidColClustered:
                case xlPyramidColStacked:
                case xlPyramidColStacked100:
                case xlPyramidBarClustered:
                case xlPyramidBarStacked:
                case xlPyramidBarStacked100:
                case xlPyramidCol:
                        mxDiagramPropertySet->setPropertyValue(SOLIDTYPE, uno::makeAny(chart::ChartSolidType::PYRAMID));
                        break;
                case xlCylinderColClustered:
                case xlCylinderColStacked:
                case xlCylinderColStacked100:
                case xlCylinderBarClustered:
                case xlCylinderBarStacked:
                case xlCylinderBarStacked100:
                case xlCylinderCol:
                        mxDiagramPropertySet->setPropertyValue(SOLIDTYPE, uno::makeAny(chart::ChartSolidType::CYLINDER));
                        break;
                default:
                    if (mxDiagramPropertySet->getPropertySetInfo()->hasPropertyByName(SOLIDTYPE))
                    {
                            mxDiagramPropertySet->setPropertyValue(SOLIDTYPE, uno::makeAny(chart::ChartSolidType::RECTANGULAR_SOLID));
                    }
                    break;
        }

        switch ( _nChartType)
        {
            case xlConeCol:
            case xlConeColClustered:
            case xlConeColStacked:
            case xlConeColStacked100:
            case xlPyramidColClustered:
            case xlPyramidColStacked:
            case xlPyramidColStacked100:
            case xlCylinderColClustered:
            case xlCylinderColStacked:
            case xlCylinderColStacked100:
            case xlColumnClustered:
            case xlColumnStacked:
            case xlColumnStacked100:
            case xl3DColumnClustered:
            case xl3DColumnStacked:
            case xl3DColumnStacked100:
            case xlSurface: // not possible
            case xlSurfaceWireframe:
            case xlSurfaceTopView:
            case xlSurfaceTopViewWireframe:
                mxDiagramPropertySet->setPropertyValue(VERTICAL, uno::makeAny( sal_True));
                break;
            default:
                if (mxDiagramPropertySet->getPropertySetInfo()->hasPropertyByName(VERTICAL))
                {
                    mxDiagramPropertySet->setPropertyValue(VERTICAL, uno::makeAny(sal_False));
                }
                break;
        }

        switch (_nChartType)
        {
            case xlColumnStacked:
            case xl3DColumnStacked:
            case xlBarStacked:
            case xl3DBarStacked:
            case xlLineStacked:
            case xlLineMarkersStacked:
            case xlAreaStacked:
            case xl3DAreaStacked:
            case xlCylinderColStacked:
            case xlCylinderBarStacked:
            case xlConeColStacked:
            case xlConeBarStacked:
            case xlPyramidColStacked:
            case xlPyramidBarStacked:
                mxDiagramPropertySet->setPropertyValue(PERCENT, uno::makeAny( sal_False ));
                mxDiagramPropertySet->setPropertyValue(STACKED, uno::makeAny( sal_True ));
                break;
            case xlPyramidColStacked100:
            case xlPyramidBarStacked100:
            case xlConeColStacked100:
            case xlConeBarStacked100:
            case xlCylinderBarStacked100:
            case xlCylinderColStacked100:
            case xl3DAreaStacked100:
            case xlLineMarkersStacked100:
            case xlAreaStacked100:
            case xlLineStacked100:
            case xl3DBarStacked100:
            case xlBarStacked100:
            case xl3DColumnStacked100:
            case xlColumnStacked100:
                mxDiagramPropertySet->setPropertyValue(STACKED, uno::makeAny( sal_True));
                mxDiagramPropertySet->setPropertyValue(PERCENT, uno::makeAny( sal_True ));
                break;
            default:
                mxDiagramPropertySet->setPropertyValue(PERCENT, uno::makeAny( sal_False));
                mxDiagramPropertySet->setPropertyValue(STACKED, uno::makeAny( sal_False));
                break;
        }
        switch (_nChartType)
        {
            case xl3DArea:
            case xl3DAreaStacked:
            case xl3DAreaStacked100:
            case xl3DBarClustered:
            case xl3DBarStacked:
            case xl3DBarStacked100:
            case xl3DColumn:
            case xl3DColumnClustered:
            case xl3DColumnStacked:
            case xl3DColumnStacked100:
            case xl3DLine:
            case xl3DPie:
            case xl3DPieExploded:
            case xlConeColClustered:
            case xlConeColStacked:
            case xlConeColStacked100:
            case xlConeBarClustered:
            case xlConeBarStacked:
            case xlConeBarStacked100:
            case xlConeCol:
            case xlPyramidColClustered:
            case xlPyramidColStacked:
            case xlPyramidColStacked100:
            case xlPyramidBarClustered:
            case xlPyramidBarStacked:
            case xlPyramidBarStacked100:
            case xlPyramidCol:
            case xlCylinderColClustered:
            case xlCylinderColStacked:
            case xlCylinderColStacked100:
            case xlCylinderBarClustered:
            case xlCylinderBarStacked:
            case xlCylinderBarStacked100:
            case xlCylinderCol:
                mxDiagramPropertySet->setPropertyValue(DIM3D, uno::makeAny( sal_True));
                break;
            default:
                if (mxDiagramPropertySet->getPropertySetInfo()->hasPropertyByName(DIM3D))
                {
                    mxDiagramPropertySet->setPropertyValue(DIM3D, uno::makeAny( sal_False));
                }
                break;
        }
    }
    catch ( const uno::Exception& )
    {
        throw script::BasicErrorException( OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, OUString() );
    }
}

void SAL_CALL
ScVbaChart::Activate() throw (script::BasicErrorException, uno::RuntimeException)
{
    // #TODO how are Chart sheets handled ( I know we don't even consider
    // them in the worksheets/sheets collections ), but.....???
    // note: in vba for excel the parent of a Chart sheet is a workbook,
    // e.g. 'ThisWorkbook'
    uno::Reference< XHelperInterface > xParent( getParent() );
    ScVbaChartObject* pChartObj = static_cast< ScVbaChartObject* >( xParent.get() );
    if ( pChartObj )
        pChartObj->Activate();
    else
        throw script::BasicErrorException( OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, OUString( "no ChartObject as parent" ) );
}

void SAL_CALL
ScVbaChart::setSourceData( const css::uno::Reference< ::ooo::vba::excel::XRange >& _xCalcRange, const css::uno::Any& _aPlotBy ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    try
    {
        uno::Sequence< table::CellRangeAddress > mRangeAddresses(1);
        table::CellRangeAddress mSingleRangeAddress;

        uno::Reference< sheet::XCellRangeAddressable > xAddressable( _xCalcRange->getCellRange(), uno::UNO_QUERY_THROW );
        mSingleRangeAddress = xAddressable->getRangeAddress();

        mRangeAddresses[0] = mSingleRangeAddress;

        mxTableChart->setRanges(mRangeAddresses);

        sal_Bool bsetRowHeaders = false;
        sal_Bool bsetColumnHeaders = false;

        ScVbaRange* pRange = static_cast< ScVbaRange* >( _xCalcRange.get() );
        if ( pRange )
        {
            ScDocument* pDoc = pRange->getScDocument();
            if ( pDoc )
            {
                bsetRowHeaders = pDoc->HasRowHeader(  static_cast< SCCOL >( mSingleRangeAddress.StartColumn ), static_cast< SCROW >( mSingleRangeAddress.StartRow ), static_cast< SCCOL >( mSingleRangeAddress.EndColumn ), static_cast< SCROW >( mSingleRangeAddress.EndRow ), static_cast< SCTAB >( mSingleRangeAddress.Sheet ) );
                bsetColumnHeaders =  pDoc->HasColHeader(  static_cast< SCCOL >( mSingleRangeAddress.StartColumn ), static_cast< SCROW >( mSingleRangeAddress.StartRow ), static_cast< SCCOL >( mSingleRangeAddress.EndColumn ), static_cast< SCROW >( mSingleRangeAddress.EndRow ), static_cast< SCTAB >( mSingleRangeAddress.Sheet ));
;
            }
        }
        mxTableChart->setHasRowHeaders(bsetRowHeaders);
        mxTableChart->setHasColumnHeaders(bsetColumnHeaders);

        if ((!bsetColumnHeaders) || (!bsetRowHeaders))
        {
            uno::Reference< chart::XChartDataArray > xChartDataArray( mxChartDocument->getData(), uno::UNO_QUERY_THROW );
            if (!bsetColumnHeaders)
            {
                xChartDataArray->setColumnDescriptions( getDefaultSeriesDescriptions(xChartDataArray->getColumnDescriptions().getLength() ));
            }
            if (!bsetRowHeaders)
            {
                xChartDataArray->setRowDescriptions(getDefaultSeriesDescriptions(xChartDataArray->getRowDescriptions().getLength() ));
            }
        }

        if ( _aPlotBy.hasValue() )
        {
            sal_Int32 nVal = 0;
            _aPlotBy >>= nVal;
            setPlotBy( nVal );
        }
        else
        {
            sal_Int32 nRows =  mSingleRangeAddress.EndRow - mSingleRangeAddress.StartRow;
            sal_Int32 nCols = mSingleRangeAddress.EndColumn - mSingleRangeAddress.StartColumn;
            // AutoDetect emulation
            if ( nRows > nCols )
                setPlotBy( xlColumns );
            else if ( nRows <= nCols )
                setPlotBy( xlRows );
        }
    }
    catch (const uno::Exception&)
    {
        throw script::BasicErrorException( OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, OUString() );
    }
}

uno::Sequence< OUString >
ScVbaChart::getDefaultSeriesDescriptions( sal_Int32 _nCount )
{
    uno::Sequence< OUString > sDescriptions ( _nCount );
    sal_Int32 nLen = sDescriptions.getLength();
    for (sal_Int32 i = 0; i < nLen; i++)
    {
        sDescriptions[i] = DEFAULTSERIESPREFIX + OUString::number(i+1);
    }
    return sDescriptions;
}

void
ScVbaChart::setDefaultChartType() throw ( script::BasicErrorException )
{
    setChartType( xlColumnClustered );
}

void
ScVbaChart::setPlotBy( ::sal_Int32 _nPlotBy ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    try
    {
        if ( !mxDiagramPropertySet.is() )
            setDefaultChartType();
        switch (_nPlotBy)
        {
            case xlRows:
                mxDiagramPropertySet->setPropertyValue( DATAROWSOURCE,  uno::makeAny( chart::ChartDataRowSource_ROWS ) );
                break;
            case xlColumns:
                mxDiagramPropertySet->setPropertyValue( DATAROWSOURCE, uno::makeAny( chart::ChartDataRowSource_COLUMNS) );
                break;
            default:
                throw script::BasicErrorException( OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, OUString() );
        }
    }
    catch (const uno::Exception&)
    {
        throw script::BasicErrorException( OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, OUString() );
    }
}

::sal_Int32 SAL_CALL
ScVbaChart::getPlotBy(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        chart::ChartDataRowSource aChartDataRowSource;
        mxDiagramPropertySet->getPropertyValue(DATAROWSOURCE) >>= aChartDataRowSource;
        if (aChartDataRowSource == chart::ChartDataRowSource_COLUMNS)
        {
            return xlColumns;
        }
        else
        {
            return xlRows;
        }
    }
    catch (const uno::Exception&)
    {
        throw script::BasicErrorException( OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, OUString() );
    }
}

void
ScVbaChart::setDiagram( const OUString& _sDiagramType ) throw( script::BasicErrorException )
{
    try
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF( mxChartDocument, uno::UNO_QUERY_THROW );
        uno::Reference< chart::XDiagram > xDiagram( xMSF->createInstance( _sDiagramType ), uno::UNO_QUERY_THROW  );
        mxChartDocument->setDiagram( xDiagram );
        mxDiagramPropertySet.set( xDiagram, uno::UNO_QUERY_THROW );
    }
    catch ( const uno::Exception& )
    {
        throw script::BasicErrorException( OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, OUString() );
    }
}

// #TODO find out why we have Location/getLocation ? there is afaiks no
// Location property, just a Location function for the Chart object
sal_Int32 SAL_CALL
ScVbaChart::Location() throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    return getLocation();
}

sal_Int32 SAL_CALL
ScVbaChart::getLocation() throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
     return -1;
}

void SAL_CALL
ScVbaChart::setLocation( ::sal_Int32 /*where*/, const css::uno::Any& /*Name*/ ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // Helper api just stubs out the code <shrug>
    // #TODO come back and make sense out of this
//        String sheetName = null;
//
//        if ((name != null) && name instanceof String) {
//            sheetName = (String) name;
//        }
//        XSpreadsheetDocument xShDoc = (XSpreadsheetDocument) UnoRuntime.queryInterface( XSpreadsheetDocument.class,getXModel() );
//        com.sun.star.sheet.XSpreadsheets xSheets = xShDoc.Sheets();
//
//        switch (where) {
//        case ClLocationType.clLocationAsObject_value: //{
//
//            if (sheetName == null) {
//                DebugHelper.writeInfo("Can't embed in Chart without knowing SheetName");
//                return;
//            }
//
//            try {
//                Any any = (Any) xSheets.getByName(sheetName);
//                chartSheet = (XSpreadsheet) any.getObject();
//
//                // chartSheet = (XSpreadsheet) xSheets.getByName( sheetName );
//            } catch (NoSuchElementException e) {
//                // TODO Auto-generated catch block
//                e.printStackTrace();
//
//                return;
//            } catch (WrappedTargetException e) {
//                // TODO Auto-generated catch block
//                e.printStackTrace();
//
//                return;
//            } catch (java.lang.Exception e) {
//                e.printStackTrace();
//            }
//
//            XTableChartsSupplier xTCS = (XTableChartsSupplier) UnoRuntime.queryInterface( XTableChartsSupplier.class, chartSheet);
//            XTableCharts xTableCharts = xTCS.getCharts();
//            XIndexAccess xIA = (XIndexAccess) UnoRuntime.queryInterface( XIndexAccess.class, xTableCharts);
//            int numCharts = xIA.getCount();
//            chartName = "Chart " + (numCharts + 1);
//
//            //}
//            break;
//
//        case ClLocationType.clLocationAsNewSheet_value:
//        case ClLocationType.clLocationAutomatic_value:default: //{
//            chartName = "Chart 1"; // Since it's a new sheet, it's the first on it...
//
//            XIndexAccess xSheetIA = (XIndexAccess) UnoRuntime.queryInterface( XIndexAccess.class, xSheets);
//
//            short newSheetNum = (short) (xSheetIA.getCount() + 1);
//
//            if (sheetName == null){
//                sheetName = "ChartSheet " + newSheetNum; // Why not?
//            }
//            // DPK TODO : Probably should use Sheets to create this!
//            xSheets.insertNewByName(sheetName, newSheetNum);
//
//            try {
//                chartSheet =
//                    (XSpreadsheet) xSheets.getByName(sheetName);
//            } catch (NoSuchElementException e) {
//                // TODO Auto-generated catch block
//                e.printStackTrace();
//
//                return;
//            } catch (WrappedTargetException e) {
//                // TODO Auto-generated catch block
//                e.printStackTrace();
//
//                return;
//            }
//
//            //}
//            break;
//        }
//
//        // Last thing should be a call to createChartForReal(), one of them
//        // should succeed.
//        createChartForReal();

}

sal_Bool SAL_CALL
ScVbaChart::getHasTitle(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    sal_Bool bHasTitle = false;
    try
    {
        mxChartPropertySet->getPropertyValue(HASMAINTITLE) >>= bHasTitle;
    }
    catch (const uno::Exception&)
    {
        throw script::BasicErrorException( OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, OUString() );
    }
    return bHasTitle;
}

void SAL_CALL
ScVbaChart::setHasTitle( ::sal_Bool bTitle ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        mxChartPropertySet->setPropertyValue(HASMAINTITLE, uno::makeAny( bTitle ));
    }
    catch (const uno::Exception&)
    {
        throw script::BasicErrorException( OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, OUString() );
    }

}

::sal_Bool SAL_CALL
ScVbaChart::getHasLegend(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    sal_Bool bHasLegend = false;
    try
    {
        mxChartPropertySet->getPropertyValue(HASLEGEND) >>= bHasLegend;
    }
    catch (const uno::Exception&)
    {
        throw script::BasicErrorException( OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, OUString() );
    }
    return bHasLegend;
}

void SAL_CALL
ScVbaChart::setHasLegend( ::sal_Bool bLegend ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        mxChartPropertySet->setPropertyValue(HASLEGEND, uno::makeAny(bLegend));
    }
    catch (const uno::Exception&)
    {
        throw script::BasicErrorException( OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, OUString() );
    }
}

uno::Reference< excel::XChartTitle > SAL_CALL
ScVbaChart::getChartTitle(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< drawing::XShape > xTitleShape = mxChartDocument->getTitle();
    // #TODO check parent
    return new ScVbaChartTitle(this, mxContext, xTitleShape);
}

uno::Any SAL_CALL
ScVbaChart::Axes( const uno::Any& Type, const uno::Any& AxisGroup ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // mmm chart probably is the parent, #TODO check parent
    uno::Reference< excel::XAxes > xAxes = new ScVbaAxes( this, mxContext, this );
    if ( !Type.hasValue() )
        return uno::makeAny( xAxes );
    return xAxes->Item( Type, AxisGroup );
}
bool
ScVbaChart::is3D() throw ( uno::RuntimeException )
{
    // #TODO perhaps provide limited Debughelper functionality
    sal_Bool is3d = false;
    mxDiagramPropertySet->getPropertyValue(DIM3D) >>= is3d;
    return is3d;
}

sal_Int32
ScVbaChart::getStackedType( sal_Int32 _nStacked, sal_Int32 _n100PercentStacked, sal_Int32 _nUnStacked ) throw ( uno::RuntimeException )
{
    // #TODO perhaps provide limited Debughelper functionality
    if (isStacked())
    {
        if (is100PercentStacked())
            return _n100PercentStacked;
        else
            return _nStacked;
    }
    else
        return _nUnStacked;
}

bool
ScVbaChart::isStacked() throw ( uno::RuntimeException )
{
    // #TODO perhaps provide limited Debughelper functionality
    sal_Bool bStacked = false;
    mxDiagramPropertySet->getPropertyValue(STACKED) >>= bStacked;
    return bStacked;
}

bool
ScVbaChart::is100PercentStacked() throw ( uno::RuntimeException )
{
    // #TODO perhaps provide limited Debughelper functionality
    sal_Bool b100Percent = false;
    mxDiagramPropertySet->getPropertyValue(PERCENT) >>= b100Percent;
    return b100Percent;
}

sal_Int32
ScVbaChart::getSolidType(sal_Int32 _nDeep, sal_Int32 _nVertiStacked, sal_Int32 _nVerti100PercentStacked, sal_Int32 _nVertiUnStacked, sal_Int32 _nHoriStacked, sal_Int32 _nHori100PercentStacked, sal_Int32 _nHoriUnStacked) throw ( script::BasicErrorException )
{
    sal_Bool bIsVertical = true;
    try
    {
        mxDiagramPropertySet->getPropertyValue(VERTICAL) >>= bIsVertical;
        sal_Bool bIsDeep = false;
        mxDiagramPropertySet->getPropertyValue(DEEP) >>= bIsDeep;

        if (bIsDeep)
        {
            return _nDeep;
        }
        else
        {
            if (bIsVertical)
            {
                return getStackedType(_nVertiStacked, _nVerti100PercentStacked, _nVertiUnStacked);
            }
            else
            {
                return getStackedType(_nHoriStacked, _nHori100PercentStacked, _nHoriUnStacked);
            }
        }
    }
    catch (const uno::Exception&)
    {
        throw script::BasicErrorException( OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, OUString() );
    }
}


sal_Int32
ScVbaChart::getStockUpDownValue(sal_Int32 _nUpDown, sal_Int32 _nNotUpDown) throw (script::BasicErrorException)
{
    sal_Bool bUpDown = false;
    try
    {
        mxDiagramPropertySet->getPropertyValue(UPDOWN) >>= bUpDown;
        if (bUpDown)
        {
            return _nUpDown;
        }
        else
        {
            return _nNotUpDown;
        }
    }
    catch (const uno::Exception&)
    {
        OUString aTemp;    // temporary needed for g++ 3.3.5
        script::BasicErrorException( aTemp, uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, OUString() );
    }
    return _nNotUpDown;
}

bool
ScVbaChart::hasMarkers() throw ( script::BasicErrorException )
{
    bool bHasMarkers = false;
    try
    {
        sal_Int32 nSymbol=0;
        mxDiagramPropertySet->getPropertyValue(SYMBOLTYPE) >>= nSymbol;
        bHasMarkers = nSymbol != chart::ChartSymbolType::NONE;
    }
    catch (const uno::Exception&)
    {
        OUString aTemp;    // temporary needed for g++ 3.3.5
        script::BasicErrorException( aTemp, uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, OUString() );
    }
    return bHasMarkers;
}

sal_Int32
ScVbaChart::getMarkerType(sal_Int32 _nWithMarkers, sal_Int32 _nWithoutMarkers) throw ( script::BasicErrorException )
{
    if (hasMarkers())
        return _nWithMarkers;
    return _nWithoutMarkers;
}

void
ScVbaChart::assignDiagramAttributes()
{
    xAxisXSupplier.set( mxDiagramPropertySet, uno::UNO_QUERY_THROW );
    xAxisYSupplier.set( mxDiagramPropertySet, uno::UNO_QUERY_THROW );
    xAxisZSupplier.set( mxDiagramPropertySet, uno::UNO_QUERY_THROW );
    xTwoAxisXSupplier.set( mxDiagramPropertySet, uno::UNO_QUERY_THROW );
    xTwoAxisYSupplier.set( mxDiagramPropertySet, uno::UNO_QUERY_THROW );
}


uno::Reference< beans::XPropertySet >
ScVbaChart::getAxisPropertySet(sal_Int32 _nAxisType, sal_Int32 _nAxisGroup) throw ( script::BasicErrorException )
{
    assignDiagramAttributes();
    uno::Reference< beans::XPropertySet > xAxisProps;
    switch(_nAxisType)
    {
        case xlCategory:
            if (_nAxisGroup == xlPrimary)
            {
                xAxisProps = xAxisXSupplier->getXAxis();
            }
            else if (_nAxisGroup == xlSecondary)
            {
                xAxisProps = xTwoAxisXSupplier->getSecondaryXAxis();
            }
            break;
        case xlSeriesAxis:
            xAxisProps = xAxisZSupplier->getZAxis();
            break;
        case xlValue:
            if (_nAxisGroup == xlPrimary)
                xAxisProps = xAxisYSupplier->getYAxis();
            else if (_nAxisGroup == xlSecondary)
                xAxisProps = xTwoAxisYSupplier->getSecondaryYAxis();
            break;
        default:
            return xAxisProps;
        }
    return xAxisProps;
}


OUString
ScVbaChart::getServiceImplName()
{
    return OUString("ScVbaChart");
}

uno::Sequence< OUString >
ScVbaChart::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = OUString("ooo.vba.excel.Chart" );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
