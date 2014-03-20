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

#include <sal/types.h>

#include <rtl/ustring.hxx>

#include "outdev.h"
#include "PhysicalFontFace.hxx"
#include "PhysicalFontCollection.hxx"

#include "PhysicalFontFamily.hxx"

void PhysicalFontFamily::CalcType( sal_uLong& rType, FontWeight& rWeight, FontWidth& rWidth,
                                   FontFamily eFamily, const utl::FontNameAttr* pFontAttr )
{
    if ( eFamily != FAMILY_DONTKNOW )
    {
        if ( eFamily == FAMILY_SWISS )
            rType |= IMPL_FONT_ATTR_SANSSERIF;
        else if ( eFamily == FAMILY_ROMAN )
            rType |= IMPL_FONT_ATTR_SERIF;
        else if ( eFamily == FAMILY_SCRIPT )
            rType |= IMPL_FONT_ATTR_SCRIPT;
        else if ( eFamily == FAMILY_MODERN )
            rType |= IMPL_FONT_ATTR_FIXED;
        else if ( eFamily == FAMILY_DECORATIVE )
            rType |= IMPL_FONT_ATTR_DECORATIVE;
    }

    if ( pFontAttr )
    {
        rType |= pFontAttr->Type;

        if ( ((rWeight == WEIGHT_DONTKNOW) || (rWeight == WEIGHT_NORMAL)) &&
             (pFontAttr->Weight != WEIGHT_DONTKNOW) )
            rWeight = pFontAttr->Weight;
        if ( ((rWidth == WIDTH_DONTKNOW) || (rWidth == WIDTH_NORMAL)) &&
             (pFontAttr->Width != WIDTH_DONTKNOW) )
            rWidth = pFontAttr->Width;
    }
}

static unsigned lcl_IsCJKFont( const OUString& rFontName )
{
    // Test, if Fontname includes CJK characters --> In this case we
    // mention that it is a CJK font
    for(int i = 0; i < rFontName.getLength(); i++)
    {
        const sal_Unicode ch = rFontName[i];
        // japanese
        if ( ((ch >= 0x3040) && (ch <= 0x30FF)) ||
             ((ch >= 0x3190) && (ch <= 0x319F)) )
            return IMPL_FONT_ATTR_CJK|IMPL_FONT_ATTR_CJK_JP;

        // korean
        if ( ((ch >= 0xAC00) && (ch <= 0xD7AF)) ||
             ((ch >= 0x3130) && (ch <= 0x318F)) ||
             ((ch >= 0x1100) && (ch <= 0x11FF)) )
            return IMPL_FONT_ATTR_CJK|IMPL_FONT_ATTR_CJK_KR;

        // chinese
        if ( ((ch >= 0x3400) && (ch <= 0x9FFF)) )
            return IMPL_FONT_ATTR_CJK|IMPL_FONT_ATTR_CJK_TC|IMPL_FONT_ATTR_CJK_SC;

        // cjk
        if ( ((ch >= 0x3000) && (ch <= 0xD7AF)) ||
             ((ch >= 0xFF00) && (ch <= 0xFFEE)) )
            return IMPL_FONT_ATTR_CJK;

    }

    return 0;
}


PhysicalFontFamily::PhysicalFontFamily( const OUString& rSearchName )
:   mpFirst( NULL ),
    maSearchName( rSearchName ),
    mnTypeFaces( 0 ),
    mnMatchType( 0 ),
    meMatchWeight( WEIGHT_DONTKNOW ),
    meMatchWidth( WIDTH_DONTKNOW ),
    meFamily( FAMILY_DONTKNOW ),
    mePitch( PITCH_DONTKNOW ),
    mnMinQuality( -1 )
{}

PhysicalFontFamily::~PhysicalFontFamily()
{
    // release all physical font faces
    while( mpFirst )
    {
        PhysicalFontFace* pFace = mpFirst;
        mpFirst = pFace->GetNextFace();
        delete pFace;
    }
}

