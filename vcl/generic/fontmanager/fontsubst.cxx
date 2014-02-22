/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include "fontsubset.hxx"
#include "salprn.hxx"

#include <unotools/fontdefs.hxx>
#include <list>





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
    
public:
    bool FindFontSubstitute( FontSelectPattern&, OUString& rMissingCodes ) const;
};

int SalGenericInstance::FetchFontSubstitutionFlags()
{
    
    int nDisableBits = 0;
#ifdef SOLARIS
    nDisableBits = 1; 
#endif
    
    const char* pEnvStr = ::getenv( "SAL_DISABLE_FC_SUBST" );
    if( pEnvStr )
    {
        if( (*pEnvStr >= '0') && (*pEnvStr <= '9') )
            nDisableBits = (*pEnvStr - '0');
        else
            nDisableBits = ~0U; 
    }
    return nDisableBits;
}

void SalGenericInstance::RegisterFontSubstitutors( ImplDevFontList* pList )
{
    
    int nDisableBits = 0;
#ifdef SOLARIS
    nDisableBits = 1; 
#endif
    
    const char* pEnvStr = ::getenv( "SAL_DISABLE_FC_SUBST" );
    if( pEnvStr )
    {
        if( (*pEnvStr >= '0') && (*pEnvStr <= '9') )
            nDisableBits = (*pEnvStr - '0');
        else
            nDisableBits = ~0U; 
    }

    
    if( (nDisableBits & 1) == 0 )
    {
        static FcPreMatchSubstititution aSubstPreMatch;
        pList->SetPreMatchHook( &aSubstPreMatch );
    }

    
    if( (nDisableBits & 2) == 0 )
    {
        static FcGlyphFallbackSubstititution aSubstFallback;
        pList->SetFallbackHook( &aSubstFallback );
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
        equal(const FontSelectPatternAttributes& rAttributes)
            : mrAttributes(rAttributes)
        {
        }
        bool operator()(const FcPreMatchSubstititution::value_type& rOther) const
            { return rOther.first == mrAttributes; }
    };
}



bool FcPreMatchSubstititution::FindFontSubstitute( FontSelectPattern &rFontSelData ) const
{
    
    if( rFontSelData.IsSymbolFont() )
        return false;
    
    if ( IsStarSymbol(rFontSelData.maSearchName) )
        return false;

    
    
    
    
    
    FontSelectPatternAttributes& rPatternAttributes = rFontSelData;
    CachedFontMapType &rCachedFontMap = const_cast<CachedFontMapType &>(maCachedFontMap);
    CachedFontMapType::iterator itr = std::find_if(rCachedFontMap.begin(), rCachedFontMap.end(), equal(rPatternAttributes));
    if (itr != rCachedFontMap.end())
    {
        
        rFontSelData.copyAttributes(itr->second);
        if (itr != rCachedFontMap.begin())
        {
            
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
        
        
        if (rCachedFontMap.size() > 8)
            rCachedFontMap.pop_back();
        rFontSelData = aOut;
    }

    return bHaveSubstitute;
}



bool FcGlyphFallbackSubstititution::FindFontSubstitute( FontSelectPattern& rFontSelData,
    OUString& rMissingCodes ) const
{
    
    if( rFontSelData.IsSymbolFont() )
        return false;
    
    if ( IsStarSymbol(rFontSelData.maSearchName) )
        return false;

    const FontSelectPattern aOut = GetFcSubstitute( rFontSelData, rMissingCodes );
    
    
    
    
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
