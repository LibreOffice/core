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

#include <editeng/itemtype.hxx>
#include <unosett.hxx>

#include "swtypes.hxx"
#include "cmdid.h"
#include "uiitems.hxx"

#include "utlui.hrc"
#include "attrdesc.hrc"
#include <unomid.h>
#include <numrule.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

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

bool SwPageFtnInfoItem::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( Which() == rAttr.Which(), "no equal attributes" );
    return ( aFtnInfo == ((SwPageFtnInfoItem&)rAttr).GetPageFtnInfo());
}

SfxItemPresentation  SwPageFtnInfoItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    OUString&           rText,
    const IntlWrapper*  pIntl
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            sal_uInt16 nHght = (sal_uInt16) GetPageFtnInfo().GetHeight();
            if ( nHght )
            {
                rText = SW_RESSTR( STR_MAX_FTN_HEIGHT ) + " " +
                        ::GetMetricText( nHght, eCoreUnit, ePresUnit, pIntl ) + " " +
                        ::GetSvxString( ::GetMetricId( ePresUnit ) );
            }
            return ePres;
        }
        default:; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

bool SwPageFtnInfoItem::QueryValue( Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bRet = true;
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
        case MID_FTN_LINE_STYLE    :
        {
            switch ( aFtnInfo.GetLineStyle( ) )
            {
                default:
                case table::BorderLineStyle::NONE : rVal <<= sal_Int8(0); break;
                case table::BorderLineStyle::SOLID: rVal <<= sal_Int8(1); break;
                case table::BorderLineStyle::DOTTED: rVal <<= sal_Int8(2); break;
                case table::BorderLineStyle::DASHED: rVal <<= sal_Int8(3); break;
            }
            break;
        }
        default:
            bRet = false;
    }
    return bRet;
}

bool SwPageFtnInfoItem::PutValue(const Any& rVal, sal_uInt8 nMemberId)
{
    sal_Int32 nSet32 = 0;
    bool bRet = true;
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
                    bRet = false;
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
                bRet = false;
        }
        break;
        case MID_LINE_RELWIDTH     :
        {
            sal_Int8 nSet = 0;
            rVal >>= nSet;
            if(nSet < 0)
                bRet = false;
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
                bRet = false;
        }
        case MID_FTN_LINE_STYLE:
        {
            ::editeng::SvxBorderStyle eStyle = table::BorderLineStyle::NONE;
            sal_Int8 nSet = 0;
            rVal >>= nSet;
            switch ( nSet )
            {
                case 1: eStyle = table::BorderLineStyle::SOLID; break;
                case 2: eStyle = table::BorderLineStyle::DOTTED; break;
                case 3: eStyle = table::BorderLineStyle::DASHED; break;
                default: break;
            }
            aFtnInfo.SetLineStyle( eStyle );
        }
        break;
        default:
            bRet = false;
    }
    return bRet;
}

SwPtrItem::SwPtrItem( const sal_uInt16 nId, void* pPtr ) :
    SfxPoolItem( nId ),
    pMisc(pPtr)
{
}

// Copy constructor

SwPtrItem::SwPtrItem( const SwPtrItem& rItem ) : SfxPoolItem( rItem )
{
    pMisc = rItem.pMisc;
}

// Cloning

SfxPoolItem* SwPtrItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SwPtrItem( *this );
}

bool SwPtrItem::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==(rAttr), "unequal types" );
    const SwPtrItem& rItem = (SwPtrItem&)rAttr;
    return ( pMisc == rItem.pMisc );
}

// SwUINumRuleItem for the NumTabPages of the FormatNumRule/Styleists

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

bool SwUINumRuleItem::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==(rAttr), "unequal types" );
    return *pRule == *((SwUINumRuleItem&)rAttr).pRule;
}

bool SwUINumRuleItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    uno::Reference< container::XIndexReplace >xRules = new SwXNumberingRules(*pRule);
    rVal.setValue(&xRules, ::getCppuType((uno::Reference< container::XIndexReplace>*)0));
    return true;
}
bool SwUINumRuleItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
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
    return true;
}

SwBackgroundDestinationItem::SwBackgroundDestinationItem(sal_uInt16  _nWhich, sal_uInt16 nValue) :
    SfxUInt16Item(_nWhich, nValue)
{
}

SfxPoolItem*     SwBackgroundDestinationItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SwBackgroundDestinationItem(Which(), GetValue());
}

SwPaMItem::SwPaMItem( const sal_uInt16 nId, SwPaM* pPaM ) :
    SfxPoolItem( nId ),
    m_pPaM(pPaM)
{
}

SwPaMItem::SwPaMItem( const SwPaMItem& rItem ) : SfxPoolItem( rItem )
{
    m_pPaM = rItem.m_pPaM;
}

SfxPoolItem* SwPaMItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SwPaMItem( *this );
}

bool SwPaMItem::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==(rAttr), "unequal types" );
    const SwPaMItem& rItem = (SwPaMItem&)rAttr;
    return ( m_pPaM == rItem.m_pPaM );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
