/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dtuple.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:52:13 $
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

#ifndef _BGFX_TUPLE_B2DTUPLE_HXX
#include <basegfx/tuple/b2dtuple.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif
#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif

namespace { struct EmptyTuple : public rtl::Static<basegfx::B2DTuple, EmptyTuple> {}; }

#ifndef _BGFX_TUPLE_B2ITUPLE_HXX
#include <basegfx/tuple/b2ituple.hxx>
#endif

namespace basegfx
{
    const B2DTuple& B2DTuple::getEmptyTuple()
    {
        return EmptyTuple::get();
    }

    B2DTuple::B2DTuple(const B2ITuple& rTup)
    :   mfX( rTup.getX() ),
        mfY( rTup.getY() )
    {}

    bool B2DTuple::equalZero() const
    {
        return (this == &getEmptyTuple() ||
                (::basegfx::fTools::equalZero(mfX) && ::basegfx::fTools::equalZero(mfY)));
    }

    bool B2DTuple::equalZero(const double& rfSmallValue) const
    {
        return (this == &getEmptyTuple() ||
                (::basegfx::fTools::equalZero(mfX, rfSmallValue) && ::basegfx::fTools::equalZero(mfY, rfSmallValue)));
    }

    bool B2DTuple::equal(const B2DTuple& rTup) const
    {
        return (
            ::basegfx::fTools::equal(mfX, rTup.mfX) &&
            ::basegfx::fTools::equal(mfY, rTup.mfY));
    }

    bool B2DTuple::equal(const B2DTuple& rTup, const double& rfSmallValue) const
    {
        return (
            ::basegfx::fTools::equal(mfX, rTup.mfX, rfSmallValue) &&
            ::basegfx::fTools::equal(mfY, rTup.mfY, rfSmallValue));
    }

    void B2DTuple::correctValues(const double fCompareValue)
    {
        if(0.0 == fCompareValue)
        {
            if(::basegfx::fTools::equalZero(mfX))
            {
                mfX = 0.0;
            }

            if(::basegfx::fTools::equalZero(mfY))
            {
                mfY = 0.0;
            }
        }
        else
        {
            if(::basegfx::fTools::equal(mfX, fCompareValue))
            {
                mfX = fCompareValue;
            }

            if(::basegfx::fTools::equal(mfY, fCompareValue))
            {
                mfY = fCompareValue;
            }
        }
    }

    B2ITuple fround(const B2DTuple& rTup)
    {
        return B2ITuple(fround(rTup.getX()), fround(rTup.getY()));
    }

} // end of namespace basegfx

// eof
