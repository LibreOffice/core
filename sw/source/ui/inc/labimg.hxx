/*************************************************************************
 *
 *  $RCSfile: labimg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-09-26 11:55:21 $
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

#ifndef _LABIMG_HXX
#define _LABIMG_HXX

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

// class SwLabItem ----------------------------------------------------------

class SwLabCfgItem;
class SwLabItem : public SfxPoolItem
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

    virtual void            Commit();
};

#endif

