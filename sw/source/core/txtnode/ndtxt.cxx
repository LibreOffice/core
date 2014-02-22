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

#include <hintids.hxx>
#include <hints.hxx>

#include <comphelper/string.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/rsiditem.hxx>
#include <editeng/tstpitem.hxx>
#include <svl/urihelper.hxx>
#include <svl/ctloptions.hxx>
#include <swmodule.hxx>
#include <txtfld.hxx>
#include <txtinet.hxx>
#include <fmtinfmt.hxx>
#include <fmtpdsc.hxx>
#include <txtatr.hxx>
#include <fmtrfmrk.hxx>
#include <txttxmrk.hxx>
#include <fchrfmt.hxx>
#include <txtftn.hxx>
#include <fmtflcnt.hxx>
#include <fmtfld.hxx>
#include <frmatr.hxx>
#include <charatr.hxx>
#include <ftnidx.hxx>
#include <ftninfo.hxx>
#include <fmtftn.hxx>
#include <fmtmeta.hxx>
#include <charfmt.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <fldbas.hxx>
#include <paratr.hxx>
#include <txtfrm.hxx>
#include <ftnfrm.hxx>
#include <ftnboss.hxx>
#include <rootfrm.hxx>
#include <pagedesc.hxx>
#include <expfld.hxx>
#include <section.hxx>
#include <mvsave.hxx>
#include <swcache.hxx>
#include <SwGrammarMarkUp.hxx>
#include <dcontact.hxx>
#include <redline.hxx>
#include <doctxm.hxx>
#include <IMark.hxx>
#include <scriptinfo.hxx>
#include <istyleaccess.hxx>
#include <SwStyleNameMapper.hxx>
#include <numrule.hxx>
#include <swtable.hxx>
#include <docsh.hxx>
#include <SwNodeNum.hxx>
#include <svl/intitem.hxx>
#include <list.hxx>
#include <switerator.hxx>
#include <attrhint.hxx>

using namespace ::com::sun::star;

typedef std::vector<SwTxtAttr*> SwpHts;

TYPEINIT1( SwTxtNode, SwCntntNode )


#ifdef DBG_UTIL
#define CHECK_SWPHINTS(pNd)  { if( pNd->GetpSwpHints() && \
                                   !pNd->GetDoc()->IsInReading() ) \
                                  pNd->GetpSwpHints()->Check(true); }
#else
#define CHECK_SWPHINTS(pNd)
#endif

SwTxtNode *SwNodes::MakeTxtNode( const SwNodeIndex & rWhere,
                                 SwTxtFmtColl *pColl,
                                 SwAttrSet* pAutoAttr )
{
    OSL_ENSURE( pColl, "Collection pointer is 0." );

    SwTxtNode *pNode = new SwTxtNode( rWhere, pColl, pAutoAttr );

    SwNodeIndex aIdx( *pNode );

    
    
    if ( IsDocNodes() )
        UpdateOutlineNode(*pNode);

    
    const SwSectionNode* pSectNd;
    if( !GetDoc()->GetCurrentViewShell() ||
        ( 0 != (pSectNd = pNode->FindSectionNode()) &&
            pSectNd->GetSection().IsHiddenFlag() ))
        return pNode;

    SwNodeIndex aTmp( rWhere );
    do {
        
        
        

        SwNode * pNd = & aTmp.GetNode();
        switch (pNd->GetNodeType())
        {
        case ND_TABLENODE:
            ((SwTableNode*)pNd)->MakeFrms( aIdx );
            return pNode;

        case ND_SECTIONNODE:
            if( ((SwSectionNode*)pNd)->GetSection().IsHidden() ||
                ((SwSectionNode*)pNd)->IsCntntHidden() )
            {
                SwNodeIndex aTmpIdx( *pNode );
                pNd = FindPrvNxtFrmNode( aTmpIdx, pNode );
                if( !pNd )
                    return pNode;
                aTmp = *pNd;
                break;
            }
            ((SwSectionNode*)pNd)->MakeFrms( aIdx );
            return pNode;

        case ND_TEXTNODE:
        case ND_GRFNODE:
        case ND_OLENODE:
            ((SwCntntNode*)pNd)->MakeFrms( *pNode );
            return pNode;

        case ND_ENDNODE:
            if( pNd->StartOfSectionNode()->IsSectionNode() &&
                aTmp.GetIndex() < rWhere.GetIndex() )
            {
                if( pNd->StartOfSectionNode()->GetSectionNode()->GetSection().IsHiddenFlag())
                {
                    if( !GoPrevSection( &aTmp, true, false ) ||
                        aTmp.GetNode().FindTableNode() !=
                            pNode->FindTableNode() )
                        return pNode;       
                }
                else
                    aTmp = *pNd->StartOfSectionNode();
                break;
            }
            else if( pNd->StartOfSectionNode()->IsTableNode() &&
                    aTmp.GetIndex() < rWhere.GetIndex() )
            {
                
                aTmp = *pNd->StartOfSectionNode();
                break;
            }
            
        default:
            if( rWhere == aTmp )
                aTmp -= 2;
            else
                return pNode;
            break;
        }
    } while( true );
}





SwTxtNode::SwTxtNode( const SwNodeIndex &rWhere,
                      SwTxtFmtColl *pTxtColl,
                      const SfxItemSet* pAutoAttr )
    : SwCntntNode( rWhere, ND_TEXTNODE, pTxtColl ),
      m_pSwpHints( 0 ),
      mpNodeNum( 0 ),
      m_bLastOutlineState( false ),
      m_bNotifiable( false ),
      
      mbEmptyListStyleSetDueToSetOutlineLevelAttr( false ),
      mbInSetOrResetAttr( false ),
      mpList( 0 )
{
    InitSwParaStatistics( true );

    if( pAutoAttr )
        SetAttr( *pAutoAttr );

    if (!IsInList() && GetNumRule() && !GetListId().isEmpty())
    {
        
        
        
        if ( !HasAttrListLevel() &&
             pTxtColl && pTxtColl->IsAssignedToListLevelOfOutlineStyle() )
        {
            SetAttrListLevel( pTxtColl->GetAssignedOutlineStyleLevel() );
        }
        AddToList();
    }
    GetNodes().UpdateOutlineNode(*this);

    m_bNotifiable = true;

    m_bContainsHiddenChars = m_bHiddenCharsHidePara = false;
    m_bRecalcHiddenCharFlags = true;
}

SwTxtNode::~SwTxtNode()
{
    
    if ( m_pSwpHints )
    {
        
        SwpHints* pTmpHints = m_pSwpHints;
        m_pSwpHints = 0;

        for( sal_uInt16 j = pTmpHints->Count(); j; )
        {
            
            
            DestroyAttr( pTmpHints->GetTextHint( --j ) );
        }

        delete pTmpHints;
    }

    
#if OSL_DEBUG_LEVEL > 0
    sal_uInt16 foo;
    assert(!GetNodes().GetOutLineNds().Seek_Entry(this, &foo));
#endif

    RemoveFromList();

    InitSwParaStatistics( false );
}

SwCntntFrm *SwTxtNode::MakeFrm( SwFrm* pSib )
{
    
    
    if (m_pSwpHints)
    {
        m_pSwpHints->MergePortions(*this);
    }
    SwCntntFrm *pFrm = new SwTxtFrm( this, pSib );
    return pFrm;
}

sal_Int32 SwTxtNode::Len() const
{
    return m_Text.getLength();
}

/*---------------------------------------------------------------------------
 * lcl_ChangeFtnRef
 *  After a split node, it's necessary to actualize the ref-pointer of the
 *  ftnfrms.
 * --------------------------------------------------------------------------*/

static void lcl_ChangeFtnRef( SwTxtNode &rNode )
{
    SwpHints *pSwpHints = rNode.GetpSwpHints();
    if( pSwpHints && rNode.GetDoc()->GetCurrentViewShell() )
    {
        SwTxtAttr* pHt;
        SwCntntFrm* pFrm = NULL;
        
        
        
        SwFtnFrm* pFirstFtnOfNode = 0;
        for( sal_uInt16 j = pSwpHints->Count(); j; )
        {
            pHt = pSwpHints->GetTextHint(--j);
            if (RES_TXTATR_FTN == pHt->Which())
            {
                if( !pFrm )
                {
                    pFrm = SwIterator<SwCntntFrm,SwTxtNode>::FirstElement( rNode );
                    if( !pFrm )
                        return;
                }
                SwTxtFtn *pAttr = (SwTxtFtn*)pHt;
                OSL_ENSURE( pAttr->GetStartNode(), "FtnAtr ohne StartNode." );
                SwNodeIndex aIdx( *pAttr->GetStartNode(), 1 );
                SwCntntNode *pNd = aIdx.GetNode().GetCntntNode();
                if ( !pNd )
                    pNd = pFrm->GetAttrSet()->GetDoc()->
                            GetNodes().GoNextSection( &aIdx, true, false );
                if ( !pNd )
                    continue;

                SwIterator<SwCntntFrm,SwCntntNode> aIter( *pNd );
                SwCntntFrm* pCntnt = aIter.First();
                if( pCntnt )
                {
                    OSL_ENSURE( pCntnt->getRootFrm() == pFrm->getRootFrm(),
                            "lcl_ChangeFtnRef: Layout double?" );
                    SwFtnFrm *pFtn = pCntnt->FindFtnFrm();
                    if( pFtn && pFtn->GetAttr() == pAttr )
                    {
                        while( pFtn->GetMaster() )
                            pFtn = pFtn->GetMaster();
                        
                        pFirstFtnOfNode = pFtn;
                        while ( pFtn )
                        {
                            pFtn->SetRef( pFrm );
                            pFtn = pFtn->GetFollow();
                            ((SwTxtFrm*)pFrm)->SetFtn( true );
                        }
                    }
#if OSL_DEBUG_LEVEL > 0
                    while( 0 != (pCntnt = aIter.Next()) )
                    {
                        SwFtnFrm *pDbgFtn = pCntnt->FindFtnFrm();
                        OSL_ENSURE( !pDbgFtn || pDbgFtn->GetRef() == pFrm,
                                "lcl_ChangeFtnRef: Who's that guy?" );
                    }
#endif
                }
            }
        } 
        
        if ( pFirstFtnOfNode )
        {
            SwCntntFrm* pCntnt = pFirstFtnOfNode->ContainsCntnt();
            if ( pCntnt )
            {
                pCntnt->_InvalidatePos();
            }
        }
    }
}

SwCntntNode *SwTxtNode::SplitCntntNode( const SwPosition &rPos )
{
    bool parentIsOutline = IsOutline();

    
    const sal_Int32 nSplitPos = rPos.nContent.GetIndex();
    const sal_Int32 nTxtLen = m_Text.getLength();
    SwTxtNode* const pNode =
        _MakeNewTxtNode( rPos.nNode, false, nSplitPos==nTxtLen );

    
    
    if (nSplitPos != 0) {
        pNode->RegisterAsCopyOf(*this, true);
        if (nSplitPos == nTxtLen)
        {
            this->RemoveMetadataReference();
            
            
        }
    }

    ResetAttr( RES_PARATR_LIST_ISRESTART );
    ResetAttr( RES_PARATR_LIST_RESTARTVALUE );
    ResetAttr( RES_PARATR_LIST_ISCOUNTED );
    if ( GetNumRule() == 0 || (parentIsOutline && !IsOutline()) )
    {
        ResetAttr( RES_PARATR_LIST_ID );
        ResetAttr( RES_PARATR_LIST_LEVEL );
    }

    if ( GetDepends() && !m_Text.isEmpty() && (nTxtLen / 2) < nSplitPos )
    {
        
        
        

        LockModify();   

        
        
        if ( HasHints() )
        {
            pNode->GetOrCreateSwpHints().SetInSplitNode(true);
        }

        
        
        SwIndex aIdx( this );
        CutText( pNode, aIdx, nSplitPos );

        if( GetWrong() )
        {
            pNode->SetWrong( GetWrong()->SplitList( nSplitPos ) );
        }
        SetWrongDirty( true );

        if( GetGrammarCheck() )
        {
            pNode->SetGrammarCheck( GetGrammarCheck()->SplitGrammarList( nSplitPos ) );
        }
        SetGrammarCheckDirty( true );

        SetWordCountDirty( true );

        
        if( GetSmartTags() )
        {
            pNode->SetSmartTags( GetSmartTags()->SplitList( nSplitPos ) );
        }
        SetSmartTagDirty( true );

        if ( pNode->HasHints() )
        {
            if ( pNode->m_pSwpHints->CanBeDeleted() )
            {
                delete pNode->m_pSwpHints;
                pNode->m_pSwpHints = 0;
            }
            else
            {
                pNode->m_pSwpHints->SetInSplitNode(false);
            }

            
            
            
            
            if ( HasHints() )
            {
                for ( sal_uInt16 j = m_pSwpHints->Count(); j; )
                {
                    SwTxtAttr* const pHt = m_pSwpHints->GetTextHint( --j );
                    if ( RES_TXTATR_FLYCNT == pHt ->Which() )
                    {
                        pHt->GetFlyCnt().GetFrmFmt()->DelFrms();
                    }
                    else if ( pHt->DontExpand() )
                    {
                        const sal_Int32* const pEnd = pHt->GetEnd();
                        if (pEnd && *pHt->GetStart() == *pEnd )
                        {
                            
                            m_pSwpHints->DeleteAtPos( j );
                            DestroyAttr( pHt );
                        }
                    }
                }
            }

        }

        SwIterator<SwCntntFrm,SwTxtNode> aIter( *this );
        for( SwCntntFrm* pFrm = aIter.First(); pFrm; pFrm = aIter.Next() )
        {
            pFrm->RegisterToNode( *pNode );
            if( pFrm->IsTxtFrm() && !pFrm->IsFollow() && ((SwTxtFrm*)pFrm)->GetOfst() )
                ((SwTxtFrm*)pFrm)->SetOfst( 0 );
        }

        if ( IsInCache() )
        {
            SwFrm::GetCache().Delete( this );
            SetInCache( sal_False );
        }

        UnlockModify(); 

        
        
        
        const SwRootFrm *pRootFrm;
        if ( (nTxtLen != nSplitPos) ||
            ( (pRootFrm = pNode->GetDoc()->GetCurrentLayout()) != 0 &&
              pRootFrm->IsAnyShellAccessible() ) )
        {
            
            if( 1 == nTxtLen - nSplitPos )
            {
                SwDelChr aHint( nSplitPos );
                pNode->NotifyClients( 0, &aHint );
            }
            else
            {
                SwDelTxt aHint( nSplitPos, nTxtLen - nSplitPos );
                pNode->NotifyClients( 0, &aHint );
            }
        }
        if ( HasHints() )
        {
            MoveTxtAttr_To_AttrSet();
        }
        pNode->MakeFrms( *this );       
        lcl_ChangeFtnRef( *this );
    }
    else
    {
        SwWrongList *pList = GetWrong();
        SetWrong( 0, false );
        SetWrongDirty( true );

        SwGrammarMarkUp *pList3 = GetGrammarCheck();
        SetGrammarCheck( 0, false );
        SetGrammarCheckDirty( true );

        SetWordCountDirty( true );

        
        SwWrongList *pList2 = GetSmartTags();
        SetSmartTags( 0, false );
        SetSmartTagDirty( true );

        SwIndex aIdx( this );
        CutText( pNode, aIdx, nSplitPos );

        
        
        if ( HasHints() )
        {
            for ( sal_uInt16 j = m_pSwpHints->Count(); j; )
            {
                SwTxtAttr* const pHt = m_pSwpHints->GetTextHint( --j );
                const sal_Int32* const pEnd = pHt->GetEnd();
                if ( pHt->DontExpand() && pEnd && (*pHt->GetStart() == *pEnd) )
                {
                    
                    m_pSwpHints->DeleteAtPos( j );
                    DestroyAttr( pHt );
                }
            }
            MoveTxtAttr_To_AttrSet();
        }

        if( pList )
        {
            pNode->SetWrong( pList->SplitList( nSplitPos ) );
            SetWrong( pList, false );
        }

        if( pList3 )
        {
            pNode->SetGrammarCheck( pList3->SplitGrammarList( nSplitPos ) );
            SetGrammarCheck( pList3, false );
        }

        
        if( pList2 )
        {
            pNode->SetSmartTags( pList2->SplitList( nSplitPos ) );
            SetSmartTags( pList2, false );
        }

        if ( GetDepends() )
        {
            MakeFrms( *pNode );     
        }
        lcl_ChangeFtnRef( *pNode );
    }

    {
        
        
        
        
        const SfxPoolItem *pItem;
        if( GetDepends() && SFX_ITEM_SET == pNode->GetSwAttrSet().
            GetItemState( RES_PAGEDESC, true, &pItem ) )
        {
            pNode->ModifyNotification( (SfxPoolItem*)pItem, (SfxPoolItem*)pItem );
        }
    }
    return pNode;
}

void SwTxtNode::MoveTxtAttr_To_AttrSet()
{
    OSL_ENSURE( m_pSwpHints, "MoveTxtAttr_To_AttrSet without SwpHints?" );
    for ( sal_uInt16 i = 0; m_pSwpHints && i < m_pSwpHints->Count(); ++i )
    {
        SwTxtAttr *pHt = m_pSwpHints->GetTextHint(i);

        if( *pHt->GetStart() )
            break;

        const sal_Int32* pHtEndIdx = pHt->GetEnd();

        if( !pHtEndIdx )
            continue;

        if (*pHtEndIdx < m_Text.getLength() || pHt->IsCharFmtAttr())
            break;

        if( !pHt->IsDontMoveAttr() &&
            SetAttr( pHt->GetAttr() ) )
        {
            m_pSwpHints->DeleteAtPos(i);
            DestroyAttr( pHt );
            --i;
        }
    }

}

SwCntntNode *SwTxtNode::JoinNext()
{
    SwNodes& rNds = GetNodes();
    SwNodeIndex aIdx( *this );
    if( SwCntntNode::CanJoinNext( &aIdx ) )
    {
        SwDoc* pDoc = rNds.GetDoc();
        std::vector<sal_uLong> aBkmkArr;
        _SaveCntntIdx( pDoc, aIdx.GetIndex(), SAL_MAX_INT32, aBkmkArr, SAVEFLY );
        SwTxtNode *pTxtNode = aIdx.GetNode().GetTxtNode();
        sal_Int32 nOldLen = m_Text.getLength();

        
        this->JoinMetadatable(*pTxtNode, !this->Len(), !pTxtNode->Len());

        SwWrongList *pList = GetWrong();
        if( pList )
        {
            pList->JoinList( pTxtNode->GetWrong(), nOldLen );
            SetWrongDirty( true );
            SetWrong( 0, false );
        }
        else
        {
            pList = pTxtNode->GetWrong();
            if( pList )
            {
                pList->Move( 0, nOldLen );
                SetWrongDirty( true );
                pTxtNode->SetWrong( 0, false );
            }
        }

        SwGrammarMarkUp *pList3 = GetGrammarCheck();
        if( pList3 )
        {
            pList3->JoinGrammarList( pTxtNode->GetGrammarCheck(), nOldLen );
            SetGrammarCheckDirty( true );
            SetGrammarCheck( 0, false );
        }
        else
        {
            pList3 = pTxtNode->GetGrammarCheck();
            if( pList3 )
            {
                pList3->MoveGrammar( 0, nOldLen );
                SetGrammarCheckDirty( true );
                pTxtNode->SetGrammarCheck( 0, false );
            }
        }

        
        SwWrongList *pList2 = GetSmartTags();
        if( pList2 )
        {
            pList2->JoinList( pTxtNode->GetSmartTags(), nOldLen );
            SetSmartTagDirty( true );
            SetSmartTags( 0, false );
        }
        else
        {
            pList2 = pTxtNode->GetSmartTags();
            if( pList2 )
            {
                pList2->Move( 0, nOldLen );
                SetSmartTagDirty( true );
                pTxtNode->SetSmartTags( 0, false );
            }
        }

        { 
            pTxtNode->CutText( this, SwIndex(pTxtNode), pTxtNode->Len() );
        }
        
        if( !aBkmkArr.empty() )
            _RestoreCntntIdx( pDoc, aBkmkArr, GetIndex(), nOldLen );

        if( pTxtNode->HasAnyIndex() )
        {
            
            pDoc->CorrAbs( aIdx, SwPosition( *this ), nOldLen, sal_True );
        }
        rNds.Delete(aIdx);
        SetWrong( pList, false );
        SetGrammarCheck( pList3, false );
        SetSmartTags( pList2, false ); 
        InvalidateNumRule();
    }
    else {
        OSL_FAIL( "kein TxtNode." );
    }

    return this;
}

