/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolypolygonfillrule.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:29:25 $
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

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONFILLRULE_HXX
#define _BGFX_POLYGON_B2DPOLYPOLYGONFILLRULE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    /** Fill rule to use for poly-polygon filling.

        The fill rule determines which areas are inside, and which are
        outside the poly-polygon.
     */
    enum FillRule
    {
        /** Areas, for which a scanline has crossed an odd number of
            vertices, are regarded 'inside', the remainder 'outside'
            of the poly-polygon.
         */
        FillRule_EVEN_ODD,

        /** For each edge a scanline crosses, a current winding number
            is updated. Downward edges count +1, upward edges count
            -1. If the total accumulated winding number for one area
            is not zero, this area is regarded 'inside', otherwise,
            'outside'.
         */
        FillRule_NONZERO_WINDING_NUMBER
    };
}

#endif /* _BGFX_POLYGON_B2DPOLYPOLYGONFILLRULE_HXX */
