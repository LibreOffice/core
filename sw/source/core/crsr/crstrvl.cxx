/*************************************************************************
 *
 *  $RCSfile: crstrvl.cxx,v $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
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
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>        // SwTxtFld
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>      // SwTxtFld
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _TXTINET_HXX //autogen
#include <txtinet.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _TXTTXMRK_HXX //autogen
#include <txttxmrk.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _CALLNK_HXX
#include <callnk.hxx>
#endif
#ifndef _DOCTXM_HXX
#include <doctxm.hxx>
#endif
#ifndef _DOCFLD_HXX
#include <docfld.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _REFFLD_HXX
#include <reffld.hxx>
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>       // SwTxtFld
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _PAGEDESC_HXX //autogen
#include <pagedesc.hxx>
#endif


// zum naechsten/vorhergehenden Punkt auf gleicher Ebene
FASTBOOL SwCrsrShell::GotoNextNum()
{
    FASTBOOL bRet = GetDoc()->GotoNextNum( *pCurCrsr->GetPoint() );
    if( bRet )
    {
        SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
        SwCrsrSaveState aSaveState( *pCurCrsr );
        if( !ActionPend() )
        {
            SET_CURR_SHELL( this );
            // dann versuche den Cursor auf die Position zu setzen,
            // auf halber Heohe vom Char-SRectangle
            Point aPt( pCurCrsr->GetPtPos() );
            SwCntntFrm * pFrm = pCurCrsr->GetCntntNode()->GetFrm( &aPt,
                                                        pCurCrsr->GetPoint() );
            pFrm->GetCharRect( aCharRect, *pCurCrsr->GetPoint() );
            aPt.Y() = aCharRect.Center().Y();
            pFrm->Calc();
            aPt.X() = pFrm->Frm().Left() + nUpDownX;
            pFrm->GetCrsrOfst( pCurCrsr->GetPoint(), aPt );
            bRet = !pCurCrsr->IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS );
            if( bRet )
                UpdateCrsr(SwCrsrShell::UPDOWN |
                        SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                        SwCrsrShell::READONLY );
        }
    }
    return bRet;
}


FASTBOOL SwCrsrShell::GotoPrevNum()
{
    FASTBOOL bRet = GetDoc()->GotoPrevNum( *pCurCrsr->GetPoint() );
    if( bRet )
    {
        SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
        SwCrsrSaveState aSaveState( *pCurCrsr );
        if( !ActionPend() )
        {
            SET_CURR_SHELL( this );
            // dann versuche den Cursor auf die Position zu setzen,
            // auf halber Heohe vom Char-SRectangle
            Point aPt( pCurCrsr->GetPtPos() );
            SwCntntFrm * pFrm = pCurCrsr->GetCntntNode()->GetFrm( &aPt,
                                                        pCurCrsr->GetPoint() );
            pFrm->GetCharRect( aCharRect, *pCurCrsr->GetPoint() );
            aPt.Y() = aCharRect.Center().Y();
            pFrm->Calc();
            aPt.X() = pFrm->Frm().Left() + nUpDownX;
            pFrm->GetCrsrOfst( pCurCrsr->GetPoint(), aPt );
            bRet = !pCurCrsr->IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS );
            if( bRet )
                UpdateCrsr(SwCrsrShell::UPDOWN |
                        SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                        SwCrsrShell::READONLY );
        }
    }
    return bRet;
}

// springe aus dem Content zum Header

FASTBOOL SwCrsrShell::GotoHeaderTxt()
{
    const SwFrm* pFrm = GetCurrFrm()->FindPageFrm();
    while( pFrm && !pFrm->IsHeaderFrm() )
        pFrm = pFrm->GetLower();
    // Header gefunden, dann suche den 1.Cntnt-Frame
    while( pFrm && !pFrm->IsCntntFrm() )
        pFrm = pFrm->GetLower();
    if( pFrm )
    {
        SET_CURR_SHELL( this );
        // hole den Header-Frame
        SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
        SwCrsrSaveState aSaveState( *pCurCrsr );
        pFrm->Calc();
        Point aPt( pFrm->Frm().Pos() + pFrm->Prt().Pos() );
        pFrm->GetCrsrOfst( pCurCrsr->GetPoint(), aPt );
        if( !pCurCrsr->IsSelOvr() )
        {
#ifdef USED
    // wenn wir immer auf der akt. Seite bleiben wollen, dann benutzen.
    // zur Zeit immer zur naechst gelegene !!
                // diese Seite, diese Fusszeile !!
                // diese Seite, diese Kopfzeile !!
            pCurCrsr->GetPtPos() = aPt;
#endif
            UpdateCrsr();
        }
        else
            pFrm = 0;
    }
    return 0 != pFrm;
}


// springe aus dem Content zum Footer

FASTBOOL SwCrsrShell::GotoFooterTxt()
{
    const SwPageFrm* pFrm = GetCurrFrm()->FindPageFrm();
    if( pFrm )
    {
        const SwFrm * pLower = pFrm->GetLower();
        while( pLower->GetNext() )
            pLower = pLower->GetNext();

        while( pLower && !pLower->IsFooterFrm() )
            pLower = pLower->GetLower();
        // Header gefunden, dann suche den 1.Cntnt-Frame
        while( pLower && !pLower->IsCntntFrm() )
            pLower = pLower->GetLower();

        if( pLower )
        {
            SET_CURR_SHELL( this );
            // hole eine Position im Footer
            SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
            SwCrsrSaveState aSaveState( *pCurCrsr );
            pLower->Calc();
            Point aPt( pLower->Frm().Pos() + pLower->Prt().Pos() );
            pLower->GetCrsrOfst( pCurCrsr->GetPoint(), aPt );
            if( !pCurCrsr->IsSelOvr() )
            {
#ifdef USED
    // wenn wir immer auf der akt. Seite bleiben wollen, dann benutzen.
    // zur Zeit immer zur naechst gelegene !!
                // diese Seite, diese Fusszeile !!
                pCurCrsr->GetPtPos() = aPt;
#endif
                UpdateCrsr();
            }
            else
                pFrm = 0;
        }
        else
            pFrm = 0;
    }
    else
        pFrm = 0;
    return 0 != pFrm;
}

FASTBOOL SwCrsrShell::SetCrsrInHdFt( USHORT nDescNo, FASTBOOL bInHeader )
{
    FASTBOOL bRet = FALSE;
    SwDoc *pDoc = GetDoc();

    SET_CURR_SHELL( this );

    if( USHRT_MAX == nDescNo )
    {
        // dann den akt. nehmen
        const SwPageFrm* pPage = GetCurrFrm()->FindPageFrm();
        if( pPage )
            for( USHORT i = 0; i < pDoc->GetPageDescCnt(); ++i )
                if( pPage->GetPageDesc() == &pDoc->GetPageDesc( i ) )
                {
                    nDescNo = i;
                    break;
                }
    }

    if( USHRT_MAX != nDescNo && nDescNo < pDoc->GetPageDescCnt() )
    {
        //dann teste mal, ob ueberhaupt das Attribut vorhanden ist.
        const SwPageDesc& rDesc = pDoc->GetPageDesc( nDescNo );
        const SwFmtCntnt* pCnt = 0;
        if( bInHeader )
        {
            // gespiegelte Seiten??? erstmal nicht beachten
            const SwFmtHeader& rHd = rDesc.GetMaster().GetHeader();
            if( rHd.GetHeaderFmt() )
                pCnt = &rHd.GetHeaderFmt()->GetCntnt();
        }
        else
        {
            const SwFmtFooter& rFt = rDesc.GetMaster().GetFooter();
            if( rFt.GetFooterFmt() )
                pCnt = &rFt.GetFooterFmt()->GetCntnt();
        }

        if( pCnt && pCnt->GetCntntIdx() )
        {
            SwNodeIndex aIdx( *pCnt->GetCntntIdx(), 1 );
            SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
            if( !pCNd )
                pCNd = pDoc->GetNodes().GoNext( &aIdx );

            const SwFrm* pFrm;
            Point aPt( pCurCrsr->GetPtPos() );

            if( pCNd && 0 != ( pFrm = pCNd->GetFrm( &aPt, 0, FALSE ) ))
            {
                // dann kann der Cursor ja auch hinein gesetzt werden
                SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
                SwCrsrSaveState aSaveState( *pCurCrsr );

                ClearMark();

                SwPosition& rPos = *pCurCrsr->GetPoint();
                rPos.nNode = *pCNd;
                rPos.nContent.Assign( pCNd, 0 );

                bRet = !pCurCrsr->IsSelOvr();
                if( bRet )
                    UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                                SwCrsrShell::READONLY );
            }
        }
    }
    return bRet;
}

// springe zum naechsten Verzeichnis

FASTBOOL SwCrsrShell::GotoNextTOXBase( const String* pName )
{
    FASTBOOL bRet = FALSE;

    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    SwCntntNode* pFnd = 0;
    for( USHORT n = rFmts.Count(); n; )
    {
        const SwSection* pSect = rFmts[ --n ]->GetSection();
        const SwSectionNode* pSectNd;
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            0 != ( pSectNd = pSect->GetFmt()->GetSectionNode() ) &&
             pCurCrsr->GetPoint()->nNode < pSectNd->GetIndex() &&
            ( !pFnd || pFnd->GetIndex() > pSectNd->GetIndex() ) &&
// JP 10.12.96: solange wir nur 3 Typen kennen und UI-seitig keine anderen
//              einstellbar sind, muss ueber den Titel gesucht werden!
//          ( !pName || *pName == ((SwTOXBaseSection*)pSect)->GetTypeName() ) &&
            ( !pName || *pName == ((SwTOXBaseSection*)pSect)->GetTOXName() )
            )
        {
            SwNodeIndex aIdx( *pSectNd, 1 );
            SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
            if( !pCNd )
                pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
            const SwCntntFrm* pCFrm;
            if( pCNd &&
                pCNd->EndOfSectionIndex() <= pSectNd->EndOfSectionIndex() &&
                0 != ( pCFrm = pCNd->GetFrm() ) &&
                ( IsReadOnlyAvailable() || !pCFrm->IsProtected() ))
            {
                pFnd = pCNd;
            }
        }
    }
    if( pFnd )
    {
        SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
        SwCrsrSaveState aSaveState( *pCurCrsr );
        pCurCrsr->GetPoint()->nNode = *pFnd;
        pCurCrsr->GetPoint()->nContent.Assign( pFnd, 0 );
        bRet = !pCurCrsr->IsSelOvr();
        if( bRet )
            UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    }
    return bRet;
}

// springe zum vorherigen Verzeichnis


FASTBOOL SwCrsrShell::GotoPrevTOXBase( const String* pName )
{
    FASTBOOL bRet = FALSE;

    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    SwCntntNode* pFnd = 0;
    for( USHORT n = rFmts.Count(); n; )
    {
        const SwSection* pSect = rFmts[ --n ]->GetSection();
        const SwSectionNode* pSectNd;
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            0 != ( pSectNd = pSect->GetFmt()->GetSectionNode() ) &&
            pCurCrsr->GetPoint()->nNode > pSectNd->EndOfSectionIndex() &&
            ( !pFnd || pFnd->GetIndex() < pSectNd->GetIndex() ) &&
// JP 10.12.96: solange wir nur 3 Typen kennen und UI-seitig keine anderen
//              einstellbar sind, muss ueber den Titel gesucht werden!
//          ( !pName || *pName == ((SwTOXBaseSection*)pSect)->GetTypeName() ) &&
            ( !pName || *pName == ((SwTOXBaseSection*)pSect)->GetTOXName() )
            )
        {
            SwNodeIndex aIdx( *pSectNd, 1 );
            SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
            if( !pCNd )
                pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
            const SwCntntFrm* pCFrm;
            if( pCNd &&
                pCNd->EndOfSectionIndex() <= pSectNd->EndOfSectionIndex() &&
                0 != ( pCFrm = pCNd->GetFrm() ) &&
                ( IsReadOnlyAvailable() || !pCFrm->IsProtected() ))
            {
                pFnd = pCNd;
            }
        }
    }

    if( pFnd )
    {
        SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
        SwCrsrSaveState aSaveState( *pCurCrsr );
        pCurCrsr->GetPoint()->nNode = *pFnd;
        pCurCrsr->GetPoint()->nContent.Assign( pFnd, 0 );
        bRet = !pCurCrsr->IsSelOvr();
        if( bRet )
            UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    }
    return bRet;
}

// springe zum Verzeichnis vom TOXMark

FASTBOOL SwCrsrShell::GotoTOXMarkBase()
{
    FASTBOOL bRet = FALSE;

    SwTOXMarks aMarks;
    USHORT nCnt = GetDoc()->GetCurTOXMark( *pCurCrsr->GetPoint(), aMarks );
    if( nCnt )
    {
        // dann nehme den 1. und hole den Verzeichnis-Typ.
        // Suche in seiner Abhaengigkeitsliste nach dem eigentlichem
        // Verzeichnis
        SwModify* pType = (SwModify*)aMarks[0]->GetRegisteredIn();
        SwClientIter aIter( *pType );
        const SwSectionNode* pSectNd;
        const SwSectionFmt* pSectFmt;

        for( SwTOXBase* pTOX =
            (SwTOXBase*)aIter.First( TYPE( SwTOXBase ));
                pTOX; pTOX = (SwTOXBase*)aIter.Next() )
            if( pTOX->ISA( SwTOXBaseSection ) &&
                0 != ( pSectFmt = ((SwTOXBaseSection*)pTOX)->GetFmt() ) &&
                0 != ( pSectNd = pSectFmt->GetSectionNode() ))
            {
                SwNodeIndex aIdx( *pSectNd, 1 );
                SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
                if( !pCNd )
                    pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
                const SwCntntFrm* pCFrm;
                if( pCNd &&
                    pCNd->EndOfSectionIndex() < pSectNd->EndOfSectionIndex() &&
                    0 != ( pCFrm = pCNd->GetFrm() ) &&
                    ( IsReadOnlyAvailable() || !pCFrm->IsProtected() ))
                {
                    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
                    SwCrsrSaveState aSaveState( *pCurCrsr );
                    pCurCrsr->GetPoint()->nNode = *pCNd;
                    pCurCrsr->GetPoint()->nContent.Assign( pCNd, 0 );
                    bRet = !pCurCrsr->IsInProtectTable() &&
                            !pCurCrsr->IsSelOvr();
                    if( bRet )
                        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
                    break;
                }
            }
    }
    return bRet;
}


    // springe zur naechsten (vorherigen) Tabellenformel
    // optional auch nur zu kaputten Formeln springen
FASTBOOL SwCrsrShell::GotoNxtPrvTblFormula( BOOL bNext, BOOL bOnlyErrors )
{
    if( IsTableMode() )
        return FALSE;

    FASTBOOL bFnd = FALSE;
    SwPosition& rPos = *pCurCrsr->GetPoint();

    Point aPt;
    SwPosition aFndPos( GetDoc()->GetNodes().GetEndOfContent() );
    if( !bNext )
        aFndPos.nNode = 0;
    _SetGetExpFld aFndGEF( aFndPos ), aCurGEF( rPos );

    {
        const SwNode* pSttNd = rPos.nNode.GetNode().FindTableBoxStartNode();
        if( pSttNd )
        {
            const SwTableBox* pTBox = pSttNd->FindTableNode()->GetTable().
                                        GetTblBox( pSttNd->GetIndex() );
            if( pTBox )
                aCurGEF = _SetGetExpFld( *pTBox );
        }
    }

    if( rPos.nNode < GetDoc()->GetNodes().GetEndOfExtras() )
        // auch beim Einsammeln wird nur der erste Frame benutzt!
        aCurGEF.SetBodyPos( *rPos.nNode.GetNode().GetCntntNode()->GetFrm(
                                &aPt, &rPos, FALSE ) );

    {
        const SfxPoolItem* pItem;
        const SwTableBox* pTBox;
        USHORT n, nMaxItems = GetDoc()->GetAttrPool().GetItemCount( RES_BOXATR_FORMULA );

        for( n = 0; n < nMaxItems; ++n )
            if( 0 != (pItem = GetDoc()->GetAttrPool().GetItem(
                                        RES_BOXATR_FORMULA, n ) ) &&
                0 != (pTBox = ((SwTblBoxFormula*)pItem)->GetTableBox() ) &&
                pTBox->GetSttNd() &&
                pTBox->GetSttNd()->GetNodes().IsDocNodes() &&
                ( !bOnlyErrors ||
                  !((SwTblBoxFormula*)pItem)->HasValidBoxes() ) )
            {
                const SwCntntFrm* pCFrm;
                SwNodeIndex aIdx( *pTBox->GetSttNd() );
                const SwCntntNode* pCNd = GetDoc()->GetNodes().GoNext( &aIdx );
                if( pCNd && 0 != ( pCFrm = pCNd->GetFrm( &aPt, 0, FALSE ) ) &&
                    (IsReadOnlyAvailable() || !pCFrm->IsProtected() ))
                {
                    _SetGetExpFld aCmp( *pTBox );
                    aCmp.SetBodyPos( *pCFrm );

                    if( bNext ? ( aCurGEF < aCmp && aCmp < aFndGEF )
                              : ( aCmp < aCurGEF && aFndGEF < aCmp ))
                    {
                        aFndGEF = aCmp;
                        bFnd = TRUE;
                    }
                }
            }
    }

    if( bFnd )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
        SwCrsrSaveState aSaveState( *pCurCrsr );

        aFndGEF.GetPosOfContent( rPos );
        pCurCrsr->DeleteMark();

        bFnd = !pCurCrsr->IsSelOvr();
        if( bFnd )
            UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                        SwCrsrShell::READONLY );
    }
    return bFnd;
}

// springe zum naechsten (vorherigen) Verzeichniseintrag
FASTBOOL SwCrsrShell::GotoNxtPrvTOXMark( BOOL bNext )
{
    if( IsTableMode() )
        return FALSE;

    FASTBOOL bFnd = FALSE;
    SwPosition& rPos = *pCurCrsr->GetPoint();

    Point aPt;
    SwPosition aFndPos( GetDoc()->GetNodes().GetEndOfContent() );
    if( !bNext )
        aFndPos.nNode = 0;
    _SetGetExpFld aFndGEF( aFndPos ), aCurGEF( rPos );

    if( rPos.nNode.GetIndex() < GetDoc()->GetNodes().GetEndOfExtras().GetIndex() )
        // auch beim Einsammeln wird nur der erste Frame benutzt!
        aCurGEF.SetBodyPos( *rPos.nNode.GetNode().
                        GetCntntNode()->GetFrm( &aPt, &rPos, FALSE ) );

    {
        const SfxPoolItem* pItem;
        const SwCntntFrm* pCFrm;
        const SwTxtNode* pTxtNd;
        const SwTxtTOXMark* pTxtTOX;
        USHORT n, nMaxItems = GetDoc()->GetAttrPool().GetItemCount( RES_TXTATR_TOXMARK );

        for( n = 0; n < nMaxItems; ++n )
            if( 0 != (pItem = GetDoc()->GetAttrPool().GetItem(
                                        RES_TXTATR_TOXMARK, n ) ) &&
                0 != (pTxtTOX = ((SwTOXMark*)pItem)->GetTxtTOXMark() ) &&
                ( pTxtNd = &pTxtTOX->GetTxtNode())->GetNodes().IsDocNodes() &&
                0 != ( pCFrm = pTxtNd->GetFrm( &aPt, 0, FALSE )) &&
                ( IsReadOnlyAvailable() || !pCFrm->IsProtected() ))
            {
                SwNodeIndex aNdIndex( *pTxtNd );    // UNIX benoetigt dieses Obj.
                _SetGetExpFld aCmp( aNdIndex, *pTxtTOX, 0 );
                aCmp.SetBodyPos( *pCFrm );

                if( bNext ? ( aCurGEF < aCmp && aCmp < aFndGEF )
                          : ( aCmp < aCurGEF && aFndGEF < aCmp ))
                {
                    aFndGEF = aCmp;
                    bFnd = TRUE;
                }
            }
    }

    if( bFnd )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
        SwCrsrSaveState aSaveState( *pCurCrsr );

        aFndGEF.GetPosOfContent( rPos );

        bFnd = !pCurCrsr->IsSelOvr();
        if( bFnd )
            UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                        SwCrsrShell::READONLY );
    }
    return bFnd;
}

/*--------------------------------------------------------------------
     Beschreibung: Traveling zwischen Markierungen
 --------------------------------------------------------------------*/

