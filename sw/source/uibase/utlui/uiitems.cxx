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

#include "strings.hrc"
#include <unomid.h>
#include <numrule.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

SwPageFootnoteInfoItem::SwPageFootnoteInfoItem( SwPageFootnoteInfo& rInfo) :
    SfxPoolItem( FN_PARAM_FTN_INFO ),
    aFootnoteInfo(rInfo)
{
}

SwPageFootnoteInfoItem::SwPageFootnoteInfoItem( const SwPageFootnoteInfoItem& rItem ) :
    SfxPoolItem( rItem ),
    aFootnoteInfo(rItem.GetPageFootnoteInfo())
{
}

 SwPageFootnoteInfoItem::~SwPageFootnoteInfoItem()
{
}

SfxPoolItem*  SwPageFootnoteInfoItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SwPageFootnoteInfoItem( *this );
}

bool SwPageFootnoteInfoItem::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( Which() == rAttr.Which(), "no equal attributes" );
    return ( aFootnoteInfo == static_cast<const SwPageFootnoteInfoItem&>(rAttr).GetPageFootnoteInfo());
}

bool SwPageFootnoteInfoItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             eCoreUnit,
    MapUnit             ePresUnit,
    OUString&           rText,
    const IntlWrapper*  pIntl
)   const
{
    const SwTwips nHght = GetPageFootnoteInfo().GetHeight();
    if ( nHght )
    {
        rText = SwResId( STR_MAX_FTN_HEIGHT ) + " " +
                ::GetMetricText( nHght, eCoreUnit, ePresUnit, pIntl ) + " " +
                ::GetSvxString( ::GetMetricId( ePresUnit ) );
    }
    return true;
}

bool SwPageFootnoteInfoItem::QueryValue( Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bRet = true;
    switch(nMemberId & ~CONVERT_TWIPS)
    {
        case MID_FTN_HEIGHT        :     rVal <<= (sal_Int32)convertTwipToMm100(aFootnoteInfo.GetHeight());break;
        case MID_LINE_WEIGHT       :     rVal <<= (sal_Int16)convertTwipToMm100(aFootnoteInfo.GetLineWidth());break;
        case MID_LINE_COLOR        :     rVal <<= (sal_Int32)aFootnoteInfo.GetLineColor().GetColor();break;
        case MID_LINE_RELWIDTH     :
        {
            Fraction aTmp( 100, 1 );
            aTmp *= aFootnoteInfo.GetWidth();
            rVal <<= (sal_Int8)(long)aTmp;
        }
        break;
        case MID_LINE_ADJUST       :     rVal <<= (sal_Int16)aFootnoteInfo.GetAdj();break;//text::HorizontalAdjust
        case MID_LINE_TEXT_DIST    :     rVal <<= (sal_Int32)convertTwipToMm100(aFootnoteInfo.GetTopDist());break;
        case MID_LINE_FOOTNOTE_DIST:     rVal <<= (sal_Int32)convertTwipToMm100(aFootnoteInfo.GetBottomDist());break;
        case MID_FTN_LINE_STYLE    :
        {
            switch ( aFootnoteInfo.GetLineStyle( ) )
            {
                default:
                case SvxBorderLineStyle::NONE : rVal <<= sal_Int8(0); break;
                case SvxBorderLineStyle::SOLID: rVal <<= sal_Int8(1); break;
                case SvxBorderLineStyle::DOTTED: rVal <<= sal_Int8(2); break;
                case SvxBorderLineStyle::DASHED: rVal <<= sal_Int8(3); break;
            }
            break;
        }
        default:
            bRet = false;
    }
    return bRet;
}

