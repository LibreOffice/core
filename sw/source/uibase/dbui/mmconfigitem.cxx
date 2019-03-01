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


#include <mmconfigitem.hxx>
#include <vector>
#include <swtypes.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/sdb/CommandType.hpp>
#include <comphelper/sequence.hxx>
#include <rtl/instance.hxx>
#include <sal/log.hxx>
#include <unotools/configitem.hxx>
#include <tools/diagnose_ex.h>
#include <mailmergehelper.hxx>
#include <swunohelper.hxx>
#include <dbmgr.hxx>
#include <view.hxx>
#include <unodispatch.hxx>
#include <wrtsh.hxx>
#include <dbui.hrc>
#include <unomid.h>

using namespace utl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;

const char cAddressDataAssignments[] = "AddressDataAssignments";
const char cDBColumnAssignments[]    = "DBColumnAssignments";
const char cDataSourceName[]         = "DataSource/DataSourceName";
const char cDataTableName[]          = "DataSource/DataTableName" ;
const char cDataCommandType[]        = "DataSource/DataCommandType";

#define SECURE_PORT     587
#define DEFAULT_PORT    25
#define POP_PORT        110
#define POP_SECURE_PORT     995
#define IMAP_PORT           143
#define IMAP_SECURE_PORT    993

struct DBAddressDataAssignment
{
    SwDBData                            aDBData;
    Sequence< OUString>           aDBColumnAssignments;
    //if loaded the name of the node has to be saved
    OUString                     sConfigNodeName;
    //all created or changed assignments need to be stored
    bool                                bColumnAssignmentsChanged;

    DBAddressDataAssignment() :
        bColumnAssignmentsChanged(false)
        {}
};

class SwMailMergeConfigItem_Impl : public utl::ConfigItem
{
    friend class SwMailMergeConfigItem;
    Reference< XDataSource>                 m_xSource;
    SharedConnection                        m_xConnection;
    Reference< XColumnsSupplier>            m_xColumnsSupplier;
    Reference< XResultSet>                  m_xResultSet;
    SwDBData                                m_aDBData;
    OUString                                m_sFilter;
    sal_Int32                               m_nResultSetCursorPos;

    std::vector<DBAddressDataAssignment>    m_aAddressDataAssignments;
    std::vector< OUString>           m_aAddressBlocks;
    sal_Int32                               m_nCurrentAddressBlock;
    bool                                m_bIsAddressBlock;
    bool                                m_bIsHideEmptyParagraphs;

    bool                                m_bIsOutputToLetter;
    bool                                m_bIncludeCountry;
    OUString                         m_sExcludeCountry;

    bool                                m_bIsGreetingLine;
    bool                                m_bIsIndividualGreetingLine;
    std::vector< OUString>           m_aFemaleGreetingLines;
    sal_Int32                               m_nCurrentFemaleGreeting;
    std::vector< OUString>           m_aMaleGreetingLines;
    sal_Int32                               m_nCurrentMaleGreeting;
    std::vector< OUString>           m_aNeutralGreetingLines;
    sal_Int32                               m_nCurrentNeutralGreeting;
    OUString                         m_sFemaleGenderValue;
    uno::Sequence< OUString>         m_aSavedDocuments;

    bool                                m_bIsGreetingLineInMail;
    bool                                m_bIsIndividualGreetingLineInMail;

    //mail settings
    OUString                         m_sMailDisplayName;
    OUString                         m_sMailAddress;
    OUString                         m_sMailReplyTo;
    OUString                         m_sMailServer;
    OUString                         m_sMailUserName;
    OUString                         m_sMailPassword;

    bool                                m_bIsSMPTAfterPOP;
    OUString                         m_sInServerName;
    sal_Int16                               m_nInServerPort;
    bool                                m_bInServerPOP;
    OUString                         m_sInServerUserName;
    OUString                         m_sInServerPassword;

    sal_Int16                               m_nMailPort;
    bool                                m_bIsMailReplyTo;
    bool                                m_bIsSecureConnection;
    bool                                m_bIsAuthentication;

    bool                                m_bIsEMailSupported;

    std::vector<std::pair<OUString, int>> m_AddressHeaderSA;

    //these addresses are not stored in the configuration
    std::vector< SwDocMergeInfo >           m_aMergeInfos;

    //we do overwrite the usersettings in a special case
    //then we do remind the usersettings here
    bool                                m_bUserSettingWereOverwritten;
    bool                                m_bIsAddressBlock_LastUserSetting;
    bool                                m_bIsGreetingLineInMail_LastUserSetting;
    bool                                m_bIsGreetingLine_LastUserSetting;

    static const Sequence< OUString>&       GetPropertyNames();

    virtual void ImplCommit() override;

public:
    SwMailMergeConfigItem_Impl();

    virtual void Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    const           Sequence< OUString>
                        GetAddressBlocks(bool bConvertToConfig = false) const;
    void                SetAddressBlocks(
                                const Sequence< OUString>& rBlocks,
                                bool bConvertFromConfig = false);
    const uno::Sequence< OUString>
                        GetGreetings(SwMailMergeConfigItem::Gender eType,
                                        bool bConvertToConfig = false) const;
    void                SetGreetings(SwMailMergeConfigItem::Gender eType,
                                    const uno::Sequence< OUString>& rBlocks,
                                    bool bConvertFromConfig = false);

    void                SetCurrentAddressBlockIndex( sal_Int32 nSet );
    sal_Int32           GetCurrentAddressBlockIndex() const
                        {   return m_nCurrentAddressBlock; }
    sal_Int32           GetCurrentGreeting(SwMailMergeConfigItem::Gender eType) const;
    void                SetCurrentGreeting(SwMailMergeConfigItem::Gender eType, sal_Int32 nIndex);

};

