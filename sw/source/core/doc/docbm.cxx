/*************************************************************************
 *
 *  $RCSfile: docbm.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:15 $
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

#include <tools/pstm.hxx>
#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _NODE_HXX //autogen
#include <node.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>
#endif
#ifndef _SWSERV_HXX
#include <swserv.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif


SV_IMPL_OP_PTRARR_SORT(SwBookmarks, SwBookmarkPtr)
SO2_IMPL_REF( SwServerObject )

#define PCURCRSR (_pCurrCrsr)
#define FOREACHPAM_START(pSttCrsr) \
    {\
        SwPaM *_pStartCrsr = pSttCrsr, *_pCurrCrsr = pSttCrsr; \
        do {

#define FOREACHPAM_END() \
        } while( (_pCurrCrsr=(SwPaM *)_pCurrCrsr->GetNext()) != _pStartCrsr ); \
    }
#define PCURSH ((SwCrsrShell*)_pStartShell)
#define FOREACHSHELL_START( pEShell ) \
    {\
        register ViewShell *_pStartShell = pEShell; \
        do { \
            if( _pStartShell->IsA( TYPE( SwCrsrShell )) ) \
            {

#define FOREACHSHELL_END( pEShell ) \
            } \
        } while((_pStartShell=(ViewShell*)_pStartShell->GetNext())!= pEShell ); \
    }


SwBookmark* SwDoc::MakeBookmark( const SwPaM& rPaM, const KeyCode& rCode,
                                const String& rName, const String& rShortName,
                                BOOKMARK_TYPE eMark )
{
    SwBookmark *pBM;
    if( MARK == eMark )
        pBM = new SwMark( *rPaM.GetPoint(), rCode, rName, rShortName );
    else if( BOOKMARK == eMark )
    {
        pBM = new SwBookmark(*rPaM.GetPoint(), rCode, rName, rShortName);
        if( rPaM.HasMark() )
            pBM->pPos2 = new SwPosition( *rPaM.GetMark() );
    }
    else
    {
        pBM = new SwUNOMark(*rPaM.GetPoint(), rCode, rName, rShortName);
        if( rPaM.HasMark() )
            pBM->pPos2 = new SwPosition( *rPaM.GetMark() );
    }

    if( !pBookmarkTbl->Insert( pBM ) )
        delete pBM, pBM = 0;
    else
    {
        if( BOOKMARK == eMark && DoesUndo() )
        {
            ClearRedo();
            AppendUndo( new SwUndoInsBookmark( *pBM ));
        }
        if(UNO_BOOKMARK != eMark)
            SetModified();
    }
    return pBM;
}

void SwDoc::DelBookmark(USHORT nPos)
{
    SwBookmark *pBM = (*pBookmarkTbl)[nPos];
    if( DoesUndo() && !pBM->IsUNOMark())
    {
        ClearRedo();
        AppendUndo( new SwUndoDelBookmark( *pBM ));
    }
    SetModified();
    pBookmarkTbl->Remove(nPos);

    SwServerObject* pServObj = pBM->GetObject();
    if( pServObj )          // dann aus der Liste entfernen
        GetLinkManager().RemoveServer( pServObj );

    delete pBM;
}

void SwDoc::DelBookmark( const String& rName )
{
    USHORT nFnd = FindBookmark( rName );
    if( USHRT_MAX != nFnd )
        DelBookmark( nFnd );
}

USHORT SwDoc::FindBookmark( const String& rName )
{
    ASSERT( rName.Len(), "wo ist der Name?" );
    for( USHORT n = pBookmarkTbl->Count(); n ; )
        if( rName.Equals( (*pBookmarkTbl)[ --n ]->GetName() ) )
            return n;
    return USHRT_MAX;
}

// Zur Vereinfachung gibt es auch den direkten Zugriff
// auf die "echten" Bookmarks

USHORT SwDoc::GetBookmarkCnt(BOOL bBkmrk) const
{
    USHORT nRet = pBookmarkTbl->Count();
    if(bBkmrk)
    {
        for( USHORT i = nRet; i; --i )
        {
            if(!(*pBookmarkTbl)[i - 1]->IsBookMark())
                nRet--;
        }
    }
    return nRet;
}


SwBookmark& SwDoc::GetBookmark(USHORT nPos, BOOL bBkmrk)
{
    if( bBkmrk )
    {
        USHORT nCount = pBookmarkTbl->Count();
        USHORT i = 0;
        do {
            if(!(*pBookmarkTbl)[i]->IsBookMark())
                nPos++;

            i++;
        }
        while( i < nPos || !(*pBookmarkTbl)[nPos]->IsBookMark() );
    }
    return *(*pBookmarkTbl)[nPos];
}


    // erzeugt einen eindeutigen Namen. Der Name selbst muss vorgegeben
    // werden, es wird dann bei gleichen Namen nur durchnumeriert.
void SwDoc::MakeUniqueBookmarkName( String& rNm )
{
    ASSERT( rNm.Len(), "es sollte ein Name vorgegeben werden!" );

    // wir erzeugen uns eine temp. Bookmark
    String sTmp;
    USHORT nCnt = 0, n;
    USHORT nBookCnt = pBookmarkTbl->Count();
    do {
        sTmp = rNm;
        sTmp += String::CreateFromInt32( ++nCnt );
        for( n = 0; n < nBookCnt; ++n )
            if( (*pBookmarkTbl)[ n ]->GetName().Equals( sTmp ))
                break;
    } while( n < nBookCnt );
    rNm = sTmp;
}

/*  */

