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

#include <tools/stream.hxx>
#include <tools/resid.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <unotools/useroptions.hxx>
#include <tools/shl.hxx>
#include <swmodule.hxx>
#include "labimg.hxx"
#include "cmdid.h"
#include "swtypes.hxx"
#include <unomid.h>

using namespace utl;
using namespace ::com::sun::star::uno;

SwLabItem::SwLabItem() :

    SfxPoolItem(FN_LABEL),
    lLeft (0),
    lUpper(0),
    nCols (1),
    nRows (1),
    nCol  (1),
    nRow  (1)
{
    bAddr     =
    bCont     =
    bSynchron = sal_False;
    bPage  = sal_True;
    lHDist   =
    lVDist   =
    lWidth   =
    lHeight  =
    lPWidth  =
    lPHeight = 5669; // 10 cm
}

SwLabItem::SwLabItem(const SwLabItem& rItem) :
    SfxPoolItem(FN_LABEL)
{
        *this = rItem;
}

SwLabItem& SwLabItem::operator =(const SwLabItem& rItem)
{
    bAddr    = rItem.bAddr;
    aWriting = rItem.aWriting;
    bCont    = rItem.bCont;
    sDBName  = rItem.sDBName;
    aLstMake = rItem.aLstMake;
    aLstType = rItem.aLstType;
    aMake    = rItem.aMake;
    aType    = rItem.aType;
    bPage    = rItem.bPage;
    bSynchron = rItem.bSynchron;
    aBin     = rItem.aBin;
    nCol     = rItem.nCol;
    nRow     = rItem.nRow;
    lHDist   = rItem.lHDist;
    lVDist   = rItem.lVDist;
    lWidth   = rItem.lWidth;
    lHeight  = rItem.lHeight;
    lLeft    = rItem.lLeft;
    lUpper   = rItem.lUpper;
    nCols    = rItem.nCols;
    nRows    = rItem.nRows;
    lPWidth  = rItem.lPWidth;
    lPHeight = rItem.lPHeight;
    aPrivFirstName =        rItem.aPrivFirstName;
    aPrivName =             rItem.aPrivName;
    aPrivShortCut =         rItem.aPrivShortCut;
    aPrivFirstName2 =       rItem.aPrivFirstName2;
    aPrivName2 =            rItem.aPrivName2;
    aPrivShortCut2 =        rItem.aPrivShortCut2;
    aPrivStreet =           rItem.aPrivStreet;
    aPrivZip =              rItem.aPrivZip;
    aPrivCity =             rItem.aPrivCity;
    aPrivCountry =          rItem.aPrivCountry;
    aPrivState =            rItem.aPrivState;
    aPrivTitle =            rItem.aPrivTitle;
    aPrivProfession =       rItem.aPrivProfession;
    aPrivPhone =            rItem.aPrivPhone;
    aPrivMobile =           rItem.aPrivMobile;
    aPrivFax =              rItem.aPrivFax;
    aPrivWWW =              rItem.aPrivWWW;
    aPrivMail =             rItem.aPrivMail;
    aCompCompany =          rItem.aCompCompany;
    aCompCompanyExt =       rItem.aCompCompanyExt;
    aCompSlogan =           rItem.aCompSlogan;
    aCompStreet =           rItem.aCompStreet;
    aCompZip =              rItem.aCompZip;
    aCompCity =             rItem.aCompCity;
    aCompCountry =          rItem.aCompCountry;
    aCompState =            rItem.aCompState;
    aCompPosition =         rItem.aCompPosition;
    aCompPhone =            rItem.aCompPhone;
    aCompMobile =           rItem.aCompMobile;
    aCompFax =              rItem.aCompFax;
    aCompWWW =              rItem.aCompWWW;
    aCompMail =             rItem.aCompMail;
    sGlossaryGroup =        rItem.sGlossaryGroup;
    sGlossaryBlockName =    rItem.sGlossaryBlockName;
    return *this;
}

