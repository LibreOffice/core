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

#include <comphelper/servicehelper.hxx>
#include <editeng/itemtype.hxx>
#include <tools/UnitConversion.hxx>
#include <unosett.hxx>

#include <swtypes.hxx>
#include <cmdid.h>
#include <uiitems.hxx>

#include <strings.hrc>
#include <unomid.h>
#include <numrule.hxx>

#include <editeng/eerdll.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

SwPageFootnoteInfoItem::SwPageFootnoteInfoItem( SwPageFootnoteInfo const & rInfo) :
    SfxPoolItem( FN_PARAM_FTN_INFO ),
    m_aFootnoteInfo(rInfo)
{
}

SwPageFootnoteInfoItem::~SwPageFootnoteInfoItem()
{
}

SwPageFootnoteInfoItem* SwPageFootnoteInfoItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SwPageFootnoteInfoItem( *this );
}

bool SwPageFootnoteInfoItem::operator==( const SfxPoolItem& rAttr ) const
{
    return SfxPoolItem::operator==(rAttr)
        && m_aFootnoteInfo == static_cast<const SwPageFootnoteInfoItem&>(rAttr).m_aFootnoteInfo;
}

bool SwPageFootnoteInfoItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             eCoreUnit,
    MapUnit             ePresUnit,
    OUString&           rText,
    const IntlWrapper&  rIntl
)   const
{
    const SwTwips nHght = GetPageFootnoteInfo().GetHeight();
    if ( nHght )
    {
        rText = SwResId( STR_MAX_FTN_HEIGHT ) + " " +
                ::GetMetricText( nHght, eCoreUnit, ePresUnit, &rIntl ) + " " +
                EditResId( ::GetMetricId( ePresUnit ) );
    }
    return true;
}

