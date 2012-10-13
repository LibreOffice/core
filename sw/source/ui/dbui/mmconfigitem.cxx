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

#include <mmconfigitem.hxx>
#include <swtypes.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include "com/sun/star/mail/MailServiceType.hpp"
#include "com/sun/star/mail/XMailService.hpp"
#include "com/sun/star/mail/MailServiceProvider.hpp"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/sdb/CommandType.hpp>
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
using ::rtl::OUString;
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
    Sequence< ::rtl::OUString>           aDBColumnAssignments;
    //if loaded the name of the node has to be saved
    ::rtl::OUString                     sConfigNodeName;
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
    ::rtl::OUString                         sFilter;
    sal_Int32                               nResultSetCursorPos;

    ::std::vector<DBAddressDataAssignment>  aAddressDataAssignments;
    ::std::vector< ::rtl::OUString>         aAddressBlocks;
    sal_Int32                               nCurrentAddressBlock;
    sal_Bool                                bIsAddressBlock;
    sal_Bool                                bIsHideEmptyParagraphs;

    sal_Bool                                bIsOutputToLetter;
    sal_Bool                                bIncludeCountry;
    ::rtl::OUString                         sExcludeCountry;

    sal_Bool                                bIsGreetingLine;
    sal_Bool                                bIsIndividualGreetingLine;
    ::std::vector< ::rtl::OUString>         aFemaleGreetingLines;
    sal_Int32                               nCurrentFemaleGreeting;
    ::std::vector< ::rtl::OUString>         aMaleGreetingLines;
    sal_Int32                               nCurrentMaleGreeting;
    ::std::vector< ::rtl::OUString>         aNeutralGreetingLines;
    sal_Int32                               nCurrentNeutralGreeting;
    ::rtl::OUString                         sFemaleGenderValue;
    uno::Sequence< ::rtl::OUString>         aSavedDocuments;

    sal_Bool                                bIsGreetingLineInMail;
    sal_Bool                                bIsIndividualGreetingLineInMail;

    //mail settings
    ::rtl::OUString                         sMailDisplayName;
    ::rtl::OUString                         sMailAddress;
    ::rtl::OUString                         sMailReplyTo;
    ::rtl::OUString                         sMailServer;
    ::rtl::OUString                         sMailUserName;
    ::rtl::OUString                         sMailPassword;

    sal_Bool                                bIsSMPTAfterPOP;
    ::rtl::OUString                         sInServerName;
    sal_Int16                               nInServerPort;
    sal_Bool                                bInServerPOP;
    ::rtl::OUString                         sInServerUserName;
    ::rtl::OUString                         sInServerPassword;

    sal_Int16                               nMailPort;
    sal_Bool                                bIsMailReplyTo;
    sal_Bool                                bIsDefaultPort;
    sal_Bool                                bIsSecureConnection;
    sal_Bool                                bIsAuthentication;

    sal_Bool                                bIsEMailSupported;

    ResStringArray                          m_AddressHeaderSA;

    //these addresses are not stored in the configuration
    ::std::vector< SwDocMergeInfo >         aMergeInfos;

    //we do overwrite the usersettings in a special case
    //than we do remind the usersettings here
    sal_Bool                                bUserSettingWereOverwritten;
    sal_Bool                                bIsAddressBlock_LastUserSetting;
    sal_Bool                                bIsGreetingLineInMail_LastUserSetting;
    sal_Bool                                bIsGreetingLine_LastUserSetting;


    const Sequence< ::rtl::OUString>&       GetPropertyNames();

public:
    SwMailMergeConfigItem_Impl();
    ~SwMailMergeConfigItem_Impl();

    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    const           Sequence< ::rtl::OUString>
                        GetAddressBlocks(sal_Bool bConvertToConfig = sal_False) const;
    void                SetAddressBlocks(
                                const Sequence< ::rtl::OUString>& rBlocks,
                                sal_Bool bConvertFromConfig = sal_False);
    const uno::Sequence< ::rtl::OUString>
                        GetGreetings(SwMailMergeConfigItem::Gender eType,
                                        sal_Bool bConvertToConfig = sal_False) const;
    void                SetGreetings(SwMailMergeConfigItem::Gender eType,
                                    const uno::Sequence< ::rtl::OUString>& rBlocks,
                                    sal_Bool bConvertFromConfig = sal_False);

    void                SetCurrentAddressBlockIndex( sal_Int32 nSet );
    sal_Int32           GetCurrentAddressBlockIndex() const
                        {   return nCurrentAddressBlock; }
    sal_Int32           GetCurrentGreeting(SwMailMergeConfigItem::Gender eType) const;
    void                SetCurrentGreeting(SwMailMergeConfigItem::Gender eType, sal_Int32 nIndex);

};

