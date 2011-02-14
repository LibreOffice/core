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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include <mmconfigitem.hxx>
#include <swtypes.hxx>
#include <tools/debug.hxx>
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
#include <unotools/configitem.hxx>
#include <mailmergehelper.hxx>
#include <swunohelper.hxx>
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <wrtsh.hxx>
#include <dbui.hrc>
#include <vector>

#include <unomid.h>

#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>

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

/*-- 16.04.2004 09:41:36---------------------------------------------------

  -----------------------------------------------------------------------*/
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

/*-- 16.04.2004 09:43:29---------------------------------------------------

  -----------------------------------------------------------------------*/
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

/*-- 06.05.2004 12:51:54---------------------------------------------------

  -----------------------------------------------------------------------*/
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
    DBG_ASSERT(aValues.getLength() == rNames.getLength(), "GetProperties failed");
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
    Sequence<OUString> aAssignments = GetNodeNames(C2U(cAddressDataAssignments));
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
            String sAssignPath = C2U(cAddressDataAssignments);
            sAssignPath += '/';
            sAssignPath += String(pAssignments[nAssign / 4]);
            sAssignPath += '/';
            pAssignProperties[nAssign] = sAssignPath;
            pAssignProperties[nAssign] += C2U(cDataSourceName);
            pAssignProperties[nAssign + 1] = sAssignPath;
            pAssignProperties[nAssign + 1] += C2U(cDataTableName);
            pAssignProperties[nAssign + 2] = sAssignPath;
            pAssignProperties[nAssign + 2] += C2U(cDataCommandType);
            pAssignProperties[nAssign + 3] = sAssignPath;
            pAssignProperties[nAssign + 3] += C2U(cDBColumnAssignments);
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
/*-- 06.05.2004 12:51:54---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailMergeConfigItem_Impl::~SwMailMergeConfigItem_Impl()
{
}
/*-- 13.03.2006 12:12:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem_Impl::SetCurrentAddressBlockIndex( sal_Int32 nSet )
{
    if(aAddressBlocks.size() >= sal::static_int_cast<sal_uInt32, sal_Int32>(nSet))
    {
        nCurrentAddressBlock = nSet;
        SetModified();
    }
}
/*-- 16.04.2004 13:06:07---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString lcl_CreateNodeName(Sequence<OUString>& rAssignments )
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
// --------------------------------------------------------------------------------
void lcl_ConvertToNumbers(OUString& rBlock, const ResStringArray& rHeaders )
{
    //convert the strings used for UI to numbers used for the configuration
    String sBlock(rBlock);
    sBlock.SearchAndReplaceAllAscii("\n", String::CreateFromAscii("\\n"));
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
// --------------------------------------------------------------------------------
void lcl_ConvertFromNumbers(OUString& rBlock, const ResStringArray& rHeaders)
{
    //convert the numbers used for the configuration to strings used for UI to numbers
    //doesn't use ReplaceAll to prevent expansion of numbers inside of the headers
    String sBlock(rBlock);
    sBlock.SearchAndReplaceAllAscii("\\n", '\n');
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
                DBG_ERROR("parse error in address block or greeting line");
            }
        }
        else
            sBlock += aNext.sText;
    }
    rBlock = sBlock;
}

/*--------------------------------------------------------------------

 --------------------------------------------------------------------*/
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
/*-- 15.04.2004 08:48:39---------------------------------------------------

  -----------------------------------------------------------------------*/
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
    Sequence<OUString> aAssignments = GetNodeNames(C2U(cAddressDataAssignments));

    ::std::vector<DBAddressDataAssignment>::iterator aAssignIter;
    for(aAssignIter = aAddressDataAssignments.begin();
                aAssignIter != aAddressDataAssignments.end(); aAssignIter++)
    {
        if(aAssignIter->bColumnAssignmentsChanged)
        {
            //create a new node name
            OUString sNewNode = aAssignIter->sConfigNodeName.getLength() ?
                        aAssignIter->sConfigNodeName :
                        lcl_CreateNodeName(aAssignments);
            OUString sSlash = C2U("/");
            OUString sNodePath = C2U(cAddressDataAssignments);
            sNodePath += sSlash;
            sNodePath += sNewNode;
            sNodePath += sSlash;
            //only one new entry is written
            Sequence< PropertyValue > aNewValues(4);
            PropertyValue* pNewValues = aNewValues.getArray();
            pNewValues[0].Name = sNodePath;
            pNewValues[0].Name += C2U(cDataSourceName);
            pNewValues[0].Value <<= aAssignIter->aDBData.sDataSource;
            pNewValues[1].Name = sNodePath;
            pNewValues[1].Name += C2U(cDataTableName);
            pNewValues[1].Value <<= aAssignIter->aDBData.sCommand;
            pNewValues[2].Name = sNodePath;
            pNewValues[2].Name += C2U(cDataCommandType);
            pNewValues[2].Value <<= aAssignIter->aDBData.nCommandType;
            pNewValues[3].Name = sNodePath;
            pNewValues[3].Name += C2U(cDBColumnAssignments);
            pNewValues[3].Value <<= aAssignIter->aDBColumnAssignments;

            SetSetProperties(C2U(cAddressDataAssignments), aNewValues);
        }
    }

    bUserSettingWereOverwritten = sal_False;
}
/*-- 06.05.2004 13:04:36---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 06.05.2004 13:04:36---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 30.04.2004 11:04:52---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 30.04.2004 11:04:52---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 11.05.2004 13:13:54---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 11.05.2004 13:13:54---------------------------------------------------

  -----------------------------------------------------------------------*/
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