SwMailMergeConfigItem_Impl::SwMailMergeConfigItem_Impl() :
    ConfigItem("Office.Writer/MailMergeWizard", ConfigItemMode::NONE),
        m_nResultSetCursorPos(-1),
        m_nCurrentAddressBlock(0),
        m_bIsAddressBlock(true),
        m_bIsHideEmptyParagraphs(false),
        m_bIsOutputToLetter(true),
        m_bIncludeCountry(false),
        m_bIsGreetingLine(true),
        m_bIsIndividualGreetingLine(false),
        m_nCurrentFemaleGreeting(0),
        m_nCurrentMaleGreeting(0),
        m_nCurrentNeutralGreeting(0),
        m_bIsGreetingLineInMail(false),
        m_bIsIndividualGreetingLineInMail(false),
        m_bIsSMPTAfterPOP(false),
        m_nInServerPort( POP_SECURE_PORT ),
        m_bInServerPOP( true ),
        m_nMailPort(SECURE_PORT),
        m_bIsMailReplyTo(false),
        m_bIsSecureConnection(true),
        m_bIsAuthentication(false),

        m_bIsEMailSupported(false),
        m_bUserSettingWereOverwritten(false),
        m_bIsAddressBlock_LastUserSetting(false),
        m_bIsGreetingLineInMail_LastUserSetting(false),
        m_bIsGreetingLine_LastUserSetting(false)
{
    for (size_t i = 0; i < SAL_N_ELEMENTS(SA_ADDRESS_HEADER); ++i)
    {
        m_AddressHeaderSA.emplace_back(SwResId(SA_ADDRESS_HEADER[i].first), SA_ADDRESS_HEADER[i].second);
    }

    const Sequence<OUString>& rNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(rNames);
    const Any* pValues = aValues.getConstArray();
    assert(aValues.getLength() == rNames.getLength());
    if(aValues.getLength() == rNames.getLength())
    {
        for(int nProp = 0; nProp < rNames.getLength(); nProp++)
        {
            switch(nProp)
            {
                case 0: pValues[nProp] >>= m_bIsOutputToLetter;  break;
                case 1: pValues[nProp] >>= m_bIncludeCountry; break;
                case 2: pValues[nProp] >>= m_sExcludeCountry; break;
                case 3:
                {
                    Sequence< OUString> aBlocks;
                    pValues[nProp] >>= aBlocks;
                    SetAddressBlocks(aBlocks, true);
                }
                break;
                case 4: pValues[nProp] >>= m_bIsAddressBlock; break;
                case 5: pValues[nProp] >>= m_bIsGreetingLine;   break;
                case 6: pValues[nProp] >>= m_bIsIndividualGreetingLine; break;
                case 7 :
                case 8 :
                case 9 :
                {
                    Sequence< OUString> aGreetings;
                    pValues[nProp] >>= aGreetings;
                    SetGreetings(SwMailMergeConfigItem::Gender(
                            SwMailMergeConfigItem::FEMALE + nProp - 7), aGreetings, true);
                }
                break;

                case 10: pValues[nProp] >>= m_nCurrentFemaleGreeting;     break;
                case 11: pValues[nProp] >>= m_nCurrentMaleGreeting;       break;
                case 12: pValues[nProp] >>= m_nCurrentNeutralGreeting;    break;
                case 13: pValues[nProp] >>= m_sFemaleGenderValue;   break;
                case 14: pValues[nProp] >>= m_sMailDisplayName;     break;
                case 15: pValues[nProp] >>= m_sMailAddress;         break;
                case 16: pValues[nProp] >>= m_bIsMailReplyTo;       break;
                case 17: pValues[nProp] >>= m_sMailReplyTo;         break;
                case 18: pValues[nProp] >>= m_sMailServer;          break;
                case 19: pValues[nProp] >>= m_nMailPort;            break;
                case 20: pValues[nProp] >>= m_bIsSecureConnection;           break;
                case 21: pValues[nProp] >>= m_bIsAuthentication;             break;
                case 22: pValues[nProp] >>= m_sMailUserName;                 break;
                case 23: pValues[nProp] >>= m_sMailPassword;                 break;
                case 24 :pValues[nProp] >>= m_aDBData.sDataSource;           break;
                case 25 :pValues[nProp] >>= m_aDBData.sCommand;              break;
                case 26 :
                {
                    short nTemp = 0;
                    if(pValues[nProp] >>= nTemp)
                        m_aDBData.nCommandType = nTemp;
                }
                break;
                case 27: pValues[nProp] >>= m_sFilter; break;
                case 28: pValues[nProp] >>= m_aSavedDocuments; break;
                case 29:
                    pValues[nProp] >>= m_bIsEMailSupported;
                break;
                case 30: pValues[nProp] >>= m_bIsGreetingLineInMail; break;
                case 31: pValues[nProp] >>= m_bIsIndividualGreetingLineInMail; break;
                case 32: pValues[nProp] >>= m_bIsSMPTAfterPOP; break;
                case 33: pValues[nProp] >>= m_sInServerName;    break;
                case 34: pValues[nProp] >>= m_nInServerPort;    break;
                case 35: pValues[nProp] >>= m_bInServerPOP; break;
                case 36: pValues[nProp] >>= m_sInServerUserName; break;
                case 37: pValues[nProp] >>= m_sInServerPassword; break;
                case 38: pValues[nProp] >>= m_bIsHideEmptyParagraphs; break;
                case 39: pValues[nProp] >>= m_nCurrentAddressBlock; break;
            }
        }
    }
    //read the list of data base assignments
    Sequence<OUString> aAssignments = GetNodeNames(cAddressDataAssignments);
    if(aAssignments.getLength())
    {
        //create a list of property names to load the URLs of all data bases
        const OUString* pAssignments = aAssignments.getConstArray();
        Sequence< OUString > aAssignProperties(4 * aAssignments.getLength());
        OUString* pAssignProperties = aAssignProperties.getArray();
        sal_Int32 nAssign;
        for(nAssign = 0; nAssign < aAssignProperties.getLength(); nAssign += 4)
        {
            OUString sAssignPath = cAddressDataAssignments;
            sAssignPath += "/";
            sAssignPath += pAssignments[nAssign / 4];
            sAssignPath += "/";
            pAssignProperties[nAssign] = sAssignPath;
            pAssignProperties[nAssign] += cDataSourceName;
            pAssignProperties[nAssign + 1] = sAssignPath;
            pAssignProperties[nAssign + 1] += cDataTableName;
            pAssignProperties[nAssign + 2] = sAssignPath;
            pAssignProperties[nAssign + 2] += cDataCommandType;
            pAssignProperties[nAssign + 3] = sAssignPath;
            pAssignProperties[nAssign + 3] += cDBColumnAssignments;
        }
        Sequence<Any> aAssignValues = GetProperties(aAssignProperties);
        const Any* pAssignValues = aAssignValues.getConstArray();
        for(nAssign = 0; nAssign < aAssignValues.getLength(); nAssign += 4 )
        {
            DBAddressDataAssignment aAssignment;
            pAssignValues[nAssign] >>=      aAssignment.aDBData.sDataSource;
            pAssignValues[nAssign + 1] >>=  aAssignment.aDBData.sCommand;
            pAssignValues[nAssign + 2] >>=  aAssignment.aDBData.nCommandType;
            pAssignValues[nAssign + 3] >>=  aAssignment.aDBColumnAssignments;
            aAssignment.sConfigNodeName = pAssignments[nAssign / 4];
            m_aAddressDataAssignments.push_back(aAssignment);
        }
    }
    //check if the saved documents still exist
    if(m_aSavedDocuments.getLength())
    {
        uno::Sequence< OUString > aTempDocuments(m_aSavedDocuments.getLength());
        OUString* pTempDocuments = aTempDocuments.getArray();
        sal_Int32 nIndex = 0;
        for(sal_Int32 i = 0; i < m_aSavedDocuments.getLength(); ++i)
        {
            if(SWUnoHelper::UCB_IsFile( m_aSavedDocuments[i] ))
            {
                pTempDocuments[nIndex++] = m_aSavedDocuments[i];
            }
        }
        if(nIndex < m_aSavedDocuments.getLength())
        {
            m_aSavedDocuments = aTempDocuments;
            m_aSavedDocuments.realloc(nIndex);
        }
    }

}

void SwMailMergeConfigItem_Impl::SetCurrentAddressBlockIndex( sal_Int32 nSet )
{
    if(m_aAddressBlocks.size() >= sal::static_int_cast<sal_uInt32, sal_Int32>(nSet))
    {
        m_nCurrentAddressBlock = nSet;
        SetModified();
    }
}

static OUString lcl_CreateNodeName(Sequence<OUString>& rAssignments )
{
    const OUString* pNames = rAssignments.getConstArray();
    sal_Int32 nStart = rAssignments.getLength();
    OUString sNewName;
    bool bFound;
    do
    {
        bFound = false;
        sNewName = "_" + OUString::number(nStart);
        //search if the name exists
        for(sal_Int32 nAssign = 0; nAssign < rAssignments.getLength(); ++nAssign)
        {
            if(pNames[nAssign] == sNewName)
            {
                bFound = true;
                ++nStart;
                break;
            }
        }
    }
    while(bFound);
    // add the new name to the array of existing names
    rAssignments.realloc(rAssignments.getLength() + 1);
    rAssignments.getArray()[rAssignments.getLength() - 1] = sNewName;
    return sNewName;
}

static void lcl_ConvertToNumbers(OUString& rBlock, const std::vector<std::pair<OUString, int>>& rHeaders )
{
    //convert the strings used for UI to numbers used for the configuration
    OUString sBlock(rBlock.replaceAll("\n", "\\n"));
    for (size_t i = 0; i < rHeaders.size(); ++i)
    {
        OUString sHeader = "<" + rHeaders[i].first + ">";
        OUString sReplace = "<" + OUStringLiteral1('0' + i) + ">";
        sBlock = sBlock.replaceAll(sHeader, sReplace);
    }
    rBlock = sBlock;
}

