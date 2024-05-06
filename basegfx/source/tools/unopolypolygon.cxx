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

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/utils/unopolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <utility>

using namespace ::com::sun::star;

namespace basegfx::unotools
{
    UnoPolyPolygon::UnoPolyPolygon( B2DPolyPolygon aPolyPoly ) :
        maPolyPoly(std::move( aPolyPoly )),
        meFillRule( rendering::FillRule_EVEN_ODD )
    {
    }

    void SAL_CALL UnoPolyPolygon::addPolyPolygon(
        const geometry::RealPoint2D&                        position,
        const uno::Reference< rendering::XPolyPolygon2D >&  polyPolygon )
    {
        std::unique_lock const guard( m_aMutex );
        modifying();

        // TODO(F1): Correctly fulfill the UNO API
        // specification. This will probably result in a vector of
        // poly-polygons to be stored in this object.

        const sal_Int32 nPolys( polyPolygon->getNumberOfPolygons() );

        if( !polyPolygon.is() || !nPolys )
        {
            // invalid or empty polygon - nothing to do.
            return;
        }

        B2DPolyPolygon        aSrcPoly;
        const UnoPolyPolygon* pSrc( dynamic_cast< UnoPolyPolygon* >(polyPolygon.get()) );

        // try to extract polygon data from interface. First,
        // check whether it's the same implementation object,
        // which we can tunnel then.
        if( pSrc )
        {
            aSrcPoly = pSrc->getPolyPolygon();
        }
        else
        {
            // not a known implementation object - try data source
            // interfaces
            uno::Reference< rendering::XBezierPolyPolygon2D > xBezierPoly(
                polyPolygon,
                uno::UNO_QUERY );

            if( xBezierPoly.is() )
            {
                aSrcPoly = unotools::polyPolygonFromBezier2DSequenceSequence(
                    xBezierPoly->getBezierSegments( 0,
                                                    nPolys,
                                                    0,
                                                    -1 ) );
            }
            else
            {
                uno::Reference< rendering::XLinePolyPolygon2D > xLinePoly(
                    polyPolygon,
                    uno::UNO_QUERY );

                // no implementation class and no data provider
                // found - contract violation.
                if( !xLinePoly.is() )
                    throw lang::IllegalArgumentException(
                        u"UnoPolyPolygon::addPolyPolygon(): Invalid input "
                        "poly-polygon, cannot retrieve vertex data"_ustr,
                        getXWeak(), 1);

                aSrcPoly = unotools::polyPolygonFromPoint2DSequenceSequence(
                    xLinePoly->getPoints( 0,
                                          nPolys,
                                          0,
                                          -1 ) );
            }
        }

        const B2DRange  aBounds( utils::getRange( aSrcPoly ) );
        const B2DVector     aOffset( unotools::b2DPointFromRealPoint2D( position ) -
                                             aBounds.getMinimum() );

        if( !aOffset.equalZero() )
        {
            const B2DHomMatrix aTranslate(utils::createTranslateB2DHomMatrix(aOffset));
            aSrcPoly.transform( aTranslate );
        }

        maPolyPoly.append( aSrcPoly );
    }

    sal_Int32 SAL_CALL UnoPolyPolygon::getNumberOfPolygons()
    {
        std::unique_lock const guard( m_aMutex );
        return maPolyPoly.count();
    }

    sal_Int32 SAL_CALL UnoPolyPolygon::getNumberOfPolygonPoints(
        sal_Int32 polygon )
    {
        std::unique_lock const guard( m_aMutex );
        checkIndex( polygon );

        return maPolyPoly.getB2DPolygon(polygon).count();
    }

    rendering::FillRule SAL_CALL UnoPolyPolygon::getFillRule()
    {
        std::unique_lock const guard( m_aMutex );
        return meFillRule;
    }

    void SAL_CALL UnoPolyPolygon::setFillRule(
        rendering::FillRule fillRule )
    {
        std::unique_lock const guard( m_aMutex );
        modifying();

        meFillRule = fillRule;
    }

    sal_Bool SAL_CALL UnoPolyPolygon::isClosed(
        sal_Int32 index )
    {
        std::unique_lock const guard( m_aMutex );
        checkIndex( index );

        return maPolyPoly.getB2DPolygon(index).isClosed();
    }