static SwMailMergeConfigItem_Impl* pOptions = NULL;
static sal_Int32            nRefCount = 0;
static ::osl::Mutex aMutex;
/*-- 15.04.2004 08:42:43---------------------------------------------------

  -----------------------------------------------------------------------*/
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
    ::osl::MutexGuard aGuard( aMutex );
    if ( !pOptions )
        pOptions = new SwMailMergeConfigItem_Impl;
    ++nRefCount;
    m_pImpl = pOptions;
}
/*-- 15.04.2004 08:43:36---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailMergeConfigItem::~SwMailMergeConfigItem()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( aMutex );
    if ( !--nRefCount )
    {
        DELETEZ( pOptions );
    }
}
/*-- 06.05.2004 14:18:10---------------------------------------------------

  -----------------------------------------------------------------------*/
void  SwMailMergeConfigItem::Commit()
{
    if(m_pImpl->IsModified())
        m_pImpl->Commit();
}
/*-- 06.05.2004 12:59:50---------------------------------------------------

  -----------------------------------------------------------------------*/
const ResStringArray&   SwMailMergeConfigItem::GetDefaultAddressHeaders() const
{
    return m_pImpl->m_AddressHeaderSA;
}
/*-- 27.04.2004 14:34:16---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetAddressBlocks(
        const Sequence< ::rtl::OUString>& rBlocks)
{
    m_pImpl->SetAddressBlocks(rBlocks);
}
/*-- 27.04.2004 14:34:16---------------------------------------------------

  -----------------------------------------------------------------------*/
const Sequence< ::rtl::OUString> SwMailMergeConfigItem::GetAddressBlocks() const
{
    return m_pImpl->GetAddressBlocks();
}
/*-- 11.05.2004 17:08:45---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwMailMergeConfigItem::IsAddressBlock()const
{
    return m_pImpl->bIsAddressBlock && IsOutputToLetter();
}
/*-- 11.05.2004 17:08:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void     SwMailMergeConfigItem::SetAddressBlock(sal_Bool bSet)
{
    m_pImpl->bUserSettingWereOverwritten = sal_False;
    if(m_pImpl->bIsAddressBlock != bSet)
    {
        m_pImpl->bIsAddressBlock = bSet;
        m_pImpl->SetModified();
    }
}

/*-- 30.08.2005 15:09:46---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwMailMergeConfigItem::IsHideEmptyParagraphs() const
{
    return m_pImpl->bIsHideEmptyParagraphs;
}
/*-- 30.08.2005 15:09:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetHideEmptyParagraphs(sal_Bool bSet)
{
    if(m_pImpl->bIsHideEmptyParagraphs != bSet)
    {
        m_pImpl->bIsHideEmptyParagraphs = bSet;
        m_pImpl->SetModified();
    }
}
/*-- 28.04.2004 13:00:02---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwMailMergeConfigItem::IsIncludeCountry() const
{
    return m_pImpl->bIncludeCountry;
}
/*-- 28.04.2004 13:00:02---------------------------------------------------

  -----------------------------------------------------------------------*/