bool PhysicalFontFamily::AddFontFace( PhysicalFontFace* pNewData )
{
    pNewData->mpNext = NULL;

    if( !mpFirst )
    {
        maName         = pNewData->GetFamilyName();
        maMapNames     = pNewData->maMapNames;
        meFamily       = pNewData->GetFamilyType();
        mePitch        = pNewData->GetPitch();
        mnMinQuality   = pNewData->mnQuality;
    }
    else
    {
        if( meFamily == FAMILY_DONTKNOW )
            meFamily = pNewData->GetFamilyType();
        if( mePitch == PITCH_DONTKNOW )
            mePitch = pNewData->GetPitch();
        if( mnMinQuality > pNewData->mnQuality )
            mnMinQuality = pNewData->mnQuality;
    }

    // set attributes for attribute based font matching
    if( pNewData->IsScalable() )
        mnTypeFaces |= FONT_FAMILY_SCALABLE;

    if( pNewData->IsSymbolFont() )
        mnTypeFaces |= FONT_FAMILY_SYMBOL;
    else
        mnTypeFaces |= FONT_FAMILY_NONESYMBOL;

    if( pNewData->GetWeight() != WEIGHT_DONTKNOW )
    {
        if( pNewData->GetWeight() >= WEIGHT_SEMIBOLD )
            mnTypeFaces |= FONT_FAMILY_BOLD;
        else if( pNewData->GetWeight() <= WEIGHT_SEMILIGHT )
            mnTypeFaces |= FONT_FAMILY_LIGHT;
        else
            mnTypeFaces |= FONT_FAMILY_NORMAL;
    }

    if( pNewData->GetSlant() == ITALIC_NONE )
        mnTypeFaces |= FONT_FAMILY_NONEITALIC;
    else if( (pNewData->GetSlant() == ITALIC_NORMAL)
         ||  (pNewData->GetSlant() == ITALIC_OBLIQUE) )
        mnTypeFaces |= FONT_FAMILY_ITALIC;

    if( (meMatchWeight == WEIGHT_DONTKNOW)
    ||  (meMatchWidth  == WIDTH_DONTKNOW)
    ||  (mnMatchType   == 0) )
    {
        // TODO: is it cheaper to calc matching attributes now or on demand?
        // calc matching attributes if other entries are already initialized

        // Do lazy, quite expensive, not needed in start-up!
        // const FontSubstConfiguration& rFontSubst = *FontSubstConfiguration::get();
        // InitMatchData( rFontSubst, maSearchName );
        // mbMatchData=true; // Somewhere else???
    }

    // reassign name (sharing saves memory)
    if( pNewData->GetFamilyName() == GetFamilyName() )
        pNewData->SetFamilyName( GetFamilyName() );

    // insert new physical font face into linked list
    // TODO: get rid of linear search?
    PhysicalFontFace* pData;
    PhysicalFontFace** ppHere = &mpFirst;
    for(; (pData=*ppHere) != NULL; ppHere=&pData->mpNext )
    {
        sal_Int32 eComp = pNewData->CompareWithSize( *pData );
        if( eComp > 0 )
            continue;
        if( eComp < 0 )
            break;

        // ignore duplicate if its quality is worse
        if( pNewData->mnQuality < pData->mnQuality )
            return false;

        // keep the device font if its quality is good enough
        if( (pNewData->mnQuality == pData->mnQuality)
        &&  (pData->mbDevice || !pNewData->mbDevice) )
            return false;

        // replace existing font face with a better one
        pNewData->mpNext = pData->mpNext;
        *ppHere = pNewData;
        delete pData;
        return true;
    }

    // insert into or append to list of physical font faces
    pNewData->mpNext = pData;
    *ppHere = pNewData;
    return true;
}

// get font attributes using the normalized font family name
void PhysicalFontFamily::InitMatchData( const utl::FontSubstConfiguration& rFontSubst,
    const OUString& rSearchName )
{
    OUString aShortName;
    OUString aMatchFamilyName(maMatchFamilyName);
    // get font attributes from the decorated font name
    rFontSubst.getMapName( rSearchName, aShortName, aMatchFamilyName,
                            meMatchWeight, meMatchWidth, mnMatchType );
    maMatchFamilyName = aMatchFamilyName;
    const utl::FontNameAttr* pFontAttr = rFontSubst.getSubstInfo( rSearchName );
    // eventually use the stripped name
    if( !pFontAttr )
        if( aShortName != rSearchName )
            pFontAttr = rFontSubst.getSubstInfo( aShortName );
    CalcType( mnMatchType, meMatchWeight, meMatchWidth, meFamily, pFontAttr );
    mnMatchType |= lcl_IsCJKFont( maName );
}

PhysicalFontFace* PhysicalFontFamily::FindBestFontFace( const FontSelectPattern& rFSD ) const
{
    if( !mpFirst )
        return NULL;
    if( !mpFirst->GetNextFace() )
        return mpFirst;

    // FontName+StyleName should map to FamilyName+StyleName
    const OUString& rSearchName = rFSD.maTargetName;
    OUString aTargetStyleName;
    const OUString* pTargetStyleName = NULL;
    if(    (rSearchName.getLength() > maSearchName.getLength())
        && rSearchName.startsWith( maSearchName ) )
    {
        aTargetStyleName = rSearchName.copy(maSearchName.getLength() + 1);
        pTargetStyleName = &aTargetStyleName;
    }

    // TODO: linear search improve!
    PhysicalFontFace* pFontFace = mpFirst;
    PhysicalFontFace* pBestFontFace = pFontFace;
    FontMatchStatus aFontMatchStatus = {0,0,0, pTargetStyleName};
    for(; pFontFace; pFontFace = pFontFace->GetNextFace() )
        if( pFontFace->IsBetterMatch( rFSD, aFontMatchStatus ) )
            pBestFontFace = pFontFace;

    return pBestFontFace;
}

// update device font list with unique font faces, with uniqueness
// meaning different font attributes, but not different fonts sizes
void PhysicalFontFamily::UpdateDevFontList( ImplGetDevFontList& rDevFontList ) const
{
    PhysicalFontFace* pPrevFace = NULL;
    for( PhysicalFontFace* pFace = mpFirst; pFace; pFace = pFace->GetNextFace() )
    {
        if( !pPrevFace || pFace->CompareIgnoreSize( *pPrevFace ) )
            rDevFontList.Add( pFace );
        pPrevFace = pFace;
    }
}

void PhysicalFontFamily::GetFontHeights( std::set<int>& rHeights ) const
{
    // add all available font heights
    for( const PhysicalFontFace* pFace = mpFirst; pFace; pFace = pFace->GetNextFace() )
        rHeights.insert( pFace->GetHeight() );
}

void PhysicalFontFamily::UpdateCloneFontList( PhysicalFontCollection& rFontCollection,
                                              bool bScalable, bool bEmbeddable ) const
{
    for( PhysicalFontFace* pFace = mpFirst; pFace; pFace = pFace->GetNextFace() )
    {
        if( bScalable && !pFace->IsScalable() )
            continue;
        if( bEmbeddable && !pFace->IsEmbeddable() && !pFace->IsSubsettable() )
            continue;

        PhysicalFontFace* pClonedFace = pFace->Clone();
        rFontCollection.Add( pClonedFace );
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
