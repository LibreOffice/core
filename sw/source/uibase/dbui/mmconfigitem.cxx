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
#include <osl/diagnose.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/mail/MailServiceType.hpp>
#include <com/sun/star/mail/XMailService.hpp>
#include <com/sun/star/mail/MailServiceProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/sdb/CommandType.hpp>
#include <comphelper/sequence.hxx>
#include <rtl/instance.hxx>
#include <unotools/configitem.hxx>
#include <mailmergehelper.hxx>
#include <swunohelper.hxx>
#include <dbmgr.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <dbui.hrc>
#include <vector>
#include <unomid.h>

using namespace utl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;

const char* cAddressDataAssignments     = "AddressDataAssignments";
const char* cDBColumnAssignments        = "DBColumnAssignments";
const char* cDataSourceName             = "DataSource/DataSourceName";
const char* cDataTableName              = "DataSource/DataTableName" ;
const char* cDataCommandType            = "DataSource/DataCommandType";

#define SECURE_PORT     465
#define DEFAULT_PORT    25
#define POP_PORT        110

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
    Reference< XDataSource>                 xSource;
    SharedConnection                        xConnection;
    Reference< XColumnsSupplier>            xColumnsSupplier;
    Reference< XStatement>                  xStatement;
    Reference< XResultSet>                  xResultSet;
    SwDBData                                aDBData;
    OUString                         sFilter;
    sal_Int32                               nResultSetCursorPos;

    ::std::vector<DBAddressDataAssignment>  aAddressDataAssignments;
    ::std::vector< OUString>         aAddressBlocks;
    sal_Int32                               nCurrentAddressBlock;
    bool                                bIsAddressBlock;
    bool                                bIsHideEmptyParagraphs;

    bool                                bIsOutputToLetter;
    bool                                bIncludeCountry;
    OUString                         sExcludeCountry;

    bool                                bIsGreetingLine;
    bool                                bIsIndividualGreetingLine;
    ::std::vector< OUString>         aFemaleGreetingLines;
    sal_Int32                               nCurrentFemaleGreeting;
    ::std::vector< OUString>         aMaleGreetingLines;
    sal_Int32                               nCurrentMaleGreeting;
    ::std::vector< OUString>         aNeutralGreetingLines;
    sal_Int32                               nCurrentNeutralGreeting;
    OUString                         sFemaleGenderValue;
    uno::Sequence< OUString>         aSavedDocuments;

    bool                                bIsGreetingLineInMail;
    bool                                bIsIndividualGreetingLineInMail;

    //mail settings
    OUString                         sMailDisplayName;
    OUString                         sMailAddress;
    OUString                         sMailReplyTo;
    OUString                         sMailServer;
    OUString                         sMailUserName;
    OUString                         sMailPassword;

    bool                                bIsSMPTAfterPOP;
    OUString                         sInServerName;
    sal_Int16                               nInServerPort;
    bool                                bInServerPOP;
    OUString                         sInServerUserName;
    OUString                         sInServerPassword;

    sal_Int16                               nMailPort;
    bool                                bIsMailReplyTo;
    bool                                bIsDefaultPort;
    bool                                bIsSecureConnection;
    bool                                bIsAuthentication;

    bool                                bIsEMailSupported;

    ResStringArray                          m_AddressHeaderSA;

    //these addresses are not stored in the configuration
    ::std::vector< SwDocMergeInfo >         aMergeInfos;

    //we do overwrite the usersettings in a special case
    //than we do remind the usersettings here
    bool                                bUserSettingWereOverwritten;
    bool                                bIsAddressBlock_LastUserSetting;
    bool                                bIsGreetingLineInMail_LastUserSetting;
    bool                                bIsGreetingLine_LastUserSetting;

    static const Sequence< OUString>&       GetPropertyNames();

    virtual void ImplCommit() override;

public:
    SwMailMergeConfigItem_Impl();
    virtual ~SwMailMergeConfigItem_Impl();

    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) override;
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
                        {   return nCurrentAddressBlock; }
    sal_Int32           GetCurrentGreeting(SwMailMergeConfigItem::Gender eType) const;
    void                SetCurrentGreeting(SwMailMergeConfigItem::Gender eType, sal_Int32 nIndex);

};

