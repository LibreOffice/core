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

#include <stdio.h>
#include <rtl/ustring.hxx>
#include <resourcemodel/ResourceModelHelper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <SettingsTable.hxx>
#include <ooxml/resourceids.hxx>
#include <ConversionHelper.hxx>

#include "dmapperLoggers.hxx"

namespace writerfilter {

using resourcemodel::resolveSprmProps;

namespace dmapper
{

struct SettingsTable_Impl
{
    DomainMapper&       m_rDMapper;
    const uno::Reference< lang::XMultiServiceFactory > m_xTextFactory;

    OUString     m_sCharacterSpacing;
    OUString     m_sDecimalSymbol;
    OUString     m_sListSeparatorForFields; 

    int                 m_nDefaultTabStop;
    int                 m_nHyphenationZone;

    bool                m_bNoPunctuationKerning;
    bool                m_doNotIncludeSubdocsInStats; 
    bool                m_bRecordChanges;
    int                 m_nEdit;
    bool                m_bFormatting;
    bool                m_bEnforcement;
    int                 m_nCryptProviderType;
    int                 m_nCryptAlgorithmClass;
    int                 m_nCryptAlgorithmType;
    OUString     m_sCryptAlgorithmSid;
    int                 m_nCryptSpinCount;
    OUString     m_sCryptProvider;
    OUString     m_sAlgIdExt;
    OUString     m_sAlgIdExtSource;
    OUString     m_sCryptProviderTypeExt;
    OUString     m_sCryptProviderTypeExtSource;
    OUString     m_sHash;
    OUString     m_sSalt;
    bool                m_bLinkStyles;
    sal_Int16           m_nZoomFactor;
    Id                  m_nView;
    bool                m_bEvenAndOddHeaders;
    bool                m_bUsePrinterMetrics;
    bool                embedTrueTypeFonts;
    bool                embedSystemFonts;
    bool                m_bDoNotUseHTMLParagraphAutoSpacing;
    bool                m_bSplitPgBreakAndParaMark;
    bool                m_bMirrorMargin;
    uno::Sequence<beans::PropertyValue> m_pThemeFontLangProps;

