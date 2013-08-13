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


#include <stdlib.h>

#include <node.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <pam.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <hints.hxx>
#include <numrule.hxx>
#include <ndtxt.hxx>
#include <ndnotxt.hxx>
#include <swtable.hxx>      // fuer erzuegen / loeschen der Table-Frames
#include <tblsel.hxx>
#include <section.hxx>
#include <ddefld.hxx>
#include <swddetbl.hxx>
#include <frame.hxx>
#include <txtatr.hxx>
#include <tox.hxx> // InvalidateTOXMark

#include <docsh.hxx>
#include <svl/smplhint.hxx>

typedef std::vector<SwStartNode*> SwSttNdPtrs;


// Funktion zum bestimmen des hoechsten Levels innerhalb des Bereiches

sal_uInt16 HighestLevel( SwNodes & rNodes, const SwNodeRange & rRange );

//-----------------------------------------------------------------------

/*******************************************************************
|*  SwNodes::SwNodes
|*
|*  Beschreibung
|*      Konstruktor; legt die vier Grundsektions (PostIts,
|*      Inserts, Icons, Inhalt) an
*******************************************************************/
SwNodes::SwNodes( SwDoc* pDocument )
    : pRoot( 0 ), pMyDoc( pDocument )
{
    bInNodesDel = bInDelUpdOutl = bInDelUpdNum = sal_False;

    OSL_ENSURE( pMyDoc, "in welchem Doc stehe ich denn?" );

    sal_uLong nPos = 0;
    SwStartNode* pSttNd = new SwStartNode( *this, nPos++ );
    pEndOfPostIts = new SwEndNode( *this, nPos++, *pSttNd );

    SwStartNode* pTmp = new SwStartNode( *this, nPos++ );
    pEndOfInserts = new SwEndNode( *this, nPos++, *pTmp );

    pTmp = new SwStartNode( *this, nPos++ );
    pTmp->pStartOfSection = pSttNd;
    pEndOfAutotext = new SwEndNode( *this, nPos++, *pTmp );

    pTmp = new SwStartNode( *this, nPos++ );
    pTmp->pStartOfSection = pSttNd;
    pEndOfRedlines = new SwEndNode( *this, nPos++, *pTmp );

    pTmp = new SwStartNode( *this, nPos++ );
    pTmp->pStartOfSection = pSttNd;
    pEndOfContent = new SwEndNode( *this, nPos++, *pTmp );

    pOutlineNds = new SwOutlineNodes;
}

/*******************************************************************
|*
|*  SwNodes::~SwNodes
|*
|*  Beschreibung
|*      dtor, loescht alle Nodes, deren Pointer in diesem dynamischen
|*      Array sind. Ist kein Problem, da Nodes ausserhalb dieses
|*      Arrays nicht erzeugt werden koennen und somit auch nicht
|*      in mehreren drin sein koennen
|*
*******************************************************************/

SwNodes::~SwNodes()
{
    delete pOutlineNds;

    {
        SwNode *pNode;
        SwNodeIndex aNdIdx( *this );
        while( true )
        {
            pNode = &aNdIdx.GetNode();
            if( pNode == pEndOfContent )
                break;

            ++aNdIdx;
            delete pNode;
        }
    }

    // jetzt muessen alle SwNodeIndizies abgemeldet sein!!!
    delete pEndOfContent;
}

void SwNodes::ChgNode( SwNodeIndex& rDelPos, sal_uLong nSz,
                        SwNodeIndex& rInsPos, sal_Bool bNewFrms )
{
    // im UndoBereich brauchen wir keine Frames
    SwNodes& rNds = rInsPos.GetNodes();
    const SwNode* pPrevInsNd = rNds[ rInsPos.GetIndex() -1 ];

    //JP 03.02.99: alle Felder als invalide erklaeren, aktu. erfolgt im
    //              Idle-Handler des Docs
    if( GetDoc()->SetFieldsDirty( true, &rDelPos.GetNode(), nSz ) &&
        rNds.GetDoc() != GetDoc() )
        rNds.GetDoc()->SetFieldsDirty( true, NULL, 0 );

    //JP 12.03.99: 63293 - Nodes vom RedlineBereich NIE aufnehmen
    sal_uLong nNd = rInsPos.GetIndex();
    bool bInsOutlineIdx = !(
            rNds.GetEndOfRedlines().StartOfSectionNode()->GetIndex() < nNd &&
            nNd < rNds.GetEndOfRedlines().GetIndex() );

    if( &rNds == this )         // im gleichen Nodes-Array -> moven !!
    {
        // wird von vorne nach hinten gemovt, so wird nach vorne immer
        // nachgeschoben, d.H. die Loeschposition ist immer gleich
        sal_uInt16 nDiff = rDelPos.GetIndex() < rInsPos.GetIndex() ? 0 : 1;

        for( sal_uLong n = rDelPos.GetIndex(); nSz; n += nDiff, --nSz )
        {
            SwNodeIndex aDelIdx( *this, n );
            SwNode& rNd = aDelIdx.GetNode();

            // #i57920# - correction of refactoring done by cws swnumtree:
            // - <SwTxtNode::SetLevel( NO_NUMBERING ) is deprecated and
            //   set <IsCounted> state of the text node to <false>, which
            //   isn't correct here.
            if ( rNd.IsTxtNode() )
            {
                SwTxtNode* pTxtNode = rNd.GetTxtNode();

                pTxtNode->RemoveFromList();

                if (pTxtNode->IsOutline())
                {
                    const SwNodePtr pSrch = (SwNodePtr)&rNd;
                    pOutlineNds->erase( pSrch );
                }
            }

            BigPtrArray::Move( aDelIdx.GetIndex(), rInsPos.GetIndex() );

            if( rNd.IsTxtNode() )
            {
                SwTxtNode& rTxtNd = (SwTxtNode&)rNd;

                rTxtNd.AddToList();

                if (bInsOutlineIdx && rTxtNd.IsOutline())
                {
                    const SwNodePtr pSrch = (SwNodePtr)&rNd;
                    pOutlineNds->insert( pSrch );
                }
                rTxtNd.InvalidateNumRule();

//FEATURE::CONDCOLL
                if( RES_CONDTXTFMTCOLL == rTxtNd.GetTxtColl()->Which() )
                    rTxtNd.ChkCondColl();
//FEATURE::CONDCOLL
            }
            else if( rNd.IsCntntNode() )
                ((SwCntntNode&)rNd).InvalidateNumRule();
        }
    }
    else
    {
        bool bSavePersData(GetDoc()->GetIDocumentUndoRedo().IsUndoNodes(rNds));
        bool bRestPersData(GetDoc()->GetIDocumentUndoRedo().IsUndoNodes(*this));
        SwDoc* pDestDoc = rNds.GetDoc() != GetDoc() ? rNds.GetDoc() : 0;
        OSL_ENSURE(!pDestDoc, "SwNodes::ChgNode(): "
            "the code to handle text fields here looks broken\n"
            "if the target is in a different document.");
        if( !bRestPersData && !bSavePersData && pDestDoc )
            bSavePersData = bRestPersData = sal_True;

        String sNumRule;
        SwNodeIndex aInsPos( rInsPos );
        for( sal_uLong n = 0; n < nSz; n++ )
        {
            SwNode* pNd = &rDelPos.GetNode();

            // NoTextNode muessen ihre Persitenten Daten mitnehmen
            if( pNd->IsNoTxtNode() )
            {
                if( bSavePersData )
                    ((SwNoTxtNode*)pNd)->SavePersistentData();
            }
            else if( pNd->IsTxtNode() )
            {
                SwTxtNode* pTxtNd = (SwTxtNode*)pNd;

                // remove outline index from old nodes array
                if (pTxtNd->IsOutline())
                {
                    pOutlineNds->erase( pNd );
                }

                // muss die Rule kopiere werden?
                if( pDestDoc )
                {
                    const SwNumRule* pNumRule = pTxtNd->GetNumRule();
                    if( pNumRule && sNumRule != pNumRule->GetName() )
                    {
                        sNumRule = pNumRule->GetName();
                        SwNumRule* pDestRule = pDestDoc->FindNumRulePtr( sNumRule );
                        if( pDestRule )
                            pDestRule->SetInvalidRule( sal_True );
                        else
                            pDestDoc->MakeNumRule( sNumRule, pNumRule );
                    }
                }
                else
                    // wenns ins UndoNodes-Array gemoved wird, sollten die
                    // Numerierungen auch aktualisiert werden.
                    pTxtNd->InvalidateNumRule();

                pTxtNd->RemoveFromList();
            }

            RemoveNode( rDelPos.GetIndex(), 1, sal_False );     // Indizies verschieben !!
            SwCntntNode * pCNd = pNd->GetCntntNode();
            rNds.InsertNode( pNd, aInsPos );

            if( pCNd )
            {
                SwTxtNode* pTxtNd = pCNd->GetTxtNode();
                if( pTxtNd )
                {
                    SwpHints * const pHts = pTxtNd->GetpSwpHints();
                    // OultineNodes set the new nodes in the array
                    if (bInsOutlineIdx && pTxtNd->IsOutline())
                    {
                        rNds.pOutlineNds->insert( pTxtNd );
                    }

                    pTxtNd->AddToList();

                    // Sonderbehandlung fuer die Felder!
                    if( pHts && pHts->Count() )
                    {
                        // this looks fishy if pDestDoc != 0
                        bool const bToUndo = !pDestDoc &&
                            GetDoc()->GetIDocumentUndoRedo().IsUndoNodes(rNds);
                        for( sal_uInt16 i = pHts->Count(); i; )
                        {
                            sal_uInt16 nDelMsg = 0;
                            SwTxtAttr * const pAttr = pHts->GetTextHint( --i );
                            switch ( pAttr->Which() )
                            {
                            case RES_TXTATR_FIELD:
                                {
                                    SwTxtFld* pTxtFld =
                                        static_cast<SwTxtFld*>(pAttr);
                                    rNds.GetDoc()->InsDelFldInFldLst( !bToUndo, *pTxtFld );

                                    const SwFieldType* pTyp = pTxtFld->GetFld().GetFld()->GetTyp();
                                    if ( RES_POSTITFLD == pTyp->Which() )
                                    {
                                        rNds.GetDoc()->GetDocShell()->Broadcast( SwFmtFldHint( &pTxtFld->GetFld(), pTxtFld->GetFld().IsFldInDoc() ? SWFMTFLD_INSERTED : SWFMTFLD_REMOVED ) );
                                    }
                                    else if( RES_DDEFLD == pTyp->Which() )
                                    {
                                        if( bToUndo )
                                            ((SwDDEFieldType*)pTyp)->DecRefCnt();
                                        else
                                            ((SwDDEFieldType*)pTyp)->IncRefCnt();
                                    }
                                    nDelMsg = RES_FIELD_DELETED;
                                }
                                break;
                            case RES_TXTATR_FTN:
                                nDelMsg = RES_FOOTNOTE_DELETED;
                                break;

                            case RES_TXTATR_TOXMARK:
                                static_cast<SwTOXMark&>(pAttr->GetAttr())
                                    .InvalidateTOXMark();
                                break;

                            case RES_TXTATR_REFMARK:
                                nDelMsg = RES_REFMARK_DELETED;
                                break;

                            case RES_TXTATR_META:
                            case RES_TXTATR_METAFIELD:
                                {
                                    SwTxtMeta *const pTxtMeta(
                                        static_cast<SwTxtMeta*>(pAttr));
                                    // force removal of UNO object
                                    pTxtMeta->ChgTxtNode(0);
                                    pTxtMeta->ChgTxtNode(pTxtNd);
                                }
                                break;

                            default:
                                break;
                            }
                            if( nDelMsg && bToUndo )
                            {
                                SwPtrMsgPoolItem aMsgHint( nDelMsg,
                                                    (void*)&pAttr->GetAttr() );
                                rNds.GetDoc()->GetUnoCallBack()->
                                            ModifyNotification( &aMsgHint, &aMsgHint );
                            }
                        }
                    }
//FEATURE::CONDCOLL
                    if( RES_CONDTXTFMTCOLL == pTxtNd->GetTxtColl()->Which() )
                        pTxtNd->ChkCondColl();
//FEATURE::CONDCOLL
                }
                else
                {
                    // in unterschiedliche Docs gemoved ?
                    // dann die Daten wieder persistent machen
                    if( pCNd->IsNoTxtNode() && bRestPersData )
                        ((SwNoTxtNode*)pCNd)->RestorePersistentData();
                }
            }
        }
    }

    //JP 03.02.99: alle Felder als invalide erklaeren, aktu. erfolgt im
    //              Idle-Handler des Docs
    GetDoc()->SetFieldsDirty( true, NULL, 0 );
    if( rNds.GetDoc() != GetDoc() )
        rNds.GetDoc()->SetFieldsDirty( true, NULL, 0 );


    if( bNewFrms )
        bNewFrms = &GetDoc()->GetNodes() == (const SwNodes*)&rNds &&
                    GetDoc()->GetCurrentViewShell();    //swmod 071108//swmod 071225
    if( bNewFrms )
    {
        // Frames besorgen:
        SwNodeIndex aIdx( *pPrevInsNd, 1 );
        SwNodeIndex aFrmNdIdx( aIdx );
        SwNode* pFrmNd = rNds.FindPrvNxtFrmNode( aFrmNdIdx,
                                        rNds[ rInsPos.GetIndex() - 1 ] );

        if( !pFrmNd && aFrmNdIdx > rNds.GetEndOfExtras().GetIndex() )
        {
            OSL_ENSURE( !this, "ob das so richtig ist ??" );
            aFrmNdIdx = rNds.GetEndOfContent();
            pFrmNd = rNds.GoPrevSection( &aFrmNdIdx, sal_True, sal_False );
            if( pFrmNd && !((SwCntntNode*)pFrmNd)->GetDepends() )
                pFrmNd = 0;
            OSL_ENSURE( pFrmNd, "ChgNode() - no FrameNode found" );
        }
        if( pFrmNd )
            while( aIdx != rInsPos )
            {
                SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
                if( pCNd )
                {
                    if( pFrmNd->IsTableNode() )
                        ((SwTableNode*)pFrmNd)->MakeFrms( aIdx );
                    else if( pFrmNd->IsSectionNode() )
                        ((SwSectionNode*)pFrmNd)->MakeFrms( aIdx );
                    else
                        ((SwCntntNode*)pFrmNd)->MakeFrms( *pCNd );
                    pFrmNd = pCNd;
                }
                ++aIdx;
            }
    }
}


