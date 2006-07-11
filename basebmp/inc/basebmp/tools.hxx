/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tools.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: thb $ $Date: 2006-07-11 15:33:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
