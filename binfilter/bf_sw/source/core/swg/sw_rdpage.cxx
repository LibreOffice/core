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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <horiornt.hxx>

#include "doc.hxx"

#include <fmtpdsc.hxx>
#include <fmthdft.hxx>

#include "rdswg.hxx"
#include "pagedesc.hxx"
#include "swgpar.hxx"       // SWGRD_XXX
#include <SwStyleNameMapper.hxx>
namespace binfilter {

//////////////////////////////////////////////////////////////////////////////

// Achtung: Jede Routine nimmt an, dass der Record-Header bereits eingelesen
// ist. Sie arbeitet den Record so weit ab wie moeglich und schliesst den
// Vorgang mit end() ab.

//////////////////////////////////////////////////////////////////////////////

// Hinzufuegen eines neuen PageDesc-Links

void SwSwgReader::AddPageDescLink( const String& rName, USHORT nOff )
{
    // Eventuell das Flag-Bit setzen,
    // dass Seitennummern vorkommen
    // Dies wird fuer aeltere Docs benoetigt
    if( nOff & 0x7FFF )
        aFile.nFlags |= SWGF_HAS_PGNUMS,
        pDoc->SetPageNums();
    PageDescLink* p = new PageDescLink( rName, nOff );
    p->pLink = pPageLinks;
    pPageLinks = p;

}

// Aufloesen aller Seitenbeschreibungs-Links

void SwSwgReader::ResolvePageDescLinks()
{
    while( pPageLinks )
    {
        PageDescLink* p = pPageLinks;
        pPageLinks = p->pLink;
        USHORT nArrLen = pDoc->GetPageDescCnt();
        USHORT i;
        for( i = 0; i < nArrLen; i++)
        {
            const SwPageDesc& rDesc = pDoc->GetPageDesc( i );
            if( rDesc.GetName() == String( *p ) )
                break;
        }
        // falls nicht da, Standardlayout nehmen
        if( i == nArrLen ) i = 0;
        // Bit 0x8000 im Offset besagt, dass bAuto zu setzen ist
        SwFmtPageDesc aAttr( &pDoc->GetPageDesc( i ) );
        aAttr.SetNumOffset( p->nOffset & 0x7FFF );
        if( p->cFmt )
            p->pFmt->SetAttr( aAttr );
        else
            p->pSet->Put( aAttr );
        delete p;
    }
}

// Suchen einer Seitenbeschreibung

SwPageDesc& SwSwgReader::FindPageDesc( USHORT nIdx )
{
    PageDescInfo* pDescs = &pLayIdx[ nIdx ];
    return pDoc->_GetPageDesc( pDescs->nActualIdx );

}

// Fussnoten-Info

void SwSwgReader::InPageFtnInfo( SwPageFtnInfo& rFtn )
{
    long nHeight, nTopDist, nBottomDist, nNum, nDenom;
    short nAdjust, nPenWidth;
    r >> nHeight
      >> nTopDist
      >> nBottomDist
      >> nAdjust
      >> nNum >> nDenom
      >> nPenWidth;
    Color aPenColor = InColor();
    rFtn.SetHeight( (SwTwips) nHeight );
    rFtn.SetTopDist( (SwTwips) nTopDist );
    rFtn.SetBottomDist( (SwTwips) nBottomDist );
    rFtn.SetAdj( (SwFtnAdj) nAdjust );
    Fraction f( nNum, nDenom );
    rFtn.SetWidth( f );
    rFtn.SetLineColor(aPenColor );
    rFtn.SetLineWidth(nPenWidth );
    r.next();
}

// Seiten-Deskriptor

#define FORCEREAD ( SWGRD_PAGEFMTS & SWGRD_FORCE )

void SwSwgReader::InPageDesc( short nIdx )
{
    long nextrec = r.getskip();
    short nFollow, nUsedOn;
    BYTE bHdrShare, bFtrShare, bLandscape = FALSE;
    USHORT nPoolId = IDX_NO_VALUE;
    sal_Char nNumType;
    String aName;

    if( !r ) return;

    aName = GetText( FALSE );
    if( ( aHdr.nVersion >= SWG_VER_COMPAT ) && ( r.next() != SWG_DATA ) )
    {
        Error(); return;
    }
    r >> nFollow >> nNumType >> nUsedOn >> bHdrShare >> bFtrShare;
    if( aHdr.nVersion >= SWG_VER_COMPAT )
    {
        BYTE cFlags;
        r >> cFlags;
        if( aHdr.nVersion >= SWG_VER_POOLIDS )
            r >> nPoolId;
        // Korrektur fuer aeltere Dokumente
        if( aHdr.nVersion <= SWG_VER_FRAMES3 )
        {
            nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, GET_POOLID_PAGEDESC );
            if( nPoolId == USHRT_MAX ) nPoolId = IDX_NO_VALUE;
        }
        else if( nPoolId < IDX_SPECIAL )
            // USER-Feld verkleinern
            nPoolId &= 0xBFFF;
        r.skip();
        bLandscape = BOOL( ( cFlags & 0x01 ) != 0 );
    }