SwMailMergeConfigItem_Impl::SwMailMergeConfigItem_Impl() :
    ConfigItem("Office.Writer/MailMergeWizard", ConfigItemMode::ImmediateUpdate),
        nResultSetCursorPos(-1),
        nCurrentAddressBlock(0),
        bIsAddressBlock(true),
        bIsHideEmptyParagraphs(false),
        bIsOutputToLetter(true),
        bIncludeCountry(false),
        bIsGreetingLine(true),
        bIsIndividualGreetingLine(false),
        nCurrentFemaleGreeting(0),
        nCurrentMaleGreeting(0),
        nCurrentNeutralGreeting(0),
        bIsGreetingLineInMail(false),
        bIsIndividualGreetingLineInMail(false),
        bIsSMPTAfterPOP(false),
        nInServerPort( POP_PORT ),
        bInServerPOP( true ),
        nMailPort(0),
        bIsMailReplyTo(false),
        bIsDefaultPort(false),
        bIsSecureConnection(false),
        bIsAuthentication(false),

        bIsEMailSupported(false),
        m_AddressHeaderSA( SW_RES( SA_ADDRESS_HEADER )),
        bUserSettingWereOverwritten(false),
        bIsAddressBlock_LastUserSetting(false),
        bIsGreetingLineInMail_LastUserSetting(false),
        bIsGreetingLine_LastUserSetting(false)
{
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
                case 0: pValues[nProp] >>= bIsOutputToLetter;  break;
                case 1: pValues[nProp] >>= bIncludeCountry; break;
                case 2: pValues[nProp] >>= sExcludeCountry; break;
                case 3:
                {
                    Sequence< OUString> aBlocks;
                    pValues[nProp] >>= aBlocks;
                    SetAddressBlocks(aBlocks, true);
                }
                break;
                case 4: pValues[nProp] >>= bIsAddressBlock; break;
                case 5: pValues[nProp] >>= bIsGreetingLine;   break;
                case 6: pValues[nProp] >>= bIsIndividualGreetingLine; break;
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

                case 10: pValues[nProp] >>= nCurrentFemaleGreeting;     break;
                case 11: pValues[nProp] >>= nCurrentMaleGreeting;       break;
                case 12: pValues[nProp] >>= nCurrentNeutralGreeting;    break;
                case 13: pValues[nProp] >>= sFemaleGenderValue;   break;
                case 14: pValues[nProp] >>= sMailDisplayName;     break;
                case 15: pValues[nProp] >>= sMailAddress;         break;
                case 16: pValues[nProp] >>= bIsMailReplyTo;       break;
                case 17: pValues[nProp] >>= sMailReplyTo;         break;
                case 18: pValues[nProp] >>= sMailServer;          break;
                case 19: bIsDefaultPort = !(pValues[nProp] >>= nMailPort); break;
                case 20: pValues[nProp] >>= bIsSecureConnection;           break;
                case 21: pValues[nProp] >>= bIsAuthentication;             break;
                case 22: pValues[nProp] >>= sMailUserName;                 break;
                case 23: pValues[nProp] >>= sMailPassword;                 break;
                case 24 :pValues[nProp] >>= aDBData.sDataSource;           break;
                case 25 :pValues[nProp] >>= aDBData.sCommand;              break;
                case 26 :
                {
                    short nTemp = 0;
                    if(pValues[nProp] >>= nTemp)
                        aDBData.nCommandType = nTemp;
                }
                break;
                case 27: pValues[nProp] >>= sFilter; break;
                case 28: pValues[nProp] >>= aSavedDocuments; break;
                case 29:
                    pValues[nProp] >>= bIsEMailSupported;
                break;
                case 30: pValues[nProp] >>= bIsGreetingLineInMail; break;
                case 31: pValues[nProp] >>= bIsIndividualGreetingLineInMail; break;
                case 32: pValues[nProp] >>= bIsSMPTAfterPOP; break;
                case 33: pValues[nProp] >>= sInServerName;    break;
                case 34: pValues[nProp] >>= nInServerPort;    break;
                case 35: pValues[nProp] >>= bInServerPOP; break;
                case 36: pValues[nProp] >>= sInServerUserName; break;
                case 37: pValues[nProp] >>= sInServerPassword; break;
                case 38: pValues[nProp] >>= bIsHideEmptyParagraphs; break;
                case 39: pValues[nProp] >>= nCurrentAddressBlock; break;
            }
        }
    }
    //read the list of data base assignments
    Sequence<OUString> aAssignments = GetNodeNames(OUString::createFromAscii(cAddressDataAssignments));
    if(aAssignments.getLength())
    {
        //create a list of property names to load the URLs of all data bases
        const OUString* pAssignments = aAssignments.getConstArray();
        Sequence< OUString > aAssignProperties(4 * aAssignments.getLength());
        OUString* pAssignProperties = aAssignProperties.getArray();
        sal_Int32 nAssign;
        for(nAssign = 0; nAssign < aAssignProperties.getLength(); nAssign += 4)
        {
            OUString sAssignPath = OUString::createFromAscii(cAddressDataAssignments);
            sAssignPath += "/";
            sAssignPath += pAssignments[nAssign / 4];
            sAssignPath += "/";
            pAssignProperties[nAssign] = sAssignPath;
            pAssignProperties[nAssign] += OUString::createFromAscii(cDataSourceName);
            pAssignProperties[nAssign + 1] = sAssignPath;
            pAssignProperties[nAssign + 1] += OUString::createFromAscii(cDataTableName);
            pAssignProperties[nAssign + 2] = sAssignPath;
            pAssignProperties[nAssign + 2] += OUString::createFromAscii(cDataCommandType);
            pAssignProperties[nAssign + 3] = sAssignPath;
            pAssignProperties[nAssign + 3] += OUString::createFromAscii(cDBColumnAssignments);
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
            aAddressDataAssignments.push_back(aAssignment);
        }
    }
    //check if the saved documents still exist
    if(aSavedDocuments.getLength())
    {
        uno::Sequence< OUString > aTempDocuments(aSavedDocuments.getLength());
        OUString* pTempDocuments = aTempDocuments.getArray();
        sal_Int32 nIndex = 0;
        for(sal_Int32 i = 0; i < aSavedDocuments.getLength(); ++i)
        {
            if(SWUnoHelper::UCB_IsFile( aSavedDocuments[i] ))
            {
                pTempDocuments[nIndex++] = aSavedDocuments[i];
            }
        }
        if(nIndex < aSavedDocuments.getLength())
        {
            aSavedDocuments = aTempDocuments;
            aSavedDocuments.realloc(nIndex);
        }
    }

}

SwMailMergeConfigItem_Impl::~SwMailMergeConfigItem_Impl()
{
}

