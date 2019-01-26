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

#include <lotfntbf.hxx>

#include <scitems.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <osl/diagnose.h>
#include <svl/itemset.hxx>

#include <global.hxx>

void LotusFontBuffer::Fill( const sal_uInt8 nIndex, SfxItemSet& rItemSet )
{
    sal_uInt8   nIntIndex = nIndex & 0x07;

    ENTRY*  pCurrent = pData + nIntIndex;

    if( pCurrent->pFont )
        rItemSet.Put( *pCurrent->pFont );

    if( pCurrent->pHeight )
        rItemSet.Put( *pCurrent->pHeight );

    if( nIndex & 0x08 )
    {
        SvxWeightItem aWeightItem( WEIGHT_BOLD, ATTR_FONT_WEIGHT );
        rItemSet.Put( aWeightItem );
    }

    if( nIndex & 0x10 )
    {
        SvxPostureItem aAttr( ITALIC_NORMAL, ATTR_FONT_POSTURE );
        rItemSet.Put( aAttr );
    }

    FontLineStyle eUnderline;
    switch( nIndex & 0x60 ) // Bit 5+6
    {
        case 0x60:
        case 0x20:  eUnderline = LINESTYLE_SINGLE;      break;
        case 0x40:  eUnderline = LINESTYLE_DOUBLE;      break;
        default:    eUnderline = LINESTYLE_NONE;
    }
    if( eUnderline != LINESTYLE_NONE )
    {
        SvxUnderlineItem aUndItem( eUnderline, ATTR_FONT_UNDERLINE );
        rItemSet.Put( aUndItem );
    }
}

void LotusFontBuffer::SetName( const sal_uInt16 nIndex, const OUString& rName )
{
    OSL_ENSURE( nIndex < nSize, "*LotusFontBuffer::SetName(): Array too small!" );
    if( nIndex < nSize )
    {
        ENTRY* pEntry = pData + nIndex;
        pEntry->TmpName( rName );

        if( pEntry->nType >= 0 )
            MakeFont( pEntry );
    }
}

void LotusFontBuffer::SetHeight( const sal_uInt16 nIndex, const sal_uInt16 nHeight )
{
    OSL_ENSURE( nIndex < nSize, "*LotusFontBuffer::SetHeight(): Array too small!" );
    if( nIndex < nSize )
        pData[ nIndex ].Height( std::make_unique<SvxFontHeightItem>( static_cast<sal_uLong>(nHeight) * 20, 100, ATTR_FONT_HEIGHT ) );
}

void LotusFontBuffer::SetType( const sal_uInt16 nIndex, const sal_uInt16 nType )
{
    OSL_ENSURE( nIndex < nSize, "*LotusFontBuffer::SetType(): Array too small!" );
    if( nIndex < nSize )
    {
        ENTRY* pEntry = pData + nIndex;
        pEntry->Type( nType );

        if( pEntry->xTmpName )
            MakeFont( pEntry );
    }
}

void LotusFontBuffer::MakeFont( ENTRY* pEntry )
{
    FontFamily      eFamily = FAMILY_DONTKNOW;
    FontPitch       ePitch = PITCH_DONTKNOW;
    rtl_TextEncoding eCharSet = RTL_TEXTENCODING_DONTKNOW;

    switch( pEntry->nType )
    {
        case 0x00:                      // Helvetica
            eFamily = FAMILY_SWISS;
            ePitch  = PITCH_VARIABLE;
            break;
        case 0x01:                      // Times Roman
            eFamily = FAMILY_ROMAN;
            ePitch  = PITCH_VARIABLE;
            break;
        case 0x02:                      // Courier
            ePitch  = PITCH_FIXED;
            break;
        case 0x03:                      // Symbol
            eCharSet = RTL_TEXTENCODING_SYMBOL;
            break;
    }

    pEntry->pFont.reset( new SvxFontItem( eFamily, *pEntry->xTmpName, EMPTY_OUSTRING, ePitch, eCharSet, ATTR_FONT ) );

    pEntry->xTmpName.reset();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
