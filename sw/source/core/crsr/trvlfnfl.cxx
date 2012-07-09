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

sal_Bool SwCrsrShell::CallCrsrFN( FNCrsr fnCrsr )
{
    SwCallLink aLk( *this ); // watch Crsr-Moves
    SwCursor* pCrsr = getShellCrsr( true );
    sal_Bool bRet = (pCrsr->*fnCrsr)();
    if( bRet )
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY );
    return bRet;
}

sal_Bool SwCursor::GotoFtnTxt()
{
    // jump from content to footnote
    sal_Bool bRet = sal_False;
    SwTxtNode* pTxtNd = GetPoint()->nNode.GetNode().GetTxtNode();

    SwTxtAttr *const pFtn( (pTxtNd)
        ? pTxtNd->GetTxtAttrForCharAt(
            GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN)
        : 0);
    if (pFtn)
    {
        SwCrsrSaveState aSaveState( *this );
        GetPoint()->nNode = *((SwTxtFtn*)pFtn)->GetStartNode();

        SwCntntNode* pCNd = GetDoc()->GetNodes().GoNextSection(
                                            &GetPoint()->nNode,
                                            sal_True, !IsReadOnlyAvailable() );
        if( pCNd )
        {
            GetPoint()->nContent.Assign( pCNd, 0 );
            bRet = !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                              nsSwCursorSelOverFlags::SELOVER_TOGGLE );
        }
    }
    return bRet;
}

sal_Bool SwCrsrShell::GotoFtnTxt()
{
    sal_Bool bRet = CallCrsrFN( &SwCursor::GotoFtnTxt );
    if( !bRet )
    {
        SwTxtNode* pTxtNd = _GetCrsr() ?
                   _GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode() : NULL;
        if( pTxtNd )
        {
            const SwFrm *pFrm = pTxtNd->getLayoutFrm( GetLayout(), &_GetCrsr()->GetSttPos(),
                                                 _GetCrsr()->Start() );
            const SwFtnBossFrm* pFtnBoss;
            bool bSkip = pFrm && pFrm->IsInFtn();
            while( pFrm && 0 != ( pFtnBoss = pFrm->FindFtnBossFrm() ) )
            {
                if( 0 != ( pFrm = pFtnBoss->FindFtnCont() ) )
                {
                    if( bSkip )
                        bSkip = false;
                    else
                    {
                        const SwCntntFrm* pCnt = static_cast<const SwLayoutFrm*>
                                                        (pFrm)->ContainsCntnt();
                        if( pCnt )
                        {
                            const SwCntntNode* pNode = pCnt->GetNode();
                            _GetCrsr()->GetPoint()->nNode = *pNode;
                            _GetCrsr()->GetPoint()->nContent.Assign(
                                const_cast<SwCntntNode*>(pNode),
                                static_cast<const SwTxtFrm*>(pCnt)->GetOfst() );
                            UpdateCrsr( SwCrsrShell::SCROLLWIN |
                                SwCrsrShell::CHKRANGE | SwCrsrShell::READONLY );
                            bRet = sal_True;
                            break;
                        }
                    }
                }
                if( pFtnBoss->GetNext() && !pFtnBoss->IsPageFrm() )
                    pFrm = pFtnBoss->GetNext();
                else
                    pFrm = pFtnBoss->GetUpper();
            }
        }
    }
    return bRet;
}

sal_Bool SwCursor::GotoFtnAnchor()
{
    // jump from footnote to anchor
    const SwNode* pSttNd = GetNode()->FindFootnoteStartNode();
    if( pSttNd )
    {
        // search in all footnotes in document for this StartIndex
        const SwTxtFtn* pTxtFtn;
        const SwFtnIdxs& rFtnArr = pSttNd->GetDoc()->GetFtnIdxs();
        for( sal_uInt16 n = 0; n < rFtnArr.size(); ++n )
            if( 0 != ( pTxtFtn = rFtnArr[ n ])->GetStartNode() &&
                pSttNd == &pTxtFtn->GetStartNode()->GetNode() )
            {
                SwCrsrSaveState aSaveState( *this );

                SwTxtNode& rTNd = (SwTxtNode&)pTxtFtn->GetTxtNode();
                GetPoint()->nNode = rTNd;
                GetPoint()->nContent.Assign( &rTNd, *pTxtFtn->GetStart() );

                return !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                  nsSwCursorSelOverFlags::SELOVER_TOGGLE );
            }
    }
    return sal_False;
}

sal_Bool SwCrsrShell::GotoFtnAnchor()
{
    // jump from footnote to anchor
    SwCallLink aLk( *this ); // watch Crsr-Moves
    sal_Bool bRet = m_pCurCrsr->GotoFtnAnchor();
    if( bRet )
    {
        // special treatment for table header row
        m_pCurCrsr->GetPtPos() = Point();
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY );
    }
    return bRet;
}

inline sal_Bool CmpLE( const SwTxtFtn& rFtn, sal_uLong nNd, xub_StrLen nCnt )
{
    sal_uLong nTNd = rFtn.GetTxtNode().GetIndex();
    return nTNd < nNd || ( nTNd == nNd && *rFtn.GetStart() <= nCnt );
}

