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

#include "SettingsTable.hxx"
#include "DocumentProtection.hxx"
#include "TagLogger.hxx"
#include "WriteProtection.hxx"

#include <vector>

#include <rtl/ustring.hxx>
#include <sfx2/zoomitem.hxx>
#include <com/sun/star/text/ParagraphHyphenationKeepType.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>

#include <ooxml/OOXMLPropertySet.hxx>
#include "ConversionHelper.hxx"
#include "DomainMapper.hxx"
#include "util.hxx"
#include <SwXDocumentSettings.hxx>
#include <unotxdoc.hxx>
#include <unostyle.hxx>
#include <unofield.hxx>
#include <unofieldcoll.hxx>

using namespace com::sun::star;

namespace writerfilter {
namespace
{
/// Maps OOXML <w:zoom w:val="..."> to SvxZoomType.
sal_Int16 lcl_GetZoomType(Id nType)
{
    switch (nType)
    {
        case NS_ooxml::LN_Value_doc_ST_Zoom_fullPage:
            return sal_Int16(SvxZoomType::WHOLEPAGE);
        case NS_ooxml::LN_Value_doc_ST_Zoom_bestFit:
            return sal_Int16(SvxZoomType::PAGEWIDTH);
        case NS_ooxml::LN_Value_doc_ST_Zoom_textFit:
            return sal_Int16(SvxZoomType::OPTIMAL);
    }

    return sal_Int16(SvxZoomType::PERCENT);
}
}

namespace dmapper
{

SettingsTable::SettingsTable(const DomainMapper& rDomainMapper)
: LoggedProperties("SettingsTable")
, LoggedTable("SettingsTable")
, m_nDefaultTabStop( 720 ) //default is 1/2 in
, m_bRecordChanges(false)
, m_bShowInsDelChanges(true)
, m_bShowFormattingChanges(false)
, m_bShowMarkupChanges(true)
, m_nZoomFactor(0)
, m_nWordCompatibilityMode(-1)
, m_nView(0)
, m_bEvenAndOddHeaders(false)
, m_bUsePrinterMetrics(false)
, embedTrueTypeFonts(false)
, embedSystemFonts(false)
, m_bDoNotUseHTMLParagraphAutoSpacing(false)
, m_bNoColumnBalance(false)
, m_bAutoHyphenation(false)
, m_bNoHyphenateCaps(false)
, m_nHyphenationZone( 360 ) // default is 1/4 in
, m_nConsecutiveHyphenLimit(0)
, m_nUseWord2013TrackBottomHyphenation(-1)
, m_nAllowHyphenationAtTrackBottom(-1)
, m_bWidowControl(false)
, m_bLongerSpaceSequence(false)
, m_bSplitPgBreakAndParaMark(false)
, m_bMirrorMargin(false)
, m_bDoNotExpandShiftReturn(false)
, m_bDisplayBackgroundShape(false)
, m_sDecimalSymbol(u"."_ustr)
, m_sListSeparator(u","_ustr)
, m_pThemeFontLangProps(3)
, m_pCurrentCompatSetting(3)
{
    if (rDomainMapper.IsRTFImport())
    {
        // HTML paragraph auto-spacing is opt-in for RTF, opt-out for OOXML.
        m_bDoNotUseHTMLParagraphAutoSpacing = true;
        // Longer space sequence is opt-in for RTF, and not in OOXML.
        m_bLongerSpaceSequence = true;
        m_bDoNotBreakWrappedTables = true;
    }
    m_pDocumentProtection = std::make_shared<DocumentProtection>();
    m_pWriteProtection = std::make_shared<WriteProtection>();
}

SettingsTable::~SettingsTable()
{
}

void SettingsTable::lcl_attribute(Id nName, const Value & val)
{
    int nIntValue = val.getInt();
    OUString sStringValue = val.getString();

    switch(nName)
    {
    case NS_ooxml::LN_CT_Zoom_percent:
        m_nZoomFactor = nIntValue;
    break;
    case NS_ooxml::LN_CT_Zoom_val:
        m_nZoomType = lcl_GetZoomType(nIntValue);
        break;
    case NS_ooxml::LN_CT_Language_val:
        m_pThemeFontLangProps.getArray()[0]
            = comphelper::makePropertyValue(u"val"_ustr, sStringValue);
        break;
    case NS_ooxml::LN_CT_Language_eastAsia:
        m_pThemeFontLangProps.getArray()[1]
            = comphelper::makePropertyValue(u"eastAsia"_ustr, sStringValue);
        break;
    case NS_ooxml::LN_CT_Language_bidi:
        m_pThemeFontLangProps.getArray()[2]
            = comphelper::makePropertyValue(u"bidi"_ustr, sStringValue);
        break;
    case NS_ooxml::LN_CT_View_val:
        m_nView = nIntValue;
        break;
    case NS_ooxml::LN_CT_DocVar_name:
        m_aDocVars.back().first = sStringValue;
        break;
    case NS_ooxml::LN_CT_DocVar_val:
        m_aDocVars.back().second =
            sStringValue.replaceAll("_x000d__x000a_", "\n")
            .replaceAll("_x000d_", "\n");
        break;
    case NS_ooxml::LN_CT_CompatSetting_name:
        m_aCurrentCompatSettingName = sStringValue;
        m_pCurrentCompatSetting.getArray()[0]
            = comphelper::makePropertyValue(u"name"_ustr, sStringValue);
        break;
    case NS_ooxml::LN_CT_CompatSetting_uri:
        m_aCurrentCompatSettingUri = sStringValue;
        m_pCurrentCompatSetting.getArray()[1]
            = comphelper::makePropertyValue(u"uri"_ustr, sStringValue);
        break;
    case NS_ooxml::LN_CT_CompatSetting_val:
        m_aCurrentCompatSettingValue = sStringValue;
        m_pCurrentCompatSetting.getArray()[2]
            = comphelper::makePropertyValue(u"val"_ustr, sStringValue);
        break;
    case NS_ooxml::LN_CT_TrackChangesView_insDel:
        m_bShowInsDelChanges = (nIntValue != 0);
        break;
    case NS_ooxml::LN_CT_TrackChangesView_formatting:
        m_bShowFormattingChanges = (nIntValue != 0);
        break;
    case NS_ooxml::LN_CT_TrackChangesView_markup:
        m_bShowMarkupChanges = (nIntValue != 0);
        break;
    default:
    {
#ifdef DBG_UTIL
        TagLogger::getInstance().element("unhandled");
#endif
    }
    }
}

void SettingsTable::lcl_sprm(Sprm& rSprm)
{
    sal_uInt32 nSprmId = rSprm.getId();

    const Value* pValue = rSprm.getValue();
    sal_Int32 nIntValue = (pValue ? pValue->getInt() : 0);
    OUString sStringValue = pValue ? pValue->getString() : OUString();

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
        resolveSprmProps(*this, rSprm);

        // If attached template cannot be located or is not a valid file, silently ignore linkStyles
        // and since LO doesn't know how to update styles anyway, just always ignore linkStyles
        if (nSprmId == NS_ooxml::LN_CT_Settings_attachedTemplate)
            m_oLinkStyles = false;
    }
    break;
    case NS_ooxml::LN_CT_Settings_stylePaneFormatFilter: // 92493;
    break;
    case NS_ooxml::LN_CT_Settings_defaultTabStop: //  92505;
    m_nDefaultTabStop = nIntValue;
    break;
    case NS_ooxml::LN_CT_Settings_linkStyles: // 92663;
    {
        if (!m_oLinkStyles.has_value())
            m_oLinkStyles = bool(nIntValue);
    }
    break;
    case NS_ooxml::LN_CT_Settings_evenAndOddHeaders:
    m_bEvenAndOddHeaders = nIntValue;
    break;
    case NS_ooxml::LN_CT_Settings_noPunctuationKerning: //  92526;
    break;
    case NS_ooxml::LN_CT_Settings_characterSpacingControl: //  92527;
     // doNotCompress, compressPunctuation, compressPunctuationAndJapaneseKana
    break;
    case NS_ooxml::LN_CT_Settings_doNotIncludeSubdocsInStats: //  92554; // Do Not Include Content in Text Boxes, Footnotes, and Endnotes in Document Statistics)
    break;
    case NS_ooxml::LN_CT_Settings_decimalSymbol: //  92562;
        m_sDecimalSymbol = sStringValue;
    break;
    case NS_ooxml::LN_CT_Settings_listSeparator: //  92563;
        m_sListSeparator = sStringValue;
    break;
    case NS_ooxml::LN_CT_Settings_rsids: //  92549; revision save Ids - probably not necessary
    break;
    case NS_ooxml::LN_CT_Settings_hyphenationZone: // 92508;
        m_nHyphenationZone = nIntValue;
    break;
    case NS_ooxml::LN_CT_Settings_consecutiveHyphenLimit:
        m_nConsecutiveHyphenLimit = nIntValue;
    break;
    case NS_ooxml::LN_CT_Compat_useFELayout: // 92422;
    // useFELayout (Do Not Bypass East Asian/Complex Script Layout Code - support of old versions of Word - ignored)
    break;
    case NS_ooxml::LN_CT_Settings_trackRevisions:
    {
        m_bRecordChanges = bool(nIntValue);
    }
    break;
    case NS_ooxml::LN_CT_Settings_revisionView:
        resolveSprmProps(*this, rSprm);
        break;
    case NS_ooxml::LN_CT_Settings_documentProtection:
        resolveSprmProps(*m_pDocumentProtection, rSprm);
        break;
    case NS_ooxml::LN_CT_Settings_writeProtection:
        resolveSprmProps(*m_pWriteProtection, rSprm);
        break;
    case NS_ooxml::LN_CT_Compat_usePrinterMetrics:
        m_bUsePrinterMetrics = nIntValue;
        break;
    case NS_ooxml::LN_CT_Settings_embedTrueTypeFonts:
        embedTrueTypeFonts = nIntValue != 0;
        break;
    case NS_ooxml::LN_CT_Settings_embedSystemFonts:
        embedSystemFonts = nIntValue != 0;
        break;
    case NS_ooxml::LN_CT_Compat_doNotUseHTMLParagraphAutoSpacing:
        m_bDoNotUseHTMLParagraphAutoSpacing = nIntValue;
        break;
    case NS_ooxml::LN_CT_Compat_splitPgBreakAndParaMark:
        m_bSplitPgBreakAndParaMark = nIntValue;
        break;
    case NS_ooxml::LN_CT_Settings_mirrorMargins:
        m_bMirrorMargin = nIntValue;
        break;
    case NS_ooxml::LN_CT_Settings_mailMerge:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties)
            pProperties->resolve(*this);
    }
    break;
    case NS_ooxml::LN_CT_MailMerge_query:
    {
        // try to get the "database.table" name from the query saved previously
        OUString sVal = sStringValue;
        if ( sVal.endsWith("$") && sVal.indexOf(".dbo.") > 0 )
        {
            sal_Int32 nSpace = sVal.lastIndexOf(' ');
            sal_Int32 nDbo = sVal.lastIndexOf(".dbo.");
            if ( nSpace > 0 && nSpace < nDbo - 1 )
            {
                m_sCurrentDatabaseDataSource = OUString::Concat(sVal.subView(nSpace + 1, nDbo - nSpace - 1)) +
                            sVal.subView(nDbo + 4, sVal.getLength() - nDbo - 5);
            }
        }
    }
    break;
    case NS_ooxml::LN_CT_Compat_compatSetting:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties)
        {
            pProperties->resolve(*this);

            beans::PropertyValue aValue;
            aValue.Name = "compatSetting";
            aValue.Value <<= m_pCurrentCompatSetting;
            m_aCompatSettings.push_back(aValue);

            OString aCompatSettingValue = rtl::OUStringToOString(
                m_aCurrentCompatSettingValue, RTL_TEXTENCODING_UTF8);
            if (m_aCurrentCompatSettingName == "allowTextAfterFloatingTableBreak"
                && m_aCurrentCompatSettingUri == "http://schemas.microsoft.com/office/word"
                && ooxml::GetBooleanValue(aCompatSettingValue))
            {
                m_bAllowTextAfterFloatingTableBreak = true;
            }
            else if (m_aCurrentCompatSettingName == "useWord2013TrackBottomHyphenation" &&
                m_aCurrentCompatSettingUri == "http://schemas.microsoft.com/office/word")
            {
                m_nUseWord2013TrackBottomHyphenation =
                        static_cast<int>(ooxml::GetBooleanValue(aCompatSettingValue));
            }
            else if (m_aCurrentCompatSettingName == "allowHyphenationAtTrackBottom" &&
                m_aCurrentCompatSettingUri == "http://schemas.microsoft.com/office/word")
            {
                m_nAllowHyphenationAtTrackBottom =
                        static_cast<int>(ooxml::GetBooleanValue(aCompatSettingValue));
            }
        }
    }
    break;
    case NS_ooxml::LN_CT_Settings_docVars:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties)
        {
            pProperties->resolve(*this);
        }
    }
    break;
    case NS_ooxml::LN_CT_DocVar:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties)
        {
            m_aDocVars.push_back(std::make_pair(OUString(), OUString()));
            pProperties->resolve(*this);
        }
    }
    break;
    case NS_ooxml::LN_CT_Compat_noColumnBalance:
        m_bNoColumnBalance = nIntValue;
        break;
    case NS_ooxml::LN_CT_Settings_autoHyphenation:
        m_bAutoHyphenation = nIntValue;
        break;
    case NS_ooxml::LN_CT_Settings_doNotHyphenateCaps:
        m_bNoHyphenateCaps = nIntValue;
        break;
    case NS_ooxml::LN_CT_Settings_widowControl:
        m_bWidowControl = nIntValue;
        break;
    case NS_ooxml::LN_CT_Settings_longerSpaceSequence:
        m_bLongerSpaceSequence = nIntValue;
        break;
    case NS_ooxml::LN_CT_Compat_doNotExpandShiftReturn:
        m_bDoNotExpandShiftReturn = true;
        break;
    case NS_ooxml::LN_CT_Compat_balanceSingleByteDoubleByteWidth:
        m_bBalanceSingleByteDoubleByteWidth = true;
        break;
    case NS_ooxml::LN_CT_Settings_displayBackgroundShape:
        m_bDisplayBackgroundShape = nIntValue;
        break;
    case NS_ooxml::LN_CT_Compat_noLeading:
        m_bNoLeading = nIntValue != 0;
        break;
    case NS_ooxml::LN_CT_Settings_gutterAtTop:
        m_bGutterAtTop = nIntValue != 0;
        break;
    case NS_ooxml::LN_CT_Compat_doNotBreakWrappedTables:
        m_bDoNotBreakWrappedTables = nIntValue != 0;
        break;
    case NS_ooxml::LN_CT_EdnProps_pos:
        if (nIntValue == NS_ooxml::LN_Value_ST_EdnPos_sectEnd)
        {
            m_bEndnoteIsCollectAtSectionEnd = true;
        }
        break;
    case NS_ooxml::LN_CT_Compat_ulTrailSpace:
        m_bMsWordUlTrailSpace = true;
        break;
    case NS_ooxml::LN_CT_Compat_doNotVertAlignCellWithSp:
        m_bDoNotVertAlignCellWithSp = nIntValue != 0;
        break;
    case NS_ooxml::LN_CT_Compat_adjustLineHeightInTable:
        m_bAdjustLineHeightInTable = nIntValue != 0;
        break;
    default:
    {
#ifdef DBG_UTIL
        TagLogger::getInstance().element("unhandled");
#endif
    }
    }
}

