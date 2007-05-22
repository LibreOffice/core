/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VCartesianAxis.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:10:47 $
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
#ifndef _CHART2_VCARTESIANAXIS_HXX
#define _CHART2_VCARTESIANAXIS_HXX

#include "VAxisBase.hxx"

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class VCartesianAxis : public VAxisBase
{
    //-------------------------------------------------------------------------
    // public methods
    //-------------------------------------------------------------------------
public:
    VCartesianAxis( const AxisProperties& rAxisProperties
           , const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier
           , sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount
           , PlottingPositionHelper* pPosHelper = NULL //takes ownership
           );

    virtual ~VCartesianAxis();

    //-------------------------------------------------------------------------
    // partly chart2::XPlotter
    //-------------------------------------------------------------------------

    /*
    virtual ::rtl::OUString SAL_CALL getCoordinateSystemTypeID(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTransformation( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTransformation >& xTransformationToLogicTarget, const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTransformation >& xTransformationToFinalPage ) throw (::com::sun::star::uno::RuntimeException);
    */

    virtual void SAL_CALL createMaximumLabels();
    virtual void SAL_CALL createLabels();
    virtual void SAL_CALL updatePositions();

    virtual void SAL_CALL createShapes();

    virtual sal_Int32 estimateMaximumAutoMainIncrementCount();

    //-------------------------------------------------------------------------
    virtual TickmarkHelper* createTickmarkHelper();

    //-------------------------------------------------------------------------
    double      getLogicValueWhereMainLineCrossesOtherAxis() const;
    bool        getLogicValueWhereExtraLineCrossesOtherAxis( double& fCrossesOtherAxis) const;
    void        get2DAxisMainLine( ::basegfx::B2DVector& rStart, ::basegfx::B2DVector& rEnd, double fCrossesOtherAxis ) const;

    //-------------------------------------------------------------------------
    //Layout interface for cartesian axes:

    //the returned value describes the minimum size that is necessary
    //for the text labels in the direction orthogonal to the axis
    //(for an y-axis a width is returned; in case of an x-axis the value describes a height)
    //the return value is measured in screen dimension
    //As an example the MinimumOrthogonalSize of an x-axis equals the
    //Font Height if the label properties allow for labels parallel to the axis.
//    sal_Int32 calculateMinimumOrthogonalSize( /*... parallel...*/ );
    //Minimum->Preferred

    //returns true if the MinimumOrthogonalSize can be calculated
    //with the creation of at most one text shape
    //(this is e.g. true if the parameters allow for labels parallel to the axis.)
//    sal_bool  canQuicklyCalculateMinimumOrthogonalSize();


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------

protected: //methods
    bool    createTextShapes( const ::com::sun::star::uno::Reference<
                       ::com::sun::star::drawing::XShapes >& xTarget
                     , TickIter& rTickIter
                     , AxisLabelProperties& rAxisLabelProperties
                     , TickmarkHelper_2D* pTickmarkHelper );

    TickmarkHelper_2D* createTickmarkHelper2D();

    void    doStaggeringOfLabels( const AxisLabelProperties& rAxisLabelProperties
                            , TickmarkHelper_2D* pTickmarkHelper2D );
    bool    isAutoStaggeringOfLabelsAllowed( const AxisLabelProperties& rAxisLabelProperties
                            , TickmarkHelper_2D* pTickmarkHelper);
    bool    isBreakOfLabelsAllowed( const AxisLabelProperties& rAxisLabelProperties
                                                     , TickmarkHelper_2D* pTickmarkHelper );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
