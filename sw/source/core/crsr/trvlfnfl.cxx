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

#include <crsrsh.hxx>
#include <doc.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <ftnfrm.hxx>
#include <swcrsr.hxx>
#include <ndtxt.hxx>
#include <txtfrm.hxx>
#include <txtftn.hxx>
#include <ftnidx.hxx>
#include <viscrs.hxx>
#include "callnk.hxx"
#include <svx/srchdlg.hxx>
#include <wrtsh.hxx>

bool SwCursorShell::CallCursorShellFN( FNCursorShell fnCursor )
{
    SwCallLink aLk( *this ); // watch Cursor-Moves
    bool bRet = (this->*fnCursor)();
    if( bRet )
        UpdateCursor( SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE |
                    SwCursorShell::READONLY );
    return bRet;
}

bool SwCursorShell::CallCursorFN( FNCursor fnCursor )
{
    // for footnote anchor<->text recency
    if (SwWrtShell* pWrtSh = dynamic_cast<SwWrtShell*>(this))
        pWrtSh->addCurrentPosition();

    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursor* pCursor = getShellCursor( true );
    bool bRet = (pCursor->*fnCursor)();
    if( bRet )
        UpdateCursor( SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE |
                    SwCursorShell::READONLY );
    return bRet;
}

bool SwCursor::GotoFootnoteText()
{
    // jump from content to footnote
    bool bRet = false;
    SwTextNode* pTextNd = GetPoint()->GetNode().GetTextNode();

    SwTextAttr *const pFootnote( pTextNd
        ? pTextNd->GetTextAttrForCharAt(
            GetPoint()->GetContentIndex(), RES_TXTATR_FTN)
        : nullptr);
    if (pFootnote)
    {
        SwCursorSaveState aSaveState( *this );
        GetPoint()->Assign( *static_cast<SwTextFootnote*>(pFootnote)->GetStartNode() );

        SwContentNode* pCNd = SwNodes::GoNextSection(
                                            GetPoint(),
                                            true, !IsReadOnlyAvailable() );
        if( pCNd )
        {
            bRet = !IsSelOvr( SwCursorSelOverFlags::CheckNodeSection |
                              SwCursorSelOverFlags::Toggle );
        }
    }
    return bRet;
}

bool SwCursorShell::GotoFootnoteText()
{
    bool bRet = CallCursorFN( &SwCursor::GotoFootnoteText );
    if( !bRet )
    {
        SwTextNode* pTextNd = GetCursor_() ?
                   GetCursor_()->GetPoint()->GetNode().GetTextNode() : nullptr;
        if( pTextNd )
        {
            std::pair<Point, bool> const tmp(GetCursor_()->GetSttPos(), true);
            const SwFrame *pFrame = pTextNd->getLayoutFrame( GetLayout(),
                                                 GetCursor_()->Start(), &tmp);
            const SwFootnoteBossFrame* pFootnoteBoss;
            bool bSkip = pFrame && pFrame->IsInFootnote();
            while( pFrame )
            {
                pFootnoteBoss = pFrame->FindFootnoteBossFrame();
                if (!pFootnoteBoss)
                    break;
                pFrame = pFootnoteBoss->FindFootnoteCont();
                if( pFrame )
                {
                    if( bSkip )
                        bSkip = false;
                    else
                    {
                        const SwContentFrame* pCnt = static_cast<const SwLayoutFrame*>
                                                        (pFrame)->ContainsContent();
                        if( pCnt )
                        {
                            SwTextFrame const*const pTF(
                                    static_cast<const SwTextFrame*>(pCnt));
                            *GetCursor_()->GetPoint() =
                                    pTF->MapViewToModelPos(pTF->GetOffset());
                            UpdateCursor( SwCursorShell::SCROLLWIN |
                                SwCursorShell::CHKRANGE | SwCursorShell::READONLY );
                            bRet = true;
                            break;
                        }
                    }
                }
                if( pFootnoteBoss->GetNext() && !pFootnoteBoss->IsPageFrame() )
                    pFrame = pFootnoteBoss->GetNext();
                else
                    pFrame = pFootnoteBoss->GetUpper();
            }
        }
    }
    return bRet;
}

