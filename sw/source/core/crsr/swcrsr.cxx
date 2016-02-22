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
#include <com/sun/star/i18n/CharType.hpp>
#include <unotools/charclass.hxx>
#include <svl/ctloptions.hxx>
#include <swmodule.hxx>
#include <fmtcntnt.hxx>
#include <swtblfmt.hxx>
#include <swcrsr.hxx>
#include <unocrsr.hxx>
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
#include <scriptinfo.hxx>
#include <crstate.hxx>
#include <docsh.hxx>
#include <viewsh.hxx>
#include <frmatr.hxx>
#include <breakit.hxx>
#include <crsskip.hxx>
#include <vcl/msgbox.hxx>
#include <mdiexp.hxx>
#include <statstr.hrc>
#include <redline.hxx>
#include <txatbase.hxx>
#include <memory>
#include <comphelper/lok.hxx>

using namespace ::com::sun::star::i18n;

static const sal_uInt16 coSrchRplcThreshold = 60000;

struct _PercentHdl
{
    SwDocShell* pDSh;
    sal_uLong nActPos;
    bool bBack, bNodeIdx;

    _PercentHdl( sal_uLong nStt, sal_uLong nEnd, SwDocShell* pSh )
        : pDSh(pSh), bBack(false), bNodeIdx(false)
    {
        nActPos = nStt;
        if( ( bBack = (nStt > nEnd )) )
        {
            sal_uLong n = nStt; nStt = nEnd; nEnd = n;
        }
        ::StartProgress( STR_STATSTR_SEARCH, nStt, nEnd );
    }

    explicit _PercentHdl( const SwPaM& rPam )
        : pDSh( rPam.GetDoc()->GetDocShell() )
    {
        sal_uLong nStt, nEnd;
        if( rPam.GetPoint()->nNode == rPam.GetMark()->nNode )
        {
            bNodeIdx = false;
            nStt = rPam.GetMark()->nContent.GetIndex();
            nEnd = rPam.GetPoint()->nContent.GetIndex();
        }
        else
        {
            bNodeIdx = true;
            nStt = rPam.GetMark()->nNode.GetIndex();
            nEnd = rPam.GetPoint()->nNode.GetIndex();
        }
        nActPos = nStt;
        if( ( bBack = (nStt > nEnd )) )
        {
            sal_uLong n = nStt; nStt = nEnd; nEnd = n;
        }
        ::StartProgress( STR_STATSTR_SEARCH, nStt, nEnd, pDSh );
    }

    ~_PercentHdl()                      { ::EndProgress( pDSh ); }

    void NextPos( sal_uLong nPos ) const
        { ::SetProgressState( bBack ? nActPos - nPos : nPos, pDSh ); }

    void NextPos( SwPosition& rPos ) const
        {
            sal_uLong nPos;
            if( bNodeIdx )
                nPos = rPos.nNode.GetIndex();
            else
                nPos = rPos.nContent.GetIndex();
            ::SetProgressState( bBack ? nActPos - nPos : nPos, pDSh );
        }
};

SwCursor::SwCursor( const SwPosition &rPos, SwPaM* pRing, bool bColumnSel )
    : SwPaM( rPos, pRing )
    , m_pSavePos(nullptr)
    , m_nRowSpanOffset(0)
    , m_nCursorBidiLevel(0)
    , m_bColumnSelection(bColumnSel)
{
}

// @@@ semantic: no copy ctor.
SwCursor::SwCursor(SwCursor const& rCpy, SwPaM *const pRing)
    : SwPaM( rCpy, pRing )
    , m_pSavePos(nullptr)
    , m_nRowSpanOffset(rCpy.m_nRowSpanOffset)
    , m_nCursorBidiLevel(rCpy.m_nCursorBidiLevel)
    , m_bColumnSelection(rCpy.m_bColumnSelection)
{
}

SwCursor::~SwCursor()
{
    while( m_pSavePos )
    {
        _SwCursor_SavePos* pNxt = m_pSavePos->pNext;
        delete m_pSavePos;
        m_pSavePos = pNxt;
    }
}

SwCursor* SwCursor::Create( SwPaM* pRing ) const
{
    return new SwCursor( *GetPoint(), pRing, false );
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
    _SwCursor_SavePos* pNew = CreateNewSavePos();
    pNew->pNext = m_pSavePos;
    m_pSavePos = pNew;
}

void SwCursor::RestoreState()
{
    if (m_pSavePos) // Robust
    {
        _SwCursor_SavePos* pDel = m_pSavePos;
        m_pSavePos = m_pSavePos->pNext;
        delete pDel;
    }
}

_SwCursor_SavePos* SwCursor::CreateNewSavePos() const
{
    return new _SwCursor_SavePos( *this );
}

/// determine if point is outside of the node-array's content area
bool SwCursor::IsNoContent() const
{
    return GetPoint()->nNode.GetIndex() <
            GetDoc()->GetNodes().GetEndOfExtras().GetIndex();
}

bool SwCursor::IsSelOvrCheck(int)
{
    return false;
}

// extracted from IsSelOvr()
bool SwTableCursor::IsSelOvrCheck(int eFlags)
{
    SwNodes& rNds = GetDoc()->GetNodes();
    // check sections of nodes array
    if( (nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION & eFlags)
        && HasMark() )
    {
        SwNodeIndex aOldPos( rNds, GetSavePos()->nNode );
        if( !CheckNodesRange( aOldPos, GetPoint()->nNode, true ))
        {
            GetPoint()->nNode = aOldPos;
            GetPoint()->nContent.Assign( GetContentNode(), GetSavePos()->nContent );
            return true;
        }
    }
    return SwCursor::IsSelOvrCheck(eFlags);
}

namespace
{
    const SwTextAttr* InputFieldAtPos(SwPosition *pPos)
    {
        SwTextNode* pTextNd = pPos->nNode.GetNode().GetTextNode();
        if (!pTextNd)
            return nullptr;
        return pTextNd->GetTextAttrAt(pPos->nContent.GetIndex(), RES_TXTATR_INPUTFIELD, SwTextNode::PARENT);
    }
}