bool SwLabItem::operator ==(const SfxPoolItem& rItem) const
{
    const SwLabItem& rLab = (const SwLabItem&) rItem;

    return bAddr    == rLab.bAddr   &&
           bCont    == rLab.bCont   &&
           bPage    == rLab.bPage   &&
           bSynchron == rLab.bSynchron &&
           aBin     == rLab.aBin    &&
           nCol     == rLab.nCol    &&
           nRow     == rLab.nRow    &&
           lHDist   == rLab.lHDist  &&
           lVDist   == rLab.lVDist  &&
           lWidth   == rLab.lWidth  &&
           lHeight  == rLab.lHeight &&
           lLeft    == rLab.lLeft   &&
           lUpper   == rLab.lUpper  &&
           nCols    == rLab.nCols   &&
           nRows    == rLab.nRows   &&
           lPWidth  == rLab.lPWidth &&
           lPHeight == rLab.lPHeight&&
           aWriting == rLab.aWriting&&
           aMake    == rLab.aMake   &&
           aType    == rLab.aType   &&
           aLstMake == rLab.aLstMake&&
           aLstType == rLab.aLstType&&
           sDBName  == rLab.sDBName &&
            aPrivFirstName ==       rLab.aPrivFirstName&&
            aPrivName ==             rLab.aPrivName&&
            aPrivShortCut ==         rLab.aPrivShortCut&&
               aPrivFirstName2 ==        rLab.aPrivFirstName2&&
            aPrivName2 ==            rLab.aPrivName2&&
            aPrivShortCut2 ==        rLab.aPrivShortCut2&&
            aPrivStreet ==           rLab.aPrivStreet&&
            aPrivZip ==              rLab.aPrivZip&&
            aPrivCity ==             rLab.aPrivCity&&
            aPrivCountry ==          rLab.aPrivCountry&&
            aPrivState ==            rLab.aPrivState&&
            aPrivTitle ==            rLab.aPrivTitle&&
            aPrivProfession ==       rLab.aPrivProfession&&
            aPrivPhone ==            rLab.aPrivPhone&&
            aPrivMobile ==           rLab.aPrivMobile&&
            aPrivFax ==              rLab.aPrivFax&&
            aPrivWWW ==              rLab.aPrivWWW&&
            aPrivMail ==             rLab.aPrivMail&&
            aCompCompany ==          rLab.aCompCompany&&
            aCompCompanyExt ==       rLab.aCompCompanyExt&&
            aCompSlogan ==           rLab.aCompSlogan&&
            aCompStreet ==           rLab.aCompStreet&&
            aCompZip ==              rLab.aCompZip&&
            aCompCity ==             rLab.aCompCity&&
            aCompCountry ==          rLab.aCompCountry&&
            aCompState ==            rLab.aCompState&&
            aCompPosition ==         rLab.aCompPosition&&
            aCompPhone ==            rLab.aCompPhone&&
            aCompMobile ==           rLab.aCompMobile&&
            aCompFax ==              rLab.aCompFax&&
            aCompWWW ==              rLab.aCompWWW&&
            aCompMail ==             rLab.aCompMail &&
            sGlossaryGroup ==        rLab.sGlossaryGroup &&
            sGlossaryBlockName ==    rLab.sGlossaryBlockName;
}

SfxPoolItem* SwLabItem::Clone(SfxItemPool*) const
{
    return new SwLabItem(*this);
}