const SwTOXMark& SwCrsrShell::GotoTOXMark( const SwTOXMark& rStart,
                                            SwTOXSearch eDir )
{
    SET_CURR_SHELL( this );
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    SwCrsrSaveState aSaveState( *pCurCrsr );

    const SwTOXMark& rNewMark = GetDoc()->GotoTOXMark( rStart, eDir,
                                                    IsReadOnlyAvailable() );
    // Position setzen
    SwPosition& rPos = *GetCrsr()->GetPoint();
    rPos.nNode = rNewMark.GetTxtTOXMark()->GetTxtNode();
    rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(),
                         *rNewMark.GetTxtTOXMark()->GetStart() );

    if( !pCurCrsr->IsSelOvr() )
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY );

    return rNewMark;
}

// springe zum naechsten / vorherigen FeldTypen

void lcl_MakeFldLst( _SetGetExpFlds& rLst, const SwFieldType& rFldType,
                        USHORT nSubType, BOOL bInReadOnly,
                        BOOL bChkInpFlag = FALSE )
{
    // es muss immer der 1. Frame gesucht werden
    Point aPt;
    SwTxtFld* pTxtFld;
    SwClientIter aIter( (SwFieldType&)rFldType );
    BOOL bSubType = nSubType != USHRT_MAX;
    for( SwClient* pLast = aIter.First( TYPE( SwFmtFld )); pLast; pLast = aIter.Next() )
        if( 0 != ( pTxtFld = ((SwFmtFld*)pLast)->GetTxtFld() ) &&
            ( !bChkInpFlag || ((SwSetExpField*)pTxtFld->GetFld().GetFld())
                                ->GetInputFlag() ) &&
            (!bSubType || (((SwFmtFld*)pLast)->GetFld()->GetSubType()
                                & 0xff ) == nSubType ))
        {
            SwCntntFrm* pCFrm;
            const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();
            if( 0 != ( pCFrm = rTxtNode.GetFrm( &aPt, 0, FALSE )) &&
                ( bInReadOnly || !pCFrm->IsProtected() ))
            {
                _SetGetExpFld* pNew = new _SetGetExpFld(
                                SwNodeIndex( rTxtNode ), pTxtFld );
                pNew->SetBodyPos( *pCFrm );
                rLst.Insert( pNew );
            }
        }
}