bool SwPageFootnoteInfoItem::PutValue(const Any& rVal, sal_uInt8 nMemberId)
{
    sal_Int32 nSet32 = 0;
    bool bRet = true;
    switch(nMemberId  & ~CONVERT_TWIPS)
    {
        case MID_LINE_COLOR        :
            rVal >>= nSet32;
            aFootnoteInfo.SetLineColor(nSet32);
        break;
        case MID_FTN_HEIGHT:
        case MID_LINE_TEXT_DIST    :
        case MID_LINE_FOOTNOTE_DIST:
                rVal >>= nSet32;
                if(nSet32 < 0)
                    bRet = false;
                else
                {
                    nSet32 = convertMm100ToTwip(nSet32);
                    switch(nMemberId & ~CONVERT_TWIPS)
                    {
                        case MID_FTN_HEIGHT:            aFootnoteInfo.SetHeight(nSet32);    break;
                        case MID_LINE_TEXT_DIST:        aFootnoteInfo.SetTopDist(nSet32);break;
                        case MID_LINE_FOOTNOTE_DIST:    aFootnoteInfo.SetBottomDist(nSet32);break;
                    }
                }
        break;
        case MID_LINE_WEIGHT       :
        {
            sal_Int16 nSet = 0;
            rVal >>= nSet;
            if(nSet >= 0)
                aFootnoteInfo.SetLineWidth(convertMm100ToTwip(nSet));
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
                aFootnoteInfo.SetWidth(Fraction(nSet, 100));
        }
        break;
        case MID_LINE_ADJUST       :
        {
            sal_Int16 nSet = 0;
            rVal >>= nSet;
            if(nSet >= 0 && nSet < 3) //text::HorizontalAdjust
                aFootnoteInfo.SetAdj((css::text::HorizontalAdjust)nSet);
            else
                bRet = false;
        }
        break;
        case MID_FTN_LINE_STYLE:
        {
            SvxBorderLineStyle eStyle = SvxBorderLineStyle::NONE;
            sal_Int8 nSet = 0;
            rVal >>= nSet;
            switch ( nSet )
            {
                case 1: eStyle = SvxBorderLineStyle::SOLID; break;
                case 2: eStyle = SvxBorderLineStyle::DOTTED; break;
                case 3: eStyle = SvxBorderLineStyle::DASHED; break;
                default: break;
            }
            aFootnoteInfo.SetLineStyle( eStyle );
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
    assert(SfxPoolItem::operator==(rAttr));
    const SwPtrItem& rItem = static_cast<const SwPtrItem&>(rAttr);
    return ( pMisc == rItem.pMisc );
}

// SwUINumRuleItem for the NumTabPages of the FormatNumRule/Styleists

SwUINumRuleItem::SwUINumRuleItem( const SwNumRule& rRul )
    : SfxPoolItem( FN_PARAM_ACT_NUMBER ), pRule( new SwNumRule( rRul ) )
{
}

SwUINumRuleItem::SwUINumRuleItem( const SwUINumRuleItem& rItem )
    : SfxPoolItem( rItem ),
    pRule( new SwNumRule( *rItem.pRule ))
{
}

SwUINumRuleItem::~SwUINumRuleItem()
{
}

SfxPoolItem*  SwUINumRuleItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SwUINumRuleItem( *this );
}

bool SwUINumRuleItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return *pRule == *static_cast<const SwUINumRuleItem&>(rAttr).pRule;
}

bool SwUINumRuleItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    uno::Reference< container::XIndexReplace >xRules = new SwXNumberingRules(*pRule);
    rVal <<= xRules;
    return true;
}
bool SwUINumRuleItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    uno::Reference< container::XIndexReplace> xRulesRef;
    if(rVal >>= xRulesRef)
    {
        uno::Reference< lang::XUnoTunnel > xTunnel(xRulesRef, uno::UNO_QUERY);
        SwXNumberingRules* pSwXRules = xTunnel.is() ? reinterpret_cast<SwXNumberingRules*>(
                    xTunnel->getSomething(SwXNumberingRules::getUnoTunnelId())) : nullptr;
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
    assert(SfxPoolItem::operator==(rAttr));
    const SwPaMItem& rItem = static_cast<const SwPaMItem&>(rAttr);
    return ( m_pPaM == rItem.m_pPaM );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