void SwMailMergeConfigItem_Impl::SetCurrentAddressBlockIndex( sal_Int32 nSet )
{
    if(aAddressBlocks.size() >= sal::static_int_cast<sal_uInt32, sal_Int32>(nSet))
    {
        nCurrentAddressBlock = nSet;
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
        sNewName = "_";
        sNewName += OUString::number(nStart);
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

static void lcl_ConvertToNumbers(OUString& rBlock, const ResStringArray& rHeaders )
{
    //convert the strings used for UI to numbers used for the configuration
    OUString sBlock(rBlock.replaceAll("\n", "\\n"));
    for(sal_uInt32 i = 0; i < rHeaders.Count(); ++i)
    {
        OUString sHeader = "<" + rHeaders.GetString( i ) + ">";
        OUString sReplace = "<" + OUString(sal_Unicode('0' + i)) + ">";
        sBlock = sBlock.replaceAll(sHeader, sReplace);
    }
    rBlock = sBlock;
}

static void lcl_ConvertFromNumbers(OUString& rBlock, const ResStringArray& rHeaders)
{
    //convert the numbers used for the configuration to strings used for UI to numbers
    //doesn't use ReplaceAll to prevent expansion of numbers inside of the headers
    OUString sBlock(rBlock.replaceAll("\\n", "\n"));
    SwAddressIterator aGreetingIter(sBlock);
    sBlock.clear();
    while(aGreetingIter.HasMore())
    {
        SwMergeAddressItem aNext = aGreetingIter.Next();
        if(aNext.bIsColumn)
        {
            //the text should be 1 characters long
            sal_Unicode cChar = aNext.sText[0];
            if(cChar >= '0' && cChar <= 'c')
            {
                sBlock += "<";
                sal_uInt16 nHeader = cChar - '0';
                if(nHeader < rHeaders.Count())
                    sBlock += rHeaders.GetString( nHeader );
                sBlock += ">";
            }
            else
            {
                SAL_WARN("sw.ui", "parse error in address block or greeting line");
            }
        }
        else
            sBlock += aNext.sText;
    }
    rBlock = sBlock;
}

const Sequence<OUString>& SwMailMergeConfigItem_Impl::GetPropertyNames()
{
    static Sequence<OUString> aNames;
    if(!aNames.getLength())
    {
        static const char* aPropNames[] =
        {
            "OutputToLetter",           // 0
            "IncludeCountry",           // 1
            "ExcludeCountry",           // 2
            "AddressBlockSettings",     // 3
            "IsAddressBlock",          // 4
            "IsGreetingLine",           // 5
            "IsIndividualGreetingLine", // 6
            "FemaleGreetingLines",      // 7
            "MaleGreetingLines",        // 8
            "NeutralGreetingLines",     // 9
            "CurrentFemaleGreeting",    // 10
            "CurrentMaleGreeting",      // 11
            "CurrentNeutralGreeting",   // 12
            "FemaleGenderValue",        // 13
            "MailDisplayName",          // 14
            "MailAddress",              // 15
            "IsMailReplyTo",            // 16
            "MailReplyTo",              // 17
            "MailServer",               // 18
            "MailPort",                 // 19
            "IsSecureConnection",       // 20
            "IsAuthentication",         // 21
            "MailUserName",             // 22
            "MailPassword",             // 23
            "DataSource/DataSourceName",// 24
            "DataSource/DataTableName", // 25
            "DataSource/DataCommandType",// 26
            "Filter",                   // 27
            "SavedDocuments",           // 28
            "EMailSupported",            // 29
            "IsEMailGreetingLine",              //30
            "IsEMailIndividualGreetingLine",     //31
            "IsSMPTAfterPOP",                    //32
            "InServerName",                      //33
            "InServerPort",                      //34
            "InServerIsPOP",                     //35
            "InServerUserName",                  //36
            "InServerPassword",                   //37
            "IsHideEmptyParagraphs",             //38
            "CurrentAddressBlock"               //39

        };
        const int nCount = sizeof(aPropNames)/sizeof(const char*);
        aNames.realloc(nCount);
        OUString* pNames = aNames.getArray();
        for(int i = 0; i < nCount; i++)
            pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}

void SwMailMergeConfigItem_Impl::Notify( const ::com::sun::star::uno::Sequence< OUString >& ) {}

void  SwMailMergeConfigItem_Impl::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case 0: pValues[nProp] <<= bIsOutputToLetter; break;
            case 1: pValues[nProp] <<= bIncludeCountry; break;
            case 2: pValues[nProp] <<= sExcludeCountry; break;
            case 3: pValues[nProp] <<= GetAddressBlocks(true); break;
            case 4:
                {
                    if( bUserSettingWereOverwritten)
                        pValues[nProp] <<= bIsAddressBlock_LastUserSetting;
                    else
                        pValues[nProp] <<= bIsAddressBlock;
                    break;
                }
            case 5:
                {
                    if( bUserSettingWereOverwritten)
                        pValues[nProp] <<= bIsGreetingLine_LastUserSetting;
                    else
                        pValues[nProp] <<= bIsGreetingLine;
                    break;
                }
            case 6: pValues[nProp] <<= bIsIndividualGreetingLine;  break;
            case 7:
            case 8:
            case 9:
                pValues[nProp] <<= GetGreetings(
                        SwMailMergeConfigItem::Gender(
                            SwMailMergeConfigItem::FEMALE + nProp - 7), true);
            break;
            case 10: pValues[nProp] <<= nCurrentFemaleGreeting;     break;
            case 11: pValues[nProp] <<= nCurrentMaleGreeting;       break;
            case 12: pValues[nProp] <<= nCurrentNeutralGreeting;    break;
            case 13: pValues[nProp] <<= sFemaleGenderValue;     break;
            case 14: pValues[nProp] <<= sMailDisplayName;     break;
            case 15: pValues[nProp] <<= sMailAddress;         break;
            case 16: pValues[nProp] <<= bIsMailReplyTo;        break;
            case 17: pValues[nProp] <<= sMailReplyTo;         break;
            case 18: pValues[nProp] <<= sMailServer;          break;
            case 19: if(!bIsDefaultPort)
                        pValues[nProp] <<= nMailPort;
            break;
            case 20: pValues[nProp] <<= bIsSecureConnection;  break;
            case 21: pValues[nProp] <<= bIsAuthentication;    break;
            case 22: pValues[nProp] <<= sMailUserName;        break;
            case 23: pValues[nProp] <<= sMailPassword;        break;
            case 24 :pValues[nProp] <<= aDBData.sDataSource;           break;
            case 25 :pValues[nProp] <<= aDBData.sCommand;              break;
            case 26 :pValues[nProp] <<= (short)aDBData.nCommandType;   break;
            case 27 :pValues[nProp] <<= sFilter; break;
            case 28 :pValues[nProp] <<= aSavedDocuments; break;
            case 29: pValues[nProp] <<= bIsEMailSupported; break;
            case 30:
                {
                    if( bUserSettingWereOverwritten)
                        pValues[nProp] <<= bIsGreetingLineInMail_LastUserSetting;
                    else
                        pValues[nProp] <<= bIsGreetingLineInMail;
                    break;
                }
            case 31: pValues[nProp] <<= bIsIndividualGreetingLineInMail; break;
            case 32: pValues[nProp] <<= bIsSMPTAfterPOP; break;
            case 33: pValues[nProp] <<= sInServerName;    break;
            case 34: pValues[nProp] <<= nInServerPort;    break;
            case 35: pValues[nProp] <<= bInServerPOP; break;
            case 36: pValues[nProp] <<= sInServerUserName; break;
            case 37: pValues[nProp] <<= sInServerPassword; break;
            case 38: pValues[nProp] <<= bIsHideEmptyParagraphs; break;
            case 39: pValues[nProp] <<= nCurrentAddressBlock; break;
        }
    }
    PutProperties(aNames, aValues);
    //store the changed / new assignments

    //load the existing node names to find new names
    Sequence<OUString> aAssignments = GetNodeNames(OUString::createFromAscii(cAddressDataAssignments));

    ::std::vector<DBAddressDataAssignment>::iterator aAssignIter;
    for(aAssignIter = aAddressDataAssignments.begin();
                aAssignIter != aAddressDataAssignments.end(); ++aAssignIter)
    {
        if(aAssignIter->bColumnAssignmentsChanged)
        {
            //create a new node name
            OUString sNewNode = !aAssignIter->sConfigNodeName.isEmpty() ?
                        aAssignIter->sConfigNodeName :
                        lcl_CreateNodeName(aAssignments);
            OUString sSlash = "/";
            OUString sNodePath = OUString::createFromAscii(cAddressDataAssignments);
            sNodePath += sSlash;
            sNodePath += sNewNode;
            sNodePath += sSlash;
            //only one new entry is written
            Sequence< PropertyValue > aNewValues(4);
            PropertyValue* pNewValues = aNewValues.getArray();
            pNewValues[0].Name = sNodePath;
            pNewValues[0].Name += OUString::createFromAscii(cDataSourceName);
            pNewValues[0].Value <<= aAssignIter->aDBData.sDataSource;
            pNewValues[1].Name = sNodePath;
            pNewValues[1].Name += OUString::createFromAscii(cDataTableName);
            pNewValues[1].Value <<= aAssignIter->aDBData.sCommand;
            pNewValues[2].Name = sNodePath;
            pNewValues[2].Name += OUString::createFromAscii(cDataCommandType);
            pNewValues[2].Value <<= aAssignIter->aDBData.nCommandType;
            pNewValues[3].Name = sNodePath;
            pNewValues[3].Name += OUString::createFromAscii(cDBColumnAssignments);
            pNewValues[3].Value <<= aAssignIter->aDBColumnAssignments;

            SetSetProperties(OUString::createFromAscii(cAddressDataAssignments), aNewValues);
        }
    }

    bUserSettingWereOverwritten = false;
}

