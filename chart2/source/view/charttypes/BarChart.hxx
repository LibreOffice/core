/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BarChart.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 12:10:02 $
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

#ifndef _CHART2_BARCHART_HXX
#define _CHART2_BARCHART_HXX

#include "VSeriesPlotter.hxx"

//.............................................................................
namespace chart
{
//.............................................................................
class BarPositionHelper;

class BarChart : public VSeriesPlotter
{
    //-------------------------------------------------------------------------
    // public methods
    //-------------------------------------------------------------------------
public:
    BarChart( const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartType >& xChartTypeModel
            , sal_Int32 nDimensionCount );
    virtual ~BarChart();

    //-------------------------------------------------------------------------
    // chart2::XPlotter
    //-------------------------------------------------------------------------

    virtual void SAL_CALL createShapes();
    /*
    virtual ::rtl::OUString SAL_CALL getCoordinateSystemTypeID(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setScales( const ::com::sun::star::uno::Sequence< ::com::sun::star::chart2::ExplicitScaleData >& rScales ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTransformation( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTransformation >& xTransformationToLogicTarget, const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTransformation >& xTransformationToFinalPage ) throw (::com::sun::star::uno::RuntimeException);
    */

    virtual void addSeries( VDataSeries* pSeries, sal_Int32 zSlot = -1, sal_Int32 xSlot = -1,sal_Int32 ySlot = -1 );

    //-------------------
    virtual ::com::sun::star::drawing::Direction3D  getPreferredDiagramAspectRatio() const;
    virtual bool keepAspectRatio() const;

    //-------------------------------------------------------------------------
    // MinimumAndMaximumSupplier
    //-------------------------------------------------------------------------
    virtual double getMinimumX();
    virtual double getMaximumX();

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------

private: //methods
    //no default constructor
    BarChart();

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createDataPoint3D_Bar(
                          const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::drawing::Position3D& rPosition
                        , const ::com::sun::star::drawing::Direction3D& rSize
                        , double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree
                        , const ::com::sun::star::uno::Reference<
                                ::com::sun::star::beans::XPropertySet >& xObjectProperties
                        , sal_Int32 nGeometry3D );

    ::com::sun::star::awt::Point getLabelScreenPositionAndAlignment(
                        LabelAlignment& rAlignment, sal_Int32 nLabelPlacement
                        , double fScaledX, double fScaledLowerYValue, double fScaledUpperYValue, double fScaledZ
                        , double fScaledLowerBarDepth, double fScaledUpperBarDepth, double fBaseValue
                        , BarPositionHelper* pPosHelper ) const;

    virtual PlottingPositionHelper& getPlottingPositionHelper( sal_Int32 nAxisIndex ) const;//nAxisIndex indicates wether the position belongs to the main axis ( nAxisIndex==0 ) or secondary axis ( nAxisIndex==1 )

    void adaptOverlapAndGapwidthForGroupBarsPerAxis();

private: //member
    BarPositionHelper*                   m_pMainPosHelper;
    ::com::sun::star::uno::Sequence< sal_Int32 > m_aOverlapSequence;
    ::com::sun::star::uno::Sequence< sal_Int32 > m_aGapwidthSequence;
};
//.............................................................................
} //namespace chart
//.............................................................................
#endif