void SettingsTable::lcl_entry(const writerfilter::Reference<Properties>::Pointer_t& ref)
{
    ref->resolve(*this);
}

//returns default TabStop in 1/100th mm
int SettingsTable::GetDefaultTabStop() const
{
    return ConversionHelper::convertTwipToMm100_Limited(m_nDefaultTabStop);
}

bool SettingsTable::GetLinkStyles() const
{
    return m_oLinkStyles.has_value() && *m_oLinkStyles;
}

sal_Int16 SettingsTable::GetZoomFactor() const
{
    return m_nZoomFactor;
}

sal_Int16 SettingsTable::GetZoomType() const { return m_nZoomType; }

Id SettingsTable::GetView() const
{
    return m_nView;
}

bool SettingsTable::GetUsePrinterMetrics() const
{
    return m_bUsePrinterMetrics;
}

bool SettingsTable::GetEvenAndOddHeaders() const
{
    return m_bEvenAndOddHeaders;
}

bool SettingsTable::GetEmbedTrueTypeFonts() const
{
    return embedTrueTypeFonts;
}

bool SettingsTable::GetEmbedSystemFonts() const
{
    return embedSystemFonts;
}

bool SettingsTable::GetDoNotUseHTMLParagraphAutoSpacing() const
{
    return m_bDoNotUseHTMLParagraphAutoSpacing;
}

