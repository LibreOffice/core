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


#include <fontcfg.hxx>
#include <i18npool/mslangid.hxx>
#include <vcl/outdev.hxx>
#include <unotools/lingucfg.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <swtypes.hxx>

#include <unomid.h>

using namespace utl;
using rtl::OUString;
using namespace com::sun::star::uno;

static inline LanguageType lcl_LanguageOfType(sal_Int16 nType, sal_Int16 eWestern, sal_Int16 eCJK, sal_Int16 eCTL)
{
    return LanguageType(
                nType < FONT_STANDARD_CJK ? eWestern :
                    nType >= FONT_STANDARD_CTL ? eCTL : eCJK);
}

Sequence<OUString> SwStdFontConfig::GetPropertyNames()
{
    Sequence<OUString> aNames;
    if(!aNames.getLength())
    {
        static const char* aPropNames[] =
        {
            "DefaultFont/Standard",    // 0
            "DefaultFont/Heading",     // 1
            "DefaultFont/List",        // 2
            "DefaultFont/Caption",     // 3
            "DefaultFont/Index",       // 4
            "DefaultFontCJK/Standard", // 5
            "DefaultFontCJK/Heading",  // 6
            "DefaultFontCJK/List",     // 7
            "DefaultFontCJK/Caption",  // 8
            "DefaultFontCJK/Index",    // 9
            "DefaultFontCTL/Standard", // 10
            "DefaultFontCTL/Heading",  // 11
            "DefaultFontCTL/List",     // 12
            "DefaultFontCTL/Caption",  // 13
            "DefaultFontCTL/Index",    // 14
            "DefaultFont/StandardHeight",    // 15
            "DefaultFont/HeadingHeight",     // 16
            "DefaultFont/ListHeight",        // 17
            "DefaultFont/CaptionHeight",     // 18
            "DefaultFont/IndexHeight",       // 19
            "DefaultFontCJK/StandardHeight", // 20
            "DefaultFontCJK/HeadingHeight",  // 21
            "DefaultFontCJK/ListHeight",     // 22
            "DefaultFontCJK/CaptionHeight",  // 23
            "DefaultFontCJK/IndexHeight",    // 24
            "DefaultFontCTL/StandardHeight", // 25
            "DefaultFontCTL/HeadingHeight",  // 26
            "DefaultFontCTL/ListHeight",     // 27
            "DefaultFontCTL/CaptionHeight",  // 28
            "DefaultFontCTL/IndexHeight"     // 29
        };
        const int nCount = sizeof(aPropNames)/sizeof(const char*);
        aNames.realloc(nCount);
        OUString* pNames = aNames.getArray();
        for(int i = 0; i < nCount; i++)
        {
            pNames[i] = OUString::createFromAscii(aPropNames[i]);
        }
    }
    return aNames;
}

SwStdFontConfig::SwStdFontConfig() :
    utl::ConfigItem(C2U("Office.Writer"))
{
    SvtLinguOptions aLinguOpt;

    SvtLinguConfig().GetOptions( aLinguOpt );

    sal_Int16   eWestern = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage, ::com::sun::star::i18n::ScriptType::LATIN),
                eCJK = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CJK, ::com::sun::star::i18n::ScriptType::ASIAN),
                eCTL = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CTL, ::com::sun::star::i18n::ScriptType::COMPLEX);

    for(sal_Int16 i = 0; i < DEF_FONT_COUNT; i++)
    {
        sDefaultFonts[i] = GetDefaultFor(i,
            lcl_LanguageOfType(i, eWestern, eCJK, eCTL));
        nDefaultFontHeight[i] = -1;
    }

    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                if( nProp < DEF_FONT_COUNT)
                {
                    OUString sVal;
                    pValues[nProp] >>= sVal;
                    sDefaultFonts[nProp] = sVal;
                }
                else
                {
                   pValues[nProp] >>= nDefaultFontHeight[nProp - DEF_FONT_COUNT];
                   nDefaultFontHeight[nProp - DEF_FONT_COUNT] = MM100_TO_TWIP(nDefaultFontHeight[nProp - DEF_FONT_COUNT]);
                }
            }
        }
    }
}

