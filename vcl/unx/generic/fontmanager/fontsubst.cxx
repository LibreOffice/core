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

#include <unx/geninst.h>
#include <outdev.h>
#include <unx/fontmanager.hxx>
#include <PhysicalFontCollection.hxx>

// platform specific font substitution hooks

class FcPreMatchSubstitution
:   public ImplPreMatchFontSubstitution
{
public:
    bool FindFontSubstitute( FontSelectPattern& ) const override;
    typedef ::std::pair<FontSelectPattern, FontSelectPattern> value_type;
private:
    typedef ::std::list<value_type> CachedFontMapType;
    mutable CachedFontMapType maCachedFontMap;
};

class FcGlyphFallbackSubstitution
:    public ImplGlyphFallbackFontSubstitution
{
    // TODO: add a cache
public:
    bool FindFontSubstitute(FontSelectPattern&, LogicalFontInstance* pLogicalFont, OUString& rMissingCodes) const override;
};

void SalGenericInstance::RegisterFontSubstitutors( PhysicalFontCollection* pFontCollection )
{
    // register font fallback substitutions
    static FcPreMatchSubstitution aSubstPreMatch;
    pFontCollection->SetPreMatchHook( &aSubstPreMatch );

    // register glyph fallback substitutions
    static FcGlyphFallbackSubstitution aSubstFallback;
    pFontCollection->SetFallbackHook( &aSubstFallback );
}

static FontSelectPattern GetFcSubstitute(const FontSelectPattern &rFontSelData, OUString& rMissingCodes)
{
    FontSelectPattern aSubstituted(rFontSelData);
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    rMgr.Substitute(aSubstituted, rMissingCodes);
    return aSubstituted;
}

namespace
{
    bool uselessmatch(const FontSelectPattern &rOrig, const FontSelectPattern &rNew)
    {
        return
          (
            rOrig.maTargetName == rNew.maSearchName &&
            rOrig.GetWeight() == rNew.GetWeight() &&
            rOrig.GetItalic() == rNew.GetItalic() &&
            rOrig.GetPitch() == rNew.GetPitch() &&
            rOrig.GetWidthType() == rNew.GetWidthType()
          );
    }

    class equal
    {
    private:
        const FontSelectPattern& mrAttributes;
    public:
        explicit equal(const FontSelectPattern& rAttributes)
            : mrAttributes(rAttributes)
        {
        }
        bool operator()(const FcPreMatchSubstitution::value_type& rOther) const
            { return rOther.first == mrAttributes; }
    };
}

bool FcPreMatchSubstitution::FindFontSubstitute(FontSelectPattern &rFontSelData) const
{
    // We don't actually want to talk to Fontconfig at all for symbol fonts
    if( rFontSelData.IsSymbolFont() )
        return false;
    // StarSymbol is a unicode font, but it still deserves the symbol flag
    if ( IsStarSymbol(rFontSelData.maSearchName) )
        return false;

    //see fdo#41556 and fdo#47636
    //fontconfig can return e.g. an italic font for a non-italic input and/or
    //different fonts depending on fontsize, bold, etc settings so don't cache
    //just on the name, cache map all the input and all the output not just map
    //from original selection to output fontname
    FontSelectPattern& rPatternAttributes = rFontSelData;
    CachedFontMapType &rCachedFontMap = maCachedFontMap;
    CachedFontMapType::iterator itr = std::find_if(rCachedFontMap.begin(), rCachedFontMap.end(), equal(rPatternAttributes));
    if (itr != rCachedFontMap.end())
    {
        // Cached substitution
        rFontSelData = itr->second;
        if (itr != rCachedFontMap.begin())
        {
            // MRU, move it to the front
            rCachedFontMap.splice(rCachedFontMap.begin(), rCachedFontMap, itr);
        }
        return true;
    }

    OUString aDummy;
    const FontSelectPattern aOut = GetFcSubstitute( rFontSelData, aDummy );

    if( aOut.maSearchName.isEmpty() )
        return false;

    const bool bHaveSubstitute = !uselessmatch( rFontSelData, aOut );

#ifdef DEBUG
    const OString aOrigName(OUStringToOString(rFontSelData.maTargetName,
        RTL_TEXTENCODING_UTF8));
    const OString aSubstName(OUStringToOString(aOut.maSearchName,
        RTL_TEXTENCODING_UTF8));
    printf( "FcPreMatchSubstitution \"%s\" bipw=%d%d%d%d -> ",
        aOrigName.getStr(), rFontSelData.GetWeight(), rFontSelData.GetItalic(),
        rFontSelData.GetPitch(), rFontSelData.GetWidthType() );
    if( !bHaveSubstitute )
        printf( "no substitute available\n" );
    else
        printf( "\"%s\" bipw=%d%d%d%d\n", aSubstName.getStr(),
        aOut.GetWeight(), aOut.GetItalic(), aOut.GetPitch(), aOut.GetWidthType() );
#endif

    if( bHaveSubstitute )
    {
        rCachedFontMap.push_front(value_type(rFontSelData, aOut));
        // Fairly arbitrary limit in this case, but I recall measuring max 8
        // fonts as the typical max amount of fonts in medium sized documents, so make it
        // a fair chunk larger to accommodate weird documents./
        if (rCachedFontMap.size() > 256)
            rCachedFontMap.pop_back();
        rFontSelData = aOut;
    }

    return bHaveSubstitute;
}

bool FcGlyphFallbackSubstitution::FindFontSubstitute(FontSelectPattern& rFontSelData,
    LogicalFontInstance* /*pLogicalFont*/,
    OUString& rMissingCodes ) const
{
    // We don't actually want to talk to Fontconfig at all for symbol fonts
    if( rFontSelData.IsSymbolFont() )
        return false;
    // StarSymbol is a unicode font, but it still deserves the symbol flag
    if ( IsStarSymbol(rFontSelData.maSearchName) )
        return false;

    const FontSelectPattern aOut = GetFcSubstitute( rFontSelData, rMissingCodes );
    // TODO: cache the unicode + srcfont specific result
    // FC doing it would be preferable because it knows the invariables
    // e.g. FC knows the FC rule that all Arial gets replaced by LiberationSans
    // whereas we would have to check for every size or attribute
    if( aOut.maSearchName.isEmpty() )
        return false;

    const bool bHaveSubstitute = !uselessmatch( rFontSelData, aOut );

#ifdef DEBUG
    const OString aOrigName(OUStringToOString(rFontSelData.maTargetName,
        RTL_TEXTENCODING_UTF8));
    const OString aSubstName(OUStringToOString(aOut.maSearchName,
        RTL_TEXTENCODING_UTF8));
    printf( "FcGFSubstitution \"%s\" bipw=%d%d%d%d ->",
        aOrigName.getStr(), rFontSelData.GetWeight(), rFontSelData.GetItalic(),
        rFontSelData.GetPitch(), rFontSelData.GetWidthType() );
    if( !bHaveSubstitute )
        printf( "no substitute available\n" );
    else
        printf( "\"%s\" bipw=%d%d%d%d\n", aSubstName.getStr(),
        aOut.GetWeight(), aOut.GetItalic(), aOut.GetPitch(), aOut.GetWidthType() );
#endif

    if( bHaveSubstitute )
        rFontSelData = aOut;

    return bHaveSubstitute;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