bool SwCursor::IsSelOvr( int eFlags )
{
    SwDoc* pDoc = GetDoc();
    SwNodes& rNds = pDoc->GetNodes();

    bool bSkipOverHiddenSections = IsSkipOverHiddenSections();
    bool bSkipOverProtectSections = IsSkipOverProtectSections();

    if ( IsSelOvrCheck( eFlags ) )
    {
        return true;
    }

    if (m_pSavePos->nNode != GetPoint()->nNode.GetIndex() &&
        // (1997) in UI-ReadOnly everything is allowed
        ( !pDoc->GetDocShell() || !pDoc->GetDocShell()->IsReadOnlyUI() ))
    {
        // check new sections
        SwNodeIndex& rPtIdx = GetPoint()->nNode;
        const SwSectionNode* pSectNd = rPtIdx.GetNode().FindSectionNode();
        if( pSectNd &&
            ((bSkipOverHiddenSections && pSectNd->GetSection().IsHiddenFlag() ) ||
            (bSkipOverProtectSections && pSectNd->GetSection().IsProtectFlag() )))
        {
            if( 0 == ( nsSwCursorSelOverFlags::SELOVER_CHANGEPOS & eFlags ) )
            {
                // then we're already done
                RestoreSavePos();
                return true;
            }

            // set cursor to new position:
            SwNodeIndex aIdx( rPtIdx );
            sal_Int32 nContentPos = m_pSavePos->nContent;
            bool bGoNxt = m_pSavePos->nNode < rPtIdx.GetIndex();
            SwContentNode* pCNd = bGoNxt
                ? rNds.GoNextSection( &rPtIdx, bSkipOverHiddenSections, bSkipOverProtectSections)
                : SwNodes::GoPrevSection( &rPtIdx, bSkipOverHiddenSections, bSkipOverProtectSections);
            if( !pCNd && ( nsSwCursorSelOverFlags::SELOVER_ENABLEREVDIREKTION & eFlags ))
            {
                bGoNxt = !bGoNxt;
                pCNd = bGoNxt ? rNds.GoNextSection( &rPtIdx, bSkipOverHiddenSections, bSkipOverProtectSections)
                    : SwNodes::GoPrevSection( &rPtIdx, bSkipOverHiddenSections, bSkipOverProtectSections);
            }

            bool bIsValidPos = nullptr != pCNd;
            const bool bValidNodesRange = bIsValidPos &&
                ::CheckNodesRange( rPtIdx, aIdx, true );
            if( !bValidNodesRange )
            {
                rPtIdx = m_pSavePos->nNode;
                if( nullptr == ( pCNd = rPtIdx.GetNode().GetContentNode() ) )
                {
                    bIsValidPos = false;
                    nContentPos = 0;
                    rPtIdx = aIdx;
                    if( nullptr == ( pCNd = rPtIdx.GetNode().GetContentNode() ) )
                    {
                        // then to the beginning of the document
                        rPtIdx = rNds.GetEndOfExtras();
                        pCNd = rNds.GoNext( &rPtIdx );
                    }
                }
            }

            // register ContentIndex:
            const sal_Int32 nTmpPos = bIsValidPos ? (bGoNxt ? 0 : pCNd->Len()) : nContentPos;
            GetPoint()->nContent.Assign( pCNd, nTmpPos );
            if( !bIsValidPos || !bValidNodesRange ||
                IsInProtectTable( true ) )
                return true;
        }

        // is there a protected section in the section?
        if( HasMark() && bSkipOverProtectSections)
        {
            sal_uLong nSttIdx = GetMark()->nNode.GetIndex(),
                nEndIdx = GetPoint()->nNode.GetIndex();
            if( nEndIdx <= nSttIdx )
            {
                sal_uLong nTmp = nSttIdx;
                nSttIdx = nEndIdx;
                nEndIdx = nTmp;
            }

            const SwSectionFormats& rFormats = pDoc->GetSections();
            for( SwSectionFormats::size_type n = 0; n < rFormats.size(); ++n )
            {
                const SwSectionFormat* pFormat = rFormats[n];
                const SvxProtectItem& rProtect = pFormat->GetProtect();
                if( rProtect.IsContentProtected() )
                {
                    const SwFormatContent& rContent = pFormat->GetContent(false);
                    OSL_ENSURE( rContent.GetContentIdx(), "No SectionNode?" );
                    sal_uLong nIdx = rContent.GetContentIdx()->GetIndex();
                    if( nSttIdx <= nIdx && nEndIdx >= nIdx )
                    {
                        // if it is no linked section then we cannot select it
                        const SwSection& rSect = *pFormat->GetSection();
                        if( CONTENT_SECTION == rSect.GetType() )
                        {
                            RestoreSavePos();
                            return true;
                        }
                    }
                }
            }
        }
    }

    const SwNode* pNd = &GetPoint()->nNode.GetNode();
    if( pNd->IsContentNode() && !dynamic_cast<SwUnoCursor*>(this) )
    {
        const SwContentFrame* pFrame = static_cast<const SwContentNode*>(pNd)->getLayoutFrame( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() );
        if ( (nsSwCursorSelOverFlags::SELOVER_CHANGEPOS & eFlags)   //allowed to change position if it's a bad one
            && pFrame && pFrame->IsValid() && !pFrame->Frame().Height()     //a bad zero height position
            && !InputFieldAtPos(GetPoint()) )                       //unless it's a (vertical) input field
        {
            // skip to the next/prev valid paragraph with a layout
            SwNodeIndex& rPtIdx = GetPoint()->nNode;
            bool bGoNxt = m_pSavePos->nNode < rPtIdx.GetIndex();
            while( nullptr != ( pFrame = ( bGoNxt ? pFrame->GetNextContentFrame() : pFrame->GetPrevContentFrame() ))
                   && 0 == pFrame->Frame().Height() )
                ;

            // #i72394# skip to prev/next valid paragraph with a layout in case
            // the first search did not succeed:
            if( !pFrame )
            {
                bGoNxt = !bGoNxt;
                pFrame = static_cast<const SwContentNode*>(pNd)->getLayoutFrame( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() );
                while ( pFrame && 0 == pFrame->Frame().Height() )
                {
                    pFrame = bGoNxt ? pFrame->GetNextContentFrame()
                        :   pFrame->GetPrevContentFrame();
                }
            }

            SwContentNode* pCNd = (pFrame != nullptr) ? const_cast<SwContentNode*>(pFrame->GetNode()) : nullptr;
            if ( pCNd != nullptr )
            {
                // set this ContentNode as new position
                rPtIdx = *pCNd;

                // assign corresponding ContentIndex
                const sal_Int32 nTmpPos = bGoNxt ? 0 : pCNd->Len();
                GetPoint()->nContent.Assign( pCNd, nTmpPos );

                if (rPtIdx.GetIndex() == m_pSavePos->nNode
                    && nTmpPos == m_pSavePos->nContent)
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
            DeleteMark();
            RestoreSavePos();
            return true; // we need a frame
        }
    }

    // is the cursor allowed to be in a protected node?
    if( 0 == ( nsSwCursorSelOverFlags::SELOVER_CHANGEPOS & eFlags ) && !IsAtValidPos() )
    {
        DeleteMark();
        RestoreSavePos();
        return true;
    }

    if( !HasMark() )
        return false;

    // check for invalid sections
    if( !::CheckNodesRange( GetMark()->nNode, GetPoint()->nNode, true ))
    {
        DeleteMark();
        RestoreSavePos();
        return true; // we need a frame
    }

    if( (pNd = &GetMark()->nNode.GetNode())->IsContentNode()
        && !static_cast<const SwContentNode*>(pNd)->getLayoutFrame( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() )
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
            const sal_uLong nRefNodeIdx =
                ( nsSwCursorSelOverFlags::SELOVER_TOGGLE & eFlags )
                ? m_pSavePos->nNode
                : GetMark()->nNode.GetIndex();
            const sal_Int32 nRefContentIdx =
                ( nsSwCursorSelOverFlags::SELOVER_TOGGLE & eFlags )
                ? m_pSavePos->nContent
                : GetMark()->nContent.GetIndex();
            const bool bIsForwardSelection =
                nRefNodeIdx < GetPoint()->nNode.GetIndex()
                || ( nRefNodeIdx == GetPoint()->nNode.GetIndex()
                     && nRefContentIdx < GetPoint()->nContent.GetIndex() );

            if ( pInputFieldTextAttrAtPoint != nullptr )
            {
                const sal_Int32 nNewPointPos =
                    bIsForwardSelection ? *(pInputFieldTextAttrAtPoint->End()) : pInputFieldTextAttrAtPoint->GetStart();
                SwTextNode* pTextNdAtPoint = GetPoint()->nNode.GetNode().GetTextNode();
                GetPoint()->nContent.Assign( pTextNdAtPoint, nNewPointPos );
            }

            if ( pInputFieldTextAttrAtMark != nullptr )
            {
                const sal_Int32 nNewMarkPos =
                    bIsForwardSelection ? pInputFieldTextAttrAtMark->GetStart() : *(pInputFieldTextAttrAtMark->End());
                SwTextNode* pTextNdAtMark = GetMark()->nNode.GetNode().GetTextNode();
                GetMark()->nContent.Assign( pTextNdAtMark, nNewMarkPos );
            }
        }
    }

    const SwTableNode* pPtNd = GetPoint()->nNode.GetNode().FindTableNode();
    const SwTableNode* pMrkNd = GetMark()->nNode.GetNode().FindTableNode();
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
    if (nsSwCursorSelOverFlags::SELOVER_CHANGEPOS & eFlags)
    {
        bool bSelTop = GetPoint()->nNode.GetIndex() <
            ((nsSwCursorSelOverFlags::SELOVER_TOGGLE & eFlags)
                 ? m_pSavePos->nNode : GetMark()->nNode.GetIndex());

        do { // loop for table after table
            sal_uLong nSEIdx = pPtNd->EndOfSectionIndex();
            sal_uLong nSttEndTable = nSEIdx + 1;

            if( bSelTop )
                nSttEndTable = rNds[ nSEIdx ]->StartOfSectionIndex() - 1;

            GetPoint()->nNode = nSttEndTable;
            const SwNode* pMyNd = &(GetNode());

            if( pMyNd->IsSectionNode() || ( pMyNd->IsEndNode() &&
                pMyNd->StartOfSectionNode()->IsSectionNode() ) )
            {
                pMyNd = bSelTop
                    ? SwNodes::GoPrevSection( &GetPoint()->nNode,true,false )
                    : rNds.GoNextSection( &GetPoint()->nNode,true,false );

                /* #i12312# Handle failure of Go{Prev|Next}Section */
                if ( nullptr == pMyNd)
                    break;

                if( nullptr != ( pPtNd = pMyNd->FindTableNode() ))
                    continue;
            }

            // we permit these
            if( pMyNd->IsContentNode() &&
                ::CheckNodesRange( GetMark()->nNode,
                GetPoint()->nNode, true ))
            {
                // table in table
                const SwTableNode* pOuterTableNd = pMyNd->FindTableNode();
                if ( pOuterTableNd )
                    pMyNd = pOuterTableNd;
                else
                {
                    SwContentNode* pCNd = const_cast<SwContentNode*>(static_cast<const SwContentNode*>(pMyNd));
                    GetPoint()->nContent.Assign( pCNd, bSelTop ? pCNd->Len() : 0 );
                    return false;
                }
            }
            if( bSelTop
                ? ( !pMyNd->IsEndNode() || nullptr == ( pPtNd = pMyNd->FindTableNode() ))
                : nullptr == ( pPtNd = pMyNd->GetTableNode() ))
                break;
        } while( true );
    }

    // stay on old position
    RestoreSavePos();
    return true;
}