void    SwStdFontConfig::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();
    SvtLinguOptions aLinguOpt;

    SvtLinguConfig().GetOptions( aLinguOpt );

    sal_Int16   eWestern = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage, ::com::sun::star::i18n::ScriptType::LATIN),
                eCJK = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CJK, ::com::sun::star::i18n::ScriptType::ASIAN),
                eCTL = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CTL, ::com::sun::star::i18n::ScriptType::COMPLEX);

    for(sal_uInt16 nProp = 0;
        nProp < sal::static_int_cast< sal_uInt16, sal_Int32 >( aNames.getLength() );
            nProp++)
    {
        if( nProp < DEF_FONT_COUNT )
        {
            if(GetDefaultFor(nProp, lcl_LanguageOfType(nProp, eWestern, eCJK, eCTL)) != sDefaultFonts[nProp])
                pValues[nProp] <<= OUString(sDefaultFonts[nProp]);
        }
        else
        {
            if(nDefaultFontHeight[nProp - DEF_FONT_COUNT] > 0)
                pValues[nProp] <<= static_cast<sal_Int32>(TWIP_TO_MM100(nDefaultFontHeight[nProp - DEF_FONT_COUNT]));
        }
    }
    PutProperties(aNames, aValues);
}

SwStdFontConfig::~SwStdFontConfig()
{
}

sal_Bool SwStdFontConfig::IsFontDefault(sal_uInt16 nFontType) const
{
    sal_Bool bSame = sal_False;
    SvtLinguOptions aLinguOpt;

    SvtLinguConfig().GetOptions( aLinguOpt );

    sal_Int16   eWestern = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage, ::com::sun::star::i18n::ScriptType::LATIN),
                eCJK = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CJK, ::com::sun::star::i18n::ScriptType::ASIAN),
                eCTL = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CTL, ::com::sun::star::i18n::ScriptType::COMPLEX);

    String sDefFont(GetDefaultFor(FONT_STANDARD, eWestern));
    String sDefFontCJK(GetDefaultFor(FONT_STANDARD_CJK, eCJK));
    String sDefFontCTL(GetDefaultFor(FONT_STANDARD_CTL, eCTL));
    LanguageType eLang = lcl_LanguageOfType(nFontType, eWestern, eCJK, eCTL);
    switch( nFontType )
    {
        case FONT_STANDARD:
            bSame = sDefaultFonts[nFontType] == sDefFont;
        break;
        case FONT_STANDARD_CJK:
            bSame = sDefaultFonts[nFontType] == sDefFontCJK;
        break;
        case FONT_STANDARD_CTL:
            bSame = sDefaultFonts[nFontType] == sDefFontCTL;
        break;
        case FONT_OUTLINE :
        case FONT_OUTLINE_CJK :
        case FONT_OUTLINE_CTL :
            bSame = sDefaultFonts[nFontType] ==
                GetDefaultFor(nFontType, eLang);
        break;
        case FONT_LIST    :
        case FONT_CAPTION :
        case FONT_INDEX   :
            bSame = sDefaultFonts[nFontType] == sDefFont &&
                    sDefaultFonts[FONT_STANDARD] == sDefFont;
        break;
        case FONT_LIST_CJK    :
        case FONT_CAPTION_CJK :
        case FONT_INDEX_CJK   :
        {
            sal_Bool b1 = sDefaultFonts[FONT_STANDARD_CJK] == sDefFontCJK;
            bSame = b1 && sDefaultFonts[nFontType] == sDefFontCJK;
        }
        break;
        case FONT_LIST_CTL    :
        case FONT_CAPTION_CTL :
        case FONT_INDEX_CTL   :
        {
            sal_Bool b1 = sDefaultFonts[FONT_STANDARD_CJK] == sDefFontCTL;
            bSame = b1 && sDefaultFonts[nFontType] == sDefFontCTL;
        }
        break;
    }
    return bSame;
}

