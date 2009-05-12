/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sweepwipe.hxx,v $
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

#if ! defined INCLUDED_SLIDESHOW_SWEEPWIPE_HXX
#define INCLUDED_SLIDESHOW_SWEEPWIPE_HXX

#include "parametricpolypolygon.hxx"


namespace slideshow {
namespace internal {

class SweepWipe : public ParametricPolyPolygon
{
public:
    SweepWipe( bool center, bool single,
               bool oppositeVertical, bool flipOnYAxis )
        : m_center(center), m_single(single),
          m_oppositeVertical(oppositeVertical), m_flipOnYAxis(flipOnYAxis)
        {}
    virtual ::basegfx::B2DPolyPolygon operator () ( double t );
private:
    const bool m_center, m_single, m_oppositeVertical, m_flipOnYAxis;
};

}
}

#endif /* INCLUDED_SLIDESHOW_SWEEPWIPE_HXX */