bool SettingsTable::GetNoColumnBalance() const
{
    return m_bNoColumnBalance;
}

bool SettingsTable::GetSplitPgBreakAndParaMark() const
{
    return m_bSplitPgBreakAndParaMark;
}

bool SettingsTable::GetMirrorMarginSettings() const
{
    return m_bMirrorMargin;
}

bool SettingsTable::GetDisplayBackgroundShape() const
{
    return m_bDisplayBackgroundShape;
}

bool SettingsTable::GetDoNotExpandShiftReturn() const
{
    return m_bDoNotExpandShiftReturn;
}

bool SettingsTable::GetBalanceSingleByteDoubleByteWidth() const
{
    return m_bBalanceSingleByteDoubleByteWidth;
}

bool SettingsTable::GetProtectForm() const
{
    return m_pDocumentProtection->getProtectForm()
           && m_pDocumentProtection->getEnforcement();
}

bool SettingsTable::GetReadOnly() const
{
    return m_pWriteProtection->getRecommended()
           || (m_pDocumentProtection->getReadOnly()
               && m_pDocumentProtection->getEnforcement());
}

bool SettingsTable::GetNoHyphenateCaps() const
{
    return m_bNoHyphenateCaps;
}

bool SettingsTable::GetMsWordUlTrailSpace() const
{
    return m_bMsWordUlTrailSpace;
}