SaveBookmark::SaveBookmark( int eType, const SwBookmark& rBkmk,
                            const SwNodeIndex & rMvPos,
                            const SwIndex* pIdx )
    : aName( rBkmk.GetName() ), aShortName( rBkmk.GetShortName() ),
    aCode( rBkmk.GetKeyCode() ), eBkmkType( (SaveBookmarkType)eType )
{
    nNode1 = rBkmk.GetPos().nNode.GetIndex();
    nCntnt1 = rBkmk.GetPos().nContent.GetIndex();

    if( BKMK_POS & eBkmkType )
    {
        nNode1 -= rMvPos.GetIndex();
        if( pIdx && !nNode1 )
            nCntnt1 -= pIdx->GetIndex();
    }

    if( rBkmk.GetOtherPos() )
    {
        nNode2 = rBkmk.GetOtherPos()->nNode.GetIndex();
        nCntnt2 = rBkmk.GetOtherPos()->nContent.GetIndex();

        if( BKMK_POS_OTHER & eBkmkType )
        {
            nNode2 -= rMvPos.GetIndex();
            if( pIdx && !nNode2 )
                nCntnt2 -= pIdx->GetIndex();
        }
    }
    else
        nNode2 = ULONG_MAX, nCntnt2 = STRING_NOTFOUND;
}

void SaveBookmark::SetInDoc( SwDoc* pDoc, const SwNodeIndex& rNewPos,
                            const SwIndex* pIdx )
{
    SwPaM aPam( rNewPos.GetNode() );
    if( pIdx )
        aPam.GetPoint()->nContent = *pIdx;

    if( ULONG_MAX != nNode2 )
    {
        aPam.SetMark();

        if( BKMK_POS_OTHER & eBkmkType )
        {
            aPam.GetMark()->nNode += nNode2;
            if( pIdx && !nNode2 )
                aPam.GetMark()->nContent += nCntnt2;
            else
                aPam.GetMark()->nContent.Assign( aPam.GetCntntNode( FALSE ),
                                                        nCntnt2 );
        }
        else
        {
            aPam.GetMark()->nNode = nNode2;
            aPam.GetMark()->nContent.Assign( aPam.GetCntntNode( FALSE ),
                                                    nCntnt2 );
        }
    }

    if( BKMK_POS & eBkmkType )
    {
        aPam.GetPoint()->nNode += nNode1;

        if( pIdx && !nNode1 )
            aPam.GetPoint()->nContent += nCntnt1;
        else
            aPam.GetPoint()->nContent.Assign( aPam.GetCntntNode(), nCntnt1 );
    }
    else
    {
        aPam.GetPoint()->nNode = nNode1;
        aPam.GetPoint()->nContent.Assign( aPam.GetCntntNode(), nCntnt1 );
    }

    if( !aPam.HasMark() ||
        CheckNodesRange( aPam.GetPoint()->nNode, aPam.GetMark()->nNode, TRUE ))
        pDoc->MakeBookmark( aPam, aCode, aName, aShortName );
}


inline int GreaterThan( const SwPosition& rPos, const SwNodeIndex& rNdIdx,
                        const SwIndex* pIdx )
{
    return pIdx ? ( rPos.nNode > rNdIdx || ( rPos.nNode == rNdIdx &&
                                        rPos.nContent >= pIdx->GetIndex() ))
                : rPos.nNode >= rNdIdx;
}
inline int Lower( const SwPosition& rPos, const SwNodeIndex& rNdIdx,
                        const SwIndex* pIdx )
{
    return rPos.nNode < rNdIdx || ( pIdx && rPos.nNode == rNdIdx &&
                                        rPos.nContent < pIdx->GetIndex() );
}
inline int Greater( const SwPosition& rPos, const SwNodeIndex& rNdIdx,
                        const SwIndex* pIdx )
{
    return rPos.nNode > rNdIdx || ( pIdx && rPos.nNode == rNdIdx &&
                                        rPos.nContent > pIdx->GetIndex() );
}