/***********************************************************************
|*
|*  SwNodes::Move
|*
|*  Beschreibung
|*  Move loescht die Node-Pointer ab und einschliesslich der Startposition
|*  bis zu und ausschliesslich der Endposition und fuegt sie an
|*  der vor der Zielposition ein.
|*  Wenn das Ziel vor dem ersten oder dem letzten zu bewegenden Element oder
|*  dazwischen liegt, geschieht nichts.
|*  Wenn der zu bewegende Bereich leer ist oder das Ende vor
|*  dem Anfang liegt, geschieht nichts.
|*
|*  Allg.: aRange beschreibt den Bereich  -exklusive- aEnd !!
|*              ( 1.Node: aStart, letzer Node: aEnd-1 !! )
|*
***********************************************************************/

sal_Bool SwNodes::_MoveNodes( const SwNodeRange& aRange, SwNodes & rNodes,
                    const SwNodeIndex& aIndex, sal_Bool bNewFrms )
{
    SwNode * pAktNode;
    if( aIndex == 0 ||
        ( (pAktNode = &aIndex.GetNode())->GetStartNode() &&
          !pAktNode->StartOfSectionIndex() ))
        return sal_False;

    SwNodeRange aRg( aRange );

    // "einfache" StartNodes oder EndNodes ueberspringen
    while( ND_STARTNODE == (pAktNode = &aRg.aStart.GetNode())->GetNodeType()
            || ( pAktNode->IsEndNode() &&
                !pAktNode->pStartOfSection->IsSectionNode() ) )
        aRg.aStart++;
    aRg.aStart--;

    // falls aEnd-1 auf keinem ContentNode steht, dann suche den vorherigen
    aRg.aEnd--;
    while( ( (( pAktNode = &aRg.aEnd.GetNode())->GetStartNode() &&
            !pAktNode->IsSectionNode() ) ||
            ( pAktNode->IsEndNode() &&
            ND_STARTNODE == pAktNode->pStartOfSection->GetNodeType()) ) &&
            aRg.aEnd > aRg.aStart )
        aRg.aEnd--;


    // wird im selben Array's verschoben, dann ueberpruefe die Einfuegepos.
    if( aRg.aStart >= aRg.aEnd )
        return sal_False;

    if( this == &rNodes )
    {
        if( ( aIndex.GetIndex()-1 >= aRg.aStart.GetIndex() &&
              aIndex.GetIndex()-1 < aRg.aEnd.GetIndex()) ||
            ( aIndex.GetIndex()-1 == aRg.aEnd.GetIndex() ) )
            return sal_False;
    }

    sal_uInt16 nLevel = 0;                  // Level-Counter
    sal_uLong nInsPos = 0;                  // Cnt fuer das TmpArray

    // das Array bildet einen Stack, es werden alle StartOfSelction's gesichert
    SwSttNdPtrs aSttNdStack;

    // setze den Start-Index
    SwNodeIndex  aIdx( aIndex );

    SwStartNode* pStartNode = aIdx.GetNode().pStartOfSection;
    aSttNdStack.insert( aSttNdStack.begin(), pStartNode );

    SwNodeRange aOrigInsPos( aIdx, -1, aIdx );      // Originale Insert Pos

    //JP 16.01.98: SectionNodes: DelFrms/MakeFrms beim obersten SectionNode!
    sal_uInt16 nSectNdCnt = 0;
    sal_Bool bSaveNewFrms = bNewFrms;

    // bis alles verschoben ist
    while( aRg.aStart < aRg.aEnd )
        switch( (pAktNode = &aRg.aEnd.GetNode())->GetNodeType() )
        {
        case ND_ENDNODE:
            {
                if( nInsPos )       // verschieb schon mal alle bis hier her
                {
                    // loeschen und kopieren. ACHTUNG: die Indizies ab
                    // "aRg.aEnd+1" werden mit verschoben !!
                    SwNodeIndex aSwIndex( aRg.aEnd, 1 );
                    ChgNode( aSwIndex, nInsPos, aIdx, bNewFrms );
                    aIdx -= nInsPos;
                    nInsPos = 0;
                }

                SwStartNode* pSttNd = pAktNode->pStartOfSection;
                if( pSttNd->IsTableNode() )
                {
                    SwTableNode* pTblNd = (SwTableNode*)pSttNd;

                    // dann bewege die gesamte Tabelle/den Bereich !!
                    nInsPos = (aRg.aEnd.GetIndex() -
                                    pSttNd->GetIndex() )+1;
                    aRg.aEnd -= nInsPos;

                    //JP 12.03.99: 63293 - Nodes vom RedlineBereich NIE aufnehmen
                    sal_uLong nNd = aIdx.GetIndex();
                    bool bInsOutlineIdx = !( rNodes.GetEndOfRedlines().
                            StartOfSectionNode()->GetIndex() < nNd &&
                            nNd < rNodes.GetEndOfRedlines().GetIndex() );

                    if( bNewFrms )
                        // loesche erstmal die Frames
                        pTblNd->DelFrms();
                    if( &rNodes == this )   // in sich selbst moven ??
                    {
                        // dann bewege alle Start/End/ContentNodes. Loesche
                        // bei den ContentNodes auch die Frames !!
                        pTblNd->pStartOfSection = aIdx.GetNode().pStartOfSection;
                        for( sal_uLong n = 0; n < nInsPos; ++n )
                        {
                            SwNodeIndex aMvIdx( aRg.aEnd, 1 );
                            SwCntntNode* pCNd = 0;
                            SwNode* pTmpNd = &aMvIdx.GetNode();
                            if( pTmpNd->IsCntntNode() )
                            {
                                pCNd = (SwCntntNode*)pTmpNd;
                                if( pTmpNd->IsTxtNode() )
                                    ((SwTxtNode*)pTmpNd)->RemoveFromList();

                                // remove outline index from old nodes array
                                if (pCNd->IsTxtNode() &&
                                    static_cast<SwTxtNode*>(pCNd)->IsOutline())
                                {
                                    pOutlineNds->erase( pCNd );
                                }
                                else
                                    pCNd = 0;
                            }

                            BigPtrArray::Move( aMvIdx.GetIndex(), aIdx.GetIndex() );

                            if( bInsOutlineIdx && pCNd )
                                pOutlineNds->insert( pCNd );
                            if( pTmpNd->IsTxtNode() )
                                ((SwTxtNode*)pTmpNd)->AddToList();
                        }
                    }
                    else
                    {
                        // StartNode holen
                        // Even aIdx points to a startnode, we need the startnode
                        // of the environment of aIdx (#i80941)
                        SwStartNode* pSttNode = aIdx.GetNode().pStartOfSection;

                        // Hole alle Boxen mit Inhalt. Deren Indizies auf die
                        // StartNodes muessen umgemeldet werden !!
                        // (Array kopieren und alle gefunden wieder loeschen;
                        //  erleichtert das suchen!!)
                        SwNodeIndex aMvIdx( aRg.aEnd, 1 );
                        for( sal_uLong n = 0; n < nInsPos; ++n )
                        {
                            SwNode* pNd = &aMvIdx.GetNode();

                            const bool bOutlNd = pNd->IsTxtNode() &&
                                static_cast<SwTxtNode*>(pNd)->IsOutline();
                            // loesche die Gliederungs-Indizies aus
                            // dem alten Nodes-Array
                            if( bOutlNd )
                                pOutlineNds->erase( pNd );

                            RemoveNode( aMvIdx.GetIndex(), 1, sal_False );
                            pNd->pStartOfSection = pSttNode;
                            rNodes.InsertNode( pNd, aIdx );

                            // setze bei Start/EndNodes die richtigen Indizies
                            if( bInsOutlineIdx && bOutlNd )
                                // und setze sie im neuen Nodes-Array
                                rNodes.pOutlineNds->insert( pNd );
                            else if( pNd->IsStartNode() )
                                pSttNode = (SwStartNode*)pNd;
                            else if( pNd->IsEndNode() )
                            {
                                pSttNode->pEndOfSection = (SwEndNode*)pNd;
                                if( pSttNode->IsSectionNode() )
                                    ((SwSectionNode*)pSttNode)->NodesArrChgd();
                                pSttNode = pSttNode->pStartOfSection;
                            }
                        }

                        if( pTblNd->GetTable().IsA( TYPE( SwDDETable ) ))
                        {
                            SwDDEFieldType* pTyp = ((SwDDETable&)pTblNd->
                                                GetTable()).GetDDEFldType();
                            if( pTyp )
                            {
                                if( rNodes.IsDocNodes() )
                                    pTyp->IncRefCnt();
                                else
                                    pTyp->DecRefCnt();
                            }
                        }

                        if (GetDoc()->GetIDocumentUndoRedo().IsUndoNodes(
                                    rNodes))
                        {
                            SwFrmFmt* pTblFmt = pTblNd->GetTable().GetFrmFmt();
                            SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT,
                                                        pTblFmt );
                            pTblFmt->ModifyNotification( &aMsgHint, &aMsgHint );
                        }
                    }
                    if( bNewFrms )
                    {
                        SwNodeIndex aTmp( aIdx );
                        pTblNd->MakeFrms( &aTmp );
                    }
                    aIdx -= nInsPos;
                    nInsPos = 0;
                }
                else if( pSttNd->GetIndex() < aRg.aStart.GetIndex() )
                {
                    // SectionNode: es wird nicht die gesamte Section
                    //              verschoben, also bewege nur die
                    //              ContentNodes
                    // StartNode:   erzeuge an der Postion eine neue Section
                    do {        // middle check loop
                        if( !pSttNd->IsSectionNode() )
                        {
                            // Start und EndNode an der InsertPos erzeugen
                            SwStartNode* pTmp = new SwStartNode( aIdx,
                                                    ND_STARTNODE,
/*?? welcher NodeTyp ??*/
                                                    SwNormalStartNode );

                            nLevel++;           // den Index auf StartNode auf den Stack
                            aSttNdStack.insert( aSttNdStack.begin() + nLevel, pTmp );

                            // noch den EndNode erzeugen
                            new SwEndNode( aIdx, *pTmp );
                        }
                        else if (GetDoc()->GetIDocumentUndoRedo().IsUndoNodes(
                                    rNodes))
                        {
                            // im UndoNodes-Array spendieren wir einen
                            // Platzhalter
                            new SwDummySectionNode( aIdx );
                        }
                        else
                        {
                            // JP 18.5.2001: neue Section anlegen?? Bug 70454
                            aRg.aEnd--;
                            break;

                        }

                        aRg.aEnd--;
                        aIdx--;
                    } while( false );
                }
                else
                {
                    // Start und EndNode komplett verschieben
// s. u. SwIndex aOldStt( pSttNd->theIndex );
//JP 21.05.97: sollte der Start genau der Start des Bereiches sein, so muss
//              der Node auf jedenfall noch besucht werden!
                    if( &aRg.aStart.GetNode() == pSttNd )
                        --aRg.aStart;

                    SwSectionNode* pSctNd = pSttNd->GetSectionNode();
                    if( bNewFrms && pSctNd )
                        pSctNd->DelFrms();

                    RemoveNode( aRg.aEnd.GetIndex(), 1, sal_False ); // EndNode loeschen
                    sal_uLong nSttPos = pSttNd->GetIndex();

                    // dieser StartNode wird spaeter wieder entfernt!
                    SwStartNode* pTmpSttNd = new SwStartNode( *this, nSttPos+1 );
                    pTmpSttNd->pStartOfSection = pSttNd->pStartOfSection;

                    RemoveNode( nSttPos, 1, sal_False ); // SttNode loeschen

                    pSttNd->pStartOfSection = aIdx.GetNode().pStartOfSection;
                    rNodes.InsertNode( pSttNd, aIdx  );
                    rNodes.InsertNode( pAktNode, aIdx );
                    aIdx--;
                    pSttNd->pEndOfSection = (SwEndNode*)pAktNode;

                    aRg.aEnd--;

                    nLevel++;           // den Index auf StartNode auf den Stack
                    aSttNdStack.insert( aSttNdStack.begin() + nLevel, pSttNd );

                    // SectionNode muss noch ein paar Indizies ummelden
                    if( pSctNd )
                    {
                        pSctNd->NodesArrChgd();
                        ++nSectNdCnt;
                        bNewFrms = sal_False;
                    }
                }
            }
            break;



        case ND_SECTIONNODE:
            if( !nLevel &&
                GetDoc()->GetIDocumentUndoRedo().IsUndoNodes(rNodes))
            {
                // dann muss an der akt. InsPos ein SectionDummyNode
                // eingefuegt werden
                if( nInsPos )       // verschieb schon mal alle bis hier her
                {
                    // loeschen und kopieren. ACHTUNG: die Indizies ab
                    // "aRg.aEnd+1" werden mit verschoben !!
                    SwNodeIndex aSwIndex( aRg.aEnd, 1 );
                    ChgNode( aSwIndex, nInsPos, aIdx, bNewFrms );
                    aIdx -= nInsPos;
                    nInsPos = 0;
                }
                new SwDummySectionNode( aIdx );
                aRg.aEnd--;
                aIdx--;
                break;
            }
            // kein break !!
        case ND_TABLENODE:
        case ND_STARTNODE:
            {
                // empty section -> nothing to do
                //  and only if it's a top level section
                if( !nInsPos && !nLevel )
                {
                    aRg.aEnd--;
                    break;
                }

                if( !nLevel )       // es wird eine Stufe runter gestuft
                {
                    // erzeuge die Runterstufung
                    SwNodeIndex aTmpSIdx( aOrigInsPos.aStart, 1 );
                    SwStartNode* pTmpStt = new SwStartNode( aTmpSIdx,
                                ND_STARTNODE,
                                ((SwStartNode*)pAktNode)->GetStartNodeType() );

                    aTmpSIdx--;

                    SwNodeIndex aTmpEIdx( aOrigInsPos.aEnd );
                    new SwEndNode( aTmpEIdx, *pTmpStt );
                    aTmpEIdx--;
                    ++aTmpSIdx;

                    // setze die StartOfSection richtig
                    aRg.aEnd++;
                    {
                        SwNodeIndex aCntIdx( aRg.aEnd );
                        for( sal_uLong n = 0; n < nInsPos; n++, aCntIdx++)
                            aCntIdx.GetNode().pStartOfSection = pTmpStt;
                    }

                    // Setze auch bei allen runtergestuften den richtigen StartNode
                    while( aTmpSIdx < aTmpEIdx )
                        if( 0 != (( pAktNode = &aTmpEIdx.GetNode())->GetEndNode()) )
                            aTmpEIdx = pAktNode->StartOfSectionIndex();
                        else
                        {
                            pAktNode->pStartOfSection = pTmpStt;
                            aTmpEIdx--;
                        }

                    aIdx--;                 // hinter den eingefuegten StartNode
                    aRg.aEnd--;             // vor den StartNode
                    // kopiere jetzt das Array. ACHTUNG: die Indizies ab
                    // "aRg.aEnd+1" werden mit verschoben !!
                    SwNodeIndex aSwIndex( aRg.aEnd, 1 );
                    ChgNode( aSwIndex, nInsPos, aIdx, bNewFrms );
                    aIdx -= nInsPos+1;
                    nInsPos = 0;
                }
                else                // es wurden alle Nodes innerhalb eines
                {                   // Start- und End-Nodes verschoben
                    OSL_ENSURE( pAktNode == aSttNdStack[nLevel] ||
                            ( pAktNode->IsStartNode() &&
                                aSttNdStack[nLevel]->IsSectionNode()),
                             "falscher StartNode" );

                    SwNodeIndex aSwIndex( aRg.aEnd, 1 );
                    ChgNode( aSwIndex, nInsPos, aIdx, bNewFrms );
                    aIdx -= nInsPos+1;      // vor den eingefuegten StartNode
                    nInsPos = 0;

                    // loesche nur noch den Pointer aus dem Nodes-Array.
                    RemoveNode( aRg.aEnd.GetIndex(), 1, sal_True );
                    aRg.aEnd--;

                    SwSectionNode* pSectNd = aSttNdStack[ nLevel ]->GetSectionNode();
                    if( pSectNd && !--nSectNdCnt )
                    {
                        SwNodeIndex aTmp( *pSectNd );
                        pSectNd->MakeFrms( &aTmp );
                        bNewFrms = bSaveNewFrms;
                    }
                    aSttNdStack.erase( aSttNdStack.begin() + nLevel );   // vom Stack loeschen
                    nLevel--;
                }

                // loesche alle entstehenden leeren Start-/End-Node-Paare
                SwNode* pTmpNode = (*this)[ aRg.aEnd.GetIndex()+1 ]->GetEndNode();
                if( pTmpNode && ND_STARTNODE == (pAktNode = &aRg.aEnd.GetNode())
                    ->GetNodeType() && pAktNode->StartOfSectionIndex() &&
                    pTmpNode->StartOfSectionNode() == pAktNode )
                {
                    DelNodes( aRg.aEnd, 2 );
                    aRg.aEnd--;
                }
            }
            break;

        case ND_TEXTNODE:
        case ND_GRFNODE:
        case ND_OLENODE:
            {
                if( bNewFrms && pAktNode->GetCntntNode() )
                    ((SwCntntNode*)pAktNode)->DelFrms();

                pAktNode->pStartOfSection = aSttNdStack[ nLevel ];
                nInsPos++;
                aRg.aEnd--;
            }
            break;

        case ND_SECTIONDUMMY:
            if (GetDoc()->GetIDocumentUndoRedo().IsUndoNodes(*this))
            {
                if( &rNodes == this )       // innerhalb vom UndoNodesArray
                {
                    // mit verschieben
                    pAktNode->pStartOfSection = aSttNdStack[ nLevel ];
                    nInsPos++;
                }
                else    // in ein "normales" Nodes-Array verschieben
                {
                    // dann muss an der akt. InsPos auch ein SectionNode
                    // (Start/Ende) stehen; dann diesen ueberspringen.
                    // Andernfalls nicht weiter beachten.
                    if( nInsPos )       // verschieb schon mal alle bis hier her
                    {
                        // loeschen und kopieren. ACHTUNG: die Indizies ab
                        // "aRg.aEnd+1" werden mit verschoben !!
                        SwNodeIndex aSwIndex( aRg.aEnd, 1 );
                        ChgNode( aSwIndex, nInsPos, aIdx, bNewFrms );
                        aIdx -= nInsPos;
                        nInsPos = 0;
                    }
                    SwNode* pTmpNd = &aIdx.GetNode();
                    if( pTmpNd->IsSectionNode() ||
                        pTmpNd->StartOfSectionNode()->IsSectionNode() )
                        aIdx--; // ueberspringen
                }
            }
            else {
                OSL_FAIL( "wie kommt diser Node ins Nodes-Array??" );
            }
            aRg.aEnd--;
            break;

        default:
            OSL_FAIL( "was ist das fuer ein Node??" );
            break;
        }

    if( nInsPos )                           // kopiere den Rest
    {
        // der Rest muesste so stimmen
        SwNodeIndex aSwIndex( aRg.aEnd, 1 );
        ChgNode( aSwIndex, nInsPos, aIdx, bNewFrms );
    }
    aRg.aEnd++;                     // wieder exklusive Ende

    // loesche alle leeren Start-/End-Node-Paare
    if( ( pAktNode = &aRg.aStart.GetNode())->GetStartNode() &&
        pAktNode->StartOfSectionIndex() &&
        aRg.aEnd.GetNode().GetEndNode() )
            DelNodes( aRg.aStart, 2 );

    // rufe jetzt noch das Update fuer die Gliederung/Nummerierung auf
    aOrigInsPos.aStart++;
    // im gleichen Nodes-Array verschoben ??,
    // dann von oben nach unten das Update aufrufen !!
    if( this == &rNodes &&
        aRg.aEnd.GetIndex() >= aOrigInsPos.aStart.GetIndex() )
    {
        UpdtOutlineIdx( aOrigInsPos.aStart.GetNode() );
        UpdtOutlineIdx( aRg.aEnd.GetNode() );
    }
    else
    {
        UpdtOutlineIdx( aRg.aEnd.GetNode() );
        rNodes.UpdtOutlineIdx( aOrigInsPos.aStart.GetNode() );
    }

    return sal_True;
}


