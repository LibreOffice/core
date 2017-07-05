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

#include <rtl/ustring.hxx>
#include <unotools/fontdefs.hxx>

#include "outdev.h"
#include "PhysicalFontCollection.hxx"

void PhysicalFontFamily::CalcType( ImplFontAttrs& rType, FontWeight& rWeight, FontWidth& rWidth,
                                   FontFamily eFamily, const utl::FontNameAttr* pFontAttr )
{
    if ( eFamily != FAMILY_DONTKNOW )
    {
        if ( eFamily == FAMILY_SWISS )
            rType |= ImplFontAttrs::SansSerif;
        else if ( eFamily == FAMILY_ROMAN )
            rType |= ImplFontAttrs::Serif;
        else if ( eFamily == FAMILY_SCRIPT )
            rType |= ImplFontAttrs::Script;
        else if ( eFamily == FAMILY_MODERN )
            rType |= ImplFontAttrs::Fixed;
        else if ( eFamily == FAMILY_DECORATIVE )
            rType |= ImplFontAttrs::Decorative;
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

static ImplFontAttrs lcl_IsCJKFont( const OUString& rFontName )
{
    // Test, if Fontname includes CJK characters --> In this case we
    // mention that it is a CJK font
    for(int i = 0; i < rFontName.getLength(); i++)
    {
        const sal_Unicode ch = rFontName[i];
        // japanese
        if ( ((ch >= 0x3040) && (ch <= 0x30FF)) ||
             ((ch >= 0x3190) && (ch <= 0x319F)) )
            return ImplFontAttrs::CJK|ImplFontAttrs::CJK_JP;

        // korean
        if ( ((ch >= 0xAC00) && (ch <= 0xD7AF)) ||
             ((ch >= 0x3130) && (ch <= 0x318F)) ||
             ((ch >= 0x1100) && (ch <= 0x11FF)) )
            return ImplFontAttrs::CJK|ImplFontAttrs::CJK_KR;

        // chinese
        if ( (ch >= 0x3400) && (ch <= 0x9FFF) )
            return ImplFontAttrs::CJK|ImplFontAttrs::CJK_TC|ImplFontAttrs::CJK_SC;

        // cjk
        if ( ((ch >= 0x3000) && (ch <= 0xD7AF)) ||
             ((ch >= 0xFF00) && (ch <= 0xFFEE)) )
            return ImplFontAttrs::CJK;

    }

    return ImplFontAttrs::None;
}

PhysicalFontFamily::PhysicalFontFamily( const OUString& rSearchName )
:   maSearchName( rSearchName ),
    mnTypeFaces( FontTypeFaces::NONE ),
    meFamily( FAMILY_DONTKNOW ),
    mePitch( PITCH_DONTKNOW ),
    mnMinQuality( -1 ),
    mnMatchType( ImplFontAttrs::None ),
    meMatchWeight( WEIGHT_DONTKNOW ),
    meMatchWidth( WIDTH_DONTKNOW )
{}

PhysicalFontFamily::~PhysicalFontFamily()
{
    // release all physical font faces
    for( std::vector< PhysicalFontFace* >::iterator it=maFontFaces.begin(); it != maFontFaces.end(); )
    {
        delete *it;
        it = maFontFaces.erase( it );
    }

}

bool PhysicalFontFamily::AddFontFace( PhysicalFontFace* pNewFontFace )
{
    if( maFontFaces.empty() )
    {
        maFamilyName   = pNewFontFace->GetFamilyName();
        maMapNames     = pNewFontFace->GetMapNames();
        meFamily       = pNewFontFace->GetFamilyType();
        mePitch        = pNewFontFace->GetPitch();
        mnMinQuality   = pNewFontFace->GetQuality();
    }
    else
    {
        if( meFamily == FAMILY_DONTKNOW )
            meFamily = pNewFontFace->GetFamilyType();
        if( mePitch == PITCH_DONTKNOW )
            mePitch = pNewFontFace->GetPitch();
        if( mnMinQuality > pNewFontFace->GetQuality() )
            mnMinQuality = pNewFontFace->GetQuality();
    }

    // set attributes for attribute based font matching
    mnTypeFaces |= FontTypeFaces::Scalable;

    if( pNewFontFace->IsSymbolFont() )
        mnTypeFaces |= FontTypeFaces::Symbol;
    else
        mnTypeFaces |= FontTypeFaces::NoneSymbol;

    if( pNewFontFace->GetWeight() != WEIGHT_DONTKNOW )
    {
        if( pNewFontFace->GetWeight() >= WEIGHT_SEMIBOLD )
            mnTypeFaces |= FontTypeFaces::Bold;
        else if( pNewFontFace->GetWeight() <= WEIGHT_SEMILIGHT )
            mnTypeFaces |= FontTypeFaces::Light;
        else
            mnTypeFaces |= FontTypeFaces::Normal;
    }

    if( pNewFontFace->GetItalic() == ITALIC_NONE )
        mnTypeFaces |= FontTypeFaces::NoneItalic;
    else if( (pNewFontFace->GetItalic() == ITALIC_NORMAL)
         ||  (pNewFontFace->GetItalic() == ITALIC_OBLIQUE) )
        mnTypeFaces |= FontTypeFaces::Italic;

    // reassign name (sharing saves memory)
    if( pNewFontFace->GetFamilyName() == GetFamilyName() )
        pNewFontFace->SetFamilyName( GetFamilyName() );

    // add the new physical font face, replacing existing font face if necessary
    // TODO: get rid of linear search?
    auto it(maFontFaces.begin());
    for (; it != maFontFaces.end(); ++it)
    {
        PhysicalFontFace* pFoundFontFace = *it;
        sal_Int32 eComp = pNewFontFace->CompareWithSize( *pFoundFontFace );
        if( eComp > 0 )
            continue;
        if( eComp < 0 )
            break;

        // ignore duplicate if its quality is worse
        if( pNewFontFace->GetQuality() < pFoundFontFace->GetQuality() )
            return false;

        // keep the device font if its quality is good enough
        if( pNewFontFace->GetQuality() == pFoundFontFace->GetQuality() )
            return false;

        // replace existing font face with a better one
        delete pFoundFontFace;
        *it = pNewFontFace; // insert at sort position
        return true;
    }

    maFontFaces.insert(it, pNewFontFace); // insert at sort position
    return true;
}

// get font attributes using the normalized font family name
void PhysicalFontFamily::InitMatchData( const utl::FontSubstConfiguration& rFontSubst,
    const OUString& rSearchName )
{
    OUString aShortName;
    OUString aMatchFamilyName(maMatchFamilyName);
    // get font attributes from the decorated font name
    utl::FontSubstConfiguration::getMapName( rSearchName, aShortName, aMatchFamilyName,
                            meMatchWeight, meMatchWidth, mnMatchType );
    maMatchFamilyName = aMatchFamilyName;
    const utl::FontNameAttr* pFontAttr = rFontSubst.getSubstInfo( rSearchName );
    // eventually use the stripped name
    if( !pFontAttr )
        if( aShortName != rSearchName )
            pFontAttr = rFontSubst.getSubstInfo( aShortName );
    CalcType( mnMatchType, meMatchWeight, meMatchWidth, meFamily, pFontAttr );
    mnMatchType |= lcl_IsCJKFont( maFamilyName );
}

PhysicalFontFace* PhysicalFontFamily::FindBestFontFace( const FontSelectPattern& rFSD ) const
{
    if( maFontFaces.empty() )
        return nullptr;
    if( maFontFaces.size() == 1)
        return maFontFaces[0];

    // FontName+StyleName should map to FamilyName+StyleName
    const OUString& rSearchName = rFSD.maTargetName;
    OUString aTargetStyleName;
    const OUString* pTargetStyleName = nullptr;
    if((rSearchName.getLength() > maSearchName.getLength())
        && rSearchName.startsWith( maSearchName ) )
    {
        aTargetStyleName = rSearchName.copy(maSearchName.getLength() + 1);
        pTargetStyleName = &aTargetStyleName;
    }

    // TODO: linear search improve!
    PhysicalFontFace* pBestFontFace = maFontFaces[0];
    FontMatchStatus aFontMatchStatus = {0,0,0, pTargetStyleName};
    for( std::vector< PhysicalFontFace* >::const_iterator it=maFontFaces.begin(); it != maFontFaces.end(); ++it )
    {
        PhysicalFontFace* pFoundFontFace = *it;
        if( pFoundFontFace->IsBetterMatch( rFSD, aFontMatchStatus ) )
            pBestFontFace = pFoundFontFace;
    }

    return pBestFontFace;
}

// update device font list with unique font faces, with uniqueness
// meaning different font attributes, but not different fonts sizes
void PhysicalFontFamily::UpdateDevFontList( ImplDeviceFontList& rDevFontList ) const
{
    PhysicalFontFace* pPrevFace = nullptr;
    for(std::vector< PhysicalFontFace* >::const_iterator it=maFontFaces.begin(); it != maFontFaces.end(); ++it )
    {
        PhysicalFontFace* pFoundFontFace = *it;
        if( !pPrevFace || pFoundFontFace->CompareIgnoreSize( *pPrevFace ) )
            rDevFontList.Add( pFoundFontFace );
        pPrevFace = pFoundFontFace;
    }
}

void PhysicalFontFamily::GetFontHeights( std::set<int>& rHeights ) const
{
    // add all available font heights
    for( std::vector< PhysicalFontFace* >::const_iterator it=maFontFaces.begin(); it != maFontFaces.end(); ++it )
    {
        PhysicalFontFace *pFoundFontFace = *it;
        rHeights.insert( pFoundFontFace->GetHeight() );
    }
}

void PhysicalFontFamily::UpdateCloneFontList(PhysicalFontCollection& rFontCollection) const
{
    OUString aFamilyName = GetEnglishSearchFontName( GetFamilyName() );
    PhysicalFontFamily* pFamily(nullptr);

    for( std::vector< PhysicalFontFace* >::const_iterator it=maFontFaces.begin(); it != maFontFaces.end(); ++it )
    {
        PhysicalFontFace *pFoundFontFace = *it;

        if (!pFamily)
        {   // tdf#98989 lazy create as family without faces won't work
            pFamily = rFontCollection.FindOrCreateFontFamily(aFamilyName);
        }
        assert(pFamily);
        PhysicalFontFace* pClonedFace = pFoundFontFace->Clone();

#if OSL_DEBUG_LEVEL > 0
        OUString aClonedFamilyName = GetEnglishSearchFontName( pClonedFace->GetFamilyName() );
        assert( aClonedFamilyName == aFamilyName );
        assert( rFontCollection.FindOrCreateFontFamily( aClonedFamilyName ) == pFamily );
#endif

        if (! pFamily->AddFontFace( pClonedFace ) )
            delete pClonedFace;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