SwCntntNode *SwTxtNode::JoinPrev()
{
    SwNodes& rNds = GetNodes();
    SwNodeIndex aIdx( *this );
    if( SwCntntNode::CanJoinPrev( &aIdx ) )
    {
        SwDoc* pDoc = rNds.GetDoc();
        std::vector<sal_uLong> aBkmkArr;
        _SaveCntntIdx( pDoc, aIdx.GetIndex(), SAL_MAX_INT32, aBkmkArr, SAVEFLY );
        SwTxtNode *pTxtNode = aIdx.GetNode().GetTxtNode();
        const sal_Int32 nLen = pTxtNode->Len();

        SwWrongList *pList = pTxtNode->GetWrong();
        if( pList )
        {
            pList->JoinList( GetWrong(), Len() );
            SetWrongDirty( true );
            pTxtNode->SetWrong( 0, false );
            SetWrong( NULL );
        }
        else
        {
            pList = GetWrong();
            if( pList )
            {
                pList->Move( 0, nLen );
                SetWrongDirty( true );
                SetWrong( 0, false );
            }
        }

        SwGrammarMarkUp *pList3 = pTxtNode->GetGrammarCheck();
        if( pList3 )
        {
            pList3->JoinGrammarList( GetGrammarCheck(), Len() );
            SetGrammarCheckDirty( true );
            pTxtNode->SetGrammarCheck( 0, false );
            SetGrammarCheck( NULL );
        }
        else
        {
            pList3 = GetGrammarCheck();
            if( pList3 )
            {
                pList3->MoveGrammar( 0, nLen );
                SetGrammarCheckDirty( true );
                SetGrammarCheck( 0, false );
            }
        }

        
        SwWrongList *pList2 = pTxtNode->GetSmartTags();
        if( pList2 )
        {
            pList2->JoinList( GetSmartTags(), Len() );
            SetSmartTagDirty( true );
            pTxtNode->SetSmartTags( 0, false );
            SetSmartTags( NULL );
        }
        else
        {
            pList2 = GetSmartTags();
            if( pList2 )
            {
                pList2->Move( 0, nLen );
                SetSmartTagDirty( true );
                SetSmartTags( 0, false );
            }
        }

        { 
            pTxtNode->CutText( this, SwIndex(this), SwIndex(pTxtNode), nLen );
        }
        
        if( !aBkmkArr.empty() )
            _RestoreCntntIdx( pDoc, aBkmkArr, GetIndex() );

        if( pTxtNode->HasAnyIndex() )
        {
            
            pDoc->CorrAbs( aIdx, SwPosition( *this ), nLen, sal_True );
        }
        rNds.Delete(aIdx);
        SetWrong( pList, false );
        SetGrammarCheck( pList3, false );
        SetSmartTags( pList2, false );
        InvalidateNumRule();
    }
    else {
        OSL_FAIL( "kein TxtNode." );
    }

    return this;
}


void SwTxtNode::NewAttrSet( SwAttrPool& rPool )
{
    OSL_ENSURE( !mpAttrSet.get(), "AttrSet ist doch gesetzt" );
    SwAttrSet aNewAttrSet( rPool, aTxtNodeSetRange );

    
    const SwFmtColl* pAnyFmtColl = &GetAnyFmtColl();
    const SwFmtColl* pFmtColl = GetFmtColl();
    OUString sVal;
    SwStyleNameMapper::FillProgName( pAnyFmtColl->GetName(), sVal, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, true );
    SfxStringItem aAnyFmtColl( RES_FRMATR_STYLE_NAME, sVal );
    if ( pFmtColl != pAnyFmtColl )
        SwStyleNameMapper::FillProgName( pFmtColl->GetName(), sVal, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, true );
    SfxStringItem aFmtColl( RES_FRMATR_CONDITIONAL_STYLE_NAME, sVal );
    aNewAttrSet.Put( aAnyFmtColl );
    aNewAttrSet.Put( aFmtColl );

    aNewAttrSet.SetParent( &pAnyFmtColl->GetAttrSet() );
    mpAttrSet = GetDoc()->GetIStyleAccess().getAutomaticStyle( aNewAttrSet, IStyleAccess::AUTO_STYLE_PARA );
}



void SwTxtNode::Update(
    SwIndex const & rPos,
    const sal_Int32 nChangeLen,
    const bool bNegative,
    const bool bDelete )
{
    SetAutoCompleteWordDirty( true );

    ::std::auto_ptr<SwpHts> pCollector;
    const sal_Int32 nChangePos = rPos.GetIndex();

    if ( HasHints() )
    {
        if ( bNegative )
        {
            const sal_Int32 nChangeEnd = nChangePos + nChangeLen;
            for ( sal_uInt16 n = 0; n < m_pSwpHints->Count(); ++n )
            {
                bool bTxtAttrChanged = false;
                bool bStartOfTxtAttrChanged = false;
                SwTxtAttr * const pHint = m_pSwpHints->GetTextHint(n);
                sal_Int32 * const pStart = pHint->GetStart();
                if ( *pStart > nChangePos )
                {
                    if ( *pStart > nChangeEnd )
                    {
                         *pStart = *pStart - nChangeLen;
                    }
                    else
                    {
                         *pStart = nChangePos;
                    }
                    bStartOfTxtAttrChanged = true;
                }

                sal_Int32 * const pEnd = pHint->GetEnd();
                if (pEnd)
                {
                    if ( *pEnd > nChangePos )
                    {
                        if( *pEnd > nChangeEnd )
                        {
                            *pEnd = *pEnd - nChangeLen;
                        }
                        else
                        {
                            *pEnd = nChangePos;
                        }
                        bTxtAttrChanged = !bStartOfTxtAttrChanged;
                    }
                }

                if ( bTxtAttrChanged
                     && pHint->Which() == RES_TXTATR_INPUTFIELD )
                {
                    SwTxtInputFld* pTxtInputFld = dynamic_cast<SwTxtInputFld*>(pHint);
                    if ( pTxtInputFld )
                    {
                        pTxtInputFld->UpdateFieldContent();
                    }
                }
            }

            m_pSwpHints->MergePortions( *this );
        }
        else
        {
            bool bNoExp = false;
            bool bResort = false;
            bool bMergePortionsNeeded = false;
            const sal_uInt16 coArrSz =
                static_cast<sal_uInt16>(RES_TXTATR_WITHEND_END) - static_cast<sal_uInt16>(RES_CHRATR_BEGIN);

            sal_Bool aDontExp[ coArrSz ];
            memset( &aDontExp, 0, coArrSz * sizeof(sal_Bool) );

            for ( sal_uInt16 n = 0; n < m_pSwpHints->Count(); ++n )
            {
                bool bTxtAttrChanged = false;
                SwTxtAttr * const pHint = m_pSwpHints->GetTextHint(n);
                sal_Int32 * const pStart = pHint->GetStart();
                sal_Int32 * const pEnd = pHint->GetEnd();
                if ( *pStart >= nChangePos )
                {
                    *pStart = *pStart + nChangeLen;
                    if ( pEnd )
                    {
                        *pEnd = *pEnd + nChangeLen;
                    }
                }
                else if ( pEnd && (*pEnd >= nChangePos) )
                {
                    if ( (*pEnd > nChangePos) || IsIgnoreDontExpand() )
                    {
                        *pEnd = *pEnd + nChangeLen;
                        bTxtAttrChanged = true;
                    }
                    else 
                    {
                        sal_uInt16 nWhPos;
                        const sal_uInt16 nWhich = pHint->Which();

                        OSL_ENSURE(!isCHRATR(nWhich), "Update: char attr hint?");
                        if (isCHRATR(nWhich) || isTXTATR_WITHEND(nWhich))
                        {
                            nWhPos = static_cast<sal_uInt16>(nWhich -
                                        RES_CHRATR_BEGIN);
                        }
                        else
                            continue;

                        if( aDontExp[ nWhPos ] )
                            continue;

                        if ( pHint->DontExpand() )
                        {
                            pHint->SetDontExpand( false );
                            bResort = true;
                            
                            if (pHint->IsFormatIgnoreEnd())
                            {
                                bMergePortionsNeeded = true;
                            }
                            if ( pHint->IsCharFmtAttr() )
                            {
                                bNoExp = true;
                                aDontExp[ static_cast<sal_uInt16>(RES_TXTATR_CHARFMT) - static_cast<sal_uInt16>(RES_CHRATR_BEGIN) ]
                                    = sal_True;
                                aDontExp[ static_cast<sal_uInt16>(RES_TXTATR_INETFMT) - static_cast<sal_uInt16>(RES_CHRATR_BEGIN) ]
                                    = sal_True;
                            }
                            else
                                aDontExp[ nWhPos ] = sal_True;
                        }
                        else if( bNoExp )
                        {
                            if ( !pCollector.get() )
                            {
                               pCollector.reset( new SwpHts );
                            }
                            for(SwpHts::iterator it =  pCollector->begin(); it != pCollector->end(); ++it)
                            {
                               SwTxtAttr *pTmp = *it;
                               if( nWhich == pTmp->Which() )
                               {
                                   pCollector->erase( it );
                                   SwTxtAttr::Destroy( pTmp,
                                       GetDoc()->GetAttrPool() );
                                   break;
                               }
                            }
                            SwTxtAttr * const pTmp =
                            MakeTxtAttr( *GetDoc(),
                                pHint->GetAttr(), nChangePos, nChangePos + nChangeLen);
                            pCollector->push_back( pTmp );
                        }
                        else
                        {
                            *pEnd = *pEnd + nChangeLen;
                            bTxtAttrChanged = true;
                        }
                    }
                }

                if ( bTxtAttrChanged
                     && pHint->Which() == RES_TXTATR_INPUTFIELD )
                {
                    SwTxtInputFld* pTxtInputFld = dynamic_cast<SwTxtInputFld*>(pHint);
                    if ( pTxtInputFld )
                    {
                        pTxtInputFld->UpdateFieldContent();
                    }
                }
            }
            if (bMergePortionsNeeded)
            {
                m_pSwpHints->MergePortions(*this); 
            }
            else if (bResort)
            {
                m_pSwpHints->Resort();
            }
        }
    }

    SwIndexReg aTmpIdxReg;
    if ( !bNegative && !bDelete )
    {
        const SwRedlineTbl& rTbl = GetDoc()->GetRedlineTbl();
        for ( sal_uInt16 i = 0; i < rTbl.size(); ++i )
        {
            SwRangeRedline *const pRedl = rTbl[ i ];
            if ( pRedl->HasMark() )
            {
                SwPosition* const pEnd = pRedl->End();
                if ( this == &pEnd->nNode.GetNode() &&
                     *pRedl->GetPoint() != *pRedl->GetMark() )
                {
                    SwIndex & rIdx = pEnd->nContent;
                    if (nChangePos == rIdx.GetIndex())
                    {
                        rIdx.Assign( &aTmpIdxReg, rIdx.GetIndex() );
                    }
                }
            }
            else if ( this == &pRedl->GetPoint()->nNode.GetNode() )
            {
                SwIndex & rIdx = pRedl->GetPoint()->nContent;
                if (nChangePos == rIdx.GetIndex())
                {
                    rIdx.Assign( &aTmpIdxReg, rIdx.GetIndex() );
                    
                    SwIndex * pIdx;
                    if ( &pRedl->GetBound( true ) == pRedl->GetPoint() )
                    {
                        pRedl->GetBound( false ) = pRedl->GetBound( true );
                        pIdx = &pRedl->GetBound( false ).nContent;
                    }
                    else
                    {
                        pRedl->GetBound( true ) = pRedl->GetBound( false );
                        pIdx = &pRedl->GetBound( true ).nContent;
                    }
                    pIdx->Assign( &aTmpIdxReg, pIdx->GetIndex() );
                }
            }
        }

        
        
        {
            const IDocumentMarkAccess* const pMarkAccess = getIDocumentMarkAccess();
            for ( IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getAllMarksBegin();
                ppMark != pMarkAccess->getAllMarksEnd();
                ++ppMark )
            {
                const ::sw::mark::IMark* const pMark = ppMark->get();
                const SwPosition* pEnd = &pMark->GetMarkEnd();
                SwIndex & rIdx = const_cast<SwIndex&>(pEnd->nContent);
                if( this == &pEnd->nNode.GetNode() &&
                    rPos.GetIndex() == rIdx.GetIndex() )
                {
                    rIdx.Assign( &aTmpIdxReg, rIdx.GetIndex() );
                }
            }
        }
    }

    
    SwIndexReg::Update( rPos, nChangeLen, bNegative, bDelete );

    if ( pCollector.get() )
    {
        const sal_uInt16 nCount = pCollector->size();
        for ( sal_uInt16 i = 0; i < nCount; ++i )
        {
            m_pSwpHints->TryInsertHint( (*pCollector)[ i ], *this );
        }
    }

    aTmpIdxReg.MoveTo( *this );
}

void SwTxtNode::_ChgTxtCollUpdateNum( const SwTxtFmtColl *pOldColl,
                                        const SwTxtFmtColl *pNewColl)
{
    SwDoc* pDoc = GetDoc();
    OSL_ENSURE( pDoc, "Kein Doc?" );
    
    
    const int nOldLevel = pOldColl && pOldColl->IsAssignedToListLevelOfOutlineStyle() ?
                     pOldColl->GetAssignedOutlineStyleLevel() : MAXLEVEL;
    const int nNewLevel = pNewColl && pNewColl->IsAssignedToListLevelOfOutlineStyle() ?
                     pNewColl->GetAssignedOutlineStyleLevel() : MAXLEVEL;

    if ( MAXLEVEL != nNewLevel && -1 != nNewLevel )
    {
        SetAttrListLevel(nNewLevel);
    }
    if (pDoc)
    {
        pDoc->GetNodes().UpdateOutlineNode(*this);
    }


    SwNodes& rNds = GetNodes();
    
    if( ( !nNewLevel || !nOldLevel) && !pDoc->GetFtnIdxs().empty() &&
        FTNNUM_CHAPTER == pDoc->GetFtnInfo().eNum &&
        rNds.IsDocNodes() )
    {
        SwNodeIndex aTmpIndex( rNds, GetIndex());

        pDoc->GetFtnIdxs().UpdateFtn( aTmpIndex);
    }

    if( RES_CONDTXTFMTCOLL == pNewColl->Which() )
    {
        
        ChkCondColl();
    }
}




bool SwTxtNode::DontExpandFmt( const SwIndex& rIdx, bool bFlag,
                                bool bFmtToTxtAttributes )
{
    const sal_Int32 nIdx = rIdx.GetIndex();
    if (bFmtToTxtAttributes && nIdx == m_Text.getLength())
    {
        FmtToTxtAttr( this );
    }

    bool bRet = false;
    if ( HasHints() )
    {
        const sal_uInt16 nEndCnt = m_pSwpHints->GetEndCount();
        sal_uInt16 nPos = nEndCnt;
        while( nPos )
        {
            SwTxtAttr *pTmp = m_pSwpHints->GetEnd( --nPos );
            sal_Int32 *pEnd = pTmp->GetEnd();
            if( !pEnd || *pEnd > nIdx )
                continue;
            if( nIdx != *pEnd )
                nPos = 0;
            else if( bFlag != pTmp->DontExpand() && !pTmp->IsLockExpandFlag()
                     && *pEnd > *pTmp->GetStart())
            {
                bRet = true;
                m_pSwpHints->NoteInHistory( pTmp );
                pTmp->SetDontExpand( bFlag );
            }
        }
    }
    return bRet;
}

static bool lcl_GetTxtAttrDefault(sal_Int32 nIndex, sal_Int32 nHintStart, sal_Int32 nHintEnd)
{
    return ((nHintStart <= nIndex) && (nIndex <  nHintEnd));
}
static bool lcl_GetTxtAttrExpand(sal_Int32 nIndex, sal_Int32 nHintStart, sal_Int32 nHintEnd)
{
    return ((nHintStart <  nIndex) && (nIndex <= nHintEnd));
}
static bool lcl_GetTxtAttrParent(sal_Int32 nIndex, sal_Int32 nHintStart, sal_Int32 nHintEnd)
{
    return ((nHintStart <  nIndex) && (nIndex <  nHintEnd));
}

static void
lcl_GetTxtAttrs(
    ::std::vector<SwTxtAttr *> *const pVector,
    SwTxtAttr **const ppTxtAttr,
    SwpHints *const pSwpHints,
    sal_Int32 const nIndex, RES_TXTATR const nWhich,
    enum SwTxtNode::GetTxtAttrMode const eMode)
{
    sal_uInt16 const nSize = (pSwpHints) ? pSwpHints->Count() : 0;
    sal_Int32 nPreviousIndex(0); 
    bool (*pMatchFunc)(sal_Int32, sal_Int32, sal_Int32)=0;
    switch (eMode)
    {
        case SwTxtNode::DEFAULT:   pMatchFunc = &lcl_GetTxtAttrDefault; break;
        case SwTxtNode::EXPAND:    pMatchFunc = &lcl_GetTxtAttrExpand;  break;
        case SwTxtNode::PARENT:    pMatchFunc = &lcl_GetTxtAttrParent;  break;
        default: assert(false);
    }

    for( sal_uInt16 i = 0; i < nSize; ++i )
    {
        SwTxtAttr *const pHint = pSwpHints->GetTextHint(i);
        sal_Int32 const nHintStart( *(pHint->GetStart()) );
        if (nIndex < nHintStart)
        {
            return; 
        }

        if (pHint->Which() != nWhich)
        {
            continue;
        }

        sal_Int32 const*const pEndIdx = pHint->GetEnd();
        
        assert(pEndIdx || pHint->HasDummyChar());
            
            
        bool const bContained( (pEndIdx)
            ? (*pMatchFunc)(nIndex, nHintStart, *pEndIdx)
            : (nHintStart == nIndex) );
        if (bContained)
        {
            if (pVector)
            {
                if (nPreviousIndex < nHintStart)
                {
                    pVector->clear(); 
                    nPreviousIndex = nHintStart;
                }
                pVector->push_back(pHint);
            }
            else
            {
                *ppTxtAttr = pHint; 
            }
            if (!pEndIdx)
            {
                break;
            }
        }
    }
}

::std::vector<SwTxtAttr *>
SwTxtNode::GetTxtAttrsAt(sal_Int32 const nIndex, RES_TXTATR const nWhich,
                        enum GetTxtAttrMode const eMode) const
{
    ::std::vector<SwTxtAttr *> ret;
    lcl_GetTxtAttrs(& ret, 0, m_pSwpHints, nIndex, nWhich, eMode);
    return ret;
}

SwTxtAttr *
SwTxtNode::GetTxtAttrAt(sal_Int32 const nIndex, RES_TXTATR const nWhich,
                        enum GetTxtAttrMode const eMode) const
{
    assert(    (nWhich == RES_TXTATR_META)
            || (nWhich == RES_TXTATR_METAFIELD)
            || (nWhich == RES_TXTATR_AUTOFMT)
            || (nWhich == RES_TXTATR_INETFMT)
            || (nWhich == RES_TXTATR_CJK_RUBY)
            || (nWhich == RES_TXTATR_UNKNOWN_CONTAINER)
            || (nWhich == RES_TXTATR_INPUTFIELD ) );
            

