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
#include "ReportComponent.hxx"

#include "corestrings.hrc"
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/debug.hxx>
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
        switch( eUiLanguage )
        {
                case LANGUAGE_KOREAN:
                case LANGUAGE_KOREAN_JOHAB:
                        eLatin = eUiLanguage;
                break;
        }

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
    osl_incrementInterlockedCount( &_rRefCount );
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
    osl_decrementInterlockedCount( &_rRefCount );
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
