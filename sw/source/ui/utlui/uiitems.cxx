/*************************************************************************
 *
 *  $RCSfile: uiitems.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:57:14 $
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
#ifndef _UNOMID_H
#include <unomid.h>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

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
    const IntlWrapper*    pIntl
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
                rText += ::GetMetricText( nHght, eCoreUnit, ePresUnit, pIntl );
                rText += ::GetSvxString( ::GetMetricId( ePresUnit ) );
            }
            return ePres;
        }
    }
    return SFX_ITEM_PRESENTATION_NONE;
}
/* -----------------------------26.04.01 12:25--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwPageFtnInfoItem::QueryValue( Any& rVal, BYTE nMemberId ) const
{
    sal_Bool bRet = sal_True;
    switch(nMemberId & ~CONVERT_TWIPS)
    {
        case MID_FTN_HEIGHT        :     rVal <<= (sal_Int32)TWIP_TO_MM100(aFtnInfo.GetHeight());break;
        case MID_LINE_WEIGHT       :     rVal <<= (sal_Int16)TWIP_TO_MM100(aFtnInfo.GetLineWidth());break;
        case MID_LINE_COLOR        :     rVal <<= (sal_Int32)aFtnInfo.GetLineColor().GetColor();break;
        case MID_LINE_RELWIDTH     :
        {
            Fraction aTmp( 100, 1 );
            aTmp *= aFtnInfo.GetWidth();
            rVal <<= (sal_Int8)(long)aTmp;
        }
        break;
        case MID_LINE_ADJUST       :     rVal <<= (sal_Int16)aFtnInfo.GetAdj();break;//com::sun::star::text::HorizontalAdjust
        case MID_LINE_TEXT_DIST    :     rVal <<= (sal_Int32)TWIP_TO_MM100(aFtnInfo.GetTopDist());break;
        case MID_LINE_FOOTNOTE_DIST:     rVal <<= (sal_Int32)TWIP_TO_MM100(aFtnInfo.GetBottomDist());break;
        default:
            bRet = sal_False;
    }
    return bRet;
}
/* -----------------------------26.04.01 12:26--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwPageFtnInfoItem::PutValue(const Any& rVal, BYTE nMemberId)
{
    sal_Int32 nSet32;
    sal_Bool bRet = sal_True;
    switch(nMemberId  & ~CONVERT_TWIPS)
    {
        case MID_LINE_COLOR        :
            rVal >>= nSet32;
            aFtnInfo.SetLineColor(nSet32);
        break;
        case MID_FTN_HEIGHT:
        case MID_LINE_TEXT_DIST    :
        case MID_LINE_FOOTNOTE_DIST:
                rVal >>= nSet32;
                if(nSet32 < 0)
                    bRet = sal_False;
                else
                {
                    nSet32 = MM100_TO_TWIP(nSet32);
                    switch(nMemberId & ~CONVERT_TWIPS)
                    {
                        case MID_FTN_HEIGHT:            aFtnInfo.SetHeight(nSet32);    break;
                        case MID_LINE_TEXT_DIST:        aFtnInfo.SetTopDist(nSet32);break;
                        case MID_LINE_FOOTNOTE_DIST:    aFtnInfo.SetBottomDist(nSet32);break;
                    }
                }
        break;
        case MID_LINE_WEIGHT       :
        {
            sal_Int16 nSet; rVal >>= nSet;
            if(nSet >= 0)
                aFtnInfo.SetLineWidth(MM100_TO_TWIP(nSet));
            else
                bRet = sal_False;
        }
        break;
        case MID_LINE_RELWIDTH     :
        {
            sal_Int8 nSet; rVal >>= nSet;
            if(nSet < 0)
                bRet = sal_False;
            else
                aFtnInfo.SetWidth(Fraction(nSet, 100));
        }
        break;
        case MID_LINE_ADJUST       :
        {
            sal_Int16 nSet; rVal >>= nSet;
            if(nSet >= 0 && nSet < 3) //com::sun::star::text::HorizontalAdjust
                aFtnInfo.SetAdj((SwFtnAdj)nSet);
            else
                bRet = sal_False;
        }
        break;
        default:
            bRet = sal_False;
    }
    return bRet;
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
    uno::Reference< container::XIndexReplace> xRulesRef;
    if(rVal >>= xRulesRef)
    {
        uno::Reference< lang::XUnoTunnel > xTunnel(xRulesRef, uno::UNO_QUERY);
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