/*******************************************************************
|*
|*  SwNodes::SectionDown
|*
|*  Beschreibung
|*    SectionDown() legt ein Paar von Start- und EndSection-Node
|*    (andere Nodes koennen dazwischen liegen) an.
|*
|*    Zustand des SRange beim Verlassen der Funktion: nStart ist der
|*    Index des ersten Node hinter dem Start Section Node, nEnd ist
|*    der Index des End Section Nodes. Beispiel: Wird Insert Section
|*    mehrmals hintereinander aufgerufen, so werden mehrere
|*    unmittelbar geschachtelte Sections (keine Content Nodes
|*    zwischen Start- bzw. End Nodes) angelegt.
|*
|*  Allg.: aRange beschreibt den Bereich  -exklusive- aEnd !!
|*              ( 1.Node: aStart, letzer Node: aEnd-1 !! )
|*
|*  Parameter
|*      SwRange &rRange
|*          IO:
|*          IN
|*          rRange.aStart: Einfuegeposition des StartNodes
|*          rRange.aEnd: Einfuegeposition des EndNodes
|*          OUT
|*          rRange.aStart: steht hinter dem eingefuegten Startnode
|*          rRange.aEnd: steht auf dem eingefuegen Endnode
|*
|*  Ausnahmen
|*   1. SRange-Anfang und SRange-Ende muessen auf dem gleichen Level sein
|*   2. duerfen nicht auf dem obersten Level sein
|*      Ist dies nicht der Fall, wird die
|*      Funktion durch Aufruf von ERR_RAISE verlassen.
|*
|*  Debug-Funktionen
|*      die Debugging Tools geben rRange beim Eintritt und beim
|*      Verlassen der Funktion aus
|*
*******************************************************************/
void SwNodes::SectionDown(SwNodeRange *pRange, SwStartNodeType eSttNdTyp )
{
    if( pRange->aStart >= pRange->aEnd ||
        pRange->aEnd >= Count() ||
        !CheckNodesRange( pRange->aStart, pRange->aEnd ))
        return;

    // Ist der Anfang vom Bereich vor oder auf einem EndNode, so loesche
    // diesen, denn sonst wuerden leere S/E-Nodes oder E/S-Nodes enstehen.
    // Bei anderen Nodes wird eine neuer StartNode eingefuegt
    SwNode * pAktNode = &pRange->aStart.GetNode();
    SwNodeIndex aTmpIdx( *pAktNode->StartOfSectionNode() );

    if( pAktNode->GetEndNode() )
        DelNodes( pRange->aStart, 1 );      // verhinder leere Section
    else
    {
        // fuege einen neuen StartNode ein
        SwNode* pSttNd = new SwStartNode( pRange->aStart, ND_STARTNODE, eSttNdTyp );
        pRange->aStart = *pSttNd;
        aTmpIdx = pRange->aStart;
    }

    // Ist das Ende vom Bereich vor oder auf einem StartNode, so loesche
    // diesen, denn sonst wuerden leere S/E-Nodes oder E/S-Nodes enstehen
    // Bei anderen Nodes wird eine neuer EndNode eingefuegt
    pRange->aEnd--;
    if( pRange->aEnd.GetNode().GetStartNode() )
        DelNodes( pRange->aEnd, 1 );
    else
    {
        pRange->aEnd++;
        // fuege einen neuen EndNode ein
        new SwEndNode( pRange->aEnd, *pRange->aStart.GetNode().GetStartNode() );
    }
    pRange->aEnd--;

    SectionUpDown( aTmpIdx, pRange->aEnd );
}