void _DelBookmarks( const SwNodeIndex& rStt, const SwNodeIndex& rEnd,
                    SaveBookmarks* pSaveBkmk,
                    const SwIndex* pSttIdx, const SwIndex* pEndIdx )
{
    // kein gueltiger Bereich ??
    if( rStt.GetIndex() > rEnd.GetIndex() || ( rStt == rEnd &&
        (!pSttIdx || pSttIdx->GetIndex() >= pEndIdx->GetIndex())) )
        return;

    // kopiere alle Bookmarks, die im Move Bereich stehen in ein
    // Array, das alle Angaben auf die Position als Offset speichert.
    // Die neue Zuordung erfolgt nach dem Moven.
    SwDoc* pDoc = rStt.GetNode().GetDoc();
    const SwBookmarks& rBkmks = pDoc->GetBookmarks();
    for( USHORT nCnt = 0; nCnt < rBkmks.Count(); ++nCnt )
    {
        // liegt auf der Position ??
        int eType = BKMK_POS_NONE;
        SwBookmark* pBkmk = rBkmks[ nCnt ];
        if( GreaterThan( pBkmk->GetPos(), rStt, pSttIdx ) &&
            Lower( pBkmk->GetPos(), rEnd, pEndIdx ))
            eType = BKMK_POS;
        if( pBkmk->GetOtherPos() &&
            GreaterThan( *pBkmk->GetOtherPos(), rStt, pSttIdx ) &&
            Lower( *pBkmk->GetOtherPos(), rEnd, pEndIdx ))
            eType |= BKMK_POS_OTHER;

        if( BKMK_POS_NONE == eType )        // auf zum naechsten
            continue;

        if( pSaveBkmk )
        {
                // Besonderheit: komplett eingeschlossen? dann mitnehmen
            if( pEndIdx && (BKMK_POS_OTHER | BKMK_POS) != eType &&
                ( ( BKMK_POS_OTHER & eType &&
                    pBkmk->GetPos().nNode == rEnd &&
                    pBkmk->GetPos().nContent == *pEndIdx ) ||
                ( BKMK_POS & eType && pBkmk->GetOtherPos() &&
                    pBkmk->GetOtherPos()->nNode == rEnd &&
                    pBkmk->GetOtherPos()->nContent == *pEndIdx ) ) )
                    eType = BKMK_POS_OTHER | BKMK_POS;

            SaveBookmark * pSBkmk = new SaveBookmark( eType, *pBkmk, rStt, pSttIdx );
            pSaveBkmk->C40_INSERT( SaveBookmark, pSBkmk, pSaveBkmk->Count() );
            pDoc->DelBookmark( nCnt-- );
        }
        else if( (BKMK_POS_OTHER | BKMK_POS ) == eType ||
                ( BKMK_POS == eType && !pBkmk->GetOtherPos() ) )
            pDoc->DelBookmark( nCnt-- );
        else
        {
            SwPosition* pPos = (SwPosition*)(BKMK_POS & eType
                                    ? &pBkmk->GetPos()
                                    : pBkmk->GetOtherPos());
            pPos->nNode = rEnd;
            if( pEndIdx )
                pPos->nContent = *pEndIdx;
            else
            {
                SwCntntNode* pCNd = pPos->nNode.GetNode().GetCntntNode();
                BOOL bStt = TRUE;
                if( !pCNd && 0 == ( pCNd = pDoc->GetNodes().GoNext( &pPos->nNode )) )
                {
                    bStt = FALSE;
                    pPos->nNode = rStt;
                    if( 0 == ( pCNd = pDoc->GetNodes().GoPrevious( &pPos->nNode )) )
                    {
                        pPos->nNode = BKMK_POS == eType
                                        ? pBkmk->GetOtherPos()->nNode
                                        : pBkmk->GetPos().nNode;
                        pCNd = pPos->nNode.GetNode().GetCntntNode();
                    }
                }
                xub_StrLen nTmp = bStt ? 0 : pCNd->Len();
                pPos->nContent.Assign( pCNd, nTmp );
            }

            // keine ungueltigen Selektionen zulassen!
            if( pBkmk->GetOtherPos() &&
                pBkmk->GetOtherPos()->nNode.GetNode().FindTableBoxStartNode() !=
                pBkmk->GetPos().nNode.GetNode().FindTableBoxStartNode() )
            {
                SwPaM aPam( pPos == pBkmk->GetOtherPos()
                                ? pBkmk->GetPos() : *pBkmk->GetOtherPos()  );
                String sNm( pBkmk->GetName() ), sShortNm( pBkmk->GetShortName() );
                KeyCode aKCode( pBkmk->GetKeyCode() );

                pDoc->DelBookmark( nCnt-- );
                pDoc->MakeBookmark( aPam, aKCode, sNm, sShortNm, BOOKMARK );
            }
        }
    }

    // kopiere alle Redlines, die im Move Bereich stehen in ein
    // Array, das alle Angaben auf die Position als Offset speichert.
    // Die neue Zuordung erfolgt nach dem Moven.
    SwRedlineTbl& rTbl = (SwRedlineTbl&)pDoc->GetRedlineTbl();
    for( nCnt = 0; nCnt < rTbl.Count(); ++nCnt )
    {
        // liegt auf der Position ??
        int eType = BKMK_POS_NONE;
        SwRedline* pRedl = rTbl[ nCnt ];

        SwPosition *pRStt = &pRedl->GetBound(TRUE),
                   *pREnd = &pRedl->GetBound(FALSE);
        if( *pRStt > *pREnd )
        {
            SwPosition *pTmp = pRStt; pRStt = pREnd, pREnd = pTmp;
        }

        if( Greater( *pRStt, rStt, pSttIdx ) && Lower( *pRStt, rEnd, pEndIdx ))
        {
            pRStt->nNode = rEnd;
            if( pEndIdx )
                pRStt->nContent = *pEndIdx;
            else
            {
                BOOL bStt = TRUE;
                SwCntntNode* pCNd = pRStt->nNode.GetNode().GetCntntNode();
                if( !pCNd && 0 == ( pCNd = pDoc->GetNodes().GoNext( &pRStt->nNode )) )
                {
                    bStt = FALSE;
                    pRStt->nNode = rStt;
                    if( 0 == ( pCNd = pDoc->GetNodes().GoPrevious( &pRStt->nNode )) )
                    {
                        pRStt->nNode = pREnd->nNode;
                        pCNd = pRStt->nNode.GetNode().GetCntntNode();
                    }
                }
                xub_StrLen nTmp = bStt ? 0 : pCNd->Len();
                pRStt->nContent.Assign( pCNd, nTmp );
            }
        }
        if( Greater( *pREnd, rStt, pSttIdx ) && Lower( *pREnd, rEnd, pEndIdx ))
        {
            pREnd->nNode = rStt;
            if( pSttIdx )
                pREnd->nContent = *pSttIdx;
            else
            {
                BOOL bStt = FALSE;
                SwCntntNode* pCNd = pREnd->nNode.GetNode().GetCntntNode();
                if( !pCNd && 0 == ( pCNd = pDoc->GetNodes().GoPrevious( &pREnd->nNode )) )
                {
                    bStt = TRUE;
                    pREnd->nNode = rEnd;
                    if( 0 == ( pCNd = pDoc->GetNodes().GoNext( &pREnd->nNode )) )
                    {
                        pREnd->nNode = pRStt->nNode;
                        pCNd = pREnd->nNode.GetNode().GetCntntNode();
                    }
                }
                xub_StrLen nTmp = bStt ? 0 : pCNd->Len();
                pREnd->nContent.Assign( pCNd, nTmp );
            }
        }
    }
}

