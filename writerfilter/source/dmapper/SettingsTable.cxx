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

#include <vector>

#include <rtl/ustring.hxx>
#include <sfx2/zoomitem.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <comphelper/sequence.hxx>
#include <ooxml/resourceids.hxx>
#include "ConversionHelper.hxx"
#include "DomainMapper.hxx"
#include "util.hxx"

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
    /** Document protection restrictions
     *
     * This element specifies the set of document protection restrictions which have been applied to the contents of a
     * WordprocessingML document.These restrictions should be enforced by applications editing this document
     * when the enforcement attribute is turned on, and ignored(but persisted) otherwise.Document protection is a
     * set of restrictions used to prevent unintentional changes to all or part of a WordprocessingML document.
     */
    struct DocumentProtection_Impl
    {
        /** Document Editing Restrictions
         *
         * Possible values:
         *  - NS_ooxml::LN_Value_doc_ST_DocProtect_none
         *  - NS_ooxml::LN_Value_doc_ST_DocProtect_readOnly
         *  - NS_ooxml::LN_Value_doc_ST_DocProtect_comments
         *  - NS_ooxml::LN_Value_doc_ST_DocProtect_trackedChanges
         *  - NS_ooxml::LN_Value_doc_ST_DocProtect_forms
         */
        sal_Int32       m_nEdit;
        bool            m_bEnforcement;
        bool            m_bFormatting;

        /** Provider type
         *
         * Possible values:
         *  "rsaAES"  - NS_ooxml::LN_Value_doc_ST_CryptProv_rsaAES
         *  "rsaFull" - NS_ooxml::LN_Value_doc_ST_CryptProv_rsaFull
         */
        sal_Int32       m_nCryptProviderType;
        OUString        m_sCryptAlgorithmClass;
        OUString        m_sCryptAlgorithmType;
        OUString        m_sCryptAlgorithmSid;
        sal_Int32       m_CryptSpinCount;
        OUString        m_sHash;
        OUString        m_sSalt;

        DocumentProtection_Impl()
            : m_nEdit(NS_ooxml::LN_Value_doc_ST_DocProtect_none) // Specifies that no editing restrictions have been applied to the document
            , m_bEnforcement(false)
            , m_bFormatting(false)
            , m_nCryptProviderType(NS_ooxml::LN_Value_doc_ST_CryptProv_rsaAES)
            , m_sCryptAlgorithmClass("hash")
            , m_sCryptAlgorithmType("typeAny")
            , m_CryptSpinCount(0)
        {
        }

        css::uno::Sequence<css::beans::PropertyValue> toSequence() const;

        bool enabled() const
        {
            return ! isNone();
        }

        bool isNone()           const { return m_nEdit == NS_ooxml::LN_Value_doc_ST_DocProtect_none; };
        // bool isReadOnly()       const { return m_nEdit == NS_ooxml::LN_Value_doc_ST_DocProtect_readOnly; };
        // bool isComments()       const { return m_nEdit == NS_ooxml::LN_Value_doc_ST_DocProtect_comments; };
        // bool isTrackChanges()   const { return m_nEdit == NS_ooxml::LN_Value_doc_ST_DocProtect_trackedChanges; };
        bool isForms()          const { return m_nEdit == NS_ooxml::LN_Value_doc_ST_DocProtect_forms; };
    };

    css::uno::Sequence<css::beans::PropertyValue> DocumentProtection_Impl::toSequence() const
    {
        std::vector<beans::PropertyValue> documentProtection;

        if (enabled())
        {
            // w:edit
            {
                beans::PropertyValue aValue;
                aValue.Name = "edit";

                switch (m_nEdit)
                {
                case NS_ooxml::LN_Value_doc_ST_DocProtect_none:             aValue.Value <<= OUString("none"); break;
                case NS_ooxml::LN_Value_doc_ST_DocProtect_readOnly:         aValue.Value <<= OUString("readOnly"); break;
                case NS_ooxml::LN_Value_doc_ST_DocProtect_comments:         aValue.Value <<= OUString("comments"); break;
                case NS_ooxml::LN_Value_doc_ST_DocProtect_trackedChanges:   aValue.Value <<= OUString("trackedChanges"); break;
                case NS_ooxml::LN_Value_doc_ST_DocProtect_forms:            aValue.Value <<= OUString("forms"); break;
                default:
                {
#ifdef DEBUG_WRITERFILTER
                    TagLogger::getInstance().element("unhandled");
#endif
                }
                }

                documentProtection.push_back(aValue);
            }

            // w:enforcement
            if (m_bEnforcement)
            {
                beans::PropertyValue aValue;
                aValue.Name = "enforcement";
                aValue.Value <<= OUString("1");
                documentProtection.push_back(aValue);
            }

            // w:formatting
            if (m_bFormatting)
            {
                beans::PropertyValue aValue;
                aValue.Name = "formatting";
                aValue.Value <<= OUString("1");
                documentProtection.push_back(aValue);
            }

            // w:cryptProviderType
            {
                beans::PropertyValue aValue;
                aValue.Name = "cryptProviderType";
                if (m_nCryptProviderType == NS_ooxml::LN_Value_doc_ST_CryptProv_rsaAES)
                    aValue.Value <<= OUString("rsaAES");
                else if (m_nCryptProviderType == NS_ooxml::LN_Value_doc_ST_CryptProv_rsaFull)
                    aValue.Value <<= OUString("rsaFull");
                documentProtection.push_back(aValue);
            }

            // w:cryptAlgorithmClass
            {
                beans::PropertyValue aValue;
                aValue.Name = "cryptAlgorithmClass";
                aValue.Value <<= m_sCryptAlgorithmClass;
                documentProtection.push_back(aValue);
            }

            // w:cryptAlgorithmType
            {
                beans::PropertyValue aValue;
                aValue.Name = "cryptAlgorithmType";
                aValue.Value <<= m_sCryptAlgorithmType;
                documentProtection.push_back(aValue);
            }

            // w:cryptAlgorithmSid
            {
                beans::PropertyValue aValue;
                aValue.Name = "cryptAlgorithmSid";
                aValue.Value <<= m_sCryptAlgorithmSid;
                documentProtection.push_back(aValue);
            }

            // w:cryptSpinCount
            {
                beans::PropertyValue aValue;
                aValue.Name = "cryptSpinCount";
                aValue.Value <<= OUString::number(m_CryptSpinCount);
                documentProtection.push_back(aValue);
            }

            // w:hash
            {
                beans::PropertyValue aValue;
                aValue.Name = "hash";
                aValue.Value <<= m_sHash;
                documentProtection.push_back(aValue);
            }

            // w:salt
            {
                beans::PropertyValue aValue;
                aValue.Name = "salt";
                aValue.Value <<= m_sSalt;
                documentProtection.push_back(aValue);
            }
        }

        return comphelper::containerToSequence(documentProtection);
    }

