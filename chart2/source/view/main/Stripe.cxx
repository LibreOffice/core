/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "Stripe.hxx"
#include "CommonConverters.hxx"
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#include <com/sun/star/drawing/DoubleSequence.hpp>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>

using namespace ::com::sun::star;

namespace chart
{

Stripe::Stripe( const drawing::Position3D& rPoint1
        , const drawing::Direction3D& rDirectionToPoint2
        , const drawing::Direction3D& rDirectionToPoint4 )
            : m_aPoint1(rPoint1)
            , m_aPoint2(rPoint1+rDirectionToPoint2)
            , m_aPoint3(m_aPoint2+rDirectionToPoint4)
            , m_aPoint4(rPoint1+rDirectionToPoint4)
            , m_bInvertNormal(false)
            , m_bManualNormalSet(false)
{
}

Stripe::Stripe( const drawing::Position3D& rPoint1
        , const drawing::Position3D& rPoint2
        , double fDepth )
        : m_aPoint1(rPoint1)
        , m_aPoint2(rPoint2)
        , m_aPoint3(rPoint2)
        , m_aPoint4(rPoint1)
        , m_bInvertNormal(false)
        , m_bManualNormalSet(false)
{
    m_aPoint3.PositionZ += fDepth;
    m_aPoint4.PositionZ += fDepth;
}

Stripe::Stripe( const drawing::Position3D& rPoint1
          , const drawing::Position3D& rPoint2
          , const drawing::Position3D& rPoint3
          , const drawing::Position3D& rPoint4 )
            : m_aPoint1(rPoint1)
            , m_aPoint2(rPoint2)
            , m_aPoint3(rPoint3)
            , m_aPoint4(rPoint4)
            , m_bInvertNormal(false)
            , m_bManualNormalSet(false)
{
}

void Stripe::SetManualNormal( const drawing::Direction3D& rNormal )
{
    m_aManualNormal = rNormal;
    m_bManualNormalSet = true;
}

void Stripe::InvertNormal( bool bInvertNormal )
{
    m_bInvertNormal = bInvertNormal;
}

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
    drawing::Direction3D aRet(1.0,0.0,0.0);

    if( m_bManualNormalSet )
        aRet = m_aManualNormal;
    else
    {
        ::basegfx::B3DPolygon aPolygon3D;
        aPolygon3D.append(Position3DToB3DPoint( m_aPoint1 ));
        aPolygon3D.append(Position3DToB3DPoint( m_aPoint2 ));
        aPolygon3D.append(Position3DToB3DPoint( m_aPoint3 ));
        aPolygon3D.append(Position3DToB3DPoint( m_aPoint4 ));
        ::basegfx::B3DVector aNormal(::basegfx::tools::getNormal(aPolygon3D));
        aRet = B3DVectorToDirection3D(aNormal);
    }

    if( m_bInvertNormal )
    {
        aRet.DirectionX *= -1.0;
        aRet.DirectionY *= -1.0;
        aRet.DirectionZ *= -1.0;
    }
    return aRet;
}

uno::Any Stripe::getNormalsPolygon() const
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

uno::Any Stripe::getTexturePolygon( short nRotatedTexture ) const
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

    if( nRotatedTexture==0 )
    {
        *pInnerSequenceX++ = 0.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 0.0;
        *pInnerSequenceY++ = 1.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 1.0;
        *pInnerSequenceY++ = 1.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 1.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceZ++ = 0.0;
    }
    else if( nRotatedTexture==1 )
    {
        *pInnerSequenceX++ = 1.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 0.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 0.0;
        *pInnerSequenceY++ = 1.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 1.0;
        *pInnerSequenceY++ = 1.0;
        *pInnerSequenceZ++ = 0.0;
    }
    else if( nRotatedTexture==2 )
    {
        *pInnerSequenceX++ = 1.0;
        *pInnerSequenceY++ = 1.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 1.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 0.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 0.0;
        *pInnerSequenceY++ = 1.0;
        *pInnerSequenceZ++ = 0.0;
    }
    else if( nRotatedTexture==3 )
    {
        *pInnerSequenceX++ = 0.0;
        *pInnerSequenceY++ = 1.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 1.0;
        *pInnerSequenceY++ = 1.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 1.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 0.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceZ++ = 0.0;
    }
    else if( nRotatedTexture==4 )
    {
        *pInnerSequenceX++ = 1.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 1.0;
        *pInnerSequenceY++ = 1.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 0.0;
        *pInnerSequenceY++ = 1.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 0.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceZ++ = 0.0;
    }
    else if( nRotatedTexture==5 )
    {
        *pInnerSequenceX++ = 0.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 1.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 1.0;
        *pInnerSequenceY++ = 1.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 0.0;
        *pInnerSequenceY++ = 1.0;
        *pInnerSequenceZ++ = 0.0;
    }
    else if( nRotatedTexture==6 )
    {
        *pInnerSequenceX++ = 0.0;
        *pInnerSequenceY++ = 1.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 0.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 1.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 1.0;
        *pInnerSequenceY++ = 1.0;
        *pInnerSequenceZ++ = 0.0;
    }
    else if( nRotatedTexture==7 )
    {
        *pInnerSequenceX++ = 1.0;
        *pInnerSequenceY++ = 1.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 0.0;
        *pInnerSequenceY++ = 1.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 0.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceZ++ = 0.0;

        *pInnerSequenceX++ = 1.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceZ++ = 0.0;
    }

    return uno::Any( &aPP, ::getCppuType((const drawing::PolyPolygonShape3D*)0) );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