    SwTxtAttr * pRet(0);
    lcl_GetTxtAttrs(0, & pRet, m_pSwpHints, nIndex, nWhich, eMode);
    return pRet;
}

const SwTxtInputFld* SwTxtNode::GetOverlappingInputFld( const SwTxtAttr& rTxtAttr ) const
{
    const SwTxtInputFld* pTxtInputFld = NULL;

    pTxtInputFld = dynamic_cast<const SwTxtInputFld*>(GetTxtAttrAt( *(rTxtAttr.GetStart()), RES_TXTATR_INPUTFIELD, PARENT ));

    if ( pTxtInputFld == NULL && rTxtAttr.End() != NULL )
    {
        pTxtInputFld = dynamic_cast<const SwTxtInputFld*>(GetTxtAttrAt( *(rTxtAttr.End()), RES_TXTATR_INPUTFIELD, PARENT ));
    }

    return pTxtInputFld;
}

SwTxtFld* SwTxtNode::GetFldTxtAttrAt(
    const sal_Int32 nIndex,
    const bool bIncludeInputFldAtStart ) const
{
    SwTxtFld* pTxtFld = NULL;

    pTxtFld = dynamic_cast<SwTxtFld*>(GetTxtAttrForCharAt( nIndex, RES_TXTATR_FIELD ));
    if ( pTxtFld == NULL )
    {
        pTxtFld = dynamic_cast<SwTxtFld*>(GetTxtAttrForCharAt( nIndex, RES_TXTATR_ANNOTATION ));
    }
    if ( pTxtFld == NULL )
    {
        pTxtFld =
            dynamic_cast<SwTxtFld*>( GetTxtAttrAt(
                nIndex,
                RES_TXTATR_INPUTFIELD,
                bIncludeInputFldAtStart ? DEFAULT : PARENT ));
    }

    return pTxtFld;
}


/*************************************************************************
 *                          CopyHint()
 *************************************************************************/

static SwCharFmt* lcl_FindCharFmt( const SwCharFmts* pCharFmts, const OUString& rName )
{
    if( !rName.isEmpty() )
    {
        SwCharFmt* pFmt;
        sal_uInt16 nArrLen = pCharFmts->size();
        for( sal_uInt16 i = 1; i < nArrLen; i++ )
        {
            pFmt = (*pCharFmts)[ i ];
            if( pFmt->GetName()==rName )
                return pFmt;
        }
    }
    return NULL;
}

void lcl_CopyHint(
    const sal_uInt16 nWhich,
    const SwTxtAttr * const pHt,
    SwTxtAttr *const pNewHt,
    SwDoc *const pOtherDoc,
    SwTxtNode *const pDest )
{
    assert(nWhich == pHt->Which()); 
    switch( nWhich )
    {
    
    case RES_TXTATR_FTN :
            assert(pDest); 
            static_cast<const SwTxtFtn*>(pHt)->CopyFtn( *static_cast<SwTxtFtn*>(pNewHt), *pDest);
            break;

    
    

    
    case RES_TXTATR_FIELD :
        {
            if( pOtherDoc != NULL )
            {
                static_cast<const SwTxtFld*>(pHt)->CopyTxtFld( static_cast<SwTxtFld*>(pNewHt) );
            }

            
            const SwFmtFld& rFld = pHt->GetFmtFld();
            if( RES_TABLEFLD == rFld.GetField()->GetTyp()->Which()
                && static_cast<const SwTblField*>(rFld.GetField())->IsIntrnlName())
            {
                
                const SwTableNode* const pDstTblNd =
                    static_cast<const SwTxtFld*>(pHt)->GetTxtNode().FindTableNode();
                if( pDstTblNd )
                {
                    SwTblField* const pTblFld =
                        const_cast<SwTblField*>(static_cast<const SwTblField*>(
                            pNewHt->GetFmtFld().GetField()));
                    pTblFld->PtrToBoxNm( &pDstTblNd->GetTable() );
                }
            }
        }
        break;

    case RES_TXTATR_INPUTFIELD :
    case RES_TXTATR_ANNOTATION :
        if( pOtherDoc != NULL )
        {
            static_cast<const SwTxtFld*>(pHt)->CopyTxtFld( static_cast<SwTxtFld*>(pNewHt) );
        }
        break;

    case RES_TXTATR_TOXMARK :
        if( pOtherDoc && pDest && pDest->GetpSwpHints()
            && USHRT_MAX != pDest->GetpSwpHints()->GetPos( pNewHt ) )
        {
            
            
            static_cast<SwTxtTOXMark*>(pNewHt)->CopyTOXMark( pOtherDoc );
        }
        break;

    case RES_TXTATR_CHARFMT :
        
        
        if( pDest && pDest->GetpSwpHints()
            && USHRT_MAX != pDest->GetpSwpHints()->GetPos( pNewHt ) )
        {
            SwCharFmt* pFmt =
                static_cast<SwCharFmt*>(pHt->GetCharFmt().GetCharFmt());

            if( pFmt && pOtherDoc )
            {
                pFmt = pOtherDoc->CopyCharFmt( *pFmt );
            }
            const_cast<SwFmtCharFmt&>( static_cast<const SwFmtCharFmt&>(
                pNewHt->GetCharFmt() ) ).SetCharFmt( pFmt );
        }
        break;
    case RES_TXTATR_INETFMT :
        {
            
            
            if( pOtherDoc && pDest && pDest->GetpSwpHints()
                && USHRT_MAX != pDest->GetpSwpHints()->GetPos( pNewHt ) )
            {
                const SwDoc* const pDoc = static_cast<const SwTxtINetFmt*>(pHt)
                    ->GetTxtNode().GetDoc();
                if ( pDoc )
                {
                    const SwCharFmts* pCharFmts = pDoc->GetCharFmts();
                    const SwFmtINetFmt& rFmt = pHt->GetINetFmt();
                    SwCharFmt* pFmt;
                    pFmt = lcl_FindCharFmt( pCharFmts, rFmt.GetINetFmt() );
                    if( pFmt )
                        pOtherDoc->CopyCharFmt( *pFmt );
                    pFmt = lcl_FindCharFmt( pCharFmts, rFmt.GetVisitedFmt() );
                    if( pFmt )
                        pOtherDoc->CopyCharFmt( *pFmt );
                }
            }
            
            
            
            SwTxtINetFmt* const pINetHt = static_cast<SwTxtINetFmt*>(pNewHt);
            if ( !pINetHt->GetpTxtNode() )
            {
                pINetHt->ChgTxtNode( pDest );
            }

            
            pINetHt->GetCharFmt();
            break;
        }
    case RES_TXTATR_META:
    case RES_TXTATR_METAFIELD:
        OSL_ENSURE( pNewHt, "copying Meta should not fail!" );
        OSL_ENSURE( pDest
                    && (CH_TXTATR_INWORD == pDest->GetTxt()[*pNewHt->GetStart()]),
            "missing CH_TXTATR?");
        break;
    }
}

/*************************************************************************
|*  SwTxtNode::CopyAttr()
|*  Beschreibung    kopiert Attribute an der Position nStart in pDest.
|*  BP 7.6.93:      Es werden mit Absicht nur die Attribute _mit_ EndIdx
|*                  kopiert! CopyAttr wird vornehmlich dann gerufen,
|*                  wenn Attribute fuer einen Node mit leerem String
|*                  gesetzt werden sollen.
*************************************************************************/

void SwTxtNode::CopyAttr( SwTxtNode *pDest, const sal_Int32 nTxtStartIdx,
                          const sal_Int32 nOldPos )
{
    if ( HasHints() )    
    {
        SwDoc* const pOtherDoc = (pDest->GetDoc() != GetDoc()) ?
                pDest->GetDoc() : 0;

        for ( sal_uInt16 i = 0; i < m_pSwpHints->Count(); i++ )
        {
            SwTxtAttr *const pHt = m_pSwpHints->GetTextHint(i);
            sal_Int32 const nAttrStartIdx = *pHt->GetStart();
            if ( nTxtStartIdx < nAttrStartIdx )
                break; 

            const sal_Int32 *const pEndIdx = pHt->GetEnd();
            if ( pEndIdx && !pHt->HasDummyChar() )
            {
                if ( ( *pEndIdx > nTxtStartIdx
                       || ( *pEndIdx == nTxtStartIdx
                            && nAttrStartIdx == nTxtStartIdx ) ) )
                {
                    sal_uInt16 const nWhich = pHt->Which();
                    if ( RES_TXTATR_REFMARK != nWhich )
                    {
                        
                        SwTxtAttr *const pNewHt =
                            pDest->InsertItem( pHt->GetAttr(), nOldPos, nOldPos, nsSetAttrMode::SETATTR_IS_COPY);
                        if ( pNewHt )
                        {
                            lcl_CopyHint( nWhich, pHt, pNewHt,
                                pOtherDoc, pDest );
                        }
                    }
                    else if( !pOtherDoc
                             ? GetDoc()->IsCopyIsMove()
                             : 0 == pOtherDoc->GetRefMark( pHt->GetRefMark().GetRefName() ) )
                    {
                        pDest->InsertItem(
                            pHt->GetAttr(), nOldPos, nOldPos, nsSetAttrMode::SETATTR_IS_COPY);
                    }
                }
            }
        }
    }

    if( this != pDest )
    {
        
        SwUpdateAttr aHint( nOldPos, nOldPos, 0 );
        pDest->ModifyNotification( 0, &aHint );
    }
}

/*************************************************************************
|*  SwTxtNode::Copy()
|*  Beschreibung        kopiert Zeichen und Attibute in pDest,
|*                      wird angehaengt
*************************************************************************/


void SwTxtNode::CopyText( SwTxtNode *const pDest,
                      const SwIndex &rStart,
                      const sal_Int32 nLen,
                      const bool bForceCopyOfAllAttrs )
{
    SwIndex const aIdx( pDest, pDest->m_Text.getLength() );
    CopyText( pDest, aIdx, rStart, nLen, bForceCopyOfAllAttrs );
}


void SwTxtNode::CopyText( SwTxtNode *const pDest,
                      const SwIndex &rDestStart,
                      const SwIndex &rStart,
                      sal_Int32 nLen,
                      const bool bForceCopyOfAllAttrs )
{
    sal_Int32 nTxtStartIdx = rStart.GetIndex();
    sal_Int32 nDestStart = rDestStart.GetIndex();      

    if (pDest->GetDoc()->IsClipBoard() && this->GetNum())
    {
        
        pDest->m_pNumStringCache.reset( (nTxtStartIdx != 0)
            ? new OUString 
            : new OUString(this->GetNumString()));
    }

    if( !nLen )
    {
        
        
        CopyAttr( pDest, nTxtStartIdx, nDestStart );

        
        if( HasSwAttrSet() )
        {
            
            
            if ( !bForceCopyOfAllAttrs &&
                 ( nDestStart ||
                   pDest->HasSwAttrSet() ||
                   nLen != pDest->GetTxt().getLength()))
            {
                SfxItemSet aCharSet( pDest->GetDoc()->GetAttrPool(),
                                    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
                                    RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
                                    RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
                                    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
                                    0 );
                aCharSet.Put( *GetpSwAttrSet() );
                if( aCharSet.Count() )
                {
                    pDest->SetAttr( aCharSet, nDestStart, nDestStart );
                }
            }
            else
            {
                GetpSwAttrSet()->CopyToModify( *pDest );
            }
        }
        return;
    }

    
    const sal_Int32 oldLen = pDest->m_Text.getLength();
    
    
    
    pDest->InsertText( m_Text.copy(nTxtStartIdx, nLen), rDestStart,
                   IDocumentContentOperations::INS_EMPTYEXPAND );

    
    nLen = pDest->m_Text.getLength() - oldLen;
    if ( !nLen ) 
        return;

    SwDoc* const pOtherDoc = (pDest->GetDoc() != GetDoc()) ? pDest->GetDoc() : 0;

    
    if( HasSwAttrSet() )
    {
        
        
        if ( !bForceCopyOfAllAttrs &&
             ( nDestStart ||
               pDest->HasSwAttrSet() ||
               nLen != pDest->GetTxt().getLength()))
        {
            SfxItemSet aCharSet( pDest->GetDoc()->GetAttrPool(),
                RES_CHRATR_BEGIN, RES_CHRATR_END-1,
                RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
                RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
                RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
                0 );
            aCharSet.Put( *GetpSwAttrSet() );
            if( aCharSet.Count() )
            {
                pDest->SetAttr( aCharSet, nDestStart, nDestStart + nLen );
            }
        }
        else
        {
            GetpSwAttrSet()->CopyToModify( *pDest );
        }
    }

    bool const bUndoNodes = !pOtherDoc
                            && GetDoc()->GetIDocumentUndoRedo().IsUndoNodes(GetNodes());

    
    
    nTxtStartIdx = rStart.GetIndex();
    const sal_Int32 nEnd = nTxtStartIdx + nLen;

    
    
    
    const sal_uInt16 nSize = m_pSwpHints ? m_pSwpHints->Count() : 0;

    
    
    
    SwpHts aArr;

    
    SwpHts aRefMrkArr;

    sal_Int32 nDeletedDummyChars(0);
    
    for (sal_uInt16 n = 0; ( n < nSize ); ++n)
    {
        const sal_Int32 nAttrStartIdx = *(*m_pSwpHints)[n]->GetStart();
        if (!( nAttrStartIdx < nEnd))
            break;

        SwTxtAttr * const pHt = m_pSwpHints->GetTextHint(n);
        const sal_Int32 * const pEndIdx = pHt->GetEnd();
        const sal_uInt16 nWhich = pHt->Which();

        
        
        
        
        
        
        const bool bCopyRefMark = RES_TXTATR_REFMARK == nWhich
                                  && ( bUndoNodes
                                       || ( !pOtherDoc
                                            ? GetDoc()->IsCopyIsMove()
                                            : 0 == pOtherDoc->GetRefMark( pHt->GetRefMark().GetRefName() ) ) );

        if ( pEndIdx
             && RES_TXTATR_REFMARK == nWhich
             && !bCopyRefMark )
        {
            continue;
        }

        
        if ( nWhich == RES_TXTATR_INPUTFIELD )
        {
            OSL_ENSURE( pEndIdx != NULL,
                    "<SwTxtNode::CopyText(..)> - RES_TXTATR_INPUTFIELD without EndIndex!" );
            if ( nAttrStartIdx < nTxtStartIdx
                 || ( pEndIdx != NULL
                      && *(pEndIdx) > nEnd ) )
            {
                continue;
            }
        }

        sal_Int32 nAttrStt = 0;
        sal_Int32 nAttrEnd = 0;

        if( nAttrStartIdx < nTxtStartIdx )
        {
            
            
            if ( pEndIdx && (*pEndIdx > nTxtStartIdx) && !pHt->HasDummyChar() )
            {
                
                nAttrStt = nDestStart;
                nAttrEnd = (*pEndIdx > nEnd)
                    ? rDestStart.GetIndex()
                    : nDestStart + (*pEndIdx) - nTxtStartIdx;
            }
            else
            {
                continue;
            }
        }
        else
        {
            
            nAttrStt = nDestStart + ( nAttrStartIdx - nTxtStartIdx );
            if( pEndIdx )
            {
                nAttrEnd = *pEndIdx > nEnd
                    ? rDestStart.GetIndex()
                    : nDestStart + ( *pEndIdx - nTxtStartIdx );
            }
            else
            {
                nAttrEnd = nAttrStt;
            }
        }

        SwTxtAttr * pNewHt = 0;

        if( pDest == this )
        {
            
            pNewHt = MakeTxtAttr( *GetDoc(), pHt->GetAttr(),
                    nAttrStt, nAttrEnd, COPY, pDest );

            lcl_CopyHint(nWhich, pHt, pNewHt, 0, pDest);
            aArr.push_back( pNewHt );
        }
        else
        {
            pNewHt = pDest->InsertItem(
                pHt->GetAttr(),
                nAttrStt - nDeletedDummyChars,
                nAttrEnd - nDeletedDummyChars,
                nsSetAttrMode::SETATTR_NOTXTATRCHR | nsSetAttrMode::SETATTR_IS_COPY);
            if (pNewHt)
            {
                lcl_CopyHint( nWhich, pHt, pNewHt, pOtherDoc, pDest );
            }
            else if (pHt->HasDummyChar())
            {
                
                
                
                ++nDeletedDummyChars;
            }
        }

        if( RES_TXTATR_REFMARK == nWhich && !pEndIdx && !bCopyRefMark )
        {
            aRefMrkArr.push_back( pNewHt );
        }
    }

    
    
    for ( sal_uInt16 i = 0; i < aArr.size(); ++i )
    {
        InsertHint( aArr[ i ], nsSetAttrMode::SETATTR_NOTXTATRCHR );
    }

    if( pDest->GetpSwpHints() )
    {
        for ( sal_uInt16 i = 0; i < aRefMrkArr.size(); ++i )
        {
            SwTxtAttr * const pNewHt = aRefMrkArr[i];
            if( pNewHt->GetEnd() )
            {
                pDest->GetpSwpHints()->Delete( pNewHt );
                pDest->DestroyAttr( pNewHt );
            }
            else
            {
                const SwIndex aIdx( pDest, *pNewHt->GetStart() );
                pDest->EraseText( aIdx, 1 );
            }
        }
    }

    CHECK_SWPHINTS(this);
}


OUString SwTxtNode::InsertText( const OUString & rStr, const SwIndex & rIdx,
        const IDocumentContentOperations::InsertFlags nMode )
{
    assert(rIdx <= m_Text.getLength()); 

    const sal_Int32 aPos = rIdx.GetIndex();
    sal_Int32 nLen = m_Text.getLength() - aPos;
    sal_Int32 const nOverflow(rStr.getLength() - GetSpaceLeft());
    SAL_WARN_IF(nOverflow > 0, "sw.core",
            "SwTxtNode::InsertText: node text with insertion > capacity.");
    OUString const sInserted(
        (nOverflow > 0) ? rStr.copy(0, rStr.getLength() - nOverflow) : rStr);
    if (sInserted.isEmpty())
    {
        return sInserted;
    }
    m_Text = m_Text.replaceAt(aPos, 0, sInserted);
    assert(GetSpaceLeft()>=0);
    nLen = m_Text.getLength() - aPos - nLen;
    assert(nLen != 0);

    bool bOldExpFlg = IsIgnoreDontExpand();
    if (nMode & IDocumentContentOperations::INS_FORCEHINTEXPAND)
    {
        SetIgnoreDontExpand( true );
    }

    Update( rIdx, nLen ); 

    if (nMode & IDocumentContentOperations::INS_FORCEHINTEXPAND)
    {
        SetIgnoreDontExpand( bOldExpFlg );
    }

    
    
    

    if ( HasHints() )
    {
        bool bMergePortionsNeeded(false);
        for ( sal_uInt16 i = 0; i < m_pSwpHints->Count() &&
                rIdx >= *(*m_pSwpHints)[i]->GetStart(); ++i )
        {
            SwTxtAttr * const pHt = m_pSwpHints->GetTextHint( i );
            sal_Int32 * const pEndIdx = pHt->GetEnd();
            if( !pEndIdx )
                continue;

            if( rIdx == *pEndIdx )
            {
                if (  (nMode & IDocumentContentOperations::INS_NOHINTEXPAND) ||
                    (!(nMode & IDocumentContentOperations::INS_FORCEHINTEXPAND)
                     && pHt->DontExpand()) )
                {
                    
                    if( rIdx == *pHt->GetStart() )
                        *pHt->GetStart() = *pHt->GetStart() - nLen;
                    *pEndIdx = *pEndIdx - nLen;
                    m_pSwpHints->DeleteAtPos(i);
                    
                    
                    
                    
                    if (pHt->IsFormatIgnoreEnd())
                    {
                        bMergePortionsNeeded = true;
                    }
                    InsertHint( pHt, nsSetAttrMode::SETATTR_NOHINTADJUST );
                }
                
                else if ( (nMode & IDocumentContentOperations::INS_EMPTYEXPAND)
                        && (*pEndIdx == *pHt->GetStart()) )
                {
                    *pHt->GetStart() = *pHt->GetStart() - nLen;
                    const sal_uInt16 nAktLen = m_pSwpHints->Count();
                    m_pSwpHints->DeleteAtPos(i);
                    InsertHint( pHt/* AUTOSTYLES:, nsSetAttrMode::SETATTR_NOHINTADJUST*/ );
                    if ( nAktLen > m_pSwpHints->Count() && i )
                    {
                        --i;
                    }
                    continue;
                }
                else
                {
                    continue;
                }
            }
            if ( !(nMode & IDocumentContentOperations::INS_NOHINTEXPAND) &&
                 rIdx == nLen && *pHt->GetStart() == rIdx.GetIndex() &&
                 !pHt->IsDontExpandStartAttr() )
            {
                
                m_pSwpHints->DeleteAtPos(i);
                *pHt->GetStart() = *pHt->GetStart() - nLen;
                
                InsertHint( pHt, nsSetAttrMode::SETATTR_NOHINTADJUST );
            }
        }
        if (bMergePortionsNeeded)
        {
            m_pSwpHints->MergePortions(*this);
        }
        TryDeleteSwpHints();
    }

    if ( GetDepends() )
    {
        SwInsTxt aHint( aPos, nLen );
        NotifyClients( 0, &aHint );
    }

    
    
    SetCalcHiddenCharFlags();

    CHECK_SWPHINTS(this);
    return sInserted;
}

