/*************************************************************************
 *
 *  $RCSfile: envimg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:35 $
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

#include "hintids.hxx"

#ifndef _SVX_ADRITEM_HXX //autogen
#include <svx/adritem.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _TOOLS_RESID_HXX //autogen
#include <tools/resid.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX //autogen
#include <svx/paperinf.hxx>
#endif



#include "errhdl.hxx"
#include "finder.hxx"
#include "swtypes.hxx"
#include "cmdid.h"
#include "cfgid.h"
#include "envimg.hxx"
#include "cfgstr.hrc"
#include "envelp.hrc"

#define ENV_VERSION  1

#ifdef WIN
#define NEXTLINE  UniString::CreateFromAscii("\r\n")
#else
#define NEXTLINE  '\n'
#endif
//#define C2S(cChar) UniString::CreateFromAscii(cChar)



String MakeSender()
{
    ASSERT( pPathFinder, "PathFinder not found" );
    SvxAddressItem aAdr( pPathFinder->GetAddress() );

    String sRet;
    String sSenderToken(SW_RES(STR_SENDER_TOKENS));
    USHORT nTokenCount = sSenderToken.GetTokenCount(';');
    BOOL bLastLength = TRUE;
    for(USHORT i = 0; i < nTokenCount; i++)
    {
        String sToken = sSenderToken.GetToken(i, ';');
        if(sToken.EqualsAscii("COMPANY"))
        {
            String sTmp = aAdr.GetToken( ADDRESS_COMPANY );
            sRet += sTmp;
            bLastLength = sTmp.Len() > 0;
        }
        else if(sToken.EqualsAscii("CR"))
        {
            if(bLastLength)
                sRet +=NEXTLINE;
            bLastLength = TRUE;
        }
        else if(sToken.EqualsAscii("FIRSTNAME"))
            sRet += aAdr.GetFirstName();
        else if(sToken.EqualsAscii("LASTNAME"))
            sRet += aAdr.GetName();
        else if(sToken.EqualsAscii("ADDRESS"))
            sRet += aAdr.GetToken( ADDRESS_STREET );
        else if(sToken.EqualsAscii("COUNTRY"))
            sRet += aAdr.GetToken( ADDRESS_COUNTRY );
        else if(sToken.EqualsAscii("POSTALCODE"))
            sRet += aAdr.GetToken( ADDRESS_PLZ );
        else if(sToken.EqualsAscii("CITY"))
            sRet += aAdr.GetToken( ADDRESS_CITY );
        else if(sToken.EqualsAscii("STATEPROV"))
            sRet += aAdr.GetToken( ADDRESS_STATE );
        else if(sToken.Len()) //spaces
            sRet += sToken;
    }
    return sRet;


    /*
    SvxAddressItem aAdr( pPathFinder->GetAddress() );
    String aSender, aTmp;
    aSender += aAdr.GetFirstName();
    aTmp = aAdr.GetName();
    if ( aTmp.Len() )  ls

    {
        if ( aSender.Len() )
            aSender += ' ';
        aSender += aTmp;
    }
    if ( aSender.Len() )
        aSender += NEXTLINE;
    aTmp = aAdr.GetToken( ADDRESS_STREET );
    if ( aTmp.Len() )
    {
        aSender += aTmp;
        aSender += NEXTLINE;
    }
    aTmp = aAdr.GetToken( ADDRESS_COUNTRY );
    if ( aTmp.Len() )
    {
        aSender += aTmp;
        aSender += '-';
    }
    aTmp = aAdr.GetToken( ADDRESS_PLZ );
    if ( aTmp.Len() )
    {
        aSender += aTmp;
        aSender += ' ';
    }
    aSender += aAdr.GetToken( ADDRESS_CITY );
    return aSender;
     */
}



SwEnvItem::SwEnvItem() :

    SfxPoolItem(FN_ENVELOP)

{
    aAddrText       = aEmptyStr;
    bSend           = TRUE;
    aSendText       = MakeSender();
    lSendFromLeft   = 566; // 1 cm
    lSendFromTop    = 566; // 1 cm
    lWidth          = lC65Width;
    lHeight         = lC65Height;
    eAlign          = ENV_HOR_LEFT;
    bPrintFromAbove = TRUE;
    lShiftRight     = 0;
    lShiftDown      = 0;

    lAddrFromLeft   = Max(lWidth, lHeight) / 2;
    lAddrFromTop    = Min(lWidth, lHeight) / 2;
}

// --------------------------------------------------------------------------