bool SwCursor::GotoFootnoteAnchor()
{
    // jump from footnote to anchor
    const SwNode* pSttNd = GetPointNode().FindFootnoteStartNode();
    if( pSttNd )
    {
        // search in all footnotes in document for this StartIndex
        const SwFootnoteIdxs& rFootnoteArr = pSttNd->GetDoc().GetFootnoteIdxs();
        for( size_t n = 0; n < rFootnoteArr.size(); ++n )
        {
            const SwTextFootnote* pTextFootnote = rFootnoteArr[ n ];
            if( nullptr != pTextFootnote->GetStartNode() &&
                pSttNd == &pTextFootnote->GetStartNode()->GetNode() )
            {
                SwCursorSaveState aSaveState( *this );

                SwTextNode& rTNd = const_cast<SwTextNode&>(pTextFootnote->GetTextNode());
                GetPoint()->Assign( rTNd, pTextFootnote->GetStart() );

                return !IsSelOvr( SwCursorSelOverFlags::CheckNodeSection |
                                  SwCursorSelOverFlags::Toggle );
            }
        }
    }
    return false;
}

bool SwCursorShell::GotoFootnoteAnchor()
{
    if (SwWrtShell* pWrtSh = dynamic_cast<SwWrtShell*>(this))
        pWrtSh->addCurrentPosition();

    // jump from footnote to anchor
    SwCallLink aLk( *this ); // watch Cursor-Moves
    bool bRet = m_pCurrentCursor->GotoFootnoteAnchor();
    if( bRet )
    {
        // special treatment for table header row
        m_pCurrentCursor->GetPtPos() = Point();
        UpdateCursor( SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE |
                    SwCursorShell::READONLY );
    }
    return bRet;
}

static bool CmpLE( const SwTextFootnote& rFootnote, SwNodeOffset nNd, sal_Int32 nCnt )
{
    const SwNodeOffset nTNd = rFootnote.GetTextNode().GetIndex();
    return nTNd < nNd || ( nTNd == nNd && rFootnote.GetStart() <= nCnt );
}

static bool CmpL( const SwTextFootnote& rFootnote, SwNodeOffset nNd, sal_Int32 nCnt )
{
    const SwNodeOffset nTNd = rFootnote.GetTextNode().GetIndex();
    return nTNd < nNd || ( nTNd == nNd && rFootnote.GetStart() < nCnt );
}

bool SwCursor::GotoNextFootnoteAnchor()
{
    const SwFootnoteIdxs& rFootnoteArr = GetDoc().GetFootnoteIdxs();
    const SwTextFootnote* pTextFootnote = nullptr;
    size_t nPos = 0;

    if( rFootnoteArr.empty() )
    {
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::NavElementNotFound );
        return false;
    }

    if( rFootnoteArr.SeekEntry( GetPoint()->GetNode(), &nPos ))
    {
        // there is a footnote with this index, so search also for the next one
        if( nPos < rFootnoteArr.size() )
        {
            SwNodeOffset nNdPos = GetPoint()->GetNodeIndex();
            const sal_Int32 nCntPos = GetPoint()->GetContentIndex();

            pTextFootnote = rFootnoteArr[ nPos ];
            // search forwards
            if( CmpLE( *pTextFootnote, nNdPos, nCntPos ) )
            {
                pTextFootnote = nullptr;
                for( ++nPos; nPos < rFootnoteArr.size(); ++nPos )
                {
                    pTextFootnote = rFootnoteArr[ nPos ];
                    if( !CmpLE( *pTextFootnote, nNdPos, nCntPos ) )
                        break; // found
                    pTextFootnote = nullptr;
                }
            }
            else if( nPos )
            {
                // search backwards
                pTextFootnote = nullptr;
                while( nPos )
                {
                    pTextFootnote = rFootnoteArr[ --nPos ];
                    if( CmpLE( *pTextFootnote, nNdPos, nCntPos ) )
                    {
                        pTextFootnote = rFootnoteArr[ ++nPos ];
                        break; // found
                    }
                }
            }
        }
    }
    else if( nPos < rFootnoteArr.size() )
        pTextFootnote = rFootnoteArr[ nPos ];

    if (pTextFootnote == nullptr)
    {
        pTextFootnote = rFootnoteArr[ 0 ];
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::EndWrapped );
    }
    else
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::Empty );

    bool bRet = nullptr != pTextFootnote;
    if( bRet )
    {
        SwCursorSaveState aSaveState( *this );

        SwTextNode& rTNd = const_cast<SwTextNode&>(pTextFootnote->GetTextNode());
        GetPoint()->Assign( rTNd, pTextFootnote->GetStart() );
        bRet = !IsSelOvr();
    }
    return bRet;
}

