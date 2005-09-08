/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LabelPositionHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:43:09 $
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

#ifndef _CHART2_VIEW_LABELPOSITIONHELPER_HXX
#define _CHART2_VIEW_LABELPOSITIONHELPER_HXX

#include "LabelAlignment.hxx"
#include "PropertyMapper.hxx"

#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POSITION3D_HPP_
#include <com/sun/star/drawing/Position3D.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class PlottingPositionHelper;
class ShapeFactory;

class LabelPositionHelper
{
public:
    LabelPositionHelper(
        PlottingPositionHelper* pPosHelper
        , sal_Int32 nDimensionCount
        , const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xLogicTarget
        , ShapeFactory* pShapeFactory );
    virtual ~LabelPositionHelper();

    ::com::sun::star::awt::Point transformSceneToScreenPosition(
            const ::com::sun::star::drawing::Position3D& rScenePosition3D ) const;
    ::com::sun::star::awt::Point transformLogicToScreenPosition(
            const ::com::sun::star::drawing::Position3D& rScenePosition3D ) const;

    static void changeTextAdjustment( tAnySequence& rPropValues, const tNameSequence& rPropNames, LabelAlignment eAlignment);
    static void doDynamicFontResize(  tAnySequence& rPropValues, const tNameSequence& rPropNames
                    , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xAxisModelProps
                    , const ::com::sun::star::awt::Size& rNewReferenceSize );

private:
    LabelPositionHelper();

protected:
    PlottingPositionHelper*  m_pPosHelper;
    sal_Int32                m_nDimensionCount;

private:
    //these members are only necessary for transformation from 3D to 2D
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes >    m_xLogicTarget;
    ShapeFactory*                                           m_pShapeFactory;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
