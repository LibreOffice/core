/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VCartesianAxis.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:38:14 $
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

#include "VMeterBase.hxx"
#include "VAxisProperties.hxx"
#include "TickmarkHelper.hxx"

// header for class Vector2D
#ifndef _VECTOR2D_HXX
#include <tools/vector2d.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class NumberFormatterWrapper;

class VCartesianAxis : public VMeterBase
{
    //-------------------------------------------------------------------------
    // public methods
    //-------------------------------------------------------------------------
public:
    VCartesianAxis( const AxisProperties& rAxisProperties
           , NumberFormatterWrapper* pNumberFormatterWrapper
           , sal_Int32 nDimensionCount
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

    virtual void SAL_CALL createShapes();

    //-------------------------------------------------------------------------
    double      getLogicValueWhereMainLineCrossesOtherAxis() const;
    bool        getLogicValueWhereExtraLineCrossesOtherAxis( double& fCrossesOtherAxis) const;
    void        get2DAxisMainLine( Vector2D& rStart, Vector2D& rEnd, double fCrossesOtherAxis ) const;

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
                     , ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos
                     , AxisLabelProperties& rAxisLabelProperties
                     , TickmarkHelper_2D* pTickmarkHelper );

private: //member
    AxisProperties              m_aAxisProperties;
    NumberFormatterWrapper*     m_pNumberFormatterWrapper;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
