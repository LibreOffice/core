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

#include "generic/geninst.h"
#include "generic/genpspgraphics.h"
#include "generic/glyphcache.hxx"

#include "vcl/sysdata.hxx"
#include "outfont.hxx"

#include "generic/printergfx.hxx"
#include "salbmp.hxx"
#include "impfont.hxx"
#include "outfont.hxx"
#include "outdev.h"
#include "fontsubset.hxx"
#include "salprn.hxx"
#include "region.h"

#include <list>

// ===========================================================================
// platform specific font substitution hooks
// ===========================================================================

struct FontSelectPatternAttributesHash
{
    size_t operator()(const FontSelectPatternAttributes& rAttributes) const
        { return rAttributes.hashCode(); }
};

class FcPreMatchSubstititution
:   public ImplPreMatchFontSubstitution
{
public:
    bool FindFontSubstitute( FontSelectPattern& ) const;
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
    bool FindFontSubstitute( FontSelectPattern&, rtl::OUString& rMissingCodes ) const;
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

void SalGenericInstance::RegisterFontSubstitutors( ImplDevFontList* pList )
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
        pList->SetPreMatchHook( &aSubstPreMatch );
    }

    // register glyph fallback substitutions (unless disabled by bit1)
    if( (nDisableBits & 2) == 0 )
    {
        static FcGlyphFallbackSubstititution aSubstFallback;
        pList->SetFallbackHook( &aSubstFallback );
    }
}

// -----------------------------------------------------------------------

static FontSelectPattern GetFcSubstitute(const FontSelectPattern &rFontSelData, rtl::OUString& rMissingCodes )
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
            rOrig.meWeight == rNew.meWeight &&
            rOrig.meItalic == rNew.meItalic &&
            rOrig.mePitch == rNew.mePitch &&
            rOrig.meWidthType == rNew.meWidthType
          );
    }

    class equal
    {
    private:
        const FontSelectPatternAttributes& mrAttributes;
    public:
        equal(const FontSelectPatternAttributes& rAttributes)
            : mrAttributes(rAttributes)
        {
        }
        bool operator()(const FcPreMatchSubstititution::value_type& rOther) const
            { return rOther.first == mrAttributes; }
    };
}

//--------------------------------------------------------------------------

bool FcPreMatchSubstititution::FindFontSubstitute( FontSelectPattern &rFontSelData ) const
{
    // We dont' actually want to talk to Fontconfig at all for symbol fonts
    if( rFontSelData.IsSymbolFont() )
        return false;
    // StarSymbol is a unicode font, but it still deserves the symbol flag
    if( 0 == rFontSelData.maSearchName.CompareIgnoreCaseToAscii( "starsymbol", 10)
    ||  0 == rFontSelData.maSearchName.CompareIgnoreCaseToAscii( "opensymbol", 10) )
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

    rtl::OUString aDummy;
    const FontSelectPattern aOut = GetFcSubstitute( rFontSelData, aDummy );

    if( !aOut.maSearchName.Len() )
        return false;

    const bool bHaveSubstitute = !uselessmatch( rFontSelData, aOut );

#ifdef DEBUG
    const rtl::OString aOrigName(rtl::OUStringToOString(rFontSelData.maTargetName,
        RTL_TEXTENCODING_UTF8));
    const rtl::OString aSubstName(rtl::OUStringToOString(aOut.maSearchName,
        RTL_TEXTENCODING_UTF8));
    printf( "FcPreMatchSubstititution \"%s\" bipw=%d%d%d%d -> ",
        aOrigName.getStr(), rFontSelData.meWeight, rFontSelData.meItalic,
        rFontSelData.mePitch, rFontSelData.meWidthType );
    if( !bHaveSubstitute )
        printf( "no substitute available\n" );
    else
        printf( "\"%s\" bipw=%d%d%d%d\n", aSubstName.getStr(),
        aOut.meWeight, aOut.meItalic, aOut.mePitch, aOut.meWidthType );
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

// -----------------------------------------------------------------------

bool FcGlyphFallbackSubstititution::FindFontSubstitute( FontSelectPattern& rFontSelData,
    rtl::OUString& rMissingCodes ) const
{
    // We dont' actually want to talk to Fontconfig at all for symbol fonts
    if( rFontSelData.IsSymbolFont() )
        return false;
    // StarSymbol is a unicode font, but it still deserves the symbol flag
    if( 0 == rFontSelData.maSearchName.CompareIgnoreCaseToAscii( "starsymbol", 10)
    ||  0 == rFontSelData.maSearchName.CompareIgnoreCaseToAscii( "opensymbol", 10) )
        return false;

    const FontSelectPattern aOut = GetFcSubstitute( rFontSelData, rMissingCodes );
    // TODO: cache the unicode + srcfont specific result
    // FC doing it would be preferable because it knows the invariables
    // e.g. FC knows the FC rule that all Arial gets replaced by LiberationSans
    // whereas we would have to check for every size or attribute
    if( !aOut.maSearchName.Len() )
        return false;

    const bool bHaveSubstitute = !uselessmatch( rFontSelData, aOut );

#ifdef DEBUG
    const rtl::OString aOrigName(rtl::OUStringToOString(rFontSelData.maTargetName,
        RTL_TEXTENCODING_UTF8));
    const rtl::OString aSubstName(rtl::OUStringToOString(aOut.maSearchName,
        RTL_TEXTENCODING_UTF8));
    printf( "FcGFSubstititution \"%s\" bipw=%d%d%d%d ->",
        aOrigName.getStr(), rFontSelData.meWeight, rFontSelData.meItalic,
        rFontSelData.mePitch, rFontSelData.meWidthType );
    if( !bHaveSubstitute )
        printf( "no substitute available\n" );
    else
        printf( "\"%s\" bipw=%d%d%d%d\n", aSubstName.getStr(),
        aOut.meWeight, aOut.meItalic, aOut.mePitch, aOut.meWidthType );
#endif

    if( bHaveSubstitute )
        rFontSelData = aOut;

    return bHaveSubstitute;
}

// ===========================================================================


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
