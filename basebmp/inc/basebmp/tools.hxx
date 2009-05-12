/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tools.hxx,v $
 * $Revision: 1.2 $
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

#ifndef INCLUDED_BASEBMP_TOOLS_HXX
#define INCLUDED_BASEBMP_TOOLS_HXX

#include <basegfx/range/b2irange.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <vigra/tuple.hxx>
#include <vigra/diff2d.hxx>

namespace basebmp
{
    inline vigra::Diff2D topLeft( const basegfx::B2IRange& rRange )
    { return vigra::Diff2D(rRange.getMinX(),rRange.getMinY()); }

    inline vigra::Diff2D bottomRight( const basegfx::B2IRange& rRange )
    { return vigra::Diff2D(rRange.getMaxX(),rRange.getMaxY()); }

    template< class Iterator, class Accessor >
    inline vigra::triple<Iterator,Iterator,Accessor>
    destIterRange(Iterator const&          begin,
                  Accessor const&          accessor,
                  const basegfx::B2IRange& rRange)
    {
        return vigra::triple<Iterator,Iterator,Accessor>(
            begin + topLeft(rRange),
            begin + bottomRight(rRange),
            accessor);
    }

    template< class Iterator, class Accessor >
    inline vigra::triple<Iterator,Iterator,Accessor>
    srcIterRange(Iterator const&          begin,
                 Accessor const&          accessor,
                 const basegfx::B2IRange& rRange)
    {
        return vigra::triple<Iterator,Iterator,Accessor>(
            begin + topLeft(rRange),
            begin + bottomRight(rRange),
            accessor);
    }

    template< class Iterator, class Accessor >
    inline vigra::pair<Iterator,Accessor>
    srcIter(Iterator const&          begin,
            Accessor const&          accessor,
            const basegfx::B2IPoint& rPt)
    {
        return vigra::pair<Iterator,Accessor>(
            begin + vigra::Diff2D(rPt.getX(),rPt.getY()),
            accessor);
    }

    template< class Iterator, class Accessor >
    inline vigra::pair<Iterator,Accessor>
    destIter(Iterator const&          begin,
             Accessor const&          accessor,
             const basegfx::B2IPoint& rPt)
    {
        return vigra::pair<Iterator,Accessor>(
            begin + vigra::Diff2D(rPt.getX(),rPt.getY()),
            accessor);
    }
}

#endif /* INCLUDED_BASEBMP_TOOLS_HXX */
