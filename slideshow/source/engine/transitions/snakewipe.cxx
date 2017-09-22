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


#include <osl/diagnose.h>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include "snakewipe.hxx"
#include "transitiontools.hxx"


namespace slideshow {
namespace internal {

SnakeWipe::SnakeWipe( sal_Int32 nElements, bool diagonal, bool flipOnYAxis )
    : m_sqrtElements( static_cast<sal_Int32>(
                          sqrt( static_cast<double>(nElements) ) ) ),
      m_elementEdge( 1.0 / m_sqrtElements ),
      m_diagonal(diagonal),
      m_flipOnYAxis(flipOnYAxis)
{
}

::basegfx::B2DPolyPolygon SnakeWipe::calcSnake( double t ) const
{
    ::basegfx::B2DPolyPolygon res;
    const double area = (t * m_sqrtElements * m_sqrtElements);
    const sal_Int32 line_ = (static_cast<sal_Int32>(area) / m_sqrtElements);
    const double line = ::basegfx::pruneScaleValue(
        static_cast<double>(line_) / m_sqrtElements );
    const double col = ::basegfx::pruneScaleValue(
        (area - (line_ * m_sqrtElements)) / m_sqrtElements );

    if (! ::basegfx::fTools::equalZero( line )) {
        ::basegfx::B2DPolygon poly;
        poly.append( ::basegfx::B2DPoint( 0.0, 0.0 ) );
        poly.append( ::basegfx::B2DPoint( 0.0, line ) );
        poly.append( ::basegfx::B2DPoint( 1.0, line ) );
        poly.append( ::basegfx::B2DPoint( 1.0, 0.0 ) );
        poly.setClosed(true);
        res.append(poly);
    }
    if (! ::basegfx::fTools::equalZero( col ))
    {
        double offset = 0.0;
        if ((line_ & 1) == 1) {
            // odd line: => right to left
            offset = (1.0 - col);
        }
        ::basegfx::B2DPolygon poly;
        poly.append( ::basegfx::B2DPoint( offset, line ) );
        poly.append( ::basegfx::B2DPoint( offset,
                                          line + m_elementEdge ) );
        poly.append( ::basegfx::B2DPoint( offset + col,
                                          line + m_elementEdge ) );
        poly.append( ::basegfx::B2DPoint( offset + col, line ) );
        poly.setClosed(true);
        res.append(poly);
    }

    return res;
}

::basegfx::B2DPolyPolygon SnakeWipe::calcHalfDiagonalSnake(
    double t, bool in ) const
{
    ::basegfx::B2DPolyPolygon res;

    if (in) {
        const double sqrtArea2 = sqrt( t * m_sqrtElements * m_sqrtElements );
        const double edge = ::basegfx::pruneScaleValue(
            static_cast<double>( static_cast<sal_Int32>(sqrtArea2) ) /
            m_sqrtElements );

        ::basegfx::B2DPolygon poly;
        if (! ::basegfx::fTools::equalZero( edge )) {
            poly.append( ::basegfx::B2DPoint( 0.0, 0.0 ) );
            poly.append( ::basegfx::B2DPoint( 0.0, edge ) );
            poly.append( ::basegfx::B2DPoint( edge, 0.0 ) );
            poly.setClosed(true);
            res.append(poly);
        }
        const double a = (M_SQRT1_2 / m_sqrtElements);
        const double d = (sqrtArea2 - static_cast<sal_Int32>(sqrtArea2));
        const double len = (t * M_SQRT2 * d);
        const double height = ::basegfx::pruneScaleValue( M_SQRT1_2 / m_sqrtElements );
        poly.clear();
        poly.append( ::basegfx::B2DPoint( 0.0, 0.0 ) );
        poly.append( ::basegfx::B2DPoint( 0.0, height ) );
        poly.append( ::basegfx::B2DPoint( len + a, height ) );
        poly.append( ::basegfx::B2DPoint( len + a, 0.0 ) );
        poly.setClosed(true);
        ::basegfx::B2DHomMatrix aTransform;

        if ((static_cast<sal_Int32>(sqrtArea2) & 1) == 1)
        {
            // odd line
            aTransform = basegfx::utils::createRotateB2DHomMatrix(M_PI_2 + M_PI_4);
            aTransform.translate(edge + m_elementEdge, 0.0);
        }
        else
        {
            aTransform = basegfx::utils::createTranslateB2DHomMatrix(-a, 0.0);
            aTransform.rotate( -M_PI_4 );
            aTransform.translate( 0.0, edge );
        }

        poly.transform( aTransform );
        res.append(poly);
    }
    else // out
    {
        const double sqrtArea2 = sqrt( t * m_sqrtElements * m_sqrtElements );
        const double edge = ::basegfx::pruneScaleValue(
            static_cast<double>( static_cast<sal_Int32>(sqrtArea2) ) /
            m_sqrtElements );

        ::basegfx::B2DPolygon poly;
        if (! ::basegfx::fTools::equalZero( edge )) {
            poly.append( ::basegfx::B2DPoint( 0.0, 1.0 ) );
            poly.append( ::basegfx::B2DPoint( edge, 1.0 ) );
            poly.append( ::basegfx::B2DPoint( 1.0, edge ) );
            poly.append( ::basegfx::B2DPoint( 1.0, 0.0 ) );
            poly.setClosed(true);
            res.append(poly);
        }
        const double a = (M_SQRT1_2 / m_sqrtElements);
        const double d = (sqrtArea2 - static_cast<sal_Int32>(sqrtArea2));
        const double len = ((1.0 - t) * M_SQRT2 * d);
        const double height = ::basegfx::pruneScaleValue( M_SQRT1_2 / m_sqrtElements );
        poly.clear();
        poly.append( ::basegfx::B2DPoint( 0.0, 0.0 ) );
        poly.append( ::basegfx::B2DPoint( 0.0, height ) );
        poly.append( ::basegfx::B2DPoint( len + a, height ) );
        poly.append( ::basegfx::B2DPoint( len + a, 0.0 ) );
        poly.setClosed(true);
        ::basegfx::B2DHomMatrix aTransform;

        if ((static_cast<sal_Int32>(sqrtArea2) & 1) == 1)
        {
            // odd line
            aTransform = basegfx::utils::createTranslateB2DHomMatrix(0.0, -height);
            aTransform.rotate( M_PI_2 + M_PI_4 );
            aTransform.translate( 1.0, edge );
        }
        else
        {
            aTransform = basegfx::utils::createRotateB2DHomMatrix(-M_PI_4);
            aTransform.translate( edge, 1.0 );
        }
        poly.transform( aTransform );
        res.append(poly);
    }

    return res;
}

::basegfx::B2DPolyPolygon SnakeWipe::operator () ( double t )
{
    ::basegfx::B2DPolyPolygon res;
    if (m_diagonal)
    {
        if (t >= 0.5) {
            res.append( calcHalfDiagonalSnake( 1.0, true ) );
            res.append( calcHalfDiagonalSnake( 2.0 * (t - 0.5), false ) );
        }
        else
            res.append( calcHalfDiagonalSnake( 2.0 * t, true ) );
    }
    else
        res = calcSnake(t);

    return m_flipOnYAxis ? flipOnYAxis(res) : res;
}

::basegfx::B2DPolyPolygon ParallelSnakesWipe::operator () ( double t )
{
    ::basegfx::B2DPolyPolygon res;
    if (m_diagonal)
    {
        OSL_ASSERT( m_opposite );
        ::basegfx::B2DPolyPolygon half(
            calcHalfDiagonalSnake( t, false /* out */ ) );
        // flip on x axis and rotate 90 degrees:
        basegfx::B2DHomMatrix aTransform(basegfx::utils::createScaleB2DHomMatrix(1.0, -1.0));
        aTransform.translate( -0.5, 0.5 );
        aTransform.rotate( M_PI_2 );
        aTransform.translate( 0.5, 0.5 );
        half.transform( aTransform );
        half.flip();
        res.append( half );

        // rotate 180 degrees:
        aTransform = basegfx::utils::createTranslateB2DHomMatrix(-0.5, -0.5);
        aTransform.rotate( M_PI );
        aTransform.translate( 0.5, 0.5 );
        half.transform( aTransform );
        res.append( half );
    }
    else
    {
        ::basegfx::B2DPolyPolygon half( calcSnake( t / 2.0 ) );
        // rotate 90 degrees:
        basegfx::B2DHomMatrix aTransform(basegfx::utils::createTranslateB2DHomMatrix(-0.5, -0.5));
        aTransform.rotate( M_PI_2 );
        aTransform.translate( 0.5, 0.5 );
        half.transform( aTransform );
        res.append( flipOnYAxis(half) );
        res.append( m_opposite ? flipOnXAxis(half) : half );
    }

    return m_flipOnYAxis ? flipOnYAxis(res) : res;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
