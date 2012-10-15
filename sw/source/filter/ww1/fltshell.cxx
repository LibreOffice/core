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


#include <ctype.h>
#include <hintids.hxx>
#include <hints.hxx>
#include <svtools/filter.hxx>

#include <vcl/graph.hxx>
#include <svl/urihelper.hxx>
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


using namespace com::sun::star;

static SwCntntNode* GetCntntNode(SwDoc* pDoc, SwNodeIndex& rIdx, sal_Bool bNext)
{
    SwCntntNode * pCNd = rIdx.GetNode().GetCntntNode();
    if(!pCNd && 0 == (pCNd = bNext ? pDoc->GetNodes().GoNext(&rIdx)
                                     : pDoc->GetNodes().GoPrevious(&rIdx)))
    {
        pCNd = bNext ? pDoc->GetNodes().GoPrevious(&rIdx)
                     : pDoc->GetNodes().GoNext(&rIdx);
        OSL_ENSURE(pCNd, "kein ContentNode gefunden");
    }
    return pCNd;
}

// ------ Stack-Eintrag fuer die gesamten - Attribute vom Text -----------
SwFltStackEntry::SwFltStackEntry(const SwPosition& rStartPos, SfxPoolItem* pHt)
    : m_aMkPos(rStartPos)
    , m_aPtPos(rStartPos)
{
    pAttr = pHt;        // speicher eine Kopie vom Attribut
    bOld    = sal_False;    // used for marking Attributes *before* skipping field results
    bOpen = sal_True;   // locke das Attribut --> darf erst
    bConsumedByField = sal_False;
}

SwFltStackEntry::~SwFltStackEntry()
{
    // Attribut kam zwar als Pointer, wird aber hier geloescht
    delete pAttr;
}

void SwFltStackEntry::SetEndPos(const SwPosition& rEndPos)
{
    // Attribut freigeben und das Ende merken.
    // Alles mit sal_uInt16's, weil sonst beim Einfuegen von neuem Text an der
    // Cursor-Position auch der Bereich vom Attribut weiter
    // verschoben wird.
    // Das ist aber nicht das gewollte!
    bOpen = sal_False;                  // freigeben und das ENDE merken
    m_aPtPos.SetPos(rEndPos);
}

bool SwFltStackEntry::MakeRegion(SwDoc* pDoc, SwPaM& rRegion, bool bCheck,
    const SwFltPosition &rMkPos, const SwFltPosition &rPtPos,
    sal_uInt16 nWhich)
{
    // does this range actually contain something?
    // empty range is allowed if at start of empty paragraph
    // fields are special: never have range, so leave them
    SwCntntNode *const pCntntNode(
        SwNodeIndex(rMkPos.m_nNode, +1).GetNode().GetCntntNode());
    if (rMkPos == rPtPos &&
        ((0 != rPtPos.m_nCntnt) || (pCntntNode && (0 != pCntntNode->Len())))
        && (RES_TXTATR_FIELD != nWhich))
    {
        return false;
    }

    // !!! Die Content-Indizies beziehen sich immer auf den Node !!!
    rRegion.GetPoint()->nNode = rMkPos.m_nNode.GetIndex() + 1;
    SwCntntNode* pCNd = GetCntntNode(pDoc, rRegion.GetPoint()->nNode, sal_True);
    rRegion.GetPoint()->nContent.Assign(pCNd, rMkPos.m_nCntnt);
    rRegion.SetMark();
    if (rMkPos.m_nNode != rPtPos.m_nNode)
    {
        rRegion.GetPoint()->nNode = rPtPos.m_nNode.GetIndex() + 1;
        pCNd = GetCntntNode(pDoc, rRegion.GetPoint()->nNode, sal_False);
    }
    rRegion.GetPoint()->nContent.Assign(pCNd, rPtPos.m_nCntnt);
    OSL_ENSURE( CheckNodesRange( rRegion.Start()->nNode,
                             rRegion.End()->nNode, sal_True ),
             "atttribute or similar crosses section-boundaries" );
    if( bCheck )
        return CheckNodesRange( rRegion.Start()->nNode,
                                rRegion.End()->nNode, sal_True );
    else
        return true;
}

bool SwFltStackEntry::MakeRegion(SwDoc* pDoc, SwPaM& rRegion, bool bCheck) const
{
    return MakeRegion(pDoc, rRegion, bCheck, m_aMkPos, m_aPtPos,
        pAttr->Which());
}

SwFltControlStack::SwFltControlStack(SwDoc* pDo, sal_uLong nFieldFl)
    : nFieldFlags(nFieldFl), pDoc(pDo), bIsEndStack(false)
{
}


SwFltControlStack::~SwFltControlStack()
{
    OSL_ENSURE(maEntries.empty(), "There are still Attributes on the stack");
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
    size_t nCnt = maEntries.size();
    sal_uLong nPosNd = rPos.nNode.GetIndex();
    sal_uInt16 nPosCt = rPos.nContent.GetIndex() - 1;

    for (size_t i=0; i < nCnt; ++i)
    {
        SwFltStackEntry& rEntry = maEntries[i];
        if (
            (rEntry.m_aMkPos.m_nNode.GetIndex()+1 == nPosNd) &&
            (rEntry.m_aMkPos.m_nCntnt >= nPosCt)
           )
        {
            rEntry.m_aMkPos.m_nCntnt++;
            OSL_ENSURE( rEntry.m_aMkPos.m_nCntnt
                <= pDoc->GetNodes()[nPosNd]->GetCntntNode()->Len(),
                    "Attribut-Anfang hinter Zeilenende" );
        }
        if (
            (rEntry.m_aPtPos.m_nNode.GetIndex()+1 == nPosNd) &&
            (rEntry.m_aPtPos.m_nCntnt >= nPosCt)
           )
        {
            rEntry.m_aPtPos.m_nCntnt++;
            OSL_ENSURE( rEntry.m_aPtPos.m_nCntnt
                <= pDoc->GetNodes()[nPosNd]->GetCntntNode()->Len(),
                    "Attribut-Ende hinter Zeilenende" );
        }
    }
}

void SwFltControlStack::MarkAllAttrsOld()
{
    size_t nCnt = maEntries.size();
    for (sal_uInt16 i=0; i < nCnt; ++i)
        maEntries[i].bOld = sal_True;
}

namespace
{
    bool couldExtendEntry(const SwFltStackEntry *pExtendCandidate,
        const SfxPoolItem& rAttr)
    {
        return (pExtendCandidate &&
                !pExtendCandidate->bConsumedByField &&
                //potentially more, but lets keep it simple
                (isPARATR_LIST(rAttr.Which()) || (isCHRATR(rAttr.Which()) && rAttr.Which() != RES_CHRATR_FONT && rAttr.Which() != RES_CHRATR_FONTSIZE)) &&
                *(pExtendCandidate->pAttr) == rAttr);
    }
}