bool SwCursor::GotoPrevFootnoteAnchor()
{
    const SwFootnoteIdxs& rFootnoteArr = GetDoc().GetFootnoteIdxs();
    const SwTextFootnote* pTextFootnote = nullptr;
    size_t nPos = 0;

    if( rFootnoteArr.empty() )
    {
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::NavElementNotFound );
        return false;
    }

    if( rFootnoteArr.SeekEntry( GetPoint()->GetNode(), &nPos ) )
    {
        // there is a footnote with this index, so search also for the next one
        SwNodeOffset nNdPos = GetPoint()->GetNodeIndex();
        const sal_Int32 nCntPos = GetPoint()->GetContentIndex();

        pTextFootnote = rFootnoteArr[ nPos ];
        // search forwards
        if( CmpL( *pTextFootnote, nNdPos, nCntPos ))
        {
            for( ++nPos; nPos < rFootnoteArr.size(); ++nPos )
            {
                pTextFootnote = rFootnoteArr[ nPos ];
                if( !CmpL( *pTextFootnote, nNdPos, nCntPos ) )
                {
                    pTextFootnote = rFootnoteArr[ nPos-1 ];
                    break;
                }
            }
        }
        else if( nPos )
        {
            // search backwards
            pTextFootnote = nullptr;
            while( nPos )
            {
                pTextFootnote = rFootnoteArr[ --nPos ];
                if( CmpL( *pTextFootnote, nNdPos, nCntPos ))
                    break; // found
                pTextFootnote = nullptr;
            }
        }
        else
            pTextFootnote = nullptr;
    }
    else if( nPos )
        pTextFootnote = rFootnoteArr[ nPos-1 ];

    if( pTextFootnote == nullptr )
    {
        pTextFootnote = rFootnoteArr[ rFootnoteArr.size() - 1 ];
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::StartWrapped );
    }
    else
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::Empty );

    bool bRet = nullptr != pTextFootnote;
    if( bRet )
    {
        SwCursorSaveState aSaveState( *this );

        SwTextNode& rTNd = const_cast<SwTextNode&>(pTextFootnote->GetTextNode());
        GetPoint()->Assign( rTNd, pTextFootnote->GetStart() );
        bRet = !IsSelOvr();
    }
    return bRet;
}

bool SwCursorShell::GotoNextFootnoteAnchor()
{
    return CallCursorFN( &SwCursor::GotoNextFootnoteAnchor );
}

bool SwCursorShell::GotoPrevFootnoteAnchor()
{
    return CallCursorFN( &SwCursor::GotoPrevFootnoteAnchor );
}

/// jump from border to anchor
void SwCursorShell::GotoFlyAnchor()
{
    CurrShell aCurr( this );
    const SwFrame* pFrame = GetCurrFrame();
    do {
        pFrame = pFrame->GetUpper();
    } while( pFrame && !pFrame->IsFlyFrame() );

    if( !pFrame ) // no FlyFrame
        return;

    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *m_pCurrentCursor );

    // jump in BodyFrame closest to FlyFrame
    SwRect aTmpRect( m_aCharRect );
    if( !pFrame->getFrameArea().Contains( aTmpRect ))
        aTmpRect = pFrame->getFrameArea();
    Point aPt( aTmpRect.Left(), aTmpRect.Top() +
                ( aTmpRect.Bottom() - aTmpRect.Top() ) / 2 );
    aPt.setX(aPt.getX() > (pFrame->getFrameArea().Left() + (pFrame->getFrameArea().SSize().Width() / 2 ))
                ? pFrame->getFrameArea().Right()
                : pFrame->getFrameArea().Left());

    const SwPageFrame* pPageFrame = pFrame->FindPageFrame();
    const SwContentFrame* pFndFrame = pPageFrame->GetContentPos( aPt, false, true );
    pFndFrame->GetModelPositionForViewPoint( m_pCurrentCursor->GetPoint(), aPt );

    bool bRet = !m_pCurrentCursor->IsInProtectTable() && !m_pCurrentCursor->IsSelOvr();
    if( bRet )
        UpdateCursor( SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE |
                    SwCursorShell::READONLY );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
