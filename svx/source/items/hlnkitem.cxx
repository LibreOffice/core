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
#include <tools/stream.hxx>
#include <svl/memberid.hrc>
#include <basic/sbxvar.hxx>

#include "svx/hlnkitem.hxx"

// -----------------------------------------------------------------------

TYPEINIT1_FACTORY(SvxHyperlinkItem, SfxPoolItem, new SvxHyperlinkItem(0));

// class SvxHyperlinkItem ------------------------------------------------

#define HYPERLINKFF_MARKER  0x599401FE

SvStream& SvxHyperlinkItem::Store( SvStream& rStrm, sal_uInt16 /*nItemVersion*/ ) const
{
    // store 'simple' data
    // UNICODE: rStrm << sName;
    rStrm.WriteUniOrByteString(sName, rStrm.GetStreamCharSet());

    // UNICODE: rStrm << sURL;
    rStrm.WriteUniOrByteString(sURL, rStrm.GetStreamCharSet());

    // UNICODE: rStrm << sTarget;
    rStrm.WriteUniOrByteString(sTarget, rStrm.GetStreamCharSet());

    rStrm << (sal_uInt32) eType;

    // marker for versioninfo
    rStrm << (sal_uInt32) HYPERLINKFF_MARKER;

    // new data
    // UNICODE: rStrm << sIntName;
    rStrm.WriteUniOrByteString(sIntName, rStrm.GetStreamCharSet());

    // macro-events
    rStrm << nMacroEvents;

    // store macros
    sal_uInt16 nCnt = pMacroTable ? (sal_uInt16)pMacroTable->size() : 0;
    sal_uInt16 nMax = nCnt;
    if( nCnt )
    {
        for ( SvxMacroTable::const_iterator it = pMacroTable->begin();
              it != pMacroTable->end(); ++it)
            if( STARBASIC != it->second.GetScriptType() )
                --nCnt;
    }

    rStrm << nCnt;

    if( nCnt )
    {
        // 1. StarBasic-Macros
        for ( SvxMacroTable::const_iterator it = pMacroTable->begin();
              it != pMacroTable->end(); ++it)
        {
            const SvxMacro& rMac = it->second;
            if( STARBASIC == rMac.GetScriptType() )
            {
                rStrm << (sal_uInt16)it->first;

                // UNICODE: rStrm << pMac->GetLibName();
                rStrm.WriteUniOrByteString(rMac.GetLibName(), rStrm.GetStreamCharSet());

                // UNICODE: rStrm << pMac->GetMacName();
                rStrm.WriteUniOrByteString(rMac.GetMacName(), rStrm.GetStreamCharSet());
            }
        }
    }

    nCnt = nMax - nCnt;
    rStrm << nCnt;
    if( nCnt )
    {
        // 2. ::com::sun::star::script::JavaScript-Macros
        for ( SvxMacroTable::const_iterator it = pMacroTable->begin();
              it != pMacroTable->end(); ++it)
        {
            const SvxMacro& rMac = it->second;
            if( STARBASIC != rMac.GetScriptType() )
            {
                rStrm << (sal_uInt16)it->first;

                // UNICODE: rStrm << pMac->GetLibName();
                rStrm.WriteUniOrByteString(rMac.GetLibName(), rStrm.GetStreamCharSet());

                // UNICODE: rStrm << pMac->GetMacName();
                rStrm.WriteUniOrByteString(rMac.GetMacName(), rStrm.GetStreamCharSet());

                rStrm << (sal_uInt16)rMac.GetScriptType();
            }
        }
    }

    return rStrm;
}

SfxPoolItem*    SvxHyperlinkItem::Create( SvStream &rStrm, sal_uInt16 /*nItemVersion*/ ) const
{
    SvxHyperlinkItem* pNew = new SvxHyperlinkItem( Which() );
    sal_uInt32 nType;

    // simple data-types
    // UNICODE: rStrm >> pNew->sName;
    pNew->sName = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());

    // UNICODE: rStrm >> pNew->sURL;
    pNew->sURL = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());

    // UNICODE: rStrm >> pNew->sTarget;
    pNew->sTarget = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());

    rStrm >> nType;
    pNew->eType = (SvxLinkInsertMode) nType;

    sal_uInt32 nPos = rStrm.Tell();
    sal_uInt32 nMarker;
    rStrm >> nMarker;
    if ( nMarker == HYPERLINKFF_MARKER )
    {
        // new data
        // UNICODE: rStrm >> pNew->sIntName;
        pNew->sIntName = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());

        // macro-events
        rStrm >> pNew->nMacroEvents;

        // macros
        sal_uInt16 nCnt;
        rStrm >> nCnt;
        while( nCnt-- )
        {
            sal_uInt16 nCurKey;
            OUString aLibName, aMacName;

            rStrm >> nCurKey;
            // UNICODE: rStrm >> aLibName;
            aLibName = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());

            // UNICODE: rStrm >> aMacName;
            aMacName = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());

            pNew->SetMacro( nCurKey, SvxMacro( aMacName, aLibName, STARBASIC ) );
        }

        rStrm >> nCnt;
        while( nCnt-- )
        {
            sal_uInt16 nCurKey, nScriptType;
            OUString aLibName, aMacName;

            rStrm >> nCurKey;

            // UNICODE: rStrm >> aLibName;
            aLibName = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());

            // UNICODE: rStrm >> aMacName;
            aMacName = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());

            rStrm >> nScriptType;

            pNew->SetMacro( nCurKey, SvxMacro( aMacName, aLibName,
                                        (ScriptType)nScriptType ) );
        }
    }
    else
        rStrm.Seek( nPos );

    return pNew;
}

