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


/*************************************************************************

    zu W4WStkEntry.bCopied  .bClosed  .bNeverIntoDoc
    ------------------------------------------------

    bCopied: Attr war auf dem Parent-Stack offen ( bLocked ) und wurde
             deshalb beim Anlegen des aktuellen Stacks auch hier als
             offen angelegt.

             Sinn:  Bei HdFt, Fussnoten und APOs muessen offene Attr
                    in neuen Stack uebertragen werden.
                    Dies geschieht durch Aufruf des Copy-Construktors
                    von W4WCtrlStack.               ----


    bClosed: Waehrend ein Child-Stack aktiv war, wurde festgestellt,
             dass dieses Attr zu schliessen ist.

             Sinn:  In HdFt, Fussnoten und APOs koennen kopierte(!)
                    Attribute geschlossen werden (s.o. unter bCopied).
                    Dann wird beim Original-Attr das bClosed ueber
                    WCtrlStack::SetLockedAttrClosed() gesetzt, damit
                    NACH dem Einlesen des Bereichs (und dem Restaurieren
                    des Original-Stacks und von pCurPaM) das Original
                    durch WCtrlStack::SetEndForClosedEntries()
                    geschlossen wird.


    bNeverIntoDoc:  Attr liegt nur dashalb auf dem Stack, damit der
                    entsprechende Schliessen-Befehl einen passenden
                    Gegenpart findet, es soll jedoch NICHT ins
                    Dokument eingesetzt sondern bloss vom Stack
                    genommen werden.

             Sinn:  Werden Attr definiert, die in IDENTISCHER Form
                    bereits im derzeit gueltigen STYLE definiert sind,
                    legen wir sie zwar hilfsweise auf den Stack,
                    vermeiden jedoch, dass sie aktiv werden, um nicht
                    ein HARTES Attribut zu setzen, wo der gewuenschte
                    Effekt ja bereits durch den Style sichergestellt ist.
                    Gesetzt wird das Flag in WCtrlStack::NewAttr()
                    und ausgewertet dann in  WCtrlStack::SetAttr().


    zu WCtrlStack::SetAttr()
    ------------------------

    bDoNotSetInDoc: ist dies TRUE, dann das Attr noch nicht
                    ins Dokument setzen, sondern auf dem Stack lassen.

             Sinn:  Falls wir den pCurPaM manuell zurueck gesetzt haben,
                    z.B.: mit pCurPaM->Move( fnMoveBackward, fnGoCntnt ),
                    liefert rPos.nNode.GetIndex() einen Wert, der dazu
                    fueher wuerde, das das Attr ins Doc gesetzt wird.
                    Um dies zu verhindern, setzen wir dieses Hilfs-Flag.

*************************************************************************/


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <hintids.hxx>

#include <bf_svx/adjitem.hxx>


#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <paratr.hxx>                   // GetAdjust()
#include <w4wstk.hxx>           // W4WStkEntry, W4WStack
#include <w4wpar.hxx>                   // SwW4WParser


