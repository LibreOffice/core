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


#include <svl/smplhint.hxx>
#include <hintids.hxx>
#include <sfx2/linkmgr.hxx>
#include <svl/itemiter.hxx>
#include <tools/resid.hxx>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <txtftn.hxx>
#include <fmtclds.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <rootfrm.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <section.hxx>
#include <UndoSection.hxx>
#include <UndoDelete.hxx>
#include <swundo.hxx>
#include <calc.hxx>
#include <swtable.hxx>
#include <swserv.hxx>
#include <frmfmt.hxx>
#include <frmtool.hxx>
#include <ftnidx.hxx>
#include <docary.hxx>
#include <redline.hxx>
#include <sectfrm.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <node2lay.hxx>
#include <doctxm.hxx>
#include <fmtftntx.hxx>
#include <comcore.hrc>
// #i27138#
#include <viewsh.hxx>
#include <txtfrm.hxx>


// #i21457# - new implementation of local method <lcl_IsInSameTblBox(..)>.
// Method now determines the previous/next on its own. Thus, it can be controlled,
// for which previous/next is checked, if it's visible.
bool lcl_IsInSameTblBox( SwNodes& _rNds,
                         const SwNode& _rNd,
                         const bool _bPrev )
{
    const SwTableNode* pTblNd = _rNd.FindTableNode();
    if ( !pTblNd )
    {
        return true;
    }

    // determine index to be checked. Its assumed that a previous/next exist.
    SwNodeIndex aChkIdx( _rNd );
    {
        // determine index of previous/next - skip hidden ones, which are
        // inside the table.
        // If found one is before/after table, this one isn't in the same
        // table box as <_rNd>.
        bool bFound = false;
        do
        {
            if ( _bPrev
                 ? !_rNds.GoPrevSection( &aChkIdx, sal_False, sal_False )
                 : !_rNds.GoNextSection( &aChkIdx, sal_False, sal_False ) )
            {
                OSL_FAIL( "<lcl_IsInSameTblBox(..)> - no previous/next!" );
                return false;
            }
            else
            {
                if ( aChkIdx < pTblNd->GetIndex() ||
                     aChkIdx > pTblNd->EndOfSectionNode()->GetIndex() )
                {
                    return false;
                }
                else
                {
                    // check, if found one isn't inside a hidden section, which
                    // is also inside the table.
                    SwSectionNode* pSectNd = aChkIdx.GetNode().FindSectionNode();
                    if ( !pSectNd ||
                         pSectNd->GetIndex() < pTblNd->GetIndex() ||
                         !pSectNd->GetSection().IsHiddenFlag() )
                    {
                        bFound = true;
                    }
                }
            }
        } while ( !bFound );
    }

    // dann suche den StartNode der Box
    const SwTableSortBoxes& rSortBoxes = pTblNd->GetTable().GetTabSortBoxes();
    sal_uLong nIdx = _rNd.GetIndex();
    for( sal_uInt16 n = 0; n < rSortBoxes.Count(); ++n )
    {
        const SwStartNode* pNd = rSortBoxes[ n ]->GetSttNd();
        if ( pNd->GetIndex() < nIdx && nIdx < pNd->EndOfSectionIndex() )
        {
            // dann muss der andere Index in derselben Section liegen
            nIdx = aChkIdx.GetIndex();
            return pNd->GetIndex() < nIdx && nIdx < pNd->EndOfSectionIndex();
        }
    }

    return true;
}

void lcl_CheckEmptyLayFrm( SwNodes& rNds, SwSectionData& rSectionData,
                        const SwNode& rStt, const SwNode& rEnd )
{
    SwNodeIndex aIdx( rStt );
    if( !rNds.GoPrevSection( &aIdx, sal_True, sal_False ) ||
        !CheckNodesRange( rStt, aIdx, sal_True ) ||
        // #i21457#
        !lcl_IsInSameTblBox( rNds, rStt, true ))
    {
        aIdx = rEnd;
        if( !rNds.GoNextSection( &aIdx, sal_True, sal_False ) ||
            !CheckNodesRange( rEnd, aIdx, sal_True ) ||
            // #i21457#
            !lcl_IsInSameTblBox( rNds, rEnd, false ))
        {
            rSectionData.SetHidden( false );
        }
    }
}

