/*************************************************************************
 *
 *  $RCSfile: labimg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:40 $
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

#ifndef _SFXCFGITEM_HXX //autogen
#include <sfx2/cfgitem.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

// class SwLabItem ----------------------------------------------------------

class SwLabItem : public SfxPoolItem
{
public:

    SwLabItem();
    SwLabItem(const SwLabItem& rItem);

    SwLabItem& operator =(const SwLabItem& rItem);

    virtual int operator ==(const SfxPoolItem& rItem) const;

    virtual SfxPoolItem*     Clone(SfxItemPool* = 0) const;

    int  Load (SvStream& rStrm, USHORT nVersion, BOOL bLabel);
    BOOL Store(SvStream& rStrm, BOOL bLabel);

    String aLstMake; //Letzte Auswahl merken.
    String aLstType;
    String sDBName;  // Verwendete Datenbank

    String aWriting; // Aufschrift
    String aMake;    // Etikettenmarke
    String aType;    // Etikettentyp
    String aBin;     // Druckerschacht
    long   lHDist;   // Horizontaler Abstand   (Benutzer)
    long   lVDist;   // Vertikaler Abstand     (Benutzer)
    long   lWidth;   // Breite                 (Benutzer)
    long   lHeight;  // Hoehe                  (Benutzer)
    long   lLeft;    // Rand links             (Benutzer)
    long   lUpper;   // Rand oben              (Benutzer)
    USHORT nCols;    // Anzahl Spalten         (Benutzer)
    USHORT nRows;    // Anzahl Zeilen          (Benutzer)
    USHORT nCol;     // Spalte fuer Einzeldruck
    USHORT nRow;     // Zeile fuer Einzeldruck
    BOOL   bAddr  :1;// Adresse als Aufschrift?
    BOOL   bCont  :1;// Endlospapier?
    BOOL   bPage  :1;// Ganze Seite oder einzelnes Etikett?
    BOOL   bSynchron:1;// Alle Etiketten synchronisieren

    //parts of the business card
    String  aPrivFirstName;
    String  aPrivName;
    String  aPrivShortCut;
    String  aPrivFirstName2;
    String  aPrivName2;
    String  aPrivShortCut2;
    String  aPrivStreet;
    String  aPrivZip;
    String  aPrivCity;
    String  aPrivCountry;
    String  aPrivState;
    String  aPrivTitle;
    String  aPrivProfession;
    String  aPrivPhone;
    String  aPrivMobile;
    String  aPrivFax;
    String  aPrivWWW;
    String  aPrivMail;
    String  aCompCompany;
    String  aCompCompanyExt;
    String  aCompSlogan;
    String  aCompStreet;
    String  aCompZip;
    String  aCompCity;
    String  aCompCountry;
    String  aCompState;
    String  aCompPosition;
    String  aCompPhone;
    String  aCompMobile;
    String  aCompFax;
    String  aCompWWW;
    String  aCompMail;

    String  sGlossaryGroup;
    String  sGlossaryBlockName;
};

// class SwLabCfgItem -------------------------------------------------------

class SwLabCfgItem : public SfxConfigItem
{
    SwLabItem aLabItem;
    SwLabItem aBusinessItem;
protected:
    virtual int  Load (SvStream& rStrm);
    virtual BOOL Store(SvStream& rStrm);
    virtual void UseDefault();

public:
    SwLabCfgItem();

    SwLabItem&      GetLabItem() {return aLabItem;}
    SwLabItem&      GetBusinessItem() {return aBusinessItem;}
    virtual String  GetName() const;
};

#endif