Sequence<OUString> SwLabCfgItem::GetPropertyNames()
{
    static const char* aLabelPropNames[] =
    {
        "Medium/Continuous",         // 0
        "Medium/Brand",             // 1
        "Medium/Type",              // 2
        "Format/Column",            // 3
        "Format/Row",               // 4
        "Format/HorizontalDistance",// 5
        "Format/VerticalDistance",  // 6
        "Format/Width",             // 7
        "Format/Height",            // 8
        "Format/LeftMargin",        // 9
        "Format/TopMargin",         //10
        "Format/PageWidth",         //11
        "Format/PageHeight",        //12
        "Option/Synchronize",       //13
        "Option/Page",              //14
        "Option/Column",            //15
        "Option/Row",               //16
        "Inscription/UseAddress",   //17
        "Inscription/Address",      //18
        "Inscription/Database"      //19
    };
    static const char* aBusinessPropNames[] =
    {
        "PrivateAddress/FirstName",             //  0
        "PrivateAddress/Name",                  //  1
        "PrivateAddress/ShortCut",              //  2
        "PrivateAddress/SecondFirstName",       //  3
        "PrivateAddress/SecondName",            //  4
        "PrivateAddress/SecondShortCut",        //  5
        "PrivateAddress/Street",                //  6
        "PrivateAddress/Zip",                   //  7
        "PrivateAddress/City",                  //  8
        "PrivateAddress/Country",               //  9
        "PrivateAddress/State",                 // 10
        "PrivateAddress/Title",                 // 11
        "PrivateAddress/Profession",            // 12
        "PrivateAddress/Phone",                 // 13
        "PrivateAddress/Mobile",                // 14
        "PrivateAddress/Fax",                   // 15
        "PrivateAddress/WebAddress",            // 16
        "PrivateAddress/Email",                 // 17
        "BusinessAddress/Company",              // 18
        "BusinessAddress/CompanyExt",           // 19
        "BusinessAddress/Slogan",               // 20
        "BusinessAddress/Street",               // 21
        "BusinessAddress/Zip",                  // 22
        "BusinessAddress/City",                 // 23
        "BusinessAddress/Country",              // 24
        "BusinessAddress/State",                // 25
        "BusinessAddress/Position",             // 26
        "BusinessAddress/Phone",                // 27
        "BusinessAddress/Mobile",               // 28
        "BusinessAddress/Fax",                  // 29
        "BusinessAddress/WebAddress",           // 30
        "BusinessAddress/Email",                // 31
        "AutoText/Group",                       // 32
        "AutoText/Block"                        // 33
    };
    const int nBusinessCount = bIsLabel ? 0 : 34;
    const int nLabelCount = bIsLabel ? 20 : 17;
    Sequence<OUString> aNames(nBusinessCount + nLabelCount);
    OUString* pNames = aNames.getArray();
    int nIndex = 0;
    for(int nLabel = 0; nLabel < nLabelCount; nLabel++)
        pNames[nIndex++] = OUString::createFromAscii(aLabelPropNames[nLabel]);
    for(int nBusiness = 0; nBusiness < nBusinessCount; nBusiness++)
        pNames[nIndex++] = OUString::createFromAscii(aBusinessPropNames[nBusiness]);
    return aNames;
}