sal_Int16 SettingsTable::GetHyphenationZone() const
{
    return m_nHyphenationZone;
}

sal_Int16 SettingsTable::GetConsecutiveHyphenLimit() const
{
    return m_nConsecutiveHyphenLimit;
}

bool SettingsTable::GetHyphenationKeep() const
{
    // if allowHyphenationAtTrackBottom is false, also if it is not defined
    // (which means the same) hyphenation is not allowed in the last line, so
    // set ParaHyphenationKeep to TRUE, and set ParaHyphenationKeepType to COLUMN
    return m_nAllowHyphenationAtTrackBottom != 1;
}

bool SettingsTable::GetHyphenationKeepLine() const
{
    // if allowHyphenationAtTrackBottom is not true and useWord2013TrackBottomHyphenation is
    // not present or it is false, shift only the hyphenated word to the next column or page,
    // not the full line, so set ParaHyphenationKeepLine to TRUE
    return GetHyphenationKeep() &&
                m_nUseWord2013TrackBottomHyphenation != 1;
}

const OUString & SettingsTable::GetDecimalSymbol() const
{
    return m_sDecimalSymbol;
}

const OUString & SettingsTable::GetListSeparator() const
{
    return m_sListSeparator;
}


uno::Sequence<beans::PropertyValue> const & SettingsTable::GetThemeFontLangProperties() const
{
    return m_pThemeFontLangProps;
}