/* --------------------------------------------------
 * LockFrms wurde im InsertSection genutzt, um zu verhindern, dass
 * SectionFrms durch das DelFrms zerstoert werden. Dies ist durch
 * den Destroy-Listen-Mechanismus ueberfluessig geworden.
 * Falls diese Methode doch noch einmal reanimiert wird, bietet es
 * sich vielleicht an, beim Entlocken die SectionFrms auf Inhalt zu
 * pruefen und dann ggf. zur Zerstoerung anzumelden.
 * --------------------------------------------------*/

// und dann waren da noch die Fussnoten:
void lcl_DeleteFtn( SwSectionNode *pNd, sal_uLong nStt, sal_uLong nEnd )
{
    SwFtnIdxs& rFtnArr = pNd->GetDoc()->GetFtnIdxs();
    if( rFtnArr.Count() )
    {
        sal_uInt16 nPos;
        rFtnArr.SeekEntry( SwNodeIndex( *pNd ), &nPos );
        SwTxtFtn* pSrch;

        // loesche erstmal alle, die dahinter stehen
        while( nPos < rFtnArr.Count() &&
            _SwTxtFtn_GetIndex( (pSrch = rFtnArr[ nPos ]) ) <= nEnd )
        {
            // Werden die Nodes nicht geloescht mussen sie bei den Seiten
            // abmeldet (Frms loeschen) werden, denn sonst bleiben sie
            // stehen (Undo loescht sie nicht!)
            pSrch->DelFrms(0);
            ++nPos;
        }

        while( nPos-- &&
            _SwTxtFtn_GetIndex( (pSrch = rFtnArr[ nPos ]) ) >= nStt )
        {
            // Werden die Nodes nicht geloescht mussen sie bei den Seiten
            // abmeldet (Frms loeschen) werden, denn sonst bleiben sie
            // stehen (Undo loescht sie nicht!)
            pSrch->DelFrms(0);
        }
    }
}

static inline bool lcl_IsTOXSection(SwSectionData const& rSectionData)
{
    return (TOX_CONTENT_SECTION == rSectionData.GetType())
        || (TOX_HEADER_SECTION  == rSectionData.GetType());
}

