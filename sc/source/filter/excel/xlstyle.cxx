/*************************************************************************
 *
 *  $RCSfile: xlstyle.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:04:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "filt_pch.hxx"
#endif
#pragma hdrstop

// ============================================================================

#ifndef SC_XLSTYLE_HXX
#include "xlstyle.hxx"
#endif


// Color data =================================================================

/** Default color table for BIFF2. */
static const ColorData pDefColorTable2[] = { 0x000000, 0xFFFFFF };

/** Default color table for BIFF3/BIFF4. */
static const ColorData pDefColorTable3[] =
{
/*  8 */    0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF,
/* 16 */    0x800000, 0x008000, 0x000080, 0x808000, 0x800080, 0x008080, 0xC0C0C0, 0x808080
};

/** Default color table for BIFF5/BIFF7. */
static const ColorData pDefColorTable5[] =
{
/*  8 */    0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF,
/* 16 */    0x800000, 0x008000, 0x000080, 0x808000, 0x800080, 0x008080, 0xC0C0C0, 0x808080,
/* 24 */    0x8080FF, 0x802060, 0xFFFFC0, 0xA0E0E0, 0x600080, 0xFF8080, 0x0080C0, 0xC0C0FF,
/* 32 */    0x000080, 0xFF00FF, 0xFFFF00, 0x00FFFF, 0x800080, 0x800000, 0x008080, 0x0000FF,
/* 40 */    0x00CFFF, 0x69FFFF, 0xE0FFE0, 0xFFFF80, 0xA6CAF0, 0xDD9CB3, 0xB38FEE, 0xE3E3E3,
/* 48 */    0x2A6FF9, 0x3FB8CD, 0x488436, 0x958C41, 0x8E5E42, 0xA0627A, 0x624FAC, 0x969696,
/* 56 */    0x1D2FBE, 0x286676, 0x004500, 0x453E01, 0x6A2813, 0x85396A, 0x4A3285, 0x424242
};

/** Default color table for BIFF8. */
static const ColorData pDefColorTable8[] =
{
/*  8 */    0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF,
/* 16 */    0x800000, 0x008000, 0x000080, 0x808000, 0x800080, 0x008080, 0xC0C0C0, 0x808080,
/* 24 */    0x9999FF, 0x993366, 0xFFFFCC, 0xCCFFFF, 0x660066, 0xFF8080, 0x0066CC, 0xCCCCFF,
/* 32 */    0x000080, 0xFF00FF, 0xFFFF00, 0x00FFFF, 0x800080, 0x800000, 0x008080, 0x0000FF,
/* 40 */    0x00CCFF, 0xCCFFFF, 0xCCFFCC, 0xFFFF99, 0x99CCFF, 0xFF99CC, 0xCC99FF, 0xFFCC99,
/* 48 */    0x3366FF, 0x33CCCC, 0x99CC00, 0xFFCC00, 0xFF9900, 0xFF6600, 0x666699, 0x969696,
/* 56 */    0x003366, 0x339966, 0x003300, 0x333300, 0x993300, 0x993366, 0x333399, 0x333333
};


// ----------------------------------------------------------------------------

XclDefaultPalette::XclDefaultPalette( XclBiff eBiff ) :
    mpColorTable( NULL ),
    mnTableSize( 0 ),
    mnIndexOffset( 0 )
{
    if( eBiff != xlBiffUnknown )
        SetDefaultColors( eBiff );
}

void XclDefaultPalette::SetDefaultColors( XclBiff eBiff )
{
    switch( eBiff )
    {
        case xlBiff2:
            mpColorTable = pDefColorTable2;
            mnTableSize = STATIC_TABLE_SIZE( pDefColorTable2 );
            mnIndexOffset = EXC_COLOR_OFFSET2;
        break;
        case xlBiff3:
        case xlBiff4:
            mpColorTable = pDefColorTable3;
            mnTableSize = STATIC_TABLE_SIZE( pDefColorTable3 );
            mnIndexOffset = EXC_COLOR_OFFSET3;
        break;
        case xlBiff5:
        case xlBiff7:
            mpColorTable = pDefColorTable5;
            mnTableSize = STATIC_TABLE_SIZE( pDefColorTable5 );
            mnIndexOffset = EXC_COLOR_OFFSET5;
        break;
        case xlBiff8:
            mpColorTable = pDefColorTable8;
            mnTableSize = STATIC_TABLE_SIZE( pDefColorTable8 );
            mnIndexOffset = EXC_COLOR_OFFSET8;
        break;
        default:
            DBG_ERROR_BIFF();
    }
}

ColorData XclDefaultPalette::GetDefColorData( sal_uInt16 nXclIndex, ColorData nDefault ) const
{
    return ((mnIndexOffset <= nXclIndex) && (nXclIndex < mnIndexOffset + mnTableSize)) ?
        mpColorTable[ nXclIndex - mnIndexOffset ] : nDefault;
}


// Font Data ==================================================================

void XclFontData::Clear()
{
    maName.Erase();
    maStyle.Erase();
    meUnderline = xlUnderlNone;
    meEscapem = xlEscNone;
    mnHeight = 0;
    mnColor = EXC_FONT_AUTOCOLOR;
    mnWeight = EXC_FONTWGHT_DONTKNOW;
    mnFamily = EXC_FONTFAM_SYSTEM;
    mnCharSet = EXC_FONTCSET_DONTKNOW;
    mbItalic = mbStrikeout = mbOutline = mbShadow = false;
}


// ============================================================================