/*******************************************************************
|*
|*  SwNodes::SectionUp
|*
|*  Beschreibung
|*      Der von rRange umspannte Bereich wird auf die naechst hoehere
|*      Ebene gehoben. Das geschieht dadurch, dass bei
|*      rRange.aStart ein Endnode und bei rRange.aEnd ein
|*      Startnode eingefuegt wird. Die Indices fuer den Bereich
|*      innerhalb von rRange werden geupdated.
|*
|*  Allg.: aRange beschreibt den Bereich  -exklusive- aEnd !!
|*              ( 1.Node: aStart, letzer Node: aEnd-1 !! )
|*
|*  Parameter
|*      SwRange &rRange
|*          IO:
|*          IN
|*          rRange.aStart: Anfang des hoeher zubewegenden Bereiches
|*          rRange.aEnd:   der 1.Node hinter dem Bereich
|*          OUT
|*          rRange.aStart:  an der ersten Position innerhalb des
|*                          hochbewegten Bereiches
|*          rRange.aEnd:    an der letzten Position innerhalb des
|*                          hochbewegten Bereiches
|*
|*  Debug-Funktionen
|*      die Debugging Tools geben rRange beim Eintritt und beim
|*      Verlassen der Funktion aus
|*
*******************************************************************/
void SwNodes::SectionUp(SwNodeRange *pRange)
{
    if( pRange->aStart >= pRange->aEnd ||
        pRange->aEnd >= Count() ||
        !CheckNodesRange( pRange->aStart, pRange->aEnd ) ||
        !( HighestLevel( *this, *pRange ) > 1 ))
        return;

    // Ist der Anfang vom Bereich vor oder auf einem StartNode, so loesche
    // diesen, denn sonst wuerden leere S/E-Nodes oder E/S-Nodes enstehen.
    // Bei anderen Nodes wird eine neuer EndNode eingefuegt
    SwNode * pAktNode = &pRange->aStart.GetNode();
    SwNodeIndex aIdx( *pAktNode->StartOfSectionNode() );
    if( pAktNode->IsStartNode() )       // selbst StartNode
    {
        SwEndNode* pEndNd = pRange->aEnd.GetNode().GetEndNode();
        if( pAktNode == pEndNd->pStartOfSection )
        {
            // dann wurde paarig aufgehoben, also nur die im Berich neu anpassen
            SwStartNode* pTmpSttNd = pAktNode->pStartOfSection;
            RemoveNode( pRange->aStart.GetIndex(), 1, sal_True );
            RemoveNode( pRange->aEnd.GetIndex(), 1, sal_True );

            SwNodeIndex aTmpIdx( pRange->aStart );
            while( aTmpIdx < pRange->aEnd )
            {
                pAktNode = &aTmpIdx.GetNode();
                pAktNode->pStartOfSection = pTmpSttNd;
                if( pAktNode->IsStartNode() )
                    aTmpIdx = pAktNode->EndOfSectionIndex() + 1;
                else
                    ++aTmpIdx;
            }
            return ;
        }
        DelNodes( pRange->aStart, 1 );
    }
    else if( aIdx == pRange->aStart.GetIndex()-1 )          // vor StartNode
        DelNodes( aIdx, 1 );
    else
        new SwEndNode( pRange->aStart, *aIdx.GetNode().GetStartNode() );

    // Ist das Ende vom Bereich vor oder auf einem StartNode, so loesche
    // diesen, denn sonst wuerden leere S/E-Nodes oder E/S-Nodes entstehen
    // Bei anderen Nodes wird eine neuer EndNode eingefuegt
    SwNodeIndex aTmpIdx( pRange->aEnd );
    if( pRange->aEnd.GetNode().IsEndNode() )
        DelNodes( pRange->aEnd, 1 );
    else
    {
        pAktNode = new SwStartNode( pRange->aEnd );
/*?? welcher NodeTyp ??*/
        aTmpIdx = *pRange->aEnd.GetNode().EndOfSectionNode();
        pRange->aEnd--;
    }

    SectionUpDown( aIdx, aTmpIdx );
}


/*************************************************************************
|*
|*  SwNodes::SectionUpDown()
|*
|*  Beschreibung
|*      Methode setzt die Indizies die bei SectionUp oder SectionDwon
|*      veraendert wurden wieder richtig, sodass die Ebenen wieder
|*      Konsistent sind.
|*
|*    Parameter
|*                      SwIndex & aStart        StartNode !!!
|*                      SwIndex & aEnd          EndPunkt
|*
*************************************************************************/
void SwNodes::SectionUpDown( const SwNodeIndex & aStart, const SwNodeIndex & aEnd )
{
    SwNode * pAktNode;
    SwNodeIndex aTmpIdx( aStart, +1 );
    // das Array bildet einen Stack, es werden alle StartOfSelction's gesichert
    SwSttNdPtrs aSttNdStack;
    SwStartNode* pTmp = aStart.GetNode().GetStartNode();
    aSttNdStack.push_back( pTmp );

    // durchlaufe bis der erste zu aendernde Start-Node gefunden wurde
    // ( Es wird vom eingefuegten EndNode bis nach vorne die Indexe gesetzt )
    for( ;; ++aTmpIdx )
    {
        pAktNode = &aTmpIdx.GetNode();
        pAktNode->pStartOfSection = aSttNdStack[ aSttNdStack.size()-1 ];

        if( pAktNode->GetStartNode() )
        {
            pTmp = (SwStartNode*)pAktNode;
            aSttNdStack.push_back( pTmp );
        }
        else if( pAktNode->GetEndNode() )
        {
            SwStartNode* pSttNd = aSttNdStack[ aSttNdStack.size() - 1 ];
            pSttNd->pEndOfSection = (SwEndNode*)pAktNode;
            aSttNdStack.pop_back();
            if( !aSttNdStack.empty() )
                continue;       // noch genuegend EndNodes auf dem Stack

            else if( aTmpIdx < aEnd )   // Uebergewicht an StartNodes
                // ist das Ende noch nicht erreicht, so hole den Start von
                // der uebergeordneten Section
            {
                aSttNdStack.insert( aSttNdStack.begin(), pSttNd->pStartOfSection );
            }
            else    // wenn ueber den Bereich hinaus, dann Ende
                break;
        }
    }
}