SwSectionNode* SwNodes::InsertTextSection(SwNodeIndex const& rNdIdx,
                                SwSectionFmt& rSectionFmt,
                                SwSectionData const& rSectionData,
                                SwTOXBase const*const pTOXBase,
                                SwNodeIndex const*const pEnde,
                                bool const bInsAtStart, bool const bCreateFrms)
{
    SwNodeIndex aInsPos( rNdIdx );
    if( !pEnde )        // kein Bereich also neue Section davor/hinter anlegen
    {
        // #i26762#
        OSL_ENSURE(!pEnde || rNdIdx <= *pEnde,
               "Section start and end in wrong order!");

        if( bInsAtStart )
        {
            if (!lcl_IsTOXSection(rSectionData))
            {
                do {
                    aInsPos--;
                } while( aInsPos.GetNode().IsSectionNode() );
                aInsPos++;
            }
        }
        else
        {
            SwNode* pNd;
            aInsPos++;
            if (!lcl_IsTOXSection(rSectionData))
            {
                while( aInsPos.GetIndex() < Count() - 1 &&
                        ( pNd = &aInsPos.GetNode())->IsEndNode() &&
                        pNd->StartOfSectionNode()->IsSectionNode())
                {
                    aInsPos++;
                }
            }
        }
    }

    SwSectionNode *const pSectNd =
            new SwSectionNode(aInsPos, rSectionFmt, pTOXBase);
    if( pEnde )
    {
        // Sonderfall fuer die Reader/Writer
        if( &pEnde->GetNode() != &GetEndOfContent() )
            aInsPos = pEnde->GetIndex()+1;
        // #i58710: We created a RTF document with a section break inside a table cell
        // We are not able to handle a section start inside a table and the section end outside.
        const SwNode* pLastNode = pSectNd->StartOfSectionNode()->EndOfSectionNode();
        if( aInsPos > pLastNode->GetIndex() )
            aInsPos = pLastNode->GetIndex();
        // Another way round: if the section starts outside a table but the end is inside...
        // aInsPos is at the moment the Position where my EndNode will be inserted
        const SwStartNode* pStartNode = aInsPos.GetNode().StartOfSectionNode();
        // This StartNode should be in front of me, but if not, I wanna survive
        sal_uLong nMyIndex = pSectNd->GetIndex();
        if( pStartNode->GetIndex() > nMyIndex ) // Suspicious!
        {
            const SwNode* pTemp;
            do
            {
                pTemp = pStartNode; // pTemp is a suspicious one
                pStartNode = pStartNode->StartOfSectionNode();
            }
            while( pStartNode->GetIndex() > nMyIndex );
            pTemp = pTemp->EndOfSectionNode();
            // If it starts behind me but ends behind my end...
            if( pTemp->GetIndex() >= aInsPos.GetIndex() )
                aInsPos = pTemp->GetIndex()+1; // ...I have to correct my end position
        }

    }
    else
    {
        SwTxtNode* pCpyTNd = rNdIdx.GetNode().GetTxtNode();
        if( pCpyTNd )
        {
            SwTxtNode* pTNd = new SwTxtNode( aInsPos, pCpyTNd->GetTxtColl() );
            if( pCpyTNd->HasSwAttrSet() )
            {
                // Task 70955 - move PageDesc/Break to the first Node of the
                //              section
                const SfxItemSet& rSet = *pCpyTNd->GetpSwAttrSet();
                if( SFX_ITEM_SET == rSet.GetItemState( RES_BREAK ) ||
                    SFX_ITEM_SET == rSet.GetItemState( RES_PAGEDESC ))
                {
                    SfxItemSet aSet( rSet );
                    if( bInsAtStart )
                        pCpyTNd->ResetAttr( RES_PAGEDESC, RES_BREAK );
                    else
                    {
                        aSet.ClearItem( RES_PAGEDESC );
                        aSet.ClearItem( RES_BREAK );
                    }
                    pTNd->SetAttr( aSet );
                }
                else
                    pTNd->SetAttr( rSet );
            }
            // den Frame anlegen nicht vergessen !!
            pCpyTNd->MakeFrms( *pTNd );
        }
        else
            new SwTxtNode( aInsPos, (SwTxtFmtColl*)GetDoc()->GetDfltTxtFmtColl() );
    }
    new SwEndNode( aInsPos, *pSectNd );

    pSectNd->GetSection().SetSectionData(rSectionData);
    SwSectionFmt* pSectFmt = pSectNd->GetSection().GetFmt();

    // Hier bietet sich als Optimierung an, vorhandene Frames nicht zu
    // zerstoeren und wieder neu anzulegen, sondern nur umzuhaengen.
    sal_Bool bInsFrm = bCreateFrms && !pSectNd->GetSection().IsHidden() &&
                   GetDoc()->GetCurrentViewShell(); //swmod 071108//swmod 071225
    SwNode2Layout *pNode2Layout = NULL;
    if( bInsFrm )
    {
        SwNodeIndex aTmp( *pSectNd );
        if( !pSectNd->GetNodes().FindPrvNxtFrmNode( aTmp, pSectNd->EndOfSectionNode() ) )
            // dann sammel mal alle Uppers ein
            pNode2Layout = new SwNode2Layout( *pSectNd );
    }

    // jetzt noch bei allen im Bereich den richtigen StartNode setzen
    sal_uLong nEnde = pSectNd->EndOfSectionIndex();
    sal_uLong nStart = pSectNd->GetIndex()+1;
    sal_uLong nSkipIdx = ULONG_MAX;
    for( sal_uLong n = nStart; n < nEnde; ++n )
    {
        SwNode* pNd = (*this)[n];

        //JP 30.04.99: Bug 65644 - alle in der NodeSection liegenden
        //              Sections unter die neue haengen
        if( ULONG_MAX == nSkipIdx )
            pNd->pStartOfSection = pSectNd;
        else if( n >= nSkipIdx )
            nSkipIdx = ULONG_MAX;

        if( pNd->IsStartNode() )
        {
            // die Verschachtelung der Formate herstellen!
            if( pNd->IsSectionNode() )
            {
                ((SwSectionNode*)pNd)->GetSection().GetFmt()->
                                    SetDerivedFrom( pSectFmt );
                ((SwSectionNode*)pNd)->DelFrms();
                n = pNd->EndOfSectionIndex();
            }
            else
            {
                if( pNd->IsTableNode() )
                    ((SwTableNode*)pNd)->DelFrms();

                if( ULONG_MAX == nSkipIdx )
                    nSkipIdx = pNd->EndOfSectionIndex();
            }
        }
        else if( pNd->IsCntntNode() )
            ((SwCntntNode*)pNd)->DelFrms();
    }

    lcl_DeleteFtn( pSectNd, nStart, nEnde );

    if( bInsFrm )
    {
        if( pNode2Layout )
        {
            sal_uLong nIdx = pSectNd->GetIndex();
            pNode2Layout->RestoreUpperFrms( pSectNd->GetNodes(), nIdx, nIdx + 1 );
            delete pNode2Layout;
        }
        else
            pSectNd->MakeFrms( &aInsPos );
    }

    return pSectNd;
}

SwSectionNode* SwNode::FindSectionNode()
{
    if( IsSectionNode() )
        return GetSectionNode();
    SwStartNode* pTmp = pStartOfSection;
    while( !pTmp->IsSectionNode() && pTmp->GetIndex() )
        pTmp = pTmp->pStartOfSection;
    return pTmp->GetSectionNode();
}


//---------
// SwSectionNode
//---------

// ugly hack to make m_pSection const
static SwSectionFmt &
lcl_initParent(SwSectionNode & rThis, SwSectionFmt & rFmt)
{
    SwSectionNode *const pParent =
        rThis.StartOfSectionNode()->FindSectionNode();
    if( pParent )
    {
        // das Format beim richtigen Parent anmelden.
        rFmt.SetDerivedFrom( pParent->GetSection().GetFmt() );
    }
    return rFmt;
}

SwSectionNode::SwSectionNode(SwNodeIndex const& rIdx,
        SwSectionFmt & rFmt, SwTOXBase const*const pTOXBase)
    : SwStartNode( rIdx, ND_SECTIONNODE )
    , m_pSection( (pTOXBase)
        ? new SwTOXBaseSection(*pTOXBase, lcl_initParent(*this, rFmt))
        : new SwSection( CONTENT_SECTION, rFmt.GetName(),
                lcl_initParent(*this, rFmt) ) )
{
    // jetzt noch die Verbindung von Format zum Node setzen
    // Modify unterdruecken, interresiert keinen
    rFmt.LockModify();
    rFmt.SetFmtAttr( SwFmtCntnt( this ) );
    rFmt.UnlockModify();
}

