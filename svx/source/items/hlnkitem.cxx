/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------
#define _SVX_HLNKITEM_CXX

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
    rStrm.WriteByteString(sName);

    // UNICODE: rStrm << sURL;
    rStrm.WriteByteString(sURL);

    // UNICODE: rStrm << sTarget;
    rStrm.WriteByteString(sTarget);

    rStrm << (sal_uInt32) eType;

    // marker for versioninfo
    rStrm << (sal_uInt32) HYPERLINKFF_MARKER;

    // new data
    // UNICODE: rStrm << sIntName;
    rStrm.WriteByteString(sIntName);

    // macro-events
    rStrm << nMacroEvents;

    // store macros
    sal_uInt16 nCnt = pMacroTable ? (sal_uInt16)pMacroTable->Count() : 0;
    sal_uInt16 nMax = nCnt;
    if( nCnt )
    {
        for( SvxMacro* pMac = pMacroTable->First(); pMac; pMac = pMacroTable->Next() )
            if( STARBASIC != pMac->GetScriptType() )
                --nCnt;
    }

    rStrm << nCnt;

    if( nCnt )
    {
        // 1. StarBasic-Macros
        for( SvxMacro* pMac = pMacroTable->First(); pMac; pMac = pMacroTable->Next() )
        {
            if( STARBASIC == pMac->GetScriptType() )
            {
                rStrm << (sal_uInt16)pMacroTable->GetCurKey();

                // UNICODE: rStrm << pMac->GetLibName();
                rStrm.WriteByteString(pMac->GetLibName());

                // UNICODE: rStrm << pMac->GetMacName();
                rStrm.WriteByteString(pMac->GetMacName());
            }
        }
    }

    nCnt = nMax - nCnt;
    rStrm << nCnt;
    if( nCnt )
    {
        // 2. ::com::sun::star::script::JavaScript-Macros
        for( SvxMacro* pMac = pMacroTable->First(); pMac; pMac = pMacroTable->Next() )
        {
            if( STARBASIC != pMac->GetScriptType() )
            {
                rStrm << (sal_uInt16)pMacroTable->GetCurKey();

                // UNICODE: rStrm << pMac->GetLibName();
                rStrm.WriteByteString(pMac->GetLibName());

                // UNICODE: rStrm << pMac->GetMacName();
                rStrm.WriteByteString(pMac->GetMacName());

                rStrm << (sal_uInt16)pMac->GetScriptType();
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
    rStrm.ReadByteString(pNew->sName);

    // UNICODE: rStrm >> pNew->sURL;
    rStrm.ReadByteString(pNew->sURL);

    // UNICODE: rStrm >> pNew->sTarget;
    rStrm.ReadByteString(pNew->sTarget);

    rStrm >> nType;
    pNew->eType = (SvxLinkInsertMode) nType;

    sal_uInt32 nPos = rStrm.Tell();
    sal_uInt32 nMarker;
    rStrm >> nMarker;
    if ( nMarker == HYPERLINKFF_MARKER )
    {
        // new data
        // UNICODE: rStrm >> pNew->sIntName;
        rStrm.ReadByteString(pNew->sIntName);

        // macro-events
        rStrm >> pNew->nMacroEvents;

        // macros
        sal_uInt16 nCnt;
        rStrm >> nCnt;
        while( nCnt-- )
        {
            sal_uInt16 nCurKey;
            String aLibName, aMacName;

            rStrm >> nCurKey;
            // UNICODE: rStrm >> aLibName;
            rStrm.ReadByteString(aLibName);

            // UNICODE: rStrm >> aMacName;
            rStrm.ReadByteString(aMacName);

            pNew->SetMacro( nCurKey, SvxMacro( aMacName, aLibName, STARBASIC ) );
        }

        rStrm >> nCnt;
        while( nCnt-- )
        {
            sal_uInt16 nCurKey, nScriptType;
            String aLibName, aMacName;

            rStrm >> nCurKey;

            // UNICODE: rStrm >> aLibName;
            rStrm.ReadByteString(aLibName);

            // UNICODE: rStrm >> aMacName;
            rStrm.ReadByteString(aMacName);

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

SvxHyperlinkItem::SvxHyperlinkItem( sal_uInt16 _nWhich, String& rName, String& rURL,
                                    String& rTarget, String& rIntName, SvxLinkInsertMode eTyp,
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

    sal_Bool bRet = ( sName   == rItem.sName   &&
                  sURL    == rItem.sURL    &&
                  sTarget == rItem.sTarget &&
                  eType   == rItem.eType   &&
                  sIntName == rItem.sIntName &&
                  nMacroEvents == rItem.nMacroEvents);
    if (!bRet)
        return sal_False;

    const SvxMacroTableDtor* pOther = ((SvxHyperlinkItem&)rAttr).pMacroTable;
    if( !pMacroTable )
        return ( !pOther || !pOther->Count() );
    if( !pOther )
        return 0 == pMacroTable->Count();

    const SvxMacroTableDtor& rOwn = *pMacroTable;
    const SvxMacroTableDtor& rOther = *pOther;

    // Anzahl unterschiedlich => auf jeden Fall ungleich
    if( rOwn.Count() != rOther.Count() )
        return sal_False;

    // einzeln vergleichen; wegen Performance ist die Reihenfolge wichtig
    for( sal_uInt16 nNo = 0; nNo < rOwn.Count(); ++nNo )
    {
        const SvxMacro *pOwnMac = rOwn.GetObject(nNo);
        const SvxMacro *pOtherMac = rOther.GetObject(nNo);
        if (    rOwn.GetKey(pOwnMac) != rOther.GetKey(pOtherMac)  ||
                pOwnMac->GetLibName() != pOtherMac->GetLibName() ||
                pOwnMac->GetMacName() != pOtherMac->GetMacName() )
            return sal_False;
    }

    return sal_True;
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

    SvxMacro *pOldMacro;
    if( 0 != ( pOldMacro = pMacroTable->Get( nEvent )) )
    {
        delete pOldMacro;
        pMacroTable->Replace( nEvent, new SvxMacro( rMacro ) );
    }
    else
        pMacroTable->Insert( nEvent, new SvxMacro( rMacro ) );
}

void SvxHyperlinkItem::SetMacroTable( const SvxMacroTableDtor& rTbl )
{
    if ( pMacroTable )
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
            rVal <<= ::rtl::OUString(sIntName.GetBuffer());
        break;
        case MID_HLINK_TEXT   :
            rVal <<= ::rtl::OUString(sName.GetBuffer());
        break;
        case MID_HLINK_URL:
            rVal <<= ::rtl::OUString(sURL.GetBuffer());
        break;
        case MID_HLINK_TARGET:
            rVal <<= ::rtl::OUString(sTarget.GetBuffer());
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
//    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    ::rtl::OUString aStr;
    sal_Int32 nVal = 0;
    switch(nMemberId)
    {
        case MID_HLINK_NAME   :
            if(!(rVal >>= aStr))
                return sal_False;
            sIntName = aStr.getStr();
        break;
        case MID_HLINK_TEXT   :
            if(!(rVal >>= aStr))
                return sal_False;
            sName = aStr.getStr();
        break;
        case MID_HLINK_URL:
            if(!(rVal >>= aStr))
                return sal_False;
            sURL = aStr.getStr();
        break;
        case MID_HLINK_TARGET:
            if(!(rVal >>= aStr))
                return sal_False;
            sTarget = aStr.getStr();
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
