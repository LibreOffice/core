/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawmodes.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 13:46:16 $
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
