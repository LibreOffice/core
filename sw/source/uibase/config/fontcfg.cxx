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

#include <fontcfg.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <osl/diagnose.h>
#include <tools/UnitConversion.hxx>
#include <vcl/outdev.hxx>
#include <comphelper/configuration.hxx>
#include <unotools/lingucfg.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>

using namespace utl;
using namespace com::sun::star::uno;

static LanguageType lcl_LanguageOfType(sal_Int16 nType, LanguageType eWestern, LanguageType eCJK, LanguageType eCTL)
{
    return nType < FONT_STANDARD_CJK
           ? eWestern
           : nType >= FONT_STANDARD_CTL ? eCTL : eCJK;
}

Sequence<OUString> const & SwStdFontConfig::GetPropertyNames()
{
    static Sequence<OUString> aNames {
        u"DefaultFont/Standard"_ustr,    // 0
        u"DefaultFont/Heading"_ustr,     // 1
        u"DefaultFont/List"_ustr,        // 2
        u"DefaultFont/Caption"_ustr,     // 3
        u"DefaultFont/Index"_ustr,       // 4
        u"DefaultFontCJK/Standard"_ustr, // 5
        u"DefaultFontCJK/Heading"_ustr,  // 6
        u"DefaultFontCJK/List"_ustr,     // 7
        u"DefaultFontCJK/Caption"_ustr,  // 8
        u"DefaultFontCJK/Index"_ustr,    // 9
        u"DefaultFontCTL/Standard"_ustr, // 10
        u"DefaultFontCTL/Heading"_ustr,  // 11
        u"DefaultFontCTL/List"_ustr,     // 12
        u"DefaultFontCTL/Caption"_ustr,  // 13
        u"DefaultFontCTL/Index"_ustr,    // 14
        u"DefaultFont/StandardHeight"_ustr,    // 15
        u"DefaultFont/HeadingHeight"_ustr,     // 16
        u"DefaultFont/ListHeight"_ustr,        // 17
        u"DefaultFont/CaptionHeight"_ustr,     // 18
        u"DefaultFont/IndexHeight"_ustr,       // 19
        u"DefaultFontCJK/StandardHeight"_ustr, // 20
        u"DefaultFontCJK/HeadingHeight"_ustr,  // 21
        u"DefaultFontCJK/ListHeight"_ustr,     // 22
        u"DefaultFontCJK/CaptionHeight"_ustr,  // 23
        u"DefaultFontCJK/IndexHeight"_ustr,    // 24
        u"DefaultFontCTL/StandardHeight"_ustr, // 25
        u"DefaultFontCTL/HeadingHeight"_ustr,  // 26
        u"DefaultFontCTL/ListHeight"_ustr,     // 27
        u"DefaultFontCTL/CaptionHeight"_ustr,  // 28
        u"DefaultFontCTL/IndexHeight"_ustr     // 29
    };
    return aNames;
}

SwStdFontConfig::SwStdFontConfig() :
    utl::ConfigItem(u"Office.Writer"_ustr)
{
    SvtLinguOptions aLinguOpt;

    if (!comphelper::IsFuzzing())
        SvtLinguConfig().GetOptions( aLinguOpt );

    LanguageType eWestern = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage, css::i18n::ScriptType::LATIN),
                 eCJK = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CJK, css::i18n::ScriptType::ASIAN),
                 eCTL = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CTL, css::i18n::ScriptType::COMPLEX);

    for(sal_Int16 i = 0; i < DEF_FONT_COUNT; i++)
    {
        m_sDefaultFonts[i] = GetDefaultFor(i,
            lcl_LanguageOfType(i, eWestern, eCJK, eCTL));
        m_nDefaultFontHeight[i] = -1;
    }

    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() != aNames.getLength())
        return;

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        if(pValues[nProp].hasValue())
        {
            if( nProp < DEF_FONT_COUNT)
            {
                OUString sVal;
                pValues[nProp] >>= sVal;
                m_sDefaultFonts[nProp] = sVal;
            }
            else
            {
               pValues[nProp] >>= m_nDefaultFontHeight[nProp - DEF_FONT_COUNT];
               m_nDefaultFontHeight[nProp - DEF_FONT_COUNT] = o3tl::toTwips(m_nDefaultFontHeight[nProp - DEF_FONT_COUNT], o3tl::Length::mm100);
            }
        }
    }
}

