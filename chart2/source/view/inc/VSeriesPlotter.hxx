/*************************************************************************
 *
 *  $RCSfile: VSeriesPlotter.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: iha $ $Date: 2003-11-15 08:54:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CHART2_VSERIESPLOTTER_HXX
#define _CHART2_VSERIESPLOTTER_HXX

#include "PlotterBase.hxx"
#include "VDataSeries.hxx"
#include "PropertyMapper.hxx"

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
    VSeriesPlotter( sal_Int32 nDimension );
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

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------

    static sal_Int32    getPointCount( const VDataSeriesGroup& rSeriesGroup );
    static sal_Int32    getPointCount( const ::std::vector< VDataSeriesGroup >& rSlots );

    static VSeriesPlotter* createSeriesPlotter( const rtl::OUString& rChartType, sal_Int32 nDimension );

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
private: //methods
    //no default constructor
    VSeriesPlotter();

protected: //methods
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
        getSeriesGroupShape( VDataSeries* pDataSeries
            , const::com::sun::star:: uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
        getLabelsGroupShape( VDataSeries* pDataSeries
            , const::com::sun::star:: uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget );

    void createDataLabel( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes >& xTarget
                , const VDataSeries& rDataSeries
                , sal_Int32 nPointIndex
                , double fValue
                , double fSumValue
                , const ::com::sun::star::awt::Point& rScreenPosition2D );

    virtual void setMappedProperties(
          const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShape >& xTarget
        , const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet >& xSource
        , const tPropertyNameMap& rMap );

protected: //member
    ::std::vector< VDataSeriesGroup >   m_aXSlots;
    static tMakePropertyNameMap         m_aShapePropertyMapForArea;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
