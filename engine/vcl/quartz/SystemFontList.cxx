/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <sal/log.hxx>

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <tools/long.hxx>

#include <unordered_set>
#include <vector>


#include <quartz/SystemFontList.hxx>
#include <impfont.hxx>
#ifdef MACOSX
#include <osx/saldata.hxx>
#include <osx/salinst.h>
#endif
#include <fontattributes.hxx>
#include <font/PhysicalFontCollection.hxx>
#include <quartz/CoreTextFontFace.hxx>
#include <quartz/salgdi.h>
#include <quartz/utils.h>
#include <sallayout.hxx>


FontAttributes DevFontFromCTFontDescriptor( CTFontDescriptorRef pFD, bool* bFontEnabled )
{
    // all CoreText fonts are device fonts that can rotate just fine
    FontAttributes rDFA;
    rDFA.SetQuality( 0 );

    // reset the font attributes
    rDFA.SetFamilyType( FAMILY_DONTKNOW );
    rDFA.SetPitch( PITCH_VARIABLE );
    rDFA.SetWidthType( WIDTH_NORMAL );
    rDFA.SetWeight( WEIGHT_NORMAL );
    rDFA.SetItalic( ITALIC_NONE );
    rDFA.SetMicrosoftSymbolEncoded( false );

    // get font name
    CFStringRef pLang = nullptr;
    CFStringRef pFamilyName = static_cast<CFStringRef>(
            CTFontDescriptorCopyLocalizedAttribute( pFD, kCTFontFamilyNameAttribute, &pLang ));

    if ( !pLang )
    {
        if( pFamilyName )
        {
            CFRelease( pFamilyName );
        }
        pFamilyName = static_cast<CFStringRef>(CTFontDescriptorCopyAttribute( pFD, kCTFontFamilyNameAttribute ));
    }

    rDFA.SetFamilyName( GetOUString( pFamilyName ) );

    // get font style
    CFStringRef pStyleName = static_cast<CFStringRef>(CTFontDescriptorCopyAttribute( pFD, kCTFontStyleNameAttribute ));
    rDFA.SetStyleName( GetOUString( pStyleName ) );

    // get font-enabled status
    if( bFontEnabled )
    {
        int bEnabled = TRUE; // by default (and when we're on macOS < 10.6) it's "enabled"
        CFNumberRef pEnabled = static_cast<CFNumberRef>(CTFontDescriptorCopyAttribute( pFD, kCTFontEnabledAttribute ));
        CFNumberGetValue( pEnabled, kCFNumberIntType, &bEnabled );
        *bFontEnabled = bEnabled;
    }

    // get font attributes
    CFDictionaryRef pAttrDict = static_cast<CFDictionaryRef>(CTFontDescriptorCopyAttribute( pFD, kCTFontTraitsAttribute ));

    if (bFontEnabled && *bFontEnabled)
    {
        // Ignore font formats not supported.
        int nFormat;
        CFNumberRef pFormat = static_cast<CFNumberRef>(CTFontDescriptorCopyAttribute(pFD, kCTFontFormatAttribute));
        CFNumberGetValue(pFormat, kCFNumberIntType, &nFormat);
        if (nFormat == kCTFontFormatUnrecognized || nFormat == kCTFontFormatPostScript || nFormat == kCTFontFormatBitmap)
        {
            SAL_INFO("vcl.fonts", "Ignoring font with unsupported format: " << rDFA.GetFamilyName());
            *bFontEnabled = false;
        }
        CFRelease(pFormat);
    }

    if( !pAttrDict )
        return rDFA;

    // get symbolic trait
    // TODO: use other traits such as MonoSpace/Condensed/Expanded or Vertical too
    SInt64 nSymbolTrait = 0;
    CFNumberRef pSymbolNum = nullptr;
    if( CFDictionaryGetValueIfPresent( pAttrDict, kCTFontSymbolicTrait, reinterpret_cast<const void**>(&pSymbolNum) ) )
    {
        CFNumberGetValue( pSymbolNum, kCFNumberSInt64Type, &nSymbolTrait );
        if (nSymbolTrait & kCTFontMonoSpaceTrait)
            rDFA.SetPitch(PITCH_FIXED);
        // The italic trait marks a face italic even when the slant trait is zero, as iOS reports
        // for a font that is not installed.
        if (nSymbolTrait & kCTFontItalicTrait)
            rDFA.SetItalic(ITALIC_NORMAL);
    }

    // get the font weight
    double fWeight = 0;
    CFNumberRef pWeightNum = static_cast<CFNumberRef>(CFDictionaryGetValue( pAttrDict, kCTFontWeightTrait ));
    // tdf#140401 check if attribute is a nullptr
    if( pWeightNum )
        CFNumberGetValue( pWeightNum, kCFNumberDoubleType, &fWeight );
    int nInt = WEIGHT_NORMAL;

    // Special case fixes

    // tdf#67744: Courier Std Medium is always bold. We get a kCTFontWeightTrait of 0.23 which
    // surely must be wrong.
    if (rDFA.GetFamilyName() == "Courier Std" &&
        (rDFA.GetStyleName() == "Medium" || rDFA.GetStyleName() == "Medium Oblique") &&
        fWeight > 0.2)
    {
        fWeight = 0;
    }

    // tdf#68889: Ditto for Gill Sans MT Pro. Here I can kinda understand it, maybe the
    // kCTFontWeightTrait is intended to give a subjective "optical" impression of how the font
    // looks, and Gill Sans MT Pro Medium is kinda heavy. But with the way LibreOffice uses fonts,
    // we still should think of it as being "medium" weight.
    if (rDFA.GetFamilyName() == "Gill Sans MT Pro" &&
        (rDFA.GetStyleName() == "Medium" || rDFA.GetStyleName() == "Medium Italic") &&
        fWeight > 0.2)
    {
        fWeight = 0;
    }

    if( fWeight > 0 )
    {
        nInt = rint(int(WEIGHT_NORMAL) + fWeight * ((WEIGHT_BLACK - WEIGHT_NORMAL)/0.68));
        if( nInt > WEIGHT_BLACK )
        {
            nInt = WEIGHT_BLACK;
        }
    }
    else if( fWeight < 0 )
    {
        nInt = rint(int(WEIGHT_NORMAL) + fWeight * ((WEIGHT_NORMAL - WEIGHT_THIN)/0.8));
        if( nInt < WEIGHT_THIN )
        {
            nInt = WEIGHT_THIN;
        }
    }
    rDFA.SetWeight( static_cast<FontWeight>(nInt) );

    // get the font slant
    double fSlant = 0;
    CFNumberRef pSlantNum = static_cast<CFNumberRef>(CFDictionaryGetValue( pAttrDict, kCTFontSlantTrait ));
    // tdf#140401 check if attribute is a nullptr
    if( pSlantNum )
        CFNumberGetValue( pSlantNum, kCFNumberDoubleType, &fSlant );
    if( fSlant >= 0.035 )
    {
        rDFA.SetItalic( ITALIC_NORMAL );
    }
    // get width trait
    double fWidth = 0;
    CFNumberRef pWidthNum = static_cast<CFNumberRef>(CFDictionaryGetValue( pAttrDict, kCTFontWidthTrait ));
    // tdf#140401 check if attribute is a nullptr
    if( pWidthNum )
        CFNumberGetValue( pWidthNum, kCFNumberDoubleType, &fWidth );
    nInt = WIDTH_NORMAL;

    if( fWidth > 0 )
    {
        nInt = rint( int(WIDTH_NORMAL) + fWidth * ((WIDTH_ULTRA_EXPANDED - WIDTH_NORMAL)/0.4));
        if( nInt > WIDTH_ULTRA_EXPANDED )
        {
            nInt = WIDTH_ULTRA_EXPANDED;
        }
    }
    else if( fWidth < 0 )
    {
        nInt = rint( int(WIDTH_NORMAL) + fWidth * ((WIDTH_NORMAL - WIDTH_ULTRA_CONDENSED)/0.5));
        if( nInt < WIDTH_ULTRA_CONDENSED )
        {
            nInt = WIDTH_ULTRA_CONDENSED;
        }
    }
    rDFA.SetWidthType( static_cast<FontWidth>(nInt) );

    // release the attribute dict that we had copied
    CFRelease( pAttrDict );

    // TODO? also use the HEAD table if available to get more attributes
//  CFDataRef CTFontCopyTable( CTFontRef, kCTFontTableHead, /*kCTFontTableOptionNoOptions*/kCTFontTableOptionExcludeSynthetic );

    return rDFA;
}