    PageDescInfo* pDescs = pLayIdx;
    // Ist es eine Poolvorlage und ist diese evtl. bereits eingelesen?
    // Dann das Pool-ID loeschen
    if( nPoolId != IDX_NO_VALUE )
      for( USHORT i = 0; i < nLay; i++, pDescs++ )
      {
        if( pDescs->nPoolId == nPoolId )
        {
            nPoolId = IDX_NO_VALUE; break;
        }
      }
    pDescs = &pLayIdx[ nIdx ];
    SwPageDesc* pDesc = NULL;
    // Ist bei alten Dokumenten der Name ein Poolformatname?
    if( aHdr.nVersion < SWG_VER_COMPAT )
    {
        nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, GET_POOLID_PAGEDESC );
        if( nPoolId == USHRT_MAX ) nPoolId = IDX_NO_VALUE;
    }
    if( nPoolId != IDX_NO_VALUE )
    {
        // Es ist eine Pool-Beschreibung. Falls eingefuegt wird,
        // dann bitte diese Beschreibung verwenden und nix weiter machen
        if( ( nOptions & FORCEREAD ) != FORCEREAD ) {
            // Ist sie ueberhaupt da?
            USHORT nArrLen = pDoc->GetPageDescCnt();
            USHORT n;
            for( n = 0; n < nArrLen; n++ )
            {
                pDesc = &pDoc->_GetPageDesc( n );
                if( pDesc->GetPoolFmtId() == nPoolId )
                {
                    nIdx = n; break;
                }
            }
            if( n < nArrLen )
            {
                r.skip( nextrec ); r.next();
                pDescs->nPoolId = nPoolId;
                pDescs->nActualIdx = nIdx;
                pDescs->nFollow = nFollow;
                // Formate muessen nicht registriert werden,
                // da Frames nie geladen werden, wenn es sich um das
                // Einfuegen von Text handelt
                return;
            } else pDesc = NULL;
        } else {
            // Neues Dokument: PageDesc anfordern und Index finden
            pDesc = pDoc->GetPageDescFromPool( nPoolId );
            USHORT nArrLen = pDoc->GetPageDescCnt();
            USHORT n;
            for( n = 0; n < nArrLen; n++ )
            {
                if( &pDoc->GetPageDesc( n ) == pDesc )
                {
                    nIdx = n; break;
                }
            }
            ASSERT( n < nArrLen, "Pool-PageDesc nicht gefunden" );
        }
    } else {
        // Keine Poolvorlage:
        // Gibt es diese Beschreibung bereits?
        USHORT nArrLen = pDoc->GetPageDescCnt();
        for( USHORT n = 0; n < nArrLen; n++ )
        {
            SwPageDesc& rCur = pDoc->_GetPageDesc( n );
            if( rCur.GetName() == aName )
            {
                // Ueberplaetten von Vorlagen?
                // Dank eines Bugs von OT koennen Vorlagen mehrfach in
                // einem Doc vorkommen. Da Seitenvorlagen "by name"
                // referenziert werden, ist dies fuer den Textteil OK.
                // Nur die Layout-Infos koennen u.U. baden gehen.
                if( ( bNew && nIdx != 0 )
                  || ( ( nOptions & FORCEREAD ) != FORCEREAD ) )
                {
                    // Dann den Index eintragen und abbrechen
                    r.skip( nextrec ); r.next();
                    pDescs->nPoolId = nPoolId;
                    pDescs->nActualIdx = n;
                    pDescs->nFollow = nFollow;
                    // Formate muessen nicht registriert werden,
                    // da Frames nie geladen werden, wenn es sich um das
                    // Einfuegen von Text handelt
                    return;
                } else {
                    nIdx = n;
                    pDesc = &rCur;
                    break;
                }
            }
        }
    }
    if( !pDesc )
    {
        nIdx = pDoc->MakePageDesc( aName );
        pDesc = &pDoc->_GetPageDesc( nIdx );
    }
    pDescs->nPoolId = nPoolId;
    pDescs->nActualIdx = nIdx;
    pDescs->nFollow = nFollow;
    SvxNumberType aType;
    aType.SetNumberingType(nNumType);
    pDesc->SetNumType( aType );
