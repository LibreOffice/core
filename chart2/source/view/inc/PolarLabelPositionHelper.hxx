/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PolarLabelPositionHelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 12:11:10 $
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

#ifndef _CHART2_VIEW_POLARLABELPOSITIONHELPER_HXX
#define _CHART2_VIEW_POLARLABELPOSITIONHELPER_HXX

#include "LabelPositionHelper.hxx"

#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POSITION3D_HPP_
#include <com/sun/star/drawing/Position3D.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class PolarPlottingPositionHelper;

class PolarLabelPositionHelper : public LabelPositionHelper
{
public:
    PolarLabelPositionHelper(
        PolarPlottingPositionHelper* pPosHelper
        , sal_Int32 nDimensionCount
        , const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xLogicTarget
        , ShapeFactory* pShapeFactory );
    virtual ~PolarLabelPositionHelper();

    ::com::sun::star::awt::Point getLabelScreenPositionAndAlignmentForLogicValues(
                        LabelAlignment& rAlignment
                        , double fLogicValueOnAngleAxis
                        , double fLogicValueOnRadiusAxis
                        , double fLogicZ
                        , sal_Int32 nScreenValueOffsetInRadiusDirection=0 ) const;

    ::com::sun::star::awt::Point getLabelScreenPositionAndAlignmentForUnitCircleValues(
                        LabelAlignment& rAlignment, sal_Int32 nLabelPlacement /*see ::com::sun::star::chart::DataLabelPlacement*/
                        , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                        , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                        , double fLogicZ
                        , sal_Int32 nScreenValueOffsetInRadiusDirection=0 ) const;

private:
    PolarPlottingPositionHelper*    m_pPosHelper;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
