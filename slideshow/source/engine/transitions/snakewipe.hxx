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

#if ! defined INCLUDED_SLIDESHOW_SNAKEWIPE_HXX
#define INCLUDED_SLIDESHOW_SNAKEWIPE_HXX

#include "parametricpolypolygon.hxx"

#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>


namespace slideshow {
namespace internal {

/// Generates a snake wipe:
class SnakeWipe : public ParametricPolyPolygon
{
public:
    SnakeWipe( sal_Int32 nElements, bool diagonal, bool flipOnYAxis );
    virtual ::basegfx::B2DPolyPolygon operator () ( double t );

protected:
    // topLeftHorizontal:
    ::basegfx::B2DPolyPolygon calcSnake( double t ) const;
    // topLeftDiagonal:
    ::basegfx::B2DPolyPolygon calcHalfDiagonalSnake( double t, bool in ) const;

    const sal_Int32 m_sqrtElements;
    const double m_elementEdge;
    const bool m_diagonal;
    const bool m_flipOnYAxis;
};

/// Generates a parallel snakes wipe:
class ParallelSnakesWipe : public SnakeWipe
{
public:
    ParallelSnakesWipe( sal_Int32 nElements,
                        bool diagonal, bool flipOnYAxis, bool opposite )
        : SnakeWipe( nElements, diagonal, flipOnYAxis ),
          m_opposite( opposite )
        {}
    virtual ::basegfx::B2DPolyPolygon operator () ( double t );
private:
    const bool m_opposite;
};

}
}

#endif /* INCLUDED_SLIDESHOW_SNAKEWIPE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