rtl::OUString& SwMailMergeConfigItem::GetExcludeCountry() const
{
    return m_pImpl->sExcludeCountry;
}
/*-- 28.04.2004 13:00:02---------------------------------------------------

  -----------------------------------------------------------------------*/
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

/*-- 28.04.2004 15:35:16---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 28.04.2004 15:38:11---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XDataSource>  SwMailMergeConfigItem::GetSource()
{
    return m_pImpl->xSource;
}
/*-- 28.04.2004 15:38:11---------------------------------------------------

  -----------------------------------------------------------------------*/
SharedConnection SwMailMergeConfigItem::GetConnection()
{
    return m_pImpl->xConnection;
}
/*-- 28.04.2004 15:38:11---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 30.04.2004 14:30:55---------------------------------------------------

  -----------------------------------------------------------------------*/
const SwDBData&    SwMailMergeConfigItem::GetCurrentDBData() const
{
    return m_pImpl->aDBData;
}

/*-- 17.06.2004 13:18:47---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 29.04.2004 11:34:36---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XResultSet>   SwMailMergeConfigItem::GetResultSet() const
{
    if(!m_pImpl->xConnection.is() && m_pImpl->aDBData.sDataSource.getLength())
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
                    xRowProperties->setPropertyValue(C2U("ApplyFilter"), makeAny(m_pImpl->sFilter.getLength()>0));
                    xRowProperties->setPropertyValue(C2U("Filter"), makeAny(m_pImpl->sFilter));
                }
                catch(Exception&)
                {
                    DBG_ERROR("exception caught in xResultSet->SetFilter()");
                }
                xRowSet->execute();
                m_pImpl->xResultSet = xRowSet.get();
                m_pImpl->xResultSet->first();
                m_pImpl->nResultSetCursorPos = 1;
            }
        }
        catch(Exception& )
        {
            DBG_ERROR("exception caught in: SwMailMergeConfigItem::GetResultSet() ");
        }
    }
    return m_pImpl->xResultSet;
}
/*-- 13.08.2004 11:49:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::DisposeResultSet()
{
    m_pImpl->xConnection.clear();
    if(m_pImpl->xResultSet.is())
    {
        ::comphelper::disposeComponent( m_pImpl->xResultSet );
    }
}
/*-- 14.05.2004 15:07:55---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString&    SwMailMergeConfigItem::GetFilter() const
{
    return m_pImpl->sFilter;
}
/*-- 14.05.2004 15:07:55---------------------------------------------------

  -----------------------------------------------------------------------*/
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
                xRowProperties->setPropertyValue(C2U("ApplyFilter"), makeAny(m_pImpl->sFilter.getLength()>0));
                xRowProperties->setPropertyValue(C2U("Filter"), makeAny(m_pImpl->sFilter));
                uno::Reference<XRowSet> xRowSet( m_pImpl->xResultSet, UNO_QUERY_THROW );
                xRowSet->execute();
            }
            catch(Exception&)
            {
                DBG_ERROR("exception caught in SwMailMergeConfigItem::SetFilter()");
            }
        }
    }
}
/*-- 29.04.2004 11:55:38---------------------------------------------------

  -----------------------------------------------------------------------*/
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
        catch(Exception&)
        {
        }
    }
    return m_pImpl->nResultSetCursorPos;
}

/*-- 27.05.2004 13:56:18---------------------------------------------------

  -----------------------------------------------------------------------*/
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
        catch(Exception&)
        {
        }
    }
    return bRet;
}
/*-- 29.04.2004 11:55:38---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwMailMergeConfigItem::GetResultSetPosition() const
{
    return m_pImpl->nResultSetCursorPos;
}
/*-- 27.05.2004 14:49:53---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 27.05.2004 14:49:53---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 27.05.2004 15:08:35---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 16.06.2004 15:15:56---------------------------------------------------

  -----------------------------------------------------------------------*/