const Sequence< OUString> SwMailMergeConfigItem_Impl::GetAddressBlocks(
        bool bConvertToConfig) const
{
    Sequence< OUString> aRet(aAddressBlocks.size());
    OUString* pRet = aRet.getArray();
    for(size_t nBlock = 0; nBlock < aAddressBlocks.size(); nBlock++)
    {
        pRet[nBlock] = aAddressBlocks[nBlock];
        if(bConvertToConfig)
            lcl_ConvertToNumbers(pRet[nBlock], m_AddressHeaderSA);
    }
    return aRet;
}

void SwMailMergeConfigItem_Impl::SetAddressBlocks(
        const Sequence< OUString>& rBlocks,
        bool bConvertFromConfig)
{
    aAddressBlocks.clear();
    for(sal_Int32 nBlock = 0; nBlock < rBlocks.getLength(); nBlock++)
    {
        OUString sBlock = rBlocks[nBlock];
        if(bConvertFromConfig)
            lcl_ConvertFromNumbers(sBlock, m_AddressHeaderSA);
        aAddressBlocks.push_back(sBlock);
    }
    nCurrentAddressBlock = 0;
    SetModified();
}

const Sequence< OUString>   SwMailMergeConfigItem_Impl::GetGreetings(
        SwMailMergeConfigItem::Gender eType, bool bConvertToConfig) const
{
    const ::std::vector< OUString>& rGreetings =
            eType == SwMailMergeConfigItem::FEMALE ? aFemaleGreetingLines :
            eType == SwMailMergeConfigItem::MALE ? aMaleGreetingLines :
                                aNeutralGreetingLines;
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
    ::std::vector< OUString>& rGreetings =
            eType == SwMailMergeConfigItem::FEMALE ? aFemaleGreetingLines :
            eType == SwMailMergeConfigItem::MALE ? aMaleGreetingLines :
                                aNeutralGreetingLines;

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
        case SwMailMergeConfigItem::FEMALE: nRet = nCurrentFemaleGreeting ; break;
        case SwMailMergeConfigItem::MALE:   nRet = nCurrentMaleGreeting   ; break;
        default:                       nRet = nCurrentNeutralGreeting; break;
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
            bChanged = nCurrentFemaleGreeting != nIndex;
            nCurrentFemaleGreeting = nIndex;
        break;
        case SwMailMergeConfigItem::MALE:
            bChanged = nCurrentMaleGreeting != nIndex;
            nCurrentMaleGreeting = nIndex;
        break;
        default:
            bChanged = nCurrentNeutralGreeting != nIndex;
            nCurrentNeutralGreeting = nIndex;
    }
    if(bChanged)
        SetModified();
}

namespace
{
    static SwMailMergeConfigItem_Impl* pOptions = NULL;
    static sal_Int32            nRefCount = 0;

    class theMailMergeConfigMutex : public rtl::Static<osl::Mutex, theMailMergeConfigMutex> {};
}

SwMailMergeConfigItem::SwMailMergeConfigItem() :
    m_bAddressInserted(false),
    m_bMergeDone(false),
    m_bGreetingInserted(false),
    m_nGreetingMoves(0),
    m_nStartPrint(0),
    m_nEndPrint(0),
    m_pSourceView(0),
    m_pTargetView(0)
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( theMailMergeConfigMutex::get() );
    if ( !pOptions )
        pOptions = new SwMailMergeConfigItem_Impl;
    ++nRefCount;
    m_pImpl = pOptions;
}

