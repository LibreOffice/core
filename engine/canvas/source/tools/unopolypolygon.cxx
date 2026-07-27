/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <basegfx/range/b2drange.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <canvas/canvastools.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unopolypolygon.hxx>
#include <utility>

using namespace ::com::sun::star;

namespace canvastools
{
    UnoPolyPolygon::UnoPolyPolygon( basegfx::B2DPolyPolygon aPolyPoly ) :
        maPolyPoly(std::move( aPolyPoly )),
        meFillRule( rendering::FillRule_EVEN_ODD )
    {
    }

    UnoPolyPolygon::UnoPolyPolygon( basegfx::B2DPolyPolygon aPolyPoly, css::rendering::FillRule fillRule ) :
        maPolyPoly(std::move( aPolyPoly )),
        meFillRule( fillRule )
    {
    }

    sal_Int32 UnoPolyPolygon::getNumberOfPolygons()
    {
        std::unique_lock const guard( m_aMutex );
        return maPolyPoly.count();
    }

    rendering::FillRule UnoPolyPolygon::getFillRule()
    {
        std::unique_lock const guard( m_aMutex );
        return meFillRule;
    }

    cpo::uno::Sequence< cpo::uno::Sequence< geometry::RealPoint2D > > UnoPolyPolygon::getPoints(
        sal_Int32 nPolygonIndex,
        sal_Int32 nNumberOfPolygons,
        sal_Int32 nPointIndex,
        sal_Int32 nNumberOfPoints )
    {
        return basegfx::unotools::pointSequenceSequenceFromB2DPolyPolygon(
            getSubsetPolyPolygon( nPolygonIndex,
                                  nNumberOfPolygons,
                                  nPointIndex,
                                  nNumberOfPoints ) );
    }

    cpo::uno::Sequence< cpo::uno::Sequence< geometry::RealBezierSegment2D > > UnoPolyPolygon::getBezierSegments(
        sal_Int32 nPolygonIndex,
        sal_Int32 nNumberOfPolygons,
        sal_Int32 nPointIndex,
        sal_Int32 nNumberOfPoints )
    {
        return basegfx::unotools::bezierSequenceSequenceFromB2DPolyPolygon(
            getSubsetPolyPolygon( nPolygonIndex,
                                  nNumberOfPolygons,
                                  nPointIndex,
                                  nNumberOfPoints ) );
    }

    basegfx::B2DPolyPolygon UnoPolyPolygon::getSubsetPolyPolygon(
        sal_Int32 nPolygonIndex,
        sal_Int32 nNumberOfPolygons,
        sal_Int32 nPointIndex,
        sal_Int32 nNumberOfPoints ) const
    {
        std::unique_lock const guard( m_aMutex );
        checkIndex( nPolygonIndex );

        const sal_Int32 nPolyCount( maPolyPoly.count() );

        // check for "full polygon" case
        if( !nPolygonIndex &&
            !nPointIndex &&
            nNumberOfPolygons == nPolyCount &&
            nNumberOfPoints == -1 )
        {
            return maPolyPoly;
        }

        basegfx::B2DPolyPolygon aSubsetPoly;

        // create temporary polygon (as an extract from maPoly,
        // which contains the requested subset)
        for( sal_Int32 i=nPolygonIndex; i<nNumberOfPolygons; ++i )
        {
            checkIndex(i);

            const basegfx::B2DPolygon& rCurrPoly( maPolyPoly.getB2DPolygon(i) );

            sal_Int32 nFirstPoint(0);
            sal_Int32 nLastPoint(nPolyCount-1);

            if( nPointIndex && i==nPolygonIndex )
            {
                // very first polygon - respect nPointIndex, if
                // not zero

                // empty polygon - impossible to specify _any_
                // legal value except 0 here!
                if( !nPolyCount)
                    throw lang::IndexOutOfBoundsException();

                nFirstPoint = nPointIndex;
            }

            if( i==nNumberOfPolygons-1 && nNumberOfPoints != -1 )
            {
                // very last polygon - respect nNumberOfPoints

                // empty polygon - impossible to specify _any_
                // legal value except -1 here!
                if( !nPolyCount )
                    throw lang::IndexOutOfBoundsException();

                nLastPoint = nFirstPoint+nNumberOfPoints;
            }

            if( !nPolyCount )
            {
                // empty polygon - index checks already performed
                // above, now simply append empty polygon
                aSubsetPoly.append( rCurrPoly );
            }
            else
            {
                if( nFirstPoint < 0 || nFirstPoint >= nPolyCount )
                    throw lang::IndexOutOfBoundsException();

                if( nLastPoint < 0 || nLastPoint >= nPolyCount )
                    throw lang::IndexOutOfBoundsException();

                basegfx::B2DPolygon aTmp;
                for( sal_Int32 j=nFirstPoint; j<nLastPoint; ++j )
                    aTmp.append( rCurrPoly.getB2DPoint(j) );

                aSubsetPoly.append( aTmp );
            }
        }

        return aSubsetPoly;
    }

    OUString UnoPolyPolygon::getImplementationName()
    {
        return u"gfx::internal::UnoPolyPolygon"_ustr;
    }

    bool UnoPolyPolygon::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService(this, ServiceName);
    }

    cpo::uno::Sequence< OUString > UnoPolyPolygon::getSupportedServiceNames()
    {
        return { u"com.sun.star.rendering.PolyPolygon2D"_ustr };
    }

    basegfx::B2DPolyPolygon UnoPolyPolygon::getPolyPolygon() const
    {
        std::unique_lock const guard( m_aMutex );

        return maPolyPoly;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
