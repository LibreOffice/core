/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <swtable.hxx>
#include <tblsel.hxx>
#include <section.hxx>
#include <ddefld.hxx>
#include <swddetbl.hxx>
#include <frame.hxx>
#include <txtatr.hxx>
#include <tox.hxx>

#include <docsh.hxx>
#include <svl/smplhint.hxx>

typedef std::vector<SwStartNode*> SwSttNdPtrs;


sal_uInt16 HighestLevel( SwNodes & rNodes, const SwNodeRange & rRange );

/** Constructor
 *
 * creates the base sections (PostIts, Inserts, AutoText, RedLines, Content)
 *
 * @param pDocument TODO: provide documentation
 */
SwNodes::SwNodes( SwDoc* pDocument )
    : pRoot( 0 ), pMyDoc( pDocument )
{
    bInNodesDel = bInDelUpdOutl = bInDelUpdNum = sal_False;

    OSL_ENSURE( pMyDoc, "in which Doc am I?" );

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

/** Destructor
 *
 * Deletes all nodes whose pointer are in a dynamic array. This should be no
 * problem as nodes cannot be created outside this array and, thus, cannot be
 * part of multiple arrays.
 */
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

    
    delete pEndOfContent;
}

void SwNodes::ChgNode( SwNodeIndex& rDelPos, sal_uLong nSz,
                        SwNodeIndex& rInsPos, sal_Bool bNewFrms )
{
    
    SwNodes& rNds = rInsPos.GetNodes();
    const SwNode* pPrevInsNd = rNds[ rInsPos.GetIndex() -1 ];

    
    
    if( GetDoc()->SetFieldsDirty( true, &rDelPos.GetNode(), nSz ) &&
        rNds.GetDoc() != GetDoc() )
        rNds.GetDoc()->SetFieldsDirty( true, NULL, 0 );

    
    sal_uLong nNd = rInsPos.GetIndex();
    bool bInsOutlineIdx = !(
            rNds.GetEndOfRedlines().StartOfSectionNode()->GetIndex() < nNd &&
            nNd < rNds.GetEndOfRedlines().GetIndex() );

    if( &rNds == this ) 
    {
        
        
        sal_uInt16 nDiff = rDelPos.GetIndex() < rInsPos.GetIndex() ? 0 : 1;

        for( sal_uLong n = rDelPos.GetIndex(); nSz; n += nDiff, --nSz )
        {
            SwNodeIndex aDelIdx( *this, n );
            SwNode& rNd = aDelIdx.GetNode();

            
            
            
            
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


                if( RES_CONDTXTFMTCOLL == rTxtNd.GetTxtColl()->Which() )
                    rTxtNd.ChkCondColl();

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
            bSavePersData = bRestPersData = true;

        OUString sNumRule;
        SwNodeIndex aInsPos( rInsPos );
        for( sal_uLong n = 0; n < nSz; n++ )
        {
            SwNode* pNd = &rDelPos.GetNode();

            
            if( pNd->IsNoTxtNode() )
            {
                if( bSavePersData )
                    ((SwNoTxtNode*)pNd)->SavePersistentData();
            }
            else if( pNd->IsTxtNode() )
            {
                SwTxtNode* pTxtNd = (SwTxtNode*)pNd;

                
                if (pTxtNd->IsOutline())
                {
                    pOutlineNds->erase( pNd );
                }

                
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
                    
                    pTxtNd->InvalidateNumRule();

                pTxtNd->RemoveFromList();
            }

            RemoveNode( rDelPos.GetIndex(), 1, sal_False ); 
            SwCntntNode * pCNd = pNd->GetCntntNode();
            rNds.InsertNode( pNd, aInsPos );

            if( pCNd )
            {
                SwTxtNode* pTxtNd = pCNd->GetTxtNode();
                if( pTxtNd )
                {
                    SwpHints * const pHts = pTxtNd->GetpSwpHints();
                    
                    if (bInsOutlineIdx && pTxtNd->IsOutline())
                    {
                        rNds.pOutlineNds->insert( pTxtNd );
                    }

                    pTxtNd->AddToList();

                    
                    if( pHts && pHts->Count() )
                    {
                        bool const bToUndo = !pDestDoc &&
                            GetDoc()->GetIDocumentUndoRedo().IsUndoNodes(rNds);
                        for( sal_uInt16 i = pHts->Count(); i; )
                        {
                            sal_uInt16 nDelMsg = 0;
                            SwTxtAttr * const pAttr = pHts->GetTextHint( --i );
                            switch ( pAttr->Which() )
                            {
                            case RES_TXTATR_FIELD:
                            case RES_TXTATR_ANNOTATION:
                            case RES_TXTATR_INPUTFIELD:
                                {
                                    SwTxtFld* pTxtFld = static_cast<SwTxtFld*>(pAttr);
                                    rNds.GetDoc()->InsDelFldInFldLst( !bToUndo, *pTxtFld );

                                    const SwFieldType* pTyp = pTxtFld->GetFmtFld().GetField()->GetTyp();
                                    if ( RES_POSTITFLD == pTyp->Which() )
                                    {
                                        rNds.GetDoc()->GetDocShell()->Broadcast(
                                            SwFmtFldHint(
                                                &pTxtFld->GetFmtFld(),
                                                ( pTxtFld->GetFmtFld().IsFldInDoc()
                                                  ? SWFMTFLD_INSERTED
                                                  : SWFMTFLD_REMOVED ) ) );
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
                    
                    if( RES_CONDTXTFMTCOLL == pTxtNd->GetTxtColl()->Which() )
                        pTxtNd->ChkCondColl();
                    
                }
                else
                {
                    
                    if( pCNd->IsNoTxtNode() && bRestPersData )
                        ((SwNoTxtNode*)pCNd)->RestorePersistentData();
                }
            }
        }
    }

    
    
    GetDoc()->SetFieldsDirty( true, NULL, 0 );
    if( rNds.GetDoc() != GetDoc() )
        rNds.GetDoc()->SetFieldsDirty( true, NULL, 0 );

    if( bNewFrms )
        bNewFrms = &GetDoc()->GetNodes() == (const SwNodes*)&rNds &&
                    GetDoc()->GetCurrentViewShell();

    if( bNewFrms )
    {
        
        SwNodeIndex aIdx( *pPrevInsNd, 1 );
        SwNodeIndex aFrmNdIdx( aIdx );
        SwNode* pFrmNd = rNds.FindPrvNxtFrmNode( aFrmNdIdx,
                                        rNds[ rInsPos.GetIndex() - 1 ] );

        if( !pFrmNd && aFrmNdIdx > rNds.GetEndOfExtras().GetIndex() )
        {
            OSL_ENSURE( !this, "here, something wrong happened" );
            aFrmNdIdx = rNds.GetEndOfContent();
            pFrmNd = rNds.GoPrevSection( &aFrmNdIdx, true, false );
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


/** move the node pointer
 *
 * Move the node pointer from "(inclusive) start position to (exclusive) end
 * position" to target position.
 * If the target is in front of the first or in the area between first and
 * last element to move, nothing happens.
 * If the area to move is empty or the end position is before the start
 * position, nothing happens.
 *
 * @param aRange range to move (excluding end node)
 * @param rNodes
 * @param aIndex
 * @param bNewFrms
 * @return
 */
sal_Bool SwNodes::_MoveNodes( const SwNodeRange& aRange, SwNodes & rNodes,
                    const SwNodeIndex& aIndex, sal_Bool bNewFrms )
{
    SwNode * pAktNode;
    if( aIndex == 0 ||
        ( (pAktNode = &aIndex.GetNode())->GetStartNode() &&
          !pAktNode->StartOfSectionIndex() ))
        return sal_False;

    SwNodeRange aRg( aRange );

    
    while( ND_STARTNODE == (pAktNode = &aRg.aStart.GetNode())->GetNodeType()
            || ( pAktNode->IsEndNode() &&
                !pAktNode->pStartOfSection->IsSectionNode() ) )
        aRg.aStart++;
    aRg.aStart--;

    
    aRg.aEnd--;
    while( ( (( pAktNode = &aRg.aEnd.GetNode())->GetStartNode() &&
            !pAktNode->IsSectionNode() ) ||
            ( pAktNode->IsEndNode() &&
            ND_STARTNODE == pAktNode->pStartOfSection->GetNodeType()) ) &&
            aRg.aEnd > aRg.aStart )
        aRg.aEnd--;

    
    if( aRg.aStart >= aRg.aEnd )
        return sal_False;

    if( this == &rNodes )
    {
        if( ( aIndex.GetIndex()-1 >= aRg.aStart.GetIndex() &&
              aIndex.GetIndex()-1 < aRg.aEnd.GetIndex()) ||
            ( aIndex.GetIndex()-1 == aRg.aEnd.GetIndex() ) )
            return sal_False;
    }

    sal_uInt16 nLevel = 0; 
    sal_uLong nInsPos = 0; 

    
    SwSttNdPtrs aSttNdStack;

    
    SwNodeIndex  aIdx( aIndex );

    SwStartNode* pStartNode = aIdx.GetNode().pStartOfSection;
    aSttNdStack.insert( aSttNdStack.begin(), pStartNode );

    SwNodeRange aOrigInsPos( aIdx, -1, aIdx ); 

    
    sal_uInt16 nSectNdCnt = 0;
    sal_Bool bSaveNewFrms = bNewFrms;

    
    while( aRg.aStart < aRg.aEnd )
        switch( (pAktNode = &aRg.aEnd.GetNode())->GetNodeType() )
        {
        case ND_ENDNODE:
            {
                if( nInsPos ) 
                {
                    
                    
                    SwNodeIndex aSwIndex( aRg.aEnd, 1 );
                    ChgNode( aSwIndex, nInsPos, aIdx, bNewFrms );
                    aIdx -= nInsPos;
                    nInsPos = 0;
                }

                SwStartNode* pSttNd = pAktNode->pStartOfSection;
                if( pSttNd->IsTableNode() )
                {
                    SwTableNode* pTblNd = (SwTableNode*)pSttNd;

                    
                    nInsPos = (aRg.aEnd.GetIndex() -
                                    pSttNd->GetIndex() )+1;
                    aRg.aEnd -= nInsPos;

                    
                    sal_uLong nNd = aIdx.GetIndex();
                    bool bInsOutlineIdx = !( rNodes.GetEndOfRedlines().
                            StartOfSectionNode()->GetIndex() < nNd &&
                            nNd < rNodes.GetEndOfRedlines().GetIndex() );

                    if( bNewFrms )
                        
                        pTblNd->DelFrms();
                    if( &rNodes == this ) 
                    {
                        
                        
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
                        
                        
                        
                        SwStartNode* pSttNode = aIdx.GetNode().pStartOfSection;

                        
                        
                        
                        
                        SwNodeIndex aMvIdx( aRg.aEnd, 1 );
                        for( sal_uLong n = 0; n < nInsPos; ++n )
                        {
                            SwNode* pNd = &aMvIdx.GetNode();

                            const bool bOutlNd = pNd->IsTxtNode() &&
                                static_cast<SwTxtNode*>(pNd)->IsOutline();
                            
                            if( bOutlNd )
                                pOutlineNds->erase( pNd );

                            RemoveNode( aMvIdx.GetIndex(), 1, sal_False );
                            pNd->pStartOfSection = pSttNode;
                            rNodes.InsertNode( pNd, aIdx );

                            
                            if( bInsOutlineIdx && bOutlNd )
                                
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
                    
                    
                    
                    do {        
                        if( !pSttNd->IsSectionNode() )
                        {
                            
                            SwStartNode* pTmp = new SwStartNode( aIdx,
                                                    ND_STARTNODE,
/*?? NodeType ??*/                                  SwNormalStartNode );

                            nLevel++; 
                            aSttNdStack.insert( aSttNdStack.begin() + nLevel, pTmp );

                            
                            new SwEndNode( aIdx, *pTmp );
                        }
                        else if (GetDoc()->GetIDocumentUndoRedo().IsUndoNodes(
                                    rNodes))
                        {
                            
                            new SwDummySectionNode( aIdx );
                        }
                        else
                        {
                            
                            aRg.aEnd--;
                            break;

                        }

                        aRg.aEnd--;
                        aIdx--;
                    } while( false );
                }
                else
                {
                    

                    
                    
                    if( &aRg.aStart.GetNode() == pSttNd )
                        --aRg.aStart;

                    SwSectionNode* pSctNd = pSttNd->GetSectionNode();
                    if( bNewFrms && pSctNd )
                        pSctNd->DelFrms();

                    RemoveNode( aRg.aEnd.GetIndex(), 1, sal_False ); 
                    sal_uLong nSttPos = pSttNd->GetIndex();

                    
                    SwStartNode* pTmpSttNd = new SwStartNode( *this, nSttPos+1 );
                    pTmpSttNd->pStartOfSection = pSttNd->pStartOfSection;

                    RemoveNode( nSttPos, 1, sal_False ); 

                    pSttNd->pStartOfSection = aIdx.GetNode().pStartOfSection;
                    rNodes.InsertNode( pSttNd, aIdx  );
                    rNodes.InsertNode( pAktNode, aIdx );
                    aIdx--;
                    pSttNd->pEndOfSection = (SwEndNode*)pAktNode;

                    aRg.aEnd--;

                    nLevel++; 
                    aSttNdStack.insert( aSttNdStack.begin() + nLevel, pSttNd );

                    
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
                
                if( nInsPos ) 
                {
                    
                    
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
            
        case ND_TABLENODE:
        case ND_STARTNODE:
            {
                
                
                if( !nInsPos && !nLevel )
                {
                    aRg.aEnd--;
                    break;
                }

                if( !nLevel ) 
                {
                    
                    SwNodeIndex aTmpSIdx( aOrigInsPos.aStart, 1 );
                    SwStartNode* pTmpStt = new SwStartNode( aTmpSIdx,
                                ND_STARTNODE,
                                ((SwStartNode*)pAktNode)->GetStartNodeType() );

                    aTmpSIdx--;

                    SwNodeIndex aTmpEIdx( aOrigInsPos.aEnd );
                    new SwEndNode( aTmpEIdx, *pTmpStt );
                    aTmpEIdx--;
                    ++aTmpSIdx;

                    
                    aRg.aEnd++;
                    {
                        SwNodeIndex aCntIdx( aRg.aEnd );
                        for( sal_uLong n = 0; n < nInsPos; n++, aCntIdx++)
                            aCntIdx.GetNode().pStartOfSection = pTmpStt;
                    }

                    
                    while( aTmpSIdx < aTmpEIdx )
                        if( 0 != (( pAktNode = &aTmpEIdx.GetNode())->GetEndNode()) )
                            aTmpEIdx = pAktNode->StartOfSectionIndex();
                        else
                        {
                            pAktNode->pStartOfSection = pTmpStt;
                            aTmpEIdx--;
                        }

                    aIdx--;     
                    aRg.aEnd--; 
                    
                    
                    SwNodeIndex aSwIndex( aRg.aEnd, 1 );
                    ChgNode( aSwIndex, nInsPos, aIdx, bNewFrms );
                    aIdx -= nInsPos+1;
                    nInsPos = 0;
                }
                else 
                {
                    OSL_ENSURE( pAktNode == aSttNdStack[nLevel] ||
                            ( pAktNode->IsStartNode() &&
                                aSttNdStack[nLevel]->IsSectionNode()),
                             "wrong StartNode" );

                    SwNodeIndex aSwIndex( aRg.aEnd, 1 );
                    ChgNode( aSwIndex, nInsPos, aIdx, bNewFrms );
                    aIdx -= nInsPos+1; 
                    nInsPos = 0;

                    
                    RemoveNode( aRg.aEnd.GetIndex(), 1, sal_True );
                    aRg.aEnd--;

                    SwSectionNode* pSectNd = aSttNdStack[ nLevel ]->GetSectionNode();
                    if( pSectNd && !--nSectNdCnt )
                    {
                        SwNodeIndex aTmp( *pSectNd );
                        pSectNd->MakeFrms( &aTmp );
                        bNewFrms = bSaveNewFrms;
                    }
                    aSttNdStack.erase( aSttNdStack.begin() + nLevel ); 
                    nLevel--;
                }

                
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
            
            {
                if( bNewFrms && pAktNode->GetCntntNode() )
                    ((SwCntntNode*)pAktNode)->DelFrms( sal_False );
                pAktNode->pStartOfSection = aSttNdStack[ nLevel ];
                nInsPos++;
                aRg.aEnd--;
            }
            break;
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
                if( &rNodes == this ) 
                {
                    
                    pAktNode->pStartOfSection = aSttNdStack[ nLevel ];
                    nInsPos++;
                }
                else 
                {
                    
                    
                    if( nInsPos ) 
                    {
                        
                        
                        SwNodeIndex aSwIndex( aRg.aEnd, 1 );
                        ChgNode( aSwIndex, nInsPos, aIdx, bNewFrms );
                        aIdx -= nInsPos;
                        nInsPos = 0;
                    }
                    SwNode* pTmpNd = &aIdx.GetNode();
                    if( pTmpNd->IsSectionNode() ||
                        pTmpNd->StartOfSectionNode()->IsSectionNode() )
                        aIdx--; 
                }
            }
            else {
                OSL_FAIL( "How can this node be in the node array?" );
            }
            aRg.aEnd--;
            break;

        default:
            OSL_FAIL( "Unknown node type" );
            break;
        }

    if( nInsPos ) 
    {
        
        SwNodeIndex aSwIndex( aRg.aEnd, 1 );
        ChgNode( aSwIndex, nInsPos, aIdx, bNewFrms );
    }
    aRg.aEnd++; 

    
    if( ( pAktNode = &aRg.aStart.GetNode())->GetStartNode() &&
        pAktNode->StartOfSectionIndex() &&
        aRg.aEnd.GetNode().GetEndNode() )
            DelNodes( aRg.aStart, 2 );

    
    aOrigInsPos.aStart++;
    
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

/** create a start/end section pair
 *
 * Other nodes might be in between.
 *
 * After this method call, the start node of pRange will be pointing to the
 * first node after the start section node and the end node will be the index
 * of the end section node. If this method is called multiple times with the
 * same input, multiple sections containing the previous ones will be created
 * (no content nodes between start or end node).
 *
 * @note Start and end node of the range must be on the same level but MUST
 *       NOT be on the top level.
 *
 * @param [IN,OUT] pRange the range (excl. end)
 * @param eSttNdTyp type of the start node
 */
void SwNodes::SectionDown(SwNodeRange *pRange, SwStartNodeType eSttNdTyp )
{
    if( pRange->aStart >= pRange->aEnd ||
        pRange->aEnd >= Count() ||
        !CheckNodesRange( pRange->aStart, pRange->aEnd ))
        return;

    
    
    
    SwNode * pAktNode = &pRange->aStart.GetNode();
    SwNodeIndex aTmpIdx( *pAktNode->StartOfSectionNode() );

    if( pAktNode->GetEndNode() )
        DelNodes( pRange->aStart, 1 ); 
    else
    {
        
        SwNode* pSttNd = new SwStartNode( pRange->aStart, ND_STARTNODE, eSttNdTyp );
        pRange->aStart = *pSttNd;
        aTmpIdx = pRange->aStart;
    }

    
    
    
    pRange->aEnd--;
    if( pRange->aEnd.GetNode().GetStartNode() )
        DelNodes( pRange->aEnd, 1 );
    else
    {
        pRange->aEnd++;
        
        new SwEndNode( pRange->aEnd, *pRange->aStart.GetNode().GetStartNode() );
    }
    pRange->aEnd--;

    SectionUpDown( aTmpIdx, pRange->aEnd );
}

/** increase level of the given range
 *
 * The range contained in pRange will be lifted to the next higher level.
 * This is done by adding a end node at pRange.start and a start node at
 * pRange.end. Furthermore all indices for this range will be updated.
 *
 * After this method call, the start node of pRange will be pointing to the
 * first node inside the lifted range and the end node will be pointing to the
 * last position inside the lifted range.
 *
 * @param [IN,OUT] pRange the range of nodes where the level should be increased
 */
void SwNodes::SectionUp(SwNodeRange *pRange)
{
    if( pRange->aStart >= pRange->aEnd ||
        pRange->aEnd >= Count() ||
        !CheckNodesRange( pRange->aStart, pRange->aEnd ) ||
        !( HighestLevel( *this, *pRange ) > 1 ))
        return;

    
    
    
    SwNode * pAktNode = &pRange->aStart.GetNode();
    SwNodeIndex aIdx( *pAktNode->StartOfSectionNode() );
    if( pAktNode->IsStartNode() )       
    {
        SwEndNode* pEndNd = pRange->aEnd.GetNode().GetEndNode();
        if( pAktNode == pEndNd->pStartOfSection )
        {
            
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
    else if( aIdx == pRange->aStart.GetIndex()-1 ) 
        DelNodes( aIdx, 1 );
    else
        new SwEndNode( pRange->aStart, *aIdx.GetNode().GetStartNode() );

    
    
    
    SwNodeIndex aTmpIdx( pRange->aEnd );
    if( pRange->aEnd.GetNode().IsEndNode() )
        DelNodes( pRange->aEnd, 1 );
    else
    {
        pAktNode = new SwStartNode( pRange->aEnd );
/*?? which NodeType ??*/
        aTmpIdx = *pRange->aEnd.GetNode().EndOfSectionNode();
        pRange->aEnd--;
    }

    SectionUpDown( aIdx, aTmpIdx );
}

/** correct indices after movement
 *
 * Update all indices after movement so that the levels are consistent again.
 *
 * @param aStart index of the start node
 * @param aEnd index of the end point
 *
 * @see SwNodes::SectionUp
 * @see SwNodes::SectionDown
 */
void SwNodes::SectionUpDown( const SwNodeIndex & aStart, const SwNodeIndex & aEnd )
{
    SwNode * pAktNode;
    SwNodeIndex aTmpIdx( aStart, +1 );
    
    SwSttNdPtrs aSttNdStack;
    SwStartNode* pTmp = aStart.GetNode().GetStartNode();
    aSttNdStack.push_back( pTmp );

    
    
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
                continue; 

            else if( aTmpIdx < aEnd ) 
                
            {
                aSttNdStack.insert( aSttNdStack.begin(), pSttNd->pStartOfSection );
            }
            else 
                break;
        }
    }
}

/** delete nodes
 *
 * This is a specific implementation of a delete function for a variable array.
 * It is necessary as there might be inconsistencies after deleting start or
 * end nodes. This method can clean those up.
 *
 * @param rIndex position to delete at (unchanged afterwards)
 * @param nNodes number of nodes to delete (default: 1)
 */
void SwNodes::Delete(const SwNodeIndex &rIndex, sal_uLong nNodes)
{
    sal_uInt16 nLevel = 0; 
    SwNode * pAktNode;

    sal_uLong nCnt = Count() - rIndex.GetIndex() - 1;
    if( nCnt > nNodes ) nCnt = nNodes;

    if( nCnt == 0 ) 
        return;

    SwNodeRange aRg( rIndex, 0, rIndex, nCnt-1 );
    
    if( ( !aRg.aStart.GetNode().StartOfSectionIndex() &&
            !aRg.aStart.GetIndex() ) ||
            ! CheckNodesRange( aRg.aStart, aRg.aEnd ) )
        return;

    
    while( ( pAktNode = &aRg.aEnd.GetNode())->GetStartNode() ||
             ( pAktNode->GetEndNode() &&
                !pAktNode->pStartOfSection->IsTableNode() ))
        aRg.aEnd--;

    nCnt = 0;

    
    
    aRg.aStart--;

    sal_Bool bSaveInNodesDel = bInNodesDel;
    bInNodesDel = sal_True;
    bool bUpdateOutline = false;

    
    while( aRg.aStart < aRg.aEnd )
    {
        pAktNode = &aRg.aEnd.GetNode();

        if( pAktNode->GetEndNode() )
        {
            
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
                RemoveNode( aRg.aEnd.GetIndex()+1, nCnt, sal_True ); 
                nCnt = 0;
                aRg.aEnd--; 
                nLevel++;
            }
        }
        else if( pAktNode->GetStartNode() ) 
        {
            if( nLevel == 0 ) 
            {
                if( nCnt )
                {
                    
                    aRg.aEnd++;
                    RemoveNode( aRg.aEnd.GetIndex(), nCnt, sal_True );
                    nCnt = 0;
                }
            }
            else 
            {
                RemoveNode( aRg.aEnd.GetIndex(), nCnt + 2, sal_True ); 
                nCnt = 0;
                nLevel--;
            }

            
            
            SwNode* pTmpNode = aRg.aEnd.GetNode().GetEndNode();
            aRg.aEnd--;
            while(  pTmpNode &&
                    ( pAktNode = &aRg.aEnd.GetNode())->GetStartNode() &&
                    pAktNode->StartOfSectionIndex() )
            {
                
                DelNodes( aRg.aEnd, 2 );
                pTmpNode = aRg.aEnd.GetNode().GetEndNode();
                aRg.aEnd--;
            }
        }
        else 
        {
            SwTxtNode* pTxtNd = pAktNode->GetTxtNode();
            if( pTxtNd )
            {
                if( pTxtNd->IsOutline())
                {
                    
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
        RemoveNode( aRg.aEnd.GetIndex(), nCnt, sal_True ); 

    
    while( aRg.aEnd.GetNode().GetEndNode() &&
            ( pAktNode = &aRg.aStart.GetNode())->GetStartNode() &&
            pAktNode->StartOfSectionIndex() )
    
    {
        DelNodes( aRg.aStart, 2 );  
        aRg.aStart--;
    }

    bInNodesDel = bSaveInNodesDel;

    if( !bInNodesDel )
    {
        
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

/** get section level at the given position
 *
 * @note The first node in an array should always be a start node.
 *       Because of this, there is a special treatment here based on the
 *       assumption that this is true in this context as well.
 *
 * @param rIdx position of the node
 * @return section level at the given position
 */
sal_uInt16 SwNodes::GetSectionLevel(const SwNodeIndex &rIdx) const {
    
    if(rIdx == 0) return 1;
    
    return rIdx.GetNode().GetSectionLevel();
}

void SwNodes::GoStartOfSection(SwNodeIndex *pIdx) const
{
    
    SwNodeIndex aTmp( *pIdx->GetNode().StartOfSectionNode(), +1 );

    
    
    while( !aTmp.GetNode().IsCntntNode() )
    {   
        if( *pIdx <= aTmp )
            return;     
        aTmp = aTmp.GetNode().EndOfSectionIndex()+1;
        if( *pIdx <= aTmp )
            return;     
    }
    (*pIdx) = aTmp;     
}

void SwNodes::GoEndOfSection(SwNodeIndex *pIdx) const
{
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

inline bool TstIdx( sal_uLong nSttIdx, sal_uLong nEndIdx, sal_uLong nStt, sal_uLong nEnd )
{
    return nStt < nSttIdx && nEnd >= nSttIdx &&
            nStt < nEndIdx && nEnd >= nEndIdx;
}

/** Check if the given range is inside the defined ranges
 *
 * The defined ranges are Content, AutoText, PostIts, Inserts, and Redlines.
 *
 * @param rStt start index of the range
 * @param rEnd end index of the range
 * @return <true> if valid range
 */
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

    return sal_False;       
}

/** Delete a number of nodes
 *
 * @param rStart starting position in this nodes array
 * @param nCnt number of nodes to delete
 */
void SwNodes::DelNodes( const SwNodeIndex & rStart, sal_uLong nCnt )
{
    sal_uLong nSttIdx = rStart.GetIndex();

    if( !nSttIdx && nCnt == GetEndOfContent().GetIndex()+1 )
    {
        
        
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
            {
                
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

        
        if( bUpdateNum )
            UpdtOutlineIdx( rStart.GetNode() );
    }
}

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

/** Calculate the highest level in a range
 *
 * @param rNodes the nodes array
 * @param rRange the range to inspect
 * @return the highest level
 */
sal_uInt16 HighestLevel( SwNodes & rNodes, const SwNodeRange & rRange )
{
    HighLevel aPara( rNodes.GetSectionLevel( rRange.aStart ));
    rNodes.ForEach( rRange.aStart, rRange.aEnd, lcl_HighestLevel, &aPara );
    return aPara.nTop;

}

/** move a range
 *
 * @param rPam the range to move
 * @param rPos to destination position in the given nodes array
 * @param rNodes the node array to move the range into
 */
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
        
        
        if( !pDestNd )
        {
            pDestNd = rNodes.MakeTxtNode( rPos.nNode, pSrcNd->GetTxtColl() );
            rPos.nNode--;
            rPos.nContent.Assign( pDestNd, 0 );
            bCopyCollFmt = true;
        }
        bSplitDestNd = pDestNd->Len() > rPos.nContent.GetIndex() ||
                        pEnd->nNode.GetNode().IsTxtNode();

        
        bool bOneNd = pStt->nNode == pEnd->nNode;
        const sal_Int32 nLen =
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

        if( bOneNd )
        {
            
            
            
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
                
                const bool bCorrEnd = aEndIdx == rPos.nNode;

                
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
        
        bSplitDestNd = true;
    }

    SwTxtNode* const pEndSrcNd = aEndIdx.GetNode().GetTxtNode();
    if ( pEndSrcNd )
    {
        {
            
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
        
        const sal_uLong nSttDiff = aSttIdx.GetIndex() - pStt->nNode.GetIndex();
        SwNodeRange aRg( aSttIdx, aEndIdx );
        _MoveNodes( aRg, rNodes, rPos.nNode );

        
        if( &rNodes == this )
        {
            pStt->nNode = aRg.aEnd.GetIndex() - nSttDiff;
        }
    }

    
    
    if ( &pStt->nNode.GetNode() == &GetEndOfContent() )
    {
        const bool bSuccess = GoPrevious( &pStt->nNode );
        OSL_ENSURE( bSuccess, "Move() - no ContentNode here" );
        (void) bSuccess;
    }
    pStt->nContent.Assign( pStt->nNode.GetNode().GetCntntNode(),
                            pStt->nContent.GetIndex() );
    
    
    
    *pEnd = *pStt;
    rPam.DeleteMark();
    GetDoc()->GetDocShell()->Broadcast( SwFmtFldHint( 0,
                rNodes.IsDocNodes() ? SWFMTFLD_INSERTED : SWFMTFLD_REMOVED ) );
}


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

    
    while( ND_STARTNODE == (pAktNode = & aRg.aStart.GetNode())->GetNodeType()
            || ( pAktNode->IsEndNode() &&
                !pAktNode->pStartOfSection->IsSectionNode() ) )
        aRg.aStart++;

    
    aRg.aEnd--;
    
    
    
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

    
    if( aRg.aStart >= aRg.aEnd )
        return;

    
    OSL_ENSURE( &aRg.aStart.GetNodes() == this,
                "aRg should use thisnodes array" );
    OSL_ENSURE( &aRg.aStart.GetNodes() == &aRg.aEnd.GetNodes(),
               "Range across different nodes arrays? You deserve punishment!");
    if( &rIndex.GetNodes() == &aRg.aStart.GetNodes() &&
        rIndex.GetIndex() >= aRg.aStart.GetIndex() &&
        rIndex.GetIndex() < aRg.aEnd.GetIndex() )
            return;

    SwNodeIndex aInsPos( rIndex );
    SwNodeIndex aOrigInsPos( rIndex, -1 ); 
    sal_uInt16 nLevel = 0;                 

    for( sal_uLong nNodeCnt = aRg.aEnd.GetIndex() - aRg.aStart.GetIndex();
            nNodeCnt > 0; --nNodeCnt )
    {
        pAktNode = &aRg.aStart.GetNode();
        switch( pAktNode->GetNodeType() )
        {
        case ND_TABLENODE:
            
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

                
                if( bTblInsDummyNode )
                    new SwDummySectionNode( aInsPos );

                
                for( aRg.aStart++; aRg.aStart.GetIndex() <
                    pAktNode->EndOfSectionIndex();
                    aRg.aStart++ )
                {
                    
                    if( bTblInsDummyNode )
                        new SwDummySectionNode( aInsPos );

                    SwStartNode* pSttNd = aRg.aStart.GetNode().GetStartNode();
                    _CopyNodes( SwNodeRange( *pSttNd, + 1,
                                            *pSttNd->EndOfSectionNode() ),
                                aInsPos, bNewFrms, sal_False );

                    
                    if( bTblInsDummyNode )
                        new SwDummySectionNode( aInsPos );
                    aRg.aStart = *pSttNd->EndOfSectionNode();
                }
                
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

        case ND_SECTIONNODE:
            
            
            
            
            if( pAktNode->EndOfSectionIndex() < aRg.aEnd.GetIndex() )
            {
                
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

        case ND_STARTNODE:
            {
                SwStartNode* pTmp = new SwStartNode( aInsPos, ND_STARTNODE,
                            ((SwStartNode*)pAktNode)->GetStartNodeType() );
                new SwEndNode( aInsPos, *pTmp );
                aInsPos--;
                nLevel++;
            }
            break;

        case ND_ENDNODE:
            if( nLevel ) 
            {
                --nLevel;
                ++aInsPos; 
            }
            else if( !pAktNode->pStartOfSection->IsSectionNode() )
            {
                
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
                
                if( !bNewFrms )
                    pNew->DelFrms();
            }
            break;

        case ND_SECTIONDUMMY:
            if (GetDoc()->GetIDocumentUndoRedo().IsUndoNodes(*this))
            {
                
                
                SwNode *const pTmpNd = & aInsPos.GetNode();
                if( pTmpNd->IsSectionNode() ||
                    pTmpNd->StartOfSectionNode()->IsSectionNode() )
                    ++aInsPos;  
            }
            else {
                OSL_FAIL( "How can this node be in the node array?" );
            }
            break;

        default:
            OSL_FAIL( "Unknown node type" );
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


/** go to next section that is not protected nor hidden
 *
 * @note if !bSkipHidden and !bSkipProtect, use GoNext/GoPrevious
 *
 * @param pIdx
 * @param bSkipHidden
 * @param bSkipProtect
 * @return
 * @see SwNodes::GoNext
 * @see SwNodes::GoPrevious
 * @see SwNodes::GoNextSection (TODO: seems to be C&P programming here)
*/
SwCntntNode* SwNodes::GoNextSection( SwNodeIndex * pIdx,
                            bool bSkipHidden, bool bSkipProtect ) const
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
                            bool bSkipHidden, bool bSkipProtect ) const
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



/** find the next/previous ContentNode or a table node with frames
 *
 * If no pEnd is given, search is started with FrameIndex; otherwise
 * search is started with the one before rFrmIdx and after pEnd.
 *
 * @param rFrmIdx node with frames to search in
 * @param pEnd ???
 * @return result node; 0 (!!!) if not found
 */
SwNode* SwNodes::FindPrvNxtFrmNode( SwNodeIndex& rFrmIdx,
                                    const SwNode* pEnd ) const
{
    SwNode* pFrmNd = 0;

    
    if( GetDoc()->GetCurrentViewShell() )
    {
        SwNode* pSttNd = &rFrmIdx.GetNode();

        
        SwSectionNode* pSectNd = pSttNd->IsSectionNode()
                    ? pSttNd->StartOfSectionNode()->FindSectionNode()
                    : pSttNd->FindSectionNode();
        if( !( pSectNd && pSectNd->GetSection().CalcHiddenFlag() ) )
        {
            
            
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

            
            else if( 0 != ( pFrmNd = GoPrevSection( &aIdx, true, false )) &&
                    ::CheckNodesRange( aIdx, rFrmIdx, true ) &&
                    
                    pFrmNd->FindTableNode() == pTableNd &&
                    
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

                
                if( ( pEnd && ( pFrmNd = &aIdx.GetNode())->IsCntntNode() ) ||
                    ( 0 != ( pFrmNd = GoNextSection( &aIdx, true, false )) &&
                    ::CheckNodesRange( aIdx, rFrmIdx, true ) &&
                    ( pFrmNd->FindTableNode() == pTableNd &&
                        
                        (!pFrmNd->FindTableNode() || pFrmNd->FindTableBoxStartNode()
                        == pSttNd->FindTableBoxStartNode() ) ) &&
                     (!pSectNd || pSttNd->IsSectionNode() ||
                      pSectNd->EndOfSectionIndex() > pFrmNd->GetIndex())
                    ))
                {
                    
                    
                    
                    SwTableNode* pTblNd;
                    if( pSttNd->IsTableNode() &&
                        0 != ( pTblNd = pFrmNd->FindTableNode() ) &&
                        
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
    if( !pRoot ) 
    {
        pRoot = &rIdx;
        pRoot->pPrev = 0;
        pRoot->pNext = 0;
    }
    else
    {
        
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