bool SwCursor::IsInProtectTable( bool bMove, bool bChgCursor )
{
    SwContentNode* pCNd = GetContentNode();
    if( !pCNd )
        return false;

    // No table, no protected cell:
    const SwTableNode* pTableNode = pCNd->FindTableNode();
    if ( !pTableNode )
        return false;

    // Current position == last save position?
    if (m_pSavePos->nNode == GetPoint()->nNode.GetIndex())
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
    if (m_pSavePos->nNode < GetPoint()->nNode.GetIndex())
    {
        // search next valid box
        // if there is another StartNode after the EndNode of a cell then
        // there is another cell
        SwNodeIndex aCellStt( *GetNode().FindTableBoxStartNode()->EndOfSectionNode(), 1 );
        bool bProt = true;
GoNextCell:
        do {
            if( !aCellStt.GetNode().IsStartNode() )
                break;
            ++aCellStt;
            if( nullptr == ( pCNd = aCellStt.GetNode().GetContentNode() ))
                pCNd = aCellStt.GetNodes().GoNext( &aCellStt );
            if( !( bProt = pCNd->IsProtect() ))
                break;
            aCellStt.Assign( *pCNd->FindTableBoxStartNode()->EndOfSectionNode(), 1 );
        } while( bProt );

SetNextCursor:
        if( !bProt ) // found free cell
        {
            GetPoint()->nNode = aCellStt;
            SwContentNode* pTmpCNd = GetContentNode();
            if( pTmpCNd )
            {
                GetPoint()->nContent.Assign( pTmpCNd, 0 );
                return false;
            }
            return IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                             nsSwCursorSelOverFlags::SELOVER_CHANGEPOS );
        }
        // end of table, so go to next node
        ++aCellStt;
        SwNode* pNd;
        if( ( pNd = &aCellStt.GetNode())->IsEndNode() || HasMark())
        {
            // if only table in FlyFrame or SSelection then stay on old position
            if( bChgCursor )
                RestoreSavePos();
            return true;
        }
        else if( pNd->IsTableNode() && aCellStt++ )
            goto GoNextCell;

        bProt = false; // index is now on a content node
        goto SetNextCursor;
    }

    // search for the previous valid box
    {
        // if there is another EndNode in front of the StartNode than there
        // exists a previous cell
        SwNodeIndex aCellStt( *GetNode().FindTableBoxStartNode(), -1 );
        SwNode* pNd;
        bool bProt = true;
GoPrevCell:
        do {
            if( !( pNd = &aCellStt.GetNode())->IsEndNode() )
                break;
            aCellStt.Assign( *pNd->StartOfSectionNode(), +1 );
            if( nullptr == ( pCNd = aCellStt.GetNode().GetContentNode() ))
                pCNd = pNd->GetNodes().GoNext( &aCellStt );
            if( !( bProt = pCNd->IsProtect() ))
                break;
            aCellStt.Assign( *pNd->FindTableBoxStartNode(), -1 );
        } while( bProt );

SetPrevCursor:
        if( !bProt ) // found free cell
        {
            GetPoint()->nNode = aCellStt;
            SwContentNode* pTmpCNd = GetContentNode();
            if( pTmpCNd )
            {
                GetPoint()->nContent.Assign( pTmpCNd, 0 );
                return false;
            }
            return IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                             nsSwCursorSelOverFlags::SELOVER_CHANGEPOS );
        }
        // at the beginning of a table, so go to next node
        --aCellStt;
        if( ( pNd = &aCellStt.GetNode())->IsStartNode() || HasMark() )
        {
            // if only table in FlyFrame or SSelection then stay on old position
            if( bChgCursor )
                RestoreSavePos();
            return true;
        }
        else if( pNd->StartOfSectionNode()->IsTableNode() && aCellStt-- )
            goto GoPrevCell;

        bProt = false; // index is now on a content node
        goto SetPrevCursor;
    }
}

/// Return <true> if cursor can be set to this position
bool SwCursor::IsAtValidPos( bool bPoint ) const
{
    const SwDoc* pDoc = GetDoc();
    const SwPosition* pPos = bPoint ? GetPoint() : GetMark();
    const SwNode* pNd = &pPos->nNode.GetNode();

    if( pNd->IsContentNode() && !static_cast<const SwContentNode*>(pNd)->getLayoutFrame( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() ) &&
        !dynamic_cast<const SwUnoCursor*>(this) )
    {
        return false;
    }

    // #i45129# - in UI-ReadOnly everything is allowed
    if( !pDoc->GetDocShell() || !pDoc->GetDocShell()->IsReadOnlyUI() )
        return true;

    const bool bCursorInReadOnly = IsReadOnlyAvailable();
    if( !bCursorInReadOnly && pNd->IsProtect() )
        return false;

    const SwSectionNode* pSectNd = pNd->FindSectionNode();
    if( pSectNd && (pSectNd->GetSection().IsHiddenFlag() ||
                    ( !bCursorInReadOnly && pSectNd->GetSection().IsProtectFlag() )))
        return false;

    return true;
}

void SwCursor::SaveTableBoxContent( const SwPosition* ) {}

/// set range for search in document
SwMoveFnCollection* SwCursor::MakeFindRange( SwDocPositions nStart,
                                SwDocPositions nEnd, SwPaM* pRange ) const
{
    pRange->SetMark();
    FillFindPos( nStart, *pRange->GetMark() );
    FillFindPos( nEnd, *pRange->GetPoint() );

    // determine direction of search
    return ( DOCPOS_START == nStart || DOCPOS_OTHERSTART == nStart ||
              (DOCPOS_CURR == nStart &&
                (DOCPOS_END == nEnd || DOCPOS_OTHEREND == nEnd ) ))
                ? fnMoveForward : fnMoveBackward;
}

