/*************************************************************************
 *
 *  $RCSfile: labimg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:36 $
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
#ifndef _FINDER_HXX
#include <finder.hxx>
#endif


#include "cmdid.h"
#include "swtypes.hxx"
#include "cfgid.h"
#include "labimg.hxx"
#include "cfgstr.hrc"

#define LAB_VERSION  1
#define LAB_VERSION2 2
#define LAB_VERSION3 3
#define LAB_VERSION4 4
#define LAB_VERSION5 5
#define LAB_VERSION6 6

#define CUR_LAB_VERSION LAB_VERSION6
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

// ----------------------------------------------------------------------------
SwLabCfgItem::SwLabCfgItem() :
    SfxConfigItem(CFG_LAB_ITEM)
{
}

// ----------------------------------------------------------------------------

int  SwLabItem::Load (SvStream& rStrm, USHORT nVersion, BOOL bLabel)
{
    unsigned char b;
    USHORT        i;
    long          l;

    rtl_TextEncoding eEncoding = gsl_getSystemTextEncoding();
    rStrm >> b; bAddr     = (BOOL) b;
    rStrm.ReadByteString(aWriting, eEncoding);
    rStrm >> b; bCont     = (BOOL) b;
    rStrm.ReadByteString(aMake, eEncoding);
    rStrm.ReadByteString(aType, eEncoding);
    rStrm >> b; bPage     = (BOOL) b;
    rStrm.ReadByteString(aBin, eEncoding);
    rStrm >> i; nCol    =   i;
    rStrm >> i; nRow    =   i;
    rStrm >> l; lHDist  =   l;
    rStrm >> l; lVDist  =   l;
    rStrm >> l; lWidth  =   l;
    rStrm >> l; lHeight =   l;
    rStrm >> l; lLeft   =   l;
    rStrm >> l; lUpper  =   l;
    rStrm >> i; nCols   =   i;
    rStrm >> i; nRows   =   i;

    if ( nVersion >= LAB_VERSION2 )
    {
        rStrm.ReadByteString(aLstMake, eEncoding);
        rStrm.ReadByteString(aLstType, eEncoding);
    }
    if ( nVersion >= LAB_VERSION3 )
        rStrm.ReadByteString(sDBName, eEncoding);

    if ( nVersion >= LAB_VERSION4 )
    {
        rStrm >> b;
        bSynchron = (BOOL)b;
    }
    if(!bLabel)
    {
        rStrm.ReadByteString(aPrivFirstName, eEncoding);
        rStrm.ReadByteString(aPrivName, eEncoding);
        rStrm.ReadByteString(aPrivShortCut, eEncoding);
        if ( nVersion >= LAB_VERSION6 )
        {
            rStrm.ReadByteString(aPrivFirstName2, eEncoding);
            rStrm.ReadByteString(aPrivName2, eEncoding);
            rStrm.ReadByteString(aPrivShortCut2, eEncoding);
        }
        rStrm.ReadByteString(aPrivStreet, eEncoding);
        rStrm.ReadByteString(aPrivZip, eEncoding);
        rStrm.ReadByteString(aPrivCity, eEncoding);
        rStrm.ReadByteString(aPrivCountry, eEncoding);
        rStrm.ReadByteString(aPrivState         , eEncoding);
        rStrm.ReadByteString(aPrivTitle         , eEncoding);
        rStrm.ReadByteString(aPrivProfession    , eEncoding);
        rStrm.ReadByteString(aPrivPhone         , eEncoding);
        rStrm.ReadByteString(aPrivMobile        , eEncoding);
        rStrm.ReadByteString(aPrivFax           , eEncoding);
        rStrm.ReadByteString(aPrivWWW           , eEncoding);
        rStrm.ReadByteString(aPrivMail          , eEncoding);
        rStrm.ReadByteString(aCompCompany       , eEncoding);
        rStrm.ReadByteString(aCompCompanyExt    , eEncoding);
        rStrm.ReadByteString(aCompSlogan        , eEncoding);
        rStrm.ReadByteString(aCompStreet        , eEncoding);
        rStrm.ReadByteString(aCompZip           , eEncoding);
        rStrm.ReadByteString(aCompCity          , eEncoding);
        rStrm.ReadByteString(aCompCountry       , eEncoding);
        rStrm.ReadByteString(aCompState         , eEncoding);
        rStrm.ReadByteString(aCompPosition      , eEncoding);
        rStrm.ReadByteString(aCompPhone         , eEncoding);
        rStrm.ReadByteString(aCompMobile        , eEncoding);
        rStrm.ReadByteString(aCompFax           , eEncoding);
        rStrm.ReadByteString(aCompWWW           , eEncoding);
        rStrm.ReadByteString(aCompMail          , eEncoding);

        rStrm.ReadByteString(sGlossaryGroup     , eEncoding);
        rStrm.ReadByteString(sGlossaryBlockName , eEncoding);
    }
    return SfxConfigItem::ERR_OK;
}
// ----------------------------------------------------------------------------

BOOL SwLabItem::Store(SvStream& rStrm, BOOL bLabel)
{
    rtl_TextEncoding eEncoding = gsl_getSystemTextEncoding();
    rStrm << (unsigned char) bAddr;
    rStrm.WriteByteString(aWriting, eEncoding);
    rStrm << (unsigned char) bCont;
    rStrm.WriteByteString(aMake, eEncoding);
    rStrm.WriteByteString(aType, eEncoding);
    rStrm << (unsigned char) bPage;
    rStrm.WriteByteString(aBin, eEncoding);
    rStrm <<                 nCol;
    rStrm <<                 nRow;
    rStrm <<                 lHDist;
    rStrm <<                 lVDist;
    rStrm <<                 lWidth;
    rStrm <<                 lHeight;
    rStrm <<                 lLeft;
    rStrm <<                 lUpper;
    rStrm <<                 nCols;
    rStrm <<                 nRows;
    rStrm.WriteByteString(aLstMake, eEncoding);
    rStrm.WriteByteString(aLstType, eEncoding);
    rStrm.WriteByteString(sDBName, eEncoding);
    rStrm <<                 bSynchron;
    if(!bLabel)
    {
        rStrm.WriteByteString(aPrivFirstName, eEncoding);
        rStrm.WriteByteString(aPrivName, eEncoding);
        rStrm.WriteByteString(aPrivShortCut, eEncoding);
        rStrm.WriteByteString(aPrivFirstName2, eEncoding);
        rStrm.WriteByteString(aPrivName2, eEncoding);
        rStrm.WriteByteString(aPrivShortCut2, eEncoding);
        rStrm.WriteByteString(aPrivStreet, eEncoding);
        rStrm.WriteByteString(aPrivZip, eEncoding);
        rStrm.WriteByteString(aPrivCity, eEncoding);
        rStrm.WriteByteString(aPrivCountry, eEncoding);
        rStrm.WriteByteString(aPrivState, eEncoding);
        rStrm.WriteByteString(aPrivTitle, eEncoding);
        rStrm.WriteByteString(aPrivProfession, eEncoding);
        rStrm.WriteByteString(aPrivPhone, eEncoding);
        rStrm.WriteByteString(aPrivMobile, eEncoding);
        rStrm.WriteByteString(aPrivFax, eEncoding);
        rStrm.WriteByteString(aPrivWWW, eEncoding);
        rStrm.WriteByteString(aPrivMail, eEncoding);
        rStrm.WriteByteString(aCompCompany, eEncoding);
        rStrm.WriteByteString(aCompCompanyExt, eEncoding);
        rStrm.WriteByteString(aCompSlogan, eEncoding);
        rStrm.WriteByteString(aCompStreet, eEncoding);
        rStrm.WriteByteString(aCompZip, eEncoding);
        rStrm.WriteByteString(aCompCity, eEncoding);
        rStrm.WriteByteString(aCompCountry, eEncoding);
        rStrm.WriteByteString(aCompState, eEncoding);
        rStrm.WriteByteString(aCompPosition, eEncoding);
        rStrm.WriteByteString(aCompPhone, eEncoding);
        rStrm.WriteByteString(aCompMobile, eEncoding);
        rStrm.WriteByteString(aCompFax, eEncoding);
        rStrm.WriteByteString(aCompWWW, eEncoding);
        rStrm.WriteByteString(aCompMail, eEncoding);

        rStrm.WriteByteString(sGlossaryGroup, eEncoding);
        rStrm.WriteByteString(sGlossaryBlockName, eEncoding);
    }

    return SfxConfigItem::ERR_OK;
}
// ----------------------------------------------------------------------------
int SwLabCfgItem::Load(SvStream& rStrm)
{
    USHORT nVersion;
    rStrm >> nVersion;

    if( nVersion >= LAB_VERSION )
    {
        aLabItem.Load(rStrm, nVersion, TRUE);
        if(LAB_VERSION5 <= nVersion)
            aBusinessItem.Load(rStrm, nVersion, FALSE);

        SetDefault(FALSE);

        return SfxConfigItem::ERR_OK;
    }
    else
        return SfxConfigItem::WARNING_VERSION;
}

BOOL SwLabCfgItem::Store(SvStream& rStrm)
{
    rStrm << (USHORT)CUR_LAB_VERSION;
    aLabItem.Store(rStrm, TRUE);
    aBusinessItem.Store(rStrm, FALSE);
    return SfxConfigItem::ERR_OK;
}
// ----------------------------------------------------------------------------
void SwLabCfgItem::UseDefault()
{
    aLabItem = SwLabItem();
    aBusinessItem = SwLabItem();
    //Fill item with user data
    SvxAddressItem aAdr( pPathFinder->GetAddress() );

    aBusinessItem.aPrivFirstName = aAdr.GetFirstName();
    aBusinessItem.aPrivName = aAdr.GetName();
    aBusinessItem.aPrivShortCut = aAdr.GetShortName();
    aBusinessItem.aCompCompany = aAdr.GetToken( ADDRESS_COMPANY       );
    aBusinessItem.aCompStreet = aBusinessItem.aPrivStreet = aAdr.GetToken( ADDRESS_STREET);

    aBusinessItem.aCompCountry = aBusinessItem.aPrivCountry = aAdr.GetToken( ADDRESS_COUNTRY);
    aBusinessItem.aCompZip = aBusinessItem.aPrivZip= aAdr.GetToken( ADDRESS_PLZ );
    aBusinessItem.aCompCity = aBusinessItem.aPrivCity = aAdr.GetToken( ADDRESS_CITY );
    aBusinessItem.aPrivTitle = aAdr.GetToken( ADDRESS_TITLE );
    aBusinessItem.aCompPosition = aAdr.GetToken( ADDRESS_POSITION );
    aBusinessItem.aPrivPhone = aAdr.GetToken( ADDRESS_TEL_PRIVATE );
    aBusinessItem.aCompPhone = aAdr.GetToken( ADDRESS_TEL_COMPANY );
    aBusinessItem.aCompFax = aBusinessItem.aPrivFax = aAdr.GetToken( ADDRESS_FAX    );
    aBusinessItem.aCompMail = aBusinessItem.aPrivMail = aAdr.GetToken( ADDRESS_EMAIL    );
    aBusinessItem.aCompState = aBusinessItem.aPrivState = aAdr.GetToken( ADDRESS_STATE  );
    aBusinessItem.bSynchron = TRUE;

    SfxConfigItem::UseDefault();
}
// ----------------------------------------------------------------------------
String SwLabCfgItem::GetName() const
{
    return SW_RESSTR(STR_CFG_LABIMG);
}



/*
$Log: not supported by cvs2svn $
Revision 1.38  2000/09/18 16:05:26  willem.vandorp
OpenOffice header added.

Revision 1.37  2000/06/26 16:32:15  jp
have to change: enums of AddressToken

Revision 1.36  2000/05/26 07:21:29  os
old SW Basic API Slots removed

Revision 1.35  2000/04/18 15:31:35  os
UNICODE

Revision 1.34  2000/02/11 14:45:49  hr
#70473# changes for unicode ( patched by automated patchtool )

Revision 1.33  2000/01/11 13:26:13  os
#71720# business cards synchronized by default

Revision 1.32  1999/12/07 16:34:13  os
#70355##70559# SHORTCUT->INITIALS; second private name

Revision 1.31  1999/09/30 07:53:06  os
Label config item and LabItem contain business information

Revision 1.30  1998/03/14 16:06:50  OM
Gelinkte Etiketten


      Rev 1.29   14 Mar 1998 17:06:50   OM
   Gelinkte Etiketten

      Rev 1.28   24 Nov 1997 11:52:10   MA
   includes

      Rev 1.27   03 Nov 1997 13:17:14   MA
   precomp entfernt

      Rev 1.26   08 Aug 1997 17:29:34   OM
   Headerfile-Umstellung

      Rev 1.25   18 Nov 1996 10:45:48   SWG
   includes

      Rev 1.24   14 Nov 1996 15:24:44   TRI
   includes

      Rev 1.23   11 Nov 1996 09:44:16   MA
   ResMgr

      Rev 1.22   26 Jul 1996 20:36:40   MA
   includes

      Rev 1.21   28 Jun 1996 10:10:52   OS
   UseDefault: Basisklasse rufen

      Rev 1.20   06 Jun 1996 12:59:42   OM
   Datenbankname merken

      Rev 1.19   27 Nov 1995 18:55:40   OS
   Umstellung 303a

      Rev 1.18   24 Nov 1995 16:59:40   OM
   PCH->PRECOMPILED

      Rev 1.17   16 Nov 1995 18:37:46   OS
   neu: Get/SetVariable, nicht impl.

      Rev 1.16   21 Sep 1995 00:40:56   JP
   Bug 19511: VersionsNummer im richtigen Format aus dem Stream lesen (int <-> USHORT!!!)

      Rev 1.15   15 Sep 1995 21:14:22   OS
   add: cfgstr.hrc

      Rev 1.14   15 Sep 1995 12:41:00   OS
   GetName() implementiert

      Rev 1.13   04 Sep 1995 13:53:58   HJS
   add: #include <sbx.hxx>

      Rev 1.12   09 Aug 1995 18:59:46   AMA
   kein GetPresentation

      Rev 1.11   07 Aug 1995 18:19:46   AMA
   Umbau: GetValueText -> GetPresentation

      Rev 1.10   30 Jul 1995 11:25:30   MA
   chg: Letze Einstellung merken fuer Label

      Rev 1.9   10 Jul 1995 09:52:36   MA
   LabelDlg optimiert und etwas aufgeraeumt.

      Rev 1.8   24 May 1995 18:17:14   ER
   Segmentierung

      Rev 1.7   17 Mar 1995 17:10:30   PK
   geht immer weiter

      Rev 1.6   15 Mar 1995 13:27:04   PK
   geht immer weiter

      Rev 1.5   06 Mar 1995 00:08:28   PK
   linkbarer envelp-zustand

      Rev 1.4   20 Feb 1995 19:39:52   PK
   erstma eingecheckt

      Rev 1.3   09 Jan 1995 16:57:48   ER
    del: envelp hrc

*/

