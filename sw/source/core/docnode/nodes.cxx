/*************************************************************************
 *
 *  $RCSfile: nodes.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:18 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <stdlib.h>

#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _NDNOTXT_HXX
#include <ndnotxt.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>      // fuer erzuegen / loeschen der Table-Frames
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _DDEFLD_HXX
#include <ddefld.hxx>
#endif
#ifndef _SWDDETBL_HXX
#include <swddetbl.hxx>
#endif
#ifndef _FRAME_HXX
#include <frame.hxx>
#endif

extern FASTBOOL CheckNodesRange( const SwNodeIndex& rStt,
                            const SwNodeIndex& rEnd, FASTBOOL bChkSection );

SV_DECL_PTRARR(SwSttNdPtrs,SwStartNode*,2,2);


//#define JP_DEBUG
#ifdef JP_DEBUG
#include "shellio.hxx"
#endif


// Funktion zum bestimmen des hoechsten Levels innerhalb des Bereiches

USHORT HighestLevel( SwNodes & rNodes, const SwNodeRange & rRange );

//-----------------------------------------------------------------------

/*******************************************************************
|*  SwNodes::SwNodes
|*
|*  Beschreibung
|*      Konstruktor; legt die vier Grundsektions (PostIts,
|*      Inserts, Icons, Inhalt) an
*******************************************************************/
SwNodes::SwNodes( SwDoc* pDocument )
    : pMyDoc( pDocument ), pRoot( 0 )
{
    bInNodesDel = bInDelUpdOutl = bInDelUpdNum = FALSE;

    ASSERT( pMyDoc, "in welchem Doc stehe ich denn?" );

    ULONG nPos = 0;
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
|*  Ersterstellung
|*      VER0100 vb 901214
|*
|*  Stand
|*      VER0100 vb 901214
|*
*******************************************************************/

SwNodes::~SwNodes()
{
    delete pOutlineNds;

    {
        SwNode *pNode;
        SwNodeIndex aNdIdx( *this );
        while( TRUE )
        {
            pNode = &aNdIdx.GetNode();
            if( pNode == pEndOfContent )
                break;

            aNdIdx++;
            delete pNode;
        }
    }

    // jetzt muessen alle SwNodeIndizies abgemeldet sein!!!
    delete pEndOfContent;
}


// Sortier-Funktion fuer das Resort der OutlineNodes-Indizies. Wenn innerhalb
// des Nodes-Arrays Elemente verschoben werden, dann muessen die Indizies
// im Outline-Array wieder in die richtige Reihenfolge sortiert werden.

int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC )
 _Optlink
#endif
    lcl_nodes_CmpFuncIdx( const void* pLower, const void* pUpper )
{
    int nRet;
    if( *(SwNode**)pLower == *(SwNode**)pUpper )
        nRet = 0;
    else if( (*(SwNode**)pLower)->GetIndex() < (*(SwNode**)pUpper)->GetIndex() )
        nRet = -1;
    else
        nRet = 1;
    return nRet;
}


