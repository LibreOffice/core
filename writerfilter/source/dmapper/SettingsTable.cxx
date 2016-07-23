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

#include <SettingsTable.hxx>

#include <vector>

#include <rtl/ustring.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <comphelper/sequence.hxx>
#include <ooxml/resourceids.hxx>
#include <ConversionHelper.hxx>
#include "util.hxx"

using namespace com::sun::star;

namespace writerfilter {

namespace dmapper
{

struct SettingsTable_Impl
{
    OUString     m_sCharacterSpacing;
    OUString     m_sDecimalSymbol;
    OUString     m_sListSeparatorForFields; //2.15.1.56 listSeparator (List Separator for Field Code Evaluation)

    int                 m_nDefaultTabStop;
    int                 m_nHyphenationZone;

    bool                m_bNoPunctuationKerning;
    bool                m_doNotIncludeSubdocsInStats; // Do Not Include Content in Text Boxes, Footnotes, and Endnotes in Document Statistics)
    bool                m_bRecordChanges;
    bool                m_bLinkStyles;
    sal_Int16           m_nZoomFactor;
    Id                  m_nView;
    bool                m_bEvenAndOddHeaders;
    bool                m_bUsePrinterMetrics;
    bool                embedTrueTypeFonts;
    bool                embedSystemFonts;
    bool                m_bDoNotUseHTMLParagraphAutoSpacing;
    bool                m_bNoColumnBalance;
    bool                m_bAutoHyphenation;
    bool                m_bWidowControl;
    bool                m_bSplitPgBreakAndParaMark;
    bool                m_bMirrorMargin;
    bool                m_bProtectForm;

    uno::Sequence<beans::PropertyValue> m_pThemeFontLangProps;

    std::vector<beans::PropertyValue> m_aCompatSettings;
    uno::Sequence<beans::PropertyValue> m_pCurrentCompatSetting;

