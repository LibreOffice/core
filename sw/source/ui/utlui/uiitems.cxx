/*************************************************************************
 *
 *  $RCSfile: uiitems.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:50 $
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

#ifndef _SVX_ITEMTYPE_HXX
#include <svx/itemtype.hxx>
#endif
#ifndef _UNOSETT_HXX
#include <unosett.hxx>
#endif

#include "uiparam.hxx"
#include "swtypes.hxx"
#include "cmdid.h"
#include "pagedesc.hxx"
#include "uiitems.hxx"

#include "utlui.hrc"
#include "attrdesc.hrc"

using namespace ::com::sun::star;

// Breitenangaben der Fussnotenlinien, mit TabPage abstimmen
static const USHORT __FAR_DATA nFtnLines[] = {
    0,
    10,
    50,
    80,
    100,
    150
};

#define FTN_LINE_STYLE_COUNT 5


SwPageFtnInfoItem::SwPageFtnInfoItem( const USHORT nId, SwPageFtnInfo& rInfo) :
    SfxPoolItem( nId ),
    aFtnInfo(rInfo)
{
}


SwPageFtnInfoItem::SwPageFtnInfoItem( const SwPageFtnInfoItem& rItem ) :
    SfxPoolItem( rItem ),
    aFtnInfo(rItem.GetPageFtnInfo())
{
}


 SwPageFtnInfoItem::~SwPageFtnInfoItem()
{
}


SfxPoolItem*  SwPageFtnInfoItem::Clone( SfxItemPool *pPool ) const
{
    return new SwPageFtnInfoItem( *this );
}


int  SwPageFtnInfoItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( Which() == rAttr.Which(), "keine gleichen Attribute" );
    return ( aFtnInfo == ((SwPageFtnInfoItem&)rAttr).GetPageFtnInfo());
}


SfxItemPresentation  SwPageFtnInfoItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    String&             rText,
    const International*    pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            USHORT nHght = (USHORT) GetPageFtnInfo().GetHeight();
            if ( nHght )
            {
                rText = SW_RESSTR( STR_MAX_FTN_HEIGHT );
                rText += ' ';
                rText += ::GetMetricText( nHght, eCoreUnit, ePresUnit );
                rText += ::GetSvxString( ::GetMetricId( ePresUnit ) );
            }
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}



SwPtrItem::SwPtrItem( const USHORT nId, void* pPtr ) :
    SfxPoolItem( nId ),
    pMisc(pPtr)
{
}

/*--------------------------------------------------------------------
    Beschreibung: Copy-Konstruktor
 --------------------------------------------------------------------*/


SwPtrItem::SwPtrItem( const SwPtrItem& rItem ) : SfxPoolItem( rItem )
{
    pMisc = rItem.pMisc;
}

/*--------------------------------------------------------------------
    Beschreibung: Clonen
 --------------------------------------------------------------------*/


SfxPoolItem* SwPtrItem::Clone( SfxItemPool *pPool ) const
{
    return new SwPtrItem( *this );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


int SwPtrItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
    const SwPtrItem& rItem = (SwPtrItem&)rAttr;
    return ( pMisc == rItem.pMisc );
}


/*-----------------12.11.97 12:55-------------------------------
 SwUINumRuleItem fuer die NumTabPages der FormatNumRule/Stylisten
---------------------------------------------------------------*/
SwUINumRuleItem::SwUINumRuleItem( const SwNumRule& rRul, const USHORT nId )
    : SfxPoolItem( nId ), pRule( new SwNumRule( rRul ) )
{
}

SwUINumRuleItem::SwUINumRuleItem( const String& rName, const USHORT nId )
    : SfxPoolItem( nId ), pRule( new SwNumRule( rName ) )
{
}

SwUINumRuleItem::SwUINumRuleItem( const SwUINumRuleItem& rItem )
    : SfxPoolItem( rItem ),
    pRule( new SwNumRule( *rItem.pRule ))
{
}

 SwUINumRuleItem::~SwUINumRuleItem()
{
    delete pRule;
}


SfxPoolItem*  SwUINumRuleItem::Clone( SfxItemPool *pPool ) const
{
    return new SwUINumRuleItem( *this );
}

int  SwUINumRuleItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
    return *pRule == *((SwUINumRuleItem&)rAttr).pRule;
}

