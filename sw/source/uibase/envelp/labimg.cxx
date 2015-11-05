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
#include <swmodule.hxx>
#include "labimg.hxx"
#include "cmdid.h"
#include "swtypes.hxx"
#include <unomid.h>

using namespace utl;
using namespace css::uno;

SwLabItem::SwLabItem() :

    SfxPoolItem(FN_LABEL),
    m_lLeft (0),
    m_lUpper(0),
    m_nCols (1),
    m_nRows (1),
    m_nCol  (1),
    m_nRow  (1)
{
    m_bAddr     =
    m_bCont     =
    m_bSynchron = false;
    m_bPage  = true;
    m_lHDist   =
    m_lVDist   =
    m_lWidth   =
    m_lHeight  =
    m_lPWidth  =
    m_lPHeight = 5669; // 10 cm
}

SwLabItem::SwLabItem(const SwLabItem& rItem) :
    SfxPoolItem(FN_LABEL)
{
        *this = rItem;
}

SwLabItem& SwLabItem::operator =(const SwLabItem& rItem)
{
    m_bAddr    = rItem.m_bAddr;
    m_aWriting = rItem.m_aWriting;
    m_bCont    = rItem.m_bCont;
    m_sDBName  = rItem.m_sDBName;
    m_aLstMake = rItem.m_aLstMake;
    m_aLstType = rItem.m_aLstType;
    m_aMake    = rItem.m_aMake;
    m_aType    = rItem.m_aType;
    m_bPage    = rItem.m_bPage;
    m_bSynchron = rItem.m_bSynchron;
    m_aBin     = rItem.m_aBin;
    m_nCol     = rItem.m_nCol;
    m_nRow     = rItem.m_nRow;
    m_lHDist   = rItem.m_lHDist;
    m_lVDist   = rItem.m_lVDist;
    m_lWidth   = rItem.m_lWidth;
    m_lHeight  = rItem.m_lHeight;
    m_lLeft    = rItem.m_lLeft;
    m_lUpper   = rItem.m_lUpper;
    m_nCols    = rItem.m_nCols;
    m_nRows    = rItem.m_nRows;
    m_lPWidth  = rItem.m_lPWidth;
    m_lPHeight = rItem.m_lPHeight;
    m_aPrivFirstName =        rItem.m_aPrivFirstName;
    m_aPrivName =             rItem.m_aPrivName;
    m_aPrivShortCut =         rItem.m_aPrivShortCut;
    m_aPrivFirstName2 =       rItem.m_aPrivFirstName2;
    m_aPrivName2 =            rItem.m_aPrivName2;
    m_aPrivShortCut2 =        rItem.m_aPrivShortCut2;
    m_aPrivStreet =           rItem.m_aPrivStreet;
    m_aPrivZip =              rItem.m_aPrivZip;
    m_aPrivCity =             rItem.m_aPrivCity;
    m_aPrivCountry =          rItem.m_aPrivCountry;
    m_aPrivState =            rItem.m_aPrivState;
    m_aPrivTitle =            rItem.m_aPrivTitle;
    m_aPrivProfession =       rItem.m_aPrivProfession;
    m_aPrivPhone =            rItem.m_aPrivPhone;
    m_aPrivMobile =           rItem.m_aPrivMobile;
    m_aPrivFax =              rItem.m_aPrivFax;
    m_aPrivWWW =              rItem.m_aPrivWWW;
    m_aPrivMail =             rItem.m_aPrivMail;
    m_aCompCompany =          rItem.m_aCompCompany;
    m_aCompCompanyExt =       rItem.m_aCompCompanyExt;
    m_aCompSlogan =           rItem.m_aCompSlogan;
    m_aCompStreet =           rItem.m_aCompStreet;
    m_aCompZip =              rItem.m_aCompZip;
    m_aCompCity =             rItem.m_aCompCity;
    m_aCompCountry =          rItem.m_aCompCountry;
    m_aCompState =            rItem.m_aCompState;
    m_aCompPosition =         rItem.m_aCompPosition;
    m_aCompPhone =            rItem.m_aCompPhone;
    m_aCompMobile =           rItem.m_aCompMobile;
    m_aCompFax =              rItem.m_aCompFax;
    m_aCompWWW =              rItem.m_aCompWWW;
    m_aCompMail =             rItem.m_aCompMail;
    m_sGlossaryGroup =        rItem.m_sGlossaryGroup;
    m_sGlossaryBlockName =    rItem.m_sGlossaryBlockName;
    return *this;
}

