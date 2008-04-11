/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: drawmodes.hxx,v $
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

#ifndef INCLUDED_BASEBMP_DRAWMODES_HXX
#define INCLUDED_BASEBMP_DRAWMODES_HXX

/* Definition of Draw modes */

namespace basebmp
{
    enum DrawMode
    {
        /** Default draw mode, which simply renders pixel in the
            requested color
         */
        DrawMode_PAINT,

        /** XOR draw mode, which XORs each existing pixel value with
            the new color.

            The result of this XOR operation strongly depends on the
            underlying pixel format, as it is defined by the bitwise
            XOR of the (potentially palette-looked-up) color value and
            the existing pixel content (being it true color or a
            palette index).
         */
        DrawMode_XOR
    };
}

#endif /* INCLUDED_BASEBMP_DRAWMODES_HXX */
