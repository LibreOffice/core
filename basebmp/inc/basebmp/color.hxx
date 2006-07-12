/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: color.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: thb $ $Date: 2006-07-12 15:09:44 $
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

    sal_uInt8 getRed() const   { return 0xFF & (mnColor >> 16); }
    sal_uInt8 getGreen() const { return 0xFF & (mnColor >> 8); }
    sal_uInt8 getBlue() const  { return 0xFF & mnColor; }

    sal_uInt8 getGreyscale() const { return (sal_uInt8)((getBlue()*28UL +
                                                         getGreen()*151 +
                                                         getRed()*77) / 256); }

    sal_uInt32 toInt32() const { return mnColor; }

    bool  operator!() const { return mnColor == 0; }
    Color operator&( sal_uInt32 nMask ) const { return Color(mnColor & nMask); }
    Color operator^( Color col ) const { return Color(col.getRed()^getRed(),
                                                      col.getGreen()^getGreen(),
                                                      col.getBlue()^getBlue()); }
    Color operator-( Color col ) const { return Color(abs((int)getRed()-col.getRed()),
                                                      abs((int)getGreen()-col.getGreen()),
                                                      abs((int)getBlue()-col.getBlue())); }
    Color operator+( Color col ) const { return Color(getRed()+col.getRed(),
                                                      getGreen()+col.getGreen(),
                                                      getBlue()+col.getBlue()); }
    Color operator*( Color col ) const { return Color((sal_uInt32)col.getRed()*getRed()/SAL_MAX_UINT8,
                                                      (sal_uInt32)col.getGreen()*getGreen()/SAL_MAX_UINT8,
                                                      (sal_uInt32)col.getBlue()*getBlue()/SAL_MAX_UINT8); }
    Color operator*( sal_uInt8 n ) const { return Color((sal_uInt32)n*getRed()/SAL_MAX_UINT8,
                                                        (sal_uInt32)n*getGreen()/SAL_MAX_UINT8,
                                                        (sal_uInt32)n*getBlue()/SAL_MAX_UINT8); }
    Color operator*( double n ) const { return Color((sal_uInt8)(n*getRed()+.5),
                                                     (sal_uInt8)(n*getGreen()+.5),
                                                     (sal_uInt8)(n*getBlue()+.5)); }
    bool operator==( const Color& rhs ) const { return (getRed()==rhs.getRed() &&
                                                        getGreen()==rhs.getGreen() &&
                                                        getBlue()==rhs.getBlue()); }
    bool operator!=( const Color& rhs ) const { return !(*this==rhs); }
    double magnitude() const { return sqrt(getRed()*getRed()
                                           + getGreen()*getGreen()
                                           + getBlue()*getBlue()); }
};

} // namespace vigra

#endif /* INCLUDED_BASEBMP_COLOR_HXX */
