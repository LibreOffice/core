/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: barndoorwipe.hxx,v $
 * $Revision: 1.5 $
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

#if ! defined INCLUDED_SLIDESHOW_BARNDOORWIPE_HXX
#define INCLUDED_SLIDESHOW_BARNDOORWIPE_HXX

#include <basegfx/polygon/b2dpolygon.hxx>

#include "parametricpolypolygon.hxx"
#include "transitiontools.hxx"


namespace slideshow {
namespace internal {

/// Generate a barn door wipe or double barn door wipe:
class BarnDoorWipe : public ParametricPolyPolygon
{
public:
    BarnDoorWipe( bool doubled = false )
        : m_unitRect( createUnitRect() ), m_doubled(doubled) {}
    virtual ::basegfx::B2DPolyPolygon operator()( double x );
private:
    const ::basegfx::B2DPolygon m_unitRect;
    const bool m_doubled;
};

}
}

#endif /* INCLUDED_SLIDESHOW_BARNDOORWIPE_HXX */