void SwNodes::ChgNode( SwNodeIndex& rDelPos, ULONG nSize,
                        SwNodeIndex& rInsPos, BOOL bNewFrms )
{
    // im UndoBereich brauchen wir keine Frames
    SwNodes& rNds = rInsPos.GetNodes();
    const SwNode* pPrevInsNd = rNds[ rInsPos.GetIndex() -1 ];

    //JP 03.02.99: alle Felder als invalide erklaeren, aktu. erfolgt im
    //              Idle-Handler des Docs
    if( GetDoc()->SetFieldsDirty( TRUE, &rDelPos.GetNode(), nSize ) &&
        rNds.GetDoc() != GetDoc() )
        rNds.GetDoc()->SetFieldsDirty( TRUE );

    //JP 12.03.99: 63293 - Nodes vom RedlineBereich NIE aufnehmen
    ULONG nNd = rInsPos.GetIndex();
    BOOL bInsOutlineIdx = !(
            rNds.GetEndOfRedlines().FindStartNode()->GetIndex() < nNd &&
            nNd < rNds.GetEndOfRedlines().GetIndex() );

    if( &rNds == this )         // im gleichen Nodes-Array -> moven !!
    {
        // wird von vorne nach hinten gemovt, so wird nach vorne immer
        // nachgeschoben, d.H. die Loeschposition ist immer gleich
        USHORT nDiff = rDelPos.GetIndex() < rInsPos.GetIndex() ? 0 : 1;
        int bOutlineNds = FALSE;

        for( ULONG n = rDelPos.GetIndex(); nSize; n += nDiff, --nSize )
        {
            SwNodeIndex aDelIdx( *this, n );
            SwNode& rNd = aDelIdx.GetNode();
            if( rNd.IsTxtNode() && NO_NUMBERING !=
                ((SwTxtNode&)rNd).GetTxtColl()->GetOutlineLevel() )
            {
                const SwNodePtr pSrch = (SwNodePtr)&rNd;
                pOutlineNds->Remove( pSrch );
            }

            BigPtrArray::Move( aDelIdx.GetIndex(), rInsPos.GetIndex() );

            if( rNd.IsTxtNode() )
            {
                SwTxtNode& rTxtNd = (SwTxtNode&)rNd;
                if( bInsOutlineIdx && NO_NUMBERING !=
                    rTxtNd.GetTxtColl()->GetOutlineLevel() )
                {
                    const SwNodePtr pSrch = (SwNodePtr)&rNd;
                    pOutlineNds->Insert( pSrch );
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
        int bSavePersData = GetDoc()->GetUndoNds() == &rNds;
        int bRestPersData = GetDoc()->GetUndoNds() == this;
        SwDoc* pDestDoc = rNds.GetDoc() != GetDoc() ? rNds.GetDoc() : 0;
        if( !bRestPersData && !bSavePersData && pDestDoc )
            bSavePersData = bRestPersData = TRUE;

        String sNumRule;
        SwNodeIndex aInsPos( rInsPos );
        for( ULONG n = 0; n < nSize; n++ )
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
                // loesche die Gliederungs-Indizies aus dem alten Nodes-Array
                if( NO_NUMBERING != pTxtNd->GetTxtColl()->GetOutlineLevel() )
                    pOutlineNds->Remove( pNd );

                // muss die Rule kopiere werden?
                if( pDestDoc )
                {
                    const SwNumRule* pNumRule = pTxtNd->GetNumRule();
                    if( pNumRule && sNumRule != pNumRule->GetName() )
                    {
                        sNumRule = pNumRule->GetName();
                        SwNumRule* pDestRule = pDestDoc->FindNumRulePtr( sNumRule );
                        if( pDestRule )
                            pDestRule->SetInvalidRule( TRUE );
                        else
                            pDestDoc->MakeNumRule( sNumRule, pNumRule );
                    }
                }
                else
                    // wenns ins UndoNodes-Array gemoved wird, sollten die
                    // Numerierungen auch aktualisiert werden.
                    pTxtNd->InvalidateNumRule();
            }

            RemoveNode( rDelPos.GetIndex(), 1, FALSE );     // Indizies verschieben !!
            SwCntntNode * pCNd = pNd->GetCntntNode();
            rNds.Insert( pNd, aInsPos );

            if( pCNd )
            {
                SwTxtNode* pTxtNd = pCNd->GetTxtNode();
                if( pTxtNd )
                {
                    const SwpHints* pHts = pTxtNd->GetpSwpHints();
                    // setze die OultineNodes im neuen Nodes-Array
                    if( bInsOutlineIdx && NO_NUMBERING !=
                        pTxtNd->GetTxtColl()->GetOutlineLevel() )
                        rNds.pOutlineNds->Insert( pTxtNd );

                    // Sonderbehandlung fuer die Felder!
                    if( pHts && pHts->Count() )
                    {
                        SwTxtFld* pTxtFld;
                        int bToUndo = &pDestDoc->GetNodes() != &rNds;
                        for( USHORT i = pHts->Count(); i; )
                            if( RES_TXTATR_FIELD == ( pTxtFld =
                                    (SwTxtFld*)(*pHts)[ --i ])->Which() )
                            {
                                rNds.GetDoc()->InsDelFldInFldLst( !bToUndo,
                                                            *pTxtFld );
                                const SwFieldType* pTyp = pTxtFld->GetFld().
                                                        GetFld()->GetTyp();
                                if( RES_DDEFLD == pTyp->Which() )
                                {
                                    if( bToUndo )
                                        ((SwDDEFieldType*)pTyp)->DecRefCnt();
                                    else
                                        ((SwDDEFieldType*)pTyp)->IncRefCnt();
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
    GetDoc()->SetFieldsDirty( TRUE );
    if( rNds.GetDoc() != GetDoc() )
        rNds.GetDoc()->SetFieldsDirty( TRUE );


    if( bNewFrms )
        bNewFrms = &GetDoc()->GetNodes() == (const SwNodes*)&rNds &&
                    GetDoc()->GetRootFrm();
    if( bNewFrms )
    {
        // Frames besorgen:
        SwNodeIndex aIdx( *pPrevInsNd, 1 );
        SwNodeIndex aFrmNdIdx( aIdx );
        SwNode* pFrmNd = rNds.FindPrvNxtFrmNode( aFrmNdIdx,
                                        rNds[ rInsPos.GetIndex() - 1 ] );

        if( !pFrmNd && aFrmNdIdx > rNds.GetEndOfExtras().GetIndex() )
        {
            ASSERT( !this, "ob das so richtig ist ??" );
            aFrmNdIdx = rNds.GetEndOfContent();
            pFrmNd = rNds.GoPrevSection( &aFrmNdIdx, TRUE, FALSE );
            if( pFrmNd && !((SwCntntNode*)pFrmNd)->GetDepends() )
                pFrmNd = 0;

#ifndef PRODUCT
            if( !pFrmNd )
                ASSERT( !this, "ChgNode() - kein FrameNode gefunden" );
#endif
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
                aIdx++;
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
|*
|*
***********************************************************************/

BOOL SwNodes::_MoveNodes( const SwNodeRange& aRange, SwNodes & rNodes,
                    const SwNodeIndex& aIndex, BOOL bNewFrms )
{
    SwNode * pAktNode;
    if( aIndex == 0 ||
        ( (pAktNode = &aIndex.GetNode())->GetStartNode() &&
          !pAktNode->StartOfSectionIndex() ))
        return FALSE;

    SwNodeRange aRg( aRange );

    // "einfache" StartNodes oder EndNodes ueberspringen
    while( ND_STARTNODE == (pAktNode = &aRg.aStart.GetNode())->GetNodeType()
            || ( pAktNode->IsEndNode() &&
                !pAktNode->pStartOfSection->IsSectionNode() ) )
        aRg.aStart++;
    aRg.aStart--;

    // falls aEnd-1 auf keinem ContentNode steht, dann suche den vorherigen
    aRg.aEnd--;
    while( (( pAktNode = &aRg.aEnd.GetNode())->GetStartNode() &&
            !pAktNode->IsSectionNode() ) ||
            ( pAktNode->IsEndNode() &&
            ND_STARTNODE == pAktNode->pStartOfSection->GetNodeType()) )
        aRg.aEnd--;


    // wird im selben Array's verschoben, dann ueberpruefe die Einfuegepos.
    if( aRg.aStart >= aRg.aEnd )
        return FALSE;

    if( this == &rNodes )
    {
        if( ( aIndex.GetIndex()-1 >= aRg.aStart.GetIndex() &&
              aIndex.GetIndex()-1 < aRg.aEnd.GetIndex()) ||
            ( aIndex.GetIndex()-1 == aRg.aEnd.GetIndex() ) )
            return FALSE;
    }

    USHORT nLevel = 0;                  // Level-Counter
    ULONG nInsPos = 0;                  // Cnt fuer das TmpArray

    // das Array bildet einen Stack, es werden alle StartOfSelction's gesichert
    SwSttNdPtrs aSttNdStack( 1, 5 );

    // setze den Start-Index
    SwNodeIndex  aIdx( aIndex );
/*
    --- JP 17.11.94: sollte ueberholt sein, wird im ChgNode schon erledigt!
    BOOL bCorrNum = pSect && pSect->aStart.GetIndex() == aIdx.GetIndex();
*/

    SwStartNode* pStartNode = aIdx.GetNode().pStartOfSection;
    aSttNdStack.C40_INSERT( SwStartNode, pStartNode, 0 );
//  aSttNdStack.Insert( rNodes[ aIdx ]->pStartOfSection, 0 );
    SwNodeRange aOrigInsPos( aIdx, -1, aIdx );      // Originale Insert Pos

    //JP 16.01.98: SectionNodes: DelFrms/MakeFrms beim obersten SectionNode!
    USHORT nSectNdCnt = 0;
    BOOL bSaveNewFrms = bNewFrms;

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
                    ULONG nNd = aIdx.GetIndex();
                    BOOL bInsOutlineIdx = !( rNodes.GetEndOfRedlines().
                            FindStartNode()->GetIndex() < nNd &&
                            nNd < rNodes.GetEndOfRedlines().GetIndex() );

                    if( bNewFrms )
                        // loesche erstmal die Frames
                        pTblNd->DelFrms();
                    if( &rNodes == this )   // in sich selbst moven ??
                    {
                        // dann bewege alle Start/End/ContentNodes. Loesche
                        // bei den ContentNodes auch die Frames !!
                        pTblNd->pStartOfSection = aIdx.GetNode().pStartOfSection;
                        for( USHORT n = 0; n < nInsPos; ++n )
                        {
                            SwNodeIndex aMvIdx( aRg.aEnd, 1 );
                            SwCntntNode* pCNd = 0;
                            SwNode* pTmpNd = &aMvIdx.GetNode();
                            if( pTmpNd->IsCntntNode() )
                            {
                                pCNd = (SwCntntNode*)pTmpNd;
//                              if( bNewFrms )
//                                  pCNd->DelFrms();

                                // setze bei Start/EndNodes die richtigen Indizies
                                // loesche die Gliederungs-Indizies aus
                                // dem alten Nodes-Array
                                if( pCNd->IsTxtNode() && NO_NUMBERING !=
                                    ((SwTxtNode*)pCNd)->GetTxtColl()->GetOutlineLevel() )
                                    pOutlineNds->Remove( pCNd );
                                else
                                    pCNd = 0;
                            }
//                          else if( bNewFrms && pTmpNd->IsSectionNode() )
//                              ((SwSectionNode*)pTmpNd)->DelFrms();
                            BigPtrArray::Move( aMvIdx.GetIndex(), aIdx.GetIndex() );

                            if( bInsOutlineIdx && pCNd )
                                pOutlineNds->Insert( pCNd );
                        }
                    }
                    else
                    {
                        // StartNode holen
                        SwStartNode* pSttNode = aIdx.GetNode().GetStartNode();
                        if( !pSttNode )
                            pSttNode = aIdx.GetNode().pStartOfSection;

                        // Hole alle Boxen mit Inhalt. Deren Indizies auf die
                        // StartNodes muessen umgemeldet werden !!
                        // (Array kopieren und alle gefunden wieder loeschen;
                        //  erleichtert das suchen!!)
                        SwNodeIndex aMvIdx( aRg.aEnd, 1 );
                        for( USHORT n = 0; n < nInsPos; ++n )
                        {
                            SwNode* pNd = &aMvIdx.GetNode();
/*                          if( bNewFrms )
                            {
                                if( pNd->IsCntntNode() )
                                    ((SwCntntNode*)pNd)->DelFrms();
                                else if( pNd->IsSectionNode() )
                                    ((SwSectionNode*)pNd)->DelFrms();
                            }
*/
                            BOOL bOutlNd = pNd->IsTxtNode() && NO_NUMBERING !=
                                ((SwTxtNode*)pNd)->GetTxtColl()->GetOutlineLevel();
                            // loesche die Gliederungs-Indizies aus
                            // dem alten Nodes-Array
                            if( bOutlNd )
                                pOutlineNds->Remove( pNd );

                            RemoveNode( aMvIdx.GetIndex(), 1, FALSE );
                            pNd->pStartOfSection = pSttNode;
                            rNodes.Insert( pNd, aIdx );

                            // setze bei Start/EndNodes die richtigen Indizies
                            if( bInsOutlineIdx && bOutlNd )
                                // und setze sie im neuen Nodes-Array
                                rNodes.pOutlineNds->Insert( pNd );
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

                        if( GetDoc()->GetUndoNds() == &rNodes )
                        {
                            SwFrmFmt* pTblFmt = pTblNd->GetTable().GetFrmFmt();
                            SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT,
                                                        pTblFmt );
                            pTblFmt->Modify( &aMsgHint, &aMsgHint );
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
                            aSttNdStack.C40_INSERT( SwStartNode, pTmp, nLevel );

                            // noch den EndNode erzeugen
                            new SwEndNode( aIdx, *pTmp );
                        }
                        else if( (const SwNodes*)&rNodes ==
                                GetDoc()->GetUndoNds() )
                        {
                            // im UndoNodes-Array spendieren wir einen
                            // Platzhalter
                            new SwNode( aIdx, ND_SECTIONDUMMY );
                        }
                        else
                            break;

                        aRg.aEnd--;
                        aIdx--;
                    } while( FALSE );
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

                    RemoveNode( aRg.aEnd.GetIndex(), 1, FALSE ); // EndNode loeschen
                    ULONG nSttPos = pSttNd->GetIndex();

                    // dieser StartNode wird spaeter wieder entfernt!
                    SwStartNode* pTmpSttNd = new SwStartNode( *this, nSttPos+1 );
                    pTmpSttNd->pStartOfSection = pSttNd->pStartOfSection;

                    RemoveNode( nSttPos, 1, FALSE ); // SttNode loeschen

                    pSttNd->pStartOfSection = aIdx.GetNode().pStartOfSection;
                    rNodes.Insert( pSttNd, aIdx  );
                    rNodes.Insert( pAktNode, aIdx );
                    aIdx--;
                    pSttNd->pEndOfSection = (SwEndNode*)pAktNode;

                    aRg.aEnd--;

                    nLevel++;           // den Index auf StartNode auf den Stack
                    aSttNdStack.C40_INSERT( SwStartNode, pSttNd, nLevel );

                    // SectionNode muss noch ein paar Indizies ummelden
                    if( pSctNd )
                    {
                        pSctNd->NodesArrChgd();
                        ++nSectNdCnt;
                        bNewFrms = FALSE;
                    }
                }
            }
            break;



        case ND_SECTIONNODE:
            if( !nLevel &&
                ( (const SwNodes*)&rNodes == GetDoc()->GetUndoNds() ) )
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
                new SwNode( aIdx, ND_SECTIONDUMMY );
                aRg.aEnd--;
                aIdx--;
                break;
            }
            // kein break !!
        case ND_TABLENODE:
        case ND_STARTNODE:
            {
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
                    aTmpSIdx++;

                    // setze die StartOfSection richtig
                    aRg.aEnd++;
                    {
                        SwNodeIndex aCntIdx( aRg.aEnd );
                        for( register USHORT n = 0; n < nInsPos; n++, aCntIdx++)
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
                    ASSERT( pAktNode == aSttNdStack[nLevel] ||
                            ( pAktNode->IsStartNode() &&
                                aSttNdStack[nLevel]->IsSectionNode()),
                             "falscher StartNode" );

                    SwNodeIndex aSwIndex( aRg.aEnd, 1 );
                    ChgNode( aSwIndex, nInsPos, aIdx, bNewFrms );
                    aIdx -= nInsPos+1;      // vor den eingefuegten StartNode
                    nInsPos = 0;

                    // loesche nur noch den Pointer aus dem Nodes-Array.
//                  RemoveNode( aRg.aEnd.GetIndex(), 1, FALSE );
                    RemoveNode( aRg.aEnd.GetIndex(), 1, TRUE );
                    aRg.aEnd--;

                    SwSectionNode* pSectNd = aSttNdStack[ nLevel ]->GetSectionNode();
                    if( pSectNd && !--nSectNdCnt )
                    {
                        SwNodeIndex aTmp( *pSectNd );
                        pSectNd->MakeFrms( &aTmp );
                        bNewFrms = bSaveNewFrms;
                    }
                    aSttNdStack.Remove( nLevel );   // vom Stack loeschen
                    nLevel--;
                }

                // loesche alle entstehenden leeren Start-/End-Node-Paare
                SwNode* pTmpNode = (*this)[ aRg.aEnd.GetIndex()+1 ]->GetEndNode();
                if( pTmpNode && ND_STARTNODE == (pAktNode = &aRg.aEnd.GetNode())
                    ->GetNodeType() && pAktNode->StartOfSectionIndex() &&
                    pTmpNode->FindStartNode() == pAktNode )
                {
                    DelNodes( aRg.aEnd, 2 );
                    aRg.aEnd--;
                }
//              aRg.aEnd--;
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
            if( (const SwNodes*)this == GetDoc()->GetUndoNds() )
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
                        pTmpNd->FindStartNode()->IsSectionNode() )
                        aIdx--; // ueberspringen
                }
            }
            else
                ASSERT( FALSE, "wie kommt diser Node ins Nodes-Array??" );
            aRg.aEnd--;
            break;

        default:
            ASSERT( FALSE, "was ist das fuer ein Node??" );
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

#ifdef JP_DEBUG
    {
extern Writer* GetDebugWriter(const String&);

        Writer* pWriter = GetDebugWriter(aEmptyStr);
        if( pWriter )
        {
            int nError;
            SvFileStream aStrm( "c:\\$$move.db", STREAM_WRITE );
            SwWriter aWriter( aStrm, *pMyDoc );
            aWriter.Write( &nError, pWriter );
        }
    }
#endif

    return TRUE;
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
|*  Ersterstellung
|*      VER0100 vb 901214
|*
|*  Stand
|*      VER0100 vb 901214
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
|*  Ersterstellung
|*      VER0100 vb 901214
|*
|*  Stand
|*      VER0100 vb 901214
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
            RemoveNode( pRange->aStart.GetIndex(), 1, TRUE );
            RemoveNode( pRange->aEnd.GetIndex(), 1, TRUE );

            SwNodeIndex aTmpIdx( pRange->aStart );
            while( aTmpIdx < pRange->aEnd )
            {
                pAktNode = &aTmpIdx.GetNode();
                pAktNode->pStartOfSection = pTmpSttNd;
                if( pAktNode->IsStartNode() )
                    aTmpIdx = pAktNode->EndOfSectionIndex() + 1;
                else
                    aTmpIdx++;
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
|*    Ersterstellung    JP 23.04.91
|*    Letzte Aenderung  JP 23.04.91
|*
*************************************************************************/
void SwNodes::SectionUpDown( const SwNodeIndex & aStart, const SwNodeIndex & aEnd )
{
    SwNode * pAktNode;
    SwNodeIndex aTmpIdx( aStart, +1 );
    // das Array bildet einen Stack, es werden alle StartOfSelction's gesichert
    SwSttNdPtrs aSttNdStack( 1, 5 );
    SwStartNode* pTmp = aStart.GetNode().GetStartNode();
    aSttNdStack.C40_INSERT( SwStartNode, pTmp, 0 );

    // durchlaufe bis der erste zu aendernde Start-Node gefunden wurde
    // ( Es wird vom eingefuegten EndNode bis nach vorne die Indexe gesetzt )
    for( ;; aTmpIdx++ )
    {
        pAktNode = &aTmpIdx.GetNode();
        pAktNode->pStartOfSection = aSttNdStack[ aSttNdStack.Count()-1 ];

        if( pAktNode->GetStartNode() )
        {
            pTmp = (SwStartNode*)pAktNode;
            aSttNdStack.C40_INSERT( SwStartNode, pTmp, aSttNdStack.Count() );
        }
        else if( pAktNode->GetEndNode() )
        {
            SwStartNode* pSttNd = aSttNdStack[ aSttNdStack.Count() - 1 ];
            pSttNd->pEndOfSection = (SwEndNode*)pAktNode;
            aSttNdStack.Remove( aSttNdStack.Count() - 1 );
            if( aSttNdStack.Count() )
                continue;       // noch genuegend EndNodes auf dem Stack

            else if( aTmpIdx < aEnd )   // Uebergewicht an StartNodes
                // ist das Ende noch nicht erreicht, so hole den Start von
                // der uebergeordneten Section
            {
                aSttNdStack.C40_INSERT( SwStartNode, pSttNd->pStartOfSection, 0 );
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
|*      USHORT nNodes bezeichnet die Anzahl der zu loeschenden
|*      Nodes; ist auf 1 defaulted
|*
|*  Debug-Funktionen
|*      geben beim Eintritt in die Funktion Position und Anzahl
|*      der zu loeschenden Nodes aus.
|*
|*  Ersterstellung
|*      VER0100 vb 901214
|*
|*  Stand
|*      VER0100 vb 901214
|*
*******************************************************************/
void SwNodes::Delete(const SwNodeIndex &rIndex, ULONG nNodes)
{
    USHORT nLevel = 0;                      // Level-Counter
    SwNode * pAktNode;

    ULONG nCnt = Count() - rIndex.GetIndex() - 1;
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

    BOOL bSaveInNodesDel = bInNodesDel;
    bInNodesDel = TRUE;
    BOOL bUpdateOutline = FALSE;

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
                USHORT nIdxPos;
                do {
                    pNd = &aRg.aEnd.GetNode();

                    if( pNd->IsTxtNode() )
                    {
                        if( NO_NUMBERING !=
                        ((SwTxtNode*)pNd)->GetTxtColl()->GetOutlineLevel() &&
                        pOutlineNds->Seek_Entry( pNd, &nIdxPos ))
                        {
                            // loesche die Gliederungs-Indizies.
                            pOutlineNds->Remove( nIdxPos );
                            bUpdateOutline = TRUE;
                        }
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
                RemoveNode( aRg.aEnd.GetIndex()+1, nCnt, TRUE );    // loesche
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
                    RemoveNode( aRg.aEnd.GetIndex(), nCnt, TRUE );
                    nCnt = 0;
                }
            }
            else    // es werden alle Nodes Innerhalb eines Start- und
            {       // End-Nodes geloescht, loesche mit Start/EndNode
                RemoveNode( aRg.aEnd.GetIndex(), nCnt + 2, TRUE );          // loesche Array
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
                if( NO_NUMBERING != pTxtNd->GetTxtColl()->GetOutlineLevel() )
                {                   // loesche die Gliederungs-Indizies.
                    pOutlineNds->Remove( pTxtNd );
                    bUpdateOutline = TRUE;
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
        RemoveNode( aRg.aEnd.GetIndex(), nCnt, TRUE );              // loesche den Rest

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
            bInDelUpdOutl = FALSE;
        }

    }
    else
    {
        if( bUpdateOutline )
            bInDelUpdOutl = TRUE;
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
|*  Ersterstellung
|*      VER0100 vb 901214
|*
|*  Stand
|*      VER0100 vb 901214
|*
*******************************************************************/
USHORT SwNodes::GetSectionLevel(const SwNodeIndex &rIdx) const {
    // Sonderbehandlung 1. Node
    if(rIdx == 0) return 1;
    /*
     * Keine Rekursion! - hier wird das SwNode::GetSectionLevel
     * aufgerufen
     */
    return (*this)[rIdx]->GetSectionLevel();

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
    SwNode* pNd;
    while( aTmp < Count()-1 && 0 == ( pNd = &aTmp.GetNode())->IsCntntNode() )
        aTmp++;

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
    SwNode* pNd;
    while( aTmp.GetIndex() && 0 == ( pNd = &aTmp.GetNode())->IsCntntNode() )
        aTmp--;

    if( !aTmp.GetIndex() )
        pNd = 0;
    else
        (*pIdx) = aTmp;
    return (SwCntntNode*)pNd;
}

SwNode* SwNodes::GoNextWithFrm(SwNodeIndex *pIdx) const
{
    if( pIdx->GetIndex() >= Count() - 1 )
        return 0;

    SwNodeIndex aTmp(*pIdx, +1);
    SwNode* pNd;
    while( aTmp < Count()-1 )
    {
        pNd = &aTmp.GetNode();
        SwModify *pMod = 0;
        if ( pNd->IsCntntNode() )
            pMod = (SwCntntNode*)pNd;
        else if ( pNd->IsTableNode() )
            pMod = ((SwTableNode*)pNd)->GetTable().GetFrmFmt();
        else if( pNd->IsEndNode() && !pNd->FindStartNode()->IsSectionNode() )
        {
            pNd = 0;
            break;
        }
        if ( pMod && pMod->GetDepends() )
        {
            SwClientIter aIter( *pMod );
            if( aIter.First( TYPE(SwFrm) ) )
                break;
        }
        aTmp++;
    }
    if( aTmp == Count()-1 )
        pNd = 0;
    else if( pNd )
        (*pIdx) = aTmp;
    return pNd;
}

SwNode* SwNodes::GoPreviousWithFrm(SwNodeIndex *pIdx) const
{
    if( !pIdx->GetIndex() )
        return 0;

    SwNodeIndex aTmp( *pIdx, -1 );
    SwNode* pNd;
    while( aTmp.GetIndex() )
    {
        pNd = &aTmp.GetNode();
        SwModify *pMod = 0;
        if ( pNd->IsCntntNode() )
            pMod = (SwCntntNode*)pNd;
        else if ( pNd->IsTableNode() )
            pMod = ((SwTableNode*)pNd)->GetTable().GetFrmFmt();
        else if( pNd->IsStartNode() && !pNd->IsSectionNode() )
        {
            pNd = 0;
            break;
        }
        if ( pMod && pMod->GetDepends() )
        {
            SwClientIter aIter( *pMod );
            if( aIter.First( TYPE(SwFrm) ) )
                break;
        }
        aTmp--;
    }

    if( !aTmp.GetIndex() )
        pNd = 0;
    else if( pNd )
        (*pIdx) = aTmp;
    return pNd;
}



/*************************************************************************
|*
|*    BOOL SwNodes::CheckNodesRange()
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
|*                      BOOL        TRUE:   Start+End in gleicher Section!
|*                                  FALSE:  Start+End in verschiedenen Sect.
|*    Return-Wert       BOOL        TRUE:   gueltiger SRange
|*                                  FALSE:  ungueltiger SRange
|*
|*    Ersterstellung    JP 23.04.91
|*    Letzte Aenderung  JP 18.06.92
|*
*************************************************************************/

inline int TstIdx( ULONG nSttIdx, ULONG nEndIdx, ULONG nStt, ULONG nEnd )
{
    return nStt < nSttIdx && nEnd >= nSttIdx &&
            nStt < nEndIdx && nEnd >= nEndIdx;
}

BOOL SwNodes::CheckNodesRange( const SwNodeIndex& rStt, const SwNodeIndex& rEnd ) const
{
    ULONG nStt = rStt.GetIndex(), nEnd = rEnd.GetIndex();
    if( TstIdx( nStt, nEnd, pEndOfContent->StartOfSectionIndex(),
                pEndOfContent->GetIndex() )) return TRUE;
    if( TstIdx( nStt, nEnd, pEndOfAutotext->StartOfSectionIndex(),
                pEndOfAutotext->GetIndex() )) return TRUE;
    if( TstIdx( nStt, nEnd, pEndOfPostIts->StartOfSectionIndex(),
                pEndOfPostIts->GetIndex() )) return TRUE;
    if( TstIdx( nStt, nEnd, pEndOfInserts->StartOfSectionIndex(),
                pEndOfInserts->GetIndex() )) return TRUE;
    if( TstIdx( nStt, nEnd, pEndOfRedlines->StartOfSectionIndex(),
                pEndOfRedlines->GetIndex() )) return TRUE;

    return FALSE;       // liegt irgendwo dazwischen, FEHLER
}


/*************************************************************************
|*
|*    void SwNodes::DelNodes()
|*
|*    Beschreibung
|*      Loesche aus den NodesArray ab einer Position entsprechend Node's.
|*
|*    Parameter         SwIndex &   Der Startpunkt im Nodes-Array
|*                      USHORT      die Anzahl
|*
|*    Ersterstellung    JP 23.04.91
|*    Letzte Aenderung  JP 23.04.91
|*
*************************************************************************/
void SwNodes::DelNodes( const SwNodeIndex & rStart, ULONG nCnt )
{
    int bUpdateNum = 0;
    ULONG nSttIdx = rStart.GetIndex();

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
            ULONG nEndIdx = (*ppEndNdArr)->GetIndex();

            if( nSttIdx != nEndIdx )
                RemoveNode( nSttIdx, nEndIdx - nSttIdx, TRUE );

            ++ppEndNdArr;
        }
    }
    else
    {
        for( ULONG n = nSttIdx, nEnd = nSttIdx + nCnt; n < nEnd; ++n )
        {
            SwNode* pNd = (*this)[ n ];

            if( pNd->IsTxtNode() &&
                NO_NUMBERING != ((SwTxtNode*)pNd)->GetTxtColl()->GetOutlineLevel() )
            {                   // loesche die Gliederungs-Indizies.
                USHORT nIdxPos;
                if( pOutlineNds->Seek_Entry( pNd, &nIdxPos ))
                {
                    pOutlineNds->Remove( nIdxPos );
                    bUpdateNum = 1;
                }
            }
            if( pNd->IsCntntNode() )
                ((SwCntntNode*)pNd)->InvalidateNumRule();
        }
        RemoveNode( nSttIdx, nCnt, TRUE );

        // rufe noch das Update fuer die Gliederungsnumerierung auf
        if( bUpdateNum )
            UpdtOutlineIdx( rStart.GetNode() );
    }
}


/*************************************************************************
|*
|*    USHORT HighestLevel( SwNodes & rNodes, const SwNodeRange & rRange )
|*
|*    Beschreibung
|*      Berechne den hoehsten Level innerhalb des Bereiches
|*
|*    Parameter         SwNodes &   das Node-Array
|*                      SwNodeRange &   der zu ueberpruefende Bereich
|*    Return            USHORT      der hoechste Level
|*
|*    Ersterstellung    JP 24.04.91
|*    Letzte Aenderung  JP 24.04.91
|*
*************************************************************************/

struct HighLevel
{
    USHORT nLevel, nTop;
    HighLevel( USHORT nLv ) : nLevel( nLv ), nTop( nLv ) {}

};

BOOL _HighestLevel( const SwNodePtr& rpNode, void * pPara )
{
    HighLevel * pHL = (HighLevel*)pPara;
    if( rpNode->GetStartNode() )
        pHL->nLevel++;
    else if( rpNode->GetEndNode() )
        pHL->nLevel--;
    if( pHL->nTop > pHL->nLevel )
        pHL->nTop = pHL->nLevel;
    return TRUE;

}

USHORT HighestLevel( SwNodes & rNodes, const SwNodeRange & rRange )
{
    HighLevel aPara( rNodes.GetSectionLevel( rRange.aStart ));
    rNodes.ForEach( rRange.aStart, rRange.aEnd, _HighestLevel, &aPara );
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
|*    Ersterstellung    JP 09.07.92
|*    Letzte Aenderung  JP 09.07.92
|*
*************************************************************************/
void SwNodes::Move( SwPaM & rPam, SwPosition & rPos, SwNodes& rNodes,
                    BOOL bSplitNd )
{
    SwPosition *pStt = (SwPosition*)rPam.Start(), *pEnd = (SwPosition*)rPam.End();

    if( !rPam.HasMark() || *pStt >= *pEnd )
        return;

    if( this == &rNodes && *pStt <= rPos && rPos < *pEnd )
        return;

    SwNodeIndex aEndIdx( pEnd->nNode );
    SwNodeIndex aSttIdx( pStt->nNode );
    SwTxtNode* pSrcNd = (*this)[ aSttIdx ]->GetTxtNode();
    SwTxtNode* pDestNd = rNodes[ rPos.nNode ]->GetTxtNode();
    BOOL bSplitDestNd = TRUE;
    BOOL bSttTxtNd = 0 != pSrcNd;
    BOOL bCopyCollFmt = pDestNd && !pDestNd->GetTxt().Len();

    if( pSrcNd )
    {
        // ist der 1.Node ein TextNode, dann muss im NodesArray auch
        // ein TextNode vorhanden sein, in den der Inhalt geschoben wird
        if( !pDestNd )
        {
            pDestNd = rNodes.MakeTxtNode( rPos.nNode, pSrcNd->GetTxtColl() );
            rPos.nNode--;
            rPos.nContent.Assign( pDestNd, 0 );
            bCopyCollFmt = TRUE;
        }
/*!NOSPLIT      bSplitDestNd = !bSplitNd &&
                        ( pDestNd->Len() > rPos.nContent.GetIndex() ||
                        !aEndIdx.GetNode().IsTxtNode() );
*/
//      ASSERT( bSplitNd, "Move mit bSplitNode = FALSE" );
        bSplitDestNd = pDestNd->Len() > rPos.nContent.GetIndex() ||
                        pEnd->nNode.GetNode().IsTxtNode();

        // verschiebe jetzt noch den Inhalt in den neuen Node
        BOOL bOneNd = pStt->nNode == pEnd->nNode;
        xub_StrLen nLen = ( bOneNd ? pEnd->nContent.GetIndex() : pSrcNd->Len() )
                        - pStt->nContent.GetIndex();

        if( !pEnd->nNode.GetNode().IsCntntNode() )
        {
            bOneNd = TRUE;
            ULONG nSttNdIdx = pStt->nNode.GetIndex() + 1,
                    nEndNdIdx = pEnd->nNode.GetIndex();
            for( ; nSttNdIdx < nEndNdIdx; ++nSttNdIdx )
                if( (*this)[ nSttNdIdx ]->IsCntntNode() )
                {
                    bOneNd = FALSE;
                    break;
                }
        }

        // das kopieren / setzen der Vorlagen darf erst nach
        // dem Splitten erfolgen
//!NOSPLIT      if( !bOneNd && ( bSplitNd || bSplitDestNd ))
        if( !bOneNd && bSplitDestNd )
        {
            if( rNodes.IsDocNodes() )
            {
                SwDoc* pInsDoc = pDestNd->GetDoc();
                BOOL bIsUndo = pInsDoc->DoesUndo();
                pInsDoc->DoUndo( FALSE );
                pInsDoc->SplitNode( rPos );
                pInsDoc->DoUndo( bIsUndo );
            }
            else
                pDestNd->SplitNode( rPos );

            if( rPos.nNode == aEndIdx )
                aEndIdx--;
            bSplitDestNd = TRUE;

            pDestNd = rNodes[ rPos.nNode.GetIndex() - 1 ]->GetTxtNode();
            if( nLen )
                pSrcNd->Cut( pDestNd, SwIndex( pDestNd, pDestNd->Len()),
                            pStt->nContent, nLen );
        }
        else if( nLen )
            pSrcNd->Cut( pDestNd, rPos.nContent, pStt->nContent, nLen );

        if( bCopyCollFmt )
        {
            SwDoc* pInsDoc = pDestNd->GetDoc();
            BOOL bIsUndo = pInsDoc->DoesUndo();
            pInsDoc->DoUndo( FALSE );
            pSrcNd->CopyCollFmt( *pDestNd );
            pInsDoc->DoUndo( bIsUndo );
        }

        if( bOneNd )        // das wars schon
        {
            // der PaM wird korrigiert, denn falls ueber Nodegrenzen verschoben
            // wurde, so stehen sie in unterschieden Nodes. Auch die Selektion
            // wird aufgehoben !
            pEnd->nContent = pStt->nContent;
            rPam.DeleteMark();
            return;
        }

        aSttIdx++;
    }
    else if( pDestNd )
    {
        if( rPos.nContent.GetIndex() )
        {
//!NOSPLIT          if( !bSplitNd && rPos.nContent.GetIndex() == pDestNd->Len() )
            if( rPos.nContent.GetIndex() == pDestNd->Len() )
                rPos.nNode++;
            else if( rPos.nContent.GetIndex() )
            {
                // falls im EndNode gesplittet wird, dann muss der EndIdx
                // korrigiert werden !!
                BOOL bCorrEnde = aEndIdx == rPos.nNode;
                // es wird kein Text an den TextNode angehaengt, also splitte ihn

                if( rNodes.IsDocNodes() )
                {
                    SwDoc* pInsDoc = pDestNd->GetDoc();
                    BOOL bIsUndo = pInsDoc->DoesUndo();
                    pInsDoc->DoUndo( FALSE );
                    pInsDoc->SplitNode( rPos );
                    pInsDoc->DoUndo( bIsUndo );
                }
                else
                    pDestNd->SplitNode( rPos );

                pDestNd = rPos.nNode.GetNode().GetTxtNode();

                if( bCorrEnde )
                    aEndIdx--;
            }
        }
        // am Ende steht noch ein leerer Text Node herum.
        bSplitDestNd = TRUE;
    }

    pSrcNd = (*this)[ aEndIdx ]->GetTxtNode();
    if( pSrcNd )
    {
//      if( pEnd->nContent.GetIndex() ? TRUE : aEndIdx != pStt->nNode )
        {
            // am Bereichsende entsteht ein neuer TextNode
            if( !bSplitDestNd )
            {
                if( rPos.nNode < rNodes.GetEndOfContent().GetIndex() )
                    rPos.nNode++;

                pDestNd = rNodes.MakeTxtNode( rPos.nNode, pSrcNd->GetTxtColl() );
                rPos.nNode--;
                rPos.nContent.Assign( pDestNd, 0 );
            }
            else
                pDestNd = rNodes[ rPos.nNode ]->GetTxtNode();

            if( pDestNd && pEnd->nContent.GetIndex() )
            {
                // verschiebe jetzt noch den Inhalt in den neuen Node
                SwIndex aIdx( pSrcNd, 0 );
                pSrcNd->Cut( pDestNd, rPos.nContent, aIdx,
                                pEnd->nContent.GetIndex());
            }

            if( bCopyCollFmt )
            {
                SwDoc* pInsDoc = pDestNd->GetDoc();
                BOOL bIsUndo = pInsDoc->DoesUndo();
                pInsDoc->DoUndo( FALSE );
                pSrcNd->CopyCollFmt( *pDestNd );
                pInsDoc->DoUndo( bIsUndo );
            }
        }
    }
    else
    {
        if( bSttTxtNd && aEndIdx.GetNode().IsCntntNode() )
            aEndIdx++;
//!NOSPLIT
        if( !bSplitDestNd )
        {
            rPos.nNode++;
            rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(), 0 );
        }
//!NOSPLIT
    }

    if( aEndIdx != aSttIdx )
    {
        // verschiebe jetzt die Nodes in das NodesArary
        SwNodeIndex aPrvIdx( rPos.nNode, -1 );
        ULONG nSttDiff = aSttIdx.GetIndex() - pStt->nNode.GetIndex();
        SwNodeRange aRg( aSttIdx, aEndIdx );
        _MoveNodes( aRg, rNodes, rPos.nNode );
        // falls ins gleiche Nodes-Array verschoben wurde, stehen die
        // Indizies jetzt auch an der neuen Position !!!!
        // (also alles wieder umsetzen)
        if( &rNodes == this )
            pStt->nNode = aRg.aEnd.GetIndex() - nSttDiff;
    }

    // falls der Start-Node verschoben wurde, in dem der Cursor stand, so
    // muss der Content im akt. Content angemeldet werden !!!
    if( &pStt->nNode.GetNode() == &GetEndOfContent() &&
        !GoPrevious( &pStt->nNode ))
    {
        ASSERT( FALSE, "Move() - kein ContentNode mehr vorhanden" );
    }
    pStt->nContent.Assign( (*this)[ pStt->nNode ]->GetCntntNode(),
                            pStt->nContent.GetIndex() );
    // der PaM wird korrigiert, denn falls ueber Nodegrenzen verschoben
    // wurde, so stehen sie in unterschielichen Nodes. Auch die Selektion
    // wird aufgehoben !
    *pEnd = *pStt;
    rPam.DeleteMark();
}



/*************************************************************************
|*
|*    SwNodes::_Copy()
|*
|*    Beschreibung
|*    Parameter         SwNodeRange&    zu kopierender Bereich
|*                      SwDoc&      in dieses Dokument
|*                      SwIndex&    auf diese Position im Nodes-Array
|*    Ersterstellung    JP 11.11.92
|*    Letzte Aenderung  JP 11.11.92
|*
*************************************************************************/

inline BYTE MaxLvl( BYTE nMin, BYTE nMax, short nNew )
{
    return (BYTE)(nNew < nMin ? nMin : nNew > nMax ? nMax : nNew);
}

void SwNodes::_CopyNodes( const SwNodeRange& rRange,
            const SwNodeIndex& rIndex, BOOL bNewFrms, BOOL bTblInsDummyNode ) const
{
    SwDoc* pDoc = rIndex.GetNode().GetDoc();

    SwNode * pAktNode;
    if( rIndex == 0 ||
        ( (pAktNode = &rIndex.GetNode())->GetStartNode() &&
          !pAktNode->StartOfSectionIndex() ))
        return;

    SwNodeRange aRg( rRange );

    // "einfache" StartNodes oder EndNodes ueberspringen
    while( ND_STARTNODE == (pAktNode = (*this)[ aRg.aStart ])->GetNodeType()
            || ( pAktNode->IsEndNode() &&
                !pAktNode->pStartOfSection->IsSectionNode() ) )
        aRg.aStart++;

    // falls aEnd-1 auf keinem ContentNode steht, dann suche den vorherigen
    aRg.aEnd--;
    while( (( pAktNode = (*this)[ aRg.aEnd ])->GetStartNode() &&
            !pAktNode->IsSectionNode() ) ||
            ( pAktNode->IsEndNode() &&
            ND_STARTNODE == pAktNode->pStartOfSection->GetNodeType()) )
        aRg.aEnd--;
    aRg.aEnd++;

    // wird im selben Array's verschoben, dann ueberpruefe die Einfuegepos.
    if( aRg.aStart >= aRg.aEnd )
        return;

    if( this == &pDoc->GetNodes() &&
        rIndex.GetIndex() >= aRg.aStart.GetIndex() &&
        rIndex.GetIndex() < aRg.aEnd.GetIndex() )
            return;

    SwNodeIndex aInsPos( rIndex );
    SwNodeIndex aOrigInsPos( rIndex, -1 );          // Originale Insert Pos
    USHORT nLevel = 0;                          // Level-Counter

    for( ULONG nNodeCnt = aRg.aEnd.GetIndex() - aRg.aStart.GetIndex();
            nNodeCnt > 0; --nNodeCnt )
    {
        pAktNode = &aRg.aStart.GetNode();
        switch( pAktNode->GetNodeType() )
        {
        case ND_TABLENODE:
            // dann kopiere mal den TableNode
            // Tabelle in Tabelle kopieren ?
            // Tabell in Fussnote kopieren ?
            if( pDoc->IsIdxInTbl( aInsPos ) ||
                ( aInsPos < pDoc->GetNodes().GetEndOfInserts().GetIndex() &&
                    pDoc->GetNodes().GetEndOfInserts().StartOfSectionIndex()
                    < aInsPos.GetIndex() ))
            {
                nNodeCnt -=
                    ( pAktNode->EndOfSectionIndex() -
                        aRg.aStart.GetIndex() );

                // dann alle Nodes der Tabelle in die akt. Zelle kopieren
                // fuer den TabellenNode einen DummyNode einfuegen?
                if( bTblInsDummyNode )
                    new SwNode( aInsPos, ND_SECTIONDUMMY );

                for( aRg.aStart++; aRg.aStart.GetIndex() <
                    pAktNode->EndOfSectionIndex();
                    aRg.aStart++ )
                {
                    // fuer den Box-StartNode einen DummyNode einfuegen?
                    if( bTblInsDummyNode )
                        new SwNode( aInsPos, ND_SECTIONDUMMY );

                    SwStartNode* pSttNd = aRg.aStart.GetNode().GetStartNode();
                    _CopyNodes( SwNodeRange( *pSttNd, + 1,
                                            *pSttNd->EndOfSectionNode() ),
                                aInsPos, bNewFrms, FALSE );

                    // fuer den Box-EndNode einen DummyNode einfuegen?
                    if( bTblInsDummyNode )
                        new SwNode( aInsPos, ND_SECTIONDUMMY );
                    aRg.aStart = *pSttNd->EndOfSectionNode();
                }
                // fuer den TabellenEndNode einen DummyNode einfuegen?
                if( bTblInsDummyNode )
                    new SwNode( aInsPos, ND_SECTIONDUMMY );
                aRg.aStart = *pAktNode->EndOfSectionNode();
            }
            else
            {
                SwNodeIndex nStt( aInsPos, -1 );
                SwTableNode* pTblNd = ((SwTableNode*)pAktNode)->
                                        MakeCopy( pDoc, aInsPos );
                nNodeCnt -= aInsPos.GetIndex() - nStt.GetIndex() -2;

                aRg.aStart = pAktNode->EndOfSectionIndex();

                if( bNewFrms && pTblNd )
                {
                    nStt = aInsPos;
                    pTblNd->MakeFrms( &nStt );
                }
            }
            break;

        case ND_SECTIONNODE:            // SectionNode
            // der gesamte Bereich oder nur ein Teil ??
            if( pAktNode->EndOfSectionIndex() < aRg.aEnd.GetIndex() )
            {
                // also der gesamte, lege einen neuen SectionNode an
                SwNodeIndex nStt( aInsPos, -1 );
                SwSectionNode* pSectNd = ((SwSectionNode*)pAktNode)->
                                    MakeCopy( pDoc, aInsPos );

                nNodeCnt -= aInsPos.GetIndex() - nStt.GetIndex() -2;
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
                aInsPos++;                      // EndNode schon vorhanden
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
            if( (const SwNodes*)this == GetDoc()->GetUndoNds() )
            {
                // dann muss an der akt. InsPos auch ein SectionNode
                // (Start/Ende) stehen; dann diesen ueberspringen.
                // Andernfalls nicht weiter beachten.
                SwNode* pTmpNd = pDoc->GetNodes()[ aInsPos ];
                if( pTmpNd->IsSectionNode() ||
                    pTmpNd->FindStartNode()->IsSectionNode() )
                    aInsPos++;  // ueberspringen
            }
            else
                ASSERT( FALSE, "wie kommt diser Node ins Nodes-Array??" );
            break;

        default:
            ASSERT( FALSE, "weder Start-/End-/Content-Node, unbekannter Typ" );
        }
        aRg.aStart++;
    }


#ifdef JP_DEBUG
    {
extern Writer* GetDebugWriter(const String&);

        Writer* pWriter = GetDebugWriter(aEmptyStr);
        if( pWriter )
        {
            int nError;
            SvFileStream aStrm( "c:\\$$copy.db", STREAM_WRITE );
            SwWriter aWriter( aStrm, *pMyDoc );
            aWriter.Write( &nError, pWriter );
        }
    }
#endif
}

void SwNodes::_DelDummyNodes( const SwNodeRange& rRg )
{
    SwNodeIndex aIdx( rRg.aStart );
    while( aIdx.GetIndex() < rRg.aEnd.GetIndex() )
    {
        if( ND_SECTIONDUMMY == aIdx.GetNode().GetNodeType() )
            RemoveNode( aIdx.GetIndex(), 1, TRUE );
        else
            aIdx++;
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
    // (beides auf FALSE ==> GoNext/GoPrevious!!!)
SwCntntNode* SwNodes::GoNextSection( SwNodeIndex * pIdx,
                            int bSkipHidden, int bSkipProtect ) const
{
    int bFirst = TRUE;
    SwNodeIndex aTmp( *pIdx );
    const SwNode* pNd;
    while( aTmp < Count() - 1 )
    {
        if( ND_SECTIONNODE == ( pNd = (*this)[aTmp])->GetNodeType() )
        {
            const SwSection& rSect = ((SwSectionNode*)pNd)->GetSection();
            if( (bSkipHidden && rSect.IsHiddenFlag()) ||
                (bSkipProtect && rSect.IsProtectFlag()) )
                // dann diese Section ueberspringen
                aTmp = *pNd->EndOfSectionNode();
            bFirst = FALSE;
        }
        else if( bFirst )
        {
            bFirst = FALSE;
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
        aTmp++;
        bFirst = FALSE;
    }
    return 0;
}

SwCntntNode* SwNodes::GoPrevSection( SwNodeIndex * pIdx,
                            int bSkipHidden, int bSkipProtect ) const
{
    int bFirst = TRUE;
    SwNodeIndex aTmp( *pIdx );
    const SwNode* pNd;
    while( aTmp > 0 )
    {
        if( ND_ENDNODE == ( pNd = (*this)[aTmp])->GetNodeType() )
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
            bFirst = FALSE;
        }
        else if( bFirst )
        {
            bFirst = FALSE;
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
    if( GetDoc()->GetRootFrm() )
    {
        SwNode* pSttNd = &rFrmIdx.GetNode();

        // wird in eine versteckte Section verschoben ??
        SwSectionNode* pSectNd = pSttNd->IsSectionNode()
                    ? pSttNd->FindStartNode()->FindSectionNode()
                    : pSttNd->FindSectionNode();
        if( !( pSectNd && pSectNd->GetSection().CalcHiddenFlag()/*IsHiddenFlag()*/ ) )
        {
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
            else if( 0 != ( pFrmNd = GoPrevSection( &aIdx, TRUE, FALSE )) &&
                    ::CheckNodesRange( aIdx, rFrmIdx, TRUE ) &&
                    // nach vorne nie aus der Tabelle hinaus!
                    pFrmNd->FindTableNode() == pSttNd->FindTableNode() &&
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
                    ( 0 != ( pFrmNd = GoNextSection( &aIdx, TRUE, FALSE )) &&
                    ::CheckNodesRange( aIdx, rFrmIdx, TRUE ) &&
                    // JP 27.01.99: wenn der "Start"Node ein TabellenNode ist,
                    // dann kann der dahinter liegende nie der gleiche sein!
                    ( pSttNd->IsTableNode() ||
                      ( pFrmNd->FindTableNode() == pSttNd->FindTableNode() &&
                        // Bug 37652: nach hinten nie aus der Tabellenzelle hinaus!
                        (!pFrmNd->FindTableNode() || pFrmNd->FindTableBoxStartNode()
                        == pSttNd->FindTableBoxStartNode() ))) &&
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
                    if( ( pSttNd->IsTableNode() ) &&
                        0 != ( pTblNd = pFrmNd->FindTableNode() ))
                    {
                        pFrmNd = pTblNd;
                        rFrmIdx = *pFrmNd;
                    }
                    else
                        rFrmIdx = aIdx;
                }
                else if( pNd->IsEndNode() && pNd->FindStartNode()->IsTableNode() )
                {
                    pFrmNd = pNd->FindStartNode();
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
                                aIdx++;
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

SwCntntFrm* SwNodes::MakeFrm( const SwNodeIndex &rIndex )
{
    SwCntntNode *pNode = rIndex.GetNode().GetCntntNode();
    return pNode ? pNode->MakeFrm() : 0;
}

void SwNodes::ForEach( const SwNodeIndex& rStart, const SwNodeIndex& rEnd,
                    FnForEach_SwNodes fnForEach, void* pArgs )
{
    BigPtrArray::ForEach( rStart.GetIndex(), rEnd.GetIndex(),
                            (FnForEach) fnForEach, pArgs );
}

struct _TempBigPtrEntry : public BigPtrEntry
{
    _TempBigPtrEntry() {}
};


void SwNodes::RemoveNode( ULONG nDelPos, ULONG nSize, FASTBOOL bDel )
{
    ULONG nEnd = nDelPos + nSize;
    SwNode* pNew = (*this)[ nEnd ];

    if( pRoot )
    {
        SwNodeIndex *p = pRoot;
        while( p )
        {
            ULONG nIdx = p->GetIndex();
            SwNodeIndex* pNext = p->pNext;
            if( nDelPos <= nIdx && nIdx < nEnd )
                (*p) = *pNew;

            p = pNext;
        }

        p = pRoot->pPrev;
        while( p )
        {
            ULONG nIdx = p->GetIndex();
            SwNodeIndex* pPrev = p->pPrev;
            if( nDelPos <= nIdx && nIdx < nEnd )
                (*p) = *pNew;

            p = pPrev;
        }
    }

    if( bDel )
    {
        ULONG nCnt = nSize;
        SwNode *pDel = (*this)[ nDelPos+nCnt-1 ], *pPrev = (*this)[ nDelPos+nCnt-2 ];

#if 1
// temp. Object setzen
        //JP 24.08.98: muessten eigentlich einzeln removed werden, weil
        //      das Remove auch rekursiv gerufen werden kann, z.B. bei
        //      zeichengebundenen Rahmen. Da aber dabei viel zu viel
        //      ablaueft, wird hier ein temp. Objekt eingefuegt, das
        //      dann mit dem Remove wieder entfernt wird.
        // siehe Bug 55406
        _TempBigPtrEntry aTempEntry;
        BigPtrEntry* pTempEntry = &aTempEntry;

        while( nCnt-- )
        {
            delete pDel;
            pDel = pPrev;
            ULONG nPrevNdIdx = pPrev->GetIndex();
            BigPtrArray::Replace( nPrevNdIdx+1, pTempEntry );
            if( nCnt )
                pPrev = (*this)[ nPrevNdIdx  - 1 ];
        }
        nDelPos = pDel->GetIndex() + 1;
    }
#else
// nach jedem Del ein Remove rufen - teuer!
        //JP 24.08.98: muessen leider einzeln removed werden, weil das
        //              auch rekursiv gerufen wird - zeichengeb. Flys!
        // siehe Bug 55406
        while( nCnt-- )
        {
            delete pDel;
            pDel = pPrev;
            ULONG nPrevNdIdx = pPrev->GetIndex();
            BigPtrArray::Remove( nPrevNdIdx+1, 1 );
            if( nCnt )
                pPrev = (*this)[ nPrevNdIdx  - 1 ];
        }
    }
    else
#endif

    BigPtrArray::Remove( nDelPos, nSize );
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
    register SwNodeIndex* pN = rIdx.pNext;
    register SwNodeIndex* pP = rIdx.pPrev;

    if( pRoot == &rIdx )
        pRoot = pP ? pP : pN;

    if( pP )
        pP->pNext = pN;
    if( pN )
        pN->pPrev = pP;

    rIdx.pNext = 0;
    rIdx.pPrev = 0;
}

void SwNodes::Insert( const SwNodePtr pNode, const SwNodeIndex& rPos )
{
    const ElementPtr pIns = pNode;
    BigPtrArray::Insert( pIns, rPos.GetIndex() );
}

void SwNodes::Insert(const SwNodePtr pNode, ULONG nPos)
{
    const ElementPtr pIns = pNode;
    BigPtrArray::Insert( pIns, nPos );
}

