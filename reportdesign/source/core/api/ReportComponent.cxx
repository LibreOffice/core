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
#include "ReportComponent.hxx"

#include "corestrings.hrc"
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontWidth.hpp>
#include "ReportControlModel.hxx"
#include <com/sun/star/reflection/XProxyFactory.hpp>
#include <com/sun/star/text/ParagraphVertAlign.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <editeng/unolingu.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/lingucfg.hxx>
#include <i18npool/mslangid.hxx>

// =============================================================================
namespace reportdesign
{
// =============================================================================
    using namespace com::sun::star;
    using namespace comphelper;

void lcl_getDefaultFonts( Font& rLatinFont, Font& rCJKFont, Font& rCTLFont,LanguageType  _eLatin,LanguageType _eCJK,LanguageType _eCTL )
{
        LanguageType eLatin = _eLatin;

        //      If the UI language is Korean, the default Latin font has to
        //      be queried for Korean, too (the Latin language from the document can't be Korean).
        //      This is the same logic as in SwDocShell::InitNew.
        LanguageType eUiLanguage = Application::GetSettings().GetUILanguage();
        if (MsLangId::isKorean(eUiLanguage))
            eLatin = eUiLanguage;

        rLatinFont = OutputDevice::GetDefaultFont( DEFAULTFONT_LATIN_PRESENTATION, eLatin, DEFAULTFONT_FLAGS_ONLYONE );
        rCJKFont = OutputDevice::GetDefaultFont( DEFAULTFONT_CJK_PRESENTATION, _eCJK, DEFAULTFONT_FLAGS_ONLYONE );
        rCTLFont = OutputDevice::GetDefaultFont( DEFAULTFONT_CTL_PRESENTATION, _eCTL, DEFAULTFONT_FLAGS_ONLYONE ) ;
}
OFormatProperties::OFormatProperties()
    :nAlign(style::ParagraphAdjust_LEFT)
    ,nFontEmphasisMark(0)
    ,nFontRelief(0)
    ,nTextColor(0)
    ,nTextLineColor(0)
    ,nCharUnderlineColor(0xFFFFFFFF)
    ,nBackgroundColor(COL_TRANSPARENT)
    ,aVerticalAlignment( style::VerticalAlignment_TOP )
    ,nCharEscapement(0)
    ,nCharCaseMap(0)
    ,nCharKerning(0)
    ,nCharEscapementHeight(100)
    ,m_bBackgroundTransparent(sal_True)
    ,bCharFlash(sal_False)
    ,bCharAutoKerning(sal_False)
    ,bCharCombineIsOn(sal_False)
    ,bCharHidden(sal_False)
    ,bCharShadowed(sal_False)
    ,bCharContoured(sal_False)
{
    try
    {
        SvtLinguConfig aLinguConfig;
        using namespace ::com::sun::star::i18n::ScriptType;

        aLinguConfig.GetProperty(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultLocale"))) >>= aCharLocale;
        LanguageType eCurLang = MsLangId::resolveSystemLanguageByScriptType(MsLangId::convertLocaleToLanguage(aCharLocale), LATIN);
        aLinguConfig.GetProperty(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultLocale_CJK")))  >>= aCharLocaleAsian;
        LanguageType eCurLangCJK = MsLangId::resolveSystemLanguageByScriptType(MsLangId::convertLocaleToLanguage(aCharLocaleAsian), ASIAN);
        aLinguConfig.GetProperty(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultLocale_CTL")))  >>= aCharLocaleComplex;
        LanguageType eCurLangCTL = MsLangId::resolveSystemLanguageByScriptType(MsLangId::convertLocaleToLanguage(aCharLocaleComplex), COMPLEX);

        Font aLatin,aCJK,aCTL;
        lcl_getDefaultFonts(aLatin,aCJK,aCTL,eCurLang,eCurLangCJK,eCurLangCTL);
        aFontDescriptor = VCLUnoHelper::CreateFontDescriptor(aLatin);
        aAsianFontDescriptor = VCLUnoHelper::CreateFontDescriptor(aCJK);
        aComplexFontDescriptor = VCLUnoHelper::CreateFontDescriptor(aCTL);
    }
    catch(const uno::Exception&)
    {
    }
    aFontDescriptor.Weight = awt::FontWeight::NORMAL;
    aFontDescriptor.CharacterWidth = awt::FontWidth::NORMAL;
}
// -----------------------------------------------------------------------------
void OReportComponentProperties::setShape(uno::Reference< drawing::XShape >& _xShape,const uno::Reference< report::XReportComponent>& _xTunnel,oslInterlockedCount& _rRefCount)
{
    osl_atomic_increment( &_rRefCount );
    {
        m_xProxy.set(_xShape,uno::UNO_QUERY);
        ::comphelper::query_aggregation(m_xProxy,m_xShape);
        ::comphelper::query_aggregation(m_xProxy,m_xProperty);
        _xShape.clear();
        m_xTypeProvider.set(m_xShape,uno::UNO_QUERY);
        m_xUnoTunnel.set(m_xShape,uno::UNO_QUERY);
        m_xServiceInfo.set(m_xShape,uno::UNO_QUERY);

        // set ourself as delegator
        if ( m_xProxy.is() )
            m_xProxy->setDelegator( _xTunnel );
    }
    osl_atomic_decrement( &_rRefCount );
}
// -----------------------------------------------------------------------------
OReportComponentProperties::~OReportComponentProperties()
{
    if ( m_xProxy.is() )
    {
        m_xProxy->setDelegator( NULL );
        m_xProxy.clear();
    }
}
// =============================================================================
} // namespace reportdesign
// =============================================================================


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