/*************************************************************************
|*
|*  SwTxtNode::Cut()
|*
|*  Beschreibung        text.doc
|*
*************************************************************************/

void SwTxtNode::CutText( SwTxtNode * const pDest,
            const SwIndex & rStart, const sal_Int32 nLen )
{
    if(pDest)
    {
        SwIndex aDestStt(pDest, pDest->GetTxt().getLength());
        CutImpl( pDest, aDestStt, rStart, nLen, false );
    }
    else
    {
        OSL_FAIL("mst: entering dead and bitrotted code; fasten your seatbelts!");
        assert(false);
        EraseText( rStart, nLen );
    }
}


void SwTxtNode::CutImpl( SwTxtNode * const pDest, const SwIndex & rDestStart,
         const SwIndex & rStart, sal_Int32 nLen, const bool bUpdate )
{
    if(!pDest)
    {
        OSL_FAIL("mst: entering dead and bitrotted code; fasten your seatbelts!");
        assert(false);
        EraseText( rStart, nLen );
        return;
    }

    
    if( GetDoc() != pDest->GetDoc() )
    {
        OSL_FAIL("mst: entering dead and bitrotted code; fasten your seatbelts!");
        assert(false);
        CopyText( pDest, rDestStart, rStart, nLen);
        EraseText(rStart, nLen);
        return;
    }

    if( !nLen )
    {
        
        
        CopyAttr( pDest, rStart.GetIndex(), rDestStart.GetIndex() );
        return;
    }

    sal_Int32 nTxtStartIdx = rStart.GetIndex();
    sal_Int32 nDestStart = rDestStart.GetIndex();      
    const sal_Int32 nInitSize = pDest->m_Text.getLength();

    
    if( pDest == this )
    {
        OSL_FAIL("mst: entering dead and bitrotted code; fasten your seatbelts!");
        assert(false);
        OUStringBuffer buf(m_Text);
        buf.insert(nDestStart, m_Text.copy(nTxtStartIdx, nLen));
        buf.remove(
            nTxtStartIdx + ((nDestStart < nTxtStartIdx) ? nLen : 0), nLen);
        m_Text = buf.makeStringAndClear();

        const sal_Int32 nEnd = rStart.GetIndex() + nLen;

        
        
        
        SwpHts aArr;

        
        
        
        sal_uInt16 nAttrCnt = 0;
        while ( m_pSwpHints && nAttrCnt < m_pSwpHints->Count() )
        {
            SwTxtAttr * const pHt = m_pSwpHints->GetTextHint(nAttrCnt);
            const sal_Int32 nAttrStartIdx = *pHt->GetStart();
            if (!( nAttrStartIdx < nEnd ))
                break;
            const sal_Int32 * const pEndIdx = pHt->GetEnd();
            const sal_uInt16 nWhich = pHt->Which();
            SwTxtAttr *pNewHt = 0;

            if(nAttrStartIdx < nTxtStartIdx)
            {
                
                if ( RES_TXTATR_REFMARK != nWhich && !pHt->HasDummyChar() &&
                    pEndIdx && *pEndIdx > nTxtStartIdx )
                {
                    
                    
                    pNewHt = MakeTxtAttr( *GetDoc(), pHt->GetAttr(), 0,
                                        *pEndIdx > nEnd
                                            ? nLen
                                            : *pEndIdx - nTxtStartIdx );
                }
            }
            else
            {
                
                if( !pEndIdx || *pEndIdx < nEnd )
                {
                    
                    m_pSwpHints->Delete( pHt );
                    
                    *pHt->GetStart() = nAttrStartIdx - nTxtStartIdx;
                    if( pEndIdx )
                        *pHt->GetEnd() = *pEndIdx - nTxtStartIdx;
                    aArr.push_back( pHt );
                    continue;           
                }
                    
                else if (RES_TXTATR_REFMARK != nWhich && !pHt->HasDummyChar())
                {
                    pNewHt = MakeTxtAttr( *GetDoc(),
                                          pHt->GetAttr(),
                                          nAttrStartIdx - nTxtStartIdx,
                                          ( *pEndIdx > nEnd
                                            ? nLen
                                            : *pEndIdx - nTxtStartIdx ));
                }
            }
            if( pNewHt )
            {
                
                lcl_CopyHint( nWhich, pHt, pNewHt, 0, this );
                aArr.push_back( pNewHt );
            }
            ++nAttrCnt;
        }

        if( bUpdate )
        {
            
            Update( rDestStart, nLen, false, true );
        }

        CHECK_SWPHINTS(this);

        Update( rStart, nLen, true, true );

        CHECK_SWPHINTS(this);

        
        if( nDestStart <= nTxtStartIdx )
        {
            nTxtStartIdx = nTxtStartIdx + nLen;
        }
        else
        {
            nDestStart = nDestStart - nLen;
        }

        for ( sal_uInt16 n = 0; n < aArr.size(); ++n )
        {
            SwTxtAttr *const pNewHt = aArr[n];
            *pNewHt->GetStart() = nDestStart + *pNewHt->GetStart();
            sal_Int32 * const pEndIdx = pNewHt->GetEnd();
            if ( pEndIdx )
            {
                *pEndIdx = nDestStart + *pEndIdx;
            }
            InsertHint( pNewHt, nsSetAttrMode::SETATTR_NOTXTATRCHR );
        }
    }
    else
    {
        pDest->m_Text = pDest->m_Text.replaceAt(nDestStart, 0,
                            m_Text.copy(nTxtStartIdx, nLen));
        m_Text = m_Text.replaceAt(nTxtStartIdx, nLen, "");
        if (GetSpaceLeft()<0)
        {   
            
            abort();
        }
        nLen = pDest->m_Text.getLength() - nInitSize; 
        if( !nLen )                 
            return;

        if( bUpdate )
        {
            
            pDest->Update( rDestStart, nLen, false, true);
        }

        CHECK_SWPHINTS(pDest);

        const sal_Int32 nEnd = rStart.GetIndex() + nLen;
        SwDoc* const pOtherDoc = (pDest->GetDoc() != GetDoc())
            ? pDest->GetDoc() : 0;
        bool const bUndoNodes = !pOtherDoc
            && GetDoc()->GetIDocumentUndoRedo().IsUndoNodes(GetNodes());

        OSL_ENSURE(!pOtherDoc,
            "mst: entering dead and bitrotted code; fasten your seatbelts!");
        assert(!pOtherDoc);

        
        if( HasSwAttrSet() )
        {
            
            if( nInitSize || pDest->HasSwAttrSet() ||
                nLen != pDest->GetTxt().getLength())
            {
                SfxItemSet aCharSet( pDest->GetDoc()->GetAttrPool(),
                                    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
                                    RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
                                    RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
                                    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
                                    0 );
                aCharSet.Put( *GetpSwAttrSet() );
                if( aCharSet.Count() )
                    pDest->SetAttr( aCharSet, nDestStart, nDestStart + nLen );
            }
            else
            {
                GetpSwAttrSet()->CopyToModify( *pDest );
            }
        }

        
        
        
        bool bMergePortionsNeeded(false);
        sal_uInt16 nAttrCnt = 0;
        while ( m_pSwpHints && (nAttrCnt < m_pSwpHints->Count()) )
        {
            SwTxtAttr * const pHt = m_pSwpHints->GetTextHint(nAttrCnt);
            const sal_Int32 nAttrStartIdx = *pHt->GetStart();
            if (!( nAttrStartIdx < nEnd ))
                break;
            const sal_Int32 * const pEndIdx = pHt->GetEnd();
            const sal_uInt16 nWhich = pHt->Which();
            SwTxtAttr *pNewHt = 0;

            
            if(nAttrStartIdx < nTxtStartIdx)
            {
                
                if( !pHt->HasDummyChar() && ( RES_TXTATR_REFMARK != nWhich
                    || bUndoNodes ) && pEndIdx && *pEndIdx > nTxtStartIdx )
                {
                    
                    
                    pNewHt = MakeTxtAttr( *pDest->GetDoc(), pHt->GetAttr(),
                                    nDestStart,
                                    nDestStart + (
                                        *pEndIdx > nEnd
                                            ? nLen
                                            : *pEndIdx - nTxtStartIdx ) );
                }
            }
            else
            {
                
                if( !pEndIdx || *pEndIdx < nEnd ||
                    (!pOtherDoc && !bUndoNodes && RES_TXTATR_REFMARK == nWhich)
                    || pHt->HasDummyChar() )
                {
                    
                    if ( GetDoc()->GetDocShell() )
                    {
                        GetDoc()->GetDocShell()->Broadcast( SfxSimpleHint(SFX_HINT_USER04));
                    }
                    
                    m_pSwpHints->Delete( pHt );
                    
                    if (pHt->IsFormatIgnoreStart() || pHt->IsFormatIgnoreEnd())
                    {
                        bMergePortionsNeeded = true;
                    }
                    *pHt->GetStart() =
                            nDestStart + (nAttrStartIdx - nTxtStartIdx);
                    if( pEndIdx )
                    {
                        *pHt->GetEnd() = nDestStart + (
                                        *pEndIdx > nEnd
                                            ? nLen
                                            : *pEndIdx - nTxtStartIdx );
                    }
                    pDest->InsertHint( pHt,
                              nsSetAttrMode::SETATTR_NOTXTATRCHR
                            | nsSetAttrMode::SETATTR_DONTREPLACE );
                    if ( GetDoc()->GetDocShell() )
                    {
                        GetDoc()->GetDocShell()->Broadcast( SfxSimpleHint(SFX_HINT_USER04));
                    }
                    continue;           
                }
                    
                else if( RES_TXTATR_REFMARK != nWhich || bUndoNodes )
                {
                    pNewHt = MakeTxtAttr( *GetDoc(), pHt->GetAttr(),
                                          nDestStart + (nAttrStartIdx - nTxtStartIdx),
                                          nDestStart + ( *pEndIdx > nEnd
                                                         ? nLen
                                                         : *pEndIdx - nTxtStartIdx ));
                }
            }
            if ( pNewHt )
            {
                const bool bSuccess( pDest->InsertHint( pNewHt,
                              nsSetAttrMode::SETATTR_NOTXTATRCHR
                            | nsSetAttrMode::SETATTR_DONTREPLACE
                            | nsSetAttrMode::SETATTR_IS_COPY) );
                if (bSuccess)
                {
                    lcl_CopyHint( nWhich, pHt, pNewHt, pOtherDoc, pDest );
                }
            }
            ++nAttrCnt;
        }
        
        
        
        
        if( m_pSwpHints && nAttrCnt < m_pSwpHints->Count() )
        {
            SwpHts aArr;
            while ( nAttrCnt < m_pSwpHints->Count() )
            {
                SwTxtAttr * const pHt = m_pSwpHints->GetTextHint(nAttrCnt);
                if ( nEnd != *pHt->GetStart() )
                    break;
                const sal_Int32 * const pEndIdx = pHt->GetEnd();
                if ( pEndIdx && *pEndIdx == nEnd )
                {
                    aArr.push_back( pHt );
                    m_pSwpHints->Delete( pHt );
                }
                else
                {
                    ++nAttrCnt;
                }
            }
            Update( rStart, nLen, true, true );

            for ( sal_uInt16 n = 0; n < aArr.size(); ++n )
            {
                SwTxtAttr * const pHt = aArr[ n ];
                *pHt->GetStart() = *pHt->GetEnd() = rStart.GetIndex();
                InsertHint( pHt );
            }
        }
        else
        {
            Update( rStart, nLen, true, true );
        }

        if (bMergePortionsNeeded)
        {
            m_pSwpHints->MergePortions(*this);
        }

        CHECK_SWPHINTS(this);
    }

    TryDeleteSwpHints();

    
    SwInsTxt aInsHint( nDestStart, nLen );
    pDest->ModifyNotification( 0, &aInsHint );
    SwDelTxt aDelHint( nTxtStartIdx, nLen );
    ModifyNotification( 0, &aDelHint );
}


void SwTxtNode::EraseText(const SwIndex &rIdx, const sal_Int32 nCount,
        const IDocumentContentOperations::InsertFlags nMode )
{
    assert(rIdx <= m_Text.getLength()); 

    const sal_Int32 nStartIdx = rIdx.GetIndex();
    const sal_Int32 nCnt = (nCount==SAL_MAX_INT32)
                      ? m_Text.getLength() - nStartIdx : nCount;
    const sal_Int32 nEndIdx = nStartIdx + nCnt;
    m_Text = m_Text.replaceAt(nStartIdx, nCnt, "");

    /* GCAttr(); alle leeren weggwerfen ist zu brutal.
     * Es duerfen nur die wegggeworfen werden,
     * die im Bereich liegen und nicht am Ende des Bereiches liegen
     */

    for ( sal_uInt16 i = 0; m_pSwpHints && i < m_pSwpHints->Count(); ++i )
    {
        SwTxtAttr *pHt = m_pSwpHints->GetTextHint(i);

        const sal_Int32 nHintStart = *pHt->GetStart();

        if ( nHintStart < nStartIdx )
            continue;

        if ( nHintStart > nEndIdx )
            break; 

        const sal_Int32* pHtEndIdx = pHt->GetEnd();
        const sal_uInt16 nWhich = pHt->Which();

        if( !pHtEndIdx )
        {
                    
            assert(pHt->HasDummyChar());
            if (isTXTATR(nWhich) &&
                (nHintStart >= nStartIdx) && (nHintStart < nEndIdx))
            {
                m_pSwpHints->DeleteAtPos(i);
                DestroyAttr( pHt );
                --i;
            }
            continue;
        }

       assert(!( (nHintStart < nEndIdx) && (*pHtEndIdx > nEndIdx)
                    && pHt->HasDummyChar() )
                
                || ((nHintStart == nStartIdx) && (nHintStart + 1 == nEndIdx)));
                

        
        
        
        
        
        
        
        if (   (*pHtEndIdx < nEndIdx)
            || ( (*pHtEndIdx == nEndIdx)     &&
                 !(IDocumentContentOperations::INS_EMPTYEXPAND & nMode)  &&
                 (  (RES_TXTATR_TOXMARK == nWhich)  ||
                    (RES_TXTATR_REFMARK == nWhich)  ||
                    (RES_TXTATR_CJK_RUBY == nWhich) ||
                    (RES_TXTATR_INPUTFIELD == nWhich) ) )
            || ( (nHintStart < nEndIdx)     &&
                 pHt->HasDummyChar()        )
           )
        {
            m_pSwpHints->DeleteAtPos(i);
            DestroyAttr( pHt );
            --i;
        }
    }

    OSL_ENSURE(rIdx.GetIndex() == nStartIdx, "huh? start index has changed?");

    TryDeleteSwpHints();

    Update( rIdx, nCnt, true );

    if( 1 == nCnt )
    {
        SwDelChr aHint( nStartIdx );
        NotifyClients( 0, &aHint );
    }
    else
    {
        SwDelTxt aHint( nStartIdx, nCnt );
        NotifyClients( 0, &aHint );
    }

    OSL_ENSURE(rIdx.GetIndex() == nStartIdx, "huh? start index has changed?");

    
    
    SetCalcHiddenCharFlags();

    CHECK_SWPHINTS(this);
}

/***********************************************************************
#*  Class       :   SwTxtNode
#*  Methode     :   GCAttr
#*
#*  Beschreibung
#*                  text.doc
#***********************************************************************/

void SwTxtNode::GCAttr()
{
    if ( !HasHints() )
        return;

    bool   bChanged = false;
    sal_Int32 nMin = m_Text.getLength();
    sal_Int32 nMax = 0;
    const bool bAll = nMin != 0; 
                           

    for ( sal_uInt16 i = 0; m_pSwpHints && i < m_pSwpHints->Count(); ++i )
    {
        SwTxtAttr * const pHt = m_pSwpHints->GetTextHint(i);

        
        const sal_Int32 * const pEndIdx = pHt->GetEnd();
        if (pEndIdx && !pHt->HasDummyChar() && (*pEndIdx == *pHt->GetStart())
            && ( bAll || pHt->Which() == RES_TXTATR_INETFMT ) )
        {
            bChanged = true;
            nMin = std::min( nMin, *pHt->GetStart() );
            nMax = std::max( nMax, *pHt->GetEnd() );
            DestroyAttr( m_pSwpHints->Cut(i) );
            --i;
        }
        else
        {
            pHt->SetDontExpand( false );
        }
    }
    TryDeleteSwpHints();

    if(bChanged)
    {
        
        SwUpdateAttr aHint( nMin, nMax, 0 );
        NotifyClients( 0, &aHint );
        SwFmtChg aNew( GetTxtColl() );
        NotifyClients( 0, &aNew );
    }
}


SwNumRule* SwTxtNode::_GetNumRule(bool bInParent) const
{
    SwNumRule* pRet = 0;

    const SfxPoolItem* pItem = GetNoCondAttr( RES_PARATR_NUMRULE, bInParent );
    bool bNoNumRule = false;
    if ( pItem )
    {
        OUString sNumRuleName =
            static_cast<const SwNumRuleItem *>(pItem)->GetValue();
        if (!sNumRuleName.isEmpty())
        {
            pRet = GetDoc()->FindNumRulePtr( sNumRuleName );
        }
        else 
            bNoNumRule = true;
    }

    if ( !bNoNumRule )
    {
        if ( pRet && pRet == GetDoc()->GetOutlineNumRule() &&
             ( !HasSwAttrSet() ||
               SFX_ITEM_SET !=
                GetpSwAttrSet()->GetItemState( RES_PARATR_NUMRULE, false ) ) )
        {
            SwTxtFmtColl* pColl = GetTxtColl();
            if ( pColl )
            {
                const SwNumRuleItem& rDirectItem = pColl->GetNumRule( sal_False );
                if ( rDirectItem.GetValue().isEmpty() )
                {
                    pRet = 0L;
                }
            }
        }
    }

    return pRet;
}

SwNumRule* SwTxtNode::GetNumRule(bool bInParent) const
{
    return _GetNumRule(bInParent);
}

void SwTxtNode::NumRuleChgd()
{
    if ( IsInList() )
    {
        SwNumRule* pNumRule = GetNumRule();
        if ( pNumRule && pNumRule != GetNum()->GetNumRule() )
        {
            mpNodeNum->ChangeNumRule( *pNumRule );
        }
    }

    if( IsInCache() )
    {
        SwFrm::GetCache().Delete( this );
        SetInCache( sal_False );
    }
    SetInSwFntCache( sal_False );

    
    
    
    
    {
        SvxLRSpaceItem& rLR = (SvxLRSpaceItem&)GetSwAttrSet().GetLRSpace();
        NotifyClients( &rLR, &rLR );
    }

    SetWordCountDirty( true );
}