const uno::Sequence< ::rtl::OUString>&
                    SwMailMergeConfigItem::GetSavedDocuments() const
{
    return m_pImpl->aSavedDocuments;
}
/*-- 16.06.2004 15:15:56---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 28.04.2004 16:15:16---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwMailMergeConfigItem::IsOutputToLetter()const
{
    return m_pImpl->bIsOutputToLetter || !IsMailAvailable();
}
/*-- 28.04.2004 16:15:16---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetOutputToLetter(sal_Bool bSet)
{
    if(m_pImpl->bIsOutputToLetter != bSet)
    {
        m_pImpl->bIsOutputToLetter = bSet;
        m_pImpl->SetModified();
    }
}
/*-- 30.04.2004 10:51:10---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwMailMergeConfigItem::IsIndividualGreeting(sal_Bool bInEMail) const
{
    return bInEMail ?
            m_pImpl->bIsIndividualGreetingLineInMail :
            m_pImpl->bIsIndividualGreetingLine;
}
/*-- 30.04.2004 10:51:10---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 30.04.2004 10:51:10---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwMailMergeConfigItem::IsGreetingLine(sal_Bool bInEMail) const
{
    return bInEMail ? m_pImpl->bIsGreetingLineInMail : m_pImpl->bIsGreetingLine;
}
/*-- 30.04.2004 10:51:10---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 30.04.2004 11:04:52---------------------------------------------------

  -----------------------------------------------------------------------*/
const Sequence< ::rtl::OUString>   SwMailMergeConfigItem::GetGreetings(
        Gender eType ) const
{
    return m_pImpl->GetGreetings(eType);
}
/*-- 30.04.2004 11:04:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void  SwMailMergeConfigItem::SetGreetings(
        Gender eType, const Sequence< ::rtl::OUString>& rSetGreetings)
{
    m_pImpl->SetGreetings( eType, rSetGreetings);
}

/*-- 11.05.2004 13:10:54---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwMailMergeConfigItem::GetCurrentGreeting(
                        SwMailMergeConfigItem::Gender eType) const
{
    return m_pImpl->GetCurrentGreeting(eType);
}
/*-- 11.05.2004 13:10:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetCurrentGreeting(Gender eType, sal_Int32 nIndex)
{
    m_pImpl->SetCurrentGreeting(eType, nIndex);
}
/*-- 12.05.2004 12:29:59---------------------------------------------------

  -----------------------------------------------------------------------*/
const ::rtl::OUString& SwMailMergeConfigItem::GetFemaleGenderValue() const
{
    return m_pImpl->sFemaleGenderValue;
}
/*-- 12.05.2004 12:29:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetFemaleGenderValue(const ::rtl::OUString rValue)
{
    if( m_pImpl->sFemaleGenderValue != rValue )
    {
        m_pImpl->sFemaleGenderValue = rValue;
        m_pImpl->SetModified();
    }
}

/*-- 30.04.2004 13:25:41---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< ::rtl::OUString> SwMailMergeConfigItem::GetColumnAssignment(
                const SwDBData& rDBData ) const
{
    Sequence< ::rtl::OUString> aRet;
    ::std::vector<DBAddressDataAssignment>::iterator aAssignIter;
    for(aAssignIter = m_pImpl->aAddressDataAssignments.begin();
                aAssignIter != m_pImpl->aAddressDataAssignments.end(); aAssignIter++)
    {
        if(aAssignIter->aDBData == rDBData)
        {
            aRet = aAssignIter->aDBColumnAssignments;
            break;
        }
    }
    return aRet;
}
/*-- 21.05.2004 12:31:31---------------------------------------------------
    returns the name that is assigned as e-mail column of the current data base
  -----------------------------------------------------------------------*/