static sal_uLong lcl_FindSelection( SwFindParas& rParas, SwCursor* pCurrentCursor,
                        SwMoveFn fnMove, SwCursor*& pFndRing,
                        SwPaM& aRegion, FindRanges eFndRngs,
                        bool bInReadOnly, bool& bCancel )
{
    SwDoc* pDoc = pCurrentCursor->GetDoc();
    bool const bDoesUndo = pDoc->GetIDocumentUndoRedo().DoesUndo();
    int nFndRet = 0;
    sal_uLong nFound = 0;
    const bool bSrchBkwrd = fnMove == fnMoveBackward;
    SwPaM *pTmpCursor = pCurrentCursor, *pSaveCursor = pCurrentCursor;

    // only create progress bar for ShellCursor
    bool bIsUnoCursor = dynamic_cast<SwUnoCursor*>(pCurrentCursor) !=  nullptr;
    std::unique_ptr<_PercentHdl> pPHdl;
    sal_uInt16 nCursorCnt = 0;
    if( FND_IN_SEL & eFndRngs )
    {
        while( pCurrentCursor != ( pTmpCursor = pTmpCursor->GetNext() ))
            ++nCursorCnt;
        if( nCursorCnt && !bIsUnoCursor )
            pPHdl.reset(new _PercentHdl( 0, nCursorCnt, pDoc->GetDocShell() ));
    }
    else
        pSaveCursor = static_cast<SwPaM*>(pSaveCursor->GetPrev());

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
            pPHdl.reset(new _PercentHdl( aRegion ));

        // as long as found and not at same position
        while(  *pSttPos <= *pEndPos &&
                0 != ( nFndRet = rParas.Find( pCurrentCursor, fnMove,
                                            &aRegion, bInReadOnly )) &&
                ( !pFndRing ||
                    *pFndRing->GetPoint() != *pCurrentCursor->GetPoint() ||
                    *pFndRing->GetMark() != *pCurrentCursor->GetMark() ))
        {
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

            if( !( eFndRngs & FND_IN_SELALL) )
            {
                bEnd = true;
                break;
            }

            if ((coSrchRplcThreshold == nFound)
                && pDoc->GetIDocumentUndoRedo().DoesUndo()
                && rParas.IsReplaceMode())
            {
                short nRet = pCurrentCursor->MaxReplaceArived();
                if( RET_YES == nRet )
                {
                    pDoc->GetIDocumentUndoRedo().DelAllUndoObj();
                    pDoc->GetIDocumentUndoRedo().DoUndo(false);
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

        if( bEnd || !( eFndRngs & ( FND_IN_SELALL | FND_IN_SEL )) )
            break;

        pTmpCursor = pTmpCursor->GetNext();
        if( nCursorCnt && pPHdl )
        {
            pPHdl->NextPos( ++pPHdl->nActPos );
        }

    } while( pTmpCursor != pSaveCursor && pTmpCursor->GetNext() != pTmpCursor);

    if( nFound && !pFndRing ) // if no ring should be created
        pFndRing = pCurrentCursor->Create();

    pDoc->GetIDocumentUndoRedo().DoUndo(bDoesUndo);
    return nFound;
}

static bool lcl_MakeSelFwrd( const SwNode& rSttNd, const SwNode& rEndNd,
                        SwPaM& rPam, bool bFirst )
{
    if( rSttNd.GetIndex() + 1 == rEndNd.GetIndex() )
        return false;

    SwNodes& rNds = rPam.GetDoc()->GetNodes();
    rPam.DeleteMark();
    SwContentNode* pCNd;
    if( !bFirst )
    {
        rPam.GetPoint()->nNode = rSttNd;
        pCNd = rNds.GoNext( &rPam.GetPoint()->nNode );
        if( !pCNd )
            return false;
        pCNd->MakeStartIndex( &rPam.GetPoint()->nContent );
    }
    else if( rSttNd.GetIndex() > rPam.GetPoint()->nNode.GetIndex() ||
             rPam.GetPoint()->nNode.GetIndex() >= rEndNd.GetIndex() )
        // not in this section
        return false;

    rPam.SetMark();
    rPam.GetPoint()->nNode = rEndNd;
    pCNd = SwNodes::GoPrevious( &rPam.GetPoint()->nNode );
    if( !pCNd )
        return false;
    pCNd->MakeEndIndex( &rPam.GetPoint()->nContent );

    return *rPam.GetMark() < *rPam.GetPoint();
}

static bool lcl_MakeSelBkwrd( const SwNode& rSttNd, const SwNode& rEndNd,
                        SwPaM& rPam, bool bFirst )
{
    if( rEndNd.GetIndex() + 1 == rSttNd.GetIndex() )
        return false;

    SwNodes& rNds = rPam.GetDoc()->GetNodes();
    rPam.DeleteMark();
    SwContentNode* pCNd;
    if( !bFirst )
    {
        rPam.GetPoint()->nNode = rSttNd;
        pCNd = SwNodes::GoPrevious( &rPam.GetPoint()->nNode );
        if( !pCNd )
            return false;
        pCNd->MakeEndIndex( &rPam.GetPoint()->nContent );
    }
    else if( rEndNd.GetIndex() > rPam.GetPoint()->nNode.GetIndex() ||
             rPam.GetPoint()->nNode.GetIndex() >= rSttNd.GetIndex() )
        return false;       // not in this section

    rPam.SetMark();
    rPam.GetPoint()->nNode = rEndNd;
    pCNd = rNds.GoNext( &rPam.GetPoint()->nNode );
    if( !pCNd )
        return false;
    pCNd->MakeStartIndex( &rPam.GetPoint()->nContent );

    return *rPam.GetPoint() < *rPam.GetMark();
}

// this method "searches" for all use cases because in SwFindParas is always the
// correct parameters and respective search method
sal_uLong SwCursor::FindAll( SwFindParas& rParas,
                            SwDocPositions nStart, SwDocPositions nEnde,
                            FindRanges eFndRngs, bool& bCancel )
{
    bCancel = false;
    SwCursorSaveState aSaveState( *this );

    // create region without adding it to the ring
    SwPaM aRegion( *GetPoint() );
    SwMoveFn fnMove = MakeFindRange( nStart, nEnde, &aRegion );

    sal_uLong nFound = 0;
    const bool bMvBkwrd = fnMove == fnMoveBackward;
    bool bInReadOnly = IsReadOnlyAvailable();

    SwCursor* pFndRing = nullptr;
    SwNodes& rNds = GetDoc()->GetNodes();

    // search in sections?
    if( FND_IN_SEL & eFndRngs )
    {
        // if string was not found in region then get all sections (cursors
        // stays unchanged)
        if( 0 == ( nFound = lcl_FindSelection( rParas, this, fnMove,
                                                pFndRing, aRegion, eFndRngs,
                                                bInReadOnly, bCancel ) ))
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
    else if( FND_IN_OTHER & eFndRngs )
    {
        // put cursor as copy of current into ring
        // chaining points always to first created, so forward
        std::unique_ptr< SwCursor > pSav( Create( this ) ); // save the current cursor

        // if already outside of body text search from this position or start at
        // 1. base section
        if( bMvBkwrd
            ? lcl_MakeSelBkwrd( rNds.GetEndOfExtras(),
                    *rNds.GetEndOfPostIts().StartOfSectionNode(),
                     *this, rNds.GetEndOfExtras().GetIndex() >=
                    GetPoint()->nNode.GetIndex() )
            : lcl_MakeSelFwrd( *rNds.GetEndOfPostIts().StartOfSectionNode(),
                    rNds.GetEndOfExtras(), *this,
                    rNds.GetEndOfExtras().GetIndex() >=
                    GetPoint()->nNode.GetIndex() ))
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
        pSav.release();

        if( !( FND_IN_SELALL & eFndRngs ))
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
    else if( FND_IN_SELALL & eFndRngs )
    {
        ::std::unique_ptr< SwCursor> pSav( Create( this ) );  // save the current cursor

        const SwNode* pSttNd = ( FND_IN_BODYONLY & eFndRngs )
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
        pSav.release();
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
        const bool bMarkPos = HasMark() && !eFndRngs;

        if( 0 != (nFound = rParas.Find( this, fnMove,
                                        &aRegion, bInReadOnly ) ? 1 : 0)
            && bMarkPos )
            *GetMark() = aMarkPos;
    }

    if( nFound && SwCursor::IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE ) )
        nFound = 0;
    return nFound;
}

void SwCursor::FillFindPos( SwDocPositions ePos, SwPosition& rPos ) const
{
    bool bIsStart = true;
    SwContentNode* pCNd = nullptr;
    SwNodes& rNds = GetDoc()->GetNodes();

    switch( ePos )
    {
    case DOCPOS_START:
        rPos.nNode = *rNds.GetEndOfContent().StartOfSectionNode();
        pCNd = rNds.GoNext( &rPos.nNode );
        break;
    case DOCPOS_END:
        rPos.nNode = rNds.GetEndOfContent();
        pCNd = SwNodes::GoPrevious( &rPos.nNode );
        bIsStart = false;
        break;
    case DOCPOS_OTHERSTART:
        rPos.nNode = *rNds[ sal_uLong(0) ];
        pCNd = rNds.GoNext( &rPos.nNode );
        break;
    case DOCPOS_OTHEREND:
        rPos.nNode = *rNds.GetEndOfContent().StartOfSectionNode();
        pCNd = SwNodes::GoPrevious( &rPos.nNode );
        bIsStart = false;
        break;
    default:
        rPos = *GetPoint();
    }

    if( pCNd )
    {
        rPos.nContent.Assign( pCNd, bIsStart ? 0 : pCNd->Len() );
    }
}

short SwCursor::MaxReplaceArived()
{
    return RET_YES;
}

bool SwCursor::IsStartWord( sal_Int16 nWordType ) const
{
    return IsStartWordWT( nWordType );
}

bool SwCursor::IsEndWord( sal_Int16 nWordType ) const
{
    return IsEndWordWT( nWordType );
}

bool SwCursor::IsInWord( sal_Int16 nWordType ) const
{
    return IsInWordWT( nWordType );
}

bool SwCursor::GoStartWord()
{
    return GoStartWordWT( WordType::ANYWORD_IGNOREWHITESPACES );
}

bool SwCursor::GoEndWord()
{
    return GoEndWordWT( WordType::ANYWORD_IGNOREWHITESPACES );
}

bool SwCursor::GoNextWord()
{
    return GoNextWordWT( WordType::ANYWORD_IGNOREWHITESPACES );
}

bool SwCursor::GoPrevWord()
{
    return GoPrevWordWT( WordType::ANYWORD_IGNOREWHITESPACES );
}

bool SwCursor::SelectWord( SwViewShell* pViewShell, const Point* pPt )
{
    return SelectWordWT( pViewShell, WordType::ANYWORD_IGNOREWHITESPACES, pPt );
}

bool SwCursor::IsStartWordWT( sal_Int16 nWordType ) const
{
    bool bRet = false;
    const SwTextNode* pTextNd = GetNode().GetTextNode();
    if( pTextNd && g_pBreakIt->GetBreakIter().is() )
    {
        const sal_Int32 nPtPos = GetPoint()->nContent.GetIndex();
        bRet = g_pBreakIt->GetBreakIter()->isBeginWord(
                            pTextNd->GetText(), nPtPos,
                            g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos )),
                            nWordType );
    }
    return bRet;
}

bool SwCursor::IsEndWordWT( sal_Int16 nWordType ) const
{
    bool bRet = false;
    const SwTextNode* pTextNd = GetNode().GetTextNode();
    if( pTextNd && g_pBreakIt->GetBreakIter().is() )
    {
        const sal_Int32 nPtPos = GetPoint()->nContent.GetIndex();
        bRet = g_pBreakIt->GetBreakIter()->isEndWord(
                            pTextNd->GetText(), nPtPos,
                            g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos ) ),
                            nWordType );

    }
    return bRet;
}

bool SwCursor::IsInWordWT( sal_Int16 nWordType ) const
{
    bool bRet = false;
    const SwTextNode* pTextNd = GetNode().GetTextNode();
    if( pTextNd && g_pBreakIt->GetBreakIter().is() )
    {
        const sal_Int32 nPtPos = GetPoint()->nContent.GetIndex();
        Boundary aBoundary = g_pBreakIt->GetBreakIter()->getWordBoundary(
                            pTextNd->GetText(), nPtPos,
                            g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos ) ),
                            nWordType,
                            true );

        bRet = aBoundary.startPos != aBoundary.endPos &&
                aBoundary.startPos <= nPtPos &&
                    nPtPos <= aBoundary.endPos;
        if(bRet)
        {
            const CharClass& rCC = GetAppCharClass();
            bRet = rCC.isLetterNumeric( pTextNd->GetText(), aBoundary.startPos );
        }
    }
    return bRet;
}

