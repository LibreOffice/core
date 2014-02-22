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

#include <config_folders.h>

#include <string.h>
#include <malloc.h>

#include <svsys.h>
#include "rtl/logfile.hxx"
#include "rtl/bootstrap.hxx"

#include "i18nlangtag/mslangid.hxx"

#include "osl/file.hxx"
#include "osl/process.h"

#include "basegfx/polygon/b2dpolygon.hxx"
#include "basegfx/matrix/b2dhommatrixtools.hxx"

#include "unotools/fontcfg.hxx"

#include "tools/stream.hxx"
#include "tools/helpers.hxx"

#include <vcl/sysdata.hxx>
#include <vcl/settings.hxx>

#include "win/saldata.hxx"
#include "win/salgdi.h"

#include "fontsubset.hxx"
#include "outdev.h"
#include "sft.hxx"

#include <algorithm>

#include <vector>
#include <set>
#include <map>

using namespace vcl;

static const int MAXFONTHEIGHT = 2048;





inline FIXED FixedFromDouble( double d )
{
    const long l = (long) ( d * 65536. );
    return *(FIXED*) &l;
}



inline int IntTimes256FromFixed(FIXED f)
{
    int nFixedTimes256 = (f.value << 8) + ((f.fract+0x80) >> 8);
    return nFixedTimes256;
}




static bool bImplSalCourierScalable = false;
static bool bImplSalCourierNew = false;







typedef std::map< OUString, ImplDevFontAttributes > FontAttrMap;

class ImplFontAttrCache
{
private:
    FontAttrMap     aFontAttributes;
    OUString        aCacheFileName;
    OUString        aBaseURL;
    sal_Bool        bModified;

protected:
    OUString OptimizeURL( const OUString& rURL ) const;

    enum{ MAGIC = 0x12349876 }; 

public:
            ImplFontAttrCache(const OUString& rCacheFileName,
                              const OUString& rBaseURL);
            ~ImplFontAttrCache();

    ImplDevFontAttributes  GetFontAttr( const OUString& rFontFileName ) const;
    void                   AddFontAttr( const OUString& rFontFileName, const ImplDevFontAttributes& );
};

ImplFontAttrCache::ImplFontAttrCache( const OUString& rFileNameURL, const OUString& rBaseURL ) : aBaseURL( rBaseURL )
{
    bModified = FALSE;
    aBaseURL = aBaseURL.toAsciiLowerCase();    

    
    osl::FileBase::getSystemPathFromFileURL( rFileNameURL, aCacheFileName );
    SvFileStream aCacheFile( aCacheFileName, STREAM_READ );
    if( !aCacheFile.IsOpen() )
        return;

    
    sal_uInt32 nCacheMagic;
    aCacheFile.ReadUInt32(nCacheMagic);
    if( nCacheMagic != ImplFontAttrCache::MAGIC )
        return;  

    
    OUString aFontFileURL;
    ImplDevFontAttributes aDFA;
    for(;;)
    {
        aFontFileURL = read_uInt16_lenPrefixed_uInt8s_ToOUString(aCacheFile, RTL_TEXTENCODING_UTF8);
        if( aFontFileURL.isEmpty() )
            break;
        aDFA.SetFamilyName(read_uInt16_lenPrefixed_uInt8s_ToOUString(aCacheFile, RTL_TEXTENCODING_UTF8));

        short n;
        aCacheFile.ReadInt16(n);
        aDFA.SetWeight(static_cast<FontWeight>(n));
        aCacheFile.ReadInt16(n);
        aDFA.SetItalic(static_cast<FontItalic>(n));
        aCacheFile.ReadInt16(n);
        aDFA.SetPitch(static_cast<FontPitch>(n));
        aCacheFile.ReadInt16(n);
        aDFA.SetWidthType(static_cast<FontWidth>(n));
        aCacheFile.ReadInt16(n);
        aDFA.SetFamilyType(static_cast<FontFamily>(n));
        aCacheFile.ReadInt16(n);
        aDFA.SetSymbolFlag(n != 0);

        OUString styleName;
        aCacheFile.ReadByteStringLine( styleName, RTL_TEXTENCODING_UTF8 );
        aDFA.SetStyleName( styleName );

        aFontAttributes[ aFontFileURL ] = aDFA;
    }
}

ImplFontAttrCache::~ImplFontAttrCache()
{
    if ( bModified )
    {
        SvFileStream aCacheFile( aCacheFileName, STREAM_WRITE|STREAM_TRUNC );
        if ( aCacheFile.IsWritable() )
        {
            sal_uInt32 nCacheMagic = ImplFontAttrCache::MAGIC;
            aCacheFile.WriteUInt32( nCacheMagic );

            
            FontAttrMap::const_iterator aIter = aFontAttributes.begin();
            while ( aIter != aFontAttributes.end() )
            {
                const OUString rFontFileURL( (*aIter).first );
                const ImplDevFontAttributes& rDFA( (*aIter).second );
                write_uInt16_lenPrefixed_uInt8s_FromOUString(aCacheFile, rFontFileURL, RTL_TEXTENCODING_UTF8);
                write_uInt16_lenPrefixed_uInt8s_FromOUString(aCacheFile, rDFA.GetFamilyName(), RTL_TEXTENCODING_UTF8);

                aCacheFile.WriteInt16(rDFA.GetWeight());
                aCacheFile.WriteInt16(rDFA.GetSlant());
                aCacheFile.WriteInt16(rDFA.GetPitch());
                aCacheFile.WriteInt16(rDFA.GetWidthType());
                aCacheFile.WriteInt16(rDFA.GetFamilyType());
                aCacheFile.WriteInt16(rDFA.IsSymbolFont() != false);

                write_uInt16_lenPrefixed_uInt8s_FromOUString(aCacheFile, rDFA.GetStyleName(), RTL_TEXTENCODING_UTF8);

                ++aIter;
            }
            
            write_uInt16_lenPrefixed_uInt8s_FromOString(aCacheFile, OString());
        }
    }
}

OUString ImplFontAttrCache::OptimizeURL( const OUString& rURL ) const
{
    OUString aOptimizedFontFileURL( rURL.toAsciiLowerCase() );
    if ( aOptimizedFontFileURL.startsWith( aBaseURL ) )
        aOptimizedFontFileURL = aOptimizedFontFileURL.copy( aBaseURL.getLength() );
    return aOptimizedFontFileURL;
}

ImplDevFontAttributes ImplFontAttrCache::GetFontAttr( const OUString& rFontFileName ) const
{
    ImplDevFontAttributes aDFA;
    FontAttrMap::const_iterator it = aFontAttributes.find( OptimizeURL( rFontFileName ) );
    if( it != aFontAttributes.end() )
    {
        aDFA = it->second;
    }
    return aDFA;
}

void ImplFontAttrCache::AddFontAttr( const OUString& rFontFileName, const ImplDevFontAttributes& rDFA )
{
    SAL_WARN_IF(rFontFileName.isEmpty() || rDFA.GetFamilyName().isEmpty(),
        "vcl.gdi", "ImplFontNameCache::AddFontName - invalid data!");
    if ( !rFontFileName.isEmpty() && !rDFA.GetFamilyName().isEmpty() )
    {
        aFontAttributes.insert( FontAttrMap::value_type( OptimizeURL( rFontFileName ), rDFA ) );
        bModified = TRUE;
    }
}




class RawFontData
{
public:
    explicit    RawFontData( HDC, DWORD nTableTag=0 );
                ~RawFontData() { delete[] mpRawBytes; }
    const unsigned char*    get() const { return mpRawBytes; }
    const unsigned char*    steal() { unsigned char* p = mpRawBytes; mpRawBytes = NULL; return p; }
    int               size() const { return mnByteCount; }

private:
    unsigned char*  mpRawBytes;
    unsigned        mnByteCount;
};

RawFontData::RawFontData( HDC hDC, DWORD nTableTag )
:   mpRawBytes( NULL )
,   mnByteCount( 0 )
{
    
    mnByteCount = ::GetFontData( hDC, nTableTag, 0, NULL, 0 );
    if( mnByteCount == GDI_ERROR )
        return;
    else if( !mnByteCount )
        return;

    
    mpRawBytes = new unsigned char[ mnByteCount ];

    
    unsigned nRawDataOfs = 0;
    DWORD nMaxChunkSize = 0x100000;
    for(;;)
    {
        
        DWORD nFDGet = mnByteCount - nRawDataOfs;
        if( nFDGet <= 0 )
            break;
        
        if( nFDGet > nMaxChunkSize )
            nFDGet = nMaxChunkSize;
        const DWORD nFDGot = ::GetFontData( hDC, nTableTag, nRawDataOfs,
            (void*)(mpRawBytes + nRawDataOfs), nFDGet );
        if( !nFDGot )
            break;
        else if( nFDGot != GDI_ERROR )
            nRawDataOfs += nFDGot;
        else
        {
            
            nMaxChunkSize /= 2;
            if( nMaxChunkSize < 0x10000 )
                break;
        }
    }

    
    if( nRawDataOfs != mnByteCount )
    {
        delete[] mpRawBytes;
        mpRawBytes = NULL;
    }
}





struct Unicode2LangType
{
    sal_UCS4 mnMinCode;
    sal_UCS4 mnMaxCode;
    LanguageType mnLangID;
};


#define LANGUAGE_DEFAULT_CJK 0xFFF0



static Unicode2LangType aLangFromCodeChart[]= {
    {0x0000, 0x007F, LANGUAGE_ENGLISH},             
    {0x0080, 0x024F, LANGUAGE_ENGLISH},             
    {0x0250, 0x02AF, LANGUAGE_SYSTEM},              
    {0x0370, 0x03FF, LANGUAGE_GREEK},               
    {0x0590, 0x05FF, LANGUAGE_HEBREW},              
    {0x0600, 0x06FF, LANGUAGE_ARABIC_PRIMARY_ONLY}, 
    {0x0900, 0x097F, LANGUAGE_HINDI},               
    {0x0980, 0x09FF, LANGUAGE_BENGALI},             
    {0x0A80, 0x0AFF, LANGUAGE_GUJARATI},            
    {0x0B00, 0x0B7F, LANGUAGE_ODIA},                
    {0x0B80, 0x0BFF, LANGUAGE_TAMIL},               
    {0x0C00, 0x0C7F, LANGUAGE_TELUGU},              
    {0x0C80, 0x0CFF, LANGUAGE_KANNADA},             
    {0x0D00, 0x0D7F, LANGUAGE_MALAYALAM},           
    {0x0D80, 0x0D7F, LANGUAGE_SINHALESE_SRI_LANKA}, 
    {0x0E00, 0x0E7F, LANGUAGE_THAI},                
    {0x0E80, 0x0EFF, LANGUAGE_LAO},                 
    {0x0F00, 0x0FFF, LANGUAGE_TIBETAN},             
    {0x1000, 0x109F, LANGUAGE_BURMESE},             
    {0x10A0, 0x10FF, LANGUAGE_GEORGIAN},            
    {0x1100, 0x11FF, LANGUAGE_KOREAN},              


    {0x13A0, 0x13FF, LANGUAGE_CHEROKEE_UNITED_STATES}, 







    {0x1780, 0x17FF, LANGUAGE_KHMER},               
    {0x18A0, 0x18AF, LANGUAGE_MONGOLIAN_MONGOLIAN_MONGOLIA}, 



    {0x19E0, 0x19FF, LANGUAGE_KHMER},               



    {0x1E00, 0x1EFF, LANGUAGE_ENGLISH},             
    {0x1F00, 0x1FFF, LANGUAGE_GREEK},               
    {0x2C60, 0x2C7F, LANGUAGE_ENGLISH},             
    {0x2E80, 0x2FFf, LANGUAGE_CHINESE_SIMPLIFIED},  
    {0x3000, 0x303F, LANGUAGE_DEFAULT_CJK},         
    {0x3040, 0x30FF, LANGUAGE_JAPANESE},            
    {0x3100, 0x312F, LANGUAGE_CHINESE_TRADITIONAL}, 
    {0x3130, 0x318F, LANGUAGE_KOREAN},              
    {0x3190, 0x319F, LANGUAGE_JAPANESE},            
    {0x31A0, 0x31BF, LANGUAGE_CHINESE_TRADITIONAL}, 
    {0x31C0, 0x31EF, LANGUAGE_DEFAULT_CJK},         
    {0x31F0, 0x31FF, LANGUAGE_JAPANESE},            
    {0x3200, 0x321F, LANGUAGE_KOREAN},              
    {0x3220, 0x325F, LANGUAGE_DEFAULT_CJK},         
    {0x3260, 0x327F, LANGUAGE_KOREAN},              
    {0x3280, 0x32CF, LANGUAGE_DEFAULT_CJK},         
    {0x32d0, 0x32FF, LANGUAGE_JAPANESE},            
    {0x3400, 0x4DBF, LANGUAGE_DEFAULT_CJK},         
    {0x4E00, 0x9FCF, LANGUAGE_DEFAULT_CJK},         
    {0xA720, 0xA7FF, LANGUAGE_ENGLISH},             
    {0xAC00, 0xD7AF, LANGUAGE_KOREAN},              
    {0xF900, 0xFAFF, LANGUAGE_DEFAULT_CJK},         
    {0xFB00, 0xFB4F, LANGUAGE_HEBREW},              
    {0xFB50, 0xFDFF, LANGUAGE_ARABIC_PRIMARY_ONLY}, 
    {0xFE70, 0xFEFE, LANGUAGE_ARABIC_PRIMARY_ONLY}, 
    {0xFF65, 0xFF9F, LANGUAGE_JAPANESE},            
    {0xFFA0, 0xFFDC, LANGUAGE_KOREAN},              
    {0x10140, 0x1018F, LANGUAGE_GREEK},             
    {0x1D200, 0x1D24F, LANGUAGE_GREEK},             
    {0x20000, 0x2A6DF, LANGUAGE_DEFAULT_CJK},       
    {0x2F800, 0x2FA1F, LANGUAGE_DEFAULT_CJK}        
};


