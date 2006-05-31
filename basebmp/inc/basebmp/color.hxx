/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: color.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2006-05-31 10:12:11 $
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

#ifndef INCLUDED_BASEBMP_COLOR_HXX
#define INCLUDED_BASEBMP_COLOR_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#include <math.h>

namespace basebmp
{

class Color
{
private:
    sal_uInt32          mnColor;

public:
    Color() : mnColor(0) {}
    Color( sal_uInt32 nVal ) : mnColor(nVal) {}
    Color( sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue ) :
        mnColor( (nRed << 16) | (nGreen << 8) | nBlue )
    {}

    void setRed( sal_uInt8 nRed ) { mnColor |= nRed << 16; mnColor &= ~((sal_uInt32)nRed << 16); }
    void setGreen( sal_uInt8 nGreen ) { mnColor |= nGreen << 8; mnColor &= ~((sal_uInt32)nGreen << 8); }
    void setBlue( sal_uInt8 nBlue ) { mnColor |= nBlue; mnColor &= ~(sal_uInt32)nBlue; }

    sal_uInt8 getRed() const   { return (mnColor & 0x00FF0000U) >> 16; }
    sal_uInt8 getGreen() const { return (mnColor & 0x0000FF00U) >> 8; }
    sal_uInt8 getBlue() const  { return mnColor & 0x000000FFU; }

    sal_uInt32 getValue() const { return mnColor; }
    operator sal_uInt32() const { return mnColor; }

    Color operator-( Color col ) const { return Color(getRed()-col.getRed(),
                                                      getGreen()-col.getGreen(),
                                                      getBlue()-col.getBlue()); }
    double magnitude() const { return sqrt(getRed()*getRed()
                                           + getGreen()*getGreen()
                                           + getBlue()*getBlue()); }
};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_COLOR_HXX */
