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

#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <crsrsh.hxx>
#include <doc.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <ftnfrm.hxx>
#include <viewimp.hxx>
#include <swcrsr.hxx>
#include <dflyobj.hxx>
#include <ndtxt.hxx>
#include <flyfrm.hxx>
#include <txtfrm.hxx>
#include <txtftn.hxx>
#include <ftnidx.hxx>
#include <viscrs.hxx>
#include <callnk.hxx>

bool SwCursorShell::CallCursorFN( FNCursor fnCursor )
{
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
    SwTextNode* pTextNd = GetPoint()->nNode.GetNode().GetTextNode();

    SwTextAttr *const pFootnote( (pTextNd)
        ? pTextNd->GetTextAttrForCharAt(
            GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN)
        : nullptr);
    if (pFootnote)
    {
        SwCursorSaveState aSaveState( *this );
        GetPoint()->nNode = *static_cast<SwTextFootnote*>(pFootnote)->GetStartNode();

        SwContentNode* pCNd = GetDoc()->GetNodes().GoNextSection(
                                            &GetPoint()->nNode,
                                            true, !IsReadOnlyAvailable() );
        if( pCNd )
        {
            GetPoint()->nContent.Assign( pCNd, 0 );
            bRet = !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                              nsSwCursorSelOverFlags::SELOVER_TOGGLE );
        }
    }
    return bRet;
}

