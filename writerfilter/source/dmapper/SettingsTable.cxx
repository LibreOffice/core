/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SettingsTable.cxx,v $
 * $Revision: 1.3 $
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

#include <resourcemodel/ResourceModelHelper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <SettingsTable.hxx>
#include <doctok/resourceids.hxx>
#include <ooxml/resourceids.hxx>
#include <stdio.h>
#include <ConversionHelper.hxx>
#include <rtl/ustring.hxx>

#ifdef DEBUG_DOMAINMAPPER
#include <resourcemodel/QNameToString.hxx>
#include "dmapperLoggers.hxx"
#endif

namespace writerfilter {

using resourcemodel::resolveSprmProps;

namespace dmapper
{
        
struct SettingsTable_Impl
{
    DomainMapper&       m_rDMapper;
    const uno::Reference< lang::XMultiServiceFactory > m_xTextFactory;
            
    ::rtl::OUString     m_sCharacterSpacing;
    ::rtl::OUString     m_sDecimalSymbol;
    ::rtl::OUString     m_sListSeparatorForFields; //2.15.1.56 listSeparator (List Separator for Field Code Evaluation)
            
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
    ::rtl::OUString     m_sCryptAlgorithmSid;
    int                 m_nCryptSpinCount;
    ::rtl::OUString     m_sCryptProvider;
    ::rtl::OUString     m_sAlgIdExt;
    ::rtl::OUString     m_sAlgIdExtSource;
    ::rtl::OUString     m_sCryptProviderTypeExt;
    ::rtl::OUString     m_sCryptProviderTypeExtSource;
    ::rtl::OUString     m_sHash;
    ::rtl::OUString     m_sSalt;
            
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
    {}
};
        
SettingsTable::SettingsTable(DomainMapper& rDMapper, const uno::Reference< lang::XMultiServiceFactory > xTextFactory) :
m_pImpl( new SettingsTable_Impl(rDMapper, xTextFactory) )
{
    // printf("SettingsTable::SettingsTable()\n");
}
        
SettingsTable::~SettingsTable()
{
    delete m_pImpl;
}
        
void SettingsTable::attribute(Id nName, Value & val)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("SettingsTable.attribute");
    dmapper_logger->attribute("name", (*QNameToString::Instance())(nName));
    dmapper_logger->attribute("value", val.toString());
#endif

    (void) nName;
    int nIntValue = val.getInt();
    (void)nIntValue;
    ::rtl::OUString sValue = val.getString();
    (void)sValue;
    //printf ( "SettingsTable::attribute(0x%.4x, 0x%.4x) [%s]\n", (unsigned int)Name, (unsigned int)nIntValue, ::rtl::OUStringToOString(sValue, RTL_TEXTENCODING_DONTKNOW).getStr());
    /* WRITERFILTERSTATUS: table: SettingsTable_attributedata */
#if 0 //no values known, yet   
            
    switch(Name)
    {
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
    case NS_ooxml:::
    break;
    default:
    {
    }
    }
#endif
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("SettingsTable.attribute");
#endif
}
        
void SettingsTable::sprm(Sprm& rSprm)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("SettingsTable.sprm");
    dmapper_logger->chars(rSprm.toString());
#endif

    sal_uInt32 nSprmId = rSprm.getId();
            
    Value::Pointer_t pValue = rSprm.getValue();
    sal_Int32 nIntValue = pValue->getInt();
    (void)nIntValue;
    rtl::OUString sStringValue = pValue->getString();
            
    //printf ( "SettingsTable::sprm(0x%.4x, 0x%.4x) [%s]\n", (unsigned int)nSprmId, (unsigned int)nIntValue, ::rtl::OUStringToOString(sStringValue, RTL_TEXTENCODING_DONTKNOW).getStr());
            
    /* WRITERFILTERSTATUS: table: SettingsTable_sprm */
    switch(nSprmId)
    {
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_Settings_zoom: //  92469;
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_Settings_proofState: //  92489;
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_Settings_attachedTemplate: //  92491;
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_Settings_hdrShapeDefaults: //  92544;
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_Settings_footnotePr: //  92545;
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_Settings_endnotePr: //  92546;
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_Settings_compat: //  92547;
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_Settings_themeFontLang: //  92552;
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_Settings_shapeDefaults: //  92560;
                    
    //PropertySetValues - need to be resolved
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties.get())
        pProperties->resolve(*this);
    }
    break;
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_Settings_stylePaneFormatFilter: // 92493;
    break;
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_Settings_defaultTabStop: //  92505;
    m_pImpl->m_nDefaultTabStop = nIntValue;
    break;
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_Settings_noPunctuationKerning: //  92526;
    m_pImpl->m_bNoPunctuationKerning = nIntValue ? true : false;
    break;
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_Settings_characterSpacingControl: //  92527;
    m_pImpl->m_sCharacterSpacing = sStringValue; // doNotCompress, compressPunctuation, compressPunctuationAndJapaneseKana  
    break;
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_Settings_doNotIncludeSubdocsInStats: //  92554; // Do Not Include Content in Text Boxes, Footnotes, and Endnotes in Document Statistics)
    m_pImpl->m_doNotIncludeSubdocsInStats = nIntValue? true : false;
    break;
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_Settings_decimalSymbol: //  92562;
    m_pImpl->m_sDecimalSymbol = sStringValue;
    break;
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_Settings_listSeparator: //  92563;
    m_pImpl->m_sListSeparatorForFields = sStringValue;
    break;
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_Settings_rsids: //  92549; revision save Ids - probably not necessary
    break;        
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_Settings_hyphenationZone: // 92508;
    m_pImpl->m_nHyphenationZone = nIntValue;
    break;
    /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
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
    default:
    {
        OSL_ENSURE( false, "unknown sprmid in SettingsTable::sprm()");
    }
    }

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("SettingsTable.sprm");
#endif
}
        
void SettingsTable::entry(int /*pos*/, writerfilter::Reference<Properties>::Pointer_t ref)
{
    // printf ( "SettingsTable::entry\n");
    ref->resolve(*this);
}
//returns default TabStop in 1/100th mm
        
/*-- 22.09.2009 10:29:32---------------------------------------------------
         
  -----------------------------------------------------------------------*/
int SettingsTable::GetDefaultTabStop() const 
{
    return ConversionHelper::convertTwipToMM100( m_pImpl->m_nDefaultTabStop );
}
            
void SettingsTable::ApplyProperties( uno::Reference< text::XTextDocument > xDoc )
{
    uno::Reference< beans::XPropertySet> xDocProps( xDoc, uno::UNO_QUERY );
            
    // Record changes value
    xDocProps->setPropertyValue( ::rtl::OUString::createFromAscii( "RecordChanges" ), uno::makeAny( m_pImpl->m_bRecordChanges ) );
}
        
        
}//namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