static void fontEnumCallBack( const void* pValue, void* pContext )
{
    CTFontDescriptorRef pFD = static_cast<CTFontDescriptorRef>(pValue);

    // tdf#163000 don't add any fonts with an 'hvgl' font table
    // macOS Sequoia added a new PingFangUI.ttc font file which
    // contains all of the PingFang font families. However, any
    // fonts loaded from this font file result in the following
    // failures:
    // - Skia renders font with wrong glyphs
    // - Export to PDF contain a damaged embedded font
    // Despite the fact that the fonts in this new font file have
    // a TrueType font type, they are missing a 'glyf' font table
    // and, instead, have a new, undefined 'hvgl' font table. See
    // the following link for more details about the new 'hvgl'
    // font table:
    // https://gitlab.freedesktop.org/freetype/freetype/-/issues/1281
    CFNumberRef pFontFormat = static_cast<CFNumberRef>(CTFontDescriptorCopyAttribute(pFD, kCTFontFormatAttribute));
    if (pFontFormat)
    {
        bool bSkipFont = false;
        int nFontFormat;
        // At least for the PingFangUI.ttc font file, the font format is
        // different on macOS and iOS
        if (CFNumberGetValue(pFontFormat, kCFNumberIntType, &nFontFormat) && (nFontFormat == kCTFontFormatOpenTypeTrueType || nFontFormat == kCTFontFormatTrueType))
        {
            CTFontRef pFont = CTFontCreateWithFontDescriptor(pFD, 0.0, nullptr);
            if (pFont)
            {
                CFArrayRef pFontTableTags = CTFontCopyAvailableTables(pFont, kCTFontTableOptionNoOptions);
                if (pFontTableTags)
                {
                    bool bGlyfTableFound = false;
                    bool bHvglTableFound = false;
                    CFIndex nFontTableTagCount = CFArrayGetCount(pFontTableTags);
                    for (CFIndex i = 0; i < nFontTableTagCount; i++)
                    {
                        CTFontTableTag nTag = reinterpret_cast<uintptr_t>(CFArrayGetValueAtIndex(pFontTableTags, i));
                        if (nTag == kCTFontTableGlyf)
                        {
                            bGlyfTableFound = true;
                            break;
                        }
                        else if (nTag == 'hvgl')
                        {
                            bHvglTableFound = true;
                        }
                    }
                    bSkipFont = !bGlyfTableFound && bHvglTableFound;
                    CFRelease(pFontTableTags);
                }
                CFRelease(pFont);
            }
        }
        CFRelease(pFontFormat);

        if (bSkipFont)
            return;
    }

    bool bFontEnabled;
    FontAttributes rDFA = DevFontFromCTFontDescriptor( pFD, &bFontEnabled );

    if( bFontEnabled)
    {
        rtl::Reference<CoreTextFontFace> pFontData = new CoreTextFontFace( rDFA, pFD );
        SystemFontList* pFontList = static_cast<SystemFontList*>(pContext);
        pFontList->AddFont( pFontData.get() );
    }
}