SwMailMergeConfigItem_Impl::SwMailMergeConfigItem_Impl() :
    ConfigItem(C2U("Office.Writer/MailMergeWizard"), 0),
        nResultSetCursorPos(-1),
        nCurrentAddressBlock(0),
        bIsAddressBlock(sal_True),
        bIsHideEmptyParagraphs(sal_False),
        bIsOutputToLetter(sal_True),
        bIncludeCountry(sal_False),
        bIsGreetingLine(sal_True),
        nCurrentFemaleGreeting(0),
        nCurrentMaleGreeting(0),
        nCurrentNeutralGreeting(0),

        bIsSMPTAfterPOP(sal_False),
        nInServerPort( POP_PORT ),
        bInServerPOP( sal_True ),
        nMailPort(0),
        bIsMailReplyTo(sal_False),
        bIsDefaultPort(sal_False),
        bIsSecureConnection(sal_False),
        bIsAuthentication(sal_False),

        bIsEMailSupported(sal_False),
        m_AddressHeaderSA( SW_RES( SA_ADDRESS_HEADER )),
        bUserSettingWereOverwritten(sal_False),
        bIsAddressBlock_LastUserSetting(sal_False),
        bIsGreetingLineInMail_LastUserSetting(sal_False),
        bIsGreetingLine_LastUserSetting(sal_False)
{
    const Sequence<OUString>& rNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(rNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == rNames.getLength(), "GetProperties failed");
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
                    Sequence< ::rtl::OUString> aBlocks;
                    pValues[nProp] >>= aBlocks;
                    SetAddressBlocks(aBlocks, sal_True);
                }
                break;
                case 4: pValues[nProp] >>= bIsAddressBlock; break;
                case 5: pValues[nProp] >>= bIsGreetingLine;   break;
                case 6: pValues[nProp] >>= bIsIndividualGreetingLine; break;
                case 7 :
                case 8 :
                case 9 :
                {
                    Sequence< ::rtl::OUString> aGreetings;
                    pValues[nProp] >>= aGreetings;
                    SetGreetings(SwMailMergeConfigItem::Gender(
                            SwMailMergeConfigItem::FEMALE + nProp - 7), aGreetings, sal_True);
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
                case 19:
                    bIsDefaultPort =
                            (pValues[nProp] >>= nMailPort) ?
                                                        sal_False : sal_True;
                break;
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
    Sequence<OUString> aAssignments = GetNodeNames(rtl::OUString::createFromAscii(cAddressDataAssignments));
    if(aAssignments.getLength())
    {
        //create a list of property names to load the URLs of all data bases
        const OUString* pAssignments = aAssignments.getConstArray();
        Sequence< ::rtl::OUString > aAssignProperties(4 * aAssignments.getLength());
        ::rtl::OUString* pAssignProperties = aAssignProperties.getArray();
        sal_Int32 nAssign;
        OUString sSlash = C2U("/");
        for(nAssign = 0; nAssign < aAssignProperties.getLength(); nAssign += 4)
        {
            String sAssignPath = rtl::OUString::createFromAscii(cAddressDataAssignments);
            sAssignPath += '/';
            sAssignPath += String(pAssignments[nAssign / 4]);
            sAssignPath += '/';
            pAssignProperties[nAssign] = sAssignPath;
            pAssignProperties[nAssign] += rtl::OUString::createFromAscii(cDataSourceName);
            pAssignProperties[nAssign + 1] = sAssignPath;
            pAssignProperties[nAssign + 1] += rtl::OUString::createFromAscii(cDataTableName);
            pAssignProperties[nAssign + 2] = sAssignPath;
            pAssignProperties[nAssign + 2] += rtl::OUString::createFromAscii(cDataCommandType);
            pAssignProperties[nAssign + 3] = sAssignPath;
            pAssignProperties[nAssign + 3] += rtl::OUString::createFromAscii(cDBColumnAssignments);
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
        uno::Sequence< ::rtl::OUString > aTempDocuments(aSavedDocuments.getLength());
        ::rtl::OUString* pTempDocuments = aTempDocuments.getArray();
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
        sNewName = C2U("_");
        sNewName += OUString::valueOf(nStart);
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
    String sBlock(rBlock);
    sBlock.SearchAndReplaceAllAscii("\n", rtl::OUString("\\n"));
    for(sal_uInt16 i = 0; i < rHeaders.Count(); ++i)
    {
        String sHeader = rHeaders.GetString( i );
        sHeader.Insert('<', 0);
        sHeader += '>';
        String sReplace(C2U("<>"));
        sReplace.Insert('0' + i, 1);
        sBlock.SearchAndReplaceAll(sHeader, sReplace);
    }
    rBlock = sBlock;
}

static void lcl_ConvertFromNumbers(OUString& rBlock, const ResStringArray& rHeaders)
{
    //convert the numbers used for the configuration to strings used for UI to numbers
    //doesn't use ReplaceAll to prevent expansion of numbers inside of the headers
    String sBlock(rBlock);
    sBlock.SearchAndReplaceAllAscii("\\n", rtl::OUString('\n'));
    SwAddressIterator aGreetingIter(sBlock);
    sBlock.Erase();
    while(aGreetingIter.HasMore())
    {
        SwMergeAddressItem aNext = aGreetingIter.Next();
        if(aNext.bIsColumn)
        {
            //the text should be 1 characters long
            sal_Unicode cChar = aNext.sText.GetChar(0);
            if(cChar >= '0' && cChar <= 'c')
            {
                sBlock += '<';
                sal_uInt16 nHeader = cChar - '0';
                if(nHeader < rHeaders.Count())
                    sBlock += rHeaders.GetString( nHeader );
                sBlock += '>';
            }
            else
            {
                OSL_FAIL("parse error in address block or greeting line");
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

void SwMailMergeConfigItem_Impl::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

void  SwMailMergeConfigItem_Impl::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case 0: pValues[nProp] <<= bIsOutputToLetter; break;//
            case 1: pValues[nProp] <<= bIncludeCountry; break;
            case 2: pValues[nProp] <<= sExcludeCountry; break;
            case 3: pValues[nProp] <<= GetAddressBlocks(sal_True); break;
            case 4:
                {
                    if( bUserSettingWereOverwritten == sal_True )
                        pValues[nProp] <<= bIsAddressBlock_LastUserSetting;
                    else
                        pValues[nProp] <<= bIsAddressBlock;
                    break;
                }
            case 5:
                {
                    if( bUserSettingWereOverwritten == sal_True )
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
                            SwMailMergeConfigItem::FEMALE + nProp - 7), sal_True);
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
                    if( bUserSettingWereOverwritten == sal_True )
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
    Sequence<OUString> aAssignments = GetNodeNames(rtl::OUString::createFromAscii(cAddressDataAssignments));

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
            OUString sSlash = C2U("/");
            OUString sNodePath = rtl::OUString::createFromAscii(cAddressDataAssignments);
            sNodePath += sSlash;
            sNodePath += sNewNode;
            sNodePath += sSlash;
            //only one new entry is written
            Sequence< PropertyValue > aNewValues(4);
            PropertyValue* pNewValues = aNewValues.getArray();
            pNewValues[0].Name = sNodePath;
            pNewValues[0].Name += rtl::OUString::createFromAscii(cDataSourceName);
            pNewValues[0].Value <<= aAssignIter->aDBData.sDataSource;
            pNewValues[1].Name = sNodePath;
            pNewValues[1].Name += rtl::OUString::createFromAscii(cDataTableName);
            pNewValues[1].Value <<= aAssignIter->aDBData.sCommand;
            pNewValues[2].Name = sNodePath;
            pNewValues[2].Name += rtl::OUString::createFromAscii(cDataCommandType);
            pNewValues[2].Value <<= aAssignIter->aDBData.nCommandType;
            pNewValues[3].Name = sNodePath;
            pNewValues[3].Name += rtl::OUString::createFromAscii(cDBColumnAssignments);
            pNewValues[3].Value <<= aAssignIter->aDBColumnAssignments;

            SetSetProperties(rtl::OUString::createFromAscii(cAddressDataAssignments), aNewValues);
        }
    }

    bUserSettingWereOverwritten = sal_False;
}

const Sequence< ::rtl::OUString> SwMailMergeConfigItem_Impl::GetAddressBlocks(
        sal_Bool bConvertToConfig) const
{
    Sequence< ::rtl::OUString> aRet(aAddressBlocks.size());
    ::rtl::OUString* pRet = aRet.getArray();
    for(sal_uInt32 nBlock = 0; nBlock < aAddressBlocks.size(); nBlock++)
    {
        pRet[nBlock] = aAddressBlocks[nBlock];
        if(bConvertToConfig)
            lcl_ConvertToNumbers(pRet[nBlock], m_AddressHeaderSA);
    }
    return aRet;
}

void SwMailMergeConfigItem_Impl::SetAddressBlocks(
        const Sequence< ::rtl::OUString>& rBlocks,
        sal_Bool bConvertFromConfig)
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

const Sequence< ::rtl::OUString>   SwMailMergeConfigItem_Impl::GetGreetings(
        SwMailMergeConfigItem::Gender eType, sal_Bool bConvertToConfig) const
{
    const ::std::vector< ::rtl::OUString>& rGreetings =
            eType == SwMailMergeConfigItem::FEMALE ? aFemaleGreetingLines :
            eType == SwMailMergeConfigItem::MALE ? aMaleGreetingLines :
                                aNeutralGreetingLines;
    Sequence< ::rtl::OUString> aRet(rGreetings.size());
    ::rtl::OUString* pRet = aRet.getArray();
    for(sal_uInt32 nGreeting = 0; nGreeting < rGreetings.size(); nGreeting++)
    {
        pRet[nGreeting] = rGreetings[nGreeting];
        if(bConvertToConfig)
            lcl_ConvertToNumbers(pRet[nGreeting], m_AddressHeaderSA);
    }
    return aRet;
}

void  SwMailMergeConfigItem_Impl::SetGreetings(
        SwMailMergeConfigItem::Gender eType,
        const Sequence< ::rtl::OUString>& rSetGreetings,
        sal_Bool bConvertFromConfig)
{
    ::std::vector< ::rtl::OUString>& rGreetings =
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
        const Sequence< ::rtl::OUString>& rBlocks)
{
    m_pImpl->SetAddressBlocks(rBlocks);
}

const Sequence< ::rtl::OUString> SwMailMergeConfigItem::GetAddressBlocks() const
{
    return m_pImpl->GetAddressBlocks();
}

sal_Bool SwMailMergeConfigItem::IsAddressBlock()const
{
    return m_pImpl->bIsAddressBlock && IsOutputToLetter();
}

void     SwMailMergeConfigItem::SetAddressBlock(sal_Bool bSet)
{
    m_pImpl->bUserSettingWereOverwritten = sal_False;
    if(m_pImpl->bIsAddressBlock != bSet)
    {
        m_pImpl->bIsAddressBlock = bSet;
        m_pImpl->SetModified();
    }
}

sal_Bool SwMailMergeConfigItem::IsHideEmptyParagraphs() const
{
    return m_pImpl->bIsHideEmptyParagraphs;
}

void SwMailMergeConfigItem::SetHideEmptyParagraphs(sal_Bool bSet)
{
    if(m_pImpl->bIsHideEmptyParagraphs != bSet)
    {
        m_pImpl->bIsHideEmptyParagraphs = bSet;
        m_pImpl->SetModified();
    }
}

sal_Bool SwMailMergeConfigItem::IsIncludeCountry() const
{
    return m_pImpl->bIncludeCountry;
}

rtl::OUString& SwMailMergeConfigItem::GetExcludeCountry() const
{
    return m_pImpl->sExcludeCountry;
}

void SwMailMergeConfigItem::SetCountrySettings(sal_Bool bSet, const rtl::OUString& rCountry)
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
        SharedConnection              xConnection,
        Reference< XColumnsSupplier>  xColumnsSupplier,
        const SwDBData& rDBData)
{
        m_pImpl->xSource            = xSource         ;
        m_pImpl->xConnection        = xConnection     ;
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
        m_pImpl->xColumnsSupplier = SwNewDBMgr::GetColumnSupplier(m_pImpl->xConnection,
                                m_pImpl->aDBData.sCommand,
                                m_pImpl->aDBData.nCommandType == CommandType::TABLE ?
                                        SW_DB_SELECT_TABLE : SW_DB_SELECT_QUERY );
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
            SwNewDBMgr::GetConnection( m_pImpl->aDBData.sDataSource, m_pImpl->xSource ),
            SharedConnection::TakeOwnership
        );
    }
    if(!m_pImpl->xResultSet.is() && m_pImpl->xConnection.is())
    {
        try
        {
            Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
            if( xMgr.is() )
            {
                Reference<XRowSet> xRowSet(
                        xMgr->createInstance(C2U("com.sun.star.sdb.RowSet")), UNO_QUERY);
                Reference<XPropertySet> xRowProperties(xRowSet, UNO_QUERY);
                xRowProperties->setPropertyValue(C2U("DataSourceName"), makeAny(m_pImpl->aDBData.sDataSource));
                xRowProperties->setPropertyValue(C2U("Command"), makeAny(m_pImpl->aDBData.sCommand));
                xRowProperties->setPropertyValue(C2U("CommandType"), makeAny(m_pImpl->aDBData.nCommandType));
                xRowProperties->setPropertyValue(C2U("FetchSize"), makeAny((sal_Int32)10));
                xRowProperties->setPropertyValue(C2U("ActiveConnection"), makeAny(m_pImpl->xConnection.getTyped()));
                try
                {
                    xRowProperties->setPropertyValue(C2U("ApplyFilter"), makeAny(!m_pImpl->sFilter.isEmpty()));
                    xRowProperties->setPropertyValue(C2U("Filter"), makeAny(m_pImpl->sFilter));
                }
                catch (const Exception&)
                {
                    OSL_FAIL("exception caught in xResultSet->SetFilter()");
                }
                xRowSet->execute();
                m_pImpl->xResultSet = xRowSet.get();
                m_pImpl->xResultSet->first();
                m_pImpl->nResultSetCursorPos = 1;
            }
        }
        catch (const Exception&)
        {
            OSL_FAIL("exception caught in: SwMailMergeConfigItem::GetResultSet() ");
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

::rtl::OUString&    SwMailMergeConfigItem::GetFilter() const
{
    return m_pImpl->sFilter;
}

void  SwMailMergeConfigItem::SetFilter(::rtl::OUString& rFilter)
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
                xRowProperties->setPropertyValue(C2U("ApplyFilter"), makeAny(!m_pImpl->sFilter.isEmpty()));
                xRowProperties->setPropertyValue(C2U("Filter"), makeAny(m_pImpl->sFilter));
                uno::Reference<XRowSet> xRowSet( m_pImpl->xResultSet, UNO_QUERY_THROW );
                xRowSet->execute();
            }
            catch (const Exception&)
            {
                OSL_FAIL("exception caught in SwMailMergeConfigItem::SetFilter()");
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
                    sal_Bool bMoved = m_pImpl->xResultSet->absolute(nTarget);
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

bool SwMailMergeConfigItem::IsRecordExcluded(sal_Int32 nRecord)
{
    bool bRet = false;
    if(nRecord > 0 && nRecord < m_aSelection.getLength())
    {
        sal_Int32 nTemp = 0;
        m_aSelection[nRecord - 1] >>= nTemp;
        bRet = nTemp < 1;
    }
    return bRet;
}

void SwMailMergeConfigItem::ExcludeRecord(sal_Int32 nRecord, bool bExclude)
{
    //nRecord is based on 1
    //the selection array contains Anys for all records
    //excluded records  contain a '-1'
    if(!m_aSelection.getLength() || nRecord > m_aSelection.getLength())
    {
        if(bExclude)
        {
            //if no selection array is available we need to create one containing the
            //entries for all available records
            if(!m_pImpl->xResultSet.is())
                GetResultSet();
            if(m_pImpl->xResultSet.is())
            {
                m_pImpl->xResultSet->last();
                sal_Int32 nEnd = m_pImpl->xResultSet->getRow();
                sal_Int32 nStart = m_aSelection.getLength();
                m_aSelection.realloc(nEnd);
                Any* pSelection = m_aSelection.getArray();
                for(sal_Int32 nIndex = nStart; nIndex < nEnd; ++nIndex)
                {
                    if((nRecord - 1) != nIndex)
                        pSelection[nIndex] <<= nIndex + 1;
                    else
                        pSelection[nIndex] <<= (sal_Int32) -1;
                }
            }
        }
    }
    else
    {
        if(nRecord > 0 && m_aSelection.getLength() > nRecord)
        {
            m_aSelection[nRecord - 1] <<= bExclude ? -1 : nRecord;
        }
    }
}

Sequence< Any > SwMailMergeConfigItem::GetSelection() const
{
    Sequence< Any > aRet(m_aSelection.getLength());
    sal_Int32 nRetIndex = 0;
    sal_Int32 nRet;
    for(sal_Int32 nIndex = 0; nIndex < m_aSelection.getLength(); ++nIndex)
    {
        m_aSelection[nIndex] >>= nRet;
        if(nRet > 0)
        {
            aRet[nRetIndex] <<= nRet;
            ++nRetIndex;
        }
    }
    aRet.realloc(nRetIndex);
    return aRet;
}

const uno::Sequence< ::rtl::OUString>&
                    SwMailMergeConfigItem::GetSavedDocuments() const
{
    return m_pImpl->aSavedDocuments;
}

void SwMailMergeConfigItem::AddSavedDocument(::rtl::OUString rName)
{
    const ::rtl::OUString* pDocs = m_pImpl->aSavedDocuments.getConstArray();
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

sal_Bool SwMailMergeConfigItem::IsOutputToLetter()const
{
    return m_pImpl->bIsOutputToLetter || !IsMailAvailable();
}

void SwMailMergeConfigItem::SetOutputToLetter(sal_Bool bSet)
{
    if(m_pImpl->bIsOutputToLetter != bSet)
    {
        m_pImpl->bIsOutputToLetter = bSet;
        m_pImpl->SetModified();
    }
}

sal_Bool SwMailMergeConfigItem::IsIndividualGreeting(sal_Bool bInEMail) const
{
    return bInEMail ?
            m_pImpl->bIsIndividualGreetingLineInMail :
            m_pImpl->bIsIndividualGreetingLine;
}

void     SwMailMergeConfigItem::SetIndividualGreeting(
                                        sal_Bool bSet, sal_Bool bInEMail)
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

sal_Bool SwMailMergeConfigItem::IsGreetingLine(sal_Bool bInEMail) const
{
    return bInEMail ? m_pImpl->bIsGreetingLineInMail : m_pImpl->bIsGreetingLine;
}

void     SwMailMergeConfigItem::SetGreetingLine(sal_Bool bSet, sal_Bool bInEMail)
{
    m_pImpl->bUserSettingWereOverwritten = sal_False;
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

const Sequence< ::rtl::OUString>   SwMailMergeConfigItem::GetGreetings(
        Gender eType ) const
{
    return m_pImpl->GetGreetings(eType);
}

void  SwMailMergeConfigItem::SetGreetings(
        Gender eType, const Sequence< ::rtl::OUString>& rSetGreetings)
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

const ::rtl::OUString& SwMailMergeConfigItem::GetFemaleGenderValue() const
{
    return m_pImpl->sFemaleGenderValue;
}

void SwMailMergeConfigItem::SetFemaleGenderValue(const ::rtl::OUString rValue)
{
    if( m_pImpl->sFemaleGenderValue != rValue )
    {
        m_pImpl->sFemaleGenderValue = rValue;
        m_pImpl->SetModified();
    }
}

Sequence< ::rtl::OUString> SwMailMergeConfigItem::GetColumnAssignment(
                const SwDBData& rDBData ) const
{
    Sequence< ::rtl::OUString> aRet;
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
::rtl::OUString     SwMailMergeConfigItem::GetAssignedColumn(sal_uInt32 nColumn) const
{
    ::rtl::OUString sRet;
    Sequence< ::rtl::OUString> aAssignment = GetColumnAssignment( m_pImpl->aDBData );
    if(aAssignment.getLength() > sal::static_int_cast< sal_Int32, sal_uInt32>(nColumn) && !aAssignment[nColumn].isEmpty())
        sRet = aAssignment[nColumn];
    else if(nColumn < m_pImpl->m_AddressHeaderSA.Count())
        sRet = m_pImpl->m_AddressHeaderSA.GetString(nColumn);
    return sRet;
}

void SwMailMergeConfigItem::SetColumnAssignment( const SwDBData& rDBData,
                            const Sequence< ::rtl::OUString>& rList)
{
    ::std::vector<DBAddressDataAssignment>::iterator aAssignIter;
    sal_Bool bFound = sal_False;
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
            bFound = sal_True;
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
    Sequence< ::rtl::OUString> aAssignment =
                        GetColumnAssignment( GetCurrentDBData() );
    const ::rtl::OUString* pAssignment = aAssignment.getConstArray();
    const Sequence< ::rtl::OUString> aBlocks = GetAddressBlocks();

    if(aBlocks.getLength() <= m_pImpl->GetCurrentAddressBlockIndex())
        return false;
    SwAddressIterator aIter(aBlocks[m_pImpl->GetCurrentAddressBlockIndex()]);
    while(aIter.HasMore())
    {
        SwMergeAddressItem aItem = aIter.Next();
        if(aItem.bIsColumn)
        {
            String sConvertedColumn = aItem.sText;
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

    if(!IsIndividualGreeting(sal_False))
        return true;

    Reference< XResultSet> xResultSet = GetResultSet();
    uno::Reference< XColumnsSupplier > xColsSupp( xResultSet, UNO_QUERY );
    if(!xColsSupp.is())
        return false;
    const ResStringArray& rHeaders = GetDefaultAddressHeaders();
    uno::Reference<container::XNameAccess> xCols = xColsSupp->getColumns();

    Sequence< ::rtl::OUString> aAssignment =
                        GetColumnAssignment( GetCurrentDBData() );
    const ::rtl::OUString* pAssignment = aAssignment.getConstArray();

    const Sequence< ::rtl::OUString> rFemaleEntries = GetGreetings(SwMailMergeConfigItem::FEMALE);
    sal_Int32 nCurrentFemale = GetCurrentGreeting(SwMailMergeConfigItem::FEMALE);
    const Sequence< ::rtl::OUString> rMaleEntries = GetGreetings(SwMailMergeConfigItem::MALE);
    sal_Int32 nCurrentMale = GetCurrentGreeting(SwMailMergeConfigItem::MALE);
    ::rtl::OUString sMale, sFemale;
    if(rFemaleEntries.getLength() > nCurrentFemale)
        sFemale = rFemaleEntries[nCurrentFemale];
    if(rMaleEntries.getLength() > nCurrentMale)
        sMale = rMaleEntries[nCurrentMale];

    ::rtl::OUString sAddress( sFemale );
    sAddress += sMale;
    SwAddressIterator aIter(sAddress);
    while(aIter.HasMore())
    {
        SwMergeAddressItem aItem = aIter.Next();
        if(aItem.bIsColumn)
        {
            String sConvertedColumn = aItem.sText;
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

::rtl::OUString     SwMailMergeConfigItem::GetMailDisplayName() const
{
    return m_pImpl->sMailDisplayName;
}

void SwMailMergeConfigItem::SetMailDisplayName(const ::rtl::OUString& rName)
{
    if(m_pImpl->sMailDisplayName != rName)
    {
        m_pImpl->sMailDisplayName = rName;
        m_pImpl->SetModified();
    }
}

::rtl::OUString     SwMailMergeConfigItem::GetMailAddress() const
{
    return m_pImpl->sMailAddress;
}

void SwMailMergeConfigItem::SetMailAddress(const ::rtl::OUString& rAddress)
{
    if(m_pImpl->sMailAddress != rAddress )
    {
        m_pImpl->sMailAddress = rAddress;
        m_pImpl->SetModified();
    }
}

sal_Bool SwMailMergeConfigItem::IsMailReplyTo() const
{
    return m_pImpl->bIsMailReplyTo;
}

void  SwMailMergeConfigItem::SetMailReplyTo(sal_Bool bSet)
{
    if(m_pImpl->bIsMailReplyTo != bSet)
    {
        m_pImpl->bIsMailReplyTo = bSet;
        m_pImpl->SetModified();
    }
}

::rtl::OUString     SwMailMergeConfigItem::GetMailReplyTo() const
{
    return m_pImpl->sMailReplyTo;
}

void SwMailMergeConfigItem::SetMailReplyTo(const ::rtl::OUString& rReplyTo)
{
    if(m_pImpl->sMailReplyTo != rReplyTo)
    {
        m_pImpl->sMailReplyTo = rReplyTo;
        m_pImpl->SetModified();
    }
}

::rtl::OUString  SwMailMergeConfigItem::GetMailServer() const
{
    return m_pImpl->sMailServer;
}

void SwMailMergeConfigItem::SetMailServer(const ::rtl::OUString& rAddress)
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
        m_pImpl->bIsDefaultPort = sal_False;
        m_pImpl->SetModified();
    }
}

sal_Bool SwMailMergeConfigItem::IsSecureConnection() const
{
    return m_pImpl->bIsSecureConnection;
}

void     SwMailMergeConfigItem::SetSecureConnection(sal_Bool bSet)
{
    if(m_pImpl->bIsSecureConnection != bSet)
    {
        m_pImpl->bIsSecureConnection = bSet;
        m_pImpl->SetModified();
    }
}

sal_Bool SwMailMergeConfigItem::IsAuthentication() const
{
    return m_pImpl->bIsAuthentication;
}

void SwMailMergeConfigItem::SetAuthentication(sal_Bool bSet)
{
    if(m_pImpl->bIsAuthentication != bSet)
    {
        m_pImpl->bIsAuthentication = bSet;
        m_pImpl->SetModified();
    }
}

::rtl::OUString     SwMailMergeConfigItem::GetMailUserName() const
{
    return m_pImpl->sMailUserName;
}

void SwMailMergeConfigItem::SetMailUserName(const ::rtl::OUString& rName)
{
    if(m_pImpl->sMailUserName != rName)
    {
        m_pImpl->sMailUserName = rName;
        m_pImpl->SetModified();
    }
}

::rtl::OUString     SwMailMergeConfigItem::GetMailPassword() const
{
    return m_pImpl->sMailPassword;
}

void SwMailMergeConfigItem::SetMailPassword(const ::rtl::OUString& rPassword)
{
    if(m_pImpl->sMailPassword != rPassword)
    {
        m_pImpl->sMailPassword = rPassword;
        m_pImpl->SetModified();
    }
}

sal_Bool SwMailMergeConfigItem::IsSMTPAfterPOP() const
{
    return m_pImpl->bIsSMPTAfterPOP;
}

void SwMailMergeConfigItem::SetSMTPAfterPOP(sal_Bool bSet)
{
    if( m_pImpl->bIsSMPTAfterPOP != bSet)
    {
        m_pImpl->bIsSMPTAfterPOP = bSet;
        m_pImpl->SetModified();
    }
}

::rtl::OUString     SwMailMergeConfigItem::GetInServerName() const
{
    return m_pImpl->sInServerName;
}

void SwMailMergeConfigItem::SetInServerName(const ::rtl::OUString& rServer)
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

sal_Bool SwMailMergeConfigItem::IsInServerPOP() const
{
    return m_pImpl->bInServerPOP;
}

void SwMailMergeConfigItem::SetInServerPOP(sal_Bool bSet)
{
    if( m_pImpl->bInServerPOP != bSet)
    {
        m_pImpl->bInServerPOP = bSet;
        m_pImpl->SetModified();
    }
}

::rtl::OUString     SwMailMergeConfigItem::GetInServerUserName() const
{
    return m_pImpl->sInServerUserName;
}

void SwMailMergeConfigItem::SetInServerUserName(const ::rtl::OUString& rName)
{
    if( m_pImpl->sInServerUserName != rName)
    {
        m_pImpl->sInServerUserName = rName;
        m_pImpl->SetModified();
    }
}

::rtl::OUString     SwMailMergeConfigItem::GetInServerPassword() const
{
    return m_pImpl->sInServerPassword;
}

void SwMailMergeConfigItem::SetInServerPassword(const ::rtl::OUString& rPassword)
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
    m_rAddressBlockFrame = ::rtl::OUString();
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
    OSL_ENSURE(m_pImpl->aMergeInfos.size() > nDocument,"invalid document index");
    return m_pImpl->aMergeInfos[nDocument];
}

sal_uInt32 SwMailMergeConfigItem::GetMergedDocumentCount() const
{
    return m_pImpl->aMergeInfos.size();
}

static SwView* lcl_ExistsView(SwView* pView)
{
    const TypeId aType(TYPE(SwView));
    SfxViewShell* pViewShell = SfxViewShell::GetFirst( &aType, sal_False );
    while(pViewShell)
    {
        if(pViewShell == pView)
            return pView;

        pViewShell = SfxViewShell::GetNext( *pViewShell, &aType, sal_False );
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
        std::vector<String> aDBNameList;
        std::vector<String> aAllDBNames;
        pView->GetWrtShell().GetAllUsedDB( aDBNameList, &aAllDBNames );
        if(!aDBNameList.empty())
        {
            // if fields are available there is usually no need of an addressblock and greeting
            if(!m_pImpl->bUserSettingWereOverwritten)
            {
                if( m_pImpl->bIsAddressBlock == sal_True
                    || m_pImpl->bIsGreetingLineInMail == sal_True
                    || m_pImpl->bIsGreetingLine == sal_True )
                {
                    //store user settings
                    m_pImpl->bUserSettingWereOverwritten = sal_True;
                    m_pImpl->bIsAddressBlock_LastUserSetting = m_pImpl->bIsAddressBlock;
                    m_pImpl->bIsGreetingLineInMail_LastUserSetting = m_pImpl->bIsGreetingLineInMail;
                    m_pImpl->bIsGreetingLine_LastUserSetting = m_pImpl->bIsGreetingLine;

                    //set all to false
                    m_pImpl->bIsAddressBlock = sal_False;
                    m_pImpl->bIsGreetingLineInMail = sal_False;
                    m_pImpl->bIsGreetingLine = sal_False;

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

            m_pImpl->bUserSettingWereOverwritten = sal_False;
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