FASTBOOL SwCrsrShell::MoveFldType( const SwFieldType* pFldType, BOOL bNext,
                                            USHORT nSubType, USHORT nResType )
{
    // sortierte Liste aller Felder
    _SetGetExpFlds aSrtLst( 64 );

    if (pFldType)
    {
        if( RES_INPUTFLD != pFldType->Which() && !pFldType->GetDepends() )
            return FALSE;

        // Modify-Object gefunden, trage alle Felder ins Array ein
        ::lcl_MakeFldLst( aSrtLst, *pFldType, nSubType, IsReadOnlyAvailable() );

        if( RES_INPUTFLD == pFldType->Which() )
        {
            // es gibt noch versteckte InputFelder in den SetExp. Feldern
            const SwFldTypes& rFldTypes = *pDoc->GetFldTypes();
            const USHORT nSize = rFldTypes.Count();

            // Alle Typen abklappern
            for( USHORT i=0; i < nSize; ++i )
                if( RES_SETEXPFLD == ( pFldType = rFldTypes[ i ] )->Which() )
                    ::lcl_MakeFldLst( aSrtLst, *pFldType, nSubType,
                                IsReadOnlyAvailable(), TRUE );
        }
    }
    else
    {
        const SwFldTypes& rFldTypes = *pDoc->GetFldTypes();
        const USHORT nSize = rFldTypes.Count();

        // Alle Typen abklappern
        for( USHORT i=0; i < nSize; ++i )
            if( nResType == ( pFldType = rFldTypes[ i ] )->Which() )
                ::lcl_MakeFldLst( aSrtLst, *pFldType, nSubType,
                                IsReadOnlyAvailable() );
    }

    // keine Felder gefunden?
    if( !aSrtLst.Count() )
        return FALSE;

    USHORT nPos;
    SwCursor* pCrsr = IsTableMode() ? pTblCrsr : pCurCrsr;
    {
        // JP 19.08.98: es muss immer ueber das Feld gesucht werden, damit
        //              auch immer das richtige gefunden wird, wenn welche in
        //              Rahmen stehen, die in einem Absatz verankert sind,
        //              in dem ein Feld steht - siehe auch Bug 55247
        const SwPosition& rPos = *pCrsr->GetPoint();

        SwTxtNode* pTNd = rPos.nNode.GetNode().GetTxtNode();
        ASSERT( pTNd, "Wo ist mein CntntNode?" );

        SwTxtFld* pTxtFld = (SwTxtFld*)pTNd->GetTxtAttr( rPos.nContent,
                                                        RES_TXTATR_FIELD );
        BOOL bDelFld = 0 == pTxtFld;
        if( bDelFld )
        {
            SwFmtFld* pFmtFld = new SwFmtFld( SwDateTimeField(
                (SwDateTimeFieldType*)pDoc->GetSysFldType( RES_DATETIMEFLD ) ) );

            pTxtFld = new SwTxtFld( *pFmtFld, rPos.nContent.GetIndex() );
            pTxtFld->ChgTxtNode( pTNd );
        }

        _SetGetExpFld aSrch( rPos.nNode, pTxtFld, &rPos.nContent );
        if( rPos.nNode.GetIndex() < pDoc->GetNodes().GetEndOfExtras().GetIndex() )
        {
            // auch beim Einsammeln wird nur der erste Frame benutzt!
            Point aPt;
            aSrch.SetBodyPos( *pTNd->GetFrm( &aPt, &rPos, FALSE ) );
        }

        BOOL bFound = aSrtLst.Seek_Entry( &aSrch, &nPos );
        if( bDelFld )
        {
            delete (SwFmtFld*)&pTxtFld->GetAttr();
            delete pTxtFld;
        }

        if( bFound )        // stehe auf einem ?
        {
            if( bNext )
            {
                if( ++nPos >= aSrtLst.Count() )
                    return FALSE;                   // schon am Ende
            }
            else if( !nPos-- )
                return FALSE;       // weiter nach vorne geht nicht
        }
        else if( bNext ? nPos >= aSrtLst.Count() : !nPos--)
            return FALSE;
    }
    const _SetGetExpFld& rFnd = **( aSrtLst.GetData() + nPos );


    SET_CURR_SHELL( this );
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    SwCrsrSaveState aSaveState( *pCrsr );

    rFnd.GetPosOfContent(  *pCrsr->GetPoint() );

    FASTBOOL bRet = !pCurCrsr->IsSelOvr();
    if( bRet )
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return bRet;
}