SwMailMergeConfigItem::~SwMailMergeConfigItem()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( theMailMergeConfigMutex::get() );
    if ( !--nRefCount )
    {
        DELETEZ( pOptions );
    }
}

void  SwMailMergeConfigItem::Commit()
{
    if(m_pImpl->IsModified())
        m_pImpl->Commit();
}

const ResStringArray&   SwMailMergeConfigItem::GetDefaultAddressHeaders() const
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
    return m_pImpl->bIsAddressBlock && IsOutputToLetter();
}

void     SwMailMergeConfigItem::SetAddressBlock(bool bSet)
{
    m_pImpl->bUserSettingWereOverwritten = false;
    if(m_pImpl->bIsAddressBlock != bSet)
    {
        m_pImpl->bIsAddressBlock = bSet;
        m_pImpl->SetModified();
    }
}

bool SwMailMergeConfigItem::IsHideEmptyParagraphs() const
{
    return m_pImpl->bIsHideEmptyParagraphs;
}

void SwMailMergeConfigItem::SetHideEmptyParagraphs(bool bSet)
{
    if(m_pImpl->bIsHideEmptyParagraphs != bSet)
    {
        m_pImpl->bIsHideEmptyParagraphs = bSet;
        m_pImpl->SetModified();
    }
}

bool SwMailMergeConfigItem::IsIncludeCountry() const
{
    return m_pImpl->bIncludeCountry;
}

OUString& SwMailMergeConfigItem::GetExcludeCountry() const
{
    return m_pImpl->sExcludeCountry;
}

void SwMailMergeConfigItem::SetCountrySettings(bool bSet, const OUString& rCountry)
{
    if(m_pImpl->sExcludeCountry != rCountry ||
        m_pImpl->bIncludeCountry != bSet)
    {
        m_pImpl->bIncludeCountry = bSet;
        m_pImpl->sExcludeCountry = bSet ? rCountry : OUString();
        m_pImpl->SetModified();
    }
}

void SwMailMergeConfigItem::SetCurrentConnection(
        Reference< XDataSource>       xSource,
        const SharedConnection&       rConnection,
        Reference< XColumnsSupplier>  xColumnsSupplier,
        const SwDBData& rDBData)
{
        m_pImpl->xSource            = xSource         ;
        m_pImpl->xConnection        = rConnection     ;
        m_pImpl->xColumnsSupplier   = xColumnsSupplier;
        m_pImpl->aDBData = rDBData;
        m_pImpl->xResultSet         = 0;
        m_pImpl->nResultSetCursorPos = 0;
        m_pImpl->SetModified();
}

Reference< XDataSource>  SwMailMergeConfigItem::GetSource()
{
    return m_pImpl->xSource;
}

SharedConnection SwMailMergeConfigItem::GetConnection()
{
    return m_pImpl->xConnection;
}

Reference< XColumnsSupplier> SwMailMergeConfigItem::GetColumnsSupplier()
{
    if(!m_pImpl->xColumnsSupplier.is() && m_pImpl->xConnection.is())
    {
        m_pImpl->xColumnsSupplier = SwDBManager::GetColumnSupplier(m_pImpl->xConnection,
                                m_pImpl->aDBData.sCommand,
                                m_pImpl->aDBData.nCommandType == CommandType::TABLE ?
                                        SwDBSelect::TABLE : SwDBSelect::QUERY );
    }
    return m_pImpl->xColumnsSupplier;
}

const SwDBData&    SwMailMergeConfigItem::GetCurrentDBData() const
{
    return m_pImpl->aDBData;
}

void SwMailMergeConfigItem::SetCurrentDBData( const SwDBData& rDBData)
{
    if(m_pImpl->aDBData != rDBData)
    {
        m_pImpl->aDBData = rDBData;
        m_pImpl->xConnection.clear();
        m_pImpl->xSource = 0;
        m_pImpl->xColumnsSupplier = 0;
        m_pImpl->SetModified();
    }
}

Reference< XResultSet>   SwMailMergeConfigItem::GetResultSet() const
{
    if(!m_pImpl->xConnection.is() && !m_pImpl->aDBData.sDataSource.isEmpty())
    {
        m_pImpl->xConnection.reset(
            SwDBManager::GetConnection( m_pImpl->aDBData.sDataSource, m_pImpl->xSource ),
            SharedConnection::TakeOwnership
        );
    }
    if(!m_pImpl->xResultSet.is() && m_pImpl->xConnection.is())
    {
        try
        {
            Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );

            Reference<XRowSet> xRowSet( xMgr->createInstance("com.sun.star.sdb.RowSet"), UNO_QUERY );
            Reference<XPropertySet> xRowProperties(xRowSet, UNO_QUERY);
            xRowProperties->setPropertyValue("DataSourceName", makeAny(m_pImpl->aDBData.sDataSource));
            xRowProperties->setPropertyValue("Command", makeAny(m_pImpl->aDBData.sCommand));
            xRowProperties->setPropertyValue("CommandType", makeAny(m_pImpl->aDBData.nCommandType));
            xRowProperties->setPropertyValue("FetchSize", makeAny((sal_Int32)10));
            xRowProperties->setPropertyValue("ActiveConnection", makeAny(m_pImpl->xConnection.getTyped()));
            try
            {
                xRowProperties->setPropertyValue("ApplyFilter", makeAny(!m_pImpl->sFilter.isEmpty()));
                xRowProperties->setPropertyValue("Filter", makeAny(m_pImpl->sFilter));
            }
            catch (const Exception& e)
            {
                SAL_WARN("sw.ui", "exception caught: " << e.Message);
            }
            xRowSet->execute();
            m_pImpl->xResultSet = xRowSet.get();
            m_pImpl->xResultSet->first();
            m_pImpl->nResultSetCursorPos = 1;
        }
        catch (const Exception& e)
        {
            SAL_WARN("sw.ui", "exception caught in: SwMailMergeConfigItem::GetResultSet() " << e.Message);
        }
    }
    return m_pImpl->xResultSet;
}

void SwMailMergeConfigItem::DisposeResultSet()
{
    m_pImpl->xConnection.clear();
    if(m_pImpl->xResultSet.is())
    {
        ::comphelper::disposeComponent( m_pImpl->xResultSet );
    }
}

