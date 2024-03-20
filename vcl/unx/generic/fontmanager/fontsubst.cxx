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

#include <unx/geninst.h>
#include <font/PhysicalFontCollection.hxx>
#include <font/fontsubstitution.hxx>
#include <unx/fontmanager.hxx>

// platform specific font substitution hooks

namespace {

class FcPreMatchSubstitution
:   public vcl::font::PreMatchFontSubstitution
{
public:
    bool FindFontSubstitute( vcl::font::FontSelectPattern& ) const override;
    typedef ::std::pair<vcl::font::FontSelectPattern, vcl::font::FontSelectPattern> value_type;
private:
    typedef ::std::list<value_type> CachedFontMapType;
    mutable CachedFontMapType maCachedFontMap;
};

class FcGlyphFallbackSubstitution
:    public vcl::font::GlyphFallbackFontSubstitution
{
    // TODO: add a cache
public:
    bool FindFontSubstitute(vcl::font::FontSelectPattern&, LogicalFontInstance* pLogicalFont, OUString& rMissingCodes) const override;
};

}

void SalGenericInstance::RegisterFontSubstitutors(vcl::font::PhysicalFontCollection* pFontCollection)
{
    // register font fallback substitutions
    static FcPreMatchSubstitution aSubstPreMatch;
    pFontCollection->SetPreMatchHook( &aSubstPreMatch );

    // register glyph fallback substitutions
    static FcGlyphFallbackSubstitution aSubstFallback;
    pFontCollection->SetFallbackHook( &aSubstFallback );
}

static vcl::font::FontSelectPattern GetFcSubstitute(const vcl::font::FontSelectPattern &rFontSelData, OUString& rMissingCodes)
{
    vcl::font::FontSelectPattern aSubstituted(rFontSelData);
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    rMgr.Substitute(aSubstituted, rMissingCodes);
    return aSubstituted;
}

namespace
{
    bool uselessmatch(const vcl::font::FontSelectPattern &rOrig, const vcl::font::FontSelectPattern &rNew)
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
        const vcl::font::FontSelectPattern& mrAttributes;
    public:
        explicit equal(const vcl::font::FontSelectPattern& rAttributes)
            : mrAttributes(rAttributes)
        {
        }
        bool operator()(const FcPreMatchSubstitution::value_type& rOther) const
            { return rOther.first == mrAttributes; }
    };
}

bool FcPreMatchSubstitution::FindFontSubstitute(vcl::font::FontSelectPattern &rFontSelData) const
{
    // We don't actually want to talk to Fontconfig at all for symbol fonts
    if( rFontSelData.IsMicrosoftSymbolEncoded() )
        return false;
    // OpenSymbol is a unicode font, but it still deserves to be treated as a symbol font
    if ( IsOpenSymbol(rFontSelData.maSearchName) )
        return false;

    //see fdo#41556 and fdo#47636
    //fontconfig can return e.g. an italic font for a non-italic input and/or
    //different fonts depending on fontsize, bold, etc settings so don't cache
    //just on the name, cache map all the input and all the output not just map
    //from original selection to output fontname
    vcl::font::FontSelectPattern& rPatternAttributes = rFontSelData;
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
    const vcl::font::FontSelectPattern aOut = GetFcSubstitute( rFontSelData, aDummy );

    if( aOut.maSearchName.isEmpty() )
        return false;

    const bool bHaveSubstitute = !uselessmatch( rFontSelData, aOut );

#if OSL_DEBUG_LEVEL >= 2
    std::ostringstream oss;
    oss << "FcPreMatchSubstitution \""
        << rFontSelData.maTargetName
        << "\" bipw="
        << rFontSelData.GetWeight()
        << rFontSelData.GetItalic()
        << rFontSelData.GetPitch()
        << rFontSelData.GetWidthType()
        << " -> ";
    if( !bHaveSubstitute )
        oss << "no substitute available.";
    else
        oss << "\""
            << aOut.maSearchName
            << "\" bipw="
            << aOut.GetWeight()
            << aOut.GetItalic()
            << aOut.GetPitch()
            << aOut.GetWidthType();
    SAL_INFO("vcl.fonts", oss.str());
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

bool FcGlyphFallbackSubstitution::FindFontSubstitute(vcl::font::FontSelectPattern& rFontSelData,
    LogicalFontInstance* /*pLogicalFont*/,
    OUString& rMissingCodes ) const
{
    // We don't actually want to talk to Fontconfig at all for symbol fonts
    if( rFontSelData.IsMicrosoftSymbolEncoded() )
        return false;
    // OpenSymbol is a unicode font, but it still deserves to be treated as a symbol font
    if ( IsOpenSymbol(rFontSelData.maSearchName) )
        return false;

    const vcl::font::FontSelectPattern aOut = GetFcSubstitute( rFontSelData, rMissingCodes );
    // TODO: cache the unicode + srcfont specific result
    // FC doing it would be preferable because it knows the invariables
    // e.g. FC knows the FC rule that all Arial gets replaced by LiberationSans
    // whereas we would have to check for every size or attribute
    if( aOut.maSearchName.isEmpty() )
        return false;

    const bool bHaveSubstitute = !uselessmatch( rFontSelData, aOut );

#if OSL_DEBUG_LEVEL >= 2
    std::ostringstream oss;
    oss << "FcGFSubstitution \""
        << rFontSelData.maTargetName
        << "\" bipw="
        << rFontSelData.GetWeight()
        << rFontSelData.GetItalic()
        << rFontSelData.GetPitch()
        << rFontSelData.GetWidthType()
        << " -> ";
    if( !bHaveSubstitute )
        oss << "no substitute available.";
    else
        oss << "\""
            << aOut.maSearchName
            << "\" bipw="
            << aOut.GetWeight()
            << aOut.GetItalic()
            << aOut.GetPitch()
            << aOut.GetWidthType();
    SAL_INFO("vcl.fonts", oss.str());
#endif

    if( bHaveSubstitute )
        rFontSelData = aOut;

    return bHaveSubstitute;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
