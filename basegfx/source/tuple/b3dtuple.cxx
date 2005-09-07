/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b3dtuple.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:53:01 $
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

#ifndef _BGFX_TUPLE_B3DTUPLE_HXX
#include <basegfx/tuple/b3dtuple.hxx>
#endif
#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif

namespace { struct EmptyTuple : public rtl::Static<basegfx::B3DTuple, EmptyTuple> {}; }

#ifndef _BGFX_TUPLE_B3ITUPLE_HXX
#include <basegfx/tuple/b3ituple.hxx>
#endif

namespace basegfx
{
    B3DTuple::B3DTuple(const B3ITuple& rTup)
    :   mfX( rTup.getX() ),
        mfY( rTup.getY() ),
        mfZ( rTup.getZ() )
    {}

    B3ITuple fround(const B3DTuple& rTup)
    {
        return B3ITuple(fround(rTup.getX()), fround(rTup.getY()), fround(rTup.getZ()));
    }

    static const B3DTuple& getEmptyTuple()
    {
        return EmptyTuple::get();
    }
} // end of namespace basegfx

// eof