LanguageType MapCharToLanguage( sal_UCS4 uChar )
{
    
    static bool bFirst = true;
    if( bFirst )
    {
        bFirst = false;

        
        
        LanguageType nDefaultLang = 0;
        HKEY hKey = NULL;
        LONG lResult = ::RegOpenKeyExA( HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Control\\Nls\\Language",
            0, KEY_QUERY_VALUE, &hKey );
        char aKeyValBuf[16];
        DWORD nKeyValSize = sizeof(aKeyValBuf);
        if( ERROR_SUCCESS == lResult )
            lResult = RegQueryValueExA( hKey, "Default", NULL, NULL, (LPBYTE)aKeyValBuf, &nKeyValSize );
        aKeyValBuf[ sizeof(aKeyValBuf)-1 ] = '\0';
        if( ERROR_SUCCESS == lResult )
            nDefaultLang = (LanguageType)rtl_str_toInt32( aKeyValBuf, 16 );

        
        
        if( !nDefaultLang )
            nDefaultLang = Application::GetSettings().GetUILanguageTag().getLanguageType();

        LanguageType nDefaultCJK = MsLangId::isCJK(nDefaultLang) ? nDefaultLang : LANGUAGE_CHINESE;

        
        static const int nCount = SAL_N_ELEMENTS(aLangFromCodeChart);
        for( int i = 0; i < nCount; ++i )
        {
            if( aLangFromCodeChart[ i].mnLangID == LANGUAGE_DEFAULT_CJK )
                aLangFromCodeChart[ i].mnLangID = nDefaultCJK;
        }
    }

    
    int nLow = 0;
    int nHigh = SAL_N_ELEMENTS(aLangFromCodeChart) - 1;
    while( nLow <= nHigh )
    {
        int nMiddle = (nHigh + nLow) / 2;
        if( uChar < aLangFromCodeChart[ nMiddle].mnMinCode )
            nHigh = nMiddle - 1;
        else if( uChar > aLangFromCodeChart[ nMiddle].mnMaxCode )
            nLow = nMiddle + 1;
        else
            return aLangFromCodeChart[ nMiddle].mnLangID;
    }

    return LANGUAGE_DONTKNOW;
}

class WinGlyphFallbackSubstititution
:    public ImplGlyphFallbackFontSubstitution
{
public:
    explicit    WinGlyphFallbackSubstititution( HDC );

    bool FindFontSubstitute( FontSelectPattern&, OUString& rMissingChars ) const;
private:
    HDC mhDC;
    bool HasMissingChars( const PhysicalFontFace*, const OUString& rMissingChars ) const;
};

inline WinGlyphFallbackSubstititution::WinGlyphFallbackSubstititution( HDC hDC )
:   mhDC( hDC )
{}

void ImplGetLogFontFromFontSelect( HDC, const FontSelectPattern*,
    LOGFONTW&, bool /*bTestVerticalAvail*/ );


bool WinGlyphFallbackSubstititution::HasMissingChars( const PhysicalFontFace* pFace, const OUString& rMissingChars ) const
{
    const ImplWinFontData* pWinFont = static_cast<const ImplWinFontData*>(pFace);
    const ImplFontCharMap* pCharMap = pWinFont->GetImplFontCharMap();
    if( !pCharMap )
    {
        
        const Size aSize( pFace->GetWidth(), pFace->GetHeight() );
        
        const FontSelectPattern aFSD( *pFace, aSize, (float)aSize.Height(), 0, false );
        
        LOGFONTW aLogFont;
        ImplGetLogFontFromFontSelect( mhDC, &aFSD, aLogFont, true );

        
        HFONT hNewFont = ::CreateFontIndirectW( &aLogFont );
        
        HFONT hOldFont = ::SelectFont( mhDC, hNewFont );

        
        pWinFont->UpdateFromHDC( mhDC );

        
        ::SelectFont( mhDC, hOldFont );
        ::DeleteFont( hNewFont );

        
        pCharMap = pWinFont->GetImplFontCharMap();
    }

    
    if( !pCharMap || pCharMap->IsDefaultMap() )
        return false;
        pCharMap->AddReference();

    int nMatchCount = 0;
    
    const sal_Int32 nStrLen = rMissingChars.getLength();
    for( sal_Int32 nStrIdx = 0; nStrIdx < nStrLen; /* ++nStrIdx unreachable code, see the 'break' below */ )
    {
        const sal_UCS4 uChar = rMissingChars.iterateCodePoints( &nStrIdx );
        nMatchCount += pCharMap->HasChar( uChar );
        break; 
    }
        pCharMap->DeReference();

    const bool bHasMatches = (nMatchCount > 0);
    return bHasMatches;
}

namespace
{
    
    ImplDevFontListData* findDevFontListByLocale(const ImplDevFontList &rDevFontList,
        const LanguageTag& rLanguageTag )
    {
        
        const utl::DefaultFontConfiguration& rDefaults =
            utl::DefaultFontConfiguration::get();
        const OUString aDefault = rDefaults.getUserInterfaceFont(rLanguageTag);
        return rDevFontList.ImplFindByTokenNames(aDefault);
    }
}



bool WinGlyphFallbackSubstititution::FindFontSubstitute( FontSelectPattern& rFontSelData, OUString& rMissingChars ) const
{
    
    LanguageType eLang = LANGUAGE_DONTKNOW;
    LanguageTag aLanguageTag( eLang);

    sal_Int32 nStrIdx = 0;
    const sal_Int32 nStrLen = rMissingChars.getLength();
    while( nStrIdx < nStrLen )
    {
        const sal_UCS4 uChar = rMissingChars.iterateCodePoints( &nStrIdx );
        eLang = MapCharToLanguage( uChar );
        if( eLang == LANGUAGE_DONTKNOW )
            continue;
        aLanguageTag.reset( eLang);
        break;
    }

    
    if( eLang == LANGUAGE_DONTKNOW )
        aLanguageTag = Application::GetSettings().GetUILanguageTag();

    
    
    const ImplDevFontList* pDevFontList = ImplGetSVData()->maGDIData.mpScreenFontList;
    /*const*/ ImplDevFontListData* pDevFont = findDevFontListByLocale(*pDevFontList, aLanguageTag);
    if( pDevFont )
    {
        const PhysicalFontFace* pFace = pDevFont->FindBestFontFace( rFontSelData );
        if( HasMissingChars( pFace, rMissingChars ) )
        {
            rFontSelData.maSearchName = pDevFont->GetSearchName();
            return true;
        }
    }

    
    pDevFont = pDevFontList->ImplFindByAttributes( IMPL_FONT_ATTR_SYMBOL,
                    rFontSelData.GetWeight(), rFontSelData.GetWidthType(),
                    rFontSelData.GetSlant(), rFontSelData.maSearchName );
    if( pDevFont )
    {
        const PhysicalFontFace* pFace = pDevFont->FindBestFontFace( rFontSelData );
        if( HasMissingChars( pFace, rMissingChars ) )
        {
            rFontSelData.maSearchName = pDevFont->GetSearchName();
            return true;
        }
    }

    
    ImplGetDevFontList* pTestFontList = pDevFontList->GetDevFontList();
    
    static const int MAX_GFBFONT_COUNT = 600;
    int nTestFontCount = pTestFontList->Count();
    if( nTestFontCount > MAX_GFBFONT_COUNT )
        nTestFontCount = MAX_GFBFONT_COUNT;

    bool bFound = false;
    for( int i = 0; i < nTestFontCount; ++i )
    {
        const PhysicalFontFace* pFace = pTestFontList->Get( i );
        bFound = HasMissingChars( pFace, rMissingChars );
        if( !bFound )
            continue;
        rFontSelData.maSearchName = pFace->GetFamilyName();
        break;
    }

    delete pTestFontList;

    return bFound;
}



struct ImplEnumInfo
{
    HDC                 mhDC;
    ImplDevFontList*    mpList;
    OUString*           mpName;
    LOGFONTA*           mpLogFontA;
    LOGFONTW*           mpLogFontW;
    UINT                mnPreferredCharSet;
    bool                mbCourier;
    bool                mbImplSalCourierScalable;
    bool                mbImplSalCourierNew;
    bool                mbPrinter;
    int                 mnFontCount;
};



static rtl_TextEncoding ImplCharSetToSal( BYTE nCharSet )
{
    rtl_TextEncoding eTextEncoding;

    if ( nCharSet == OEM_CHARSET )
    {
        UINT nCP = (sal_uInt16)GetOEMCP();
        switch ( nCP )
        {
            
            
            case 1004:  eTextEncoding = RTL_TEXTENCODING_MS_1252; break;
            case 65400: eTextEncoding = RTL_TEXTENCODING_SYMBOL; break;
            default:
                eTextEncoding = rtl_getTextEncodingFromWindowsCodePage(nCP);
                break;
        };
    }
    else
    {
        if( nCharSet )
            eTextEncoding = rtl_getTextEncodingFromWindowsCharset( nCharSet );
        else
            eTextEncoding = RTL_TEXTENCODING_UNICODE;
    }

    return eTextEncoding;
}



static FontFamily ImplFamilyToSal( BYTE nFamily )
{
    switch ( nFamily & 0xF0 )
    {
        case FF_DECORATIVE:
            return FAMILY_DECORATIVE;

        case FF_MODERN:
            return FAMILY_MODERN;

        case FF_ROMAN:
            return FAMILY_ROMAN;

        case FF_SCRIPT:
            return FAMILY_SCRIPT;

        case FF_SWISS:
            return FAMILY_SWISS;

        default:
            break;
    }

    return FAMILY_DONTKNOW;
}



static BYTE ImplFamilyToWin( FontFamily eFamily )
{
    switch ( eFamily )
    {
        case FAMILY_DECORATIVE:
            return FF_DECORATIVE;

        case FAMILY_MODERN:
            return FF_MODERN;

        case FAMILY_ROMAN:
            return FF_ROMAN;

        case FAMILY_SCRIPT:
            return FF_SCRIPT;

        case FAMILY_SWISS:
            return FF_SWISS;

        case FAMILY_SYSTEM:
            return FF_SWISS;

        default:
            break;
    }

    return FF_DONTCARE;
}



static FontWeight ImplWeightToSal( int nWeight )
{
    if ( nWeight <= FW_THIN )
        return WEIGHT_THIN;
    else if ( nWeight <= FW_ULTRALIGHT )
        return WEIGHT_ULTRALIGHT;
    else if ( nWeight <= FW_LIGHT )
        return WEIGHT_LIGHT;
    else if ( nWeight < FW_MEDIUM )
        return WEIGHT_NORMAL;
    else if ( nWeight == FW_MEDIUM )
        return WEIGHT_MEDIUM;
    else if ( nWeight <= FW_SEMIBOLD )
        return WEIGHT_SEMIBOLD;
    else if ( nWeight <= FW_BOLD )
        return WEIGHT_BOLD;
    else if ( nWeight <= FW_ULTRABOLD )
        return WEIGHT_ULTRABOLD;
    else
        return WEIGHT_BLACK;
}



static int ImplWeightToWin( FontWeight eWeight )
{
    switch ( eWeight )
    {
        case WEIGHT_THIN:
            return FW_THIN;

        case WEIGHT_ULTRALIGHT:
            return FW_ULTRALIGHT;

        case WEIGHT_LIGHT:
            return FW_LIGHT;

        case WEIGHT_SEMILIGHT:
        case WEIGHT_NORMAL:
            return FW_NORMAL;

        case WEIGHT_MEDIUM:
            return FW_MEDIUM;

        case WEIGHT_SEMIBOLD:
            return FW_SEMIBOLD;

        case WEIGHT_BOLD:
            return FW_BOLD;

        case WEIGHT_ULTRABOLD:
            return FW_ULTRABOLD;

        case WEIGHT_BLACK:
            return FW_BLACK;

        default:
            break;
    }

    return 0;
}



inline FontPitch ImplLogPitchToSal( BYTE nPitch )
{
    if ( nPitch & FIXED_PITCH )
        return PITCH_FIXED;
    else
        return PITCH_VARIABLE;
}



inline FontPitch ImplMetricPitchToSal( BYTE nPitch )
{
    
    if ( !(nPitch & TMPF_FIXED_PITCH) )
        return PITCH_FIXED;
    else
        return PITCH_VARIABLE;
}