bool SwCursor::IsStartEndSentence( bool bEnd ) const
{
    bool bRet = bEnd ?
                    GetContentNode() && GetPoint()->nContent == GetContentNode()->Len() :
                    GetPoint()->nContent.GetIndex() == 0;

    if( !bRet )
    {
        SwCursor aCursor(*GetPoint(), nullptr, false);
        SwPosition aOrigPos = *aCursor.GetPoint();
        aCursor.GoSentence( bEnd ? SwCursor::END_SENT : SwCursor::START_SENT );
        bRet = aOrigPos == *aCursor.GetPoint();
    }
    return bRet;
}

bool SwCursor::GoStartWordWT( sal_Int16 nWordType )
{
    bool bRet = false;
    const SwTextNode* pTextNd = GetNode().GetTextNode();
    if( pTextNd && g_pBreakIt->GetBreakIter().is() )
    {
        SwCursorSaveState aSave( *this );
        sal_Int32 nPtPos = GetPoint()->nContent.GetIndex();
        nPtPos = g_pBreakIt->GetBreakIter()->getWordBoundary(
                            pTextNd->GetText(), nPtPos,
                            g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos ) ),
                            nWordType,
                            false ).startPos;

        if (nPtPos < pTextNd->GetText().getLength() && nPtPos >= 0)
        {
            GetPoint()->nContent = nPtPos;
            if( !IsSelOvr() )
                bRet = true;
        }
    }
    return bRet;
}

bool SwCursor::GoEndWordWT( sal_Int16 nWordType )
{
    bool bRet = false;
    const SwTextNode* pTextNd = GetNode().GetTextNode();
    if( pTextNd && g_pBreakIt->GetBreakIter().is() )
    {
        SwCursorSaveState aSave( *this );
        sal_Int32 nPtPos = GetPoint()->nContent.GetIndex();
        nPtPos = g_pBreakIt->GetBreakIter()->getWordBoundary(
                            pTextNd->GetText(), nPtPos,
                            g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos ) ),
                            nWordType,
                            true ).endPos;

        if (nPtPos <= pTextNd->GetText().getLength() && nPtPos >= 0 &&
            GetPoint()->nContent.GetIndex() != nPtPos )
        {
            GetPoint()->nContent = nPtPos;
            if( !IsSelOvr() )
                bRet = true;
        }
    }
    return bRet;
}

bool SwCursor::GoNextWordWT( sal_Int16 nWordType )
{
    bool bRet = false;
    const SwTextNode* pTextNd = GetNode().GetTextNode();
    if( pTextNd && g_pBreakIt->GetBreakIter().is() )
    {
        SwCursorSaveState aSave( *this );
        sal_Int32 nPtPos = GetPoint()->nContent.GetIndex();

        nPtPos = g_pBreakIt->GetBreakIter()->nextWord(
                                pTextNd->GetText(), nPtPos,
            g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos, 1 ) ),
                    nWordType ).startPos;

        if (nPtPos <= pTextNd->GetText().getLength() && nPtPos >= 0)
        {
            GetPoint()->nContent = nPtPos;
            if( !IsSelOvr() )
                bRet = true;
        }
    }
    return bRet;
}

bool SwCursor::GoPrevWordWT( sal_Int16 nWordType )
{
    bool bRet = false;
    const SwTextNode* pTextNd = GetNode().GetTextNode();
    if( pTextNd && g_pBreakIt->GetBreakIter().is() )
    {
        SwCursorSaveState aSave( *this );
        sal_Int32 nPtPos = GetPoint()->nContent.GetIndex();
        const sal_Int32 nPtStart = nPtPos;

        if( nPtPos )
            --nPtPos;
        nPtPos = g_pBreakIt->GetBreakIter()->previousWord(
                                pTextNd->GetText(), nPtStart,
            g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos, 1 ) ),
                    nWordType ).startPos;

        if (nPtPos < pTextNd->GetText().getLength() && nPtPos >= 0)
        {
            GetPoint()->nContent = nPtPos;
            if( !IsSelOvr() )
                bRet = true;
        }
    }
    return bRet;
}

