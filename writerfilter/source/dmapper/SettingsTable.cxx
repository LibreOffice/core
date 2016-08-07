/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <stdio.h>
#include <rtl/ustring.hxx>
#include <resourcemodel/ResourceModelHelper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <SettingsTable.hxx>
#include <resourcemodel/ResourceModelHelper.hxx>
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
    (void) nName;
    int nIntValue = val.getInt();
    (void)nIntValue;
    ::rtl::OUString sValue = val.getString();
    (void)sValue;
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
}

void SettingsTable::lcl_sprm(Sprm& rSprm)
{
    sal_uInt32 nSprmId = rSprm.getId();

    Value::Pointer_t pValue = rSprm.getValue();
    sal_Int32 nIntValue = pValue->getInt();
    (void)nIntValue;
    rtl::OUString sStringValue = pValue->getString();

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
#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->element("SettingsTable.unhandled");
#endif
    }
    }
}

void SettingsTable::lcl_entry(int /*pos*/, writerfilter::Reference<Properties>::Pointer_t ref)
{
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