inline BYTE ImplPitchToWin( FontPitch ePitch )
{
    if ( ePitch == PITCH_FIXED )
        return FIXED_PITCH;
    else if ( ePitch == PITCH_VARIABLE )
        return VARIABLE_PITCH;
    else
        return DEFAULT_PITCH;
}



static ImplDevFontAttributes WinFont2DevFontAttributes( const ENUMLOGFONTEXA& rEnumFont,
    const NEWTEXTMETRICA& rMetric, DWORD nFontType )
{
    ImplDevFontAttributes aDFA;

    const LOGFONTA rLogFont = rEnumFont.elfLogFont;

    
    aDFA.SetFamilyType(ImplFamilyToSal( rLogFont.lfPitchAndFamily ));
    aDFA.SetWidthType(WIDTH_DONTKNOW);
    aDFA.SetWeight(ImplWeightToSal( rLogFont.lfWeight ));
    aDFA.SetItalic((rLogFont.lfItalic) ? ITALIC_NORMAL : ITALIC_NONE);
    aDFA.SetPitch(ImplLogPitchToSal( rLogFont.lfPitchAndFamily ));
    aDFA.SetSymbolFlag(rLogFont.lfCharSet == SYMBOL_CHARSET);

    
    aDFA.SetFamilyName(ImplSalGetUniString( rLogFont.lfFaceName ));

    
    const char* pStyleName = (const char*)rEnumFont.elfStyle;
    const char* pEnd = pStyleName + sizeof( rEnumFont.elfStyle );
    const char* p = pStyleName;
    for(; *p && (p < pEnd); ++p )
        if( (0x00 < *p) && (*p < 0x20) )
            break;
    if( p < pEnd )
        aDFA.SetStyleName(ImplSalGetUniString( pStyleName ));

    
    aDFA.mbOrientation  = (nFontType & RASTER_FONTTYPE) == 0;
    aDFA.mbDevice       = (rMetric.tmPitchAndFamily & TMPF_DEVICE) != 0;

    aDFA.mbEmbeddable   = false;
    aDFA.mbSubsettable  = false;
    if( 0 != (rMetric.ntmFlags & (NTM_TT_OPENTYPE | NTM_PS_OPENTYPE))
     || 0 != (rMetric.tmPitchAndFamily & TMPF_TRUETYPE))
        aDFA.mbSubsettable = true;
    else if( 0 != (rMetric.ntmFlags & NTM_TYPE1) ) 
        aDFA.mbEmbeddable = true;

    
    
    
    aDFA.mnQuality = 0;
    if( rMetric.tmPitchAndFamily & TMPF_TRUETYPE )
        aDFA.mnQuality += 50;
    if( 0 != (rMetric.ntmFlags & (NTM_TT_OPENTYPE | NTM_PS_OPENTYPE)) )
        aDFA.mnQuality += 10;
    if( aDFA.mbSubsettable )
        aDFA.mnQuality += 200;
    else if( aDFA.mbEmbeddable )
        aDFA.mnQuality += 100;

    
    if( aDFA.mbEmbeddable )
    {
        if( aDFA.GetFamilyName() == "AvantGarde"
        ||  aDFA.GetFamilyName() == "Bookman"
        ||  aDFA.GetFamilyName() == "Courier"
        ||  aDFA.GetFamilyName() == "Helvetica"
        ||  aDFA.GetFamilyName() == "NewCenturySchlbk"
        ||  aDFA.GetFamilyName() == "Palatino"
        ||  aDFA.GetFamilyName() == "Symbol"
        ||  aDFA.GetFamilyName() == "Times"
        ||  aDFA.GetFamilyName() == "ZapfChancery"
        ||  aDFA.GetFamilyName() == "ZapfDingbats" )
            aDFA.mnQuality += 500;
    }

    
    return aDFA;
}



static ImplDevFontAttributes WinFont2DevFontAttributes( const ENUMLOGFONTEXW& rEnumFont,
    const NEWTEXTMETRICW& rMetric, DWORD nFontType )
{
    ImplDevFontAttributes aDFA;

    const LOGFONTW rLogFont = rEnumFont.elfLogFont;

    
    aDFA.SetFamilyType(ImplFamilyToSal( rLogFont.lfPitchAndFamily ));
    aDFA.SetWidthType(WIDTH_DONTKNOW);
    aDFA.SetWeight(ImplWeightToSal( rLogFont.lfWeight ));
    aDFA.SetItalic((rLogFont.lfItalic) ? ITALIC_NORMAL : ITALIC_NONE);
    aDFA.SetPitch(ImplLogPitchToSal( rLogFont.lfPitchAndFamily ));
    aDFA.SetSymbolFlag(rLogFont.lfCharSet == SYMBOL_CHARSET);

    
    aDFA.SetFamilyName(OUString(reinterpret_cast<const sal_Unicode*>(rLogFont.lfFaceName)));

    
    const wchar_t* pStyleName = rEnumFont.elfStyle;
    const wchar_t* pEnd = pStyleName + sizeof(rEnumFont.elfStyle)/sizeof(*rEnumFont.elfStyle);
    const wchar_t* p = pStyleName;
    for(; *p && (p < pEnd); ++p )
        if( *p < 0x0020 )
            break;
    if( p < pEnd )
        aDFA.SetStyleName(OUString(reinterpret_cast<const sal_Unicode*>(pStyleName)));

    
    aDFA.mbOrientation  = (nFontType & RASTER_FONTTYPE) == 0;
    aDFA.mbDevice       = (rMetric.tmPitchAndFamily & TMPF_DEVICE) != 0;

    aDFA.mbEmbeddable   = false;
    aDFA.mbSubsettable  = false;
    if( 0 != (rMetric.ntmFlags & (NTM_TT_OPENTYPE | NTM_PS_OPENTYPE))
     || 0 != (rMetric.tmPitchAndFamily & TMPF_TRUETYPE))
        aDFA.mbSubsettable = true;
    else if( 0 != (rMetric.ntmFlags & NTM_TYPE1) ) 
        aDFA.mbEmbeddable = true;

    
    
    
    aDFA.mnQuality = 0;
    if( rMetric.tmPitchAndFamily & TMPF_TRUETYPE )
        aDFA.mnQuality += 50;
    if( 0 != (rMetric.ntmFlags & (NTM_TT_OPENTYPE | NTM_PS_OPENTYPE)) )
        aDFA.mnQuality += 10;
    if( aDFA.mbSubsettable )
        aDFA.mnQuality += 200;
    else if( aDFA.mbEmbeddable )
        aDFA.mnQuality += 100;

    
    if( aDFA.mbEmbeddable )
    {
        if( aDFA.GetFamilyName() == "AvantGarde"
        ||  aDFA.GetFamilyName() == "Bookman"
        ||  aDFA.GetFamilyName() == "Courier"
        ||  aDFA.GetFamilyName() == "Helvetica"
        ||  aDFA.GetFamilyName() == "NewCenturySchlbk"
        ||  aDFA.GetFamilyName() == "Palatino"
        ||  aDFA.GetFamilyName() == "Symbol"
        ||  aDFA.GetFamilyName() == "Times"
        ||  aDFA.GetFamilyName() == "ZapfChancery"
        ||  aDFA.GetFamilyName() == "ZapfDingbats" )
            aDFA.mnQuality += 500;
    }

    
    return aDFA;
}



static ImplWinFontData* ImplLogMetricToDevFontDataA( const ENUMLOGFONTEXA* pLogFont,
                                         const NEWTEXTMETRICA* pMetric,
                                         DWORD nFontType )
{
    int nHeight = 0;
    if ( nFontType & RASTER_FONTTYPE )
        nHeight = pMetric->tmHeight - pMetric->tmInternalLeading;

    ImplWinFontData* pData = new ImplWinFontData(
        WinFont2DevFontAttributes(*pLogFont, *pMetric, nFontType),
        nHeight,
        pLogFont->elfLogFont.lfCharSet,
        pMetric->tmPitchAndFamily );

    return pData;
}



static ImplWinFontData* ImplLogMetricToDevFontDataW( const ENUMLOGFONTEXW* pLogFont,
                                         const NEWTEXTMETRICW* pMetric,
                                         DWORD nFontType )
{
    int nHeight = 0;
    if ( nFontType & RASTER_FONTTYPE )
        nHeight = pMetric->tmHeight - pMetric->tmInternalLeading;

    ImplWinFontData* pData = new ImplWinFontData(
        WinFont2DevFontAttributes(*pLogFont, *pMetric, nFontType),
        nHeight,
        pLogFont->elfLogFont.lfCharSet,
        pMetric->tmPitchAndFamily );

    return pData;
}



void ImplSalLogFontToFontW( HDC hDC, const LOGFONTW& rLogFont, Font& rFont )
{
    OUString aFontName( reinterpret_cast<const sal_Unicode*>(rLogFont.lfFaceName) );
    if (!aFontName.isEmpty())
    {
        rFont.SetName( aFontName );
        rFont.SetCharSet( ImplCharSetToSal( rLogFont.lfCharSet ) );
        rFont.SetFamily( ImplFamilyToSal( rLogFont.lfPitchAndFamily ) );
        rFont.SetPitch( ImplLogPitchToSal( rLogFont.lfPitchAndFamily ) );
        rFont.SetWeight( ImplWeightToSal( rLogFont.lfWeight ) );

        long nFontHeight = rLogFont.lfHeight;
        if ( nFontHeight < 0 )
            nFontHeight = -nFontHeight;
        long nDPIY = GetDeviceCaps( hDC, LOGPIXELSY );
        if( !nDPIY )
            nDPIY = 600;
        nFontHeight *= 72;
        nFontHeight += nDPIY/2;
        nFontHeight /= nDPIY;
        rFont.SetSize( Size( 0, nFontHeight ) );
        rFont.SetOrientation( (short)rLogFont.lfEscapement );
        if ( rLogFont.lfItalic )
            rFont.SetItalic( ITALIC_NORMAL );
        else
            rFont.SetItalic( ITALIC_NONE );
        if ( rLogFont.lfUnderline )
            rFont.SetUnderline( UNDERLINE_SINGLE );
        else
            rFont.SetUnderline( UNDERLINE_NONE );
        if ( rLogFont.lfStrikeOut )
            rFont.SetStrikeout( STRIKEOUT_SINGLE );
        else
            rFont.SetStrikeout( STRIKEOUT_NONE );
    }
}


#if ENABLE_GRAPHITE

#ifdef DEBUG
static FILE * grLogFile = NULL;
static FILE * grLog()
{
    std::string logFileName(getenv("TEMP"));
    logFileName.append("\\grface.log");
    if (grLogFile == NULL) grLogFile = fopen(logFileName.c_str(),"w");
    else fflush(grLogFile);
    return grLogFile;
}
#undef NDEBUG
#endif

const void * getGrTable(const void* appFaceHandle, unsigned int name, size_t *len)
{
    const GrFontData * fontTables = reinterpret_cast<const GrFontData*>(appFaceHandle);
    return fontTables->getTable(name, len);
}

GrFontData::GrFontData(HDC hDC) :
    mhDC(hDC), mpFace(NULL), mnRefCount(1)
{
    
    
    static const char* pGraphiteCacheStr = getenv( "SAL_GRAPHITE_CACHE_SIZE" );
    unsigned long graphiteSegCacheSize = pGraphiteCacheStr ? (atoi(pGraphiteCacheStr)) : 0;
    if (graphiteSegCacheSize > 500)
        mpFace = gr_make_face_with_seg_cache(this, getGrTable,
            graphiteSegCacheSize, gr_face_preloadGlyphs | gr_face_cacheCmap);
    else
        mpFace = gr_make_face(this, getGrTable,
            gr_face_preloadGlyphs | gr_face_cacheCmap);
#ifdef DEBUG
        fprintf(grLog(), "gr_make_face %lx for WinFontData %lx\n", (unsigned long)mpFace,
            (unsigned long)this);
#endif
    mhDC = NULL;
}

GrFontData::~GrFontData()
{
    if (mpFace)
    {
#ifdef DEBUG
        fprintf(grLog(), "gr_face_destroy %lx for WinFontData %lx\n", (unsigned long)mpFace,
            (unsigned long)this);
#endif
        gr_face_destroy(mpFace);
        mpFace = NULL;
    }
    std::vector<RawFontData*>::iterator i = mvData.begin();
    while (i != mvData.end())
    {
        delete *i;
        ++i;
    }
    mvData.clear();
}

const void * GrFontData::getTable(unsigned int name, size_t *len) const
{
#ifdef DEBUG
#undef NDEBUG
#endif
    assert(mhDC);
    
    union TtfTag {
        unsigned int i;
        unsigned char c[4];
    };
    TtfTag littleEndianTag;
    littleEndianTag.i = name;
    TtfTag bigEndianTag;
    bigEndianTag.c[0] = littleEndianTag.c[3];
    bigEndianTag.c[1] = littleEndianTag.c[2];
    bigEndianTag.c[2] = littleEndianTag.c[1];
    bigEndianTag.c[3] = littleEndianTag.c[0];
    mvData.push_back(new RawFontData(mhDC, bigEndianTag.i));
    const RawFontData * data = mvData[mvData.size()-1];
    if (data && (data->size() > 0))
    {
        if (len)
            *len = data->size();
        return reinterpret_cast<const void *>(data->get());
    }
    else
    {
        if (len)
            *len = 0;
        return NULL;
    }
}
#endif