OUString&    SwMailMergeConfigItem::GetFilter() const
{
    return m_pImpl->sFilter;
}

void  SwMailMergeConfigItem::SetFilter(OUString& rFilter)
{
    if(m_pImpl->sFilter != rFilter)
    {
        m_pImpl->sFilter = rFilter;
        m_pImpl->SetModified();
        Reference<XPropertySet> xRowProperties(m_pImpl->xResultSet, UNO_QUERY);
        if(xRowProperties.is())
        {
            try
            {
                xRowProperties->setPropertyValue("ApplyFilter", makeAny(!m_pImpl->sFilter.isEmpty()));
                xRowProperties->setPropertyValue("Filter", makeAny(m_pImpl->sFilter));
                uno::Reference<XRowSet> xRowSet( m_pImpl->xResultSet, UNO_QUERY_THROW );
                xRowSet->execute();
            }
            catch (const Exception& e)
            {
                SAL_WARN("sw.ui", "exception caught in SwMailMergeConfigItem::SetFilter(): " << e.Message);
            }
        }
    }
}

sal_Int32 SwMailMergeConfigItem::MoveResultSet(sal_Int32 nTarget)
{
    if(!m_pImpl->xResultSet.is())
        GetResultSet();
    if(m_pImpl->xResultSet.is())
    {
        try
        {
            //no action if the resultset is already at the right position
            if(m_pImpl->xResultSet->getRow() != nTarget)
            {
                if(nTarget > 0)
                {
                    bool bMoved = m_pImpl->xResultSet->absolute(nTarget);
                    if(!bMoved)
                    {
                        if(nTarget > 1)
                            m_pImpl->xResultSet->last();
                        else if(nTarget == 1)
                            m_pImpl->xResultSet->first();
                    }
                }
                else if(nTarget == -1)
                    m_pImpl->xResultSet->last();
                m_pImpl->nResultSetCursorPos = m_pImpl->xResultSet->getRow();
            }
        }
        catch (const Exception&)
        {
        }
    }
    return m_pImpl->nResultSetCursorPos;
}