SwEnvItem::SwEnvItem(const SwEnvItem& rItem) :

    SfxPoolItem(FN_ENVELOP),

    aAddrText      (rItem.aAddrText),
    bSend          (rItem.bSend),
    aSendText      (rItem.aSendText),
    lSendFromLeft  (rItem.lSendFromLeft),
    lSendFromTop   (rItem.lSendFromTop),
    lAddrFromLeft  (rItem.lAddrFromLeft),
    lAddrFromTop   (rItem.lAddrFromTop),
    lWidth         (rItem.lWidth),
    lHeight        (rItem.lHeight),
    eAlign         (rItem.eAlign),
    bPrintFromAbove(rItem.bPrintFromAbove),
    lShiftRight    (rItem.lShiftRight),
    lShiftDown     (rItem.lShiftDown),
    aSlot          (rItem.aSlot)

{
}

// --------------------------------------------------------------------------



SwEnvItem& SwEnvItem::operator =(const SwEnvItem& rItem)
{
    aAddrText       = rItem.aAddrText;
    bSend           = rItem.bSend;
    aSendText       = rItem.aSendText;
    lSendFromLeft   = rItem.lSendFromLeft;
    lSendFromTop    = rItem.lSendFromTop;
    lAddrFromLeft   = rItem.lAddrFromLeft;
    lAddrFromTop    = rItem.lAddrFromTop;
    lWidth          = rItem.lWidth;
    lHeight         = rItem.lHeight;
    eAlign          = rItem.eAlign;
    bPrintFromAbove = rItem.bPrintFromAbove;
    lShiftRight     = rItem.lShiftRight;
    lShiftDown      = rItem.lShiftDown;
    aSlot           = rItem.aSlot;

    return *this;
}

// --------------------------------------------------------------------------



int SwEnvItem::operator ==(const SfxPoolItem& rItem) const
{
    const SwEnvItem& rEnv = (const SwEnvItem&) rItem;

    return aAddrText       == rEnv.aAddrText       &&
           bSend           == rEnv.bSend           &&
           aSendText       == rEnv.aSendText       &&
           lSendFromLeft   == rEnv.lSendFromLeft   &&
           lSendFromTop    == rEnv.lSendFromTop    &&
           lAddrFromLeft   == rEnv.lAddrFromLeft   &&
           lAddrFromTop    == rEnv.lAddrFromTop    &&
           lWidth          == rEnv.lWidth          &&
           lHeight         == rEnv.lHeight         &&
           eAlign          == rEnv.eAlign          &&
           bPrintFromAbove == rEnv.bPrintFromAbove &&
           lShiftRight     == rEnv.lShiftRight     &&
           lShiftDown      == rEnv.lShiftDown      &&
           aSlot           == rEnv.aSlot;
}

// --------------------------------------------------------------------------



SfxPoolItem* SwEnvItem::Clone(SfxItemPool*) const
{
    return new SwEnvItem(*this);
}


// --------------------------------------------------------------------------

SwEnvCfgItem::SwEnvCfgItem() :
    SfxConfigItem(CFG_ENV_ITEM)
{
}
// ----------------------------------------------------------------------------

int SwEnvCfgItem::Load(SvStream& rStrm)
{
    USHORT nVersion;
    rStrm >> nVersion;

    if (nVersion == ENV_VERSION)
    {
        unsigned char b;
        long          l;
        USHORT        i;
        rtl_TextEncoding eEncoding = gsl_getSystemTextEncoding();
        rStrm.ReadByteString(aEnvItem.aAddrText, eEncoding);
        rStrm >> b; aEnvItem.bSend           = (BOOL)       b;
        rStrm.ReadByteString(aEnvItem.aSendText, eEncoding);
        rStrm >> l; aEnvItem.lAddrFromLeft   =              l;
        rStrm >> l; aEnvItem.lAddrFromTop    =              l;
        rStrm >> l; aEnvItem.lSendFromLeft   =              l;
        rStrm >> l; aEnvItem.lSendFromTop    =              l;
        rStrm >> l; aEnvItem.lWidth          =              l;
        rStrm >> l; aEnvItem.lHeight         =              l;
        rStrm >> i; aEnvItem.eAlign          = (SwEnvAlign) i;
        rStrm >> b; aEnvItem.bPrintFromAbove = (BOOL)       b;
        rStrm >> l; aEnvItem.lShiftRight     =              l;
        rStrm >> l; aEnvItem.lShiftDown      =              l;
        rStrm.ReadByteString(aEnvItem.aSlot, eEncoding);

        SetDefault(FALSE);

        return SfxConfigItem::ERR_OK;
    }
    else
        return SfxConfigItem::WARNING_VERSION;
}

// --------------------------------------------------------------------------