SwLabCfgItem::SwLabCfgItem(sal_Bool bLabel) :
    ConfigItem(bLabel ? OUString("Office.Writer/Label") : OUString("Office.Writer/BusinessCard")),
    bIsLabel(bLabel)
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    bool bNoConfigValues = true;
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0, nProperty = 0; nProp < aNames.getLength(); nProp++, nProperty++)
        {

            if(pValues[nProp].hasValue())
            {
                //to have a contiuous switch an offset is added
                if(nProp == 17 && !bIsLabel)
                    nProperty += 3;
                if(nProperty >= 20)
                    bNoConfigValues = false;
                switch(nProperty)
                {
                    case  0: aItem.bCont = *(sal_Bool*)pValues[nProp].getValue(); break;// "Medium/Continuous",
                    case  1: pValues[nProp] >>= aItem.aMake;            break;// "Medium/Brand",
                    case  2: pValues[nProp] >>= aItem.aType;            break;// "Medium/Type",
                    case  3: pValues[nProp] >>= aItem.nCols;            break;// "Format/Column",
                    case  4: pValues[nProp] >>= aItem.nRows;            break;// "Format/Row",
                    case  5:
                        pValues[nProp] >>= aItem.lHDist;
                        aItem.lHDist = convertMm100ToTwip(aItem.lHDist);
                    break;// "Format/HorizontalDistance",
                    case  6:
                        pValues[nProp] >>= aItem.lVDist;
                        aItem.lVDist = convertMm100ToTwip(aItem.lVDist);
                    break;// "Format/VerticalDistance",
                    case  7:
                        pValues[nProp] >>= aItem.lWidth;
                        aItem.lWidth = convertMm100ToTwip(aItem.lWidth);
                    break;// "Format/Width",
                    case  8:
                        pValues[nProp] >>= aItem.lHeight;
                        aItem.lHeight = convertMm100ToTwip(aItem.lHeight);
                    break;// "Format/Height",
                    case  9:
                        pValues[nProp] >>= aItem.lLeft;
                        aItem.lLeft = convertMm100ToTwip(aItem.lLeft);
                    break;// "Format/LeftMargin",
                    case 10:
                        pValues[nProp] >>= aItem.lUpper;
                        aItem.lUpper = convertMm100ToTwip(aItem.lUpper);
                    break;// "Format/TopMargin",
                    case 11:
                        pValues[nProp] >>= aItem.lPWidth;
                        aItem.lPWidth = convertMm100ToTwip(aItem.lPWidth);
                    break;// "Format/PageWidth",
                    case 12:
                        pValues[nProp] >>= aItem.lPHeight;
                        aItem.lPHeight = convertMm100ToTwip(aItem.lPHeight);
                    break;// "Format/PageHeight",
                    case 13: aItem.bSynchron = *(sal_Bool*)pValues[nProp].getValue(); break;// "Option/Synchronize",
                    case 14: aItem.bPage = *(sal_Bool*)pValues[nProp].getValue(); break;// "Option/Page",
                    case 15: pValues[nProp] >>= aItem.nCol;             break;// "Option/Column",
                    case 16: pValues[nProp] >>= aItem.nRow;             break;// "Option/Row"
                    case 17: aItem.bAddr = *(sal_Bool*)pValues[nProp].getValue(); break;// "Inscription/UseAddress",
                    case 18: pValues[nProp] >>= aItem.aWriting;         break;// "Inscription/Address",
                    case 19: pValues[nProp] >>= aItem.sDBName;          break;// "Inscription/Database"
                    case 20: pValues[nProp] >>= aItem.aPrivFirstName;   break;// "PrivateAddress/FirstName",
                    case 21: pValues[nProp] >>= aItem.aPrivName;        break;// "PrivateAddress/Name",
                    case 22: pValues[nProp] >>= aItem.aPrivShortCut;    break;// "PrivateAddress/ShortCut",
                    case 23: pValues[nProp] >>= aItem.aPrivFirstName2;  break;// "PrivateAddress/SecondFirstName",
                    case 24: pValues[nProp] >>= aItem.aPrivName2;       break;// "PrivateAddress/SecondName",
                    case 25: pValues[nProp] >>= aItem.aPrivShortCut2;   break;// "PrivateAddress/SecondShortCut",
                    case 26: pValues[nProp] >>= aItem.aPrivStreet;      break;// "PrivateAddress/Street",
                    case 27: pValues[nProp] >>= aItem.aPrivZip;         break;// "PrivateAddress/Zip",
                    case 28: pValues[nProp] >>= aItem.aPrivCity;        break;// "PrivateAddress/City",
                    case 29: pValues[nProp] >>= aItem.aPrivCountry;     break;// "PrivateAddress/Country",
                    case 30: pValues[nProp] >>= aItem.aPrivState;       break;// "PrivateAddress/State",
                    case 31: pValues[nProp] >>= aItem.aPrivTitle;       break;// "PrivateAddress/Title",
                    case 32: pValues[nProp] >>= aItem.aPrivProfession;  break;// "PrivateAddress/Profession",
                    case 33: pValues[nProp] >>= aItem.aPrivPhone;       break;// "PrivateAddress/Phone",
                    case 34: pValues[nProp] >>= aItem.aPrivMobile;      break;// "PrivateAddress/Mobile",
                    case 35: pValues[nProp] >>= aItem.aPrivFax;         break;// "PrivateAddress/Fax",
                    case 36: pValues[nProp] >>= aItem.aPrivWWW;         break;// "PrivateAddress/WebAddress",
                    case 37: pValues[nProp] >>= aItem.aPrivMail;        break;// "PrivateAddress/Email",
                    case 38: pValues[nProp] >>= aItem.aCompCompany;     break;// "BusinessAddress/Company",
                    case 39: pValues[nProp] >>= aItem.aCompCompanyExt;  break;// "BusinessAddress/CompanyExt",
                    case 40: pValues[nProp] >>= aItem.aCompSlogan;      break;// "BusinessAddress/Slogan",
                    case 41: pValues[nProp] >>= aItem.aCompStreet;      break;// "BusinessAddress/Street",
                    case 42: pValues[nProp] >>= aItem.aCompZip;         break;// "BusinessAddress/Zip",
                    case 43: pValues[nProp] >>= aItem.aCompCity;        break;// "BusinessAddress/City",
                    case 44: pValues[nProp] >>= aItem.aCompCountry;     break;// "BusinessAddress/Country",
                    case 45: pValues[nProp] >>= aItem.aCompState;       break;// "BusinessAddress/State",
                    case 46: pValues[nProp] >>= aItem.aCompPosition;    break;// "BusinessAddress/Position",
                    case 47: pValues[nProp] >>= aItem.aCompPhone;       break;// "BusinessAddress/Phone",
                    case 48: pValues[nProp] >>= aItem.aCompMobile;      break;// "BusinessAddress/Mobile",
                    case 49: pValues[nProp] >>= aItem.aCompFax;         break;// "BusinessAddress/Fax",
                    case 50: pValues[nProp] >>= aItem.aCompWWW;         break;// "BusinessAddress/WebAddress",
                    case 51: pValues[nProp] >>= aItem.aCompMail;        break;// "BusinessAddress/Email",
                    case 52: pValues[nProp] >>= aItem.sGlossaryGroup;   break;// "AutoText/Group"
                    case 53: pValues[nProp] >>= aItem.sGlossaryBlockName; break;// "AutoText/Block"
                }
            }
        }
    }
    if(!bIsLabel && bNoConfigValues)
    {

        SvtUserOptions& rUserOpt = SW_MOD()->GetUserOptions();
        aItem.aPrivFirstName = rUserOpt.GetFirstName();
        aItem.aPrivName = rUserOpt.GetLastName();
        aItem.aPrivShortCut = rUserOpt.GetID();
        aItem.aCompCompany = rUserOpt.GetCompany();
        aItem.aCompStreet = aItem.aPrivStreet = rUserOpt.GetStreet();

        aItem.aCompCountry = aItem.aPrivCountry = rUserOpt.GetCountry();
        aItem.aCompZip = aItem.aPrivZip= rUserOpt.GetZip();
        aItem.aCompCity = aItem.aPrivCity = rUserOpt.GetCity();
        aItem.aPrivTitle = rUserOpt.GetTitle();
        aItem.aCompPosition = rUserOpt.GetPosition();
        aItem.aPrivPhone = rUserOpt.GetTelephoneHome();
        aItem.aCompPhone = rUserOpt.GetTelephoneWork();
        aItem.aCompFax = aItem.aPrivFax = rUserOpt.GetFax();
        aItem.aCompMail = aItem.aPrivMail = rUserOpt.GetEmail();
        aItem.aCompState = aItem.aPrivState = rUserOpt.GetState();
        aItem.bSynchron = sal_True;
        SetModified();
    }
}