FASTBOOL SwCrsrShell::GotoFld( const SwFmtFld& rFld )
{
    FASTBOOL bRet = FALSE;
    if( rFld.GetTxtFld() )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen

        SwCursor* pCrsr = IsTableMode() ? pTblCrsr : pCurCrsr;
        SwCrsrSaveState aSaveState( *pCrsr );

        SwTxtNode* pTNd = (SwTxtNode*)rFld.GetTxtFld()->GetpTxtNode();
        pCrsr->GetPoint()->nNode = *pTNd;
        pCrsr->GetPoint()->nContent.Assign( pTNd, *rFld.GetTxtFld()->GetStart() );

        bRet = !pCrsr->IsSelOvr();
        if( bRet )
            UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    }
    return bRet;
}


void SwCrsrShell::GotoOutline( USHORT nIdx )
{
    SwCursor* pCrsr = IsTableMode() ? pTblCrsr : pCurCrsr;

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    SwCrsrSaveState aSaveState( *pCrsr );

    const SwNodes& rNds = GetDoc()->GetNodes();
    SwTxtNode* pTxtNd = (SwTxtNode*)rNds.GetOutLineNds()[ nIdx ]->GetTxtNode();
    pCrsr->GetPoint()->nNode = *pTxtNd;
    pCrsr->GetPoint()->nContent.Assign( pTxtNd, 0 );

    if( !pCrsr->IsSelOvr() )
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
}


FASTBOOL SwCrsrShell::GotoOutline( const String& rName )
{
    SwCursor* pCrsr = IsTableMode() ? pTblCrsr : pCurCrsr;

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    SwCrsrSaveState aSaveState( *pCrsr );

    FASTBOOL bRet = FALSE;
    if( pDoc->GotoOutline( *pCrsr->GetPoint(), rName ) && !pCrsr->IsSelOvr() )
    {
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
        bRet = TRUE;
    }
    return bRet;
}



FASTBOOL SwCrsrShell::GotoNextOutline()         // naechster Node mit Outline-Num.
{
    SwCursor* pCrsr = IsTableMode() ? pTblCrsr : pCurCrsr;
    const SwNodes& rNds = GetDoc()->GetNodes();

    SwNode* pNd = pCrsr->GetNode();
    USHORT nPos;
    if( rNds.GetOutLineNds().Seek_Entry( pNd, &nPos ))
        ++nPos;

    if( nPos == rNds.GetOutLineNds().Count() )
        return FALSE;

    pNd = rNds.GetOutLineNds()[ nPos ];

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    SwCrsrSaveState aSaveState( *pCrsr );
    pCrsr->GetPoint()->nNode = *pNd;
    pCrsr->GetPoint()->nContent.Assign( (SwTxtNode*)pNd, 0 );

    FASTBOOL bRet = !pCrsr->IsSelOvr();
    if( bRet )
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return bRet;
}


FASTBOOL SwCrsrShell::GotoPrevOutline()         // vorheriger Node mit Outline-Num.
{
    SwCursor* pCrsr = IsTableMode() ? pTblCrsr : pCurCrsr;
    const SwNodes& rNds = GetDoc()->GetNodes();

    SwNode* pNd = pCrsr->GetNode();
    USHORT nPos;
    rNds.GetOutLineNds().Seek_Entry( pNd, &nPos );

    FASTBOOL bRet = FALSE;
    if( nPos )
    {
        --nPos; // davor

        pNd = rNds.GetOutLineNds()[ nPos ];
        if( pNd->GetIndex() > pCrsr->GetPoint()->nNode.GetIndex() )
            return FALSE;

        SET_CURR_SHELL( this );
        SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
        SwCrsrSaveState aSaveState( *pCrsr );
        pCrsr->GetPoint()->nNode = *pNd;
        pCrsr->GetPoint()->nContent.Assign( (SwTxtNode*)pNd, 0 );

        bRet = !pCrsr->IsSelOvr();
        if( bRet )
            UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    }
    return bRet;
}


    // suche die "Outline-Position" vom vorherigen Outline-Node mit dem
    // Level.
USHORT SwCrsrShell::GetOutlinePos( BYTE nLevel )
{
    SwPaM* pCrsr = IsTableMode() ? pTblCrsr : pCurCrsr;
    const SwNodes& rNds = GetDoc()->GetNodes();

    SwNode* pNd = pCrsr->GetNode();
    USHORT nPos;
    if( rNds.GetOutLineNds().Seek_Entry( pNd, &nPos ))
        nPos++;         // steht auf der Position, fuers while zum Naechsten

    while( nPos-- )     // immer den davor testen !
    {
        pNd = rNds.GetOutLineNds()[ nPos ];
        if( ((SwTxtNode*)pNd)->GetTxtColl()->GetOutlineLevel() <= nLevel )
            return nPos;
    }
    return USHRT_MAX;       // davor keiner mehr also Ende
}


FASTBOOL SwCrsrShell::MakeOutlineSel( USHORT nSttPos, USHORT nEndPos,
                                    BOOL bWithChilds )
{
    const SwNodes& rNds = GetDoc()->GetNodes();
    const SwOutlineNodes& rOutlNds = rNds.GetOutLineNds();
    if( !rOutlNds.Count() )     // wie jetzt ???
        return FALSE;

    SET_CURR_SHELL( this );
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen

    if( nSttPos > nEndPos )         // sollte jemand das vertauscht haben?
    {
        ASSERT( !this, "Start- > Ende-Position im Array" );
        USHORT nTmp = nSttPos;
        nSttPos = nEndPos;
        nEndPos = nTmp;
    }

    SwNode* pSttNd = rOutlNds[ nSttPos ];
    SwNode* pEndNd = rOutlNds[ nEndPos ];

    if( bWithChilds )
    {
        BYTE nLevel = pEndNd->GetTxtNode()->GetTxtColl()->GetOutlineLevel();
        for( ++nEndPos; nEndPos < rOutlNds.Count(); ++nEndPos )
        {
            pEndNd = rOutlNds[ nEndPos ];
            BYTE nNxtLevel = pEndNd->GetTxtNode()->GetTxtColl()->GetOutlineLevel();
            if( nNxtLevel <= nLevel )
                break;          // EndPos steht jetzt auf dem naechsten
        }
    }
    // ohne Childs, dann aber zumindest auf den naechsten
    else if( ++nEndPos < rOutlNds.Count() )
        pEndNd = rOutlNds[ nEndPos ];

    if( nEndPos == rOutlNds.Count() )       // kein Ende gefunden
        pEndNd = &rNds.GetEndOfContent();

    KillPams();

    SwCrsrSaveState aSaveState( *pCurCrsr );

    // Jetzt das Ende ans Ende vom voherigen ContentNode setzen
    pCurCrsr->GetPoint()->nNode = *pSttNd;
    pCurCrsr->GetPoint()->nContent.Assign( pSttNd->GetCntntNode(), 0 );
    pCurCrsr->SetMark();
    pCurCrsr->GetPoint()->nNode = *pEndNd;
    pCurCrsr->Move( fnMoveBackward, fnGoNode );     // ans Ende vom Vorgaenger

    // und schon ist alles selektiert
    FASTBOOL bRet = !pCurCrsr->IsSelOvr();
    if( bRet )
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return bRet;
}


// springe zu dieser Refmark
FASTBOOL SwCrsrShell::GotoRefMark( const String& rRefMark, USHORT nSubType,
                                    USHORT nSeqNo )
{
    SET_CURR_SHELL( this );
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    SwCrsrSaveState aSaveState( *pCurCrsr );

    USHORT nPos;
    SwTxtNode* pTxtNd = SwGetRefFieldType::FindAnchor( GetDoc(), rRefMark,
                                                    nSubType, nSeqNo, &nPos );
    if( pTxtNd )
    {
        pCurCrsr->GetPoint()->nNode = *pTxtNd;
        pCurCrsr->GetPoint()->nContent.Assign( pTxtNd, nPos );

        if( !pCurCrsr->IsSelOvr() )
        {
            UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
            return TRUE;
        }
    }
    return FALSE;
}


FASTBOOL SwCrsrShell::GetContentAtPos( const Point& rPt,
                                        SwContentAtPos& rCntntAtPos,
                                        FASTBOOL bSetCrsr,
                                        SwRect* pFldRect )
{
    SET_CURR_SHELL( this );
    FASTBOOL bRet = FALSE;

    if( !IsTableMode() )
    {
        Point aPt( rPt );
        SwPosition aPos( *pCurCrsr->GetPoint() );

        SwTxtNode* pTxtNd;
        SwCntntFrm *pFrm;
        SwTxtAttr* pTxtAttr;
        SwCrsrMoveState aTmpState;
        aTmpState.bFieldInfo = TRUE;
        aTmpState.bExactOnly = !( SwContentAtPos::SW_OUTLINE &
                                    rCntntAtPos.eCntntAtPos );
        aTmpState.bSetInReadOnly = IsReadOnlyAvailable();

        BOOL bCrsrFoundExact = GetLayout()->GetCrsrOfst( &aPos, aPt, &aTmpState );
        pTxtNd = aPos.nNode.GetNode().GetTxtNode();

        const SwNodes& rNds = GetDoc()->GetNodes();
        if( pTxtNd && SwContentAtPos::SW_OUTLINE & rCntntAtPos.eCntntAtPos
            && rNds.GetOutLineNds().Count() )
        {
            const SwTxtNode* pONd = pTxtNd->FindOutlineNodeOfLevel( MAXLEVEL-1);
            if( pONd )
            {
                rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_OUTLINE;
                rCntntAtPos.sStr = pONd->GetExpandTxt( 0, STRING_LEN, TRUE );
                bRet = TRUE;
            }
        }
        else if( bCrsrFoundExact && pTxtNd )
        {
            if( !aTmpState.bPosCorr )
            {
                if( ( SwContentAtPos::SW_FIELD | SwContentAtPos::SW_CLICKFIELD )
                    & rCntntAtPos.eCntntAtPos && !aTmpState.bFtnNoInfo )
                {
                    pTxtAttr = pTxtNd->GetTxtAttr( aPos.nContent.GetIndex(),
                                                    RES_TXTATR_FIELD );
                    const SwField* pFld = pTxtAttr
                                            ? pTxtAttr->GetFld().GetFld()
                                            : 0;
                    if( SwContentAtPos::SW_CLICKFIELD & rCntntAtPos.eCntntAtPos &&
                        pFld && !pFld->HasClickHdl() )
                        pFld = 0;

                    if( pFld )
                    {
                        if( pFldRect && 0 != ( pFrm = pTxtNd->GetFrm( &aPt ) ) )
                            pFrm->GetCharRect( *pFldRect, aPos, &aTmpState );

                        if( bSetCrsr )
                        {
                            SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
                            SwCrsrSaveState aSaveState( *pCurCrsr );
                            pCurCrsr->DeleteMark();
                            *pCurCrsr->GetPoint() = aPos;
                            if( pCurCrsr->IsSelOvr() )
                            {
                                // Click-Felder in geschuetzten Bereichen zulassen
                                // Nur Platzhalter geht nicht!
                                if( SwContentAtPos::SW_FIELD & rCntntAtPos.eCntntAtPos
                                     || RES_JUMPEDITFLD == pFld->Which() )
                                    pFld = 0;
                            }
                            else
                                UpdateCrsr();
                        }
                        else if( RES_TABLEFLD == pFld->Which() &&
                                ((SwTblField*)pFld)->IsIntrnlName() )
                        {
                            // erzeuge aus der internen (fuer CORE)
                            // die externe (fuer UI) Formel
                            const SwTableNode* pTblNd = pTxtNd->FindTableNode();
                            if( pTblNd )        // steht in einer Tabelle
                                ((SwTblField*)pFld)->PtrToBoxNm( &pTblNd->GetTable() );
                        }
                    }

                    if( pFld )
                    {
                        rCntntAtPos.aFnd.pFld = pFld;
                        rCntntAtPos.pFndTxtAttr = pTxtAttr;
                        rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_FIELD;
                        bRet = TRUE;
                    }
                }

                if( !bRet && SwContentAtPos::SW_FTN & rCntntAtPos.eCntntAtPos )
                {
                    if( aTmpState.bFtnNoInfo )
                    {
                        // stehe ueber dem Zeichen der Fussnote (??)
                        bRet = TRUE;
                        if( bSetCrsr )
                        {
                            *pCurCrsr->GetPoint() = aPos;
                            if( !GotoFtnAnchor() )
                                bRet = FALSE;
                        }
                        if( bRet )
                            rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_FTN;
                    }
                    else if( 0 != ( pTxtAttr = pTxtNd->GetTxtAttr(
                                aPos.nContent.GetIndex(), RES_TXTATR_FTN )) )
                    {
                        bRet = TRUE;
                        if( bSetCrsr )
                        {
                            SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
                            SwCrsrSaveState aSaveState( *pCurCrsr );
                            pCurCrsr->GetPoint()->nNode = *((SwTxtFtn*)pTxtAttr)->GetStartNode();
                            SwCntntNode* pCNd = GetDoc()->GetNodes().GoNextSection(
                                            &pCurCrsr->GetPoint()->nNode,
                                            TRUE, !IsReadOnlyAvailable() );

                            if( pCNd )
                            {
                                pCurCrsr->GetPoint()->nContent.Assign( pCNd, 0 );
                                if( pCurCrsr->IsSelOvr( SELOVER_CHECKNODESSECTION |
                                                SELOVER_TOGGLE ))
                                    bRet = FALSE;
                                else
                                    UpdateCrsr();
                            }
                            else
                                bRet = FALSE;
                        }

                        if( bRet )
                        {
                            rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_FTN;
                            rCntntAtPos.pFndTxtAttr = pTxtAttr;
                            rCntntAtPos.aFnd.pAttr = &pTxtAttr->GetAttr();

                            if( pFldRect && 0 != ( pFrm = pTxtNd->GetFrm( &aPt ) ) )
                                pFrm->GetCharRect( *pFldRect, aPos, &aTmpState );
                        }
                    }
                }

                if( !bRet && ( SwContentAtPos::SW_TOXMARK |
                               SwContentAtPos::SW_REFMARK ) &
                        rCntntAtPos.eCntntAtPos && !aTmpState.bFtnNoInfo )
                {
                    pTxtAttr = 0;
                    if( SwContentAtPos::SW_TOXMARK & rCntntAtPos.eCntntAtPos )
                        pTxtAttr = pTxtNd->GetTxtAttr( aPos.nContent,
                                                        RES_TXTATR_TOXMARK );

                    if( !pTxtAttr &&
                        SwContentAtPos::SW_REFMARK & rCntntAtPos.eCntntAtPos )
                        pTxtAttr = pTxtNd->GetTxtAttr( aPos.nContent,
                                                        RES_TXTATR_REFMARK );

                    if( pTxtAttr )
                    {
                        bRet = TRUE;
                        if( bSetCrsr )
                        {
                            SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
                            SwCrsrSaveState aSaveState( *pCurCrsr );
                            pCurCrsr->DeleteMark();
                            *pCurCrsr->GetPoint() = aPos;
                            if( pCurCrsr->IsSelOvr( SELOVER_CHECKNODESSECTION | SELOVER_TOGGLE ) )
                                bRet = FALSE;
                            else
                                UpdateCrsr();
                        }

                        if( bRet )
                        {
                            const xub_StrLen* pEnd = pTxtAttr->GetEnd();
                            if( pEnd )
                                rCntntAtPos.sStr = pTxtNd->GetExpandTxt(
                                            *pTxtAttr->GetStart(),
                                            *pEnd - *pTxtAttr->GetStart(),
                                            FALSE );
                            else if( RES_TXTATR_TOXMARK == pTxtAttr->Which())
                                rCntntAtPos.sStr = pTxtAttr->GetTOXMark().
                                                    GetAlternativeText();

                            rCntntAtPos.eCntntAtPos =
                                RES_TXTATR_TOXMARK == pTxtAttr->Which()
                                            ? SwContentAtPos::SW_TOXMARK
                                            : SwContentAtPos::SW_REFMARK;
                            rCntntAtPos.pFndTxtAttr = pTxtAttr;
                            rCntntAtPos.aFnd.pAttr = &pTxtAttr->GetAttr();

                            if( pFldRect && 0 != ( pFrm = pTxtNd->GetFrm( &aPt ) ) )
                                pFrm->GetCharRect( *pFldRect, aPos, &aTmpState );
                        }
                    }
                }

                if( !bRet && SwContentAtPos::SW_INETATTR & rCntntAtPos.eCntntAtPos
                    && !aTmpState.bFtnNoInfo )
                {
                    pTxtAttr = pTxtNd->GetTxtAttr( aPos.nContent,
                                                    RES_TXTATR_INETFMT );
                    // nur INetAttrs mit URLs "erkennen"
                    if( pTxtAttr && pTxtAttr->GetINetFmt().GetValue().Len() )
                    {
                        bRet = TRUE;
                        if( bSetCrsr )
                        {
                            SwCrsrSaveState aSaveState( *pCurCrsr );
                            SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
                            pCurCrsr->DeleteMark();
                            *pCurCrsr->GetPoint() = aPos;
                            if( pCurCrsr->IsSelOvr(SELOVER_CHECKNODESSECTION | SELOVER_TOGGLE) )
                                bRet = FALSE;
                            else
                                UpdateCrsr();
                        }
                        if( bRet )
                        {
                            rCntntAtPos.sStr = pTxtNd->GetExpandTxt(
                                        *pTxtAttr->GetStart(),
                                        *pTxtAttr->GetEnd() - *pTxtAttr->GetStart(),
                                        FALSE );

                            rCntntAtPos.aFnd.pAttr = &pTxtAttr->GetAttr();
                            rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_INETATTR;
                            rCntntAtPos.pFndTxtAttr = pTxtAttr;

                            if( pFldRect && 0 != ( pFrm = pTxtNd->GetFrm( &aPt ) ) )
                                pFrm->GetCharRect( *pFldRect, aPos, &aTmpState );
                        }
                    }
                }

                if( !bRet && SwContentAtPos::SW_REDLINE & rCntntAtPos.eCntntAtPos )
                {
                    const SwRedline* pRedl = GetDoc()->GetRedline( aPos );
                    if( pRedl )
                    {
                        rCntntAtPos.aFnd.pRedl = pRedl;
                        rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_REDLINE;
                        rCntntAtPos.pFndTxtAttr = 0;
                        bRet = TRUE;

                        if( pFldRect && 0 != ( pFrm = pTxtNd->GetFrm( &aPt ) ) )
                            pFrm->GetCharRect( *pFldRect, aPos, &aTmpState );
                    }
                }
            }

            if( !bRet && (
                SwContentAtPos::SW_TABLEBOXFML & rCntntAtPos.eCntntAtPos
#ifndef PRODUCT
                || SwContentAtPos::SW_TABLEBOXVALUE & rCntntAtPos.eCntntAtPos
#endif
                ))
            {
                const SwTableNode* pTblNd;
                const SwTableBox* pBox;
                const SwStartNode* pSttNd = pTxtNd->FindTableBoxStartNode();
                const SfxPoolItem* pItem;
                if( pSttNd && 0 != ( pTblNd = pTxtNd->FindTableNode()) &&
                    0 != ( pBox = pTblNd->GetTable().GetTblBox(
                                    pSttNd->GetIndex() )) &&
#ifndef PRODUCT
                    ( SFX_ITEM_SET == pBox->GetFrmFmt()->GetItemState(
                        RES_BOXATR_FORMULA, FALSE, &pItem ) ||
                      SFX_ITEM_SET == pBox->GetFrmFmt()->GetItemState(
                        RES_BOXATR_VALUE, FALSE, &pItem ))
#else
                    SFX_ITEM_SET == pBox->GetFrmFmt()->GetItemState(
                        RES_BOXATR_FORMULA, FALSE, &pItem )
#endif
                    )
                {
                    SwFrm* pF = pTxtNd->GetFrm( &aPt );
                    if( pF )
                    {
                        // dann aber den CellFrame
                        pFrm = (SwCntntFrm*)pF;
                        while( pF && !pF->IsCellFrm() )
                            pF = pF->GetUpper();
                    }

                    // es wurde ein
                    if( aTmpState.bPosCorr )
                    {
                        if( pF && !pF->Frm().IsInside( aPt ))
                            pF = 0;
                    }
                    else if( !pF )
                        pF = pFrm;

                    if( pF )            // nur dann ist es gueltig!!
                    {
                        // erzeuge aus der internen (fuer CORE)
                        // die externe (fuer UI) Formel
                        rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_TABLEBOXFML;
#ifndef PRODUCT
                        if( RES_BOXATR_VALUE == pItem->Which() )
                            rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_TABLEBOXVALUE;
                        else
#endif
                        ((SwTblBoxFormula*)pItem)->PtrToBoxNm( &pTblNd->GetTable() );

                        bRet = TRUE;
                        if( bSetCrsr )
                        {
                            SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
                            SwCrsrSaveState aSaveState( *pCurCrsr );
                            *pCurCrsr->GetPoint() = aPos;
                            if( pCurCrsr->IsSelOvr( SELOVER_CHECKNODESSECTION | SELOVER_TOGGLE) )
                                bRet = FALSE;
                            else
                                UpdateCrsr();
                        }

                        if( bRet )
                        {
                            if( pFldRect )
                            {
                                *pFldRect = pF->Prt();
                                *pFldRect += pF->Frm().Pos();
                            }
                            rCntntAtPos.pFndTxtAttr = 0;
                            rCntntAtPos.aFnd.pAttr = pItem;
                        }
                    }
                }
            }

#ifndef PRODUCT
            if( !bRet && SwContentAtPos::SW_CURR_ATTRS & rCntntAtPos.eCntntAtPos )
            {
                xub_StrLen n = aPos.nContent.GetIndex();
                SfxItemSet aSet( GetDoc()->GetAttrPool(), POOLATTR_BEGIN,
                                                          POOLATTR_END - 1 );
                if( pTxtNd->GetpSwpHints() )
                {
                    for( USHORT i = 0; i < pTxtNd->GetSwpHints().Count(); ++i )
                    {
                        const SwTxtAttr* pHt = pTxtNd->GetSwpHints()[i];
                        xub_StrLen nAttrStart = *pHt->GetStart();
                        if( nAttrStart > n )        // ueber den Bereich hinaus
                            break;

                        if( 0 != pHt->GetEnd() && (
                            ( nAttrStart < n &&
                                ( pHt->DontExpand() ? n < *pHt->GetEnd()
                                                    : n <= *pHt->GetEnd() )) ||
                            ( n == nAttrStart &&
                                ( nAttrStart == *pHt->GetEnd() || !n ))) )
                        {
                            aSet.Put( pHt->GetAttr() );
                        }
                    }
                    if( pTxtNd->GetpSwAttrSet() &&
                        pTxtNd->GetSwAttrSet().Count() )
                    {
                        SfxItemSet aFmtSet( pTxtNd->GetSwAttrSet() );
                        // aus dem Format-Set alle entfernen, die im TextSet auch gesetzt sind
                        aFmtSet.Differentiate( aSet );
                        // jetzt alle zusammen "mergen"
                        aSet.Put( aFmtSet );
                    }
                }
                else
                    pTxtNd->SwCntntNode::GetAttr( aSet );

                rCntntAtPos.sStr.AssignAscii(
                                    RTL_CONSTASCII_STRINGPARAM( "Pos: (" ));
                rCntntAtPos.sStr += String::CreateFromInt32( aPos.nNode.GetIndex());
                rCntntAtPos.sStr += ':';
                rCntntAtPos.sStr += String::CreateFromInt32( aPos.nContent.GetIndex());
                rCntntAtPos.sStr += ')';
                rCntntAtPos.sStr.AppendAscii(
                                RTL_CONSTASCII_STRINGPARAM( "\nAbs.Vorl.: " ));
                rCntntAtPos.sStr += pTxtNd->GetFmtColl()->GetName();
                if( pTxtNd->GetCondFmtColl() )
                    rCntntAtPos.sStr.AppendAscii(
                                RTL_CONSTASCII_STRINGPARAM( "\nBed.Vorl.: " ))
                        += pTxtNd->GetCondFmtColl()->GetName();

                if( aSet.Count() )
                {
                    String sAttrs;
                    SfxItemIter aIter( aSet );
                    const SfxPoolItem* pItem = aIter.FirstItem();
                    while( TRUE )
                    {
                        if( !IsInvalidItem( pItem ))
                        {
                            String aStr;
                            GetDoc()->GetAttrPool().GetPresentation( *pItem,
                                            SFX_ITEM_PRESENTATION_COMPLETE,
                                            SFX_MAPUNIT_CM, aStr );
                            if( sAttrs.Len() )
                                sAttrs.AppendAscii(
                                        RTL_CONSTASCII_STRINGPARAM( ", " ));
                            sAttrs += aStr;
                        }
                        if( aIter.IsAtEnd() )
                            break;
                        pItem = aIter.NextItem();
                    }
                    if( sAttrs.Len() )
                    {
                        if( rCntntAtPos.sStr.Len() )
                            rCntntAtPos.sStr += '\n';
                        rCntntAtPos.sStr.AppendAscii(
                                RTL_CONSTASCII_STRINGPARAM( "Attr: " ) )
                                += sAttrs;
                    }
                }
                bRet = TRUE;
                rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_CURR_ATTRS;
            }
#endif
        }
    }

    if( !bRet )
    {
        rCntntAtPos.eCntntAtPos = SwContentAtPos::SW_NOTHING;
        rCntntAtPos.aFnd.pFld = 0;
    }
    return bRet;
}

