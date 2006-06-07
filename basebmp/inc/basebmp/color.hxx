/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: color.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hdu $ $Date: 2006-06-07 09:21:55 $
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
#include <vigra/mathutil.hxx>
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

    sal_uInt8 getRed() const   { return 0xFF & (mnColor >> 16); }
    sal_uInt8 getGreen() const { return 0xFF & (mnColor >> 8); }
    sal_uInt8 getBlue() const  { return 0xFF & mnColor; }

    sal_uInt32 getValue() const { return mnColor; }
    operator sal_uInt32() const { return mnColor; }

    Color operator-( Color col ) const { return Color(vigra::abs((int)getRed()-col.getRed()),
                                                      vigra::abs((int)getGreen()-col.getGreen()),
                                                      vigra::abs((int)getBlue()-col.getBlue())); }
    Color operator*( sal_uInt8 n ) const { return Color(((sal_uInt32)n*getRed())/255,
                                                        ((sal_uInt32)n*getGreen())/255,
                                                        ((sal_uInt32)n*getBlue())/255); }
    Color operator*( double n ) const { return Color((sal_uInt8)(n*getRed()+.5),
                                                     (sal_uInt8)(n*getGreen()+.5),
                                                     (sal_uInt8)(n*getBlue()+.5)); }
    double magnitude() const { return sqrt(getRed()*getRed()
                                           + getGreen()*getGreen()
                                           + getBlue()*getBlue()); }
};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_COLOR_HXX */