void SwFltControlStack::NewAttr(const SwPosition& rPos, const SfxPoolItem& rAttr)
{
    sal_uInt16 nWhich = rAttr.Which();
    // Ende von evtl. gleichen Attributen auf dem Stack Setzen, damit sich die
    // Attribute nicht auf dem Stack haeufen
    SwFltStackEntry *pExtendCandidate = SetAttr(rPos, nWhich);
    if (couldExtendEntry(pExtendCandidate, rAttr))
    {
        //Here we optimize by seeing if there is an attribute uncommited
        //to the document which
        //
        //a) has the same value as this attribute
        //b) is already open, or ends at the same place as where we're starting
        //from. If so we merge it with this one and elide adding another
        //to the stack
        pExtendCandidate->SetEndPos(rPos);
        pExtendCandidate->bOpen=true;
    }
    else
        maEntries.push_back(new SwFltStackEntry(rPos, rAttr.Clone()));
}

void SwFltControlStack::DeleteAndDestroy(Entries::size_type nCnt)
{
    OSL_ENSURE(nCnt < maEntries.size(), "Out of range!");
    if (nCnt < maEntries.size())
    {
        myEIter aElement = maEntries.begin() + nCnt;
        maEntries.erase(aElement);
    }
}

// SwFltControlStack::StealAttr() loescht Attribute des angegebenen Typs vom Stack.
// Als nAttrId sind erlaubt: 0 fuer alle, oder ein spezieller Typ.
// Damit erscheinen sie nicht in der Doc-Struktur. Dabei werden nur die
// Attribute entfernt, die im selben Absatz wie rPos stehen.
// Wird fuer Grafik-Apos -> Grafiken benutzt.
void SwFltControlStack::StealAttr(const SwNodeIndex& rNode, sal_uInt16 nAttrId)
{
    size_t nCnt = maEntries.size();

    while (nCnt)
    {
        nCnt --;
        SwFltStackEntry& rEntry = maEntries[nCnt];
        if (rEntry.m_aPtPos.m_nNode.GetIndex()+1 == rNode.GetIndex() &&
            (!nAttrId || nAttrId == rEntry.pAttr->Which()))
        {
            DeleteAndDestroy(nCnt);     // loesche aus dem Stack
        }
    }
}

// SwFltControlStack::KillUnlockedAttr() loescht alle Attribute vom Stack,
// welche punktuell auf rPos aufgespannt sind.
// Damit erscheinen sie nicht in der Doc-Struktur.
// Wird im WW Import benoetigt zum ignorieren der auf dem 0x0c Section-
// Break-Symbol gesetzten Attribute.
void SwFltControlStack::KillUnlockedAttrs(const SwPosition& rPos)
{
    SwFltPosition aFltPos(rPos);

    size_t nCnt = maEntries.size();
    while( nCnt )
    {
        nCnt --;
        SwFltStackEntry& rEntry = maEntries[nCnt];
        if(    !rEntry.bOld
            && !rEntry.bOpen
            && (rEntry.m_aMkPos == aFltPos)
            && (rEntry.m_aPtPos == aFltPos))
        {
            DeleteAndDestroy( nCnt ); // loesche aus dem Stack
        }
    }
}

// Alle gelockten Attribute freigeben (unlocken) und das Ende setzen,
// alle anderen im Document setzen und wieder aus dem Stack loeschen
// Returned, ob das gesuchte Attribut / die gesuchten Attribute
// ueberhaupt auf dem Stack standen
SwFltStackEntry* SwFltControlStack::SetAttr(const SwPosition& rPos,
    sal_uInt16 nAttrId, sal_Bool bTstEnde, long nHand,
    sal_Bool consumedByField)
{
    SwFltStackEntry *pRet = NULL;

    SwFltPosition aFltPos(rPos);

    OSL_ENSURE(!nAttrId ||
        (POOLATTR_BEGIN <= nAttrId && POOLATTR_END > nAttrId) ||
        (RES_FLTRATTR_BEGIN <= nAttrId && RES_FLTRATTR_END > nAttrId),
        "Falsche Id fuers Attribut");

    myEIter aI = maEntries.begin();
    while (aI != maEntries.end())
    {
        SwFltStackEntry& rEntry = *aI;
        if (rEntry.bOpen)
        {
            // setze das Ende vom Attribut
            bool bF = false;
            if (!nAttrId )
            {
                bF = true;
            }
            else if (nAttrId == rEntry.pAttr->Which())
            {
                if( nAttrId != RES_FLTR_BOOKMARK )
                {
                    // Handle abfragen
                    bF = true;
                }
                else if (nHand == ((SwFltBookmark*)(rEntry.pAttr))->GetHandle())
                {
                    bF = true;
                }
            }
            if (bF)
            {
                rEntry.bConsumedByField = consumedByField;
                rEntry.SetEndPos(rPos);
                if (nAttrId == rEntry.pAttr->Which())
                {
                    //potential candidate for merging with an identical
                    //property beginning at rPos
                    pRet = &rEntry;
                }
            }
            ++aI;
            continue;
        }

        // ist die Endposition die Cursor-Position, dann noch nicht
        // ins Dokument setzen, es muss noch Text folgen;
        // ausser am Dokumentende. (Attribut-Expandierung !!)
        // Beim Ende-Stack niemals ausser am DocEnde reinsetzen
        if (bTstEnde)
        {
            if (bIsEndStack)
            {
                ++aI;
                continue;
            }

            //defer inserting this attribute into the document until
            //we advance to the next node, or finish processing the document
            if (rEntry.m_aPtPos.m_nNode.GetIndex() == aFltPos.m_nNode.GetIndex())
            {
                if (nAttrId == rEntry.pAttr->Which() &&
                    rEntry.m_aPtPos.m_nCntnt == aFltPos.m_nCntnt)
                {
                    //potential candidate for merging with an identical
                    //property beginning at rPos
                    pRet = &rEntry;
                }

                ++aI;
                continue;
            }
        }
        SetAttrInDoc(rPos, rEntry);
        aI = maEntries.erase(aI);
    }

    return pRet;
}

static void MakePoint(const SwFltStackEntry& rEntry, SwDoc* pDoc,
    SwPaM& rRegion)
{
    // der Anker ist der Point vom Pam. Dieser wird beim Einfugen von Text usw.
    // veraendert; darum wird er auf dem Stack gespeichert. Das Attribut muss
    // nur noch im Format gesetzt werden.
    rRegion.DeleteMark();
    rRegion.GetPoint()->nNode = rEntry.m_aMkPos.m_nNode.GetIndex() + 1;
    SwCntntNode* pCNd = GetCntntNode(pDoc, rRegion.GetPoint()->nNode, sal_True);
    rRegion.GetPoint()->nContent.Assign(pCNd, rEntry.m_aMkPos.m_nCntnt);
}

// MakeBookRegionOrPoint() ist wie MakeRegionOrPoint, aber die besonderen
// Beschraenkungen von Bookmarks in Tabellen werden beachtet.
// ( Anfang und Ende muessen in selber Zelle sein )
static void MakeBookRegionOrPoint(const SwFltStackEntry& rEntry, SwDoc* pDoc,
                    SwPaM& rRegion, sal_Bool bCheck )
{
    if (rEntry.MakeRegion(pDoc, rRegion, bCheck )){
//      sal_Bool b1 = rNds[rRegion.GetPoint()->nNode]->FindTableNode() != 0;
        if (rRegion.GetPoint()->nNode.GetNode().FindTableBoxStartNode()
              != rRegion.GetMark()->nNode.GetNode().FindTableBoxStartNode())
        {
            rRegion.Exchange();         // Ungueltiger Bereich
            rRegion.DeleteMark();       // -> beide auf Mark
        }
    }else{
        MakePoint(rEntry, pDoc, rRegion);
    }
}