uno::Sequence<beans::PropertyValue> SettingsTable::GetCompatSettings()
{
    if ( GetWordCompatibilityMode() == -1 )
    {
        // the default value for an undefined compatibilityMode is 12 (Word 2007)
        uno::Sequence<beans::PropertyValue> aCompatSetting( comphelper::InitPropertySequence({
            { "name", uno::Any(u"compatibilityMode"_ustr) },
            { "uri", uno::Any(u"http://schemas.microsoft.com/office/word"_ustr) },
            { "val", uno::Any(u"12"_ustr) } //12: Use word processing features specified in ECMA-376. This is the default.
        }));

        beans::PropertyValue aValue;
        aValue.Name = "compatSetting";
        aValue.Value <<= aCompatSetting;

        m_aCompatSettings.push_back(aValue);
    }

    return comphelper::containerToSequence(m_aCompatSettings);
}

uno::Sequence<beans::PropertyValue> SettingsTable::GetDocumentProtectionSettings() const
{
    return m_pDocumentProtection->toSequence();
}

uno::Sequence<beans::PropertyValue> SettingsTable::GetWriteProtectionSettings() const
{
    return m_pWriteProtection->toSequence();
}

const OUString & SettingsTable::GetCurrentDatabaseDataSource() const
{
    return m_sCurrentDatabaseDataSource;
}

