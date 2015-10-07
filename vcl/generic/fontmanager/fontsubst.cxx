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

#include "generic/geninst.h"
#include "generic/genpspgraphics.h"
#include "generic/glyphcache.hxx"

#include "vcl/sysdata.hxx"
#include "outfont.hxx"

#include "generic/printergfx.hxx"
#include "salbmp.hxx"
#include "impfont.hxx"
#include "outdev.h"
#include "PhysicalFontCollection.hxx"
#include "fontsubset.hxx"
#include "salprn.hxx"

#include <unotools/fontdefs.hxx>
#include <list>

// platform specific font substitution hooks

class FcPreMatchSubstititution
:   public ImplPreMatchFontSubstitution
{
public:
    bool FindFontSubstitute( FontSelectPattern& ) const SAL_OVERRIDE;
    typedef ::std::pair<FontSelectPatternAttributes, FontSelectPatternAttributes> value_type;
private:
    typedef ::std::list<value_type> CachedFontMapType;
    mutable CachedFontMapType maCachedFontMap;
};

class FcGlyphFallbackSubstititution
:    public ImplGlyphFallbackFontSubstitution
{
    // TODO: add a cache
public:
    bool FindFontSubstitute( FontSelectPattern&, OUString& rMissingCodes ) const SAL_OVERRIDE;
};

int SalGenericInstance::FetchFontSubstitutionFlags()
{
    // init font substitution defaults
    int nDisableBits = 0;
#ifdef SOLARIS
    nDisableBits = 1; // disable "font fallback" here on default
#endif
    // apply the environment variable if any
    const char* pEnvStr = ::getenv( "SAL_DISABLE_FC_SUBST" );
    if( pEnvStr )
    {
        if( (*pEnvStr >= '0') && (*pEnvStr <= '9') )
            nDisableBits = (*pEnvStr - '0');
        else
            nDisableBits = ~0U; // no specific bits set: disable all
    }
    return nDisableBits;
}

void SalGenericInstance::RegisterFontSubstitutors( PhysicalFontCollection* pFontCollection )
{
    // init font substitution defaults
    int nDisableBits = 0;
#ifdef SOLARIS
    nDisableBits = 1; // disable "font fallback" here on default
#endif
    // apply the environment variable if any
    const char* pEnvStr = ::getenv( "SAL_DISABLE_FC_SUBST" );
    if( pEnvStr )
    {
        if( (*pEnvStr >= '0') && (*pEnvStr <= '9') )
            nDisableBits = (*pEnvStr - '0');
        else
            nDisableBits = ~0U; // no specific bits set: disable all
    }

    // register font fallback substitutions (unless disabled by bit0)
    if( (nDisableBits & 1) == 0 )
    {
        static FcPreMatchSubstititution aSubstPreMatch;
        pFontCollection->SetPreMatchHook( &aSubstPreMatch );
    }

    // register glyph fallback substitutions (unless disabled by bit1)
    if( (nDisableBits & 2) == 0 )
    {
        static FcGlyphFallbackSubstititution aSubstFallback;
        pFontCollection->SetFallbackHook( &aSubstFallback );
    }
}

static FontSelectPattern GetFcSubstitute(const FontSelectPattern &rFontSelData, OUString& rMissingCodes )
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
            rOrig.GetSlant() == rNew.GetSlant() &&
            rOrig.GetPitch() == rNew.GetPitch() &&
            rOrig.GetWidthType() == rNew.GetWidthType()
          );
    }

    class equal
    {
    private:
        const FontSelectPatternAttributes& mrAttributes;
    public:
        explicit equal(const FontSelectPatternAttributes& rAttributes)
            : mrAttributes(rAttributes)
        {
        }
        bool operator()(const FcPreMatchSubstititution::value_type& rOther) const
            { return rOther.first == mrAttributes; }
    };
}

bool FcPreMatchSubstititution::FindFontSubstitute( FontSelectPattern &rFontSelData ) const
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
    FontSelectPatternAttributes& rPatternAttributes = rFontSelData;
    CachedFontMapType &rCachedFontMap = const_cast<CachedFontMapType &>(maCachedFontMap);
    CachedFontMapType::iterator itr = std::find_if(rCachedFontMap.begin(), rCachedFontMap.end(), equal(rPatternAttributes));
    if (itr != rCachedFontMap.end())
    {
        // Cached substitution
        rFontSelData.copyAttributes(itr->second);
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
    printf( "FcPreMatchSubstititution \"%s\" bipw=%d%d%d%d -> ",
        aOrigName.getStr(), rFontSelData.GetWeight(), rFontSelData.GetSlant(),
        rFontSelData.GetPitch(), rFontSelData.GetWidthType() );
    if( !bHaveSubstitute )
        printf( "no substitute available\n" );
    else
        printf( "\"%s\" bipw=%d%d%d%d\n", aSubstName.getStr(),
        aOut.GetWeight(), aOut.GetSlant(), aOut.GetPitch(), aOut.GetWidthType() );
#endif

    if( bHaveSubstitute )
    {
        rCachedFontMap.push_front(value_type(rFontSelData, aOut));
        //fairly arbitrary limit in this case, but I recall measuring max 8
        //fonts as the typical max amount of fonts in medium sized documents
        if (rCachedFontMap.size() > 8)
            rCachedFontMap.pop_back();
        rFontSelData = aOut;
    }

    return bHaveSubstitute;
}

bool FcGlyphFallbackSubstititution::FindFontSubstitute( FontSelectPattern& rFontSelData,
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
    printf( "FcGFSubstititution \"%s\" bipw=%d%d%d%d ->",
        aOrigName.getStr(), rFontSelData.GetWeight(), rFontSelData.GetSlant(),
        rFontSelData.GetPitch(), rFontSelData.GetWidthType() );
    if( !bHaveSubstitute )
        printf( "no substitute available\n" );
    else
        printf( "\"%s\" bipw=%d%d%d%d\n", aSubstName.getStr(),
        aOut.GetWeight(), aOut.GetSlant(), aOut.GetPitch(), aOut.GetWidthType() );
#endif

    if( bHaveSubstitute )
        rFontSelData = aOut;

    return bHaveSubstitute;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