bool SwLabItem::operator ==(const SfxPoolItem& rItem) const
{
    const SwLabItem& rLab = static_cast<const SwLabItem&>( rItem);

    return m_bAddr    == rLab.m_bAddr   &&
           m_bCont    == rLab.m_bCont   &&
           m_bPage    == rLab.m_bPage   &&
           m_bSynchron == rLab.m_bSynchron &&
           m_aBin     == rLab.m_aBin    &&
           m_nCol     == rLab.m_nCol    &&
           m_nRow     == rLab.m_nRow    &&
           m_lHDist   == rLab.m_lHDist  &&
           m_lVDist   == rLab.m_lVDist  &&
           m_lWidth   == rLab.m_lWidth  &&
           m_lHeight  == rLab.m_lHeight &&
           m_lLeft    == rLab.m_lLeft   &&
           m_lUpper   == rLab.m_lUpper  &&
           m_nCols    == rLab.m_nCols   &&
           m_nRows    == rLab.m_nRows   &&
           m_lPWidth  == rLab.m_lPWidth &&
           m_lPHeight == rLab.m_lPHeight&&
           m_aWriting == rLab.m_aWriting&&
           m_aMake    == rLab.m_aMake   &&
           m_aType    == rLab.m_aType   &&
           m_aLstMake == rLab.m_aLstMake&&
           m_aLstType == rLab.m_aLstType&&
           m_sDBName  == rLab.m_sDBName &&
            m_aPrivFirstName ==       rLab.m_aPrivFirstName&&
            m_aPrivName ==             rLab.m_aPrivName&&
            m_aPrivShortCut ==         rLab.m_aPrivShortCut&&
               m_aPrivFirstName2 ==        rLab.m_aPrivFirstName2&&
            m_aPrivName2 ==            rLab.m_aPrivName2&&
            m_aPrivShortCut2 ==        rLab.m_aPrivShortCut2&&
            m_aPrivStreet ==           rLab.m_aPrivStreet&&
            m_aPrivZip ==              rLab.m_aPrivZip&&
            m_aPrivCity ==             rLab.m_aPrivCity&&
            m_aPrivCountry ==          rLab.m_aPrivCountry&&
            m_aPrivState ==            rLab.m_aPrivState&&
            m_aPrivTitle ==            rLab.m_aPrivTitle&&
            m_aPrivProfession ==       rLab.m_aPrivProfession&&
            m_aPrivPhone ==            rLab.m_aPrivPhone&&
            m_aPrivMobile ==           rLab.m_aPrivMobile&&
            m_aPrivFax ==              rLab.m_aPrivFax&&
            m_aPrivWWW ==              rLab.m_aPrivWWW&&
            m_aPrivMail ==             rLab.m_aPrivMail&&
            m_aCompCompany ==          rLab.m_aCompCompany&&
            m_aCompCompanyExt ==       rLab.m_aCompCompanyExt&&
            m_aCompSlogan ==           rLab.m_aCompSlogan&&
            m_aCompStreet ==           rLab.m_aCompStreet&&
            m_aCompZip ==              rLab.m_aCompZip&&
            m_aCompCity ==             rLab.m_aCompCity&&
            m_aCompCountry ==          rLab.m_aCompCountry&&
            m_aCompState ==            rLab.m_aCompState&&
            m_aCompPosition ==         rLab.m_aCompPosition&&
            m_aCompPhone ==            rLab.m_aCompPhone&&
            m_aCompMobile ==           rLab.m_aCompMobile&&
            m_aCompFax ==              rLab.m_aCompFax&&
            m_aCompWWW ==              rLab.m_aCompWWW&&
            m_aCompMail ==             rLab.m_aCompMail &&
            m_sGlossaryGroup ==        rLab.m_sGlossaryGroup &&
            m_sGlossaryBlockName ==    rLab.m_sGlossaryBlockName;
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

SwLabCfgItem::SwLabCfgItem(bool bLabel) :
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
                    case  0: aItem.m_bCont = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break;// "Medium/Continuous",
                    case  1: pValues[nProp] >>= aItem.m_aMake;            break;// "Medium/Brand",
                    case  2: pValues[nProp] >>= aItem.m_aType;            break;// "Medium/Type",
                    case  3: pValues[nProp] >>= aItem.m_nCols;            break;// "Format/Column",
                    case  4: pValues[nProp] >>= aItem.m_nRows;            break;// "Format/Row",
                    case  5:
                        pValues[nProp] >>= aItem.m_lHDist;
                        aItem.m_lHDist = convertMm100ToTwip(aItem.m_lHDist);
                    break;// "Format/HorizontalDistance",
                    case  6:
                        pValues[nProp] >>= aItem.m_lVDist;
                        aItem.m_lVDist = convertMm100ToTwip(aItem.m_lVDist);
                    break;// "Format/VerticalDistance",
                    case  7:
                        pValues[nProp] >>= aItem.m_lWidth;
                        aItem.m_lWidth = convertMm100ToTwip(aItem.m_lWidth);
                    break;// "Format/Width",
                    case  8:
                        pValues[nProp] >>= aItem.m_lHeight;
                        aItem.m_lHeight = convertMm100ToTwip(aItem.m_lHeight);
                    break;// "Format/Height",
                    case  9:
                        pValues[nProp] >>= aItem.m_lLeft;
                        aItem.m_lLeft = convertMm100ToTwip(aItem.m_lLeft);
                    break;// "Format/LeftMargin",
                    case 10:
                        pValues[nProp] >>= aItem.m_lUpper;
                        aItem.m_lUpper = convertMm100ToTwip(aItem.m_lUpper);
                    break;// "Format/TopMargin",
                    case 11:
                        pValues[nProp] >>= aItem.m_lPWidth;
                        aItem.m_lPWidth = convertMm100ToTwip(aItem.m_lPWidth);
                    break;// "Format/PageWidth",
                    case 12:
                        pValues[nProp] >>= aItem.m_lPHeight;
                        aItem.m_lPHeight = convertMm100ToTwip(aItem.m_lPHeight);
                    break;// "Format/PageHeight",
                    case 13: aItem.m_bSynchron = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break;// "Option/Synchronize",
                    case 14: aItem.m_bPage = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break;// "Option/Page",
                    case 15: pValues[nProp] >>= aItem.m_nCol;             break;// "Option/Column",
                    case 16: pValues[nProp] >>= aItem.m_nRow;             break;// "Option/Row"
                    case 17: aItem.m_bAddr = *static_cast<sal_Bool const *>(pValues[nProp].getValue()); break;// "Inscription/UseAddress",
                    case 18: pValues[nProp] >>= aItem.m_aWriting;         break;// "Inscription/Address",
                    case 19: pValues[nProp] >>= aItem.m_sDBName;          break;// "Inscription/Database"
                    case 20: pValues[nProp] >>= aItem.m_aPrivFirstName;   break;// "PrivateAddress/FirstName",
                    case 21: pValues[nProp] >>= aItem.m_aPrivName;        break;// "PrivateAddress/Name",
                    case 22: pValues[nProp] >>= aItem.m_aPrivShortCut;    break;// "PrivateAddress/ShortCut",
                    case 23: pValues[nProp] >>= aItem.m_aPrivFirstName2;  break;// "PrivateAddress/SecondFirstName",
                    case 24: pValues[nProp] >>= aItem.m_aPrivName2;       break;// "PrivateAddress/SecondName",
                    case 25: pValues[nProp] >>= aItem.m_aPrivShortCut2;   break;// "PrivateAddress/SecondShortCut",
                    case 26: pValues[nProp] >>= aItem.m_aPrivStreet;      break;// "PrivateAddress/Street",
                    case 27: pValues[nProp] >>= aItem.m_aPrivZip;         break;// "PrivateAddress/Zip",
                    case 28: pValues[nProp] >>= aItem.m_aPrivCity;        break;// "PrivateAddress/City",
                    case 29: pValues[nProp] >>= aItem.m_aPrivCountry;     break;// "PrivateAddress/Country",
                    case 30: pValues[nProp] >>= aItem.m_aPrivState;       break;// "PrivateAddress/State",
                    case 31: pValues[nProp] >>= aItem.m_aPrivTitle;       break;// "PrivateAddress/Title",
                    case 32: pValues[nProp] >>= aItem.m_aPrivProfession;  break;// "PrivateAddress/Profession",
                    case 33: pValues[nProp] >>= aItem.m_aPrivPhone;       break;// "PrivateAddress/Phone",
                    case 34: pValues[nProp] >>= aItem.m_aPrivMobile;      break;// "PrivateAddress/Mobile",
                    case 35: pValues[nProp] >>= aItem.m_aPrivFax;         break;// "PrivateAddress/Fax",
                    case 36: pValues[nProp] >>= aItem.m_aPrivWWW;         break;// "PrivateAddress/WebAddress",
                    case 37: pValues[nProp] >>= aItem.m_aPrivMail;        break;// "PrivateAddress/Email",
                    case 38: pValues[nProp] >>= aItem.m_aCompCompany;     break;// "BusinessAddress/Company",
                    case 39: pValues[nProp] >>= aItem.m_aCompCompanyExt;  break;// "BusinessAddress/CompanyExt",
                    case 40: pValues[nProp] >>= aItem.m_aCompSlogan;      break;// "BusinessAddress/Slogan",
                    case 41: pValues[nProp] >>= aItem.m_aCompStreet;      break;// "BusinessAddress/Street",
                    case 42: pValues[nProp] >>= aItem.m_aCompZip;         break;// "BusinessAddress/Zip",
                    case 43: pValues[nProp] >>= aItem.m_aCompCity;        break;// "BusinessAddress/City",
                    case 44: pValues[nProp] >>= aItem.m_aCompCountry;     break;// "BusinessAddress/Country",
                    case 45: pValues[nProp] >>= aItem.m_aCompState;       break;// "BusinessAddress/State",
                    case 46: pValues[nProp] >>= aItem.m_aCompPosition;    break;// "BusinessAddress/Position",
                    case 47: pValues[nProp] >>= aItem.m_aCompPhone;       break;// "BusinessAddress/Phone",
                    case 48: pValues[nProp] >>= aItem.m_aCompMobile;      break;// "BusinessAddress/Mobile",
                    case 49: pValues[nProp] >>= aItem.m_aCompFax;         break;// "BusinessAddress/Fax",
                    case 50: pValues[nProp] >>= aItem.m_aCompWWW;         break;// "BusinessAddress/WebAddress",
                    case 51: pValues[nProp] >>= aItem.m_aCompMail;        break;// "BusinessAddress/Email",
                    case 52: pValues[nProp] >>= aItem.m_sGlossaryGroup;   break;// "AutoText/Group"
                    case 53: pValues[nProp] >>= aItem.m_sGlossaryBlockName; break;// "AutoText/Block"
                }
            }
        }
    }
    if(!bIsLabel && bNoConfigValues)
    {

        SvtUserOptions& rUserOpt = SW_MOD()->GetUserOptions();
        aItem.m_aPrivFirstName = rUserOpt.GetFirstName();
        aItem.m_aPrivName = rUserOpt.GetLastName();
        aItem.m_aPrivShortCut = rUserOpt.GetID();
        aItem.m_aCompCompany = rUserOpt.GetCompany();
        aItem.m_aCompStreet = aItem.m_aPrivStreet = rUserOpt.GetStreet();

        aItem.m_aCompCountry = aItem.m_aPrivCountry = rUserOpt.GetCountry();
        aItem.m_aCompZip = aItem.m_aPrivZip= rUserOpt.GetZip();
        aItem.m_aCompCity = aItem.m_aPrivCity = rUserOpt.GetCity();
        aItem.m_aPrivTitle = rUserOpt.GetTitle();
        aItem.m_aCompPosition = rUserOpt.GetPosition();
        aItem.m_aPrivPhone = rUserOpt.GetTelephoneHome();
        aItem.m_aCompPhone = rUserOpt.GetTelephoneWork();
        aItem.m_aCompFax = aItem.m_aPrivFax = rUserOpt.GetFax();
        aItem.m_aCompMail = aItem.m_aPrivMail = rUserOpt.GetEmail();
        aItem.m_aCompState = aItem.m_aPrivState = rUserOpt.GetState();
        aItem.m_bSynchron = true;
        SetModified();
    }
}

