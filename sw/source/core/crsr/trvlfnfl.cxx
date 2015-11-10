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

bool SwCrsrShell::CallCrsrFN( FNCrsr fnCrsr )
{
    SwCallLink aLk( *this ); // watch Crsr-Moves
    SwCursor* pCrsr = getShellCrsr( true );
    bool bRet = (pCrsr->*fnCrsr)();
    if( bRet )
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY );
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
        SwCrsrSaveState aSaveState( *this );
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

bool SwCrsrShell::GotoFootnoteText()
{
    bool bRet = CallCrsrFN( &SwCursor::GotoFootnoteText );
    if( !bRet )
    {
        SwTextNode* pTextNd = _GetCrsr() ?
                   _GetCrsr()->GetPoint()->nNode.GetNode().GetTextNode() : nullptr;
        if( pTextNd )
        {
            const SwFrm *pFrm = pTextNd->getLayoutFrm( GetLayout(), &_GetCrsr()->GetSttPos(),
                                                 _GetCrsr()->Start() );
            const SwFootnoteBossFrm* pFootnoteBoss;
            bool bSkip = pFrm && pFrm->IsInFootnote();
            while( pFrm && nullptr != ( pFootnoteBoss = pFrm->FindFootnoteBossFrm() ) )
            {
                if( nullptr != ( pFrm = pFootnoteBoss->FindFootnoteCont() ) )
                {
                    if( bSkip )
                        bSkip = false;
                    else
                    {
                        const SwContentFrm* pCnt = static_cast<const SwLayoutFrm*>
                                                        (pFrm)->ContainsContent();
                        if( pCnt )
                        {
                            const SwContentNode* pNode = pCnt->GetNode();
                            _GetCrsr()->GetPoint()->nNode = *pNode;
                            _GetCrsr()->GetPoint()->nContent.Assign(
                                const_cast<SwContentNode*>(pNode),
                                static_cast<const SwTextFrm*>(pCnt)->GetOfst() );
                            UpdateCrsr( SwCrsrShell::SCROLLWIN |
                                SwCrsrShell::CHKRANGE | SwCrsrShell::READONLY );
                            bRet = true;
                            break;
                        }
                    }
                }
                if( pFootnoteBoss->GetNext() && !pFootnoteBoss->IsPageFrm() )
                    pFrm = pFootnoteBoss->GetNext();
                else
                    pFrm = pFootnoteBoss->GetUpper();
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
                SwCrsrSaveState aSaveState( *this );

                SwTextNode& rTNd = (SwTextNode&)pTextFootnote->GetTextNode();
                GetPoint()->nNode = rTNd;
                GetPoint()->nContent.Assign( &rTNd, pTextFootnote->GetStart() );

                return !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                  nsSwCursorSelOverFlags::SELOVER_TOGGLE );
            }
    }
    return false;
}

bool SwCrsrShell::GotoFootnoteAnchor()
{
    // jump from footnote to anchor
    SwCallLink aLk( *this ); // watch Crsr-Moves
    bool bRet = m_pCurCrsr->GotoFootnoteAnchor();
    if( bRet )
    {
        // special treatment for table header row
        m_pCurCrsr->GetPtPos() = Point();
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY );
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
        SwCrsrSaveState aSaveState( *this );

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
        SwCrsrSaveState aSaveState( *this );

        SwTextNode& rTNd = (SwTextNode&)pTextFootnote->GetTextNode();
        GetPoint()->nNode = rTNd;
        GetPoint()->nContent.Assign( &rTNd, pTextFootnote->GetStart() );
        bRet = !IsSelOvr();
    }
    return bRet;
}

bool SwCrsrShell::GotoNextFootnoteAnchor()
{
    return CallCrsrFN( &SwCursor::GotoNextFootnoteAnchor );
}

bool SwCrsrShell::GotoPrevFootnoteAnchor()
{
    return CallCrsrFN( &SwCursor::GotoPrevFootnoteAnchor );
}

/// jump from border to anchor
bool SwCrsrShell::GotoFlyAnchor()
{
    SET_CURR_SHELL( this );
    const SwFrm* pFrm = GetCurrFrm();
    do {
        pFrm = pFrm->GetUpper();
    } while( pFrm && !pFrm->IsFlyFrm() );

    if( !pFrm ) // no FlyFrame
        return false;

    SwCallLink aLk( *this ); // watch Crsr-Moves
    SwCrsrSaveState aSaveState( *m_pCurCrsr );

    // jump in BodyFrame closest to FlyFrame
    SwRect aTmpRect( m_aCharRect );
    if( !pFrm->Frm().IsInside( aTmpRect ))
        aTmpRect = pFrm->Frm();
    Point aPt( aTmpRect.Left(), aTmpRect.Top() +
                ( aTmpRect.Bottom() - aTmpRect.Top() ) / 2 );
    aPt.setX(aPt.getX() > (pFrm->Frm().Left() + (pFrm->Frm().SSize().Width() / 2 ))
                ? pFrm->Frm().Right()
                : pFrm->Frm().Left());

    const SwPageFrm* pPageFrm = pFrm->FindPageFrm();
    const SwContentFrm* pFndFrm = pPageFrm->GetContentPos( aPt, false, true );
    pFndFrm->GetCrsrOfst( m_pCurCrsr->GetPoint(), aPt );

    bool bRet = !m_pCurCrsr->IsInProtectTable() && !m_pCurCrsr->IsSelOvr();
    if( bRet )
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY );
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