/*  */


// Aufbau vom Array: 2 longs,
//  1. Long enthaelt Type und Position im DocArray,
//  2. die ContentPosition
//
//  CntntType --
//          0x8000 = Bookmark Pos1
//          0x8001 = Bookmark Pos2
//          0x2000 = Absatzgebundener Rahmen
//          0x2001 = Auto-Absatzgebundener Rahmen, der umgehaengt werden soll
//          0x1000 = Redline Mark
//          0x1001 = Redline Point
//          0x0800 = Crsr aus der CrsrShell Mark
//          0x0801 = Crsr aus der CrsrShell Point
//          0x0400 = UnoCrsr Mark
//          0x0401 = UnoCrsr Point
//

class _SwSaveTypeCountContent
{
    union {
        struct { USHORT nType, nCount; } TC;
        ULONG nTypeCount;
        } TYPECOUNT;
    xub_StrLen nContent;

public:
    _SwSaveTypeCountContent() { TYPECOUNT.nTypeCount = 0; nContent = 0; }
    _SwSaveTypeCountContent( USHORT nType )
        {
            SetTypeAndCount( nType, 0 );
            nContent = 0;
        }
    _SwSaveTypeCountContent( const SvULongs& rArr, USHORT& rPos )
        {
            TYPECOUNT.nTypeCount = rArr[ rPos++ ];
            nContent = rArr[ rPos++ ];
        }
    void Add( SvULongs& rArr )
    {
        rArr.Insert( TYPECOUNT.nTypeCount, rArr.Count() );
        rArr.Insert( nContent, rArr.Count() );
    }

    void SetType( USHORT n )        { TYPECOUNT.TC.nType = n; }
    USHORT GetType() const          { return TYPECOUNT.TC.nType; }
    void IncType()                  { ++TYPECOUNT.TC.nType; }
    void DecType()                  { --TYPECOUNT.TC.nType; }

    void SetCount( USHORT n )       { TYPECOUNT.TC.nCount = n; }
    USHORT GetCount() const         { return TYPECOUNT.TC.nCount; }
    USHORT IncCount()               { return ++TYPECOUNT.TC.nCount; }
    USHORT DecCount()               { return --TYPECOUNT.TC.nCount; }

