/*************************************************************************
 *
 *  $RCSfile: labimg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2001-03-30 14:27:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _RESID_HXX //autogen
#include <vcl/resid.hxx>
#endif

#ifndef _SVX_ADRITEM_HXX
#include <svx/adritem.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#include "labimg.hxx"
#include "cmdid.h"
#include "swtypes.hxx"

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

#define C2U(cChar) OUString::createFromAscii(cChar)

// ----------------------------------------------------------------------------
SwLabItem::SwLabItem() :

    SfxPoolItem(FN_LABEL),
    nCol  (1),
    nRow  (1),
    lLeft (0),
    lUpper(0),
    nCols (1),
    nRows (1)
{
    bAddr = bCont = bSynchron = FALSE;
    bPage = TRUE;
    lHDist  =
    lVDist  =
    lWidth  =
    lHeight = 5669; // 10 cm
}

// ----------------------------------------------------------------------------
SwLabItem::SwLabItem(const SwLabItem& rItem) :
    SfxPoolItem(FN_LABEL)
{
        *this = rItem;
}
// ----------------------------------------------------------------------------
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

// --------------------------------------------------------------------------



int SwLabItem::operator ==(const SfxPoolItem& rItem) const
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
           aWriting == rLab.aWriting&&
           aMake    == rLab.aMake   &&
           aType    == rLab.aType   &&
           aLstMake == rLab.aLstMake&&
           aLstType == rLab.aLstType&&
           sDBName  == rLab.sDBName &&
               aPrivFirstName ==        rLab.aPrivFirstName&&
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

// --------------------------------------------------------------------------
SfxPoolItem* SwLabItem::Clone(SfxItemPool*) const
{
    return new SwLabItem(*this);
}

/* -----------------------------25.09.00 16:25--------------------------------

 ---------------------------------------------------------------------------*/