bool SwCursor::SelectWordWT( SwViewShell* pViewShell, sal_Int16 nWordType, const Point* pPt )
{
    SwCursorSaveState aSave( *this );

    bool bRet = false;
    DeleteMark();
    const SwRootFrame* pLayout = pViewShell->GetLayout();
    if( pPt && nullptr != pLayout )
    {
        // set the cursor to the layout position
        Point aPt( *pPt );
        pLayout->GetCursorOfst( GetPoint(), aPt );
    }

    const SwTextNode* pTextNd = GetNode().GetTextNode();
    if( pTextNd && g_pBreakIt->GetBreakIter().is() )
    {
        // Should we select the whole fieldmark?
        const IDocumentMarkAccess* pMarksAccess = GetDoc()->getIDocumentMarkAccess( );
        sw::mark::IMark* pMark = GetPoint() ? pMarksAccess->getFieldmarkFor( *GetPoint( ) ) : nullptr;
        if ( pMark )
        {
            const SwPosition rStart = pMark->GetMarkStart();
            GetPoint()->nNode = rStart.nNode;
            GetPoint()->nContent = rStart.nContent;
            ++GetPoint()->nContent; // Don't select the start delimiter

            const SwPosition rEnd = pMark->GetMarkEnd();

            if ( rStart != rEnd )
            {
                SetMark();
                GetMark()->nNode = rEnd.nNode;
                GetMark()->nContent = rEnd.nContent;
                --GetMark()->nContent; //Don't select the end delimiter
            }
            bRet = true;
        }
        else
        {
            bool bForward = true;
            sal_Int32 nPtPos = GetPoint()->nContent.GetIndex();
            Boundary aBndry( g_pBreakIt->GetBreakIter()->getWordBoundary(
                                pTextNd->GetText(), nPtPos,
                                g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos ) ),
                                nWordType,
                                bForward ));

            if (comphelper::LibreOfficeKit::isActive() && aBndry.startPos == aBndry.endPos && nPtPos > 0)
            {
                // nPtPos is the end of the paragraph, select the last word then.
                --nPtPos;
                aBndry = Boundary( g_pBreakIt->GetBreakIter()->getWordBoundary(
                                    pTextNd->GetText(), nPtPos,
                                    g_pBreakIt->GetLocale( pTextNd->GetLang( nPtPos ) ),
                                    nWordType,
                                    bForward ));
            }

            if( aBndry.startPos != aBndry.endPos )
            {
                GetPoint()->nContent = aBndry.endPos;
                if( !IsSelOvr() )
                {
                    SetMark();
                    GetMark()->nContent = aBndry.startPos;
                    if (sw::mark::IMark* pAnnotationMark = pMarksAccess->getAnnotationMarkFor(*GetPoint()))
                    {
                        // An annotation mark covers the selected word. Check
                        // if it covers only the word: in that case we select
                        // the comment anchor as well.
                        bool bStartMatch = GetMark()->nNode == pAnnotationMark->GetMarkStart().nNode &&
                            GetMark()->nContent == pAnnotationMark->GetMarkStart().nContent;
                        bool bEndMatch = GetPoint()->nNode == pAnnotationMark->GetMarkEnd().nNode &&
                            GetPoint()->nContent.GetIndex() + 1 == pAnnotationMark->GetMarkEnd().nContent.GetIndex();
                        if (bStartMatch && bEndMatch)
                            ++GetPoint()->nContent;
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
        const SwDoc& rDoc = *pTextNd->GetDoc();
        const bool bShowChg = IDocumentRedlineAccess::IsShowChanges( rDoc.getIDocumentRedlineAccess().GetRedlineMode() );
        if ( bShowChg )
        {
            sal_uInt16 nAct = rDoc.getIDocumentRedlineAccess().GetRedlinePos( *pTextNd, USHRT_MAX );
            for ( ; nAct < rDoc.getIDocumentRedlineAccess().GetRedlineTable().size(); nAct++ )
            {
                const SwRangeRedline* pRed = rDoc.getIDocumentRedlineAccess().GetRedlineTable()[ nAct ];
                if ( pRed->Start()->nNode > pTextNd->GetIndex() )
                    break;

                if( nsRedlineType_t::REDLINE_DELETE == pRed->GetType() )
                {
                    sal_Int32 nStart, nEnd;
                    pRed->CalcStartEnd( pTextNd->GetIndex(), nStart, nEnd );

                    while ( nStart < nEnd && nStart < sNodeText.getLength() )
                        sNodeText = sNodeText.replaceAt( nStart++, 1, OUString(CH_TXTATR_INWORD) );
                }
            }
        }
        aRes = sNodeText;
    }
    return aRes;
}

bool SwCursor::GoSentence( SentenceMoveType eMoveType )
{
    bool bRet = false;
    const SwTextNode* pTextNd = GetNode().GetTextNode();
    if( pTextNd && g_pBreakIt->GetBreakIter().is() )
    {
        OUString sNodeText( lcl_MaskDeletedRedlines( pTextNd ) );

        SwCursorSaveState aSave( *this );
        sal_Int32 nPtPos = GetPoint()->nContent.GetIndex();
        switch ( eMoveType )
        {
        case START_SENT: /* when modifying: see also ExpandToSentenceBorders below! */
            nPtPos = g_pBreakIt->GetBreakIter()->beginOfSentence(
                                    sNodeText,
                                    nPtPos, g_pBreakIt->GetLocale(
                                            pTextNd->GetLang( nPtPos ) ));
            break;
        case END_SENT: /* when modifying: see also ExpandToSentenceBorders below! */
            nPtPos = g_pBreakIt->GetBreakIter()->endOfSentence(
                                    sNodeText,
                                    nPtPos, g_pBreakIt->GetLocale(
                                                pTextNd->GetLang( nPtPos ) ));
            break;
        case NEXT_SENT:
            {
                nPtPos = g_pBreakIt->GetBreakIter()->endOfSentence(
                                        sNodeText,
                                        nPtPos, g_pBreakIt->GetLocale(
                                                    pTextNd->GetLang( nPtPos ) ));
                while (nPtPos>=0 && ++nPtPos < sNodeText.getLength()
                       && sNodeText[nPtPos] == ' ' /*isWhiteSpace( aText.GetChar(nPtPos)*/ )
                    ;
                break;
            }
        case PREV_SENT:
            nPtPos = g_pBreakIt->GetBreakIter()->beginOfSentence(
                                    sNodeText,
                                    nPtPos, g_pBreakIt->GetLocale(
                                                pTextNd->GetLang( nPtPos ) ));
            if (nPtPos == 0)
                return false;   // the previous sentence is not in this paragraph
            if (nPtPos > 0)
                nPtPos = g_pBreakIt->GetBreakIter()->beginOfSentence(
                                    sNodeText,
                                    nPtPos - 1, g_pBreakIt->GetLocale(
                                                pTextNd->GetLang( nPtPos ) ));
            break;
        }

        // it is allowed to place the PaM just behind the last
        // character in the text thus <= ...Len
        if (nPtPos <= pTextNd->GetText().getLength() && nPtPos >= 0)
        {
            GetPoint()->nContent = nPtPos;
            if( !IsSelOvr() )
                bRet = true;
        }
    }
    return bRet;
}

bool SwCursor::ExpandToSentenceBorders()
{
    bool bRes = false;
    const SwTextNode* pStartNd = Start()->nNode.GetNode().GetTextNode();
    const SwTextNode* pEndNd   = End()->nNode.GetNode().GetTextNode();
    if (pStartNd && pEndNd && g_pBreakIt->GetBreakIter().is())
    {
        if (!HasMark())
            SetMark();

        OUString sStartText( lcl_MaskDeletedRedlines( pStartNd ) );
        OUString sEndText( pStartNd == pEndNd? sStartText : lcl_MaskDeletedRedlines( pEndNd ) );

        SwCursorSaveState aSave( *this );
        sal_Int32 nStartPos = Start()->nContent.GetIndex();
        sal_Int32 nEndPos   = End()->nContent.GetIndex();

        nStartPos = g_pBreakIt->GetBreakIter()->beginOfSentence(
                                sStartText, nStartPos,
                                g_pBreakIt->GetLocale( pStartNd->GetLang( nStartPos ) ) );
        nEndPos   = g_pBreakIt->GetBreakIter()->endOfSentence(
                                sEndText, nEndPos,
                                g_pBreakIt->GetLocale( pEndNd->GetLang( nEndPos ) ) );

        // it is allowed to place the PaM just behind the last
        // character in the text thus <= ...Len
        bool bChanged = false;
        if (nStartPos <= pStartNd->GetText().getLength() && nStartPos >= 0)
        {
            GetMark()->nContent = nStartPos;
            bChanged = true;
        }
        if (nEndPos <= pEndNd->GetText().getLength() && nEndPos >= 0)
        {
            GetPoint()->nContent = nEndPos;
            bChanged = true;
        }
        if (bChanged && !IsSelOvr())
            bRes = true;
    }
    return bRes;
}

bool SwTableCursor::LeftRight( bool bLeft, sal_uInt16 nCnt, sal_uInt16 /*nMode*/,
    bool /*bVisualAllowed*/, bool /*bSkipHidden*/, bool /*bInsertCursor*/ )
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
    SwNode& rNode = GetPoint()->nNode.GetNode();

    if( rNode.IsTextNode() )
    {
        const SwTextNode& rTNd = *rNode.GetTextNode();
        SwIndex& rIdx = GetPoint()->nContent;
        sal_Int32 nPos = rIdx.GetIndex();

        const SvtCTLOptions& rCTLOptions = SW_MOD()->GetCTLOptions();
        if ( bVisualAllowed && rCTLOptions.IsCTLFontEnabled() &&
             SvtCTLOptions::MOVEMENT_VISUAL ==
             rCTLOptions.GetCTLCursorMovement() )
        {
            // for visual cursor travelling (used in bidi layout)
            // we first have to convert the logic to a visual position
            Point aPt;
            pSttFrame = rTNd.getLayoutFrame( GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, GetPoint() );
            if( pSttFrame )
            {
                sal_uInt8 nCursorLevel = GetCursorBidiLevel();
                bool bForward = ! io_rbLeft;
                const_cast<SwTextFrame*>(static_cast<const SwTextFrame*>(pSttFrame))->PrepareVisualMove( nPos, nCursorLevel,
                                                         bForward, bInsertCursor );
                rIdx = nPos;
                SetCursorBidiLevel( nCursorLevel );
                io_rbLeft = ! bForward;
            }
        }
        else
        {
            const SwScriptInfo* pSI = SwScriptInfo::GetScriptInfo( rTNd );
            if ( pSI )
            {
                const sal_Int32 nMoveOverPos = io_rbLeft ?
                                               ( nPos ? nPos - 1 : 0 ) :
                                                nPos;
                SetCursorBidiLevel( pSI->DirType( nMoveOverPos ) );
            }
        }
    }
    return pSttFrame;
}

bool SwCursor::LeftRight( bool bLeft, sal_uInt16 nCnt, sal_uInt16 nMode,
                          bool bVisualAllowed,bool bSkipHidden, bool bInsertCursor )
{
    // calculate cursor bidi level
    SwNode& rNode = GetPoint()->nNode.GetNode();
    const SwContentFrame* pSttFrame = // may side-effect bLeft!
        DoSetBidiLevelLeftRight(bLeft, bVisualAllowed, bInsertCursor);

    // can the cursor be moved n times?
    SwCursorSaveState aSave( *this );
    SwMoveFn fnMove = bLeft ? fnMoveBackward : fnMoveForward;

    SwGoInDoc fnGo;
    if ( bSkipHidden )
        fnGo = CRSR_SKIP_CELLS == nMode ? fnGoContentCellsSkipHidden : fnGoContentSkipHidden;
    else
        fnGo = CRSR_SKIP_CELLS == nMode ? fnGoContentCells : fnGoContent;

    while( nCnt )
    {
        SwNodeIndex aOldNodeIdx( GetPoint()->nNode );

        if ( !Move( fnMove, fnGo ) )
            break;

        // If we were located inside a covered cell but our position has been
        // corrected, we check if the last move has moved the cursor to a
        // different table cell. In this case we set the cursor to the stored
        // covered position and redo the move:
        if (m_nRowSpanOffset)
        {
            const SwNode* pOldTabBoxSttNode = aOldNodeIdx.GetNode().FindTableBoxStartNode();
            const SwTableNode* pOldTabSttNode = pOldTabBoxSttNode ? pOldTabBoxSttNode->FindTableNode() : nullptr;
            const SwNode* pNewTabBoxSttNode = GetPoint()->nNode.GetNode().FindTableBoxStartNode();
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
                        static_cast<sal_uInt16>(pTableBox->getRowSpan() + m_nRowSpanOffset));
                    SwNodeIndex& rPtIdx = GetPoint()->nNode;
                    SwNodeIndex aNewIdx( *pTableBox->GetSttNd() );
                    rPtIdx = aNewIdx;

                    GetDoc()->GetNodes().GoNextSection( &rPtIdx, false, false );
                    SwContentNode* pContentNode = GetContentNode();
                    if ( pContentNode )
                    {
                        GetPoint()->nContent.Assign( pContentNode, bLeft ? pContentNode->Len() : 0 );

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
        const SwNode* pTableBoxStartNode = GetPoint()->nNode.GetNode().FindTableBoxStartNode();
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
                 SwNodeIndex& rPtIdx = GetPoint()->nNode;
                SwNodeIndex aNewIdx( *pTableBox->GetSttNd() );
                rPtIdx = aNewIdx;

                GetDoc()->GetNodes().GoNextSection( &rPtIdx, false, false );
                SwContentNode* pContentNode = GetContentNode();
                if ( pContentNode )
                {
                    GetPoint()->nContent.Assign( pContentNode, bLeft ? pContentNode->Len() : 0 );
                }
            }
        }
        --nCnt;
    }

    // here come some special rules for visual cursor travelling
    if ( pSttFrame )
    {
        SwNode& rTmpNode = GetPoint()->nNode.GetNode();
        if ( &rTmpNode != &rNode && rTmpNode.IsTextNode() )
        {
            Point aPt;
            const SwContentFrame* pEndFrame = rTmpNode.GetTextNode()->getLayoutFrame( GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, GetPoint() );
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
            !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                       nsSwCursorSelOverFlags::SELOVER_CHANGEPOS );
}

// calculate cursor bidi level: extracted from UpDown()
void SwCursor::DoSetBidiLevelUpDown()
{
    SwNode& rNode = GetPoint()->nNode.GetNode();
    if ( rNode.IsTextNode() )
    {
        const SwScriptInfo* pSI =
            SwScriptInfo::GetScriptInfo( *rNode.GetTextNode() );
        if ( pSI )
        {
            SwIndex& rIdx = GetPoint()->nContent;
            const sal_Int32 nPos = rIdx.GetIndex();

            if (nPos && nPos < rNode.GetTextNode()->GetText().getLength())
            {
                const sal_uInt8 nCurrLevel = pSI->DirType( nPos );
                const sal_uInt8 nPrevLevel = pSI->DirType( nPos - 1 );

                if ( nCurrLevel % 2 != nPrevLevel % 2 )
                {
                    // set cursor level to the lower of the two levels
                    SetCursorBidiLevel( std::min( nCurrLevel, nPrevLevel ) );
                }
                else
                    SetCursorBidiLevel( nCurrLevel );
            }
        }
    }
}

bool SwCursor::UpDown( bool bUp, sal_uInt16 nCnt,
                            Point* pPt, long nUpDownX )
{
    SwTableCursor* pTableCursor = dynamic_cast<SwTableCursor*>(this);
    bool bAdjustTableCursor = false;

    // If the point/mark of the table cursor in the same box then set cursor to
    // beginning of the box
    if( pTableCursor && GetNode().StartOfSectionNode() ==
                    GetNode( false ).StartOfSectionNode() )
    {
        if ( End() != GetPoint() )
            Exchange();
        bAdjustTableCursor = true;
    }

    bool bRet = false;
    Point aPt;
    if( pPt )
        aPt = *pPt;
    SwContentFrame* pFrame = GetContentNode()->getLayoutFrame( GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, GetPoint() );

    if( pFrame )
    {
        SwCursorSaveState aSave( *this );

        if( !pPt )
        {
            SwRect aTmpRect;
            pFrame->GetCharRect( aTmpRect, *GetPoint() );
            aPt = aTmpRect.Pos();

            nUpDownX = pFrame->IsVertical() ?
                aPt.getY() - pFrame->Frame().Top() :
                aPt.getX() - pFrame->Frame().Left();
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
            const SwStartNode* pTableNd = GetNode().FindTableBoxStartNode();
            OSL_ENSURE( pTableNd, "pTableCursor without SwTableNode?" );

            if ( pTableNd ) // safety first
            {
                const SwNode* pEndNd = pTableNd->EndOfSectionNode();
                GetPoint()->nNode = *pEndNd;
                pTableCursor->Move( fnMoveBackward, fnGoNode );
                   pFrame = GetContentNode()->getLayoutFrame( GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, GetPoint() );
            }
        }

        while( nCnt &&
               (bUp ? pFrame->UnitUp( this, nUpDownX, bInReadOnly )
                    : pFrame->UnitDown( this, nUpDownX, bInReadOnly ) ) &&
                CheckNodesRange( aOldPos.nNode, GetPoint()->nNode, bChkRange ))
        {
               pFrame = GetContentNode()->getLayoutFrame( GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, GetPoint() );
            --nCnt;
        }

        // iterate over whole number of items?
        if( !nCnt && !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ) )
        {
            if( !pTableCursor )
            {
                // try to position the cursor at half of the char-rect's height
                pFrame = GetContentNode()->getLayoutFrame( GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, GetPoint() );
                SwCursorMoveState eTmpState( MV_UPDOWN );
                eTmpState.m_bSetInReadOnly = bInReadOnly;
                SwRect aTmpRect;
                pFrame->GetCharRect( aTmpRect, *GetPoint(), &eTmpState );
                if ( pFrame->IsVertical() )
                {
                    aPt.setX(aTmpRect.Center().getX());
                    pFrame->Calc(pFrame->getRootFrame()->GetCurrShell()->GetOut());
                    aPt.setY(pFrame->Frame().Top() + nUpDownX);
                }
                else
                {
                    aPt.setY(aTmpRect.Center().getY());
                    pFrame->Calc(pFrame->getRootFrame()->GetCurrShell()->GetOut());
                    aPt.setX(pFrame->Frame().Left() + nUpDownX);
                }
                pFrame->GetCursorOfst( GetPoint(), aPt, &eTmpState );
            }
            bRet = !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE | nsSwCursorSelOverFlags::SELOVER_CHANGEPOS );
        }
        else
            *GetPoint() = aOldPos;

        DoSetBidiLevelUpDown(); // calculate cursor bidi level
    }
    return bRet;
}

