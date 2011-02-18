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

#ifndef _CHART2_AREACHART_HXX
#define _CHART2_AREACHART_HXX

#include "VSeriesPlotter.hxx"
#include <com/sun/star/chart2/CurveStyle.hpp>

//.............................................................................
namespace chart
{
//.............................................................................
class AreaPositionHelper;

class AreaChart : public VSeriesPlotter
{
    //-------------------------------------------------------------------------
    // public methods
    //-------------------------------------------------------------------------
public:
    AreaChart( const ::com::sun::star::uno::Reference<
             ::com::sun::star::chart2::XChartType >& xChartTypeModel
             , sal_Int32 nDimensionCount
             , bool bCategoryXAxis, bool bNoArea=false
             , PlottingPositionHelper* pPlottingPositionHelper=NULL //takes owner ship
             , bool bConnectLastToFirstPoint=false
             , bool bExpandIfValuesCloseToBorder=true
             , sal_Int32 nKeepAspectRatio=-1 //0->no 1->yes other value->automatic
             , const ::com::sun::star::drawing::Direction3D& rAspectRatio=::com::sun::star::drawing::Direction3D(1,1,1)//only taken into account if nKeepAspectRatio==1
             );
    virtual ~AreaChart();

    virtual void createShapes();
    virtual void addSeries( VDataSeries* pSeries, sal_Int32 zSlot = -1, sal_Int32 xSlot = -1,sal_Int32 ySlot = -1 );

    //-------------------
    virtual ::com::sun::star::drawing::Direction3D  getPreferredDiagramAspectRatio() const;
    virtual bool keepAspectRatio() const;

    //-------------------------------------------------------------------------
    // MinimumAndMaximumSupplier
    //-------------------------------------------------------------------------
    virtual double getMaximumX();
    virtual bool isExpandIfValuesCloseToBorder( sal_Int32 nDimensionIndex );
    virtual bool isSeperateStackingForDifferentSigns( sal_Int32 nDimensionIndex );

    //-------------------------------------------------------------------------

    virtual ::com::sun::star::chart2::LegendSymbolStyle getLegendSymbolStyle();
    virtual ::com::sun::star::uno::Any getExplicitSymbol( const VDataSeries& rSeries, sal_Int32 nPointIndex=-1/*-1 for series symbol*/ );

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
private: //methods
    //no default constructor
    AreaChart();

    void impl_createSeriesShapes();
    bool impl_createArea( VDataSeries* pSeries
                , ::com::sun::star::drawing::PolyPolygonShape3D* pSeriesPoly
                , ::com::sun::star::drawing::PolyPolygonShape3D* pPreviousSeriesPoly
                , PlottingPositionHelper* pPosHelper );
    bool impl_createLine( VDataSeries* pSeries
                , ::com::sun::star::drawing::PolyPolygonShape3D* pSeriesPoly
                , PlottingPositionHelper* pPosHelper );

private: //member
    PlottingPositionHelper*             m_pMainPosHelper;

    bool                                m_bArea;//false -> line or symbol only
    bool                                m_bLine;
    bool                                m_bSymbol;
    bool                                m_bIsPolarCooSys;//used e.g. for net chart (the data labels need to be placed different)
    bool                                m_bConnectLastToFirstPoint;//used e.g. for net chart
    bool                                m_bExpandIfValuesCloseToBorder; // e.g. false for net charts

    sal_Int32                           m_nKeepAspectRatio; //0->no 1->yes other value->automatic
    ::com::sun::star::drawing::Direction3D m_aGivenAspectRatio; //only used if nKeepAspectRatio==1

    //Properties for splines:
    ::com::sun::star::chart2::CurveStyle    m_eCurveStyle;
    sal_Int32                                       m_nCurveResolution;
    sal_Int32                                       m_nSplineOrder;

    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes > m_xSeriesTarget;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes > m_xErrorBarTarget;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes > m_xTextTarget;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes > m_xRegressionCurveEquationTarget;
};
//.............................................................................
} //namespace chart
//.............................................................................
#endif
