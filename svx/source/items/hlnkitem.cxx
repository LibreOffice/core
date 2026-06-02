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

#include <svx/svxids.hrc>
#include <sfx2/event.hxx>
#include <svx/hlnkitem.hxx>
#include <utility>


SfxPoolItem* SvxHyperlinkItem::CreateDefault() { return new  SvxHyperlinkItem(TypedWhichId<SvxHyperlinkItem>(0));}

SvxHyperlinkItem::SvxHyperlinkItem( const SvxHyperlinkItem& rHyperlinkItem )
    : SfxPoolItem(rHyperlinkItem),
    m_sName(rHyperlinkItem.m_sName),
    m_sURL(rHyperlinkItem.m_sURL),
    m_sTarget(rHyperlinkItem.m_sTarget),
    m_eType(rHyperlinkItem.m_eType),
    m_sReplacementText(rHyperlinkItem.m_sReplacementText),
    m_showName(rHyperlinkItem.m_showName),
    m_showText(rHyperlinkItem.m_showText),
    m_sIntName(rHyperlinkItem.m_sIntName),
    m_nMacroEvents(rHyperlinkItem.m_nMacroEvents),
    m_bTextIsHint(rHyperlinkItem.m_bTextIsHint)
{
    if( rHyperlinkItem.GetMacroTable() )
        m_pMacroTable.reset( new SvxMacroTableDtor( *rHyperlinkItem.GetMacroTable() ) );

};

SvxHyperlinkItem* SvxHyperlinkItem::Clone( SfxItemPool* ) const
{
    return new SvxHyperlinkItem( *this );
}

bool SvxHyperlinkItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    const SvxHyperlinkItem& rItem = static_cast<const SvxHyperlinkItem&>(rAttr);

    bool bRet = ( m_sName   == rItem.m_sName   &&
                  m_sURL    == rItem.m_sURL    &&
                  m_sTarget == rItem.m_sTarget &&
                  m_eType   == rItem.m_eType   &&
                  m_sIntName == rItem.m_sIntName &&
                  m_nMacroEvents == rItem.m_nMacroEvents &&
                  m_sReplacementText == rItem.m_sReplacementText &&
                  m_showText == rItem.m_showText &&
                  m_showName == rItem.m_showName &&
                  m_bTextIsHint == rItem.m_bTextIsHint);
    if (!bRet)
        return false;

    const SvxMacroTableDtor* pOther = static_cast<const SvxHyperlinkItem&>(rAttr).m_pMacroTable.get();
    if( !m_pMacroTable )
        return ( !pOther || pOther->empty() );
    if( !pOther )
        return m_pMacroTable->empty();

    const SvxMacroTableDtor& rOwn = *m_pMacroTable;
    const SvxMacroTableDtor& rOther = *pOther;

    return rOwn == rOther;
}

void SvxHyperlinkItem::SetMacro( HyperDialogEvent nEvent, const SvxMacro& rMacro )
{
    SvMacroItemId nSfxEvent = SvMacroItemId::NONE;
    switch( nEvent )
    {
        case HyperDialogEvent::MouseOverObject:
            nSfxEvent = SvMacroItemId::OnMouseOver;
            break;
        case HyperDialogEvent::MouseClickObject:
            nSfxEvent = SvMacroItemId::OnClick;
            break;
        case HyperDialogEvent::MouseOutObject:
            nSfxEvent = SvMacroItemId::OnMouseOut;
            break;
        default: break;
    }

    if( !m_pMacroTable )
        m_pMacroTable.reset( new SvxMacroTableDtor );

    m_pMacroTable->Insert( nSfxEvent, rMacro);
}

void SvxHyperlinkItem::SetMacroTable( const SvxMacroTableDtor& rTbl )
{
    m_pMacroTable.reset( new SvxMacroTableDtor ( rTbl ) );
}

bool SvxHyperlinkItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
//    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_HLINK_NAME   :
            rVal <<= m_sIntName;
        break;
        case MID_HLINK_TEXT   :
            rVal <<= m_sName;
        break;
        case MID_HLINK_URL:
            rVal <<= m_sURL;
        break;
        case MID_HLINK_TARGET:
            rVal <<= m_sTarget;
        break;
        case MID_HLINK_TYPE:
            rVal <<= static_cast<sal_Int32>(m_eType);
        break;
        case MID_HLINK_REPLACEMENTTEXT:
            rVal <<= m_sReplacementText;
        break;
        case MID_HLINK_TEXTISHINT:
            rVal <<= m_bTextIsHint;
        break;
        default:
            return false;
    }

    return true;
}

bool SvxHyperlinkItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    OUString aStr;
    sal_Int32 nVal = 0;
    switch(nMemberId)
    {
        case MID_HLINK_NAME   :
            if(!(rVal >>= aStr))
                return false;
            m_sIntName = aStr;
        break;
        case MID_HLINK_TEXT   :
            if(!(rVal >>= aStr))
                return false;
            m_sName = aStr;
        break;
        case MID_HLINK_URL:
            if(!(rVal >>= aStr))
                return false;
            m_sURL = aStr;
        break;
        case MID_HLINK_TARGET:
            if(!(rVal >>= aStr))
                return false;
            m_sTarget = aStr;
        break;
        case MID_HLINK_TYPE:
            if(!(rVal >>= nVal))
                return false;
            m_eType = static_cast<SvxLinkInsertMode>(static_cast<sal_uInt16>(nVal));
        break;
        case MID_HLINK_REPLACEMENTTEXT:
            if(!(rVal >>= aStr))
                return false;
            m_sReplacementText = aStr;
        break;
        case MID_HLINK_TEXTISHINT:
        {
            bool bTextIsHint = false;
            if(!(rVal >>= bTextIsHint))
                return false;
            m_bTextIsHint = bTextIsHint;
        }
        break;
        // Currently no way to put showName or showValue; these are set by the shell
        default:
            return false;
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
