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

#include <hintids.hxx>
#include <hints.hxx>

#include <editeng/fontitem.hxx>
#include <editeng/brkitem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/rsiditem.hxx>
#include <editeng/tstpitem.hxx>
#include <svl/urihelper.hxx>
#include <svl/ctloptions.hxx>
#include <tools/multisel.hxx>
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
#include <pam.hxx>                  // fuer SwPosition
#include <fldbas.hxx>
#include <paratr.hxx>
#include <txtfrm.hxx>
#include <ftnfrm.hxx>
#include <ftnboss.hxx>
#include <rootfrm.hxx>
#include <pagedesc.hxx>             // fuer SwPageDesc
#include <expfld.hxx>               // fuer SwTblField
#include <section.hxx>              // fuer SwSection
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

// Leider ist das SwpHints nicht ganz wasserdicht:
// Jeder darf an den Hints rumfummeln, ohne die Sortierreihenfolge
// und Verkettung sicherstellen zu muessen.
#ifdef DBG_UTIL
#define CHECK_SWPHINTS(pNd)  { if( pNd->GetpSwpHints() && \
                                   !pNd->GetDoc()->IsInReading() ) \
                                  pNd->GetpSwpHints()->Check(); }
#else
#define CHECK_SWPHINTS(pNd)
#endif

SwTxtNode *SwNodes::MakeTxtNode( const SwNodeIndex & rWhere,
                                 SwTxtFmtColl *pColl,
                                 SwAttrSet* pAutoAttr )
{
    OSL_ENSURE( pColl, "Collectionpointer ist 0." );

    SwTxtNode *pNode = new SwTxtNode( rWhere, pColl, pAutoAttr );

    SwNodeIndex aIdx( *pNode );

    // #125329#
    // call method <UpdateOutlineNode(..)> only for the document nodes array
    if ( IsDocNodes() )
        UpdateOutlineNode(*pNode);

    //Wenn es noch kein Layout gibt oder in einer versteckten Section
    // stehen, brauchen wir uns um das MakeFrms nicht bemuehen.
    const SwSectionNode* pSectNd;
    if( !GetDoc()->GetCurrentViewShell() || //swmod 071108//swmod 071225
        ( 0 != (pSectNd = pNode->FindSectionNode()) &&
            pSectNd->GetSection().IsHiddenFlag() ))
        return pNode;

    SwNodeIndex aTmp( rWhere );
    do {
        // max. 2 Durchlaeufe:
        // 1. den Nachfolger nehmen
        // 2. den Vorgaenger

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
                    if( !GoPrevSection( &aTmp, sal_True, sal_False ) ||
                        aTmp.GetNode().FindTableNode() !=
                            pNode->FindTableNode() )
                        return pNode;       // schade, das wars
                }
                else
                    aTmp = *pNd->StartOfSectionNode();
                break;
            }
            else if( pNd->StartOfSectionNode()->IsTableNode() &&
                    aTmp.GetIndex() < rWhere.GetIndex() )
            {
                // wir stehen hinter einem TabellenNode
                aTmp = *pNd->StartOfSectionNode();
                break;
            }
            // kein break !!!
        default:
            if( rWhere == aTmp )
                aTmp -= 2;
            else
                return pNode;
            break;
        }
    } while( sal_True );
}

// --------------------
// SwTxtNode
// --------------------