/*******************************************************************
|*
|*  SwNodes::Delete
|*
|*  Beschreibung
|*      Spezielle Implementierung der Delete-Funktion des
|*      variablen Array. Diese spezielle Implementierung ist
|*      notwendig, da durch das Loeschen von Start- bzw.
|*      Endnodes Inkonsistenzen entstehen koennen. Diese werden
|*      durch diese Funktion beseitigt.
|*
|*  Parameter
|*      IN
|*      SwIndex &rIndex bezeichnet die Position, an der
|*      geloescht wird
|*      rIndex ist nach Aufruf der Funktion unveraendert (Kopie?!)
|*      sal_uInt16 nNodes bezeichnet die Anzahl der zu loeschenden
|*      Nodes; ist auf 1 defaulted
|*
|*  Debug-Funktionen
|*      geben beim Eintritt in die Funktion Position und Anzahl
|*      der zu loeschenden Nodes aus.
|*
*******************************************************************/
void SwNodes::Delete(const SwNodeIndex &rIndex, sal_uLong nNodes)
{
    sal_uInt16 nLevel = 0;                      // Level-Counter
    SwNode * pAktNode;

    sal_uLong nCnt = Count() - rIndex.GetIndex() - 1;
    if( nCnt > nNodes ) nCnt = nNodes;

    if( nCnt == 0 )         // keine Anzahl -> return
        return;

    SwNodeRange aRg( rIndex, 0, rIndex, nCnt-1 );
    // ueberprufe ob rIndex..rIndex + nCnt ueber einen Bereich hinausragt !!
    if( ( !aRg.aStart.GetNode().StartOfSectionIndex() &&
            !aRg.aStart.GetIndex() ) ||
            ! CheckNodesRange( aRg.aStart, aRg.aEnd ) )
        return;


    // falls aEnd auf keinem ContentNode steht, dann suche den vorherigen
    while( ( pAktNode = &aRg.aEnd.GetNode())->GetStartNode() ||
             ( pAktNode->GetEndNode() &&
                !pAktNode->pStartOfSection->IsTableNode() ))
        aRg.aEnd--;

    nCnt = 0;
    // Start erhoehen, damit auf < abgefragt wird. ( bei <= kann es zu
    // Problemen fuehren; ist aEnd == aStart und wird aEnd geloscht,
    // so ist aEnd <= aStart
    aRg.aStart--;

    sal_Bool bSaveInNodesDel = bInNodesDel;
    bInNodesDel = sal_True;
    bool bUpdateOutline = false;

    // bis alles geloescht ist
    while( aRg.aStart < aRg.aEnd )
    {
        pAktNode = &aRg.aEnd.GetNode();

        if( pAktNode->GetEndNode() )
        {
            // die gesamte Section loeschen ?
            if( pAktNode->StartOfSectionIndex() > aRg.aStart.GetIndex() )
            {
                SwTableNode* pTblNd = pAktNode->pStartOfSection->GetTableNode();
                if( pTblNd )
                    pTblNd->DelFrms();

                SwNode *pNd, *pChkNd = pAktNode->pStartOfSection;
                sal_uInt16 nIdxPos;
                do {
                    pNd = &aRg.aEnd.GetNode();

                    if( pNd->IsTxtNode() )
                    {
                        SwTxtNode *const pTxtNode(static_cast<SwTxtNode*>(pNd));
                        if (pTxtNode->IsOutline() &&
                                pOutlineNds->Seek_Entry( pNd, &nIdxPos ))
                        {
                            // loesche die Gliederungs-Indizies.
                            pOutlineNds->erase(nIdxPos);
                            bUpdateOutline = true;
                        }
                        pTxtNode->InvalidateNumRule();
                    }
                    else if( pNd->IsEndNode() &&
                            pNd->pStartOfSection->IsTableNode() )
                        ((SwTableNode*)pNd->pStartOfSection)->DelFrms();

                    aRg.aEnd--;
                    nCnt++;

                } while( pNd != pChkNd );
            }
            else
            {
                RemoveNode( aRg.aEnd.GetIndex()+1, nCnt, sal_True );    // loesche
                nCnt = 0;
                aRg.aEnd--;             // vor den EndNode
                nLevel++;
            }
        }
        else if( pAktNode->GetStartNode() )   // StartNode gefunden
        {
            if( nLevel == 0 )       // es wird eine Stufe runter gestuft
            {
                if( nCnt )
                {
                    // loesche jetzt das Array
                    aRg.aEnd++;
                    RemoveNode( aRg.aEnd.GetIndex(), nCnt, sal_True );
                    nCnt = 0;
                }
            }
            else    // es werden alle Nodes Innerhalb eines Start- und
            {       // End-Nodes geloescht, loesche mit Start/EndNode
                RemoveNode( aRg.aEnd.GetIndex(), nCnt + 2, sal_True );          // loesche Array
                nCnt = 0;
                nLevel--;
            }

            // nach dem loeschen kann aEnd auf einem EndNode stehen
            // loesche alle leeren Start-/End-Node-Paare
            SwNode* pTmpNode = aRg.aEnd.GetNode().GetEndNode();
            aRg.aEnd--;
            while(  pTmpNode &&
                    ( pAktNode = &aRg.aEnd.GetNode())->GetStartNode() &&
                    pAktNode->StartOfSectionIndex() )
            {
                // loesche den EndNode und StartNode
                DelNodes( aRg.aEnd, 2 );
                pTmpNode = aRg.aEnd.GetNode().GetEndNode();
                aRg.aEnd--;
            }
        }
        else        // normaler Node, also ins TmpArray einfuegen
        {
            SwTxtNode* pTxtNd = pAktNode->GetTxtNode();
            if( pTxtNd )
            {
                if( pTxtNd->IsOutline())
                {                   // loesche die Gliederungs-Indizies.
                    pOutlineNds->erase( pTxtNd );
                    bUpdateOutline = true;
                }
                pTxtNd->InvalidateNumRule();
            }
            else if( pAktNode->IsCntntNode() )
                ((SwCntntNode*)pAktNode)->InvalidateNumRule();

            aRg.aEnd--;
            nCnt++;
        }
    }

    aRg.aEnd++;
    if( nCnt != 0 )
        RemoveNode( aRg.aEnd.GetIndex(), nCnt, sal_True );              // loesche den Rest

    // loesche alle leeren Start-/End-Node-Paare
    while( aRg.aEnd.GetNode().GetEndNode() &&
            ( pAktNode = &aRg.aStart.GetNode())->GetStartNode() &&
            pAktNode->StartOfSectionIndex() )
    // aber ja keinen der heiligen 5.
    {
        DelNodes( aRg.aStart, 2 );  // loesche den Start- und EndNode
        aRg.aStart--;
    }

    bInNodesDel = bSaveInNodesDel;

    if( !bInNodesDel )
    {
        // rufe jetzt noch das Update fuer die Gliederung/Nummerierung auf
        if( bUpdateOutline || bInDelUpdOutl )
        {
            UpdtOutlineIdx( aRg.aEnd.GetNode() );
            bInDelUpdOutl = sal_False;
        }

    }
    else
    {
        if( bUpdateOutline )
            bInDelUpdOutl = sal_True;
    }
}

/*******************************************************************
|*
|*  SwNodes::GetSectionLevel
|*
|*  Beschreibung
|*      Die Funktion liefert den Sectionlevel an der durch
|*      aIndex bezeichneten Position. Die Funktion ruft die
|*      GetSectionlevel-Funktion des durch aIndex bezeichneten
|*      Nodes. Diese ist eine virtuelle Funktion, die fuer
|*      Endnodes speziell implementiert werden musste.
|*      Die Sectionlevels werden ermittelt, indem rekursiv durch
|*      die Nodesstruktur (jeweils zum naechsten theEndOfSection)
|*      gegangen wird, bis die oberste Ebene erreicht ist
|*      (theEndOfSection == 0)
|*
|*  Parameter
|*      aIndex bezeichnet die Position des Nodes, dessen
|*      Sectionlevel ermittelt werden soll. Hier wird eine Kopie
|*      uebergeben, da eine Veraenderung der Variablen in der
|*      rufenden Funktion nicht wuenschenswert ist.
|*
|*  Ausnahmen
|*      Der erste Node im Array  sollte immer ein Startnode sein.
|*      Dieser erfaehrt in der Funktion SwNodes::GetSectionLevel()
|*      eine Sonderbehandlung; es wird davon ausgegangen, dass der
|*      erste Node auch ein Startnode ist.
|*
*******************************************************************/
sal_uInt16 SwNodes::GetSectionLevel(const SwNodeIndex &rIdx) const {
    // Sonderbehandlung 1. Node
    if(rIdx == 0) return 1;
    /*
     * Keine Rekursion! - hier wird das SwNode::GetSectionLevel
     * aufgerufen
     */
    return rIdx.GetNode().GetSectionLevel();
}

void SwNodes::GoStartOfSection(SwNodeIndex *pIdx) const
{
    // hinter den naechsten Startnode
    SwNodeIndex aTmp( *pIdx->GetNode().StartOfSectionNode(), +1 );

    // steht der Index auf keinem ContentNode, dann gehe dahin. Ist aber
    // kein weiterer vorhanden, dann lasse den Index an alter Pos stehen !!!
    while( !aTmp.GetNode().IsCntntNode() )
    {   // gehe vom StartNode ( es kann nur ein StartNode sein ! ) an sein
        // Ende
        if( *pIdx <= aTmp )
            return;     // FEHLER: Steht schon hinter der Sektion
        aTmp = aTmp.GetNode().EndOfSectionIndex()+1;
        if( *pIdx <= aTmp )
            return;     // FEHLER: Steht schon hinter der Sektion
    }
    (*pIdx) = aTmp;     // steht auf einem ContentNode
}

void SwNodes::GoEndOfSection(SwNodeIndex *pIdx) const
{
    // falls er vor einem Endnode steht --> nichts tun
    if( !pIdx->GetNode().IsEndNode() )
        (*pIdx) = *pIdx->GetNode().EndOfSectionNode();
}

SwCntntNode* SwNodes::GoNext(SwNodeIndex *pIdx) const
{
    if( pIdx->GetIndex() >= Count() - 1 )
        return 0;

    SwNodeIndex aTmp(*pIdx, +1);
    SwNode* pNd = 0;
    while( aTmp < Count()-1 && 0 == ( pNd = &aTmp.GetNode())->IsCntntNode() )
        ++aTmp;

    if( aTmp == Count()-1 )
        pNd = 0;
    else
        (*pIdx) = aTmp;
    return (SwCntntNode*)pNd;
}

SwCntntNode* SwNodes::GoPrevious(SwNodeIndex *pIdx) const
{
    if( !pIdx->GetIndex() )
        return 0;

    SwNodeIndex aTmp( *pIdx, -1 );
    SwNode* pNd = 0;
    while( aTmp.GetIndex() && 0 == ( pNd = &aTmp.GetNode())->IsCntntNode() )
        aTmp--;

    if( !aTmp.GetIndex() )
        pNd = 0;
    else
        (*pIdx) = aTmp;
    return (SwCntntNode*)pNd;
}

/*************************************************************************
|*
|*    sal_Bool SwNodes::CheckNodesRange()
|*
|*    Beschreibung
|*      Teste ob der uebergene SRange nicht ueber die Grenzen der
|*      einzelnen Bereiche (PosIts, Autotext, Content, Icons und Inserts )
|*      hinaus reicht.
|*      Nach Wahrscheinlichkeit des Ranges sortiert.
|*
|*  Alg.: Da festgelegt ist, das aRange.aEnd den 1.Node hinter dem Bereich
|*        bezeichnet, wird hier auf aEnd <= End.. getestet !!
|*
|*    Parameter         SwIndex &   Start-Index vom Bereich
|*                      SwIndex &   End-Index vom Bereich
|*                      sal_Bool        sal_True:   Start+End in gleicher Section!
|*                                  sal_False:  Start+End in verschiedenen Sect.
|*    Return-Wert       sal_Bool        sal_True:   gueltiger SRange
|*                                  sal_False:  ungueltiger SRange
|*
*************************************************************************/

inline int TstIdx( sal_uLong nSttIdx, sal_uLong nEndIdx, sal_uLong nStt, sal_uLong nEnd )
{
    return nStt < nSttIdx && nEnd >= nSttIdx &&
            nStt < nEndIdx && nEnd >= nEndIdx;
}

sal_Bool SwNodes::CheckNodesRange( const SwNodeIndex& rStt, const SwNodeIndex& rEnd ) const
{
    sal_uLong nStt = rStt.GetIndex(), nEnd = rEnd.GetIndex();
    if( TstIdx( nStt, nEnd, pEndOfContent->StartOfSectionIndex(),
                pEndOfContent->GetIndex() )) return sal_True;
    if( TstIdx( nStt, nEnd, pEndOfAutotext->StartOfSectionIndex(),
                pEndOfAutotext->GetIndex() )) return sal_True;
    if( TstIdx( nStt, nEnd, pEndOfPostIts->StartOfSectionIndex(),
                pEndOfPostIts->GetIndex() )) return sal_True;
    if( TstIdx( nStt, nEnd, pEndOfInserts->StartOfSectionIndex(),
                pEndOfInserts->GetIndex() )) return sal_True;
    if( TstIdx( nStt, nEnd, pEndOfRedlines->StartOfSectionIndex(),
                pEndOfRedlines->GetIndex() )) return sal_True;

    return sal_False;       // liegt irgendwo dazwischen, FEHLER
}


/*************************************************************************
|*
|*    void SwNodes::DelNodes()
|*
|*    Beschreibung
|*      Loesche aus den NodesArray ab einer Position entsprechend Node's.
|*
|*    Parameter         SwIndex &   Der Startpunkt im Nodes-Array
|*                      sal_uInt16      die Anzahl
|*
*************************************************************************/
void SwNodes::DelNodes( const SwNodeIndex & rStart, sal_uLong nCnt )
{
    sal_uLong nSttIdx = rStart.GetIndex();

    if( !nSttIdx && nCnt == GetEndOfContent().GetIndex()+1 )
    {
        // es wird das gesamte Nodes-Array zerstoert, man ist im Doc DTOR!
        // Die initialen Start-/End-Nodes duerfen nur im SwNodes-DTOR
        // zerstoert werden!
        SwNode* aEndNdArr[] = { pEndOfContent,
                                pEndOfPostIts, pEndOfInserts,
                                pEndOfAutotext, pEndOfRedlines,
                                0
                              };

        SwNode** ppEndNdArr = aEndNdArr;
        while( *ppEndNdArr )
        {
            nSttIdx = (*ppEndNdArr)->StartOfSectionIndex() + 1;
            sal_uLong nEndIdx = (*ppEndNdArr)->GetIndex();

            if( nSttIdx != nEndIdx )
                RemoveNode( nSttIdx, nEndIdx - nSttIdx, sal_True );

            ++ppEndNdArr;
        }
    }
    else
    {
        int bUpdateNum = 0;
        for( sal_uLong n = nSttIdx, nEnd = nSttIdx + nCnt; n < nEnd; ++n )
        {
            SwNode* pNd = (*this)[ n ];

            if (pNd->IsTxtNode() && static_cast<SwTxtNode*>(pNd)->IsOutline())
            {                   // loesche die Gliederungs-Indizies.
                sal_uInt16 nIdxPos;
                if( pOutlineNds->Seek_Entry( pNd, &nIdxPos ))
                {
                    pOutlineNds->erase(nIdxPos);
                    bUpdateNum = 1;
                }
            }
            if( pNd->IsCntntNode() )
            {
                ((SwCntntNode*)pNd)->InvalidateNumRule();
                ((SwCntntNode*)pNd)->DelFrms();
            }
        }
        RemoveNode( nSttIdx, nCnt, sal_True );

        // rufe noch das Update fuer die Gliederungsnumerierung auf
        if( bUpdateNum )
            UpdtOutlineIdx( rStart.GetNode() );
    }
}


