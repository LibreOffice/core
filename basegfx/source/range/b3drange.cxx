/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b3drange.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:50:36 $
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

#ifndef _BGFX_RANGE_B3DRANGE_HXX
#include <basegfx/range/b3drange.hxx>
#endif

#ifndef _BGFX_RANGE_B3IRANGE_HXX
#include <basegfx/range/b3irange.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

namespace basegfx
{
    B3DRange::B3DRange(const B3IRange& rRange) :
        maRangeX( rRange.getMinX() ),
        maRangeY( rRange.getMinY() ),
        maRangeZ( rRange.getMinZ() )
    {
        maRangeX.expand( rRange.getMaxX() );
        maRangeY.expand( rRange.getMaxY() );
        maRangeZ.expand( rRange.getMaxZ() );
    }

    B3IRange fround(const B3DRange& rRange )
    {
        return B3IRange(fround(rRange.getMinX()),
                        fround(rRange.getMinY()),
                        fround(rRange.getMinZ()),
                        fround(rRange.getMaxX()),
                        fround(rRange.getMaxY()),
                        fround(rRange.getMaxZ()));
    }

} // end of namespace basegfx

// eof
