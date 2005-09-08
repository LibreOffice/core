/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VSeriesPlotter.hxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:46:06 $
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
#ifndef _CHART2_VSERIESPLOTTER_HXX
#define _CHART2_VSERIESPLOTTER_HXX

#include "PlotterBase.hxx"
#include "VDataSeries.hxx"
#include "PropertyMapper.hxx"
#include "LabelAlignment.hxx"

#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPE_HPP_
#include <com/sun/star/chart2/XChartType.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class MinimumAndMaximumSupplier
{
public:
    virtual double getMinimumX() = 0;
    virtual double getMaximumX() = 0;

    //problem y maybe not is always the second border to ask for
    virtual double getMinimumYInRange( double fMinimumX, double fMaximumX ) = 0;
    virtual double getMaximumYInRange( double fMinimumX, double fMaximumX ) = 0;

    //problem: z maybe not independent in future
    virtual double getMinimumZ() = 0;
    virtual double getMaximumZ() = 0;
};

//enum StackType { STACK_NORMAL, STACK_NONE, STACK_BESIDES, STACK_ONTOP, STACK_BEHIND };

class VDataSeriesGroup
{
    //a list of series that have the same BoundedCoordinateSystem
    //they are used to be plotted maybe in a stacked manner by a plotter

public:
    VDataSeriesGroup();
    VDataSeriesGroup( VDataSeries* pSeries );
    VDataSeriesGroup( const ::std::vector< VDataSeries* >& rSeriesVector );
    virtual ~VDataSeriesGroup();

    void addSeries( VDataSeries* pSeries );//takes ownership of pSeries
    sal_Int32 getSeriesCount() const;
    void deleteSeries();

    void getMinimumAndMaximiumX( double& rfMinimum, double& rfMaximum ) const;
    void getMinimumAndMaximiumYInContinuousXRange( double& rfMinY, double& rfMaxY, double fMinX, double fMaxX ) const;

    void calculateYSumsForCategory( sal_Int32 nCategoryIndex, double& rfPositiveSum, double& rfNegativeSum );
    bool calculateYMinAndMaxForCategoryRange( sal_Int32 nStartCategoryIndex, sal_Int32 nEndCategoryIndex
                                                , double& rfMinimum, double& rfMaximum );

    void setSums( double fPositiveSum, double fNegativeSum );
    bool getSums( double& rfPositiveSum, double& rfNegativeSum ) const;

    ::std::vector< VDataSeries* >   m_aSeriesVector;

private:
    bool        m_bSumValuesDirty;
    double      m_fPositiveSum;
    double      m_fNegativeSum;

    bool        m_bMaxPointCountDirty;
    sal_Int32   m_nMaxPointCount;
};

class VSeriesPlotter : public PlotterBase, public MinimumAndMaximumSupplier
{
    //-------------------------------------------------------------------------
    // public methods
    //-------------------------------------------------------------------------
public:
    virtual ~VSeriesPlotter();

    /*
    * A new series can be positioned relative to other series in a chart.
    * This positioning has two dimensions. First a series can be placed
    * next to each other on the category axis. This position is indicated by xSlot.
    * Second a series can be stacked on top of another. This position is indicated by ySlot.
    * The positions are counted from 0 on.
    * xSlot < 0                     : append the series to already existing x series
    * xSlot > occupied              : append the series to already existing x series
    *
    * If the xSlot is already occupied the given ySlot decides what should happen:
    * ySlot < -1                    : move all existing series in the xSlot to next slot
    * ySlot == -1                   : stack on top at given x position
    * ySlot == already occupied     : insert at given y and x position
    * ySlot > occupied              : stack on top at given x position
    */
    virtual void addSeries( VDataSeries* pSeries, sal_Int32 xSlot = -1,sal_Int32 ySlot = -1 );

    //-------------------------------------------------------------------------
    // MinimumAndMaximumSupplier
    //-------------------------------------------------------------------------

    virtual double getMinimumX();
    virtual double getMaximumX();

    virtual double getMinimumYInRange( double fMinimumX, double fMaximumX );
    virtual double getMaximumYInRange( double fMinimumX, double fMaximumX );

    virtual double getMinimumZ();
    virtual double getMaximumZ();

    //------

    void getMinimumAndMaximiumX( double& rfMinimum, double& rfMaximum ) const;
    void getMinimumAndMaximiumYInContinuousXRange( double& rfMinY, double& rfMaxY, double fMinX, double fMaxX ) const;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------

    static sal_Int32    getPointCount( const VDataSeriesGroup& rSeriesGroup );
    static sal_Int32    getPointCount( const ::std::vector< VDataSeriesGroup >& rSlots );

    static VSeriesPlotter* createSeriesPlotter( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::chart2::XChartType >& xChartTypeModel );

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
private: //methods
    //no default constructor
    VSeriesPlotter();

protected: //methods

    VSeriesPlotter( const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartType >& xChartTypeModel
                , bool bCategoryXAxis=true );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
        getSeriesGroupShape( VDataSeries* pDataSeries
            , const::com::sun::star:: uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget );

    //the following group shapes will be created as children of SeriesGroupShape on demand
    //they can be used to assure that some parts of a series shape are always in front of others (e.g. symbols in front of lines)
    //parameter xTarget will be used as parent for the series group shape
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
        getSeriesGroupShapeFrontChild( VDataSeries* pDataSeries
            , const::com::sun::star:: uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget );
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
        getSeriesGroupShapeBackChild( VDataSeries* pDataSeries
            , const::com::sun::star:: uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
        getLabelsGroupShape( VDataSeries& rDataSeries
            , const::com::sun::star:: uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
        getErrorBarsGroupShape( VDataSeries& rDataSeries
            , const::com::sun::star:: uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget );

    void createDataLabel( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes >& xTarget
                , VDataSeries& rDataSeries
                , sal_Int32 nPointIndex
                , double fValue
                , double fSumValue
                , const ::com::sun::star::awt::Point& rScreenPosition2D
                , LabelAlignment eAlignment=LABEL_ALIGN_CENTER );

    /** creates two T-shaped error bars in both directions (up/down or
        left/right depending on the bVertical parameter)

        @param rPos
            logic coordinates

        @param xErrorBarProperties
            the XPropertySet returned by the DataPoint-property "ErrorBarX" or
            "ErrorBarY".

        @param nIndex
            the index of the data point in rData for which the calculation is
            done.

        @param bVertical
            for y-error bars this is true, for x-error-bars it is false.
     */
    virtual void createErrorBar(
          const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget
        , const ::com::sun::star::drawing::Position3D & rPos
        , const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & xErrorBarProperties
        , const ::com::sun::star::uno::Sequence< double > & rData
        , sal_Int32 nIndex
        , bool bVertical
        );

    virtual void createErrorBar_Y( const ::com::sun::star::drawing::Position3D& rUnscaledLogicPosition
        , VDataSeries& rVDataSeries, sal_Int32 nPointIndex
        , const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget );

    virtual void createRegressionCurvesShapes( const VDataSeries& rVDataSeries
        , const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget );

    virtual void setMappedProperties(
          const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShape >& xTarget
        , const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet >& xSource
        , const tPropertyNameMap& rMap );

protected: //member
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartType >    m_xChartTypeModel;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >           m_xChartTypeModelProps;

    ::std::vector< VDataSeriesGroup >   m_aXSlots;
    static tMakePropertyNameMap         m_aShapePropertyMapForArea;

    bool                                m_bCategoryXAxis;//true->xvalues are indices (this would not be necessary if series for category chart wouldn't have x-values)
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