SystemFontList::SystemFontList()
  : mpCTFontCollection( nullptr )
  , mpCTFontArray( nullptr )
{}

SystemFontList::~SystemFontList()
{
    maFontContainer.clear();

    if( mpCTFontArray )
    {
        CFRelease( mpCTFontArray );
    }
    if( mpCTFontCollection )
    {
        CFRelease( mpCTFontCollection );
    }
}

void SystemFontList::AddFont( CoreTextFontFace* pFontData )
{
    sal_IntPtr nFontId = pFontData->GetFontId();
    maFontContainer[ nFontId ] = pFontData;
}

void SystemFontList::AnnounceFonts( vcl::font::PhysicalFontCollection& rFontCollection ) const
{
    for(const auto& rEntry : maFontContainer )
    {
        rFontCollection.Add( rEntry.second.get() );
    }
}

CoreTextFontFace* SystemFontList::GetFontDataFromId( sal_IntPtr nFontId ) const
{
    auto it = maFontContainer.find( nFontId );
    if( it == maFontContainer.end() )
    {
        return nullptr;
    }
    return (*it).second.get();
}

bool SystemFontList::Init()
{
    // enumerate available system fonts
    static const int nMaxDictEntries = 8;
    CFMutableDictionaryRef pCFDict = CFDictionaryCreateMutable( nullptr,
                                                                nMaxDictEntries,
                                                                &kCFTypeDictionaryKeyCallBacks,
                                                                &kCFTypeDictionaryValueCallBacks );

    CFDictionaryAddValue( pCFDict, kCTFontCollectionRemoveDuplicatesOption, kCFBooleanTrue );
    mpCTFontCollection = CTFontCollectionCreateFromAvailableFonts( pCFDict );
    CFRelease( pCFDict );
    mpCTFontArray = CTFontCollectionCreateMatchingFontDescriptors( mpCTFontCollection );

    const int nFontCount = CFArrayGetCount( mpCTFontArray );
    const CFRange aFullRange = CFRangeMake( 0, nFontCount );
    CFArrayApplyFunction( mpCTFontArray, aFullRange, fontEnumCallBack, this );

#ifdef IOS
    AddBundledFonts();
#endif

    return true;
}