    void SAL_CALL UnoPolyPolygon::setClosed(
        sal_Int32 index,
        sal_Bool closedState )
    {
        std::unique_lock const guard( m_aMutex );
        modifying();

        if( index == -1 )
        {
            // set all
            maPolyPoly.setClosed( closedState );
        }
        else
        {
            checkIndex( index );

            // fetch referenced polygon, change state
            B2DPolygon aTmp( maPolyPoly.getB2DPolygon(index) );
            aTmp.setClosed( closedState );

            // set back to container
            maPolyPoly.setB2DPolygon( index, aTmp );
        }
    }

    uno::Sequence< uno::Sequence< geometry::RealPoint2D > > SAL_CALL UnoPolyPolygon::getPoints(
        sal_Int32 nPolygonIndex,
        sal_Int32 nNumberOfPolygons,
        sal_Int32 nPointIndex,
        sal_Int32 nNumberOfPoints )
    {
        return unotools::pointSequenceSequenceFromB2DPolyPolygon(
            getSubsetPolyPolygon( nPolygonIndex,
                                  nNumberOfPolygons,
                                  nPointIndex,
                                  nNumberOfPoints ) );
    }

    void SAL_CALL UnoPolyPolygon::setPoints(
        const uno::Sequence< uno::Sequence< geometry::RealPoint2D > >& points,
        sal_Int32 nPolygonIndex )
    {
        std::unique_lock const guard( m_aMutex );
        modifying();

        const B2DPolyPolygon& rNewPolyPoly(
            unotools::polyPolygonFromPoint2DSequenceSequence( points ) );

        if( nPolygonIndex == -1 )
        {
            maPolyPoly = rNewPolyPoly;
        }
        else
        {
            checkIndex( nPolygonIndex );

            maPolyPoly.insert( nPolygonIndex, rNewPolyPoly );
        }
    }

    geometry::RealPoint2D SAL_CALL UnoPolyPolygon::getPoint(
        sal_Int32 nPolygonIndex,
        sal_Int32 nPointIndex )
    {
        std::unique_lock const guard( m_aMutex );
        checkIndex( nPolygonIndex );

        const B2DPolygon& rPoly( maPolyPoly.getB2DPolygon( nPolygonIndex ) );

        if( nPointIndex < 0 || o3tl::make_unsigned(nPointIndex) >= rPoly.count() )
            throw lang::IndexOutOfBoundsException();

        return unotools::point2DFromB2DPoint( rPoly.getB2DPoint( nPointIndex ) );
    }

    void SAL_CALL UnoPolyPolygon::setPoint(
        const geometry::RealPoint2D& point,
        sal_Int32 nPolygonIndex,
        sal_Int32 nPointIndex )
    {
        std::unique_lock const guard( m_aMutex );
        checkIndex( nPolygonIndex );
        modifying();

        B2DPolygon aPoly( maPolyPoly.getB2DPolygon( nPolygonIndex ) );

        if( nPointIndex < 0 || o3tl::make_unsigned(nPointIndex) >= aPoly.count() )
            throw lang::IndexOutOfBoundsException();

        aPoly.setB2DPoint( nPointIndex,
                           unotools::b2DPointFromRealPoint2D( point ) );
        maPolyPoly.setB2DPolygon( nPolygonIndex, aPoly );
    }

    uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > > SAL_CALL UnoPolyPolygon::getBezierSegments(
        sal_Int32 nPolygonIndex,
        sal_Int32 nNumberOfPolygons,
        sal_Int32 nPointIndex,
        sal_Int32 nNumberOfPoints )
    {
        return unotools::bezierSequenceSequenceFromB2DPolyPolygon(
            getSubsetPolyPolygon( nPolygonIndex,
                                  nNumberOfPolygons,
                                  nPointIndex,
                                  nNumberOfPoints ) );
    }

