/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VDiagram.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:21:22 $
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
#ifndef _CHART2_VDIAGRAM_HXX
#define _CHART2_VDIAGRAM_HXX

#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX_HPP_
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <com/sun/star/chart2/XDiagram.hpp>
#endif

#ifndef _CHART2_VIEW_SHAPEFACTORY_HXX
#include "ShapeFactory.hxx"
#endif

#ifndef _BGFX_RANGE_B2IRECTANGLE_HXX
#include <basegfx/range/b2irectangle.hxx>
#endif

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

    void SAL_CALL init( const ::com::sun::star::uno::Reference<
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

    ::com::sun::star::awt::Point    m_aAvailablePosIncludingAxes;
    ::com::sun::star::awt::Size     m_aAvailableSizeIncludingAxes;

    ::com::sun::star::awt::Point    m_aCurrentPosWithoutAxes;
    ::com::sun::star::awt::Size     m_aCurrentSizeWithoutAxes;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif


