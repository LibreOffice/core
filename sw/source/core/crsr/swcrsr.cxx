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

#include <hintids.hxx>
#include <editeng/protitem.hxx>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <unotools/charclass.hxx>
#include <svl/ctloptions.hxx>
#include <svl/srchitem.hxx>
#include <swmodule.hxx>
#include <fmtcntnt.hxx>
#include <swtblfmt.hxx>
#include <swcrsr.hxx>
#include <unocrsr.hxx>
#include <bookmark.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>
#include <section.hxx>
#include <swtable.hxx>
#include <cntfrm.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <notxtfrm.hxx>
#include <scriptinfo.hxx>
#include <crstate.hxx>
#include <docsh.hxx>
#include <viewsh.hxx>
#include <frmatr.hxx>
#include <breakit.hxx>
#include <mdiexp.hxx>
#include <strings.hrc>
#include <redline.hxx>
#include <txatbase.hxx>
#include <IDocumentMarkAccess.hxx>
#include <memory>
#include <comphelper/lok.hxx>
#include <editsh.hxx>

#include <viewopt.hxx>

using namespace ::com::sun::star::i18n;

const sal_uInt16 coSrchRplcThreshold = 60000;

namespace {

struct PercentHdl
{
    SwDocShell* pDSh;
    sal_uLong nActPos;
    bool bBack, bNodeIdx;

    PercentHdl( sal_uLong nStt, sal_uLong nEnd, SwDocShell* pSh )
        : pDSh(pSh), nActPos(nStt), bBack(false), bNodeIdx(false)
    {
        bBack = (nStt > nEnd);
        if( bBack )
            std::swap( nStt, nEnd );
        ::StartProgress( STR_STATSTR_SEARCH, nStt, nEnd );
    }

    explicit PercentHdl( const SwPaM& rPam )
        : pDSh( rPam.GetDoc().GetDocShell() )
    {
        sal_Int32 nStt, nEnd;
        if( rPam.GetPoint()->GetNode() == rPam.GetMark()->GetNode() )
        {
            bNodeIdx = false;
            nStt = rPam.GetMark()->GetContentIndex();
            nEnd = rPam.GetPoint()->GetContentIndex();
        }
        else
        {
            bNodeIdx = true;
            nStt = sal_Int32(rPam.GetMark()->GetNodeIndex());
            nEnd = sal_Int32(rPam.GetPoint()->GetNodeIndex());
        }
        nActPos = nStt;
        bBack = (nStt > nEnd );
        if( bBack )
            std::swap( nStt, nEnd );
        ::StartProgress( STR_STATSTR_SEARCH, nStt, nEnd, pDSh );
    }

    ~PercentHdl()                      { ::EndProgress( pDSh ); }

    void NextPos( sal_uLong nPos ) const
        { ::SetProgressState( bBack ? nActPos - nPos : nPos, pDSh ); }

    void NextPos( SwPosition const & rPos ) const
        {
            sal_Int32 nPos;
            if( bNodeIdx )
                nPos = sal_Int32(rPos.GetNodeIndex());
            else
                nPos = rPos.GetContentIndex();
            ::SetProgressState( bBack ? nActPos - nPos : nPos, pDSh );
        }
};

}

SwCursor::SwCursor( const SwPosition &rPos, SwPaM* pRing )
    : SwPaM( rPos, pRing )
    , m_nRowSpanOffset(0)
    , m_nCursorBidiLevel(0)
    , m_bColumnSelection(false)
{
}

// @@@ semantic: no copy ctor.
SwCursor::SwCursor(SwCursor const& rCpy, SwPaM *const pRing)
    : SwPaM( rCpy, pRing )
    , m_nRowSpanOffset(rCpy.m_nRowSpanOffset)
    , m_nCursorBidiLevel(rCpy.m_nCursorBidiLevel)
    , m_bColumnSelection(rCpy.m_bColumnSelection)
{
}

SwCursor::~SwCursor()
{
}

SwCursor* SwCursor::Create( SwPaM* pRing ) const
{
    return new SwCursor( *GetPoint(), pRing );
}

bool SwCursor::IsReadOnlyAvailable() const
{
    return false;
}

bool SwCursor::IsSkipOverHiddenSections() const
{
    return true;
}

bool SwCursor::IsSkipOverProtectSections() const
{
    return !IsReadOnlyAvailable();
}

// CreateNewSavePos is virtual so that derived classes of cursor can implement
// own SaveObjects if needed and validate them in the virtual check routines.
void SwCursor::SaveState()
{
    m_vSavePos.emplace_back( *this );
}

void SwCursor::RestoreState()
{
    if (!m_vSavePos.empty()) // Robust
    {
        m_vSavePos.pop_back();
    }
}

/// determine if point is outside of the node-array's content area
bool SwCursor::IsNoContent() const
{
    return GetPoint()->GetNodeIndex() <
            GetDoc().GetNodes().GetEndOfExtras().GetIndex();
}

bool SwCursor::IsSelOvrCheck(SwCursorSelOverFlags)
{
    return false;
}

// extracted from IsSelOvr()
bool SwTableCursor::IsSelOvrCheck(SwCursorSelOverFlags eFlags)
{
    SwNodes& rNds = GetDoc().GetNodes();
    // check sections of nodes array
    if( (SwCursorSelOverFlags::CheckNodeSection & eFlags)
        && HasMark() )
    {
        SwNodeIndex aOldPos( rNds, GetSavePos()->nNode );
        if( !CheckNodesRange( aOldPos.GetNode(), GetPoint()->GetNode(), true ))
        {
            GetPoint()->Assign( aOldPos );
            GetPoint()->SetContent( GetSavePos()->nContent );
            return true;
        }
    }
    return SwCursor::IsSelOvrCheck(eFlags);
}

namespace
{
    const SwTextAttr* InputFieldAtPos(SwPosition const *pPos)
    {
        SwTextNode* pTextNd = pPos->GetNode().GetTextNode();
        if (!pTextNd)
            return nullptr;
        return pTextNd->GetTextAttrAt(pPos->GetContentIndex(), RES_TXTATR_INPUTFIELD, ::sw::GetTextAttrMode::Parent);
    }
}

bool SwCursor::IsSelOvr(SwCursorSelOverFlags const eFlags)
{
    SwDoc& rDoc = GetDoc();
    SwNodes& rNds = rDoc.GetNodes();

    bool bSkipOverHiddenSections = IsSkipOverHiddenSections();
    bool bSkipOverProtectSections = IsSkipOverProtectSections();

    if ( IsSelOvrCheck( eFlags ) )
    {
        return true;
    }

    if (m_vSavePos.back().nNode != GetPoint()->GetNodeIndex() &&
        // (1997) in UI-ReadOnly everything is allowed
        ( !rDoc.GetDocShell() || !rDoc.GetDocShell()->IsReadOnlyUI() ))
    {
        // check new sections
        SwPosition& rPtPos = *GetPoint();
        const SwSectionNode* pSectNd = rPtPos.GetNode().FindSectionNode();
        if( pSectNd &&
            ((bSkipOverHiddenSections && pSectNd->GetSection().IsHiddenFlag() ) ||
            (bSkipOverProtectSections && pSectNd->GetSection().IsProtectFlag() )))
        {
            if( !( SwCursorSelOverFlags::ChangePos & eFlags ) )
            {
                // then we're already done
                RestoreSavePos();
                return true;
            }

            // set cursor to new position:
            SwNodeIndex aIdx( rPtPos.GetNode() );
            sal_Int32 nContentPos = m_vSavePos.back().nContent;
            bool bGoNxt = m_vSavePos.back().nNode < rPtPos.GetNodeIndex();
            SwContentNode* pCNd = bGoNxt
                ? SwNodes::GoNextSection( &rPtPos, bSkipOverHiddenSections, bSkipOverProtectSections)
                : SwNodes::GoPrevSection( &rPtPos, bSkipOverHiddenSections, bSkipOverProtectSections);
            if( !pCNd && ( SwCursorSelOverFlags::EnableRevDirection & eFlags ))
            {
                bGoNxt = !bGoNxt;
                pCNd = bGoNxt ? SwNodes::GoNextSection( &rPtPos, bSkipOverHiddenSections, bSkipOverProtectSections)
                    : SwNodes::GoPrevSection( &rPtPos, bSkipOverHiddenSections, bSkipOverProtectSections);
            }

            bool bIsValidPos = nullptr != pCNd;
            const bool bValidNodesRange = bIsValidPos &&
                ::CheckNodesRange( rPtPos.GetNode(), aIdx.GetNode(), true );
            if( !bValidNodesRange )
            {
                rPtPos.Assign( m_vSavePos.back().nNode );
                pCNd = rPtPos.GetNode().GetContentNode();
                if( !pCNd )
                {
                    bIsValidPos = false;
                    nContentPos = 0;
                    rPtPos.Assign( aIdx );
                    pCNd = rPtPos.GetNode().GetContentNode();
                    if( !pCNd )
                    {
                        // then to the beginning of the document
                        rPtPos.Assign( rNds.GetEndOfExtras() );
                        pCNd = SwNodes::GoNext(&rPtPos);
                    }
                }
            }

            // register ContentIndex:
            const sal_Int32 nTmpPos = bIsValidPos ? (bGoNxt ? 0 : pCNd->Len()) : nContentPos;
            GetPoint()->SetContent( nTmpPos );
            if( !bIsValidPos || !bValidNodesRange ||
                IsInProtectTable( true ) )
                return true;
        }

        // is there a protected section in the section?
        if( HasMark() && bSkipOverProtectSections)
        {
            SwNodeOffset nSttIdx = GetMark()->GetNodeIndex(),
                nEndIdx = GetPoint()->GetNodeIndex();
            if( nEndIdx <= nSttIdx )
                std::swap( nSttIdx, nEndIdx );

            const SwSectionFormats& rFormats = rDoc.GetSections();
            for( SwSectionFormats::size_type n = 0; n < rFormats.size(); ++n )
            {
                const SwSectionFormat* pFormat = rFormats[n];
                const SvxProtectItem& rProtect = pFormat->GetProtect();
                if( rProtect.IsContentProtected() )
                {
                    const SwFormatContent& rContent = pFormat->GetContent(false);
                    OSL_ENSURE( rContent.GetContentIdx(), "No SectionNode?" );
                    SwNodeOffset nIdx = rContent.GetContentIdx()->GetIndex();
                    if( nSttIdx <= nIdx && nEndIdx >= nIdx )
                    {
                        // if it is no linked section then we cannot select it
                        const SwSection& rSect = *pFormat->GetSection();
                        if( SectionType::Content == rSect.GetType() )
                        {
                            RestoreSavePos();
                            return true;
                        }
                    }
                }
            }
        }
    }

    const SwNode* pNd = &GetPoint()->GetNode();
    if( pNd->IsContentNode() && !dynamic_cast<SwUnoCursor*>(this) )
    {
        const SwContentFrame* pFrame = static_cast<const SwContentNode*>(pNd)->getLayoutFrame( rDoc.getIDocumentLayoutAccess().GetCurrentLayout() );
        if ( (SwCursorSelOverFlags::ChangePos & eFlags)   //allowed to change position if it's a bad one
            && pFrame && pFrame->isFrameAreaDefinitionValid()
            && !pFrame->getFrameArea().Height()     //a bad zero height position
            && !InputFieldAtPos(GetPoint()) )                       //unless it's a (vertical) input field
        {
            // skip to the next/prev valid paragraph with a layout
            SwPosition& rPtPos = *GetPoint();
            bool bGoNxt = m_vSavePos.back().nNode < rPtPos.GetNodeIndex();
            for (;;)
            {
                pFrame = bGoNxt ? pFrame->FindNextCnt(true) : pFrame->FindPrevCnt();
                if (!pFrame || 0 != pFrame->getFrameArea().Height() )
                    break;
            }

            // #i72394# skip to prev/next valid paragraph with a layout in case
            // the first search did not succeed:
            if( !pFrame )
            {
                bGoNxt = !bGoNxt;
                pFrame = static_cast<const SwContentNode*>(pNd)->getLayoutFrame( rDoc.getIDocumentLayoutAccess().GetCurrentLayout() );
                while ( pFrame && 0 == pFrame->getFrameArea().Height() )
                {
                    pFrame = bGoNxt ? pFrame->FindNextCnt(true) : pFrame->FindPrevCnt();
                }
            }

            if (pFrame != nullptr)
            {
                if (pFrame->IsTextFrame())
                {
                    SwTextFrame const*const pTextFrame(static_cast<SwTextFrame const*>(pFrame));
                    *GetPoint() = pTextFrame->MapViewToModelPos(TextFrameIndex(
                            bGoNxt ? 0 : pTextFrame->GetText().getLength()));
                }
                else
                {
                    assert(pFrame->IsNoTextFrame());
                    SwContentNode *const pCNd = const_cast<SwContentNode*>(
                        static_cast<SwNoTextFrame const*>(pFrame)->GetNode());
                    assert(pCNd);

                    // set this ContentNode as new position
                    rPtPos.Assign( *pCNd );
                    // assign corresponding ContentIndex
                    const sal_Int32 nTmpPos = bGoNxt ? 0 : pCNd->Len();
                    GetPoint()->SetContent( nTmpPos );
                }


                if (rPtPos.GetNodeIndex() == m_vSavePos.back().nNode
                    && GetPoint()->GetContentIndex() == m_vSavePos.back().nContent)
                {
                    // new position equals saved one
                    // --> trigger restore of saved pos by setting <pFrame> to NULL - see below
                    pFrame = nullptr;
                }

                if ( IsInProtectTable( true ) )
                {
                    // new position in protected table
                    // --> trigger restore of saved pos by setting <pFrame> to NULL - see below
                    pFrame = nullptr;
                }
            }
        }

        if( !pFrame )
        {
            assert(!m_vSavePos.empty());
            SwContentNode const*const pSaveNode(rNds[m_vSavePos.back().nNode]->GetContentNode());
            // if the old position already didn't have a frame, allow moving
            // anyway, hope the caller can handle that
            if (pSaveNode && pSaveNode->getLayoutFrame(rDoc.getIDocumentLayoutAccess().GetCurrentLayout()))
            {
                DeleteMark();
                RestoreSavePos();
                return true; // we need a frame
            }
        }
    }

    // is the cursor allowed to be in a protected node?
    if( !( SwCursorSelOverFlags::ChangePos & eFlags ) && !IsAtValidPos() )
    {
        DeleteMark();
        RestoreSavePos();
        return true;
    }

    if( !HasMark() )
        return false;

    // check for invalid sections
    if( !::CheckNodesRange( GetMark()->GetNode(), GetPoint()->GetNode(), true ))
    {
        DeleteMark();
        RestoreSavePos();
        return true; // we need a frame
    }

    pNd = &GetMark()->GetNode();
    if( pNd->IsContentNode()
        && !static_cast<const SwContentNode*>(pNd)->getLayoutFrame( rDoc.getIDocumentLayoutAccess().GetCurrentLayout() )
        && !dynamic_cast<SwUnoCursor*>(this) )
    {
        DeleteMark();
        RestoreSavePos();
        return true; // we need a frame
    }

    // ensure that selection is only inside an InputField or contains the InputField completely
    {
        const SwTextAttr* pInputFieldTextAttrAtPoint = InputFieldAtPos(GetPoint());
        const SwTextAttr* pInputFieldTextAttrAtMark = InputFieldAtPos(GetMark());

        if ( pInputFieldTextAttrAtPoint != pInputFieldTextAttrAtMark )
        {
            const SwNodeOffset nRefNodeIdx =
                ( SwCursorSelOverFlags::Toggle & eFlags )
                ? m_vSavePos.back().nNode
                : GetMark()->GetNodeIndex();
            const sal_Int32 nRefContentIdx =
                ( SwCursorSelOverFlags::Toggle & eFlags )
                ? m_vSavePos.back().nContent
                : GetMark()->GetContentIndex();
            const bool bIsForwardSelection =
                nRefNodeIdx < GetPoint()->GetNodeIndex()
                || ( nRefNodeIdx == GetPoint()->GetNodeIndex()
                     && nRefContentIdx < GetPoint()->GetContentIndex() );

            if ( pInputFieldTextAttrAtPoint != nullptr )
            {
                const sal_Int32 nNewPointPos =
                    bIsForwardSelection ? *(pInputFieldTextAttrAtPoint->End()) : pInputFieldTextAttrAtPoint->GetStart();
                GetPoint()->SetContent( nNewPointPos );
            }

            if ( pInputFieldTextAttrAtMark != nullptr )
            {
                const sal_Int32 nNewMarkPos =
                    bIsForwardSelection ? pInputFieldTextAttrAtMark->GetStart() : *(pInputFieldTextAttrAtMark->End());
                GetMark()->SetContent( nNewMarkPos );
            }
        }
    }

    const SwTableNode* pPtNd = GetPoint()->GetNode().FindTableNode();
    const SwTableNode* pMrkNd = GetMark()->GetNode().FindTableNode();
    // both in no or in same table node
    if( ( !pMrkNd && !pPtNd ) || pPtNd == pMrkNd )
        return false;

    // in different tables or only mark in table
    if( pMrkNd )
    {
        // not allowed, so go back to old position
        RestoreSavePos();
        // Cursor stays at old position
        return true;
    }

    // Note: this cannot happen in TableMode
    // Only Point in Table then go behind/in front of table
    if (SwCursorSelOverFlags::ChangePos & eFlags)
    {
        bool bSelTop = GetPoint()->GetNodeIndex() <
            ((SwCursorSelOverFlags::Toggle & eFlags)
                 ? m_vSavePos.back().nNode : GetMark()->GetNodeIndex());

        do { // loop for table after table
            SwNodeOffset nSEIdx = pPtNd->EndOfSectionIndex();
            SwNodeOffset nSttEndTable = nSEIdx + 1;

            if( bSelTop )
                nSttEndTable = rNds[ nSEIdx ]->StartOfSectionIndex() - 1;

            GetPoint()->Assign( nSttEndTable );
            const SwNode* pMyNd = &(GetPointNode());

            if( pMyNd->IsSectionNode() || ( pMyNd->IsEndNode() &&
                pMyNd->StartOfSectionNode()->IsSectionNode() ) )
            {
                pMyNd = bSelTop
                    ? SwNodes::GoPrevSection( GetPoint(),true,false )
                    : SwNodes::GoNextSection( GetPoint(),true,false );

                /* #i12312# Handle failure of Go{Prev|Next}Section */
                if ( nullptr == pMyNd)
                    break;

                pPtNd = pMyNd->FindTableNode();
                if( pPtNd )
                    continue;
            }

            // we permit these
            if( pMyNd->IsContentNode() &&
                ::CheckNodesRange( GetMark()->GetNode(),
                GetPoint()->GetNode(), true ))
            {
                // table in table
                const SwTableNode* pOuterTableNd = pMyNd->FindTableNode();
                if ( pOuterTableNd )
                    pMyNd = pOuterTableNd;
                else
                {
                    SwContentNode* pCNd = const_cast<SwContentNode*>(static_cast<const SwContentNode*>(pMyNd));
                    GetPoint()->SetContent( bSelTop ? pCNd->Len() : 0 );
                    return false;
                }
            }
            if( bSelTop )
            {
                if ( !pMyNd->IsEndNode() )
                    break;
                pPtNd = pMyNd->FindTableNode();
            }
            else
                pPtNd = pMyNd->GetTableNode();
            if (!pPtNd)
                break;
        } while( true );
    }

    // stay on old position
    RestoreSavePos();
    return true;
}

bool SwCursor::IsInProtectTable( bool bMove, bool bChgCursor )
{
    SwContentNode* pCNd = GetPointContentNode();
    if( !pCNd )
        return false;

    // No table, no protected cell:
    const SwTableNode* pTableNode = pCNd->FindTableNode();
    if ( !pTableNode )
        return false;

    // Current position == last save position?
    if (m_vSavePos.back().nNode == GetPoint()->GetNodeIndex())
        return false;

    // Check for covered cell:
    bool bInCoveredCell = false;
    const SwStartNode* pTmpSttNode = pCNd->FindTableBoxStartNode();
    OSL_ENSURE( pTmpSttNode, "In table, therefore I expect to get a SwTableBoxStartNode" );
    const SwTableBox* pBox = pTmpSttNode ? pTableNode->GetTable().GetTableBox( pTmpSttNode->GetIndex() ) : nullptr; //Robust #151355
    if ( pBox && pBox->getRowSpan() < 1 ) // Robust #151270
        bInCoveredCell = true;

    // Positions of covered cells are not acceptable:
    if ( !bInCoveredCell )
    {
        // Position not protected?
        if ( !pCNd->IsProtect() )
            return false;

        // Cursor in protected cells allowed?
        if ( IsReadOnlyAvailable() )
            return false;
    }

    // If we reach this point, we are in a protected or covered table cell!

    if( !bMove )
    {
        if( bChgCursor )
            // restore the last save position
            RestoreSavePos();

        return true; // Cursor stays at old position
    }

    // We are in a protected table cell. Traverse top to bottom?
    if (m_vSavePos.back().nNode < GetPoint()->GetNodeIndex())
    {
        // search next valid box
        // if there is another StartNode after the EndNode of a cell then
        // there is another cell
        SwNodeIndex aCellStt( *GetPointNode().FindTableBoxStartNode()->EndOfSectionNode(), 1 );
        bool bProt = true;
GoNextCell:
        for (;;) {
            if( !aCellStt.GetNode().IsStartNode() )
                break;
            ++aCellStt;
            pCNd = aCellStt.GetNode().GetContentNode();
            if( !pCNd )
                pCNd = SwNodes::GoNext(&aCellStt);
            bProt = pCNd->IsProtect();
            if( !bProt )
                break;
            aCellStt.Assign( *pCNd->FindTableBoxStartNode()->EndOfSectionNode(), 1 );
        }

SetNextCursor:
        if( !bProt ) // found free cell
        {
            GetPoint()->Assign( aCellStt );
            SwContentNode* pTmpCNd = GetPointContentNode();
            if( pTmpCNd )
            {
                GetPoint()->SetContent( 0 );
                return false;
            }
            return IsSelOvr( SwCursorSelOverFlags::Toggle |
                             SwCursorSelOverFlags::ChangePos );
        }
        // end of table, so go to next node
        ++aCellStt;
        SwNode* pNd = &aCellStt.GetNode();
        if( pNd->IsEndNode() || HasMark())
        {
            // if only table in FlyFrame or SSelection then stay on old position
            if( bChgCursor )
                RestoreSavePos();
            return true;
        }
        else if( pNd->IsTableNode() )
        {
            ++aCellStt;
            goto GoNextCell;
        }

        bProt = false; // index is now on a content node
        goto SetNextCursor;
    }

    // search for the previous valid box
    {
        // if there is another EndNode in front of the StartNode than there
        // exists a previous cell
        SwNodeIndex aCellStt( *GetPointNode().FindTableBoxStartNode(), -1 );
        SwNode* pNd;
        bool bProt = true;
GoPrevCell:
        for (;;) {
            pNd = &aCellStt.GetNode();
            if( !pNd->IsEndNode() )
                break;
            aCellStt.Assign( *pNd->StartOfSectionNode(), +1 );
            pCNd = aCellStt.GetNode().GetContentNode();
            if( !pCNd )
                pCNd = SwNodes::GoNext(&aCellStt);
            bProt = pCNd->IsProtect();
            if( !bProt )
                break;
            aCellStt.Assign( *pNd->FindTableBoxStartNode(), -1 );
        }

SetPrevCursor:
        if( !bProt ) // found free cell
        {
            GetPoint()->Assign( aCellStt );
            SwContentNode* pTmpCNd = GetPointContentNode();
            if( pTmpCNd )
            {
                GetPoint()->SetContent(  0 );
                return false;
            }
            return IsSelOvr( SwCursorSelOverFlags::Toggle |
                             SwCursorSelOverFlags::ChangePos );
        }
        // at the beginning of a table, so go to next node
        --aCellStt;
        pNd = &aCellStt.GetNode();
        if( pNd->IsStartNode() || HasMark() )
        {
            // if only table in FlyFrame or SSelection then stay on old position
            if( bChgCursor )
                RestoreSavePos();
            return true;
        }
        else if( pNd->StartOfSectionNode()->IsTableNode() )
        {
            --aCellStt;
            goto GoPrevCell;
        }

        bProt = false; // index is now on a content node
        goto SetPrevCursor;
    }
}

/// Return <true> if cursor can be set to this position
bool SwCursor::IsAtValidPos( bool bPoint ) const
{
    const SwDoc& rDoc = GetDoc();
    const SwPosition* pPos = bPoint ? GetPoint() : GetMark();
    const SwNode* pNd = &pPos->GetNode();

    if( pNd->IsContentNode() && !static_cast<const SwContentNode*>(pNd)->getLayoutFrame( rDoc.getIDocumentLayoutAccess().GetCurrentLayout() ) &&
        !dynamic_cast<const SwUnoCursor*>(this) )
    {
        return false;
    }

    // #i45129# - in UI-ReadOnly everything is allowed
    if( !rDoc.GetDocShell() || !rDoc.GetDocShell()->IsReadOnlyUI() )
        return true;

    const bool bCursorInReadOnly = IsReadOnlyAvailable();
    if( !bCursorInReadOnly && pNd->IsProtect() )
        return false;

    const SwSectionNode* pSectNd = pNd->FindSectionNode();
    return !pSectNd
           || !(pSectNd->GetSection().IsHiddenFlag() ||
                ( !bCursorInReadOnly && pSectNd->GetSection().IsProtectFlag() ));
}

void SwCursor::SaveTableBoxContent( const SwPosition* ) {}

/// set range for search in document
SwMoveFnCollection const & SwCursor::MakeFindRange( SwDocPositions nStart,
                                SwDocPositions nEnd, SwPaM* pRange ) const
{
    pRange->SetMark();
    FillFindPos( nStart, *pRange->GetMark() );
    FillFindPos( nEnd, *pRange->GetPoint() );

    // determine direction of search
    return ( SwDocPositions::Start == nStart || SwDocPositions::OtherStart == nStart ||
              (SwDocPositions::Curr == nStart &&
                (SwDocPositions::End == nEnd || SwDocPositions::OtherEnd == nEnd ) ))
                ? fnMoveForward : fnMoveBackward;
}

static sal_Int32 lcl_FindSelection( SwFindParas& rParas, SwCursor* pCurrentCursor,
                        SwMoveFnCollection const & fnMove, SwCursor*& pFndRing,
                        SwPaM& aRegion, FindRanges eFndRngs,
                        bool bInReadOnly, bool& bCancel )
{
    SwDoc& rDoc = pCurrentCursor->GetDoc();
    bool const bDoesUndo = rDoc.GetIDocumentUndoRedo().DoesUndo();
    int nFndRet = 0;
    sal_Int32 nFound = 0;
    const bool bSrchBkwrd = &fnMove == &fnMoveBackward;
    SwPaM *pTmpCursor = pCurrentCursor, *pSaveCursor = pCurrentCursor;
    std::unique_ptr<SvxSearchItem> xSearchItem;

    // only create progress bar for ShellCursor
    bool bIsUnoCursor = dynamic_cast<SwUnoCursor*>(pCurrentCursor) !=  nullptr;
    std::unique_ptr<PercentHdl> pPHdl;
    sal_uInt16 nCursorCnt = 0;
    if( FindRanges::InSel & eFndRngs )
    {
        while( pCurrentCursor != ( pTmpCursor = pTmpCursor->GetNext() ))
            ++nCursorCnt;
        if( nCursorCnt && !bIsUnoCursor )
            pPHdl.reset(new PercentHdl( 0, nCursorCnt, rDoc.GetDocShell() ));
    }
    else
        pSaveCursor = pSaveCursor->GetPrev();

    bool bEnd = false;
    do {
        aRegion.SetMark();
        // independent from search direction: SPoint is always bigger than mark
        // if the search area is valid
        SwPosition *pSttPos = aRegion.GetMark(),
                        *pEndPos = aRegion.GetPoint();
        *pSttPos = *pTmpCursor->Start();
        *pEndPos = *pTmpCursor->End();
        if( bSrchBkwrd )
            aRegion.Exchange();

        if( !nCursorCnt && !pPHdl && !bIsUnoCursor )
            pPHdl.reset(new PercentHdl( aRegion ));

        // as long as found and not at same position
        while(  *pSttPos <= *pEndPos )
        {
            nFndRet = rParas.DoFind(*pCurrentCursor, fnMove, aRegion, bInReadOnly, xSearchItem);
            if( 0 == nFndRet ||
                ( pFndRing &&
                  *pFndRing->GetPoint() == *pCurrentCursor->GetPoint() &&
                  *pFndRing->GetMark() == *pCurrentCursor->GetMark() ))
                break;
            if( !( FIND_NO_RING & nFndRet ))
            {
                // #i24084# - create ring similar to the one in CreateCursor
                SwCursor* pNew = pCurrentCursor->Create( pFndRing );
                if( !pFndRing )
                    pFndRing = pNew;

                pNew->SetMark();
                *pNew->GetMark() = *pCurrentCursor->GetMark();
            }

            ++nFound;

            if( !( eFndRngs & FindRanges::InSelAll) )
            {
                bEnd = true;
                break;
            }

            if ((coSrchRplcThreshold == nFound)
                && rDoc.GetIDocumentUndoRedo().DoesUndo()
                && rParas.IsReplaceMode())
            {
                short nRet = pCurrentCursor->MaxReplaceArived();
                if( RET_YES == nRet )
                {
                    rDoc.GetIDocumentUndoRedo().DelAllUndoObj();
                    rDoc.GetIDocumentUndoRedo().DoUndo(false);
                }
                else
                {
                    bEnd = true;
                    if(RET_CANCEL == nRet)
                    {
                        bCancel = true;
                    }
                    break;
                }
            }

            if( bSrchBkwrd )
                // move pEndPos in front of the found area
                *pEndPos = *pCurrentCursor->Start();
            else
                // move pSttPos behind the found area
                *pSttPos = *pCurrentCursor->End();

            if( *pSttPos == *pEndPos )
                // in area but at the end => done
                break;

            if( !nCursorCnt && pPHdl )
            {
                pPHdl->NextPos( *aRegion.GetMark() );
            }
        }

        if( bEnd || !( eFndRngs & ( FindRanges::InSelAll | FindRanges::InSel )) )
            break;

        pTmpCursor = pTmpCursor->GetNext();
        if( nCursorCnt && pPHdl )
        {
            pPHdl->NextPos( ++pPHdl->nActPos );
        }

    } while( pTmpCursor != pSaveCursor && pTmpCursor->GetNext() != pTmpCursor);

    if( nFound && !pFndRing ) // if no ring should be created
        pFndRing = pCurrentCursor->Create();

    rDoc.GetIDocumentUndoRedo().DoUndo(bDoesUndo);
    return nFound;
}

static bool lcl_MakeSelFwrd( const SwNode& rSttNd, const SwNode& rEndNd,
                        SwPaM& rPam, bool bFirst )
{
    if( rSttNd.GetIndex() + 1 == rEndNd.GetIndex() )
        return false;

    rPam.DeleteMark();
    SwContentNode* pCNd;
    if( !bFirst )
    {
        rPam.GetPoint()->Assign(rSttNd);
        pCNd = SwNodes::GoNext(rPam.GetPoint());
        if( !pCNd )
            return false;
        rPam.GetPoint()->AssignStartIndex(*pCNd);
    }
    else if( rSttNd.GetIndex() > rPam.GetPoint()->GetNodeIndex() ||
             rPam.GetPoint()->GetNodeIndex() >= rEndNd.GetIndex() )
        // not in this section
        return false;

    rPam.SetMark();
    rPam.GetPoint()->Assign(rEndNd);
    pCNd = SwNodes::GoPrevious(rPam.GetPoint(), true);
    if( !pCNd )
        return false;
    rPam.GetPoint()->AssignEndIndex(*pCNd);

    return *rPam.GetMark() < *rPam.GetPoint();
}

static bool lcl_MakeSelBkwrd( const SwNode& rSttNd, const SwNode& rEndNd,
                        SwPaM& rPam, bool bFirst )
{
    if( rEndNd.GetIndex() + 1 == rSttNd.GetIndex() )
        return false;

    rPam.DeleteMark();
    SwContentNode* pCNd;
    if( !bFirst )
    {
        rPam.GetPoint()->Assign(rSttNd);
        pCNd = SwNodes::GoPrevious(rPam.GetPoint(), true);
        if( !pCNd )
            return false;
        rPam.GetPoint()->AssignEndIndex(*pCNd);
    }
    else if( rEndNd.GetIndex() > rPam.GetPoint()->GetNodeIndex() ||
             rPam.GetPoint()->GetNodeIndex() >= rSttNd.GetIndex() )
        return false;       // not in this section

    rPam.SetMark();
    rPam.GetPoint()->Assign(rEndNd);
    pCNd = SwNodes::GoNext(rPam.GetPoint());
    if( !pCNd )
        return false;
    rPam.GetPoint()->SetContent(0);

    return *rPam.GetPoint() < *rPam.GetMark();
}

// this method "searches" for all use cases because in SwFindParas is always the
// correct parameters and respective search method
sal_Int32 SwCursor::FindAll( SwFindParas& rParas,
                            SwDocPositions nStart, SwDocPositions nEnd,
                            FindRanges eFndRngs, bool& bCancel )
{
    bCancel = false;
    SwCursorSaveState aSaveState( *this );

    // create region without adding it to the ring
    SwPaM aRegion( *GetPoint() );
    SwMoveFnCollection const & fnMove = MakeFindRange( nStart, nEnd, &aRegion );

    sal_Int32 nFound = 0;
    const bool bMvBkwrd = &fnMove == &fnMoveBackward;
    bool bInReadOnly = IsReadOnlyAvailable();
    std::unique_ptr<SvxSearchItem> xSearchItem;

    SwCursor* pFndRing = nullptr;
    SwNodes& rNds = GetDoc().GetNodes();

    // search in sections?
    if( FindRanges::InSel & eFndRngs )
    {
        // if string was not found in region then get all sections (cursors
        // stays unchanged)
        nFound = lcl_FindSelection( rParas, this, fnMove,
                                    pFndRing, aRegion, eFndRngs,
                                    bInReadOnly, bCancel );
        if( 0 == nFound )
            return nFound;

        // found string at least once; it's all in new Cursor ring thus delete old one
        while( GetNext() != this )
            delete GetNext();

        *GetPoint() = *pFndRing->GetPoint();
        SetMark();
        *GetMark() = *pFndRing->GetMark();
        pFndRing->GetRingContainer().merge( GetRingContainer() );
        delete pFndRing;
    }
    else if( FindRanges::InOther & eFndRngs )
    {
        // put cursor as copy of current into ring
        // chaining points always to first created, so forward
        SwCursor* pSav = Create( this ); // save the current cursor

        // if already outside of body text search from this position or start at
        // 1. base section
        if( bMvBkwrd
            ? lcl_MakeSelBkwrd( rNds.GetEndOfExtras(),
                    *rNds.GetEndOfPostIts().StartOfSectionNode(),
                     *this, rNds.GetEndOfExtras().GetIndex() >=
                    GetPoint()->GetNodeIndex() )
            : lcl_MakeSelFwrd( *rNds.GetEndOfPostIts().StartOfSectionNode(),
                    rNds.GetEndOfExtras(), *this,
                    rNds.GetEndOfExtras().GetIndex() >=
                    GetPoint()->GetNodeIndex() ))
        {
            nFound = lcl_FindSelection( rParas, this, fnMove, pFndRing,
                                        aRegion, eFndRngs, bInReadOnly, bCancel );
        }

        if( !nFound )
        {
            // put back the old one
            *GetPoint() = *pSav->GetPoint();
            if( pSav->HasMark() )
            {
                SetMark();
                *GetMark() = *pSav->GetMark();
            }
            else
                DeleteMark();
            return 0;
        }

        if( !( FindRanges::InSelAll & eFndRngs ))
        {
            // there should only be a single one, thus add it
            // independent from search direction: SPoint is always bigger than
            // mark if the search area is valid
            *GetPoint() = *pFndRing->GetPoint();
            SetMark();
            *GetMark() = *pFndRing->GetMark();
        }
        else
        {
            // found string at least once; it's all in new Cursor ring thus delete old one
            while( GetNext() != this )
                delete GetNext();

            *GetPoint() = *pFndRing->GetPoint();
            SetMark();
            *GetMark() = *pFndRing->GetMark();
            pFndRing->GetRingContainer().merge( GetRingContainer() );
        }
        delete pFndRing;
    }
    else if( FindRanges::InSelAll & eFndRngs )
    {
        SwCursor* pSav = Create( this );    // save the current cursor

        const SwNode* pSttNd = ( FindRanges::InBodyOnly & eFndRngs )
                            ? rNds.GetEndOfContent().StartOfSectionNode()
                            : rNds.GetEndOfPostIts().StartOfSectionNode();

        if( bMvBkwrd
            ? lcl_MakeSelBkwrd( rNds.GetEndOfContent(), *pSttNd, *this, false )
            : lcl_MakeSelFwrd( *pSttNd, rNds.GetEndOfContent(), *this, false ))
        {
            nFound = lcl_FindSelection( rParas, this, fnMove, pFndRing,
                                        aRegion, eFndRngs, bInReadOnly, bCancel );
        }

        if( !nFound )
        {
            // put back the old one
            *GetPoint() = *pSav->GetPoint();
            if( pSav->HasMark() )
            {
                SetMark();
                *GetMark() = *pSav->GetMark();
            }
            else
                DeleteMark();
            return 0;
        }
        while( GetNext() != this )
            delete GetNext();

        *GetPoint() = *pFndRing->GetPoint();
        SetMark();
        *GetMark() = *pFndRing->GetMark();
        pFndRing->GetRingContainer().merge( GetRingContainer() );
        delete pFndRing;
    }
    else
    {
        // if a GetMark is set then keep the GetMark of the found object
        // This allows spanning an area with this search.
        SwPosition aMarkPos( *GetMark() );
        const bool bMarkPos = HasMark() && (eFndRngs == FindRanges::InBody);

        nFound = rParas.DoFind(*this, fnMove, aRegion, bInReadOnly, xSearchItem) ? 1 : 0;
        if (0 != nFound && bMarkPos)
            *GetMark() = std::move(aMarkPos);
    }

    if( nFound && SwCursor::IsSelOvr( SwCursorSelOverFlags::Toggle ) )
        nFound = 0;
    return nFound;
}

void SwCursor::FillFindPos( SwDocPositions ePos, SwPosition& rPos ) const
{
    bool bIsStart = true;
    SwContentNode* pCNd = nullptr;
    SwNodes& rNds = GetDoc().GetNodes();

    switch( ePos )
    {
    case SwDocPositions::Start:
        rPos.Assign(*rNds.GetEndOfContent().StartOfSectionNode());
        pCNd = SwNodes::GoNext(&rPos);
        break;
    case SwDocPositions::End:
        rPos.Assign(rNds.GetEndOfContent());
        pCNd = SwNodes::GoPrevious( &rPos );
        bIsStart = false;
        break;
    case SwDocPositions::OtherStart:
        rPos.Assign( *rNds[ SwNodeOffset(0) ] );
        pCNd = SwNodes::GoNext(&rPos);
        break;
    case SwDocPositions::OtherEnd:
        rPos.Assign( *rNds.GetEndOfContent().StartOfSectionNode() );
        pCNd = SwNodes::GoPrevious( &rPos );
        bIsStart = false;
        break;
    default:
        rPos = *GetPoint();
    }

    if( pCNd && !bIsStart )
    {
        rPos.AssignEndIndex( *pCNd );
    }
}

short SwCursor::MaxReplaceArived()
{
    return RET_YES;
}

namespace {

struct HideWrapper
{
    // either the frame's text or the node's text (possibly pre-filtered)
    OUString const* m_pText;
    // this is actually a TextFrameIndex but all of the i18n code uses sal_Int32
    sal_Int32 m_nPtIndex;
    // if mapping is needed, use this frame
    SwTextFrame * m_pFrame;
    // input in the constructor, output (via mapping) in the destructor
    SwTextNode *& m_rpTextNode;
    sal_Int32 & m_rPtPos;

    HideWrapper(SwRootFrame const*const pLayout,
            SwTextNode *& rpTextNode, sal_Int32 & rPtPos,
            OUString const*const pFilteredNodeText = nullptr)
        : m_pText(pFilteredNodeText)
        , m_pFrame(nullptr)
        , m_rpTextNode(rpTextNode)
        , m_rPtPos(rPtPos)
    {
        if (pLayout && pLayout->HasMergedParas())
        {
            m_pFrame = static_cast<SwTextFrame*>(rpTextNode->getLayoutFrame(pLayout));
            m_pText = &m_pFrame->GetText();
            m_nPtIndex = sal_Int32(m_pFrame->MapModelToView(rpTextNode, rPtPos));
        }
        else
        {
            if (!m_pText)
            {
                m_pText = &rpTextNode->GetText();
            }
            m_nPtIndex = rPtPos;
        }
    }
    ~HideWrapper()
    {
        AssignBack(m_rpTextNode, m_rPtPos);
    }
    void AssignBack(SwTextNode *& rpTextNode, sal_Int32 & rPtPos)
    {
        if (0 <= m_nPtIndex && m_pFrame)
        {
            std::pair<SwTextNode*, sal_Int32> const pos(
                    m_pFrame->MapViewToModel(TextFrameIndex(m_nPtIndex)));
            rpTextNode = pos.first;
            rPtPos = pos.second;
        }
        else
        {
            rPtPos = m_nPtIndex;
        }
    }
};

} // namespace

bool SwCursor::SelectWord( SwViewShell const * pViewShell, const Point* pPt )
{
    return SelectWordWT( pViewShell, WordType::ANYWORD_IGNOREWHITESPACES, pPt );
}

bool SwCursor::IsStartWordWT(sal_Int16 nWordType, SwRootFrame const*const pLayout) const
{
    bool bRet = false;
    SwTextNode* pTextNd = GetPointNode().GetTextNode();
    if (pTextNd)
    {
        sal_Int32 nPtPos = GetPoint()->GetContentIndex();

        HideWrapper w(pLayout, pTextNd, nPtPos);

        bRet = g_pBreakIt->GetBreakIter()->isBeginWord(
                            *w.m_pText, w.m_nPtIndex,
                            g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos )),
                            nWordType );
    }
    return bRet;
}

bool SwCursor::IsEndWordWT(sal_Int16 nWordType, SwRootFrame const*const pLayout) const
{
    bool bRet = false;
    SwTextNode* pTextNd = GetPointNode().GetTextNode();
    if (pTextNd)
    {
        sal_Int32 nPtPos = GetPoint()->GetContentIndex();

        HideWrapper w(pLayout, pTextNd, nPtPos);

        bRet = g_pBreakIt->GetBreakIter()->isEndWord(
                            *w.m_pText, w.m_nPtIndex,
                            g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos ) ),
                            nWordType );

    }
    return bRet;
}

bool SwCursor::IsInWordWT(sal_Int16 nWordType, SwRootFrame const*const pLayout) const
{
    bool bRet = false;
    SwTextNode* pTextNd = GetPointNode().GetTextNode();
    if (pTextNd)
    {
        sal_Int32 nPtPos = GetPoint()->GetContentIndex();

        {
            HideWrapper w(pLayout, pTextNd, nPtPos);

            Boundary aBoundary = g_pBreakIt->GetBreakIter()->getWordBoundary(
                                *w.m_pText, w.m_nPtIndex,
                                g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos ) ),
                                nWordType,
                                true );

            bRet = aBoundary.startPos != aBoundary.endPos &&
                    aBoundary.startPos <= w.m_nPtIndex &&
                        w.m_nPtIndex <= aBoundary.endPos;
            w.m_nPtIndex = aBoundary.startPos; // hack: convert startPos back...
        }
        if(bRet)
        {
            const CharClass& rCC = GetAppCharClass();
            bRet = rCC.isLetterNumeric(pTextNd->GetText(), nPtPos);
        }
    }
    return bRet;
}

bool SwCursor::IsStartEndSentence(bool bEnd, SwRootFrame const*const pLayout) const
{
    bool bRet = bEnd ?
                    GetPointContentNode() && GetPoint()->GetContentIndex() == GetPointContentNode()->Len() :
                    GetPoint()->GetContentIndex() == 0;

    if ((pLayout != nullptr && pLayout->HasMergedParas()) || !bRet)
    {
        SwCursor aCursor(*GetPoint(), nullptr);
        SwPosition aOrigPos = *aCursor.GetPoint();
        aCursor.GoSentence(bEnd ? SwCursor::END_SENT : SwCursor::START_SENT, pLayout);
        bRet = aOrigPos == *aCursor.GetPoint();
    }
    return bRet;
}

bool SwCursor::GoStartWordWT(sal_Int16 nWordType, SwRootFrame const*const pLayout)
{
    bool bRet = false;
    SwTextNode* pTextNd = GetPointNode().GetTextNode();
    if (pTextNd)
    {
        SwCursorSaveState aSave( *this );
        sal_Int32 nPtPos = GetPoint()->GetContentIndex();

        {
            HideWrapper w(pLayout, pTextNd, nPtPos);

            w.m_nPtIndex = g_pBreakIt->GetBreakIter()->getWordBoundary(
                            *w.m_pText, w.m_nPtIndex,
                            g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos ) ),
                            nWordType,
                            false ).startPos;
        }

        if (nPtPos < pTextNd->GetText().getLength() && nPtPos >= 0)
        {
            GetPoint()->Assign(*pTextNd, nPtPos);
            if( !IsSelOvr() )
                bRet = true;
        }
    }
    return bRet;
}

bool SwCursor::GoEndWordWT(sal_Int16 nWordType, SwRootFrame const*const pLayout)
{
    bool bRet = false;
    SwTextNode* pTextNd = GetPointNode().GetTextNode();
    if (pTextNd)
    {
        SwCursorSaveState aSave( *this );
        sal_Int32 nPtPos = GetPoint()->GetContentIndex();

        {
            HideWrapper w(pLayout, pTextNd, nPtPos);

            w.m_nPtIndex = g_pBreakIt->GetBreakIter()->getWordBoundary(
                            *w.m_pText, w.m_nPtIndex,
                            g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos ) ),
                            nWordType,
                            true ).endPos;
        }

        if (nPtPos <= pTextNd->GetText().getLength() && nPtPos >= 0 &&
            GetPoint()->GetContentIndex() != nPtPos )
        {
            GetPoint()->Assign(*pTextNd, nPtPos);
            if( !IsSelOvr() )
                bRet = true;
        }
    }
    return bRet;
}

bool SwCursor::GoNextWordWT(sal_Int16 nWordType, SwRootFrame const*const pLayout)
{
    bool bRet = false;
    SwTextNode* pTextNd = GetPointNode().GetTextNode();
    if (pTextNd)
    {
        SwCursorSaveState aSave( *this );
        sal_Int32 nPtPos = GetPoint()->GetContentIndex();

        {
            HideWrapper w(pLayout, pTextNd, nPtPos);

            w.m_nPtIndex = g_pBreakIt->GetBreakIter()->nextWord(
                        *w.m_pText, w.m_nPtIndex,
                        g_pBreakIt->GetLocale( pTextNd->GetLang(nPtPos, 1) ),
                        nWordType ).startPos;
        }

        if (nPtPos <= pTextNd->GetText().getLength() && nPtPos >= 0)
        {
            GetPoint()->Assign(*pTextNd, nPtPos);
            if( !IsSelOvr() )
                bRet = true;
        }
    }
    return bRet;
}

bool SwCursor::GoPrevWordWT(sal_Int16 nWordType, SwRootFrame const*const pLayout)
{
    bool bRet = false;
    SwTextNode* pTextNd = GetPointNode().GetTextNode();
    if (pTextNd)
    {
        SwCursorSaveState aSave( *this );
        sal_Int32 nPtPos = GetPoint()->GetContentIndex();

        {
            HideWrapper w(pLayout, pTextNd, nPtPos);

            const sal_Int32 nPtStart = w.m_nPtIndex;
            if (w.m_nPtIndex)
            {
                --w.m_nPtIndex;
                w.AssignBack(pTextNd, nPtPos);
            }

            w.m_nPtIndex = g_pBreakIt->GetBreakIter()->previousWord(
                        *w.m_pText, nPtStart,
                        g_pBreakIt->GetLocale( pTextNd->GetLang(nPtPos, 1) ),
                                nWordType ).startPos;
        }

        if (nPtPos < pTextNd->GetText().getLength() && nPtPos >= 0)
        {
            GetPoint()->Assign(*pTextNd, nPtPos);
            if( !IsSelOvr() )
                bRet = true;
        }
    }
    return bRet;
}

bool SwCursor::SelectWordWT( SwViewShell const * pViewShell, sal_Int16 nWordType, const Point* pPt )
{
    SwCursorSaveState aSave( *this );

    bool bRet = false;
    DeleteMark();
    const SwRootFrame* pLayout = pViewShell->GetLayout();
    if( pPt && nullptr != pLayout )
    {
        // set the cursor to the layout position
        Point aPt( *pPt );
        pLayout->GetModelPositionForViewPoint( GetPoint(), aPt );
    }

    SwTextNode* pTextNd = GetPointNode().GetTextNode();
    if (pTextNd)
    {
        // Should we select the whole fieldmark?
        const IDocumentMarkAccess* pMarksAccess = GetDoc().getIDocumentMarkAccess( );
        sw::mark::IFieldmark const*const pMark(pMarksAccess->getInnerFieldmarkFor(*GetPoint()));
        if (pMark && (IDocumentMarkAccess::GetType(*pMark) == IDocumentMarkAccess::MarkType::TEXT_FIELDMARK
                      || IDocumentMarkAccess::GetType(*pMark) == IDocumentMarkAccess::MarkType::DATE_FIELDMARK))
        {
            *GetPoint() = sw::mark::FindFieldSep(*pMark);
            GetPoint()->AdjustContent(+1); // Don't select the separator

            const SwPosition& rEnd = pMark->GetMarkEnd();

            assert(pMark->GetMarkEnd() != *GetPoint());
            SetMark();
            *GetMark() = rEnd;
            GetMark()->AdjustContent(-1); // Don't select the end delimiter

            bRet = true;
        }
        else
        {
            bool bForward = true;
            sal_Int32 nPtPos = GetPoint()->GetContentIndex();

            HideWrapper w(pViewShell->GetLayout(), pTextNd, nPtPos);

            Boundary aBndry( g_pBreakIt->GetBreakIter()->getWordBoundary(
                                *w.m_pText, w.m_nPtIndex,
                                g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos ) ),
                                nWordType,
                                bForward ));

            if (comphelper::LibreOfficeKit::isActive() && aBndry.startPos == aBndry.endPos && w.m_nPtIndex > 0)
            {
                // nPtPos is the end of the paragraph, select the last word then.
                --w.m_nPtIndex;
                w.AssignBack(pTextNd, nPtPos);

                aBndry = g_pBreakIt->GetBreakIter()->getWordBoundary(
                                    *w.m_pText, w.m_nPtIndex,
                                    g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos ) ),
                                    nWordType,
                                    bForward );

            }

            SwTextNode * pStartNode(pTextNd);
            sal_Int32 nStartIndex;
            w.m_nPtIndex = aBndry.startPos;
            w.AssignBack(pStartNode, nStartIndex);

            SwTextNode * pEndNode(pTextNd);
            sal_Int32 nEndIndex;
            w.m_nPtIndex = aBndry.endPos;
            w.AssignBack(pEndNode, nEndIndex);

            if( aBndry.startPos != aBndry.endPos )
            {
                GetPoint()->Assign(*pEndNode, nEndIndex);
                if( !IsSelOvr() )
                {
                    SetMark();
                    GetMark()->Assign(*pStartNode, nStartIndex);
                    if (sw::mark::IMark* pAnnotationMark = pMarksAccess->getAnnotationMarkFor(*GetPoint()))
                    {
                        // An annotation mark covers the selected word. Check
                        // if it covers only the word: in that case we select
                        // the comment anchor as well.
                        bool bStartMatch = GetMark()->GetNode() == pAnnotationMark->GetMarkStart().GetNode() &&
                            GetMark()->GetContentIndex() == pAnnotationMark->GetMarkStart().GetContentIndex();
                        bool bEndMatch = GetPoint()->GetNode() == pAnnotationMark->GetMarkEnd().GetNode() &&
                            GetPoint()->GetContentIndex() + 1 == pAnnotationMark->GetMarkEnd().GetContentIndex();
                        if (bStartMatch && bEndMatch)
                            GetPoint()->AdjustContent(+1);
                    }
                    if( !IsSelOvr() )
                        bRet = true;
                }
            }
        }
    }

    if( !bRet )
    {
        DeleteMark();
        RestoreSavePos();
    }
    return bRet;
}

static OUString lcl_MaskDeletedRedlines( const SwTextNode* pTextNd )
{
    OUString aRes;
    if (pTextNd)
    {
        //mask deleted redlines
        OUString sNodeText(pTextNd->GetText());
        const SwDoc& rDoc = pTextNd->GetDoc();
        const bool bShowChg = IDocumentRedlineAccess::IsShowChanges( rDoc.getIDocumentRedlineAccess().GetRedlineFlags() );
        if ( bShowChg )
        {
            SwRedlineTable::size_type nAct = rDoc.getIDocumentRedlineAccess().GetRedlinePos( *pTextNd, RedlineType::Any );
            for ( ; nAct < rDoc.getIDocumentRedlineAccess().GetRedlineTable().size(); nAct++ )
            {
                const SwRangeRedline* pRed = rDoc.getIDocumentRedlineAccess().GetRedlineTable()[ nAct ];
                if ( pRed->Start()->GetNode() > *pTextNd )
                    break;

                if( RedlineType::Delete == pRed->GetType() )
                {
                    sal_Int32 nStart, nEnd;
                    pRed->CalcStartEnd( pTextNd->GetIndex(), nStart, nEnd );

                    while ( nStart < nEnd && nStart < sNodeText.getLength() )
                        sNodeText = sNodeText.replaceAt( nStart++, 1, rtl::OUStringChar(CH_TXTATR_INWORD) );
                }
            }
        }
        aRes = sNodeText;
    }
    return aRes;
}

bool SwCursor::GoSentence(SentenceMoveType eMoveType, SwRootFrame const*const pLayout)
{
    bool bRet = false;
    SwTextNode* pTextNd = GetPointNode().GetTextNode();
    if (pTextNd)
    {
        OUString const sNodeText(lcl_MaskDeletedRedlines(pTextNd));

        SwCursorSaveState aSave( *this );
        sal_Int32 nPtPos = GetPoint()->GetContentIndex();

        {
            HideWrapper w(pLayout, pTextNd, nPtPos, &sNodeText);

            switch ( eMoveType )
            {
            case START_SENT: /* when modifying: see also ExpandToSentenceBorders below! */
                w.m_nPtIndex = g_pBreakIt->GetBreakIter()->beginOfSentence(
                            *w.m_pText, w.m_nPtIndex,
                            g_pBreakIt->GetLocale(pTextNd->GetLang(nPtPos)));
                break;
            case END_SENT: /* when modifying: see also ExpandToSentenceBorders below! */
                w.m_nPtIndex = g_pBreakIt->GetBreakIter()->endOfSentence(
                            *w.m_pText, w.m_nPtIndex,
                            g_pBreakIt->GetLocale(pTextNd->GetLang(nPtPos)));
                break;
            case NEXT_SENT:
                {
                    w.m_nPtIndex = g_pBreakIt->GetBreakIter()->endOfSentence(
                            *w.m_pText, w.m_nPtIndex,
                            g_pBreakIt->GetLocale(pTextNd->GetLang(nPtPos)));
                    if (w.m_nPtIndex >= 0 && w.m_nPtIndex < w.m_pText->getLength())
                    {
                        do
                        {
                            ++w.m_nPtIndex;
                        }
                        while (w.m_nPtIndex < w.m_pText->getLength()
                               && (*w.m_pText)[w.m_nPtIndex] == ' ');
                    }
                    break;
                }
            case PREV_SENT:
                w.m_nPtIndex = g_pBreakIt->GetBreakIter()->beginOfSentence(
                            *w.m_pText, w.m_nPtIndex,
                            g_pBreakIt->GetLocale(pTextNd->GetLang(nPtPos)));

                if (w.m_nPtIndex == 0)
                    return false;   // the previous sentence is not in this paragraph
                if (w.m_nPtIndex > 0)
                {
                    w.m_nPtIndex = g_pBreakIt->GetBreakIter()->beginOfSentence(
                            *w.m_pText, w.m_nPtIndex - 1,
                            g_pBreakIt->GetLocale(pTextNd->GetLang(nPtPos)));
                }
                break;
            }
        }

        // it is allowed to place the PaM just behind the last
        // character in the text thus <= ...Len
        if (nPtPos <= pTextNd->GetText().getLength() && nPtPos >= 0)
        {
            GetPoint()->Assign(*pTextNd, nPtPos);
            if( !IsSelOvr() )
                bRet = true;
        }
    }
    return bRet;
}

void SwCursor::ExpandToSentenceBorders(SwRootFrame const*const pLayout)
{
    SwTextNode* pStartNd = Start()->GetNode().GetTextNode();
    SwTextNode* pEndNd   = End()->GetNode().GetTextNode();
    if (!pStartNd || !pEndNd)
        return;

    if (!HasMark())
        SetMark();

    OUString sStartText( lcl_MaskDeletedRedlines( pStartNd ) );
    OUString sEndText( pStartNd == pEndNd? sStartText : lcl_MaskDeletedRedlines( pEndNd ) );

    SwCursorSaveState aSave( *this );
    sal_Int32 nStartPos = Start()->GetContentIndex();
    sal_Int32 nEndPos   = End()->GetContentIndex();

    {
        HideWrapper w(pLayout, pStartNd, nStartPos, &sStartText);

        w.m_nPtIndex = g_pBreakIt->GetBreakIter()->beginOfSentence(
                            *w.m_pText, w.m_nPtIndex,
                            g_pBreakIt->GetLocale( pStartNd->GetLang( nStartPos ) ) );
    }
    {
        HideWrapper w(pLayout, pEndNd, nEndPos, &sEndText);

        w.m_nPtIndex = g_pBreakIt->GetBreakIter()->endOfSentence(
                            *w.m_pText, w.m_nPtIndex,
                            g_pBreakIt->GetLocale( pEndNd->GetLang( nEndPos ) ) );
    }

    // it is allowed to place the PaM just behind the last
    // character in the text thus <= ...Len
    if (nStartPos <= pStartNd->GetText().getLength() && nStartPos >= 0)
    {
        GetMark()->Assign(*pStartNd, nStartPos);
    }
    if (nEndPos <= pEndNd->GetText().getLength() && nEndPos >= 0)
    {
        GetPoint()->Assign(*pEndNd, nEndPos);
    }
}

bool SwTableCursor::LeftRight( bool bLeft, sal_uInt16 nCnt, SwCursorSkipMode /*nMode*/,
    bool /*bVisualAllowed*/, bool /*bSkipHidden*/, bool /*bInsertCursor*/,
    SwRootFrame const*, bool /*isFieldNames*/)
{
    return bLeft ? GoPrevCell( nCnt )
                 : GoNextCell( nCnt );
}

// calculate cursor bidi level: extracted from LeftRight()
const SwContentFrame*
SwCursor::DoSetBidiLevelLeftRight(
    bool & io_rbLeft, bool bVisualAllowed, bool bInsertCursor)
{
    // calculate cursor bidi level
    const SwContentFrame* pSttFrame = nullptr;
    SwNode& rNode = GetPoint()->GetNode();

    if( rNode.IsTextNode() )
    {
        const SwTextNode& rTNd = *rNode.GetTextNode();
        sal_Int32 nPos = GetPoint()->GetContentIndex();

        if ( bVisualAllowed && SvtCTLOptions::IsCTLFontEnabled() &&
             SvtCTLOptions::MOVEMENT_VISUAL == SvtCTLOptions::GetCTLCursorMovement() )
        {
            // for visual cursor travelling (used in bidi layout)
            // we first have to convert the logic to a visual position
            Point aPt;
            std::pair<Point, bool> const tmp(aPt, true);
            pSttFrame = rTNd.getLayoutFrame(
                    GetDoc().getIDocumentLayoutAccess().GetCurrentLayout(),
                    GetPoint(), &tmp);
            if( pSttFrame )
            {
                sal_uInt8 nCursorLevel = GetCursorBidiLevel();
                bool bForward = ! io_rbLeft;
                SwTextFrame *const pTF(const_cast<SwTextFrame*>(
                            static_cast<const SwTextFrame*>(pSttFrame)));
                TextFrameIndex nTFIndex(pTF->MapModelToViewPos(*GetPoint()));
                pTF->PrepareVisualMove( nTFIndex, nCursorLevel,
                                                         bForward, bInsertCursor );
                *GetPoint() = pTF->MapViewToModelPos(nTFIndex);
                SetCursorBidiLevel( nCursorLevel );
                io_rbLeft = ! bForward;
            }
        }
        else
        {
            SwTextFrame const* pFrame;
            const SwScriptInfo* pSI = SwScriptInfo::GetScriptInfo(rTNd, &pFrame);
            if ( pSI )
            {
                const sal_Int32 nMoveOverPos = io_rbLeft ?
                                               ( nPos ? nPos - 1 : 0 ) :
                                                nPos;
                TextFrameIndex nIndex(pFrame->MapModelToView(&rTNd, nMoveOverPos));
                SetCursorBidiLevel( pSI->DirType(nIndex) );
            }
        }
    }
    return pSttFrame;
}

bool SwCursor::LeftRight( bool bLeft, sal_uInt16 nCnt, SwCursorSkipMode nMode,
                          bool bVisualAllowed,bool bSkipHidden, bool bInsertCursor,
                          SwRootFrame const*const pLayout, bool isFieldNames)
{
    // calculate cursor bidi level
    SwNode& rNode = GetPoint()->GetNode();
    const SwContentFrame* pSttFrame = // may side-effect bLeft!
        DoSetBidiLevelLeftRight(bLeft, bVisualAllowed, bInsertCursor);

    // can the cursor be moved n times?
    SwCursorSaveState aSave( *this );
    SwMoveFnCollection const & fnMove = bLeft ? fnMoveBackward : fnMoveForward;

    SwGoInDoc fnGo;
    if ( bSkipHidden )
        fnGo = SwCursorSkipMode::Cells == nMode ? GoInContentCellsSkipHidden : GoInContentSkipHidden;
    else
        fnGo = SwCursorSkipMode::Cells == nMode ? GoInContentCells : GoInContent;

    SwTextFrame const* pFrame(nullptr);
    if (pLayout)
    {
        pFrame = static_cast<SwTextFrame*>(rNode.GetContentNode()->getLayoutFrame(pLayout));
        if (pFrame)
        {
            while (pFrame->GetPrecede())
            {
                pFrame = static_cast<SwTextFrame const*>(pFrame->GetPrecede());
            }
        }
    }

    while( nCnt )
    {
        SwNodeIndex aOldNodeIdx( GetPoint()->GetNode() );

        TextFrameIndex beforeIndex(-1);
        if (pFrame)
        {
            beforeIndex = pFrame->MapModelToViewPos(*GetPoint());
        }

        if (!bLeft && pLayout && pLayout->GetFieldmarkMode() == sw::FieldmarkMode::ShowResult)
        {
            SwTextNode const*const pNode(GetPoint()->GetNode().GetTextNode());
            assert(pNode);
            if (pNode->Len() != GetPoint()->GetContentIndex()
                && pNode->GetText()[GetPoint()->GetContentIndex()] == CH_TXT_ATR_FIELDSTART)
            {
                IDocumentMarkAccess const& rIDMA(*GetDoc().getIDocumentMarkAccess());
                sw::mark::IFieldmark const*const pMark(rIDMA.getFieldmarkAt(*GetPoint()));
                assert(pMark);
                *GetPoint() = sw::mark::FindFieldSep(*pMark);
            }
        }

        if ( !Move( fnMove, fnGo ) )
        {
            const SwEditShell* pSh = GetDoc().GetEditShell();
            const SwViewOption* pViewOptions = pSh ? pSh->GetViewOptions() : nullptr;
            if (pViewOptions && pViewOptions->IsShowOutlineContentVisibilityButton())
            {
                // Fixes crash that occurs in documents with outline content folded at the end of
                // the document. When the cursor is at the end of the visible document and
                // right arrow key is pressed Move fails after moving the cursor to the
                // end of the document model, which doesn't have a node frame and causes
                // weird numbers to be displayed in the statusbar page number count. Left
                // arrow, when in this state, causes a crash without RestoredSavePos() added here.
                RestoreSavePos();
            }
            break;
        }

        if (pFrame)
        {
            SwTextFrame const* pNewFrame(static_cast<SwTextFrame const*>(
                GetPoint()->GetNode().GetContentNode()->getLayoutFrame(pLayout)));
            if (pNewFrame)
            {
                while (pNewFrame->GetPrecede())
                {
                    pNewFrame = static_cast<SwTextFrame const*>(pNewFrame->GetPrecede());
                }
            }
            // sw_redlinehide: fully redline-deleted nodes don't have frames...
            if (pFrame == pNewFrame || !pNewFrame)
            {
                if (!pNewFrame || beforeIndex == pFrame->MapModelToViewPos(*GetPoint()))
                {
                    continue; // moving inside delete redline, doesn't count...
                }
            }
            else
            {
                // assume iteration is stable & returns the same frame
                assert(!pFrame->IsAnFollow(pNewFrame) && !pNewFrame->IsAnFollow(pFrame));
                pFrame = pNewFrame;
            }
        }

        if (bLeft && pLayout && pLayout->GetFieldmarkMode() == sw::FieldmarkMode::ShowCommand)
        {
            SwTextNode const*const pNode(GetPoint()->GetNode().GetTextNode());
            assert(pNode);
            if (pNode->Len() != GetPoint()->GetContentIndex()
                && pNode->GetText()[GetPoint()->GetContentIndex()] == CH_TXT_ATR_FIELDEND)
            {
                IDocumentMarkAccess const& rIDMA(*GetDoc().getIDocumentMarkAccess());
                sw::mark::IFieldmark const*const pMark(rIDMA.getFieldmarkAt(*GetPoint()));
                assert(pMark);
                *GetPoint() = sw::mark::FindFieldSep(*pMark);
            }
        }

        if (isFieldNames)
        {
            SwTextNode const*const pNode(GetPoint()->GetNode().GetTextNode());
            assert(pNode);
            SwTextAttr const*const pInputField(pNode->GetTextAttrAt(
                GetPoint()->GetContentIndex(), RES_TXTATR_INPUTFIELD, ::sw::GetTextAttrMode::Parent));
            if (pInputField)
            {
                continue; // skip over input fields
            }
        }

        // If we were located inside a covered cell but our position has been
        // corrected, we check if the last move has moved the cursor to a
        // different table cell. In this case we set the cursor to the stored
        // covered position and redo the move:
        if (m_nRowSpanOffset)
        {
            const SwNode* pOldTabBoxSttNode = aOldNodeIdx.GetNode().FindTableBoxStartNode();
            const SwTableNode* pOldTabSttNode = pOldTabBoxSttNode ? pOldTabBoxSttNode->FindTableNode() : nullptr;
            const SwNode* pNewTabBoxSttNode = GetPoint()->GetNode().FindTableBoxStartNode();
            const SwTableNode* pNewTabSttNode = pNewTabBoxSttNode ? pNewTabBoxSttNode->FindTableNode() : nullptr;

            const bool bCellChanged = pOldTabSttNode && pNewTabSttNode &&
                                      pOldTabSttNode == pNewTabSttNode &&
                                      pOldTabBoxSttNode && pNewTabBoxSttNode &&
                                      pOldTabBoxSttNode != pNewTabBoxSttNode;

            if ( bCellChanged )
            {
                // Set cursor to start/end of covered cell:
                SwTableBox* pTableBox = pOldTabBoxSttNode->GetTableBox();
                if ( pTableBox && pTableBox->getRowSpan() > 1 )
                {
                    pTableBox = & pTableBox->FindEndOfRowSpan(
                        pOldTabSttNode->GetTable(),
                        o3tl::narrowing<sal_uInt16>(pTableBox->getRowSpan() + m_nRowSpanOffset));
                    SwPosition& rPtPos = *GetPoint();
                    SwNodeIndex aNewIdx( *pTableBox->GetSttNd() );
                    rPtPos.Assign( aNewIdx );

                    SwNodes::GoNextSection(&rPtPos, false, false);
                    SwContentNode* pContentNode = GetPointContentNode();
                    if ( pContentNode )
                    {
                        GetPoint()->SetContent( bLeft ? pContentNode->Len() : 0 );

                        // Redo the move:
                        if ( !Move( fnMove, fnGo ) )
                            break;
                    }
                }
                m_nRowSpanOffset = 0;
            }
        }

        // Check if I'm inside a covered cell. Correct cursor if necessary and
        // store covered cell:
        const SwNode* pTableBoxStartNode = GetPoint()->GetNode().FindTableBoxStartNode();
        if ( pTableBoxStartNode )
        {
            const SwTableBox* pTableBox = pTableBoxStartNode->GetTableBox();
            if ( pTableBox && pTableBox->getRowSpan() < 1 )
            {
                // Store the row span offset:
                m_nRowSpanOffset = pTableBox->getRowSpan();

                // Move cursor to non-covered cell:
                const SwTableNode* pTableNd = pTableBoxStartNode->FindTableNode();
                pTableBox = & pTableBox->FindStartOfRowSpan( pTableNd->GetTable() );
                SwPosition& rPtPos = *GetPoint();
                SwNodeIndex aNewIdx( *pTableBox->GetSttNd() );
                rPtPos.Assign( aNewIdx );

                SwNodes::GoNextSection(&rPtPos, false, false);
                SwContentNode* pContentNode = GetPointContentNode();
                if ( pContentNode )
                {
                    GetPoint()->SetContent( bLeft ? pContentNode->Len() : 0 );
                }
            }
        }
        --nCnt;
    }

    // here come some special rules for visual cursor travelling
    if ( pSttFrame )
    {
        SwNode& rTmpNode = GetPoint()->GetNode();
        if ( &rTmpNode != &rNode && rTmpNode.IsTextNode() )
        {
            Point aPt;
            std::pair<Point, bool> const tmp(aPt, true);
            const SwContentFrame* pEndFrame = rTmpNode.GetTextNode()->getLayoutFrame(
                GetDoc().getIDocumentLayoutAccess().GetCurrentLayout(),
                GetPoint(), &tmp);
            if ( pEndFrame )
            {
                if ( ! pEndFrame->IsRightToLeft() != ! pSttFrame->IsRightToLeft() )
                {
                    if ( ! bLeft )
                        pEndFrame->RightMargin( this );
                    else
                        pEndFrame->LeftMargin( this );
                }
            }
        }
    }

    return 0 == nCnt && !IsInProtectTable( true ) &&
            !IsSelOvr( SwCursorSelOverFlags::Toggle |
                       SwCursorSelOverFlags::ChangePos );
}

// calculate cursor bidi level: extracted from UpDown()
void SwCursor::DoSetBidiLevelUpDown()
{
    SwNode& rNode = GetPoint()->GetNode();
    if ( !rNode.IsTextNode() )
        return;

    SwTextFrame const* pFrame;
    const SwScriptInfo* pSI =
        SwScriptInfo::GetScriptInfo( *rNode.GetTextNode(), &pFrame );
    if ( !pSI )
        return;

    const sal_Int32 nPos = GetPoint()->GetContentIndex();

    if (!(nPos && nPos < rNode.GetTextNode()->GetText().getLength()))
        return;

    TextFrameIndex const nIndex(pFrame->MapModelToView(rNode.GetTextNode(), nPos));
    const sal_uInt8 nCurrLevel = pSI->DirType( nIndex );
    const sal_uInt8 nPrevLevel = pSI->DirType( nIndex - TextFrameIndex(1) );

    if ( nCurrLevel % 2 != nPrevLevel % 2 )
    {
        // set cursor level to the lower of the two levels
        SetCursorBidiLevel( std::min( nCurrLevel, nPrevLevel ) );
    }
    else
        SetCursorBidiLevel( nCurrLevel );
}

bool SwCursor::UpDown( bool bUp, sal_uInt16 nCnt,
                       Point const * pPt, tools::Long nUpDownX,
                       SwRootFrame & rLayout)
{
    SwTableCursor* pTableCursor = dynamic_cast<SwTableCursor*>(this);
    bool bAdjustTableCursor = false;

    // If the point/mark of the table cursor in the same box then set cursor to
    // beginning of the box
    if( pTableCursor && GetPointNode().StartOfSectionNode() ==
                    GetMarkNode().StartOfSectionNode() )
    {
        if ( End() != GetPoint() )
            Exchange();
        bAdjustTableCursor = true;
    }

    bool bRet = false;
    Point aPt;
    if( pPt )
        aPt = *pPt;
    std::pair<Point, bool> const temp(aPt, true);
    SwContentFrame* pFrame = GetPointContentNode()->getLayoutFrame(&rLayout, GetPoint(), &temp);

    if( pFrame )
    {
        SwCursorSaveState aSave( *this );

        if( !pPt )
        {
            SwRect aTmpRect;
            pFrame->GetCharRect( aTmpRect, *GetPoint() );
            aPt = aTmpRect.Pos();

            nUpDownX = pFrame->IsVertical() ?
                aPt.getY() - pFrame->getFrameArea().Top() :
                aPt.getX() - pFrame->getFrameArea().Left();
        }

        // It is allowed to move footnotes in other footnotes but not sections
        const bool bChkRange = !pFrame->IsInFootnote() || HasMark();
        const SwPosition aOldPos( *GetPoint() );
        const bool bInReadOnly = IsReadOnlyAvailable();

        if ( bAdjustTableCursor && !bUp )
        {
            // Special case: We have a table cursor but the start box has more
            // than one paragraph. If we want to go down, we have to set the
            // point to the last frame in the table box. This is only necessary
            // if we do not already have a table selection
            const SwStartNode* pTableNd = GetPointNode().FindTableBoxStartNode();
            OSL_ENSURE( pTableNd, "pTableCursor without SwTableNode?" );

            if ( pTableNd ) // safety first
            {
                const SwNode* pEndNd = pTableNd->EndOfSectionNode();
                GetPoint()->Assign( *pEndNd );
                pTableCursor->Move( fnMoveBackward, GoInNode );
                std::pair<Point, bool> const tmp(aPt, true);
                pFrame = GetPointContentNode()->getLayoutFrame(&rLayout, GetPoint(), &tmp);
            }
        }

        while( nCnt &&
               (bUp ? pFrame->UnitUp( this, nUpDownX, bInReadOnly )
                    : pFrame->UnitDown( this, nUpDownX, bInReadOnly ) ) &&
                CheckNodesRange( aOldPos.GetNode(), GetPoint()->GetNode(), bChkRange ))
        {
            std::pair<Point, bool> const tmp(aPt, true);
            pFrame = GetPointContentNode()->getLayoutFrame(&rLayout, GetPoint(), &tmp);
            --nCnt;
        }

        // iterate over whole number of items?
        if( !nCnt && !IsSelOvr( SwCursorSelOverFlags::Toggle |
                                SwCursorSelOverFlags::ChangePos ) )
        {
            if( !pTableCursor )
            {
                // try to position the cursor at half of the char-rect's height
                DisableCallbackAction a(rLayout);
                std::pair<Point, bool> const tmp(aPt, true);
                pFrame = GetPointContentNode()->getLayoutFrame(&rLayout, GetPoint(), &tmp);
                SwCursorMoveState eTmpState( CursorMoveState::UpDown );
                eTmpState.m_bSetInReadOnly = bInReadOnly;
                SwRect aTmpRect;
                pFrame->GetCharRect( aTmpRect, *GetPoint(), &eTmpState );
                if ( pFrame->IsVertical() )
                {
                    aPt.setX(aTmpRect.Center().getX());
                    pFrame->Calc(rLayout.GetCurrShell()->GetOut());
                    aPt.setY(pFrame->getFrameArea().Top() + nUpDownX);
                }
                else
                {
                    aPt.setY(aTmpRect.Center().getY());
                    pFrame->Calc(rLayout.GetCurrShell()->GetOut());
                    aPt.setX(pFrame->getFrameArea().Left() + nUpDownX);
                }
                pFrame->GetModelPositionForViewPoint( GetPoint(), aPt, &eTmpState );
            }
            bRet = !IsSelOvr( SwCursorSelOverFlags::Toggle | SwCursorSelOverFlags::ChangePos );
        }
        else if (!pFrame->IsInFootnote()) // tdf#150457 Jump to the begin/end
                                          // of the first/last line only if the
                                          // cursor is not inside a footnote
        {
            sal_Int32 nOffset = 0;

            // Jump to beginning or end of line when the cursor at first or last line.
            if(!bUp)
            {
                SwTextNode* pTextNd = GetPoint()->GetNode().GetTextNode();
                if (pTextNd)
                    nOffset = pTextNd->GetText().getLength();
            }
            const SwPosition aPos(*GetPointContentNode(), nOffset);

            //if cursor has already been at start or end of file,
            //Update cursor to change nUpDownX.
            if ( aOldPos.GetContentIndex() == nOffset )
            {
                if (SwEditShell* pSh = GetDoc().GetEditShell())
                    pSh->UpdateCursor();
                bRet = false;
            }
            else{
                *GetPoint() = aPos; // just give a new position
                bRet = true;
            }

        }
        else
            *GetPoint() = aOldPos;

        DoSetBidiLevelUpDown(); // calculate cursor bidi level
    }
    return bRet;
}

bool SwCursor::LeftRightMargin(SwRootFrame const& rLayout, bool bLeft, bool bAPI)
{
    Point aPt;
    std::pair<Point, bool> const tmp(aPt, true);
    SwContentFrame const*const pFrame = GetPointContentNode()->getLayoutFrame(
        &rLayout, GetPoint(), &tmp);

    // calculate cursor bidi level
    if ( pFrame )
        SetCursorBidiLevel( pFrame->IsRightToLeft() ? 1 : 0 );

    SwCursorSaveState aSave( *this );
    return pFrame
           && (bLeft ? pFrame->LeftMargin( this ) : pFrame->RightMargin( this, bAPI ) )
           && !IsSelOvr( SwCursorSelOverFlags::Toggle | SwCursorSelOverFlags::ChangePos );
}

bool SwCursor::IsAtLeftRightMargin(SwRootFrame const& rLayout, bool bLeft, bool bAPI) const
{
    bool bRet = false;
    Point aPt;
    std::pair<Point, bool> const tmp(aPt, true);
    SwContentFrame const*const pFrame = GetPointContentNode()->getLayoutFrame(
        &rLayout, GetPoint(), &tmp);
    if( pFrame )
    {
        SwPaM aPam( *GetPoint() );
        if( !bLeft && aPam.GetPoint()->GetContentIndex() )
            aPam.GetPoint()->AdjustContent(-1);
        bRet = (bLeft ? pFrame->LeftMargin( &aPam )
                      : pFrame->RightMargin( &aPam, bAPI ))
                && (!pFrame->IsTextFrame()
                    || static_cast<SwTextFrame const*>(pFrame)->MapModelToViewPos(*aPam.GetPoint())
                        == static_cast<SwTextFrame const*>(pFrame)->MapModelToViewPos(*GetPoint()));
    }
    return bRet;
}

bool SwCursor::SttEndDoc( bool bStt )
{
    SwCursorSaveState aSave( *this );
    // Never jump over section boundaries during selection!
    // Can the cursor still moved on?
    SwMoveFnCollection const & fnMove = bStt ? fnMoveBackward : fnMoveForward;
    bool bRet = (!HasMark() || !IsNoContent() ) &&
                    Move( fnMove, GoInDoc ) &&
                    !IsInProtectTable( true ) &&
                    !IsSelOvr( SwCursorSelOverFlags::Toggle |
                               SwCursorSelOverFlags::ChangePos |
                               SwCursorSelOverFlags::EnableRevDirection );
    return bRet;
}

bool SwCursor::GoPrevNextCell( bool bNext, sal_uInt16 nCnt )
{
    const SwTableNode* pTableNd = GetPoint()->GetNode().FindTableNode();
    if( !pTableNd )
        return false;

    // If there is another EndNode in front of the cell's StartNode then there
    // exists a previous cell
    SwCursorSaveState aSave( *this );
    SwPosition& rPtPos = *GetPoint();

    while( nCnt-- )
    {
        const SwNode* pTableBoxStartNode = rPtPos.GetNode().FindTableBoxStartNode();
        const SwTableBox* pTableBox = pTableBoxStartNode->GetTableBox();

        // Check if we have to move the cursor to a covered cell before
        // proceeding:
        if (m_nRowSpanOffset)
        {
            if ( pTableBox && pTableBox->getRowSpan() > 1 )
            {
                pTableBox = & pTableBox->FindEndOfRowSpan( pTableNd->GetTable(),
                    o3tl::narrowing<sal_uInt16>(pTableBox->getRowSpan() + m_nRowSpanOffset));
                rPtPos.Assign( *pTableBox->GetSttNd() );
                pTableBoxStartNode = rPtPos.GetNode().FindTableBoxStartNode();
            }
            m_nRowSpanOffset = 0;
        }

        const SwNode* pTmpNode = bNext ?
                                 pTableBoxStartNode->EndOfSectionNode() :
                                 pTableBoxStartNode;

        SwNodeIndex aCellIdx( *pTmpNode, bNext ? 1 : -1 );
        if(  (bNext && !aCellIdx.GetNode().IsStartNode()) ||
            (!bNext && !aCellIdx.GetNode().IsEndNode()) )
            return false;

        if (bNext)
            rPtPos.Assign( aCellIdx );
        else
            rPtPos.Assign(*aCellIdx.GetNode().StartOfSectionNode());

        pTableBoxStartNode = rPtPos.GetNode().FindTableBoxStartNode();
        pTableBox = pTableBoxStartNode->GetTableBox();
        if ( pTableBox && pTableBox->getRowSpan() < 1 )
        {
            m_nRowSpanOffset = pTableBox->getRowSpan();
            // move cursor to non-covered cell:
            pTableBox = & pTableBox->FindStartOfRowSpan( pTableNd->GetTable() );
            rPtPos.Assign( *pTableBox->GetSttNd() );
        }
    }

    rPtPos.Adjust(SwNodeOffset(1));
    if( !rPtPos.GetNode().IsContentNode() )
        SwNodes::GoNextSection(&rPtPos, true, false);
    GetPoint()->SetContent( 0 );

    return !IsInProtectTable( true );
}

bool SwTableCursor::GotoTable( const OUString& )
{
    return false; // invalid action
}

bool SwCursor::GotoTable( const OUString& rName )
{
    bool bRet = false;
    if ( !HasMark() )
    {
        SwTable* pTmpTable = SwTable::FindTable( GetDoc().FindTableFormatByName( rName ) );
        if( pTmpTable )
        {
            // a table in a normal nodes array
            SwCursorSaveState aSave( *this );
            GetPoint()->Assign( *pTmpTable->GetTabSortBoxes()[ 0 ]->
                                GetSttNd()->FindTableNode() );
            Move( fnMoveForward, GoInContent );
            bRet = !IsSelOvr();
        }
    }
    return bRet;
}

bool SwCursor::GotoTableBox( const OUString& rName )
{
    bool bRet = false;
    const SwTableNode* pTableNd = GetPoint()->GetNode().FindTableNode();
    if( pTableNd )
    {
        // retrieve box by name
        const SwTableBox* pTableBox = pTableNd->GetTable().GetTableBox( rName );
        if( pTableBox && pTableBox->GetSttNd() &&
            ( !pTableBox->GetFrameFormat()->GetProtect().IsContentProtected() ||
              IsReadOnlyAvailable() ) )
        {
            SwCursorSaveState aSave( *this );
            GetPoint()->Assign( *pTableBox->GetSttNd() );
            Move( fnMoveForward, GoInContent );
            bRet = !IsSelOvr();
        }
    }
    return bRet;
}

bool SwCursor::MovePara(SwWhichPara fnWhichPara, SwMoveFnCollection const & fnPosPara )
{
    // for optimization test something before
    const SwNode* pNd = &GetPoint()->GetNode();
    bool bShortCut = false;
    if ( fnWhichPara == GoCurrPara )
    {
        // #i41048#
        // If fnWhichPara == GoCurrPara then (*fnWhichPara)( *this, fnPosPara )
        // can already move the cursor to a different text node. In this case
        // we better check if IsSelOvr().
        const SwContentNode* pContentNd = pNd->GetContentNode();
        if ( pContentNd )
        {
            const sal_Int32 nSttEnd = &fnPosPara == &fnMoveForward ? 0 : pContentNd->Len();
            if ( GetPoint()->GetContentIndex() != nSttEnd )
                bShortCut = true;
        }
    }
    else
    {
        if ( pNd->IsTextNode() &&
             pNd->GetNodes()[ pNd->GetIndex() +
                    SwNodeOffset(fnWhichPara == GoNextPara ? 1 : -1 ) ]->IsTextNode() )
            bShortCut = true;
    }

    if ( bShortCut )
        return (*fnWhichPara)( *this, fnPosPara );

    // else we must use the SaveStructure, because the next/prev is not
    // a same node type.
    SwCursorSaveState aSave( *this );
    return (*fnWhichPara)( *this, fnPosPara ) &&
            !IsInProtectTable( true ) &&
            !IsSelOvr( SwCursorSelOverFlags::Toggle |
                       SwCursorSelOverFlags::ChangePos );
}

bool SwCursor::MoveSection( SwWhichSection fnWhichSect,
                                SwMoveFnCollection const & fnPosSect)
{
    SwCursorSaveState aSave( *this );
    return (*fnWhichSect)( *this, fnPosSect ) &&
            !IsInProtectTable( true ) &&
            !IsSelOvr( SwCursorSelOverFlags::Toggle |
                       SwCursorSelOverFlags::ChangePos );
}

void SwCursor::RestoreSavePos()
{
    // This method is not supposed to be used in cases when nodes may be
    // deleted; detect such cases, but do not crash (example: fdo#40831).
    SwNodeOffset uNodeCount(GetPoint()->GetNodes().Count());
    OSL_ENSURE(m_vSavePos.empty() || m_vSavePos.back().nNode < uNodeCount,
        "SwCursor::RestoreSavePos: invalid node: "
        "probably something was deleted; consider using SwUnoCursor instead");
    if (m_vSavePos.empty() || m_vSavePos.back().nNode >= uNodeCount)
        return;

    GetPoint()->Assign( m_vSavePos.back().nNode );

    sal_Int32 nIdx = 0;
    if ( GetPointContentNode() )
    {
        if (m_vSavePos.back().nContent <= GetPointContentNode()->Len())
            nIdx = m_vSavePos.back().nContent;
        else
        {
            nIdx = GetPointContentNode()->Len();
            OSL_FAIL("SwCursor::RestoreSavePos: invalid content index");
        }
    }
    GetPoint()->SetContent( nIdx );
}

SwTableCursor::SwTableCursor( const SwPosition &rPos )
    : SwCursor( rPos, nullptr )
{
    m_bParked = false;
    m_bChanged = false;
    m_nTablePtNd = SwNodeOffset(0);
    m_nTableMkNd = SwNodeOffset(0);
    m_nTablePtCnt = 0;
    m_nTableMkCnt = 0;
}

SwTableCursor::~SwTableCursor() {}

static bool
lcl_SeekEntry(const SwSelBoxes& rTmp, SwStartNode const*const pSrch,
        size_t & o_rFndPos)
{
    SwNodeOffset nIdx = pSrch->GetIndex();

    size_t nO = rTmp.size();
    if( nO > 0 )
    {
        nO--;
        size_t nU = 0;
        while( nU <= nO )
        {
            size_t nM = nU + ( nO - nU ) / 2;
            if( rTmp[ nM ]->GetSttNd() == pSrch )
            {
                o_rFndPos = nM;
                return true;
            }
            else if( rTmp[ nM ]->GetSttIdx() < nIdx )
                nU = nM + 1;
            else if( nM == 0 )
                return false;
            else
                nO = nM - 1;
        }
    }
    return false;
}

SwCursor* SwTableCursor::MakeBoxSels( SwCursor* pCurrentCursor )
{
    if (m_bChanged)
    {
        if (m_bParked)
        {
            // move back into content
            Exchange();
            Move( fnMoveForward );
            Exchange();
            Move( fnMoveForward );
            m_bParked = false;
        }

        m_bChanged = false;

        // create temporary copies so that all boxes that
        // have already cursors can be removed
        SwSelBoxes aTmp(m_SelectedBoxes);

        // compare old and new ones
        SwNodes& rNds = pCurrentCursor->GetDoc().GetNodes();
        const SwStartNode* pSttNd;
        SwCursor* pCur = pCurrentCursor;
        do {
            size_t nPos;
            bool bDel = false;
            pSttNd = pCur->GetPoint()->GetNode().FindTableBoxStartNode();
            if( !pCur->HasMark() || !pSttNd ||
                pSttNd != pCur->GetMark()->GetNode().FindTableBoxStartNode() )
                bDel = true;

            else if( lcl_SeekEntry( aTmp, pSttNd, nPos ))
            {
                SwNodeIndex aIdx( *pSttNd, 1 );
                const SwNode* pNd = &aIdx.GetNode();
                if( !pNd->IsContentNode() )
                    pNd = SwNodes::GoNextSection(&aIdx, true, false);

                SwPosition* pPos = pCur->GetMark();
                if( pNd != &pPos->GetNode() )
                    pPos->Assign( *pNd );
                pPos->SetContent( 0 );

                aIdx.Assign( *pSttNd->EndOfSectionNode(), - 1 );
                pNd = &aIdx.GetNode();
                if( !pNd->IsContentNode() )
                    pNd = SwNodes::GoPrevSection( &aIdx, true, false );

                pPos = pCur->GetPoint();
                if (pNd && pNd != &pPos->GetNode())
                    pPos->Assign( *pNd );
                pPos->SetContent( pNd ? static_cast<const SwContentNode*>(pNd)->Len() : 0);

                aTmp.erase( aTmp.begin() + nPos );
            }
            else
                bDel = true;

            pCur = pCur->GetNext();
            if( bDel )
            {
                SwCursor* pDel = pCur->GetPrev();
                if (pDel == dynamic_cast<SwShellCursor*>(pCurrentCursor))
                    pCurrentCursor = pDel->GetPrev();

                if( pDel == pCurrentCursor )
                    pCurrentCursor->DeleteMark();
                else
                    delete pDel;
            }
        } while ( pCurrentCursor != pCur );

        for (size_t nPos = 0; nPos < aTmp.size(); ++nPos)
        {
            pSttNd = aTmp[ nPos ]->GetSttNd();

            SwNodeIndex aIdx( *pSttNd, 1 );
            if( &aIdx.GetNodes() != &rNds )
                break;
            SwNode* pNd = &aIdx.GetNode();
            if( !pNd->IsContentNode() )
                pNd = SwNodes::GoNextSection(&aIdx, true, false);

            SwPaM *const pNew = (!pCurrentCursor->IsMultiSelection() && !pCurrentCursor->HasMark())
                ? pCurrentCursor
                : pCurrentCursor->Create( pCurrentCursor );
            pNew->GetPoint()->Assign( *pNd );
            pNew->SetMark();

            SwPosition* pPos = pNew->GetPoint();
            pPos->Assign( *pSttNd->EndOfSectionNode(), - 1 );
            pNd = &pPos->GetNode();
            if( !pNd->IsContentNode() )
                pNd = SwNodes::GoPrevSection( pPos, true, false );
            if (pNd)
                pPos->AssignEndIndex(*static_cast<SwContentNode*>(pNd));
        }
    }
    return pCurrentCursor;
}

void SwTableCursor::InsertBox( const SwTableBox& rTableBox )
{
    SwTableBox* pBox = const_cast<SwTableBox*>(&rTableBox);
    m_SelectedBoxes.insert(pBox);
    m_bChanged = true;
}

void SwTableCursor::DeleteBox(size_t const nPos)
{
    m_SelectedBoxes.erase(m_SelectedBoxes.begin() + nPos);
    m_bChanged = true;
}

bool SwTableCursor::NewTableSelection()
{
    bool bRet = false;
    const SwNode *pStart = GetPointNode().FindTableBoxStartNode();
    const SwNode *pEnd = GetMarkNode().FindTableBoxStartNode();
    if( pStart && pEnd )
    {
        const SwTableNode *pTableNode = pStart->FindTableNode();
        if( pTableNode == pEnd->FindTableNode() &&
            pTableNode->GetTable().IsNewModel() )
        {
            bRet = true;
            SwSelBoxes aNew(m_SelectedBoxes);
            pTableNode->GetTable().CreateSelection( pStart, pEnd, aNew,
                SwTable::SEARCH_NONE, false );
            ActualizeSelection( aNew );
        }
    }
    return bRet;
}

void SwTableCursor::ActualizeSelection( const SwSelBoxes &rNew )
{
    size_t nOld = 0, nNew = 0;
    while (nOld < m_SelectedBoxes.size() && nNew < rNew.size())
    {
        SwTableBox const*const pPOld = m_SelectedBoxes[ nOld ];
        const SwTableBox* pPNew = rNew[ nNew ];
        if( pPOld == pPNew )
        {   // this box will stay
            ++nOld;
            ++nNew;
        }
        else if( pPOld->GetSttIdx() < pPNew->GetSttIdx() )
        {
            DeleteBox( nOld ); // this box has to go
        }
        else
        {
            InsertBox( *pPNew ); // this is a new one
            ++nOld;
            ++nNew;
        }
    }

    while (nOld < m_SelectedBoxes.size())
    {
        DeleteBox( nOld ); // some more to delete
    }

    for ( ; nNew < rNew.size(); ++nNew ) // some more to insert
    {
        InsertBox( *rNew[ nNew ] );
    }
}

bool SwTableCursor::IsCursorMovedUpdate()
{
    if( !IsCursorMoved() )
        return false;

    m_nTableMkNd = GetMark()->GetNodeIndex();
    m_nTablePtNd = GetPoint()->GetNodeIndex();
    m_nTableMkCnt = GetMark()->GetContentIndex();
    m_nTablePtCnt = GetPoint()->GetContentIndex();
    return true;
}

/// park table cursor on the boxes' start node
void SwTableCursor::ParkCursor()
{
    // de-register index from text node
    SwNode* pNd = &GetPoint()->GetNode();
    if( !pNd->IsStartNode() )
        pNd = pNd->StartOfSectionNode();
    GetPoint()->Assign(*pNd);

    pNd = &GetMark()->GetNode();
    if( !pNd->IsStartNode() )
        pNd = pNd->StartOfSectionNode();
    GetMark()->Assign(*pNd);

    m_bChanged = true;
    m_bParked = true;
}

bool SwTableCursor::HasReadOnlyBoxSel() const
{
    bool bRet = false;
    for (size_t n = m_SelectedBoxes.size(); n; )
    {
        if (m_SelectedBoxes[--n]->GetFrameFormat()->GetProtect().IsContentProtected())
        {
            bRet = true;
            break;
        }
    }
    return bRet;
}

bool SwTableCursor::HasHiddenBoxSel() const
{
    bool bRet = false;
    for (size_t n = m_SelectedBoxes.size(); n; )
    {
        if (m_SelectedBoxes[--n]->GetFrameFormat()->IsHidden())
        {
            bRet = true;
            break;
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