static bool lcl_isDefault(const uno::Reference<beans::XPropertyState>& xPropertyState, const OUString& rPropertyName)
{
    return xPropertyState->getPropertyState(rPropertyName) == beans::PropertyState_DEFAULT_VALUE;
}

void SettingsTable::ApplyProperties(rtl::Reference<SwXTextDocument> const& xDoc)
{
    if (!xDoc)
        throw uno::RuntimeException();
    rtl::Reference<SwXDocumentSettings> xDocumentSettings(xDoc->createDocumentSettings());

    // Shared between DOCX and RTF, unconditional flags.
    xDocumentSettings->setPropertyValue(u"TableRowKeep"_ustr, uno::Any(true));
    xDocumentSettings->setPropertyValue(u"AddVerticalFrameOffsets"_ustr, uno::Any(true));
    xDocumentSettings->setPropertyValue(u"ApplyParagraphMarkFormatToEmptyLineAtEndOfParagraph"_ustr,
                                uno::Any(true));
    // rely on default for HyphenateURLs=false
    // rely on default for IgnoreHiddenCharsForLineCalculation=true

    if (GetWordCompatibilityMode() <= 14)
    {
        xDocumentSettings->setPropertyValue(u"MsWordCompMinLineHeightByFly"_ustr, uno::Any(true));
        xDocumentSettings->setPropertyValue(u"TabOverMargin"_ustr, uno::Any(true));
        xDocumentSettings->setPropertyValue(u"AddFrameOffsets"_ustr, uno::Any(true)); // tdf#138782
    }

    // Show changes value
    bool bHideChanges = !m_bShowInsDelChanges || !m_bShowMarkupChanges;
    xDoc->setPropertyValue(u"ShowChanges"_ustr, uno::Any( !bHideChanges || m_bShowFormattingChanges ) );

    // Record changes value
    xDoc->setPropertyValue(u"RecordChanges"_ustr, uno::Any( m_bRecordChanges ) );
    // Password protected Record changes
    if (m_bRecordChanges && m_pDocumentProtection->getRedlineProtection()
        && m_pDocumentProtection->getEnforcement())
    {
        // use dummy protection key to forbid disabling of Record changes without a notice
        // (extending the recent GrabBag support)    TODO support password verification...
        css::uno::Sequence<sal_Int8> aDummyKey { 1 };
        xDoc->setPropertyValue(u"RedlineProtectionKey"_ustr, uno::Any( aDummyKey ));
    }

    // Create or overwrite DocVars based on found in settings
    if (m_aDocVars.size())
    {
        rtl::Reference< SwXTextFieldMasters > xFieldMasterAccess = xDoc->getSwXTextFieldMasters();
        for (const auto& docVar : m_aDocVars)
        {
            rtl::Reference< SwXFieldMaster > xMaster;
            OUString sFieldMasterService("com.sun.star.text.FieldMaster.User." + docVar.first);

            // Find or create Field Master
            if (xFieldMasterAccess->hasByName(sFieldMasterService))
            {
                xMaster = xFieldMasterAccess->getFieldMasterByName(sFieldMasterService);
            }
            else
            {
                xMaster = xDoc->createFieldMaster(u"com.sun.star.text.FieldMaster.User");
                xMaster->setPropertyValue(getPropertyName(PROP_NAME), uno::Any(docVar.first));
                uno::Reference<text::XDependentTextField> xField(
                    xDoc->createInstance(u"com.sun.star.text.TextField.User"_ustr),
                    uno::UNO_QUERY);
                xField->attachTextFieldMaster(xMaster);
            }

            xMaster->setPropertyValue(getPropertyName(PROP_CONTENT), uno::Any(docVar.second));
        }
    }

    if (m_bDoNotBreakWrappedTables)
    {
        // Map <w:doNotBreakWrappedTables> to the DoNotBreakWrappedTables compat flag.
        xDocumentSettings->setPropertyValue(u"DoNotBreakWrappedTables"_ustr, uno::Any(true));
    }

    if (m_bAllowTextAfterFloatingTableBreak)
    {
        xDocumentSettings->setPropertyValue(u"AllowTextAfterFloatingTableBreak"_ustr, uno::Any(true));
    }

    // Auto hyphenation: turns on hyphenation by default, <w:suppressAutoHyphens/> may still disable it at a paragraph level.
    // Situation is similar for RTF_WIDOWCTRL, which turns on widow / orphan control by default.
    if (!(m_bAutoHyphenation || m_bNoHyphenateCaps || m_bWidowControl))
        return;

    rtl::Reference<SwXStyleFamilies> xStyleFamilies = xDoc->getSwStyleFamilies();
    rtl::Reference<SwXStyleFamily> xParagraphStyles = xStyleFamilies->GetParagraphStyles();
    rtl::Reference<SwXBaseStyle> xDefault = xParagraphStyles->getStyleByName(u"Standard"_ustr);
    uno::Reference<beans::XPropertyState> xPropertyState(static_cast<cppu::OWeakObject*>(xDefault.get()), uno::UNO_QUERY);
    if (m_bAutoHyphenation && lcl_isDefault(xPropertyState, u"ParaIsHyphenation"_ustr))
    {
        xDefault->setPropertyValue(u"ParaIsHyphenation"_ustr, uno::Any(true));
    }
    if (m_bNoHyphenateCaps)
    {
        xDefault->setPropertyValue(u"ParaHyphenationNoCaps"_ustr, uno::Any(true));
    }
    if (m_nHyphenationZone)
    {
        xDefault->setPropertyValue(u"ParaHyphenationZone"_ustr, uno::Any(GetHyphenationZone()));
    }
    if (m_nConsecutiveHyphenLimit)
    {
        xDefault->setPropertyValue(u"ParaHyphenationMaxHyphens"_ustr, uno::Any(GetConsecutiveHyphenLimit()));
    }
    if (m_bWidowControl && lcl_isDefault(xPropertyState, u"ParaWidows"_ustr) && lcl_isDefault(xPropertyState, u"ParaOrphans"_ustr))
    {
        uno::Any aAny(static_cast<sal_Int8>(2));
        xDefault->setPropertyValue(u"ParaWidows"_ustr, aAny);
        xDefault->setPropertyValue(u"ParaOrphans"_ustr, aAny);
    }
    if ( GetHyphenationKeep() )
    {
        xDefault->setPropertyValue(u"ParaHyphenationKeep"_ustr, uno::Any(true));
        xDefault->setPropertyValue(u"ParaHyphenationKeepType"_ustr, uno::Any(text::ParagraphHyphenationKeepType::COLUMN));
        xDefault->setPropertyValue(u"ParaHyphenationKeepLine"_ustr, uno::Any(GetHyphenationKeepLine()));
    }
}