#ifdef DBG_UTIL
//remove superfluous SectionFrms
SwFrm* SwClearDummies( SwFrm* pFrm )
{
    SwFrm* pTmp = pFrm;
    while( pTmp )
    {
        OSL_ENSURE( !pTmp->GetUpper(), "SwClearDummies: No Upper allowed!" );
        if( pTmp->IsSctFrm() )
        {
            SwSectionFrm* pSectFrm = (SwSectionFrm*)pFrm;
            pTmp = pTmp->GetNext();
            if( !pSectFrm->GetLower() )
            {
                if( pSectFrm->GetPrev() )
                    pSectFrm->GetPrev()->pNext = pTmp;
                else
                    pFrm = pTmp;
                if( pTmp )
                    pTmp->pPrev = pSectFrm->GetPrev();
                delete pSectFrm;
            }
        }
        else
            pTmp = pTmp->GetNext();
    }
    return pFrm;
}
#endif

SwSectionNode::~SwSectionNode()
{
    // mba: test if iteration works as clients will be removed in callback
    // use hint which allows to specify, if the content shall be saved or not
    m_pSection->GetFmt()->CallSwClientNotify( SwSectionFrmMoveAndDeleteHint( sal_True ) );
    SwSectionFmt* pFmt = m_pSection->GetFmt();
    if( pFmt )
    {
        // das Attribut entfernen, weil die Section ihr Format loescht
        // und falls das Cntnt-Attribut vorhanden ist, die Section aufhebt.
        pFmt->LockModify();
        pFmt->ResetFmtAttr( RES_CNTNT );
        pFmt->UnlockModify();
    }
}


SwFrm *SwSectionNode::MakeFrm( SwFrm *pSib )
{
    m_pSection->m_Data.SetHiddenFlag(false);
    return new SwSectionFrm( *m_pSection, pSib );
}