bool SwTxtNode::IsNumbered() const
{
    SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : 0L;
    return pRule && IsCountedInList();
}

bool SwTxtNode::HasMarkedLabel() const
{
    bool bResult = false;

    if ( IsInList() )
    {
        bResult =
            GetDoc()->getListByName( GetListId() )->IsListLevelMarked( GetActualListLevel() );
    }

    return bResult;
}


SwTxtNode* SwTxtNode::_MakeNewTxtNode( const SwNodeIndex& rPos, bool bNext,
                                       bool bChgFollow )
{
    /* hartes PageBreak/PageDesc/ColumnBreak aus AUTO-Set ignorieren */
    SwAttrSet* pNewAttrSet = 0;
    
    bool bClearHardSetNumRuleWhenFmtCollChanges( false );
    if( HasSwAttrSet() )
    {
        pNewAttrSet = new SwAttrSet( *GetpSwAttrSet() );
        const SfxItemSet* pTmpSet = GetpSwAttrSet();

        if( bNext )     
            pTmpSet = pNewAttrSet;

        
        bool bRemoveFromCache = false;
        std::vector<sal_uInt16> aClearWhichIds;
        if ( bNext )
            bRemoveFromCache = ( 0 != pNewAttrSet->ClearItem( RES_PAGEDESC ) );
        else
            aClearWhichIds.push_back( RES_PAGEDESC );

        if( SFX_ITEM_SET == pTmpSet->GetItemState( RES_BREAK, false ) )
        {
            if ( bNext )
                pNewAttrSet->ClearItem( RES_BREAK );
            else
                aClearWhichIds.push_back( RES_BREAK );
            bRemoveFromCache = true;
        }
        if( SFX_ITEM_SET == pTmpSet->GetItemState( RES_KEEP, false ) )
        {
            if ( bNext )
                pNewAttrSet->ClearItem( RES_KEEP );
            else
                aClearWhichIds.push_back( RES_KEEP );
            bRemoveFromCache = true;
        }
        if( SFX_ITEM_SET == pTmpSet->GetItemState( RES_PARATR_SPLIT, false ) )
        {
            if ( bNext )
                pNewAttrSet->ClearItem( RES_PARATR_SPLIT );
            else
                aClearWhichIds.push_back( RES_PARATR_SPLIT );
            bRemoveFromCache = true;
        }
        if(SFX_ITEM_SET == pTmpSet->GetItemState(RES_PARATR_NUMRULE, false))
        {
            SwNumRule * pRule = GetNumRule();

            if (pRule && IsOutline())
            {
                if ( bNext )
                    pNewAttrSet->ClearItem(RES_PARATR_NUMRULE);
                else
                {
                    
                    
                    
                    bClearHardSetNumRuleWhenFmtCollChanges = true;
                }
                bRemoveFromCache = true;
            }
        }

        if ( !aClearWhichIds.empty() )
            bRemoveFromCache = 0 != ClearItemsFromAttrSet( aClearWhichIds );

        if( !bNext && bRemoveFromCache && IsInCache() )
        {
            SwFrm::GetCache().Delete( this );
            SetInCache( sal_False );
        }
    }
    SwNodes& rNds = GetNodes();

    SwTxtFmtColl* pColl = GetTxtColl();

    SwTxtNode *pNode = new SwTxtNode( rPos, pColl, pNewAttrSet );

    delete pNewAttrSet;

    const SwNumRule* pRule = GetNumRule();
    if( pRule && pRule == pNode->GetNumRule() && rNds.IsDocNodes() ) 
    {
        
        
        
        if ( !bNext && !IsCountedInList() )
            SetCountedInList(true);
    }

    
    
    
    if( pColl != pNode->GetTxtColl() ||
        ( bChgFollow && pColl != GetTxtColl() ))
        return pNode;       

    pNode->_ChgTxtCollUpdateNum( 0, pColl ); 
    if( bNext || !bChgFollow )
        return pNode;

    SwTxtFmtColl *pNextColl = &pColl->GetNextTxtFmtColl();
    
    
    if (pNextColl != pColl)
    {
        
        if ( bClearHardSetNumRuleWhenFmtCollChanges )
        {
            std::vector<sal_uInt16> aClearWhichIds;
            aClearWhichIds.push_back( RES_PARATR_NUMRULE );
            if ( ClearItemsFromAttrSet( aClearWhichIds ) != 0 && IsInCache() )
            {
                SwFrm::GetCache().Delete( this );
                SetInCache( sal_False );
            }
        }
    }
    ChgFmtColl( pNextColl );

    return pNode;
}

SwCntntNode* SwTxtNode::AppendNode( const SwPosition & rPos )
{
    
    SwNodeIndex aIdx( rPos.nNode, 1 );
    SwTxtNode* pNew = _MakeNewTxtNode( aIdx, true );

    
    pNew->ResetAttr( RES_PARATR_LIST_ISRESTART );
    pNew->ResetAttr( RES_PARATR_LIST_RESTARTVALUE );
    pNew->ResetAttr( RES_PARATR_LIST_ISCOUNTED );
    if ( pNew->GetNumRule() == 0 )
    {
        pNew->ResetAttr( RES_PARATR_LIST_ID );
        pNew->ResetAttr( RES_PARATR_LIST_LEVEL );
    }

    if (!IsInList() && GetNumRule() && !GetListId().isEmpty())
    {
        AddToList();
    }

    if( GetDepends() )
        MakeFrms( *pNew );
    return pNew;
}

/*************************************************************************
 *                      SwTxtNode::GetTxtAttr
 *************************************************************************/

SwTxtAttr * SwTxtNode::GetTxtAttrForCharAt(
    const sal_Int32 nIndex,
    const RES_TXTATR nWhich ) const
{
    if ( HasHints() )
    {
        for ( sal_uInt16 i = 0; i < m_pSwpHints->Count(); ++i )
        {
            SwTxtAttr * const pHint = m_pSwpHints->GetTextHint(i);
            const sal_Int32 nStartPos = *pHint->GetStart();
            if ( nIndex < nStartPos )
            {
                return 0;
            }
            if ( (nIndex == nStartPos) && pHint->HasDummyChar() )
            {
                return ( RES_TXTATR_END == nWhich || nWhich == pHint->Which() )
                       ? pHint : 0;
            }
        }
    }
    return 0;
}


bool SwTxtNode::HasNumber() const
{
    bool bResult = false;

    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : 0L;
    if ( pRule )
    {
        int nLevel = GetActualListLevel();

        if (nLevel < 0)
            nLevel = 0;

        if (nLevel >= MAXLEVEL)
            nLevel = MAXLEVEL - 1;

        SwNumFmt aFmt(pRule->Get(static_cast<sal_uInt16>(nLevel)));

        
        bResult = aFmt.IsEnumeration() &&
            SVX_NUM_NUMBER_NONE != aFmt.GetNumberingType();
    }

    return bResult;
}

bool SwTxtNode::HasBullet() const
{
    bool bResult = false;

    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : 0L;
    if ( pRule )
    {
        int nLevel = GetActualListLevel();

        if (nLevel < 0)
            nLevel = 0;

        if (nLevel >= MAXLEVEL)
            nLevel = MAXLEVEL - 1;

        SwNumFmt aFmt(pRule->Get( static_cast<sal_uInt16>(nLevel) ));

        bResult = aFmt.IsItemize();
    }

    return bResult;
}




OUString SwTxtNode::GetNumString( const bool _bInclPrefixAndSuffixStrings,
        const unsigned int _nRestrictToThisLevel ) const
{
    if (GetDoc()->IsClipBoard() && m_pNumStringCache.get())
    {
        
        return *m_pNumStringCache;
    }
    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : 0L;
    if ( pRule &&
         IsCountedInList() )
    {
        int nLevel = GetActualListLevel();

        if (nLevel < 0)
            nLevel = 0;

        if (nLevel >= MAXLEVEL)
            nLevel = MAXLEVEL - 1;

        SvxNumberType const& rNumberType(
                pRule->Get( static_cast<sal_uInt16>(nLevel) ) );
        if (rNumberType.IsTxtFmt() ||
        //
            (style::NumberingType::NUMBER_NONE == rNumberType.GetNumberingType()))
        {
            return pRule->MakeNumString( GetNum()->GetNumberVector(),
                                     _bInclPrefixAndSuffixStrings ? sal_True : sal_False,
                                     sal_False,
                                     _nRestrictToThisLevel );
        }
    }

    return OUString();
}

long SwTxtNode::GetLeftMarginWithNum( bool bTxtLeft ) const
{
    long nRet = 0;
    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : 0L;
    if( pRule )
    {
        int nLevel = GetActualListLevel();

        if (nLevel < 0)
            nLevel = 0;

        if (nLevel >= MAXLEVEL)
            nLevel = MAXLEVEL - 1;

        const SwNumFmt& rFmt = pRule->Get(static_cast<sal_uInt16>(nLevel));

        if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
        {
            nRet = rFmt.GetAbsLSpace();

            if( !bTxtLeft )
            {
                if( 0 > rFmt.GetFirstLineOffset() &&
                    nRet > -rFmt.GetFirstLineOffset() )
                    nRet = nRet + rFmt.GetFirstLineOffset();
                else
                    nRet = 0;
            }

            if( pRule->IsAbsSpaces() )
                nRet = nRet - GetSwAttrSet().GetLRSpace().GetLeft();
        }
        else if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
        {
            if ( AreListLevelIndentsApplicable() )
            {
                nRet = rFmt.GetIndentAt();
                
                
                if ( !bTxtLeft &&
                     rFmt.GetFirstLineIndent() < 0 )
                {
                    nRet = nRet + rFmt.GetFirstLineIndent();
                }
            }
        }
    }

    return nRet;
}

bool SwTxtNode::GetFirstLineOfsWithNum( short& rFLOffset ) const
{
    
    rFLOffset = 0;

    
    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : 0L;
    if ( pRule )
    {
        if ( IsCountedInList() )
        {
            int nLevel = GetActualListLevel();

            if (nLevel < 0)
                nLevel = 0;

            if (nLevel >= MAXLEVEL)
                nLevel = MAXLEVEL - 1;

            const SwNumFmt& rFmt = pRule->Get(static_cast<sal_uInt16>(nLevel));
            if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                rFLOffset = rFmt.GetFirstLineOffset();

                if (!getIDocumentSettingAccess()->get(IDocumentSettingAccess::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING))
                {
                    SvxLRSpaceItem aItem = GetSwAttrSet().GetLRSpace();
                    rFLOffset = rFLOffset + aItem.GetTxtFirstLineOfst();
                }
            }
            else if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                if ( AreListLevelIndentsApplicable() )
                {
                    rFLOffset = static_cast<sal_uInt16>(rFmt.GetFirstLineIndent());
                }
                else if (!getIDocumentSettingAccess()->get(IDocumentSettingAccess::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING))
                {
                    SvxLRSpaceItem aItem = GetSwAttrSet().GetLRSpace();
                    rFLOffset = aItem.GetTxtFirstLineOfst();
                }
            }
        }

        return true;
    }

    rFLOffset = GetSwAttrSet().GetLRSpace().GetTxtFirstLineOfst();
    return false;
}

//
SwTwips SwTxtNode::GetAdditionalIndentForStartingNewList() const
{
    SwTwips nAdditionalIndent = 0;

    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : 0L;
    if ( pRule )
    {
        int nLevel = GetActualListLevel();

        if (nLevel < 0)
            nLevel = 0;

        if (nLevel >= MAXLEVEL)
            nLevel = MAXLEVEL - 1;

        const SwNumFmt& rFmt = pRule->Get(static_cast<sal_uInt16>(nLevel));
        if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
        {
            nAdditionalIndent = GetSwAttrSet().GetLRSpace().GetLeft();

            if (getIDocumentSettingAccess()->get(IDocumentSettingAccess::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING))
            {
                nAdditionalIndent = nAdditionalIndent -
                                    GetSwAttrSet().GetLRSpace().GetTxtFirstLineOfst();
            }
        }
        else if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
        {
            if ( AreListLevelIndentsApplicable() )
            {
                nAdditionalIndent = rFmt.GetIndentAt() + rFmt.GetFirstLineIndent();
            }
            else
            {
                nAdditionalIndent = GetSwAttrSet().GetLRSpace().GetLeft();
                if (getIDocumentSettingAccess()->get(IDocumentSettingAccess::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING))
                {
                    nAdditionalIndent = nAdditionalIndent -
                                        GetSwAttrSet().GetLRSpace().GetTxtFirstLineOfst();
                }
            }
        }
    }
    else
    {
        nAdditionalIndent = GetSwAttrSet().GetLRSpace().GetLeft();
    }

    return nAdditionalIndent;
}


void SwTxtNode::ClearLRSpaceItemDueToListLevelIndents( SvxLRSpaceItem& o_rLRSpaceItem ) const
{
    if ( AreListLevelIndentsApplicable() )
    {
        const SwNumRule* pRule = GetNumRule();
        if ( pRule && GetActualListLevel() >= 0 )
        {
            int nLevel = GetActualListLevel();

            if (nLevel < 0)
                nLevel = 0;

            if (nLevel >= MAXLEVEL)
                nLevel = MAXLEVEL - 1;

            const SwNumFmt& rFmt = pRule->Get(static_cast<sal_uInt16>(nLevel));
            if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                o_rLRSpaceItem = aLR;
            }
        }
    }
}


long SwTxtNode::GetLeftMarginForTabCalculation() const
{
    long nLeftMarginForTabCalc = 0;

    bool bLeftMarginForTabCalcSetToListLevelIndent( false );
    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : 0;
    if( pRule )
    {
        const SwNumFmt& rFmt = pRule->Get(static_cast<sal_uInt16>(GetActualListLevel()));
        if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
        {
            if ( AreListLevelIndentsApplicable() )
            {
                nLeftMarginForTabCalc = rFmt.GetIndentAt();
                bLeftMarginForTabCalcSetToListLevelIndent = true;
            }
        }
    }
    if ( !bLeftMarginForTabCalcSetToListLevelIndent )
    {
        nLeftMarginForTabCalc = GetSwAttrSet().GetLRSpace().GetTxtLeft();
    }

    return nLeftMarginForTabCalc;
}

static void Replace0xFF(
    SwTxtNode const& rNode,
    OUStringBuffer & rTxt,
    sal_Int32 & rTxtStt,
    sal_Int32 nEndPos,
    sal_Bool const bExpandFlds,
    sal_Bool const bExpandFtn = sal_True )
{
    if (rNode.GetpSwpHints())
    {
        sal_Unicode cSrchChr = CH_TXTATR_BREAKWORD;
        for( int nSrchIter = 0; 2 > nSrchIter; ++nSrchIter, cSrchChr = CH_TXTATR_INWORD )
        {
            sal_Int32 nPos = rTxt.indexOf(cSrchChr);
            while (-1 != nPos && nPos < nEndPos)
            {
                const SwTxtAttr* const pAttr =
                    rNode.GetTxtAttrForCharAt(rTxtStt + nPos);
                if( pAttr )
                {
                    switch( pAttr->Which() )
                    {
                    case RES_TXTATR_FIELD:
                    case RES_TXTATR_ANNOTATION:
                        rTxt.remove(nPos, 1);
                        if( bExpandFlds )
                        {
                            const OUString aExpand(
                                static_cast<SwTxtFld const*>(pAttr)->GetFmtFld().GetField()->ExpandField(true));
                            rTxt.insert(nPos, aExpand);
                            nPos = nPos + aExpand.getLength();
                            nEndPos = nEndPos + aExpand.getLength();
                            rTxtStt = rTxtStt - aExpand.getLength();
                        }
                        ++rTxtStt;
                        break;

                    case RES_TXTATR_FTN:
                        rTxt.remove(nPos, 1);
                        if( bExpandFlds && bExpandFtn )
                        {
                            const SwFmtFtn& rFtn = pAttr->GetFtn();
                            OUString sExpand;
                            if( !rFtn.GetNumStr().isEmpty() )
                                sExpand = rFtn.GetNumStr();
                            else if( rFtn.IsEndNote() )
                                sExpand = rNode.GetDoc()->GetEndNoteInfo().aFmt.
                                                GetNumStr( rFtn.GetNumber() );
                            else
                                sExpand = rNode.GetDoc()->GetFtnInfo().aFmt.
                                                GetNumStr( rFtn.GetNumber() );
                            rTxt.insert(nPos, sExpand);
                            nPos = nPos + sExpand.getLength();
                            nEndPos = nEndPos + sExpand.getLength();
                            rTxtStt = rTxtStt - sExpand.getLength();
                        }
                        ++rTxtStt;
                        break;

                    default:
                        rTxt.remove(nPos, 1);
                        ++rTxtStt;
                    }
                }
                else
                    ++nPos, ++nEndPos;
                nPos = rTxt.indexOf(cSrchChr, nPos);
            }
        }
    }
}

/*************************************************************************
 *                      SwTxtNode::GetExpandTxt
 * Expand fields
 *************************************************************************/

OUString SwTxtNode::GetExpandTxt(  const sal_Int32 nIdx,
                                   const sal_Int32 nLen,
                                   const bool bWithNum,
                                   const bool bAddSpaceAfterListLabelStr,
                                   const bool bWithSpacesForLevel,
                                   const bool bWithFtn ) const

{
    OUStringBuffer aTxt(
        (nLen == -1) ? GetTxt().copy(nIdx) : GetTxt().copy(nIdx, nLen));
    sal_Int32 nTxtStt = nIdx;
    Replace0xFF(*this, aTxt, nTxtStt, aTxt.getLength(), true, bWithFtn );

    
    comphelper::string::remove(aTxt, CH_TXT_ATR_INPUTFIELDSTART);
    comphelper::string::remove(aTxt, CH_TXT_ATR_INPUTFIELDEND);

    if( bWithNum )
    {
        if ( !GetNumString().isEmpty() )
        {
            if ( bAddSpaceAfterListLabelStr )
            {
                const sal_Unicode aSpace = ' ';
                aTxt.insert(0, aSpace);
            }
            aTxt.insert(0, GetNumString());
        }
    }

    if (bWithSpacesForLevel)
    {
        const sal_Unicode aSpace = ' ';
        for (int nLevel = GetActualListLevel(); nLevel > 0; --nLevel)
        {
            aTxt.insert(0, aSpace);
            aTxt.insert(0, aSpace);
        }
    }

    return aTxt.makeStringAndClear();
}

bool SwTxtNode::GetExpandTxt( SwTxtNode& rDestNd, const SwIndex* pDestIdx,
                        sal_Int32 nIdx, sal_Int32 nLen, bool bWithNum,
                        bool bWithFtn, bool bReplaceTabsWithSpaces ) const
{
    if( &rDestNd == this )
        return false;

    SwIndex aDestIdx(&rDestNd, rDestNd.GetTxt().getLength());
    if( pDestIdx )
        aDestIdx = *pDestIdx;
    const sal_Int32 nDestStt = aDestIdx.GetIndex();

    
    OUStringBuffer buf(GetTxt());
    if( bReplaceTabsWithSpaces )
        buf.replace('\t', ' ');

    
    const sal_Unicode cChar = CH_TXTATR_BREAKWORD;
    SwScriptInfo::MaskHiddenRanges(*this, buf, 0, buf.getLength(), cChar);

    buf.remove(0, nIdx);
    if (nLen != -1)
    {
        buf.truncate(nLen);
    }
    
    {
        comphelper::string::remove(buf, CH_TXT_ATR_INPUTFIELDSTART);
        comphelper::string::remove(buf, CH_TXT_ATR_INPUTFIELDEND);
    }
    rDestNd.InsertText(buf.makeStringAndClear(), aDestIdx);
    nLen = aDestIdx.GetIndex() - nDestStt;

    
    if ( HasHints() )
    {
        sal_Int32 nInsPos = nDestStt - nIdx;
        for ( sal_uInt16 i = 0; i < m_pSwpHints->Count(); i++ )
        {
            const SwTxtAttr* pHt = (*m_pSwpHints)[i];
            const sal_Int32 nAttrStartIdx = *pHt->GetStart();
            const sal_uInt16 nWhich = pHt->Which();
            if (nIdx + nLen <= nAttrStartIdx)
                break;      

            const sal_Int32 *pEndIdx = pHt->End();
            if( pEndIdx && *pEndIdx > nIdx &&
                ( RES_CHRATR_FONT == nWhich ||
                  RES_TXTATR_CHARFMT == nWhich ||
                  RES_TXTATR_AUTOFMT == nWhich ))
            {
                const SvxFontItem* const pFont =
                    static_cast<const SvxFontItem*>(
                        CharFmt::GetItem( *pHt, RES_CHRATR_FONT ));
                if ( pFont && RTL_TEXTENCODING_SYMBOL == pFont->GetCharSet() )
                {
                    
                    rDestNd.InsertItem( *const_cast<SvxFontItem*>(pFont),
                            nInsPos + nAttrStartIdx, nInsPos + *pEndIdx );
                }
            }
            else if ( pHt->HasDummyChar() && (nAttrStartIdx >= nIdx) )
            {
                aDestIdx = nInsPos + nAttrStartIdx;
                switch( nWhich )
                {
                case RES_TXTATR_FIELD:
                case RES_TXTATR_ANNOTATION:
                    {
                        OUString const aExpand(
                            static_cast<SwTxtFld const*>(pHt)->GetFmtFld().GetField()->ExpandField(true) );
                        if (!aExpand.isEmpty())
                        {
                            ++aDestIdx;     
                            OUString const ins(
                                rDestNd.InsertText( aExpand, aDestIdx));
                            SAL_INFO_IF(ins.getLength() != aExpand.getLength(),
                                    "sw.core", "GetExpandTxt lossage");
                            aDestIdx = nInsPos + nAttrStartIdx;
                            nInsPos += ins.getLength();
                        }
                        rDestNd.EraseText( aDestIdx, 1 );
                        --nInsPos;
                    }
                    break;

                case RES_TXTATR_FTN:
                    {
                        if ( bWithFtn )
                        {
                            const SwFmtFtn& rFtn = pHt->GetFtn();
                            OUString sExpand;
                            if( !rFtn.GetNumStr().isEmpty() )
                                sExpand = rFtn.GetNumStr();
                            else if( rFtn.IsEndNote() )
                                sExpand = GetDoc()->GetEndNoteInfo().aFmt.
                                GetNumStr( rFtn.GetNumber() );
                            else
                                sExpand = GetDoc()->GetFtnInfo().aFmt.
                                                GetNumStr( rFtn.GetNumber() );
                            if( !sExpand.isEmpty() )
                            {
                                ++aDestIdx;     
                                SvxEscapementItem aItem(
                                        SVX_ESCAPEMENT_SUPERSCRIPT );
                                rDestNd.InsertItem(
                                        aItem,
                                        aDestIdx.GetIndex(),
                                        aDestIdx.GetIndex() );
                                OUString const ins( rDestNd.InsertText(sExpand, aDestIdx, IDocumentContentOperations::INS_EMPTYEXPAND));
                                SAL_INFO_IF(ins.getLength() != sExpand.getLength(),
                                        "sw.core", "GetExpandTxt lossage");
                                aDestIdx = nInsPos + nAttrStartIdx;
                                nInsPos += ins.getLength();
                            }
                        }
                        rDestNd.EraseText( aDestIdx, 1 );
                        --nInsPos;
                    }
                    break;

                default:
                    rDestNd.EraseText( aDestIdx, 1 );
                    --nInsPos;
                }
            }
        }
    }

    if( bWithNum )
    {
        aDestIdx = nDestStt;
        rDestNd.InsertText( GetNumString(), aDestIdx );
    }

    aDestIdx = 0;
    sal_Int32 nStartDelete(-1);
    while (aDestIdx < rDestNd.GetTxt().getLength())
    {
        sal_Unicode const cur(rDestNd.GetTxt()[aDestIdx.GetIndex()]);
        if (   (cChar == cur) 
            || (CH_TXT_ATR_FIELDSTART  == cur) 
            || (CH_TXT_ATR_FIELDEND    == cur)
            || (CH_TXT_ATR_FORMELEMENT == cur))
        {
            if (-1 == nStartDelete)
            {
                nStartDelete = aDestIdx.GetIndex(); 
            }
            ++aDestIdx;
            if (aDestIdx < rDestNd.GetTxt().getLength())
            {
                continue;
            } 
        }
        else
        {
            if (-1 == nStartDelete)
            {
                ++aDestIdx;
                continue;
            } 
        }
        assert(-1 != nStartDelete); 
        rDestNd.EraseText(
            SwIndex(&rDestNd, nStartDelete),
            aDestIdx.GetIndex() - nStartDelete);
        assert(aDestIdx.GetIndex() == nStartDelete);
        nStartDelete = -1; 
    }

    return true;
}

OUString SwTxtNode::GetRedlineTxt( sal_Int32 nIdx, sal_Int32 nLen,
                                   bool bExpandFlds, bool bWithNum ) const
{
    std::vector<sal_Int32> aRedlArr;
    const SwDoc* pDoc = GetDoc();
    sal_uInt16 nRedlPos = pDoc->GetRedlinePos( *this, nsRedlineType_t::REDLINE_DELETE );
    if( USHRT_MAX != nRedlPos )
    {
        
        const sal_uLong nNdIdx = GetIndex();
        for( ; nRedlPos < pDoc->GetRedlineTbl().size() ; ++nRedlPos )
        {
            const SwRangeRedline* pTmp = pDoc->GetRedlineTbl()[ nRedlPos ];
            if( nsRedlineType_t::REDLINE_DELETE == pTmp->GetType() )
            {
                const SwPosition *pRStt = pTmp->Start(), *pREnd = pTmp->End();
                if( pRStt->nNode < nNdIdx )
                {
                    if( pREnd->nNode > nNdIdx )
                        
                        return OUString();
                    else if( pREnd->nNode == nNdIdx )
                    {
                        
                        aRedlArr.push_back( 0 );
                        aRedlArr.push_back( pREnd->nContent.GetIndex() );
                    }
                }
                else if( pRStt->nNode == nNdIdx )
                {
                    
                    aRedlArr.push_back( pRStt->nContent.GetIndex() );
                    if( pREnd->nNode == nNdIdx )
                        aRedlArr.push_back( pREnd->nContent.GetIndex() );
                    else
                    {
                        aRedlArr.push_back(GetTxt().getLength());
                        break;      
                    }
                }
                else
                    break;      
            }
        }
    }

    OUStringBuffer aTxt((nLen > GetTxt().getLength())
                ? GetTxt().copy(nIdx)
                : GetTxt().copy(nIdx, nLen));

    sal_Int32 nTxtStt = nIdx;
    sal_Int32 nIdxEnd = nIdx + aTxt.getLength();
    for( size_t n = 0; n < aRedlArr.size(); n += 2 )
    {
        sal_Int32 nStt = aRedlArr[ n ];
        sal_Int32 nEnd = aRedlArr[ n+1 ];
        if( ( nIdx <= nStt && nStt <= nIdxEnd ) ||
            ( nIdx <= nEnd && nEnd <= nIdxEnd ))
        {
            if( nStt < nIdx ) nStt = nIdx;
            if( nIdxEnd < nEnd ) nEnd = nIdxEnd;
            const sal_Int32 nDelCnt = nEnd - nStt;
            aTxt.remove(nStt - nTxtStt, nDelCnt);
            Replace0xFF(*this, aTxt, nTxtStt, nStt - nTxtStt, bExpandFlds);
            nTxtStt += nDelCnt;
        }
        else if( nStt >= nIdxEnd )
            break;
    }
    Replace0xFF(*this, aTxt, nTxtStt, aTxt.getLength(), bExpandFlds);

    if( bWithNum )
        aTxt.insert(0, GetNumString());
    return aTxt.makeStringAndClear();
}

/*************************************************************************
 *                        SwTxtNode::ReplaceText
 *************************************************************************/

void SwTxtNode::ReplaceText( const SwIndex& rStart, const sal_Int32 nDelLen,
                             const OUString & rStr)
{
    assert( rStart.GetIndex() < m_Text.getLength()     
         && rStart.GetIndex() + nDelLen <= m_Text.getLength());

    sal_Int32 const nOverflow(rStr.getLength() - nDelLen - GetSpaceLeft());
    SAL_WARN_IF(nOverflow > 0, "sw.core",
            "SwTxtNode::ReplaceText: node text with insertion > node capacity.");
    OUString const sInserted(
        (nOverflow > 0) ? rStr.copy(0, rStr.getLength() - nOverflow) : rStr);
    if (sInserted.isEmpty() && 0 == nDelLen)
    {
        return; 
    }

    const sal_Int32 nStartPos = rStart.GetIndex();
    sal_Int32 nEndPos = nStartPos + nDelLen;
    sal_Int32 nLen = nDelLen;
    for( sal_Int32 nPos = nStartPos; nPos < nEndPos; ++nPos )
    {
        if ((CH_TXTATR_BREAKWORD == m_Text[nPos]) ||
            (CH_TXTATR_INWORD    == m_Text[nPos]))
        {
            SwTxtAttr *const pHint = GetTxtAttrForCharAt( nPos );
            if (pHint)
            {
               assert(!( pHint->GetEnd() && pHint->HasDummyChar()
                            && (*pHint->GetStart() < nEndPos)
                            && (*pHint->GetEnd()   > nEndPos) ));
                    
                DeleteAttribute( pHint );
                --nEndPos;
                --nLen;
            }
        }
    }

    bool bOldExpFlg = IsIgnoreDontExpand();
    SetIgnoreDontExpand( true );

    if (nLen && sInserted.getLength())
    {
        
        
        m_Text = m_Text.replaceAt(nStartPos, 1, sInserted.copy(0, 1));

        ++((SwIndex&)rStart);
        m_Text = m_Text.replaceAt(rStart.GetIndex(), nLen - 1, "");
        Update( rStart, nLen - 1, true );

        OUString aTmpTxt( sInserted.copy(1) );
        m_Text = m_Text.replaceAt(rStart.GetIndex(), 0, aTmpTxt);
        Update( rStart, aTmpTxt.getLength(), false );
    }
    else
    {
        m_Text = m_Text.replaceAt(nStartPos, nLen, "");
        Update( rStart, nLen, true );

        m_Text = m_Text.replaceAt(nStartPos, 0, sInserted);
        Update( rStart, sInserted.getLength(), false );
    }

    SetIgnoreDontExpand( bOldExpFlg );
    SwDelTxt aDelHint( nStartPos, nDelLen );
    NotifyClients( 0, &aDelHint );

    SwInsTxt aHint( nStartPos, sInserted.getLength() );
    NotifyClients( 0, &aHint );
}

namespace {
    static void lcl_ResetParAttrs( SwTxtNode &rTxtNode )
    {
        std::set<sal_uInt16> aAttrs;
        aAttrs.insert( aAttrs.end(), RES_PARATR_LIST_ID );
        aAttrs.insert( aAttrs.end(), RES_PARATR_LIST_LEVEL );
        aAttrs.insert( aAttrs.end(), RES_PARATR_LIST_ISRESTART );
        aAttrs.insert( aAttrs.end(), RES_PARATR_LIST_RESTARTVALUE );
        aAttrs.insert( aAttrs.end(), RES_PARATR_LIST_ISCOUNTED );
        SwPaM aPam( rTxtNode );
        
        
        rTxtNode.GetDoc()->ResetAttrs( aPam, false, aAttrs, false );
    }

    
    
    
    
    
    
    
    
    
    void HandleModifyAtTxtNode( SwTxtNode& rTxtNode,
                                const SfxPoolItem* pOldValue,
                                const SfxPoolItem* pNewValue )
    {
        const sal_uInt16 nWhich = pOldValue ? pOldValue->Which() :
                              pNewValue ? pNewValue->Which() : 0 ;
        bool bNumRuleSet = false;
        bool bParagraphStyleChanged = false;
        OUString sNumRule;
        OUString sOldNumRule;
        switch ( nWhich )
        {
            case RES_FMT_CHG:
            {
                bParagraphStyleChanged = true;
                if( rTxtNode.GetNodes().IsDocNodes() )
                {
                    const SwNumRule* pFormerNumRuleAtTxtNode =
                        rTxtNode.GetNum() ? rTxtNode.GetNum()->GetNumRule() : 0;
                    if ( pFormerNumRuleAtTxtNode )
                    {
                        sOldNumRule = pFormerNumRuleAtTxtNode->GetName();
                    }
                    if ( rTxtNode.IsEmptyListStyleDueToSetOutlineLevelAttr() )
                    {
                        const SwNumRuleItem& rNumRuleItem = rTxtNode.GetTxtColl()->GetNumRule();
                        if ( !rNumRuleItem.GetValue().isEmpty() )
                        {
                            rTxtNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
                        }
                    }
                    const SwNumRule* pNumRuleAtTxtNode = rTxtNode.GetNumRule();
                    if ( pNumRuleAtTxtNode )
                    {
                        bNumRuleSet = true;
                        sNumRule = pNumRuleAtTxtNode->GetName();
                    }
                }
                break;
            }
            case RES_ATTRSET_CHG:
            {
                const SfxPoolItem* pItem = 0;
                const SwNumRule* pFormerNumRuleAtTxtNode =
                    rTxtNode.GetNum() ? rTxtNode.GetNum()->GetNumRule() : 0;
                if ( pFormerNumRuleAtTxtNode )
                {
                    sOldNumRule = pFormerNumRuleAtTxtNode->GetName();
                }

                if ( dynamic_cast<const SwAttrSetChg*>(pNewValue)->GetChgSet()->GetItemState( RES_PARATR_NUMRULE, false, &pItem ) ==
                        SFX_ITEM_SET )
                {
                    
                    rTxtNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
                    bNumRuleSet = true;
                }
                
                
                const SwNumRule* pNumRuleAtTxtNode = rTxtNode.GetNumRule();
                if ( pNumRuleAtTxtNode )
                {
                    sNumRule = pNumRuleAtTxtNode->GetName();
                }
                break;
            }
            case RES_PARATR_NUMRULE:
            {
                if ( rTxtNode.GetNodes().IsDocNodes() )
                {
                    const SwNumRule* pFormerNumRuleAtTxtNode =
                        rTxtNode.GetNum() ? rTxtNode.GetNum()->GetNumRule() : 0;
                    if ( pFormerNumRuleAtTxtNode )
                    {
                        sOldNumRule = pFormerNumRuleAtTxtNode->GetName();
                    }

                    if ( pNewValue )
                    {
                        
                        rTxtNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
                        bNumRuleSet = true;
                    }
                    
                    
                    const SwNumRule* pNumRuleAtTxtNode = rTxtNode.GetNumRule();
                    if ( pNumRuleAtTxtNode )
                    {
                        sNumRule = pNumRuleAtTxtNode->GetName();
                    }
                }
                break;
            }
        }
        if ( sNumRule != sOldNumRule )
        {
            if ( bNumRuleSet )
            {
                if (sNumRule.isEmpty())
                {
                    rTxtNode.RemoveFromList();
                    if ( bParagraphStyleChanged )
                    {
                        lcl_ResetParAttrs(rTxtNode);
                    }
                }
                else
                {
                    rTxtNode.RemoveFromList();
                    
                    
                    if (sNumRule==SwNumRule::GetOutlineRuleName())
                    {
                        
                        OSL_ENSURE( rTxtNode.GetTxtColl()->IsAssignedToListLevelOfOutlineStyle(),
                                "<HandleModifyAtTxtNode()> - text node with outline style, but its paragraph style is not assigned to outline style." );
                        const int nNewListLevel =
                            rTxtNode.GetTxtColl()->GetAssignedOutlineStyleLevel();
                        if ( 0 <= nNewListLevel && nNewListLevel < MAXLEVEL )
                        {
                            rTxtNode.SetAttrListLevel( nNewListLevel );
                        }
                    }
                    rTxtNode.AddToList();
                }
            }
            else 
            {
                rTxtNode.RemoveFromList();
                if ( bParagraphStyleChanged )
                {
                    lcl_ResetParAttrs(rTxtNode);
                    
                    if ( dynamic_cast<const SfxUInt16Item &>(rTxtNode.GetAttr( RES_PARATR_OUTLINELEVEL, sal_False )).GetValue() > 0 )
                    {
                        rTxtNode.SetEmptyListStyleDueToSetOutlineLevelAttr();
                    }
                }
            }
        }
        else if (!sNumRule.isEmpty() && !rTxtNode.IsInList())
        {
            rTxtNode.AddToList();
        }
    }
    
}

void SwTxtNode::Modify( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue )
{
    bool bWasNotifiable = m_bNotifiable;
    m_bNotifiable = false;

    
    
    
    
    
    if( pOldValue && pNewValue && RES_FMT_CHG == pOldValue->Which() &&
        GetRegisteredIn() == ((SwFmtChg*)pNewValue)->pChangedFmt &&
        GetNodes().IsDocNodes() )
    {
        _ChgTxtCollUpdateNum(
                        (SwTxtFmtColl*)((SwFmtChg*)pOldValue)->pChangedFmt,
                        (SwTxtFmtColl*)((SwFmtChg*)pNewValue)->pChangedFmt );
    }

    if ( !mbInSetOrResetAttr )
    {
        HandleModifyAtTxtNode( *this, pOldValue, pNewValue );
    }

    SwCntntNode::Modify( pOldValue, pNewValue );

    SwDoc * pDoc = GetDoc();
    
    if ( pDoc && !pDoc->IsInDtor() && &pDoc->GetNodes() == &GetNodes() )
    {
        pDoc->GetNodes().UpdateOutlineNode(*this);
    }

    m_bNotifiable = bWasNotifiable;

    if (pOldValue && (RES_REMOVE_UNO_OBJECT == pOldValue->Which()))
    {   
        SetXParagraph(::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextContent>(0));
    }
}

SwFmtColl* SwTxtNode::ChgFmtColl( SwFmtColl *pNewColl )
{
    OSL_ENSURE( pNewColl,"ChgFmtColl: Collectionpointer has value 0." );
    OSL_ENSURE( HAS_BASE( SwTxtFmtColl, pNewColl ),
                "ChgFmtColl: is not a Text Collection pointer." );

    SwTxtFmtColl *pOldColl = GetTxtColl();
    if( pNewColl != pOldColl )
    {
        SetCalcHiddenCharFlags();
        SwCntntNode::ChgFmtColl( pNewColl );
        OSL_ENSURE( !mbInSetOrResetAttr,
                "DEBUG OSL_ENSURE(ON - <SwTxtNode::ChgFmtColl(..)> called during <Set/ResetAttr(..)>" );
        if ( !mbInSetOrResetAttr )
        {
            SwFmtChg aTmp1( pOldColl );
            SwFmtChg aTmp2( pNewColl );
            HandleModifyAtTxtNode( *this, &aTmp1, &aTmp2  );
        }
    }

    
    if( GetNodes().IsDocNodes() )
    {
        _ChgTxtCollUpdateNum( pOldColl, static_cast<SwTxtFmtColl *>(pNewColl) );
    }

    GetNodes().UpdateOutlineNode(*this);

    return pOldColl;
}

SwNodeNum* SwTxtNode::CreateNum() const
{
    if ( !mpNodeNum )
    {
        mpNodeNum = new SwNodeNum( const_cast<SwTxtNode*>(this) );
    }
    return mpNodeNum;
}