namespace binfilter {

SV_IMPL_PTRARR( W4WCtrlStkEntries,W4WStkEntryPtr )


static SwCntntNode* GetCntntNode( SwNodeIndex& rIdx, BOOL bNext )
{
    SwCntntNode* pCNd = rIdx.GetNode().GetCntntNode();
    if( !pCNd && 0 == ( pCNd = bNext ? rIdx.GetNodes().GoNext( &rIdx )
                     : rIdx.GetNodes().GoPrevious( &rIdx ) ))
    {
    pCNd = bNext ? rIdx.GetNodes().GoPrevious( &rIdx )
             : rIdx.GetNodes().GoNext( &rIdx );
        ASSERT( pCNd, "kein ContentNode gefunden" );
    }
    return pCNd;
}

// ------ Stack-Eintrag fuer die gesamten - Attribute vom Text -----------


W4WStkEntry::W4WStkEntry( const SwPosition & rStartPos,
                                  SfxPoolItem * pHt,
                                  BOOL bInitCopied,
                                  BOOL bInitNID )
    : nMkNode( rStartPos.nNode, -1 ), nPtNode( nMkNode )
{
    // Anfang vom Bereich merken
    nMkCntnt = rStartPos.nContent.GetIndex();
    pAttr = pHt;            // speicher eine Kopie vom Attribut
    bLocked = TRUE;         // locke das Attribut --> darf erst
    bCopied = bInitCopied;  // gesetzt werden, wenn es wieder geunlocked ist
    bClosed = FALSE;
    bNeverIntoDoc = bInitNID;
}


W4WStkEntry::~W4WStkEntry()
{
    // Attribut kam zwar als Pointer, wird aber hier geloescht
    if( pAttr )
        delete pAttr;
}


void W4WStkEntry::SetEndPos( const SwPosition & rEndPos )
{
    /*
     * Attribut freigeben und das Ende merken.
     * Alles mit USHORT's, weil sonst beim Einfuegen von neuem Text an der
     * Cursor-Position auch der Bereich vom Attribut weiter
     * verschoben wird.
     * Das ist aber nicht das gewollte!
     */
    bLocked = FALSE;                                        // freigeben und das ENDE merken
    nPtNode = rEndPos.nNode.GetIndex()-1;
    nPtCntnt = rEndPos.nContent.GetIndex();
}

BOOL W4WStkEntry::MakeRegion( SwPaM& rRegion ) const
{
    // wird ueberhaupt ein Bereich umspannt ??
    // - ist kein Bereich, dann nicht returnen wenn am Anfang vom Absatz
    // - Felder aussortieren, koennen keinen Bereich haben !!
    if( nMkNode.GetIndex() == nPtNode.GetIndex() && nMkCntnt == nPtCntnt &&
        nPtCntnt && RES_TXTATR_FIELD != pAttr->Which() )
        return FALSE;

    // !!! Die Content-Indizies beziehen sich immer auf den Node !!!
    rRegion.GetPoint()->nNode = nMkNode.GetIndex() + 1;
    SwCntntNode* pCNd = GetCntntNode( rRegion.GetPoint()->nNode, TRUE );
    rRegion.GetPoint()->nContent.Assign( pCNd, nMkCntnt );
    rRegion.SetMark();
    rRegion.GetPoint()->nNode = nPtNode.GetIndex() + 1;
    pCNd = GetCntntNode( rRegion.GetPoint()->nNode, FALSE );
    rRegion.GetPoint()->nContent.Assign( pCNd, nPtCntnt );
    return TRUE;
}



W4WCtrlStack::W4WCtrlStack( SwW4WParser& rPars )
    : W4WCtrlStkEntries( 10, 5 ), pParentStack( 0 ), pParser( &rPars )
{
}



// W4WCtrlStack Copy-Ctor ist dafuer da,
// bei HdFt, Fussnoten und APOs
// die Char- und Para-Attribute des Haupttextes zu uebernehmen

W4WCtrlStack::W4WCtrlStack( W4WCtrlStack& rCpy, const SwPosition& rPos )
    : W4WCtrlStkEntries( 10, 5 ),
      pParser( rCpy.pParser ),
      pParentStack( &rCpy )
{
    USHORT nCnt = rCpy.Count();

    W4WStkEntry* pEntry;

    for ( USHORT i=0; i < nCnt; i++ )
    {
        pEntry = rCpy[ i ];
        USHORT nWhich = pEntry->pAttr->Which();
        if( pEntry->bLocked
            && ( ( nWhich >= RES_CHRATR_BEGIN && nWhich < RES_CHRATR_END )
             || ( nWhich >= RES_PARATR_BEGIN && nWhich < RES_PARATR_END ) ) )
        {
            W4WStkEntry *pTmp =
                new W4WStkEntry( rPos, pEntry->pAttr->Clone(), TRUE );
            Insert( pTmp, Count() );
        }
    }
}


W4WCtrlStack::~W4WCtrlStack()
{
    ASSERT( !Count(), "noch Attribute auf dem Stack" );
}


// W4WCtrlStack::NewAttr() beendet alle gleichen Attribute,
// die vielleicht auf dem Stack liegen und legt neues Attribut an.
// Falls identisches Attr im gueltigen Style definiert ist,
// wird beim neuen Attr das Flag bNeverIntoDoc gesetzt.

void W4WCtrlStack::NewAttr( const SwPosition&  rPos,
                            const SfxPoolItem& rAttr )
{
    register USHORT nAttrId = rAttr.Which();        // Id ermitteln
    SetAttr( rPos, nAttrId );                       // altes Attr beenden

    SwTxtFmtColl* pAktColl;							// auf Stack nachsehen
    BOOL bFoundInStyle =
        (   (      ( POOLATTR_BEGIN <= nAttrId )
                && ( POOLATTR_END   >  nAttrId )
            )
            && ( 0     != ( pAktColl = pParser->GetAktColl() ) )
            && ( rAttr == pAktColl->GetAttr( nAttrId )         ) );

    W4WStkEntry *pTmp =
        new W4WStkEntry( rPos, rAttr.Clone(), 		// neues Attr anlegen
                        FALSE, bFoundInStyle );
    Insert( pTmp, Count() );
}


// W4WCtrlStack::StealAttr() loescht Attribute des angegebenen Typs vom Stack.
// Als nAttrId sind erlaubt: 0 fuer alle, oder ein spezieller Typ.
// Damit erscheinen sie nicht in der Doc-Struktur. Dabei werden nur die
// Attribute entfernt, die im selben Absatz wie pPos stehen.
// Sinn ist, z.B. bei doppelten Tabs (bei WW-Import staendig) keine Berge von
// ueberfluessigen Attributen entstehen zu lassen

void W4WCtrlStack::StealAttr( const SwPosition& rPos, USHORT nAttrId )
{
    USHORT nCnt = Count();
    if( nCnt )
    {
        const ULONG nNodeIndex = rPos.nNode.GetIndex();

        register W4WStkEntry* pEntry;

        while( --nCnt )
        {
            pEntry = /*(W4WStkEntry*)*/(*this)[ nCnt ];
            if(    ( nNodeIndex == pEntry->nPtNode.GetIndex()+1 )
                && (   !nAttrId
                    || ( nAttrId == pEntry->pAttr->Which() ) ) )
                // loesche aus dem Stack
                DeleteAndDestroy( nCnt );
        }
    }
}


// W4WCtrlStack::SetLockedAttrClosed() sucht auf dem nach Attributen des
// angegebenen Typs und setzt sie bClosed.
// Anschliessend rekurs. Aufruf auf dem Parent-Stack.
void W4WCtrlStack::SetLockedAttrClosed( USHORT nAttrId )
{
    ASSERT(    ( POOLATTR_BEGIN     <= nAttrId && POOLATTR_END > nAttrId )
            || ( RES_FLTRATTR_BEGIN <= nAttrId && RES_FLTRATTR_END > nAttrId ),
        "Attribut-Id ist ungueltig" );

    USHORT nCnt = Count();
    if( nCnt )
    {
        register W4WStkEntry* pEntry;

        while( --nCnt )
        {
            pEntry = GetObject( nCnt );
            /*
                nachschauen, ob Id gleich und Attr offen
            */
            if(    pEntry->bLocked
                && ( nAttrId == pEntry->pAttr->Which() ) )
            {
                pEntry->bClosed = TRUE;
                if( pEntry->bCopied && pParentStack )
                    pParentStack->SetLockedAttrClosed( nAttrId );
            }
        }
    }
}


// W4WCtrlStack::StealWWTabAttr wird gebraucht, um beim WW2-Import von Tabellen
// falsche Absatz-Adjust-Attribute wieder herauszufischen. Hierbei handelt es
// sich um harte ADJLEFT-Attribute, die noch offen sind. ( siehe auch w4wpar2 )

void W4WCtrlStack::StealWWTabAttr( const SwPosition& rPos )
{
    USHORT nCnt = Count();
    if( nCnt )
    {
        const ULONG nIdx = rPos.nNode.GetIndex();
        register W4WStkEntry* pEntry;

        while( --nCnt )
        {
            pEntry = /*(W4WStkEntry*)*/GetObject( nCnt );
            register SfxPoolItem* pAt = pEntry->pAttr;
            if ( pAt->Which() == RES_PARATR_ADJUST
                && pEntry->bLocked
                && ((SvxAdjustItem*)pAt)->GetAdjust() == SVX_ADJUST_LEFT
                && pEntry->nPtNode.GetIndex()+1 == nIdx )
                                                    //  selber Absatz
                DeleteAndDestroy( nCnt );               // loesche aus dem Stack
        }
    }
}


// Alle gelockten Attribute freigeben (unlocken) und das Ende setzen,
// alle anderen im Document setzen und wieder aus dem Stack loeschen
// Returned, ob das gesuchte Attribut / die gesuchten Attribute
// ueberhaupt auf dem Stack standen

BOOL W4WCtrlStack::SetAttr( const SwPosition& rPos, USHORT nAttrId,
                            BOOL bTstEnde, BOOL bDoNotSetInDoc )
{
    ASSERT( !nAttrId ||
        ( POOLATTR_BEGIN <= nAttrId && POOLATTR_END > nAttrId ) ||
        ( RES_FLTRATTR_BEGIN <= nAttrId && RES_FLTRATTR_END > nAttrId ),
        "Falsche Id fuers Attribut" )

    BOOL bFound = FALSE;
    USHORT nCnt = Count();

    if( !nCnt ) return FALSE;

    W4WStkEntry* pEntry;

    for ( USHORT i=0; i < nCnt; i++ )
    {
        pEntry = GetObject( i );
        if( pEntry->bLocked )
        {
            // setze das Ende vom Attribut
            if( !nAttrId || nAttrId == pEntry->pAttr->Which() )
            {
                if( nAttrId && pEntry->bCopied && pParentStack )
                    pParentStack->SetLockedAttrClosed( nAttrId );
        pEntry->SetEndPos( rPos );
                bFound = TRUE;
            }
            continue;
        }

        // ist die Endposition die Cursor-Position, dann noch nicht
        // ins Dokument setzen, es muss noch Text folgen;
        // ausser am Dokumentende. (Attribut-Expandierung !!)
        if(     bTstEnde
            && (   bDoNotSetInDoc
                || ( pEntry->nPtNode.GetIndex()+1 == rPos.nNode.GetIndex())))
            continue;

        if( !pEntry->bNeverIntoDoc )
        {
            SwPaM aTmpCrsr( rPos );
            SetAttrInDoc( aTmpCrsr, *pEntry );
        }
        DeleteAndDestroy( i );  // loesche diesen Entry vom Stack,
        i--; nCnt--;            // danach rutschen alle folgenden nach unten
    }
    return bFound;
}


void W4WCtrlStack::SetEndForClosedEntries( const SwPosition& rPos )
{
    USHORT nCnt = Count();

    if( !nCnt ) return;

    W4WStkEntry* pEntry;

    for( USHORT i=0; i < nCnt; i++ )
    {
        pEntry = GetObject( i );
        if( pEntry->bLocked && pEntry->bClosed )
            /*
                setze das Ende vom Attribut
            */
            pEntry->SetEndPos( rPos );
    }
}


void W4WCtrlStack::SetAttrInDoc( SwPaM& rRegion, const W4WStkEntry& rEntry )
{
    if( rEntry.bNeverIntoDoc ) return;      // nur eine Sicherheitsabfrage ;-)

    SwDoc* pDoc = rRegion.GetDoc();

    switch( rEntry.pAttr->Which() )
    {
    case RES_FLTR_ANCHOR:
        {
            // der Anker ist der Point vom Pam. Dieser wird beim Einfugen
            // von Text usw. veraendert; darum wird er auf dem Stack
            // gespeichert Stack. Das Attribut muss nur noch im Format
            // gesetzt werden.
            rRegion.DeleteMark();
        rRegion.GetPoint()->nNode = rEntry.nMkNode.GetIndex() + 1;
            SwCntntNode* pCNd = GetCntntNode( rRegion.GetPoint()->nNode, TRUE );
        rRegion.GetPoint()->nContent.Assign( pCNd, rEntry.nMkCntnt );

        SwFrmFmt* pFmt = ((SwW4WAnchor*)rEntry.pAttr)->GetFlyFmt();
            SwFmtAnchor aAnchor( pFmt->GetAnchor() );
        aAnchor.SetAnchor( rRegion.GetPoint() );
            pFmt->SetAttr( aAnchor );
        }
        break;

    case RES_FLTR_STYLESHEET:
        {
        rRegion.DeleteMark();
        rRegion.GetPoint()->nNode = rEntry.nMkNode.GetIndex() + 1;
            SwCntntNode* pCNd = GetCntntNode( rRegion.GetPoint()->nNode, TRUE );
        rRegion.GetPoint()->nContent.Assign( pCNd, rEntry.nMkCntnt );
        rRegion.SetMark();
        rRegion.GetPoint()->nNode = rEntry.nPtNode.GetIndex() + 1;
            pCNd = GetCntntNode( rRegion.GetPoint()->nNode, FALSE );
        rRegion.GetPoint()->nContent.Assign( pCNd, rEntry.nPtCntnt );

        USHORT nId = ((SwW4WStyle*)rEntry.pAttr)->GetStyleId();

            USHORT nOldId = pParser->GetAktCollId();                        // Id vom aktuellen StyleSheet
        pParser->SetAktCollId( nId );
            SwTxtFmtColl* pTFC = pParser->GetAktColl();
        pDoc->SetTxtFmtColl( rRegion, pTFC, FALSE );
            pParser->SetAktCollId( nOldId );                // restore It !!!
        }
        break;
    case RES_TXTATR_FIELD:
        break;
/*
// JP 14.01.97: sollte auch ueber die normle Attribut-SS gehen!
    case RES_TXTATR_TOXMARK:
        {
            USHORT nLen;
            if ( rRegion.GetPoint()->nNode == rRegion.GetMark()->nNode ){
                                                                    // Anf u. Ende im selben Absatz ?
                nLen = rRegion.GetPoint()->nContent.GetIndex()
                      - rRegion.GetMark()->nContent.GetIndex();
            }else{                                                      // nein
        nLen = pDoc->GetNodes()[rRegion.GetMark()->nNode]
                        ->GetTxtNode()->GetTxt().Len()                          // nur bis zum Ende des Absatzes
                      - rRegion.GetMark()->nContent.GetIndex();
            }
            if ( nLen )
                pDoc->Insert( *rRegion.GetPoint(), nLen,
                                *(SwTOXMark*)rEntry.pAttr );
        }
        break;
*/
    default:
        if( rEntry.MakeRegion( rRegion ) )
            pDoc->Insert( rRegion, *rEntry.pAttr );
        break;
    }
}


SfxPoolItem* W4WCtrlStack::GetFmtStkAttr( USHORT nWhich, USHORT * pPos )
{
    W4WStkEntry* pEntry;
    USHORT nSize = Count();

    while( nSize )
    {
        // ist es das gesuchte Attribut ? (gueltig sind nur gelockte,
        // also akt. gesetzte Attribute!!)
        if( ( pEntry = (*this)[ --nSize ] )->bLocked &&
            pEntry->pAttr->Which() == nWhich )
        {
            if( pPos )
                *pPos = nSize;
            return (SfxPoolItem*)pEntry->pAttr;             // Ok, dann Ende
        }
    }
    return 0;
}


const SfxPoolItem* W4WCtrlStack::GetFmtAttr( const SwPaM& rPaM, USHORT nWhich )
{
    SfxPoolItem* pHt = GetFmtStkAttr( nWhich );
    if( pHt )
        return (const SfxPoolItem*)pHt;

    // im Stack ist das Attribut nicht vorhanden, also befrage das Dokument
    SwCntntNode * pNd = rPaM.GetCntntNode();
    if( !pNd )                      // kein ContentNode, dann das dflt. Attribut
        return &rPaM.GetDoc()->GetAttrPool().GetDefaultItem( nWhich );
    return &pNd->GetAttr( nWhich );
}


BOOL W4WCtrlStack::IsAttrOpen( USHORT nAttrId )
{
    USHORT nCnt = Count();

    if( !nCnt )     return FALSE;

    W4WStkEntry* pEntry;

    for ( USHORT i=0; i < nCnt; i++ )
    {
        pEntry = GetObject( i );
        if( pEntry->bLocked && nAttrId == pEntry->pAttr->Which() )
            return TRUE;
    }
    return FALSE;
}


// weitere Teile aus ehemaligem swrtf.hxx

SwW4WStyle::SwW4WStyle( USHORT nId )
    : SfxPoolItem( RES_FLTR_STYLESHEET ), nStyleId( nId )
{}


SwW4WStyle::SwW4WStyle( const SwW4WStyle & rRTFFld )
    : SfxPoolItem( RES_FLTR_STYLESHEET ), nStyleId( rRTFFld.nStyleId ) {}


SfxPoolItem* SwW4WStyle::Clone( SfxItemPool* ) const
{   return new SwW4WStyle( *this ); }


int SwW4WStyle::operator==( const SfxPoolItem& rHt ) const
{   return nStyleId == ((const SwW4WStyle&)rHt).nStyleId; }

//------ hier steht die Klasse fuer das SwW4WAnchor -----------

SwW4WAnchor::SwW4WAnchor( SwFlyFrmFmt* pFmt )
    : SfxPoolItem( RES_FLTR_ANCHOR ), pFlyFmt( pFmt )
{}

SwW4WAnchor::SwW4WAnchor( const SwW4WAnchor& rCpy )
    : SfxPoolItem( RES_FLTR_ANCHOR ), pFlyFmt( rCpy.pFlyFmt )
{}

int SwW4WAnchor::operator==( const SfxPoolItem& rItem ) const
{
    return pFlyFmt == ((SwW4WAnchor&)rItem).pFlyFmt;
}

SfxPoolItem* SwW4WAnchor::Clone( SfxItemPool* ) const
{
    return new SwW4WAnchor( *this );
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
