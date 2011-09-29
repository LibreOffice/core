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
#include "precompiled_vcl.hxx"
#include "generic/geninst.h"
#include "generic/pspgraphics.h"
#include "generic/glyphcache.hxx"

#include "vcl/sysdata.hxx"
#include "outfont.hxx"
#include <i18npool/mslangid.hxx>

#include "generic/printergfx.hxx"
#include "salbmp.hxx"
#include "impfont.hxx"
#include "outfont.hxx"
#include "outdev.h"
#include "fontsubset.hxx"
#include "salprn.hxx"
#include "region.h"

// ===========================================================================
// platform specific font substitution hooks
// ===========================================================================

class FcPreMatchSubstititution
:   public ImplPreMatchFontSubstitution
{
public:
    bool FindFontSubstitute( ImplFontSelectData& ) const;
};

class FcGlyphFallbackSubstititution
:    public ImplGlyphFallbackFontSubstitution
{
    // TODO: add a cache
public:
    bool FindFontSubstitute( ImplFontSelectData&, OUString& rMissingCodes ) const;
};

void RegisterFontSubstitutors( ImplDevFontList* pList )
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

static ImplFontSelectData GetFcSubstitute(const ImplFontSelectData &rFontSelData, OUString& rMissingCodes )
{
    ImplFontSelectData aRet(rFontSelData);

    const rtl::OString aLangAttrib = MsLangId::convertLanguageToIsoByteString( rFontSelData.meLanguage );

    FontItalic eItalic = rFontSelData.GetSlant();
    FontWeight eWeight = rFontSelData.GetWeight();
    FontWidth eWidth = rFontSelData.GetWidthType();
    FontPitch ePitch = rFontSelData.GetPitch();

    const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    aRet.maSearchName = rMgr.Substitute( rFontSelData.maTargetName, rMissingCodes, aLangAttrib, eItalic, eWeight, eWidth, ePitch);

    aRet.meItalic    = eItalic;
    aRet.meWeight    = eWeight;
    aRet.meWidthType = eWidth;
    aRet.mePitch     = ePitch;

    return aRet;
}

namespace
{
    bool uselessmatch(const ImplFontSelectData &rOrig, const ImplFontSelectData &rNew)
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
}

//--------------------------------------------------------------------------

bool FcPreMatchSubstititution::FindFontSubstitute( ImplFontSelectData &rFontSelData ) const
{
    // We dont' actually want to talk to Fontconfig at all for symbol fonts
    if( rFontSelData.IsSymbolFont() )
        return false;
    // StarSymbol is a unicode font, but it still deserves the symbol flag
    if( 0 == rFontSelData.maSearchName.CompareIgnoreCaseToAscii( "starsymbol", 10)
    ||  0 == rFontSelData.maSearchName.CompareIgnoreCaseToAscii( "opensymbol", 10) )
        return false;

    //Note: see fdo#41556 if you feel compelled to cache the results here,
    //remember that fontconfig can return e.g. an italic font for a non-italic
    //input and/or different fonts depending on fontsize, bold, etc settings so
    //don't cache just on the name, cache on all the input and be don't just
    //return the original selection data with the fontname updated
    rtl::OUString aDummy;
    const ImplFontSelectData aOut = GetFcSubstitute( rFontSelData, aDummy );

    if( !aOut.maSearchName.Len() )
        return false;

    const bool bHaveSubstitute = !uselessmatch( rFontSelData, aOut );

#ifdef DEBUG
    const ByteString aOrigName( rFontSelData.maTargetName, RTL_TEXTENCODING_UTF8 );
    const ByteString aSubstName( aOut.maSearchName, RTL_TEXTENCODING_UTF8 );
    printf( "FcPreMatchSubstititution \"%s\" bipw=%d%d%d%d -> ",
        aOrigName.GetBuffer(), rFontSelData.meWeight, rFontSelData.meItalic,
        rFontSelData.mePitch, rFontSelData.meWidthType );
    if( !bHaveSubstitute )
        printf( "no substitute available\n" );
    else
        printf( "\"%s\" bipw=%d%d%d%d\n", aSubstName.GetBuffer(),
        aOut.meWeight, aOut.meItalic, aOut.mePitch, aOut.meWidthType );
#endif

    if( bHaveSubstitute )
        rFontSelData = aOut;

    return bHaveSubstitute;
}

// -----------------------------------------------------------------------

bool FcGlyphFallbackSubstititution::FindFontSubstitute( ImplFontSelectData& rFontSelData,
    rtl::OUString& rMissingCodes ) const
{
    // We dont' actually want to talk to Fontconfig at all for symbol fonts
    if( rFontSelData.IsSymbolFont() )
    return false;
    // StarSymbol is a unicode font, but it still deserves the symbol flag
    if( 0 == rFontSelData.maSearchName.CompareIgnoreCaseToAscii( "starsymbol", 10)
    ||  0 == rFontSelData.maSearchName.CompareIgnoreCaseToAscii( "opensymbol", 10) )
        return false;

    const ImplFontSelectData aOut = GetFcSubstitute( rFontSelData, rMissingCodes );
    // TODO: cache the unicode + srcfont specific result
    // FC doing it would be preferable because it knows the invariables
    // e.g. FC knows the FC rule that all Arial gets replaced by LiberationSans
    // whereas we would have to check for every size or attribute
    if( !aOut.maSearchName.Len() )
        return false;

    const bool bHaveSubstitute = !uselessmatch( rFontSelData, aOut );

#ifdef DEBUG
    const ByteString aOrigName( rFontSelData.maTargetName, RTL_TEXTENCODING_UTF8 );
    const ByteString aSubstName( aOut.maSearchName, RTL_TEXTENCODING_UTF8 );
    printf( "FcGFSubstititution \"%s\" bipw=%d%d%d%d ->",
        aOrigName.GetBuffer(), rFontSelData.meWeight, rFontSelData.meItalic,
        rFontSelData.mePitch, rFontSelData.meWidthType );
    if( !bHaveSubstitute )
        printf( "no substitute available\n" );
    else
        printf( "\"%s\" bipw=%d%d%d%d\n", aSubstName.GetBuffer(),
        aOut.meWeight, aOut.meItalic, aOut.mePitch, aOut.meWidthType );
#endif

    if( bHaveSubstitute )
        rFontSelData = aOut;

    return bHaveSubstitute;
}

// ===========================================================================


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
