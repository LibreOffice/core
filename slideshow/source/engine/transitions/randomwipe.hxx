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

#if ! defined INCLUDED_SLIDESHOW_RANDOMWIPE_HXX
#define INCLUDED_SLIDESHOW_RANDOMWIPE_HXX

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <boost/scoped_array.hpp>

#include "parametricpolypolygon.hxx"
#include "transitiontools.hxx"


namespace slideshow {
namespace internal {

class RandomWipe : public ParametricPolyPolygon
{
public:
    RandomWipe( sal_Int32 nElements,
                bool randomBars /* true: generates a horizontal random bar wipe,
                                   false: generates a dissolve wipe */ );
    virtual ::basegfx::B2DPolyPolygon operator () ( double t );
private:
    ::boost::scoped_array< ::basegfx::B2DPoint > m_positions;
    sal_Int32 m_nElements;
    ::basegfx::B2DPolygon m_rect;
};

}
}

#endif /* INCLUDED_SLIDESHOW_RANDOMWIPE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
