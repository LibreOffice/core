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
    SwCallLink aLk( *this ); 
    SwCursor* pCrsr = getShellCrsr( true );
    sal_Bool bRet = (pCrsr->*fnCrsr)();
    if( bRet )
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY );
    return bRet;
}

sal_Bool SwCursor::GotoFtnTxt()
{
    
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
    
    const SwNode* pSttNd = GetNode()->FindFootnoteStartNode();
    if( pSttNd )
    {
        
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
    
    SwCallLink aLk( *this ); 
    sal_Bool bRet = m_pCurCrsr->GotoFtnAnchor();
    if( bRet )
    {
        
        m_pCurCrsr->GetPtPos() = Point();
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY );
    }
    return bRet;
}

inline sal_Bool CmpLE( const SwTxtFtn& rFtn, sal_uLong nNd, sal_Int32 nCnt )
{
    const sal_uLong nTNd = rFtn.GetTxtNode().GetIndex();
    return nTNd < nNd || ( nTNd == nNd && *rFtn.GetStart() <= nCnt );
}

inline sal_Bool CmpL( const SwTxtFtn& rFtn, sal_uLong nNd, sal_Int32 nCnt )
{
    const sal_uLong nTNd = rFtn.GetTxtNode().GetIndex();
    return nTNd < nNd || ( nTNd == nNd && *rFtn.GetStart() < nCnt );
}

sal_Bool SwCursor::GotoNextFtnAnchor()
{
    const SwFtnIdxs& rFtnArr = GetDoc()->GetFtnIdxs();
    const SwTxtFtn* pTxtFtn = 0;
    sal_uInt16 nPos;

    if( rFtnArr.SeekEntry( GetPoint()->nNode, &nPos ))
    {
        
        if( nPos < rFtnArr.size() )
        {
            sal_uLong nNdPos = GetPoint()->nNode.GetIndex();
            const sal_Int32 nCntPos = GetPoint()->nContent.GetIndex();

            pTxtFtn = rFtnArr[ nPos ];
            
            if( CmpLE( *pTxtFtn, nNdPos, nCntPos ) )
            {
                pTxtFtn = 0;
                for( ++nPos; nPos < rFtnArr.size(); ++nPos )
                {
                    pTxtFtn = rFtnArr[ nPos ];
                    if( !CmpLE( *pTxtFtn, nNdPos, nCntPos ) )
                        break; 
                    pTxtFtn = 0;
                }
            }
            else if( nPos )
            {
                
                pTxtFtn = 0;
                while( nPos )
                {
                    pTxtFtn = rFtnArr[ --nPos ];
                    if( CmpLE( *pTxtFtn, nNdPos, nCntPos ) )
                    {
                        pTxtFtn = rFtnArr[ ++nPos ];
                        break; 
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
        
        sal_uLong nNdPos = GetPoint()->nNode.GetIndex();
        const sal_Int32 nCntPos = GetPoint()->nContent.GetIndex();

        pTxtFtn = rFtnArr[ nPos ];
        
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
            
            pTxtFtn = 0;
            while( nPos )
            {
                pTxtFtn = rFtnArr[ --nPos ];
                if( CmpL( *pTxtFtn, nNdPos, nCntPos ))
                    break; 
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


sal_Bool SwCrsrShell::GotoFlyAnchor()
{
    SET_CURR_SHELL( this );
    const SwFrm* pFrm = GetCurrFrm();
    do {
        pFrm = pFrm->GetUpper();
    } while( pFrm && !pFrm->IsFlyFrm() );

    if( !pFrm ) 
        return sal_False;

    SwCallLink aLk( *this ); 
    SwCrsrSaveState aSaveState( *m_pCurCrsr );

    
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