::rtl::OUString     SwMailMergeConfigItem::GetAssignedColumn(sal_uInt32 nColumn) const
{
    ::rtl::OUString sRet;
    Sequence< ::rtl::OUString> aAssignment = GetColumnAssignment( m_pImpl->aDBData );
    if(aAssignment.getLength() > sal::static_int_cast< sal_Int32, sal_uInt32>(nColumn) && aAssignment[nColumn].getLength())
        sRet = aAssignment[nColumn];
    else if(nColumn < m_pImpl->m_AddressHeaderSA.Count())
        sRet = m_pImpl->m_AddressHeaderSA.GetString(nColumn);
    return sRet;
}
/*-- 30.04.2004 13:25:41---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetColumnAssignment( const SwDBData& rDBData,
                            const Sequence< ::rtl::OUString>& rList)
{
    ::std::vector<DBAddressDataAssignment>::iterator aAssignIter;
    sal_Bool bFound = sal_False;
    for(aAssignIter = m_pImpl->aAddressDataAssignments.begin();
                aAssignIter != m_pImpl->aAddressDataAssignments.end(); aAssignIter++)
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

/*-- 07.09.2005 11:50:27---------------------------------------------------

  -----------------------------------------------------------------------*/
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
                if(rHeaders.GetString(nColumn) == aItem.sText &&
                    pAssignment[nColumn].getLength())
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
/*-- 07.09.2005 11:50:27---------------------------------------------------

  -----------------------------------------------------------------------*/
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
                if(rHeaders.GetString(nColumn) == aItem.sText &&
                    pAssignment[nColumn].getLength())
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
/*-- 05.05.2004 16:10:07---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString     SwMailMergeConfigItem::GetMailDisplayName() const
{
    return m_pImpl->sMailDisplayName;
}
/*-- 05.05.2004 16:10:08---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetMailDisplayName(const ::rtl::OUString& rName)
{
    if(m_pImpl->sMailDisplayName != rName)
    {
        m_pImpl->sMailDisplayName = rName;
        m_pImpl->SetModified();
    }
}
/*-- 05.05.2004 16:10:09---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString     SwMailMergeConfigItem::GetMailAddress() const
{
    return m_pImpl->sMailAddress;
}
/*-- 05.05.2004 16:10:09---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetMailAddress(const ::rtl::OUString& rAddress)
{
    if(m_pImpl->sMailAddress != rAddress )
    {
        m_pImpl->sMailAddress = rAddress;
        m_pImpl->SetModified();
    }
}

/*-- 07.05.2004 12:40:59---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwMailMergeConfigItem::IsMailReplyTo() const
{
    return m_pImpl->bIsMailReplyTo;
}
/*-- 07.05.2004 12:40:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void  SwMailMergeConfigItem::SetMailReplyTo(sal_Bool bSet)
{
    if(m_pImpl->bIsMailReplyTo != bSet)
    {
        m_pImpl->bIsMailReplyTo = bSet;
        m_pImpl->SetModified();
    }
}
/*-- 05.05.2004 16:10:09---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString     SwMailMergeConfigItem::GetMailReplyTo() const
{
    return m_pImpl->sMailReplyTo;
}
/*-- 05.05.2004 16:10:09---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetMailReplyTo(const ::rtl::OUString& rReplyTo)
{
    if(m_pImpl->sMailReplyTo != rReplyTo)
    {
        m_pImpl->sMailReplyTo = rReplyTo;
        m_pImpl->SetModified();
    }
}
/*-- 05.05.2004 16:10:09---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString  SwMailMergeConfigItem::GetMailServer() const
{
    return m_pImpl->sMailServer;
}
/*-- 05.05.2004 16:10:10---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetMailServer(const ::rtl::OUString& rAddress)
{
    if(m_pImpl->sMailServer != rAddress)
    {
        m_pImpl->sMailServer = rAddress;
        m_pImpl->SetModified();
    }
}
/*-- 05.05.2004 16:10:10---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int16 SwMailMergeConfigItem::GetMailPort() const
{
    return m_pImpl->bIsDefaultPort ?
             (m_pImpl->bIsSecureConnection ? SECURE_PORT : DEFAULT_PORT) :
             m_pImpl->nMailPort;
}
/*-- 05.05.2004 16:10:10---------------------------------------------------

  -----------------------------------------------------------------------*/