//Methode erzeugt fuer den vorhergehenden Node alle Ansichten vom
//Dokument. Die erzeugten Contentframes werden in das entsprechende
//Layout gehaengt.
void SwSectionNode::MakeFrms(const SwNodeIndex & rIdx )
{
    // also nehme meinen nachfolgenden oder vorhergehenden ContentFrame:
    SwNodes& rNds = GetNodes();
    if( rNds.IsDocNodes() && rNds.GetDoc()->GetCurrentViewShell() ) //swmod 071108//swmod 071225
    {
        if( GetSection().IsHidden() || IsCntntHidden() )
        {
            SwNodeIndex aIdx( *EndOfSectionNode() );
            SwCntntNode* pCNd = rNds.GoNextSection( &aIdx, sal_True, sal_False );
            if( !pCNd )
            {
                aIdx = *this;
                if( 0 == ( pCNd = rNds.GoPrevSection( &aIdx, sal_True, sal_False )) )
                    return ;
            }
            pCNd = aIdx.GetNode().GetCntntNode();
            pCNd->MakeFrms( (SwCntntNode&)rIdx.GetNode() );
        }
        else
        {
            SwNode2Layout aNode2Layout( *this, rIdx.GetIndex() );
            SwFrm *pFrm, *pNew;
            while( 0 != (pFrm = aNode2Layout.NextFrm()) )
            {
                OSL_ENSURE( pFrm->IsSctFrm(), "Depend von Section keine Section." );
                pNew = rIdx.GetNode().GetCntntNode()->MakeFrm( pFrm );

                SwSectionNode* pS = rIdx.GetNode().FindSectionNode();

                // Assure that node is not inside a table, which is inside the
                // found section.
                if ( pS )
                {
                    SwTableNode* pTableNode = rIdx.GetNode().FindTableNode();
                    if ( pTableNode &&
                         pTableNode->GetIndex() > pS->GetIndex() )
                    {
                        pS = 0;
                    }
                }

                // if the node is in a section, the sectionframe now
                // has to be created..
                // boolean to control <Init()> of a new section frame.
                bool bInitNewSect = false;
                if( pS )
                {
                    SwSectionFrm *pSct = new SwSectionFrm( pS->GetSection(), pFrm );
                    // prepare <Init()> of new section frame.
                    bInitNewSect = true;
                    SwLayoutFrm* pUp = pSct;
                    while( pUp->Lower() )  // for columned sections
                    {
                        OSL_ENSURE( pUp->Lower()->IsLayoutFrm(),"Who's in there?" );
                        pUp = (SwLayoutFrm*)pUp->Lower();
                    }
                    pNew->Paste( pUp, NULL );
                    // #i27138#
                    // notify accessibility paragraphs objects about changed
                    // CONTENT_FLOWS_FROM/_TO relation.
                    // Relation CONTENT_FLOWS_FROM for next paragraph will change
                    // and relation CONTENT_FLOWS_TO for previous paragraph will change.
                    if ( pNew->IsTxtFrm() )
                    {
                        ViewShell* pViewShell( pNew->getRootFrm()->GetCurrShell() );
                        if ( pViewShell && pViewShell->GetLayout() &&
                             pViewShell->GetLayout()->IsAnyShellAccessible() )
                        {
                            pViewShell->InvalidateAccessibleParaFlowRelation(
                                dynamic_cast<SwTxtFrm*>(pNew->FindNextCnt( true )),
                                dynamic_cast<SwTxtFrm*>(pNew->FindPrevCnt( true )) );
                        }
                    }
                    pNew = pSct;
                }

                // wird ein Node vorher oder nachher mit Frames versehen
                if ( rIdx < GetIndex() )
                    // der neue liegt vor mir
                    pNew->Paste( pFrm->GetUpper(), pFrm );
                else
                    // der neue liegt hinter mir
                    pNew->Paste( pFrm->GetUpper(), pFrm->GetNext() );
                // #i27138#
                // notify accessibility paragraphs objects about changed
                // CONTENT_FLOWS_FROM/_TO relation.
                // Relation CONTENT_FLOWS_FROM for next paragraph will change
                // and relation CONTENT_FLOWS_TO for previous paragraph will change.
                if ( pNew->IsTxtFrm() )
                {
                    ViewShell* pViewShell( pNew->getRootFrm()->GetCurrShell() );
                    if ( pViewShell && pViewShell->GetLayout() &&
                         pViewShell->GetLayout()->IsAnyShellAccessible() )
                    {
                        pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTxtFrm*>(pNew->FindNextCnt( true )),
                            dynamic_cast<SwTxtFrm*>(pNew->FindPrevCnt( true )) );
                    }
                }
                if ( bInitNewSect )
                    static_cast<SwSectionFrm*>(pNew)->Init();
            }
        }
    }
}

//Fuer jedes vorkommen im Layout einen SectionFrm anlegen und vor den
//entsprechenden CntntFrm pasten.

void SwSectionNode::MakeFrms( SwNodeIndex* pIdxBehind, SwNodeIndex* pEndIdx )
{
    OSL_ENSURE( pIdxBehind, "kein Index" );
    SwNodes& rNds = GetNodes();
    SwDoc* pDoc = rNds.GetDoc();

    *pIdxBehind = *this;

    m_pSection->m_Data.SetHiddenFlag(true);

    if( rNds.IsDocNodes() )
    {
        SwNodeIndex *pEnd = pEndIdx ? pEndIdx :
                            new SwNodeIndex( *EndOfSectionNode(), 1 );
        ::MakeFrms( pDoc, *pIdxBehind, *pEnd );
        if( !pEndIdx )
            delete pEnd;
    }

}