Sequence<rtl::OUString> SwLabCfgItem::GetPropertyNames()
{
    static const char* aLabelPropNames[] =
    {
        "Medium/Continous",         // 0
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
        "Option/Synchronize",       //11
        "Option/Page",              //12
        "Option/Column",            //13
        "Option/Row",               //14
        "Inscription/UseAddress",   //15
        "Inscription/Address",      //16
        "Inscription/Database"      //17
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
    const int nLabelCount = bIsLabel ? 18 : 15;
    Sequence<OUString> aNames(nBusinessCount + nLabelCount);
    OUString* pNames = aNames.getArray();
    int nIndex = 0;
    for(int nLabel = 0; nLabel < nLabelCount; nLabel++)
        pNames[nIndex++] = OUString::createFromAscii(aLabelPropNames[nLabel]);
    for(int nBusiness = 0; nBusiness < nBusinessCount; nBusiness++)
        pNames[nIndex++] = OUString::createFromAscii(aBusinessPropNames[nBusiness]);
    return aNames;
}
/* ----------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
SwLabCfgItem::SwLabCfgItem(sal_Bool bLabel) :
    ConfigItem(bLabel ? C2U("Office.Writer/Label") :  C2U("Office.Writer/BusinessCard")),
    bIsLabel(bLabel)
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    sal_Bool bNoConfigValues = sal_True;
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0, nProperty = 0; nProp < aNames.getLength(); nProp++, nProperty++)
        {

            if(pValues[nProp].hasValue())
            {
                //to have a contiuous switch an offset is added
                if(nProp == 15 && !bIsLabel)
                    nProperty += 3;
                if(nProperty >= 18)
                    bNoConfigValues = sal_False;
                switch(nProperty)
                {
                    case  0: aItem.bCont = *(sal_Bool*)pValues[nProp].getValue(); break;// "Medium/Continous",
                    case  1: pValues[nProp] >>= aItem.aMake;            break;// "Medium/Brand",
                    case  2: pValues[nProp] >>= aItem.aType;            break;// "Medium/Type",
                    case  3: pValues[nProp] >>= aItem.nCols;            break;// "Format/Column",
                    case  4: pValues[nProp] >>= aItem.nRows;            break;// "Format/Row",
                    case  5:
                        pValues[nProp] >>= aItem.lHDist;
                        aItem.lHDist = MM100_TO_TWIP(aItem.lHDist);
                    break;// "Format/HorizontalDistance",
                    case  6:
                        pValues[nProp] >>= aItem.lVDist;
                        aItem.lVDist = MM100_TO_TWIP(aItem.lVDist);
                    break;// "Format/VerticalDistance",
                    case  7:
                        pValues[nProp] >>= aItem.lWidth;
                        aItem.lWidth = MM100_TO_TWIP(aItem.lWidth);
                    break;// "Format/Width",
                    case  8:
                        pValues[nProp] >>= aItem.lHeight;
                        aItem.lHeight = MM100_TO_TWIP(aItem.lHeight);
                    break;// "Format/Height",
                    case  9:
                        pValues[nProp] >>= aItem.lLeft;
                        aItem.lLeft = MM100_TO_TWIP(aItem.lLeft);
                    break;// "Format/LeftMargin",
                    case 10:
                        pValues[nProp] >>= aItem.lUpper;
                        aItem.lUpper = MM100_TO_TWIP(aItem.lUpper);
                    break;// "Format/TopMargin",
                    case 11: aItem.bSynchron = *(sal_Bool*)pValues[nProp].getValue(); break;// "Option/Synchronize",
                    case 12: aItem.bPage = *(sal_Bool*)pValues[nProp].getValue(); break;// "Option/Page",
                    case 13: pValues[nProp] >>= aItem.nCol;     break;// "Option/Column",
                    case 14: pValues[nProp] >>= aItem.nRow;     break;// "Option/Row"
                    case 15: aItem.bAddr = *(sal_Bool*)pValues[nProp].getValue();       break;// "Inscription/UseAddress",
                    case 16: pValues[nProp] >>= aItem.aWriting;         break;// "Inscription/Address",
                    case 17: pValues[nProp] >>= aItem.sDBName;          break;// "Inscription/Database"
                    case 18: pValues[nProp] >>= aItem.aPrivFirstName;   break;// "PrivateAddress/FirstName",
                    case 19: pValues[nProp] >>= aItem.aPrivName;        break;// "PrivateAddress/Name",
                    case 20: pValues[nProp] >>= aItem.aPrivShortCut;    break;// "PrivateAddress/ShortCut",
                    case 21: pValues[nProp] >>= aItem.aPrivFirstName2;  break;// "PrivateAddress/SecondFirstName",
                    case 22: pValues[nProp] >>= aItem.aPrivName2;       break;// "PrivateAddress/SecondName",
                    case 23: pValues[nProp] >>= aItem.aPrivShortCut2;   break;// "PrivateAddress/SecondShortCut",
                    case 24: pValues[nProp] >>= aItem.aPrivStreet;      break;// "PrivateAddress/Street",
                    case 25: pValues[nProp] >>= aItem.aPrivZip;         break;// "PrivateAddress/Zip",
                    case 26: pValues[nProp] >>= aItem.aPrivCity;        break;// "PrivateAddress/City",
                    case 27: pValues[nProp] >>= aItem.aPrivCountry;     break;// "PrivateAddress/Country",
                    case 28: pValues[nProp] >>= aItem.aPrivState;       break;// "PrivateAddress/State",
                    case 29: pValues[nProp] >>= aItem.aPrivTitle;       break;// "PrivateAddress/Title",
                    case 30: pValues[nProp] >>= aItem.aPrivProfession;  break;// "PrivateAddress/Profession",
                    case 31: pValues[nProp] >>= aItem.aPrivPhone;       break;// "PrivateAddress/Phone",
                    case 32: pValues[nProp] >>= aItem.aPrivMobile;      break;// "PrivateAddress/Mobile",
                    case 33: pValues[nProp] >>= aItem.aPrivFax;         break;// "PrivateAddress/Fax",
                    case 34: pValues[nProp] >>= aItem.aPrivWWW;         break;// "PrivateAddress/WebAddress",
                    case 35: pValues[nProp] >>= aItem.aPrivMail;        break;// "PrivateAddress/Email",
                    case 36: pValues[nProp] >>= aItem.aCompCompany;     break;// "BusinessAddress/Company",
                    case 37: pValues[nProp] >>= aItem.aCompCompanyExt;  break;// "BusinessAddress/CompanyExt",
                    case 38: pValues[nProp] >>= aItem.aCompSlogan;      break;// "BusinessAddress/Slogan",
                    case 39: pValues[nProp] >>= aItem.aCompStreet;      break;// "BusinessAddress/Street",
                    case 40: pValues[nProp] >>= aItem.aCompZip;         break;// "BusinessAddress/Zip",
                    case 41: pValues[nProp] >>= aItem.aCompCity;        break;// "BusinessAddress/City",
                    case 42: pValues[nProp] >>= aItem.aCompCountry;     break;// "BusinessAddress/Country",
                    case 43: pValues[nProp] >>= aItem.aCompState;       break;// "BusinessAddress/State",
                    case 44: pValues[nProp] >>= aItem.aCompPosition;    break;// "BusinessAddress/Position",
                    case 45: pValues[nProp] >>= aItem.aCompPhone;       break;// "BusinessAddress/Phone",
                    case 46: pValues[nProp] >>= aItem.aCompMobile;      break;// "BusinessAddress/Mobile",
                    case 47: pValues[nProp] >>= aItem.aCompFax;         break;// "BusinessAddress/Fax",
                    case 48: pValues[nProp] >>= aItem.aCompWWW;         break;// "BusinessAddress/WebAddress",
                    case 49: pValues[nProp] >>= aItem.aCompMail;        break;// "BusinessAddress/Email",
                    case 50: pValues[nProp] >>= aItem.sGlossaryGroup;   break;// "AutoText/Group"
                    case 51: pValues[nProp] >>= aItem.sGlossaryBlockName; break;// "AutoText/Block"
                }
            }
        }
    }
    if(!bIsLabel && bNoConfigValues)
    {

        SvxAddressItem aAdr;
        aItem.aPrivFirstName = aAdr.GetFirstName();
        aItem.aPrivName = aAdr.GetName();
        aItem.aPrivShortCut = aAdr.GetShortName();
        aItem.aCompCompany = aAdr.GetToken( POS_COMPANY       );
        aItem.aCompStreet = aItem.aPrivStreet = aAdr.GetToken( POS_STREET);

        aItem.aCompCountry = aItem.aPrivCountry = aAdr.GetToken( POS_COUNTRY);
        aItem.aCompZip = aItem.aPrivZip= aAdr.GetToken( POS_PLZ );
        aItem.aCompCity = aItem.aPrivCity = aAdr.GetToken( POS_CITY );
        aItem.aPrivTitle = aAdr.GetToken( POS_TITLE );
        aItem.aCompPosition = aAdr.GetToken( POS_POSITION );
        aItem.aPrivPhone = aAdr.GetToken( POS_TEL_PRIVATE );
        aItem.aCompPhone = aAdr.GetToken( POS_TEL_COMPANY );
        aItem.aCompFax = aItem.aPrivFax = aAdr.GetToken( POS_FAX    );
        aItem.aCompMail = aItem.aPrivMail = aAdr.GetToken( POS_EMAIL    );
        aItem.aCompState = aItem.aPrivState = aAdr.GetToken( POS_STATE  );
        aItem.bSynchron = TRUE;
        SetModified();
    }
}
/* -----------------------------25.09.00 16:26--------------------------------

 ---------------------------------------------------------------------------*/
void    SwLabCfgItem::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0, nProperty = 0; nProp < aNames.getLength(); nProp++, nProperty++)
    {
        //to have a contiuous switch an offset is added
        if(nProp == 15 && !bIsLabel)
            nProperty += 3;
        switch(nProperty)
        {
            case  0: pValues[nProp].setValue(&aItem.bCont, rType); break;// "Medium/Continous",
            case  1: pValues[nProp] <<= aItem.aMake;            break;// "Medium/Brand",
            case  2: pValues[nProp] <<= aItem.aType;            break;// "Medium/Type",
            case  3: pValues[nProp] <<= aItem.nCols;            break;// "Format/Column",
            case  4: pValues[nProp] <<= aItem.nRows;            break;// "Format/Row",
            case  5: pValues[nProp] <<= TWIP_TO_MM100(aItem.lHDist);break;// "Format/HorizontalDistance",
            case  6: pValues[nProp] <<= TWIP_TO_MM100(aItem.lVDist);break;// "Format/VerticalDistance",
            case  7: pValues[nProp] <<= TWIP_TO_MM100(aItem.lWidth);            break;// "Format/Width",
            case  8: pValues[nProp] <<= TWIP_TO_MM100(aItem.lHeight);           break;// "Format/Height",
            case  9: pValues[nProp] <<= TWIP_TO_MM100(aItem.lLeft);         break;// "Format/LeftMargin",
            case 10: pValues[nProp] <<= TWIP_TO_MM100(aItem.lUpper);            break;// "Format/TopMargin",
            case 11: pValues[nProp].setValue(&aItem.bSynchron, rType); break;// "Option/Synchronize",
            case 12: pValues[nProp].setValue(&aItem.bPage, rType); break;// "Option/Page",
            case 13: pValues[nProp] <<= aItem.nCol;     break;// "Option/Column",
            case 14: pValues[nProp] <<= aItem.nRow;     break;// "Option/Row"
            case 15: pValues[nProp].setValue(&aItem.bAddr, rType);      break;// "Inscription/UseAddress",
            case 16: pValues[nProp] <<= aItem.aWriting;         break;// "Inscription/Address",
            case 17: pValues[nProp] <<= aItem.sDBName;          break;// "Inscription/Database"
            case 18: pValues[nProp] <<= aItem.aPrivFirstName;   break;// "PrivateAddress/FirstName",
            case 19: pValues[nProp] <<= aItem.aPrivName;        break;// "PrivateAddress/Name",
            case 20: pValues[nProp] <<= aItem.aPrivShortCut;    break;// "PrivateAddress/ShortCut",
            case 21: pValues[nProp] <<= aItem.aPrivFirstName2;  break;// "PrivateAddress/SecondFirstName",
            case 22: pValues[nProp] <<= aItem.aPrivName2;       break;// "PrivateAddress/SecondName",
            case 23: pValues[nProp] <<= aItem.aPrivShortCut2;   break;// "PrivateAddress/SecondShortCut",
            case 24: pValues[nProp] <<= aItem.aPrivStreet;      break;// "PrivateAddress/Street",
            case 25: pValues[nProp] <<= aItem.aPrivZip;         break;// "PrivateAddress/Zip",
            case 26: pValues[nProp] <<= aItem.aPrivCity;        break;// "PrivateAddress/City",
            case 27: pValues[nProp] <<= aItem.aPrivCountry;     break;// "PrivateAddress/Country",
            case 28: pValues[nProp] <<= aItem.aPrivState;       break;// "PrivateAddress/State",
            case 29: pValues[nProp] <<= aItem.aPrivTitle;       break;// "PrivateAddress/Title",
            case 30: pValues[nProp] <<= aItem.aPrivProfession;  break;// "PrivateAddress/Profession",
            case 31: pValues[nProp] <<= aItem.aPrivPhone;       break;// "PrivateAddress/Phone",
            case 32: pValues[nProp] <<= aItem.aPrivMobile;      break;// "PrivateAddress/Mobile",
            case 33: pValues[nProp] <<= aItem.aPrivFax;         break;// "PrivateAddress/Fax",
            case 34: pValues[nProp] <<= aItem.aPrivWWW;         break;// "PrivateAddress/WebAddress",
            case 35: pValues[nProp] <<= aItem.aPrivMail;        break;// "PrivateAddress/Email",
            case 36: pValues[nProp] <<= aItem.aCompCompany;     break;// "BusinessAddress/Company",
            case 37: pValues[nProp] <<= aItem.aCompCompanyExt;  break;// "BusinessAddress/CompanyExt",
            case 38: pValues[nProp] <<= aItem.aCompSlogan;      break;// "BusinessAddress/Slogan",
            case 39: pValues[nProp] <<= aItem.aCompStreet;      break;// "BusinessAddress/Street",
            case 40: pValues[nProp] <<= aItem.aCompZip;         break;// "BusinessAddress/Zip",
            case 41: pValues[nProp] <<= aItem.aCompCity;        break;// "BusinessAddress/City",
            case 42: pValues[nProp] <<= aItem.aCompCountry;     break;// "BusinessAddress/Country",
            case 43: pValues[nProp] <<= aItem.aCompState;       break;// "BusinessAddress/State",
            case 44: pValues[nProp] <<= aItem.aCompPosition;    break;// "BusinessAddress/Position",
            case 45: pValues[nProp] <<= aItem.aCompPhone;       break;// "BusinessAddress/Phone",
            case 46: pValues[nProp] <<= aItem.aCompMobile;      break;// "BusinessAddress/Mobile",
            case 47: pValues[nProp] <<= aItem.aCompFax;         break;// "BusinessAddress/Fax",
            case 48: pValues[nProp] <<= aItem.aCompWWW;         break;// "BusinessAddress/WebAddress",
            case 49: pValues[nProp] <<= aItem.aCompMail;        break;// "BusinessAddress/Email",
            case 50: pValues[nProp] <<= aItem.sGlossaryGroup;   break;// "AutoText/Group"
            case 51: pValues[nProp] <<= aItem.sGlossaryBlockName; break;// "AutoText/Block"
        }
    }
    PutProperties(aNames, aValues);
}