static void lcl_ConvertFromNumbers(OUString& rBlock, const std::vector<std::pair<OUString, int>>& rHeaders)
{
    //convert the numbers used for the configuration to strings used for UI to numbers
    //doesn't use ReplaceAll to prevent expansion of numbers inside of the headers
    SwAddressIterator aGreetingIter(rBlock.replaceAll("\\n", "\n"));
    OUStringBuffer sBlock;
    while(aGreetingIter.HasMore())
    {
        SwMergeAddressItem aNext = aGreetingIter.Next();
        if(aNext.bIsColumn)
        {
            //the text should be 1 characters long
            sal_Unicode cChar = aNext.sText[0];
            if(cChar >= '0' && cChar <= 'c')
            {
                sBlock.append("<");
                sal_uInt16 nHeader = cChar - '0';
                if(nHeader < rHeaders.size())
                    sBlock.append(rHeaders[nHeader].first);
                sBlock.append(">");
            }
            else
            {
                SAL_WARN("sw.ui", "parse error in address block or greeting line");
            }
        }
        else
            sBlock.append(aNext.sText);
    }
    rBlock = sBlock.makeStringAndClear();
}

const Sequence<OUString>& SwMailMergeConfigItem_Impl::GetPropertyNames()
{
    static Sequence<OUString> aNames {
        "OutputToLetter",            // 0
        "IncludeCountry",            // 1
        "ExcludeCountry",            // 2
        "AddressBlockSettings",      // 3
        "IsAddressBlock",            // 4
        "IsGreetingLine",            // 5
        "IsIndividualGreetingLine",  // 6
        "FemaleGreetingLines",       // 7
        "MaleGreetingLines",         // 8
        "NeutralGreetingLines",      // 9
        "CurrentFemaleGreeting",     // 10
        "CurrentMaleGreeting",       // 11
        "CurrentNeutralGreeting",    // 12
        "FemaleGenderValue",         // 13
        "MailDisplayName",           // 14
        "MailAddress",               // 15
        "IsMailReplyTo",             // 16
        "MailReplyTo",               // 17
        "MailServer",                // 18
        "MailPort",                  // 19
        "IsSecureConnection",        // 20
        "IsAuthentication",          // 21
        "MailUserName",              // 22
        "MailPassword",              // 23
        "DataSource/DataSourceName", // 24
        "DataSource/DataTableName",  // 25
        "DataSource/DataCommandType",// 26
        "Filter",                    // 27
        "SavedDocuments",            // 28
        "EMailSupported",            // 29
        "IsEMailGreetingLine",               //30
        "IsEMailIndividualGreetingLine",     //31
        "IsSMPTAfterPOP",                    //32
        "InServerName",                      //33
        "InServerPort",                      //34
        "InServerIsPOP",                     //35
        "InServerUserName",                  //36
        "InServerPassword",                  //37
        "IsHideEmptyParagraphs",             //38
        "CurrentAddressBlock"                //39
    };
    return aNames;
}

void SwMailMergeConfigItem_Impl::Notify( const css::uno::Sequence< OUString >& ) {}

void  SwMailMergeConfigItem_Impl::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case 0: pValues[nProp] <<= m_bIsOutputToLetter; break;
            case 1: pValues[nProp] <<= m_bIncludeCountry; break;
            case 2: pValues[nProp] <<= m_sExcludeCountry; break;
            case 3: pValues[nProp] <<= GetAddressBlocks(true); break;
            case 4:
                {
                    if( m_bUserSettingWereOverwritten)
                        pValues[nProp] <<= m_bIsAddressBlock_LastUserSetting;
                    else
                        pValues[nProp] <<= m_bIsAddressBlock;
                    break;
                }
            case 5:
                {
                    if( m_bUserSettingWereOverwritten)
                        pValues[nProp] <<= m_bIsGreetingLine_LastUserSetting;
                    else
                        pValues[nProp] <<= m_bIsGreetingLine;
                    break;
                }
            case 6: pValues[nProp] <<= m_bIsIndividualGreetingLine;  break;
            case 7:
            case 8:
            case 9:
                pValues[nProp] <<= GetGreetings(
                        SwMailMergeConfigItem::Gender(
                            SwMailMergeConfigItem::FEMALE + nProp - 7), true);
            break;
            case 10: pValues[nProp] <<= m_nCurrentFemaleGreeting;     break;
            case 11: pValues[nProp] <<= m_nCurrentMaleGreeting;       break;
            case 12: pValues[nProp] <<= m_nCurrentNeutralGreeting;    break;
            case 13: pValues[nProp] <<= m_sFemaleGenderValue;     break;
            case 14: pValues[nProp] <<= m_sMailDisplayName;     break;
            case 15: pValues[nProp] <<= m_sMailAddress;         break;
            case 16: pValues[nProp] <<= m_bIsMailReplyTo;        break;
            case 17: pValues[nProp] <<= m_sMailReplyTo;         break;
            case 18: pValues[nProp] <<= m_sMailServer;          break;
            case 19: pValues[nProp] <<= m_nMailPort;            break;
            case 20: pValues[nProp] <<= m_bIsSecureConnection;  break;
            case 21: pValues[nProp] <<= m_bIsAuthentication;    break;
            case 22: pValues[nProp] <<= m_sMailUserName;        break;
            case 23: pValues[nProp] <<= m_sMailPassword;        break;
            case 24 :pValues[nProp] <<= m_aDBData.sDataSource;           break;
            case 25 :pValues[nProp] <<= m_aDBData.sCommand;              break;
            case 26 :pValues[nProp] <<= static_cast<short>(m_aDBData.nCommandType);   break;
            case 27 :pValues[nProp] <<= m_sFilter; break;
            case 28 :pValues[nProp] <<= m_aSavedDocuments; break;
            case 29: pValues[nProp] <<= m_bIsEMailSupported; break;
            case 30:
                {
                    if( m_bUserSettingWereOverwritten)
                        pValues[nProp] <<= m_bIsGreetingLineInMail_LastUserSetting;
                    else
                        pValues[nProp] <<= m_bIsGreetingLineInMail;
                    break;
                }
            case 31: pValues[nProp] <<= m_bIsIndividualGreetingLineInMail; break;
            case 32: pValues[nProp] <<= m_bIsSMPTAfterPOP; break;
            case 33: pValues[nProp] <<= m_sInServerName;    break;
            case 34: pValues[nProp] <<= m_nInServerPort;    break;
            case 35: pValues[nProp] <<= m_bInServerPOP; break;
            case 36: pValues[nProp] <<= m_sInServerUserName; break;
            case 37: pValues[nProp] <<= m_sInServerPassword; break;
            case 38: pValues[nProp] <<= m_bIsHideEmptyParagraphs; break;
            case 39: pValues[nProp] <<= m_nCurrentAddressBlock; break;
        }
    }
    PutProperties(aNames, aValues);
    //store the changed / new assignments

    //load the existing node names to find new names
    Sequence<OUString> aAssignments = GetNodeNames(cAddressDataAssignments);

    for(const auto& rAssignment : m_aAddressDataAssignments)
    {
        if(rAssignment.bColumnAssignmentsChanged)
        {
            //create a new node name
            OUString sNewNode = !rAssignment.sConfigNodeName.isEmpty() ?
                        rAssignment.sConfigNodeName :
                        lcl_CreateNodeName(aAssignments);
            OUString sSlash = "/";
            OUString sNodePath = cAddressDataAssignments;
            sNodePath += sSlash;
            sNodePath += sNewNode;
            sNodePath += sSlash;
            //only one new entry is written
            Sequence< PropertyValue > aNewValues(4);
            PropertyValue* pNewValues = aNewValues.getArray();
            pNewValues[0].Name = sNodePath;
            pNewValues[0].Name += cDataSourceName;
            pNewValues[0].Value <<= rAssignment.aDBData.sDataSource;
            pNewValues[1].Name = sNodePath;
            pNewValues[1].Name += cDataTableName;
            pNewValues[1].Value <<= rAssignment.aDBData.sCommand;
            pNewValues[2].Name = sNodePath;
            pNewValues[2].Name += cDataCommandType;
            pNewValues[2].Value <<= rAssignment.aDBData.nCommandType;
            pNewValues[3].Name = sNodePath;
            pNewValues[3].Name += cDBColumnAssignments;
            pNewValues[3].Value <<= rAssignment.aDBColumnAssignments;

            SetSetProperties(cAddressDataAssignments, aNewValues);
        }
    }

    m_bUserSettingWereOverwritten = false;
}