    SettingsTable_Impl() :
      m_nDefaultTabStop( 720 ) //default is 1/2 in
    , m_nHyphenationZone(0)
    , m_bNoPunctuationKerning(false)
    , m_doNotIncludeSubdocsInStats(false)
    , m_bRecordChanges(false)
    , m_bLinkStyles(false)
    , m_nZoomFactor(0)
    , m_nView(0)
    , m_bEvenAndOddHeaders(false)
    , m_bUsePrinterMetrics(false)
    , embedTrueTypeFonts(false)
    , embedSystemFonts(false)
    , m_bDoNotUseHTMLParagraphAutoSpacing(false)
    , m_bNoColumnBalance(false)
    , m_bAutoHyphenation(false)
    , m_bWidowControl(false)
    , m_bSplitPgBreakAndParaMark(false)
    , m_bMirrorMargin(false)
    , m_bProtectForm(false)
    , m_pThemeFontLangProps(3)
    , m_pCurrentCompatSetting(3)
    {}

};

SettingsTable::SettingsTable()
: LoggedProperties("SettingsTable")
, LoggedTable("SettingsTable")
, m_pImpl( new SettingsTable_Impl )
{

}

SettingsTable::~SettingsTable()
{
}

void SettingsTable::lcl_attribute(Id nName, Value & val)
{
    int nIntValue = val.getInt();
    OUString sStringValue = val.getString();

    switch(nName)
    {
    case NS_ooxml::LN_CT_Zoom_percent:
        m_pImpl->m_nZoomFactor = nIntValue;
    break;
    case NS_ooxml::LN_CT_Language_val:
        m_pImpl->m_pThemeFontLangProps[0].Name = "val";
        m_pImpl->m_pThemeFontLangProps[0].Value <<= sStringValue;
        break;
    case NS_ooxml::LN_CT_Language_eastAsia:
        m_pImpl->m_pThemeFontLangProps[1].Name = "eastAsia";
        m_pImpl->m_pThemeFontLangProps[1].Value <<= sStringValue;
        break;
    case NS_ooxml::LN_CT_Language_bidi:
        m_pImpl->m_pThemeFontLangProps[2].Name = "bidi";
        m_pImpl->m_pThemeFontLangProps[2].Value <<= sStringValue;
        break;
    case NS_ooxml::LN_CT_View_val:
        m_pImpl->m_nView = nIntValue;
        break;
    case NS_ooxml::LN_CT_CompatSetting_name:
        m_pImpl->m_pCurrentCompatSetting[0].Name = "name";
        m_pImpl->m_pCurrentCompatSetting[0].Value <<= sStringValue;
        break;
    case NS_ooxml::LN_CT_CompatSetting_uri:
        m_pImpl->m_pCurrentCompatSetting[1].Name = "uri";
        m_pImpl->m_pCurrentCompatSetting[1].Value <<= sStringValue;
        break;
    case NS_ooxml::LN_CT_CompatSetting_val:
        m_pImpl->m_pCurrentCompatSetting[2].Name = "val";
        m_pImpl->m_pCurrentCompatSetting[2].Value <<= sStringValue;
        break;
    case NS_ooxml::LN_CT_DocProtect_edit:
        m_pImpl->m_bProtectForm = (nIntValue == NS_ooxml::LN_Value_doc_ST_DocProtect_forms);
        break;
    case NS_ooxml::LN_CT_DocProtect_enforcement:
        m_pImpl->m_bProtectForm &= (bool)nIntValue;
        break;
    default:
    {
#ifdef DEBUG_WRITERFILTER
        TagLogger::getInstance().element("unhandled");
#endif
    }
    }
}

void SettingsTable::lcl_sprm(Sprm& rSprm)
{
    sal_uInt32 nSprmId = rSprm.getId();

    Value::Pointer_t pValue = rSprm.getValue();
    sal_Int32 nIntValue = pValue->getInt();
    (void)nIntValue;
    OUString sStringValue = pValue->getString();

    switch(nSprmId)
    {
    case NS_ooxml::LN_CT_Settings_zoom: //  92469;
    case NS_ooxml::LN_CT_Settings_proofState: //  92489;
    case NS_ooxml::LN_CT_Settings_attachedTemplate: //  92491;
    case NS_ooxml::LN_CT_Settings_hdrShapeDefaults: //  92544;
    case NS_ooxml::LN_CT_Settings_footnotePr: //  92545;
    case NS_ooxml::LN_CT_Settings_endnotePr: //  92546;
    case NS_ooxml::LN_CT_Settings_compat: //  92547;
    case NS_ooxml::LN_CT_Settings_themeFontLang: //  92552;
    case NS_ooxml::LN_CT_Settings_shapeDefaults: //  92560;
    case NS_ooxml::LN_CT_Settings_view:
    //PropertySetValues - need to be resolved
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties.get())
            pProperties->resolve(*this);
    }
    break;
    case NS_ooxml::LN_CT_Settings_stylePaneFormatFilter: // 92493;
    break;
    case NS_ooxml::LN_CT_Settings_defaultTabStop: //  92505;
    m_pImpl->m_nDefaultTabStop = nIntValue;
    break;
    case NS_ooxml::LN_CT_Settings_linkStyles: // 92663;
    m_pImpl->m_bLinkStyles = nIntValue;
    break;
    case NS_ooxml::LN_CT_Settings_evenAndOddHeaders:
    m_pImpl->m_bEvenAndOddHeaders = nIntValue;
    break;
    case NS_ooxml::LN_CT_Settings_noPunctuationKerning: //  92526;
    m_pImpl->m_bNoPunctuationKerning = nIntValue != 0;
    break;
    case NS_ooxml::LN_CT_Settings_characterSpacingControl: //  92527;
    m_pImpl->m_sCharacterSpacing = sStringValue; // doNotCompress, compressPunctuation, compressPunctuationAndJapaneseKana
    break;
    case NS_ooxml::LN_CT_Settings_doNotIncludeSubdocsInStats: //  92554; // Do Not Include Content in Text Boxes, Footnotes, and Endnotes in Document Statistics)
    m_pImpl->m_doNotIncludeSubdocsInStats = nIntValue != 0;
    break;
    case NS_ooxml::LN_CT_Settings_decimalSymbol: //  92562;
    m_pImpl->m_sDecimalSymbol = sStringValue;
    break;
    case NS_ooxml::LN_CT_Settings_listSeparator: //  92563;
    m_pImpl->m_sListSeparatorForFields = sStringValue;
    break;
    case NS_ooxml::LN_CT_Settings_rsids: //  92549; revision save Ids - probably not necessary
    break;
    case NS_ooxml::LN_CT_Settings_hyphenationZone: // 92508;
    m_pImpl->m_nHyphenationZone = nIntValue;
    break;
    case NS_ooxml::LN_CT_Compat_useFELayout: // 92422;
    // useFELayout (Do Not Bypass East Asian/Complex Script Layout Code - support of old versions of Word - ignored)
    break;
    case NS_ooxml::LN_CT_Settings_trackRevisions:
    {
        m_pImpl->m_bRecordChanges = bool(rSprm.getValue( )->getInt( ) );
    }
    break;
    case NS_ooxml::LN_CT_Settings_documentProtection:
        {
            resolveSprmProps(*this, rSprm);
        }
        break;
    case NS_ooxml::LN_CT_Compat_usePrinterMetrics:
        m_pImpl->m_bUsePrinterMetrics = nIntValue;
        break;
    case NS_ooxml::LN_CT_Settings_embedTrueTypeFonts:
        m_pImpl->embedTrueTypeFonts = nIntValue != 0;
        break;
    case NS_ooxml::LN_CT_Settings_embedSystemFonts:
        m_pImpl->embedSystemFonts = nIntValue != 0;
        break;
    case NS_ooxml::LN_CT_Compat_doNotUseHTMLParagraphAutoSpacing:
        m_pImpl->m_bDoNotUseHTMLParagraphAutoSpacing = nIntValue;
        break;
    case NS_ooxml::LN_CT_Compat_splitPgBreakAndParaMark:
        m_pImpl->m_bSplitPgBreakAndParaMark = nIntValue;
        break;
    case NS_ooxml::LN_CT_Settings_mirrorMargins:
        m_pImpl->m_bMirrorMargin = nIntValue;
        break;
    case NS_ooxml::LN_CT_Compat_compatSetting:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties.get())
        {
            pProperties->resolve(*this);

            beans::PropertyValue aValue;
            aValue.Name = "compatSetting";
            aValue.Value = uno::makeAny(m_pImpl->m_pCurrentCompatSetting);
            m_pImpl->m_aCompatSettings.push_back(aValue);
        }
    }
    break;
    case NS_ooxml::LN_CT_Compat_noColumnBalance:
        m_pImpl->m_bNoColumnBalance = nIntValue;
        break;
    case NS_ooxml::LN_CT_Settings_autoHyphenation:
        m_pImpl->m_bAutoHyphenation = nIntValue;
        break;
    case NS_ooxml::LN_CT_Settings_widowControl:
        m_pImpl->m_bWidowControl = nIntValue;
        break;
    default:
    {
#ifdef DEBUG_WRITERFILTER
        TagLogger::getInstance().element("unhandled");
#endif
    }
    }
}