struct SettingsTable_Impl
{
    int                 m_nDefaultTabStop;

    bool                m_bRecordChanges;
    bool                m_bLinkStyles;
    sal_Int16           m_nZoomFactor;
    sal_Int16 m_nZoomType = 0;
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
    bool                m_bDoNotExpandShiftReturn;
    bool                m_bProtectForm;
    bool                m_bDisplayBackgroundShape;

    uno::Sequence<beans::PropertyValue> m_pThemeFontLangProps;

    std::vector<beans::PropertyValue> m_aCompatSettings;
    uno::Sequence<beans::PropertyValue> m_pCurrentCompatSetting;

    DocumentProtection_Impl m_DocumentProtection;

    SettingsTable_Impl() :
      m_nDefaultTabStop( 720 ) //default is 1/2 in
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
    , m_bDoNotExpandShiftReturn(false)
    , m_bProtectForm(false)
    , m_bDisplayBackgroundShape(false)
    , m_pThemeFontLangProps(3)
    , m_pCurrentCompatSetting(3)
    {}

};

SettingsTable::SettingsTable(const DomainMapper& rDomainMapper)
: LoggedProperties("SettingsTable")
, LoggedTable("SettingsTable")
, m_pImpl( new SettingsTable_Impl )
{
    // HTML paragraph auto-spacing is opt-in for RTF, opt-out for OOXML.
    if (rDomainMapper.IsRTFImport())
        m_pImpl->m_bDoNotUseHTMLParagraphAutoSpacing = true;
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
    case NS_ooxml::LN_CT_Zoom_val:
        m_pImpl->m_nZoomType = lcl_GetZoomType(nIntValue);
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
    case NS_ooxml::LN_CT_DocProtect_edit: // 92037
        m_pImpl->m_DocumentProtection.m_nEdit = nIntValue;
        m_pImpl->m_bProtectForm = m_pImpl->m_DocumentProtection.isForms();
        break;
    case NS_ooxml::LN_CT_DocProtect_enforcement: // 92039
        m_pImpl->m_DocumentProtection.m_bEnforcement = (nIntValue != 0);
        m_pImpl->m_bProtectForm &= static_cast<bool>(nIntValue);
        break;
    case NS_ooxml::LN_CT_DocProtect_formatting: // 92038
        m_pImpl->m_DocumentProtection.m_bFormatting = (nIntValue != 0);
        break;
    case NS_ooxml::LN_AG_Password_cryptProviderType: // 92025
        m_pImpl->m_DocumentProtection.m_nCryptProviderType = nIntValue;
        break;
    case NS_ooxml::LN_AG_Password_cryptAlgorithmClass: // 92026
        if (nIntValue == NS_ooxml::LN_Value_doc_ST_AlgClass_hash) // 92023
            m_pImpl->m_DocumentProtection.m_sCryptAlgorithmClass = "hash";
        break;
    case NS_ooxml::LN_AG_Password_cryptAlgorithmType: // 92027
        if (nIntValue == NS_ooxml::LN_Value_doc_ST_AlgType_typeAny) // 92024
            m_pImpl->m_DocumentProtection.m_sCryptAlgorithmType = "typeAny";
        break;
    case NS_ooxml::LN_AG_Password_cryptAlgorithmSid: // 92028
        m_pImpl->m_DocumentProtection.m_sCryptAlgorithmSid = sStringValue;
        break;
    case NS_ooxml::LN_AG_Password_cryptSpinCount: // 92029
        m_pImpl->m_DocumentProtection.m_CryptSpinCount = nIntValue;
        break;
    case NS_ooxml::LN_AG_Password_hash: // 92035
        m_pImpl->m_DocumentProtection.m_sHash = sStringValue;
        break;
    case NS_ooxml::LN_AG_Password_salt: // 92036
        m_pImpl->m_DocumentProtection.m_sSalt = sStringValue;
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
    break;
    case NS_ooxml::LN_CT_Settings_characterSpacingControl: //  92527;
     // doNotCompress, compressPunctuation, compressPunctuationAndJapaneseKana
    break;
    case NS_ooxml::LN_CT_Settings_doNotIncludeSubdocsInStats: //  92554; // Do Not Include Content in Text Boxes, Footnotes, and Endnotes in Document Statistics)
    break;
    case NS_ooxml::LN_CT_Settings_decimalSymbol: //  92562;
    break;
    case NS_ooxml::LN_CT_Settings_listSeparator: //  92563;
    break;
    case NS_ooxml::LN_CT_Settings_rsids: //  92549; revision save Ids - probably not necessary
    break;
    case NS_ooxml::LN_CT_Settings_hyphenationZone: // 92508;
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
        resolveSprmProps(*this, rSprm);
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
            aValue.Value <<= m_pImpl->m_pCurrentCompatSetting;
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
    case NS_ooxml::LN_CT_Compat_doNotExpandShiftReturn:
        m_pImpl->m_bDoNotExpandShiftReturn = true;
        break;
    case NS_ooxml::LN_CT_Settings_displayBackgroundShape:
        m_pImpl->m_bDisplayBackgroundShape = nIntValue;
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

sal_Int16 SettingsTable::GetZoomType() const { return m_pImpl->m_nZoomType; }

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

bool SettingsTable::GetDisplayBackgroundShape() const
{
    return m_pImpl->m_bDisplayBackgroundShape;
}

bool SettingsTable::GetDoNotExpandShiftReturn() const
{
    return m_pImpl->m_bDoNotExpandShiftReturn;
}

bool SettingsTable::GetProtectForm() const
{
    return m_pImpl->m_bProtectForm;
}
uno::Sequence<beans::PropertyValue> const & SettingsTable::GetThemeFontLangProperties() const
{
    return m_pImpl->m_pThemeFontLangProps;
}

uno::Sequence<beans::PropertyValue> SettingsTable::GetCompatSettings() const
{
    return comphelper::containerToSequence(m_pImpl->m_aCompatSettings);
}

css::uno::Sequence<css::beans::PropertyValue> SettingsTable::GetDocumentProtectionSettings() const
{
    return m_pImpl->m_DocumentProtection.toSequence();
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

sal_Int32 SettingsTable::GetWordCompatibilityMode() const
{
    for (const auto& rProp : m_pImpl->m_aCompatSettings)
    {
        if (rProp.Name == "compatSetting")
        {
            css::uno::Sequence<css::beans::PropertyValue> aCurrentCompatSettings;
            rProp.Value >>= aCurrentCompatSettings;

            OUString sName;
            OUString sUri;
            OUString sVal;

            aCurrentCompatSettings[0].Value >>= sName;
            aCurrentCompatSettings[1].Value >>= sUri;
            aCurrentCompatSettings[2].Value >>= sVal;

            if (sName == "compatibilityMode" && sUri == "http://schemas.microsoft.com/office/word")
            {
                return sVal.toInt32();
            }
        }
    }

    return -1; // Word compatibility mode not found
}

}//namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