BOOL SwUINumRuleItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    uno::Reference< container::XIndexReplace >xRules = new SwXNumberingRules(*pRule);
    rVal.setValue(&xRules, ::getCppuType((uno::Reference< container::XIndexReplace>*)0));
    return TRUE;
}
BOOL SwUINumRuleItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    if(rVal.getValueType() == ::getCppuType((uno::Reference< container::XIndexReplace >*)0) )
    {
        uno::Reference< container::XIndexReplace>* pxRulesRef = (uno::Reference< container::XIndexReplace>*)rVal.getValue();
        uno::Reference< lang::XUnoTunnel > xTunnel(*pxRulesRef, uno::UNO_QUERY);
        SwXNumberingRules* pSwXRules = xTunnel.is() ? (SwXNumberingRules*)
                    xTunnel->getSomething(SwXNumberingRules::getUnoTunnelId()) : 0;
        if(pSwXRules)
        {
            *pRule = *pSwXRules->GetNumRule();
        }
    }
    return TRUE;
}
/* -----------------17.06.98 17:43-------------------
 *
 * --------------------------------------------------*/
SwBackgroundDestinationItem::SwBackgroundDestinationItem(USHORT  nWhich, USHORT nValue) :
    SfxUInt16Item(nWhich, nValue)
{
}
/* -----------------17.06.98 17:44-------------------
 *
 * --------------------------------------------------*/
SfxPoolItem*     SwBackgroundDestinationItem::Clone( SfxItemPool *pPool ) const
{
    return new SwBackgroundDestinationItem(Which(), GetValue());
}

/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.37  2000/09/18 16:06:19  willem.vandorp
    OpenOffice header added.

    Revision 1.36  2000/05/26 16:55:03  os
    includes removed

    Revision 1.35  2000/05/26 07:21:35  os
    old SW Basic API Slots removed

    Revision 1.34  2000/03/23 07:55:38  os
    UNO III

    Revision 1.33  2000/03/03 15:17:05  os
    StarView remainders removed

    Revision 1.32  1999/09/01 08:41:58  OS
    #66082# GetPresentation: new Parameter


      Rev 1.31   01 Sep 1999 10:41:58   OS
   #66082# GetPresentation: new Parameter

      Rev 1.30   23 Feb 1999 10:22:36   OS
   #61767# Kapitelnumerierung funktioniert wieder

      Rev 1.29   27 Jan 1999 10:00:32   OS
   #56371# TF_ONE51

      Rev 1.28   26 Jun 1998 17:35:36   OS
   SwBGDestItem mit Which #51751#

      Rev 1.27   17 Jun 1998 18:19:06   OS
   SwBackgroundDestinationItem

      Rev 1.26   27 May 1998 13:40:12   OS
   Put/QueryValue fuer SwUINumRuleItem

      Rev 1.25   24 Mar 1998 13:46:28   JP
   SwUINumRule: CTORen mit Name oder NumRule, nicht mehr mit NumRulePointer

      Rev 1.24   29 Nov 1997 15:08:58   MA
   includes

      Rev 1.23   21 Nov 1997 12:10:18   MA
   includes

      Rev 1.22   17 Nov 1997 10:21:32   JP
   Umstellung Numerierung

      Rev 1.21   03 Nov 1997 13:59:22   MA
   precomp entfernt

      Rev 1.20   06 Sep 1997 10:58:32   OS
   PenStyle der Fussnoten per Basic funktioniert #43272#

      Rev 1.19   11 Nov 1996 10:54:14   MA
   ResMgr

      Rev 1.18   28 Aug 1996 15:42:40   OS
   includes

      Rev 1.17   20 Aug 1996 16:24:56   OS
   SwPtrItem jetzt hier

      Rev 1.16   25 Mar 1996 13:31:50   OS
   Bugs im SwPageFtnInfoItem behoben

      Rev 1.15   31 Jan 1996 09:43:46   OS
   Masseinheit fuer SwPageFtnInfoItem

      Rev 1.14   04 Dec 1995 16:05:52   OS
   Set/FillVariable impl.

      Rev 1.13   27 Nov 1995 18:55:14   OS
   Umstellung 303a

      Rev 1.12   24 Nov 1995 16:57:24   OM
   PCH->PRECOMPILED

      Rev 1.11   16 Nov 1995 18:30:36   OS
   neu: Get/SetVariable, nicht impl.

      Rev 1.10   08 Nov 1995 12:17:22   OS
   Change => Set

      Rev 1.9   11 Oct 1995 17:50:54   MA
   fix: __EXPORT'iert

      Rev 1.8   28 Sep 1995 08:05:46   OS
   Typen fuer MSC richtig gecastet

------------------------------------------------------------------------*/