/*************************************************************************
|*
|*    sal_uInt16 HighestLevel( SwNodes & rNodes, const SwNodeRange & rRange )
|*
|*    Beschreibung
|*      Berechne den hoehsten Level innerhalb des Bereiches
|*
|*    Parameter         SwNodes &   das Node-Array
|*                      SwNodeRange &   der zu ueberpruefende Bereich
|*    Return            sal_uInt16      der hoechste Level
|*
*************************************************************************/

struct HighLevel
{
    sal_uInt16 nLevel, nTop;
    HighLevel( sal_uInt16 nLv ) : nLevel( nLv ), nTop( nLv ) {}

};

static bool lcl_HighestLevel( const SwNodePtr& rpNode, void * pPara )
{
    HighLevel * pHL = (HighLevel*)pPara;
    if( rpNode->GetStartNode() )
        pHL->nLevel++;
    else if( rpNode->GetEndNode() )
        pHL->nLevel--;
    if( pHL->nTop > pHL->nLevel )
        pHL->nTop = pHL->nLevel;
    return true;

}

sal_uInt16 HighestLevel( SwNodes & rNodes, const SwNodeRange & rRange )
{
    HighLevel aPara( rNodes.GetSectionLevel( rRange.aStart ));
    rNodes.ForEach( rRange.aStart, rRange.aEnd, lcl_HighestLevel, &aPara );
    return aPara.nTop;

}

/*************************************************************************
|*
|*    SwNodes::Move()
|*
|*    Beschreibung
|*    Parameter         SwPaM&      zu kopierender Bereich
|*                      SwNodes&    in dieses Nodes-Array
|*                      SwPosition& auf diese Position im Nodes-Array
|*
*************************************************************************/
void SwNodes::MoveRange( SwPaM & rPam, SwPosition & rPos, SwNodes& rNodes )
{
    SwPosition * const pStt = rPam.Start();
    SwPosition * const pEnd = rPam.End();

    if( !rPam.HasMark() || *pStt >= *pEnd )
        return;

    if( this == &rNodes && *pStt <= rPos && rPos < *pEnd )
        return;

    SwNodeIndex aEndIdx( pEnd->nNode );
    SwNodeIndex aSttIdx( pStt->nNode );
    SwTxtNode *const pSrcNd = aSttIdx.GetNode().GetTxtNode();
    SwTxtNode * pDestNd = rPos.nNode.GetNode().GetTxtNode();
    bool bSplitDestNd = true;
    bool bCopyCollFmt = pDestNd && pDestNd->GetTxt().isEmpty();

    if( pSrcNd )
    {
        // ist der 1.Node ein TextNode, dann muss im NodesArray auch
        // ein TextNode vorhanden sein, in den der Inhalt geschoben wird
        if( !pDestNd )
        {
            pDestNd = rNodes.MakeTxtNode( rPos.nNode, pSrcNd->GetTxtColl() );
            rPos.nNode--;
            rPos.nContent.Assign( pDestNd, 0 );
            bCopyCollFmt = true;
        }
        bSplitDestNd = pDestNd->Len() > rPos.nContent.GetIndex() ||
                        pEnd->nNode.GetNode().IsTxtNode();

        // verschiebe jetzt noch den Inhalt in den neuen Node
        bool bOneNd = pStt->nNode == pEnd->nNode;
        const xub_StrLen nLen =
                ( (bOneNd) ? pEnd->nContent.GetIndex() : pSrcNd->Len() )
                - pStt->nContent.GetIndex();

        if( !pEnd->nNode.GetNode().IsCntntNode() )
        {
            bOneNd = true;
            sal_uLong nSttNdIdx = pStt->nNode.GetIndex() + 1;
            const sal_uLong nEndNdIdx = pEnd->nNode.GetIndex();
            for( ; nSttNdIdx < nEndNdIdx; ++nSttNdIdx )
            {
                if( (*this)[ nSttNdIdx ]->IsCntntNode() )
                {
                    bOneNd = false;
                    break;
                }
            }
        }

        // das kopieren / setzen der Vorlagen darf erst nach
        // dem Splitten erfolgen
        if( !bOneNd && bSplitDestNd )
        {
            if( !rPos.nContent.GetIndex() )
            {
                bCopyCollFmt = true;
            }
            if( rNodes.IsDocNodes() )
            {
                SwDoc* const pInsDoc = pDestNd->GetDoc();
                ::sw::UndoGuard const ug(pInsDoc->GetIDocumentUndoRedo());
                pInsDoc->SplitNode( rPos, false );
            }
            else
            {
                pDestNd->SplitCntntNode( rPos );
            }

            if( rPos.nNode == aEndIdx )
            {
                aEndIdx--;
            }
            bSplitDestNd = true;

            pDestNd = rNodes[ rPos.nNode.GetIndex() - 1 ]->GetTxtNode();
            if( nLen )
            {
                pSrcNd->CutText( pDestNd, SwIndex( pDestNd, pDestNd->Len()),
                            pStt->nContent, nLen );
            }
        }
        else if ( nLen )
        {
            pSrcNd->CutText( pDestNd, rPos.nContent, pStt->nContent, nLen );
        }

        if( bCopyCollFmt )
        {
            SwDoc* const pInsDoc = pDestNd->GetDoc();
            ::sw::UndoGuard const undoGuard(pInsDoc->GetIDocumentUndoRedo());
            pSrcNd->CopyCollFmt( *pDestNd );
            bCopyCollFmt = false;
        }

        if( bOneNd )        // das wars schon
        {
            // der PaM wird korrigiert, denn falls ueber Nodegrenzen verschoben
            // wurde, so stehen sie in unterschieden Nodes. Auch die Selektion
            // wird aufgehoben !
            pEnd->nContent = pStt->nContent;
            rPam.DeleteMark();
            GetDoc()->GetDocShell()->Broadcast( SwFmtFldHint( 0,
                rNodes.IsDocNodes() ? SWFMTFLD_INSERTED : SWFMTFLD_REMOVED ) );
            return;
        }

        ++aSttIdx;
    }
    else if( pDestNd )
    {
        if( rPos.nContent.GetIndex() )
        {
            if( rPos.nContent.GetIndex() == pDestNd->Len() )
            {
                rPos.nNode++;
            }
            else if( rPos.nContent.GetIndex() )
            {
                // falls im EndNode gesplittet wird, dann muss der EndIdx
                // korrigiert werden !!
                const bool bCorrEnd = aEndIdx == rPos.nNode;
                // es wird kein Text an den TextNode angehaengt, also splitte ihn

                if( rNodes.IsDocNodes() )
                {
                    SwDoc* const pInsDoc = pDestNd->GetDoc();
                    ::sw::UndoGuard const ug(pInsDoc->GetIDocumentUndoRedo());
                    pInsDoc->SplitNode( rPos, false );
                }
                else
                {
                    pDestNd->SplitCntntNode( rPos );
                }

                pDestNd = rPos.nNode.GetNode().GetTxtNode();

                if ( bCorrEnd )
                {
                    aEndIdx--;
                }
            }
        }
        // am Ende steht noch ein leerer Text Node herum.
        bSplitDestNd = true;
    }

    SwTxtNode* const pEndSrcNd = aEndIdx.GetNode().GetTxtNode();
    if ( pEndSrcNd )
    {
        {
            // am Bereichsende entsteht ein neuer TextNode
            if( !bSplitDestNd )
            {
                if( rPos.nNode < rNodes.GetEndOfContent().GetIndex() )
                {
                    rPos.nNode++;
                }

                pDestNd =
                    rNodes.MakeTxtNode( rPos.nNode, pEndSrcNd->GetTxtColl() );
                rPos.nNode--;
                rPos.nContent.Assign( pDestNd, 0 );
            }
            else
            {
                pDestNd = rPos.nNode.GetNode().GetTxtNode();
            }

            if( pDestNd && pEnd->nContent.GetIndex() )
            {
                // verschiebe jetzt noch den Inhalt in den neuen Node
                SwIndex aIdx( pEndSrcNd, 0 );
                pEndSrcNd->CutText( pDestNd, rPos.nContent, aIdx,
                                pEnd->nContent.GetIndex());
            }

            if( bCopyCollFmt )
            {
                SwDoc* const pInsDoc = pDestNd->GetDoc();
                ::sw::UndoGuard const ug(pInsDoc->GetIDocumentUndoRedo());
                pEndSrcNd->CopyCollFmt( *pDestNd );
            }
        }
    }
    else
    {
        if ( pSrcNd && aEndIdx.GetNode().IsCntntNode() )
        {
            ++aEndIdx;
        }
        if( !bSplitDestNd )
        {
            rPos.nNode++;
            rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(), 0 );
        }
    }

    if( aEndIdx != aSttIdx )
    {
        // verschiebe jetzt die Nodes in das NodesArary
        const sal_uLong nSttDiff = aSttIdx.GetIndex() - pStt->nNode.GetIndex();
        SwNodeRange aRg( aSttIdx, aEndIdx );
        _MoveNodes( aRg, rNodes, rPos.nNode );
        // falls ins gleiche Nodes-Array verschoben wurde, stehen die
        // Indizies jetzt auch an der neuen Position !!!!
        // (also alles wieder umsetzen)
        if( &rNodes == this )
        {
            pStt->nNode = aRg.aEnd.GetIndex() - nSttDiff;
        }
    }

    // falls der Start-Node verschoben wurde, in dem der Cursor stand, so
    // muss der Content im akt. Content angemeldet werden !!!
    if ( &pStt->nNode.GetNode() == &GetEndOfContent() )
    {
        const bool bSuccess = GoPrevious( &pStt->nNode );
        OSL_ENSURE( bSuccess, "Move() - no ContentNode here" );
        (void) bSuccess;
    }
    pStt->nContent.Assign( pStt->nNode.GetNode().GetCntntNode(),
                            pStt->nContent.GetIndex() );
    // der PaM wird korrigiert, denn falls ueber Nodegrenzen verschoben
    // wurde, so stehen sie in unterschielichen Nodes. Auch die Selektion
    // wird aufgehoben !
    *pEnd = *pStt;
    rPam.DeleteMark();
    GetDoc()->GetDocShell()->Broadcast( SwFmtFldHint( 0,
                rNodes.IsDocNodes() ? SWFMTFLD_INSERTED : SWFMTFLD_REMOVED ) );
}



/*************************************************************************
|*
|*    SwNodes::_Copy()
|*
|*    Beschreibung
|*    Parameter         SwNodeRange&    zu kopierender Bereich
|*                      SwDoc&      in dieses Dokument
|*                      SwIndex&    auf diese Position im Nodes-Array
|*
*************************************************************************/