// IterateNumrulePiece() sucht von rTmpStart bis rEnd den ersten
// fuer Numrules gueltigen Bereich heraus.
//
// rNds sind die Doc-Nodes
// rEnd ist Bereichs-Ende,
// rTmpStart ist ReinRaus-Parameter: Anfang des zu untersuchenden Bereiches rein,
//                                   Anfang des gueltigen Bereichs raus
// rTmpEnd ist raus-Parameter
// Return-Bool ist sal_True fuer gueltigen Bereich
static sal_Bool IterateNumrulePiece( const SwNodeIndex& rEnd,
                                SwNodeIndex& rTmpStart, SwNodeIndex& rTmpEnd )
{
    while( ( rTmpStart <= rEnd )
           && !( rTmpStart.GetNode().IsTxtNode() ) )    // suche gueltigen Anfang
        ++rTmpStart;

    rTmpEnd = rTmpStart;
    while( ( rTmpEnd <= rEnd )
           && ( rTmpEnd.GetNode().IsTxtNode() ) )       // suche gueltiges Ende + 1
        ++rTmpEnd;

    rTmpEnd--;                                      // gueltiges Ende

    return rTmpStart <= rTmpEnd;                    // gueltig ?
}

void SwFltControlStack::SetAttrInDoc(const SwPosition& rTmpPos,
    SwFltStackEntry& rEntry)
{
    SwPaM aRegion( rTmpPos );

    switch(rEntry.pAttr->Which())
    {
    case RES_FLTR_ANCHOR:
        {
            SwFrmFmt* pFmt = ((SwFltAnchor*)rEntry.pAttr)->GetFrmFmt();
            if (pFmt != NULL)
            {
                MakePoint(rEntry, pDoc, aRegion);
                SwFmtAnchor aAnchor(pFmt->GetAnchor());
                aAnchor.SetAnchor(aRegion.GetPoint());
                pFmt->SetFmtAttr(aAnchor);
                // Damit die Frames bei Einfuegen in existierendes Doc
                //  erzeugt werden (erst nach Setzen des Ankers!):
                if(pDoc->GetCurrentViewShell()  //swmod 071108//swmod 071225
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
            const String& rNumNm = ((SfxStringItem*)rEntry.pAttr)->GetValue();
            SwNumRule* pRul = pDoc->FindNumRulePtr( rNumNm );
            if( pRul )
            {
                if( rEntry.MakeRegion(pDoc, aRegion, sal_True))
                {
                    SwNodeIndex aTmpStart( aRegion.Start()->nNode );
                    SwNodeIndex aTmpEnd( aTmpStart );
                    SwNodeIndex& rRegEndNd = aRegion.End()->nNode;
                    while( IterateNumrulePiece( rRegEndNd,
                                                aTmpStart, aTmpEnd ) )
                    {
                        SwPaM aTmpPam( aTmpStart, aTmpEnd );
                        // no start of a new list
                        pDoc->SetNumRule( aTmpPam, *pRul, false );

                        aTmpStart = aTmpEnd;    // Start fuer naechstes Teilstueck
                        ++aTmpStart;
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
            SwFltBookmark* pB = (SwFltBookmark*)rEntry.pAttr;
            const String& rName = ((SwFltBookmark*)rEntry.pAttr)->GetName();

            if (IsFlagSet(BOOK_TO_VAR_REF))
            {
                if (pB->IsPgRef() && !pB->IsRef())
                {
                            // XRefs und Bookmarks sind bereits geUpcased
                    MakeBookRegionOrPoint(rEntry, pDoc, aRegion, sal_True);
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
                    MakePoint(rEntry, pDoc, aRegion);
                    pDoc->InsertPoolItem(aRegion, SwFmtFld(aFld), 0);
                    MoveAttrs( *(aRegion.GetPoint()) );
                }
            }
            if( !pB->IsOnlyRef() &&
                ( !IsFlagSet(HYPO) || IsFlagSet(BOOK_AND_REF) ) && !rEntry.bConsumedByField)
            {
                MakeBookRegionOrPoint(rEntry, pDoc, aRegion, sal_True);
                pDoc->getIDocumentMarkAccess()->makeMark( aRegion, rName, IDocumentMarkAccess::BOOKMARK);
            }
        }
        break;
    case RES_FLTR_TOX:
        {
            MakePoint(rEntry, pDoc, aRegion);

            SwPosition* pPoint = aRegion.GetPoint();

            SwFltTOX* pTOXAttr = (SwFltTOX*)rEntry.pAttr;

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
                            && SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, sal_False, &pItem ) )
                        {
                            aBkSet.Put( *pItem );
                            pNd->ResetAttr( RES_BREAK );
                        }
                        if(    !pTOXAttr->HadPageDescItem()
                            && SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC, sal_False, &pItem ) )
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
        MakePoint(rEntry, pDoc, aRegion);   // bislang immer Point==Mark
        pDoc->InsertSwSection(aRegion,
                *(static_cast<SwFltSection*>(rEntry.pAttr))->GetSectionData(),
                0, 0, false);
        delete (((SwFltSection*)rEntry.pAttr)->GetSectionData());
        break;
    case RES_FLTR_REDLINE:
        {
            if (rEntry.MakeRegion(pDoc, aRegion, sal_True))
            {
              pDoc->SetRedlineMode((RedlineMode_t)(   nsRedlineMode_t::REDLINE_ON
                                              | nsRedlineMode_t::REDLINE_SHOW_INSERT
                                              | nsRedlineMode_t::REDLINE_SHOW_DELETE ));
                SwFltRedline& rFltRedline = *((SwFltRedline*)rEntry.pAttr);

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
        if (rEntry.MakeRegion(pDoc, aRegion, sal_False))
        {
            pDoc->InsertPoolItem(aRegion, *rEntry.pAttr, 0);
        }
        break;
    }
}

SfxPoolItem* SwFltControlStack::GetFmtStackAttr(sal_uInt16 nWhich, sal_uInt16 * pPos)
{
    size_t nSize = maEntries.size();

    while (nSize)
    {
        // ist es das gesuchte Attribut ? (gueltig sind nur gelockte,
        // also akt. gesetzte Attribute!!)
        SwFltStackEntry &rEntry = maEntries[--nSize];
        if (rEntry.bOpen && rEntry.pAttr->Which() == nWhich)
        {
            if (pPos)
                *pPos = nSize;
            return (SfxPoolItem*)rEntry.pAttr;      // Ok, dann Ende
        }
    }
    return 0;
}

const SfxPoolItem* SwFltControlStack::GetFmtAttr(const SwPosition& rPos, sal_uInt16 nWhich)
{
    SfxPoolItem* pHt = GetFmtStackAttr(nWhich);
    if (pHt)
        return (const SfxPoolItem*)pHt;

    // im Stack ist das Attribut nicht vorhanden, also befrage das Dokument
    SwCntntNode * pNd = rPos.nNode.GetNode().GetCntntNode();

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
    sal_uInt16 nStartIdx = pStt->nContent.GetIndex();
    SwNodeIndex aEndNode(pEnd->nNode, -1);
    sal_uInt16 nEndIdx = pEnd->nContent.GetIndex();

    //We don't support deleting content that is over one node, or removing a node.
    OSL_ENSURE(aEndNode == aStartNode, "nodes must be the same, or this method extended");
    if (aEndNode != aStartNode)
        return;

    for (size_t nSize = maEntries.size(); nSize > 0;)
    {
        SwFltStackEntry& rEntry = maEntries[--nSize];

        bool bEntryStartAfterSelStart =
            (rEntry.m_aMkPos.m_nNode == aStartNode &&
             rEntry.m_aMkPos.m_nCntnt >= nStartIdx);

        bool bEntryStartBeforeSelEnd =
            (rEntry.m_aMkPos.m_nNode == aEndNode &&
             rEntry.m_aMkPos.m_nCntnt <= nEndIdx);

        bool bEntryEndAfterSelStart = false;
        bool bEntryEndBeforeSelEnd = false;
        if (!rEntry.bOpen)
        {
            bEntryEndAfterSelStart =
                (rEntry.m_aPtPos.m_nNode == aStartNode &&
                 rEntry.m_aPtPos.m_nCntnt >= nStartIdx);

            bEntryEndBeforeSelEnd =
                (rEntry.m_aPtPos.m_nNode == aEndNode &&
                 rEntry.m_aPtPos.m_nCntnt <= nEndIdx);
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
                rEntry.m_aMkPos.SetPos(aStartNode, nStartIdx);
            }
            else
                rEntry.m_aMkPos.m_nCntnt -= nCntntDiff;
        }

        if (bEntryEndAfterSelStart)
        {
            if (bEntryEndBeforeSelEnd)
                rEntry.m_aPtPos.SetPos(aStartNode, nStartIdx);
            else
                rEntry.m_aPtPos.m_nCntnt -= nCntntDiff;
        }

        //That's what Open is, end equal to start, and nPtCntnt is invalid
        if (rEntry.bOpen)
            rEntry.m_aPtPos = rEntry.m_aMkPos;
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

SfxPoolItem* SwFltAnchor::Clone(SfxItemPool*) const
{
    return new SwFltAnchor(*this);
}

// SwFltAnchorClient

SwFltAnchorClient::SwFltAnchorClient(SwFltAnchor * pFltAnchor)
: m_pFltAnchor(pFltAnchor)
{
}

void  SwFltAnchorClient::Modify(const SfxPoolItem *, const SfxPoolItem * pNew)
{
    if (pNew->Which() == RES_FMT_CHG)
    {
        const SwFmtChg * pFmtChg = dynamic_cast<const SwFmtChg *> (pNew);

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
                                long nHand, sal_Bool bOnlyR )
    : SfxPoolItem(RES_FLTR_BOOKMARK), nHandle(nHand), aName(rNa), aVal(rVa),
    bOnlyRef(bOnlyR), bRef(sal_False), bPgRef(sal_False)
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

SwFltTOX::SwFltTOX(SwTOXBase* pBase, sal_uInt16 _nCols)
    : SfxPoolItem(RES_FLTR_TOX), pTOXBase(pBase), nCols( _nCols ),
      bHadBreakItem( sal_False ), bHadPageDescItem( sal_False )
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

SfxPoolItem* SwFltSection::Clone(SfxItemPool*) const
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
SwFltShell::SwFltShell(SwDoc* pDoc, SwPaM& rPaM, const String& rBaseURL, sal_Bool bNew, sal_uLong nFieldFl) :
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
    bStdPD(sal_False),
    bProtect(sal_False)
{
    memset( pColls, 0, sizeof( pColls ) );
    pOutDoc = new SwFltOutDoc( *pDoc, pPaM, aStack, aEndStack );
    pOut = pOutDoc;

    if( !bNewDoc ){     // in ein Dokument einfuegen ?
                        // Da immer ganze Zeile eingelesen werden, muessen
                        // evtl. Zeilen eingefuegt / aufgebrochen werden
        const SwPosition* pPos = pPaM->GetPoint();
        const SwTxtNode* pSttNd = pPos->nNode.GetNode().GetTxtNode();
        sal_uInt16 nCntPos = pPos->nContent.GetIndex();
        if( nCntPos && pSttNd->GetTxt().Len() )
                                            // EinfuegePos nicht in leerer Zeile
            pDoc->SplitNode( *pPos, false );        // neue Zeile erzeugen
        if( pSttNd->GetTxt().Len() ){       // EinfuegePos nicht am Ende der Zeile
            pDoc->SplitNode( *pPos, false );        // neue Zeile
            pPaM->Move( fnMoveBackward );   // gehe in leere Zeile
        }

        // verhinder das Einlesen von Tabellen in Fussnoten / Tabellen
        sal_uLong nNd = pPos->nNode.GetIndex();
        sal_Bool bReadNoTbl = 0 != pSttNd->FindTableNode() ||
            ( nNd < pDoc->GetNodes().GetEndOfInserts().GetIndex() &&
            pDoc->GetNodes().GetEndOfInserts().StartOfSectionIndex() < nNd );
        if( bReadNoTbl )
            pOutDoc->SetReadNoTable();
    }
    pCurrentPageDesc =  &pDoc->GetPageDesc( 0 );  // Standard

}

SwFltShell::~SwFltShell()
{
    sal_uInt16 i;

    if (eSubMode == Style)
        EndStyle();
    if( pOutDoc->IsInTable() )          // falls nicht ordentlich abgeschlossen
        EndTable();
    if( pOutDoc->IsInFly() )
        EndFly();

    GetDoc().SetUpdateExpFldStat(true);
    GetDoc().SetInitDBFields(sal_True);
    aStack.SetAttr(*pPaM->GetPoint(), 0, sal_False);
    aStack.SetAttr(*pPaM->GetPoint(), 0, sal_False);
    aEndStack.SetAttr(*pPaM->GetPoint(), 0, sal_False);
    aEndStack.SetAttr(*pPaM->GetPoint(), 0, sal_False);
    if( bProtect ){     // Das ganze Doc soll geschuetzt sein

        SwDoc& rDoc = GetDoc();
                        // 1. SectionFmt und Section anlegen
        SwSectionFmt* pSFmt = rDoc.MakeSectionFmt( 0 );
        SwSectionData aSectionData(CONTENT_SECTION, rtl::OUString("PMW-Protect"));
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
                pDocSh->SetReadOnlyUI( sal_True );
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
        delete pColls[i];
    delete pOutDoc;
}

SwFltShell& SwFltShell::operator << ( const String& rStr )
{
    OSL_ENSURE(eSubMode != Style, "char insert while in style-mode");
    GetDoc().InsertString( *pPaM, rStr );
    return *this;
}

void SwFltShell::ConvertUStr( String& rInOut )
{
    rInOut = GetAppCharClass().uppercase( rInOut );
}

// QuoteString() wandelt CRs abhaengig von nFieldIniFlags in '\n' oder "\0x0d"
OUString SwFltShell::QuoteStr( const OUString& rIn )
{
    OUStringBuffer sOut( rIn );
    sal_Bool bAllowCr = aStack.IsFlagSet( SwFltControlStack::ALLOW_FLD_CR );

    for( sal_Int32 n = 0; n < sOut.getLength(); ++n )
    {
        switch( sOut[ n ] )
        {
        case 0x0a:
            sOut.remove( n, 1 );             // 0xd 0xa wird zu \n
            break;

        case 0x0b:
        case 0x0c:
        case 0x0d:
            if( bAllowCr )
                sOut[n] = '\n';
            break;
        }
    }
    return sOut.makeStringAndClear();
}

SwFltShell& SwFltShell::operator << ( const sal_Unicode c )
{
    OSL_ENSURE( eSubMode != Style, "char insert while in style-mode");
    GetDoc().InsertString( *pPaM, rtl::OUString(c) );
    return *this;
}

SwFltShell& SwFltShell::AddError( const sal_Char* pErr )
{
    String aName(rtl::OUString("ErrorTag"));
    SwFieldType* pFT = GetDoc().GetFldType( RES_SETEXPFLD, aName, false );
    if( pFT == 0)
    {
        SwSetExpFieldType aS(&GetDoc(), aName, nsSwGetSetExpType::GSE_STRING);
        pFT = GetDoc().InsertFldType(aS);
    }
    SwSetExpField aFld( (SwSetExpFieldType*)pFT,
                        rtl::OUString::createFromAscii( pErr ));
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
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic;
    // one of: GFF_NOT GFF_BMP GFF_GIF GFF_JPG GFF_PCD GFF_PCX GFF_PNG
    // GFF_TIF GFF_XBM GFF_DXF GFF_MET GFF_PCT GFF_SGF GFF_SVM GFF_WMF
    // GFF_SGV GFF_XXX
    INetURLObject aDir(
        URIHelper::SmartRel2Abs(
            INetURLObject(GetBaseURL()), rPicName,
            URIHelper::GetMaybeFileHdl()) );
    switch (rFilter.ImportGraphic(aGraphic, aDir))
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

SwFltShell& SwFltShell::SetStyle( sal_uInt16 nStyle )
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
        OSL_FAIL( "Ungueltiger SwFltStyleCode" );
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
    aEndStack.SetAttr( *pPaM->GetPoint(), RES_FLTR_BOOKMARK, sal_True, nHandle );
}

SwFltShell& SwFltShell::EndItem( sal_uInt16 nAttrId )
{
    switch( nAttrId )
    {
    case RES_FLTR_BOOKMARK:
        OSL_FAIL( "Falscher Aufruf fuer Bookmark-Ende" );
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

const SfxPoolItem& SwFltOutDoc::GetAttr(sal_uInt16 nWhich)
{
    return *rStack.GetFmtAttr(*pPaM->GetPoint(), nWhich);
}

const SfxPoolItem& SwFltFormatCollection::GetAttr(sal_uInt16 nWhich)
{
    return GetColl()->GetFmtAttr(nWhich);   // mit Parents
}

// GetNodeOrStyAttr holt Attribute fuer Toggle- und Modify-Attribute:
// Bei Formatdefinitionen aus dem altuellen Style mit Parents
// sonst aus dem Node mit Parents
// Im Stack wird nicht nachgesehen

const SfxPoolItem& SwFltOutDoc::GetNodeOrStyAttr(sal_uInt16 nWhich)
{
    SwCntntNode * pNd = pPaM->GetPoint()->nNode.GetNode().GetCntntNode();
    if (pNd)            // ContentNode: Attribut mit Parent
        return pNd->GetAttr(nWhich);
    else                // kein ContentNode, dann das dflt. Attribut
        return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
}

const SfxPoolItem& SwFltFormatCollection::GetNodeOrStyAttr(sal_uInt16 nWhich)
{
    return GetColl()->GetFmtAttr(nWhich);   // mit Parents
}

const SfxPoolItem& SwFltShell::GetNodeOrStyAttr(sal_uInt16 nWhich)
{
    return pOut->GetNodeOrStyAttr( nWhich );
}

const SfxPoolItem& SwFltShell::GetAttr(sal_uInt16 nWhich)
{
    return pOut->GetAttr( nWhich );
}

const SfxPoolItem& SwFltShell::GetFlyFrmAttr(sal_uInt16 nWhich)
{
    return pOut->GetFlyFrmAttr( nWhich );
}

SwFieldType* SwFltShell::GetSysFldType(sal_uInt16 eWhich)
{
    return GetDoc().GetSysFldType(eWhich);
}

sal_Bool SwFltShell::GetWeightBold()
{
    return ((SvxWeightItem&)GetNodeOrStyAttr(RES_CHRATR_WEIGHT)).GetWeight()
                                != WEIGHT_NORMAL;
}

sal_Bool SwFltShell::GetPostureItalic()
{
    return ((SvxPostureItem&)GetNodeOrStyAttr(RES_CHRATR_POSTURE)).GetPosture()
                                != ITALIC_NONE;
}

sal_Bool SwFltShell::GetCrossedOut()
{
    return ((SvxCrossedOutItem&)GetNodeOrStyAttr(RES_CHRATR_CROSSEDOUT))
                                    .GetStrikeout() != STRIKEOUT_NONE;
}

sal_Bool SwFltShell::GetContour()
{
    return ((SvxContourItem&)GetNodeOrStyAttr(RES_CHRATR_CONTOUR)).GetValue();
}

sal_Bool SwFltShell::GetCaseKapitaelchen()
{
    return ((SvxCaseMapItem&)GetNodeOrStyAttr(RES_CHRATR_CASEMAP))
                                    .GetCaseMap() == SVX_CASEMAP_KAPITAELCHEN;
}

sal_Bool SwFltShell::GetCaseVersalien()
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

const SfxPoolItem& SwFltOutBase::GetCellAttr(sal_uInt16 nWhich)
{
    OSL_FAIL("GetCellAttr ausserhalb von normalem Text");
    return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
}

sal_Bool SwFltOutBase::BeginTable()
{
    OSL_FAIL("BeginTable ausserhalb von normalem Text");
    return sal_False;
}

void SwFltOutBase::NextTableCell()
{
    OSL_FAIL("NextTableCell ausserhalb von normalem Text");
}

void SwFltOutBase::NextTableRow()
{
    OSL_FAIL("NextTableRow ausserhalb von normalem Text");
}

void SwFltOutBase::SetTableWidth(SwTwips /*nW*/)
{
    OSL_FAIL("SetTableWidth ausserhalb von normalem Text");
}

void SwFltOutBase::SetTableOrient(sal_Int16 /*eOri*/)
{
    OSL_FAIL("SetTableOrient ausserhalb von normalem Text");
}

void SwFltOutBase::SetCellWidth(SwTwips /*nWidth*/, sal_uInt16 /*nCell*/)
{
    OSL_FAIL("SetCellWidth ausserhalb von normalem Text");
}

void SwFltOutBase::SetCellHeight(SwTwips /*nH*/)
{
    OSL_FAIL("SetCellHeight ausserhalb von normalem Text");
}

void SwFltOutBase::SetCellBorder(const SvxBoxItem& /*rFmtBox*/, sal_uInt16 /*nCell*/)
{
    OSL_FAIL("SetCellBorder ausserhalb von normalem Text");
}

void SwFltOutBase::SetCellSpace(sal_uInt16 /*nSp*/)
{
    OSL_FAIL("SetCellSpace ausserhalb von normalem Text");
}

void SwFltOutBase::DeleteCell(sal_uInt16 /*nCell*/)
{
    OSL_FAIL("DeleteCell ausserhalb von normalem Text");
}

void SwFltOutBase::EndTable()
{
    OSL_FAIL("EndTable ausserhalb von normalem Text");
}

/*virtual*/ sal_Bool SwFltOutDoc::IsInTable()
{
    return pTable != 0;
};

sal_Bool SwFltOutDoc::BeginTable()
{
    if(bReadNoTbl)
        return sal_False;

    if (pTable){
        OSL_FAIL("BeginTable in Table");
        return sal_False;
    }
                            // Alle Attribute schliessen, da sonst Attribute
                            // entstehen koennen, die in Flys reinragen
    rStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
    rEndStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );

// create table:
    OSL_ENSURE(pTabSavedPos == NULL, "SwFltOutDoc");
    pTabSavedPos = new SwPosition(*pPaM->GetPoint());
    pTable = GetDoc().InsertTable(
            SwInsertTableOptions( tabopts::HEADLINE_NO_BORDER, 1 ),
            *pTabSavedPos, 1, 1, text::HoriOrientation::LEFT, 0, 0, sal_False, sal_False ); // TODO MULTIHEADER
    nTableWidth = 0;
    ((SwTable*)pTable)->LockModify();   // Nichts automatisch anpassen!
// set pam in 1. table cell
    usTableX =
    usTableY = 0;
    SeekCell(usTableY, usTableX, sal_True);
    return sal_True;
}

SwTableBox* SwFltOutDoc::GetBox(sal_uInt16 ny, sal_uInt16 nx /*= USHRT_MAX */)
{
    if(!pTable){
        OSL_ENSURE(pTable, "GetBox ohne Tabelle");
        return 0;
    }
    if( nx == USHRT_MAX )   // aktuelle Zelle
        nx = usTableX;

// get structs to table cells
    const SwTableLines* pTableLines = &pTable->GetTabLines();
    if(!pTableLines){
        OSL_FAIL("SwFltOutDoc:GetBox:pTableLines");
        return 0;
    }
    if( ny >= pTableLines->size() ){   // Notbremse
        OSL_FAIL( "SwFltOutDoc:GetBox:ny >= Count()");
        ny = pTableLines->size() - 1;
    }
    SwTableLine* pTableLine = (*pTableLines)[ny];
    if(!pTableLine){
        OSL_FAIL("SwFltOutDoc:GetBox:pTableLine");
        return 0;
    }
    SwTableBoxes* pTableBoxes = &pTableLine->GetTabBoxes();
    if(!pTableBoxes){
        OSL_FAIL("SwFltOutDoc:GetBox:pTableBoxes");
        return 0;
    }
    if( nx >= pTableBoxes->size() ){   // Notbremse
        OSL_FAIL("SwFltOutDoc:GetBox:nx >= Count()");
        nx = pTableBoxes->size() - 1;
    }
    SwTableBox* pTableBox = (*pTableBoxes)[nx];

    OSL_ENSURE(pTableBox != 0, "SwFltOutDoc:GetBox:pTableBox");
    return pTableBox;
}

void SwFltOutDoc::NextTableCell()
{
    if(!pTable){
        OSL_ENSURE(pTable, "NextTableCell ohne Tabelle");
        return;
    }
    const SwTableLines* pTableLines = &pTable->GetTabLines();
    SwTableLine* pTableLine = (*pTableLines)[usTableY];
    SwTableBoxes* pTableBoxes = &pTableLine->GetTabBoxes();
    SwTableBox* pTableBox = (*pTableBoxes)[usTableX];
    OSL_ENSURE(pTableBox != 0, "SwFltOutDoc:NextTableCell:pTableBox");
    if(!pTableBox)
        return;
//#pragma message(__FILE__ "(?) : Sw's const problem")
// insert cells:
    if (++usTableX >= pTableBoxes->size())
        GetDoc().GetNodes().InsBoxen(
         GetDoc().IsIdxInTbl(pPaM->GetPoint()->nNode),
         pTableLine,
         (SwTableBoxFmt*)pTableBox->GetFrmFmt(),
         GetDoc().GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false ),
         0,
         pTableBoxes->size());
    SeekCell(usTableY, usTableX, sal_True);
    pTableBox = (*pTableBoxes)[usTableX];
    OSL_ENSURE(pTableBox != 0, "SwFltOutDoc:pTableBox");
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
        aSelBoxes.insert( pTableBox );
        GetDoc().InsertRow(aSelBoxes);
        usTableX = 0;
        SeekCell(++usTableY, usTableX, sal_True);
        GetDoc().SetTxtFmtColl(*pPaM,
            GetDoc().GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false ));
    }
}

void SwFltOutDoc::SetTableWidth(SwTwips nSwWidth)
{
    if(!pTable){
        OSL_ENSURE(pTable, "SetTableWidth ohne Tabelle");
        return;
    }
    OSL_ENSURE( nSwWidth > MINLAY, "Tabellenbreite <= MINLAY" );
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
        OSL_ENSURE(pTable, "SetTableOrient ohne Tabelle");
        return;
    }
    pTable->GetFrmFmt()->SetFmtAttr( SwFmtHoriOrient( 0, eOri ));
}

void SwFltOutDoc::SetCellWidth(SwTwips nWidth, sal_uInt16 nCell /* = USHRT_MAX */ )
{
    if(!pTable){
        OSL_ENSURE(pTable, "SetCellWidth ohne Tabelle");
        return;
    }
    OSL_ENSURE( nWidth > MINLAY, "Tabellenzellenbreite <= MINLAY" );
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
        OSL_ENSURE(pTable, "SetCellHeight ohne Tabelle");
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

const SfxPoolItem& SwFltOutDoc::GetCellAttr(sal_uInt16 nWhich)
{
    if (!pTable){
        OSL_ENSURE(pTable, "GetCellAttr ohne Table");
        return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
    }

    SwTableBox* pTableBox = GetBox(usTableY, usTableX);
    if(!pTableBox)
        return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
    return pTableBox->GetFrmFmt()->GetFmtAttr( nWhich );
}

void SwFltOutDoc::SetCellBorder(const SvxBoxItem& rFmtBox,
                               sal_uInt16 nCell /* = USHRT_MAX */ )
{
    SwTableBox* pTableBox = GetBox(usTableY, nCell);
    if(pTableBox)
        pTableBox->GetFrmFmt()->SetFmtAttr(rFmtBox);
}

// nicht aktiviert !!!
void SwFltOutDoc::SetCellSpace(sal_uInt16 nDist)
{
    if(!pTable){
        OSL_ENSURE(pTable, "SetCellSpace ohne Tabelle");
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

void SwFltOutDoc::DeleteCell(sal_uInt16 nCell /* = USHRT_MAX */)
{
    SwTableBox* pTableBox = GetBox(usTableY, nCell);
    if( pTableBox )
    {
        SwSelBoxes aSelBoxes;
        aSelBoxes.insert( pTableBox );
        GetDoc().DeleteRowCol(aSelBoxes);
        usTableX--;
    }
}

void SwFltOutDoc::SplitTable()
{
    if(!pTable)
    {
        OSL_ENSURE(pTable, "SplitTable ohne Tabelle");
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
        OSL_ENSURE(pTable, "EndTable ohne Table");
        return;
    }
                            // Alle Attribute schliessen, da sonst Attribute
                            // entstehen koennen, die in Flys reinragen
    rStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
    rEndStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );

    if (GetDoc().GetCurrentViewShell()){    //swmod 071108//swmod 071225
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

sal_Bool SwFltOutDoc::SeekCell(short nRow, short nCol, sal_Bool bPam)
{
// get structs to table cells
    const SwTableLines* pTableLines = &pTable->GetTabLines();
    SwTableLine* pTableLine = (*pTableLines)[usTableY];
    SwTableBoxes* pTableBoxes = &pTableLine->GetTabBoxes();
    SwTableBox* pTableBox = (*pTableBoxes)[usTableX];

    if ((sal_uInt16)nRow >= pTableLines->size())
    {
       OSL_ENSURE((sal_uInt16)nRow >= pTableLines->size(), "SwFltOutDoc");
        return sal_False;
    }
    pTableLine = (*pTableLines)[nRow];
    pTableBoxes = &pTableLine->GetTabBoxes();
    if (nCol >= (short)pTableBoxes->size())
        return sal_False;
    pTableBox = (*pTableBoxes)[nCol];
    if( !pTableBox->GetSttNd() )
    {
        OSL_ENSURE(pTableBox->GetSttNd(), "SwFltOutDoc");
        return sal_False;
    }
    if(bPam)
    {
        pPaM->GetPoint()->nNode = pTableBox->GetSttIdx() + 1;
        pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(), 0);
        GetDoc().SetTxtFmtColl(*pPaM,
            GetDoc().GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false ));
    }
    return sal_True;
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

sal_Bool SwFltOutBase::BeginFly( RndStdIds eAnchor /*= FLY_AT_PARA*/,
                           sal_Bool bAbsolutePos /*= sal_False*/,
                           const SfxItemSet* pMoreAttrs /*= 0*/)
{
    (void) pMoreAttrs; // unused in non-debug
    OSL_ENSURE(!pMoreAttrs, "SwFltOutBase:BeginFly mit pMoreAttrs" );
    eFlyAnchor = eAnchor;
    bFlyAbsPos = bAbsolutePos;      // Bloedsinn eigentlich
    return sal_True;
}

/*virtual*/ void SwFltOutBase::SetFlyAnchor( RndStdIds eAnchor )
{
    if( !IsInFly() ){
        OSL_FAIL( "SetFlyAnchor() ohne Fly" );
        return;
    }
    if ( eAnchor == FLY_AS_CHAR ){
        OSL_FAIL( "SetFlyAnchor( FLY_AS_CHAR ) nicht implementiert" );
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

/* virtual */ sal_Bool SwFltOutDoc::IsInFly()
{
    return pFly != 0;
};

SwFrmFmt* SwFltOutDoc::MakeFly( RndStdIds eAnchor, SfxItemSet* pSet )
{
    pFly = (SwFlyFrmFmt*)GetDoc().MakeFlySection( eAnchor, pPaM->GetPoint(),
                                                    pSet );
    return pFly;
}

sal_Bool SwFltOutDoc::BeginFly( RndStdIds eAnchor,
                           sal_Bool bAbsolutePos ,
                           const SfxItemSet* pMoreAttrs)

{
    SwFltOutBase::BeginFly( eAnchor, bAbsolutePos, 0 );
    SfxItemSet* pSet = NewFlyDefaults();

// Alle Attribute schliessen, da sonst Attribute entstehen koennen,
// die in Flys reinragen
    rStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
    rEndStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );

// create Fly:
    OSL_ENSURE(pFlySavedPos == NULL, "BeginFly in Fly");    // rekursiv geht noch nicht
    pFlySavedPos = new SwPosition(*pPaM->GetPoint());


    SwFmtAnchor aAnchor( eAnchor, 1 );

// Wenn Fly-Attribute im Style waren, dann jetzt als Defaults reinsetzen
    if (pMoreAttrs)
        pSet->Put(*pMoreAttrs);

//  dieses NICHT bei Seitengebundenem Fly mit Seiten-NUMMER !
    aAnchor.SetAnchor(pPaM->GetPoint());    // braucht erstaunlicherweise
                                            // den Stack nicht

    pSet->Put( aAnchor );
    SwFrmFmt* pF = MakeFly( eAnchor, pSet );
    delete pSet;

// set pam in Fly
    const SwFmtCntnt& rCntnt = pF->GetCntnt();
    OSL_ENSURE( rCntnt.GetCntntIdx(), "Kein Inhalt vorbereitet." );
    pPaM->GetPoint()->nNode = rCntnt.GetCntntIdx()->GetIndex() + 1;
    SwCntntNode *pNode = pPaM->GetCntntNode();
    pPaM->GetPoint()->nContent.Assign( pNode, 0 );

    return sal_True;
}

/*virtual*/ void SwFltOutDoc::SetFlyFrmAttr(const SfxPoolItem& rAttr)
{
    if (pFly){
        pFly->SetFmtAttr( rAttr );
    }else{
        OSL_ENSURE(pFly, "SetFlyAttr ohne Doc-Fly");
        return;
    }
}

/*virtual*/ const SfxPoolItem& SwFltOutDoc::GetFlyFrmAttr(sal_uInt16 nWhich)
{
    if (pFly){
        return pFly->GetFmtAttr( nWhich );
    }else{
        OSL_ENSURE(pFly, "GetFlyAttr ohne Fly");
        return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
    }
}

void SwFltOutDoc::EndFly()
{
    if( pTable ){
        OSL_FAIL( "SwFltOutDoc::EndFly() in Table" );
        return;
    }
                        // Alle Attribute schliessen, da sonst Attribute
                        // entstehen koennen, die aus Flys rausragen
    rStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
    rEndStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );

    *pPaM->GetPoint() = *pFlySavedPos;              // restore Cursor
    delete pFlySavedPos;
    pFlySavedPos = 0;
    SwFltOutBase::EndFly();
    pFly = 0;
}

//-----------------------------------------------------------------------------
// Flys in SwFltFormatCollection
//-----------------------------------------------------------------------------
/*virtual*/ sal_Bool SwFltFormatCollection::IsInFly()
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

/*virtual*/ const SfxPoolItem& SwFltFormatCollection::GetFlyFrmAttr(sal_uInt16 nWhich)
{
    if( pFlyAttrs )
        return pFlyAttrs->Get( nWhich, sal_False );
    else
        return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
}

sal_Bool SwFltFormatCollection::BeginFly( RndStdIds eAnchor,
                           sal_Bool bAbsolutePos,
                           const SfxItemSet* pMoreAttrs)

{
    SwFltOutBase::BeginFly( eAnchor, bAbsolutePos, pMoreAttrs );
    bHasFly = sal_True;
    return sal_True;
}

void SwFltFormatCollection::EndFly()    // Wird nie aufgerufen
{
}

sal_Bool SwFltFormatCollection::BeginStyleFly( SwFltOutDoc* pOutDoc )
{
    OSL_ENSURE( pOutDoc, "BeginStyleFly ohne pOutDoc" );
    OSL_ENSURE( pOutDoc && !pOutDoc->IsInFly(), "BeginStyleFly in Fly" );
    if( pOutDoc && !pOutDoc->IsInFly() )
        return pOutDoc->BeginFly( eFlyAnchor, bFlyAbsPos, pFlyAttrs );
    else
        return sal_False;
}

//-----------------------------------------------------------------------------
// Flys in SwFltShell
//-----------------------------------------------------------------------------

sal_Bool SwFltShell::BeginFly( RndStdIds eAnchor,
                           sal_Bool bAbsolutePos)
{
    if (pOut->IsInFly()){
        OSL_FAIL("BeginFly in Fly");
        return sal_False;
    }
    if (pOutDoc->IsInTable()){
        OSL_FAIL("BeginFly in Table");
        return sal_False;
    }
    pOut->BeginFly( eAnchor, bAbsolutePos, pColls[nAktStyle]->GetpFlyAttrs() );
    eSubMode = Fly;
    return sal_True;
}

void SwFltShell::SetFlyXPos( short nXPos, sal_Int16 eHRel,
                             sal_Int16 eHAlign)
{
    SetFlyFrmAttr( SwFmtHoriOrient( nXPos, eHAlign, eHRel ) );
}

void SwFltShell::SetFlyYPos( short nYPos, sal_Int16 eVRel,
                             sal_Int16 eVAlign)
{
    SetFlyFrmAttr( SwFmtVertOrient( nYPos, eVAlign, eVRel ) );
}


void SwFltShell::EndFly()
{
    if (!pOut->IsInFly()){
        OSL_FAIL("EndFly ohne Fly");
        return;
    }
    if (pOutDoc->IsInTable()){      // Table verschraenkt mit Fly macht keinen Sinn
        OSL_FAIL("EndFly in Table ( verschraenkt )");
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
        OSL_FAIL("Fussnote in Fly nicht erlaubt");
        return;
    }
    if( pOutDoc->IsInTable() ){
        OSL_FAIL("Fussnote in Table z.Zt. nicht erlaubt");
        return;
    }

// Alle Attribute schliessen, da sonst Attribute entstehen koennen,
// die in Fussnoten reinragen
    aStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
//  EndStack erstmal nicht zwangs-Schliessen, damit Bookmarks ueber
//  Fussnoten im PMW uebernommen werden

    SwFmtFtn aFtn;
    GetDoc().InsertPoolItem(*pPaM, aFtn, 0);
    OSL_ENSURE(pSavedPos == NULL, "SwFltShell");
    pSavedPos = new SwPosition(*pPaM->GetPoint());
    pPaM->Move(fnMoveBackward, fnGoCntnt);
    SwTxtNode* pTxt = pPaM->GetNode()->GetTxtNode();
    SwTxtAttr *const pFN = pTxt->GetTxtAttrForCharAt(
        pPaM->GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN);
    if( !pFN ){         // Passiert z.B. bei Fussnote in Fly
        OSL_ENSURE(pFN, "Probleme beim Anlegen des Fussnoten-Textes");
        return;
    }
    const SwNodeIndex* pStartIndex = ((SwTxtFtn*)pFN)->GetStartNode();
    OSL_ENSURE(pStartIndex, "Probleme beim Anlegen des Fussnoten-Textes");
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
    aStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
//  EndStack erstmal nicht zwangs-Schliessen, damit Bookmarks ueber
//  Fussnoten im PMW uebernommen werden

    *pPaM->GetPoint() = *pSavedPos;             // restore Cursor
    delete pSavedPos;
    pSavedPos = 0;
}

void SwFltShell::BeginHeader(SwPageDesc* /*pPD*/)
{
    SwFrmFmt* pFmt = &pCurrentPageDesc->GetMaster();
    SwFrmFmt* pHdFtFmt;
    pFmt->SetFmtAttr(SwFmtHeader(sal_True));
    pHdFtFmt = (SwFrmFmt*)pFmt->GetHeader().GetHeaderFmt();
    const SwNodeIndex* pStartIndex = pHdFtFmt->GetCntnt().GetCntntIdx();
    if (!pStartIndex)
        return;
    OSL_ENSURE(pSavedPos == NULL, "SwFltShell");
    pSavedPos = new SwPosition(*pPaM->GetPoint());
    pPaM->GetPoint()->nNode = pStartIndex->GetIndex() + 1;
    pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(), 0);
    eSubMode = Header;
}

void SwFltShell::BeginFooter(SwPageDesc* /*pPD*/)
{
    SwFrmFmt* pFmt =  &pCurrentPageDesc->GetMaster();
    SwFrmFmt* pHdFtFmt;
    pFmt->SetFmtAttr(SwFmtFooter(sal_True));
    pHdFtFmt = (SwFrmFmt*)pFmt->GetFooter().GetFooterFmt();
    const SwNodeIndex* pStartIndex = pHdFtFmt->GetCntnt().GetCntntIdx();
    if (!pStartIndex)
        return;
    OSL_ENSURE(pSavedPos == NULL, "SwFltShell");
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

    sal_Bool bFollow = (pFirstPageDesc != 0);
    SwPageDesc* pNewPD;
    sal_uInt16 nPos;
    if (bFollow && pFirstPageDesc->GetFollow() != pFirstPageDesc)
        return pFirstPageDesc;      // Fehler: hat schon Follow
// Erkennung doppelter Namen fehlt noch (Wahrscheinlichkeit
// fuer dopp. Namen ist gering)

    nPos = GetDoc().MakePageDesc( ViewShell::GetShellRes()->GetPageDescName(
                                   GetDoc().GetPageDescCnt(), bFollow ? ShellResource::FOLLOW_PAGE : ShellResource::NORMAL_PAGE),
                                pFirstPageDesc, sal_False );

    pNewPD =  &GetDoc().GetPageDesc(nPos);
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
    bHasFly( sal_False )
{
    Reset();            // Default-Attrs loeschen und Auto-Flag
}

SwFltFormatCollection::SwFltFormatCollection(
    SwDoc& _rDoc, const String& rName ) :
    SwFltOutBase(_rDoc),
    pFlyAttrs( 0 ),
    bHasFly( sal_False )
{
    pColl = _rDoc.MakeTxtFmtColl(rName, (SwTxtFmtColl*)_rDoc.GetDfltTxtFmtColl());
    Reset();            // Default-Attrs loeschen und Auto-Flag
}

void SwFltShell::NextStyle(sal_uInt16 nWhich, sal_uInt16 nNext)
{
        OSL_ENSURE(pColls[nWhich], "Next style for noexistent style" );
        OSL_ENSURE(pColls[nNext], "Next style to noexistent style" );
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
    rDoc.ChgPageDesc(0, rDoc.GetPageDesc(0));

    // PageDescs "Konvert..."
    for (sal_uInt16 i = nInPageDescOffset; i < rDoc.GetPageDescCnt(); ++i)
        rDoc.ChgPageDesc(i, rDoc.GetPageDesc(i));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