SwTxtNode::SwTxtNode( const SwNodeIndex &rWhere,
                      SwTxtFmtColl *pTxtColl,
                      const SfxItemSet* pAutoAttr )
    : SwCntntNode( rWhere, ND_TEXTNODE, pTxtColl ),
      m_pSwpHints( 0 ),
      mpNodeNum( 0 ),
      m_bLastOutlineState( false ),
      m_bNotifiable( false ),
      // #i70748#
      mbEmptyListStyleSetDueToSetOutlineLevelAttr( false ),
      mbInSetOrResetAttr( false ),
      mpList( 0 )
{
    InitSwParaStatistics( true );

    // soll eine Harte-Attributierung gesetzt werden?
    if( pAutoAttr )
        SetAttr( *pAutoAttr );

    if ( !IsInList() && GetNumRule() && GetListId().Len() > 0 )
    {
        // #i101516#
        // apply paragraph style's assigned outline style list level as
        // list level of the paragraph, if it has none set already.
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
    // delete loescht nur die Pointer, nicht die Arrayelemente!
    if ( m_pSwpHints )
    {
        // damit Attribute die ihren Inhalt entfernen nicht doppelt
        // geloescht werden.
        SwpHints* pTmpHints = m_pSwpHints;
        m_pSwpHints = 0;

        for( sal_uInt16 j = pTmpHints->Count(); j; )
            // erst muss das Attribut aus dem Array entfernt werden,
            // denn sonst wuerde es sich selbst loeschen (Felder) !!!!
            DestroyAttr( pTmpHints->GetTextHint( --j ) );

        delete pTmpHints;
    }

    // must be removed from outline nodes by now
#if OSL_DEBUG_LEVEL > 0
    sal_uInt16 foo;
    assert(!GetNodes().GetOutLineNds().Seek_Entry(this, &foo));
#endif

    RemoveFromList();

    InitSwParaStatistics( false );
}

SwCntntFrm *SwTxtNode::MakeFrm( SwFrm* pSib )
{
    SwCntntFrm *pFrm = new SwTxtFrm( this, pSib );
    return pFrm;
}

xub_StrLen SwTxtNode::Len() const
{
    return m_Text.Len();
}

/*---------------------------------------------------------------------------
 * lcl_ChangeFtnRef
 *  After a split node, it's necessary to actualize the ref-pointer of the
 *  ftnfrms.
 * --------------------------------------------------------------------------*/

static void lcl_ChangeFtnRef( SwTxtNode &rNode )
{
    SwpHints *pSwpHints = rNode.GetpSwpHints();
    if( pSwpHints && rNode.GetDoc()->GetCurrentViewShell() )    //swmod 071108//swmod 071225
    {
        SwTxtAttr* pHt;
        SwCntntFrm* pFrm = NULL;
        // OD 07.11.2002 #104840# - local variable to remember first footnote
        // of node <rNode> in order to invalidate position of its first content.
        // Thus, in its <MakeAll()> it will checked its position relative to its reference.
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
                            GetNodes().GoNextSection( &aIdx, sal_True, sal_False );
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
                        // #104840# - remember footnote frame
                        pFirstFtnOfNode = pFtn;
                        while ( pFtn )
                        {
                            pFtn->SetRef( pFrm );
                            pFtn = pFtn->GetFollow();
                            ((SwTxtFrm*)pFrm)->SetFtn( sal_True );
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
        } // end of for-loop on <SwpHints>
        // #104840# - invalidate
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

    // lege den Node "vor" mir an
    const xub_StrLen nSplitPos = rPos.nContent.GetIndex();
    const xub_StrLen nTxtLen = m_Text.Len();
    SwTxtNode* const pNode =
        _MakeNewTxtNode( rPos.nNode, sal_False, nSplitPos==nTxtLen );

    // the first paragraph gets the XmlId,
    // _except_ if it is empty and the second is not empty
    if (nSplitPos != 0) {
        pNode->RegisterAsCopyOf(*this, true);
        if (nSplitPos == nTxtLen)
        {
            this->RemoveMetadataReference();
            // NB: SwUndoSplitNode will call pNode->JoinNext,
            // which is sufficient even in this case!
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

    if ( GetDepends() && m_Text.Len() && (nTxtLen / 2) < nSplitPos )
    {
// JP 25.04.95: Optimierung fuer SplitNode:
//              Wird am Ende vom Node gesplittet, dann verschiebe die
//              Frames vom akt. auf den neuen und erzeuge fuer den akt.
//              neue. Dadurch entfaellt das neu aufbauen vom Layout.

        LockModify();   // Benachrichtigungen abschalten

        // werden FlyFrames mit verschoben, so muessen diese nicht ihre
        // Frames zerstoeren. Im SwTxtFly::SetAnchor wird es abgefragt!
        if ( HasHints() )
        {
            pNode->GetOrCreateSwpHints().SetInSplitNode(true);
        }

        //Ersten Teil des Inhalts in den neuen Node uebertragen und
        //im alten Node loeschen.
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

        // SMARTTAGS
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

            // alle zeichengebundenen Rahmen, die im neuen Absatz laden
            // muessen aus den alten Frame entfernt werden:
            // JP 01.10.96: alle leeren und nicht zu expandierenden
            //              Attribute loeschen
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
                        const xub_StrLen* const pEnd = pHt->GetEnd();
                        if (pEnd && *pHt->GetStart() == *pEnd )
                        {
                            // delete it!
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

        UnlockModify(); // Benachrichtigungen wieder freischalten

        // If there is an accessible layout we must call modify even
        // with length zero, because we have to notify about the changed
        // text node.
        const SwRootFrm *pRootFrm;
        if ( (nTxtLen != nSplitPos) ||
            ( (pRootFrm = pNode->GetDoc()->GetCurrentLayout()) != 0 &&
              pRootFrm->IsAnyShellAccessible() ) )  //swmod 080218
        {
            // dann sage den Frames noch, das am Ende etwas "geloescht" wurde
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
        pNode->MakeFrms( *this );       // neue Frames anlegen.
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

        // SMARTTAGS
        SwWrongList *pList2 = GetSmartTags();
        SetSmartTags( 0, false );
        SetSmartTagDirty( true );

        SwIndex aIdx( this );
        CutText( pNode, aIdx, nSplitPos );

        // JP 01.10.96: alle leeren und nicht zu expandierenden
        //              Attribute loeschen
        if ( HasHints() )
        {
            for ( sal_uInt16 j = m_pSwpHints->Count(); j; )
            {
                SwTxtAttr* const pHt = m_pSwpHints->GetTextHint( --j );
                const xub_StrLen* const pEnd = pHt->GetEnd();
                if ( pHt->DontExpand() && pEnd && (*pHt->GetStart() == *pEnd) )
                {
                    // delete it!
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

        // SMARTTAGS
        if( pList2 )
        {
            pNode->SetSmartTags( pList2->SplitList( nSplitPos ) );
            SetSmartTags( pList2, false );
        }

        if ( GetDepends() )
        {
            MakeFrms( *pNode );     // neue Frames anlegen.
        }
        lcl_ChangeFtnRef( *pNode );
    }

    {
        //Hint fuer Pagedesc versenden. Das mueste eigntlich das Layout im
        //Paste der Frames selbst erledigen, aber das fuehrt dann wiederum
        //zu weiteren Folgefehlern, die mit Laufzeitkosten geloest werden
        //muesten. #56977# #55001# #56135#
        const SfxPoolItem *pItem;
        if( GetDepends() && SFX_ITEM_SET == pNode->GetSwAttrSet().
            GetItemState( RES_PAGEDESC, sal_True, &pItem ) )
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

        const xub_StrLen* pHtEndIdx = pHt->GetEnd();

        if( !pHtEndIdx )
            continue;

        if ( *pHtEndIdx < m_Text.Len() || pHt->IsCharFmtAttr() )
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
        _SaveCntntIdx( pDoc, aIdx.GetIndex(), USHRT_MAX, aBkmkArr, SAVEFLY );
        SwTxtNode *pTxtNode = aIdx.GetNode().GetTxtNode();
        xub_StrLen nOldLen = m_Text.Len();

        // METADATA: merge
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

        // SMARTTAGS
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

        { // wg. SwIndex
            pTxtNode->CutText( this, SwIndex(pTxtNode), pTxtNode->Len() );
        }
        // verschiebe noch alle Bookmarks/TOXMarks
        if( !aBkmkArr.empty() )
            _RestoreCntntIdx( pDoc, aBkmkArr, GetIndex(), nOldLen );

        if( pTxtNode->HasAnyIndex() )
        {
            // alle Crsr/StkCrsr/UnoCrsr aus dem Loeschbereich verschieben
            pDoc->CorrAbs( aIdx, SwPosition( *this ), nOldLen, sal_True );
        }
        rNds.Delete(aIdx);
        SetWrong( pList, false );
        SetGrammarCheck( pList3, false );
        SetSmartTags( pList2, false ); // SMARTTAGS
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
        _SaveCntntIdx( pDoc, aIdx.GetIndex(), USHRT_MAX, aBkmkArr, SAVEFLY );
        SwTxtNode *pTxtNode = aIdx.GetNode().GetTxtNode();
        xub_StrLen nLen = pTxtNode->Len();

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

        // SMARTTAGS
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

        { // wg. SwIndex
            pTxtNode->CutText( this, SwIndex(this), SwIndex(pTxtNode), nLen );
        }
        // verschiebe noch alle Bookmarks/TOXMarks
        if( !aBkmkArr.empty() )
            _RestoreCntntIdx( pDoc, aBkmkArr, GetIndex() );

        if( pTxtNode->HasAnyIndex() )
        {
            // alle Crsr/StkCrsr/UnoCrsr aus dem Loeschbereich verschieben
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

// erzeugt einen AttrSet mit Bereichen fuer Frame-/Para/Char-Attributen
void SwTxtNode::NewAttrSet( SwAttrPool& rPool )
{
    OSL_ENSURE( !mpAttrSet.get(), "AttrSet ist doch gesetzt" );
    SwAttrSet aNewAttrSet( rPool, aTxtNodeSetRange );

    // put names of parent style and conditional style:
    const SwFmtColl* pAnyFmtColl = &GetAnyFmtColl();
    const SwFmtColl* pFmtColl = GetFmtColl();
    String sVal;
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


// override SwIndexReg::Update => text hints do not need SwIndex for start/end!
void SwTxtNode::Update( SwIndex const & rPos, const xub_StrLen nChangeLen,
                        const bool bNegative, const bool bDelete )
{
    SetAutoCompleteWordDirty( sal_True );

    ::std::auto_ptr<SwpHts> pCollector;
    const xub_StrLen nChangePos = rPos.GetIndex();

    if ( HasHints() )
    {
        if ( bNegative )
        {
            const xub_StrLen nChangeEnd = nChangePos + nChangeLen;
            for ( sal_uInt16 n = 0; n < m_pSwpHints->Count(); ++n )
            {
                SwTxtAttr * const pHint = m_pSwpHints->GetTextHint(n);
                xub_StrLen * const pStart = pHint->GetStart();
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
                }

                xub_StrLen * const pEnd = pHint->GetEnd();
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
                    }
                }
            }

            m_pSwpHints->MergePortions( *this );
        }
        else
        {
            bool bNoExp = false;
            bool bResort = false;
            const sal_uInt16 coArrSz = static_cast<sal_uInt16>(RES_TXTATR_WITHEND_END) -
                                   static_cast<sal_uInt16>(RES_CHRATR_BEGIN);

            sal_Bool aDontExp[ coArrSz ];
            memset( &aDontExp, 0, coArrSz * sizeof(sal_Bool) );

            for ( sal_uInt16 n = 0; n < m_pSwpHints->Count(); ++n )
            {
                SwTxtAttr * const pHint = m_pSwpHints->GetTextHint(n);
                xub_StrLen * const pStart = pHint->GetStart();
                xub_StrLen * const pEnd = pHint->GetEnd();
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
                    }
                    else // *pEnd == nChangePos
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
                             SwTxtAttr * const pTmp = MakeTxtAttr( *GetDoc(),
                                 pHint->GetAttr(),
                                 nChangePos, nChangePos + nChangeLen);
                             pCollector->push_back( pTmp );
                        }
                        else
                        {
                            *pEnd = *pEnd + nChangeLen;
                        }
                    }
                }
            }
            if ( bResort )
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
            SwRedline *const pRedl = rTbl[ i ];
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
                    // mst: FIXME: why does this adjust the unused position???
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

        const IDocumentMarkAccess* const pMarkAccess = getIDocumentMarkAccess();
        for(IDocumentMarkAccess::const_iterator_t ppMark =
                pMarkAccess->getMarksBegin();
            ppMark != pMarkAccess->getMarksEnd();
            ++ppMark)
        {
            // Bookmarks must never grow to either side, when
            // editing (directly) to the left or right (#i29942#)!
            // And a bookmark with same start and end must remain
            // to the left of the inserted text (used in XML import).
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

    // base class
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
    // erfrage die OutlineLevel und update gegebenenfalls das Nodes-Array,
    // falls sich die Level geaendert haben !
    const int nOldLevel = pOldColl && pOldColl->IsAssignedToListLevelOfOutlineStyle() ?
                     pOldColl->GetAssignedOutlineStyleLevel() : MAXLEVEL;
    const int nNewLevel = pNewColl && pNewColl->IsAssignedToListLevelOfOutlineStyle() ?
                     pNewColl->GetAssignedOutlineStyleLevel() : MAXLEVEL;

    if ( MAXLEVEL != nNewLevel )
    {
        SetAttrListLevel(nNewLevel);
    }

    {
        if (pDoc)
            pDoc->GetNodes().UpdateOutlineNode(*this);
    }


    SwNodes& rNds = GetNodes();
    // Update beim Level 0 noch die Fussnoten !!
    if( ( !nNewLevel || !nOldLevel) && !pDoc->GetFtnIdxs().empty() &&
        FTNNUM_CHAPTER == pDoc->GetFtnInfo().eNum &&
        rNds.IsDocNodes() )
    {
        SwNodeIndex aTmpIndex( rNds, GetIndex());

        pDoc->GetFtnIdxs().UpdateFtn( aTmpIndex);
    }

    if( RES_CONDTXTFMTCOLL == pNewColl->Which() )
    {
        // Erfrage die akt. Condition des TextNodes:
        ChkCondColl();
    }
}

// Wenn man sich genau am Ende einer Text- bzw. INetvorlage befindet,
// bekommt diese das DontExpand-Flag verpasst

sal_Bool SwTxtNode::DontExpandFmt( const SwIndex& rIdx, bool bFlag,
                                sal_Bool bFmtToTxtAttributes )
{
    const xub_StrLen nIdx = rIdx.GetIndex();
    if ( bFmtToTxtAttributes && nIdx == m_Text.Len() )
    {
        FmtToTxtAttr( this );
    }

    sal_Bool bRet = sal_False;
    if ( HasHints() )
    {
        const sal_uInt16 nEndCnt = m_pSwpHints->GetEndCount();
        sal_uInt16 nPos = nEndCnt;
        while( nPos )
        {
            SwTxtAttr *pTmp = m_pSwpHints->GetEnd( --nPos );
            xub_StrLen *pEnd = pTmp->GetEnd();
            if( !pEnd || *pEnd > nIdx )
                continue;
            if( nIdx != *pEnd )
                nPos = 0;
            else if( bFlag != pTmp->DontExpand() && !pTmp->IsLockExpandFlag()
                     && *pEnd > *pTmp->GetStart())
            {
                bRet = sal_True;
                m_pSwpHints->NoteInHistory( pTmp );
                pTmp->SetDontExpand( bFlag );
            }
        }
    }
    return bRet;
}

static bool lcl_GetTxtAttrDefault(xub_StrLen const nIndex,
    xub_StrLen const nHintStart, xub_StrLen const nHintEnd)
{
    return ((nHintStart <= nIndex) && (nIndex <  nHintEnd));
}
static bool lcl_GetTxtAttrExpand(xub_StrLen const nIndex,
    xub_StrLen const nHintStart, xub_StrLen const nHintEnd)
{
    return ((nHintStart <  nIndex) && (nIndex <= nHintEnd));
}
static bool lcl_GetTxtAttrParent(xub_StrLen const nIndex,
    xub_StrLen const nHintStart, xub_StrLen const nHintEnd)
{
    return ((nHintStart <  nIndex) && (nIndex <  nHintEnd));
}

static void
lcl_GetTxtAttrs(
    ::std::vector<SwTxtAttr *> *const pVector, SwTxtAttr **const ppTxtAttr,
    SwpHints *const pSwpHints,
    xub_StrLen const nIndex, RES_TXTATR const nWhich,
    enum SwTxtNode::GetTxtAttrMode const eMode)
{
    sal_uInt16 const nSize = (pSwpHints) ? pSwpHints->Count() : 0;
    xub_StrLen nPreviousIndex(0); // index of last hint with nWhich
    bool (*pMatchFunc)(xub_StrLen const, xub_StrLen const, xub_StrLen const)=0;
    switch (eMode)
    {
        case SwTxtNode::DEFAULT:   pMatchFunc = &lcl_GetTxtAttrDefault; break;
        case SwTxtNode::EXPAND:    pMatchFunc = &lcl_GetTxtAttrExpand;  break;
        case SwTxtNode::PARENT:    pMatchFunc = &lcl_GetTxtAttrParent;  break;
        default: OSL_ASSERT(false);
    }

    for( sal_uInt16 i = 0; i < nSize; ++i )
    {
        SwTxtAttr *const pHint = pSwpHints->GetTextHint(i);
        xub_StrLen const nHintStart( *(pHint->GetStart()) );
        if (nIndex < nHintStart)
        {
            return; // hints are sorted by start, so we are done...
        }

        if (pHint->Which() != nWhich)
        {
            continue;
        }

        xub_StrLen const*const pEndIdx = pHint->GetEnd();
        OSL_ENSURE(pEndIdx || pHint->HasDummyChar(),
                "hint with no end and no dummy char?");
            // Wenn bExpand gesetzt ist, wird das Verhalten bei Eingabe
            // simuliert, d.h. der Start wuede verschoben, das Ende expandiert,
        bool const bContained( (pEndIdx)
            ? (*pMatchFunc)(nIndex, nHintStart, *pEndIdx)
            : (nHintStart == nIndex) );
        if (bContained)
        {
            if (pVector)
            {
                if (nPreviousIndex < nHintStart)
                {
                    pVector->clear(); // clear hints that are outside pHint
                    nPreviousIndex = nHintStart;
                }
                pVector->push_back(pHint);
            }
            else
            {
                *ppTxtAttr = pHint; // and possibly overwrite outer hint
            }
            if (!pEndIdx)
            {
                break;
            }
        }
    }
}

::std::vector<SwTxtAttr *>
SwTxtNode::GetTxtAttrsAt(xub_StrLen const nIndex, RES_TXTATR const nWhich,
                        enum GetTxtAttrMode const eMode) const
{
    ::std::vector<SwTxtAttr *> ret;
    lcl_GetTxtAttrs(& ret, 0, m_pSwpHints, nIndex, nWhich, eMode);
    return ret;
}

SwTxtAttr *
SwTxtNode::GetTxtAttrAt(xub_StrLen const nIndex, RES_TXTATR const nWhich,
                        enum GetTxtAttrMode const eMode) const
{
    OSL_ENSURE(    (nWhich == RES_TXTATR_META)
            || (nWhich == RES_TXTATR_METAFIELD)
            || (nWhich == RES_TXTATR_AUTOFMT)
            || (nWhich == RES_TXTATR_INETFMT)
            || (nWhich == RES_TXTATR_CJK_RUBY)
            || (nWhich == RES_TXTATR_UNKNOWN_CONTAINER),
        "GetTxtAttrAt() will give wrong result for this hint!");

    SwTxtAttr * pRet(0);
    lcl_GetTxtAttrs(0, & pRet, m_pSwpHints, nIndex, nWhich, eMode);
    return pRet;
}

/*************************************************************************
 *                          CopyHint()
 *************************************************************************/

static SwCharFmt* lcl_FindCharFmt( const SwCharFmts* pCharFmts, const XubString& rName )
{
    if( rName.Len() )
    {
        SwCharFmt* pFmt;
        sal_uInt16 nArrLen = pCharFmts->size();
        for( sal_uInt16 i = 1; i < nArrLen; i++ )
        {
            pFmt = (*pCharFmts)[ i ];
            if( pFmt->GetName().CompareTo( rName ) == COMPARE_EQUAL )
                return pFmt;
        }
    }
    return NULL;
}

static void lcl_CopyHint( const sal_uInt16 nWhich, const SwTxtAttr * const pHt,
    SwTxtAttr *const pNewHt, SwDoc *const pOtherDoc, SwTxtNode *const pDest )
{
    OSL_ENSURE( nWhich == pHt->Which(), "Falsche Hint-Id" );
    switch( nWhich )
    {
        // copy nodesarray section with footnote content
        case RES_TXTATR_FTN :
            OSL_ENSURE(pDest, "lcl_CopyHint: no destination text node?");
            static_cast<const SwTxtFtn*>(pHt)->CopyFtn(
                *static_cast<SwTxtFtn*>(pNewHt), *pDest);
            break;

        // Beim Kopieren von Feldern in andere Dokumente
        // muessen die Felder bei ihren neuen Feldtypen angemeldet werden.

        // TabellenFormel muessen relativ kopiert werden.
        case RES_TXTATR_FIELD :
            {
                const SwFmtFld& rFld = pHt->GetFld();
                if( pOtherDoc )
                {
                    static_cast<const SwTxtFld*>(pHt)->CopyFld(
                        static_cast<SwTxtFld*>(pNewHt) );
                }

                // Tabellenformel ??
                if( RES_TABLEFLD == rFld.GetFld()->GetTyp()->Which()
                    && static_cast<const SwTblField*>(rFld.GetFld())->IsIntrnlName())
                {
                    // wandel die interne in eine externe Formel um
                    const SwTableNode* const pDstTblNd =
                        static_cast<const SwTxtFld*>(pHt)->
                                            GetTxtNode().FindTableNode();
                    if( pDstTblNd )
                    {
                        SwTblField* const pTblFld = const_cast<SwTblField*>(
                            static_cast<const SwTblField*>(
                                pNewHt->GetFld().GetFld()));
                        pTblFld->PtrToBoxNm( &pDstTblNd->GetTable() );
                    }
                }
            }
            break;

        case RES_TXTATR_TOXMARK :
            if( pOtherDoc && pDest && pDest->GetpSwpHints()
                && USHRT_MAX != pDest->GetpSwpHints()->GetPos( pNewHt ) )
            {
                // Beim Kopieren von TOXMarks(Client) in andere Dokumente
                // muss der Verzeichnis (Modify) ausgetauscht werden
                static_cast<SwTxtTOXMark*>(pNewHt)->CopyTOXMark( pOtherDoc );
            }
            break;

        case RES_TXTATR_CHARFMT :
            // Wenn wir es mit einer Zeichenvorlage zu tun haben,
            // muessen wir natuerlich auch die Formate kopieren.
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
            // Wenn wir es mit benutzerdefinierten INet-Zeichenvorlagen
            // zu tun haben, muessen wir natuerlich auch die Formate kopieren.
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
            //JP 24.04.98: Bug 49753 - ein TextNode muss am Attribut
            //              gesetzt sein, damit die Vorlagen erzeugt
            //              werden koenne
            SwTxtINetFmt* const pINetHt = static_cast<SwTxtINetFmt*>(pNewHt);
            if ( !pINetHt->GetpTxtNode() )
            {
                pINetHt->ChgTxtNode( pDest );
            }

            //JP 22.10.97: Bug 44875 - Verbindung zum Format herstellen
            pINetHt->GetCharFmt();
            break;
        }
        case RES_TXTATR_META:
        case RES_TXTATR_METAFIELD:
            OSL_ENSURE(pNewHt, "copying Meta should not fail!");
            OSL_ENSURE(pDest && (CH_TXTATR_BREAKWORD ==
                                pDest->GetTxt().GetChar(*pNewHt->GetStart())),
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

void SwTxtNode::CopyAttr( SwTxtNode *pDest, const xub_StrLen nTxtStartIdx,
                          const xub_StrLen nOldPos )
{
    if ( HasHints() )    // keine Attribute, keine Kekse
    {
        SwDoc* const pOtherDoc = (pDest->GetDoc() != GetDoc()) ?
                pDest->GetDoc() : 0;

        for ( sal_uInt16 i = 0; i < m_pSwpHints->Count(); i++ )
        {
            SwTxtAttr *const pHt = m_pSwpHints->GetTextHint(i);
            xub_StrLen const nAttrStartIdx = *pHt->GetStart();
            if ( nTxtStartIdx < nAttrStartIdx )
                break;      // ueber das Textende, da nLen == 0

            const xub_StrLen *const pEndIdx = pHt->GetEnd();
            if ( pEndIdx && !pHt->HasDummyChar() )
            {
                if( ( *pEndIdx > nTxtStartIdx ||
                      ( *pEndIdx == nTxtStartIdx &&
                        nAttrStartIdx == nTxtStartIdx ) ) )
                {
                    sal_uInt16 const nWhich = pHt->Which();
                    if ( RES_TXTATR_REFMARK != nWhich )
                    {
                        // attribute in the area => copy
                        SwTxtAttr *const pNewHt = pDest->InsertItem(
                                pHt->GetAttr(), nOldPos, nOldPos,
                                nsSetAttrMode::SETATTR_IS_COPY);
                        if ( pNewHt )
                        {
                            lcl_CopyHint( nWhich, pHt, pNewHt,
                                    pOtherDoc, pDest );
                        }
                    }
                    else if( !pOtherDoc ? GetDoc()->IsCopyIsMove()
                                        : 0 == pOtherDoc->GetRefMark(
                                        pHt->GetRefMark().GetRefName() ) )
                    {
                        pDest->InsertItem( pHt->GetAttr(), nOldPos, nOldPos,
                                nsSetAttrMode::SETATTR_IS_COPY);
                    }
                }
            }
        }
    }

    if( this != pDest )
    {
        // Frames benachrichtigen, sonst verschwinden die Ftn-Nummern
        SwUpdateAttr aHint( nOldPos, nOldPos, 0 );
        pDest->ModifyNotification( 0, &aHint );
    }
}

/*************************************************************************
|*  SwTxtNode::Copy()
|*  Beschreibung        kopiert Zeichen und Attibute in pDest,
|*                      wird angehaengt
*************************************************************************/

// #i96213#
// introduction of new optional parameter to control, if all attributes have to be copied.
void SwTxtNode::CopyText( SwTxtNode *const pDest,
                      const SwIndex &rStart,
                      const xub_StrLen nLen,
                      const bool bForceCopyOfAllAttrs )
{
    SwIndex aIdx( pDest, pDest->m_Text.Len() );
    CopyText( pDest, aIdx, rStart, nLen, bForceCopyOfAllAttrs );
}

// #i96213#
// introduction of new optional parameter to control, if all attributes have to be copied.
void SwTxtNode::CopyText( SwTxtNode *const pDest,
                      const SwIndex &rDestStart,
                      const SwIndex &rStart,
                      xub_StrLen nLen,
                      const bool bForceCopyOfAllAttrs )
{
    xub_StrLen nTxtStartIdx = rStart.GetIndex();
    xub_StrLen nDestStart = rDestStart.GetIndex();      // alte Pos merken

    if (pDest->GetDoc()->IsClipBoard() && this->GetNum())
    {
        // #i111677# cache expansion of source (for clipboard)
        pDest->m_pNumStringCache.reset(
            new ::rtl::OUString(this->GetNumString()));
    }

    if( !nLen )
    {
        // wurde keine Laenge angegeben, dann Kopiere die Attribute
        // an der Position rStart.
        CopyAttr( pDest, nTxtStartIdx, nDestStart );

        // harte Absatz umspannende Attribute kopieren
        if( HasSwAttrSet() )
        {
            // alle, oder nur die CharAttribute ?
            // #i96213#
            if ( !bForceCopyOfAllAttrs &&
                 ( nDestStart ||
                   pDest->HasSwAttrSet() ||
                   nLen != pDest->GetTxt().Len() ) )
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

    // 1. Text kopieren
    const xub_StrLen oldLen = pDest->m_Text.Len();
    //JP 15.02.96: Bug 25537 - Attributbehandlung am Ende fehlt! Darum
    //              ueber die InsertMethode den Text einfuegen und nicht
    //              selbst direkt
    pDest->InsertText( m_Text.Copy( nTxtStartIdx, nLen ), rDestStart,
                   IDocumentContentOperations::INS_EMPTYEXPAND );

    // um reale Groesse Updaten !
    nLen = pDest->m_Text.Len() - oldLen;
    if ( !nLen ) // string not longer?
        return;

    SwDoc* const pOtherDoc = (pDest->GetDoc() != GetDoc()) ?
            pDest->GetDoc() : 0;

    // harte Absatz umspannende Attribute kopieren
    if( HasSwAttrSet() )
    {
        // alle, oder nur die CharAttribute ?
        // #i96213#
        if ( !bForceCopyOfAllAttrs &&
             ( nDestStart ||
               pDest->HasSwAttrSet() ||
               nLen != pDest->GetTxt().Len() ) )
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

    // Ende erst jetzt holen, weil beim Kopieren in sich selbst der
    // Start-Index und alle Attribute vorher aktualisiert werden.
    nTxtStartIdx = rStart.GetIndex();
    const xub_StrLen nEnd = nTxtStartIdx + nLen;

    // 2. Attribute kopieren
    // durch das Attribute-Array, bis der Anfang des Geltungsbereiches
    // des Attributs hinter dem zu kopierenden Bereich liegt
    const sal_uInt16 nSize = m_pSwpHints ? m_pSwpHints->Count() : 0;

    // wird in sich selbst kopiert, dann kann beim Einfuegen ein
    // Attribut geloescht werden. Darum erst ins Tmp-Array kopieren und
    // dann erst ins eigene uebertragen.
    SwpHts aArr;

    // Del-Array fuer alle RefMarks ohne Ausdehnung
    SwpHts aRefMrkArr;

    sal_uInt16 nDeletedDummyChars(0);
        //Achtung: kann ungueltig sein!!
    for (sal_uInt16 n = 0; ( n < nSize ); ++n)
    {
        const xub_StrLen nAttrStartIdx = *(*m_pSwpHints)[n]->GetStart();
        if (!( nAttrStartIdx < nEnd))
            break;

        SwTxtAttr * const pHt = m_pSwpHints->GetTextHint(n);
        const xub_StrLen * const pEndIdx = pHt->GetEnd();
        const sal_uInt16 nWhich = pHt->Which();

        // JP 26.04.94: REFMARK's werden nie kopiert. Hat das Refmark aber
        //              keinen Bereich umspannt, so steht im Text ein 255
        //              dieses muss entfernt werden. Trick: erst kopieren,
        //              erkennen und sammeln, nach dem kopieren Loeschen.
        //              Nimmt sein Zeichen mit ins Grab !!
        // JP 14.08.95: Duerfen RefMarks gemovt werden?
        int bCopyRefMark = RES_TXTATR_REFMARK == nWhich && ( bUndoNodes ||
                           (!pOtherDoc ? GetDoc()->IsCopyIsMove()
                                      : 0 == pOtherDoc->GetRefMark(
                                        pHt->GetRefMark().GetRefName() )));

        if( pEndIdx && RES_TXTATR_REFMARK == nWhich && !bCopyRefMark )
        {
            continue;
        }

        xub_StrLen nAttrStt;
        xub_StrLen nAttrEnd;

        if( nAttrStartIdx < nTxtStartIdx )
        {
            // start is before selection
            // copy hints with end and CH_TXTATR only if dummy char is copied
            if ( pEndIdx && (*pEndIdx > nTxtStartIdx) && !pHt->HasDummyChar() )
            {
                // attribute with extent and the end is in the selection
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
            // start is in the selection
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
            // copy the hint here, but insert it later
            pNewHt = MakeTxtAttr( *GetDoc(), pHt->GetAttr(),
                    nAttrStt, nAttrEnd, COPY, pDest );

            lcl_CopyHint(nWhich, pHt, pNewHt, 0, pDest);
            aArr.push_back( pNewHt );
        }
        else
        {
            pNewHt = pDest->InsertItem( pHt->GetAttr(), nAttrStt - nDeletedDummyChars,
                nAttrEnd - nDeletedDummyChars,
                      nsSetAttrMode::SETATTR_NOTXTATRCHR
                    | nsSetAttrMode::SETATTR_IS_COPY);
            if (pNewHt)
            {
                lcl_CopyHint( nWhich, pHt, pNewHt, pOtherDoc, pDest );
            }
            else if (pHt->HasDummyChar())
            {
                // The attribute that has failed to be copied would insert
                // dummy char, so positions of the following attributes have
                // to be shifted by one to compensate for that missing char.
                ++nDeletedDummyChars;
            }
        }

        if( RES_TXTATR_REFMARK == nWhich && !pEndIdx && !bCopyRefMark )
        {
            aRefMrkArr.push_back( pNewHt );
        }
    }

    // nur falls im Array Attribute stehen (kann nur beim Kopieren
    // sich selbst passieren!!)
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


void SwTxtNode::InsertText( const XubString & rStr, const SwIndex & rIdx,
        const IDocumentContentOperations::InsertFlags nMode )
{
    OSL_ENSURE( rIdx <= m_Text.Len(), "SwTxtNode::InsertText: invalid index." );
    OSL_ENSURE( (sal_uLong)m_Text.Len() + (sal_uLong)rStr.Len() <= STRING_LEN,
            "SwTxtNode::InsertText: node text with insertion > STRING_LEN." );

    xub_StrLen aPos = rIdx.GetIndex();
    xub_StrLen nLen = m_Text.Len() - aPos;
    m_Text.Insert( rStr, aPos );
    nLen = m_Text.Len() - aPos - nLen;

    if ( !nLen ) return;

    bool bOldExpFlg = IsIgnoreDontExpand();
    if (nMode & IDocumentContentOperations::INS_FORCEHINTEXPAND)
    {
        SetIgnoreDontExpand( true );
    }

    Update( rIdx, nLen ); // text content changed!

    if (nMode & IDocumentContentOperations::INS_FORCEHINTEXPAND)
    {
        SetIgnoreDontExpand( bOldExpFlg );
    }

    // analog zu Insert(char) in txtedt.cxx:
    // 1) bei bHintExp leere Hints an rIdx.GetIndex suchen und aufspannen
    // 2) bei bHintExp == sal_False mitgezogene Feldattribute zuruecksetzen

    if ( HasHints() )
    {
        for ( sal_uInt16 i = 0; i < m_pSwpHints->Count() &&
                rIdx >= *(*m_pSwpHints)[i]->GetStart(); ++i )
        {
            SwTxtAttr * const pHt = m_pSwpHints->GetTextHint( i );
            xub_StrLen * const pEndIdx = pHt->GetEnd();
            if( !pEndIdx )
                continue;

            if( rIdx == *pEndIdx )
            {
                if (  (nMode & IDocumentContentOperations::INS_NOHINTEXPAND) ||
                    (!(nMode & IDocumentContentOperations::INS_FORCEHINTEXPAND)
                     && pHt->DontExpand()) )
                {
                    // bei leeren Attributen auch Start veraendern
                    if( rIdx == *pHt->GetStart() )
                        *pHt->GetStart() = *pHt->GetStart() - nLen;
                    *pEndIdx = *pEndIdx - nLen;
                    m_pSwpHints->DeleteAtPos(i);
                    InsertHint( pHt, nsSetAttrMode::SETATTR_NOHINTADJUST );
                }
                // empty hints at insert position?
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
                // Kein Feld, am Absatzanfang, HintExpand
                m_pSwpHints->DeleteAtPos(i);
                *pHt->GetStart() = *pHt->GetStart() - nLen;
                InsertHint( pHt, nsSetAttrMode::SETATTR_NOHINTADJUST );
            }
        }
        TryDeleteSwpHints();
    }

    if ( GetDepends() )
    {
        SwInsTxt aHint( aPos, nLen );
        NotifyClients( 0, &aHint );
    }

    // By inserting a character, the hidden flags
    // at the TxtNode can become invalid:
    SetCalcHiddenCharFlags();

    CHECK_SWPHINTS(this);
}

/*************************************************************************
|*
|*  SwTxtNode::Cut()
|*
|*  Beschreibung        text.doc
|*
*************************************************************************/

void SwTxtNode::CutText( SwTxtNode * const pDest,
            const SwIndex & rStart, const xub_StrLen nLen )
{
    if(pDest)
    {
        SwIndex aDestStt( pDest, pDest->GetTxt().Len() );
        CutImpl( pDest, aDestStt, rStart, nLen, false );
    }
    else
    {
        OSL_FAIL("mst: entering dead and bitrotted code; fasten your seatbelts!");
        EraseText( rStart, nLen );
    }
}


void SwTxtNode::CutImpl( SwTxtNode * const pDest, const SwIndex & rDestStart,
         const SwIndex & rStart, xub_StrLen nLen, const bool bUpdate )
{
    if(!pDest)
    {
        OSL_FAIL("mst: entering dead and bitrotted code; fasten your seatbelts!");
        EraseText( rStart, nLen );
        return;
    }

    // nicht im Dokument verschieben ?
    if( GetDoc() != pDest->GetDoc() )
    {
        OSL_FAIL("mst: entering dead and bitrotted code; fasten your seatbelts!");
        CopyText( pDest, rDestStart, rStart, nLen);
        EraseText(rStart, nLen);
        return;
    }

    if( !nLen )
    {
        // wurde keine Laenge angegeben, dann Kopiere die Attribute
        // an der Position rStart.
        CopyAttr( pDest, rStart.GetIndex(), rDestStart.GetIndex() );
        return;
    }

    xub_StrLen nTxtStartIdx = rStart.GetIndex();
    xub_StrLen nDestStart = rDestStart.GetIndex();      // alte Pos merken
    const xub_StrLen nInitSize = pDest->m_Text.Len();

    // wird in sich selbst verschoben, muss es gesondert behandelt werden !!
    if( pDest == this )
    {
        OSL_FAIL("mst: entering dead and bitrotted code; fasten your seatbelts!");
        m_Text.Insert( m_Text, nTxtStartIdx, nLen, nDestStart );
        m_Text.Erase( nTxtStartIdx + (nDestStart<nTxtStartIdx ? nLen : 0), nLen );

        const xub_StrLen nEnd = rStart.GetIndex() + nLen;

        // dann suche mal alle Attribute zusammen, die im verschobenen
        // Bereich liegen. Diese werden in das extra Array verschoben,
        // damit sich die Indizies beim Updaten nicht veraendern !!!
        SwpHts aArr;

        // 2. Attribute verschieben
        // durch das Attribute-Array, bis der Anfang des Geltungsbereiches
        // des Attributs hinter dem zu verschiebenden Bereich liegt
        sal_uInt16 nAttrCnt = 0;
        while ( m_pSwpHints && nAttrCnt < m_pSwpHints->Count() )
        {
            SwTxtAttr * const pHt = m_pSwpHints->GetTextHint(nAttrCnt);
            const xub_StrLen nAttrStartIdx = *pHt->GetStart();
            if (!( nAttrStartIdx < nEnd ))
                break;
            const xub_StrLen * const pEndIdx = pHt->GetEnd();
            const sal_uInt16 nWhich = pHt->Which();
            SwTxtAttr *pNewHt = 0;

            if(nAttrStartIdx < nTxtStartIdx)
            {
                // Anfang liegt vor dem Bereich
                if ( RES_TXTATR_REFMARK != nWhich && !pHt->HasDummyChar() &&
                    pEndIdx && *pEndIdx > nTxtStartIdx )
                {
                    // Attribut mit einem Bereich
                    // und das Ende des Attribut liegt im Bereich
                    pNewHt = MakeTxtAttr( *GetDoc(), pHt->GetAttr(), 0,
                                        *pEndIdx > nEnd
                                            ? nLen
                                            : *pEndIdx - nTxtStartIdx );
                }
            }
            else
            {
                // der Anfang liegt vollstaendig im Bereich
                if( !pEndIdx || *pEndIdx < nEnd )
                {
                    // Attribut verschieben
                    m_pSwpHints->Delete( pHt );
                    // die Start/End Indicies neu setzen
                    *pHt->GetStart() = nAttrStartIdx - nTxtStartIdx;
                    if( pEndIdx )
                        *pHt->GetEnd() = *pEndIdx - nTxtStartIdx;
                    aArr.push_back( pHt );
                    continue;           // while-Schleife weiter, ohne ++ !
                }
                    // das Ende liegt dahinter
                else if (RES_TXTATR_REFMARK != nWhich && !pHt->HasDummyChar())
                {
                    pNewHt = MakeTxtAttr( *GetDoc(), pHt->GetAttr(),
                            nAttrStartIdx - nTxtStartIdx,
                            !pEndIdx ? 0
                                     : ( *pEndIdx > nEnd
                                            ? nLen
                                            : *pEndIdx - nTxtStartIdx ));
                }
            }
            if( pNewHt )
            {
                // die Daten kopieren
                lcl_CopyHint( nWhich, pHt, pNewHt, 0, this );
                aArr.push_back( pNewHt );
            }
            ++nAttrCnt;
        }

        if( bUpdate )
        {
            // Update aller Indizies
            Update( rDestStart, nLen, sal_False, sal_True );
        }

        CHECK_SWPHINTS(this);

        Update( rStart, nLen, sal_True, sal_True );

        CHECK_SWPHINTS(this);

        // dann setze die kopierten/geloeschten Attribute in den Node
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
            xub_StrLen * const pEndIdx = pNewHt->GetEnd();
            if ( pEndIdx )
            {
                *pEndIdx = nDestStart + *pEndIdx;
            }
            InsertHint( pNewHt, nsSetAttrMode::SETATTR_NOTXTATRCHR );
        }
    }
    else
    {
        pDest->m_Text.Insert( m_Text, nTxtStartIdx, nLen, nDestStart );
        m_Text.Erase( nTxtStartIdx, nLen );
        nLen = pDest->m_Text.Len() - nInitSize; // update w/ current size!
        if( !nLen )                 // String nicht gewachsen ??
            return;

        if( bUpdate )
        {
            // Update aller Indizies
            pDest->Update( rDestStart, nLen, sal_False, sal_True);
        }

        CHECK_SWPHINTS(pDest);

        const xub_StrLen nEnd = rStart.GetIndex() + nLen;
        SwDoc* const pOtherDoc = (pDest->GetDoc() != GetDoc())
            ? pDest->GetDoc() : 0;
        bool const bUndoNodes = !pOtherDoc
            && GetDoc()->GetIDocumentUndoRedo().IsUndoNodes(GetNodes());

        OSL_ENSURE(!pOtherDoc,
            "mst: entering dead and bitrotted code; fasten your seatbelts!");

        // harte Absatz umspannende Attribute kopieren
        if( HasSwAttrSet() )
        {
            // alle, oder nur die CharAttribute ?
            if( nInitSize || pDest->HasSwAttrSet() ||
                nLen != pDest->GetTxt().Len() )
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

        // 2. Attribute verschieben
        // durch das Attribute-Array, bis der Anfang des Geltungsbereiches
        // des Attributs hinter dem zu verschiebenden Bereich liegt
        sal_uInt16 nAttrCnt = 0;
        while ( m_pSwpHints && (nAttrCnt < m_pSwpHints->Count()) )
        {
            SwTxtAttr * const pHt = m_pSwpHints->GetTextHint(nAttrCnt);
            const xub_StrLen nAttrStartIdx = *pHt->GetStart();
            if (!( nAttrStartIdx < nEnd ))
                break;
            const xub_StrLen * const pEndIdx = pHt->GetEnd();
            const sal_uInt16 nWhich = pHt->Which();
            SwTxtAttr *pNewHt = 0;

            // if the hint has a dummy character, then it must not be split!
            if(nAttrStartIdx < nTxtStartIdx)
            {
                // Anfang liegt vor dem Bereich
                if( !pHt->HasDummyChar() && ( RES_TXTATR_REFMARK != nWhich
                    || bUndoNodes ) && pEndIdx && *pEndIdx > nTxtStartIdx )
                {
                    // Attribut mit einem Bereich
                    // und das Ende des Attribut liegt im Bereich
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
                // der Anfang liegt vollstaendig im Bereich
                if( !pEndIdx || *pEndIdx < nEnd ||
                    (!pOtherDoc && !bUndoNodes && RES_TXTATR_REFMARK == nWhich)
                    || pHt->HasDummyChar() )
                {
                    // do not delete note and later add it -> sidebar flickering
                    if ( GetDoc()->GetDocShell() )
                    {
                        GetDoc()->GetDocShell()->Broadcast( SfxSimpleHint(SFX_HINT_USER04));
                    }
                    // Attribut verschieben
                    m_pSwpHints->Delete( pHt );
                    // die Start/End Indicies neu setzen
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
                    continue;           // while-Schleife weiter, ohne ++ !
                }
                    // das Ende liegt dahinter
                else if( RES_TXTATR_REFMARK != nWhich || bUndoNodes )
                {
                    pNewHt = MakeTxtAttr( *GetDoc(), pHt->GetAttr(),
                            nDestStart + (nAttrStartIdx - nTxtStartIdx),
                            !pEndIdx ? 0
                                     : nDestStart + ( *pEndIdx > nEnd
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
        // sollten jetzt noch leere Attribute rumstehen, dann haben diese
        // eine hoehere Praezedenz. Also herausholen und das Array updaten.
        // Die dabei entstehenden leeren Hints werden von den gesicherten
        // "uebergeplaettet".   (Bug: 6977)
        if( m_pSwpHints && nAttrCnt < m_pSwpHints->Count() )
        {
            SwpHts aArr;
            while ( nAttrCnt < m_pSwpHints->Count() )
            {
                SwTxtAttr * const pHt = m_pSwpHints->GetTextHint(nAttrCnt);
                if ( nEnd != *pHt->GetStart() )
                    break;
                const xub_StrLen * const pEndIdx = pHt->GetEnd();
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
            Update( rStart, nLen, sal_True, sal_True );

            for ( sal_uInt16 n = 0; n < aArr.size(); ++n )
            {
                SwTxtAttr * const pHt = aArr[ n ];
                *pHt->GetStart() = *pHt->GetEnd() = rStart.GetIndex();
                InsertHint( pHt );
            }
        }
        else
        {
            Update( rStart, nLen, sal_True, sal_True );
        }

        CHECK_SWPHINTS(this);
    }

    TryDeleteSwpHints();

    // Frames benachrichtigen;
    SwInsTxt aInsHint( nDestStart, nLen );
    pDest->ModifyNotification( 0, &aInsHint );
    SwDelTxt aDelHint( nTxtStartIdx, nLen );
    ModifyNotification( 0, &aDelHint );
}


void SwTxtNode::EraseText(const SwIndex &rIdx, const xub_StrLen nCount,
        const IDocumentContentOperations::InsertFlags nMode )
{
    OSL_ENSURE( rIdx <= m_Text.Len(), "SwTxtNode::EraseText: invalid index." );

    const xub_StrLen nStartIdx = rIdx.GetIndex();
    const xub_StrLen nCnt = (STRING_LEN == nCount)
                      ? m_Text.Len() - nStartIdx : nCount;
    const xub_StrLen nEndIdx = nStartIdx + nCnt;
    m_Text.Erase( nStartIdx, nCnt );

    /* GCAttr(); alle leeren weggwerfen ist zu brutal.
     * Es duerfen nur die wegggeworfen werden,
     * die im Bereich liegen und nicht am Ende des Bereiches liegen
     */

    for ( sal_uInt16 i = 0; m_pSwpHints && i < m_pSwpHints->Count(); ++i )
    {
        SwTxtAttr *pHt = m_pSwpHints->GetTextHint(i);

        const xub_StrLen nHintStart = *pHt->GetStart();

        if ( nHintStart < nStartIdx )
            continue;

        if ( nHintStart > nEndIdx )
            break; // hints are sorted by end, so break here

        const xub_StrLen* pHtEndIdx = pHt->GetEnd();
        const sal_uInt16 nWhich = pHt->Which();

        if( !pHtEndIdx )
        {
            OSL_ENSURE(pHt->HasDummyChar(),
                    "attribute with neither end nor CH_TXTATR?");
            if (isTXTATR(nWhich) &&
                (nHintStart >= nStartIdx) && (nHintStart < nEndIdx))
            {
                m_pSwpHints->DeleteAtPos(i);
                DestroyAttr( pHt );
                --i;
            }
            continue;
        }

       OSL_ENSURE(!( (nHintStart < nEndIdx) && (*pHtEndIdx > nEndIdx)
                    && pHt->HasDummyChar() )
                // next line: deleting exactly dummy char: DeleteAttributes
                || ((nHintStart == nStartIdx) && (nHintStart + 1 == nEndIdx)),
                "ERROR: deleting left-overlapped attribute with CH_TXTATR");

        // Delete the hint if:
        // 1. The hint ends before the deletion end position or
        // 2. The hint ends at the deletion end position and
        //    we are not in empty expand mode and
        //    the hint is a [toxmark|refmark|ruby] text attribute
        // 3. deleting exactly the dummy char of an hint with end and dummy
        //    char deletes the hint
        if (   (*pHtEndIdx < nEndIdx)
            || ( (*pHtEndIdx == nEndIdx)     &&
                 !(IDocumentContentOperations::INS_EMPTYEXPAND & nMode)  &&
                 (  (RES_TXTATR_TOXMARK == nWhich)  ||
                    (RES_TXTATR_REFMARK == nWhich)  ||
                 // #i62668# Ruby text attribute must be
                 // treated just like toxmark and refmarks
                    (RES_TXTATR_CJK_RUBY == nWhich) ) )
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

    Update( rIdx, nCnt, sal_True );

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

    // By deleting a character, the hidden flags
    // at the TxtNode can become invalid:
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
    sal_uInt16 nMin = m_Text.Len(),
           nMax = 0;
    sal_Bool bAll = nMin != 0; // Bei leeren Absaetzen werden nur die
                           // INet-Formate entfernt.

    for ( sal_uInt16 i = 0; m_pSwpHints && i < m_pSwpHints->Count(); ++i )
    {
        SwTxtAttr * const pHt = m_pSwpHints->GetTextHint(i);

        // wenn Ende und Start gleich sind --> loeschen
        const xub_StrLen * const pEndIdx = pHt->GetEnd();
        if (pEndIdx && !pHt->HasDummyChar() && (*pEndIdx == *pHt->GetStart())
            && ( bAll || pHt->Which() == RES_TXTATR_INETFMT ) )
        {
            bChanged = true;
            nMin = Min( nMin, *pHt->GetStart() );
            nMax = Max( nMax, *pHt->GetEnd() );
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
        //TxtFrm's reagieren auf aHint, andere auf aNew
        SwUpdateAttr aHint( nMin, nMax, 0 );
        NotifyClients( 0, &aHint );
        SwFmtChg aNew( GetTxtColl() );
        NotifyClients( 0, &aNew );
    }
}

// #i23726#
SwNumRule* SwTxtNode::_GetNumRule(sal_Bool bInParent) const
{
    SwNumRule* pRet = 0;

    const SfxPoolItem* pItem = GetNoCondAttr( RES_PARATR_NUMRULE, bInParent );
    bool bNoNumRule = false;
    if ( pItem )
    {
        String sNumRuleName = static_cast<const SwNumRuleItem *>(pItem)->GetValue();
        if (sNumRuleName.Len() > 0)
        {
            pRet = GetDoc()->FindNumRulePtr( sNumRuleName );
        }
        else // numbering is turned off
            bNoNumRule = true;
    }

    if ( !bNoNumRule )
    {
        if ( pRet && pRet == GetDoc()->GetOutlineNumRule() &&
             ( !HasSwAttrSet() ||
               SFX_ITEM_SET !=
                GetpSwAttrSet()->GetItemState( RES_PARATR_NUMRULE, sal_False ) ) )
        {
            SwTxtFmtColl* pColl = GetTxtColl();
            if ( pColl )
            {
                const SwNumRuleItem& rDirectItem = pColl->GetNumRule( sal_False );
                if ( rDirectItem.GetValue().Len() == 0 )
                {
                    pRet = 0L;
                }
            }
        }
    }

    return pRet;
}

SwNumRule* SwTxtNode::GetNumRule(sal_Bool bInParent) const
{
    SwNumRule * pRet = _GetNumRule(bInParent);

    return pRet;
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

    // Sending "noop" modify in order to cause invalidations of registered
    // <SwTxtFrm> instances to get the list style change respectively the change
    // in the list tree reflected in the layout.
    // Important note:
    {
        SvxLRSpaceItem& rLR = (SvxLRSpaceItem&)GetSwAttrSet().GetLRSpace();
        NotifyClients( &rLR, &rLR );
    }

    SetWordCountDirty( true );
}

// -> #i27615#
sal_Bool SwTxtNode::IsNumbered() const
{
    sal_Bool bResult = sal_False;

    SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : 0L;
    if ( pRule && IsCountedInList() )
        bResult = sal_True;

    return bResult;
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
// <- #i27615#

SwTxtNode* SwTxtNode::_MakeNewTxtNode( const SwNodeIndex& rPos, sal_Bool bNext,
                                        sal_Bool bChgFollow )
{
    /* hartes PageBreak/PageDesc/ColumnBreak aus AUTO-Set ignorieren */
    SwAttrSet* pNewAttrSet = 0;
    // #i75353#
    bool bClearHardSetNumRuleWhenFmtCollChanges( false );
    if( HasSwAttrSet() )
    {
        pNewAttrSet = new SwAttrSet( *GetpSwAttrSet() );
        const SfxItemSet* pTmpSet = GetpSwAttrSet();

        if( bNext )     // der naechste erbt keine Breaks!
            pTmpSet = pNewAttrSet;

        // PageBreaks/PageDesc/ColBreak rausschmeissen.
        sal_Bool bRemoveFromCache = sal_False;
        std::vector<sal_uInt16> aClearWhichIds;
        if ( bNext )
            bRemoveFromCache = ( 0 != pNewAttrSet->ClearItem( RES_PAGEDESC ) );
        else
            aClearWhichIds.push_back( RES_PAGEDESC );

        if( SFX_ITEM_SET == pTmpSet->GetItemState( RES_BREAK, sal_False ) )
        {
            if ( bNext )
                pNewAttrSet->ClearItem( RES_BREAK );
            else
                aClearWhichIds.push_back( RES_BREAK );
            bRemoveFromCache = sal_True;
        }
        if( SFX_ITEM_SET == pTmpSet->GetItemState( RES_KEEP, sal_False ) )
        {
            if ( bNext )
                pNewAttrSet->ClearItem( RES_KEEP );
            else
                aClearWhichIds.push_back( RES_KEEP );
            bRemoveFromCache = sal_True;
        }
        if( SFX_ITEM_SET == pTmpSet->GetItemState( RES_PARATR_SPLIT, sal_False ) )
        {
            if ( bNext )
                pNewAttrSet->ClearItem( RES_PARATR_SPLIT );
            else
                aClearWhichIds.push_back( RES_PARATR_SPLIT );
            bRemoveFromCache = sal_True;
        }
        if(SFX_ITEM_SET == pTmpSet->GetItemState(RES_PARATR_NUMRULE, sal_False))
        {
            SwNumRule * pRule = GetNumRule();

            if (pRule && IsOutline())
            {
                if ( bNext )
                    pNewAttrSet->ClearItem(RES_PARATR_NUMRULE);
                else
                {
                    // #i75353#
                    // No clear of hard set numbering rule at an outline paragraph at this point.
                    // Only if the paragraph style changes - see below.
                    bClearHardSetNumRuleWhenFmtCollChanges = true;
                }
                bRemoveFromCache = sal_True;
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
    if( pRule && pRule == pNode->GetNumRule() && rNds.IsDocNodes() ) // #115901#
    {
        // #i55459#
        // - correction: parameter <bNext> has to be checked, as it was in the
        //   previous implementation.
        if ( !bNext && !IsCountedInList() )
            SetCountedInList(true);
    }

    // jetzt kann es sein, das durch die Nummerierung dem neuen Node eine
    // Vorlage aus dem Pool zugewiesen wurde. Dann darf diese nicht
    // nochmal uebergeplaettet werden !!
    if( pColl != pNode->GetTxtColl() ||
        ( bChgFollow && pColl != GetTxtColl() ))
        return pNode;       // mehr duerfte nicht gemacht werden oder ????

    pNode->_ChgTxtCollUpdateNum( 0, pColl ); // fuer Nummerierung/Gliederung
    if( bNext || !bChgFollow )
        return pNode;

    SwTxtFmtColl *pNextColl = &pColl->GetNextTxtFmtColl();
    // #i101870#
    // perform action on different paragraph styles before applying the new paragraph style
    if (pNextColl != pColl)
    {
        // #i75353#
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
    // Position hinter dem eingefuegt wird
    SwNodeIndex aIdx( rPos.nNode, 1 );
    SwTxtNode* pNew = _MakeNewTxtNode( aIdx, sal_True );

    // reset list attributes at appended text node
    pNew->ResetAttr( RES_PARATR_LIST_ISRESTART );
    pNew->ResetAttr( RES_PARATR_LIST_RESTARTVALUE );
    pNew->ResetAttr( RES_PARATR_LIST_ISCOUNTED );
    if ( pNew->GetNumRule() == 0 )
    {
        pNew->ResetAttr( RES_PARATR_LIST_ID );
        pNew->ResetAttr( RES_PARATR_LIST_LEVEL );
    }

    if ( !IsInList() && GetNumRule() && GetListId().Len() > 0 )
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

SwTxtAttr * SwTxtNode::GetTxtAttrForCharAt( const xub_StrLen nIndex,
    const RES_TXTATR nWhich ) const
{
    if ( HasHints() )
    {
        for ( sal_uInt16 i = 0; i < m_pSwpHints->Count(); ++i )
        {
            SwTxtAttr * const pHint = m_pSwpHints->GetTextHint(i);
            const xub_StrLen nStartPos = *pHint->GetStart();
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

// -> #i29560#
sal_Bool SwTxtNode::HasNumber() const
{
    sal_Bool bResult = sal_False;

    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : 0L;
    if ( pRule )
    {
        SwNumFmt aFmt(pRule->Get( static_cast<sal_uInt16>(GetActualListLevel())));

        // #i40041#
        bResult = aFmt.IsEnumeration() &&
            SVX_NUM_NUMBER_NONE != aFmt.GetNumberingType();
    }

    return bResult;
}

sal_Bool SwTxtNode::HasBullet() const
{
    sal_Bool bResult = sal_False;

    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : 0L;
    if ( pRule )
    {
        SwNumFmt aFmt(pRule->Get( static_cast<sal_uInt16>(GetActualListLevel())));

        bResult = aFmt.IsItemize();
    }

    return bResult;
}
// <- #i29560#

// #128041# - introduce parameter <_bInclPrefixAndSuffixStrings>
//i53420 added max outline parameter
XubString SwTxtNode::GetNumString( const bool _bInclPrefixAndSuffixStrings, const unsigned int _nRestrictToThisLevel ) const
{
    if (GetDoc()->IsClipBoard() && m_pNumStringCache.get())
    {
        // #i111677# do not expand number strings in clipboard documents
        return *m_pNumStringCache;
    }
    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : 0L;
    if ( pRule &&
         IsCountedInList() )
    {
        SvxNumberType const& rNumberType(
                pRule->Get( static_cast<sal_uInt16>(GetActualListLevel()) ) );
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

    return aEmptyStr;
}

long SwTxtNode::GetLeftMarginWithNum( sal_Bool bTxtLeft ) const
{
    long nRet = 0;
    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : 0L;
    if( pRule )
    {
        const SwNumFmt& rFmt = pRule->Get(static_cast<sal_uInt16>(GetActualListLevel()));

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
                // #i90401#
                // Only negative first line indents have consider for the left margin
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

sal_Bool SwTxtNode::GetFirstLineOfsWithNum( short& rFLOffset ) const
{
    sal_Bool bRet( sal_False );
    // #i95907#
    rFLOffset = 0;

    // #i51089#
    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : 0L;
    if ( pRule )
    {
        if ( IsCountedInList() )
        {
            const SwNumFmt& rFmt = pRule->Get(static_cast<sal_uInt16>(GetActualListLevel()));
            if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                rFLOffset = pRule->Get( static_cast<sal_uInt16>(GetActualListLevel() )).GetFirstLineOffset();

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

        bRet = sal_True;
    }
    else
    {
        rFLOffset = GetSwAttrSet().GetLRSpace().GetTxtFirstLineOfst();
    }

    return bRet;
}

//
SwTwips SwTxtNode::GetAdditionalIndentForStartingNewList() const
{
    SwTwips nAdditionalIndent = 0;

    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : 0L;
    if ( pRule )
    {
        const SwNumFmt& rFmt = pRule->Get(static_cast<sal_uInt16>(GetActualListLevel()));
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

// #i96772#
void SwTxtNode::ClearLRSpaceItemDueToListLevelIndents( SvxLRSpaceItem& o_rLRSpaceItem ) const
{
    if ( AreListLevelIndentsApplicable() )
    {
        const SwNumRule* pRule = GetNumRule();
        if ( pRule && GetActualListLevel() >= 0 )
        {
            const SwNumFmt& rFmt = pRule->Get(static_cast<sal_uInt16>(GetActualListLevel()));
            if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                o_rLRSpaceItem = aLR;
            }
        }
    }
}

// #i91133#
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

void SwTxtNode::Replace0xFF( XubString& rTxt, xub_StrLen& rTxtStt,
                            xub_StrLen nEndPos, sal_Bool bExpandFlds ) const
{
    if( GetpSwpHints() )
    {
        sal_Unicode cSrchChr = CH_TXTATR_BREAKWORD;
        for( int nSrchIter = 0; 2 > nSrchIter; ++nSrchIter,
                                cSrchChr = CH_TXTATR_INWORD )
        {
            xub_StrLen nPos = rTxt.Search( cSrchChr );
            while( STRING_NOTFOUND != nPos && nPos < nEndPos )
            {
                const SwTxtAttr* const pAttr =
                    GetTxtAttrForCharAt( rTxtStt + nPos );
                if( pAttr )
                {
                    switch( pAttr->Which() )
                    {
                    case RES_TXTATR_FIELD:
                        rTxt.Erase( nPos, 1 );
                        if( bExpandFlds )
                        {
                            const XubString aExpand(
                                static_cast<SwTxtFld const*>(pAttr)->GetFld()
                                    .GetFld()->ExpandField(true));
                            rTxt.Insert( aExpand, nPos );
                            nPos = nPos + aExpand.Len();
                            nEndPos = nEndPos + aExpand.Len();
                            rTxtStt = rTxtStt - aExpand.Len();
                        }
                        ++rTxtStt;
                        break;
                    case RES_TXTATR_FTN:
                        rTxt.Erase( nPos, 1 );
                        if( bExpandFlds )
                        {
                            const SwFmtFtn& rFtn = pAttr->GetFtn();
                            XubString sExpand;
                            if( rFtn.GetNumStr().Len() )
                                sExpand = rFtn.GetNumStr();
                            else if( rFtn.IsEndNote() )
                                sExpand = GetDoc()->GetEndNoteInfo().aFmt.
                                                GetNumStr( rFtn.GetNumber() );
                            else
                                sExpand = GetDoc()->GetFtnInfo().aFmt.
                                                GetNumStr( rFtn.GetNumber() );
                            rTxt.Insert( sExpand, nPos );
                            nPos = nPos + sExpand.Len();
                            nEndPos = nEndPos + sExpand.Len();
                            rTxtStt = rTxtStt - sExpand.Len();
                        }
                        ++rTxtStt;
                        break;
                    default:
                        rTxt.Erase( nPos, 1 );
                        ++rTxtStt;
                    }
                }
                else
                    ++nPos, ++nEndPos;
                nPos = rTxt.Search( cSrchChr, nPos );
            }
        }
    }
}

/*************************************************************************
 *                      SwTxtNode::GetExpandTxt
 * Expand fields
 *************************************************************************/
// #i83479# - handling of new parameters
XubString SwTxtNode::GetExpandTxt( const xub_StrLen nIdx,
                                   const xub_StrLen nLen,
                                   const bool bWithNum,
                                   const bool bAddSpaceAfterListLabelStr,
                                   const bool bWithSpacesForLevel ) const
{
    XubString aTxt( GetTxt().Copy( nIdx, nLen ) );
    xub_StrLen nTxtStt = nIdx;
    Replace0xFF( aTxt, nTxtStt, aTxt.Len(), sal_True );
    if( bWithNum )
    {
        XubString aListLabelStr = GetNumString();
        if ( aListLabelStr.Len() > 0 )
        {
            if ( bAddSpaceAfterListLabelStr )
            {
                const sal_Unicode aSpace = ' ';
                aTxt.Insert( aSpace, 0 );
            }
            aTxt.Insert( GetNumString(), 0 );
        }
    }

    if ( bWithSpacesForLevel && GetActualListLevel() > 0 )
    {
        int nLevel( GetActualListLevel() );
        while ( nLevel > 0 )
        {
            const sal_Unicode aSpace = ' ';
            aTxt.Insert( aSpace , 0 );
            aTxt.Insert( aSpace , 0 );
            --nLevel;
        }
    }

    return aTxt;
}

sal_Bool SwTxtNode::GetExpandTxt( SwTxtNode& rDestNd, const SwIndex* pDestIdx,
                        xub_StrLen nIdx, xub_StrLen nLen, sal_Bool bWithNum,
                        sal_Bool bWithFtn, sal_Bool bReplaceTabsWithSpaces ) const
{
    if( &rDestNd == this )
        return sal_False;

    SwIndex aDestIdx( &rDestNd, rDestNd.GetTxt().Len() );
    if( pDestIdx )
        aDestIdx = *pDestIdx;
    xub_StrLen nDestStt = aDestIdx.GetIndex();

    // Text einfuegen
    String sTmpText = GetTxt();
    if( bReplaceTabsWithSpaces )
        sTmpText.SearchAndReplaceAll('\t', ' ');

    // mask hidden characters
    const sal_Unicode cChar = CH_TXTATR_BREAKWORD;
    sal_uInt16 nHiddenChrs =
        SwScriptInfo::MaskHiddenRanges( *this, sTmpText, 0, sTmpText.Len(), cChar );

    sTmpText = sTmpText.Copy( nIdx, nLen );
    rDestNd.InsertText( sTmpText, aDestIdx );
    nLen = aDestIdx.GetIndex() - nDestStt;

    // alle FontAttribute mit CHARSET Symbol in dem Bereich setzen
    if ( HasHints() )
    {
        xub_StrLen nInsPos = nDestStt - nIdx;
        for ( sal_uInt16 i = 0; i < m_pSwpHints->Count(); i++ )
        {
            const SwTxtAttr* pHt = (*m_pSwpHints)[i];
            const xub_StrLen nAttrStartIdx = *pHt->GetStart();
            const sal_uInt16 nWhich = pHt->Which();
            if (nIdx + nLen <= nAttrStartIdx)
                break;      // ueber das Textende

            const xub_StrLen *pEndIdx = pHt->GetEnd();
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
                    // attribute in area => copy
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
                    {
                        XubString const aExpand(
                            static_cast<SwTxtFld const*>(pHt)->GetFld().GetFld()
                                ->ExpandField(true));
                        if( aExpand.Len() )
                        {
                            ++aDestIdx;     // dahinter einfuegen;
                            rDestNd.InsertText( aExpand, aDestIdx );
                            aDestIdx = nInsPos + nAttrStartIdx;
                            nInsPos = nInsPos + aExpand.Len();
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
                            XubString sExpand;
                            if( rFtn.GetNumStr().Len() )
                                sExpand = rFtn.GetNumStr();
                            else if( rFtn.IsEndNote() )
                                sExpand = GetDoc()->GetEndNoteInfo().aFmt.
                                                GetNumStr( rFtn.GetNumber() );
                            else
                                sExpand = GetDoc()->GetFtnInfo().aFmt.
                                                GetNumStr( rFtn.GetNumber() );
                            if( sExpand.Len() )
                            {
                                ++aDestIdx;     // insert behind
                                SvxEscapementItem aItem(
                                        SVX_ESCAPEMENT_SUPERSCRIPT );
                                rDestNd.InsertItem(aItem,
                                        aDestIdx.GetIndex(),
                                        aDestIdx.GetIndex() );
                                rDestNd.InsertText( sExpand, aDestIdx,
                                  IDocumentContentOperations::INS_EMPTYEXPAND);
                                aDestIdx = nInsPos + nAttrStartIdx;
                                nInsPos = nInsPos + sExpand.Len();
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

    if ( nHiddenChrs > 0 )
    {
        aDestIdx = 0;
        while ( aDestIdx < rDestNd.GetTxt().Len() )
        {
            if ( cChar == rDestNd.GetTxt().GetChar( aDestIdx.GetIndex() ) )
            {
                xub_StrLen nIndex = aDestIdx.GetIndex();
                while ( nIndex < rDestNd.GetTxt().Len() &&
                        cChar == rDestNd.GetTxt().GetChar( ++nIndex ) )
                    ;
                rDestNd.EraseText( aDestIdx, nIndex - aDestIdx.GetIndex() );
            }
            else
                ++aDestIdx;
        }
    }

    return sal_True;
}

struct block
{
    sal_Int32 m_nStart;
    sal_Int32 m_nLen;
    bool m_bVisible;
    std::vector<const SwTxtAttr*> m_aAttrs;
    block(sal_Int32 nStart, sal_Int32 nLen, bool bVisible)
        : m_nStart(nStart), m_nLen(nLen), m_bVisible(bVisible)
    {
    }
};

struct containsPos
{
    const sal_Int32 m_nPos;
    containsPos(const sal_Int32 nPos)
        : m_nPos(nPos)
    {
    }
    bool operator() (const block& rIn) const
    {
        return m_nPos >= rIn.m_nStart && m_nPos < rIn.m_nStart + rIn.m_nLen;
    }
};

ModelToViewHelper::ModelToViewHelper(const SwTxtNode &rNode, int eMode)
{
    const rtl::OUString& rNodeText = rNode.GetTxt();
    m_aRetText = rNodeText;

    if (eMode == PASSTHROUGH)
        return;

    Range aRange( 0, rNodeText.isEmpty() ? 0 : rNodeText.getLength() - 1);
    MultiSelection aHiddenMulti(aRange);

    if (eMode & HIDEINVISIBLE)
        SwScriptInfo::selectHiddenTextProperty(rNode, aHiddenMulti);

    if (eMode & HIDEREDLINED)
        SwScriptInfo::selectRedLineDeleted(rNode, aHiddenMulti);

    std::vector<block> aBlocks;

    sal_Int32 nShownStart = 0;
    for (size_t i = 0; i < aHiddenMulti.GetRangeCount(); ++i)
    {
        const Range& rRange = aHiddenMulti.GetRange(i);
        sal_Int32 nHiddenStart = rRange.Min();
        sal_Int32 nHiddenEnd = rRange.Max() + 1;
        sal_Int32 nHiddenLen = nHiddenEnd - nHiddenStart;

        sal_Int32 nShownEnd = nHiddenStart;
        sal_Int32 nShownLen = nShownEnd - nShownStart;

        if (nShownLen)
            aBlocks.push_back(block(nShownStart, nShownLen, true));

        if (nHiddenLen)
            aBlocks.push_back(block(nHiddenStart, nHiddenLen, false));

        nShownStart = nHiddenEnd;
    }

    sal_Int32 nTrailingShownLen = rNodeText.getLength() - nShownStart;
    if (nTrailingShownLen)
        aBlocks.push_back(block(nShownStart, nTrailingShownLen, true));

    if (eMode & EXPANDFIELDS)
    {
        const SwpHints* pSwpHints2 = rNode.GetpSwpHints();
        for ( sal_uInt16 i = 0; pSwpHints2 && i < pSwpHints2->Count(); ++i )
        {
            const SwTxtAttr* pAttr = (*pSwpHints2)[i];
            if (pAttr->HasDummyChar())
            {
                xub_StrLen nDummyCharPos = *pAttr->GetStart();
                if (aHiddenMulti.IsSelected(nDummyCharPos))
                    continue;
                std::vector<block>::iterator aFind = std::find_if(aBlocks.begin(), aBlocks.end(), containsPos(nDummyCharPos));
                aFind->m_aAttrs.push_back(pAttr);
            }
        }
    }

    sal_Int32 nOffset = 0;
    for (std::vector<block>::iterator i = aBlocks.begin(); i != aBlocks.end(); ++i)
    {
        if (!i->m_bVisible)
        {
            const sal_Int32 nHiddenStart = i->m_nStart;
            const sal_Int32 nHiddenLen = i->m_nLen;

            m_aRetText = m_aRetText.replaceAt( nOffset + nHiddenStart, nHiddenLen, rtl::OUString() );
            m_aMap.push_back( ConversionMapEntry( nHiddenStart, nOffset + nHiddenStart ) );
            nOffset -= nHiddenLen;
        }
        else
        {
            for (std::vector<const SwTxtAttr*>::iterator j = i->m_aAttrs.begin(); j != i->m_aAttrs.end(); ++j)
            {
                const SwTxtAttr* pAttr = *j;
                xub_StrLen nFieldPos = *pAttr->GetStart();
                rtl::OUString aExpand;
                switch (pAttr->Which())
                {
                    case RES_TXTATR_FIELD:
                        aExpand =
                            static_cast<SwTxtFld const*>(pAttr)->GetFld().GetFld()
                                ->ExpandField(true);
                        break;
                    case RES_TXTATR_FTN:
                        {
                            const SwFmtFtn& rFtn = static_cast<SwTxtFtn const*>(pAttr)->GetFtn();
                            const SwDoc *pDoc = rNode.GetDoc();
                            aExpand = rFtn.GetViewNumStr(*pDoc);
                        }
                        break;
                    default:
                        break;
                }
                m_aRetText = m_aRetText.replaceAt( nOffset + nFieldPos, 1, aExpand );
                m_aMap.push_back( ConversionMapEntry( nFieldPos, nOffset + nFieldPos ) );
                nOffset += ( aExpand.getLength() - 1 );
            }
        }
    }

    if ( !m_aMap.empty() )
        m_aMap.push_back( ConversionMapEntry( rNodeText.getLength()+1, m_aRetText.getLength()+1 ) );
}

XubString SwTxtNode::GetRedlineTxt( xub_StrLen nIdx, xub_StrLen nLen,
                                sal_Bool bExpandFlds, sal_Bool bWithNum ) const
{
    std::vector<sal_uInt16> aRedlArr;
    const SwDoc* pDoc = GetDoc();
    sal_uInt16 nRedlPos = pDoc->GetRedlinePos( *this, nsRedlineType_t::REDLINE_DELETE );
    if( USHRT_MAX != nRedlPos )
    {
        // es existiert fuer den Node irgendein Redline-Delete-Object
        const sal_uLong nNdIdx = GetIndex();
        for( ; nRedlPos < pDoc->GetRedlineTbl().size() ; ++nRedlPos )
        {
            const SwRedline* pTmp = pDoc->GetRedlineTbl()[ nRedlPos ];
            if( nsRedlineType_t::REDLINE_DELETE == pTmp->GetType() )
            {
                const SwPosition *pRStt = pTmp->Start(), *pREnd = pTmp->End();
                if( pRStt->nNode < nNdIdx )
                {
                    if( pREnd->nNode > nNdIdx )
                        // Absatz ist komplett geloescht
                        return aEmptyStr;
                    else if( pREnd->nNode == nNdIdx )
                    {
                        // von 0 bis nContent ist alles geloescht
                        aRedlArr.push_back( xub_StrLen(0) );
                        aRedlArr.push_back( pREnd->nContent.GetIndex() );
                    }
                }
                else if( pRStt->nNode == nNdIdx )
                {
                    //aRedlArr.Insert( pRStt->nContent.GetIndex(), aRedlArr.Count() );
                    aRedlArr.push_back( pRStt->nContent.GetIndex() );
                    if( pREnd->nNode == nNdIdx )
                        aRedlArr.push_back( pREnd->nContent.GetIndex() );
                    else
                    {
                        aRedlArr.push_back( GetTxt().Len() );
                        break;      // mehr kann nicht kommen
                    }
                }
                else
                    break;      // mehr kann nicht kommen
            }
        }
    }

    XubString aTxt( GetTxt().Copy( nIdx, nLen ) );

    xub_StrLen nTxtStt = nIdx, nIdxEnd = nIdx + aTxt.Len();
    for( sal_uInt16 n = 0; n < aRedlArr.size(); n += 2 )
    {
        xub_StrLen nStt = aRedlArr[ n ], nEnd = aRedlArr[ n+1 ];
        if( ( nIdx <= nStt && nStt <= nIdxEnd ) ||
            ( nIdx <= nEnd && nEnd <= nIdxEnd ))
        {
            if( nStt < nIdx ) nStt = nIdx;
            if( nIdxEnd < nEnd ) nEnd = nIdxEnd;
            xub_StrLen nDelCnt = nEnd - nStt;
            aTxt.Erase( nStt - nTxtStt, nDelCnt );
            Replace0xFF( aTxt, nTxtStt, nStt - nTxtStt, bExpandFlds );
            nTxtStt = nTxtStt + nDelCnt;
        }
        else if( nStt >= nIdxEnd )
            break;
    }
    Replace0xFF( aTxt, nTxtStt, aTxt.Len(), bExpandFlds );

    if( bWithNum )
        aTxt.Insert( GetNumString(), 0 );
    return aTxt;
}

/*************************************************************************
 *                        SwTxtNode::ReplaceText
 *************************************************************************/

void SwTxtNode::ReplaceText( const SwIndex& rStart, const xub_StrLen nDelLen,
                             const XubString& rText )
{
    OSL_ENSURE( rStart.GetIndex() < m_Text.Len() &&
            rStart.GetIndex() + nDelLen <= m_Text.Len(),
            "SwTxtNode::ReplaceText: index out of bounds" );
    const xub_StrLen nStartPos = rStart.GetIndex();
    xub_StrLen nEndPos = nStartPos + nDelLen;
    xub_StrLen nLen = nDelLen;
    for ( xub_StrLen nPos = nStartPos; nPos < nEndPos; ++nPos )
    {
        if ( ( CH_TXTATR_BREAKWORD == m_Text.GetChar( nPos ) ) ||
             ( CH_TXTATR_INWORD    == m_Text.GetChar( nPos ) ) )
        {
            SwTxtAttr *const pHint = GetTxtAttrForCharAt( nPos );
            if (pHint)
            {
               OSL_ENSURE(!( pHint->GetEnd() && pHint->HasDummyChar()
                            && (*pHint->GetStart() < nEndPos)
                            && (*pHint->GetEnd()   > nEndPos) ),
                    "ReplaceText: ERROR: "
                    "deleting left-overlapped attribute with CH_TXTATR");
                DeleteAttribute( pHint );
                --nEndPos;
                --nLen;
            }
        }
    }

    bool bOldExpFlg = IsIgnoreDontExpand();
    SetIgnoreDontExpand( true );

    if( nLen && rText.Len() )
    {
        // dann das 1. Zeichen ersetzen den Rest loschen und einfuegen
        // Dadurch wird die Attributierung des 1. Zeichen expandiert!
        m_Text.SetChar( nStartPos, rText.GetChar( 0 ) );

        ++((SwIndex&)rStart);
        m_Text.Erase( rStart.GetIndex(), nLen - 1 );
        Update( rStart, nLen - 1, true );

        XubString aTmpTxt( rText ); aTmpTxt.Erase( 0, 1 );
        m_Text.Insert( aTmpTxt, rStart.GetIndex() );
        Update( rStart, aTmpTxt.Len(), false );
    }
    else
    {
        m_Text.Erase( nStartPos, nLen );
        Update( rStart, nLen, true );

        m_Text.Insert( rText, nStartPos );
        Update( rStart, rText.Len(), false );
    }

    SetIgnoreDontExpand( bOldExpFlg );
    SwDelTxt aDelHint( nStartPos, nDelLen );
    NotifyClients( 0, &aDelHint );

    SwInsTxt aHint( nStartPos, rText.Len() );
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
        // #i96644#
        // suppress side effect "send data changed events"
        rTxtNode.GetDoc()->ResetAttrs( aPam, false, aAttrs, false );
    }

    // Helper method for special handling of modified attributes at text node.
    // The following is handled:
    // (1) on changing the paragraph style - RES_FMT_CHG:
    // Check, if list style of the text node is changed. If yes, add respectively
    // remove the text node to the corresponding list.
    // (2) on changing the attributes - RES_ATTRSET_CHG:
    // Same as (1).
    // (3) on changing the list style - RES_PARATR_NUMRULE:
    // Same as (1).
    void HandleModifyAtTxtNode( SwTxtNode& rTxtNode,
                                const SfxPoolItem* pOldValue,
                                const SfxPoolItem* pNewValue )
    {
        const sal_uInt16 nWhich = pOldValue ? pOldValue->Which() :
                              pNewValue ? pNewValue->Which() : 0 ;
        bool bNumRuleSet = false;
        bool bParagraphStyleChanged = false;
        String sNumRule;
        String sOldNumRule;
        switch ( nWhich )
        {
            case RES_FMT_CHG:
            {
                bParagraphStyleChanged = true;
                if( rTxtNode.GetNodes().IsDocNodes() )
                {
                    // #i70748#
                    const SwNumRule* pFormerNumRuleAtTxtNode =
                        rTxtNode.GetNum() ? rTxtNode.GetNum()->GetNumRule() : 0;
                    if ( pFormerNumRuleAtTxtNode )
                    {
                        sOldNumRule = pFormerNumRuleAtTxtNode->GetName();
                    }
                    // #i70748#
                    if ( rTxtNode.IsEmptyListStyleDueToSetOutlineLevelAttr() )
                    {
                        const SwNumRuleItem& rNumRuleItem = rTxtNode.GetTxtColl()->GetNumRule();
                        if ( rNumRuleItem.GetValue().Len() > 0 )
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
                // #i70748#
                const SwNumRule* pFormerNumRuleAtTxtNode =
                    rTxtNode.GetNum() ? rTxtNode.GetNum()->GetNumRule() : 0;
                if ( pFormerNumRuleAtTxtNode )
                {
                    sOldNumRule = pFormerNumRuleAtTxtNode->GetName();
                }

                if ( dynamic_cast<const SwAttrSetChg*>(pNewValue)->GetChgSet()->GetItemState( RES_PARATR_NUMRULE, sal_False, &pItem ) ==
                        SFX_ITEM_SET )
                {
                    // #i70748#
                    rTxtNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
                    bNumRuleSet = true;
                }
                // #i70748#
                // The new list style set at the paragraph.
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
                        // #i70748#
                        rTxtNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
                        bNumRuleSet = true;
                    }
                    // #i70748#
                    // The new list style set at the paragraph.
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
                if ( sNumRule.Len() == 0 )
                {
                    rTxtNode.RemoveFromList();
                    if ( bParagraphStyleChanged )
                    {
                        ::lcl_ResetParAttrs(rTxtNode);
                    }
                }
                else
                {
                    rTxtNode.RemoveFromList();
                    // If new list style is the outline style, apply outline
                    // level as the list level.
                    if ( sNumRule.EqualsAscii(SwNumRule::GetOutlineRuleName()) )
                    {
                        // #i70748#
                        OSL_ENSURE( rTxtNode.GetTxtColl()->IsAssignedToListLevelOfOutlineStyle(),
                                "<HandleModifyAtTxtNode()> - text node with outline style, but its paragraph style is not assigned to outline style." );
                        int nNewListLevel =
                            rTxtNode.GetTxtColl()->GetAssignedOutlineStyleLevel();
                        if ( 0 <= nNewListLevel && nNewListLevel < MAXLEVEL )
                        {
                            rTxtNode.SetAttrListLevel( nNewListLevel );
                        }
                    }
                    rTxtNode.AddToList();
                }
            }
            else // <sNumRule.Len() == 0 && sOldNumRule.Len() != 0>
            {
                rTxtNode.RemoveFromList();
                if ( bParagraphStyleChanged )
                {
                    ::lcl_ResetParAttrs(rTxtNode);
                    // #i70748#
                    if ( dynamic_cast<const SfxUInt16Item &>(rTxtNode.GetAttr( RES_PARATR_OUTLINELEVEL, sal_False )).GetValue() > 0 )
                    {
                        rTxtNode.SetEmptyListStyleDueToSetOutlineLevelAttr();
                    }
                }
            }
        }
        else if ( sNumRule.Len() > 0 && !rTxtNode.IsInList() )
        {
            rTxtNode.AddToList();
        }
    }
    // End of method <HandleModifyAtTxtNode>
}

void SwTxtNode::Modify( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue )
{
    bool bWasNotifiable = m_bNotifiable;
    m_bNotifiable = false;

    // Bug 24616/24617:
    //      Modify ueberladen, damit beim Loeschen von Vorlagen diese
    //      wieder richtig verwaltet werden (Outline-Numerierung!!)
    //  Bug25481:
    //      bei Nodes im Undo nie _ChgTxtCollUpdateNum rufen.
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
    // #125329# - assure that text node is in document nodes array
    if ( pDoc && !pDoc->IsInDtor() && &pDoc->GetNodes() == &GetNodes() )
    {
        pDoc->GetNodes().UpdateOutlineNode(*this);
    }

    m_bNotifiable = bWasNotifiable;

    if (pOldValue && (RES_REMOVE_UNO_OBJECT == pOldValue->Which()))
    {   // invalidate cached uno object
        SetXParagraph(::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextContent>(0));
    }
}

SwFmtColl* SwTxtNode::ChgFmtColl( SwFmtColl *pNewColl )
{
    OSL_ENSURE( pNewColl,"ChgFmtColl: Collectionpointer ist 0." );
    OSL_ENSURE( HAS_BASE( SwTxtFmtColl, pNewColl ),
                "ChgFmtColl: ist kein Text-Collectionpointer." );

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

    // nur wenn im normalen Nodes-Array
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

//#outline level, zhaojianwei
int SwTxtNode::GetAttrOutlineLevel() const
{
    return ((const SfxUInt16Item &)GetAttr(RES_PARATR_OUTLINELEVEL)).GetValue();
}
void SwTxtNode::SetAttrOutlineLevel(int nLevel)
{
    OSL_ENSURE( 0 <= nLevel && nLevel <= MAXLEVEL ,"SwTxtNode: Level Out Of Range" );//#outline level,zhaojianwei
    if ( 0 <= nLevel && nLevel <= MAXLEVEL )
    {
        SetAttr( SfxUInt16Item( RES_PARATR_OUTLINELEVEL,
                                static_cast<sal_uInt16>(nLevel) ) );
    }
}
//<-end

// #i70748#
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
        OSL_FAIL( "<SwTxtNode::SetAttrListLevel()> - value of parameter <nLevel> is out of valid range" );
        return;
    }

    SfxInt16Item aNewListLevelItem( RES_PARATR_LIST_LEVEL,
                                    static_cast<sal_Int16>(nLevel) );
    SetAttr( aNewListLevelItem );
}

bool SwTxtNode::HasAttrListLevel() const
{
    return GetpSwAttrSet() &&
           GetpSwAttrSet()->GetItemState( RES_PARATR_LIST_LEVEL, sal_False ) == SFX_ITEM_SET;
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
//    CreateNum()->SetRestart(bRestart);
    if ( !bRestart )
    {
        // attribute not contained in paragraph style's attribute set. Thus,
        // it can be reset to the attribute pool default by resetting the attribute.
        ResetAttr( RES_PARATR_LIST_ISRESTART );
    }
    else
    {
        SfxBoolItem aNewIsRestartItem( RES_PARATR_LIST_ISRESTART,
                                       sal_True );
        SetAttr( aNewIsRestartItem );
    }
}

bool SwTxtNode::IsListRestart() const
{
//    return GetNum() ? GetNum()->IsRestart() : false;
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
    bool bRet = false;

    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : 0L;
    if ( pRule && IsCountedInList())
    {
        // #i87154#
        // Correction of #newlistlevelattrs#:
        // The numbering type has to be checked for bullet lists.
        const SwNumFmt& rFmt = pRule->Get( static_cast<sal_uInt16>(GetActualListLevel() ));
        if ( SVX_NUM_NUMBER_NONE != rFmt.GetNumberingType() ||
             pRule->MakeNumString( *(GetNum()) ).Len() > 0 )
        {
            bRet = true;
        }
    }

    return bRet;
}

void SwTxtNode::SetAttrListRestartValue( SwNumberTree::tSwNumTreeNumber nNumber )
{
//    CreateNum()->SetStart(nNumber);
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
           GetpSwAttrSet()->GetItemState( RES_PARATR_LIST_RESTARTVALUE, sal_False ) == SFX_ITEM_SET;
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
//    return GetNum() ? GetNum()->GetStart() : 1;
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
        // attribute not contained in paragraph style's attribute set. Thus,
        // it can be reset to the attribute pool default by resetting the attribute.
        ResetAttr( RES_PARATR_LIST_ISCOUNTED );
    }
    else
    {
        SfxBoolItem aIsCountedInListItem( RES_PARATR_LIST_ISCOUNTED, sal_False );
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

    const String sListId = GetListId();
    if ( sListId.Len() > 0 )
    {
        SwList* pList = GetDoc()->getListByName( sListId );
        if ( pList == 0 )
        {
            // Create corresponding list.
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

void SwTxtNode::SetListId( const String sListId )
{
    const SfxStringItem& rListIdItem =
            dynamic_cast<const SfxStringItem&>(GetAttr( RES_PARATR_LIST_ID ));
    if ( rListIdItem.GetValue() != sListId )
    {
        if ( sListId.Len() == 0 )
        {
            ResetAttr( RES_PARATR_LIST_ID );
        }
        else
        {
            SfxStringItem aNewListIdItem( RES_PARATR_LIST_ID, sListId );
            SetAttr( aNewListIdItem );
        }
    }
}

String SwTxtNode::GetListId() const
{
    String sListId;

    const SfxStringItem& rListIdItem =
                dynamic_cast<const SfxStringItem&>(GetAttr( RES_PARATR_LIST_ID ));
    sListId = rListIdItem.GetValue();

    // As long as no explicit list id attribute is set, use the list id of
    // the list, which has been created for the applied list style.
    if ( sListId.Len() == 0 )
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

    @author OD

    @return boolean
*/
bool SwTxtNode::AreListLevelIndentsApplicable() const
{
    bool bAreListLevelIndentsApplicable( true );

    if ( !GetNum() || !GetNum()->GetNumRule() )
    {
        // no list style applied to paragraph
        bAreListLevelIndentsApplicable = false;
    }
    else if ( HasSwAttrSet() &&
              GetpSwAttrSet()->GetItemState( RES_LR_SPACE, sal_False ) == SFX_ITEM_SET )
    {
        // paragraph has hard-set indent attributes
        bAreListLevelIndentsApplicable = false;
    }
    else if ( HasSwAttrSet() &&
              GetpSwAttrSet()->GetItemState( RES_PARATR_NUMRULE, sal_False ) == SFX_ITEM_SET )
    {
        // list style is directly applied to paragraph and paragraph has no
        // hard-set indent attributes
        bAreListLevelIndentsApplicable = true;
    }
    else
    {
        // list style is applied through one of the paragraph styles and
        // paragraph has no hard-set indent attributes

        // check, paragraph's
        const SwTxtFmtColl* pColl = GetTxtColl();
        while ( pColl )
        {
            if ( pColl->GetAttrSet().GetItemState( RES_LR_SPACE, sal_False ) == SFX_ITEM_SET )
            {
                // indent attributes found in the paragraph style hierarchy.
                bAreListLevelIndentsApplicable = false;
                break;
            }

            if ( pColl->GetAttrSet().GetItemState( RES_PARATR_NUMRULE, sal_False ) == SFX_ITEM_SET )
            {
                // paragraph style with the list style found and until now no
                // indent attributes are found in the paragraph style hierarchy.
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

    @author OD

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
                // tab stop position are treated to be relative to the "before text"
                // indent value of the paragraph. Thus, adjust <nListTabStopPos>.
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

/** Retrieves the character following the list label, if the paragraph's
    list level defines one.

    @author OD

    @return XubString - the list tab stop position
*/
XubString SwTxtNode::GetLabelFollowedBy() const
{
    XubString aLabelFollowedBy;

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
                    const sal_Unicode aTab = '\t';
                    aLabelFollowedBy.Insert( aTab, 0 );
                }
                break;
                case SvxNumberFormat::SPACE:
                {
                    const sal_Unicode aSpace = ' ';
                    aLabelFollowedBy.Insert( aSpace, 0 );
                }
                break;
                case SvxNumberFormat::NOTHING:
                {
                    // intentionally left blank.
                }
                break;
                default:
                {
                    OSL_FAIL( "<SwTxtNode::GetLabelFollowedBy()> - unknown SvxNumberFormat::GetLabelFollowedBy() return value" );
                }
            }
        }
    }

    return aLabelFollowedBy;
}

void SwTxtNode::CalcHiddenCharFlags() const
{
    xub_StrLen nStartPos;
    xub_StrLen nEndPos;
    // Update of the flags is done inside GetBoundsOfHiddenRange()
    SwScriptInfo::GetBoundsOfHiddenRange( *this, 0, nStartPos, nEndPos );
}

// #i12836# enhanced pdf export
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
    // Helper class for special handling of setting attributes at text node:
    // In constructor an instance of the helper class recognize whose attributes
    // are set and perform corresponding actions before the intrinsic set of
    // attributes has been taken place.
    // In the destructor - after the attributes have been set at the text
    // node - corresponding actions are performed.
    // The following is handled:
    // (1) When the list style attribute - RES_PARATR_NUMRULE - is set,
    //     (A) list style attribute is empty -> the text node is removed from
    //         its list.
    //     (B) list style attribute is not empty
    //         (a) text node has no list style -> add text node to its list after
    //             the attributes have been set.
    //         (b) text node has list style -> change of list style is notified
    //             after the attributes have been set.
    // (2) When the list id attribute - RES_PARATR_LIST_ID - is set and changed,
    //     the text node is removed from its current list before the attributes
    //     are set and added to its new list after the attributes have been set.
    // (3) Notify list tree, if list level - RES_PARATR_LIST_LEVEL - is set
    //     and changed after the attributes have been set
    // (4) Notify list tree, if list restart - RES_PARATR_LIST_ISRESTART - is set
    //     and changed after the attributes have been set
    // (5) Notify list tree, if list restart value - RES_PARATR_LIST_RESTARTVALUE -
    //     is set and changed after the attributes have been set
    // (6) Notify list tree, if count in list - RES_PARATR_LIST_ISCOUNTED - is set
    //     and changed after the attributes have been set
    // (7) Set or Reset emtpy list style due to changed outline level - RES_PARATR_OUTLINELEVEL.
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
            // #i70748#
            bool mbOutlineLevelSet;
    };

    HandleSetAttrAtTxtNode::HandleSetAttrAtTxtNode( SwTxtNode& rTxtNode,
                                                    const SfxPoolItem& pItem )
        : mrTxtNode( rTxtNode ),
          mbAddTxtNodeToList( false ),
          mbUpdateListLevel( false ),
          mbUpdateListRestart( false ),
          mbUpdateListCount( false ),
          // #i70748#
          mbOutlineLevelSet( false )
    {
        switch ( pItem.Which() )
        {
            // handle RES_PARATR_NUMRULE
            case RES_PARATR_NUMRULE:
            {
                mrTxtNode.RemoveFromList();

                const SwNumRuleItem& pNumRuleItem =
                                dynamic_cast<const SwNumRuleItem&>(pItem);
                if ( pNumRuleItem.GetValue().Len() > 0 )
                {
                    mbAddTxtNodeToList = true;
                    // #i105562#
                    //
                    mrTxtNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
                }
            }
            break;

            // handle RES_PARATR_LIST_ID
            case RES_PARATR_LIST_ID:
            {
                const SfxStringItem& pListIdItem =
                                        dynamic_cast<const SfxStringItem&>(pItem);
                OSL_ENSURE( pListIdItem.GetValue().Len() > 0,
                        "<HandleSetAttrAtTxtNode(..)> - empty list id attribute not excepted. Serious defect -> please inform OD." );
                const String sListIdOfTxtNode = rTxtNode.GetListId();
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

            // handle RES_PARATR_LIST_LEVEL
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

            // handle RES_PARATR_LIST_ISRESTART
            case RES_PARATR_LIST_ISRESTART:
            {
                const SfxBoolItem& aListIsRestartItem =
                                    dynamic_cast<const SfxBoolItem&>(pItem);
                if ( aListIsRestartItem.GetValue() !=
                                    (mrTxtNode.IsListRestart() ? sal_True : sal_False) )
                {
                    mbUpdateListRestart = true;
                }
            }
            break;

            // handle RES_PARATR_LIST_RESTARTVALUE
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

            // handle RES_PARATR_LIST_ISCOUNTED
            case RES_PARATR_LIST_ISCOUNTED:
            {
                const SfxBoolItem& aIsCountedInListItem =
                                    dynamic_cast<const SfxBoolItem&>(pItem);
                if ( aIsCountedInListItem.GetValue() !=
                                    (mrTxtNode.IsCountedInList() ? sal_True : sal_False) )
                {
                    mbUpdateListCount = true;
                }
            }
            break;

            // #i70748#
            // handle RES_PARATR_OUTLINELEVEL
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
          // #i70748#
          mbOutlineLevelSet( false )
    {
        const SfxPoolItem* pItem = 0;
        // handle RES_PARATR_NUMRULE
        if ( rItemSet.GetItemState( RES_PARATR_NUMRULE, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            mrTxtNode.RemoveFromList();

            const SwNumRuleItem* pNumRuleItem =
                            dynamic_cast<const SwNumRuleItem*>(pItem);
            if ( pNumRuleItem->GetValue().Len() > 0 )
            {
                mbAddTxtNodeToList = true;
                // #i70748#
                mrTxtNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
            }
        }

        // handle RES_PARATR_LIST_ID
        if ( rItemSet.GetItemState( RES_PARATR_LIST_ID, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            const SfxStringItem* pListIdItem =
                                    dynamic_cast<const SfxStringItem*>(pItem);
            const String sListIdOfTxtNode = mrTxtNode.GetListId();
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

        // handle RES_PARATR_LIST_LEVEL
        if ( rItemSet.GetItemState( RES_PARATR_LIST_LEVEL, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            const SfxInt16Item* pListLevelItem =
                                dynamic_cast<const SfxInt16Item*>(pItem);
            if ( pListLevelItem->GetValue() != mrTxtNode.GetAttrListLevel() )
            {
                mbUpdateListLevel = true;
            }
        }

        // handle RES_PARATR_LIST_ISRESTART
        if ( rItemSet.GetItemState( RES_PARATR_LIST_ISRESTART, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            const SfxBoolItem* pListIsRestartItem =
                                dynamic_cast<const SfxBoolItem*>(pItem);
            if ( pListIsRestartItem->GetValue() !=
                                    (mrTxtNode.IsListRestart() ? sal_True : sal_False) )
            {
                mbUpdateListRestart = true;
            }
        }

        // handle RES_PARATR_LIST_RESTARTVALUE
        if ( rItemSet.GetItemState( RES_PARATR_LIST_RESTARTVALUE, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            const SfxInt16Item* pListRestartValueItem =
                                dynamic_cast<const SfxInt16Item*>(pItem);
            if ( !mrTxtNode.HasAttrListRestartValue() ||
                 pListRestartValueItem->GetValue() != mrTxtNode.GetAttrListRestartValue() )
            {
                mbUpdateListRestart = true;
            }
        }

        // handle RES_PARATR_LIST_ISCOUNTED
        if ( rItemSet.GetItemState( RES_PARATR_LIST_ISCOUNTED, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            const SfxBoolItem* pIsCountedInListItem =
                                dynamic_cast<const SfxBoolItem*>(pItem);
            if ( pIsCountedInListItem->GetValue() !=
                                (mrTxtNode.IsCountedInList() ? sal_True : sal_False) )
            {
                mbUpdateListCount = true;
            }
        }

        // #i70748#
        // handle RES_PARATR_OUTLINELEVEL
        if ( rItemSet.GetItemState( RES_PARATR_OUTLINELEVEL, sal_False, &pItem ) == SFX_ITEM_SET )
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

        // #i70748#
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
                                                            sal_True, &pItem )
                                                                != SFX_ITEM_SET )
                {
                    mrTxtNode.SetEmptyListStyleDueToSetOutlineLevelAttr();
                }
            }
        }
    }
    // End of class <HandleSetAttrAtTxtNode>
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
    // Helper class for special handling of resetting attributes at text node:
    // In constructor an instance of the helper class recognize whose attributes
    // are reset and perform corresponding actions before the intrinsic reset of
    // attributes has been taken place.
    // In the destructor - after the attributes have been reset at the text
    // node - corresponding actions are performed.
    // The following is handled:
    // (1) When the list style attribute - RES_PARATR_NUMRULE - is reset,
    //     the text is removed from its list before the attributes have been reset.
    // (2) When the list id attribute - RES_PARATR_LIST_ID - is reset,
    //     the text is removed from its list before the attributes have been reset.
    // (3) Notify list tree, if list level - RES_PARATR_LIST_LEVEL - is reset.
    // (4) Notify list tree, if list restart - RES_PARATR_LIST_ISRESTART - is reset.
    // (5) Notify list tree, if list restart value - RES_PARATR_LIST_RESTARTVALUE - is reset.
    // (6) Notify list tree, if count in list - RES_PARATR_LIST_ISCOUNTED - is reset.
    // (7) Reset empty list style, if outline level attribute - RES_PARATR_OUTLINELEVEL - is reset.
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
            // RES_PARATR_NUMRULE and RES_PARATR_LIST_ID
            if ( nWhich1 <= RES_PARATR_NUMRULE && RES_PARATR_NUMRULE <= nWhich2 )
            {
                bRemoveFromList = mrTxtNode.GetNumRule() != 0;
                mbListStyleOrIdReset = true;
            }
            else if ( nWhich1 <= RES_PARATR_LIST_ID && RES_PARATR_LIST_ID <= nWhich2 )
            {
                bRemoveFromList = mrTxtNode.GetpSwAttrSet() &&
                    mrTxtNode.GetpSwAttrSet()->GetItemState( RES_PARATR_LIST_ID, sal_False ) == SFX_ITEM_SET;
                // #i92898#
                mbListStyleOrIdReset = true;
            }

            if ( !bRemoveFromList )
            {
                // RES_PARATR_LIST_LEVEL
                mbUpdateListLevel = ( nWhich1 <= RES_PARATR_LIST_LEVEL &&
                                      RES_PARATR_LIST_LEVEL <= nWhich2 &&
                                      mrTxtNode.HasAttrListLevel() );

                // RES_PARATR_LIST_ISRESTART and RES_PARATR_LIST_RESTARTVALUE
                mbUpdateListRestart =
                    ( nWhich1 <= RES_PARATR_LIST_ISRESTART && RES_PARATR_LIST_ISRESTART <= nWhich2 &&
                      mrTxtNode.IsListRestart() ) ||
                    ( nWhich1 <= RES_PARATR_LIST_RESTARTVALUE && RES_PARATR_LIST_RESTARTVALUE <= nWhich2 &&
                      mrTxtNode.HasAttrListRestartValue() );

                // RES_PARATR_LIST_ISCOUNTED
                mbUpdateListCount =
                    ( nWhich1 <= RES_PARATR_LIST_ISCOUNTED && RES_PARATR_LIST_ISCOUNTED <= nWhich2 &&
                      !mrTxtNode.IsCountedInList() );
            }

            // #i70748#
            // RES_PARATR_OUTLINELEVEL
            if ( nWhich1 <= RES_PARATR_OUTLINELEVEL && RES_PARATR_OUTLINELEVEL <= nWhich2 )
            {
                mrTxtNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
            }
        }
        else
        {
            // RES_PARATR_NUMRULE and RES_PARATR_LIST_ID
            if ( nWhich1 == RES_PARATR_NUMRULE )
            {
                bRemoveFromList = mrTxtNode.GetNumRule() != 0;
                mbListStyleOrIdReset = true;
            }
            else if ( nWhich1 == RES_PARATR_LIST_ID )
            {
                bRemoveFromList = mrTxtNode.GetpSwAttrSet() &&
                    mrTxtNode.GetpSwAttrSet()->GetItemState( RES_PARATR_LIST_ID, sal_False ) == SFX_ITEM_SET;
                // #i92898#
                mbListStyleOrIdReset = true;
            }
            // #i70748#
            // RES_PARATR_OUTLINELEVEL
            else if ( nWhich1 == RES_PARATR_OUTLINELEVEL )
            {
                mrTxtNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
            }

            if ( !bRemoveFromList )
            {
                // RES_PARATR_LIST_LEVEL
                mbUpdateListLevel = nWhich1 == RES_PARATR_LIST_LEVEL &&
                                    mrTxtNode.HasAttrListLevel();

                // RES_PARATR_LIST_ISRESTART and RES_PARATR_LIST_RESTARTVALUE
                mbUpdateListRestart = ( nWhich1 == RES_PARATR_LIST_ISRESTART &&
                                        mrTxtNode.IsListRestart() ) ||
                                      ( nWhich1 == RES_PARATR_LIST_RESTARTVALUE &&
                                        mrTxtNode.HasAttrListRestartValue() );

                // RES_PARATR_LIST_ISCOUNTED
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
                // RES_PARATR_NUMRULE and RES_PARATR_LIST_ID
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
                          mrTxtNode.GetpSwAttrSet()->GetItemState( RES_PARATR_LIST_ID, sal_False ) == SFX_ITEM_SET );
                    // #i92898#
                    mbListStyleOrIdReset = true;
                }
                // #i70748#
                // RES_PARATR_OUTLINELEVEL
                else if ( *it == RES_PARATR_OUTLINELEVEL )
                {
                    mrTxtNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
                }

                if ( !bRemoveFromList )
                {
                    // RES_PARATR_LIST_LEVEL
                    mbUpdateListLevel = mbUpdateListLevel ||
                                        ( *it == RES_PARATR_LIST_LEVEL &&
                                          mrTxtNode.HasAttrListLevel() );

                    // RES_PARATR_LIST_ISRESTART and RES_PARATR_LIST_RESTARTVALUE
                    mbUpdateListRestart = mbUpdateListRestart ||
                                          ( *it == RES_PARATR_LIST_ISRESTART &&
                                            mrTxtNode.IsListRestart() ) ||
                                          ( *it == RES_PARATR_LIST_RESTARTVALUE &&
                                            mrTxtNode.HasAttrListRestartValue() );

                    // RES_PARATR_LIST_ISCOUNTED
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
        // #i70748#
        mrTxtNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
    }

    HandleResetAttrAtTxtNode::~HandleResetAttrAtTxtNode()
    {
        if ( mbListStyleOrIdReset && !mrTxtNode.IsInList() )
        {
            // check, if in spite of the reset of the list style or the list id
            // the paragraph still has to be added to a list.
            if ( mrTxtNode.GetNumRule() &&
                 mrTxtNode.GetListId().Len() > 0 )
            {
                // #i96062#
                // If paragraph has no list level attribute set and list style
                // is the outline style, apply outline level as the list level.
                if ( !mrTxtNode.HasAttrListLevel() &&
                     mrTxtNode.GetNumRule()->GetName().EqualsAscii(
                        SwNumRule::GetOutlineRuleName()) &&
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
            // #i70748#
            // #i105562#
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
    // End of class <HandleResetAttrAtTxtNode>
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


sal_uInt32 SwTxtNode::GetRsid( xub_StrLen nStt, xub_StrLen nEnd ) const
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
    SvxRsidItem &rItem = ( SvxRsidItem& ) GetAttr( RES_PARATR_RSID );

    return rItem.GetValue();
}

bool SwTxtNode::CompareParRsid( const SwTxtNode &rTxtNode ) const
{
    sal_uInt32 nThisRsid = GetParRsid();
    sal_uInt32 nRsid = rTxtNode.GetParRsid();

    return nThisRsid == nRsid;
}

bool SwTxtNode::CompareRsid( const SwTxtNode &rTxtNode, xub_StrLen nStt1, xub_StrLen nStt2,
                            xub_StrLen nEnd1,  xub_StrLen nEnd2 ) const

{
    sal_uInt32 nThisRsid = GetRsid( nStt1, nEnd1 ? nEnd1 : nStt1 );
    sal_uInt32 nRsid = rTxtNode.GetRsid( nStt2, nEnd2 ? nEnd2 : nStt2 );

    return nThisRsid == nRsid;
}

// sw::Metadatable
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