void SettingsTable::lcl_entry(int /*pos*/, writerfilter::Reference<Properties>::Pointer_t ref)
{
    ref->resolve(*this);
}

//returns default TabStop in 1/100th mm
int SettingsTable::GetDefaultTabStop() const
{
    return ConversionHelper::convertTwipToMM100( m_pImpl->m_nDefaultTabStop );
}

bool SettingsTable::GetLinkStyles() const
{
    return m_pImpl->m_bLinkStyles;
}

sal_Int16 SettingsTable::GetZoomFactor() const
{
    return m_pImpl->m_nZoomFactor;
}

Id SettingsTable::GetView() const
{
    return m_pImpl->m_nView;
}

bool SettingsTable::GetUsePrinterMetrics() const
{
    return m_pImpl->m_bUsePrinterMetrics;
}

bool SettingsTable::GetEvenAndOddHeaders() const
{
    return m_pImpl->m_bEvenAndOddHeaders;
}

bool SettingsTable::GetEmbedTrueTypeFonts() const
{
    return m_pImpl->embedTrueTypeFonts;
}

bool SettingsTable::GetEmbedSystemFonts() const
{
    return m_pImpl->embedSystemFonts;
}

bool SettingsTable::GetDoNotUseHTMLParagraphAutoSpacing() const
{
    return m_pImpl->m_bDoNotUseHTMLParagraphAutoSpacing;
}