// befindet sich der Node in einem geschuetzten Bereich?
FASTBOOL SwContentAtPos::IsInProtectSect() const
{
    const SwTxtNode* pNd = 0;
    if( pFndTxtAttr )
    {
        switch( eCntntAtPos )
        {
        case SW_FIELD:
        case SW_CLICKFIELD:
            pNd = ((SwTxtFld*)pFndTxtAttr)->GetpTxtNode();
            break;

        case SW_FTN:
            pNd = &((SwTxtFtn*)pFndTxtAttr)->GetTxtNode();
            break;

        case SW_INETATTR:
            pNd = ((SwTxtINetFmt*)pFndTxtAttr)->GetpTxtNode();
            break;
        }
    }

    const SwCntntFrm* pFrm;
    return pNd && ( pNd->IsInProtectSect() ||
                    ( 0 != ( pFrm = pNd->GetFrm(0,0,FALSE)) &&
                        pFrm->IsProtected() ));
}

FASTBOOL SwCrsrShell::SelectTxtAttr( USHORT nWhich, BOOL bExpand,
                                    const SwTxtAttr* pTxtAttr )
{
    SET_CURR_SHELL( this );
    FASTBOOL bRet = FALSE;

    if( !IsTableMode() )
    {
        SwPosition& rPos = *pCurCrsr->GetPoint();
        if( !pTxtAttr )
        {
            SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
            pTxtAttr = pTxtNd ? pTxtNd->GetTxtAttr( rPos.nContent,
                                                    nWhich, bExpand ) : 0;
        }

        if( pTxtAttr )
        {
            SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
            SwCrsrSaveState aSaveState( *pCurCrsr );

            pCurCrsr->DeleteMark();
            rPos.nContent = *pTxtAttr->GetStart();
            pCurCrsr->SetMark();
            const xub_StrLen* pEnd = pTxtAttr->GetEnd();
            rPos.nContent = pEnd ? *pEnd : *pTxtAttr->GetStart() + 1;

            if( !pCurCrsr->IsSelOvr() )
            {
                UpdateCrsr();
                bRet = TRUE;
            }
        }
    }
    return bRet;
}