bool SwCursor::LeftRightMargin( bool bLeft, bool bAPI )
{
    Point aPt;
    SwContentFrame * pFrame = GetContentNode()->getLayoutFrame( GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, GetPoint() );

    // calculate cursor bidi level
    if ( pFrame )
        SetCursorBidiLevel( pFrame->IsRightToLeft() ? 1 : 0 );

    SwCursorSaveState aSave( *this );
    return pFrame
           && (bLeft ? pFrame->LeftMargin( this ) : pFrame->RightMargin( this, bAPI ) )
           && !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE | nsSwCursorSelOverFlags::SELOVER_CHANGEPOS );
}

bool SwCursor::IsAtLeftRightMargin( bool bLeft, bool bAPI ) const
{
    bool bRet = false;
    Point aPt;
    SwContentFrame * pFrame = GetContentNode()->getLayoutFrame( GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, GetPoint() );
    if( pFrame )
    {
        SwPaM aPam( *GetPoint() );
        if( !bLeft && aPam.GetPoint()->nContent.GetIndex() )
            --aPam.GetPoint()->nContent;
        bRet = (bLeft ? pFrame->LeftMargin( &aPam )
                      : pFrame->RightMargin( &aPam, bAPI ))
                && *aPam.GetPoint() == *GetPoint();
    }
    return bRet;
}

bool SwCursor::SttEndDoc( bool bStt )
{
    SwCursorSaveState aSave( *this );
    // Never jump over section boundaries during selection!
    // Can the cursor still moved on?
    SwMoveFn fnMove = bStt ? fnMoveBackward : fnMoveForward;
    bool bRet = (!HasMark() || !IsNoContent() ) &&
                    Move( fnMove, fnGoDoc ) &&
                    !IsInProtectTable( true ) &&
                    !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                               nsSwCursorSelOverFlags::SELOVER_CHANGEPOS |
                               nsSwCursorSelOverFlags::SELOVER_ENABLEREVDIREKTION );
    return bRet;
}