void SwLabCfgItem::Notify( const css::uno::Sequence< OUString >& ) {}

void SwLabCfgItem::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0, nProperty = 0; nProp < aNames.getLength(); nProp++, nProperty++)
    {
        //to have a contiuous switch an offset is added
        if(nProp == 17 && !bIsLabel)
            nProperty += 3;
        switch(nProperty)
        {
            case  0: pValues[nProp] <<= aItem.m_bCont;            break;// "Medium/Continuous",
            case  1: pValues[nProp] <<= aItem.m_aMake;            break;// "Medium/Brand",
            case  2: pValues[nProp] <<= aItem.m_aType;            break;// "Medium/Type",
            case  3: pValues[nProp] <<= aItem.m_nCols;            break;// "Format/Column",
            case  4: pValues[nProp] <<= aItem.m_nRows;            break;// "Format/Row",
            case  5: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(aItem.m_lHDist));break;// "Format/HorizontalDistance",
            case  6: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(aItem.m_lVDist));break;// "Format/VerticalDistance",
            case  7: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(aItem.m_lWidth));            break;// "Format/Width",
            case  8: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(aItem.m_lHeight));           break;// "Format/Height",
            case  9: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(aItem.m_lLeft));         break;// "Format/LeftMargin",
            case 10: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(aItem.m_lUpper));            break;// "Format/TopMargin",
            case 11: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(aItem.m_lPWidth)); break;// "Format/Page Width",
            case 12: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(aItem.m_lPHeight)); break;// "Format/PageHeight",
            case 13: pValues[nProp] <<= aItem.m_bSynchron;        break;// "Option/Synchronize",
            case 14: pValues[nProp] <<= aItem.m_bPage;            break;// "Option/Page",
            case 15: pValues[nProp] <<= aItem.m_nCol;            break;// "Option/Column",
            case 16: pValues[nProp] <<= aItem.m_nRow;            break;// "Option/Row"
            case 17: pValues[nProp] <<= aItem.m_bAddr;            break;// "Inscription/UseAddress",
            case 18: pValues[nProp] <<= aItem.m_aWriting;         break;// "Inscription/Address",
            case 19: pValues[nProp] <<= aItem.m_sDBName;          break;// "Inscription/Database"
            case 20: pValues[nProp] <<= aItem.m_aPrivFirstName;   break;// "PrivateAddress/FirstName",
            case 21: pValues[nProp] <<= aItem.m_aPrivName;        break;// "PrivateAddress/Name",
            case 22: pValues[nProp] <<= aItem.m_aPrivShortCut;    break;// "PrivateAddress/ShortCut",
            case 23: pValues[nProp] <<= aItem.m_aPrivFirstName2;  break;// "PrivateAddress/SecondFirstName",
            case 24: pValues[nProp] <<= aItem.m_aPrivName2;       break;// "PrivateAddress/SecondName",
            case 25: pValues[nProp] <<= aItem.m_aPrivShortCut2;   break;// "PrivateAddress/SecondShortCut",
            case 26: pValues[nProp] <<= aItem.m_aPrivStreet;      break;// "PrivateAddress/Street",
            case 27: pValues[nProp] <<= aItem.m_aPrivZip;         break;// "PrivateAddress/Zip",
            case 28: pValues[nProp] <<= aItem.m_aPrivCity;        break;// "PrivateAddress/City",
            case 29: pValues[nProp] <<= aItem.m_aPrivCountry;     break;// "PrivateAddress/Country",
            case 30: pValues[nProp] <<= aItem.m_aPrivState;       break;// "PrivateAddress/State",
            case 31: pValues[nProp] <<= aItem.m_aPrivTitle;       break;// "PrivateAddress/Title",
            case 32: pValues[nProp] <<= aItem.m_aPrivProfession;  break;// "PrivateAddress/Profession",
            case 33: pValues[nProp] <<= aItem.m_aPrivPhone;       break;// "PrivateAddress/Phone",
            case 34: pValues[nProp] <<= aItem.m_aPrivMobile;      break;// "PrivateAddress/Mobile",
            case 35: pValues[nProp] <<= aItem.m_aPrivFax;         break;// "PrivateAddress/Fax",
            case 36: pValues[nProp] <<= aItem.m_aPrivWWW;         break;// "PrivateAddress/WebAddress",
            case 37: pValues[nProp] <<= aItem.m_aPrivMail;        break;// "PrivateAddress/Email",
            case 38: pValues[nProp] <<= aItem.m_aCompCompany;     break;// "BusinessAddress/Company",
            case 39: pValues[nProp] <<= aItem.m_aCompCompanyExt;  break;// "BusinessAddress/CompanyExt",
            case 40: pValues[nProp] <<= aItem.m_aCompSlogan;      break;// "BusinessAddress/Slogan",
            case 41: pValues[nProp] <<= aItem.m_aCompStreet;      break;// "BusinessAddress/Street",
            case 42: pValues[nProp] <<= aItem.m_aCompZip;         break;// "BusinessAddress/Zip",
            case 43: pValues[nProp] <<= aItem.m_aCompCity;        break;// "BusinessAddress/City",
            case 44: pValues[nProp] <<= aItem.m_aCompCountry;     break;// "BusinessAddress/Country",
            case 45: pValues[nProp] <<= aItem.m_aCompState;       break;// "BusinessAddress/State",
            case 46: pValues[nProp] <<= aItem.m_aCompPosition;    break;// "BusinessAddress/Position",
            case 47: pValues[nProp] <<= aItem.m_aCompPhone;       break;// "BusinessAddress/Phone",
            case 48: pValues[nProp] <<= aItem.m_aCompMobile;      break;// "BusinessAddress/Mobile",
            case 49: pValues[nProp] <<= aItem.m_aCompFax;         break;// "BusinessAddress/Fax",
            case 50: pValues[nProp] <<= aItem.m_aCompWWW;         break;// "BusinessAddress/WebAddress",
            case 51: pValues[nProp] <<= aItem.m_aCompMail;        break;// "BusinessAddress/Email",
            case 52: pValues[nProp] <<= aItem.m_sGlossaryGroup;   break;// "AutoText/Group"
            case 53: pValues[nProp] <<= aItem.m_sGlossaryBlockName; break;// "AutoText/Block"
        }
    }
    PutProperties(aNames, aValues);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
