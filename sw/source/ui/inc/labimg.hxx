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



#ifndef _LABIMG_HXX
#define _LABIMG_HXX

#include <svl/poolitem.hxx>
#include <unotools/configitem.hxx>
#include "swdllapi.h"

// class SwLabItem ----------------------------------------------------------

class SwLabCfgItem;
class SW_DLLPUBLIC SwLabItem : public SfxPoolItem
{

public:

    SwLabItem();
    SwLabItem(const SwLabItem& rItem);

    SwLabItem& operator =(const SwLabItem& rItem);

    virtual int operator ==(const SfxPoolItem& rItem) const;

    virtual SfxPoolItem*     Clone(SfxItemPool* = 0) const;

    rtl::OUString   aLstMake; //Letzte Auswahl merken.
    rtl::OUString   aLstType;
    rtl::OUString   sDBName;  // Verwendete Datenbank

    rtl::OUString   aWriting; // Aufschrift
    rtl::OUString   aMake;   // Etikettenmarke
    rtl::OUString   aType;   // Etikettentyp
    rtl::OUString   aBin;    // Druckerschacht
    sal_Int32       lHDist;  // Horizontaler Abstand   (Benutzer)
    sal_Int32       lVDist;  // Vertikaler Abstand     (Benutzer)
    sal_Int32       lWidth;  // Breite                 (Benutzer)
    sal_Int32       lHeight;  // Hoehe                 (Benutzer)
    sal_Int32       lLeft;   // Rand links             (Benutzer)
    sal_Int32       lUpper;  // Rand oben              (Benutzer)
    sal_Int32       nCols;   // Anzahl Spalten         (Benutzer)
    sal_Int32       nRows;   // Anzahl Zeilen          (Benutzer)
    sal_Int32       nCol;    // Spalte fuer Einzeldruck
    sal_Int32       nRow;    // Zeile fuer Einzeldruck
    sal_Int32       lPaperWidth;  // Paper Width           (user)
    sal_Int32       lPaperHeight; // Paper Height          (user)
    sal_Bool        bAddr;// Adresse als Aufschrift?
    sal_Bool        bCont;// Endlospapier?
    sal_Bool        bPage;// Ganze Seite oder einzelnes Etikett?
    sal_Bool        bSynchron;// Alle Etiketten synchronisieren

    //parts of the business card
    rtl::OUString   aPrivFirstName;
    rtl::OUString   aPrivName;
    rtl::OUString   aPrivShortCut;
    rtl::OUString   aPrivFirstName2;
    rtl::OUString   aPrivName2;
    rtl::OUString   aPrivShortCut2;
    rtl::OUString   aPrivStreet;
    rtl::OUString   aPrivZip;
    rtl::OUString   aPrivCity;
    rtl::OUString   aPrivCountry;
    rtl::OUString   aPrivState;
    rtl::OUString   aPrivTitle;
    rtl::OUString   aPrivProfession;
    rtl::OUString   aPrivPhone;
    rtl::OUString   aPrivMobile;
    rtl::OUString   aPrivFax;
    rtl::OUString   aPrivWWW;
    rtl::OUString   aPrivMail;
    rtl::OUString   aCompCompany;
    rtl::OUString   aCompCompanyExt;
    rtl::OUString   aCompSlogan;
    rtl::OUString   aCompStreet;
    rtl::OUString   aCompZip;
    rtl::OUString   aCompCity;
    rtl::OUString   aCompCountry;
    rtl::OUString   aCompState;
    rtl::OUString   aCompPosition;
    rtl::OUString   aCompPhone;
    rtl::OUString   aCompMobile;
    rtl::OUString   aCompFax;
    rtl::OUString   aCompWWW;
    rtl::OUString   aCompMail;

    rtl::OUString   sGlossaryGroup;
    rtl::OUString   sGlossaryBlockName;
};

// class SwLabCfgItem -------------------------------------------------------

class SwLabCfgItem : public utl::ConfigItem
{
    SwLabItem   aItem;
    sal_Bool    bIsLabel;

    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();
public:
    SwLabCfgItem(sal_Bool bLabel);

    SwLabItem&              GetItem() {return aItem;}

    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
};

#endif

