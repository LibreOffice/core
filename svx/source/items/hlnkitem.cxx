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

SvxHyperlinkItem::SvxHyperlinkItem( const SvxHyperlinkItem& rHyperlinkItem ):
            SfxPoolItem(rHyperlinkItem)
{
    sName   = rHyperlinkItem.sName;
    sURL    = rHyperlinkItem.sURL;
    sTarget = rHyperlinkItem.sTarget;
    eType   = rHyperlinkItem.eType;
    sIntName = rHyperlinkItem.sIntName;
    nMacroEvents = rHyperlinkItem.nMacroEvents;
    sReplacementText = rHyperlinkItem.sReplacementText;

    if( rHyperlinkItem.GetMacroTable() )
        pMacroTable.reset( new SvxMacroTableDtor( *rHyperlinkItem.GetMacroTable() ) );

};

SvxHyperlinkItem::SvxHyperlinkItem( TypedWhichId<SvxHyperlinkItem> _nWhich, OUString aName, OUString aURL,
                                    OUString aTarget, OUString aIntName, SvxLinkInsertMode eTyp,
                                    HyperDialogEvent nEvents, SvxMacroTableDtor const *pMacroTbl, OUString aReplacementText):
    SfxPoolItem (_nWhich ),
    sName       (std::move(aName)),
    sURL        (std::move(aURL)),
    sTarget     (std::move(aTarget)),
    eType       (eTyp),
    sReplacementText (std::move(aReplacementText)),
    sIntName (std::move(aIntName)),
    nMacroEvents (nEvents)
{
    if (pMacroTbl)
        pMacroTable.reset( new SvxMacroTableDtor ( *pMacroTbl ) );
}

SvxHyperlinkItem* SvxHyperlinkItem::Clone( SfxItemPool* ) const
{
    return new SvxHyperlinkItem( *this );
}

bool SvxHyperlinkItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    const SvxHyperlinkItem& rItem = static_cast<const SvxHyperlinkItem&>(rAttr);

    bool bRet = ( sName   == rItem.sName   &&
                  sURL    == rItem.sURL    &&
                  sTarget == rItem.sTarget &&
                  eType   == rItem.eType   &&
                  sIntName == rItem.sIntName &&
                  nMacroEvents == rItem.nMacroEvents &&
                  sReplacementText == rItem.sReplacementText);
    if (!bRet)
        return false;

    const SvxMacroTableDtor* pOther = static_cast<const SvxHyperlinkItem&>(rAttr).pMacroTable.get();
    if( !pMacroTable )
        return ( !pOther || pOther->empty() );
    if( !pOther )
        return pMacroTable->empty();

    const SvxMacroTableDtor& rOwn = *pMacroTable;
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

    if( !pMacroTable )
        pMacroTable.reset( new SvxMacroTableDtor );

    pMacroTable->Insert( nSfxEvent, rMacro);
}

void SvxHyperlinkItem::SetMacroTable( const SvxMacroTableDtor& rTbl )
{
    pMacroTable.reset( new SvxMacroTableDtor ( rTbl ) );
}

bool SvxHyperlinkItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
//    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_HLINK_NAME   :
            rVal <<= sIntName;
        break;
        case MID_HLINK_TEXT   :
            rVal <<= sName;
        break;
        case MID_HLINK_URL:
            rVal <<= sURL;
        break;
        case MID_HLINK_TARGET:
            rVal <<= sTarget;
        break;
        case MID_HLINK_TYPE:
            rVal <<= static_cast<sal_Int32>(eType);
        break;
        case MID_HLINK_REPLACEMENTTEXT:
            rVal <<= sReplacementText;
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
            sIntName = aStr;
        break;
        case MID_HLINK_TEXT   :
            if(!(rVal >>= aStr))
                return false;
            sName = aStr;
        break;
        case MID_HLINK_URL:
            if(!(rVal >>= aStr))
                return false;
            sURL = aStr;
        break;
        case MID_HLINK_TARGET:
            if(!(rVal >>= aStr))
                return false;
            sTarget = aStr;
        break;
        case MID_HLINK_TYPE:
            if(!(rVal >>= nVal))
                return false;
            eType = static_cast<SvxLinkInsertMode>(static_cast<sal_uInt16>(nVal));
        break;
        case MID_HLINK_REPLACEMENTTEXT:
            if(!(rVal >>= aStr))
                return false;
            sReplacementText = aStr;
        break;
        default:
            return false;
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