void SwStdFontConfig::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();
    SvtLinguOptions aLinguOpt;

    SvtLinguConfig().GetOptions( aLinguOpt );

    LanguageType eWestern = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage, css::i18n::ScriptType::LATIN),
                 eCJK = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CJK, css::i18n::ScriptType::ASIAN),
                 eCTL = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CTL, css::i18n::ScriptType::COMPLEX);

    for(sal_uInt16 nProp = 0;
        nProp < sal::static_int_cast< sal_uInt16, sal_Int32 >( aNames.getLength() );
            nProp++)
    {
        if( nProp < DEF_FONT_COUNT )
        {
            if(GetDefaultFor(nProp, lcl_LanguageOfType(nProp, eWestern, eCJK, eCTL)) != m_sDefaultFonts[nProp])
                pValues[nProp] <<= m_sDefaultFonts[nProp];
        }
        else
        {
            if(m_nDefaultFontHeight[nProp - DEF_FONT_COUNT] > 0)
                pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(m_nDefaultFontHeight[nProp - DEF_FONT_COUNT]));
        }
    }
    PutProperties(aNames, aValues);
}

SwStdFontConfig::~SwStdFontConfig()
{
}

bool SwStdFontConfig::IsFontDefault(sal_uInt16 nFontType) const
{
    bool bSame = false;
    SvtLinguOptions aLinguOpt;

    if (!comphelper::IsFuzzing())
        SvtLinguConfig().GetOptions(aLinguOpt);

    LanguageType eWestern = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage, css::i18n::ScriptType::LATIN),
                 eCJK = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CJK, css::i18n::ScriptType::ASIAN),
                 eCTL = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CTL, css::i18n::ScriptType::COMPLEX);

    OUString sDefFont(GetDefaultFor(FONT_STANDARD, eWestern));
    OUString sDefFontCJK(GetDefaultFor(FONT_STANDARD_CJK, eCJK));
    OUString sDefFontCTL(GetDefaultFor(FONT_STANDARD_CTL, eCTL));
    LanguageType eLang = lcl_LanguageOfType(nFontType, eWestern, eCJK, eCTL);
    switch( nFontType )
    {
        case FONT_STANDARD:
            bSame = m_sDefaultFonts[nFontType] == sDefFont;
        break;
        case FONT_STANDARD_CJK:
            bSame = m_sDefaultFonts[nFontType] == sDefFontCJK;
        break;
        case FONT_STANDARD_CTL:
            bSame = m_sDefaultFonts[nFontType] == sDefFontCTL;
        break;
        case FONT_OUTLINE :
        case FONT_OUTLINE_CJK :
        case FONT_OUTLINE_CTL :
            bSame = m_sDefaultFonts[nFontType] ==
                GetDefaultFor(nFontType, eLang);
        break;
        case FONT_LIST    :
        case FONT_CAPTION :
        case FONT_INDEX   :
            bSame = m_sDefaultFonts[nFontType] == sDefFont &&
                    m_sDefaultFonts[FONT_STANDARD] == sDefFont;
        break;
        case FONT_LIST_CJK    :
        case FONT_CAPTION_CJK :
        case FONT_INDEX_CJK   :
        {
            bool b1 = m_sDefaultFonts[FONT_STANDARD_CJK] == sDefFontCJK;
            bSame = b1 && m_sDefaultFonts[nFontType] == sDefFontCJK;
        }
        break;
        case FONT_LIST_CTL    :
        case FONT_CAPTION_CTL :
        case FONT_INDEX_CTL   :
        {
            bool b1 = m_sDefaultFonts[FONT_STANDARD_CJK] == sDefFontCTL;
            bSame = b1 && m_sDefaultFonts[nFontType] == sDefFontCTL;
        }
        break;
    }
    return bSame;
}

