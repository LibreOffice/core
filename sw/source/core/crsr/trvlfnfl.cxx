/*************************************************************************
 *
 *  $RCSfile: trvlfnfl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif

#ifndef _CRSRSH_HXX
#include <crsrsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif
#ifndef _SWCRSR_HXX
#include <swcrsr.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FTNIDX_HXX //autogen
#include <ftnidx.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _CALLNK_HXX
#include <callnk.hxx>
#endif

FASTBOOL SwCrsrShell::CallCrsrFN( FNCrsr fnCrsr )
{
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
    SwCursor* pCrsr = pTblCrsr ? pTblCrsr : pCurCrsr;
    FASTBOOL bRet = (pCrsr->*fnCrsr)();
    if( bRet )
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY );
    return bRet;
}

FASTBOOL SwCursor::GotoFtnTxt()
{
    // springe aus dem Content zur Fussnote
    FASTBOOL bRet = FALSE;
    SwTxtAttr *pFtn;
    SwTxtNode* pTxtNd = GetPoint()->nNode.GetNode().GetTxtNode();

    if( pTxtNd && 0 != (
        pFtn = pTxtNd->GetTxtAttr( GetPoint()->nContent, RES_TXTATR_FTN ) ))
    {
        SwCrsrSaveState aSaveState( *this );
        GetPoint()->nNode = *((SwTxtFtn*)pFtn)->GetStartNode();

        SwCntntNode* pCNd = GetDoc()->GetNodes().GoNextSection(
                                            &GetPoint()->nNode,
                                            TRUE, !IsReadOnlyAvailable() );
        if( pCNd )
        {
            GetPoint()->nContent.Assign( pCNd, 0 );
            bRet = !IsSelOvr( SELOVER_CHECKNODESSECTION | SELOVER_TOGGLE );
        }
    }
    return bRet;
}

FASTBOOL SwCrsrShell::GotoFtnTxt()
{
    return CallCrsrFN( &SwCursor::GotoFtnTxt );
}


FASTBOOL SwCursor::GotoFtnAnchor()
{
    // springe aus der Fussnote zum Anker
    const SwNode* pSttNd = GetNode()->FindFootnoteStartNode();
    if( pSttNd )
    {
        // durchsuche alle Fussnoten im Dokument nach diesem StartIndex
        const SwTxtFtn* pTxtFtn;
        const SwFtnIdxs& rFtnArr = pSttNd->GetDoc()->GetFtnIdxs();
        for( USHORT n = 0; n < rFtnArr.Count(); ++n )
            if( 0 != ( pTxtFtn = rFtnArr[ n ])->GetStartNode() &&
                pSttNd == &pTxtFtn->GetStartNode()->GetNode() )
            {
                SwCrsrSaveState aSaveState( *this );

                SwTxtNode& rTNd = (SwTxtNode&)pTxtFtn->GetTxtNode();

                GetPoint()->nNode = rTNd;
                GetPoint()->nContent.Assign( &rTNd, *pTxtFtn->GetStart() );

                return !IsSelOvr( SELOVER_CHECKNODESSECTION | SELOVER_TOGGLE );
            }
    }
    return FALSE;
}

FASTBOOL SwCrsrShell::GotoFtnAnchor()
{
    // springe aus der Fussnote zum Anker
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
    FASTBOOL bRet = pCurCrsr->GotoFtnAnchor();
    if( bRet )
    {
        // BUG 5996: Tabellen-Kopfzeile sonderbehandeln
        pCurCrsr->GetPtPos() = Point();
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY );
    }
    return bRet;
}

inline CmpLE( const SwTxtFtn& rFtn, ULONG nNd, xub_StrLen nCnt )
{
    ULONG nTNd = rFtn.GetTxtNode().GetIndex();
    return nTNd < nNd || ( nTNd == nNd && *rFtn.GetStart() <= nCnt );
}
inline CmpL( const SwTxtFtn& rFtn, ULONG nNd, xub_StrLen nCnt )
{
    ULONG nTNd = rFtn.GetTxtNode().GetIndex();
    return nTNd < nNd || ( nTNd == nNd && *rFtn.GetStart() < nCnt );
}

FASTBOOL SwCursor::GotoNextFtnAnchor()
{
    const SwFtnIdxs& rFtnArr = GetDoc()->GetFtnIdxs();
    const SwTxtFtn* pTxtFtn = 0;
    USHORT nPos;

    if( rFtnArr.SeekEntry( GetPoint()->nNode, &nPos ))
    {
        // es gibt eine Fussnote mit dem Index, suche also die
        // naechstgelegene
        if( nPos < rFtnArr.Count() )
        {
            ULONG nNdPos = GetPoint()->nNode.GetIndex();
            xub_StrLen nCntPos = GetPoint()->nContent.GetIndex();

            pTxtFtn = rFtnArr[ nPos ];
            // suche vorewaerts zur naechsten
            if( CmpLE( *pTxtFtn, nNdPos, nCntPos ) )
            {
                pTxtFtn = 0;
                for( ++nPos; nPos < rFtnArr.Count(); ++nPos )
                {
                    pTxtFtn = rFtnArr[ nPos ];
                    if( !CmpLE( *pTxtFtn, nNdPos, nCntPos ) )
                        break;      // gefunden
                    pTxtFtn = 0;
                }
            }
            else if( nPos )
            {
                // suche rueckwaerts zur vorherigen
                pTxtFtn = 0;
                while( nPos )
                {
                    pTxtFtn = rFtnArr[ --nPos ];
                    if( CmpLE( *pTxtFtn, nNdPos, nCntPos ) )
                    {
                        pTxtFtn = rFtnArr[ ++nPos ];
                        break;      // gefunden
                    }
//                  pTxtFtn = 0;
                }
            }
        }
    }
    else if( nPos < rFtnArr.Count() )
        pTxtFtn = rFtnArr[ nPos ];

    FASTBOOL bRet = 0 != pTxtFtn;
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

FASTBOOL SwCursor::GotoPrevFtnAnchor()
{
    const SwFtnIdxs& rFtnArr = GetDoc()->GetFtnIdxs();
    const SwTxtFtn* pTxtFtn = 0;
    USHORT nPos;

    if( rFtnArr.SeekEntry( GetPoint()->nNode, &nPos ) )
    {
        // es gibt eine Fussnote mit dem Index, suche also die
        // naechstgelegene
        ULONG nNdPos = GetPoint()->nNode.GetIndex();
        xub_StrLen nCntPos = GetPoint()->nContent.GetIndex();

        pTxtFtn = rFtnArr[ nPos ];
        // suche vorwaerts zur naechsten
        if( CmpL( *pTxtFtn, nNdPos, nCntPos ))
        {
            for( ++nPos; nPos < rFtnArr.Count(); ++nPos )
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
            // suche rueckwaerts zur vorherigen
            pTxtFtn = 0;
            while( nPos )
            {
                pTxtFtn = rFtnArr[ --nPos ];
                if( CmpL( *pTxtFtn, nNdPos, nCntPos ))
                    break;      // gefunden
                pTxtFtn = 0;
            }
        }
        else
            pTxtFtn = 0;
    }
    else if( nPos )
        pTxtFtn = rFtnArr[ nPos-1 ];

    FASTBOOL bRet = 0 != pTxtFtn;
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

FASTBOOL SwCursor::GotoNextFtnCntnt()
{
    SwCrsrSaveState aSaveState( *this );

    const SwNode* pSttNd = GetNode()->FindFootnoteStartNode();
    if( pSttNd )
        GotoFtnAnchor();

    SwTxtFtn *pFtn;
    SwTxtNode* pTxtNd = GetPoint()->nNode.GetNode().GetTxtNode();

    if( !pTxtNd || 0 == ( pFtn = (SwTxtFtn*)pTxtNd->GetTxtAttr(
                                GetPoint()->nContent, RES_TXTATR_FTN ) ))
        return GotoNextFtnAnchor();

    FASTBOOL bRet = FALSE;
    const SwFtnIdxs& rFtnArr = GetDoc()->GetFtnIdxs();
    USHORT nPos = rFtnArr.GetPos( pFtn );
    if( nPos < rFtnArr.Count() - 1 )
    {
        pFtn = rFtnArr[ nPos + 1 ];
        pTxtNd = (SwTxtNode*)&pFtn->GetTxtNode();
        GetPoint()->nNode = *pTxtNd;
        GetPoint()->nContent.Assign( pTxtNd, *pFtn->GetStart() );
        bRet = !IsSelOvr();
    }
    return bRet;
}

FASTBOOL SwCursor::GotoPrevFtnCntnt()
{
    SwCrsrSaveState aSaveState( *this );
    const SwNode* pSttNd = GetNode()->FindFootnoteStartNode();
    if( pSttNd )
        GotoFtnAnchor();

    SwTxtFtn *pFtn;
    SwTxtNode* pTxtNd = GetPoint()->nNode.GetNode().GetTxtNode();

    if( !pTxtNd || 0 == ( pFtn = (SwTxtFtn*)pTxtNd->GetTxtAttr(
                                GetPoint()->nContent, RES_TXTATR_FTN ) ))
        return GotoPrevFtnAnchor();

    FASTBOOL bRet = FALSE;
    const SwFtnIdxs& rFtnArr = GetDoc()->GetFtnIdxs();
    USHORT nPos = rFtnArr.GetPos( pFtn );
    if( nPos && USHRT_MAX != nPos )
    {
        pFtn = rFtnArr[ nPos - 1 ];
        pTxtNd = (SwTxtNode*)&pFtn->GetTxtNode();
        GetPoint()->nNode = *pTxtNd;
        GetPoint()->nContent.Assign( pTxtNd, *pFtn->GetStart() );
        bRet = !IsSelOvr();
    }
    return bRet;
}

FASTBOOL SwCrsrShell::GotoNextFtnAnchor()
{
    return CallCrsrFN( &SwCursor::GotoNextFtnAnchor );
}

FASTBOOL SwCrsrShell::GotoPrevFtnAnchor()
{
    return CallCrsrFN( &SwCursor::GotoPrevFtnAnchor );
}

FASTBOOL SwCrsrShell::GotoNextFtnCntnt()
{
    return CallCrsrFN( &SwCursor::GotoNextFtnCntnt );
}

FASTBOOL SwCrsrShell::GotoPrevFtnCntnt()
{
    return CallCrsrFN( &SwCursor::GotoPrevFtnCntnt );
}

/*  */