void SwNodes::_CopyNodes( const SwNodeRange& rRange,
            const SwNodeIndex& rIndex, sal_Bool bNewFrms, sal_Bool bTblInsDummyNode ) const
{
    SwDoc* pDoc = rIndex.GetNode().GetDoc();

    SwNode * pAktNode;
    if( rIndex == 0 ||
        ( (pAktNode = &rIndex.GetNode())->GetStartNode() &&
          !pAktNode->StartOfSectionIndex() ))
        return;

    SwNodeRange aRg( rRange );

    // "einfache" StartNodes oder EndNodes ueberspringen
    while( ND_STARTNODE == (pAktNode = & aRg.aStart.GetNode())->GetNodeType()
            || ( pAktNode->IsEndNode() &&
                !pAktNode->pStartOfSection->IsSectionNode() ) )
        aRg.aStart++;

    // falls aEnd-1 auf keinem ContentNode steht, dann suche den vorherigen
    aRg.aEnd--;
    // #i107142#: if aEnd is start node of a special section, do nothing.
    // Otherwise this could lead to crash: going through all previous
    // special section nodes and then one before the first.
    if (aRg.aEnd.GetNode().StartOfSectionIndex() != 0)
    {
        while( ((pAktNode = & aRg.aEnd.GetNode())->GetStartNode() &&
                !pAktNode->IsSectionNode() ) ||
                ( pAktNode->IsEndNode() &&
                ND_STARTNODE == pAktNode->pStartOfSection->GetNodeType()) )
        {
            aRg.aEnd--;
        }
    }
    aRg.aEnd++;

    // wird im selben Array's verschoben, dann ueberpruefe die Einfuegepos.
    if( aRg.aStart >= aRg.aEnd )
        return;

    // when inserting into the source range, nothing need to be done
    OSL_ENSURE( &aRg.aStart.GetNodes() == this,
                "aRg should use thisnodes array" );
    OSL_ENSURE( &aRg.aStart.GetNodes() == &aRg.aEnd.GetNodes(),
               "Range across different nodes arrays? You deserve punishment!");
    if( &rIndex.GetNodes() == &aRg.aStart.GetNodes() &&
        rIndex.GetIndex() >= aRg.aStart.GetIndex() &&
        rIndex.GetIndex() < aRg.aEnd.GetIndex() )
            return;

    SwNodeIndex aInsPos( rIndex );
    SwNodeIndex aOrigInsPos( rIndex, -1 );          // Originale Insert Pos
    sal_uInt16 nLevel = 0;                          // Level-Counter

    for( sal_uLong nNodeCnt = aRg.aEnd.GetIndex() - aRg.aStart.GetIndex();
            nNodeCnt > 0; --nNodeCnt )
    {
        pAktNode = &aRg.aStart.GetNode();
        switch( pAktNode->GetNodeType() )
        {
        case ND_TABLENODE:
            // dann kopiere mal den TableNode
            // Tabell in Fussnote kopieren ?
            if( aInsPos < pDoc->GetNodes().GetEndOfInserts().GetIndex() &&
                    pDoc->GetNodes().GetEndOfInserts().StartOfSectionIndex()
                    < aInsPos.GetIndex() )
            {
                sal_uLong nDistance =
                    ( pAktNode->EndOfSectionIndex() -
                        aRg.aStart.GetIndex() );
                if (nDistance < nNodeCnt)
                    nNodeCnt -= nDistance;
                else
                    nNodeCnt = 1;

                // dann alle Nodes der Tabelle in die akt. Zelle kopieren
                // fuer den TabellenNode einen DummyNode einfuegen?
                if( bTblInsDummyNode )
                    new SwDummySectionNode( aInsPos );

                for( aRg.aStart++; aRg.aStart.GetIndex() <
                    pAktNode->EndOfSectionIndex();
                    aRg.aStart++ )
                {
                    // fuer den Box-StartNode einen DummyNode einfuegen?
                    if( bTblInsDummyNode )
                        new SwDummySectionNode( aInsPos );

                    SwStartNode* pSttNd = aRg.aStart.GetNode().GetStartNode();
                    _CopyNodes( SwNodeRange( *pSttNd, + 1,
                                            *pSttNd->EndOfSectionNode() ),
                                aInsPos, bNewFrms, sal_False );

                    // fuer den Box-EndNode einen DummyNode einfuegen?
                    if( bTblInsDummyNode )
                        new SwDummySectionNode( aInsPos );
                    aRg.aStart = *pSttNd->EndOfSectionNode();
                }
                // fuer den TabellenEndNode einen DummyNode einfuegen?
                if( bTblInsDummyNode )
                    new SwDummySectionNode( aInsPos );
                aRg.aStart = *pAktNode->EndOfSectionNode();
            }
            else
            {
                SwNodeIndex nStt( aInsPos, -1 );
                SwTableNode* pTblNd = ((SwTableNode*)pAktNode)->
                                        MakeCopy( pDoc, aInsPos );
                sal_uLong nDistance = aInsPos.GetIndex() - nStt.GetIndex() - 2;
                if (nDistance < nNodeCnt)
                    nNodeCnt -= nDistance;
                else
                    nNodeCnt = 1;

                aRg.aStart = pAktNode->EndOfSectionIndex();

                if( bNewFrms && pTblNd )
                {
                    nStt = aInsPos;
                    pTblNd->MakeFrms( &nStt );
                }
            }
            break;

        case ND_SECTIONNODE:            // SectionNode
            // If the end of the section is outside the copy range,
            // the section node will skipped, not copied!
            // If someone want to change this behaviour, he has to adjust the function
            // lcl_NonCopyCount(..) in ndcopy.cxx which relies on it.
            if( pAktNode->EndOfSectionIndex() < aRg.aEnd.GetIndex() )
            {
                // also der gesamte, lege einen neuen SectionNode an
                SwNodeIndex nStt( aInsPos, -1 );
                SwSectionNode* pSectNd = ((SwSectionNode*)pAktNode)->
                                    MakeCopy( pDoc, aInsPos );

                sal_uLong nDistance = aInsPos.GetIndex() - nStt.GetIndex() - 2;
                if (nDistance < nNodeCnt)
                    nNodeCnt -= nDistance;
                else
                    nNodeCnt = 1;
                aRg.aStart = pAktNode->EndOfSectionIndex();

                if( bNewFrms && pSectNd &&
                    !pSectNd->GetSection().IsHidden() )
                    pSectNd->MakeFrms( &nStt );
            }
            break;

        case ND_STARTNODE:              // StartNode gefunden
            {
                SwStartNode* pTmp = new SwStartNode( aInsPos, ND_STARTNODE,
                            ((SwStartNode*)pAktNode)->GetStartNodeType() );
                new SwEndNode( aInsPos, *pTmp );
                aInsPos--;
                nLevel++;
            }
            break;

        case ND_ENDNODE:
            if( nLevel )                        // vollstaendige Section
            {
                --nLevel;
                ++aInsPos;                      // EndNode schon vorhanden
            }
            else if( !pAktNode->pStartOfSection->IsSectionNode() )
            {
                // erzeuge eine Section an der originalen InsertPosition
                SwNodeRange aTmpRg( aOrigInsPos, 1, aInsPos );
                pDoc->GetNodes().SectionDown( &aTmpRg,
                        pAktNode->pStartOfSection->GetStartNodeType() );
            }
            break;

        case ND_TEXTNODE:
        case ND_GRFNODE:
        case ND_OLENODE:
            {
                SwCntntNode* pNew = ((SwCntntNode*)pAktNode)->MakeCopy(
                                            pDoc, aInsPos );
                if( !bNewFrms )         // dflt. werden die Frames immer angelegt
                    pNew->DelFrms();
            }
            break;

        case ND_SECTIONDUMMY:
            if (GetDoc()->GetIDocumentUndoRedo().IsUndoNodes(*this))
            {
                // dann muss an der akt. InsPos auch ein SectionNode
                // (Start/Ende) stehen; dann diesen ueberspringen.
                // Andernfalls nicht weiter beachten.
                SwNode *const pTmpNd = & aInsPos.GetNode();
                if( pTmpNd->IsSectionNode() ||
                    pTmpNd->StartOfSectionNode()->IsSectionNode() )
                    ++aInsPos;  // ueberspringen
            }
            else {
                OSL_FAIL( "wie kommt diser Node ins Nodes-Array??" );
            }
            break;

        default:
            OSL_FAIL( "weder Start-/End-/Content-Node, unbekannter Typ" );
        }
        aRg.aStart++;
    }
}

void SwNodes::_DelDummyNodes( const SwNodeRange& rRg )
{
    SwNodeIndex aIdx( rRg.aStart );
    while( aIdx.GetIndex() < rRg.aEnd.GetIndex() )
    {
        if( ND_SECTIONDUMMY == aIdx.GetNode().GetNodeType() )
            RemoveNode( aIdx.GetIndex(), 1, sal_True );
        else
            ++aIdx;
    }
}

SwStartNode* SwNodes::MakeEmptySection( const SwNodeIndex& rIdx,
                                        SwStartNodeType eSttNdTyp )
{
    SwStartNode* pSttNd = new SwStartNode( rIdx, ND_STARTNODE, eSttNdTyp );
    new SwEndNode( rIdx, *pSttNd );
    return pSttNd;
}


SwStartNode* SwNodes::MakeTextSection( const SwNodeIndex & rWhere,
                                        SwStartNodeType eSttNdTyp,
                                        SwTxtFmtColl *pColl,
                                        SwAttrSet* pAutoAttr )
{
    SwStartNode* pSttNd = new SwStartNode( rWhere, ND_STARTNODE, eSttNdTyp );
    new SwEndNode( rWhere, *pSttNd );
    MakeTxtNode( SwNodeIndex( rWhere, - 1 ), pColl, pAutoAttr );
    return pSttNd;
}

    // zum naechsten Content-Node, der nicht geschuetzt oder versteckt ist
    // (beides auf sal_False ==> GoNext/GoPrevious!!!)
SwCntntNode* SwNodes::GoNextSection( SwNodeIndex * pIdx,
                            int bSkipHidden, int bSkipProtect ) const
{
    bool bFirst = true;
    SwNodeIndex aTmp( *pIdx );
    const SwNode* pNd;
    while( aTmp < Count() - 1 )
    {
        pNd = & aTmp.GetNode();
        if (ND_SECTIONNODE == pNd->GetNodeType())
        {
            const SwSection& rSect = ((SwSectionNode*)pNd)->GetSection();
            if( (bSkipHidden && rSect.IsHiddenFlag()) ||
                (bSkipProtect && rSect.IsProtectFlag()) )
                // dann diese Section ueberspringen
                aTmp = *pNd->EndOfSectionNode();
            bFirst = false;
        }
        else if( bFirst )
        {
            bFirst = false;
            if( pNd->pStartOfSection->IsSectionNode() )
            {
                const SwSection& rSect = ((SwSectionNode*)pNd->
                                pStartOfSection)->GetSection();
                if( (bSkipHidden && rSect.IsHiddenFlag()) ||
                    (bSkipProtect && rSect.IsProtectFlag()) )
                    // dann diese Section ueberspringen
                    aTmp = *pNd->EndOfSectionNode();
            }
        }
        else if( ND_CONTENTNODE & pNd->GetNodeType() )
        {
            const SwSectionNode* pSectNd;
            if( ( bSkipHidden || bSkipProtect ) &&
                0 != (pSectNd = pNd->FindSectionNode() ) &&
                ( ( bSkipHidden && pSectNd->GetSection().IsHiddenFlag() ) ||
                  ( bSkipProtect && pSectNd->GetSection().IsProtectFlag() )) )
            {
                aTmp = *pSectNd->EndOfSectionNode();
            }
            else
            {
                (*pIdx) = aTmp;
                return (SwCntntNode*)pNd;
            }
        }
        ++aTmp;
        bFirst = false;
    }
    return 0;
}