String  SwStdFontConfig::GetDefaultFor(sal_uInt16 nFontType, LanguageType eLang)
{
    String sRet;
    sal_uInt16 nFontId;
    switch( nFontType )
    {
        case FONT_OUTLINE :
            nFontId = DEFAULTFONT_LATIN_HEADING;
        break;
        case FONT_OUTLINE_CJK :
            nFontId = DEFAULTFONT_CJK_HEADING;
        break;
        case FONT_OUTLINE_CTL :
            nFontId = DEFAULTFONT_CTL_HEADING;
        break;
        case FONT_STANDARD_CJK:
        case FONT_LIST_CJK    :
        case FONT_CAPTION_CJK :
        case FONT_INDEX_CJK   :
            nFontId = DEFAULTFONT_CJK_TEXT;
        break;
        case FONT_STANDARD_CTL:
        case FONT_LIST_CTL    :
        case FONT_CAPTION_CTL :
        case FONT_INDEX_CTL   :
            nFontId = DEFAULTFONT_CTL_TEXT;
        break;
        default:
            nFontId = DEFAULTFONT_LATIN_TEXT;
    }
    Font aFont = OutputDevice::GetDefaultFont(nFontId, eLang, DEFAULTFONT_FLAGS_ONLYONE);
    return  aFont.GetName();
}

sal_Int32 SwStdFontConfig::GetDefaultHeightFor(sal_uInt16 nFontType, LanguageType eLang)
{
    sal_Int32 nRet = FONTSIZE_DEFAULT;
    switch( nFontType )
    {
        case  FONT_OUTLINE:
        case  FONT_OUTLINE_CJK:
        case  FONT_OUTLINE_CTL:
            nRet = FONTSIZE_OUTLINE;
            break;
        case FONT_STANDARD_CJK:
            nRet = FONTSIZE_CJK_DEFAULT;
            break;
    }
    if( eLang == LANGUAGE_THAI && nFontType >= FONT_STANDARD_CTL )
    {
        nRet = nRet * 4 / 3;
    }
    return nRet;
}

void SwStdFontConfig::ChangeInt( sal_uInt16 nFontType, sal_Int32 nHeight )
{
    OSL_ENSURE( nFontType < DEF_FONT_COUNT, "invalid index in SwStdFontConfig::ChangInt()");
    if( nFontType < DEF_FONT_COUNT && nDefaultFontHeight[nFontType] != nHeight)
    {
        SvtLinguOptions aLinguOpt;
        SvtLinguConfig().GetOptions( aLinguOpt );

        sal_Int16 eWestern = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage, ::com::sun::star::i18n::ScriptType::LATIN),
                  eCJK = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CJK, ::com::sun::star::i18n::ScriptType::ASIAN),
                  eCTL = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CTL, ::com::sun::star::i18n::ScriptType::COMPLEX);

        // #i92090# default height value sets back to -1
        const sal_Int32 nDefaultHeight = GetDefaultHeightFor(nFontType, lcl_LanguageOfType(nFontType, eWestern, eCJK, eCTL));
        const bool bIsDefaultHeight = nHeight == nDefaultHeight;
        if( bIsDefaultHeight && nDefaultFontHeight[nFontType] > 0 )
        {
            SetModified();
            nDefaultFontHeight[nFontType] = -1;
        }
        else if( !bIsDefaultHeight && nHeight != nDefaultFontHeight[nFontType] )
        {
            SetModified();
            nDefaultFontHeight[nFontType] = nHeight;
        }
    }
}

sal_Int32 SwStdFontConfig::GetFontHeight( sal_uInt8 nFont, sal_uInt8 nScriptType, LanguageType eLang )
{
    OSL_ENSURE(nFont + FONT_PER_GROUP * nScriptType < DEF_FONT_COUNT, "wrong index in SwStdFontConfig::GetFontHeight()");
    sal_Int32 nRet = nDefaultFontHeight[nFont + FONT_PER_GROUP * nScriptType];
    if(nRet <= 0)
        return GetDefaultHeightFor(nFont + FONT_PER_GROUP * nScriptType, eLang);
    return nRet;
}

void SwStdFontConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
