/*************************************************************************
 *
 *  $RCSfile: fltshell.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include <ctype.h>
#include <stdio.h> // sprintf

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX //autogen
#include <svx/impgrf.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX //autogen
#include <svx/cmapitem.hxx>
#endif
#ifndef _SVX_ITEM_HXX //autogen
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTRFMRK_HXX //autogen
#include <fmtrfmrk.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>           // RES_SETEXPFLD
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>          // class SwFmtRefMark
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>          // class SwTableLines, ...
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>           // SwExpField
#endif
#ifndef _SECTION_HXX
#include <section.hxx>          // class SwSection
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>           // class SwSelBoxes
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>            // class SwDocSh
#endif
#ifndef _FLTSHELL_HXX
#include <fltshell.hxx>
#endif

#define MAX_FIELDLEN 64000

SV_IMPL_PTRARR(SwFltControlStackEntries, SwFltStackEntryPtr)

static SwCntntNode* GetCntntNode(SwDoc* pDoc, SwNodeIndex& rIdx, BOOL bNext)
{
    SwCntntNode* pCNd = pDoc->GetNodes()[ rIdx ]->GetCntntNode();
    if(!pCNd && 0 == (pCNd = bNext ? pDoc->GetNodes().GoNext(&rIdx)
                                     : pDoc->GetNodes().GoPrevious(&rIdx)))
    {
        pCNd = bNext ? pDoc->GetNodes().GoPrevious(&rIdx)
                     : pDoc->GetNodes().GoNext(&rIdx);
        ASSERT(pCNd, "kein ContentNode gefunden");
    }
    return pCNd;
}

// ------ Stack-Eintrag fuer die gesamten - Attribute vom Text -----------
SwFltStackEntry::SwFltStackEntry(const SwPosition& rStartPos, SfxPoolItem* pHt ) :
    nMkNode(rStartPos.nNode, -1),
    nPtNode(nMkNode)
{
    // Anfang vom Bereich merken
    nMkCntnt = rStartPos.nContent.GetIndex();
    pAttr = pHt;        // speicher eine Kopie vom Attribut
    bOld    = FALSE;    // used for marking Attributes *before* skipping field results
    bLocked = TRUE;     // locke das Attribut --> darf erst
    bCopied = FALSE;    // gesetzt werden, wenn es wieder geunlocked ist
}

SwFltStackEntry::SwFltStackEntry(const SwFltStackEntry& rEntry) :
    nMkNode(rEntry.nMkNode),
    nPtNode(rEntry.nPtNode)
{
    pAttr   = rEntry.pAttr->Clone();
    nMkCntnt= rEntry.nMkCntnt;
    bOld    = rEntry.bOld;
    bLocked = bCopied = TRUE; // when rEntry were NOT bLocked we would never have been called
}


SwFltStackEntry::~SwFltStackEntry()
{
    // Attribut kam zwar als Pointer, wird aber hier geloescht
    if (pAttr)
        delete pAttr;
}

void SwFltStackEntry::SetEndPos(const SwPosition& rEndPos)
{
    // Attribut freigeben und das Ende merken.
    // Alles mit USHORT's, weil sonst beim Einfuegen von neuem Text an der
    // Cursor-Position auch der Bereich vom Attribut weiter
    // verschoben wird.
    // Das ist aber nicht das gewollte!
    bLocked = FALSE;                    // freigeben und das ENDE merken
    nPtNode = rEndPos.nNode.GetIndex()-1;
    nPtCntnt = rEndPos.nContent.GetIndex();
}

BOOL SwFltStackEntry::MakeRegion(SwDoc* pDoc, SwPaM& rRegion, BOOL bCheck )
{
    // wird ueberhaupt ein Bereich umspannt ??
    // - ist kein Bereich, dann nicht returnen wenn am Anfang vom Absatz
    // - Felder aussortieren, koennen keinen Bereich haben !!
#ifdef DEBUG
    USHORT nWhich = pAttr->Which();
#endif
    if (nMkNode.GetIndex() == nPtNode.GetIndex()
     && nMkCntnt == nPtCntnt
     && nPtCntnt
     && RES_TXTATR_FIELD != pAttr->Which())
        return FALSE;

    // !!! Die Content-Indizies beziehen sich immer auf den Node !!!
    rRegion.GetPoint()->nNode = nMkNode.GetIndex() + 1;
    SwCntntNode* pCNd = GetCntntNode(pDoc, rRegion.GetPoint()->nNode, TRUE);
    rRegion.GetPoint()->nContent.Assign(pCNd, nMkCntnt);
    rRegion.SetMark();
    if( nMkNode != nPtNode )
    {
        rRegion.GetPoint()->nNode = nPtNode.GetIndex() + 1;
        pCNd = GetCntntNode(pDoc, rRegion.GetPoint()->nNode, FALSE);
    }
    rRegion.GetPoint()->nContent.Assign(pCNd, nPtCntnt);
#ifdef DEBUG
    ASSERT( CheckNodesRange( rRegion.Start()->nNode,
                             rRegion.End()->nNode, TRUE ),
             "Attribut oder AEhnliches ueber Bereichs-Grenzen" );
#endif
    if( bCheck )
        return CheckNodesRange( rRegion.Start()->nNode,
                                rRegion.End()->nNode, TRUE );
    else
        return TRUE;
}


SwFltControlStack::SwFltControlStack(SwDoc* pDo, ULONG nFieldFl) :
    SwFltControlStackEntries(10, 5),
    pDoc(pDo),
    nFieldFlags(nFieldFl),
    bIsEndStack(FALSE)
{
}


SwFltControlStack::~SwFltControlStack()
{
    ASSERT(!Count(), "noch Attribute auf dem Stack");
}

// MoveAttrs() ist fuer folgendes Problem:
// Wenn ueber den Stack ein Feld wie z.B. "Variable setzen" gesetzt wird,
// verschiebt sich der Text um ein \xff - Zeichen, und alle folgenden
// Attribute stimmen in ihrer Position nicht mehr.
// Dann muss MoveAttrs() nach dem Setzen des Attributes ins Doc gerufen werden,
// so dass alle Attribut-Positionen,
// die im selben Absatz weiter hinten stehen, um 1 Zeichen weiter
// nach rechts verschoben werden.
void SwFltControlStack::MoveAttrs( const SwPosition& rPos )
{
    USHORT nCnt = Count();
    SwFltStackEntry* pEntry;
    ULONG nPosNd = rPos.nNode.GetIndex();
    USHORT nPosCt = rPos.nContent.GetIndex() - 1;

    for (USHORT i=0; i < nCnt; i++){
        pEntry = (*this)[ i ];
        if(( pEntry->nMkNode.GetIndex() + 1 == nPosNd )
           &&( pEntry->nMkCntnt >= nPosCt )){
            pEntry->nMkCntnt++;
            ASSERT( pEntry->nMkCntnt
                <= pDoc->GetNodes()[nPosNd]->GetCntntNode()->Len(),
                    "Attribut-Anfang hinter Zeilenende" );
        }
        if(( pEntry->nPtNode.GetIndex() + 1 == nPosNd )
           &&( pEntry->nPtCntnt >= nPosCt )){
            pEntry->nPtCntnt++;
            ASSERT( pEntry->nPtCntnt
                <= pDoc->GetNodes()[nPosNd]->GetCntntNode()->Len(),
                    "Attribut-Ende hinter Zeilenende" );
        }
    }
}



// all attributes that are on rPos will but changed so that their Point or their
// Mark (whichever had been on rPos) will be set to the next node
void SwFltControlStack::MoveAttrsToNextNode( const SwNodeIndex& rNdIdx )
{
    USHORT nCnt = Count();
    SwFltStackEntry* pEntry;

    for (USHORT i=0; i < nCnt; i++)
    {
        pEntry = (*this)[ i ];
        if(    ( pEntry->nMkNode  == rNdIdx ) )
            pEntry->nMkNode++;

        if(    ( pEntry->nPtNode  == rNdIdx ) )
            pEntry->nPtNode++;
    }
}


void SwFltControlStack::MarkAllAttrsOld()
{
    USHORT nCnt = Count();
    for (USHORT i=0; i < nCnt; i++)
        (*this)[ i ]->bOld = TRUE;
}


void SwFltControlStack::NewAttr(const SwPosition& rPos, const SfxPoolItem & rAttr )
{
    SwFltStackEntry *pTmp = new SwFltStackEntry(rPos, rAttr.Clone() );
    register USHORT nWhich = pTmp->pAttr->Which();
    SetAttr(rPos, nWhich);// Ende von evtl. gleichen Attributen auf dem Stack
                                // Setzen, damit sich die Attribute nicht auf
                                // dem Stack haeufen
    Insert(pTmp, Count());
}

// SwFltControlStack::StealAttr() loescht Attribute des angegebenen Typs vom Stack.
// Als nAttrId sind erlaubt: 0 fuer alle, oder ein spezieller Typ.
// Damit erscheinen sie nicht in der Doc-Struktur. Dabei werden nur die
// Attribute entfernt, die im selben Absatz wie pPos stehen.
// Wird fuer Grafik-Apos -> Grafiken benutzt.
void SwFltControlStack::StealAttr(const SwPosition* pPos, USHORT nAttrId /* = 0 */)
{
    USHORT nCnt = Count();

    register SwFltStackEntry* pEntry;

    while (nCnt)
    {
        nCnt --;
        pEntry = (*this)[ nCnt ];
        if (pEntry->nPtNode.GetIndex()+1 == pPos->nNode.GetIndex() &&
            (!nAttrId || nAttrId == pEntry->pAttr->Which()))
            DeleteAndDestroy(nCnt);     // loesche aus dem Stack
    }
}

// SwFltControlStack::KillUnlockedAttr() loescht alle Attribute vom Stack,
// welche punktuell auf pPos aufgespannt sind.
// Damit erscheinen sie nicht in der Doc-Struktur.
// Wird im WW Import benoetigt zum ignorieren der auf dem 0x0c Section-
// Break-Symbol gesetzten Attribute.
void SwFltControlStack::KillUnlockedAttrs(const SwPosition& pPos)
{
    SwNodeIndex aAktNode( pPos.nNode, -1 );
    USHORT nAktIdx = pPos.nContent.GetIndex();

    USHORT nCnt = Count();
    register SwFltStackEntry* pEntry;
    while( nCnt )
    {
        nCnt --;
        pEntry = (*this)[ nCnt ];
        if(    !pEntry->bOld
            && !pEntry->bLocked
            && (pEntry->nMkNode  == aAktNode)
            && (pEntry->nMkCntnt == nAktIdx )
            && (pEntry->nPtNode  == aAktNode)
            && (pEntry->nPtCntnt == nAktIdx ))
        {
            DeleteAndDestroy( nCnt ); // loesche aus dem Stack
        }
    }
}

// Alle gelockten Attribute freigeben (unlocken) und das Ende setzen,
// alle anderen im Document setzen und wieder aus dem Stack loeschen
// Returned, ob das gesuchte Attribut / die gesuchten Attribute
// ueberhaupt auf dem Stack standen
BOOL SwFltControlStack::SetAttr(const SwPosition& rPos, USHORT nAttrId,
                                BOOL bTstEnde, long nHand )
{
    ASSERT(!nAttrId ||
        (POOLATTR_BEGIN <= nAttrId && POOLATTR_END > nAttrId) ||
        (RES_FLTRATTR_BEGIN <= nAttrId && RES_FLTRATTR_END > nAttrId),
        "Falsche Id fuers Attribut")

    BOOL bFound = FALSE;
    USHORT nCnt = Count();

    SwFltStackEntry* pEntry;

    for (USHORT i=0; i < nCnt; i++)
    {
        pEntry = (*this)[ i ];
        if (pEntry->bLocked)
        {
            // setze das Ende vom Attribut
            BOOL bF = FALSE;
            if (!nAttrId ){
                bF = TRUE;
            }else if( nAttrId == pEntry->pAttr->Which()){
                if( nAttrId != RES_FLTR_BOOKMARK ){     // Handle abfragen
                    bF = TRUE;
                }else if( nHand == ((SwFltBookmark*)(pEntry->pAttr))->GetHandle() )
                {
                    bF = TRUE;
                }
            }
            if( bF ){
                pEntry->SetEndPos(rPos);
                bFound = TRUE;
            }
#if 0
            if (!nAttrId || nAttrId == pEntry->pAttr->Which())
            {
                pEntry->SetEndPos(rPos);
                bFound = TRUE;
            }
#endif
            continue;
        }

        // ist die Endposition die Cursor-Position, dann noch nicht
        // ins Dokument setzen, es muss noch Text folgen;
        // ausser am Dokumentende. (Attribut-Expandierung !!)
        // Beim Ende-Stack niemals ausser am DocEnde reinsetzen
        if (bTstEnde)
        {
            if (bIsEndStack || pEntry->nPtNode.GetIndex()+1 ==
                        rPos.nNode.GetIndex())
            continue;
        }
        SetAttrInDoc(rPos, pEntry);
        DeleteAndDestroy(i);        // loesche aus dem Stack
        i--; nCnt--;        // Danach rutschen alle folgenden nach unten
    }
    return bFound;
}

static void MakePoint(SwFltStackEntry* pEntry, SwDoc* pDoc, SwPaM& rRegion)
{
            // der Anker ist der Point vom Pam. Dieser wird beim Einfugen
            // von Text usw. veraendert; darum wird er auf dem Stack
            // gespeichert. Das Attribut muss nur noch im Format
            // gesetzt werden.
    rRegion.DeleteMark();
    rRegion.GetPoint()->nNode = pEntry->nMkNode.GetIndex() + 1;
    SwCntntNode* pCNd = GetCntntNode(pDoc, rRegion.GetPoint()->nNode, TRUE);
    rRegion.GetPoint()->nContent.Assign(pCNd, pEntry->nMkCntnt);
}

static void MakeRegionOrPoint(SwFltStackEntry* pEntry, SwDoc* pDoc,
                    SwPaM& rRegion, BOOL bCheck )
{
    if (!pEntry->MakeRegion(pDoc, rRegion, bCheck ))
        MakePoint(pEntry, pDoc, rRegion);
}

// MakeBookRegionOrPoint() ist wie MakeRegionOrPoint, aber die besonderen
// Beschraenkungen von Bookmarks in Tabellen werden beachtet.
// ( Anfang und Ende muessen in selber Zelle sein )
static void MakeBookRegionOrPoint(SwFltStackEntry* pEntry, SwDoc* pDoc,
                    SwPaM& rRegion, BOOL bCheck )
{
    if (pEntry->MakeRegion(pDoc, rRegion, bCheck )){
        const SwNodes& rNds = pDoc->GetNodes();
//      BOOL b1 = rNds[rRegion.GetPoint()->nNode]->FindTableNode() != 0;
//      const SwStartNode* p1 = rNds[rRegion.GetPoint()->nNode]->FindTableBoxStartNode();
//      const SwStartNode* p2 = rNds[rRegion.GetMark()->nNode]->FindTableBoxStartNode();
        if( rNds[rRegion.GetPoint()->nNode]->FindTableBoxStartNode()
              != rNds[rRegion.GetMark()->nNode]->FindTableBoxStartNode() ){
            rRegion.Exchange();         // Ungueltiger Bereich
            rRegion.DeleteMark();       // -> beide auf Mark
        }
    }else{
        MakePoint(pEntry, pDoc, rRegion);
    }
}

static void MakePosition(SwFltStackEntry* pEntry, SwDoc* pDoc, SwPosition& rPosi)
{
    rPosi.nNode = pEntry->nMkNode.GetIndex() + 1;
    SwCntntNode* pCNd = GetCntntNode(pDoc, rPosi.nNode, TRUE);
    rPosi.nContent.Assign(pCNd, pEntry->nMkCntnt);
}

#ifdef DEBUG
extern FASTBOOL CheckNodesRange( const SwNodeIndex& rStt,
                    const SwNodeIndex& rEnd, FASTBOOL bChkSection );
#endif

// IterateNumrulePiece() sucht von rTmpStart bis rEnd den ersten
// fuer Numrules gueltigen Bereich heraus.
//
// rNds sind die Doc-Nodes
// rEnd ist Bereichs-Ende,
// rTmpStart ist ReinRaus-Parameter: Anfang des zu untersuchenden Bereiches rein,
//                                   Anfang des gueltigen Bereichs raus
// rTmpEnd ist raus-Parameter
// Return-Bool ist TRUE fuer gueltigen Bereich
static BOOL IterateNumrulePiece( const SwNodeIndex& rEnd,
                                SwNodeIndex& rTmpStart, SwNodeIndex& rTmpEnd )
{
    while( ( rTmpStart <= rEnd )
           && !( rTmpStart.GetNode().IsTxtNode() ) )    // suche gueltigen Anfang
        rTmpStart++;

    rTmpEnd = rTmpStart;
    while( ( rTmpEnd <= rEnd )
           && ( rTmpEnd.GetNode().IsTxtNode() ) )       // suche gueltiges Ende + 1
        rTmpEnd++;

    rTmpEnd--;                                      // gueltiges Ende

    return rTmpStart <= rTmpEnd;                    // gueltig ?
}

void SwFltControlStack::SetAttrInDoc(const SwPosition& rTmpPos, SwFltStackEntry* pEntry)
{
    SwPaM aRegion( rTmpPos );

    switch(pEntry->pAttr->Which())
    {
    case RES_FLTR_ANCHOR:
        {
            MakePoint(pEntry, pDoc, aRegion);
            SwFrmFmt* pFmt = ((SwFltAnchor*)pEntry->pAttr)->GetFrmFmt();
            SwFmtAnchor aAnchor(pFmt->GetAnchor());
            aAnchor.SetAnchor(aRegion.GetPoint());
            pFmt->SetAttr(aAnchor);
                    // Damit die Frames bei Einfuegen in existierendes Doc
                    //  erzeugt werden (erst nach Setzen des Ankers!):
            if(pDoc->GetRootFrm()
                && FLY_AT_CNTNT == pFmt->GetAnchor().GetAnchorId()){
                pFmt->MakeFrms();
            }
        }
        break;
    case RES_FLTR_STYLESHEET:
        break;
    case RES_TXTATR_FIELD:
        break;
    case RES_TXTATR_TOXMARK:
        break;
    case RES_FLTR_NUMRULE:          // Numrule 'reinsetzen
        {
            const String& rNumNm = ((SfxStringItem*)pEntry->pAttr)->GetValue();
            SwNumRule* pRul = pDoc->FindNumRulePtr( rNumNm );
            if( pRul )
            {
                if( pEntry->MakeRegion(pDoc, aRegion, TRUE))
                {
                    SwNodeIndex aTmpStart( aRegion.Start()->nNode );
                    SwNodeIndex aTmpEnd( aTmpStart );
                    SwNodeIndex& rRegEndNd = aRegion.End()->nNode;
                    while( IterateNumrulePiece( rRegEndNd,
                                                aTmpStart, aTmpEnd ) )
                    {
                        SwPaM aTmpPam( aTmpStart, aTmpEnd );
                        pDoc->SetNumRule( aTmpPam, *pRul );

                        aTmpStart = aTmpEnd;    // Start fuer naechstes Teilstueck
                        aTmpStart++;
                    }
                }
                else
                    pDoc->DelNumRule( rNumNm );
            }
        }
        break;
    case RES_FLTR_NUMRULE_NUM:
        break;
    case RES_FLTR_BOOKMARK:         // eigentlich nur fuer den Ende-Stack
        {
            SwFltBookmark* pB = (SwFltBookmark*)pEntry->pAttr;
            const String& rName = ((SwFltBookmark*)pEntry->pAttr)->GetName();

            if (IsFlagSet(BOOK_TO_VAR_REF))
            {
                if (pB->IsPgRef() && !pB->IsRef())
                {
                            // XRefs und Bookmarks sind bereits geUpcased
                    MakeBookRegionOrPoint(pEntry, pDoc, aRegion, TRUE);
                    pDoc->Insert(aRegion, SwFmtRefMark(rName));
                }
                else if( !pB->IsOnlyRef() )
                {
                    SwFieldType* pFT = pDoc->GetFldType(RES_SETEXPFLD, rName);
                    if (!pFT)
                    {                       // FieldType anlegen
                        SwSetExpFieldType aS(pDoc, rName, GSE_STRING);
                        pFT = pDoc->InsertFldType(aS);
                    }
                    SwSetExpField aFld((SwSetExpFieldType*)pFT,
                                        pB->GetValSys());
                    aFld.SetSubType(SUB_INVISIBLE);
                    MakePoint(pEntry, pDoc, aRegion);
                    pDoc->Insert(aRegion, SwFmtFld(aFld));
                    MoveAttrs( *(aRegion.GetPoint()) );
                }
            }
            if( !pB->IsOnlyRef() &&
                ( !IsFlagSet(HYPO) || IsFlagSet(BOOK_AND_REF) ) )
            {
                MakeBookRegionOrPoint(pEntry, pDoc, aRegion, TRUE);
                pDoc->MakeBookmark( aRegion, aEmptyKeyCode, rName, aEmptyStr);
            }
        }
        break;
    case RES_FLTR_TOX:
        {
            MakePoint(pEntry, pDoc, aRegion);

            SwPosition* pPoint = aRegion.GetPoint();

            SwFltTOX* pTOXAttr = (SwFltTOX*)pEntry->pAttr;

            // test if on this node there had been a pagebreak BEFORE the
            //     tox attribut was put on the stack
            SfxItemSet aBkSet( pDoc->GetAttrPool(), RES_PAGEDESC, RES_BREAK );
            SwCntntNode* pNd = 0;
            if( !pTOXAttr->HadBreakItem() || !pTOXAttr->HadPageDescItem() )
            {
                pNd = pPoint->nNode.GetNode().GetCntntNode();
                if( pNd )
                {
                    const SfxItemSet* pSet = pNd->GetpSwAttrSet();
                    const SfxPoolItem* pItem;
                    if( pSet )
                    {
                        if(    !pTOXAttr->HadBreakItem()
                            && SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, FALSE, &pItem ) )
                        {
                            aBkSet.Put( *pItem );
                            pNd->ResetAttr( RES_BREAK );
                        }
                        if(    !pTOXAttr->HadPageDescItem()
                            && SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC, FALSE, &pItem ) )
                        {
                            aBkSet.Put( *pItem );
                            pNd->ResetAttr( RES_PAGEDESC );
                        }
                    }
                }
            }

            // now insert the TOX
            const SwTOXBaseSection* pSect
                = pDoc->InsertTableOf(  *pPoint,
                                        *pTOXAttr->GetBase(),
                                        FALSE  );
            delete pTOXAttr->GetBase();


            // set (aboved saved and removed) the break item at the node following the TOX
            if( aBkSet.Count() )
                pNd->SetAttr( aBkSet );


            USHORT nCols = ((SwFltTOX*)pEntry->pAttr)->GetCols();
            aRegion.Move( fnMoveBackward );
            SwTOXBase* pBase = (SwTOXBase*)pDoc->GetCurTOX( *aRegion.GetPoint() );
            if( pBase )
            {
                SfxItemSet aSet( pDoc->GetAttrPool(), RES_COL, RES_COL );
                SwFmtCol aCol;
                aCol.Init( nCols, 708, USHRT_MAX );
                aSet.Put( aCol );
                pBase->SetAttrSet( aSet );
            }
        }
        break;
    case RES_FLTR_SECTION:
        MakePoint(pEntry, pDoc, aRegion);   // bislang immer Point==Mark
        pDoc->Insert(aRegion, *((SwFltSection*)pEntry->pAttr)->GetSection(),
                     0, FALSE);
        delete(((SwFltSection*)pEntry->pAttr)->GetSection());
        break;
    case RES_FLTR_REDLINE:
        {
            if (pEntry->MakeRegion(pDoc, aRegion, TRUE))
            {
                pDoc->SetRedlineMode(   REDLINE_ON
                                      | REDLINE_SHOW_INSERT
                                      | REDLINE_SHOW_DELETE );
                SwFltRedline& rFltRedline = *((SwFltRedline*)pEntry->pAttr);

                if( USHRT_MAX != rFltRedline.nAutorNoPrev )
                {
                    SwRedlineData aData(rFltRedline.eTypePrev,
                                        rFltRedline.nAutorNoPrev,
                                        rFltRedline.aStampPrev,
                                        aEmptyStr,
                                        0
                                        );
                    pDoc->AppendRedline(new SwRedline(aData, aRegion));
                }
                SwRedlineData aData(rFltRedline.eType,
                                    rFltRedline.nAutorNo,
                                    rFltRedline.aStamp,
                                    aEmptyStr,
                                    0
                                    );
                pDoc->AppendRedline( new SwRedline(aData, aRegion) );
                pDoc->SetRedlineMode(   REDLINE_NONE
                                      | REDLINE_SHOW_INSERT
                                      | REDLINE_SHOW_DELETE );
            }
        }
        break;
    default:
        if (pEntry->MakeRegion(pDoc, aRegion, FALSE))
            pDoc->Insert(aRegion, *pEntry->pAttr);
        break;
    }
}

SfxPoolItem* SwFltControlStack::GetFmtStackAttr(USHORT nWhich, USHORT * pPos)
{
    SwFltStackEntry* pEntry;
    USHORT nSize = Count();

    while (nSize)
    {
        // ist es das gesuchte Attribut ? (gueltig sind nur gelockte,
        // also akt. gesetzte Attribute!!)
        if ((pEntry = (*this)[ --nSize ])->bLocked &&
            pEntry->pAttr->Which() == nWhich)
        {
            if (pPos)
                *pPos = nSize;
            return (SfxPoolItem*)pEntry->pAttr;     // Ok, dann Ende
        }
    }
    return 0;
}

const SfxPoolItem* SwFltControlStack::GetOpenStackAttr(const SwPosition& rPos, USHORT nWhich)
{
    SwFltStackEntry* pEntry;
    USHORT nSize = Count();
    SwNodeIndex aAktNode( rPos.nNode, -1 );
    USHORT nAktIdx = rPos.nContent.GetIndex();

    while (nSize)
    {
        // ist es das gesuchte Attribut ? (gueltig sind nur gelockte,
        // also akt. gesetzte, noch offene Attribute!!)
        pEntry = (*this)[ --nSize ];
        if(    pEntry->bLocked
            && (pEntry->pAttr->Which() == nWhich)
            && (pEntry->nMkNode  == aAktNode)
            && (pEntry->nMkCntnt == nAktIdx ))
        {
            return (SfxPoolItem*)pEntry->pAttr;     // Ok, dann Ende
        }
    }
    return 0;
}

const SfxPoolItem* SwFltControlStack::GetFmtAttr(const SwPosition& rPos, USHORT nWhich)
{
    SfxPoolItem* pHt = GetFmtStackAttr(nWhich);
    if (pHt)
        return (const SfxPoolItem*)pHt;

    // im Stack ist das Attribut nicht vorhanden, also befrage das Dokument
//  SwCntntNode * pNd = rPaM.GetCntntNode();
    SwCntntNode * pNd = pDoc->GetNodes()[ rPos.nNode ]->GetCntntNode();

    if (!pNd)           // kein ContentNode, dann das dflt. Attribut
        return &pDoc->GetAttrPool().GetDefaultItem(nWhich);
    return &pNd->GetAttr(nWhich);
}

BOOL SwFltControlStack::IsAttrOpen(USHORT nAttrId)
{
    USHORT nCnt = Count();

    if (!nCnt)  return FALSE;

    SwFltStackEntry* pEntry;

    for (USHORT i=0; i < nCnt; i++)
    {
        pEntry = (*this)[ i ];
        if (pEntry->bLocked && nAttrId == pEntry->pAttr->Which())
            return TRUE;
    }
    return FALSE;
}

//------ hier stehen die Methoden von SwFltAnchor -----------
SwFltAnchor::SwFltAnchor(SwFrmFmt* pFmt) :
    SfxPoolItem(RES_FLTR_ANCHOR), pFrmFmt(pFmt)
{
}

SwFltAnchor::SwFltAnchor(const SwFltAnchor& rCpy) :
    SfxPoolItem(RES_FLTR_ANCHOR), pFrmFmt(rCpy.pFrmFmt)
{
}

int SwFltAnchor::operator==(const SfxPoolItem& rItem) const
{
    return pFrmFmt == ((SwFltAnchor&)rItem).pFrmFmt;
}

SfxPoolItem* __EXPORT SwFltAnchor::Clone(SfxItemPool*) const
{
    return new SwFltAnchor(*this);
}

//------ hier stehen die Methoden von SwFltRedline -----------
int SwFltRedline::operator==(const SfxPoolItem& rItem) const
{
    return this == &rItem;
}

SfxPoolItem* SwFltRedline::Clone( SfxItemPool* ) const
{
    return new SwFltRedline(*this);
}

//------ hier stehen die Methoden von SwFltBookmark -----------
SwFltBookmark::SwFltBookmark( const String& rNa, const String& rVa,
                                long nHand, BOOL bOnlyR )
    : SfxPoolItem(RES_FLTR_BOOKMARK), nHandle(nHand), aName(rNa), aVal(rVa),
    bOnlyRef(bOnlyR), bRef(FALSE), bPgRef(FALSE)
{
        // eSrc: CHARSET_DONTKNOW fuer keine UEbersetzung bei operator <<
        // Upcase wird immer gemacht.
        // bei XXXStack.NewAttr(...) wird nie eine UEbersetzung vorgenommen.
        // ansonsten: uebergebener Src-Charset fuer aName
        // im Filter eingestellter Src-Charset fuer aVal ( Text )
}

SwFltBookmark::SwFltBookmark(const SwFltBookmark& rCpy)
    : SfxPoolItem(RES_FLTR_BOOKMARK),
    nHandle(rCpy.nHandle),
    aName(rCpy.aName),
    aVal(rCpy.aVal),
    bOnlyRef(rCpy.bOnlyRef),
    bRef(rCpy.bRef),
    bPgRef(rCpy.bPgRef)
{
}

int SwFltBookmark::operator==(const SfxPoolItem& rItem) const
{
    return (aName == ((SwFltBookmark&)rItem).aName)
            && (nHandle == ((SwFltBookmark&)rItem).nHandle);
}

SfxPoolItem* SwFltBookmark::Clone(SfxItemPool*) const
{
    return new SwFltBookmark(*this);
}

//------ hier stehen die Methoden von SwFltTOX -----------

SwFltTOX::SwFltTOX(SwTOXBase* pBase, USHORT _nCols)
    : SfxPoolItem(RES_FLTR_TOX), pTOXBase(pBase), nCols( _nCols ),
      bHadBreakItem( FALSE ), bHadPageDescItem( FALSE )
{
}

SwFltTOX::SwFltTOX(const SwFltTOX& rCpy)
    : SfxPoolItem(RES_FLTR_TOX), pTOXBase(rCpy.pTOXBase), nCols( rCpy.nCols ),
      bHadBreakItem( rCpy.bHadBreakItem ), bHadPageDescItem( rCpy.bHadPageDescItem )
{
}

int SwFltTOX::operator==(const SfxPoolItem& rItem) const
{
    return pTOXBase == ((SwFltTOX&)rItem).pTOXBase;
}

SfxPoolItem* SwFltTOX::Clone(SfxItemPool*) const
{
    return new SwFltTOX(*this);
}

//------ hier stehen die Methoden von SwFltSwSection -----------

SwFltSection::SwFltSection(SwSection *pSect) :
    SfxPoolItem(RES_FLTR_SECTION), pSection(pSect)
{
}

SwFltSection::SwFltSection(const SwFltSection& rCpy) :
    SfxPoolItem(RES_FLTR_SECTION), pSection(rCpy.pSection)
{
}

int SwFltSection::operator==(const SfxPoolItem& rItem) const
{
    return pSection == ((SwFltSection&)rItem).pSection;
}

SfxPoolItem* __EXPORT SwFltSection::Clone(SfxItemPool*) const
{
    return new SwFltSection(*this);
}

//------ hier stehen die Methoden von SwFltEndStack -----------

// Bei Auftreten einer Referenz wird die entsprechende Textmarke als
// RefMark statt als Bookmark behandelt