ImplWinFontData::ImplWinFontData( const ImplDevFontAttributes& rDFS,
    int nHeight, BYTE eWinCharSet, BYTE nPitchAndFamily )
:   PhysicalFontFace( rDFS, 0 ),
    mnId( 0 ),
    mbDisableGlyphApi( false ),
    mbHasKoreanRange( false ),
    mbHasCJKSupport( false ),
#if ENABLE_GRAPHITE
    mpGraphiteData(NULL),
    mbHasGraphiteSupport( false ),
#endif
    mbHasArabicSupport ( false ),
    mbFontCapabilitiesRead( false ),
    mpUnicodeMap( NULL ),
    mpEncodingVector( NULL ),
    mpFontCharSets( NULL ),
    mnFontCharSetCount( 0 ),
    meWinCharSet( eWinCharSet ),
    mnPitchAndFamily( nPitchAndFamily ),
    mbAliasSymbolsHigh( false ),
    mbAliasSymbolsLow( false ),
    mbGsubRead( false )
{
    SetBitmapSize( 0, nHeight );

    if( eWinCharSet == SYMBOL_CHARSET )
    {
        if( (nPitchAndFamily & TMPF_TRUETYPE) != 0 )
        {
            
            mbAliasSymbolsHigh = true;
        }
        else if( (nPitchAndFamily & (TMPF_VECTOR|TMPF_DEVICE))
                                 == (TMPF_VECTOR|TMPF_DEVICE) )
        {
            
            
            mbAliasSymbolsLow  = true;
        }
        else if( (nPitchAndFamily & (TMPF_VECTOR|TMPF_TRUETYPE)) == 0 )
        {
            
            mbAliasSymbolsHigh = true;
        }
    }
#ifdef DEBUG
    fprintf(grLog(), "ImplWinFontData::ImplWinFontData() %lx\n", (unsigned long)this);
#endif
}



ImplWinFontData::~ImplWinFontData()
{
    delete[] mpFontCharSets;

    if( mpUnicodeMap )
        mpUnicodeMap->DeReference();
#if ENABLE_GRAPHITE
    if (mpGraphiteData)
        mpGraphiteData->DeReference();
#ifdef DEBUG
    fprintf(grLog(), "ImplWinFontData::~ImplWinFontData %lx\n", (unsigned long)this);
#endif
#endif 
    delete mpEncodingVector;
}



sal_IntPtr ImplWinFontData::GetFontId() const
{
    return mnId;
}



static unsigned GetUInt( const unsigned char* p ) { return((p[0]<<24)+(p[1]<<16)+(p[2]<<8)+p[3]);}
static inline DWORD CalcTag( const char p[4]) { return (p[0]+(p[1]<<8)+(p[2]<<16)+(p[3]<<24)); }



void ImplWinFontData::UpdateFromHDC( HDC hDC ) const
{
    
    if( mpUnicodeMap != NULL )
        return;

    ReadCmapTable( hDC );
    GetFontCapabilities( hDC );
#if ENABLE_GRAPHITE
    static const char* pDisableGraphiteText = getenv( "SAL_DISABLE_GRAPHITE" );
    if( !pDisableGraphiteText || (pDisableGraphiteText[0] == '0') )
    {
        const DWORD nSilfTag = CalcTag("Silf");
        const RawFontData aRawFontData( hDC, nSilfTag );
        mbHasGraphiteSupport = (aRawFontData.size() > 0);
        if (mbHasGraphiteSupport)
        {
#ifdef DEBUG
            fprintf(grLog(), "ImplWinFontData::UpdateFromHDC %lx\n",
            (unsigned long)this);
#endif
            if (mpGraphiteData == NULL)
            {
                mpGraphiteData = new GrFontData(hDC);
                if (!mpGraphiteData->getFace())
                {
                    mbHasGraphiteSupport = false;
                    delete mpGraphiteData;
                    mpGraphiteData = NULL;
                }
            }
        }
    }
#endif

    
    
    TEXTMETRICA aTextMetric;
    if( ::GetTextMetricsA( hDC, &aTextMetric ) )
        if( !(aTextMetric.tmPitchAndFamily & TMPF_TRUETYPE)
        ||   (aTextMetric.tmPitchAndFamily & TMPF_DEVICE) )
            mbDisableGlyphApi = true;

}

#if ENABLE_GRAPHITE
const gr_face* ImplWinFontData::GraphiteFace() const
{
#ifdef DEBUG
    fprintf(grLog(), "ImplWinFontData::GraphiteFace %lx has face %lx\n",
        (unsigned long)this, mpGraphiteData? mpGraphiteData->getFace(): 0);
#endif
    return (mpGraphiteData)? mpGraphiteData->getFace() : NULL;
}
#endif


bool ImplWinFontData::HasGSUBstitutions( HDC hDC ) const
{
    if( !mbGsubRead )
        ReadGsubTable( hDC );
    return !maGsubTable.empty();
}



bool ImplWinFontData::IsGSUBstituted( sal_UCS4 cChar ) const
{
    return( maGsubTable.find( cChar ) != maGsubTable.end() );
}



const ImplFontCharMap* ImplWinFontData::GetImplFontCharMap() const
{
    if( !mpUnicodeMap )
        return NULL;
    return mpUnicodeMap;
}

bool ImplWinFontData::GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    rFontCapabilities = maFontCapabilities;
    return !rFontCapabilities.maUnicodeRange.empty() || !rFontCapabilities.maCodePageRange.empty();
}



void ImplWinFontData::ReadGsubTable( HDC hDC ) const
{
    mbGsubRead = true;

    
    const DWORD GsubTag = CalcTag( "GSUB" );
    DWORD nRC = ::GetFontData( hDC, GsubTag, 0, NULL, 0 );
    if( (nRC == GDI_ERROR) || !nRC )
        return;

    
    

    
    const RawFontData aRawFontData( hDC );
    if( !aRawFontData.get() )
        return;

    
    sal_uInt32 nFaceNum = 0;
    if( !*aRawFontData.get() )  
        nFaceNum = ~0U;  

    TrueTypeFont* pTTFont = NULL;
    ::OpenTTFontBuffer( (void*)aRawFontData.get(), aRawFontData.size(), nFaceNum, &pTTFont );
    if( !pTTFont )
        return;

    
    static const sal_Unicode aGSUBCandidates[] = {
        0x0020, 0x0080, 
        0x2000, 0x2600, 
        0x3000, 0x3100, 
        0x3300, 0x3400, 
        0xFF00, 0xFFF0, 
    0 };

    for( const sal_Unicode* pPair = aGSUBCandidates; *pPair; pPair += 2 )
        for( sal_Unicode cChar = pPair[0]; cChar < pPair[1]; ++cChar )
            if( ::MapChar( pTTFont, cChar, false ) != ::MapChar( pTTFont, cChar, true ) )
                maGsubTable.insert( cChar ); 

    CloseTTFont( pTTFont );
}



void ImplWinFontData::ReadCmapTable( HDC hDC ) const
{
    if( mpUnicodeMap != NULL )
        return;

    bool bIsSymbolFont = (meWinCharSet == SYMBOL_CHARSET);
    
    const DWORD nCmapTag = CalcTag( "cmap" );
    const RawFontData aRawFontData( hDC, nCmapTag );
    
    if( aRawFontData.get() ) {
        CmapResult aResult;
        ParseCMAP( aRawFontData.get(), aRawFontData.size(), aResult );
        mbDisableGlyphApi |= aResult.mbRecoded;
        aResult.mbSymbolic = bIsSymbolFont;
        if( aResult.mnRangeCount > 0 )
            mpUnicodeMap = new ImplFontCharMap( aResult );
    }

    if( !mpUnicodeMap )
        mpUnicodeMap = ImplFontCharMap::GetDefaultMap( bIsSymbolFont );
    mpUnicodeMap->AddReference();
}

void ImplWinFontData::GetFontCapabilities( HDC hDC ) const
{
    
    if( mbFontCapabilitiesRead )
        return;

    mbFontCapabilitiesRead = true;

    
    DWORD nLength;
    const DWORD GsubTag = CalcTag( "GSUB" );
    nLength = ::GetFontData( hDC, GsubTag, 0, NULL, 0 );
    if( (nLength != GDI_ERROR) && nLength )
    {
        std::vector<unsigned char> aTable( nLength );
        unsigned char* pTable = &aTable[0];
        ::GetFontData( hDC, GsubTag, 0, pTable, nLength );
        vcl::getTTScripts(maFontCapabilities.maGSUBScriptTags, pTable, nLength);
    }

    
    const DWORD OS2Tag = CalcTag( "OS/2" );
    nLength = ::GetFontData( hDC, OS2Tag, 0, NULL, 0 );
    if( (nLength != GDI_ERROR) && nLength )
    {
        std::vector<unsigned char> aTable( nLength );
        unsigned char* pTable = &aTable[0];
        ::GetFontData( hDC, OS2Tag, 0, pTable, nLength );
        if (vcl::getTTCoverage(maFontCapabilities.maUnicodeRange, maFontCapabilities.maCodePageRange, pTable, nLength))
        {
            
            
            
            sal_uInt32 ulUnicodeRange1 = GetUInt( pTable + 42 );
            sal_uInt32 ulUnicodeRange2 = GetUInt( pTable + 46 );

            mbHasCJKSupport = (ulUnicodeRange2 & 0x2DF00000);
            mbHasKoreanRange= (ulUnicodeRange1 & 0x10000000)
                            | (ulUnicodeRange2 & 0x01100000);
            mbHasArabicSupport = (ulUnicodeRange1 & 0x00002000);
        }
    }
}



void WinSalGraphics::SetTextColor( SalColor nSalColor )
{
    COLORREF aCol = PALETTERGB( SALCOLOR_RED( nSalColor ),
                                SALCOLOR_GREEN( nSalColor ),
                                SALCOLOR_BLUE( nSalColor ) );

    if( !mbPrinter &&
        GetSalData()->mhDitherPal &&
        ImplIsSysColorEntry( nSalColor ) )
    {
        aCol = PALRGB_TO_RGB( aCol );
    }

    ::SetTextColor( getHDC(), aCol );
}



int CALLBACK SalEnumQueryFontProcExW( const ENUMLOGFONTEXW*,
                                      const NEWTEXTMETRICEXW*,
                                      DWORD, LPARAM lParam )
{
    *((bool*)(void*)lParam) = true;
    return 0;
}



void ImplGetLogFontFromFontSelect( HDC hDC,
                                   const FontSelectPattern* pFont,
                                   LOGFONTW& rLogFont,
                                   bool /*bTestVerticalAvail*/ )
{
    OUString   aName;
    if ( pFont->mpFontData )
        aName = pFont->mpFontData->GetFamilyName();
    else
        aName = pFont->GetFamilyName().getToken( 0, ';' );

    UINT nNameLen = aName.getLength();
    if ( nNameLen > (sizeof( rLogFont.lfFaceName )/sizeof( wchar_t ))-1 )
        nNameLen = (sizeof( rLogFont.lfFaceName )/sizeof( wchar_t ))-1;
    memcpy( rLogFont.lfFaceName, aName.getStr(), nNameLen*sizeof( wchar_t ) );
    rLogFont.lfFaceName[nNameLen] = 0;

    if( !pFont->mpFontData )
    {
        rLogFont.lfCharSet = pFont->IsSymbolFont() ? SYMBOL_CHARSET : DEFAULT_CHARSET;
        rLogFont.lfPitchAndFamily = ImplPitchToWin( pFont->GetPitch() )
                                  | ImplFamilyToWin( pFont->GetFamilyType() );
    }
    else
    {
        const ImplWinFontData* pWinFontData = static_cast<const ImplWinFontData*>( pFont->mpFontData );
        rLogFont.lfCharSet        = pWinFontData->GetCharSet();
        rLogFont.lfPitchAndFamily = pWinFontData->GetPitchAndFamily();
    }

    rLogFont.lfWeight          = ImplWeightToWin( pFont->GetWeight() );
    rLogFont.lfHeight          = (LONG)-pFont->mnHeight;
    rLogFont.lfWidth           = (LONG)pFont->mnWidth;
    rLogFont.lfUnderline       = 0;
    rLogFont.lfStrikeOut       = 0;
    rLogFont.lfItalic          = (pFont->GetSlant()) != ITALIC_NONE;
    rLogFont.lfEscapement      = pFont->mnOrientation;
    rLogFont.lfOrientation     = rLogFont.lfEscapement;
    rLogFont.lfClipPrecision   = CLIP_DEFAULT_PRECIS;
    rLogFont.lfQuality         = DEFAULT_QUALITY;
    rLogFont.lfOutPrecision    = OUT_TT_PRECIS;
    if ( pFont->mnOrientation )
        rLogFont.lfClipPrecision |= CLIP_LH_ANGLES;

    
    if ( pFont->mbNonAntialiased )
        rLogFont.lfQuality = NONANTIALIASED_QUALITY;

    
    if( pFont->mbVertical && nNameLen )
    {
        
        memmove( &rLogFont.lfFaceName[1], &rLogFont.lfFaceName[0],
            sizeof(rLogFont.lfFaceName)-sizeof(rLogFont.lfFaceName[0]) );
        rLogFont.lfFaceName[0] = '@';

        
        bool bAvailable = false;
        EnumFontFamiliesExW( hDC, &rLogFont, (FONTENUMPROCW)SalEnumQueryFontProcExW,
                         (LPARAM)&bAvailable, 0 );

        if( !bAvailable )
        {
            
            memcpy( &rLogFont.lfFaceName[0], aName.getStr(), nNameLen*sizeof(wchar_t) );
            if( nNameLen < LF_FACESIZE )
                rLogFont.lfFaceName[nNameLen] = '\0';
        }
    }
}