SvxHyperlinkItem::SvxHyperlinkItem( const SvxHyperlinkItem& rHyperlinkItem ):
            SfxPoolItem(rHyperlinkItem)
{
    sName   = rHyperlinkItem.sName;
    sURL    = rHyperlinkItem.sURL;
    sTarget = rHyperlinkItem.sTarget;
    eType   = rHyperlinkItem.eType;
    sIntName = rHyperlinkItem.sIntName;
    nMacroEvents = rHyperlinkItem.nMacroEvents;

    if( rHyperlinkItem.GetMacroTbl() )
        pMacroTable = new SvxMacroTableDtor( *rHyperlinkItem.GetMacroTbl() );
    else
        pMacroTable=NULL;

};

SvxHyperlinkItem::SvxHyperlinkItem( sal_uInt16 _nWhich, const OUString& rName, const OUString& rURL,
                                    const OUString& rTarget, const OUString& rIntName, SvxLinkInsertMode eTyp,
                                    sal_uInt16 nEvents, SvxMacroTableDtor *pMacroTbl ):
    SfxPoolItem (_nWhich),
    sName       (rName),
    sURL        (rURL),
    sTarget     (rTarget),
    eType       (eTyp),
    sIntName (rIntName),
    nMacroEvents (nEvents)
{
    if (pMacroTbl)
        pMacroTable = new SvxMacroTableDtor ( *pMacroTbl );
    else
        pMacroTable=NULL;
}

SfxPoolItem* SvxHyperlinkItem::Clone( SfxItemPool* ) const
{
    return new SvxHyperlinkItem( *this );
}

int SvxHyperlinkItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unterschiedliche Typen" );

    const SvxHyperlinkItem& rItem = (const SvxHyperlinkItem&) rAttr;

    bool bRet = ( sName   == rItem.sName   &&
                  sURL    == rItem.sURL    &&
                  sTarget == rItem.sTarget &&
                  eType   == rItem.eType   &&
                  sIntName == rItem.sIntName &&
                  nMacroEvents == rItem.nMacroEvents);
    if (!bRet)
        return sal_False;

    const SvxMacroTableDtor* pOther = ((SvxHyperlinkItem&)rAttr).pMacroTable;
    if( !pMacroTable )
        return ( !pOther || pOther->empty() );
    if( !pOther )
        return pMacroTable->empty();

    const SvxMacroTableDtor& rOwn = *pMacroTable;
    const SvxMacroTableDtor& rOther = *pOther;

    return rOwn == rOther;
}

void SvxHyperlinkItem::SetMacro( sal_uInt16 nEvent, const SvxMacro& rMacro )
{
    if( nEvent < EVENT_SFX_START )
    {
        switch( nEvent )
        {
            case HYPERDLG_EVENT_MOUSEOVER_OBJECT:
                nEvent = SFX_EVENT_MOUSEOVER_OBJECT;
                break;
            case HYPERDLG_EVENT_MOUSECLICK_OBJECT:
                nEvent = SFX_EVENT_MOUSECLICK_OBJECT;
                break;
            case HYPERDLG_EVENT_MOUSEOUT_OBJECT:
                nEvent = SFX_EVENT_MOUSEOUT_OBJECT;
                break;
        }
    }

    if( !pMacroTable )
        pMacroTable = new SvxMacroTableDtor;

    pMacroTable->Insert( nEvent, rMacro);
}

void SvxHyperlinkItem::SetMacroTable( const SvxMacroTableDtor& rTbl )
{
    delete pMacroTable;

    pMacroTable = new SvxMacroTableDtor ( rTbl );
}

bool SvxHyperlinkItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
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
            rVal <<= (sal_Int32) eType;
        break;
        default:
            return false;
    }

    return true;
}

bool SvxHyperlinkItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    OUString aStr;
    sal_Int32 nVal = 0;
    switch(nMemberId)
    {
        case MID_HLINK_NAME   :
            if(!(rVal >>= aStr))
                return sal_False;
            sIntName = aStr;
        break;
        case MID_HLINK_TEXT   :
            if(!(rVal >>= aStr))
                return sal_False;
            sName = aStr;
        break;
        case MID_HLINK_URL:
            if(!(rVal >>= aStr))
                return sal_False;
            sURL = aStr.getStr();
        break;
        case MID_HLINK_TARGET:
            if(!(rVal >>= aStr))
                return sal_False;
            sTarget = aStr;
        break;
        case MID_HLINK_TYPE:
            if(!(rVal >>= nVal))
                return sal_False;
            eType = (SvxLinkInsertMode) (sal_uInt16) nVal;
        break;
        default:
            return false;
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