const Sequence< OUString> SwMailMergeConfigItem_Impl::GetAddressBlocks(
        bool bConvertToConfig) const
{
    Sequence< OUString> aRet(m_aAddressBlocks.size());
    OUString* pRet = aRet.getArray();
    for(size_t nBlock = 0; nBlock < m_aAddressBlocks.size(); nBlock++)
    {
        pRet[nBlock] = m_aAddressBlocks[nBlock];
        if(bConvertToConfig)
            lcl_ConvertToNumbers(pRet[nBlock], m_AddressHeaderSA);
    }
    return aRet;
}

void SwMailMergeConfigItem_Impl::SetAddressBlocks(
        const Sequence< OUString>& rBlocks,
        bool bConvertFromConfig)
{
    m_aAddressBlocks.clear();
    for(sal_Int32 nBlock = 0; nBlock < rBlocks.getLength(); nBlock++)
    {
        OUString sBlock = rBlocks[nBlock];
        if(bConvertFromConfig)
            lcl_ConvertFromNumbers(sBlock, m_AddressHeaderSA);
        m_aAddressBlocks.push_back(sBlock);
    }
    m_nCurrentAddressBlock = 0;
    SetModified();
}

const Sequence< OUString>   SwMailMergeConfigItem_Impl::GetGreetings(
        SwMailMergeConfigItem::Gender eType, bool bConvertToConfig) const
{
    const std::vector< OUString>& rGreetings =
            eType == SwMailMergeConfigItem::FEMALE ? m_aFemaleGreetingLines :
            eType == SwMailMergeConfigItem::MALE ? m_aMaleGreetingLines :
                                m_aNeutralGreetingLines;
    Sequence< OUString> aRet(rGreetings.size());
    OUString* pRet = aRet.getArray();
    for(size_t nGreeting = 0; nGreeting < rGreetings.size(); nGreeting++)
    {
        pRet[nGreeting] = rGreetings[nGreeting];
        if(bConvertToConfig)
            lcl_ConvertToNumbers(pRet[nGreeting], m_AddressHeaderSA);
    }
    return aRet;
}

void  SwMailMergeConfigItem_Impl::SetGreetings(
        SwMailMergeConfigItem::Gender eType,
        const Sequence< OUString>& rSetGreetings,
        bool bConvertFromConfig)
{
    std::vector< OUString>& rGreetings =
            eType == SwMailMergeConfigItem::FEMALE ? m_aFemaleGreetingLines :
            eType == SwMailMergeConfigItem::MALE ? m_aMaleGreetingLines :
                                m_aNeutralGreetingLines;

    rGreetings.clear();
    for(sal_Int32 nGreeting = 0; nGreeting < rSetGreetings.getLength(); nGreeting++)
    {
        OUString sGreeting = rSetGreetings[nGreeting];
        if(bConvertFromConfig)
            lcl_ConvertFromNumbers(sGreeting, m_AddressHeaderSA);
        rGreetings.push_back(sGreeting);
    }
    SetModified();
}

sal_Int32 SwMailMergeConfigItem_Impl::GetCurrentGreeting(
                            SwMailMergeConfigItem::Gender eType) const
{
    sal_Int32 nRet;
    switch(eType)
    {
        case SwMailMergeConfigItem::FEMALE: nRet = m_nCurrentFemaleGreeting ; break;
        case SwMailMergeConfigItem::MALE:   nRet = m_nCurrentMaleGreeting   ; break;
        default:                       nRet = m_nCurrentNeutralGreeting; break;
    }
    return nRet;
}

void SwMailMergeConfigItem_Impl::SetCurrentGreeting(
        SwMailMergeConfigItem::Gender eType, sal_Int32 nIndex)
{
    bool bChanged = false;
    switch(eType)
    {
        case SwMailMergeConfigItem::FEMALE:
            bChanged = m_nCurrentFemaleGreeting != nIndex;
            m_nCurrentFemaleGreeting = nIndex;
        break;
        case SwMailMergeConfigItem::MALE:
            bChanged = m_nCurrentMaleGreeting != nIndex;
            m_nCurrentMaleGreeting = nIndex;
        break;
        default:
            bChanged = m_nCurrentNeutralGreeting != nIndex;
            m_nCurrentNeutralGreeting = nIndex;
    }
    if(bChanged)
        SetModified();
}

SwMailMergeConfigItem::SwMailMergeConfigItem() :
    m_pImpl(new SwMailMergeConfigItem_Impl),
    m_bAddressInserted(false),
    m_bGreetingInserted(false),
    m_nGreetingMoves(0),
    m_pSourceView(nullptr),
    m_pTargetView(nullptr)
{
}

void SwMailMergeConfigItem::stopDBChangeListening()
{
    if (m_xDBChangedListener.is())
    {
        uno::Reference<view::XSelectionSupplier> xSupplier = m_pSourceView->GetUNOObject();
        xSupplier->removeSelectionChangeListener(m_xDBChangedListener);
        m_xDBChangedListener.clear();
    }
}

void SwMailMergeConfigItem::updateCurrentDBDataFromDocument()
{
    const SwDBData& rDBData = m_pSourceView->GetWrtShell().GetDBDesc();
    SetCurrentDBData(rDBData);
}

SwMailMergeConfigItem::~SwMailMergeConfigItem()
{
    stopDBChangeListening();
}

void  SwMailMergeConfigItem::Commit()
{
    if(m_pImpl->IsModified())
        m_pImpl->Commit();
}

const std::vector<std::pair<OUString, int>>&   SwMailMergeConfigItem::GetDefaultAddressHeaders() const
{
    return m_pImpl->m_AddressHeaderSA;
}

void SwMailMergeConfigItem::SetAddressBlocks(
        const Sequence< OUString>& rBlocks)
{
    m_pImpl->SetAddressBlocks(rBlocks);
}

const Sequence< OUString> SwMailMergeConfigItem::GetAddressBlocks() const
{
    return m_pImpl->GetAddressBlocks();
}

bool SwMailMergeConfigItem::IsAddressBlock()const
{
    return m_pImpl->m_bIsAddressBlock && IsOutputToLetter();
}

void     SwMailMergeConfigItem::SetAddressBlock(bool bSet)
{
    m_pImpl->m_bUserSettingWereOverwritten = false;
    if(m_pImpl->m_bIsAddressBlock != bSet)
    {
        m_pImpl->m_bIsAddressBlock = bSet;
        m_pImpl->SetModified();
    }
}

bool SwMailMergeConfigItem::IsHideEmptyParagraphs() const
{
    return m_pImpl->m_bIsHideEmptyParagraphs;
}

void SwMailMergeConfigItem::SetHideEmptyParagraphs(bool bSet)
{
    if(m_pImpl->m_bIsHideEmptyParagraphs != bSet)
    {
        m_pImpl->m_bIsHideEmptyParagraphs = bSet;
        m_pImpl->SetModified();
    }
}

bool SwMailMergeConfigItem::IsIncludeCountry() const
{
    return m_pImpl->m_bIncludeCountry;
}

OUString& SwMailMergeConfigItem::GetExcludeCountry() const
{
    return m_pImpl->m_sExcludeCountry;
}

void SwMailMergeConfigItem::SetCountrySettings(bool bSet, const OUString& rCountry)
{
    if(m_pImpl->m_sExcludeCountry != rCountry ||
        m_pImpl->m_bIncludeCountry != bSet)
    {
        m_pImpl->m_bIncludeCountry = bSet;
        m_pImpl->m_sExcludeCountry = bSet ? rCountry : OUString();
        m_pImpl->SetModified();
    }
}