void SwSectionNode::DelFrms()
{
    sal_uLong nStt = GetIndex()+1, nEnd = EndOfSectionIndex();
    if( nStt >= nEnd )
    {
        return ;
    }

    SwNodes& rNds = GetNodes();
    m_pSection->GetFmt()->DelFrms();

    // unser Flag muessen wir noch aktualisieren
    m_pSection->m_Data.SetHiddenFlag(true);

    // Bug 30582: falls der Bereich in Fly oder TabellenBox ist, dann
    //              kann er nur "gehiddet" werden, wenn weiterer Content
    //              vorhanden ist, der "Frames" haelt. Sonst hat der
    //              Fly/TblBox-Frame keinen Lower !!!
    {
        SwNodeIndex aIdx( *this );
        if( !rNds.GoPrevSection( &aIdx, sal_True, sal_False ) ||
            !CheckNodesRange( *this, aIdx, sal_True ) ||
            // #i21457#
            !lcl_IsInSameTblBox( rNds, *this, true ))
        {
            aIdx = *EndOfSectionNode();
            if( !rNds.GoNextSection( &aIdx, sal_True, sal_False ) ||
                !CheckNodesRange( *EndOfSectionNode(), aIdx, sal_True ) ||
                // #i21457#
                !lcl_IsInSameTblBox( rNds, *EndOfSectionNode(), false ))
            {
                m_pSection->m_Data.SetHiddenFlag(false);
            }
        }
    }
}

SwSectionNode* SwSectionNode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
{
    // in welchen Array steht ich denn: Nodes, UndoNodes ??
    const SwNodes& rNds = GetNodes();

    // das SectionFrmFmt kopieren
    SwSectionFmt* pSectFmt = pDoc->MakeSectionFmt( 0 );
    pSectFmt->CopyAttrs( *GetSection().GetFmt() );

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SwTOXBase> pTOXBase;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if (TOX_CONTENT_SECTION == GetSection().GetType())
    {
        OSL_ENSURE( GetSection().ISA( SwTOXBaseSection ), "no TOXBaseSection!" );
        SwTOXBaseSection const& rTBS(
            dynamic_cast<SwTOXBaseSection const&>(GetSection()));
        pTOXBase.reset( new SwTOXBase(rTBS, pDoc) );
    }

    SwSectionNode *const pSectNd =
        new SwSectionNode(rIdx, *pSectFmt, pTOXBase.get());
    SwEndNode* pEndNd = new SwEndNode( rIdx, *pSectNd );
    SwNodeIndex aInsPos( *pEndNd );

    // Werte uebertragen
    SwSection *const pNewSect = pSectNd->m_pSection.get();

    if (TOX_CONTENT_SECTION != GetSection().GetType())
    {
        // beim Move den Namen beibehalten
        if( rNds.GetDoc() == pDoc && pDoc->IsCopyIsMove() )
        {
            pNewSect->SetSectionName( GetSection().GetSectionName() );
        }
        else
        {
            pNewSect->SetSectionName(
                pDoc->GetUniqueSectionName( &GetSection().GetSectionName() ));
        }
    }


    pNewSect->SetType( GetSection().GetType() );
    pNewSect->SetCondition( GetSection().GetCondition() );
    pNewSect->SetLinkFileName( GetSection().GetLinkFileName() );
    if( !pNewSect->IsHiddenFlag() && GetSection().IsHidden() )
        pNewSect->SetHidden( sal_True );
    if( !pNewSect->IsProtectFlag() && GetSection().IsProtect() )
        pNewSect->SetProtect( sal_True );
    // edit in readonly sections
    if( !pNewSect->IsEditInReadonlyFlag() && GetSection().IsEditInReadonly() )
        pNewSect->SetEditInReadonly( sal_True );

    SwNodeRange aRg( *this, +1, *EndOfSectionNode() );  // (wo stehe in denn nun ??)
    rNds._Copy( aRg, aInsPos, sal_False );

    // loesche alle Frames vom kopierten Bereich, diese werden beim
    // erzeugen des SectionFrames angelegt !
    pSectNd->DelFrms();

    // dann kopiere auch noch die Links/Server
    if( pNewSect->IsLinkType() )        // den Link eintragen
        pNewSect->CreateLink( pDoc->GetCurrentViewShell() ? CREATE_CONNECT  //swmod 071108//swmod 071225
                                                 : CREATE_NONE );

    // falls als Server aus dem Undo kopiert wird, wieder eintragen
    if (m_pSection->IsServer()
        && pDoc->GetIDocumentUndoRedo().IsUndoNodes(rNds))
    {
        pNewSect->SetRefObject( m_pSection->GetObject() );
        pDoc->GetLinkManager().InsertServer( pNewSect->GetObject() );
    }

    // METADATA: copy xml:id; must be done after insertion of node
    pSectFmt->RegisterAsCopyOf(*GetSection().GetFmt());

    return pSectNd;
}