SwCntntNode* SwNodes::GoPrevSection( SwNodeIndex * pIdx,
                            int bSkipHidden, int bSkipProtect ) const
{
    bool bFirst = true;
    SwNodeIndex aTmp( *pIdx );
    const SwNode* pNd;
    while( aTmp > 0 )
    {
        pNd = & aTmp.GetNode();
        if (ND_ENDNODE == pNd->GetNodeType())
        {
            if( pNd->pStartOfSection->IsSectionNode() )
            {
                const SwSection& rSect = ((SwSectionNode*)pNd->
                                            pStartOfSection)->GetSection();
                if( (bSkipHidden && rSect.IsHiddenFlag()) ||
                    (bSkipProtect && rSect.IsProtectFlag()) )
                    // dann diese Section ueberspringen
                    aTmp = *pNd->StartOfSectionNode();
            }
            bFirst = false;
        }
        else if( bFirst )
        {
            bFirst = false;
            if( pNd->pStartOfSection->IsSectionNode() )
            {
                const SwSection& rSect = ((SwSectionNode*)pNd->
                                pStartOfSection)->GetSection();
                if( (bSkipHidden && rSect.IsHiddenFlag()) ||
                    (bSkipProtect && rSect.IsProtectFlag()) )
                    // dann diese Section ueberspringen
                    aTmp = *pNd->StartOfSectionNode();
            }
        }
        else if( ND_CONTENTNODE & pNd->GetNodeType() )
        {
            const SwSectionNode* pSectNd;
            if( ( bSkipHidden || bSkipProtect ) &&
                0 != (pSectNd = pNd->FindSectionNode() ) &&
                ( ( bSkipHidden && pSectNd->GetSection().IsHiddenFlag() ) ||
                  ( bSkipProtect && pSectNd->GetSection().IsProtectFlag() )) )
            {
                aTmp = *pSectNd;
            }
            else
            {
                (*pIdx) = aTmp;
                return (SwCntntNode*)pNd;
            }
        }
        aTmp--;
    }
    return 0;
}


    // suche den vorhergehenden [/nachfolgenden ] ContentNode oder
    // TabellenNode mit Frames. Wird kein Ende angeben, dann wird mit
    // dem FrameIndex begonnen; ansonsten, wird mit dem vor rFrmIdx und
    // dem hintern pEnd die Suche gestartet. Sollte kein gueltiger Node
    // gefunden werden, wird 0 returnt. rFrmIdx zeigt auf dem Node mit
    // Frames
SwNode* SwNodes::FindPrvNxtFrmNode( SwNodeIndex& rFrmIdx,
                                    const SwNode* pEnd ) const
{
    SwNode* pFrmNd = 0;

    // habe wir gar kein Layout, vergiss es
    if( GetDoc()->GetCurrentViewShell() )   //swmod 071108//swmod 071225
    {
        SwNode* pSttNd = &rFrmIdx.GetNode();

        // wird in eine versteckte Section verschoben ??
        SwSectionNode* pSectNd = pSttNd->IsSectionNode()
                    ? pSttNd->StartOfSectionNode()->FindSectionNode()
                    : pSttNd->FindSectionNode();
        if( !( pSectNd && pSectNd->GetSection().CalcHiddenFlag()/*IsHiddenFlag()*/ ) )
        {
            // in a table in table situation we have to assure that we don't leave the
            // outer table cell when the inner table is looking for a PrvNxt...
            SwTableNode* pTableNd = pSttNd->IsTableNode()
                    ? pSttNd->StartOfSectionNode()->FindTableNode()
                    : pSttNd->FindTableNode();
            SwNodeIndex aIdx( rFrmIdx );
            SwNode* pNd;
            if( pEnd )
            {
                aIdx--;
                pNd = &aIdx.GetNode();
            }
            else
                pNd = pSttNd;

            if( ( pFrmNd = pNd )->IsCntntNode() )
                rFrmIdx = aIdx;

                // suche nach vorne/hinten nach einem Content Node
            else if( 0 != ( pFrmNd = GoPrevSection( &aIdx, sal_True, sal_False )) &&
                    ::CheckNodesRange( aIdx, rFrmIdx, true ) &&
                    // nach vorne nie aus der Tabelle hinaus!
                    pFrmNd->FindTableNode() == pTableNd &&
                    // Bug 37652: nach hinten nie aus der Tabellenzelle hinaus!
                    (!pFrmNd->FindTableNode() || pFrmNd->FindTableBoxStartNode()
                        == pSttNd->FindTableBoxStartNode() ) &&
                     (!pSectNd || pSttNd->IsSectionNode() ||
                      pSectNd->GetIndex() < pFrmNd->GetIndex())
                    )
            {
                rFrmIdx = aIdx;
            }
            else
            {
                if( pEnd )
                    aIdx = pEnd->GetIndex() + 1;
                else
                    aIdx = rFrmIdx;

                // JP 19.09.93: aber nie die Section dafuer verlassen !!
                if( ( pEnd && ( pFrmNd = &aIdx.GetNode())->IsCntntNode() ) ||
                    ( 0 != ( pFrmNd = GoNextSection( &aIdx, sal_True, sal_False )) &&
                    ::CheckNodesRange( aIdx, rFrmIdx, true ) &&
                    ( pFrmNd->FindTableNode() == pTableNd &&
                        // Bug 37652: nach hinten nie aus der Tabellenzelle hinaus!
                        (!pFrmNd->FindTableNode() || pFrmNd->FindTableBoxStartNode()
                        == pSttNd->FindTableBoxStartNode() ) ) &&
                     (!pSectNd || pSttNd->IsSectionNode() ||
                      pSectNd->EndOfSectionIndex() > pFrmNd->GetIndex())
                    ))
                {
                    //JP 18.02.99: Undo von Merge einer Tabelle mit der
                    // der vorherigen, wenn dahinter auch noch eine steht
                    // falls aber der Node in einer Tabelle steht, muss
                    // natuerlich dieser returnt werden, wenn der SttNode eine
                    // Section oder Tabelle ist!
                    SwTableNode* pTblNd;
                    if( pSttNd->IsTableNode() &&
                        0 != ( pTblNd = pFrmNd->FindTableNode() ) &&
                        // TABLE IN TABLE:
                        pTblNd != pSttNd->StartOfSectionNode()->FindTableNode() )
                    {
                        pFrmNd = pTblNd;
                        rFrmIdx = *pFrmNd;
                    }
                    else
                        rFrmIdx = aIdx;
                }
                else if( pNd->IsEndNode() && pNd->StartOfSectionNode()->IsTableNode() )
                {
                    pFrmNd = pNd->StartOfSectionNode();
                    rFrmIdx = *pFrmNd;
                }
                else
                {
                    if( pEnd )
                        aIdx = pEnd->GetIndex() + 1;
                    else
                        aIdx = rFrmIdx.GetIndex() + 1;

                    if( (pFrmNd = &aIdx.GetNode())->IsTableNode() )
                        rFrmIdx = aIdx;
                    else
                    {
                        pFrmNd = 0;

                        // is there some sectionnodes before a tablenode?
                        while( aIdx.GetNode().IsSectionNode() )
                        {
                            const SwSection& rSect = aIdx.GetNode().
                                GetSectionNode()->GetSection();
                            if( rSect.IsHiddenFlag() )
                                aIdx = aIdx.GetNode().EndOfSectionIndex()+1;
                            else
                                ++aIdx;
                        }
                        if( aIdx.GetNode().IsTableNode() )
                        {
                            rFrmIdx = aIdx;
                            pFrmNd = &aIdx.GetNode();
                        }
                    }
                }
            }
        }
    }
    return pFrmNd;
}

void SwNodes::ForEach( const SwNodeIndex& rStart, const SwNodeIndex& rEnd,
                    FnForEach_SwNodes fnForEach, void* pArgs )
{
    BigPtrArray::ForEach( rStart.GetIndex(), rEnd.GetIndex(),
                            (FnForEach) fnForEach, pArgs );
}

namespace {

struct TempBigPtrEntry : public BigPtrEntry
{
    TempBigPtrEntry() {}
};

}

void SwNodes::RemoveNode( sal_uLong nDelPos, sal_uLong nSz, sal_Bool bDel )
{
    sal_uLong nEnd = nDelPos + nSz;
    SwNode* pNew = (*this)[ nEnd ];

    if( pRoot )
    {
        SwNodeIndex *p = pRoot;
        while( p )
        {
            sal_uLong nIdx = p->GetIndex();
            SwNodeIndex* pNext = p->pNext;
            if( nDelPos <= nIdx && nIdx < nEnd )
                (*p) = *pNew;

            p = pNext;
        }

        p = pRoot->pPrev;
        while( p )
        {
            sal_uLong nIdx = p->GetIndex();
            SwNodeIndex* pPrev = p->pPrev;
            if( nDelPos <= nIdx && nIdx < nEnd )
                (*p) = *pNew;

            p = pPrev;
        }
    }

    {
        for (sal_uLong nCnt = 0; nCnt < nSz; nCnt++)
        {
            SwTxtNode * pTxtNd = ((*this)[ nDelPos + nCnt ])->GetTxtNode();

            if (pTxtNd)
            {
                pTxtNd->RemoveFromList();
            }
        }
    }

    std::vector<TempBigPtrEntry> aTempEntries;
    if( bDel )
    {
        sal_uLong nCnt = nSz;
        SwNode *pDel = (*this)[ nDelPos+nCnt-1 ], *pPrev = (*this)[ nDelPos+nCnt-2 ];

// temp. Object setzen
        //JP 24.08.98: muessten eigentlich einzeln removed werden, weil
        //      das Remove auch rekursiv gerufen werden kann, z.B. bei
        //      zeichengebundenen Rahmen. Da aber dabei viel zu viel
        //      ablaueft, wird hier ein temp. Objekt eingefuegt, das
        //      dann mit dem Remove wieder entfernt wird.
        // siehe Bug 55406
        aTempEntries.resize(nCnt);

        while( nCnt-- )
        {
            delete pDel;
            pDel = pPrev;
            sal_uLong nPrevNdIdx = pPrev->GetIndex();
            BigPtrEntry* pTempEntry = &aTempEntries[nCnt];
            BigPtrArray::Replace( nPrevNdIdx+1, pTempEntry );
            if( nCnt )
                pPrev = (*this)[ nPrevNdIdx  - 1 ];
        }
        nDelPos = pDel->GetIndex() + 1;
    }

    BigPtrArray::Remove( nDelPos, nSz );
}

void SwNodes::RegisterIndex( SwNodeIndex& rIdx )
{
    if( !pRoot )        // noch keine Root gesetzt?
    {
        pRoot = &rIdx;
        pRoot->pPrev = 0;
        pRoot->pNext = 0;
    }
    else
    {
        // immer hinter die Root haengen
        rIdx.pNext = pRoot->pNext;
        pRoot->pNext = &rIdx;
        rIdx.pPrev = pRoot;
        if( rIdx.pNext )
            rIdx.pNext->pPrev = &rIdx;
    }
}

void SwNodes::DeRegisterIndex( SwNodeIndex& rIdx )
{
    SwNodeIndex* pN = rIdx.pNext;
    SwNodeIndex* pP = rIdx.pPrev;

    if( pRoot == &rIdx )
        pRoot = pP ? pP : pN;

    if( pP )
        pP->pNext = pN;
    if( pN )
        pN->pPrev = pP;

    rIdx.pNext = 0;
    rIdx.pPrev = 0;
}

void SwNodes::InsertNode( const SwNodePtr pNode,
                          const SwNodeIndex& rPos )
{
    const ElementPtr pIns = pNode;
    BigPtrArray::Insert( pIns, rPos.GetIndex() );
}

void SwNodes::InsertNode( const SwNodePtr pNode,
                          sal_uLong nPos )
{
    const ElementPtr pIns = pNode;
    BigPtrArray::Insert( pIns, nPos );
}

// ->#112139#
SwNode * SwNodes::DocumentSectionStartNode(SwNode * pNode) const
{
    if (NULL != pNode)
    {
        SwNodeIndex aIdx(*pNode);

        if (aIdx <= (*this)[0]->EndOfSectionIndex())
            pNode = (*this)[0];
        else
        {
            while ((*this)[0] != pNode->StartOfSectionNode())
                pNode = pNode->StartOfSectionNode();
        }
    }

    return pNode;
}

SwNode * SwNodes::DocumentSectionEndNode(SwNode * pNode) const
{
    return DocumentSectionStartNode(pNode)->EndOfSectionNode();
}

sal_Bool SwNodes::IsDocNodes() const
{
    return this == &pMyDoc->GetNodes();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
