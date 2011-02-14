/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <editeng/itemtype.hxx>
#include <unosett.hxx>

#include "swtypes.hxx"
#include "cmdid.h"
#include "uiitems.hxx"

#include "utlui.hrc"
#include "attrdesc.hrc"
#ifndef _UNOMID_H
#include <unomid.h>
#endif
#include <numrule.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

// Breitenangaben der Fussnotenlinien, mit TabPage abstimmen
static const sal_uInt16 __FAR_DATA nFtnLines[] = {
    0,
    10,
    50,
    80,
    100,
    150
};

#define FTN_LINE_STYLE_COUNT 5


SwPageFtnInfoItem::SwPageFtnInfoItem( const sal_uInt16 nId, SwPageFtnInfo& rInfo) :
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


SfxPoolItem*  SwPageFtnInfoItem::Clone( SfxItemPool * /*pPool*/ ) const
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
            sal_uInt16 nHght = (sal_uInt16) GetPageFtnInfo().GetHeight();
            if ( nHght )
            {
                rText = SW_RESSTR( STR_MAX_FTN_HEIGHT );
                rText += ' ';
                rText += ::GetMetricText( nHght, eCoreUnit, ePresUnit, pIntl );
                rText += ::GetSvxString( ::GetMetricId( ePresUnit ) );
            }
            return ePres;
        }
        default:; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}
/* -----------------------------26.04.01 12:25--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwPageFtnInfoItem::QueryValue( Any& rVal, sal_uInt8 nMemberId ) const
{
    sal_Bool bRet = sal_True;
    switch(nMemberId & ~CONVERT_TWIPS)
    {
        case MID_FTN_HEIGHT        :     rVal <<= (sal_Int32)TWIP_TO_MM100(aFtnInfo.GetHeight());break;
        case MID_LINE_WEIGHT       :     rVal <<= (sal_Int16)TWIP_TO_MM100_UNSIGNED(aFtnInfo.GetLineWidth());break;
        case MID_LINE_COLOR        :     rVal <<= (sal_Int32)aFtnInfo.GetLineColor().GetColor();break;
        case MID_LINE_RELWIDTH     :
        {
            Fraction aTmp( 100, 1 );
            aTmp *= aFtnInfo.GetWidth();
            rVal <<= (sal_Int8)(long)aTmp;
        }
        break;
        case MID_LINE_ADJUST       :     rVal <<= (sal_Int16)aFtnInfo.GetAdj();break;//text::HorizontalAdjust
        case MID_LINE_TEXT_DIST    :     rVal <<= (sal_Int32)TWIP_TO_MM100(aFtnInfo.GetTopDist());break;
        case MID_LINE_FOOTNOTE_DIST:     rVal <<= (sal_Int32)TWIP_TO_MM100(aFtnInfo.GetBottomDist());break;
        default:
            bRet = sal_False;
    }
    return bRet;
}
/* -----------------------------26.04.01 12:26--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwPageFtnInfoItem::PutValue(const Any& rVal, sal_uInt8 nMemberId)
{
    sal_Int32 nSet32 = 0;
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
            sal_Int16 nSet = 0;
            rVal >>= nSet;
            if(nSet >= 0)
                aFtnInfo.SetLineWidth(MM100_TO_TWIP(nSet));
            else
                bRet = sal_False;
        }
        break;
        case MID_LINE_RELWIDTH     :
        {
            sal_Int8 nSet = 0;
            rVal >>= nSet;
            if(nSet < 0)
                bRet = sal_False;
            else
                aFtnInfo.SetWidth(Fraction(nSet, 100));
        }
        break;
        case MID_LINE_ADJUST       :
        {
            sal_Int16 nSet = 0;
            rVal >>= nSet;
            if(nSet >= 0 && nSet < 3) //text::HorizontalAdjust
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

SwPtrItem::SwPtrItem( const sal_uInt16 nId, void* pPtr ) :
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


SfxPoolItem* SwPtrItem::Clone( SfxItemPool * /*pPool*/ ) const
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
SwUINumRuleItem::SwUINumRuleItem( const SwNumRule& rRul, const sal_uInt16 nId )
    : SfxPoolItem( nId ), pRule( new SwNumRule( rRul ) )
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


SfxPoolItem*  SwUINumRuleItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SwUINumRuleItem( *this );
}

int  SwUINumRuleItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
    return *pRule == *((SwUINumRuleItem&)rAttr).pRule;
}

sal_Bool SwUINumRuleItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    uno::Reference< container::XIndexReplace >xRules = new SwXNumberingRules(*pRule);
    rVal.setValue(&xRules, ::getCppuType((uno::Reference< container::XIndexReplace>*)0));
    return sal_True;
}
sal_Bool SwUINumRuleItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    uno::Reference< container::XIndexReplace> xRulesRef;
    if(rVal >>= xRulesRef)
    {
        uno::Reference< lang::XUnoTunnel > xTunnel(xRulesRef, uno::UNO_QUERY);
        SwXNumberingRules* pSwXRules = xTunnel.is() ? reinterpret_cast<SwXNumberingRules*>(
                    xTunnel->getSomething(SwXNumberingRules::getUnoTunnelId())) : 0;
        if(pSwXRules)
        {
            *pRule = *pSwXRules->GetNumRule();
        }
    }
    return sal_True;
}
/* -----------------17.06.98 17:43-------------------
 *
 * --------------------------------------------------*/
SwBackgroundDestinationItem::SwBackgroundDestinationItem(sal_uInt16  _nWhich, sal_uInt16 nValue) :
    SfxUInt16Item(_nWhich, nValue)
{
}
/* -----------------17.06.98 17:44-------------------
 *
 * --------------------------------------------------*/
SfxPoolItem*     SwBackgroundDestinationItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SwBackgroundDestinationItem(Which(), GetValue());
}