sal_Bool SwSectionNode::IsCntntHidden() const
{
    OSL_ENSURE( !m_pSection->IsHidden(),
            "That's simple: Hidden Section => Hidden Content" );
    SwNodeIndex aTmp( *this, 1 );
    sal_uLong nEnd = EndOfSectionIndex();
    while( aTmp < nEnd )
    {
        if( aTmp.GetNode().IsSectionNode() )
        {
            const SwSection& rSect = ((SwSectionNode&)aTmp.GetNode()).GetSection();
            if( rSect.IsHiddenFlag() )
                // dann diese Section ueberspringen
                aTmp = *aTmp.GetNode().EndOfSectionNode();
        }
        else
        {
            if( aTmp.GetNode().IsCntntNode() || aTmp.GetNode().IsTableNode() )
                return sal_False; // Nicht versteckter Inhalt wurde gefunden
            OSL_ENSURE( aTmp.GetNode().IsEndNode(), "EndNode expected" );
        }
        aTmp++;
    }
    return sal_True; // Alles versteckt
}


void SwSectionNode::NodesArrChgd()
{
    SwSectionFmt *const pFmt = m_pSection->GetFmt();
    if( pFmt )
    {
        SwNodes& rNds = GetNodes();
        SwDoc* pDoc = pFmt->GetDoc();

        if( !rNds.IsDocNodes() )
        {
            SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT, pFmt );
            pFmt->ModifyNotification( &aMsgHint, &aMsgHint );
        }

        pFmt->LockModify();
        pFmt->SetFmtAttr( SwFmtCntnt( this ));
        pFmt->UnlockModify();

        SwSectionNode* pSectNd = StartOfSectionNode()->FindSectionNode();
        // set the correct parent from the new section
        pFmt->SetDerivedFrom( pSectNd ? pSectNd->GetSection().GetFmt()
                                      : pDoc->GetDfltFrmFmt() );

        // jetzt noch bei allen im Bereich den richtigen StartNode setzen
        sal_uLong nStart = GetIndex()+1, nEnde = EndOfSectionIndex();
        for( sal_uLong n = nStart; n < nEnde; ++n )
            // die Verschachtelung der Formate herstellen!
            if( 0 != ( pSectNd = rNds[ n ]->GetSectionNode() ) )
            {
                pSectNd->GetSection().GetFmt()->SetDerivedFrom( pFmt );
                n = pSectNd->EndOfSectionIndex();
            }

        // verschieben vom Nodes- ins UndoNodes-Array?
        if( rNds.IsDocNodes() )
        {
            OSL_ENSURE( pDoc == GetDoc(),
                    "verschieben in unterschiedliche Documente?" );
            if( m_pSection->IsLinkType() )      // den Link austragen
                m_pSection->CreateLink( pDoc->GetCurrentViewShell() ? CREATE_CONNECT    //swmod 071108
                                                         : CREATE_NONE );//swmod 071225
            if (m_pSection->IsServer())
            {
                pDoc->GetLinkManager().InsertServer( m_pSection->GetObject() );
            }
        }
        else
        {
            if (CONTENT_SECTION != m_pSection->GetType())
            {
                pDoc->GetLinkManager().Remove( &m_pSection->GetBaseLink() );
            }

            if (m_pSection->IsServer())
            {
                pDoc->GetLinkManager().RemoveServer( m_pSection->GetObject() );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
