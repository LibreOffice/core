/*************************************************************************
 *
 *  $RCSfile: VDiagram.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:33 $
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
#ifndef _CHART2_VDIAGRAM_HXX
#define _CHART2_VDIAGRAM_HXX

#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX_HPP_
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <drafts/com/sun/star/chart2/XDiagram.hpp>
#endif

#ifndef _CHART2_VIEW_SHAPEFACTORY_HXX
#include "ShapeFactory.hxx"
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
                  ::drafts::com::sun::star::chart2::XDiagram > & xDiagram,
              sal_Int32 nDimension=3, sal_Bool bPolar=sal_False);
    virtual ~VDiagram();

    void SAL_CALL init( const ::com::sun::star::uno::Reference<
                     ::com::sun::star::drawing::XShapes >& xLogicTarget
             , const ::com::sun::star::uno::Reference<
                     ::com::sun::star::drawing::XShapes >& xFinalTarget
             , const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory);

    void    setSceneMatrix( const ::com::sun::star::drawing::HomogenMatrix& rMatrix );

    void    createShapes( const ::com::sun::star::awt::Point& rPos
                        , const ::com::sun::star::awt::Size& rSize );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
            getCoordinateRegion();

    //    updateShapes(..);
    // const awt::Point& rPos, const awt::Size& rSize );

private: //methods
    void    createShapes_2d( const ::com::sun::star::awt::Point& rPos
                        , const ::com::sun::star::awt::Size& rSize );
    void    createShapes_3d( const ::com::sun::star::awt::Point& rPos
                        , const ::com::sun::star::awt::Size& rSize );

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

    ::com::sun::star::drawing::HomogenMatrix                                m_oSceneMatrix;
    sal_Int32                                                               m_nDimension;
    sal_Bool                                                                m_bPolar;
    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XDiagram >                        m_xDiagram;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif


