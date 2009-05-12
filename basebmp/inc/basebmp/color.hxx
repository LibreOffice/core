/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: color.hxx,v $
 * $Revision: 1.17 $
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

#ifndef INCLUDED_BASEBMP_COLOR_HXX
#define INCLUDED_BASEBMP_COLOR_HXX

#include <sal/types.h>
#include <rtl/math.hxx>

namespace basebmp
{

class Color
{
private:
    sal_uInt32          mnColor;

public:
    typedef sal_uInt32  value_type;
    typedef sal_uInt8   component_type;

    Color() : mnColor(0) {}
    explicit Color( sal_uInt32 nVal ) : mnColor(nVal) {}
    Color( sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue ) :
        mnColor( ((sal_uInt32)nRed << 16) | ((sal_uInt32)nGreen << 8) | nBlue )
    {}

    void setRed( sal_uInt8 nRed ) { mnColor &= ~0x00FF0000UL; mnColor |= (sal_uInt32)nRed << 16; }
    void setGreen( sal_uInt8 nGreen ) { mnColor &= ~0x0000FF00UL; mnColor |= (sal_uInt32)nGreen << 8; }
    void setBlue( sal_uInt8 nBlue ) { mnColor &= ~0x000000FFUL; mnColor |= nBlue; }

    void setGrey( sal_uInt8 nGreyVal ) { mnColor = (sal_uInt32)nGreyVal << 16 | (sal_uInt32)nGreyVal << 8 | nGreyVal; }

    sal_uInt8 getRed() const   { return 0xFF & (sal_uInt8)(mnColor >> 16); }
    sal_uInt8 getGreen() const { return 0xFF & (sal_uInt8)(mnColor >> 8); }
    sal_uInt8 getBlue() const  { return 0xFF & (sal_uInt8)mnColor; }

    sal_uInt8 getGreyscale() const { return (sal_uInt8)((getBlue()*28UL +
                                                         getGreen()*151 +
                                                         getRed()*77) / 256); }

    sal_uInt32 toInt32() const { return mnColor; }

    bool  operator!() const { return mnColor == 0; }
    Color operator&( sal_uInt32 nMask ) const { return Color(mnColor & nMask); }
    Color operator^( Color col ) const { return Color(col.getRed()^getRed(),
                                                      col.getGreen()^getGreen(),
                                                      col.getBlue()^getBlue()); }
    Color operator-( Color col ) const { return Color((sal_uInt8)abs((int)getRed()-col.getRed()),
                                                      (sal_uInt8)abs((int)getGreen()-col.getGreen()),
                                                      (sal_uInt8)abs((int)getBlue()-col.getBlue())); }
    Color operator+( Color col ) const { return Color(getRed()+col.getRed(),
                                                      getGreen()+col.getGreen(),
                                                      getBlue()+col.getBlue()); }
    Color operator*( Color col ) const { return Color((sal_uInt8)((sal_uInt32)col.getRed()*getRed()/SAL_MAX_UINT8),
                                                      (sal_uInt8)((sal_uInt32)col.getGreen()*getGreen()/SAL_MAX_UINT8),
                                                      (sal_uInt8)((sal_uInt32)col.getBlue()*getBlue()/SAL_MAX_UINT8)); }
    Color operator*( sal_uInt8 n ) const { return Color((sal_uInt8)((sal_uInt32)n*getRed()/SAL_MAX_UINT8),
                                                        (sal_uInt8)((sal_uInt32)n*getGreen()/SAL_MAX_UINT8),
                                                        (sal_uInt8)((sal_uInt32)n*getBlue()/SAL_MAX_UINT8)); }
    Color operator*( double n ) const { return Color((sal_uInt8)(n*getRed()+.5),
                                                     (sal_uInt8)(n*getGreen()+.5),
                                                     (sal_uInt8)(n*getBlue()+.5)); }
    bool operator==( const Color& rhs ) const { return (getRed()==rhs.getRed() &&
                                                        getGreen()==rhs.getGreen() &&
                                                        getBlue()==rhs.getBlue()); }
    bool operator!=( const Color& rhs ) const { return !(*this==rhs); }
    double magnitude() const { return sqrt((double)getRed()*getRed()
                                           + getGreen()*getGreen()
                                           + getBlue()*getBlue()); }
};

} // namespace vigra

#endif /* INCLUDED_BASEBMP_COLOR_HXX */