BOOL SwEnvCfgItem::Store(SvStream& rStrm)
{
    rStrm << (USHORT)ENV_VERSION;

    rtl_TextEncoding eEncoding = gsl_getSystemTextEncoding();
    rStrm.WriteByteString(aEnvItem.aAddrText, eEncoding);
    rStrm << (unsigned char) aEnvItem.bSend;
    rStrm.WriteByteString(aEnvItem.aSendText, eEncoding);
    rStrm <<                 aEnvItem.lAddrFromLeft;
    rStrm <<                 aEnvItem.lAddrFromTop;
    rStrm <<                 aEnvItem.lSendFromLeft;
    rStrm <<                 aEnvItem.lSendFromTop;
    rStrm <<                 aEnvItem.lWidth;
    rStrm <<                 aEnvItem.lHeight;
    rStrm << (USHORT)        aEnvItem.eAlign;
    rStrm << (unsigned char) aEnvItem.bPrintFromAbove;
    rStrm <<                 aEnvItem.lShiftRight;
    rStrm <<                 aEnvItem.lShiftDown;
    rStrm.WriteByteString(aEnvItem.aSlot, eEncoding);

    return SfxConfigItem::ERR_OK;
}

// --------------------------------------------------------------------------



void SwEnvCfgItem::UseDefault()
{
    aEnvItem = SwEnvItem();
    SfxConfigItem::UseDefault();

}



String SwEnvCfgItem::GetName() const
{
    return SW_RESSTR(STR_CFG_ENVIMG);
}



/*
$Log: not supported by cvs2svn $
Revision 1.36  2000/09/18 16:05:25  willem.vandorp
OpenOffice header added.

Revision 1.35  2000/06/26 16:32:11  jp
have to change: enums of AddressToken

Revision 1.34  2000/05/26 07:21:29  os
old SW Basic API Slots removed

Revision 1.33  2000/04/19 13:58:58  os
#74742# STATEPROV added

Revision 1.32  2000/04/18 15:31:35  os
UNICODE

Revision 1.31  2000/04/11 13:37:29  os
#74742# Sender field of envelopes and labels language specific

Revision 1.30  1999/10/22 14:09:27  jp
have to change - SearchFile with SfxIniManager, dont use SwFinder for this

Revision 1.29  1997/11/24 10:52:12  MA
includes


      Rev 1.28   24 Nov 1997 11:52:12   MA
   includes

      Rev 1.27   03 Nov 1997 13:17:16   MA
   precomp entfernt

      Rev 1.26   08 Apr 1997 10:30:14   OM
   Fehlende Includes

      Rev 1.25   08 Apr 1997 10:06:30   NF
   includes...

      Rev 1.24   11 Nov 1996 09:44:18   MA
   ResMgr

      Rev 1.23   26 Jul 1996 20:36:40   MA
   includes

      Rev 1.22   28 Jun 1996 10:10:52   OS
   UseDefault: Basisklasse rufen

      Rev 1.21   19 Mar 1996 16:33:48   MA
   chg: Umstellungen, Finder und AdrItem

      Rev 1.20   15 Mar 1996 13:29:24   MA
   opt: Finder abgespeckt

      Rev 1.19   27 Nov 1995 18:55:34   OS
   Umstellung 303a

      Rev 1.18   24 Nov 1995 16:59:40   OM
   PCH->PRECOMPILED

      Rev 1.17   16 Nov 1995 18:37:44   OS
   neu: Get/SetVariable, nicht impl.

      Rev 1.16   15 Sep 1995 21:14:22   OS
   add: cfgstr.hrc

      Rev 1.15   15 Sep 1995 12:41:00   OS
   GetName() implementiert

      Rev 1.14   11 Sep 1995 18:06:08   HJS
   add: sbx.hxx

      Rev 1.13   09 Aug 1995 18:59:36   AMA
   kein GetPresentation

      Rev 1.12   07 Aug 1995 18:19:58   AMA
   Umbau: GetValueText -> GetPresentation

      Rev 1.11   08 Jun 1995 19:02:06   OM
   Absturz unter OS/2 gefixt

      Rev 1.10   30 May 1995 17:01:32   ER
   Umstellung SwPaper...  SvxPaper...

      Rev 1.9   24 May 1995 18:14:22   ER
   Segmentierung

      Rev 1.8   23 Mar 1995 18:33:20   PK
   geht immer weiter ...

      Rev 1.7   17 Mar 1995 17:10:04   PK
   geht immer weiter

      Rev 1.6   06 Mar 1995 00:08:20   PK
   linkbarer envelp-zustand

      Rev 1.5   04 Mar 1995 22:55:14   PK
   geht immer weiter

      Rev 1.4   20 Feb 1995 19:39:44   PK
   erstma eingecheckt

      Rev 1.3   18 Nov 1994 15:55:28   MA
   min -> Min, max -> Max

      Rev 1.2   25 Oct 1994 17:33:38   ER
   add: PCH

      Rev 1.1   15 Apr 1994 17:28:30   PK
   warnungen raus, strings optimiert

      Rev 1.0   22 Mar 1994 17:48:14   PK
   umschlaege vorerst fertig
*/