FASTBOOL SwCrsrShell::GotoINetAttr( const SwTxtINetFmt& rAttr )
{
    FASTBOOL bRet = FALSE;
    if( rAttr.GetpTxtNode() )
    {
        SwCursor* pCrsr = IsTableMode() ? pTblCrsr : pCurCrsr;

        SET_CURR_SHELL( this );
        SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
        SwCrsrSaveState aSaveState( *pCrsr );

        pCrsr->GetPoint()->nNode = *rAttr.GetpTxtNode();
        pCrsr->GetPoint()->nContent.Assign( (SwTxtNode*)rAttr.GetpTxtNode(),
                                            *rAttr.GetStart() );
        bRet = !pCrsr->IsSelOvr();
        if( bRet )
            UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    }
    return bRet;
}


const SwFmtINetFmt* SwCrsrShell::FindINetAttr( const String& rName ) const
{
    return pDoc->FindINetAttr( rName );
}

FASTBOOL SwCrsrShell::GetShadowCrsrPos( const Point& rPt, SwFillMode eFillMode,
                                SwRect& rRect, SwHoriOrient& rOrient )
{

    SET_CURR_SHELL( this );
    FASTBOOL bRet = FALSE;

    if( !IsTableMode() && !HasSelection() && GetDoc()->DoesUndo() )
    {
        Point aPt( rPt );
        SwPosition aPos( *pCurCrsr->GetPoint() );

        SwFillCrsrPos aFPos( eFillMode );
        SwCrsrMoveState aTmpState( &aFPos );

        if( GetLayout()->GetCrsrOfst( &aPos, aPt, &aTmpState ) &&
            !aPos.nNode.GetNode().IsProtect())
        {
            // Start-Position im geschuetzten Bereich?
            rRect = aFPos.aCrsr;
            rOrient = aFPos.eOrient;
            bRet = TRUE;
        }
    }
    return bRet;
}

FASTBOOL SwCrsrShell::SetShadowCrsrPos( const Point& rPt, SwFillMode eFillMode )
{
    SET_CURR_SHELL( this );
    FASTBOOL bRet = FALSE;

    if( !IsTableMode() && !HasSelection() && GetDoc()->DoesUndo() )
    {
        Point aPt( rPt );
        SwPosition aPos( *pCurCrsr->GetPoint() );

        SwFillCrsrPos aFPos( eFillMode );
        SwCrsrMoveState aTmpState( &aFPos );

        if( GetLayout()->GetCrsrOfst( &aPos, aPt, &aTmpState ) )
        {
            SwCallLink aLk( *this );        // Crsr-Moves ueberwachen
            StartAction();

            SwCntntNode* pCNd = aPos.nNode.GetNode().GetCntntNode();
            USHORT nUndoId = UNDO_INS_FROM_SHADOWCRSR;
            // Werden nur die Absatzattribute Adjust oder LRSpace gesetzt,
            // dann sollte der naechste Aufruf die NICHT wieder entfernen.
            if( 0 == aFPos.nParaCnt + aFPos.nColumnCnt &&
                ( FILL_INDENT == aFPos.eMode ||
                  ( HORI_NONE != aFPos.eOrient &&
                    0 == aFPos.nTabCnt + aFPos.nSpaceCnt )) &&
                pCNd && pCNd->Len() )
                nUndoId = 0;

            GetDoc()->StartUndo( nUndoId );

            SwTxtFmtColl* pNextFmt = 0;
            SwTxtNode* pTNd = pCNd->GetTxtNode();
            if( pTNd )
                pNextFmt = &pTNd->GetTxtColl()->GetNextTxtFmtColl();

            const SwSectionNode* pSectNd = pCNd->FindSectionNode();
            if( pSectNd && aFPos.nParaCnt )
            {
                SwNodeIndex aEnd( aPos.nNode, 1 );
                while( aEnd.GetNode().IsEndNode() &&
                        (const SwNode*)&aEnd.GetNode() !=
                        pSectNd->EndOfSectionNode() )
                    aEnd++;

                if( aEnd.GetNode().IsEndNode() &&
                    pCNd->Len() == aPos.nContent.GetIndex() )
                    aPos.nNode = *pSectNd->EndOfSectionNode();
            }

            for( USHORT n = 0; n < aFPos.nParaCnt + aFPos.nColumnCnt; ++n )
            {
                GetDoc()->AppendTxtNode( aPos );
                if( !n && pNextFmt )
                {
                    *pCurCrsr->GetPoint() = aPos;
                    GetDoc()->SetTxtFmtColl( *pCurCrsr, pNextFmt, FALSE );
                    //JP 04.11.97: erstmal keine Folgevorlage der
                    //              Folgevorlage beachten
                    // pNextFmt = pNextFmt->GetNextTxtFmtColl();
                }
                if( n < aFPos.nColumnCnt )
                {
                    *pCurCrsr->GetPoint() = aPos;
                    GetDoc()->Insert( *pCurCrsr,
                            SvxFmtBreakItem( SVX_BREAK_COLUMN_BEFORE ));
                }
            }

            *pCurCrsr->GetPoint() = aPos;
            switch( aFPos.eMode )
            {
            case FILL_INDENT:
                if( 0 != (pCNd = aPos.nNode.GetNode().GetCntntNode() ))
                {
                    SfxItemSet aSet( GetDoc()->GetAttrPool(),
                                    RES_LR_SPACE, RES_LR_SPACE,
                                    RES_PARATR_ADJUST, RES_PARATR_ADJUST,
                                    0 );
                    SvxLRSpaceItem aLR( (SvxLRSpaceItem&)
                                        pCNd->GetAttr( RES_LR_SPACE ) );
                    aLR.SetTxtLeft( aFPos.nTabCnt );
                    aLR.SetTxtFirstLineOfst( 0 );
                    aSet.Put( aLR );

                    const SvxAdjustItem& rAdj = (SvxAdjustItem&)pCNd->
                                        GetAttr( RES_PARATR_ADJUST );
                    if( SVX_ADJUST_LEFT != rAdj.GetAdjust() )
                        aSet.Put( SvxAdjustItem( SVX_ADJUST_LEFT ) );

                    GetDoc()->Insert( *pCurCrsr, aSet );
                }
                else
                    ASSERT( !this, "wo ist mein CntntNode?" );
                break;

            case FILL_TAB:
            case FILL_SPACE:
                {
                    String sInsert;
                    if( aFPos.nTabCnt )
                        sInsert.Fill( aFPos.nTabCnt, '\t' );
                    if( aFPos.nSpaceCnt )
                    {
                        String sSpace;
                        sSpace.Fill( aFPos.nSpaceCnt );
                        sInsert += sSpace;
                    }
                    if( sInsert.Len() )
                        GetDoc()->Insert( *pCurCrsr, sInsert );
                }
                // kein break - Ausrichtung muss noch gesetzt werden
            case FILL_MARGIN:
                if( HORI_NONE != aFPos.eOrient )
                {
                    SvxAdjustItem aAdj( SVX_ADJUST_LEFT );
                    switch( aFPos.eOrient )
                    {
                    case HORI_CENTER:
                        aAdj.SetAdjust( SVX_ADJUST_CENTER );
                        break;
                    case HORI_RIGHT:
                        aAdj.SetAdjust( SVX_ADJUST_RIGHT );
                        break;
                    }
                    GetDoc()->Insert( *pCurCrsr, aAdj );
                }
                break;
            }

            GetDoc()->EndUndo( nUndoId );
            EndAction();

            bRet = TRUE;
        }
    }
    return bRet;
}