OUString  SwStdFontConfig::GetDefaultFor(sal_uInt16 nFontType, LanguageType eLang)
{
    DefaultFontType nFontId;
    switch( nFontType )
    {
        case FONT_OUTLINE :
            nFontId = DefaultFontType::LATIN_HEADING;
        break;
        case FONT_OUTLINE_CJK :
            nFontId = DefaultFontType::CJK_HEADING;
        break;
        case FONT_OUTLINE_CTL :
            nFontId = DefaultFontType::CTL_HEADING;
        break;
        case FONT_STANDARD_CJK:
        case FONT_LIST_CJK    :
        case FONT_CAPTION_CJK :
        case FONT_INDEX_CJK   :
            nFontId = DefaultFontType::CJK_TEXT;
        break;
        case FONT_STANDARD_CTL:
        case FONT_LIST_CTL    :
        case FONT_CAPTION_CTL :
        case FONT_INDEX_CTL   :
            nFontId = DefaultFontType::CTL_TEXT;
        break;
        default:
            nFontId = DefaultFontType::LATIN_TEXT;
    }
    vcl::Font aFont = OutputDevice::GetDefaultFont(nFontId, eLang, GetDefaultFontFlags::OnlyOne);
    return  aFont.GetFamilyName();
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
    if( eLang == LANGUAGE_KOREAN )
    {
        nRet = FONTSIZE_KOREAN_DEFAULT;
    }
    return nRet;
}

void SwStdFontConfig::ChangeInt( sal_uInt16 nFontType, sal_Int32 nHeight )
{
    OSL_ENSURE( nFontType < DEF_FONT_COUNT, "invalid index in SwStdFontConfig::ChangeInt()");
    if( nFontType >= DEF_FONT_COUNT || m_nDefaultFontHeight[nFontType] == nHeight)
        return;

    SvtLinguOptions aLinguOpt;
    if (!comphelper::IsFuzzing())
        SvtLinguConfig().GetOptions( aLinguOpt );

    LanguageType eWestern = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage, css::i18n::ScriptType::LATIN),
                 eCJK = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CJK, css::i18n::ScriptType::ASIAN),
                 eCTL = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CTL, css::i18n::ScriptType::COMPLEX);

    // #i92090# default height value sets back to -1
    const sal_Int32 nDefaultHeight = GetDefaultHeightFor(nFontType, lcl_LanguageOfType(nFontType, eWestern, eCJK, eCTL));
    const bool bIsDefaultHeight = nHeight == nDefaultHeight;
    if( bIsDefaultHeight && m_nDefaultFontHeight[nFontType] > 0 )
    {
        SetModified();
        m_nDefaultFontHeight[nFontType] = -1;
    }
    else if( !bIsDefaultHeight && nHeight != m_nDefaultFontHeight[nFontType] )
    {
        SetModified();
        m_nDefaultFontHeight[nFontType] = nHeight;
    }
}

sal_Int32 SwStdFontConfig::GetFontHeight( sal_uInt8 nFont, sal_uInt8 nScriptType, LanguageType eLang )
{
    OSL_ENSURE(nFont + FONT_PER_GROUP * nScriptType < DEF_FONT_COUNT, "wrong index in SwStdFontConfig::GetFontHeight()");
    sal_Int32 nRet = m_nDefaultFontHeight[nFont + FONT_PER_GROUP * nScriptType];
    if(nRet <= 0)
        return GetDefaultHeightFor(nFont + FONT_PER_GROUP * nScriptType, eLang);
    return nRet;
}

void SwStdFontConfig::Notify( const css::uno::Sequence< OUString >& ) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