HFONT WinSalGraphics::ImplDoSetFont( FontSelectPattern* i_pFont, float& o_rFontScale, HFONT& o_rOldFont )
{
    HFONT hNewFont = 0;

    HDC hdcScreen = 0;
    if( mbVirDev )
        
        hdcScreen = GetDC(0);

    LOGFONTW aLogFont;
    ImplGetLogFontFromFontSelect( getHDC(), i_pFont, aLogFont, true );

    
    
    if( mbScreen
    &&  (i_pFont->mnWidth != 0
      || i_pFont->mnOrientation != 0
      || i_pFont->mpFontData == NULL
      || (i_pFont->mpFontData->GetHeight() != i_pFont->mnHeight))
    && !bImplSalCourierScalable
    && bImplSalCourierNew
    && (ImplSalWICompareAscii( aLogFont.lfFaceName, "Courier" ) == 0) )
        lstrcpynW( aLogFont.lfFaceName, L"Courier New", 11 );

    
    
    if( (-aLogFont.lfHeight <= MAXFONTHEIGHT)
    &&  (+aLogFont.lfWidth <= MAXFONTHEIGHT) )
    {
        o_rFontScale = 1.0;
    }
    else if( -aLogFont.lfHeight >= +aLogFont.lfWidth )
    {
        o_rFontScale = -aLogFont.lfHeight / (float)MAXFONTHEIGHT;
        aLogFont.lfHeight = -MAXFONTHEIGHT;
        aLogFont.lfWidth = FRound( aLogFont.lfWidth / o_rFontScale );
    }
    else 
    {
        o_rFontScale = +aLogFont.lfWidth / (float)MAXFONTHEIGHT;
        aLogFont.lfWidth = +MAXFONTHEIGHT;
        aLogFont.lfHeight = FRound( aLogFont.lfHeight / o_rFontScale );
    }

    hNewFont = ::CreateFontIndirectW( &aLogFont );
    if( hdcScreen )
    {
        
        
        
        SelectFont( hdcScreen, SelectFont( hdcScreen , hNewFont ) );
    }
    o_rOldFont = ::SelectFont( getHDC(), hNewFont );

    TEXTMETRICW aTextMetricW;
    if( !::GetTextMetricsW( getHDC(), &aTextMetricW ) )
    {
        
        
        lstrcpynW( aLogFont.lfFaceName, L"Courier New", 11 );
        aLogFont.lfPitchAndFamily = FIXED_PITCH;
        HFONT hNewFont2 = CreateFontIndirectW( &aLogFont );
        SelectFont( getHDC(), hNewFont2 );
        DeleteFont( hNewFont );
        hNewFont = hNewFont2;
    }

    if( hdcScreen )
        ::ReleaseDC( NULL, hdcScreen );

    return hNewFont;
}

sal_uInt16 WinSalGraphics::SetFont( FontSelectPattern* pFont, int nFallbackLevel )
{
    
    if( !pFont )
    {
        
        if( mhDefFont )
            ::SelectFont( getHDC(), mhDefFont );
        mfCurrentFontScale = mfFontScale[nFallbackLevel];
        
        for( int i = nFallbackLevel; i < MAX_FALLBACK; ++i )
        {
            if( mhFonts[i] )
                ::DeleteFont( mhFonts[i] );
            mhFonts[ i ] = 0;
        }
        mhDefFont = 0;
        return 0;
    }

    DBG_ASSERT( pFont->mpFontData, "WinSalGraphics mpFontData==NULL");
    mpWinFontEntry[ nFallbackLevel ] = reinterpret_cast<ImplWinFontEntry*>( pFont->mpFontEntry );
    mpWinFontData[ nFallbackLevel ] = static_cast<const ImplWinFontData*>( pFont->mpFontData );

    HFONT hOldFont = 0;
    HFONT hNewFont = ImplDoSetFont( pFont, mfFontScale[ nFallbackLevel ], hOldFont );
    mfCurrentFontScale = mfFontScale[nFallbackLevel];

    if( !mhDefFont )
    {
        
        mhDefFont = hOldFont;
    }
    else
    {
        
        for( int i = nFallbackLevel; i < MAX_FALLBACK; ++i )
        {
            if( mhFonts[i] )
            {
                ::DeleteFont( mhFonts[i] );
                mhFonts[i] = 0;
            }
        }
    }

    
    mhFonts[ nFallbackLevel ] = hNewFont;
    
    if( mpWinFontData[ nFallbackLevel ] )
        mpWinFontData[ nFallbackLevel ]->UpdateFromHDC( getHDC() );

    if( !nFallbackLevel )
    {
        mbFontKernInit = TRUE;
        if ( mpFontKernPairs )
        {
            delete[] mpFontKernPairs;
            mpFontKernPairs = NULL;
        }
        mnFontKernPairCount = 0;
    }

    mnFontCharSetCount = 0;

    
    
    
    if ( mbPrinter )
        return SAL_SETFONT_USEDRAWTEXTARRAY;
    else
        return 0;
}



void WinSalGraphics::GetFontMetric( ImplFontMetricData* pMetric, int nFallbackLevel )
{
    
    HFONT hOldFont = SelectFont( getHDC(), mhFonts[nFallbackLevel] );

    wchar_t aFaceName[LF_FACESIZE+60];
    if( ::GetTextFaceW( getHDC(), sizeof(aFaceName)/sizeof(wchar_t), aFaceName ) )
        pMetric->SetFamilyName(OUString(reinterpret_cast<const sal_Unicode*>(aFaceName)));

    
    TEXTMETRICA aWinMetric;
    const bool bOK = GetTextMetricsA( getHDC(), &aWinMetric );
    
    SelectFont( getHDC(), hOldFont );
    if( !bOK )
        return;

    
    pMetric->SetFamilyType(ImplFamilyToSal( aWinMetric.tmPitchAndFamily ));
    pMetric->SetSymbolFlag(aWinMetric.tmCharSet == SYMBOL_CHARSET);
    pMetric->SetWeight(ImplWeightToSal( aWinMetric.tmWeight ));
    pMetric->SetPitch(ImplMetricPitchToSal( aWinMetric.tmPitchAndFamily ));
    pMetric->SetItalic(aWinMetric.tmItalic ? ITALIC_NORMAL : ITALIC_NONE);
    pMetric->mnSlant        = 0;

    
    pMetric->mbDevice       = (aWinMetric.tmPitchAndFamily & TMPF_DEVICE) != 0;
    pMetric->mbScalableFont = (aWinMetric.tmPitchAndFamily & (TMPF_VECTOR|TMPF_TRUETYPE)) != 0;
    if( pMetric->mbScalableFont )
    {
        
        
        DWORD nKernPairs = ::GetKerningPairsA( getHDC(), 0, NULL );
        pMetric->mbKernableFont = (nKernPairs > 0);
    }
    else
    {
        
        pMetric->mnOrientation  = 0;
        
        pMetric->mbKernableFont = false;
    }

    
    pMetric->mnWidth        = static_cast<int>( mfFontScale[nFallbackLevel] * aWinMetric.tmAveCharWidth );
    pMetric->mnIntLeading   = static_cast<int>( mfFontScale[nFallbackLevel] * aWinMetric.tmInternalLeading );
    pMetric->mnExtLeading   = static_cast<int>( mfFontScale[nFallbackLevel] * aWinMetric.tmExternalLeading );
    pMetric->mnAscent       = static_cast<int>( mfFontScale[nFallbackLevel] * aWinMetric.tmAscent );
    pMetric->mnDescent      = static_cast<int>( mfFontScale[nFallbackLevel] * aWinMetric.tmDescent );

    
    
    
    if( mpWinFontData[nFallbackLevel] && mpWinFontData[nFallbackLevel]->SupportsCJK() )
    {
        pMetric->mnIntLeading += pMetric->mnExtLeading;

        
        
        
        const long nHalfTmpExtLeading = pMetric->mnExtLeading / 2;
        const long nOtherHalfTmpExtLeading = pMetric->mnExtLeading - nHalfTmpExtLeading;

        
        
        long nCJKExtLeading = static_cast<long>(0.30 * (pMetric->mnAscent + pMetric->mnDescent));
        nCJKExtLeading -= pMetric->mnExtLeading;
        pMetric->mnExtLeading = (nCJKExtLeading > 0) ? nCJKExtLeading : 0;

        pMetric->mnAscent   += nHalfTmpExtLeading;
        pMetric->mnDescent  += nOtherHalfTmpExtLeading;
    }

    pMetric->mnMinKashida = GetMinKashidaWidth();
}



sal_uLong WinSalGraphics::GetKernPairs()
{
    if ( mbFontKernInit )
    {
        if( mpFontKernPairs )
        {
            delete[] mpFontKernPairs;
            mpFontKernPairs = NULL;
        }
        mnFontKernPairCount = 0;

        KERNINGPAIR* pPairs = NULL;
        int nCount = ::GetKerningPairsW( getHDC(), 0, NULL );
        if( nCount )
        {
            pPairs = new KERNINGPAIR[ nCount+1 ];
            mpFontKernPairs = pPairs;
            mnFontKernPairCount = nCount;
            ::GetKerningPairsW( getHDC(), nCount, pPairs );
        }

        mbFontKernInit = FALSE;

        std::sort( mpFontKernPairs, mpFontKernPairs + mnFontKernPairCount, ImplCmpKernData );
    }

    return mnFontKernPairCount;
}



const ImplFontCharMap* WinSalGraphics::GetImplFontCharMap() const
{
    if( !mpWinFontData[0] )
        return ImplFontCharMap::GetDefaultMap();
    return mpWinFontData[0]->GetImplFontCharMap();
}

bool WinSalGraphics::GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    if( !mpWinFontData[0] )
        return false;
    return mpWinFontData[0]->GetImplFontCapabilities(rFontCapabilities);
}



int CALLBACK SalEnumFontsProcExA( const ENUMLOGFONTEXA* pLogFont,
                                  const NEWTEXTMETRICEXA* pMetric,
                                  DWORD nFontType, LPARAM lParam )
{
    ImplEnumInfo* pInfo = (ImplEnumInfo*)(void*)lParam;
    if ( !pInfo->mpName )
    {
        
        if ( pLogFont->elfLogFont.lfFaceName[0] != '@' )
        {
            if ( !pInfo->mbImplSalCourierNew )
                pInfo->mbImplSalCourierNew = stricmp( pLogFont->elfLogFont.lfFaceName, "Courier New" ) == 0;
            if ( !pInfo->mbImplSalCourierScalable )
                pInfo->mbCourier = stricmp( pLogFont->elfLogFont.lfFaceName, "Courier" ) == 0;
            else
                pInfo->mbCourier = FALSE;
            OUString aName( ImplSalGetUniString( pLogFont->elfLogFont.lfFaceName ) );
            pInfo->mpName = &aName;
            strncpy( pInfo->mpLogFontA->lfFaceName, pLogFont->elfLogFont.lfFaceName, LF_FACESIZE );
            pInfo->mpLogFontA->lfCharSet = pLogFont->elfLogFont.lfCharSet;
            EnumFontFamiliesExA( pInfo->mhDC, pInfo->mpLogFontA, (FONTENUMPROCA)SalEnumFontsProcExA,
                                 (LPARAM)(void*)pInfo, 0 );
            pInfo->mpLogFontA->lfFaceName[0] = '\0';
            pInfo->mpLogFontA->lfCharSet = DEFAULT_CHARSET;
            pInfo->mpName = NULL;
            pInfo->mbCourier = FALSE;
        }
    }
    else
    {
        
        if( pInfo->mbPrinter )
            if( (nFontType & RASTER_FONTTYPE) && !(nFontType & DEVICE_FONTTYPE) )
                return 1;

        ImplWinFontData* pData = ImplLogMetricToDevFontDataA( pLogFont, &(pMetric->ntmTm), nFontType );
        pData->SetFontId( sal_IntPtr( pInfo->mnFontCount++ ) );

        
        
        
        if ( pInfo->mnPreferredCharSet == pLogFont->elfLogFont.lfCharSet )
            pData->mnQuality += 100;

        
        if( pInfo->mbCourier )
            pInfo->mbImplSalCourierScalable |= pData->IsScalable();

        pInfo->mpList->Add( pData );
    }

    return 1;
}



