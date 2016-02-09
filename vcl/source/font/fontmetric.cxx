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

#include <i18nlangtag/mslangid.hxx>
#include <vcl/fontcharmap.hxx>
#include <vcl/metric.hxx>

#include "impfontmetric.hxx"
#include "impfontmetricdata.hxx"
#include "PhysicalFontFace.hxx"

#include <vector>
#include <set>
#include <cstdio>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
using namespace ::utl;

FontMetric::FontMetric()
:   mpImplMetric( new ImplFontMetric() )
{}

FontMetric::FontMetric( const FontMetric& rFontMetric )
    : Font( rFontMetric )
    , mpImplMetric( rFontMetric.mpImplMetric )
{}

FontMetric::~FontMetric()
{
    mpImplMetric = nullptr;
}

FontMetric& FontMetric::operator=( const FontMetric& rFontMetric )
{
    Font::operator=( rFontMetric );

    if( mpImplMetric != rFontMetric.mpImplMetric )
    {
        mpImplMetric = rFontMetric.mpImplMetric;
    }

    return *this;
}

bool FontMetric::operator==( const FontMetric& rFontMetric ) const
{
    if( !Font::operator==( rFontMetric ) )
        return false;
    if( mpImplMetric == rFontMetric.mpImplMetric )
        return true;
    if( *mpImplMetric == *rFontMetric.mpImplMetric  )
        return true;
    return false;
}

FontType FontMetric::GetType() const
{
    return (mpImplMetric->IsScalable() ? TYPE_SCALABLE : TYPE_RASTER);
}

long FontMetric::GetAscent() const
{
    return mpImplMetric->GetAscent();
}

void FontMetric::SetAscent( long nAscent )
{
    mpImplMetric->SetAscent( nAscent );
}

long FontMetric::GetDescent() const
{
    return mpImplMetric->GetDescent();
}

void FontMetric::SetDescent( long nDescent )
{
    mpImplMetric->SetDescent( nDescent );
}

long FontMetric::GetInternalLeading() const
{
    return mpImplMetric->GetInternalLeading();
}

void FontMetric::SetInternalLeading( long nLeading )
{
    mpImplMetric->SetInternalLeading( nLeading );
}

long FontMetric::GetExternalLeading() const
{
    return mpImplMetric->GetExternalLeading();
}

void FontMetric::SetExternalLeading( long nLeading )
{
    mpImplMetric->SetExternalLeading( nLeading );
}

long FontMetric::GetLineHeight() const
{
    return mpImplMetric->GetLineHeight();
}

void FontMetric::SetLineHeight( long nHeight )
{
    mpImplMetric->SetLineHeight( nHeight );
}

long FontMetric::GetSlant() const
{
    return mpImplMetric->GetSlant();
}

void FontMetric::SetSlant( long nSlant )
{
    mpImplMetric->SetSlant( nSlant );
}

long FontMetric::GetBulletOffset() const
{
    return mpImplMetric->GetBulletOffset();
}

void FontMetric::SetBulletOffset( long nOffset )
{
    mpImplMetric->SetBulletOffset( nOffset );
}

bool FontMetric::IsScalable() const
{
    return mpImplMetric->IsScalable();
}

void FontMetric::SetScalableFlag(bool bScalable)
{
    mpImplMetric->SetScalableFlag( bScalable );
}

bool FontMetric::IsFullstopCentered() const
{
    return mpImplMetric->IsFullstopCentered();
}

void FontMetric::SetFullstopCenteredFlag(bool bScalable)
{
    mpImplMetric->SetFullstopCenteredFlag( bScalable );
}

bool FontMetric::IsBuiltInFont() const
{
    return mpImplMetric->IsBuiltInFont();
}

void FontMetric::SetBuiltInFontFlag( bool bIsBuiltInFont )
{
    mpImplMetric->SetBuiltInFontFlag( bIsBuiltInFont );
}


ImplFontMetric::ImplFontMetric()
:   mnAscent( 0 ),
    mnDescent( 0 ),
    mnIntLeading( 0 ),
    mnExtLeading( 0 ),
    mnLineHeight( 0 ),
    mnSlant( 0 ),
    mnBulletOffset( 0 ),
    mnRefCount( 0 ),
    mbScalableFont( false ),
    mbFullstopCentered( false ),
    mbDevice( false )
{}

bool ImplFontMetric::operator==( const ImplFontMetric& r ) const
{
    if(    mbScalableFont     != r.mbScalableFont
        || mbFullstopCentered != r.mbFullstopCentered
        || mbDevice           != r.mbDevice) // mbDevice == built-in font flag
        return false;
    if( mnAscent     != r.mnAscent )
        return false;
    if( mnDescent    != r.mnDescent )
        return false;
    if( mnIntLeading != r.mnIntLeading )
        return false;
    if( mnExtLeading != r.mnExtLeading )
        return false;
    if( mnSlant      != r.mnSlant )
        return false;

    return true;
}