void SwMailMergeConfigItem::SetCurrentConnection(
        Reference< XDataSource> const & xSource,
        const SharedConnection&       rConnection,
        Reference< XColumnsSupplier> const & xColumnsSupplier,
        const SwDBData& rDBData)
{
        m_pImpl->m_xSource            = xSource         ;
        m_pImpl->m_xConnection        = rConnection     ;
        m_pImpl->m_xColumnsSupplier   = xColumnsSupplier;
        m_pImpl->m_aDBData = rDBData;
        m_pImpl->m_xResultSet         = nullptr;
        m_pImpl->m_nResultSetCursorPos = 0;
        m_pImpl->SetModified();
}

Reference< XDataSource> const & SwMailMergeConfigItem::GetSource()
{
    return m_pImpl->m_xSource;
}

SharedConnection const & SwMailMergeConfigItem::GetConnection()
{
    return m_pImpl->m_xConnection;
}

Reference< XColumnsSupplier> const & SwMailMergeConfigItem::GetColumnsSupplier()
{
    if(!m_pImpl->m_xColumnsSupplier.is() && m_pImpl->m_xConnection.is())
    {
        m_pImpl->m_xColumnsSupplier = SwDBManager::GetColumnSupplier(m_pImpl->m_xConnection,
                                m_pImpl->m_aDBData.sCommand,
                                m_pImpl->m_aDBData.nCommandType == CommandType::TABLE ?
                                        SwDBSelect::TABLE : SwDBSelect::QUERY );
    }
    return m_pImpl->m_xColumnsSupplier;
}

const SwDBData&    SwMailMergeConfigItem::GetCurrentDBData() const
{
    return m_pImpl->m_aDBData;
}

void SwMailMergeConfigItem::SetCurrentDBData( const SwDBData& rDBData)
{
    if(m_pImpl->m_aDBData != rDBData)
    {
        m_pImpl->m_aDBData = rDBData;
        m_pImpl->m_xConnection.clear();
        m_pImpl->m_xSource = nullptr;
        m_pImpl->m_xResultSet = nullptr;
        m_pImpl->m_xColumnsSupplier = nullptr;
        m_pImpl->SetModified();
    }
}

Reference< XResultSet> const & SwMailMergeConfigItem::GetResultSet() const
{
    if(!m_pImpl->m_xConnection.is() && !m_pImpl->m_aDBData.sDataSource.isEmpty())
    {
        m_pImpl->m_xConnection.reset(
            SwDBManager::GetConnection(m_pImpl->m_aDBData.sDataSource, m_pImpl->m_xSource, m_pSourceView),
            SharedConnection::TakeOwnership
        );
    }
    if(!m_pImpl->m_xResultSet.is() && m_pImpl->m_xConnection.is())
    {
        try
        {
            Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );

            Reference<XRowSet> xRowSet( xMgr->createInstance("com.sun.star.sdb.RowSet"), UNO_QUERY );
            Reference<XPropertySet> xRowProperties(xRowSet, UNO_QUERY);
            xRowProperties->setPropertyValue("DataSourceName", makeAny(m_pImpl->m_aDBData.sDataSource));
            xRowProperties->setPropertyValue("Command", makeAny(m_pImpl->m_aDBData.sCommand));
            xRowProperties->setPropertyValue("CommandType", makeAny(m_pImpl->m_aDBData.nCommandType));
            xRowProperties->setPropertyValue("FetchSize", makeAny(sal_Int32(10)));
            xRowProperties->setPropertyValue("ActiveConnection", makeAny(m_pImpl->m_xConnection.getTyped()));
            try
            {
                xRowProperties->setPropertyValue("ApplyFilter", makeAny(!m_pImpl->m_sFilter.isEmpty()));
                xRowProperties->setPropertyValue("Filter", makeAny(m_pImpl->m_sFilter));
            }
            catch (const Exception&)
            {
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN("sw.ui", "exception caught: " << exceptionToString(ex));
            }
            xRowSet->execute();
            m_pImpl->m_xResultSet = xRowSet.get();
            m_pImpl->m_xResultSet->first();
            m_pImpl->m_nResultSetCursorPos = 1;
        }
        catch (const Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("sw.ui", "exception caught in: SwMailMergeConfigItem::GetResultSet() " << exceptionToString(ex));
        }
    }
    return m_pImpl->m_xResultSet;
}

void SwMailMergeConfigItem::DisposeResultSet()
{
    m_pImpl->m_xConnection.clear();
    if(m_pImpl->m_xResultSet.is())
    {
        ::comphelper::disposeComponent( m_pImpl->m_xResultSet );
    }
}

OUString&    SwMailMergeConfigItem::GetFilter() const
{
    return m_pImpl->m_sFilter;
}

void  SwMailMergeConfigItem::SetFilter(OUString const & rFilter)
{
    if(m_pImpl->m_sFilter != rFilter)
    {
        m_pImpl->m_sFilter = rFilter;
        m_pImpl->SetModified();
        Reference<XPropertySet> xRowProperties(m_pImpl->m_xResultSet, UNO_QUERY);
        if(xRowProperties.is())
        {
            try
            {
                xRowProperties->setPropertyValue("ApplyFilter", makeAny(!m_pImpl->m_sFilter.isEmpty()));
                xRowProperties->setPropertyValue("Filter", makeAny(m_pImpl->m_sFilter));
                uno::Reference<XRowSet> xRowSet( m_pImpl->m_xResultSet, UNO_QUERY_THROW );
                xRowSet->execute();
            }
            catch (const Exception&)
            {
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN("sw.ui", "exception caught in SwMailMergeConfigItem::SetFilter(): " << exceptionToString(ex));
            }
        }
    }
}

sal_Int32 SwMailMergeConfigItem::MoveResultSet(sal_Int32 nTarget)
{
    if(!m_pImpl->m_xResultSet.is())
        GetResultSet();
    if(m_pImpl->m_xResultSet.is())
    {
        try
        {
            //no action if the resultset is already at the right position
            if(m_pImpl->m_xResultSet->getRow() != nTarget)
            {
                if(nTarget > 0)
                {
                    bool bMoved = m_pImpl->m_xResultSet->absolute(nTarget);
                    if(!bMoved)
                    {
                        if(nTarget > 1)
                            m_pImpl->m_xResultSet->last();
                        else if(nTarget == 1)
                            m_pImpl->m_xResultSet->first();
                    }
                }
                else if(nTarget == -1)
                    m_pImpl->m_xResultSet->last();
                m_pImpl->m_nResultSetCursorPos = m_pImpl->m_xResultSet->getRow();
            }
        }
        catch (const Exception&)
        {
        }
    }
    return m_pImpl->m_nResultSetCursorPos;
}

bool SwMailMergeConfigItem::IsResultSetFirstLast(bool& bIsFirst, bool& bIsLast)
{
    bool bRet = false;
    if(!m_pImpl->m_xResultSet.is())
        GetResultSet();
    if(m_pImpl->m_xResultSet.is())
    {
        try
        {
            bIsFirst = m_pImpl->m_xResultSet->isFirst();
            bIsLast = m_pImpl->m_xResultSet->isLast();
            bRet = true;
        }
        catch (const Exception&)
        {
        }
    }
    return bRet;
}

sal_Int32 SwMailMergeConfigItem::GetResultSetPosition() const
{
    return m_pImpl->m_nResultSetCursorPos;
}

bool SwMailMergeConfigItem::IsRecordExcluded(sal_Int32 nRecord) const
    { return m_aExcludedRecords.find(nRecord) != m_aExcludedRecords.end(); }

void SwMailMergeConfigItem::ExcludeRecord(sal_Int32 nRecord, bool bExclude)
{
    if(bExclude)
        m_aExcludedRecords.insert(nRecord);
    else
        m_aExcludedRecords.erase(nRecord);
}

uno::Sequence<uno::Any> SwMailMergeConfigItem::GetSelection() const
{
    if(!m_pImpl->m_xResultSet.is())
        GetResultSet();
    if(!m_pImpl->m_xResultSet.is())
        return {};
    m_pImpl->m_xResultSet->last();
    sal_Int32 nResultSetSize = m_pImpl->m_xResultSet->getRow()+1;
    std::vector<uno::Any> vResult;
    vResult.reserve(nResultSetSize);
    for(sal_Int32 nIdx=1; nIdx<nResultSetSize;++nIdx)
        if(!IsRecordExcluded(nIdx))
            vResult.push_back(uno::makeAny<sal_Int32>(nIdx));
    return comphelper::containerToSequence(vResult);
}