bool SwPageFootnoteInfoItem::QueryValue( Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bRet = true;
    switch(nMemberId & ~CONVERT_TWIPS)
    {
        case MID_FTN_HEIGHT        :     rVal <<= static_cast<sal_Int32>(convertTwipToMm100(m_aFootnoteInfo.GetHeight()));break;
        case MID_LINE_WEIGHT       :     rVal <<= static_cast<sal_Int16>(convertTwipToMm100(m_aFootnoteInfo.GetLineWidth()));break;
        case MID_LINE_COLOR        :     rVal <<= m_aFootnoteInfo.GetLineColor();break;
        case MID_LINE_RELWIDTH     :
        {
            Fraction aTmp( 100, 1 );
            aTmp *= m_aFootnoteInfo.GetWidth();
            rVal <<= static_cast<sal_Int8>(static_cast<tools::Long>(aTmp));
        }
        break;
        case MID_LINE_ADJUST       :     rVal <<= static_cast<sal_Int16>(m_aFootnoteInfo.GetAdj());break;//text::HorizontalAdjust
        case MID_LINE_TEXT_DIST    :     rVal <<= static_cast<sal_Int32>(convertTwipToMm100(m_aFootnoteInfo.GetTopDist()));break;
        case MID_LINE_FOOTNOTE_DIST:     rVal <<= static_cast<sal_Int32>(convertTwipToMm100(m_aFootnoteInfo.GetBottomDist()));break;
        case MID_FTN_LINE_STYLE    :
        {
            switch ( m_aFootnoteInfo.GetLineStyle( ) )
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
    Color aColor;
    bool bRet = true;
    switch(nMemberId  & ~CONVERT_TWIPS)
    {
        case MID_LINE_COLOR        :
            rVal >>= aColor;
            m_aFootnoteInfo.SetLineColor(aColor);
        break;
        case MID_FTN_HEIGHT:
        case MID_LINE_TEXT_DIST    :
        case MID_LINE_FOOTNOTE_DIST:
                rVal >>= nSet32;
                if(nSet32 < 0)
                    bRet = false;
                else
                {
                    nSet32 = o3tl::toTwips(nSet32, o3tl::Length::mm100);
                    switch(nMemberId & ~CONVERT_TWIPS)
                    {
                        case MID_FTN_HEIGHT:            m_aFootnoteInfo.SetHeight(nSet32);    break;
                        case MID_LINE_TEXT_DIST:        m_aFootnoteInfo.SetTopDist(nSet32);break;
                        case MID_LINE_FOOTNOTE_DIST:    m_aFootnoteInfo.SetBottomDist(nSet32);break;
                    }
                }
        break;
        case MID_LINE_WEIGHT       :
        {
            sal_Int16 nSet = 0;
            rVal >>= nSet;
            if(nSet >= 0)
                m_aFootnoteInfo.SetLineWidth(o3tl::toTwips(nSet, o3tl::Length::mm100));
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
                m_aFootnoteInfo.SetWidth(Fraction(nSet, 100));
        }
        break;
        case MID_LINE_ADJUST       :
        {
            sal_Int16 nSet = 0;
            rVal >>= nSet;
            if(nSet >= 0 && nSet < 3) //text::HorizontalAdjust
                m_aFootnoteInfo.SetAdj(static_cast<css::text::HorizontalAdjust>(nSet));
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
            m_aFootnoteInfo.SetLineStyle( eStyle );
        }
        break;
        default:
            bRet = false;
    }
    return bRet;
}

SwPtrItem::SwPtrItem( const sal_uInt16 nId, void* pPtr ) :
    SfxPoolItem( nId ),
    m_pMisc(pPtr)
{
}

// Cloning

SwPtrItem* SwPtrItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SwPtrItem( *this );
}

bool SwPtrItem::operator==( const SfxPoolItem& rAttr ) const
{
    return SfxPoolItem::operator==(rAttr)
        && m_pMisc == static_cast<const SwPtrItem&>(rAttr).m_pMisc;
}

// SwUINumRuleItem for the NumTabPages of the FormatNumRule/Styleists

SwUINumRuleItem::SwUINumRuleItem( const SwNumRule& rRul )
    : SfxPoolItem( FN_PARAM_ACT_NUMBER )
    , m_pRule( new SwNumRule( rRul ) )
{
}

SwUINumRuleItem::SwUINumRuleItem( const SwUINumRuleItem& rItem )
    : SfxPoolItem( rItem ),
    m_pRule( new SwNumRule( *rItem.m_pRule ))
{
}

SwUINumRuleItem::~SwUINumRuleItem()
{
}

SwUINumRuleItem* SwUINumRuleItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SwUINumRuleItem( *this );
}

bool SwUINumRuleItem::operator==( const SfxPoolItem& rAttr ) const
{
    return SfxPoolItem::operator==(rAttr)
        && *m_pRule == *static_cast<const SwUINumRuleItem&>(rAttr).m_pRule;
}

bool SwUINumRuleItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    uno::Reference< container::XIndexReplace >xRules = new SwXNumberingRules(*m_pRule);
    rVal <<= xRules;
    return true;
}
bool SwUINumRuleItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    uno::Reference< container::XIndexReplace> xRulesRef;
    if(rVal >>= xRulesRef)
    {
        auto pSwXRules = dynamic_cast<SwXNumberingRules*>(xRulesRef.get());
        if(pSwXRules)
        {
            *m_pRule = *pSwXRules->GetNumRule();
        }
    }
    return true;
}

SwPaMItem::SwPaMItem( const sal_uInt16 nId, SwPaM* pPaM ) :
    SfxPoolItem( nId ),
    m_pPaM(pPaM)
{
}

SwPaMItem* SwPaMItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SwPaMItem( *this );
}

bool SwPaMItem::operator==( const SfxPoolItem& rAttr ) const
{
    return SfxPoolItem::operator==(rAttr)
        && m_pPaM ==  static_cast<const SwPaMItem&>(rAttr).m_pPaM;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