bool SwMailMergeConfigItem::IsResultSetFirstLast(bool& bIsFirst, bool& bIsLast)
{
    bool bRet = false;
    if(!m_pImpl->xResultSet.is())
        GetResultSet();
    if(m_pImpl->xResultSet.is())
    {
        try
        {
            bIsFirst = m_pImpl->xResultSet->isFirst();
            bIsLast = m_pImpl->xResultSet->isLast();
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
    return m_pImpl->nResultSetCursorPos;
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
    if(!m_pImpl->xResultSet.is())
        GetResultSet();
    if(!m_pImpl->xResultSet.is())
        return {};
    m_pImpl->xResultSet->last();
    sal_Int32 nResultSetSize = m_pImpl->xResultSet->getRow()+1;
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
    return m_pImpl->aSavedDocuments;
}

void SwMailMergeConfigItem::AddSavedDocument(const OUString& rName)
{
    const OUString* pDocs = m_pImpl->aSavedDocuments.getConstArray();
    bool bFound = false;
    for(sal_Int32 nDoc = 0; nDoc < m_pImpl->aSavedDocuments.getLength(); ++nDoc)
    {
        if(pDocs[nDoc] == rName)
        {
            bFound = true;
            break;
        }
    }
    if(!bFound)
    {
        m_pImpl->aSavedDocuments.realloc(m_pImpl->aSavedDocuments.getLength() + 1);
        m_pImpl->aSavedDocuments[m_pImpl->aSavedDocuments.getLength() - 1] = rName;
    }
}

bool SwMailMergeConfigItem::IsOutputToLetter()const
{
    return m_pImpl->bIsOutputToLetter || !IsMailAvailable();
}

void SwMailMergeConfigItem::SetOutputToLetter(bool bSet)
{
    if(m_pImpl->bIsOutputToLetter != bSet)
    {
        m_pImpl->bIsOutputToLetter = bSet;
        m_pImpl->SetModified();
    }
}

bool SwMailMergeConfigItem::IsIndividualGreeting(bool bInEMail) const
{
    return bInEMail ?
            m_pImpl->bIsIndividualGreetingLineInMail :
            m_pImpl->bIsIndividualGreetingLine;
}

void     SwMailMergeConfigItem::SetIndividualGreeting(
                                        bool bSet, bool bInEMail)
{
    if(bInEMail)
    {
        if(m_pImpl->bIsIndividualGreetingLineInMail != bSet)
        {
            m_pImpl->bIsIndividualGreetingLineInMail = bSet;
            m_pImpl->SetModified();
        }
    }
    else
    {
        if(m_pImpl->bIsIndividualGreetingLine != bSet)
        {
            m_pImpl->bIsIndividualGreetingLine = bSet;
            m_pImpl->SetModified();
        }
    }
}

bool SwMailMergeConfigItem::IsGreetingLine(bool bInEMail) const
{
    return bInEMail ? m_pImpl->bIsGreetingLineInMail : m_pImpl->bIsGreetingLine;
}

void     SwMailMergeConfigItem::SetGreetingLine(bool bSet, bool bInEMail)
{
    m_pImpl->bUserSettingWereOverwritten = false;
    if(bInEMail)
    {
        if(m_pImpl->bIsGreetingLineInMail != bSet)
        {
            m_pImpl->bIsGreetingLineInMail = bSet;
            m_pImpl->SetModified();
        }
    }
    else
    {
        if(m_pImpl->bIsGreetingLine != bSet)
        {
            m_pImpl->bIsGreetingLine = bSet;
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
    return m_pImpl->sFemaleGenderValue;
}

void SwMailMergeConfigItem::SetFemaleGenderValue(const OUString& rValue)
{
    if( m_pImpl->sFemaleGenderValue != rValue )
    {
        m_pImpl->sFemaleGenderValue = rValue;
        m_pImpl->SetModified();
    }
}

Sequence< OUString> SwMailMergeConfigItem::GetColumnAssignment(
                const SwDBData& rDBData ) const
{
    Sequence< OUString> aRet;
    ::std::vector<DBAddressDataAssignment>::iterator aAssignIter;
    for(aAssignIter = m_pImpl->aAddressDataAssignments.begin();
                aAssignIter != m_pImpl->aAddressDataAssignments.end(); ++aAssignIter)
    {
        if(aAssignIter->aDBData == rDBData)
        {
            aRet = aAssignIter->aDBColumnAssignments;
            break;
        }
    }
    return aRet;
}

// returns the name that is assigned as e-mail column of the current data base
OUString     SwMailMergeConfigItem::GetAssignedColumn(sal_uInt32 nColumn) const
{
    OUString sRet;
    Sequence< OUString> aAssignment = GetColumnAssignment( m_pImpl->aDBData );
    if(aAssignment.getLength() > sal::static_int_cast< sal_Int32, sal_uInt32>(nColumn) && !aAssignment[nColumn].isEmpty())
        sRet = aAssignment[nColumn];
    else if(nColumn < m_pImpl->m_AddressHeaderSA.Count())
        sRet = m_pImpl->m_AddressHeaderSA.GetString(nColumn);
    return sRet;
}

void SwMailMergeConfigItem::SetColumnAssignment( const SwDBData& rDBData,
                            const Sequence< OUString>& rList)
{
    ::std::vector<DBAddressDataAssignment>::iterator aAssignIter;
    bool bFound = false;
    for(aAssignIter = m_pImpl->aAddressDataAssignments.begin();
                aAssignIter != m_pImpl->aAddressDataAssignments.end(); ++aAssignIter)
    {
        if(aAssignIter->aDBData == rDBData)
        {
            if(aAssignIter->aDBColumnAssignments != rList)
            {
                aAssignIter->aDBColumnAssignments = rList;
                aAssignIter->bColumnAssignmentsChanged = true;
            }
            bFound = true;
            break;
        }
    }
    if(!bFound)
    {
        DBAddressDataAssignment aAssignment;
        aAssignment.aDBData = rDBData;
        aAssignment.aDBColumnAssignments = rList;
        aAssignment.bColumnAssignmentsChanged = true;
        m_pImpl->aAddressDataAssignments.push_back(aAssignment);
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

    const ResStringArray& rHeaders = GetDefaultAddressHeaders();
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
            for(sal_uInt16 nColumn = 0;
                    nColumn < rHeaders.Count() && nColumn < aAssignment.getLength();
                                                                                ++nColumn)
            {
                if (rHeaders.GetString(nColumn).equals(aItem.sText) &&
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
    const ResStringArray& rHeaders = GetDefaultAddressHeaders();
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
            for(sal_uInt16 nColumn = 0;
                    nColumn < rHeaders.Count() && nColumn < aAssignment.getLength();
                                                                                ++nColumn)
            {
                if (rHeaders.GetString(nColumn).equals(aItem.sText) &&
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

OUString     SwMailMergeConfigItem::GetMailDisplayName() const
{
    return m_pImpl->sMailDisplayName;
}

void SwMailMergeConfigItem::SetMailDisplayName(const OUString& rName)
{
    if(m_pImpl->sMailDisplayName != rName)
    {
        m_pImpl->sMailDisplayName = rName;
        m_pImpl->SetModified();
    }
}

OUString     SwMailMergeConfigItem::GetMailAddress() const
{
    return m_pImpl->sMailAddress;
}

void SwMailMergeConfigItem::SetMailAddress(const OUString& rAddress)
{
    if(m_pImpl->sMailAddress != rAddress )
    {
        m_pImpl->sMailAddress = rAddress;
        m_pImpl->SetModified();
    }
}

bool SwMailMergeConfigItem::IsMailReplyTo() const
{
    return m_pImpl->bIsMailReplyTo;
}

void  SwMailMergeConfigItem::SetMailReplyTo(bool bSet)
{
    if(m_pImpl->bIsMailReplyTo != bSet)
    {
        m_pImpl->bIsMailReplyTo = bSet;
        m_pImpl->SetModified();
    }
}

OUString     SwMailMergeConfigItem::GetMailReplyTo() const
{
    return m_pImpl->sMailReplyTo;
}

void SwMailMergeConfigItem::SetMailReplyTo(const OUString& rReplyTo)
{
    if(m_pImpl->sMailReplyTo != rReplyTo)
    {
        m_pImpl->sMailReplyTo = rReplyTo;
        m_pImpl->SetModified();
    }
}

OUString  SwMailMergeConfigItem::GetMailServer() const
{
    return m_pImpl->sMailServer;
}

void SwMailMergeConfigItem::SetMailServer(const OUString& rAddress)
{
    if(m_pImpl->sMailServer != rAddress)
    {
        m_pImpl->sMailServer = rAddress;
        m_pImpl->SetModified();
    }
}

sal_Int16 SwMailMergeConfigItem::GetMailPort() const
{
    return m_pImpl->bIsDefaultPort ?
             (m_pImpl->bIsSecureConnection ? SECURE_PORT : DEFAULT_PORT) :
             m_pImpl->nMailPort;
}

void     SwMailMergeConfigItem::SetMailPort(sal_Int16 nSet)
{
    if(m_pImpl->nMailPort != nSet || m_pImpl->bIsDefaultPort)
    {
        m_pImpl->nMailPort = nSet;
        m_pImpl->bIsDefaultPort = false;
        m_pImpl->SetModified();
    }
}

bool SwMailMergeConfigItem::IsSecureConnection() const
{
    return m_pImpl->bIsSecureConnection;
}

void     SwMailMergeConfigItem::SetSecureConnection(bool bSet)
{
    if(m_pImpl->bIsSecureConnection != bSet)
    {
        m_pImpl->bIsSecureConnection = bSet;
        m_pImpl->SetModified();
    }
}

bool SwMailMergeConfigItem::IsAuthentication() const
{
    return m_pImpl->bIsAuthentication;
}

void SwMailMergeConfigItem::SetAuthentication(bool bSet)
{
    if(m_pImpl->bIsAuthentication != bSet)
    {
        m_pImpl->bIsAuthentication = bSet;
        m_pImpl->SetModified();
    }
}

OUString     SwMailMergeConfigItem::GetMailUserName() const
{
    return m_pImpl->sMailUserName;
}

void SwMailMergeConfigItem::SetMailUserName(const OUString& rName)
{
    if(m_pImpl->sMailUserName != rName)
    {
        m_pImpl->sMailUserName = rName;
        m_pImpl->SetModified();
    }
}

OUString     SwMailMergeConfigItem::GetMailPassword() const
{
    return m_pImpl->sMailPassword;
}

void SwMailMergeConfigItem::SetMailPassword(const OUString& rPassword)
{
    if(m_pImpl->sMailPassword != rPassword)
    {
        m_pImpl->sMailPassword = rPassword;
        m_pImpl->SetModified();
    }
}

bool SwMailMergeConfigItem::IsSMTPAfterPOP() const
{
    return m_pImpl->bIsSMPTAfterPOP;
}

void SwMailMergeConfigItem::SetSMTPAfterPOP(bool bSet)
{
    if( m_pImpl->bIsSMPTAfterPOP != bSet)
    {
        m_pImpl->bIsSMPTAfterPOP = bSet;
        m_pImpl->SetModified();
    }
}

OUString     SwMailMergeConfigItem::GetInServerName() const
{
    return m_pImpl->sInServerName;
}

void SwMailMergeConfigItem::SetInServerName(const OUString& rServer)
{
    if(m_pImpl->sInServerName != rServer)
    {
        m_pImpl->sInServerName = rServer;
        m_pImpl->SetModified();
    }
}

sal_Int16           SwMailMergeConfigItem::GetInServerPort() const
{
    return m_pImpl->nInServerPort;
}

void SwMailMergeConfigItem::SetInServerPort(sal_Int16 nSet)
{
    if( m_pImpl->nInServerPort != nSet)
    {
        m_pImpl->nInServerPort = nSet;
        m_pImpl->SetModified();
    }
}

bool SwMailMergeConfigItem::IsInServerPOP() const
{
    return m_pImpl->bInServerPOP;
}

void SwMailMergeConfigItem::SetInServerPOP(bool bSet)
{
    if( m_pImpl->bInServerPOP != bSet)
    {
        m_pImpl->bInServerPOP = bSet;
        m_pImpl->SetModified();
    }
}

OUString     SwMailMergeConfigItem::GetInServerUserName() const
{
    return m_pImpl->sInServerUserName;
}

void SwMailMergeConfigItem::SetInServerUserName(const OUString& rName)
{
    if( m_pImpl->sInServerUserName != rName)
    {
        m_pImpl->sInServerUserName = rName;
        m_pImpl->SetModified();
    }
}

OUString     SwMailMergeConfigItem::GetInServerPassword() const
{
    return m_pImpl->sInServerPassword;
}

void SwMailMergeConfigItem::SetInServerPassword(const OUString& rPassword)
{
    if(m_pImpl->sInServerPassword != rPassword)
    {
        m_pImpl->sInServerPassword = rPassword;
        m_pImpl->SetModified();
    }
}

void SwMailMergeConfigItem::DocumentReloaded()
{
    m_bMergeDone = false;
    m_bGreetingInserted = false;
    m_bAddressInserted = false;
    m_rAddressBlockFrame.clear();
}

bool SwMailMergeConfigItem::IsMailAvailable() const
{
    return m_pImpl->bIsEMailSupported;
}

void SwMailMergeConfigItem::AddMergedDocument(SwDocMergeInfo& rInfo)
{
    m_pImpl->aMergeInfos.push_back(rInfo);
}

SwDocMergeInfo& SwMailMergeConfigItem::GetDocumentMergeInfo(sal_uInt32 nDocument)
{
    assert(nDocument < m_pImpl->aMergeInfos.size());
    return m_pImpl->aMergeInfos[nDocument];
}

sal_uInt32 SwMailMergeConfigItem::GetMergedDocumentCount() const
{
    return m_pImpl->aMergeInfos.size();
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
    return 0;
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
        m_pImpl->aMergeInfos.clear();
    }
}

SwView* SwMailMergeConfigItem::GetSourceView()
{
    m_pSourceView = lcl_ExistsView(m_pSourceView);
    return m_pSourceView;
}

void SwMailMergeConfigItem::SetSourceView(SwView* pView)
{
    m_pSourceView = pView;

    if(pView)
    {
        std::vector<OUString> aDBNameList;
        std::vector<OUString> aAllDBNames;
        pView->GetWrtShell().GetAllUsedDB( aDBNameList, &aAllDBNames );
        if(!aDBNameList.empty())
        {
            // if fields are available there is usually no need of an addressblock and greeting
            if(!m_pImpl->bUserSettingWereOverwritten)
            {
                if( m_pImpl->bIsAddressBlock
                    || m_pImpl->bIsGreetingLineInMail
                    || m_pImpl->bIsGreetingLine )
                {
                    //store user settings
                    m_pImpl->bUserSettingWereOverwritten = true;
                    m_pImpl->bIsAddressBlock_LastUserSetting = m_pImpl->bIsAddressBlock;
                    m_pImpl->bIsGreetingLineInMail_LastUserSetting = m_pImpl->bIsGreetingLineInMail;
                    m_pImpl->bIsGreetingLine_LastUserSetting = m_pImpl->bIsGreetingLine;

                    //set all to false
                    m_pImpl->bIsAddressBlock = false;
                    m_pImpl->bIsGreetingLineInMail = false;
                    m_pImpl->bIsGreetingLine = false;

                    m_pImpl->SetModified();
                }
            }
        }
        else if( m_pImpl->bUserSettingWereOverwritten )
        {
            //restore last user settings:
            m_pImpl->bIsAddressBlock = m_pImpl->bIsAddressBlock_LastUserSetting;
            m_pImpl->bIsGreetingLineInMail = m_pImpl->bIsGreetingLineInMail_LastUserSetting;
            m_pImpl->bIsGreetingLine = m_pImpl->bIsGreetingLine_LastUserSetting;

            m_pImpl->bUserSettingWereOverwritten = false;
        }
    }
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