int CALLBACK SalEnumFontsProcExW( const ENUMLOGFONTEXW* pLogFont,
                                  const NEWTEXTMETRICEXW* pMetric,
                                  DWORD nFontType, LPARAM lParam )
{
    ImplEnumInfo* pInfo = (ImplEnumInfo*)(void*)lParam;
    if ( !pInfo->mpName )
    {
        
        if ( pLogFont->elfLogFont.lfFaceName[0] != '@' )
        {
            if ( !pInfo->mbImplSalCourierNew )
                pInfo->mbImplSalCourierNew = ImplSalWICompareAscii( pLogFont->elfLogFont.lfFaceName, "Courier New" ) == 0;
            if ( !pInfo->mbImplSalCourierScalable )
                pInfo->mbCourier = ImplSalWICompareAscii( pLogFont->elfLogFont.lfFaceName, "Courier" ) == 0;
            else
                pInfo->mbCourier = FALSE;
            OUString aName = OUString(reinterpret_cast<const sal_Unicode*>(pLogFont->elfLogFont.lfFaceName));
            pInfo->mpName = &aName;
            memcpy( pInfo->mpLogFontW->lfFaceName, pLogFont->elfLogFont.lfFaceName, (aName.getLength()+1)*sizeof( wchar_t ) );
            pInfo->mpLogFontW->lfCharSet = pLogFont->elfLogFont.lfCharSet;
            EnumFontFamiliesExW( pInfo->mhDC, pInfo->mpLogFontW, (FONTENUMPROCW)SalEnumFontsProcExW,
                                 (LPARAM)(void*)pInfo, 0 );
            pInfo->mpLogFontW->lfFaceName[0] = '\0';
            pInfo->mpLogFontW->lfCharSet = DEFAULT_CHARSET;
            pInfo->mpName = NULL;
            pInfo->mbCourier = FALSE;
        }
    }
    else
    {
        
        if( pInfo->mbPrinter )
            if( (nFontType & RASTER_FONTTYPE) && !(nFontType & DEVICE_FONTTYPE) )
                return 1;

        ImplWinFontData* pData = ImplLogMetricToDevFontDataW( pLogFont, &(pMetric->ntmTm), nFontType );
        pData->SetFontId( sal_IntPtr( pInfo->mnFontCount++ ) );

        
        if( pInfo->mbCourier )
            pInfo->mbImplSalCourierScalable |= pData->IsScalable();

        pInfo->mpList->Add( pData );
    }

    return 1;
}



struct TempFontItem
{
    OUString maFontFilePath;
    OString maResourcePath;
    TempFontItem* mpNextItem;
};

#ifdef FR_PRIVATE
static int WINAPI __AddFontResourceExW( LPCWSTR lpszfileName, DWORD fl, PVOID pdv )
{
    typedef int (WINAPI *AddFontResourceExW_FUNC)(LPCWSTR, DWORD, PVOID );

    static AddFontResourceExW_FUNC  pFunc = NULL;
    static HMODULE                  hmGDI = NULL;

    if ( !pFunc && !hmGDI )
    {
        hmGDI = GetModuleHandleA( "GDI32" );
        if ( hmGDI )
            pFunc = reinterpret_cast<AddFontResourceExW_FUNC>( GetProcAddress( hmGDI, "AddFontResourceExW" ) );
    }

    if ( pFunc )
        return pFunc( lpszfileName, fl, pdv );
    else
    {
        SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
        return 0;
    }
}
#endif

bool ImplAddTempFont( SalData& rSalData, const OUString& rFontFileURL )
{
    int nRet = 0;
    OUString aUSytemPath;
    OSL_VERIFY( !osl::FileBase::getSystemPathFromFileURL( rFontFileURL, aUSytemPath ) );

#ifdef FR_PRIVATE
    nRet = __AddFontResourceExW( reinterpret_cast<LPCWSTR>(aUSytemPath.getStr()), FR_PRIVATE, NULL );
#endif

    if ( !nRet )
    {
        static int nCounter = 0;
        char aFileName[] = "soAA.fot";
        aFileName[2] = sal::static_int_cast<char>('A' + (15 & (nCounter>>4)));
        aFileName[3] = sal::static_int_cast<char>('A' + (15 & nCounter));
        char aResourceName[512];
        int nMaxLen = sizeof(aResourceName)/sizeof(*aResourceName) - 16;
        int nLen = ::GetTempPathA( nMaxLen, aResourceName );
        ::strncpy( aResourceName + nLen, aFileName, sizeof( aResourceName )- nLen );
        
        aResourceName[ (sizeof(aResourceName)/sizeof(*aResourceName))-1 ] = 0;
        ::DeleteFileA( aResourceName );

        rtl_TextEncoding theEncoding = osl_getThreadTextEncoding();
        OString aCFileName = OUStringToOString( aUSytemPath, theEncoding );
        
        if( !::CreateScalableFontResourceA( 0, aResourceName, aCFileName.getStr(), NULL ) )
            return false;
        ++nCounter;

        nRet = ::AddFontResourceA( aResourceName );
        if( nRet > 0 )
        {
            TempFontItem* pNewItem = new TempFontItem;
            pNewItem->maResourcePath = OString( aResourceName );
            pNewItem->maFontFilePath = aUSytemPath.getStr();
            pNewItem->mpNextItem = rSalData.mpTempFontItem;
            rSalData.mpTempFontItem = pNewItem;
        }
    }

    return (nRet > 0);
}



void ImplReleaseTempFonts( SalData& rSalData )
{
    int nCount = 0;
    while( TempFontItem* p = rSalData.mpTempFontItem )
    {
        ++nCount;
        if( p->maResourcePath.getLength() )
        {
            const char* pResourcePath = p->maResourcePath.getStr();
            ::RemoveFontResourceA( pResourcePath );
            ::DeleteFileA( pResourcePath );
        }
        else
        {
            ::RemoveFontResourceW( reinterpret_cast<LPCWSTR>(p->maFontFilePath.getStr()) );
        }

        rSalData.mpTempFontItem = p->mpNextItem;
        delete p;
    }

#ifndef FR_PRIVATE
    
    
    if( nCount > 0 )
        ::PostMessage( HWND_BROADCAST, WM_FONTCHANGE, 0, NULL );
#endif 
}



static bool ImplGetFontAttrFromFile( const OUString& rFontFileURL,
    ImplDevFontAttributes& rDFA )
{
    OUString aUSytemPath;
    OSL_VERIFY( !osl::FileBase::getSystemPathFromFileURL( rFontFileURL, aUSytemPath ) );

    
    
    rDFA.mnQuality    = 1000;
    rDFA.mbDevice     = true;
    rDFA.SetFamilyType(FAMILY_DONTKNOW);
    rDFA.SetWidthType(WIDTH_DONTKNOW);
    rDFA.SetWeight(WEIGHT_DONTKNOW);
    rDFA.SetItalic(ITALIC_DONTKNOW);
    rDFA.SetPitch(PITCH_DONTKNOW);
    rDFA.mbSubsettable= true;
    rDFA.mbEmbeddable = false;

    
    char aFileName[] = "soAAT.fot";
    char aResourceName[512];
    int nMaxLen = sizeof(aResourceName)/sizeof(*aResourceName) - 16;
    int nLen = ::GetTempPathA( nMaxLen, aResourceName );
    ::strncpy( aResourceName + nLen, aFileName, std::max( 0, nMaxLen - nLen ));
    ::DeleteFileA( aResourceName );

    
    rtl_TextEncoding theEncoding = osl_getThreadTextEncoding();
    OString aCFileName = OUStringToOString( aUSytemPath, theEncoding );
    ::CreateScalableFontResourceA( 0, aResourceName, aCFileName.getStr(), NULL );

    
    OUString aFotFileName = OStringToOUString( aResourceName, osl_getThreadTextEncoding() );
    osl::FileBase::getFileURLFromSystemPath( aFotFileName, aFotFileName );
    osl::File aFotFile( aFotFileName );
    osl::FileBase::RC aError = aFotFile.open( osl_File_OpenFlag_Read );
    if( aError != osl::FileBase::E_None )
        return false;

    sal_uInt64  nBytesRead = 0;
    char        aBuffer[4096];
    aFotFile.read( aBuffer, sizeof( aBuffer ), nBytesRead );
    
    aFotFile.close();
    ::DeleteFileA( aResourceName );

    
    sal_uInt64 i = 0x4F6;
    sal_uInt64 nNameOfs = i;
    while( (i < nBytesRead) && (aBuffer[i++] != 0) );
    
    while( (i < nBytesRead) && (aBuffer[i++] != 0) );
    
    int nStyleOfs = i;
    while( (i < nBytesRead) && (aBuffer[i++] != 0) );
    if( i >= nBytesRead )
        return false;

    
    char *pName = aBuffer + nNameOfs;
    rDFA.SetFamilyName(OUString(pName, strlen(pName), osl_getThreadTextEncoding()));
    char *pStyle = aBuffer + nStyleOfs;
    rDFA.SetStyleName(OUString(pStyle, strlen(pStyle), osl_getThreadTextEncoding() ));

    
    const char nFSS = aBuffer[ 0x4C7 ];
    if( nFSS & 0x01 )   
        rDFA.SetItalic(ITALIC_NORMAL);
    
    
    if( nFSS & 0x40 )   
    {
        rDFA.SetWeight(WEIGHT_NORMAL);
        rDFA.SetItalic(ITALIC_NONE);
    }

    
    int nWinWeight = (aBuffer[0x4D7] & 0xFF) + ((aBuffer[0x4D8] & 0xFF) << 8);
    rDFA.SetWeight(ImplWeightToSal( nWinWeight ));

    rDFA.SetSymbolFlag(false);          
    rDFA.SetPitch(PITCH_DONTKNOW); 

    
    rDFA.SetFamilyType(ImplFamilyToSal( aBuffer[0x4DE] ));

    
    
    
    
    

    return true;
}



bool WinSalGraphics::AddTempDevFont( ImplDevFontList* pFontList,
    const OUString& rFontFileURL, const OUString& rFontName )
{
    SAL_INFO( "vcl.gdi", "WinSalGraphics::AddTempDevFont(): " << OUStringToOString( rFontFileURL, RTL_TEXTENCODING_UTF8 ).getStr() );

    ImplDevFontAttributes aDFA;
    aDFA.SetFamilyName(rFontName);
    aDFA.mnQuality    = 1000;
    aDFA.mbDevice     = true;

    
    if( rFontName.isEmpty() && mpFontAttrCache )
        aDFA = mpFontAttrCache->GetFontAttr( rFontFileURL );

    
    if( aDFA.GetFamilyName().isEmpty() )
    {
        ImplGetFontAttrFromFile( rFontFileURL, aDFA );
        if( mpFontAttrCache && !aDFA.GetFamilyName().isEmpty() )
            mpFontAttrCache->AddFontAttr( rFontFileURL, aDFA );
    }

    if ( aDFA.GetFamilyName().isEmpty() )
        return false;

    
    if( !ImplAddTempFont( *GetSalData(), rFontFileURL ) )
        return false;

    UINT nPreferredCharSet = DEFAULT_CHARSET;

    
    aDFA.SetSymbolFlag(false); 
    aDFA.SetFamilyType(FAMILY_DONTKNOW);
    aDFA.SetWidthType(WIDTH_DONTKNOW);
    aDFA.SetWeight(WEIGHT_DONTKNOW);
    aDFA.SetItalic(ITALIC_DONTKNOW);
    aDFA.SetPitch(PITCH_DONTKNOW);
    aDFA.mbSubsettable= true;
    aDFA.mbEmbeddable = false;

    /*
    
    aDFS.maName = 
    if( rFontName != aDFS.maName )
        aDFS.maMapName = aFontName;
    */

    ImplWinFontData* pFontData = new ImplWinFontData( aDFA, 0,
        sal::static_int_cast<BYTE>(nPreferredCharSet),
        sal::static_int_cast<BYTE>(TMPF_VECTOR|TMPF_TRUETYPE) );
    pFontData->SetFontId( reinterpret_cast<sal_IntPtr>(pFontData) );
    pFontList->Add( pFontData );
    return true;
}



