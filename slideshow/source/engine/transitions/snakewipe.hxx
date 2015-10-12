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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_TRANSITIONS_SNAKEWIPE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_TRANSITIONS_SNAKEWIPE_HXX

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
    virtual ::basegfx::B2DPolyPolygon operator () ( double t ) override;

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
    virtual ::basegfx::B2DPolyPolygon operator () ( double t ) override;
private:
    const bool m_opposite;
};

}
}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_TRANSITIONS_SNAKEWIPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
