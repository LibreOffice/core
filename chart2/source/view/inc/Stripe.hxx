/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _CHART_STRIPE_HXX
#define _CHART_STRIPE_HXX

#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/uno/Any.h>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/** A Stripe represents a 2 dimensional foursquare plane in a 3 dimaensional room.

@todo could: it is not necessary to have 4 point members here; it would be sufficient to have one point and 2 directions
*/

class Stripe
{
public:
    Stripe( const ::com::sun::star::drawing::Position3D& rPoint1
        , const ::com::sun::star::drawing::Direction3D& rDirectionToPoint2
        , const ::com::sun::star::drawing::Direction3D& rDirectionToPoint4 );

    Stripe( const ::com::sun::star::drawing::Position3D& rPoint1
        , const ::com::sun::star::drawing::Position3D& rPoint2
        , double fDepth );

    Stripe( const ::com::sun::star::drawing::Position3D& rPoint1
        , const ::com::sun::star::drawing::Position3D& rPoint2
        , const ::com::sun::star::drawing::Position3D& rPoint3
        , const ::com::sun::star::drawing::Position3D& rPoint4 );

    void SetManualNormal( const ::com::sun::star::drawing::Direction3D& rNormal );
    ::com::sun::star::drawing::Direction3D getNormal() const;

    void InvertNormal( bool bInvertNormal );

    ::com::sun::star::uno::Any      getPolyPolygonShape3D() const;
    ::com::sun::star::uno::Any      getNormalsPolygon() const;
    ::com::sun::star::uno::Any      getTexturePolygon( short nRotatedTexture ) const; //0 to 7 are the different possibilities

    ::com::sun::star::drawing::Position3D GetPosition1() const { return m_aPoint1; }
    ::com::sun::star::drawing::Position3D GetPosition2() const { return m_aPoint2; }
    ::com::sun::star::drawing::Position3D GetPosition3() const { return m_aPoint3; }
    ::com::sun::star::drawing::Position3D GetPosition4() const { return m_aPoint4; }

private:
    ::com::sun::star::drawing::Position3D m_aPoint1;
    ::com::sun::star::drawing::Position3D m_aPoint2;
    ::com::sun::star::drawing::Position3D m_aPoint3;
    ::com::sun::star::drawing::Position3D m_aPoint4;

    bool m_bInvertNormal;
    bool m_bManualNormalSet;
    ::com::sun::star::drawing::Direction3D m_aManualNormal;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