inline sal_Bool CmpL( const SwTxtFtn& rFtn, sal_uLong nNd, xub_StrLen nCnt )
{
    sal_uLong nTNd = rFtn.GetTxtNode().GetIndex();
    return nTNd < nNd || ( nTNd == nNd && *rFtn.GetStart() < nCnt );
}

sal_Bool SwCursor::GotoNextFtnAnchor()
{
    const SwFtnIdxs& rFtnArr = GetDoc()->GetFtnIdxs();
    const SwTxtFtn* pTxtFtn = 0;
    sal_uInt16 nPos;

    if( rFtnArr.SeekEntry( GetPoint()->nNode, &nPos ))
    {
        // there is a footnote with this index, so search also for the next one
        if( nPos < rFtnArr.size() )
        {
            sal_uLong nNdPos = GetPoint()->nNode.GetIndex();
            xub_StrLen nCntPos = GetPoint()->nContent.GetIndex();

            pTxtFtn = rFtnArr[ nPos ];
            // search forwards
            if( CmpLE( *pTxtFtn, nNdPos, nCntPos ) )
            {
                pTxtFtn = 0;
                for( ++nPos; nPos < rFtnArr.size(); ++nPos )
                {
                    pTxtFtn = rFtnArr[ nPos ];
                    if( !CmpLE( *pTxtFtn, nNdPos, nCntPos ) )
                        break; // found
                    pTxtFtn = 0;
                }
            }
            else if( nPos )
            {
                // search backwards
                pTxtFtn = 0;
                while( nPos )
                {
                    pTxtFtn = rFtnArr[ --nPos ];
                    if( CmpLE( *pTxtFtn, nNdPos, nCntPos ) )
                    {
                        pTxtFtn = rFtnArr[ ++nPos ];
                        break; // found
                    }
                }
            }
        }
    }
    else if( nPos < rFtnArr.size() )
        pTxtFtn = rFtnArr[ nPos ];

    sal_Bool bRet = 0 != pTxtFtn;
    if( bRet )
    {
        SwCrsrSaveState aSaveState( *this );

        SwTxtNode& rTNd = (SwTxtNode&)pTxtFtn->GetTxtNode();
        GetPoint()->nNode = rTNd;
        GetPoint()->nContent.Assign( &rTNd, *pTxtFtn->GetStart() );
        bRet = !IsSelOvr();
    }
    return bRet;
}

sal_Bool SwCursor::GotoPrevFtnAnchor()
{
    const SwFtnIdxs& rFtnArr = GetDoc()->GetFtnIdxs();
    const SwTxtFtn* pTxtFtn = 0;
    sal_uInt16 nPos;

    if( rFtnArr.SeekEntry( GetPoint()->nNode, &nPos ) )
    {
        // there is a footnote with this index, so search also for the next one
        sal_uLong nNdPos = GetPoint()->nNode.GetIndex();
        xub_StrLen nCntPos = GetPoint()->nContent.GetIndex();

        pTxtFtn = rFtnArr[ nPos ];
        // search forwards
        if( CmpL( *pTxtFtn, nNdPos, nCntPos ))
        {
            for( ++nPos; nPos < rFtnArr.size(); ++nPos )
            {
                pTxtFtn = rFtnArr[ nPos ];
                if( !CmpL( *pTxtFtn, nNdPos, nCntPos ) )
                {
                    pTxtFtn = rFtnArr[ nPos-1 ];
                    break;
                }
            }
        }
        else if( nPos )
        {
            // search backwards
            pTxtFtn = 0;
            while( nPos )
            {
                pTxtFtn = rFtnArr[ --nPos ];
                if( CmpL( *pTxtFtn, nNdPos, nCntPos ))
                    break; // found
                pTxtFtn = 0;
            }
        }
        else
            pTxtFtn = 0;
    }
    else if( nPos )
        pTxtFtn = rFtnArr[ nPos-1 ];

    sal_Bool bRet = 0 != pTxtFtn;
    if( bRet )
    {
        SwCrsrSaveState aSaveState( *this );

        SwTxtNode& rTNd = (SwTxtNode&)pTxtFtn->GetTxtNode();
        GetPoint()->nNode = rTNd;
        GetPoint()->nContent.Assign( &rTNd, *pTxtFtn->GetStart() );
        bRet = !IsSelOvr();
    }
    return bRet;
}

sal_Bool SwCrsrShell::GotoNextFtnAnchor()
{
    return CallCrsrFN( &SwCursor::GotoNextFtnAnchor );
}

sal_Bool SwCrsrShell::GotoPrevFtnAnchor()
{
    return CallCrsrFN( &SwCursor::GotoPrevFtnAnchor );
}

/// jump from border to anchor
sal_Bool SwCrsrShell::GotoFlyAnchor()
{
    SET_CURR_SHELL( this );
    const SwFrm* pFrm = GetCurrFrm();
    do {
        pFrm = pFrm->GetUpper();
    } while( pFrm && !pFrm->IsFlyFrm() );

    if( !pFrm ) // no FlyFrame
        return sal_False;

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
    const SwCntntFrm* pFndFrm = pPageFrm->GetCntntPos( aPt, sal_False, sal_True );
    pFndFrm->GetCrsrOfst( m_pCurCrsr->GetPoint(), aPt );

    sal_Bool bRet = !m_pCurCrsr->IsInProtectTable() && !m_pCurCrsr->IsSelOvr();
    if( bRet )
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY );
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