    void SetTypeAndCount( USHORT nT, USHORT nC )
        { TYPECOUNT.TC.nCount = nC; TYPECOUNT.TC.nType = nT; }

    void SetContent( xub_StrLen n )     { nContent = n; }
    xub_StrLen GetContent() const       { return nContent; }
};


void _ChkPaM( SvULongs& rSaveArr, ULONG nNode, xub_StrLen nCntnt,
                const SwPaM& rPam, _SwSaveTypeCountContent& rSave,
                BOOL bChkSelDirection )
{
    // SelektionsRichtung beachten
    BOOL bBound1IsStart = !bChkSelDirection ? TRUE :
                        ( *rPam.GetPoint() < *rPam.GetMark()
                            ? rPam.GetPoint() == &rPam.GetBound()
                            : rPam.GetMark() == &rPam.GetBound());

    const SwPosition* pPos = &rPam.GetBound( TRUE );
    if( pPos->nNode.GetIndex() == nNode &&
        ( bBound1IsStart ? pPos->nContent.GetIndex() < nCntnt
                            : pPos->nContent.GetIndex() <= nCntnt ))
    {
        rSave.SetContent( pPos->nContent.GetIndex() );
        rSave.Add( rSaveArr );
    }

    pPos = &rPam.GetBound( FALSE );
    if( pPos->nNode.GetIndex() == nNode &&
        ( (bBound1IsStart && bChkSelDirection)
                    ? pPos->nContent.GetIndex() <= nCntnt
                    : pPos->nContent.GetIndex() < nCntnt ))
    {
        rSave.SetContent( pPos->nContent.GetIndex() );
        rSave.IncType();
        rSave.Add( rSaveArr );
        rSave.DecType();
    }
}


