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
#ifndef _CHART2_VDIAGRAM_HXX
#define _CHART2_VDIAGRAM_HXX

#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include "ShapeFactory.hxx"
#include <basegfx/range/b2irectangle.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

class ShapeFactory;

//-----------------------------------------------------------------------------
/** The VDiagram is responsible to generate the visible parts of the Diagram
that is wall, floor, axes and data series.
The axes and data series are subobjects which are created and managed by the
diagram.
*/

class VDiagram
{
public: //methods
    VDiagram( const ::com::sun::star::uno::Reference<
                  ::com::sun::star::chart2::XDiagram > & xDiagram
                , const ::com::sun::star::drawing::Direction3D& rPreferredAspectRatio
                , sal_Int32 nDimension=3, sal_Bool bPolar=sal_False );
    virtual ~VDiagram();

    void init( const ::com::sun::star::uno::Reference<
                     ::com::sun::star::drawing::XShapes >& xLogicTarget
             , const ::com::sun::star::uno::Reference<
                     ::com::sun::star::drawing::XShapes >& xFinalTarget
             , const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory);

    void    createShapes( const ::com::sun::star::awt::Point& rPos
                        , const ::com::sun::star::awt::Size& rSize );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
            getCoordinateRegion();

    ::basegfx::B2IRectangle    getCurrentRectangle();

    void    reduceToMimimumSize();

    ::basegfx::B2IRectangle    adjustPosAndSize( const ::com::sun::star::awt::Point& rPos
                                    , const ::com::sun::star::awt::Size& rAvailableSize );

    ::basegfx::B2IRectangle    adjustInnerSize( const ::basegfx::B2IRectangle& rConsumedOuterRect );

    //    updateShapes(..);
    // const awt::Point& rPos, const awt::Size& rSize );

private: //methods
    void    createShapes_2d();
    void    createShapes_3d();

    ::basegfx::B2IRectangle    adjustPosAndSize_2d( const ::com::sun::star::awt::Point& rPos
                        , const ::com::sun::star::awt::Size& rAvailableSize );
    ::basegfx::B2IRectangle    adjustPosAndSize_3d( const ::com::sun::star::awt::Point& rPos
                        , const ::com::sun::star::awt::Size& rAvailableSize );

    void    adjustAspectRatio3d( const ::com::sun::star::awt::Size& rAvailableSize );

private: //members
    VDiagram(const VDiagram& rD);

    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes >                    m_xLogicTarget;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes >                    m_xFinalTarget;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory>           m_xShapeFactory;
    ShapeFactory*                                                           m_pShapeFactory;

    // this is the surrounding shape which contains floor, wall and coordinate
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   m_xOuterGroupShape;
    // this is an additional inner shape that represents the coordinate region -  that is - where to place data points
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   m_xCoordinateRegionShape;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   m_xWall2D;

    sal_Int32                                                               m_nDimensionCount;
    sal_Bool                                                                m_bPolar;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDiagram >                                m_xDiagram;

    ::com::sun::star::drawing::Direction3D                                  m_aPreferredAspectRatio;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > m_xAspectRatio3D;

    double m_fXAnglePi;
    double m_fYAnglePi;
    double m_fZAnglePi;
    sal_Bool m_bRightAngledAxes;

    ::com::sun::star::awt::Point    m_aAvailablePosIncludingAxes;
    ::com::sun::star::awt::Size     m_aAvailableSizeIncludingAxes;

    ::com::sun::star::awt::Point    m_aCurrentPosWithoutAxes;
    ::com::sun::star::awt::Size     m_aCurrentSizeWithoutAxes;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif


