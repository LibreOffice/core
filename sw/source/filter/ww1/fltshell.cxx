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
#include "precompiled_sw.hxx"

#include <ctype.h>
#include <hintids.hxx>
#include <hints.hxx>
#include <svtools/filter.hxx>

#include <vcl/graph.hxx>
#include <svl/urihelper.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crsditem.hxx>
#include <svl/stritem.hxx>
#include <unotools/charclass.hxx>
#include <txtftn.hxx>
#include <fmtpdsc.hxx>
#include <fmtftn.hxx>
#include <fmtanchr.hxx>
#include <fmtrfmrk.hxx>
#include <fmtclds.hxx>
#include <fmtfld.hxx>
#include <fmtfsize.hxx>
#include <fmthdft.hxx>
#include <fmtcntnt.hxx>
#include <redline.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <frmatr.hxx>
#include <fldbas.hxx>           // RES_SETEXPFLD
#include <charatr.hxx>          // class SwFmtRefMark
#include <swtable.hxx>          // class SwTableLines, ...
#include <tox.hxx>
#include <expfld.hxx>           // SwExpField
#include <section.hxx>          // class SwSection
#include <tblsel.hxx>           // class SwSelBoxes
#include <pagedesc.hxx>
#include <docsh.hxx>            // class SwDocSh
#include <fltshell.hxx>
#include <viewsh.hxx>
#include <shellres.hxx>


#define MAX_FIELDLEN 64000

using namespace com::sun::star;

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
    bConsumedByField = FALSE;
}

SwFltStackEntry::SwFltStackEntry(const SwFltStackEntry& rEntry) :
    nMkNode(rEntry.nMkNode),
    nPtNode(rEntry.nPtNode)
{
    pAttr   = rEntry.pAttr->Clone();
    nMkCntnt= rEntry.nMkCntnt;
    bOld    = rEntry.bOld;
    bLocked = bCopied = TRUE; // when rEntry were NOT bLocked we would never have been called
    bConsumedByField = rEntry.bConsumedByField;
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
    if (
         nMkNode.GetIndex() == nPtNode.GetIndex() && nMkCntnt == nPtCntnt &&
         nPtCntnt && RES_TXTATR_FIELD != pAttr->Which()
       )
    {
        return FALSE;
    }

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
#if OSL_DEBUG_LEVEL > 1
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

SwFltControlStack::SwFltControlStack(SwDoc* pDo, ULONG nFieldFl)
    : nFieldFlags(nFieldFl), pDoc(pDo), bIsEndStack(false)
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
    USHORT nCnt = static_cast< USHORT >(Count());
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

void SwFltControlStack::MarkAllAttrsOld()
{
    USHORT nCnt = static_cast< USHORT >(Count());
    for (USHORT i=0; i < nCnt; i++)
        (*this)[ i ]->bOld = TRUE;
}

void SwFltControlStack::NewAttr(const SwPosition& rPos, const SfxPoolItem & rAttr )
{
    SwFltStackEntry *pTmp = new SwFltStackEntry(rPos, rAttr.Clone() );
    USHORT nWhich = pTmp->pAttr->Which();
    SetAttr(rPos, nWhich);// Ende von evtl. gleichen Attributen auf dem Stack
                                // Setzen, damit sich die Attribute nicht auf
                                // dem Stack haeufen
    maEntries.push_back(pTmp);
}

void SwFltControlStack::DeleteAndDestroy(Entries::size_type nCnt)
{
    ASSERT(nCnt < maEntries.size(), "Out of range!");
    if (nCnt < maEntries.size())
    {
        myEIter aElement = maEntries.begin() + nCnt;
        delete *aElement;
        maEntries.erase(aElement);
    }
}

// SwFltControlStack::StealAttr() loescht Attribute des angegebenen Typs vom Stack.
// Als nAttrId sind erlaubt: 0 fuer alle, oder ein spezieller Typ.
// Damit erscheinen sie nicht in der Doc-Struktur. Dabei werden nur die
// Attribute entfernt, die im selben Absatz wie pPos stehen.
// Wird fuer Grafik-Apos -> Grafiken benutzt.
void SwFltControlStack::StealAttr(const SwPosition* pPos, USHORT nAttrId /* = 0 */)
{
    USHORT nCnt = static_cast< USHORT >(Count());

    SwFltStackEntry* pEntry;

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

    USHORT nCnt = static_cast< USHORT >(Count());
    SwFltStackEntry* pEntry;
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
void SwFltControlStack::SetAttr(const SwPosition& rPos, USHORT nAttrId,
                                BOOL bTstEnde, long nHand, BOOL consumedByField )
{
    ASSERT(!nAttrId ||
        (POOLATTR_BEGIN <= nAttrId && POOLATTR_END > nAttrId) ||
        (RES_FLTRATTR_BEGIN <= nAttrId && RES_FLTRATTR_END > nAttrId),
        "Falsche Id fuers Attribut")

    USHORT nCnt = static_cast< USHORT >(Count());

    SwFltStackEntry* pEntry;

    for (USHORT i=0; i < nCnt; i++)
    {
        pEntry = (*this)[ i ];
        if (pEntry->bLocked)
        {
            // setze das Ende vom Attribut
            bool bF = false;
            if (!nAttrId ){
                bF = true;
            }else if( nAttrId == pEntry->pAttr->Which()){
                if( nAttrId != RES_FLTR_BOOKMARK ){     // Handle abfragen
                    bF = true;
                }else if( nHand == ((SwFltBookmark*)(pEntry->pAttr))->GetHandle() )
                {
                    bF = true;
                }
            }
            if (bF) {
                pEntry->bConsumedByField = consumedByField;
                pEntry->SetEndPos(rPos);
            }
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

#if OSL_DEBUG_LEVEL > 1
extern BOOL CheckNodesRange( const SwNodeIndex& rStt,
                    const SwNodeIndex& rEnd, BOOL bChkSection );
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
            SwFrmFmt* pFmt = ((SwFltAnchor*)pEntry->pAttr)->GetFrmFmt();
            if (pFmt != NULL)
            {
                MakePoint(pEntry, pDoc, aRegion);
                SwFmtAnchor aAnchor(pFmt->GetAnchor());
                aAnchor.SetAnchor(aRegion.GetPoint());
                pFmt->SetFmtAttr(aAnchor);
                // Damit die Frames bei Einfuegen in existierendes Doc
                //  erzeugt werden (erst nach Setzen des Ankers!):
                if(pDoc->GetRootFrm()
                   && (FLY_AT_PARA == pFmt->GetAnchor().GetAnchorId()))
                {
                    pFmt->MakeFrms();
                }
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
                        // --> OD 2008-03-17 #refactorlists#
                        // no start of a new list
                        pDoc->SetNumRule( aTmpPam, *pRul, false );
                        // <--

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
                    pDoc->InsertPoolItem(aRegion, SwFmtRefMark(rName), 0);
                }
                else if( !pB->IsOnlyRef() )
                {
                    SwFieldType* pFT = pDoc->GetFldType(RES_SETEXPFLD, rName, false);
                    if (!pFT)
                    {                       // FieldType anlegen
                        SwSetExpFieldType aS(pDoc, rName, nsSwGetSetExpType::GSE_STRING);
                        pFT = pDoc->InsertFldType(aS);
                    }
                    SwSetExpField aFld((SwSetExpFieldType*)pFT,
                                        pB->GetValSys());
                    aFld.SetSubType( nsSwExtendedSubType::SUB_INVISIBLE );
                    MakePoint(pEntry, pDoc, aRegion);
                    pDoc->InsertPoolItem(aRegion, SwFmtFld(aFld), 0);
                    MoveAttrs( *(aRegion.GetPoint()) );
                }
            }
            if( !pB->IsOnlyRef() &&
                ( !IsFlagSet(HYPO) || IsFlagSet(BOOK_AND_REF) ) && !pEntry->bConsumedByField)
            {
                MakeBookRegionOrPoint(pEntry, pDoc, aRegion, TRUE);
                pDoc->getIDocumentMarkAccess()->makeMark( aRegion, rName, IDocumentMarkAccess::BOOKMARK);
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

            delete pTOXAttr->GetBase();

            // set (aboved saved and removed) the break item at the node following the TOX
            if( aBkSet.Count() )
                pNd->SetAttr( aBkSet );
        }
        break;
    case RES_FLTR_SECTION:
        MakePoint(pEntry, pDoc, aRegion);   // bislang immer Point==Mark
        pDoc->InsertSwSection(aRegion,
                *(static_cast<SwFltSection*>(pEntry->pAttr))->GetSectionData(),
                0, 0, false);
        delete (((SwFltSection*)pEntry->pAttr)->GetSectionData());
        break;
    case RES_FLTR_REDLINE:
        {
            if (pEntry->MakeRegion(pDoc, aRegion, TRUE))
            {
              pDoc->SetRedlineMode((RedlineMode_t)(   nsRedlineMode_t::REDLINE_ON
                                              | nsRedlineMode_t::REDLINE_SHOW_INSERT
                                              | nsRedlineMode_t::REDLINE_SHOW_DELETE ));
                SwFltRedline& rFltRedline = *((SwFltRedline*)pEntry->pAttr);

                if( USHRT_MAX != rFltRedline.nAutorNoPrev )
                {
                    SwRedlineData aData(rFltRedline.eTypePrev,
                                        rFltRedline.nAutorNoPrev,
                                        rFltRedline.aStampPrev,
                                        aEmptyStr,
                                        0
                                        );
                    pDoc->AppendRedline(new SwRedline(aData, aRegion), true);
                }
                SwRedlineData aData(rFltRedline.eType,
                                    rFltRedline.nAutorNo,
                                    rFltRedline.aStamp,
                                    aEmptyStr,
                                    0
                                    );
                pDoc->AppendRedline( new SwRedline(aData, aRegion), true );
                pDoc->SetRedlineMode((RedlineMode_t)( nsRedlineMode_t::REDLINE_NONE
                                                | nsRedlineMode_t::REDLINE_SHOW_INSERT
                                                | nsRedlineMode_t::REDLINE_SHOW_DELETE ));
            }
        }
        break;
    default:
        if (pEntry->MakeRegion(pDoc, aRegion, FALSE))
        {
            pDoc->InsertPoolItem(aRegion, *pEntry->pAttr, 0);
        }
        break;
    }
}

SfxPoolItem* SwFltControlStack::GetFmtStackAttr(USHORT nWhich, USHORT * pPos)
{
    SwFltStackEntry* pEntry;
    USHORT nSize = static_cast< USHORT >(Count());

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

void SwFltControlStack::Delete(const SwPaM &rPam)
{
    const SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();

    if( !rPam.HasMark() || *pStt >= *pEnd )
        return;

    SwNodeIndex aStartNode(pStt->nNode, -1);
    USHORT nStartIdx = pStt->nContent.GetIndex();
    SwNodeIndex aEndNode(pEnd->nNode, -1);
    USHORT nEndIdx = pEnd->nContent.GetIndex();

    //We don't support deleting content that is over one node, or removing a node.
    ASSERT(aEndNode == aStartNode, "nodes must be the same, or this method extended");
    if (aEndNode != aStartNode)
        return;

    for (USHORT nSize = static_cast< USHORT >(Count()); nSize > 0;)
    {
        SwFltStackEntry* pEntry = (*this)[--nSize];

        bool bEntryStartAfterSelStart =
            (pEntry->nMkNode == aStartNode && pEntry->nMkCntnt >= nStartIdx);

        bool bEntryStartBeforeSelEnd =
            (pEntry->nMkNode == aEndNode && pEntry->nMkCntnt <= nEndIdx);

        bool bEntryEndAfterSelStart = false;
        bool bEntryEndBeforeSelEnd = false;
        if (!pEntry->bLocked)
        {
            bEntryEndAfterSelStart =
                (pEntry->nPtNode == aStartNode && pEntry->nPtCntnt >= nStartIdx);

            bEntryEndBeforeSelEnd =
                (pEntry->nPtNode == aEndNode && pEntry->nPtCntnt <= nEndIdx);
        }

        bool bTotallyContained = false;
        if (
             bEntryStartAfterSelStart && bEntryStartBeforeSelEnd &&
             bEntryEndAfterSelStart && bEntryEndBeforeSelEnd
           )
        {
           bTotallyContained = true;
        }

        if (bTotallyContained)
        {
            //after start, before end, delete
            DeleteAndDestroy(nSize);
            continue;
        }

        xub_StrLen nCntntDiff = nEndIdx - nStartIdx;

        //to be adjusted
        if (bEntryStartAfterSelStart)
        {
            if (bEntryStartBeforeSelEnd)
            {
                //move start to new start
                pEntry->nMkNode = aStartNode;
                pEntry->nMkCntnt = nStartIdx;
            }
            else
                pEntry->nMkCntnt = pEntry->nMkCntnt - nCntntDiff;
        }

        if (bEntryEndAfterSelStart)
        {
            if (bEntryEndBeforeSelEnd)
            {
                pEntry->nPtNode = aStartNode;
                pEntry->nPtCntnt = nStartIdx;
            }
            else
                pEntry->nPtCntnt = pEntry->nPtCntnt - nCntntDiff;
        }

        //That's what locked is, end equal to start, and nPtCntnt is invalid
        if (pEntry->bLocked)
            pEntry->nPtNode = pEntry->nMkNode;
    }
}

//------ hier stehen die Methoden von SwFltAnchor -----------
SwFltAnchor::SwFltAnchor(SwFrmFmt* pFmt) :
    SfxPoolItem(RES_FLTR_ANCHOR), pFrmFmt(pFmt)
{
    pClient = new SwFltAnchorClient(this);
    pFrmFmt->Add(pClient);
}

SwFltAnchor::SwFltAnchor(const SwFltAnchor& rCpy) :
    SfxPoolItem(RES_FLTR_ANCHOR), pFrmFmt(rCpy.pFrmFmt)
{
    pClient = new SwFltAnchorClient(this);
    pFrmFmt->Add(pClient);
}

SwFltAnchor::~SwFltAnchor()
{
    delete pClient;
}

void SwFltAnchor::SetFrmFmt(SwFrmFmt * _pFrmFmt)
{
    pFrmFmt = _pFrmFmt;
}

const SwFrmFmt * SwFltAnchor::GetFrmFmt() const
{
    return pFrmFmt;
}

SwFrmFmt * SwFltAnchor::GetFrmFmt()
{
    return pFrmFmt;
}

int SwFltAnchor::operator==(const SfxPoolItem& rItem) const
{
    return pFrmFmt == ((SwFltAnchor&)rItem).pFrmFmt;
}

SfxPoolItem* __EXPORT SwFltAnchor::Clone(SfxItemPool*) const
{
    return new SwFltAnchor(*this);
}

// SwFltAnchorClient

SwFltAnchorClient::SwFltAnchorClient(SwFltAnchor * pFltAnchor)
: m_pFltAnchor(pFltAnchor)
{
}

void  SwFltAnchorClient::Modify(SfxPoolItem *, SfxPoolItem * pNew)
{
    if (pNew->Which() == RES_FMT_CHG)
    {
        SwFmtChg * pFmtChg = dynamic_cast<SwFmtChg *> (pNew);

        if (pFmtChg != NULL)
        {
            SwFrmFmt * pFrmFmt = dynamic_cast<SwFrmFmt *> (pFmtChg->pChangedFmt);

            if (pFrmFmt != NULL)
                m_pFltAnchor->SetFrmFmt(pFrmFmt);
        }
    }
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

SwFltSection::SwFltSection(SwSectionData *const pSect)
    : SfxPoolItem(RES_FLTR_SECTION)
    , m_pSection(pSect)
{
}

SwFltSection::SwFltSection(const SwFltSection& rCpy)
    : SfxPoolItem(RES_FLTR_SECTION)
    , m_pSection(rCpy.m_pSection)
{
}

int SwFltSection::operator==(const SfxPoolItem& rItem) const
{
    return m_pSection == ((SwFltSection&)rItem).m_pSection;
}

SfxPoolItem* __EXPORT SwFltSection::Clone(SfxItemPool*) const
{
    return new SwFltSection(*this);
}

///////////////////////////////////////////////////////////////////////
//
// hier beginnt der von mdt erzeugte code. dieser ist eine shell auf
// der writer-seite nach moeglichkeit bald fuer alle filter. die ganze
// schwierigkeit, texte & formatattribute einzufuegen, die positionen
// zu verwalten, styles & kopf/fuszzeilen etc.
//

//////////////////////////////////////////////////////////// SwFltShell
SwFltShell::SwFltShell(SwDoc* pDoc, SwPaM& rPaM, const String& rBaseURL, BOOL bNew, ULONG nFieldFl) :
    pCurrentPageDesc(0),
    pSavedPos(0),
    eSubMode(None),
    nAktStyle(0),
    aStack(pDoc, nFieldFl),
    aEndStack(pDoc, nFieldFl),
    pPaM(new SwPaM(*(rPaM.GetPoint()))),
    sBaseURL(rBaseURL),
    nPageDescOffset(GetDoc().GetPageDescCnt()),
    eSrcCharSet(RTL_TEXTENCODING_MS_1252),
    bNewDoc(bNew),
    bStdPD(FALSE),
    bProtect(FALSE)
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
            pDoc->SplitNode( *pPos, false );        // neue Zeile erzeugen
        if( pSttNd->GetTxt().Len() ){       // EinfuegePos nicht am Ende der Zeile
            pDoc->SplitNode( *pPos, false );        // neue Zeile
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
    pCurrentPageDesc =  &((SwPageDesc&)const_cast<const SwDoc *>(pDoc)
                          ->GetPageDesc( 0 ));  // Standard

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

    GetDoc().SetUpdateExpFldStat(true);
    GetDoc().SetInitDBFields(TRUE);
    aStack.SetAttr(*pPaM->GetPoint(), 0, FALSE);
    aStack.SetAttr(*pPaM->GetPoint(), 0, FALSE);
    aEndStack.SetAttr(*pPaM->GetPoint(), 0, FALSE);
    aEndStack.SetAttr(*pPaM->GetPoint(), 0, FALSE);
    if( bProtect ){     // Das ganze Doc soll geschuetzt sein

        SwDoc& rDoc = GetDoc();
                        // 1. SectionFmt und Section anlegen
        SwSectionFmt* pSFmt = rDoc.MakeSectionFmt( 0 );
        SwSectionData aSectionData( CONTENT_SECTION, String::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM("PMW-Protect") ));
        aSectionData.SetProtectFlag( true );
                        // 2. Start- und EndIdx suchen
        const SwNode* pEndNd = &rDoc.GetNodes().GetEndOfContent();
        SwNodeIndex aEndIdx( *pEndNd, -1L );
        const SwStartNode* pSttNd = pEndNd->StartOfSectionNode();
        SwNodeIndex aSttIdx( *pSttNd, 1L );         // +1 -> hinter StartNode
                                                    // Section einfuegen
                        // Section einfuegen
        rDoc.GetNodes().InsertTextSection(
                aSttIdx, *pSFmt, aSectionData, 0, &aEndIdx, false );

        if( !IsFlagSet(SwFltControlStack::DONT_HARD_PROTECT) ){
            SwDocShell* pDocSh = rDoc.GetDocShell();
            if( pDocSh )
                pDocSh->SetReadOnlyUI( TRUE );
        }
    }
        // Pagedescriptoren am Dokument updaten (nur so werden auch die
        // linken Seiten usw. eingestellt).

    GetDoc().ChgPageDesc( 0,
                          const_cast<const SwDoc &>(GetDoc()).
                          GetPageDesc( 0 ));    // PageDesc "Standard"
    for (i=nPageDescOffset;i<GetDoc().GetPageDescCnt();i++)
    {
        const SwPageDesc& rPD = const_cast<const SwDoc &>(GetDoc()).
            GetPageDesc(i);
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
    GetDoc().InsertString( *pPaM, rStr );
    return *this;
}

void SwFltShell::ConvertUStr( String& rInOut )
{
    GetAppCharClass().toUpper( rInOut );
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
    GetDoc().InsertString( *pPaM, c );
    return *this;
}

SwFltShell& SwFltShell::AddError( const sal_Char* pErr )
{
    String aName( String::CreateFromAscii(
                    RTL_CONSTASCII_STRINGPARAM( "ErrorTag" )));
    SwFieldType* pFT = GetDoc().GetFldType( RES_SETEXPFLD, aName, false );
    if( pFT == 0)
    {
        SwSetExpFieldType aS(&GetDoc(), aName, nsSwGetSetExpType::GSE_STRING);
        pFT = GetDoc().InsertFldType(aS);
    }
    SwSetExpField aFld( (SwSetExpFieldType*)pFT,
                        String::CreateFromAscii( pErr ));
    //, VVF_INVISIBLE
    GetDoc().InsertPoolItem(*pPaM, SwFmtFld(aFld), 0);
    return *this;
}

SwFltShell& SwFltShell::operator << (Graphic& rGraphic)
{
    // embedded Grafik !!
    GetDoc().Insert(*pPaM, aEmptyStr, aEmptyStr, &rGraphic, NULL, NULL, NULL);
    return *this;
}

void SwFltShell::NextParagraph()
{
    GetDoc().AppendTxtNode(*pPaM->GetPoint());
}

void SwFltShell::NextPage()
{
    NextParagraph();
    GetDoc().InsertPoolItem(*pPaM,
        SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE, RES_BREAK), 0);
}

SwFltShell& SwFltShell::AddGraphic( const String& rPicName )
{
    // embedded:
    GraphicFilter* pFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic;
    // one of: GFF_NOT GFF_BMP GFF_GIF GFF_JPG GFF_PCD GFF_PCX GFF_PNG
    // GFF_TIF GFF_XBM GFF_DXF GFF_MET GFF_PCT GFF_SGF GFF_SVM GFF_WMF
    // GFF_SGV GFF_XXX
    INetURLObject aDir(
        URIHelper::SmartRel2Abs(
            INetURLObject(GetBaseURL()), rPicName,
            URIHelper::GetMaybeFileHdl()) );
    switch (pFilter->ImportGraphic(aGraphic, aDir))
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
    return *this;
}

SwFltShell& SwFltShell::SetStyle( USHORT nStyle )
{
    SwFltFormatCollection* p = pColls[ nStyle ];

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
    GetDoc().InsertPoolItem(*pPaM, SwFmtFld(rField), 0);
    return *this;
}

/*virtual*/ SwFltOutBase& SwFltOutDoc::operator << (const SfxPoolItem& rItem)
{
    rStack.NewAttr(*pPaM->GetPoint(), rItem);
    return *this;
}

/*virtual*/ SwFltOutBase& SwFltFormatCollection::operator <<
                                (const SfxPoolItem& rItem)
{
    pColl->SetFmtAttr(rItem);
    return *this;
}

const SfxPoolItem& SwFltOutDoc::GetAttr(USHORT nWhich)
{
    return *rStack.GetFmtAttr(*pPaM->GetPoint(), nWhich);
}

const SfxPoolItem& SwFltFormatCollection::GetAttr(USHORT nWhich)
{
    return GetColl()->GetFmtAttr(nWhich);   // mit Parents
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
    return GetColl()->GetFmtAttr(nWhich);   // mit Parents
}

const SfxPoolItem& SwFltShell::GetNodeOrStyAttr(USHORT nWhich)
{
    return pOut->GetNodeOrStyAttr( nWhich );
}

const SfxPoolItem& SwFltShell::GetAttr(USHORT nWhich)
{
    return pOut->GetAttr( nWhich );
}

const SfxPoolItem& SwFltShell::GetFlyFrmAttr(USHORT nWhich)
{
    return pOut->GetFlyFrmAttr( nWhich );
}

SwFieldType* SwFltShell::GetSysFldType(USHORT eWhich)
{
    return GetDoc().GetSysFldType(eWhich);
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

SwFltOutBase::~SwFltOutBase()
{
}

SwFltOutBase::SwFltOutBase(SwDoc& rDocu)
    : rDoc(rDocu), eFlyAnchor(FLY_AT_PARA), bFlyAbsPos(false)
{
}

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

void SwFltOutBase::SetTableWidth(SwTwips /*nW*/)
{
    ASSERT(FALSE, "SetTableWidth ausserhalb von normalem Text");
}

void SwFltOutBase::SetTableOrient(sal_Int16 /*eOri*/)
{
    ASSERT(FALSE, "SetTableOrient ausserhalb von normalem Text");
}

void SwFltOutBase::SetCellWidth(SwTwips /*nWidth*/, USHORT /*nCell*/)
{
    ASSERT(FALSE, "SetCellWidth ausserhalb von normalem Text");
}

void SwFltOutBase::SetCellHeight(SwTwips /*nH*/)
{
    ASSERT(FALSE, "SetCellHeight ausserhalb von normalem Text");
}

void SwFltOutBase::SetCellBorder(const SvxBoxItem& /*rFmtBox*/, USHORT /*nCell*/)
{
    ASSERT(FALSE, "SetCellBorder ausserhalb von normalem Text");
}

void SwFltOutBase::SetCellSpace(USHORT /*nSp*/)
{
    ASSERT(FALSE, "SetCellSpace ausserhalb von normalem Text");
}

void SwFltOutBase::DeleteCell(USHORT /*nCell*/)
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
    pTable = GetDoc().InsertTable(
            SwInsertTableOptions( tabopts::HEADLINE_NO_BORDER, 1 ),
            *pTabSavedPos, 1, 1, text::HoriOrientation::LEFT, 0, 0, FALSE, FALSE ); // TODO MULTIHEADER
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
         GetDoc().GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false ),
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
    if (pTableBox)
    {
// duplicate row:
        SwSelBoxes aSelBoxes;
        aSelBoxes.Insert( pTableBox );
        GetDoc().InsertRow(aSelBoxes);
        usTableX = 0;
        SeekCell(++usTableY, usTableX, TRUE);
        GetDoc().SetTxtFmtColl(*pPaM,
            GetDoc().GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false ));
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
        pTable->GetFrmFmt()->SetFmtAttr( SwFmtFrmSize(ATT_VAR_SIZE, nSwWidth));
        nTableWidth = nSwWidth;
    }
}

void SwFltOutDoc::SetTableOrient(sal_Int16 eOri)
{
    if(!pTable){
        ASSERT(pTable, "SetTableOrient ohne Tabelle");
        return;
    }
    pTable->GetFrmFmt()->SetFmtAttr( SwFmtHoriOrient( 0, eOri ));
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
        pTableBox->GetFrmFmt()->SetFmtAttr(aFmtFrmSize);
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
    pTableLine->GetFrmFmt()->SetFmtAttr(aFmtFrmSize);
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
    return pTableBox->GetFrmFmt()->GetFmtAttr( nWhich );
}

void SwFltOutDoc::SetCellBorder(const SvxBoxItem& rFmtBox,
                               USHORT nCell /* = USHRT_MAX */ )
{
    SwTableBox* pTableBox = GetBox(usTableY, nCell);
    if(pTableBox)
        pTableBox->GetFrmFmt()->SetFmtAttr(rFmtBox);
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
                        &pTableBox->GetFrmFmt()->GetFmtAttr( RES_BOX )));

    // versteh ich nich, sven: if (!nDist) nDist = 18; // ca. 0.03 cm
    if (nDist > 42) // max. 0.7 mm
        nDist = 42;
    else
        if (nDist < MIN_BORDER_DIST)
            nDist = MIN_BORDER_DIST;
    aFmtBox.SetDistance(nDist);
    pTableBox->GetFrmFmt()->SetFmtAttr(aFmtBox);
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
    if(!pTable)
    {
        ASSERT(pTable, "SplitTable ohne Tabelle");
        return;
    }
    SwTableBox* pAktBox = GetBox(usTableY, usTableX);
    SwTableBox* pSplitBox = GetBox(usTableY - 1, 0);
    GetDoc().GetNodes().SplitTable(SwNodeIndex(*pSplitBox->GetSttNd()), false);
    pTable = &pAktBox->GetSttNd()->FindTableNode()->GetTable();
    usTableY = 0;
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
#if OSL_DEBUG_LEVEL > 1
        const SwTxtFmtColl* p = GetDoc().GetDfltTxtFmtColl();
        p = GetDoc().GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false );
#endif
        GetDoc().SetTxtFmtColl(*pPaM,
            GetDoc().GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false ));
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
    p->Put( SwFmtHoriOrient( 0, text::HoriOrientation::NONE, text::RelOrientation::FRAME ));
    return p;
}

BOOL SwFltOutBase::BeginFly( RndStdIds eAnchor /*= FLY_AT_PARA*/,
                           BOOL bAbsolutePos /*= FALSE*/,
                           const SfxItemSet*
#ifdef DBG_UTIL
                               pMoreAttrs /*= 0*/
#endif
                            )
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
    if ( eAnchor == FLY_AS_CHAR ){
        ASSERT( FALSE, "SetFlyAnchor( FLY_AS_CHAR ) nicht implementiert" );
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

BOOL SwFltOutDoc::BeginFly( RndStdIds eAnchor /*= FLY_AT_PARA*/,
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
    if (pFly){
        pFly->SetFmtAttr( rAttr );
    }else{
        ASSERT(pFly, "SetFlyAttr ohne Doc-Fly");
        return;
    }
}

/*virtual*/ const SfxPoolItem& SwFltOutDoc::GetFlyFrmAttr(USHORT nWhich)
{
    if (pFly){
        return pFly->GetFmtAttr( nWhich );
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

BOOL SwFltFormatCollection::BeginFly( RndStdIds eAnchor /*= FLY_AT_PARA*/,
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

BOOL SwFltShell::BeginFly( RndStdIds eAnchor /*= FLY_AT_PARA*/,
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

void SwFltShell::SetFlyXPos( short nXPos, sal_Int16 eHRel /*= text::RelOrientation::FRAME*/,
                             sal_Int16 eHAlign /*= text::HoriOrientation::NONE*/ )
{
    SetFlyFrmAttr( SwFmtHoriOrient( nXPos, eHAlign, eHRel ) );
}

void SwFltShell::SetFlyYPos( short nYPos, sal_Int16 eVRel /*= text::RelOrientation::FRAME*/,
                             sal_Int16 eVAlign /*= text::VertOrientation::NONE*/ )
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
    GetDoc().InsertPoolItem(*pPaM, aFtn, 0);
    ASSERT(pSavedPos == NULL, "SwFltShell");
    pSavedPos = new SwPosition(*pPaM->GetPoint());
    pPaM->Move(fnMoveBackward, fnGoCntnt);
    SwTxtNode* pTxt = pPaM->GetNode()->GetTxtNode();
    SwTxtAttr *const pFN = pTxt->GetTxtAttrForCharAt(
        pPaM->GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN);
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

void SwFltShell::BeginHeader(SwPageDesc* /*pPD*/)
{
    SwFrmFmt* pFmt = &pCurrentPageDesc->GetMaster(
     ); //(bUseLeft) ?  &pCurrentPageDesc->GetLeft() :
    SwFrmFmt* pHdFtFmt;
    pFmt->SetFmtAttr(SwFmtHeader(TRUE));
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

void SwFltShell::BeginFooter(SwPageDesc* /*pPD*/)
{
    SwFrmFmt* pFmt =  &pCurrentPageDesc->GetMaster(
     ); //(bUseLeft) ?  &pCurrentPageDesc->GetLeft() :
    SwFrmFmt* pHdFtFmt;
    pFmt->SetFmtAttr(SwFmtFooter(TRUE));
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

    nPos = GetDoc().MakePageDesc( ViewShell::GetShellRes()->GetPageDescName(
                                   GetDoc().GetPageDescCnt(), FALSE, bFollow ),
                                pFirstPageDesc, FALSE );

    pNewPD =  &((SwPageDesc&)const_cast<const SwDoc &>(GetDoc()).
                GetPageDesc(nPos));
    if (bFollow)
    {               // Dieser ist der folgende von pPageDesc
        pFirstPageDesc->SetFollow(pNewPD);
        pNewPD->SetFollow(pNewPD);
    }
    else
    {
        GetDoc().InsertPoolItem( *pPaM, SwFmtPageDesc( pNewPD ), 0 );
    }
    pNewPD->WriteUseOn( // alle Seiten
     (UseOnPage)(nsUseOnPage::PD_ALL | nsUseOnPage::PD_HEADERSHARE | nsUseOnPage::PD_FOOTERSHARE));
    return pNewPD;
}

///////////////////////////////////////////////// SwFltFormatCollection
SwFltFormatCollection::SwFltFormatCollection(
    SwDoc& _rDoc, RES_POOL_COLLFMT_TYPE nType ) :
    SwFltOutBase(_rDoc),
    pColl(_rDoc.GetTxtCollFromPool( static_cast< sal_uInt16 >(nType), false )),
    pFlyAttrs( 0 ),
    bHasFly( FALSE )
{
    Reset();            // Default-Attrs loeschen und Auto-Flag
}

SwFltFormatCollection::SwFltFormatCollection(
    SwDoc& _rDoc, const String& rName ) :
    SwFltOutBase(_rDoc),
    pFlyAttrs( 0 ),
    bHasFly( FALSE )
{
    pColl = _rDoc.MakeTxtFmtColl(rName, (SwTxtFmtColl*)_rDoc.GetDfltTxtFmtColl());
    Reset();            // Default-Attrs loeschen und Auto-Flag
}

void SwFltShell::NextStyle(USHORT nWhich, USHORT nNext)
{
        ASSERT(pColls[nWhich], "Next style for noexistent style" );
        ASSERT(pColls[nNext], "Next style to noexistent style" );
        if( pColls[nWhich] && pColls[nNext] )
            pColls[nWhich]->GetColl()->SetNextTxtFmtColl(
                 *pColls[nNext]->GetColl() );
}

// UpdatePageDescs muss am Ende des Einlesevorganges aufgerufen werden, damit
// der Writer den Inhalt der Pagedescs wirklich akzeptiert
void UpdatePageDescs(SwDoc &rDoc, sal_uInt16 nInPageDescOffset)
{
    // Pagedescriptoren am Dokument updaten (nur so werden auch die
    // linken Seiten usw. eingestellt).

    // PageDesc "Standard"
    rDoc.ChgPageDesc(0, const_cast<const SwDoc &>(rDoc).GetPageDesc(0));

    // PageDescs "Konvert..."
    for (sal_uInt16 i = nInPageDescOffset; i < rDoc.GetPageDescCnt(); ++i)
        rDoc.ChgPageDesc(i, const_cast<const SwDoc &>(rDoc).GetPageDesc(i));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