    SettingsTable_Impl( DomainMapper& rDMapper, const uno::Reference< lang::XMultiServiceFactory > xTextFactory ) :
    m_rDMapper( rDMapper )
    , m_xTextFactory( xTextFactory )
    , m_nDefaultTabStop( 720 ) 
    , m_nHyphenationZone(0)
    , m_bNoPunctuationKerning(false)
    , m_doNotIncludeSubdocsInStats(false)
    , m_bRecordChanges(false)
    , m_nEdit(NS_ooxml::LN_Value_wordprocessingml_ST_DocProtect_none)
    , m_bFormatting(false)
    , m_bEnforcement(false)
    , m_nCryptProviderType(NS_ooxml::LN_Value_wordprocessingml_ST_CryptProv_rsaAES)
    , m_nCryptAlgorithmClass(NS_ooxml::LN_Value_wordprocessingml_ST_AlgClass_hash)
    , m_nCryptAlgorithmType(NS_ooxml::LN_Value_wordprocessingml_ST_AlgType_typeAny)
    , m_nCryptSpinCount(0)
    , m_bLinkStyles(false)
    , m_nZoomFactor(0)
    , m_nView(0)
    , m_bEvenAndOddHeaders(false)
    , m_bUsePrinterMetrics(false)
    , embedTrueTypeFonts(false)
    , embedSystemFonts(false)
    , m_bDoNotUseHTMLParagraphAutoSpacing(false)
    , m_bSplitPgBreakAndParaMark(false)
    , m_bMirrorMargin(false)
    , m_pThemeFontLangProps(3)
    {}

};

SettingsTable::SettingsTable(DomainMapper& rDMapper, const uno::Reference< lang::XMultiServiceFactory > xTextFactory)
: LoggedProperties(dmapper_logger, "SettingsTable")
, LoggedTable(dmapper_logger, "SettingsTable")
, m_pImpl( new SettingsTable_Impl(rDMapper, xTextFactory) )
{

}

SettingsTable::~SettingsTable()
{
    delete m_pImpl;
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
    default:
    {
#ifdef DEBUG_DMAPPER_SETTINGS_TABLE
        dmapper_logger->element("unhandled");
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
    case NS_ooxml::LN_CT_Settings_zoom: 
    case NS_ooxml::LN_CT_Settings_proofState: 
    case NS_ooxml::LN_CT_Settings_attachedTemplate: 
    case NS_ooxml::LN_CT_Settings_hdrShapeDefaults: 
    case NS_ooxml::LN_CT_Settings_footnotePr: 
    case NS_ooxml::LN_CT_Settings_endnotePr: 
    case NS_ooxml::LN_CT_Settings_compat: 
    case NS_ooxml::LN_CT_Settings_themeFontLang: 
    case NS_ooxml::LN_CT_Settings_shapeDefaults: 
    case NS_ooxml::LN_CT_Settings_view:

    
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties.get())
        pProperties->resolve(*this);
    }
    break;
    case NS_ooxml::LN_CT_Settings_stylePaneFormatFilter: 
    break;
    case NS_ooxml::LN_CT_Settings_defaultTabStop: 
    m_pImpl->m_nDefaultTabStop = nIntValue;
    break;
    case NS_ooxml::LN_CT_Settings_linkStyles: 
    m_pImpl->m_bLinkStyles = nIntValue;
    break;
    case NS_ooxml::LN_CT_Settings_evenAndOddHeaders:
    m_pImpl->m_bEvenAndOddHeaders = nIntValue;
    break;
    case NS_ooxml::LN_CT_Settings_noPunctuationKerning: 
    m_pImpl->m_bNoPunctuationKerning = nIntValue ? true : false;
    break;
    case NS_ooxml::LN_CT_Settings_characterSpacingControl: 
    m_pImpl->m_sCharacterSpacing = sStringValue; 
    break;
    case NS_ooxml::LN_CT_Settings_doNotIncludeSubdocsInStats: 
    m_pImpl->m_doNotIncludeSubdocsInStats = nIntValue? true : false;
    break;
    case NS_ooxml::LN_CT_Settings_decimalSymbol: 
    m_pImpl->m_sDecimalSymbol = sStringValue;
    break;
    case NS_ooxml::LN_CT_Settings_listSeparator: 
    m_pImpl->m_sListSeparatorForFields = sStringValue;
    break;
    case NS_ooxml::LN_CT_Settings_rsids: 
    break;
    case NS_ooxml::LN_CT_Settings_hyphenationZone: 
    m_pImpl->m_nHyphenationZone = nIntValue;
    break;
    case NS_ooxml::LN_CT_Compat_useFELayout: 
    
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
    default:
    {
#ifdef DEBUG_DMAPPER_SETTINGS_TABLE
        dmapper_logger->element("unhandled");
#endif
    }
    }
}

void SettingsTable::lcl_entry(int /*pos*/, writerfilter::Reference<Properties>::Pointer_t ref)
{
    ref->resolve(*this);
}




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

bool SettingsTable::GetSplitPgBreakAndParaMark() const
{
    return m_pImpl->m_bSplitPgBreakAndParaMark;
}

bool SettingsTable::GetMirrorMarginSettings() const
{
    return m_pImpl->m_bMirrorMargin;
}

uno::Sequence<beans::PropertyValue> SettingsTable::GetThemeFontLangProperties() const
{
    return m_pImpl->m_pThemeFontLangProps;
}

void SettingsTable::ApplyProperties( uno::Reference< text::XTextDocument > xDoc )
{
    uno::Reference< beans::XPropertySet> xDocProps( xDoc, uno::UNO_QUERY );

    
    if (xDocProps.is())
        xDocProps->setPropertyValue("RecordChanges", uno::makeAny( m_pImpl->m_bRecordChanges ) );
}


}
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
