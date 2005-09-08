/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Stripe.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:50:36 $
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
#include "Stripe.hxx"

#ifndef __CHART_COMMON_CONVERTERS_HXX
#include "CommonConverters.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONSHAPE3D_HPP_
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_DOUBLESEQUENCE_HPP_
#include <com/sun/star/drawing/DoubleSequence.hpp>
#endif

#ifndef _POLY3D_HXX
#include <svx/poly3d.hxx>
#endif

using namespace ::com::sun::star;

//.............................................................................
namespace chart
{
//.............................................................................

Stripe::Stripe( const drawing::Position3D& rPoint1
        , const drawing::Direction3D& rDirectionToPoint2
        , const drawing::Direction3D& rDirectionToPoint4 )
            : m_aPoint1(rPoint1)
            , m_aPoint2(rPoint1+rDirectionToPoint2)
            , m_aPoint3(m_aPoint2+rDirectionToPoint4)
            , m_aPoint4(rPoint1+rDirectionToPoint4)
{

}

drawing::Direction3D Stripe::GetDirectionTo4() const
{
    return m_aPoint4-m_aPoint1;
}

/*
Stripe::Stripe( const drawing::Position3D& rPoint1
          , const drawing::Position3D& rPoint2
          , const drawing::Position3D& rPoint3
          , const drawing::Position3D& rPoint4 )
            : m_aPoint1(rPoint1)
            , m_aPoint2(rPoint2)
            , m_aPoint3(rPoint3)
            , m_aPoint4(rPoint4)
{

}
*/

uno::Any Stripe::getPolyPolygonShape3D() const
{
    drawing::PolyPolygonShape3D aPP;

    aPP.SequenceX.realloc(1);
    aPP.SequenceY.realloc(1);
    aPP.SequenceZ.realloc(1);

    drawing::DoubleSequence* pOuterSequenceX = aPP.SequenceX.getArray();
    drawing::DoubleSequence* pOuterSequenceY = aPP.SequenceY.getArray();
    drawing::DoubleSequence* pOuterSequenceZ = aPP.SequenceZ.getArray();

    pOuterSequenceX->realloc(4);
    pOuterSequenceY->realloc(4);
    pOuterSequenceZ->realloc(4);

    double* pInnerSequenceX = pOuterSequenceX->getArray();
    double* pInnerSequenceY = pOuterSequenceY->getArray();
    double* pInnerSequenceZ = pOuterSequenceZ->getArray();

    *pInnerSequenceX++ = m_aPoint1.PositionX;
    *pInnerSequenceY++ = m_aPoint1.PositionY;
    *pInnerSequenceZ++ = m_aPoint1.PositionZ;

    *pInnerSequenceX++ = m_aPoint2.PositionX;
    *pInnerSequenceY++ = m_aPoint2.PositionY;
    *pInnerSequenceZ++ = m_aPoint2.PositionZ;

    *pInnerSequenceX++ = m_aPoint3.PositionX;
    *pInnerSequenceY++ = m_aPoint3.PositionY;
    *pInnerSequenceZ++ = m_aPoint3.PositionZ;

    *pInnerSequenceX++ = m_aPoint4.PositionX;
    *pInnerSequenceY++ = m_aPoint4.PositionY;
    *pInnerSequenceZ++ = m_aPoint4.PositionZ;

    return uno::Any( &aPP, ::getCppuType((const drawing::PolyPolygonShape3D*)0) );
}

drawing::Direction3D Stripe::getNormal() const
{
    Polygon3D aPolygon3D(3);
    aPolygon3D[0] = Position3DToVector3D( m_aPoint1 );
    aPolygon3D[1] = Position3DToVector3D( m_aPoint2 );
    aPolygon3D[2] = Position3DToVector3D( m_aPoint3 );
    Vector3D aNormal = aPolygon3D.GetNormal();
    return Vector3DToDirection3D(aNormal);
}

uno::Any Stripe::getNormalsPolyPolygonShape3D() const
{
    drawing::PolyPolygonShape3D aPP;

    aPP.SequenceX.realloc(1);
    aPP.SequenceY.realloc(1);
    aPP.SequenceZ.realloc(1);

    drawing::DoubleSequence* pOuterSequenceX = aPP.SequenceX.getArray();
    drawing::DoubleSequence* pOuterSequenceY = aPP.SequenceY.getArray();
    drawing::DoubleSequence* pOuterSequenceZ = aPP.SequenceZ.getArray();

    pOuterSequenceX->realloc(4);
    pOuterSequenceY->realloc(4);
    pOuterSequenceZ->realloc(4);

    double* pInnerSequenceX = pOuterSequenceX->getArray();
    double* pInnerSequenceY = pOuterSequenceY->getArray();
    double* pInnerSequenceZ = pOuterSequenceZ->getArray();

    drawing::Direction3D aNormal( getNormal() );

    for(sal_Int32 nN=4; --nN; )
    {
        *pInnerSequenceX++ = aNormal.DirectionX;
        *pInnerSequenceY++ = aNormal.DirectionY;
        *pInnerSequenceZ++ = aNormal.DirectionZ;
    }
    return uno::Any( &aPP, ::getCppuType((const drawing::PolyPolygonShape3D*)0) );
}

//.............................................................................
} //namespace chart
//.............................................................................