void SwLabCfgItem::Notify( const ::com::sun::star::uno::Sequence< OUString >& ) {}

void    SwLabCfgItem::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0, nProperty = 0; nProp < aNames.getLength(); nProp++, nProperty++)
    {
        //to have a contiuous switch an offset is added
        if(nProp == 17 && !bIsLabel)
            nProperty += 3;
        switch(nProperty)
        {
            case  0: pValues[nProp].setValue(&aItem.bCont, rType); break;// "Medium/Continuous",
            case  1: pValues[nProp] <<= aItem.aMake;            break;// "Medium/Brand",
            case  2: pValues[nProp] <<= aItem.aType;            break;// "Medium/Type",
            case  3: pValues[nProp] <<= aItem.nCols;            break;// "Format/Column",
            case  4: pValues[nProp] <<= aItem.nRows;            break;// "Format/Row",
            case  5: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(aItem.lHDist));break;// "Format/HorizontalDistance",
            case  6: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(aItem.lVDist));break;// "Format/VerticalDistance",
            case  7: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(aItem.lWidth));            break;// "Format/Width",
            case  8: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(aItem.lHeight));           break;// "Format/Height",
            case  9: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(aItem.lLeft));         break;// "Format/LeftMargin",
            case 10: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(aItem.lUpper));            break;// "Format/TopMargin",
            case 11: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(aItem.lPWidth)); break;// "Format/Page Width",
            case 12: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(aItem.lPHeight)); break;// "Format/PageHeight",
            case 13: pValues[nProp].setValue(&aItem.bSynchron, rType); break;// "Option/Synchronize",
            case 14: pValues[nProp].setValue(&aItem.bPage, rType); break;// "Option/Page",
            case 15: pValues[nProp] <<= aItem.nCol;            break;// "Option/Column",
            case 16: pValues[nProp] <<= aItem.nRow;            break;// "Option/Row"
            case 17: pValues[nProp].setValue(&aItem.bAddr, rType); break;// "Inscription/UseAddress",
            case 18: pValues[nProp] <<= aItem.aWriting;         break;// "Inscription/Address",
            case 19: pValues[nProp] <<= aItem.sDBName;          break;// "Inscription/Database"
            case 20: pValues[nProp] <<= aItem.aPrivFirstName;   break;// "PrivateAddress/FirstName",
            case 21: pValues[nProp] <<= aItem.aPrivName;        break;// "PrivateAddress/Name",
            case 22: pValues[nProp] <<= aItem.aPrivShortCut;    break;// "PrivateAddress/ShortCut",
            case 23: pValues[nProp] <<= aItem.aPrivFirstName2;  break;// "PrivateAddress/SecondFirstName",
            case 24: pValues[nProp] <<= aItem.aPrivName2;       break;// "PrivateAddress/SecondName",
            case 25: pValues[nProp] <<= aItem.aPrivShortCut2;   break;// "PrivateAddress/SecondShortCut",
            case 26: pValues[nProp] <<= aItem.aPrivStreet;      break;// "PrivateAddress/Street",
            case 27: pValues[nProp] <<= aItem.aPrivZip;         break;// "PrivateAddress/Zip",
            case 28: pValues[nProp] <<= aItem.aPrivCity;        break;// "PrivateAddress/City",
            case 29: pValues[nProp] <<= aItem.aPrivCountry;     break;// "PrivateAddress/Country",
            case 30: pValues[nProp] <<= aItem.aPrivState;       break;// "PrivateAddress/State",
            case 31: pValues[nProp] <<= aItem.aPrivTitle;       break;// "PrivateAddress/Title",
            case 32: pValues[nProp] <<= aItem.aPrivProfession;  break;// "PrivateAddress/Profession",
            case 33: pValues[nProp] <<= aItem.aPrivPhone;       break;// "PrivateAddress/Phone",
            case 34: pValues[nProp] <<= aItem.aPrivMobile;      break;// "PrivateAddress/Mobile",
            case 35: pValues[nProp] <<= aItem.aPrivFax;         break;// "PrivateAddress/Fax",
            case 36: pValues[nProp] <<= aItem.aPrivWWW;         break;// "PrivateAddress/WebAddress",
            case 37: pValues[nProp] <<= aItem.aPrivMail;        break;// "PrivateAddress/Email",
            case 38: pValues[nProp] <<= aItem.aCompCompany;     break;// "BusinessAddress/Company",
            case 39: pValues[nProp] <<= aItem.aCompCompanyExt;  break;// "BusinessAddress/CompanyExt",
            case 40: pValues[nProp] <<= aItem.aCompSlogan;      break;// "BusinessAddress/Slogan",
            case 41: pValues[nProp] <<= aItem.aCompStreet;      break;// "BusinessAddress/Street",
            case 42: pValues[nProp] <<= aItem.aCompZip;         break;// "BusinessAddress/Zip",
            case 43: pValues[nProp] <<= aItem.aCompCity;        break;// "BusinessAddress/City",
            case 44: pValues[nProp] <<= aItem.aCompCountry;     break;// "BusinessAddress/Country",
            case 45: pValues[nProp] <<= aItem.aCompState;       break;// "BusinessAddress/State",
            case 46: pValues[nProp] <<= aItem.aCompPosition;    break;// "BusinessAddress/Position",
            case 47: pValues[nProp] <<= aItem.aCompPhone;       break;// "BusinessAddress/Phone",
            case 48: pValues[nProp] <<= aItem.aCompMobile;      break;// "BusinessAddress/Mobile",
            case 49: pValues[nProp] <<= aItem.aCompFax;         break;// "BusinessAddress/Fax",
            case 50: pValues[nProp] <<= aItem.aCompWWW;         break;// "BusinessAddress/WebAddress",
            case 51: pValues[nProp] <<= aItem.aCompMail;        break;// "BusinessAddress/Email",
            case 52: pValues[nProp] <<= aItem.sGlossaryGroup;   break;// "AutoText/Group"
            case 53: pValues[nProp] <<= aItem.sGlossaryBlockName; break;// "AutoText/Block"
        }
    }
    PutProperties(aNames, aValues);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
