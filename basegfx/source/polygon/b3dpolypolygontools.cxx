/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b3dpolypolygontools.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:49:06 $
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

#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#endif

#ifndef _BGFX_RANGE_B3DRANGE_HXX
#include <basegfx/range/b3drange.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYPOLYGON_HXX
#include <basegfx/polygon/b3dpolypolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolygontools.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#include <numeric>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace tools
    {
        // B3DPolyPolygon tools
        ::basegfx::B3DRange getRange(const ::basegfx::B3DPolyPolygon& rCandidate)
        {
            ::basegfx::B3DRange aRetval;
            const sal_uInt32 nPolygonCount(rCandidate.count());

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                ::basegfx::B3DPolygon aCandidate = rCandidate.getB3DPolygon(a);
                aRetval.expand(::basegfx::tools::getRange(aCandidate));
            }

            return aRetval;
        }

        ::basegfx::B3DPolyPolygon applyLineDashing(const ::basegfx::B3DPolyPolygon& rCandidate, const ::std::vector<double>& raDashDotArray, double fFullDashDotLen)
        {
            ::basegfx::B3DPolyPolygon aRetval;

            if(0.0 == fFullDashDotLen && raDashDotArray.size())
            {
                // calculate fFullDashDotLen from raDashDotArray
                fFullDashDotLen = ::std::accumulate(raDashDotArray.begin(), raDashDotArray.end(), 0.0);
            }

            if(rCandidate.count() && fFullDashDotLen > 0.0)
            {
                for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
                {
                    ::basegfx::B3DPolygon aCandidate = rCandidate.getB3DPolygon(a);
                    aRetval.append(applyLineDashing(aCandidate, raDashDotArray, fFullDashDotLen));
                }
            }

            return aRetval;
        }
    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

// eof