void WinSalGraphics::GetDevFontList( ImplDevFontList* pFontList )
{
    
    static bool bOnce = true;
    if( bOnce )
    {
        bOnce = false;

        
        
        
        
        OUString aPath;
        osl_getExecutableFile( &aPath.pData );
        aPath = aPath.copy( 0, aPath.lastIndexOf('/') );
        OUString aFontDirUrl = aPath.copy( 0, aPath.lastIndexOf('/') );
        aFontDirUrl += "/" LIBO_SHARE_FOLDER "/fonts/truetype";

        
        osl::Directory aFontDir( aFontDirUrl );
        osl::FileBase::RC rcOSL = aFontDir.open();
        if( rcOSL == osl::FileBase::E_None )
        {
            osl::DirectoryItem aDirItem;
            OUString aEmptyString;

            OUString aBootStrap;
            rtl::Bootstrap::get( OUString("BRAND_BASE_DIR"), aBootStrap );
            aBootStrap += "/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap" );
            rtl::Bootstrap aBootstrap( aBootStrap );
            OUString aUserPath;
            aBootstrap.getFrom( OUString( "UserInstallation" ), aUserPath );
            aUserPath += "/user/config/fontnames.dat";
            OUString aBaseURL = aPath.copy( 0, aPath.lastIndexOf('/')+1 );
            mpFontAttrCache = new ImplFontAttrCache( aUserPath, aBaseURL );

            while( aFontDir.getNextItem( aDirItem, 10 ) == osl::FileBase::E_None )
            {
                osl::FileStatus aFileStatus( osl_FileStatus_Mask_FileURL );
                rcOSL = aDirItem.getFileStatus( aFileStatus );
                if ( rcOSL == osl::FileBase::E_None )
                    AddTempDevFont( pFontList, aFileStatus.getFileURL(), aEmptyString );
            }

            delete mpFontAttrCache; 
            mpFontAttrCache = NULL;
        }
    }

    ImplEnumInfo aInfo;
    aInfo.mhDC          = getHDC();
    aInfo.mpList        = pFontList;
    aInfo.mpName        = NULL;
    aInfo.mpLogFontA    = NULL;
    aInfo.mpLogFontW    = NULL;
    aInfo.mbCourier     = false;
    aInfo.mbPrinter     = mbPrinter;
    aInfo.mnFontCount   = 0;
    if ( !mbPrinter )
    {
        aInfo.mbImplSalCourierScalable  = false;
        aInfo.mbImplSalCourierNew       = false;
    }
    else
    {
        aInfo.mbImplSalCourierScalable  = true;
        aInfo.mbImplSalCourierNew       = true;
    }

    aInfo.mnPreferredCharSet = DEFAULT_CHARSET;
    DWORD nCP = GetACP();
    CHARSETINFO aCharSetInfo;
    if ( TranslateCharsetInfo( (DWORD*)(sal_IntPtr)nCP, &aCharSetInfo, TCI_SRCCODEPAGE ) )
        aInfo.mnPreferredCharSet = aCharSetInfo.ciCharset;

    LOGFONTW aLogFont;
    memset( &aLogFont, 0, sizeof( aLogFont ) );
    aLogFont.lfCharSet = DEFAULT_CHARSET;
    aInfo.mpLogFontW = &aLogFont;
    EnumFontFamiliesExW( getHDC(), &aLogFont,
        (FONTENUMPROCW)SalEnumFontsProcExW, (LPARAM)(void*)&aInfo, 0 );

    
    
    if ( !mbPrinter )
    {
        bImplSalCourierScalable = aInfo.mbImplSalCourierScalable;
        bImplSalCourierNew      = aInfo.mbImplSalCourierNew;
    }

    
    static WinGlyphFallbackSubstititution aSubstFallback( getHDC() );
    pFontList->SetFallbackHook( &aSubstFallback );
}

void WinSalGraphics::ClearDevFontCache()
{
    
}



bool WinSalGraphics::GetGlyphBoundRect( sal_GlyphId aGlyphId, Rectangle& rRect )
{
    HDC hDC = getHDC();

    
    MAT2 aMat;
    aMat.eM11 = aMat.eM22 = FixedFromDouble( 1.0 );
    aMat.eM12 = aMat.eM21 = FixedFromDouble( 0.0 );

    UINT nGGOFlags = GGO_METRICS;
    if( !(aGlyphId & GF_ISCHAR) )
        nGGOFlags |= GGO_GLYPH_INDEX;
    aGlyphId &= GF_IDXMASK;

    GLYPHMETRICS aGM;
    aGM.gmptGlyphOrigin.x = aGM.gmptGlyphOrigin.y = 0;
    aGM.gmBlackBoxX = aGM.gmBlackBoxY = 0;
    DWORD nSize = ::GetGlyphOutlineW( hDC, aGlyphId, nGGOFlags, &aGM, 0, NULL, &aMat );
    if( nSize == GDI_ERROR )
        return false;

    rRect = Rectangle( Point( +aGM.gmptGlyphOrigin.x, -aGM.gmptGlyphOrigin.y ),
        Size( aGM.gmBlackBoxX, aGM.gmBlackBoxY ) );
    rRect.Left()    = static_cast<int>( mfCurrentFontScale * rRect.Left() );
    rRect.Right()   = static_cast<int>( mfCurrentFontScale * rRect.Right() );
    rRect.Top()     = static_cast<int>( mfCurrentFontScale * rRect.Top() );
    rRect.Bottom()  = static_cast<int>( mfCurrentFontScale * rRect.Bottom() );
    return true;
}



bool WinSalGraphics::GetGlyphOutline( sal_GlyphId aGlyphId,
    ::basegfx::B2DPolyPolygon& rB2DPolyPoly )
{
    rB2DPolyPoly.clear();

    HDC  hDC = getHDC();

    
    MAT2 aMat;
    aMat.eM11 = aMat.eM22 = FixedFromDouble( 1.0 );
    aMat.eM12 = aMat.eM21 = FixedFromDouble( 0.0 );

    UINT nGGOFlags = GGO_NATIVE;
    if( !(aGlyphId & GF_ISCHAR) )
        nGGOFlags |= GGO_GLYPH_INDEX;
    aGlyphId &= GF_IDXMASK;

    GLYPHMETRICS aGlyphMetrics;
    const DWORD nSize1 = ::GetGlyphOutlineW( hDC, aGlyphId, nGGOFlags, &aGlyphMetrics, 0, NULL, &aMat );
    if( !nSize1 )       
        return true;
    else if( nSize1 == GDI_ERROR )
        return false;

    BYTE* pData = new BYTE[ nSize1 ];
    const DWORD nSize2 = ::GetGlyphOutlineW( hDC, aGlyphId, nGGOFlags,
              &aGlyphMetrics, nSize1, pData, &aMat );

    if( nSize1 != nSize2 )
        return false;

    
    int     nPtSize = 512;
    Point*  pPoints = new Point[ nPtSize ];
    BYTE*   pFlags = new BYTE[ nPtSize ];

    TTPOLYGONHEADER* pHeader = (TTPOLYGONHEADER*)pData;
    while( (BYTE*)pHeader < pData+nSize2 )
    {
        
        if( pHeader->dwType != TT_POLYGON_TYPE )
            break;

        
        
        sal_uInt16 nPnt = 0;

        long nX = IntTimes256FromFixed( pHeader->pfxStart.x );
        long nY = IntTimes256FromFixed( pHeader->pfxStart.y );
        pPoints[ nPnt ] = Point( nX, nY );
        pFlags[ nPnt++ ] = POLY_NORMAL;

        bool bHasOfflinePoints = false;
        TTPOLYCURVE* pCurve = (TTPOLYCURVE*)( pHeader + 1 );
        pHeader = (TTPOLYGONHEADER*)( (BYTE*)pHeader + pHeader->cb );
        while( (BYTE*)pCurve < (BYTE*)pHeader )
        {
            int nNeededSize = nPnt + 16 + 3 * pCurve->cpfx;
            if( nPtSize < nNeededSize )
            {
                Point* pOldPoints = pPoints;
                BYTE* pOldFlags = pFlags;
                nPtSize = 2 * nNeededSize;
                pPoints = new Point[ nPtSize ];
                pFlags = new BYTE[ nPtSize ];
                for( sal_uInt16 i = 0; i < nPnt; ++i )
                {
                    pPoints[ i ] = pOldPoints[ i ];
                    pFlags[ i ] = pOldFlags[ i ];
                }
                delete[] pOldPoints;
                delete[] pOldFlags;
            }

            int i = 0;
            if( TT_PRIM_LINE == pCurve->wType )
            {
                while( i < pCurve->cpfx )
                {
                    nX = IntTimes256FromFixed( pCurve->apfx[ i ].x );
                    nY = IntTimes256FromFixed( pCurve->apfx[ i ].y );
                    ++i;
                    pPoints[ nPnt ] = Point( nX, nY );
                    pFlags[ nPnt ] = POLY_NORMAL;
                    ++nPnt;
                }
            }
            else if( TT_PRIM_QSPLINE == pCurve->wType )
            {
                bHasOfflinePoints = true;
                while( i < pCurve->cpfx )
                {
                    
                    nX = IntTimes256FromFixed( pCurve->apfx[ i ].x );
                    nY = IntTimes256FromFixed( pCurve->apfx[ i ].y );
                    ++i;
                    Point aControlP( nX, nY );

                    
                    
                    nX = pPoints[ nPnt-1 ].X() + 2 * aControlP.X();
                    nY = pPoints[ nPnt-1 ].Y() + 2 * aControlP.Y();
                    pPoints[ nPnt+0 ] = Point( (2*nX+3)/6, (2*nY+3)/6 );
                    pFlags[ nPnt+0 ] = POLY_CONTROL;

                    
                    nX = IntTimes256FromFixed( pCurve->apfx[ i ].x );
                    nY = IntTimes256FromFixed( pCurve->apfx[ i ].y );

                    if ( i+1 >= pCurve->cpfx )
                    {
                        
                        ++i;
                    }
                    else
                    {
                        
                        nX += IntTimes256FromFixed( pCurve->apfx[ i-1 ].x );
                        nY += IntTimes256FromFixed( pCurve->apfx[ i-1 ].y );
                        nX = (nX + 1) / 2;
                        nY = (nY + 1) / 2;
                        
                        
                    }

                    pPoints[ nPnt+2 ] = Point( nX, nY );
                    pFlags[ nPnt+2 ] = POLY_NORMAL;

                    
                    
                    nX = pPoints[ nPnt+2 ].X() + 2 * aControlP.X();
                    nY = pPoints[ nPnt+2 ].Y() + 2 * aControlP.Y();
                    pPoints[ nPnt+1 ] = Point( (2*nX+3)/6, (2*nY+3)/6 );
                    pFlags[ nPnt+1 ] = POLY_CONTROL;

                    nPnt += 3;
                }
            }

            
            pCurve = (TTPOLYCURVE*)&pCurve->apfx[ i ];
        }

        
        
        
        
        
        if(pPoints[nPnt - 1] != pPoints[0])
        {
            if( bHasOfflinePoints )
                pFlags[nPnt] = pFlags[0];

            pPoints[nPnt++] = pPoints[0];
        }

        
        for( int i = 0; i < nPnt; ++i )
            pPoints[i].Y() = -pPoints[i].Y();

        
        Polygon aPoly( nPnt, pPoints, (bHasOfflinePoints ? pFlags : NULL) );
        
        
        rB2DPolyPoly.append( aPoly.getB2DPolygon() );
    }

    delete[] pPoints;
    delete[] pFlags;

    delete[] pData;

    
    if( rB2DPolyPoly.count() )
    {
        const double fFactor(mfCurrentFontScale/256);
        rB2DPolyPoly.transform(basegfx::tools::createScaleB2DHomMatrix(fFactor, fFactor));
    }

    return true;
}



class ScopedFont
{
public:
    explicit ScopedFont(WinSalGraphics & rData);

    ~ScopedFont();

private:
    WinSalGraphics & m_rData;
    HFONT m_hOrigFont;
};

ScopedFont::ScopedFont(WinSalGraphics & rData): m_rData(rData)
{
    m_hOrigFont = m_rData.mhFonts[0];
    m_rData.mhFonts[0] = 0; 
}

ScopedFont::~ScopedFont()
{
    if( m_hOrigFont )
    {
        
        HFONT hTempFont = m_rData.mhFonts[0];
        m_rData.mhFonts[0] = m_hOrigFont;
        SelectObject( m_rData.getHDC(), m_hOrigFont );
        DeleteObject( hTempFont );
    }
}

class ScopedTrueTypeFont
{
public:
    inline ScopedTrueTypeFont(): m_pFont(0) {}

    ~ScopedTrueTypeFont();

    int open(void * pBuffer, sal_uInt32 nLen, sal_uInt32 nFaceNum);

    inline TrueTypeFont * get() const { return m_pFont; }

private:
    TrueTypeFont * m_pFont;
};

ScopedTrueTypeFont::~ScopedTrueTypeFont()
{
    if (m_pFont != 0)
        CloseTTFont(m_pFont);
}

int ScopedTrueTypeFont::open(void * pBuffer, sal_uInt32 nLen,
                             sal_uInt32 nFaceNum)
{
    OSL_ENSURE(m_pFont == 0, "already open");
    return OpenTTFontBuffer(pBuffer, nLen, nFaceNum, &m_pFont);
}