void _SaveCntntIdx( SwDoc* pDoc, ULONG nNode, xub_StrLen nCntnt,
                    SvULongs& rSaveArr, BYTE nSaveFly )
{
    // 1. Bookmarks
    _SwSaveTypeCountContent aSave;
    aSave.SetTypeAndCount( 0x8000, 0 );

    const SwBookmarks& rBkmks = pDoc->GetBookmarks();
    for( ; aSave.GetCount() < rBkmks.Count(); aSave.IncCount() )
    {
        const SwBookmark* pBkmk = rBkmks[ aSave.GetCount() ];
        if( pBkmk->GetPos().nNode.GetIndex() == nNode &&
            pBkmk->GetPos().nContent.GetIndex() < nCntnt )
        {
            aSave.SetContent( pBkmk->GetPos().nContent.GetIndex() );
            aSave.Add( rSaveArr );
        }

        if( pBkmk->GetOtherPos() && pBkmk->GetOtherPos()->nNode.GetIndex() ==
            nNode && pBkmk->GetOtherPos()->nContent.GetIndex() < nCntnt )
        {
            aSave.SetContent( pBkmk->GetOtherPos()->nContent.GetIndex() );
            aSave.IncType();
            aSave.Add( rSaveArr );
            aSave.DecType();
        }
    }

    // 2. Redlines
    aSave.SetTypeAndCount( 0x1000, 0 );
    const SwRedlineTbl& rRedlTbl = pDoc->GetRedlineTbl();
    for( ; aSave.GetCount() < rRedlTbl.Count(); aSave.IncCount() )
    {
        const SwRedline* pRdl = rRedlTbl[ aSave.GetCount() ];
        if( pRdl->GetPoint()->nNode.GetIndex() == nNode &&
            pRdl->GetPoint()->nContent.GetIndex() < nCntnt )
        {
            aSave.SetContent( pRdl->GetPoint()->nContent.GetIndex() );
            aSave.IncType();
            aSave.Add( rSaveArr );
            aSave.DecType();
        }

        if( pRdl->HasMark() &&
            pRdl->GetMark()->nNode.GetIndex() == nNode &&
            pRdl->GetMark()->nContent.GetIndex() < nCntnt )
        {
            aSave.SetContent( pRdl->GetMark()->nContent.GetIndex() );
            aSave.Add( rSaveArr );
        }
    }

    // 4. Absatzgebundene Objekte
    {
        SwCntntNode *pNode = pDoc->GetNodes()[nNode]->GetCntntNode();
        if( pNode )
        {
            const SwPosition* pAPos;

            SwFrm* pFrm = pNode->GetFrm();
#ifdef DEBUG
            static BOOL bViaDoc = FALSE;
            if( bViaDoc )
                pFrm = NULL;
#endif
            if( pFrm ) // gibt es ein Layout? Dann ist etwas billiger...
            {
                if( pFrm->GetDrawObjs() )
                {
                    const SwDrawObjs& rDObj = *pFrm->GetDrawObjs();
                    for( USHORT n = rDObj.Count(); n; )
                    {
                        SdrObject *pObj = rDObj[ --n ];
                        SwFrmFmt* pFmt = ((SwContact*)GetUserCall(pObj))->GetFmt();
                        const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
                        if( ( ( nSaveFly && FLY_AT_CNTNT == rAnchor.GetAnchorId() ) ||
                              FLY_AUTO_CNTNT == rAnchor.GetAnchorId() ) &&
                            ( 0 != ( pAPos = rAnchor.GetCntntAnchor() ) ) )
                        {
                            aSave.SetType( 0x2000 );
                            aSave.SetContent( pAPos->nContent.GetIndex() );

                            ASSERT( nNode == pAPos->nNode.GetIndex(),
                                    "_SaveCntntIdx: Wrong Node-Index" );
                            if( FLY_AUTO_CNTNT == rAnchor.GetAnchorId() )
                            {
                                if( nCntnt <= aSave.GetContent() )
                                {
                                    if( SAVEFLY_SPLIT == nSaveFly )
                                        aSave.IncType(); // = 0x2001;
                                    else
                                        continue;
                                }
                            }
                            aSave.SetCount( pDoc->GetSpzFrmFmts()->Count() );
                            while( aSave.GetCount() &&
                                   pFmt != (*pDoc->GetSpzFrmFmts())[
                                                aSave.DecCount() ] )
                                ; // nothing
                            ASSERT( pFmt == (*pDoc->GetSpzFrmFmts())[
                                                    aSave.GetCount() ],
                                    "_SaveCntntIdx: Lost FrameFormat" );
                            aSave.Add( rSaveArr );
                        }
                    }
                }
            }
            else // Schade, kein Layout, dann ist es eben etwas teurer...
            {
                for( aSave.SetCount( pDoc->GetSpzFrmFmts()->Count() );
                        aSave.GetCount() ; )
                {
                    SwFrmFmt* pFrmFmt = (*pDoc->GetSpzFrmFmts())[
                                                aSave.DecCount() ];
                    if ( RES_FLYFRMFMT != pFrmFmt->Which() &&
                            RES_DRAWFRMFMT != pFrmFmt->Which() )
                        continue;

                    const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
                    if( ( FLY_AT_CNTNT == rAnchor.GetAnchorId() ||
                            FLY_AUTO_CNTNT == rAnchor.GetAnchorId() ) &&
                        0 != ( pAPos = rAnchor.GetCntntAnchor()) &&
                        nNode == pAPos->nNode.GetIndex() )
                    {
                        aSave.SetType( 0x2000 );
                        aSave.SetContent( pAPos->nContent.GetIndex() );
                        if( FLY_AUTO_CNTNT == rAnchor.GetAnchorId() )
                        {
                            if( nCntnt <= aSave.GetContent() )
                            {
                                if( SAVEFLY_SPLIT == nSaveFly )
                                    aSave.IncType(); // = 0x2001;
                                else
                                    continue;
                            }
                        }
                        aSave.Add( rSaveArr );
                    }
                }
            }
        }
    }
    // 5. CrsrShell
    {
        SwCrsrShell* pShell = pDoc->GetEditShell();
        if( pShell )
        {
            aSave.SetTypeAndCount( 0x800, 0 );
            FOREACHSHELL_START( pShell )
                register SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
                if( _pStkCrsr )
                do {
                    ::_ChkPaM( rSaveArr, nNode, nCntnt, *_pStkCrsr,
                                aSave, FALSE );
                    aSave.IncCount();
                } while ( (_pStkCrsr != 0 ) &&
                    ((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );

                FOREACHPAM_START( PCURSH->_GetCrsr() )
                    ::_ChkPaM( rSaveArr, nNode, nCntnt, *PCURCRSR,
                                aSave, FALSE );
                    aSave.IncCount();
                FOREACHPAM_END()

            FOREACHSHELL_END( pShell )
        }
    }
    // 6. UnoCrsr
    {
        aSave.SetTypeAndCount( 0x400, 0 );
        register const SwUnoCrsrTbl& rTbl = pDoc->GetUnoCrsrTbl();
        for( USHORT n = 0; n < rTbl.Count(); ++n )
        {
            FOREACHPAM_START( rTbl[ n ] )
                ::_ChkPaM( rSaveArr, nNode, nCntnt, *PCURCRSR, aSave, FALSE );
                aSave.IncCount();
            FOREACHPAM_END()

            SwUnoTableCrsr* pUnoTblCrsr = (SwUnoTableCrsr*)*rTbl[ n ];
            if( pUnoTblCrsr )
            {
                FOREACHPAM_START( &pUnoTblCrsr->GetSelRing() )
                    ::_ChkPaM( rSaveArr, nNode, nCntnt, *PCURCRSR, aSave, FALSE );
                    aSave.IncCount();
                FOREACHPAM_END()
            }
        }
    }
}


void _RestoreCntntIdx( SwDoc* pDoc, SvULongs& rSaveArr,
                        ULONG nNode, xub_StrLen nOffset, BOOL bAuto )
{
    SwCntntNode* pCNd = pDoc->GetNodes()[ nNode ]->GetCntntNode();
    const SwBookmarks& rBkmks = pDoc->GetBookmarks();
    const SwRedlineTbl& rRedlTbl = pDoc->GetRedlineTbl();
    SwSpzFrmFmts* pSpz = pDoc->GetSpzFrmFmts();
    USHORT n = 0;
    while( n < rSaveArr.Count() )
    {
        _SwSaveTypeCountContent aSave( rSaveArr, n );
        SwPosition* pPos = 0;
        switch( aSave.GetType() )
        {
        case 0x8000:
            pPos = (SwPosition*)&rBkmks[ aSave.GetCount() ]->GetPos();
            break;
        case 0x8001:
            pPos = (SwPosition*)rBkmks[ aSave.GetCount() ]->GetOtherPos();
            break;
        case 0x1001:
            pPos = (SwPosition*)rRedlTbl[ aSave.GetCount() ]->GetPoint();
            break;
        case 0x1000:
            pPos = (SwPosition*)rRedlTbl[ aSave.GetCount() ]->GetMark();
            break;
        case 0x2000:
            {
                SwFrmFmt *pFrmFmt = (*pSpz)[ aSave.GetCount() ];
                const SwFmtAnchor& rFlyAnchor = pFrmFmt->GetAnchor();
                if( rFlyAnchor.GetCntntAnchor() )
                {
                    SwFmtAnchor aNew( rFlyAnchor );
                    SwPosition aNewPos( *rFlyAnchor.GetCntntAnchor() );
                    aNewPos.nNode = *pCNd;
                    if( FLY_AUTO_CNTNT == rFlyAnchor.GetAnchorId() )
                        aNewPos.nContent.Assign( pCNd,
                                                 aSave.GetContent() + nOffset );
                    else
                        aNewPos.nContent.Assign( 0, 0 );
                    aNew.SetAnchor( &aNewPos );
                    pFrmFmt->SetAttr( aNew );
                }
            }
            break;
        case 0x2001:
            if( bAuto )
            {
                SwFrmFmt *pFrmFmt = (*pSpz)[ aSave.GetCount() ];
                SfxPoolItem *pAnchor = (SfxPoolItem*)&pFrmFmt->GetAnchor();
                pFrmFmt->SwModify::Modify( pAnchor, pAnchor );
            }
            break;

        case 0x0800:
        case 0x0801:
            {
                USHORT nCnt = 0;
                SwCrsrShell* pShell = pDoc->GetEditShell();
                if( pShell )
                {
                    FOREACHSHELL_START( pShell )
                        register SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
                        if( _pStkCrsr )
                        do {
                            if( aSave.GetCount() == nCnt )
                            {
                                pPos = &_pStkCrsr->GetBound( 0x0800 ==
                                                    aSave.GetType() );
                                break;
                            }
                            ++nCnt;
                        } while ( (_pStkCrsr != 0 ) &&
                            ((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );

                        if( pPos )
                            break;

                        FOREACHPAM_START( PCURSH->_GetCrsr() )
                            if( aSave.GetCount() == nCnt )
                            {
                                pPos = &PCURCRSR->GetBound( 0x0800 ==
                                                    aSave.GetType() );
                                break;
                            }
                            ++nCnt;
                        FOREACHPAM_END()
                        if( pPos )
                            break;

                    FOREACHSHELL_END( pShell )
                }
            }
            break;

        case 0x0400:
        case 0x0401:
            {
                USHORT nCnt = 0;
                register const SwUnoCrsrTbl& rTbl = pDoc->GetUnoCrsrTbl();
                for( USHORT i = 0; i < rTbl.Count(); ++i )
                {
                    FOREACHPAM_START( rTbl[ i ] )
                        if( aSave.GetCount() == nCnt )
                        {
                            pPos = &PCURCRSR->GetBound( 0x0400 ==
                                                    aSave.GetType() );
                            break;
                        }
                        ++nCnt;
                    FOREACHPAM_END()
                    if( pPos )
                        break;

                    SwUnoTableCrsr* pUnoTblCrsr = (SwUnoTableCrsr*)*rTbl[ i ];
                    if( pUnoTblCrsr )
                    {
                        FOREACHPAM_START( &pUnoTblCrsr->GetSelRing() )
                            if( aSave.GetCount() == nCnt )
                            {
                                pPos = &PCURCRSR->GetBound( 0x0400 ==
                                                    aSave.GetType() );
                                break;
                            }
                            ++nCnt;
                        FOREACHPAM_END()
                    }
                    if( pPos )
                        break;
                }
            }
            break;
        }

        if( pPos )
        {
            pPos->nNode = *pCNd;
            pPos->nContent.Assign( pCNd, aSave.GetContent() + nOffset );
        }
    }
}

void _RestoreCntntIdx( SvULongs& rSaveArr, const SwNode& rNd,
                        xub_StrLen nLen, xub_StrLen nChkLen )
{
    const SwDoc* pDoc = rNd.GetDoc();
    const SwBookmarks& rBkmks = pDoc->GetBookmarks();
    const SwRedlineTbl& rRedlTbl = pDoc->GetRedlineTbl();
    const SwSpzFrmFmts* pSpz = pDoc->GetSpzFrmFmts();
    SwCntntNode* pCNd = (SwCntntNode*)rNd.GetCntntNode();

    USHORT n = 0;
    while( n < rSaveArr.Count() )
    {
        _SwSaveTypeCountContent aSave( rSaveArr, n );
        if( aSave.GetContent() >= nChkLen )
            rSaveArr[ n + 1 ] -= nChkLen;
        else
        {
            SwPosition* pPos = 0;
            switch( aSave.GetType() )
            {
            case 0x8000:
                pPos = (SwPosition*)&rBkmks[ aSave.GetCount() ]->GetPos();
                break;
            case 0x8001:
                pPos = (SwPosition*)rBkmks[ aSave.GetCount() ]->GetOtherPos();
                break;
            case 0x1001:
                pPos = (SwPosition*)rRedlTbl[ aSave.GetCount() ]->GetPoint();
                break;
            case 0x1000:
                pPos = (SwPosition*)rRedlTbl[ aSave.GetCount() ]->GetMark();
                break;
            case 0x2000:
            case 0x2001:
                {
                    SwFrmFmt *pFrmFmt = (*pSpz)[ aSave.GetCount() ];
                    const SwFmtAnchor& rFlyAnchor = pFrmFmt->GetAnchor();
                    if( rFlyAnchor.GetCntntAnchor() )
                    {
                        SwFmtAnchor aNew( rFlyAnchor );
                        SwPosition aNewPos( *rFlyAnchor.GetCntntAnchor() );
                        aNewPos.nNode = rNd;
                        if( FLY_AUTO_CNTNT == rFlyAnchor.GetAnchorId() )
                            aNewPos.nContent.Assign( pCNd, Min(
                                                     aSave.GetContent(), nLen ) );
                        else
                            aNewPos.nContent.Assign( 0, 0 );
                        aNew.SetAnchor( &aNewPos );
                        pFrmFmt->SetAttr( aNew );
                    }
                }
                break;

            case 0x0800:
            case 0x0801:
                {
                    USHORT nCnt = 0;
                    SwCrsrShell* pShell = pDoc->GetEditShell();
                    if( pShell )
                    {
                        FOREACHSHELL_START( pShell )
                            register SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
                            if( _pStkCrsr )
                            do {
                                if( aSave.GetCount() == nCnt )
                                {
                                    pPos = &_pStkCrsr->GetBound( 0x0800 ==
                                                aSave.GetType() );
                                    break;
                                }
                                ++nCnt;
                            } while ( (_pStkCrsr != 0 ) &&
                                ((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );

                            if( pPos )
                                break;

                            FOREACHPAM_START( PCURSH->_GetCrsr() )
                                if( aSave.GetCount() == nCnt )
                                {
                                    pPos = &PCURCRSR->GetBound( 0x0800 ==
                                                aSave.GetType() );
                                    break;
                                }
                                ++nCnt;
                            FOREACHPAM_END()
                            if( pPos )
                                break;

                        FOREACHSHELL_END( pShell )
                    }
                }
                break;

            case 0x0400:
            case 0x0401:
                {
                    USHORT nCnt = 0;
                    register const SwUnoCrsrTbl& rTbl = pDoc->GetUnoCrsrTbl();
                    for( USHORT i = 0; i < rTbl.Count(); ++i )
                    {
                        FOREACHPAM_START( rTbl[ i ] )
                            if( aSave.GetCount() == nCnt )
                            {
                                pPos = &PCURCRSR->GetBound( 0x0400 ==
                                                    aSave.GetType() );
                                break;
                            }
                            ++nCnt;
                        FOREACHPAM_END()
                        if( pPos )
                            break;

                        SwUnoTableCrsr* pUnoTblCrsr = (SwUnoTableCrsr*)*rTbl[ i ];
                        if( pUnoTblCrsr )
                        {
                            FOREACHPAM_START( &pUnoTblCrsr->GetSelRing() )
                                if( aSave.GetCount() == nCnt )
                                {
                                    pPos = &PCURCRSR->GetBound( 0x0400 ==
                                                    aSave.GetType() );
                                    break;
                                }
                                ++nCnt;
                            FOREACHPAM_END()
                        }
                        if( pPos )
                            break;
                    }
                }
                break;
            }

            if( pPos )
            {
                pPos->nNode = rNd;
                pPos->nContent.Assign( pCNd, Min( aSave.GetContent(), nLen ) );
            }

            rSaveArr.Remove( n, 2 );
            n -= 2;
        }
    }
}