void SwFltEndStack::SetBookRef( const String& rName, BOOL bPgRef)
{
    SfxPoolItem* pAttr;
    USHORT nSize = Count();

    while( nSize )
    {
        // ist es das gesuchte Attribut ?
        pAttr = (*this)[ --nSize ]->pAttr;
        if( (pAttr->Which() == RES_FLTR_BOOKMARK ) &&
//JP 11.05.00 - UNICODE-FRAGE - reicht ein EqualsIgnoreCaseAscii oder
//                              muß man das ueber die International machen?
             ((SwFltBookmark*)pAttr)->GetName().EqualsIgnoreCaseAscii( rName ))
        {
            if (bPgRef)
                ((SwFltBookmark*)pAttr)->SetPgRef();
            else
                ((SwFltBookmark*)pAttr)->SetRef();
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////
//
// hier beginnt der von mdt erzeugte code. dieser ist eine shell auf
// der writer-seite nach moeglichkeit bald fuer alle filter. die ganze
// schwierigkeit, texte & formatattribute einzufuegen, die positionen
// zu verwalten, styles & kopf/fuszzeilen etc.
//

//////////////////////////////////////////////////////////// SwFltShell
SwFltShell::SwFltShell(SwDoc* pDoc, SwPaM& rPaM, BOOL bNew, ULONG nFieldFl)
    : aStack(pDoc, nFieldFl),
    aEndStack(pDoc, nFieldFl),
    pCurrentPageDesc(0),
    eSrcCharSet( RTL_TEXTENCODING_MS_1252 ),
    nPageDescOffset(GetDoc().GetPageDescCnt()),
    pPaM(new SwPaM( *(rPaM.GetPoint()))),
    bNewDoc(bNew),
    bStdPD(FALSE),
    bProtect(FALSE),
    nAktStyle(0),
    pSavedPos(0),
    eSubMode(None)
{
    memset( pColls, 0, sizeof( pColls ) );
    pOutDoc = new SwFltOutDoc( *pDoc, pPaM, aStack, aEndStack );
    pOut = pOutDoc;

    if( !bNewDoc ){     // in ein Dokument einfuegen ?
                        // Da immer ganze Zeile eingelesen werden, muessen
                        // evtl. Zeilen eingefuegt / aufgebrochen werden
        const SwPosition* pPos = pPaM->GetPoint();
        const SwTxtNode* pSttNd = pPos->nNode.GetNode().GetTxtNode();
        USHORT nCntPos = pPos->nContent.GetIndex();
        if( nCntPos && pSttNd->GetTxt().Len() )
                                            // EinfuegePos nicht in leerer Zeile
            pDoc->SplitNode( *pPos );       // neue Zeile erzeugen
        if( pSttNd->GetTxt().Len() ){       // EinfuegePos nicht am Ende der Zeile
            pDoc->SplitNode( *pPos );       // neue Zeile
            pPaM->Move( fnMoveBackward );   // gehe in leere Zeile
        }

        // verhinder das Einlesen von Tabellen in Fussnoten / Tabellen
        ULONG nNd = pPos->nNode.GetIndex();
        BOOL bReadNoTbl = 0 != pSttNd->FindTableNode() ||
            ( nNd < pDoc->GetNodes().GetEndOfInserts().GetIndex() &&
            pDoc->GetNodes().GetEndOfInserts().StartOfSectionIndex() < nNd );
        if( bReadNoTbl )
            pOutDoc->SetReadNoTable();
    }
    pCurrentPageDesc =  &((SwPageDesc&)pDoc->GetPageDesc( 0 )); // Standard

}

SwFltShell::~SwFltShell()
{
    USHORT i;

    if (eSubMode == Style)
        EndStyle();
    if( pOutDoc->IsInTable() )          // falls nicht ordentlich abgeschlossen
        EndTable();
    if( pOutDoc->IsInFly() )
        EndFly();

    GetDoc().SetUpdateExpFldStat();
    GetDoc().SetInitDBFields(TRUE);
    aStack.SetAttr(*pPaM->GetPoint(), 0, FALSE);
    aStack.SetAttr(*pPaM->GetPoint(), 0, FALSE);
    aEndStack.SetAttr(*pPaM->GetPoint(), 0, FALSE);
    aEndStack.SetAttr(*pPaM->GetPoint(), 0, FALSE);
    if( bProtect ){     // Das ganze Doc soll geschuetzt sein

        SwDoc& rDoc = GetDoc();
                        // 1. SectionFmt und Section anlegen
        SwSectionFmt* pSFmt = rDoc.MakeSectionFmt( 0 );
        SwSection aS( CONTENT_SECTION, String::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM("PMW-Protect") ));
        aS.SetProtect( TRUE );
                        // 2. Start- und EndIdx suchen
        const SwNode* pEndNd = &rDoc.GetNodes().GetEndOfContent();
        SwNodeIndex aEndIdx( *pEndNd, -1L );
        const SwStartNode* pSttNd = pEndNd->StartOfSectionNode();
        SwNodeIndex aSttIdx( *pSttNd, 1L );         // +1 -> hinter StartNode
                                                    // Section einfuegen
                        // Section einfuegen
        rDoc.GetNodes().InsertSection( aSttIdx, *pSFmt, aS, &aEndIdx, FALSE );

        if( !IsFlagSet(SwFltControlStack::DONT_HARD_PROTECT) ){
            SwDocShell* pDocSh = rDoc.GetDocShell();
            if( pDocSh )
                pDocSh->SetReadOnlyUI( TRUE );
        }
    }
        // Pagedescriptoren am Dokument updaten (nur so werden auch die
        // linken Seiten usw. eingestellt).

    GetDoc().ChgPageDesc( 0, GetDoc().GetPageDesc( 0 ));    // PageDesc "Standard"
    for (i=nPageDescOffset;i<GetDoc().GetPageDescCnt();i++)
    {
        const SwPageDesc& rPD = GetDoc().GetPageDesc(i);
        GetDoc().ChgPageDesc(i, rPD);
    }

    delete pPaM;
    for (i=0; i<sizeof(pColls)/sizeof(*pColls); i++)
        if( pColls[i] )
            delete pColls[i];
    delete pOutDoc;
}

SwFltShell& SwFltShell::operator << ( const String& rStr )
{
    ASSERT(eSubMode != Style, "char insert while in style-mode");
    GetDoc().Insert( *pPaM, rStr );
    return *this;
}

void SwFltShell::ConvertUStr( String& rInOut )
{
    GetAppCharClass().toUpper( rInOut );
}
void SwFltShell::ConvertLStr( String& rInOut )
{
    GetAppCharClass().toLower( rInOut );
}

// QuoteString() wandelt CRs abhaengig von nFieldIniFlags in '\n' oder "\0x0d"
String SwFltShell::QuoteStr( const String& rIn )
{
    String sOut( rIn );
    BOOL bAllowCr = aStack.IsFlagSet( SwFltControlStack::ALLOW_FLD_CR );

    for( xub_StrLen n = 0; n < sOut.Len(); ++n )
    {
        switch( sOut.GetChar( n ) )
        {
        case 0x0a:
            sOut.Erase( n, 1 );             // 0xd 0xa wird zu \n
            break;

        case 0x0b:
        case 0x0c:
        case 0x0d:
            if( bAllowCr )
                 sOut.SetChar( n, '\n' );
            break;
        }
    }
    return sOut;
}

SwFltShell& SwFltShell::operator << ( const sal_Unicode c )
{
    ASSERT( eSubMode != Style, "char insert while in style-mode");
    GetDoc().Insert( *pPaM, c );
    return *this;
}

SwFltShell& SwFltShell::AddError( const sal_Char* pErr )
{
    String aName( String::CreateFromAscii(
                    RTL_CONSTASCII_STRINGPARAM( "ErrorTag" )));
    SwFieldType* pFT = GetDoc().GetFldType( RES_SETEXPFLD, aName );
    if( pFT == 0)
    {
        SwSetExpFieldType aS(&GetDoc(), aName, GSE_STRING);
        pFT = GetDoc().InsertFldType(aS);
    }
    SwSetExpField aFld( (SwSetExpFieldType*)pFT,
                        String::CreateFromAscii( pErr ));
    //, VVF_INVISIBLE
    GetDoc().Insert(*pPaM, SwFmtFld(aFld));
    return *this;
}

SwFltShell& SwFltShell::AddLinkedSection( const String& rFileName )
{
    String aStr( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Ww1" )));
    SwSection* pSection = new SwSection( FILE_LINK_SECTION,
                                     GetDoc().GetUniqueSectionName( &aStr ));
    pSection->SetLinkFileName( INetURLObject::RelToAbs( rFileName ) );
    pSection->SetProtect( TRUE );
    *this << SwFltSection(pSection);
    EndItem( RES_FLTR_SECTION);
    return *this;
}

SwFltShell& SwFltShell::operator << (Graphic& rGraphic)
{
    SwFlyFrmFmt* pFlyFmt = GetDoc().Insert( *pPaM, aEmptyStr, aEmptyStr,
                                            &rGraphic); // embedded Grafik !!
    return *this;
}

SwFltShell& SwFltShell::AddGraphic( const String& rPicName )
{
    if (0) // gelinkt?
    {
        SwFlyFrmFmt* pFlyFmt = GetDoc().Insert(*pPaM,
         INetURLObject::RelToAbs( rPicName ), // Name der Grafik !!
         aEmptyStr);
    }
    else
    {
    // embedded:
        GraphicFilter* pFilter = ::GetGrfFilter();
        Graphic aGraphic;
    // one of: GFF_NOT GFF_BMP GFF_GIF GFF_JPG GFF_PCD GFF_PCX GFF_PNG
    // GFF_TIF GFF_XBM GFF_DXF GFF_MET GFF_PCT GFF_SGF GFF_SVM GFF_WMF
    // GFF_SGV GFF_XXX
        INetURLObject aDir( URIHelper::SmartRelToAbs(rPicName) );
        switch ( pFilter->ImportGraphic( aGraphic, aDir ))
        {
        case GRFILTER_OK:
            *this << aGraphic;
            break;

        case GRFILTER_OPENERROR:
        case GRFILTER_IOERROR:
        case GRFILTER_FORMATERROR:
        case GRFILTER_VERSIONERROR:
        case GRFILTER_FILTERERROR:
        case GRFILTER_ABORT:
        case GRFILTER_TOOBIG:
        default:
            AddError( "picture import error" );
            break;
        }
    }
    return *this;
}

/*SwFltShell& SwFltShell::operator << (SwFltCharSet nCharSet)
{
    eSrcCharSet = nCharSet;
    return *this;
}*/

SwFltShell& SwFltShell::SetStyle( USHORT nStyle )
{
#ifndef MAC
    SwFltFormatCollection* p = pColls[ nStyle ];
#else
    SwFltFormatCollection* p = pColls[ nStyle ];
#endif

    if (p)
    {
        if( !pOutDoc->IsInTable() && nStyle != nAktStyle )
        {
            if( pColls[nAktStyle]->IsInFly() && pOutDoc->IsInFly() )
                pOutDoc->EndFly();
            if( p->IsInFly() )
                p->BeginStyleFly( pOutDoc );
        }
        GetDoc().SetTxtFmtColl(*pPaM, p->GetColl());
        nAktStyle = nStyle;
    }
    else
    {
        ASSERT( FALSE, "Ungueltiger SwFltStyleCode" );
    }
    return *this;
}

SwFltShell& SwFltShell::operator << (SwFltBookmark& aBook)
{
    ConvertUStr( aBook.aName );
    aBook.aVal = QuoteStr(aBook.aVal);
    aEndStack.NewAttr(*pPaM->GetPoint(), aBook);
    return *this;
}

void SwFltShell::SetBookEnd(long nHandle)
{
    aEndStack.SetAttr( *pPaM->GetPoint(), RES_FLTR_BOOKMARK, TRUE, nHandle );
}

SwFltShell& SwFltShell::EndItem( USHORT nAttrId )
{
    switch( nAttrId )
    {
    case RES_FLTR_BOOKMARK:
        ASSERT( FALSE, "Falscher Aufruf fuer Bookmark-Ende" );
        break;

    case RES_FLTR_TOX:
        aEndStack.SetAttr(*pPaM->GetPoint(), nAttrId);
        break;

    default:
        aStack.SetAttr(*pPaM->GetPoint(), nAttrId);
        break;
    }
    return *this;
}

SwFltShell& SwFltShell::operator << (const SwField& rField)
{
    GetDoc().Insert(*pPaM, SwFmtFld(rField));
    return *this;
}

/*virtual*/ SwFltOutBase& SwFltOutDoc::operator << (const SfxPoolItem& rItem)
{
    rStack.NewAttr(*pPaM->GetPoint(), rItem);
    return *this;
}

SwFltShell& SwFltShell::operator << (const SwFltTOX& rItem)
{
    aEndStack.NewAttr(*pPaM->GetPoint(), rItem);
    return *this;
}

/*virtual*/ SwFltOutBase& SwFltFormatCollection::operator <<
                                (const SfxPoolItem& rItem)
{
    pColl->SetAttr(rItem);
    return *this;
}


#if 0
SwFltShell& SwFltShell::operator >> (SfxPoolItem& rItem)
{
    //rItem = *GetFmtAttr(rItem.Which());
#pragma message(__FILE__ "(?) : ^das waer schoen!")
    //~ mdt: remove from stack if item begins here.
    return *this;
}
#endif

const SfxPoolItem& SwFltOutDoc::GetAttr(USHORT nWhich)
{
    return *rStack.GetFmtAttr(*pPaM->GetPoint(), nWhich);
}

const SfxPoolItem& SwFltFormatCollection::GetAttr(USHORT nWhich)
{
    return GetColl()->GetAttr(nWhich);  // mit Parents
}

// GetNodeOrStyAttr holt Attribute fuer Toggle- und Modify-Attribute:
// Bei Formatdefinitionen aus dem altuellen Style mit Parents
// sonst aus dem Node mit Parents
// Im Stack wird nicht nachgesehen

const SfxPoolItem& SwFltOutDoc::GetNodeOrStyAttr(USHORT nWhich)
{
    SwCntntNode * pNd = GetDoc().GetNodes()[ pPaM->GetPoint()->nNode ]
                            ->GetCntntNode();
    if (pNd)            // ContentNode: Attribut mit Parent
        return pNd->GetAttr(nWhich);
    else                // kein ContentNode, dann das dflt. Attribut
        return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
}

const SfxPoolItem& SwFltFormatCollection::GetNodeOrStyAttr(USHORT nWhich)
{
    return GetColl()->GetAttr(nWhich);  // mit Parents
}


BOOL SwFltShell::GetWeightBold()
{
    return ((SvxWeightItem&)GetNodeOrStyAttr(RES_CHRATR_WEIGHT)).GetWeight()
                                != WEIGHT_NORMAL;
}

BOOL SwFltShell::GetPostureItalic()
{
    return ((SvxPostureItem&)GetNodeOrStyAttr(RES_CHRATR_POSTURE)).GetPosture()
                                != ITALIC_NONE;
}

BOOL SwFltShell::GetCrossedOut()
{
    return ((SvxCrossedOutItem&)GetNodeOrStyAttr(RES_CHRATR_CROSSEDOUT))
                                    .GetStrikeout() != STRIKEOUT_NONE;
}

BOOL SwFltShell::GetContour()
{
    return ((SvxContourItem&)GetNodeOrStyAttr(RES_CHRATR_CONTOUR)).GetValue();
}

BOOL SwFltShell::GetCaseKapitaelchen()
{
    return ((SvxCaseMapItem&)GetNodeOrStyAttr(RES_CHRATR_CASEMAP))
                                    .GetCaseMap() == SVX_CASEMAP_KAPITAELCHEN;
}

BOOL SwFltShell::GetCaseVersalien()
{
    return ((SvxCaseMapItem&)GetNodeOrStyAttr(RES_CHRATR_CASEMAP))
                                    .GetCaseMap() == SVX_CASEMAP_VERSALIEN;
}

//-------------------------------------------------------------------------
// Tabellen
//-------------------------------------------------------------------------
const SfxPoolItem& SwFltOutBase::GetCellAttr(USHORT nWhich)
{
    ASSERT(FALSE, "GetCellAttr ausserhalb von normalem Text");
    return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
}

BOOL SwFltOutBase::BeginTable()
{
    ASSERT(FALSE, "BeginTable ausserhalb von normalem Text");
    return FALSE;
}

void SwFltOutBase::NextTableCell()
{
    ASSERT(FALSE, "NextTableCell ausserhalb von normalem Text");
}

void SwFltOutBase::NextTableRow()
{
    ASSERT(FALSE, "NextTableRow ausserhalb von normalem Text");
}

void SwFltOutBase::SetTableWidth(SwTwips nW)
{
    ASSERT(FALSE, "SetTableWidth ausserhalb von normalem Text");
}

void SwFltOutBase::SetTableOrient(SwHoriOrient eOri)
{
    ASSERT(FALSE, "SetTableOrient ausserhalb von normalem Text");
}

void SwFltOutBase::SetCellWidth(SwTwips nWidth, USHORT nCell)
{
    ASSERT(FALSE, "SetCellWidth ausserhalb von normalem Text");
}

void SwFltOutBase::SetCellHeight(SwTwips nH)
{
    ASSERT(FALSE, "SetCellHeight ausserhalb von normalem Text");
}

void SwFltOutBase::SetCellBorder(const SvxBoxItem& rFmtBox, USHORT nCell)
{
    ASSERT(FALSE, "SetCellBorder ausserhalb von normalem Text");
}

void SwFltOutBase::SetCellSpace(USHORT nSp)
{
    ASSERT(FALSE, "SetCellSpace ausserhalb von normalem Text");
}

void SwFltOutBase::DeleteCell(USHORT nCell)
{
    ASSERT(FALSE, "DeleteCell ausserhalb von normalem Text");
}

void SwFltOutBase::EndTable()
{
    ASSERT(FALSE, "EndTable ausserhalb von normalem Text");
}

/*virtual*/ BOOL SwFltOutDoc::IsInTable()
{
    return pTable != 0;
};

BOOL SwFltOutDoc::BeginTable()
{
    if(bReadNoTbl)
        return FALSE;

    if (pTable){
        ASSERT(FALSE, "BeginTable in Table");
        return FALSE;
    }
                            // Alle Attribute schliessen, da sonst Attribute
                            // entstehen koennen, die in Flys reinragen
    rStack.SetAttr( *pPaM->GetPoint(), 0, FALSE );
    rEndStack.SetAttr( *pPaM->GetPoint(), 0, FALSE );

// create table:
    ASSERT(pTabSavedPos == NULL, "SwFltOutDoc");
    pTabSavedPos = new SwPosition(*pPaM->GetPoint());
    pTable = GetDoc().InsertTable(*pTabSavedPos, 1, 1, HORI_LEFT);
    nTableWidth = 0;
    ((SwTable*)pTable)->LockModify();   // Nichts automatisch anpassen!
// set pam in 1. table cell
    usTableX =
    usTableY = 0;
    SeekCell(usTableY, usTableX, TRUE);
    return TRUE;
}

SwTableBox* SwFltOutDoc::GetBox(USHORT ny, USHORT nx /*= USHRT_MAX */)
{
    if(!pTable){
        ASSERT(pTable, "GetBox ohne Tabelle");
        return 0;
    }
    if( nx == USHRT_MAX )   // aktuelle Zelle
        nx = usTableX;

// get structs to table cells
    const SwTableLines* pTableLines = &pTable->GetTabLines();
    if(!pTableLines){
        ASSERT(FALSE, "SwFltOutDoc:GetBox:pTableLines");
        return 0;
    }
    if( ny >= pTableLines->Count() ){   // Notbremse
        ASSERT( FALSE, "SwFltOutDoc:GetBox:ny >= Count()");
        ny = pTableLines->Count() - 1;
    }
    SwTableLine* pTableLine = (*pTableLines)[ny];
    if(!pTableLine){
        ASSERT(FALSE, "SwFltOutDoc:GetBox:pTableLine");
        return 0;
    }
    SwTableBoxes* pTableBoxes = &pTableLine->GetTabBoxes();
    if(!pTableBoxes){
        ASSERT(FALSE, "SwFltOutDoc:GetBox:pTableBoxes");
        return 0;
    }
    if( nx >= pTableBoxes->Count() ){   // Notbremse
        ASSERT(FALSE, "SwFltOutDoc:GetBox:nx >= Count()");
        nx = pTableBoxes->Count() - 1;
    }
    SwTableBox* pTableBox = (*pTableBoxes)[nx];

    ASSERT(pTableBox != 0, "SwFltOutDoc:GetBox:pTableBox");
    return pTableBox;
}

void SwFltOutDoc::NextTableCell()
{
    if(!pTable){
        ASSERT(pTable, "NextTableCell ohne Tabelle");
        return;
    }
    const SwTableLines* pTableLines = &pTable->GetTabLines();
    SwTableLine* pTableLine = (*pTableLines)[usTableY];
    SwTableBoxes* pTableBoxes = &pTableLine->GetTabBoxes();
    SwTableBox* pTableBox = (*pTableBoxes)[usTableX];
    ASSERT(pTableBox != 0, "SwFltOutDoc:NextTableCell:pTableBox");
    if(!pTableBox)
        return;
//#pragma message(__FILE__ "(?) : Sw's const problem")
// insert cells:
    if (++usTableX >= pTableBoxes->Count())
        GetDoc().GetNodes().InsBoxen(
         GetDoc().IsIdxInTbl(pPaM->GetPoint()->nNode),
         pTableLine,
         (SwTableBoxFmt*)pTableBox->GetFrmFmt(),
         GetDoc().GetTxtCollFromPool(RES_POOLCOLL_STANDARD),
         0,
         pTableBoxes->Count());
    SeekCell(usTableY, usTableX, TRUE);
    pTableBox = (*pTableBoxes)[usTableX];
    ASSERT(pTableBox != 0, "SwFltOutDoc:pTableBox");
    if(pTableBox)
        (*pTableBoxes)[usTableX]->ClaimFrmFmt();
}

void SwFltOutDoc::NextTableRow()
{
    SwTableBox* pTableBox = GetBox(usTableY, 0);
    if(pTableBox){
// duplicate row:
        SwSelBoxes aSelBoxes;
        aSelBoxes.Insert( pTableBox );
        GetDoc().InsertRow(aSelBoxes);
//      GetDoc().InsertRow(pTable->SelLineFromBox(pTableBox, aSelBoxes));
#ifdef DEBUG
        const SwTableLines* pTableLines = &pTable->GetTabLines();
        SwTableLine* pTableLine = (*pTableLines)[usTableY+1];
        SwTableBoxes* pTableBoxes = &pTableLine->GetTabBoxes();
        USHORT nBx = pTableBoxes->Count();
        SwTableBox* pTableBox = (*pTableBoxes)[0];
        SwFrmFmt* pFmt = pTableBox->GetFrmFmt();

        SwFmtFrmSize &rSz = ((SwFmtFrmSize&)pFmt->GetAttr(RES_FRM_SIZE));
        SwTwips nW = rSz.GetWidth();
#endif
        usTableX = 0;
        SeekCell(++usTableY, usTableX, TRUE);
        GetDoc().SetTxtFmtColl(*pPaM,
            GetDoc().GetTxtCollFromPool(RES_POOLCOLL_STANDARD));
    }
}

void SwFltOutDoc::SetTableWidth(SwTwips nSwWidth)
{
    if(!pTable){
        ASSERT(pTable, "SetTableWidth ohne Tabelle");
        return;
    }
    ASSERT( nSwWidth > MINLAY, "Tabellenbreite <= MINLAY" );
    if( nSwWidth != nTableWidth ){
        if( nTableWidth )           // Nicht beim ersten Setzen
            SplitTable();
        pTable->GetFrmFmt()->SetAttr( SwFmtFrmSize(ATT_VAR_SIZE, nSwWidth));
        nTableWidth = nSwWidth;
    }
}

void SwFltOutDoc::SetTableOrient(SwHoriOrient eOri)
{
    if(!pTable){
        ASSERT(pTable, "SetTableOrient ohne Tabelle");
        return;
    }
    pTable->GetFrmFmt()->SetAttr( SwFmtHoriOrient( 0, eOri ));
}

void SwFltOutDoc::SetCellWidth(SwTwips nWidth, USHORT nCell /* = USHRT_MAX */ )
{
    if(!pTable){
        ASSERT(pTable, "SetCellWidth ohne Tabelle");
        return;
    }
    ASSERT( nWidth > MINLAY, "Tabellenzellenbreite <= MINLAY" );
    if (nWidth < MINLAY)
        nWidth = MINLAY;

    SwTableBox* pTableBox = GetBox(usTableY, nCell);
    if(pTableBox && pTableBox->GetFrmFmt() ){
        SwFmtFrmSize aFmtFrmSize(ATT_FIX_SIZE);
        aFmtFrmSize.SetWidth(nWidth);
        pTableBox->GetFrmFmt()->SetAttr(aFmtFrmSize);
    }
}

void SwFltOutDoc::SetCellHeight(SwTwips nHeight)
{
    if(!pTable){
        ASSERT(pTable, "SetCellHeight ohne Tabelle");
        return;
    }

    const SwTableLines* pTableLines = &pTable->GetTabLines();
    SwTableLine* pTableLine = (*pTableLines)[usTableY];
    SwFmtFrmSize aFmtFrmSize(ATT_MIN_SIZE, 0, 0);
    if (nHeight < MINLAY)
        nHeight = MINLAY;
    aFmtFrmSize.SetHeight(nHeight);
    pTableLine->GetFrmFmt()->SetAttr(aFmtFrmSize);
}

const SfxPoolItem& SwFltOutDoc::GetCellAttr(USHORT nWhich)
{
    if (!pTable){
        ASSERT(pTable, "GetCellAttr ohne Table");
        return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
    }

    SwTableBox* pTableBox = GetBox(usTableY, usTableX);
    if(!pTableBox)
        return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
    return pTableBox->GetFrmFmt()->GetAttr( nWhich );
}

void SwFltOutDoc::SetCellBorder(const SvxBoxItem& rFmtBox,
                               USHORT nCell /* = USHRT_MAX */ )
{
    SwTableBox* pTableBox = GetBox(usTableY, nCell);
    if(pTableBox)
        pTableBox->GetFrmFmt()->SetAttr(rFmtBox);
}

// nicht aktiviert !!!
void SwFltOutDoc::SetCellSpace(USHORT nDist)
{
    if(!pTable){
        ASSERT(pTable, "SetCellSpace ohne Tabelle");
        return;
    }
    SwTableBox* pTableBox = GetBox(usTableY, usTableX);
    if(!pTableBox)
        return;

    SvxBoxItem aFmtBox( *((SvxBoxItem*)
                        &pTableBox->GetFrmFmt()->GetAttr( RES_BOX )));

    // versteh ich nich, sven: if (!nDist) nDist = 18; // ca. 0.03 cm
    if (nDist > 42) // max. 0.7 mm
        nDist = 42;
    else
        if (nDist < MIN_BORDER_DIST)
            nDist = MIN_BORDER_DIST;
    aFmtBox.SetDistance(nDist);
    pTableBox->GetFrmFmt()->SetAttr(aFmtBox);
}

void SwFltOutDoc::DeleteCell(USHORT nCell /* = USHRT_MAX */)
{
    SwTableBox* pTableBox = GetBox(usTableY, nCell);
    if(pTableBox){
        SwSelBoxes aSelBoxes;
        aSelBoxes.Insert( pTableBox );
        GetDoc().DeleteRowCol(aSelBoxes);
        usTableX--;
    }
}

void SwFltOutDoc::SplitTable()
{
    if(!pTable){
        ASSERT(pTable, "SplitTable ohne Tabelle");
        return;
    }
    SwTableBox* pAktBox = GetBox(usTableY, usTableX);
    SwTableBox* pSplitBox = GetBox(usTableY - 1, 0);
    BOOL bOk = 0 != GetDoc().GetNodes().SplitTable(
                            SwNodeIndex( *pSplitBox->GetSttNd() ), FALSE );
    pTable = &pAktBox->GetSttNd()->FindTableNode()->GetTable();
    usTableY = 0;
//  GetDoc().Insert(*pPaM, "SplitTable", GetSystemCharSet());
}

void SwFltOutDoc::EndTable()
{
    if (!pTable){
        ASSERT(pTable, "EndTable ohne Table");
        return;
    }
                            // Alle Attribute schliessen, da sonst Attribute
                            // entstehen koennen, die in Flys reinragen
    rStack.SetAttr( *pPaM->GetPoint(), 0, FALSE );
    rEndStack.SetAttr( *pPaM->GetPoint(), 0, FALSE );

    if (GetDoc().GetRootFrm()){
        SwTableNode* pTableNode = GetDoc().IsIdxInTbl(
            pPaM->GetPoint()->nNode);
        pTableNode->DelFrms();
        pTableNode->MakeFrms(&pPaM->GetPoint()->nNode);
    }

    *pPaM->GetPoint() = *pTabSavedPos;              // restore Cursor
    delete pTabSavedPos;
    pTabSavedPos = 0;
    ((SwTable*)pTable)->UnlockModify(); // Test, nuetzt nichts gegen Assert
    pTable = 0;
    nTableWidth = 0;
}

BOOL SwFltOutDoc::SeekCell(short nRow, short nCol, BOOL bPam)
{
// get structs to table cells
    const SwTableLines* pTableLines = &pTable->GetTabLines();
    SwTableLine* pTableLine = (*pTableLines)[usTableY];
    SwTableBoxes* pTableBoxes = &pTableLine->GetTabBoxes();
    SwTableBox* pTableBox = (*pTableBoxes)[usTableX];

    if ((USHORT)nRow >= pTableLines->Count())
    {
        ASSERT((USHORT)nRow >= pTableLines->Count(), "SwFltOutDoc");
        return FALSE;
    }
    pTableLine = (*pTableLines)[nRow];
    pTableBoxes = &pTableLine->GetTabBoxes();
    if (nCol >= pTableBoxes->Count())
        return FALSE;
    pTableBox = (*pTableBoxes)[nCol];
    if( !pTableBox->GetSttNd() )
    {
        ASSERT(pTableBox->GetSttNd(), "SwFltOutDoc");
        return FALSE;
    }
    if(bPam)
    {
        pPaM->GetPoint()->nNode = pTableBox->GetSttIdx() + 1;
        pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(), 0);
//#pragma message(__FILE__ "(?) : Sw's const problem")
#ifdef DEBUG
        const SwTxtFmtColl* p = GetDoc().GetDfltTxtFmtColl();
        p = GetDoc().GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
#endif
        GetDoc().SetTxtFmtColl(*pPaM,
            GetDoc().GetTxtCollFromPool(RES_POOLCOLL_STANDARD));
    }
    return TRUE;
}


//-----------------------------------------------------------------------------
// Flys in SwFltOutBase
//-----------------------------------------------------------------------------

SfxItemSet* SwFltOutBase::NewFlyDefaults()
{
// Unbedingt noetige Standardwerte setzen ( falls diese Werte nicht
// spaeter explizit gesetzt werden )

    SfxItemSet* p = new SfxItemSet( GetDoc().GetAttrPool(),
                                    RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
    SwFmtFrmSize aSz( ATT_VAR_SIZE, MINFLY, MINFLY );
                                        // Default: Breite 100% ( = PMW:Auto )
    aSz.SetWidthPercent( 100 );         // Hoehe: Auto
    p->Put( aSz );
    p->Put( SwFmtHoriOrient( 0, HORI_NONE, FRAME ));
    return p;
}

BOOL SwFltOutBase::BeginFly( RndStdIds eAnchor /*= FLY_AT_CNTNT*/,
                           BOOL bAbsolutePos /*= FALSE*/,
                           const SfxItemSet* pMoreAttrs /*= 0*/ )
{
    ASSERT(!pMoreAttrs, "SwFltOutBase:BeginFly mit pMoreAttrs" );
    eFlyAnchor = eAnchor;
    bFlyAbsPos = bAbsolutePos;      // Bloedsinn eigentlich
    return TRUE;
}

/*virtual*/ void SwFltOutBase::SetFlyAnchor( RndStdIds eAnchor )
{
    if( !IsInFly() ){
        ASSERT( FALSE, "SetFlyAnchor() ohne Fly" );
        return;
    }
    if( eAnchor == FLY_IN_CNTNT ){
        ASSERT( FALSE, "SetFlyAnchor( FLY_IN_CNTNT ) nicht implementiert" );
        return;
    }
    SwFmtAnchor& rAnchor = (SwFmtAnchor&)GetFlyFrmAttr( RES_ANCHOR );
    rAnchor.SetType( eAnchor );
}

void SwFltOutBase::EndFly()
{
    if( bFlyAbsPos ){
        // hier muessen die absoluten Positionen am Fly noch in
        // die Writer-Koordinaten umgerechnet werden.
    }
}

//-----------------------------------------------------------------------------
// Flys in SwFltDoc
//-----------------------------------------------------------------------------

/* virtual */ BOOL SwFltOutDoc::IsInFly()
{
    return pFly != 0;
};

SwFrmFmt* SwFltOutDoc::MakeFly( RndStdIds eAnchor, SfxItemSet* pSet )
{
    pFly = (SwFlyFrmFmt*)GetDoc().MakeFlySection( eAnchor, pPaM->GetPoint(),
                                                    pSet );
    return pFly;
}

BOOL SwFltOutDoc::BeginFly( RndStdIds eAnchor /*= FLY_AT_CNTNT*/,
                           BOOL bAbsolutePos /*= FALSE*/,
                           const SfxItemSet* pMoreAttrs /*= 0*/ )

{
    SwFltOutBase::BeginFly( eAnchor, bAbsolutePos, 0 );
    SfxItemSet* pSet = NewFlyDefaults();

// Alle Attribute schliessen, da sonst Attribute entstehen koennen,
// die in Flys reinragen
    rStack.SetAttr( *pPaM->GetPoint(), 0, FALSE );
    rEndStack.SetAttr( *pPaM->GetPoint(), 0, FALSE );

// create Fly:
    ASSERT(pFlySavedPos == NULL, "BeginFly in Fly");    // rekursiv geht noch nicht
    pFlySavedPos = new SwPosition(*pPaM->GetPoint());


    SwFmtAnchor aAnchor( eAnchor, 1 );

// Wenn Fly-Attribute im Style waren, dann jetzt als Defaults reinsetzen
    if (pMoreAttrs)
        pSet->Put(*pMoreAttrs);

//  dieses NICHT bei Seitengebundenem Fly mit Seiten-NUMMER !
    aAnchor.SetAnchor(pPaM->GetPoint());    // braucht erstaunlicherweise
                                            // den Stack nicht
//  aStack.NewAttr( *pPaM->GetPoint(), SwFltAnchor( pFly ) );

    pSet->Put( aAnchor );
    SwFrmFmt* pF = MakeFly( eAnchor, pSet );
    delete pSet;

// set pam in Fly
    const SwFmtCntnt& rCntnt = pF->GetCntnt();
    ASSERT( rCntnt.GetCntntIdx(), "Kein Inhalt vorbereitet." );
    pPaM->GetPoint()->nNode = rCntnt.GetCntntIdx()->GetIndex() + 1;
    SwCntntNode *pNode = pPaM->GetCntntNode();
    pPaM->GetPoint()->nContent.Assign( pNode, 0 );

    return TRUE;
}

/*virtual*/ void SwFltOutDoc::SetFlyFrmAttr(const SfxPoolItem& rAttr)
{
    if (pFly)
        pFly->SetAttr( rAttr );
    else
        ASSERT(pFly, "SetFlyAttr ohne Doc-Fly");
}

/*virtual*/ const SfxPoolItem& SwFltOutDoc::GetFlyFrmAttr(USHORT nWhich)
{
    if (pFly){
        return pFly->GetAttr( nWhich );
    }else{
        ASSERT(pFly, "GetFlyAttr ohne Fly");
        return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
    }
}

void SwFltOutDoc::EndFly()
{
    if( pTable ){
        ASSERT( FALSE, "SwFltOutDoc::EndFly() in Table" );
        return;
    }
                        // Alle Attribute schliessen, da sonst Attribute
                        // entstehen koennen, die aus Flys rausragen
    rStack.SetAttr( *pPaM->GetPoint(), 0, FALSE );
    rEndStack.SetAttr( *pPaM->GetPoint(), 0, FALSE );

    *pPaM->GetPoint() = *pFlySavedPos;              // restore Cursor
    delete pFlySavedPos;
    pFlySavedPos = 0;
    SwFltOutBase::EndFly();
    pFly = 0;
}

//-----------------------------------------------------------------------------
// Flys in SwFltFormatCollection
//-----------------------------------------------------------------------------
/*virtual*/ BOOL SwFltFormatCollection::IsInFly()
{
    return bHasFly;
};

/*virtual*/ void SwFltFormatCollection::SetFlyFrmAttr(const SfxPoolItem& rAttr)
{
    if (!pFlyAttrs)
        pFlyAttrs = new SfxItemSet( GetDoc().GetAttrPool(),
                             RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
    pFlyAttrs->Put( rAttr );
}

/*virtual*/ const SfxPoolItem& SwFltFormatCollection::GetFlyFrmAttr(USHORT nWhich)
{
//  ASSERT( pFlyAttrs, "GetFlyFrmAttr ohne Coll-FlyAttrs" );
    if( pFlyAttrs )
        return pFlyAttrs->Get( nWhich, FALSE );
    else
        return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
}

BOOL SwFltFormatCollection::BeginFly( RndStdIds eAnchor /*= FLY_AT_CNTNT*/,
                           BOOL bAbsolutePos /*= FALSE*/,
                           const SfxItemSet* pMoreAttrs /*= 0*/ )

{
    SwFltOutBase::BeginFly( eAnchor, bAbsolutePos, pMoreAttrs );
    bHasFly = TRUE;
    return TRUE;
}

void SwFltFormatCollection::EndFly()    // Wird nie aufgerufen
{
}

BOOL SwFltFormatCollection::BeginStyleFly( SwFltOutDoc* pOutDoc )
{
    ASSERT( pOutDoc, "BeginStyleFly ohne pOutDoc" );
    ASSERT( pOutDoc && !pOutDoc->IsInFly(), "BeginStyleFly in Fly" );
    if( pOutDoc && !pOutDoc->IsInFly() )
        return pOutDoc->BeginFly( eFlyAnchor, bFlyAbsPos, pFlyAttrs );
    else
        return FALSE;
}

//-----------------------------------------------------------------------------
// Flys in SwFltShell
//-----------------------------------------------------------------------------

BOOL SwFltShell::BeginFly( RndStdIds eAnchor /*= FLY_AT_CNTNT*/,
                           BOOL bAbsolutePos /*= FALSE*/ )

{
    if (pOut->IsInFly()){
        ASSERT(FALSE, "BeginFly in Fly");
        return FALSE;
    }
    if (pOutDoc->IsInTable()){
        ASSERT(FALSE, "BeginFly in Table");
        return FALSE;
    }
    pOut->BeginFly( eAnchor, bAbsolutePos, pColls[nAktStyle]->GetpFlyAttrs() );
    eSubMode = Fly;
    return TRUE;
}

void SwFltShell::SetFlyXPos( short nXPos,SwRelationOrient eHRel /*= FRAME*/,
                             SwHoriOrient eHAlign /*= HORI_NONE*/ )
{
    SetFlyFrmAttr( SwFmtHoriOrient( nXPos, eHAlign, eHRel ) );
}

void SwFltShell::SetFlyYPos( short nYPos, SwRelationOrient eVRel /*= FRAME*/,
                             SwVertOrient eVAlign /*= VERT_NONE*/ )
{
    SetFlyFrmAttr( SwFmtVertOrient( nYPos, eVAlign, eVRel ) );
}


void SwFltShell::EndFly()
{
    if (!pOut->IsInFly()){
        ASSERT(FALSE, "EndFly ohne Fly");
        return;
    }
    if (pOutDoc->IsInTable()){      // Table verschraenkt mit Fly macht keinen Sinn
        ASSERT(FALSE, "EndFly in Table ( verschraenkt )");
        EndTable();     // -> Table beenden
    }
    pOut->EndFly();
    eSubMode = None;
}

//-----------------------------------------------------------------------------
// Fussnoten
//-----------------------------------------------------------------------------

void SwFltShell::BeginFootnote()
{
    if( pOut->IsInFly() ){          // Passiert z.B. bei Fussnote in Fly
        ASSERT(FALSE, "Fussnote in Fly nicht erlaubt");
        return;
    }
    if( pOutDoc->IsInTable() ){
        ASSERT(FALSE, "Fussnote in Table z.Zt. nicht erlaubt");
        return;
    }

// Alle Attribute schliessen, da sonst Attribute entstehen koennen,
// die in Fussnoten reinragen
    aStack.SetAttr( *pPaM->GetPoint(), 0, FALSE );
//  aEndStack.SetAttr( *pPaM->GetPoint(), 0, FALSE );
//  EndStack erstmal nicht zwangs-Schliessen, damit Bookmarks ueber
//  Fussnoten im PMW uebernommen werden

    SwFmtFtn aFtn;
    GetDoc().Insert(*pPaM, aFtn);
    ASSERT(pSavedPos == NULL, "SwFltShell");
    pSavedPos = new SwPosition(*pPaM->GetPoint());
    pPaM->Move(fnMoveBackward, fnGoCntnt);
    SwTxtNode* pTxt = pPaM->GetNode()->GetTxtNode();
    SwTxtAttr* pFN = pTxt->GetTxtAttr(pPaM->GetPoint()->nContent,
     RES_TXTATR_FTN);
    if( !pFN ){         // Passiert z.B. bei Fussnote in Fly
        ASSERT(pFN, "Probleme beim Anlegen des Fussnoten-Textes");
        return;
    }
    const SwNodeIndex* pStartIndex = ((SwTxtFtn*)pFN)->GetStartNode();
    ASSERT(pStartIndex, "Probleme beim Anlegen des Fussnoten-Textes");
    pPaM->GetPoint()->nNode = pStartIndex->GetIndex() + 1;
    pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(), 0);
    eSubMode = Footnote;
}

void SwFltShell::EndFootnote()
{
    if(!pSavedPos)
        return;
                        // Alle Attribute schliessen, da sonst Attribute
                        // entstehen koennen, die aus Fussnoten rausragen
    aStack.SetAttr( *pPaM->GetPoint(), 0, FALSE );
//  aEndStack.SetAttr( *pPaM->GetPoint(), 0, FALSE );
//  EndStack erstmal nicht zwangs-Schliessen, damit Bookmarks ueber
//  Fussnoten im PMW uebernommen werden

    *pPaM->GetPoint() = *pSavedPos;             // restore Cursor
    delete pSavedPos;
    pSavedPos = 0;
}

void SwFltShell::BeginHeader(SwPageDesc* pPD)
{
    SwFrmFmt* pFmt = &pCurrentPageDesc->GetMaster(
     ); //(bUseLeft) ?  &pCurrentPageDesc->GetLeft() :
    SwFrmFmt* pHdFtFmt;
    pFmt->SetAttr(SwFmtHeader(TRUE));
    pHdFtFmt = (SwFrmFmt*)pFmt->GetHeader().GetHeaderFmt();
    const SwNodeIndex* pStartIndex = pHdFtFmt->GetCntnt().GetCntntIdx();
    if (!pStartIndex)
        return;
    ASSERT(pSavedPos == NULL, "SwFltShell");
    pSavedPos = new SwPosition(*pPaM->GetPoint());
    pPaM->GetPoint()->nNode = pStartIndex->GetIndex() + 1;
    pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(), 0);
    eSubMode = Header;
}

void SwFltShell::BeginFooter(SwPageDesc* pPD)
{
    SwFrmFmt* pFmt =  &pCurrentPageDesc->GetMaster(
     ); //(bUseLeft) ?  &pCurrentPageDesc->GetLeft() :
    SwFrmFmt* pHdFtFmt;
    pFmt->SetAttr(SwFmtFooter(TRUE));
    pHdFtFmt = (SwFrmFmt*)pFmt->GetFooter().GetFooterFmt();
    const SwNodeIndex* pStartIndex = pHdFtFmt->GetCntnt().GetCntntIdx();
    if (!pStartIndex)
        return;
    ASSERT(pSavedPos == NULL, "SwFltShell");
    pSavedPos = new SwPosition(*pPaM->GetPoint());
    pPaM->GetPoint()->nNode = pStartIndex->GetIndex() + 1;
    pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(), 0);
    eSubMode = Footer;
}

void SwFltShell::EndHeaderFooter()
{
    *pPaM->GetPoint() = *pSavedPos;             // restore Cursor
    delete pSavedPos;
    pSavedPos = 0;
}

SwPageDesc* SwFltShell::MakePageDesc(SwPageDesc* pFirstPageDesc)
{
    if(bStdPD)                      // keine Neuen PageDescs
        return pCurrentPageDesc;

    BOOL bFollow = (pFirstPageDesc != 0);
    SwPageDesc* pNewPD;
    USHORT nPos;
    if (bFollow && pFirstPageDesc->GetFollow() != pFirstPageDesc)
        return pFirstPageDesc;      // Fehler: hat schon Follow
// Erkennung doppelter Namen fehlt noch (Wahrscheinlichkeit
// fuer dopp. Namen ist gering)

// Namen zusammenbasteln
    String aNm( String::CreateFromAscii(
                            RTL_CONSTASCII_STRINGPARAM( "Konvert ")));
    if (bFollow)
        aNm.AppendAscii( "Folge" );
    aNm += String(GetDoc().GetPageDescCnt());
    if (!bFollow)
        nPos = GetDoc().MakePageDesc(aNm, 0);
    else
        nPos = GetDoc().MakePageDesc(aNm,
         pFirstPageDesc);// alles uebernehmen
    pNewPD =  &((SwPageDesc&)GetDoc().GetPageDesc(nPos));
    if (bFollow)
    {               // Dieser ist der folgende von pPageDesc
        pFirstPageDesc->SetFollow(pNewPD);
        pNewPD->SetFollow(pNewPD);
    }
    else
        GetDoc().Insert( *pPaM, SwFmtPageDesc( pNewPD ) );
    pNewPD->WriteUseOn( // alle Seiten
     (UseOnPage)(PD_ALL | PD_HEADERSHARE | PD_FOOTERSHARE));
    return pNewPD;
}

///////////////////////////////////////////////// SwFltFormatCollection
SwFltFormatCollection::SwFltFormatCollection(
    SwDoc& rDoc, RES_POOL_COLLFMT_TYPE nType) :
    SwFltOutBase(rDoc),
    pColl(rDoc.GetTxtCollFromPool(nType)),
    pFlyAttrs( 0 ),
    bHasFly( FALSE )
{
    Reset();            // Default-Attrs loeschen und Auto-Flag
}

SwFltFormatCollection::SwFltFormatCollection(
    SwDoc& rDoc, const String& rName ) :
    SwFltOutBase(rDoc),
    pFlyAttrs( 0 ),
    bHasFly( FALSE )
{
    pColl = rDoc.MakeTxtFmtColl(rName, (SwTxtFmtColl*)rDoc.GetDfltTxtFmtColl());
    Reset();            // Default-Attrs loeschen und Auto-Flag
}

#if 0
SwTxtFmtColl* SwFltFormatCollection::Search(String aName, CharSet eSrc)
{
    aName.Convert(eSrc);
    SwTxtFmtColl* pColl = GetDoc().FindTxtFmtCollByName(aName);
    if(!pColl)
    {
        USHORT n = GetDoc().GetPoolId(aName, GET_POOLID_TXTCOLL);
        if(USHRT_MAX != n)
            pColl = GetDoc().GetTxtCollFromPool(n);
    }
    return pColl;
}
#endif
void SwFltShell::NextStyle(USHORT nWhich, USHORT nNext)
{
        ASSERT(pColls[nWhich], "Next style for noexistent style" );
        ASSERT(pColls[nNext], "Next style to noexistent style" );
        if( pColls[nWhich] && pColls[nNext] )
            pColls[nWhich]->GetColl()->SetNextTxtFmtColl(
                 *pColls[nNext]->GetColl() );
}

/**********************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww1/fltshell.cxx,v 1.1.1.1 2000-09-18 17:14:57 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.57  2000/09/18 16:04:55  willem.vandorp
      OpenOffice header added.

      Revision 1.56  2000/08/30 16:46:59  jp
      use CharClass instead of international

      Revision 1.55  2000/07/28 15:26:54  khz
      #73796# don't delete NumRule from Attr but set it into pDoc

      Revision 1.54  2000/07/25 16:58:34  khz
      #74876# MakeRegion() don't use different Nodes for same Node Index

      Revision 1.53  2000/07/24 16:39:49  khz
      #75701# set Break or PageDesc item BEHIND TOX if not found before the #0x0C

      Revision 1.52  2000/07/12 11:37:05  khz
      #76503# SwFltStackEntry.bOld prevents from killing attributes in ww8par.cxx

      Revision 1.51  2000/06/28 08:10:17  khz
      #70915# new: MoveAttrsToNextTextNode() [see coment in ww8/ww8par.cxx]

      Revision 1.50  2000/06/26 12:53:42  os
      INetURLObject::SmartRelToAbs removed

      Revision 1.49  2000/06/13 11:22:33  os
      SUPD removed

      Revision 1.48  2000/06/13 09:38:40  os
      using UCB

      Revision 1.47  2000/05/26 11:41:20  hr
      SwFltShell& SwFltShell::operator << ( const sal_Unicode c )

      Revision 1.46  2000/05/11 16:06:41  jp
      Changes for Unicode

      Revision 1.45  2000/02/11 14:39:29  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.44  1999/11/25 17:00:35  khz
      Task #70133# new method SwFltControlStack::KillUnlockedAttrs()

      Revision 1.43  1999/11/25 16:38:15  khz
      Task #70133# new method SwFltControlStack::KillUnlockedAttrs()

      Revision 1.42  1999/10/29 12:08:16  khz
      import new TOX_CONTENT and TOX_INDEX features

      Revision 1.41  1999/10/25 19:35:23  tl
      ongoing ONE_LINGU implementation

      Revision 1.40  1999/10/08 09:29:10  khz
      Import Redlining

      Revision 1.39  1999/04/27 12:19:10  KHZ
      Task #65353# Section ins Doc einfuegen nur mit Parameter bUpdate = FALSE


      Rev 1.38   27 Apr 1999 14:19:10   KHZ
   Task #65353# Section ins Doc einfuegen nur mit Parameter bUpdate = FALSE

      Rev 1.37   12 May 1998 15:52:14   JP
   rund um Flys/DrawObjs im Doc/FESh umgestellt/optimiert

      Rev 1.36   27 Feb 1998 19:12:42   HJS
   SH 27.02.98

      Rev 1.33   22 Jan 1998 20:06:44   JP
   CTOR des SwPaM umgestellt

      Rev 1.32   02 Dec 1997 17:25:56   MA
   includes

      Rev 1.31   29 Nov 1997 17:37:12   MA
   includes

      Rev 1.30   17 Nov 1997 09:57:14   JP
   Umstellung Numerierung

      Rev 1.29   30 Oct 1997 18:17:16   AMA
   Chg: Kein AutoFlag mehr an Break bzw. PageDesc-Attributen

      Rev 1.28   09 Oct 1997 14:14:32   JP
   Aenderungen von SH und Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.27   02 Oct 1997 15:26:00   OM
   Feldumstellung

      Rev 1.26   12 Sep 1997 11:17:24   OS
   ITEMID_* definiert

      Rev 1.25   04 Sep 1997 11:18:00   JP
   include

      Rev 1.24   03 Sep 1997 14:17:32   SH
   Flys, Tabellen ok, Style-Abhaengigkeiten u.v.a.m

      Rev 1.23   15 Aug 1997 12:50:36   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.22   12 Aug 1997 14:16:32   OS
   Header-Umstellung

      Rev 1.21   11 Aug 1997 14:07:40   OM
   Headerfile-Umstellung

      Rev 1.20   11 Aug 1997 12:50:34   SH
   Flys und einfache Tabellen und Tabulatoren

      Rev 1.19   18 Jul 1997 09:31:36   MA
   Stand von SH, #41738# GPF und Tabellen etwas besser

      Rev 1.16   11 Jul 1997 14:54:44   SH
   #40772#, #41459#

      Rev 1.15   16 Jun 1997 13:08:58   MA
   Stand Micheal Dietrich

      Rev 1.13   27 Apr 1997 16:27:28   SH
   Sonderwunsch Hypo: Ref-Feld auf Frage-Feld

      Rev 1.12   01 Apr 1997 22:41:04   SH
   MoveAttrs fuer Hypo-Bookmarks

      Rev 1.11   12 Mar 1997 19:13:06   SH
   MDT: Progressbar, Pagedesc-Bug, Spalten, Anfang Tabellen u.a.

      Rev 1.10   24 Jan 1997 19:49:32   SH
   Bookmarks verschraenkt und verschachtelt

      Rev 1.9   22 Jan 1997 12:01:52   JP
   neu: Tabellen Box Attribute

      Rev 1.8   10 Jan 1997 18:42:00   SH
   Stabiler und mehr von MDT

      Rev 1.7   28 Nov 1996 18:07:52   SH
   Schneller Schoener Weiter von MDT

      Rev 1.6   29 Oct 1996 13:11:24   JP
   am Doc ist das NodesArray nur noch ueber Get..() zugaenglich

      Rev 1.5   24 Oct 1996 17:16:44   JP
   Optimierung: Find...ByName

      Rev 1.4   09 Oct 1996 13:08:14   JP
   SwNodes::GoNext/-Prev (-Sect) returnt statt BOOL den gefundenen ContentNode

      Rev 1.3   08 Oct 1996 22:09:18   SH
   Zusammenfassung Anchor + GrafAnchor

      Rev 1.2   30 Sep 1996 23:20:30   SH
   neu von MDA

**********************************************************************/
//)}]
