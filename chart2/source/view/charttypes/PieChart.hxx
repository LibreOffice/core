/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PieChart.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:16:19 $
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

#ifndef _CHART2_PIECHART_HXX
#define _CHART2_PIECHART_HXX

#include "VSeriesPlotter.hxx"

//.............................................................................
namespace chart
{
//.............................................................................
class PiePositionHelper;

class PieChart : public VSeriesPlotter
{
    //-------------------------------------------------------------------------
    // public methods
    //-------------------------------------------------------------------------
public:
    PieChart( const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartType >& xChartTypeModel
            , sal_Int32 nDimensionCount );
    virtual ~PieChart();

    //-------------------------------------------------------------------------
    // chart2::XPlotter
    //-------------------------------------------------------------------------

    virtual void SAL_CALL createShapes();

    virtual void SAL_CALL setScales(
          const ::com::sun::star::uno::Sequence<
            ::com::sun::star::chart2::ExplicitScaleData >& rScales
            , sal_Bool bSwapXAndYAxis )
                throw (::com::sun::star::uno::RuntimeException);
    /*
    virtual ::rtl::OUString SAL_CALL getCoordinateSystemTypeID(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setScales( const ::com::sun::star::uno::Sequence< ::com::sun::star::chart2::ExplicitScaleData >& rScales ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTransformation( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTransformation >& xTransformationToLogicTarget, const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTransformation >& xTransformationToFinalPage ) throw (::com::sun::star::uno::RuntimeException);
    */

    virtual void addSeries( VDataSeries* pSeries, sal_Int32 zSlot = -1, sal_Int32 xSlot = -1,sal_Int32 ySlot = -1 );

    //-------------------
    virtual ::com::sun::star::drawing::Direction3D  getPreferredDiagramAspectRatio() const;
    virtual bool keepAspectRatio() const;

    //MinimumAndMaximumSupplier
    virtual double getMinimumX();
    virtual double getMaximumX();
    virtual double getMinimumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex );
    virtual double getMaximumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex );

    virtual bool isExpandBorderToIncrementRhythm( sal_Int32 nDimensionIndex );
    virtual bool isExpandIfValuesCloseToBorder( sal_Int32 nDimensionIndex );
    virtual bool isExpandWideValuesToZero( sal_Int32 nDimensionIndex );
    virtual bool isExpandNarrowValuesTowardZero( sal_Int32 nDimensionIndex );
    virtual bool isSeperateStackingForDifferentSigns( sal_Int32 nDimensionIndex );

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
private: //methods
    //no default constructor
    PieChart();

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createDataPoint(  const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::uno::Reference<
                                ::com::sun::star::beans::XPropertySet >& xObjectProperties
                        , double fUnitCircleStartAngleDegree, double fWidthAngleDegree
                        , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                        , double fLogicZ, double fDepth, double fExplodePercentage
                        , tPropertyNameValueMap* pOverWritePropertiesMap );

    bool                isSingleRingChart() const;
    double              getMaxOffset() const;

private: //member
    PiePositionHelper*    m_pPosHelper;
    bool                  m_bUseRings;
};
//.............................................................................
} //namespace chart
//.............................................................................
#endif
