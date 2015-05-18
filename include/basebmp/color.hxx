/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

}

#endif /* INCLUDED_BASEBMP_COLOR_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
