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

#if ! defined INCLUDED_SLIDESHOW_FIGUREWIPE_HXX
#define INCLUDED_SLIDESHOW_FIGUREWIPE_HXX

#include "parametricpolypolygon.hxx"


namespace slideshow {
namespace internal {

class FigureWipe : public ParametricPolyPolygon
{
public:
    static FigureWipe * createTriangleWipe();
    static FigureWipe * createArrowHeadWipe();
    static FigureWipe * createStarWipe( sal_Int32 nPoints );
    static FigureWipe * createPentagonWipe();
    static FigureWipe * createHexagonWipe();

    virtual ::basegfx::B2DPolyPolygon operator () ( double t );
private:
    FigureWipe( ::basegfx::B2DPolygon const & figure ) : m_figure(figure) {}
    const ::basegfx::B2DPolygon m_figure;
};

}
}

#endif /* INCLUDED_SLIDESHOW_FIGUREWIPE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