#ifdef IOS
namespace
{
// The faces of the files under share/fonts/truetype, made once per process. iOS refuses to
// register a bundled file when a hidden system font has the same name. A descriptor created
// from the file needs no registration.
std::vector<rtl::Reference<CoreTextFontFace>> MakeBundledFontFaces()
{
    std::vector<rtl::Reference<CoreTextFontFace>> aFaces;

    OUString aFontDirUrl( u"$BRAND_BASE_DIR/share/fonts/truetype"_ustr );
    rtl::Bootstrap::expandMacros( aFontDirUrl );

    osl::Directory aFontDir( aFontDirUrl );
    if( aFontDir.open() != osl::FileBase::E_None )
        return aFaces;

    osl::DirectoryItem aItem;
    while( aFontDir.getNextItem( aItem ) == osl::FileBase::E_None )
    {
        osl::FileStatus aStatus( osl_FileStatus_Mask_FileURL );
        if( aItem.getFileStatus( aStatus ) != osl::FileBase::E_None )
            continue;

        const OUString aFileUrl = aStatus.getFileURL();
        if( !aFileUrl.endsWithIgnoreAsciiCase( ".ttf" )
            && !aFileUrl.endsWithIgnoreAsciiCase( ".otf" )
            && !aFileUrl.endsWithIgnoreAsciiCase( ".ttc" ) )
            continue;

        OUString aSystemPath;
        if( osl::FileBase::getSystemPathFromFileURL( aFileUrl, aSystemPath )
            != osl::FileBase::E_None )
            continue;

        CFStringRef pPath = CreateCFString( aSystemPath );
        CFURLRef pUrl = CFURLCreateWithFileSystemPath( nullptr, pPath, kCFURLPOSIXPathStyle,
                                                       false );
        CFRelease( pPath );
        if( !pUrl )
            continue;

        CFArrayRef pDescriptors = CTFontManagerCreateFontDescriptorsFromURL( pUrl );
        CFRelease( pUrl );
        if( !pDescriptors )
        {
            SAL_INFO( "vcl.fonts", "No font descriptor from bundled file: " << aFileUrl );
            continue;
        }

        const CFIndex nCount = CFArrayGetCount( pDescriptors );
        for( CFIndex i = 0; i < nCount; ++i )
        {
            CTFontDescriptorRef pFD
                = static_cast<CTFontDescriptorRef>(CFArrayGetValueAtIndex( pDescriptors, i ));
            // The descriptor of the file has no weight or slant traits. A realized font has them.
            CTFontRef pFont = CTFontCreateWithFontDescriptor( pFD, 0.0, nullptr );
            if( !pFont )
                continue;
            CTFontDescriptorRef pRealized = CTFontCopyFontDescriptor( pFont );
            CFRelease( pFont );
            if( !pRealized )
                continue;
            FontAttributes aDFA = DevFontFromCTFontDescriptor( pRealized, nullptr );
            aFaces.emplace_back( new CoreTextFontFace( aDFA, pRealized ) );
            CFRelease( pRealized );
        }
        CFRelease( pDescriptors );
    }
    return aFaces;
}

const std::vector<rtl::Reference<CoreTextFontFace>>& GetBundledFontFaces()
{
    static const std::vector<rtl::Reference<CoreTextFontFace>> aFaces = MakeBundledFontFaces();
    return aFaces;
}
}

void SystemFontList::AddBundledFonts()
{
    // Skip the faces that the system enumeration already provides.
    std::unordered_set<OUString> aSeen;
    for( const auto& rEntry : maFontContainer )
        aSeen.insert( rEntry.second->GetFamilyName() + "|" + rEntry.second->GetStyleName() );

    for( const auto& rFace : GetBundledFontFaces() )
    {
        if( aSeen.insert( rFace->GetFamilyName() + "|" + rFace->GetStyleName() ).second )
            AddFont( rFace.get() );
    }
}
#endif

std::unique_ptr<SystemFontList> GetCoretextFontList()
{
    std::unique_ptr<SystemFontList> pList(new SystemFontList());
    if( !pList->Init() )
    {
        return nullptr;
    }

    return pList;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