const uno::Sequence< OUString>&
                    SwMailMergeConfigItem::GetSavedDocuments() const
{
    return m_pImpl->m_aSavedDocuments;
}

bool SwMailMergeConfigItem::IsOutputToLetter()const
{
    return m_pImpl->m_bIsOutputToLetter || !IsMailAvailable();
}

void SwMailMergeConfigItem::SetOutputToLetter(bool bSet)
{
    if(m_pImpl->m_bIsOutputToLetter != bSet)
    {
        m_pImpl->m_bIsOutputToLetter = bSet;
        m_pImpl->SetModified();
    }
}

bool SwMailMergeConfigItem::IsIndividualGreeting(bool bInEMail) const
{
    return bInEMail ?
            m_pImpl->m_bIsIndividualGreetingLineInMail :
            m_pImpl->m_bIsIndividualGreetingLine;
}

void     SwMailMergeConfigItem::SetIndividualGreeting(
                                        bool bSet, bool bInEMail)
{
    if(bInEMail)
    {
        if(m_pImpl->m_bIsIndividualGreetingLineInMail != bSet)
        {
            m_pImpl->m_bIsIndividualGreetingLineInMail = bSet;
            m_pImpl->SetModified();
        }
    }
    else
    {
        if(m_pImpl->m_bIsIndividualGreetingLine != bSet)
        {
            m_pImpl->m_bIsIndividualGreetingLine = bSet;
            m_pImpl->SetModified();
        }
    }
}

bool SwMailMergeConfigItem::IsGreetingLine(bool bInEMail) const
{
    return bInEMail ? m_pImpl->m_bIsGreetingLineInMail : m_pImpl->m_bIsGreetingLine;
}

void     SwMailMergeConfigItem::SetGreetingLine(bool bSet, bool bInEMail)
{
    m_pImpl->m_bUserSettingWereOverwritten = false;
    if(bInEMail)
    {
        if(m_pImpl->m_bIsGreetingLineInMail != bSet)
        {
            m_pImpl->m_bIsGreetingLineInMail = bSet;
            m_pImpl->SetModified();
        }
    }
    else
    {
        if(m_pImpl->m_bIsGreetingLine != bSet)
        {
            m_pImpl->m_bIsGreetingLine = bSet;
            m_pImpl->SetModified();
        }
    }
}

const Sequence< OUString>   SwMailMergeConfigItem::GetGreetings(
        Gender eType ) const
{
    return m_pImpl->GetGreetings(eType);
}

void  SwMailMergeConfigItem::SetGreetings(
        Gender eType, const Sequence< OUString>& rSetGreetings)
{
    m_pImpl->SetGreetings( eType, rSetGreetings);
}

sal_Int32 SwMailMergeConfigItem::GetCurrentGreeting(
                        SwMailMergeConfigItem::Gender eType) const
{
    return m_pImpl->GetCurrentGreeting(eType);
}

void SwMailMergeConfigItem::SetCurrentGreeting(Gender eType, sal_Int32 nIndex)
{
    m_pImpl->SetCurrentGreeting(eType, nIndex);
}

const OUString& SwMailMergeConfigItem::GetFemaleGenderValue() const
{
    return m_pImpl->m_sFemaleGenderValue;
}

void SwMailMergeConfigItem::SetFemaleGenderValue(const OUString& rValue)
{
    if( m_pImpl->m_sFemaleGenderValue != rValue )
    {
        m_pImpl->m_sFemaleGenderValue = rValue;
        m_pImpl->SetModified();
    }
}

Sequence< OUString> SwMailMergeConfigItem::GetColumnAssignment(
                const SwDBData& rDBData ) const
{
    Sequence< OUString> aRet;
    auto aAssignIter = std::find_if(m_pImpl->m_aAddressDataAssignments.begin(), m_pImpl->m_aAddressDataAssignments.end(),
        [&rDBData](const DBAddressDataAssignment& rAssignment) { return rAssignment.aDBData == rDBData; });
    if (aAssignIter != m_pImpl->m_aAddressDataAssignments.end())
    {
        aRet = aAssignIter->aDBColumnAssignments;
    }
    return aRet;
}

// returns the name that is assigned as e-mail column of the current data base
OUString     SwMailMergeConfigItem::GetAssignedColumn(sal_uInt32 nColumn) const
{
    OUString sRet;
    Sequence< OUString> aAssignment = GetColumnAssignment( m_pImpl->m_aDBData );
    if(aAssignment.getLength() > sal::static_int_cast< sal_Int32, sal_uInt32>(nColumn) && !aAssignment[nColumn].isEmpty())
        sRet = aAssignment[nColumn];
    else if(nColumn < m_pImpl->m_AddressHeaderSA.size())
        sRet = m_pImpl->m_AddressHeaderSA[nColumn].first;
    return sRet;
}

void SwMailMergeConfigItem::SetColumnAssignment( const SwDBData& rDBData,
                            const Sequence< OUString>& rList)
{
    auto aAssignIter = std::find_if(m_pImpl->m_aAddressDataAssignments.begin(), m_pImpl->m_aAddressDataAssignments.end(),
        [&rDBData](const DBAddressDataAssignment& rAssignment) { return rAssignment.aDBData == rDBData; });
    if (aAssignIter != m_pImpl->m_aAddressDataAssignments.end())
    {
        if(aAssignIter->aDBColumnAssignments != rList)
        {
            aAssignIter->aDBColumnAssignments = rList;
            aAssignIter->bColumnAssignmentsChanged = true;
        }
    }
    else
    {
        DBAddressDataAssignment aAssignment;
        aAssignment.aDBData = rDBData;
        aAssignment.aDBColumnAssignments = rList;
        aAssignment.bColumnAssignmentsChanged = true;
        m_pImpl->m_aAddressDataAssignments.push_back(aAssignment);
    }
    m_pImpl->SetModified();
}

bool SwMailMergeConfigItem::IsAddressFieldsAssigned() const
{
    bool bResult = true;
    Reference< XResultSet> xResultSet = GetResultSet();
    uno::Reference< XColumnsSupplier > xColsSupp( xResultSet, UNO_QUERY );
    if(!xColsSupp.is())
        return false;
    uno::Reference<container::XNameAccess> xCols = xColsSupp->getColumns();

    const std::vector<std::pair<OUString, int>>& rHeaders = GetDefaultAddressHeaders();
    Sequence< OUString> aAssignment =
                        GetColumnAssignment( GetCurrentDBData() );
    const OUString* pAssignment = aAssignment.getConstArray();
    const Sequence< OUString> aBlocks = GetAddressBlocks();

    if(aBlocks.getLength() <= m_pImpl->GetCurrentAddressBlockIndex())
        return false;
    SwAddressIterator aIter(aBlocks[m_pImpl->GetCurrentAddressBlockIndex()]);
    while(aIter.HasMore())
    {
        SwMergeAddressItem aItem = aIter.Next();
        if(aItem.bIsColumn)
        {
            OUString sConvertedColumn = aItem.sText;
            for(sal_uInt32 nColumn = 0;
                    nColumn < rHeaders.size() && nColumn < sal_uInt32(aAssignment.getLength());
                                                                                ++nColumn)
            {
                if (rHeaders[nColumn].first == aItem.sText &&
                    !pAssignment[nColumn].isEmpty())
                {
                    sConvertedColumn = pAssignment[nColumn];
                    break;
                }
            }
            //find out if the column exists in the data base
            if(!xCols->hasByName(sConvertedColumn))
            {
                bResult = false;
                break;
            }
        }
    }
    return bResult;
}