ImplFontMetricData::ImplFontMetricData( const FontSelectPattern& rFontSelData )
    : FontAttributes( rFontSelData )
    , mnRefCount ( 0 )
    , mnWidth ( rFontSelData.mnWidth )
    , mnOrientation( (short)(rFontSelData.mnOrientation) )
    , mnAscent( 0 )
    , mnDescent( 0 )
    , mnIntLeading( 0 )
    , mnExtLeading( 0 )
    , mnSlant( 0 )
    , mnMinKashida( 0 )
    , mbScalableFont( false )
    , mbTrueTypeFont( false )
    , mbKernableFont( false )
    , mbFullstopCentered( false )
    , mnBulletOffset( 0 )
    , mnUnderlineSize( 0 )
    , mnUnderlineOffset( 0 )
    , mnBUnderlineSize( 0 )
    , mnBUnderlineOffset( 0 )
    , mnDUnderlineSize( 0 )
    , mnDUnderlineOffset1( 0 )
    , mnDUnderlineOffset2( 0 )
    , mnWUnderlineSize( 0 )
    , mnWUnderlineOffset( 0 )
    , mnAboveUnderlineSize( 0 )
    , mnAboveUnderlineOffset( 0 )
    , mnAboveBUnderlineSize( 0 )
    , mnAboveBUnderlineOffset( 0 )
    , mnAboveDUnderlineSize( 0 )
    , mnAboveDUnderlineOffset1( 0 )
    , mnAboveDUnderlineOffset2( 0 )
    , mnAboveWUnderlineSize( 0 )
    , mnAboveWUnderlineOffset( 0 )
    , mnStrikeoutSize( 0 )
    , mnStrikeoutOffset( 0 )
    , mnBStrikeoutSize( 0 )
    , mnBStrikeoutOffset( 0 )
    , mnDStrikeoutSize( 0 )
    , mnDStrikeoutOffset1( 0 )
    , mnDStrikeoutOffset2( 0 )
{
    // intialize the used font name
    if( rFontSelData.mpFontData )
    {
        SetFamilyName( rFontSelData.mpFontData->GetFamilyName() );
        SetStyleName( rFontSelData.mpFontData->GetStyleName() );
        SetBuiltInFontFlag( rFontSelData.mpFontData->IsBuiltInFont() );
        SetKernableFlag( true );
    }
    else
    {
        sal_Int32 nTokenPos = 0;
        SetFamilyName( GetNextFontToken( rFontSelData.GetFamilyName(), nTokenPos ) );
        SetStyleName( rFontSelData.GetStyleName() );
        SetBuiltInFontFlag( false );
        SetKernableFlag( false );
    }
}