bool SwCursorShell::GotoFootnoteText()
{
    bool bRet = CallCursorFN( &SwCursor::GotoFootnoteText );
    if( !bRet )
    {
        SwTextNode* pTextNd = _GetCursor() ?
                   _GetCursor()->GetPoint()->nNode.GetNode().GetTextNode() : nullptr;
        if( pTextNd )
        {
            const SwFrame *pFrame = pTextNd->getLayoutFrame( GetLayout(), &_GetCursor()->GetSttPos(),
                                                 _GetCursor()->Start() );
            const SwFootnoteBossFrame* pFootnoteBoss;
            bool bSkip = pFrame && pFrame->IsInFootnote();
            while( pFrame && nullptr != ( pFootnoteBoss = pFrame->FindFootnoteBossFrame() ) )
            {
                if( nullptr != ( pFrame = pFootnoteBoss->FindFootnoteCont() ) )
                {
                    if( bSkip )
                        bSkip = false;
                    else
                    {
                        const SwContentFrame* pCnt = static_cast<const SwLayoutFrame*>
                                                        (pFrame)->ContainsContent();
                        if( pCnt )
                        {
                            const SwContentNode* pNode = pCnt->GetNode();
                            _GetCursor()->GetPoint()->nNode = *pNode;
                            _GetCursor()->GetPoint()->nContent.Assign(
                                const_cast<SwContentNode*>(pNode),
                                static_cast<const SwTextFrame*>(pCnt)->GetOfst() );
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
    const SwNode* pSttNd = GetNode().FindFootnoteStartNode();
    if( pSttNd )
    {
        // search in all footnotes in document for this StartIndex
        const SwTextFootnote* pTextFootnote;
        const SwFootnoteIdxs& rFootnoteArr = pSttNd->GetDoc()->GetFootnoteIdxs();
        for( size_t n = 0; n < rFootnoteArr.size(); ++n )
            if( nullptr != ( pTextFootnote = rFootnoteArr[ n ])->GetStartNode() &&
                pSttNd == &pTextFootnote->GetStartNode()->GetNode() )
            {
                SwCursorSaveState aSaveState( *this );

                SwTextNode& rTNd = (SwTextNode&)pTextFootnote->GetTextNode();
                GetPoint()->nNode = rTNd;
                GetPoint()->nContent.Assign( &rTNd, pTextFootnote->GetStart() );

                return !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                  nsSwCursorSelOverFlags::SELOVER_TOGGLE );
            }
    }
    return false;
}

bool SwCursorShell::GotoFootnoteAnchor()
{
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

inline bool CmpLE( const SwTextFootnote& rFootnote, sal_uLong nNd, sal_Int32 nCnt )
{
    const sal_uLong nTNd = rFootnote.GetTextNode().GetIndex();
    return nTNd < nNd || ( nTNd == nNd && rFootnote.GetStart() <= nCnt );
}

inline bool CmpL( const SwTextFootnote& rFootnote, sal_uLong nNd, sal_Int32 nCnt )
{
    const sal_uLong nTNd = rFootnote.GetTextNode().GetIndex();
    return nTNd < nNd || ( nTNd == nNd && rFootnote.GetStart() < nCnt );
}

bool SwCursor::GotoNextFootnoteAnchor()
{
    const SwFootnoteIdxs& rFootnoteArr = GetDoc()->GetFootnoteIdxs();
    const SwTextFootnote* pTextFootnote = nullptr;
    size_t nPos = 0;

    if( rFootnoteArr.SeekEntry( GetPoint()->nNode, &nPos ))
    {
        // there is a footnote with this index, so search also for the next one
        if( nPos < rFootnoteArr.size() )
        {
            sal_uLong nNdPos = GetPoint()->nNode.GetIndex();
            const sal_Int32 nCntPos = GetPoint()->nContent.GetIndex();

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

    bool bRet = nullptr != pTextFootnote;
    if( bRet )
    {
        SwCursorSaveState aSaveState( *this );

        SwTextNode& rTNd = (SwTextNode&)pTextFootnote->GetTextNode();
        GetPoint()->nNode = rTNd;
        GetPoint()->nContent.Assign( &rTNd, pTextFootnote->GetStart() );
        bRet = !IsSelOvr();
    }
    return bRet;
}

bool SwCursor::GotoPrevFootnoteAnchor()
{
    const SwFootnoteIdxs& rFootnoteArr = GetDoc()->GetFootnoteIdxs();
    const SwTextFootnote* pTextFootnote = nullptr;
    size_t nPos = 0;

    if( rFootnoteArr.SeekEntry( GetPoint()->nNode, &nPos ) )
    {
        // there is a footnote with this index, so search also for the next one
        sal_uLong nNdPos = GetPoint()->nNode.GetIndex();
        const sal_Int32 nCntPos = GetPoint()->nContent.GetIndex();

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

    bool bRet = nullptr != pTextFootnote;
    if( bRet )
    {
        SwCursorSaveState aSaveState( *this );

        SwTextNode& rTNd = (SwTextNode&)pTextFootnote->GetTextNode();
        GetPoint()->nNode = rTNd;
        GetPoint()->nContent.Assign( &rTNd, pTextFootnote->GetStart() );
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
bool SwCursorShell::GotoFlyAnchor()
{
    SET_CURR_SHELL( this );
    const SwFrame* pFrame = GetCurrFrame();
    do {
        pFrame = pFrame->GetUpper();
    } while( pFrame && !pFrame->IsFlyFrame() );

    if( !pFrame ) // no FlyFrame
        return false;

    SwCallLink aLk( *this ); // watch Cursor-Moves
    SwCursorSaveState aSaveState( *m_pCurrentCursor );

    // jump in BodyFrame closest to FlyFrame
    SwRect aTmpRect( m_aCharRect );
    if( !pFrame->Frame().IsInside( aTmpRect ))
        aTmpRect = pFrame->Frame();
    Point aPt( aTmpRect.Left(), aTmpRect.Top() +
                ( aTmpRect.Bottom() - aTmpRect.Top() ) / 2 );
    aPt.setX(aPt.getX() > (pFrame->Frame().Left() + (pFrame->Frame().SSize().Width() / 2 ))
                ? pFrame->Frame().Right()
                : pFrame->Frame().Left());

    const SwPageFrame* pPageFrame = pFrame->FindPageFrame();
    const SwContentFrame* pFndFrame = pPageFrame->GetContentPos( aPt, false, true );
    pFndFrame->GetCursorOfst( m_pCurrentCursor->GetPoint(), aPt );

    bool bRet = !m_pCurrentCursor->IsInProtectTable() && !m_pCurrentCursor->IsSelOvr();
    if( bRet )
        UpdateCursor( SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE |
                    SwCursorShell::READONLY );
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
