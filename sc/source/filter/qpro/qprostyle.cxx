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

#include <sal/config.h>
#include <sfx2/docfile.hxx>

#include "qproform.hxx"
#include "qpro.hxx"
#include "qprostyle.hxx"

#include <scitems.hxx>
#include <svx/algitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crossedoutitem.hxx>
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
#include "formulacell.hxx"

ScQProStyle::ScQProStyle()
{
    memset (maAlign, 0, sizeof (maAlign));
    memset (maFont, 0, sizeof (maFont));
    memset (maFontRecord, 0, sizeof (maFontRecord));
    memset (maFontHeight, 0, sizeof (maFontHeight));
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
        aWrapItem.SetValue( true );
        rItemSet.Put( aWrapItem );
    }

    // Font Attributes
    sal_uInt16 nTmpFnt = maFontRecord[ maFont[ nStyle ] ];
    bool bIsBold, bIsItalic, bIsUnderLine;

    bIsBold = ( nTmpFnt & 0x0001 ) != 0;
    bIsItalic = ( nTmpFnt & 0x0002 ) != 0;
    bIsUnderLine = ( nTmpFnt & 0x0004 ) != 0;
    //(nTmpFnt & 0x0020 ) for StrikeThrough

    if( bIsBold )
        rItemSet.Put( SvxWeightItem( WEIGHT_BOLD,ATTR_FONT_WEIGHT) );
    if( bIsItalic )
        rItemSet.Put( SvxPostureItem( ITALIC_NORMAL, ATTR_FONT_POSTURE ) );
    if( bIsUnderLine )
        rItemSet.Put( SvxUnderlineItem( LINESTYLE_SINGLE, ATTR_FONT_UNDERLINE ) );

    if (maFontHeight[ maFont [ nStyle ] ])
        rItemSet.Put( SvxFontHeightItem( (sal_uLong) (20 * maFontHeight[ maFont[ nStyle ] ] ), 100, ATTR_FONT_HEIGHT ) );

    OUString fntName = maFontType[ maFont[ nStyle ] ];
    rItemSet.Put( SvxFontItem( FAMILY_SYSTEM, fntName, EMPTY_OUSTRING, PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, ATTR_FONT ) );

    pDoc->ApplyPattern( nCol, nRow, nTab, aPattern );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
