/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONFILLRULE_HXX
#define _BGFX_POLYGON_B2DPOLYPOLYGONFILLRULE_HXX

#include <sal/types.h>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