std::pair<bool, bool> SettingsTable::GetCompatSettingHasAndValue( std::u16string_view sCompatName ) const
{
    bool bHas = false;
    bool bRet = false;
    for (const auto& rProp : m_aCompatSettings)
    {
        if (rProp.Name == "compatSetting") //always true
        {
            css::uno::Sequence<css::beans::PropertyValue> aCurrentCompatSettings;
            rProp.Value >>= aCurrentCompatSettings;

            OUString sName;
            aCurrentCompatSettings[0].Value >>= sName;
            if ( sName != sCompatName )
                continue;

            OUString sUri;
            aCurrentCompatSettings[1].Value >>= sUri;
            if ( sUri != "http://schemas.microsoft.com/office/word" )
                continue;

            OUString sVal;
            aCurrentCompatSettings[2].Value >>= sVal;
            // if repeated, what happens?  Last one wins
            bRet = sVal.toBoolean();
            bHas = true;
        }
    }

    return std::pair<bool, bool>(bHas, bRet);
}

//Keep this function in-sync with the one in sw/.../docxattributeoutput.cxx
sal_Int32 SettingsTable::GetWordCompatibilityMode()
{
    if ( m_nWordCompatibilityMode != -1 )
        return m_nWordCompatibilityMode;

    for (const auto& rProp : m_aCompatSettings)
    {
        if (rProp.Name == "compatSetting") //always true
        {
            css::uno::Sequence<css::beans::PropertyValue> aCurrentCompatSettings;
            rProp.Value >>= aCurrentCompatSettings;

            OUString sName;
            aCurrentCompatSettings[0].Value >>= sName;
            if ( sName != "compatibilityMode" )
                continue;

            OUString sUri;
            aCurrentCompatSettings[1].Value >>= sUri;
            if ( sUri != "http://schemas.microsoft.com/office/word" )
                continue;

            OUString sVal;
            aCurrentCompatSettings[2].Value >>= sVal;
            const sal_Int32 nValidMode = sVal.toInt32();
            // if repeated, highest mode wins in MS Word. 11 is the first valid mode.
            if ( nValidMode > 10 && nValidMode > m_nWordCompatibilityMode )
                m_nWordCompatibilityMode = nValidMode;
        }
    }

    return m_nWordCompatibilityMode;
}

bool SettingsTable::GetLongerSpaceSequence() const
{
    return m_bLongerSpaceSequence;
}

bool SettingsTable::GetNoLeading() const
{
    return m_bNoLeading;
}

bool SettingsTable::GetGutterAtTop() const { return m_bGutterAtTop; }

bool SettingsTable::GetRecordChanges() const { return m_bRecordChanges; }

bool SettingsTable::GetEndnoteIsCollectAtSectionEnd() const
{
    return m_bEndnoteIsCollectAtSectionEnd;
}

bool SettingsTable::GetDoNotVertAlignCellWithSp() const
{
    return m_bDoNotVertAlignCellWithSp;
}

bool SettingsTable::GetAdjustLineHeightInTable() const
{
    return m_bAdjustLineHeightInTable;
}

}//namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
