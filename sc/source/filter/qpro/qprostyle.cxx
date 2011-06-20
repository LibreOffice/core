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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


#include <sal/config.h>
#include <stdio.h>
#include <sfx2/docfile.hxx>

#include "qproform.hxx"
#include "qpro.hxx"
#include "qprostyle.hxx"

#include <tools/color.hxx>
#include <scitems.hxx>
#include <svx/algitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/justifyitem.hxx>
#include <map>

#include "global.hxx"
#include "scerrors.hxx"
#include "docpool.hxx"
#include "patattr.hxx"
#include "filter.hxx"
#include "document.hxx"
#include "cell.hxx"

ScQProStyle::ScQProStyle()
{
    rtl_fillMemory (maAlign, sizeof (maAlign), 0);
    rtl_fillMemory (maFont, sizeof (maFont), 0);
    rtl_fillMemory (maFontRecord, sizeof (maFontRecord), 0);
    rtl_fillMemory (maFontHeight, sizeof (maFontHeight), 0);
}

void ScQProStyle::SetFormat( ScDocument *pDoc, sal_uInt8 nCol, sal_uInt16 nRow, SCTAB nTab, sal_uInt16 nStyle )
{
    if (nStyle >= maxsize)
        return;

    ScPatternAttr aPattern(pDoc->GetPool());
    SfxItemSet& rItemSet = aPattern.GetItemSet();

    sal_uInt8 nTmp = maAlign[ nStyle ];
    sal_uInt8 nHor = ( nTmp & 0x07 );
    sal_uInt8 nVer = ( nTmp & 0x18 );
    sal_uInt8 nOrient = ( nTmp & 0x60 );

    // Horizontal Alignment
    SvxCellHorJustify eJustify = SVX_HOR_JUSTIFY_STANDARD;
    switch( nHor )
    {
        case 0x00:
            eJustify = SVX_HOR_JUSTIFY_STANDARD;
            break;

        case 0x01:
            eJustify = SVX_HOR_JUSTIFY_LEFT;
            break;

        case 0x02:
            eJustify = SVX_HOR_JUSTIFY_CENTER;
            break;

        case 0x03:
            eJustify = SVX_HOR_JUSTIFY_RIGHT;
            break;

        case 0x04:
            eJustify = SVX_HOR_JUSTIFY_BLOCK;
            break;
    }
    rItemSet.Put( SvxHorJustifyItem( eJustify, ATTR_HOR_JUSTIFY ) );

    // Vertical Alignment
    SvxCellVerJustify eVerJustify = SVX_VER_JUSTIFY_STANDARD;
    switch( nVer )
    {
        case 0x00:
            eVerJustify = SVX_VER_JUSTIFY_BOTTOM;
            break;

        case 0x08:
            eVerJustify = SVX_VER_JUSTIFY_CENTER;
            break;

        case 0x10:
            eVerJustify = SVX_VER_JUSTIFY_TOP;
            break;
    }

    rItemSet.Put(SvxVerJustifyItem( eVerJustify, ATTR_VER_JUSTIFY ) );

    // Orientation
    SvxCellOrientation eOrient = SVX_ORIENTATION_STANDARD;
    switch( nOrient )
    {
        case 0x20:
            eOrient = SVX_ORIENTATION_TOPBOTTOM;
            break;

    }
    rItemSet.Put( SvxOrientationItem( eOrient, 0) );

    // Wrap cell contents
    if( nTmp & 0x80 )
    {
        SfxBoolItem aWrapItem( ATTR_LINEBREAK );
        aWrapItem.SetValue( sal_True );
        rItemSet.Put( aWrapItem );
    }

    // Font Attributes
    sal_uInt16 nTmpFnt = maFontRecord[ maFont[ nStyle ] ];
    sal_Bool bIsBold, bIsItalic, bIsUnderLine;

    bIsBold = ( nTmpFnt & 0x0001 ) != 0;
    bIsItalic = ( nTmpFnt & 0x0002 ) != 0;
    bIsUnderLine = ( nTmpFnt & 0x0004 ) != 0;
    //(nTmpFnt & 0x0020 ) for StrikeThrough

    if( bIsBold )
        rItemSet.Put( SvxWeightItem( WEIGHT_BOLD,ATTR_FONT_WEIGHT) );
    if( bIsItalic )
        rItemSet.Put( SvxPostureItem( ITALIC_NORMAL, ATTR_FONT_POSTURE ) );
    if( bIsUnderLine )
        rItemSet.Put( SvxUnderlineItem( UNDERLINE_SINGLE, ATTR_FONT_UNDERLINE ) );

    if (maFontHeight[ maFont [ nStyle ] ])
        rItemSet.Put( SvxFontHeightItem( (sal_uLong) (20 * maFontHeight[ maFont[ nStyle ] ] ), 100, ATTR_FONT_HEIGHT ) );

    String fntName = maFontType[ maFont[ nStyle ] ];
    rItemSet.Put( SvxFontItem( FAMILY_SYSTEM, fntName, EMPTY_STRING, PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, ATTR_FONT ) );

    pDoc->ApplyPattern( nCol, nRow, nTab, aPattern );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