void     SwMailMergeConfigItem::SetMailPort(sal_Int16 nSet)
{
    if(m_pImpl->nMailPort != nSet || m_pImpl->bIsDefaultPort)
    {
        m_pImpl->nMailPort = nSet;
        m_pImpl->bIsDefaultPort = sal_False;
        m_pImpl->SetModified();
    }
}
/*-- 05.05.2004 16:10:11---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwMailMergeConfigItem::IsSecureConnection() const
{
    return m_pImpl->bIsSecureConnection;
}
/*-- 05.05.2004 16:10:12---------------------------------------------------

  -----------------------------------------------------------------------*/
void     SwMailMergeConfigItem::SetSecureConnection(sal_Bool bSet)
{
    if(m_pImpl->bIsSecureConnection != bSet)
    {
        m_pImpl->bIsSecureConnection = bSet;
        m_pImpl->SetModified();
    }
}
/*-- 05.05.2004 16:10:12---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwMailMergeConfigItem::IsAuthentication() const
{
    return m_pImpl->bIsAuthentication;
}
/*-- 05.05.2004 16:10:13---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetAuthentication(sal_Bool bSet)
{
    if(m_pImpl->bIsAuthentication != bSet)
    {
        m_pImpl->bIsAuthentication = bSet;
        m_pImpl->SetModified();
    }
}
/*-- 05.05.2004 16:10:13---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString     SwMailMergeConfigItem::GetMailUserName() const
{
    return m_pImpl->sMailUserName;
}
/*-- 05.05.2004 16:10:13---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetMailUserName(const ::rtl::OUString& rName)
{
    if(m_pImpl->sMailUserName != rName)
    {
        m_pImpl->sMailUserName = rName;
        m_pImpl->SetModified();
    }
}
/*-- 05.05.2004 16:10:14---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString     SwMailMergeConfigItem::GetMailPassword() const
{
    return m_pImpl->sMailPassword;
}
/*-- 05.05.2004 16:10:14---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetMailPassword(const ::rtl::OUString& rPassword)
{
    if(m_pImpl->sMailPassword != rPassword)
    {
        m_pImpl->sMailPassword = rPassword;
        m_pImpl->SetModified();
    }
}
/*-- 19.08.2004 14:44:57---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwMailMergeConfigItem::IsSMTPAfterPOP() const
{
    return m_pImpl->bIsSMPTAfterPOP;
}
/*-- 19.08.2004 14:44:57---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetSMTPAfterPOP(sal_Bool bSet)
{
    if( m_pImpl->bIsSMPTAfterPOP != bSet)
    {
        m_pImpl->bIsSMPTAfterPOP = bSet;
        m_pImpl->SetModified();
    }
}
/*-- 19.08.2004 14:44:57---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString     SwMailMergeConfigItem::GetInServerName() const
{
    return m_pImpl->sInServerName;
}
/*-- 19.08.2004 14:44:57---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetInServerName(const ::rtl::OUString& rServer)
{
    if(m_pImpl->sInServerName != rServer)
    {
        m_pImpl->sInServerName = rServer;
        m_pImpl->SetModified();
    }
}
/*-- 19.08.2004 14:44:58---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int16           SwMailMergeConfigItem::GetInServerPort() const
{
    return m_pImpl->nInServerPort;
}
/*-- 19.08.2004 14:44:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetInServerPort(sal_Int16 nSet)
{
    if( m_pImpl->nInServerPort != nSet)
    {
        m_pImpl->nInServerPort = nSet;
        m_pImpl->SetModified();
    }
}
/*-- 20.08.2004 08:52:48---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwMailMergeConfigItem::IsInServerPOP() const
{
    return m_pImpl->bInServerPOP;
}
/*-- 20.08.2004 08:52:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetInServerPOP(sal_Bool bSet)
{
    if( m_pImpl->bInServerPOP != bSet)
    {
        m_pImpl->bInServerPOP = bSet;
        m_pImpl->SetModified();
    }
}
/*-- 19.08.2004 14:44:58---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString     SwMailMergeConfigItem::GetInServerUserName() const
{
    return m_pImpl->sInServerUserName;
}
/*-- 19.08.2004 14:44:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetInServerUserName(const ::rtl::OUString& rName)
{
    if( m_pImpl->sInServerUserName != rName)
    {
        m_pImpl->sInServerUserName = rName;
        m_pImpl->SetModified();
    }
}
/*-- 19.08.2004 14:44:59---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString     SwMailMergeConfigItem::GetInServerPassword() const
{
    return m_pImpl->sInServerPassword;
}
/*-- 19.08.2004 14:45:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetInServerPassword(const ::rtl::OUString& rPassword)
{
    if(m_pImpl->sInServerPassword != rPassword)
    {
        m_pImpl->sInServerPassword = rPassword;
        m_pImpl->SetModified();
    }
}

/*-- 02.09.2004 14:43:27---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::DocumentReloaded()
{
    m_bMergeDone = false;
    m_bGreetingInserted = false;
    m_bAddressInserted = false;
    m_rAddressBlockFrame = ::rtl::OUString();
}
/*-- 16.06.2004 12:24:18---------------------------------------------------

  -----------------------------------------------------------------------*/