SwNumberTree::tNumberVector SwTxtNode::GetNumberVector() const
{
    if ( GetNum() )
    {
        return GetNum()->GetNumberVector();
    }
    else
    {
        SwNumberTree::tNumberVector aResult;
        return aResult;
    }
}

bool SwTxtNode::IsOutline() const
{
    bool bResult = false;

    if ( GetAttrOutlineLevel() > 0 )
    {
        bResult = !IsInRedlines();
    }
    else
    {
        const SwNumRule* pRule( GetNum() ? GetNum()->GetNumRule() : 0L );
        if ( pRule && pRule->IsOutlineRule() )
        {
            bResult = !IsInRedlines();
        }
    }

    return bResult;
}

bool SwTxtNode::IsOutlineStateChanged() const
{
    return IsOutline() != m_bLastOutlineState;
}

void SwTxtNode::UpdateOutlineState()
{
    m_bLastOutlineState = IsOutline();
}

int SwTxtNode::GetAttrOutlineLevel() const
{
    return ((const SfxUInt16Item &)GetAttr(RES_PARATR_OUTLINELEVEL)).GetValue();
}

void SwTxtNode::SetAttrOutlineLevel(int nLevel)
{
    assert(0 <= nLevel && nLevel <= MAXLEVEL); 
    if ( 0 <= nLevel && nLevel <= MAXLEVEL )
    {
        SetAttr( SfxUInt16Item( RES_PARATR_OUTLINELEVEL,
                                static_cast<sal_uInt16>(nLevel) ) );
    }
}


bool SwTxtNode::IsEmptyListStyleDueToSetOutlineLevelAttr()
{
    return mbEmptyListStyleSetDueToSetOutlineLevelAttr;
}

void SwTxtNode::SetEmptyListStyleDueToSetOutlineLevelAttr()
{
    if ( !mbEmptyListStyleSetDueToSetOutlineLevelAttr )
    {
        SetAttr( SwNumRuleItem() );
        mbEmptyListStyleSetDueToSetOutlineLevelAttr = true;
    }
}

void SwTxtNode::ResetEmptyListStyleDueToResetOutlineLevelAttr()
{
    if ( mbEmptyListStyleSetDueToSetOutlineLevelAttr )
    {
        ResetAttr( RES_PARATR_NUMRULE );
        mbEmptyListStyleSetDueToSetOutlineLevelAttr = false;
    }
}

void SwTxtNode::SetAttrListLevel( int nLevel )
{
    if ( nLevel < 0 || nLevel >= MAXLEVEL )
    {
        assert(false); 
        return;
    }

    SfxInt16Item aNewListLevelItem( RES_PARATR_LIST_LEVEL,
                                    static_cast<sal_Int16>(nLevel) );
    SetAttr( aNewListLevelItem );
}

bool SwTxtNode::HasAttrListLevel() const
{
    return GetpSwAttrSet() &&
           GetpSwAttrSet()->GetItemState( RES_PARATR_LIST_LEVEL, false ) == SFX_ITEM_SET;
}

int SwTxtNode::GetAttrListLevel() const
{
    int nAttrListLevel = 0;

    const SfxInt16Item& aListLevelItem =
        dynamic_cast<const SfxInt16Item&>(GetAttr( RES_PARATR_LIST_LEVEL ));
    nAttrListLevel = static_cast<int>(aListLevelItem.GetValue());

    return nAttrListLevel;
}

int SwTxtNode::GetActualListLevel() const
{
    return GetNum() ? GetNum()->GetLevelInListTree() : -1;
}

void SwTxtNode::SetListRestart( bool bRestart )
{
    if ( !bRestart )
    {
        
        
        ResetAttr( RES_PARATR_LIST_ISRESTART );
    }
    else
    {
        SfxBoolItem aNewIsRestartItem( RES_PARATR_LIST_ISRESTART,
                                       true );
        SetAttr( aNewIsRestartItem );
    }
}

bool SwTxtNode::IsListRestart() const
{
    const SfxBoolItem& aIsRestartItem =
        dynamic_cast<const SfxBoolItem&>(GetAttr( RES_PARATR_LIST_ISRESTART ));

    return aIsRestartItem.GetValue() ? true : false;
}

/** Returns if the paragraph has a visible numbering or bullet.
    This includes all kinds of numbering/bullet/outlines.
    The concrete list label string has to be checked, too.
 */
bool SwTxtNode::HasVisibleNumberingOrBullet() const
{
    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : 0L;
    if ( pRule && IsCountedInList())
    {
        int nLevel = GetActualListLevel();

        if (nLevel < 0)
            nLevel = 0;

        if (nLevel >= MAXLEVEL)
            nLevel = MAXLEVEL - 1;

        
        
        
        const SwNumFmt& rFmt = pRule->Get( static_cast<sal_uInt16>(nLevel ));
        return SVX_NUM_NUMBER_NONE != rFmt.GetNumberingType() ||
               !pRule->MakeNumString( *(GetNum()) ).isEmpty();
    }

    return false;
}

void SwTxtNode::SetAttrListRestartValue( SwNumberTree::tSwNumTreeNumber nNumber )
{
    const bool bChanged( HasAttrListRestartValue()
                         ? GetAttrListRestartValue() != nNumber
                         : nNumber != USHRT_MAX );

    if ( bChanged || !HasAttrListRestartValue() )
    {
        if ( nNumber == USHRT_MAX )
        {
            ResetAttr( RES_PARATR_LIST_RESTARTVALUE );
        }
        else
        {
            SfxInt16Item aNewListRestartValueItem( RES_PARATR_LIST_RESTARTVALUE,
                                                   static_cast<sal_Int16>(nNumber) );
            SetAttr( aNewListRestartValueItem );
        }
    }
}

bool SwTxtNode::HasAttrListRestartValue() const
{
    return GetpSwAttrSet() &&
           GetpSwAttrSet()->GetItemState( RES_PARATR_LIST_RESTARTVALUE, false ) == SFX_ITEM_SET;
}
SwNumberTree::tSwNumTreeNumber SwTxtNode::GetAttrListRestartValue() const
{
    OSL_ENSURE( HasAttrListRestartValue(),
            "<SwTxtNode::GetAttrListRestartValue()> - only ask for list restart value, if attribute is set at text node." );

    const SfxInt16Item& aListRestartValueItem =
        dynamic_cast<const SfxInt16Item&>(GetAttr( RES_PARATR_LIST_RESTARTVALUE ));
    return static_cast<SwNumberTree::tSwNumTreeNumber>(aListRestartValueItem.GetValue());
}

SwNumberTree::tSwNumTreeNumber SwTxtNode::GetActualListStartValue() const
{
    SwNumberTree::tSwNumTreeNumber nListRestartValue = 1;

    if ( IsListRestart() && HasAttrListRestartValue() )
    {
        nListRestartValue = GetAttrListRestartValue();
    }
    else
    {
        SwNumRule* pRule = GetNumRule();
        if ( pRule )
        {
            const SwNumFmt* pFmt =
                    pRule->GetNumFmt( static_cast<sal_uInt16>(GetAttrListLevel()) );
            if ( pFmt )
            {
                nListRestartValue = pFmt->GetStart();
            }
        }
    }

    return nListRestartValue;
}

bool SwTxtNode::IsNotifiable() const
{
    return m_bNotifiable && IsNotificationEnabled();
}

bool SwTxtNode::IsNotificationEnabled() const
{
    bool bResult = false;
    const SwDoc * pDoc = GetDoc();
    if( pDoc )
    {
        bResult = pDoc->IsInReading() || pDoc->IsInDtor() ? false : true;
    }
    return bResult;
}

void SwTxtNode::SetCountedInList( bool bCounted )
{
    if ( bCounted )
    {
        
        
        ResetAttr( RES_PARATR_LIST_ISCOUNTED );
    }
    else
    {
        SfxBoolItem aIsCountedInListItem( RES_PARATR_LIST_ISCOUNTED, false );
        SetAttr( aIsCountedInListItem );
    }
}

bool SwTxtNode::IsCountedInList() const
{
    const SfxBoolItem& aIsCountedInListItem =
        dynamic_cast<const SfxBoolItem&>(GetAttr( RES_PARATR_LIST_ISCOUNTED ));

    return aIsCountedInListItem.GetValue() ? true : false;
}

void SwTxtNode::AddToList()
{
    if ( IsInList() )
    {
        OSL_FAIL( "<SwTxtNode::AddToList()> - the text node is already added to a list. Serious defect -> please inform OD" );
        return;
    }

    const OUString sListId = GetListId();
    if (!sListId.isEmpty())
    {
        SwList* pList = GetDoc()->getListByName( sListId );
        if ( pList == 0 )
        {
            
            SwNumRule* pNumRule = GetNumRule();
            if ( pNumRule )
            {
                pList = GetDoc()->createList( sListId, GetNumRule()->GetName() );
            }
        }
        OSL_ENSURE( pList != 0,
                "<SwTxtNode::AddToList()> - no list for given list id. Serious defect -> please inform OD" );
        if ( pList )
        {
            pList->InsertListItem( *CreateNum(), GetAttrListLevel() );
            mpList = pList;
        }
    }
}

void SwTxtNode::RemoveFromList()
{
    if ( IsInList() )
    {
        mpList->RemoveListItem( *mpNodeNum );
        mpList = 0;
        delete mpNodeNum;
        mpNodeNum = 0L;

        SetWordCountDirty( true );
    }
}

bool SwTxtNode::IsInList() const
{
    return GetNum() != 0 && GetNum()->GetParent() != 0;
}

bool SwTxtNode::IsFirstOfNumRule() const
{
    bool bResult = false;

    if ( GetNum() && GetNum()->GetNumRule())
        bResult = GetNum()->IsFirst();

    return bResult;
}

void SwTxtNode::SetListId(OUString const& rListId)
{
    const SfxStringItem& rListIdItem =
            dynamic_cast<const SfxStringItem&>(GetAttr( RES_PARATR_LIST_ID ));
    if (rListIdItem.GetValue() != rListId)
    {
        if (rListId.isEmpty())
        {
            ResetAttr( RES_PARATR_LIST_ID );
        }
        else
        {
            SfxStringItem aNewListIdItem(RES_PARATR_LIST_ID, rListId);
            SetAttr( aNewListIdItem );
        }
    }
}

OUString SwTxtNode::GetListId() const
{
    OUString sListId;

    const SfxStringItem& rListIdItem =
                dynamic_cast<const SfxStringItem&>(GetAttr( RES_PARATR_LIST_ID ));
    sListId = rListIdItem.GetValue();

    
    
    if (sListId.isEmpty())
    {
        SwNumRule* pRule = GetNumRule();
        if ( pRule )
        {
            sListId = pRule->GetDefaultListId();
        }
    }

    return sListId;
}

/** Determines, if the list level indent attributes can be applied to the
    paragraph.

    The list level indents can be applied to the paragraph under the one
    of following conditions:
    - the list style is directly applied to the paragraph and the paragraph
      has no own indent attributes.
    - the list style is applied to the paragraph through one of its paragraph
      styles, the paragraph has no own indent attributes and on the paragraph
      style hierarchy from the paragraph to the paragraph style with the
      list style no indent attributes are found.

    @return boolean
*/
bool SwTxtNode::AreListLevelIndentsApplicable() const
{
    bool bAreListLevelIndentsApplicable( true );

    if ( !GetNum() || !GetNum()->GetNumRule() )
    {
        
        bAreListLevelIndentsApplicable = false;
    }
    else if ( HasSwAttrSet() &&
              GetpSwAttrSet()->GetItemState( RES_LR_SPACE, false ) == SFX_ITEM_SET )
    {
        
        bAreListLevelIndentsApplicable = false;
    }
    else if ( HasSwAttrSet() &&
              GetpSwAttrSet()->GetItemState( RES_PARATR_NUMRULE, false ) == SFX_ITEM_SET )
    {
        
        
        bAreListLevelIndentsApplicable = true;
    }
    else
    {
        
        

        
        const SwTxtFmtColl* pColl = GetTxtColl();
        while ( pColl )
        {
            if ( pColl->GetAttrSet().GetItemState( RES_LR_SPACE, false ) == SFX_ITEM_SET )
            {
                
                bAreListLevelIndentsApplicable = false;
                break;
            }

            if ( pColl->GetAttrSet().GetItemState( RES_PARATR_NUMRULE, false ) == SFX_ITEM_SET )
            {
                
                
                bAreListLevelIndentsApplicable = true;
                break;
            }

            pColl = dynamic_cast<const SwTxtFmtColl*>(pColl->DerivedFrom());
            OSL_ENSURE( pColl,
                    "<SwTxtNode::AreListLevelIndentsApplicable()> - something wrong in paragraph's style hierarchy. The applied list style is not found." );
        }
    }

    return bAreListLevelIndentsApplicable;
}

/** Retrieves the list tab stop position, if the paragraph's list level defines
    one and this list tab stop has to merged into the tap stops of the paragraph

    @param nListTabStopPosition
    output parameter - containing the list tab stop position

    @return boolean - indicating, if a list tab stop position is provided
*/
bool SwTxtNode::GetListTabStopPosition( long& nListTabStopPosition ) const
{
    bool bListTanStopPositionProvided( false );

    const SwNumRule* pNumRule = GetNum() ? GetNum()->GetNumRule() : 0;
    if ( pNumRule && HasVisibleNumberingOrBullet() && GetActualListLevel() >= 0 )
    {
        const SwNumFmt& rFmt = pNumRule->Get( static_cast<sal_uInt16>(GetActualListLevel()) );
        if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT &&
             rFmt.GetLabelFollowedBy() == SvxNumberFormat::LISTTAB )
        {
            bListTanStopPositionProvided = true;
            nListTabStopPosition = rFmt.GetListtabPos();

            if ( getIDocumentSettingAccess()->get(IDocumentSettingAccess::TABS_RELATIVE_TO_INDENT) )
            {
                
                
                if ( AreListLevelIndentsApplicable() )
                {
                    nListTabStopPosition -= rFmt.GetIndentAt();
                }
                else if (!getIDocumentSettingAccess()->get(IDocumentSettingAccess::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING))
                {
                    SvxLRSpaceItem aItem = GetSwAttrSet().GetLRSpace();
                    nListTabStopPosition -= aItem.GetTxtLeft();
                }
            }
        }
    }

    return bListTanStopPositionProvided;
}

OUString SwTxtNode::GetLabelFollowedBy() const
{
    const SwNumRule* pNumRule = GetNum() ? GetNum()->GetNumRule() : 0;
    if ( pNumRule && HasVisibleNumberingOrBullet() && GetActualListLevel() >= 0 )
    {
        const SwNumFmt& rFmt = pNumRule->Get( static_cast<sal_uInt16>(GetActualListLevel()) );
        if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
        {
            switch ( rFmt.GetLabelFollowedBy() )
            {
                case SvxNumberFormat::LISTTAB:
                {
                    return OUString("\t");
                }
                break;
                case SvxNumberFormat::SPACE:
                {
                    return OUString(" ");
                }
                break;
                case SvxNumberFormat::NOTHING:
                {
                    
                }
                break;
                default:
                {
                    OSL_FAIL( "<SwTxtNode::GetLabelFollowedBy()> - unknown SvxNumberFormat::GetLabelFollowedBy() return value" );
                }
            }
        }
    }

    return OUString();
}

void SwTxtNode::CalcHiddenCharFlags() const
{
    sal_Int32 nStartPos;
    sal_Int32 nEndPos;
    
    SwScriptInfo::GetBoundsOfHiddenRange( *this, 0, nStartPos, nEndPos );
}


bool SwTxtNode::IsHidden() const
{
    if ( HasHiddenParaField() || HasHiddenCharAttribute( true ) )
        return true;

    const SwSectionNode* pSectNd = FindSectionNode();
    if ( pSectNd && pSectNd->GetSection().IsHiddenFlag() )
        return true;

    return false;
}

namespace {
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    class HandleSetAttrAtTxtNode
    {
        public:
            HandleSetAttrAtTxtNode( SwTxtNode& rTxtNode,
                                    const SfxPoolItem& pItem );
            HandleSetAttrAtTxtNode( SwTxtNode& rTxtNode,
                                    const SfxItemSet& rItemSet );
            ~HandleSetAttrAtTxtNode();

        private:
            SwTxtNode& mrTxtNode;
            bool mbAddTxtNodeToList;
            bool mbUpdateListLevel;
            bool mbUpdateListRestart;
            bool mbUpdateListCount;
            
            bool mbOutlineLevelSet;
    };

    HandleSetAttrAtTxtNode::HandleSetAttrAtTxtNode( SwTxtNode& rTxtNode,
                                                    const SfxPoolItem& pItem )
        : mrTxtNode( rTxtNode ),
          mbAddTxtNodeToList( false ),
          mbUpdateListLevel( false ),
          mbUpdateListRestart( false ),
          mbUpdateListCount( false ),
          
          mbOutlineLevelSet( false )
    {
        switch ( pItem.Which() )
        {
            
            case RES_PARATR_NUMRULE:
            {
                mrTxtNode.RemoveFromList();

                const SwNumRuleItem& pNumRuleItem =
                                dynamic_cast<const SwNumRuleItem&>(pItem);
                if ( !pNumRuleItem.GetValue().isEmpty() )
                {
                    mbAddTxtNodeToList = true;
                    
                    //
                    mrTxtNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
                }
            }
            break;

            
            case RES_PARATR_LIST_ID:
            {
                const SfxStringItem& pListIdItem =
                                        dynamic_cast<const SfxStringItem&>(pItem);
                OSL_ENSURE( pListIdItem.GetValue().getLength() > 0,
                        "<HandleSetAttrAtTxtNode(..)> - empty list id attribute not excepted. Serious defect -> please inform OD." );
                const OUString sListIdOfTxtNode = rTxtNode.GetListId();
                if ( pListIdItem.GetValue() != sListIdOfTxtNode )
                {
                    mbAddTxtNodeToList = true;
                    if ( mrTxtNode.IsInList() )
                    {
                        mrTxtNode.RemoveFromList();
                    }
                }
            }
            break;

            
            case RES_PARATR_LIST_LEVEL:
            {
                const SfxInt16Item& aListLevelItem =
                                    dynamic_cast<const SfxInt16Item&>(pItem);
                if ( aListLevelItem.GetValue() != mrTxtNode.GetAttrListLevel() )
                {
                    mbUpdateListLevel = true;
                }
            }
            break;

            
            case RES_PARATR_LIST_ISRESTART:
            {
                const SfxBoolItem& aListIsRestartItem =
                                    dynamic_cast<const SfxBoolItem&>(pItem);
                if ( aListIsRestartItem.GetValue() !=
                                    mrTxtNode.IsListRestart() )
                {
                    mbUpdateListRestart = true;
                }
            }
            break;

            
            case RES_PARATR_LIST_RESTARTVALUE:
            {
                const SfxInt16Item& aListRestartValueItem =
                                    dynamic_cast<const SfxInt16Item&>(pItem);
                if ( !mrTxtNode.HasAttrListRestartValue() ||
                     aListRestartValueItem.GetValue() != mrTxtNode.GetAttrListRestartValue() )
                {
                    mbUpdateListRestart = true;
                }
            }
            break;

            
            case RES_PARATR_LIST_ISCOUNTED:
            {
                const SfxBoolItem& aIsCountedInListItem =
                                    dynamic_cast<const SfxBoolItem&>(pItem);
                if ( aIsCountedInListItem.GetValue() !=
                                    mrTxtNode.IsCountedInList() )
                {
                    mbUpdateListCount = true;
                }
            }
            break;

            
            
            case RES_PARATR_OUTLINELEVEL:
            {
                const SfxUInt16Item& aOutlineLevelItem =
                                    dynamic_cast<const SfxUInt16Item&>(pItem);
                if ( aOutlineLevelItem.GetValue() != mrTxtNode.GetAttrOutlineLevel() )
                {
                    mbOutlineLevelSet = true;
                }
            }
            break;
        }

    }