// springe aus dem Content zum Rahmen

FASTBOOL SwCrsrShell::GotoFlyTxt()
{
    SET_CURR_SHELL( this );

    // alle DrawSeiten nach Objekten durchsuchen.
    USHORT nMaxPages = GetDoc()->GetDrawModel() ?
        GetDoc()->GetDrawModel()->GetPageCount() : 0;
    if( !nMaxPages )
        return FALSE;

    const SwFrm* pFrm = GetCurrFrm();
    const SwLayoutFrm *pFndFly = 0;

    Point aPt( pFrm->Frm().Pos() );
    Point aNxtPt( LONG_MAX, LONG_MAX );

    for( USHORT nPg = 0; nPg < nMaxPages; ++nPg )
    {
        SdrPage* pActPage = GetDoc()->GetDrawModel()->GetPage( nPg );
        ASSERT( pActPage, "UUPs, keine Seite im Container" );

        ULONG nObjs = pActPage->GetObjCount();
        ASSERT( nObjs, "UUPs, was fuer Object auf der Seite" );

        for( ULONG nObj = 0; nObj < nObjs; ++nObj )
        {
            SdrObject *pObj = pActPage->GetObj( nObj );
            ASSERT( pObj ,
                    "UUPs, kein Object" );
            SwVirtFlyDrawObj *pVirt = pObj->IsWriterFlyFrame() ?
                                                    (SwVirtFlyDrawObj*)pObj : 0;
            if( pVirt && pVirt->GetFlyFrm()->Lower() &&
                !pVirt->GetFlyFrm()->Lower()->IsNoTxtFrm() )
            {

                // Wie ist die Logik? Alle, die unterhalb des aktuellen Frames
                // liegen (alle, die in den Frame hineinragen werden
                // nicht beachtet !!)
                Point aFlyPos( pVirt->GetFlyFrm()->Frm().Pos() );
                aFlyPos += pVirt->GetFlyFrm()->Prt().Pos();
                if( ( aFlyPos.Y() > aPt.Y() ||      // alle die unterhalb/daneben liegen,
                    ( aFlyPos.Y() == aPt.Y() && aFlyPos.X() > aPt.X())) &&
                    ( aFlyPos.Y() < aNxtPt.Y() ||   // Position kleiner
                    ( aFlyPos.Y() == aNxtPt.Y() && aFlyPos.X() < aNxtPt.X() )) )
                {
                    aNxtPt = aFlyPos;
                    pFndFly = pVirt->GetFlyFrm();
                }
            }
        }
    }

    FASTBOOL bRet = FALSE;
    if( pFndFly )
    {
        // Calc() ??
        SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
        SwCrsrSaveState aSaveState( *pCurCrsr );

        aPt = pFndFly->Frm().Pos() + pFndFly->Prt().Pos();
        pFndFly->GetCrsrOfst( pCurCrsr->GetPoint(), aPt );
        bRet = !pCurCrsr->IsInProtectTable( TRUE ) && !pCurCrsr->IsSelOvr();
        if( bRet )
            UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                        SwCrsrShell::READONLY );
    }
    return bRet;
}

// springe aus dem Rahmen zum Anker


FASTBOOL SwCrsrShell::GotoFlyAnchor()
{
    SET_CURR_SHELL( this );
    const SwFrm* pFrm = GetCurrFrm();
    do {
        pFrm = pFrm->GetUpper();
    } while( pFrm && !pFrm->IsFlyFrm() );

    if( !pFrm )     // ist kein FlyFrame
        return FALSE;

    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
    SwCrsrSaveState aSaveState( *pCurCrsr );

    // springe in den BodyFrame, der am naechsten vom Fly liegt
    SwRect aTmpRect( aCharRect );
    if( !pFrm->Frm().IsInside( aTmpRect ))
        aTmpRect = pFrm->Frm();
    Point aPt( aTmpRect.Left(), aTmpRect.Top() +
                ( aTmpRect.Bottom() - aTmpRect.Top() ) / 2 );
    aPt.X() = aPt.X() > (pFrm->Frm().Left() + (pFrm->Frm().SSize().Width() / 2 ))
                ? pFrm->Frm().Right()
                : pFrm->Frm().Left();

    const SwPageFrm* pPageFrm = pFrm->FindPageFrm();
    const SwCntntFrm* pFndFrm = pPageFrm->GetCntntPos( aPt, FALSE, TRUE );
    pFndFrm->GetCrsrOfst( pCurCrsr->GetPoint(), aPt );

    FASTBOOL bRet = !pCurCrsr->IsInProtectTable() && !pCurCrsr->IsSelOvr();
    if( bRet )
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY );
    return bRet;
}