const SwRedline* SwCrsrShell::SelNextRedline()
{
    const SwRedline* pFnd = 0;
    if( !IsTableMode() )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
        SwCrsrSaveState aSaveState( *pCurCrsr );

        pFnd = GetDoc()->SelNextRedline( *pCurCrsr );
        if( pFnd && !pCurCrsr->IsInProtectTable() && !pCurCrsr->IsSelOvr() )
            UpdateCrsr( SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
        else
            pFnd = 0;
    }
    return pFnd;
}

const SwRedline* SwCrsrShell::SelPrevRedline()
{
    const SwRedline* pFnd = 0;
    if( !IsTableMode() )
    {
        SET_CURR_SHELL( this );
        SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
        SwCrsrSaveState aSaveState( *pCurCrsr );

        pFnd = GetDoc()->SelPrevRedline( *pCurCrsr );
        if( pFnd && !pCurCrsr->IsInProtectTable() && !pCurCrsr->IsSelOvr() )
            UpdateCrsr( SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
        else
            pFnd = 0;
    }
    return pFnd;
}

const SwRedline* SwCrsrShell::_GotoRedline( USHORT nArrPos, BOOL bSelect )
{
    const SwRedline* pFnd = 0;
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    SwCrsrSaveState aSaveState( *pCurCrsr );

    pFnd = GetDoc()->GetRedlineTbl()[ nArrPos ];
    if( pFnd )
    {
        *pCurCrsr->GetPoint() = *pFnd->Start();

        SwCntntNode* pCNd;
        SwNodeIndex* pIdx = &pCurCrsr->GetPoint()->nNode;
        if( !pIdx->GetNode().IsCntntNode() &&
            0 != ( pCNd = GetDoc()->GetNodes().GoNextSection( pIdx,
                                    TRUE, IsReadOnlyAvailable() )) )
        {
            if( *pIdx <= pFnd->End()->nNode )
                pCurCrsr->GetPoint()->nContent.Assign( pCNd, 0 );
            else
                pFnd = 0;
        }

        if( pFnd && bSelect )
        {
            pCurCrsr->SetMark();
            if( REDLINE_FMTCOLL == pFnd->GetType() )
            {
                pCNd = pIdx->GetNode().GetCntntNode();
                pCurCrsr->GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
                pCurCrsr->GetMark()->nContent.Assign( pCNd, 0 );
            }
            else
                *pCurCrsr->GetPoint() = *pFnd->End();

            pIdx = &pCurCrsr->GetPoint()->nNode;
            if( !pIdx->GetNode().IsCntntNode() &&
                0 != ( pCNd = GetDoc()->GetNodes().GoPrevSection( pIdx,
                                            TRUE, IsReadOnlyAvailable() )) )
            {
                if( *pIdx >= pCurCrsr->GetMark()->nNode )
                    pCurCrsr->GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
                else
                    pFnd = 0;
            }
        }

        if( !pFnd )
        {
            pCurCrsr->DeleteMark();
            pCurCrsr->RestoreSavePos();
        }
        else if( bSelect && *pCurCrsr->GetMark() == *pCurCrsr->GetPoint() )
            pCurCrsr->DeleteMark();

        if( pFnd && !pCurCrsr->IsInProtectTable() && !pCurCrsr->IsSelOvr() )
            UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE
                        | SwCrsrShell::READONLY );
        else
        {
            pFnd = 0;
            if( bSelect )
                pCurCrsr->DeleteMark();
        }
    }
    return pFnd;
}

const SwRedline* SwCrsrShell::GotoRedline( USHORT nArrPos, BOOL bSelect )
{
    const SwRedline* pFnd = 0;
    if( !IsTableMode() )
    {
        SET_CURR_SHELL( this );

        const SwRedlineTbl& rTbl = GetDoc()->GetRedlineTbl();
        const SwRedline* pTmp = rTbl[ nArrPos ];
        USHORT nSeqNo = pTmp->GetSeqNo();
        if( nSeqNo && bSelect )
        {
            BOOL bCheck = FALSE;
            int nLoopCnt = 2;
            USHORT nArrSavPos = nArrPos;

            do {
                pTmp = _GotoRedline( nArrPos, TRUE );

                if( !pFnd )
                    pFnd = pTmp;

                if( pTmp && bCheck )
                {
                    // checke auf Ueberlappungen. Das kann durch
                    // FmtColl-Redlines kommen, die auf den gesamten Absatz
                    // aus gedehnt werden.

                    SwPaM* pCur = pCurCrsr, *pNext = (SwPaM*)pCur->GetNext();
                    SwPosition* pCStt = pCur->Start(), *pCEnd = pCur->End();
                    while( pCur != pNext )
                    {
                        const SwPosition *pNStt = pNext->Start(),
                                         *pNEnd = pNext->End();

                        BOOL bDel = TRUE;
                        switch( ::ComparePosition( *pCStt, *pCEnd,
                                                   *pNStt, *pNEnd ))
                        {
                        case POS_INSIDE:    // Pos1 liegt vollstaendig in Pos2
                            if( !pCur->HasMark() )
                            {
                                pCur->SetMark();
                                *pCur->GetMark() = *pNStt;
                            }
                            else
                                *pCStt = *pNStt;
                            *pCEnd = *pNEnd;
                            break;

                        case POS_OUTSIDE:   // Pos2 liegt vollstaendig in Pos1
                        case POS_EQUAL:     // Pos1 ist genauso gross wie Pos2
                            break;

                        case POS_OVERLAP_BEFORE:        // Pos1 ueberlappt Pos2 am Anfang
                            if( !pCur->HasMark() )
                                pCur->SetMark();
                            *pCEnd = *pNEnd;
                            break;
                        case POS_OVERLAP_BEHIND:        // Pos1 ueberlappt Pos2 am Ende
                            if( !pCur->HasMark() )
                            {
                                pCur->SetMark();
                                *pCur->GetMark() = *pNStt;
                            }
                            else
                                *pCStt = *pNStt;
                            break;

                        default:
                            bDel = FALSE;
                        }

                        if( bDel )
                        {
                            // den brauchen wir nicht mehr
                            SwPaM* pPrev = (SwPaM*)pNext->GetPrev();
                            delete pNext;
                            pNext = pPrev;
                        }
                        pNext = (SwPaM*)pNext->GetNext();
                    }
                }

                USHORT nFndPos = 2 == nLoopCnt
                                    ? rTbl.FindNextOfSeqNo( nArrPos )
                                    : rTbl.FindPrevOfSeqNo( nArrPos );
                if( USHRT_MAX != nFndPos ||
                    ( 0 != ( --nLoopCnt ) && USHRT_MAX != (
                            nFndPos = rTbl.FindPrevOfSeqNo( nArrSavPos ))) )
                {
                    if( pTmp )
                    {
                        // neuen Cursor erzeugen
                        CreateCrsr();
                        bCheck = TRUE;
                    }
                    nArrPos = nFndPos;
                }
                else
                    nLoopCnt = 0;

            } while( nLoopCnt );
        }
        else
            pFnd = _GotoRedline( nArrPos, bSelect );
    }
    return pFnd;
}