//  Steckt im UseOn drin
//  pDesc->ChgHeaderShare( BOOL( bHdrShare ) );
//  pDesc->ChgFooterShare( BOOL( bFtrShare ) );
    pDesc->WriteUseOn( UseOnPage( nUsedOn ) );

    BOOL bDone = FALSE;
    pDesc->ResetAllMasterAttr();
    pDesc->ResetAllLeftAttr();
    r.next();
    // Header und Footer sind Shared-Formate
    nStatus |= SWGSTAT_SHAREDFMT;
    while( !bDone )
      switch( r.cur() ) {
        case SWG_FOOTINFO:
            // Vorsicht: SWG_FOOTINFO kann auch auf dem TopLevel
            // vorkommen!
            if( r.tell() > nextrec ) bDone = TRUE;
            else InPageFtnInfo( pDesc->GetFtnInfo() ); break;
        case SWG_MASTERFMT:
            InFormat( &pDesc->GetMaster() );
            RegisterFmt( pDesc->GetMaster() );
            break;
        case SWG_LEFTFMT:
            InFormat( &pDesc->GetLeft() );
            RegisterFmt( pDesc->GetLeft() );
            break;
        case SWG_JOBSETUP:
        case SWG_COMMENT:
            r.skipnext(); break;
        default:
            bDone = TRUE;
    }
    nStatus &= ~SWGSTAT_SHAREDFMT;

    // ST special: 1 Bit des Masterfmts wird fuer Landscape verwendet!
    // Dies auch nur bis Version 1.x!
    if( aHdr.nVersion < SWG_VER_COMPAT )
        bLandscape = BOOL( nFlagBits & 0x01 );
    pDesc->SetLandscape( bLandscape );
    if( nPoolId != IDX_NO_VALUE )
        pDesc->SetPoolFmtId( nPoolId );

    // Hdr/Ftr-Formate umsetzen:
    // Gleichzeitig muessen (bei aelteren Dateien) die vorher
    // eingelesenen Left-Hdr/Ftr-Formate neu registriert werden,
    // damit die Frame-Leseroutinen den rechten Bezug haben.
    // Die erste Frame-Version darf gar nicht mehr laden, da die Content-
    // Section des ueberbratenen Formats verschwindet!
    if( pDesc->IsHeaderShared() )
    {
        const SwFmtHeader& rFmtM = pDesc->GetMaster().GetHeader();
        const SwFmtHeader& rFmtL = pDesc->GetLeft().GetHeader();
        const SwFmt* pFmt1 = rFmtL.GetHeaderFmt();
        const SwFmt* pFmt2 = rFmtM.GetHeaderFmt();
        if( pFmt1 && pFmt2 )
            ReRegisterFmt( *pFmt1, *pFmt2 );
        pDesc->GetLeft().SetAttr( rFmtM );
    }
    if( pDesc->IsFooterShared() )
    {
        const SwFmtFooter& rFmtM = pDesc->GetMaster().GetFooter();
        const SwFmtFooter& rFmtL = pDesc->GetLeft().GetFooter();
        const SwFmt* pFmt1 = rFmtL.GetFooterFmt();
        const SwFmt* pFmt2 = rFmtM.GetFooterFmt();
        if( pFmt1 && pFmt2 )
            ReRegisterFmt( *pFmt1, *pFmt2 );
        pDesc->GetLeft().SetAttr( rFmtM );
    }
}


void SwSwgReader::InPageDescs()
{
    USHORT nDesc;
    r >> nDesc;
    r.next();
    if( nDesc > 0 )
    {
        nLay = nDesc;
        PageDescInfo* p =
        pLayIdx = new PageDescInfo[ nDesc ];
        // einlesen
        USHORT i;
        for( i = 0; i < nDesc && r.good(); i++, p++ )
        {
            p->nActualIdx = 0;
            p->nPoolId =
            p->nFollow = IDX_NO_VALUE;
        }
        for( i = 0; i < nDesc && r.good(); i++ )
            InPageDesc( i );
        // Follows aufloesen
        p = pLayIdx;
        for( i = 0; i < nDesc; i++, p++ )
        {
            if( p->nFollow != IDX_NO_VALUE )
            {
                SwPageDesc& rDesc = pDoc->_GetPageDesc( p->nActualIdx );
                rDesc.SetFollow
                ( &pDoc->GetPageDesc( LayoutIdx( p->nFollow ) ) );
            }
        }
    }
    else Error();
}



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