bool SwMailMergeConfigItem::IsGreetingFieldsAssigned() const
{
    bool bResult = true;

    if(!IsIndividualGreeting(false))
        return true;

    Reference< XResultSet> xResultSet = GetResultSet();
    uno::Reference< XColumnsSupplier > xColsSupp( xResultSet, UNO_QUERY );
    if(!xColsSupp.is())
        return false;
    const std::vector<std::pair<OUString, int>>& rHeaders = GetDefaultAddressHeaders();
    uno::Reference<container::XNameAccess> xCols = xColsSupp->getColumns();

    Sequence< OUString> aAssignment =
                        GetColumnAssignment( GetCurrentDBData() );
    const OUString* pAssignment = aAssignment.getConstArray();

    const Sequence< OUString> rFemaleEntries = GetGreetings(SwMailMergeConfigItem::FEMALE);
    sal_Int32 nCurrentFemale = GetCurrentGreeting(SwMailMergeConfigItem::FEMALE);
    const Sequence< OUString> rMaleEntries = GetGreetings(SwMailMergeConfigItem::MALE);
    sal_Int32 nCurrentMale = GetCurrentGreeting(SwMailMergeConfigItem::MALE);
    OUString sMale, sFemale;
    if(rFemaleEntries.getLength() > nCurrentFemale)
        sFemale = rFemaleEntries[nCurrentFemale];
    if(rMaleEntries.getLength() > nCurrentMale)
        sMale = rMaleEntries[nCurrentMale];

    OUString sAddress( sFemale );
    sAddress += sMale;
    SwAddressIterator aIter(sAddress);
    while(aIter.HasMore())
    {
        SwMergeAddressItem aItem = aIter.Next();
        if(aItem.bIsColumn)
        {
            OUString sConvertedColumn = aItem.sText;
            for(sal_uInt32 nColumn = 0;
                    nColumn < rHeaders.size() && nColumn < sal_uInt32(aAssignment.getLength());
                                                                                ++nColumn)
            {
                if (rHeaders[nColumn].first == aItem.sText &&
                    !pAssignment[nColumn].isEmpty())
                {
                    sConvertedColumn = pAssignment[nColumn];
                    break;
                }
            }
            //find out if the column exists in the data base
            if(!xCols->hasByName(sConvertedColumn))
            {
                bResult = false;
                break;
            }
        }
    }
    return bResult;
}

OUString const & SwMailMergeConfigItem::GetMailDisplayName() const
{
    return m_pImpl->m_sMailDisplayName;
}

void SwMailMergeConfigItem::SetMailDisplayName(const OUString& rName)
{
    if(m_pImpl->m_sMailDisplayName != rName)
    {
        m_pImpl->m_sMailDisplayName = rName;
        m_pImpl->SetModified();
    }
}

OUString const & SwMailMergeConfigItem::GetMailAddress() const
{
    return m_pImpl->m_sMailAddress;
}

void SwMailMergeConfigItem::SetMailAddress(const OUString& rAddress)
{
    if(m_pImpl->m_sMailAddress != rAddress )
    {
        m_pImpl->m_sMailAddress = rAddress;
        m_pImpl->SetModified();
    }
}

bool SwMailMergeConfigItem::IsMailReplyTo() const
{
    return m_pImpl->m_bIsMailReplyTo;
}

void  SwMailMergeConfigItem::SetMailReplyTo(bool bSet)
{
    if(m_pImpl->m_bIsMailReplyTo != bSet)
    {
        m_pImpl->m_bIsMailReplyTo = bSet;
        m_pImpl->SetModified();
    }
}

OUString const & SwMailMergeConfigItem::GetMailReplyTo() const
{
    return m_pImpl->m_sMailReplyTo;
}

void SwMailMergeConfigItem::SetMailReplyTo(const OUString& rReplyTo)
{
    if(m_pImpl->m_sMailReplyTo != rReplyTo)
    {
        m_pImpl->m_sMailReplyTo = rReplyTo;
        m_pImpl->SetModified();
    }
}

OUString const & SwMailMergeConfigItem::GetMailServer() const
{
    return m_pImpl->m_sMailServer;
}

void SwMailMergeConfigItem::SetMailServer(const OUString& rAddress)
{
    if(m_pImpl->m_sMailServer != rAddress)
    {
        m_pImpl->m_sMailServer = rAddress;
        m_pImpl->SetModified();
    }
}

sal_Int16 SwMailMergeConfigItem::GetMailPort() const
{
    // provide appropriate TCP port, based on SSL/STARTTLS status, if current port is one of the defaults
    switch (m_pImpl->m_nMailPort)
    {
    case SECURE_PORT:
    case DEFAULT_PORT:
        return m_pImpl->m_bIsSecureConnection ? SECURE_PORT : DEFAULT_PORT;
        break;
    default:
        return m_pImpl->m_nMailPort;
    }
}

void     SwMailMergeConfigItem::SetMailPort(sal_Int16 nSet)
{
    if(m_pImpl->m_nMailPort != nSet)
    {
        m_pImpl->m_nMailPort = nSet;
        m_pImpl->SetModified();
    }
}

bool SwMailMergeConfigItem::IsSecureConnection() const
{
    return m_pImpl->m_bIsSecureConnection;
}

void     SwMailMergeConfigItem::SetSecureConnection(bool bSet)
{
    if(m_pImpl->m_bIsSecureConnection != bSet)
    {
        m_pImpl->m_bIsSecureConnection = bSet;
        m_pImpl->SetModified();
    }
}

bool SwMailMergeConfigItem::IsAuthentication() const
{
    return m_pImpl->m_bIsAuthentication;
}

void SwMailMergeConfigItem::SetAuthentication(bool bSet)
{
    if(m_pImpl->m_bIsAuthentication != bSet)
    {
        m_pImpl->m_bIsAuthentication = bSet;
        m_pImpl->SetModified();
    }
}

OUString const & SwMailMergeConfigItem::GetMailUserName() const
{
    return m_pImpl->m_sMailUserName;
}

void SwMailMergeConfigItem::SetMailUserName(const OUString& rName)
{
    if(m_pImpl->m_sMailUserName != rName)
    {
        m_pImpl->m_sMailUserName = rName;
        m_pImpl->SetModified();
    }
}

OUString const & SwMailMergeConfigItem::GetMailPassword() const
{
    return m_pImpl->m_sMailPassword;
}

void SwMailMergeConfigItem::SetMailPassword(const OUString& rPassword)
{
    if(m_pImpl->m_sMailPassword != rPassword)
    {
        m_pImpl->m_sMailPassword = rPassword;
        m_pImpl->SetModified();
    }
}

bool SwMailMergeConfigItem::IsSMTPAfterPOP() const
{
    return m_pImpl->m_bIsSMPTAfterPOP;
}

void SwMailMergeConfigItem::SetSMTPAfterPOP(bool bSet)
{
    if( m_pImpl->m_bIsSMPTAfterPOP != bSet)
    {
        m_pImpl->m_bIsSMPTAfterPOP = bSet;
        m_pImpl->SetModified();
    }
}

OUString const & SwMailMergeConfigItem::GetInServerName() const
{
    return m_pImpl->m_sInServerName;
}

void SwMailMergeConfigItem::SetInServerName(const OUString& rServer)
{
    if(m_pImpl->m_sInServerName != rServer)
    {
        m_pImpl->m_sInServerName = rServer;
        m_pImpl->SetModified();
    }
}

sal_Int16           SwMailMergeConfigItem::GetInServerPort() const
{
    // provide appropriate TCP port as user toggles between POP/IMAP if current port is one of the defaults
    switch (m_pImpl->m_nInServerPort)
    {
    case POP_SECURE_PORT:
    case POP_PORT:
    case IMAP_SECURE_PORT:
    case IMAP_PORT:
        if ( m_pImpl->m_bInServerPOP )
            return m_pImpl->m_bIsSecureConnection ? POP_SECURE_PORT : POP_PORT;
        else
            return m_pImpl->m_bIsSecureConnection ? IMAP_SECURE_PORT : IMAP_PORT;
        break;
    default:
        return m_pImpl->m_nInServerPort;
    }
}