    void SAL_CALL UnoPolyPolygon::setBezierSegments(
        const uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > >&  points,
        sal_Int32                                                               nPolygonIndex )
    {
        std::unique_lock const guard( m_aMutex );
        modifying();
        const B2DPolyPolygon& rNewPolyPoly(
            unotools::polyPolygonFromBezier2DSequenceSequence( points ) );

        if( nPolygonIndex == -1 )
        {
            maPolyPoly = rNewPolyPoly;
        }
        else
        {
            checkIndex( nPolygonIndex );

            maPolyPoly.insert( nPolygonIndex, rNewPolyPoly );
        }
    }

    geometry::RealBezierSegment2D SAL_CALL UnoPolyPolygon::getBezierSegment( sal_Int32 nPolygonIndex,
                                                                             sal_Int32 nPointIndex )
    {
        std::unique_lock const guard( m_aMutex );
        checkIndex( nPolygonIndex );

        const B2DPolygon& rPoly( maPolyPoly.getB2DPolygon( nPolygonIndex ) );
        const sal_uInt32  nPointCount(rPoly.count());

        if( nPointIndex < 0 || o3tl::make_unsigned(nPointIndex) >= nPointCount )
            throw lang::IndexOutOfBoundsException();

        const B2DPoint& rPt( rPoly.getB2DPoint( nPointIndex ) );
        const B2DPoint& rCtrl0( rPoly.getNextControlPoint(nPointIndex) );
        const B2DPoint& rCtrl1( rPoly.getPrevControlPoint((nPointIndex + 1) % nPointCount) );

        return geometry::RealBezierSegment2D( rPt.getX(),
                                              rPt.getY(),
                                              rCtrl0.getX(),
                                              rCtrl0.getY(),
                                              rCtrl1.getX(),
                                              rCtrl1.getY() );
    }

    void SAL_CALL UnoPolyPolygon::setBezierSegment( const geometry::RealBezierSegment2D& segment,
                                                         sal_Int32                       nPolygonIndex,
                                                         sal_Int32                       nPointIndex )
    {
        std::unique_lock const guard( m_aMutex );
        checkIndex( nPolygonIndex );
        modifying();

        B2DPolygon aPoly( maPolyPoly.getB2DPolygon( nPolygonIndex ) );
        const sal_uInt32 nPointCount(aPoly.count());

        if( nPointIndex < 0 || o3tl::make_unsigned(nPointIndex) >= nPointCount )
            throw lang::IndexOutOfBoundsException();

        aPoly.setB2DPoint( nPointIndex,
                           B2DPoint( segment.Px,
                                     segment.Py ) );
        aPoly.setNextControlPoint(nPointIndex,
                                  B2DPoint(segment.C1x, segment.C1y));
        aPoly.setPrevControlPoint((nPointIndex + 1) % nPointCount,
                                  B2DPoint(segment.C2x, segment.C2y));

        maPolyPoly.setB2DPolygon( nPolygonIndex, aPoly );
    }

    B2DPolyPolygon UnoPolyPolygon::getSubsetPolyPolygon(
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

        B2DPolyPolygon aSubsetPoly;

        // create temporary polygon (as an extract from maPoly,
        // which contains the requested subset)
        for( sal_Int32 i=nPolygonIndex; i<nNumberOfPolygons; ++i )
        {
            checkIndex(i);

            const B2DPolygon& rCurrPoly( maPolyPoly.getB2DPolygon(i) );

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

                B2DPolygon aTmp;
                for( sal_Int32 j=nFirstPoint; j<nLastPoint; ++j )
                    aTmp.append( rCurrPoly.getB2DPoint(j) );

                aSubsetPoly.append( aTmp );
            }
        }

        return aSubsetPoly;
    }

    OUString SAL_CALL UnoPolyPolygon::getImplementationName()
    {
        return u"gfx::internal::UnoPolyPolygon"_ustr;
    }

    sal_Bool SAL_CALL UnoPolyPolygon::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService(this, ServiceName);
    }

    uno::Sequence< OUString > SAL_CALL UnoPolyPolygon::getSupportedServiceNames()
    {
        return { u"com.sun.star.rendering.PolyPolygon2D"_ustr };
    }

    B2DPolyPolygon UnoPolyPolygon::getPolyPolygon() const
    {
        std::unique_lock const guard( m_aMutex );

        return maPolyPoly;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
