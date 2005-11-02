/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b1drange.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:59:41 $
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

#ifndef _BGFX_RANGE_B1DRANGE_HXX
#include <basegfx/range/b1drange.hxx>
#endif
#ifndef _BGFX_RANGE_B1IRANGE_HXX
#include <basegfx/range/b1irange.hxx>
#endif
#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

namespace basegfx
{
    B1DRange::B1DRange( const B1IRange& rRange ) :
        maRange()
    {
        if( !rRange.isEmpty() )
        {
            maRange = rRange.getMinimum();
            expand(rRange.getMaximum());
        }
    }

    B1IRange fround(const B1DRange& rRange)
    {
        return rRange.isEmpty() ?
            B1IRange() :
            B1IRange( fround( rRange.getMinimum()),
                      fround( rRange.getMaximum()) );
    }

} // end of namespace basegfx

// eof