bool SwCursor::GoPrevNextCell( bool bNext, sal_uInt16 nCnt )
{
    const SwTableNode* pTableNd = GetPoint()->nNode.GetNode().FindTableNode();
    if( !pTableNd )
        return false;

    // If there is another EndNode in front of the cell's StartNode then there
    // exists a previous cell
    SwCursorSaveState aSave( *this );
    SwNodeIndex& rPtIdx = GetPoint()->nNode;

    while( nCnt-- )
    {
        const SwNode* pTableBoxStartNode = rPtIdx.GetNode().FindTableBoxStartNode();
        const SwTableBox* pTableBox = pTableBoxStartNode->GetTableBox();

        // Check if we have to move the cursor to a covered cell before
        // proceeding:
        if (m_nRowSpanOffset)
        {
            if ( pTableBox && pTableBox->getRowSpan() > 1 )
            {
                pTableBox = & pTableBox->FindEndOfRowSpan( pTableNd->GetTable(),
                    static_cast<sal_uInt16>(pTableBox->getRowSpan() + m_nRowSpanOffset));
                SwNodeIndex aNewIdx( *pTableBox->GetSttNd() );
                rPtIdx = aNewIdx;
                pTableBoxStartNode = rPtIdx.GetNode().FindTableBoxStartNode();
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

        rPtIdx = bNext ? aCellIdx : SwNodeIndex(*aCellIdx.GetNode().StartOfSectionNode());

        pTableBoxStartNode = rPtIdx.GetNode().FindTableBoxStartNode();
        pTableBox = pTableBoxStartNode->GetTableBox();
        if ( pTableBox && pTableBox->getRowSpan() < 1 )
        {
            m_nRowSpanOffset = pTableBox->getRowSpan();
            // move cursor to non-covered cell:
            pTableBox = & pTableBox->FindStartOfRowSpan( pTableNd->GetTable() );
            SwNodeIndex aNewIdx( *pTableBox->GetSttNd() );
            rPtIdx = aNewIdx;
        }
    }

    ++rPtIdx;
    if( !rPtIdx.GetNode().IsContentNode() )
        GetDoc()->GetNodes().GoNextSection( &rPtIdx, true, false );
    GetPoint()->nContent.Assign( GetContentNode(), 0 );

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
        SwTable* pTmpTable = SwTable::FindTable( GetDoc()->FindTableFormatByName( rName ) );
        if( pTmpTable )
        {
            // a table in a normal nodes array
            SwCursorSaveState aSave( *this );
            GetPoint()->nNode = *pTmpTable->GetTabSortBoxes()[ 0 ]->
                                GetSttNd()->FindTableNode();
            Move( fnMoveForward, fnGoContent );
            bRet = !IsSelOvr();
        }
    }
    return bRet;
}

bool SwCursor::GotoTableBox( const OUString& rName )
{
    bool bRet = false;
    const SwTableNode* pTableNd = GetPoint()->nNode.GetNode().FindTableNode();
    if( pTableNd )
    {
        // retrieve box by name
        const SwTableBox* pTableBox = pTableNd->GetTable().GetTableBox( rName );
        if( pTableBox && pTableBox->GetSttNd() &&
            ( !pTableBox->GetFrameFormat()->GetProtect().IsContentProtected() ||
              IsReadOnlyAvailable() ) )
        {
            SwCursorSaveState aSave( *this );
            GetPoint()->nNode = *pTableBox->GetSttNd();
            Move( fnMoveForward, fnGoContent );
            bRet = !IsSelOvr();
        }
    }
    return bRet;
}

bool SwCursor::MovePara(SwWhichPara fnWhichPara, SwPosPara fnPosPara )
{
    // for optimization test something before
    const SwNode* pNd = &GetPoint()->nNode.GetNode();
    bool bShortCut = false;
    if ( fnWhichPara == fnParaCurr )
    {
        // #i41048#
        // If fnWhichPara == fnParaCurr then (*fnWhichPara)( *this, fnPosPara )
        // can already move the cursor to a different text node. In this case
        // we better check if IsSelOvr().
        const SwContentNode* pContentNd = pNd->GetContentNode();
        if ( pContentNd )
        {
            const sal_Int32 nSttEnd = fnPosPara == fnMoveForward ? 0 : pContentNd->Len();
            if ( GetPoint()->nContent.GetIndex() != nSttEnd )
                bShortCut = true;
        }
    }
    else
    {
        if ( pNd->IsTextNode() &&
             pNd->GetNodes()[ pNd->GetIndex() +
                    (fnWhichPara == fnParaNext ? 1 : -1 ) ]->IsTextNode() )
            bShortCut = true;
    }

    if ( bShortCut )
        return (*fnWhichPara)( *this, fnPosPara );

    // else we must use the SaveStructure, because the next/prev is not
    // a same node type.
    SwCursorSaveState aSave( *this );
    return (*fnWhichPara)( *this, fnPosPara ) &&
            !IsInProtectTable( true ) &&
            !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                       nsSwCursorSelOverFlags::SELOVER_CHANGEPOS );
}

bool SwCursor::MoveSection( SwWhichSection fnWhichSect,
                                SwPosSection fnPosSect)
{
    SwCursorSaveState aSave( *this );
    return (*fnWhichSect)( *this, fnPosSect ) &&
            !IsInProtectTable( true ) &&
            !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                       nsSwCursorSelOverFlags::SELOVER_CHANGEPOS );
}

void SwCursor::RestoreSavePos()
{
    // This method is not supposed to be used in cases when nodes may be
    // deleted; detect such cases, but do not crash (example: fdo#40831).
    sal_uLong uNodeCount = GetPoint()->nNode.GetNodes().Count();
    OSL_ENSURE(!m_pSavePos || m_pSavePos->nNode < uNodeCount,
        "SwCursor::RestoreSavePos: invalid node: "
        "probably something was deleted; consider using SwUnoCursor instead");
    if (m_pSavePos && m_pSavePos->nNode < uNodeCount)
    {
        GetPoint()->nNode = m_pSavePos->nNode;

        sal_Int32 nIdx = 0;
        if ( GetContentNode() )
        {
            if (m_pSavePos->nContent <= GetContentNode()->Len())
                nIdx = m_pSavePos->nContent;
            else
            {
                nIdx = GetContentNode()->Len();
                OSL_FAIL("SwCursor::RestoreSavePos: invalid content index");
            }
        }
        GetPoint()->nContent.Assign( GetContentNode(), nIdx );
    }
}

SwTableCursor::SwTableCursor( const SwPosition &rPos, SwPaM* pRing )
    : SwCursor( rPos, pRing, false )
{
    m_bParked = false;
    m_bChanged = false;
    m_nTablePtNd = 0;
    m_nTableMkNd = 0;
    m_nTablePtCnt = 0;
    m_nTableMkCnt = 0;
}

SwTableCursor::~SwTableCursor() {}

static bool
lcl_SeekEntry(const SwSelBoxes& rTmp, SwStartNode const*const pSrch,
        size_t & o_rFndPos)
{
    sal_uLong nIdx = pSrch->GetIndex();

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

SwCursor* SwTableCursor::MakeBoxSels( SwCursor* pAktCursor )
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
        SwNodes& rNds = pAktCursor->GetDoc()->GetNodes();
        const SwStartNode* pSttNd;
        SwPaM* pCur = pAktCursor;
        do {
            size_t nPos;
            bool bDel = false;
            pSttNd = pCur->GetPoint()->nNode.GetNode().FindTableBoxStartNode();
            if( !pCur->HasMark() || !pSttNd ||
                pSttNd != pCur->GetMark()->nNode.GetNode().FindTableBoxStartNode() )
                bDel = true;

            else if( lcl_SeekEntry( aTmp, pSttNd, nPos ))
            {
                SwNodeIndex aIdx( *pSttNd, 1 );
                const SwNode* pNd = &aIdx.GetNode();
                if( !pNd->IsContentNode() )
                    pNd = rNds.GoNextSection( &aIdx, true, false );

                SwPosition* pPos = pCur->GetMark();
                if( pNd != &pPos->nNode.GetNode() )
                    pPos->nNode = *pNd;
                pPos->nContent.Assign( const_cast<SwContentNode*>(static_cast<const SwContentNode*>(pNd)), 0 );

                aIdx.Assign( *pSttNd->EndOfSectionNode(), - 1 );
                if( !( pNd = &aIdx.GetNode())->IsContentNode() )
                    pNd = SwNodes::GoPrevSection( &aIdx, true, false );

                pPos = pCur->GetPoint();
                if (pNd && pNd != &pPos->nNode.GetNode())
                    pPos->nNode = *pNd;
                pPos->nContent.Assign(const_cast<SwContentNode*>(static_cast<const SwContentNode*>(pNd)), pNd ? static_cast<const SwContentNode*>(pNd)->Len() : 0);

                aTmp.erase( aTmp.begin() + nPos );
            }
            else
                bDel = true;

            pCur = pCur->GetNext();
            if( bDel )
            {
                SwPaM* pDel = pCur->GetPrev();

                if( pDel == pAktCursor )
                    pAktCursor->DeleteMark();
                else
                    delete pDel;
            }
        } while ( pAktCursor != pCur );

        for (size_t nPos = 0; nPos < aTmp.size(); ++nPos)
        {
            pSttNd = aTmp[ nPos ]->GetSttNd();

            SwNodeIndex aIdx( *pSttNd, 1 );
            if( &aIdx.GetNodes() != &rNds )
                break;
            SwNode* pNd = &aIdx.GetNode();
            if( !pNd->IsContentNode() )
                pNd = rNds.GoNextSection( &aIdx, true, false );

            SwPaM *const pNew = (!pAktCursor->IsMultiSelection() && !pAktCursor->HasMark())
                ? pAktCursor
                : pAktCursor->Create( pAktCursor );
            pNew->GetPoint()->nNode = *pNd;
            pNew->GetPoint()->nContent.Assign( static_cast<SwContentNode*>(pNd), 0 );
            pNew->SetMark();

            SwPosition* pPos = pNew->GetPoint();
            pPos->nNode.Assign( *pSttNd->EndOfSectionNode(), - 1 );
            if( !( pNd = &pPos->nNode.GetNode())->IsContentNode() )
                pNd = SwNodes::GoPrevSection( &pPos->nNode, true, false );

            pPos->nContent.Assign(static_cast<SwContentNode*>(pNd), pNd ? static_cast<SwContentNode*>(pNd)->Len() : 0);
        }
    }
    return pAktCursor;
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
    const SwNode *pStart = GetContentNode()->FindTableBoxStartNode();
    const SwNode *pEnd = GetContentNode(false)->FindTableBoxStartNode();
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

    m_nTableMkNd = GetMark()->nNode.GetIndex();
    m_nTablePtNd = GetPoint()->nNode.GetIndex();
    m_nTableMkCnt = GetMark()->nContent.GetIndex();
    m_nTablePtCnt = GetPoint()->nContent.GetIndex();
    return true;
}

/// park table cursor on the boxes' start node
void SwTableCursor::ParkCursor()
{
    // de-register index from text node
    SwNode* pNd = &GetPoint()->nNode.GetNode();
    if( !pNd->IsStartNode() )
        pNd = pNd->StartOfSectionNode();
    GetPoint()->nNode = *pNd;
    GetPoint()->nContent.Assign( nullptr, 0 );

    pNd = &GetMark()->nNode.GetNode();
    if( !pNd->IsStartNode() )
        pNd = pNd->StartOfSectionNode();
    GetMark()->nNode = *pNd;
    GetMark()->nContent.Assign( nullptr, 0 );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