void SwMailMergeConfigItem::SetInServerPort(sal_Int16 nSet)
{
    if( m_pImpl->m_nInServerPort != nSet)
    {
        m_pImpl->m_nInServerPort = nSet;
        m_pImpl->SetModified();
    }
}

bool SwMailMergeConfigItem::IsInServerPOP() const
{
    return m_pImpl->m_bInServerPOP;
}

void SwMailMergeConfigItem::SetInServerPOP(bool bSet)
{
    if( m_pImpl->m_bInServerPOP != bSet)
    {
        m_pImpl->m_bInServerPOP = bSet;
        m_pImpl->SetModified();
    }
}

OUString const & SwMailMergeConfigItem::GetInServerUserName() const
{
    return m_pImpl->m_sInServerUserName;
}

void SwMailMergeConfigItem::SetInServerUserName(const OUString& rName)
{
    if( m_pImpl->m_sInServerUserName != rName)
    {
        m_pImpl->m_sInServerUserName = rName;
        m_pImpl->SetModified();
    }
}

OUString const & SwMailMergeConfigItem::GetInServerPassword() const
{
    return m_pImpl->m_sInServerPassword;
}

void SwMailMergeConfigItem::SetInServerPassword(const OUString& rPassword)
{
    if(m_pImpl->m_sInServerPassword != rPassword)
    {
        m_pImpl->m_sInServerPassword = rPassword;
        m_pImpl->SetModified();
    }
}

void SwMailMergeConfigItem::DocumentReloaded()
{
    m_bGreetingInserted = false;
    m_bAddressInserted = false;
}

bool SwMailMergeConfigItem::IsMailAvailable() const
{
    return m_pImpl->m_bIsEMailSupported;
}

void SwMailMergeConfigItem::AddMergedDocument(SwDocMergeInfo const & rInfo)
{
    m_pImpl->m_aMergeInfos.push_back(rInfo);
}

SwDocMergeInfo& SwMailMergeConfigItem::GetDocumentMergeInfo(sal_uInt32 nDocument)
{
    assert(nDocument < m_pImpl->m_aMergeInfos.size());
    return m_pImpl->m_aMergeInfos[nDocument];
}


sal_uInt32 SwMailMergeConfigItem::GetMergedDocumentCount()
{
    if(m_pTargetView)
        return m_pImpl->m_aMergeInfos.size();
    else
    {
        sal_Int32 nRestore = GetResultSetPosition();
        MoveResultSet(-1);
        sal_Int32 nRet = GetResultSetPosition();
        MoveResultSet( nRestore );
        nRet -= m_aExcludedRecords.size();
        return nRet >= 0 ? nRet : 0;
    }
}

static SwView* lcl_ExistsView(SwView* pView)
{
    SfxViewShell* pViewShell = SfxViewShell::GetFirst( false, checkSfxViewShell<SwView> );
    while(pViewShell)
    {
        if(pViewShell == pView)
            return pView;

        pViewShell = SfxViewShell::GetNext( *pViewShell, false, checkSfxViewShell<SwView> );
    }
    return nullptr;
}

SwView*  SwMailMergeConfigItem::GetTargetView()
{
    //make sure that the pointer is really valid - the document may have been closed manually
    if(m_pTargetView)
    {
        m_pTargetView = lcl_ExistsView(m_pTargetView);
    }
    return m_pTargetView;
}

void  SwMailMergeConfigItem::SetTargetView(SwView* pView)
{
    m_pTargetView = pView;
    //reset the document merge counter
    if(!m_pTargetView)
    {
        m_pImpl->m_aMergeInfos.clear();
    }
}

SwView* SwMailMergeConfigItem::GetSourceView()
{
    m_pSourceView = lcl_ExistsView(m_pSourceView);
    return m_pSourceView;
}

//This implements XSelectionChangeListener and XDispatch because the
//broadcaster uses this combo to determine if to send the database-changed
//update. Its probably that listening to statusChanged at some other level is
//equivalent to this. See the other call to SwXDispatch::GetDBChangeURL for
//the broadcaster of the event.
class DBChangeListener : public cppu::WeakImplHelper<css::view::XSelectionChangeListener, css::frame::XDispatch>
{
    SwMailMergeConfigItem& m_rParent;
public:
    explicit DBChangeListener(SwMailMergeConfigItem& rParent)
        : m_rParent(rParent)
    {
    }

    virtual void SAL_CALL selectionChanged(const EventObject& /*rEvent*/) override
    {
    }

    virtual void SAL_CALL disposing(const EventObject&) override
    {
        m_rParent.stopDBChangeListening();
    }

    virtual void SAL_CALL dispatch(const css::util::URL& rURL, const css::uno::Sequence< css::beans::PropertyValue >& /*rArgs*/) override
    {
        if (rURL.Complete.equalsAscii(SwXDispatch::GetDBChangeURL()))
            m_rParent.updateCurrentDBDataFromDocument();
    }

    virtual void SAL_CALL addStatusListener(const css::uno::Reference< css::frame::XStatusListener >&, const css::util::URL&) override
    {
    }

    virtual void SAL_CALL removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >&, const css::util::URL&) override
    {
    }
};

void SwMailMergeConfigItem::SetSourceView(SwView* pView)
{
    if (m_xDBChangedListener.is())
    {
        uno::Reference<view::XSelectionSupplier> xSupplier = m_pSourceView->GetUNOObject();
        xSupplier->removeSelectionChangeListener(m_xDBChangedListener);
        m_xDBChangedListener.clear();
    }

    m_pSourceView = pView;

    if (!m_pSourceView)
        return;

    std::vector<OUString> aDBNameList;
    std::vector<OUString> aAllDBNames;
    m_pSourceView->GetWrtShell().GetAllUsedDB( aDBNameList, &aAllDBNames );
    if(!aDBNameList.empty())
    {
        // if fields are available there is usually no need of an addressblock and greeting
        if(!m_pImpl->m_bUserSettingWereOverwritten)
        {
            if( m_pImpl->m_bIsAddressBlock
                || m_pImpl->m_bIsGreetingLineInMail
                || m_pImpl->m_bIsGreetingLine )
            {
                //store user settings
                m_pImpl->m_bUserSettingWereOverwritten = true;
                m_pImpl->m_bIsAddressBlock_LastUserSetting = m_pImpl->m_bIsAddressBlock;
                m_pImpl->m_bIsGreetingLineInMail_LastUserSetting = m_pImpl->m_bIsGreetingLineInMail;
                m_pImpl->m_bIsGreetingLine_LastUserSetting = m_pImpl->m_bIsGreetingLine;

                //set all to false
                m_pImpl->m_bIsAddressBlock = false;
                m_pImpl->m_bIsGreetingLineInMail = false;
                m_pImpl->m_bIsGreetingLine = false;

                m_pImpl->SetModified();
            }
        }
    }
    else if( m_pImpl->m_bUserSettingWereOverwritten )
    {
        //restore last user settings:
        m_pImpl->m_bIsAddressBlock = m_pImpl->m_bIsAddressBlock_LastUserSetting;
        m_pImpl->m_bIsGreetingLineInMail = m_pImpl->m_bIsGreetingLineInMail_LastUserSetting;
        m_pImpl->m_bIsGreetingLine = m_pImpl->m_bIsGreetingLine_LastUserSetting;

        m_pImpl->m_bUserSettingWereOverwritten = false;
    }

    if (!m_xDBChangedListener.is())
    {
        m_xDBChangedListener.set(new DBChangeListener(*this));
    }

    uno::Reference<view::XSelectionSupplier> xSupplier = m_pSourceView->GetUNOObject();
    xSupplier->addSelectionChangeListener(m_xDBChangedListener);
}

void SwMailMergeConfigItem::SetCurrentAddressBlockIndex( sal_Int32 nSet )
{
    m_pImpl->SetCurrentAddressBlockIndex( nSet );
}

sal_Int32 SwMailMergeConfigItem::GetCurrentAddressBlockIndex() const
{
    return m_pImpl->GetCurrentAddressBlockIndex();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