bool SettingsTable::GetNoColumnBalance() const
{
    return m_pImpl->m_bNoColumnBalance;
}

bool SettingsTable::GetSplitPgBreakAndParaMark() const
{
    return m_pImpl->m_bSplitPgBreakAndParaMark;
}

bool SettingsTable::GetMirrorMarginSettings() const
{
    return m_pImpl->m_bMirrorMargin;
}

bool SettingsTable::GetProtectForm() const
{
    return m_pImpl->m_bProtectForm;
}
uno::Sequence<beans::PropertyValue> SettingsTable::GetThemeFontLangProperties() const
{
    return m_pImpl->m_pThemeFontLangProps;
}

uno::Sequence<beans::PropertyValue> SettingsTable::GetCompatSettings() const
{
    return comphelper::containerToSequence(m_pImpl->m_aCompatSettings);
}

static bool lcl_isDefault(const uno::Reference<beans::XPropertyState>& xPropertyState, const OUString& rPropertyName)
{
    return xPropertyState->getPropertyState(rPropertyName) == beans::PropertyState_DEFAULT_VALUE;
}

void SettingsTable::ApplyProperties(uno::Reference<text::XTextDocument> const& xDoc)
{
    uno::Reference< beans::XPropertySet> xDocProps( xDoc, uno::UNO_QUERY );

    // Record changes value
    if (xDocProps.is())
        xDocProps->setPropertyValue("RecordChanges", uno::makeAny( m_pImpl->m_bRecordChanges ) );

    // Auto hyphenation: turns on hyphenation by default, <w:suppressAutoHyphens/> may still disable it at a paragraph level.
    // Situation is similar for RTF_WIDOWCTRL, which turns on widow / orphan control by default.
    if (m_pImpl->m_bAutoHyphenation || m_pImpl->m_bWidowControl)
    {
        uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xDoc, uno::UNO_QUERY);
        if (!xStyleFamiliesSupplier.is())
            return;

        uno::Reference<container::XNameAccess> xStyleFamilies = xStyleFamiliesSupplier->getStyleFamilies();
        uno::Reference<container::XNameContainer> xParagraphStyles = xStyleFamilies->getByName("ParagraphStyles").get< uno::Reference<container::XNameContainer> >();
        uno::Reference<style::XStyle> xDefault = xParagraphStyles->getByName("Standard").get< uno::Reference<style::XStyle> >();
        uno::Reference<beans::XPropertyState> xPropertyState(xDefault, uno::UNO_QUERY);
        if (m_pImpl->m_bAutoHyphenation && lcl_isDefault(xPropertyState, "ParaIsHyphenation"))
        {
            uno::Reference<beans::XPropertySet> xPropertySet(xDefault, uno::UNO_QUERY);
            xPropertySet->setPropertyValue("ParaIsHyphenation", uno::makeAny(true));
        }
        if (m_pImpl->m_bWidowControl && lcl_isDefault(xPropertyState, "ParaWidows") && lcl_isDefault(xPropertyState, "ParaOrphans"))
        {
            uno::Reference<beans::XPropertySet> xPropertySet(xDefault, uno::UNO_QUERY);
            uno::Any aAny = uno::makeAny(static_cast<sal_Int8>(2));
            xPropertySet->setPropertyValue("ParaWidows", aAny);
            xPropertySet->setPropertyValue("ParaOrphans", aAny);
        }
    }
}

}//namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