bool WinSalGraphics::CreateFontSubset( const OUString& rToFile,
    const PhysicalFontFace* pFont, sal_GlyphId* pGlyphIds, sal_uInt8* pEncoding,
    sal_Int32* pGlyphWidths, int nGlyphCount, FontSubsetInfo& rInfo )
{
    

    
    
    
    FontSelectPattern aIFSD( *pFont, Size(0,1000), 1000.0, 0, false );

    
    ScopedFont aOldFont(*this);
    float fScale = 1.0;
    HFONT hOldFont = 0;
    ImplDoSetFont( &aIFSD, fScale, hOldFont );

    ImplWinFontData* pWinFontData = (ImplWinFontData*)aIFSD.mpFontData;

#if OSL_DEBUG_LEVEL > 1
    
    TEXTMETRICA aWinMetric;
    if( !::GetTextMetricsA( getHDC(), &aWinMetric ) )
        return FALSE;

    DBG_ASSERT( !(aWinMetric.tmPitchAndFamily & TMPF_DEVICE), "cannot subset device font" );
    DBG_ASSERT( aWinMetric.tmPitchAndFamily & TMPF_TRUETYPE, "can only subset TT font" );
#endif

    OUString aSysPath;
    if( osl_File_E_None != osl_getSystemPathFromFileURL( rToFile.pData, &aSysPath.pData ) )
        return FALSE;
    const rtl_TextEncoding aThreadEncoding = osl_getThreadTextEncoding();
    const OString aToFile(OUStringToOString(aSysPath, aThreadEncoding));

    
    const DWORD nCffTag = CalcTag( "CFF " );
    const RawFontData aRawCffData( getHDC(), nCffTag );
    if( aRawCffData.get() )
    {
        pWinFontData->UpdateFromHDC( getHDC() );
        const ImplFontCharMap* pCharMap = pWinFontData->GetImplFontCharMap();
        pCharMap->AddReference();

        sal_GlyphId aRealGlyphIds[ 256 ];
        for( int i = 0; i < nGlyphCount; ++i )
        {
            
            
            sal_GlyphId aGlyphId = pGlyphIds[i] & GF_IDXMASK;
            if( pGlyphIds[i] & GF_ISCHAR ) 
                aGlyphId = pCharMap->GetGlyphIndex( aGlyphId );
            if( (pGlyphIds[i] & (GF_ROTMASK|GF_GSUB)) != 0) 
                {/*####*/}

            aRealGlyphIds[i] = aGlyphId;
        }

        pCharMap->DeReference(); 

        
        FILE* pOutFile = fopen( aToFile.getStr(), "wb" );
        rInfo.LoadFont( FontSubsetInfo::CFF_FONT, aRawCffData.get(), aRawCffData.size() );
        bool bRC = rInfo.CreateFontSubset( FontSubsetInfo::TYPE1_PFB, pOutFile, NULL,
                aRealGlyphIds, pEncoding, nGlyphCount, pGlyphWidths );
        fclose( pOutFile );
        return bRC;
    }

    
    const RawFontData xRawFontData( getHDC(), 0 );
    if( !xRawFontData.get() )
        return FALSE;

    
    sal_uInt32 nFaceNum = 0;
    if( !*xRawFontData.get() )  
        nFaceNum = ~0U;  

    ScopedTrueTypeFont aSftTTF;
    int nRC = aSftTTF.open( (void*)xRawFontData.get(), xRawFontData.size(), nFaceNum );
    if( nRC != SF_OK )
        return FALSE;

    TTGlobalFontInfo aTTInfo;
    ::GetTTGlobalFontInfo( aSftTTF.get(), &aTTInfo );
    rInfo.m_nFontType   = FontSubsetInfo::SFNT_TTF;
    rInfo.m_aPSName     = ImplSalGetUniString( aTTInfo.psname );
    rInfo.m_nAscent     = aTTInfo.winAscent;
    rInfo.m_nDescent    = aTTInfo.winDescent;
    rInfo.m_aFontBBox   = Rectangle( Point( aTTInfo.xMin, aTTInfo.yMin ),
                                    Point( aTTInfo.xMax, aTTInfo.yMax ) );
    rInfo.m_nCapHeight  = aTTInfo.yMax; 

    
    
    int nOrigCount = nGlyphCount;
    sal_uInt16    aShortIDs[ 256 ];
    sal_uInt8 aTempEncs[ 256 ];

    int nNotDef=-1, i;
    for( i = 0; i < nGlyphCount; ++i )
    {
        aTempEncs[i] = pEncoding[i];
        sal_GlyphId aGlyphId = pGlyphIds[i] & GF_IDXMASK;
        if( pGlyphIds[i] & GF_ISCHAR )
        {
            sal_Unicode cChar = static_cast<sal_Unicode>(aGlyphId); 
            const bool bVertical = ((pGlyphIds[i] & (GF_ROTMASK|GF_GSUB)) != 0);
            aGlyphId = ::MapChar( aSftTTF.get(), cChar, bVertical );
            if( (aGlyphId == 0) && pFont->IsSymbolFont() )
            {
                
                cChar = (cChar & 0xF000) ? (cChar & 0x00FF) : (cChar | 0xF000);
                aGlyphId = ::MapChar( aSftTTF.get(), cChar, bVertical );
            }
        }
        aShortIDs[i] = static_cast<sal_uInt16>( aGlyphId );
        if( !aGlyphId )
            if( nNotDef < 0 )
                nNotDef = i; 
    }

    if( nNotDef != 0 )
    {
        
        if( nNotDef < 0 )
            nNotDef = nGlyphCount++;

        
        aShortIDs[ nNotDef ] = aShortIDs[0];
        aTempEncs[ nNotDef ] = aTempEncs[0];
        aShortIDs[0] = 0;
        aTempEncs[0] = 0;
    }
    DBG_ASSERT( nGlyphCount < 257, "too many glyphs for subsetting" );

    
    TTSimpleGlyphMetrics* pMetrics =
        ::GetTTSimpleGlyphMetrics( aSftTTF.get(), aShortIDs, nGlyphCount, aIFSD.mbVertical );
    if( !pMetrics )
        return FALSE;
    sal_uInt16 nNotDefAdv   = pMetrics[0].adv;
    pMetrics[0].adv         = pMetrics[nNotDef].adv;
    pMetrics[nNotDef].adv   = nNotDefAdv;
    for( i = 0; i < nOrigCount; ++i )
        pGlyphWidths[i] = pMetrics[i].adv;
    free( pMetrics );

    
    nRC = ::CreateTTFromTTGlyphs( aSftTTF.get(), aToFile.getStr(), aShortIDs,
            aTempEncs, nGlyphCount, 0, NULL, 0 );
    return (nRC == SF_OK);
}


const void* WinSalGraphics::GetEmbedFontData( const PhysicalFontFace* pFont,
    const sal_Unicode* pUnicodes, sal_Int32* pCharWidths,
    FontSubsetInfo& rInfo, long* pDataLen )
{
    
    
    FontSelectPattern aIFSD( *pFont, Size(0,1000), 1000.0, 0, false );

    
    ScopedFont aOldFont(*this);
    SetFont( &aIFSD, 0 );

    
    RawFontData aRawFontData( getHDC() );
    *pDataLen = aRawFontData.size();
    if( !aRawFontData.get() )
        return NULL;

    
    TEXTMETRICA aTm;
    if( !::GetTextMetricsA( getHDC(), &aTm ) )
        *pDataLen = 0;
    const bool bPFA = (*aRawFontData.get() < 0x80);
    rInfo.m_nFontType = bPFA ? FontSubsetInfo::TYPE1_PFA : FontSubsetInfo::TYPE1_PFB;
    WCHAR aFaceName[64];
    sal_Int32 nFNLen = ::GetTextFaceW( getHDC(), 64, aFaceName );
    
    while( nFNLen > 0 && aFaceName[nFNLen-1] == 0 )
        nFNLen--;
    if( nFNLen == 0 )
        *pDataLen = 0;
    rInfo.m_aPSName     = OUString(reinterpret_cast<const sal_Unicode*>(aFaceName), nFNLen);
    rInfo.m_nAscent     = +aTm.tmAscent;
    rInfo.m_nDescent    = -aTm.tmDescent;
    rInfo.m_aFontBBox   = Rectangle( Point( -aTm.tmOverhang, -aTm.tmDescent ),
              Point( aTm.tmMaxCharWidth, aTm.tmAscent+aTm.tmExternalLeading ) );
    rInfo.m_nCapHeight  = aTm.tmAscent; 

    
    for( int i = 0; i < 256; ++i )
    {
        int nCharWidth = 0;
        const sal_Unicode cChar = pUnicodes[i];
        if( !::GetCharWidth32W( getHDC(), cChar, cChar, &nCharWidth ) )
            *pDataLen = 0;
        pCharWidths[i] = nCharWidth;
    }

    if( !*pDataLen )
        return NULL;

    const unsigned char* pData = aRawFontData.steal();
    return (void*)pData;
}


void WinSalGraphics::FreeEmbedFontData( const void* pData, long /*nLen*/ )
{
    delete[] reinterpret_cast<char*>(const_cast<void*>(pData));
}


const Ucs2SIntMap* WinSalGraphics::GetFontEncodingVector( const PhysicalFontFace* pFont, const Ucs2OStrMap** pNonEncoded )
{
    
    if( !pFont->IsEmbeddable() )
        return NULL;

    
    
    if( pNonEncoded )
        *pNonEncoded = NULL;

    const ImplWinFontData* pWinFontData = static_cast<const ImplWinFontData*>(pFont);
    const Ucs2SIntMap* pEncoding = pWinFontData->GetEncodingVector();
    if( pEncoding == NULL )
    {
        Ucs2SIntMap* pNewEncoding = new Ucs2SIntMap;
        for( sal_Unicode i = 32; i < 256; ++i )
            (*pNewEncoding)[i] = i;
        pWinFontData->SetEncodingVector( pNewEncoding );
    pEncoding = pNewEncoding;
    }

    return pEncoding;
}


void WinSalGraphics::GetGlyphWidths( const PhysicalFontFace* pFont,
                                     bool bVertical,
                                     Int32Vector& rWidths,
                                     Ucs2UIntMap& rUnicodeEnc )
{
    
    
    FontSelectPattern aIFSD( *pFont, Size(0,1000), 1000.0, 0, false );

    
    ScopedFont aOldFont(*this);

    float fScale = 0.0;
    HFONT hOldFont = 0;
    ImplDoSetFont( &aIFSD, fScale, hOldFont );

    if( pFont->IsSubsettable() )
    {
        
        const RawFontData xRawFontData( getHDC() );
        if( !xRawFontData.get() )
            return;

        
        sal_uInt32 nFaceNum = 0;
        if( !*xRawFontData.get() )  
            nFaceNum = ~0U;  

        ScopedTrueTypeFont aSftTTF;
        int nRC = aSftTTF.open( (void*)xRawFontData.get(), xRawFontData.size(), nFaceNum );
        if( nRC != SF_OK )
            return;

        int nGlyphs = GetTTGlyphCount( aSftTTF.get() );
        if( nGlyphs > 0 )
        {
            rWidths.resize(nGlyphs);
            std::vector<sal_uInt16> aGlyphIds(nGlyphs);
            for( int i = 0; i < nGlyphs; i++ )
                aGlyphIds[i] = sal_uInt16(i);
            TTSimpleGlyphMetrics* pMetrics = ::GetTTSimpleGlyphMetrics( aSftTTF.get(),
                                                                        &aGlyphIds[0],
                                                                        nGlyphs,
                                                                        bVertical );
            if( pMetrics )
            {
                for( int i = 0; i< nGlyphs; i++ )
                    rWidths[i] = pMetrics[i].adv;
                free( pMetrics );
                rUnicodeEnc.clear();
            }
            const ImplWinFontData* pWinFont = static_cast<const ImplWinFontData*>(pFont);
            const ImplFontCharMap* pMap = pWinFont->GetImplFontCharMap();
            DBG_ASSERT( pMap && pMap->GetCharCount(), "no map" );
            pMap->AddReference();

            int nCharCount = pMap->GetCharCount();
            sal_uInt32 nChar = pMap->GetFirstChar();
            for( int i = 0; i < nCharCount; i++ )
            {
                if( nChar < 0x00010000 )
                {
                    sal_uInt16 nGlyph = ::MapChar( aSftTTF.get(),
                                                   static_cast<sal_Ucs>(nChar),
                                                   bVertical );
                    if( nGlyph )
                        rUnicodeEnc[ static_cast<sal_Unicode>(nChar) ] = nGlyph;
                }
                nChar = pMap->GetNextChar( nChar );
            }

            pMap->DeReference(); 
        }
    }
    else if( pFont->IsEmbeddable() )
    {
        
        rWidths.clear();
        rUnicodeEnc.clear();
        rWidths.reserve( 224 );
        for( sal_Unicode i = 32; i < 256; ++i )
        {
            int nCharWidth = 0;
            if( ::GetCharWidth32W( getHDC(), i, i, &nCharWidth ) )
            {
                rUnicodeEnc[ i ] = rWidths.size();
                rWidths.push_back( nCharWidth );
            }
        }
    }
}


void WinSalGraphics::DrawServerFontLayout( const ServerFontLayout& )
{}


SystemFontData WinSalGraphics::GetSysFontData( int nFallbacklevel ) const
{
    SystemFontData aSysFontData;

    if (nFallbacklevel >= MAX_FALLBACK) nFallbacklevel = MAX_FALLBACK - 1;
    if (nFallbacklevel < 0 ) nFallbacklevel = 0;

    aSysFontData.hFont = mhFonts[nFallbacklevel];

    OSL_TRACE("\r\n:WinSalGraphics::GetSysFontData(): FontID: %p, Fallback level: %d",
              aSysFontData.hFont, nFallbacklevel);

    return aSysFontData;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
