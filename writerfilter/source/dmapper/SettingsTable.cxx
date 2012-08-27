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

#include <stdio.h>
#include <rtl/ustring.hxx>
#include <resourcemodel/ResourceModelHelper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <SettingsTable.hxx>
#include <doctok/resourceids.hxx>
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
    OUString     m_sListSeparatorForFields; //2.15.1.56 listSeparator (List Separator for Field Code Evaluation)

    int                 m_nDefaultTabStop;
    int                 m_nHyphenationZone;

    bool                m_bNoPunctuationKerning;
    bool                m_doNotIncludeSubdocsInStats; // Do Not Include Content in Text Boxes, Footnotes, and Endnotes in Document Statistics)
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
    bool                m_bEvenAndOddHeaders;
    bool                m_bUsePrinterMetrics;

    SettingsTable_Impl( DomainMapper& rDMapper, const uno::Reference< lang::XMultiServiceFactory > xTextFactory ) :
    m_rDMapper( rDMapper )
    , m_xTextFactory( xTextFactory )
    , m_nDefaultTabStop( 720 ) //default is 1/2 in
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
    , m_bEvenAndOddHeaders(false)
    , m_bUsePrinterMetrics(false)
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

    switch(nName)
    {
    case NS_ooxml::LN_CT_Zoom_percent:
        m_pImpl->m_nZoomFactor = nIntValue;
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
    case NS_ooxml::LN_CT_Settings_zoom: //  92469;
    case NS_ooxml::LN_CT_Settings_proofState: //  92489;
    case NS_ooxml::LN_CT_Settings_attachedTemplate: //  92491;
    case NS_ooxml::LN_CT_Settings_hdrShapeDefaults: //  92544;
    case NS_ooxml::LN_CT_Settings_footnotePr: //  92545;
    case NS_ooxml::LN_CT_Settings_endnotePr: //  92546;
    case NS_ooxml::LN_CT_Settings_compat: //  92547;
    case NS_ooxml::LN_CT_Settings_themeFontLang: //  92552;
    case NS_ooxml::LN_CT_Settings_shapeDefaults: //  92560;

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
    m_pImpl->m_bNoPunctuationKerning = nIntValue ? true : false;
    break;
    case NS_ooxml::LN_CT_Settings_characterSpacingControl: //  92527;
    m_pImpl->m_sCharacterSpacing = sStringValue; // doNotCompress, compressPunctuation, compressPunctuationAndJapaneseKana
    break;
    case NS_ooxml::LN_CT_Settings_doNotIncludeSubdocsInStats: //  92554; // Do Not Include Content in Text Boxes, Footnotes, and Endnotes in Document Statistics)
    m_pImpl->m_doNotIncludeSubdocsInStats = nIntValue? true : false;
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

bool SettingsTable::GetUsePrinterMetrics() const
{
    return m_pImpl->m_bUsePrinterMetrics;
}

bool SettingsTable::GetEvenAndOddHeaders() const
{
    return m_pImpl->m_bEvenAndOddHeaders;
}

void SettingsTable::ApplyProperties( uno::Reference< text::XTextDocument > xDoc )
{
    uno::Reference< beans::XPropertySet> xDocProps( xDoc, uno::UNO_QUERY );

    // Record changes value
    if (xDocProps.is())
        xDocProps->setPropertyValue("RecordChanges", uno::makeAny( m_pImpl->m_bRecordChanges ) );
}


}//namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