void ImplFontMetricData::ImplInitTextLineSize( const OutputDevice* pDev )
{
    long nDescent = mnDescent;
    if ( nDescent <= 0 )
    {
        nDescent = mnAscent / 10;
        if ( !nDescent )
            nDescent = 1;
    }

    // #i55341# for some fonts it is not a good idea to calculate
    // their text line metrics from the real font descent
    // => work around this problem just for these fonts
    if( 3*nDescent > mnAscent )
        nDescent = mnAscent / 3;

    long nLineHeight = ((nDescent*25)+50) / 100;
    if ( !nLineHeight )
        nLineHeight = 1;
    long nLineHeight2 = nLineHeight / 2;
    if ( !nLineHeight2 )
        nLineHeight2 = 1;

    long nBLineHeight = ((nDescent*50)+50) / 100;
    if ( nBLineHeight == nLineHeight )
        nBLineHeight++;
    long nBLineHeight2 = nBLineHeight/2;
    if ( !nBLineHeight2 )
        nBLineHeight2 = 1;

    long n2LineHeight = ((nDescent*16)+50) / 100;
    if ( !n2LineHeight )
        n2LineHeight = 1;
    long n2LineDY = n2LineHeight;
     /* #117909#
      * add some pixels to minimum double line distance on higher resolution devices
      */
    long nMin2LineDY = 1 + pDev->GetDPIY()/150;
    if ( n2LineDY < nMin2LineDY )
        n2LineDY = nMin2LineDY;
    long n2LineDY2 = n2LineDY/2;
    if ( !n2LineDY2 )
        n2LineDY2 = 1;

    long nUnderlineOffset = mnDescent/2 + 1;
    long nStrikeoutOffset = -((mnAscent - mnIntLeading) / 3);

    mnUnderlineSize        = nLineHeight;
    mnUnderlineOffset      = nUnderlineOffset - nLineHeight2;

    mnBUnderlineSize       = nBLineHeight;
    mnBUnderlineOffset     = nUnderlineOffset - nBLineHeight2;

    mnDUnderlineSize       = n2LineHeight;
    mnDUnderlineOffset1    = nUnderlineOffset - n2LineDY2 - n2LineHeight;
    mnDUnderlineOffset2    = mnDUnderlineOffset1 + n2LineDY + n2LineHeight;

    long nWCalcSize = mnDescent;
    if ( nWCalcSize < 6 )
    {
        if ( (nWCalcSize == 1) || (nWCalcSize == 2) )
            mnWUnderlineSize = nWCalcSize;
        else
            mnWUnderlineSize = 3;
    }
    else
        mnWUnderlineSize = ((nWCalcSize*50)+50) / 100;

    // Don't assume that wavelines are never placed below the descent, because for most fonts the waveline
    // is drawn into the text
    mnWUnderlineOffset     = nUnderlineOffset;

    mnStrikeoutSize        = nLineHeight;
    mnStrikeoutOffset      = nStrikeoutOffset - nLineHeight2;

    mnBStrikeoutSize       = nBLineHeight;
    mnBStrikeoutOffset     = nStrikeoutOffset - nBLineHeight2;

    mnDStrikeoutSize       = n2LineHeight;
    mnDStrikeoutOffset1    = nStrikeoutOffset - n2LineDY2 - n2LineHeight;
    mnDStrikeoutOffset2    = mnDStrikeoutOffset1 + n2LineDY + n2LineHeight;

    const vcl::Font& rFont ( pDev->GetFont() );
    bool bCentered = true;
    if (MsLangId::isCJK(rFont.GetLanguage()))
    {
        const OUString sFullstop( sal_Unicode( 0x3001 ) ); // Fullwidth fullstop
        Rectangle aRect;
        pDev->GetTextBoundRect( aRect, sFullstop );
        const sal_uInt16 nH = rFont.GetFontSize().Height();
        const sal_uInt16 nB = aRect.Left();
        // Use 18.75% as a threshold to define a centered fullwidth fullstop.
        // In general, nB/nH < 5% for most Japanese fonts.
        bCentered = nB > (((nH >> 1)+nH)>>3);
    }
    SetFullstopCenteredFlag( bCentered );

    mnBulletOffset = ( pDev->GetTextWidth( OUString( sal_Unicode( 0x20 ) ) ) - pDev->GetTextWidth( OUString( sal_Unicode( 0xb7 ) ) ) ) >> 1 ;

}


void ImplFontMetricData::ImplInitAboveTextLineSize()
{
    long nIntLeading = mnIntLeading;
    // TODO: assess usage of nLeading below (changed in extleading CWS)
    // if no leading is available, we assume 15% of the ascent
    if ( nIntLeading <= 0 )
    {
        nIntLeading = mnAscent*15/100;
        if ( !nIntLeading )
            nIntLeading = 1;
    }

    long nLineHeight = ((nIntLeading*25)+50) / 100;
    if ( !nLineHeight )
        nLineHeight = 1;

    long nBLineHeight = ((nIntLeading*50)+50) / 100;
    if ( nBLineHeight == nLineHeight )
        nBLineHeight++;

    long n2LineHeight = ((nIntLeading*16)+50) / 100;
    if ( !n2LineHeight )
        n2LineHeight = 1;

    long nCeiling = -mnAscent;

    mnAboveUnderlineSize       = nLineHeight;
    mnAboveUnderlineOffset     = nCeiling + (nIntLeading - nLineHeight + 1) / 2;

    mnAboveBUnderlineSize      = nBLineHeight;
    mnAboveBUnderlineOffset    = nCeiling + (nIntLeading - nBLineHeight + 1) / 2;

    mnAboveDUnderlineSize      = n2LineHeight;
    mnAboveDUnderlineOffset1   = nCeiling + (nIntLeading - 3*n2LineHeight + 1) / 2;
    mnAboveDUnderlineOffset2   = nCeiling + (nIntLeading +   n2LineHeight + 1) / 2;

    long nWCalcSize = nIntLeading;
    if ( nWCalcSize < 6 )
    {
        if ( (nWCalcSize == 1) || (nWCalcSize == 2) )
            mnAboveWUnderlineSize = nWCalcSize;
        else
            mnAboveWUnderlineSize = 3;
    }
    else
        mnAboveWUnderlineSize = ((nWCalcSize*50)+50) / 100;

    mnAboveWUnderlineOffset = nCeiling + (nIntLeading + 1) / 2;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