bool SwMailMergeConfigItem::IsMailAvailable() const
{
    return m_pImpl->bIsEMailSupported;
}
/*-- 21.05.2004 12:20:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::AddMergedDocument(SwDocMergeInfo& rInfo)
{
    m_pImpl->aMergeInfos.push_back(rInfo);
}
/*-- 21.05.2004 12:20:05---------------------------------------------------

  -----------------------------------------------------------------------*/
SwDocMergeInfo& SwMailMergeConfigItem::GetDocumentMergeInfo(sal_uInt32 nDocument)
{
    DBG_ASSERT(m_pImpl->aMergeInfos.size() > nDocument,"invalid document index");
    return m_pImpl->aMergeInfos[nDocument];
}
/*-- 14.06.2004 11:46:26---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_uInt32 SwMailMergeConfigItem::GetMergedDocumentCount() const
{
    return m_pImpl->aMergeInfos.size();
}
/*-- 11.06.2004 10:38:39---------------------------------------------------

  -----------------------------------------------------------------------*/
SwView* lcl_ExistsView(SwView* pView)
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
/*-- 16.06.2004 15:02:35---------------------------------------------------

  -----------------------------------------------------------------------*/
SwView*  SwMailMergeConfigItem::GetTargetView()
{
    //make sure that the pointer is really valid - the document may have been closed manually
    if(m_pTargetView)
    {
        m_pTargetView = lcl_ExistsView(m_pTargetView);
    }
    return m_pTargetView;
}
/*-- 02.09.2004 17:04:11---------------------------------------------------

  -----------------------------------------------------------------------*/
void  SwMailMergeConfigItem::SetTargetView(SwView* pView)
{
    m_pTargetView = pView;
    //reset the document merge counter
    if(!m_pTargetView)
    {
        m_pImpl->aMergeInfos.clear();
    }
}
/*-- 16.06.2004 15:02:35---------------------------------------------------

  -----------------------------------------------------------------------*/
SwView* SwMailMergeConfigItem::GetSourceView()
{
    m_pSourceView = lcl_ExistsView(m_pSourceView);
    return m_pSourceView;
}

/*-- 04.11.2004 19:53 ---------------------------------------------------
  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetSourceView(SwView* pView)
{
    m_pSourceView = pView;

    if(pView)
    {
        SvStringsDtor aDBNameList(5, 1);
        SvStringsDtor aAllDBNames(5, 5);
        pView->GetWrtShell().GetAllUsedDB( aDBNameList, &aAllDBNames );
        if(aDBNameList.Count())
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

/*-- 13.03.2006 12:15:06---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeConfigItem::SetCurrentAddressBlockIndex( sal_Int32 nSet )
{
    m_pImpl->SetCurrentAddressBlockIndex( nSet );
}
/*-- 13.03.2006 12:15:07---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwMailMergeConfigItem::GetCurrentAddressBlockIndex() const
{
    return m_pImpl->GetCurrentAddressBlockIndex();
}