    HandleSetAttrAtTxtNode::HandleSetAttrAtTxtNode( SwTxtNode& rTxtNode,
                                                    const SfxItemSet& rItemSet )
        : mrTxtNode( rTxtNode ),
          mbAddTxtNodeToList( false ),
          mbUpdateListLevel( false ),
          mbUpdateListRestart( false ),
          mbUpdateListCount( false ),
          
          mbOutlineLevelSet( false )
    {
        const SfxPoolItem* pItem = 0;
        
        if ( rItemSet.GetItemState( RES_PARATR_NUMRULE, false, &pItem ) == SFX_ITEM_SET )
        {
            mrTxtNode.RemoveFromList();

            const SwNumRuleItem* pNumRuleItem =
                            dynamic_cast<const SwNumRuleItem*>(pItem);
            if ( !pNumRuleItem->GetValue().isEmpty() )
            {
                mbAddTxtNodeToList = true;
                
                mrTxtNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
            }
        }

        
        if ( rItemSet.GetItemState( RES_PARATR_LIST_ID, false, &pItem ) == SFX_ITEM_SET )
        {
            const SfxStringItem* pListIdItem =
                                    dynamic_cast<const SfxStringItem*>(pItem);
            const OUString sListIdOfTxtNode = mrTxtNode.GetListId();
            if ( pListIdItem &&
                 pListIdItem->GetValue() != sListIdOfTxtNode )
            {
                mbAddTxtNodeToList = true;
                if ( mrTxtNode.IsInList() )
                {
                    mrTxtNode.RemoveFromList();
                }
            }
        }

        
        if ( rItemSet.GetItemState( RES_PARATR_LIST_LEVEL, false, &pItem ) == SFX_ITEM_SET )
        {
            const SfxInt16Item* pListLevelItem =
                                dynamic_cast<const SfxInt16Item*>(pItem);
            if ( pListLevelItem->GetValue() != mrTxtNode.GetAttrListLevel() )
            {
                mbUpdateListLevel = true;
            }
        }

        
        if ( rItemSet.GetItemState( RES_PARATR_LIST_ISRESTART, false, &pItem ) == SFX_ITEM_SET )
        {
            const SfxBoolItem* pListIsRestartItem =
                                dynamic_cast<const SfxBoolItem*>(pItem);
            if ( pListIsRestartItem->GetValue() != mrTxtNode.IsListRestart() )
            {
                mbUpdateListRestart = true;
            }
        }

        
        if ( rItemSet.GetItemState( RES_PARATR_LIST_RESTARTVALUE, false, &pItem ) == SFX_ITEM_SET )
        {
            const SfxInt16Item* pListRestartValueItem =
                                dynamic_cast<const SfxInt16Item*>(pItem);
            if ( !mrTxtNode.HasAttrListRestartValue() ||
                 pListRestartValueItem->GetValue() != mrTxtNode.GetAttrListRestartValue() )
            {
                mbUpdateListRestart = true;
            }
        }

        
        if ( rItemSet.GetItemState( RES_PARATR_LIST_ISCOUNTED, false, &pItem ) == SFX_ITEM_SET )
        {
            const SfxBoolItem* pIsCountedInListItem =
                                dynamic_cast<const SfxBoolItem*>(pItem);
            if ( pIsCountedInListItem->GetValue() !=
                                mrTxtNode.IsCountedInList() )
            {
                mbUpdateListCount = true;
            }
        }

        
        
        if ( rItemSet.GetItemState( RES_PARATR_OUTLINELEVEL, false, &pItem ) == SFX_ITEM_SET )
        {
            const SfxUInt16Item* pOutlineLevelItem =
                                dynamic_cast<const SfxUInt16Item*>(pItem);
            if ( pOutlineLevelItem->GetValue() != mrTxtNode.GetAttrOutlineLevel() )
            {
                mbOutlineLevelSet = true;
            }
        }
    }

    HandleSetAttrAtTxtNode::~HandleSetAttrAtTxtNode()
    {
        if ( mbAddTxtNodeToList )
        {
            SwNumRule* pNumRuleAtTxtNode = mrTxtNode.GetNumRule();
            if ( pNumRuleAtTxtNode )
            {
                mrTxtNode.AddToList();
            }
        }
        else
        {
            if ( mbUpdateListLevel && mrTxtNode.IsInList() )
            {
                const_cast<SwNodeNum*>(mrTxtNode.GetNum())->SetLevelInListTree(
                                                    mrTxtNode.GetAttrListLevel() );
            }

            if ( mbUpdateListRestart && mrTxtNode.IsInList() )
            {
                SwNodeNum* pNodeNum = const_cast<SwNodeNum*>(mrTxtNode.GetNum());
                pNodeNum->InvalidateMe();
                pNodeNum->NotifyInvalidSiblings();
            }

            if ( mbUpdateListCount && mrTxtNode.IsInList() )
            {
                const_cast<SwNodeNum*>(mrTxtNode.GetNum())->InvalidateAndNotifyTree();
            }
        }

        
        if ( mbOutlineLevelSet )
        {
            if ( mrTxtNode.GetAttrOutlineLevel() == 0 )
            {
                mrTxtNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
            }
            else
            {
                const SfxPoolItem* pItem = 0;
                if ( mrTxtNode.GetSwAttrSet().GetItemState( RES_PARATR_NUMRULE,
                                                            true, &pItem )
                                                                != SFX_ITEM_SET )
                {
                    mrTxtNode.SetEmptyListStyleDueToSetOutlineLevelAttr();
                }
            }
        }
    }
    
}

sal_Bool SwTxtNode::SetAttr( const SfxPoolItem& pItem )
{
    const bool bOldIsSetOrResetAttr( mbInSetOrResetAttr );
    mbInSetOrResetAttr = true;

    HandleSetAttrAtTxtNode aHandleSetAttr( *this, pItem );

    sal_Bool bRet = SwCntntNode::SetAttr( pItem );

    mbInSetOrResetAttr = bOldIsSetOrResetAttr;

    return bRet;
}

sal_Bool SwTxtNode::SetAttr( const SfxItemSet& rSet )
{
    const bool bOldIsSetOrResetAttr( mbInSetOrResetAttr );
    mbInSetOrResetAttr = true;

    HandleSetAttrAtTxtNode aHandleSetAttr( *this, rSet );

    sal_Bool bRet = SwCntntNode::SetAttr( rSet );

    mbInSetOrResetAttr = bOldIsSetOrResetAttr;

    return bRet;
}

namespace {
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    class HandleResetAttrAtTxtNode
    {
        public:
            HandleResetAttrAtTxtNode( SwTxtNode& rTxtNode,
                                      const sal_uInt16 nWhich1,
                                      const sal_uInt16 nWhich2 );
            HandleResetAttrAtTxtNode( SwTxtNode& rTxtNode,
                                      const std::vector<sal_uInt16>& rWhichArr );
            HandleResetAttrAtTxtNode( SwTxtNode& rTxtNode );

            ~HandleResetAttrAtTxtNode();

        private:
            SwTxtNode& mrTxtNode;
            bool mbListStyleOrIdReset;
            bool mbUpdateListLevel;
            bool mbUpdateListRestart;
            bool mbUpdateListCount;
    };

    HandleResetAttrAtTxtNode::HandleResetAttrAtTxtNode( SwTxtNode& rTxtNode,
                                                        const sal_uInt16 nWhich1,
                                                        const sal_uInt16 nWhich2 )
        : mrTxtNode( rTxtNode ),
          mbListStyleOrIdReset( false ),
          mbUpdateListLevel( false ),
          mbUpdateListRestart( false ),
          mbUpdateListCount( false )
    {
        bool bRemoveFromList( false );
        if ( nWhich2 != 0 && nWhich2 > nWhich1 )
        {
            
            if ( nWhich1 <= RES_PARATR_NUMRULE && RES_PARATR_NUMRULE <= nWhich2 )
            {
                bRemoveFromList = mrTxtNode.GetNumRule() != 0;
                mbListStyleOrIdReset = true;
            }
            else if ( nWhich1 <= RES_PARATR_LIST_ID && RES_PARATR_LIST_ID <= nWhich2 )
            {
                bRemoveFromList = mrTxtNode.GetpSwAttrSet() &&
                    mrTxtNode.GetpSwAttrSet()->GetItemState( RES_PARATR_LIST_ID, false ) == SFX_ITEM_SET;
                
                mbListStyleOrIdReset = true;
            }

            if ( !bRemoveFromList )
            {
                
                mbUpdateListLevel = ( nWhich1 <= RES_PARATR_LIST_LEVEL &&
                                      RES_PARATR_LIST_LEVEL <= nWhich2 &&
                                      mrTxtNode.HasAttrListLevel() );

                
                mbUpdateListRestart =
                    ( nWhich1 <= RES_PARATR_LIST_ISRESTART && RES_PARATR_LIST_ISRESTART <= nWhich2 &&
                      mrTxtNode.IsListRestart() ) ||
                    ( nWhich1 <= RES_PARATR_LIST_RESTARTVALUE && RES_PARATR_LIST_RESTARTVALUE <= nWhich2 &&
                      mrTxtNode.HasAttrListRestartValue() );

                
                mbUpdateListCount =
                    ( nWhich1 <= RES_PARATR_LIST_ISCOUNTED && RES_PARATR_LIST_ISCOUNTED <= nWhich2 &&
                      !mrTxtNode.IsCountedInList() );
            }

            
            
            if ( nWhich1 <= RES_PARATR_OUTLINELEVEL && RES_PARATR_OUTLINELEVEL <= nWhich2 )
            {
                mrTxtNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
            }
        }
        else
        {
            
            if ( nWhich1 == RES_PARATR_NUMRULE )
            {
                bRemoveFromList = mrTxtNode.GetNumRule() != 0;
                mbListStyleOrIdReset = true;
            }
            else if ( nWhich1 == RES_PARATR_LIST_ID )
            {
                bRemoveFromList = mrTxtNode.GetpSwAttrSet() &&
                    mrTxtNode.GetpSwAttrSet()->GetItemState( RES_PARATR_LIST_ID, false ) == SFX_ITEM_SET;
                
                mbListStyleOrIdReset = true;
            }
            
            
            else if ( nWhich1 == RES_PARATR_OUTLINELEVEL )
            {
                mrTxtNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
            }

            if ( !bRemoveFromList )
            {
                
                mbUpdateListLevel = nWhich1 == RES_PARATR_LIST_LEVEL &&
                                    mrTxtNode.HasAttrListLevel();

                
                mbUpdateListRestart = ( nWhich1 == RES_PARATR_LIST_ISRESTART &&
                                        mrTxtNode.IsListRestart() ) ||
                                      ( nWhich1 == RES_PARATR_LIST_RESTARTVALUE &&
                                        mrTxtNode.HasAttrListRestartValue() );

                
                mbUpdateListCount = nWhich1 == RES_PARATR_LIST_ISCOUNTED &&
                                    !mrTxtNode.IsCountedInList();
            }
        }

        if ( bRemoveFromList && mrTxtNode.IsInList() )
        {
            mrTxtNode.RemoveFromList();
        }
    }

    HandleResetAttrAtTxtNode::HandleResetAttrAtTxtNode( SwTxtNode& rTxtNode,
                                                        const std::vector<sal_uInt16>& rWhichArr )
        : mrTxtNode( rTxtNode ),
          mbListStyleOrIdReset( false ),
          mbUpdateListLevel( false ),
          mbUpdateListRestart( false ),
          mbUpdateListCount( false )
    {
        bool bRemoveFromList( false );
        {
            std::vector<sal_uInt16>::const_iterator it;
            for ( it = rWhichArr.begin(); it != rWhichArr.end(); ++it)
            {
                
                if ( *it == RES_PARATR_NUMRULE )
                {
                    bRemoveFromList = bRemoveFromList ||
                                      mrTxtNode.GetNumRule() != 0;
                    mbListStyleOrIdReset = true;
                }
                else if ( *it == RES_PARATR_LIST_ID )
                {
                    bRemoveFromList = bRemoveFromList ||
                        ( mrTxtNode.GetpSwAttrSet() &&
                          mrTxtNode.GetpSwAttrSet()->GetItemState( RES_PARATR_LIST_ID, false ) == SFX_ITEM_SET );
                    
                    mbListStyleOrIdReset = true;
                }
                
                
                else if ( *it == RES_PARATR_OUTLINELEVEL )
                {
                    mrTxtNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
                }

                if ( !bRemoveFromList )
                {
                    
                    mbUpdateListLevel = mbUpdateListLevel ||
                                        ( *it == RES_PARATR_LIST_LEVEL &&
                                          mrTxtNode.HasAttrListLevel() );

                    
                    mbUpdateListRestart = mbUpdateListRestart ||
                                          ( *it == RES_PARATR_LIST_ISRESTART &&
                                            mrTxtNode.IsListRestart() ) ||
                                          ( *it == RES_PARATR_LIST_RESTARTVALUE &&
                                            mrTxtNode.HasAttrListRestartValue() );

                    
                    mbUpdateListCount = mbUpdateListCount ||
                                        ( *it == RES_PARATR_LIST_ISCOUNTED &&
                                          !mrTxtNode.IsCountedInList() );
                }
            }
        }

        if ( bRemoveFromList && mrTxtNode.IsInList() )
        {
            mrTxtNode.RemoveFromList();
        }
    }

    HandleResetAttrAtTxtNode::HandleResetAttrAtTxtNode( SwTxtNode& rTxtNode )
        : mrTxtNode( rTxtNode ),
          mbListStyleOrIdReset( false ),
          mbUpdateListLevel( false ),
          mbUpdateListRestart( false ),
          mbUpdateListCount( false )
    {
        mbListStyleOrIdReset = true;
        if ( rTxtNode.IsInList() )
        {
            rTxtNode.RemoveFromList();
        }
        
        mrTxtNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
    }

    HandleResetAttrAtTxtNode::~HandleResetAttrAtTxtNode()
    {
        if ( mbListStyleOrIdReset && !mrTxtNode.IsInList() )
        {
            
            
            if (mrTxtNode.GetNumRule() && !mrTxtNode.GetListId().isEmpty())
            {
                
                
                
                if ( !mrTxtNode.HasAttrListLevel() &&
                     mrTxtNode.GetNumRule()->GetName()==SwNumRule::GetOutlineRuleName() &&
                     mrTxtNode.GetTxtColl()->IsAssignedToListLevelOfOutlineStyle() )
                {
                    int nNewListLevel = mrTxtNode.GetTxtColl()->GetAssignedOutlineStyleLevel();
                    if ( 0 <= nNewListLevel && nNewListLevel < MAXLEVEL )
                    {
                        mrTxtNode.SetAttrListLevel( nNewListLevel );
                    }
                }
                mrTxtNode.AddToList();
            }
            
            
            else if ( mrTxtNode.GetpSwAttrSet() &&
                      dynamic_cast<const SfxUInt16Item &>(mrTxtNode.GetAttr( RES_PARATR_OUTLINELEVEL, sal_False )).GetValue() > 0 )
            {
                mrTxtNode.SetEmptyListStyleDueToSetOutlineLevelAttr();
            }
        }

        if ( mrTxtNode.IsInList() )
        {
            if ( mbUpdateListLevel )
            {
                SwNodeNum* pNodeNum = const_cast<SwNodeNum*>(mrTxtNode.GetNum());
                pNodeNum->SetLevelInListTree( mrTxtNode.GetAttrListLevel() );
            }

            if ( mbUpdateListRestart )
            {
                SwNodeNum* pNodeNum = const_cast<SwNodeNum*>(mrTxtNode.GetNum());
                pNodeNum->InvalidateMe();
                pNodeNum->NotifyInvalidSiblings();
            }

            if ( mbUpdateListCount )
            {
                SwNodeNum* pNodeNum = const_cast<SwNodeNum*>(mrTxtNode.GetNum());
                pNodeNum->InvalidateAndNotifyTree();
            }
        }
    }
    
}

sal_Bool SwTxtNode::ResetAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 )
{
    const bool bOldIsSetOrResetAttr( mbInSetOrResetAttr );
    mbInSetOrResetAttr = true;

    HandleResetAttrAtTxtNode aHandleResetAttr( *this, nWhich1, nWhich2 );

    sal_Bool bRet = SwCntntNode::ResetAttr( nWhich1, nWhich2 );

    mbInSetOrResetAttr = bOldIsSetOrResetAttr;

    return bRet;
}

sal_Bool SwTxtNode::ResetAttr( const std::vector<sal_uInt16>& rWhichArr )
{
    const bool bOldIsSetOrResetAttr( mbInSetOrResetAttr );
    mbInSetOrResetAttr = true;

    HandleResetAttrAtTxtNode aHandleResetAttr( *this, rWhichArr );

    sal_Bool bRet = SwCntntNode::ResetAttr( rWhichArr );

    mbInSetOrResetAttr = bOldIsSetOrResetAttr;

    return bRet;
}

sal_uInt16 SwTxtNode::ResetAllAttr()
{
    const bool bOldIsSetOrResetAttr( mbInSetOrResetAttr );
    mbInSetOrResetAttr = true;

    HandleResetAttrAtTxtNode aHandleResetAttr( *this );

    sal_uInt16 nRet = SwCntntNode::ResetAllAttr();

    mbInSetOrResetAttr = bOldIsSetOrResetAttr;

    return nRet;
}


sal_uInt32 SwTxtNode::GetRsid( sal_Int32 nStt, sal_Int32 nEnd ) const
{
    SfxItemSet aSet( (SfxItemPool&) (GetDoc()->GetAttrPool()), RES_CHRATR_RSID, RES_CHRATR_RSID );
    if ( GetAttr(aSet, nStt, nEnd) )
    {
        SvxRsidItem* pRsid = (SvxRsidItem*)aSet.GetItem(RES_CHRATR_RSID);
        if( pRsid )
            return pRsid->GetValue();
    }

    return 0;
}

sal_uInt32 SwTxtNode::GetParRsid() const
{
    return reinterpret_cast<const SvxRsidItem&>(GetAttr( RES_PARATR_RSID )).GetValue();
}

bool SwTxtNode::CompareParRsid( const SwTxtNode &rTxtNode ) const
{
    sal_uInt32 nThisRsid = GetParRsid();
    sal_uInt32 nRsid = rTxtNode.GetParRsid();

    return nThisRsid == nRsid;
}

bool SwTxtNode::CompareRsid( const SwTxtNode &rTxtNode, sal_Int32 nStt1, sal_Int32 nStt2,
                            sal_Int32 nEnd1, sal_Int32 nEnd2 ) const

{
    sal_uInt32 nThisRsid = GetRsid( nStt1, nEnd1 ? nEnd1 : nStt1 );
    sal_uInt32 nRsid = rTxtNode.GetRsid( nStt2, nEnd2 ? nEnd2 : nStt2 );

    return nThisRsid == nRsid;
}


::sfx2::IXmlIdRegistry& SwTxtNode::GetRegistry()
{
    return GetDoc()->GetXmlIdRegistry();
}

bool SwTxtNode::IsInClipboard() const
{
    return GetDoc()->IsClipBoard();
}

bool SwTxtNode::IsInUndo() const
{
    return GetDoc()->GetIDocumentUndoRedo().IsUndoNodes(GetNodes());
}

bool SwTxtNode::IsInContent() const
{
    return !GetDoc()->IsInHeaderFooter( SwNodeIndex(*this) );
}

void SwTxtNode::SwClientNotify( const SwModify& rModify, const SfxHint& rHint )
{
    const SwAttrHint* pHint = dynamic_cast<const SwAttrHint*>(&rHint);
    if ( pHint && pHint->GetId() == RES_CONDTXTFMTCOLL && &rModify == GetRegisteredIn() )
        ChkCondColl();
}

#include <unoparagraph.hxx>

uno::Reference< rdf::XMetadatable >
SwTxtNode::MakeUnoObject()
{
    const uno::Reference<rdf::XMetadatable> xMeta(
            SwXParagraph::CreateXParagraph(*GetDoc(), *this), uno::UNO_QUERY);
    return xMeta;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
