/*************************************************************************
 *
 *  $RCSfile: fews.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:20 $
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

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _SVDVMARK_HXX //autogen
#include <svx/svdvmark.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif

#ifndef _FESH_HXX
#include <fesh.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif
#ifndef _DVIEW_HXX
#include <dview.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SECTFRM_HXX
#include <sectfrm.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _CELLFRM_HXX
#include <cellfrm.hxx>
#endif
#ifndef _FLYFRMS_HXX
#include <flyfrms.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif
#ifndef _EDIMP_HXX
#include <edimp.hxx>
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif

using namespace ::com::sun::star;
TYPEINIT1(SwFEShell,SwEditShell)

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methode    :  EndAllActionAndCall()
#*
#*  Datum      :  MA 03. May. 93
#*  Update     :  MA 31. Oct. 95
#***********************************************************************/

void SwFEShell::EndAllActionAndCall()
{
    ViewShell *pTmp = this;
    do {
        if( pTmp->IsA( TYPE(SwCrsrShell) ) )
        {
            ((SwFEShell*)pTmp)->EndAction();
            ((SwFEShell*)pTmp)->CallChgLnk();
        }
        else
            pTmp->EndAction();
    } while( this != ( pTmp = (ViewShell*)pTmp->GetNext() ));
}


/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  GetCntntPos
#*  Beschreibung:  Ermitteln des Cntnt's der dem Punkt am naechsten liegt
#*  Datum       :  MA 02. Jun. 92
#*  Update      :  MA 02. May. 95
#***********************************************************************/

Point SwFEShell::GetCntntPos( const Point& rPoint, BOOL bNext ) const
{
    SET_CURR_SHELL( (ViewShell*)this );
    if ( bNext )
        return GetLayout()->GetNextCntntPos( rPoint, IsTableMode() );
    return GetLayout()->GetPrevCntntPos( rPoint );
}


const SwRect& SwFEShell::GetAnyCurRect( CurRectType eType, const Point* pPt,
                                        const SvEmbeddedObject *pObj ) const
{
    const SwFrm *pFrm = Imp()->HasDrawView()
                ? ::GetFlyFromMarked( &Imp()->GetDrawView()->GetMarkList(),
                                      (ViewShell*)this)
                : 0;

    if( !pFrm )
    {
        if( pPt )
        {
            SwPosition aPos( *GetCrsr()->GetPoint() );
            Point aPt( *pPt );
            GetLayout()->GetCrsrOfst( &aPos, aPt );
            SwCntntNode *pNd = aPos.nNode.GetNode().GetCntntNode();
            pFrm = pNd->GetFrm( pPt );
        }
        else
            pFrm = GetCurrFrm();
    }

    if( !pFrm )
        return GetLayout()->Frm();

    FASTBOOL  bFrm  = TRUE;
    switch ( eType )
    {
        case RECT_PAGE_PRT:         bFrm = FALSE; /* no break */
        case RECT_PAGE :            pFrm = pFrm->FindPageFrm();
                                    break;

        case RECT_PAGE_CALC:        pFrm->Calc();
                                    pFrm = pFrm->FindPageFrm();
                                    pFrm->Calc();
                                    break;

        case RECT_FLY_PRT_EMBEDDED: bFrm = FALSE; /* no break */
        case RECT_FLY_EMBEDDED:     pFrm = pObj ? FindFlyFrm( pObj )
                                                : pFrm->IsFlyFrm()
                                                    ? pFrm
                                                    : pFrm->FindFlyFrm();
                                    break;

        case RECT_OUTTABSECTION_PRT:
        case RECT_OUTTABSECTION :   if( pFrm->IsInTab() )
                                        pFrm = pFrm->FindTabFrm();
                                    else
                                        ASSERT( FALSE, "Missing Table" );
                                    /* KEIN BREAK */
        case RECT_SECTION_PRT:
        case RECT_SECTION:          if( pFrm->IsInSct() )
                                        pFrm = pFrm->FindSctFrm();
                                    else
                                        ASSERT( FALSE, "Missing section" );

                                    if( RECT_OUTTABSECTION_PRT == eType ||
                                        RECT_SECTION_PRT == eType )
                                        bFrm = FALSE;
                                    break;

        case RECT_HEADERFOOTER_PRT: bFrm = FALSE; /* no break */
        case RECT_HEADERFOOTER:     if( 0 == (pFrm = pFrm->FindFooterOrHeader()) )
                                        return GetLayout()->Frm();
                                    break;

//JP 20.08.98: wo zu diese Statements? unnoetiger Code
//      case RECT_FRM:
//      default:                    break;
    }
    return bFrm ? pFrm->Frm() : pFrm->Prt();
}


USHORT SwFEShell::GetPageNumber( const Point &rPoint ) const
{
    const SwFrm *pPage = GetLayout()->Lower();
    while ( pPage && !pPage->Frm().IsInside( rPoint ) )
        pPage = pPage->GetNext();
    if ( pPage )
        return ((const SwPageFrm*)pPage)->GetPhyPageNum();
    else
        return 0;
}


BOOL SwFEShell::GetPageNumber( long nYPos, BOOL bAtCrsrPos, USHORT& rPhyNum, USHORT& rVirtNum, String &rDisplay) const
{
    const SwFrm *pPage;

    if ( bAtCrsrPos )                   //Seite vom Crsr besorgen
    {
        pPage = GetCurrFrm( FALSE );
        if ( pPage )
            pPage = pPage->FindPageFrm();
    }
    else if ( nYPos > -1 )              //Seite ueber die Positon ermitteln
    {
        pPage = GetLayout()->Lower();
        while( pPage &&  (pPage->Frm().Bottom() < nYPos ||
                            nYPos < pPage->Frm().Top() ) )
            pPage = pPage->GetNext();
    }
    else                                //Die erste sichtbare Seite
    {
        pPage = Imp()->GetFirstVisPage();
        if ( pPage && ((SwPageFrm*)pPage)->IsEmptyPage() )
            pPage = pPage->GetNext();
    }

    if( pPage )
    {
        rPhyNum  = ((const SwPageFrm*)pPage)->GetPhyPageNum();
        rVirtNum = ((const SwPageFrm*)pPage)->GetVirtPageNum();
        const SwNumType& rNum = ((const SwPageFrm*)pPage)->GetPageDesc()->GetNumType();
        rDisplay = rNum.GetNumStr( rVirtNum );
    }

    return 0 != pPage;
}

#ifdef USED
/*************************************************************************
|*
|*  SwFEShell::GetHeadFootFrmRect()
|*
|*  Ersterstellung      MA 08. Feb. 95
|*  Letzte Aenderung    MA 08. Feb. 95
|
|*************************************************************************/

//Das FrmRect von Header bzw. Footer wird relativ zur Seite ermittelt.
//Der long ist 0 wenn der Crsr nicht in Header oder Footer steht.
//Andernfalls markiert der long den maximalen bzw. minimalen Spielraum
//fuer die Hoehe von Header bzw. Footer.

long SwFEShell::GetHeadFootFrmRect( SwRect &rToFill ) const
{
    ASSERT( GetCurrFrm(), "Crsr geparkt?" );

    long nRet = 0;
    const SwFrm *pFrm = GetCurrFrm();
    while ( pFrm && !pFrm->IsHeaderFrm() && !pFrm->IsFooterFrm() )
        pFrm = pFrm->GetUpper();

    if ( pFrm )
    {
        const SwPageFrm *pPage = pFrm->FindPageFrm();
        rToFill = pFrm->Frm();
        rToFill.Pos() -= pPage->Frm().Pos();

        //Wenn Kopf-/Fusszeilen vergroessert werden, sollte die Resthoehe der
        //PrtArea der Seite wenigstens 2cm (lMinBorder) betragen.
        const SwFrm *pBody = pPage->FindBodyCont();
        nRet = pBody->Frm().Top();
        if ( pFrm->IsHeaderFrm() )
        {
            nRet += pBody->Prt().Bottom();
            nRet -= lMinBorder;
            nRet -= pBody->Prt().Top();
        }
        else if ( pFrm->IsFooterFrm() )
        {
            nRet += pBody->Prt().Top();
            nRet += lMinBorder;
            nRet += pBody->Frm().Height() -
                    (pBody->Prt().Height() + pBody->Prt().Top());
        }
        nRet -= pPage->Frm().Top();
    }
    return nRet;
}
#endif

/*************************************************************************
|*
|*  SwFEShell::GetFrmType()
|*
|*  Ersterstellung      MA 12. Jan. 93
|*  Letzte Aenderung    AMA 25. Nov. 98
|*
*************************************************************************/

USHORT SwFEShell::GetFrmType( const Point *pPt, BOOL bStopAtFly ) const
{
    USHORT nReturn = FRMTYPE_NONE;
    SwFrm *pFrm;
    if ( pPt )
    {
        SwPosition aPos( *GetCrsr()->GetPoint() );
        Point aPt( *pPt );
        GetLayout()->GetCrsrOfst( &aPos, aPt );
        SwCntntNode *pNd = aPos.nNode.GetNode().GetCntntNode();
        pFrm = pNd->GetFrm( pPt );
    }
    else
        pFrm = GetCurrFrm( FALSE );
    while ( pFrm )
    {
        switch ( pFrm->GetType() )
        {
            case FRM_COLUMN:    if( pFrm->GetUpper()->IsSctFrm() )
                                {
                                    // Check, if isn't not only a single column
                                    // from a section with footnotes at the end.
                                    if( pFrm->GetNext() || pFrm->GetPrev() )
                                        // Sectioncolumns
                                        nReturn |= ( nReturn & FRMTYPE_TABLE ) ?
                                            FRMTYPE_COLSECTOUTTAB : FRMTYPE_COLSECT;
                                }
                                else // nur Seiten und Rahmenspalten
                                    nReturn |= FRMTYPE_COLUMN;
                                break;
            case FRM_PAGE:      nReturn |= FRMTYPE_PAGE;
                                if( ((SwPageFrm*)pFrm)->IsFtnPage() )
                                    nReturn |= FRMTYPE_FTNPAGE;
                                break;
            case FRM_HEADER:    nReturn |= FRMTYPE_HEADER;      break;
            case FRM_FOOTER:    nReturn |= FRMTYPE_FOOTER;      break;
            case FRM_BODY:      if( pFrm->GetUpper()->IsPageFrm() ) // nicht bei ColumnFrms
                                    nReturn |= FRMTYPE_BODY;
                                break;
            case FRM_FTN:       nReturn |= FRMTYPE_FOOTNOTE;    break;
            case FRM_FLY:       if( ((SwFlyFrm*)pFrm)->IsFlyLayFrm() )
                                    nReturn |= FRMTYPE_FLY_FREE;
                                else if ( ((SwFlyFrm*)pFrm)->IsFlyAtCntFrm() )
                                    nReturn |= FRMTYPE_FLY_ATCNT;
                                else
                                {
                                    ASSERT( ((SwFlyFrm*)pFrm)->IsFlyInCntFrm(),
                                            "Neuer Rahmentyp?" );
                                    nReturn |= FRMTYPE_FLY_INCNT;
                                }
                                nReturn |= FRMTYPE_FLY_ANY;
                                if( bStopAtFly )
                                    return nReturn;
                                break;
            case FRM_TAB:
            case FRM_ROW:
            case FRM_CELL:      nReturn |= FRMTYPE_TABLE;       break;
            default:            /* do nothing */                break;
        }
        if ( pFrm->IsFlyFrm() )
            pFrm = ((SwFlyFrm*)pFrm)->GetAnchor();
        else
            pFrm = pFrm->GetUpper();
    }
    return nReturn;
}

/*************************************************************************
|*
|*  SwFEShell::ShLooseFcs(), ShGetFcs()
|*
|*  Ersterstellung      MA 10. May. 93
|*  Letzte Aenderung    MA 09. Sep. 98
|*
*************************************************************************/

void SwFEShell::ShGetFcs( BOOL bUpdate )
{
    ::SetShell( this );
    SwCrsrShell::ShGetFcs( bUpdate );

    if ( HasDrawView() )
    {
        Imp()->GetDrawView()->SetMarkHdlHidden( FALSE );
        if ( Imp()->GetDrawView()->HasMarkedObj() )
            FrameNotify( this, FLY_DRAG_START );
    }
}

void SwFEShell::ShLooseFcs()
{
    SwCrsrShell::ShLooseFcs();

    if ( HasDrawView() && Imp()->GetDrawView()->HasMarkedObj() )
    {
        Imp()->GetDrawView()->SetMarkHdlHidden( TRUE );
        FrameNotify( this, FLY_DRAG_END );
    }
//  ::ResetShell();
}

/*************************************************************************
|*
|*  SwFEShell::GetPhyPageNum()
|*  SwFEShell::GetVirtPageNum()
|*
|*  Ersterstellung      OK 07.07.93 08:20
|*  Letzte Aenderung    MA 03. Jan. 94
|*
*************************************************************************/

USHORT SwFEShell::GetPhyPageNum()
{
    SwFrm *pFrm = GetCurrFrm();
    if ( pFrm )
        return pFrm->GetPhyPageNum();
    return 0;
}

USHORT SwFEShell::GetVirtPageNum( const BOOL bCalcFrm )
{
    SwFrm *pFrm = GetCurrFrm( bCalcFrm );
    if ( pFrm )
        return pFrm->GetVirtPageNum();
    return 0;
}

/*************************************************************************
|*
|*  void lcl_SetAPageOffset()
|*  void SwFEShell::SetNewPageOffset()
|*  void SwFEShell::SetPageOffset()
|*  USHORT SwFEShell::GetPageOffset() const
|*
|*  Ersterstellung      OK 07.07.93 08:20
|*  Letzte Aenderung    MA 30. Mar. 95
|*
*************************************************************************/

void lcl_SetAPageOffset( USHORT nOffset, SwPageFrm* pPage, SwFEShell* pThis )
{
    pThis->StartAllAction();
    ASSERT( pPage->FindFirstBodyCntnt(),
            "SwFEShell _SetAPageOffset() ohne CntntFrm" );

    SwFmtPageDesc aDesc( pPage->GetPageDesc() );
    aDesc.SetNumOffset( nOffset );

    SwFrm *pFrm = pThis->GetCurrFrm( FALSE );
    if ( pFrm->IsInTab() )
        pThis->GetDoc()->SetAttr( aDesc, *pFrm->FindTabFrm()->GetFmt() );
    else
        pThis->GetDoc()->Insert( *pThis->GetCrsr(), aDesc );

    pThis->EndAllAction();
}

void SwFEShell::SetNewPageOffset( USHORT nOffset )
{
    GetLayout()->SetVirtPageNum( TRUE );
    const SwPageFrm *pPage = GetCurrFrm( FALSE )->FindPageFrm();
    lcl_SetAPageOffset( nOffset, (SwPageFrm*)pPage, this );
}

void SwFEShell::SetPageOffset( USHORT nOffset )
{
    const SwPageFrm *pPage = GetCurrFrm( FALSE )->FindPageFrm();
    const SwRootFrm* pLayout = GetLayout();
    while ( pPage )
    {
        const SwFrm *pFlow = pPage->FindFirstBodyCntnt();
        if ( pFlow )
        {
            if ( pFlow->IsInTab() )
                pFlow = pFlow->FindTabFrm();
            const SwFmtPageDesc& rPgDesc = pFlow->GetAttrSet()->GetPageDesc();
            if ( rPgDesc.GetNumOffset() )
            {
                pLayout->SetVirtPageNum( TRUE );
                lcl_SetAPageOffset( nOffset, (SwPageFrm*)pPage, this );
                break;
            }
        }
        pPage = (SwPageFrm*)pPage->GetPrev();
    }
}

USHORT SwFEShell::GetPageOffset() const
{
    const SwPageFrm *pPage = GetCurrFrm()->FindPageFrm();
    while ( pPage )
    {
        const SwFrm *pFlow = pPage->FindFirstBodyCntnt();
        if ( pFlow )
        {
            if ( pFlow->IsInTab() )
                pFlow = pFlow->FindTabFrm();
            const USHORT nOffset = pFlow->GetAttrSet()->GetPageDesc().GetNumOffset();
            if ( nOffset )
                return nOffset;
        }
        pPage = (SwPageFrm*)pPage->GetPrev();
    }
    return 0;
}

/*************************************************************************
|*
|*  SwFEShell::InsertLabel()
|*
|*  Ersterstellung      MA 10. Feb. 94
|*  Letzte Aenderung    MA 10. Feb. 94
|*
*************************************************************************/

void SwFEShell::InsertLabel( const SwLabelType eType, const String &rTxt,
                             const BOOL bBefore, const USHORT nId,
                             const BOOL bCpyBrd )
{
    //NodeIndex der CrsrPosition besorgen, den Rest kann das Dokument
    //selbst erledigen.
    SwCntntFrm *pCnt = LTYPE_DRAW==eType ? 0 : GetCurrFrm( FALSE );
    if( LTYPE_DRAW==eType || pCnt )
    {
        StartAllAction();

        ULONG nIdx = 0;
        SwFlyFrmFmt* pFlyFmt = 0;
        switch( eType )
        {
        case LTYPE_OBJECT:
        case LTYPE_FLY:
            if( pCnt->IsInFly() )
            {
                //Bei Flys den Index auf den StartNode herunterreichen.
                nIdx = pCnt->FindFlyFrm()->
                            GetFmt()->GetCntnt().GetCntntIdx()->GetIndex();
//warum?? Bug 61913     ParkCrsr( GetCrsr()->GetPoint()->nNode );
            }
            break;
        case LTYPE_TABLE:
            if( pCnt->IsInTab() )
            {
                //Bei Tabellen den Index auf den TblNode herunterreichen.
                const SwTable& rTbl = *pCnt->FindTabFrm()->GetTable();
                nIdx = rTbl.GetTabSortBoxes()[ 0 ]
                            ->GetSttNd()->FindTableNode()->GetIndex();
            }
            break;
        case LTYPE_DRAW:
            if( Imp()->GetDrawView() )
            {
                SwDrawView *pDView = Imp()->GetDrawView();
                const SdrMarkList& rMrkList = pDView->GetMarkList();
                StartUndo();

                ULONG nCount = rMrkList.GetMarkCount();
                for( ULONG i=0; i < nCount; i++ )
                {
                    SdrObject *pObj = rMrkList.GetMark(i)->GetObj();
                    if( !pObj->IsWriterFlyFrame() )
                    {
                        SwFlyFrmFmt *pFmt =
                            GetDoc()->InsertDrawLabel( rTxt, nId, *pObj );
                        if( !pFlyFmt )
                            pFlyFmt = pFmt;
                    }
                }
                EndUndo();
            }
            break;
        default:
            ASSERT( !this, "Crsr weder in Tabelle noch in Fly." );
        }

        if( nIdx )
            pFlyFmt = GetDoc()->InsertLabel( eType, rTxt, bBefore, nId,
                                             nIdx, bCpyBrd );

        SwFlyFrm* pFrm;
        const Point aPt( GetCrsrDocPos() );
        if( pFlyFmt && 0 != ( pFrm = pFlyFmt->GetFrm( &aPt )))
            SelectFlyFrm( *pFrm, TRUE );

        EndAllActionAndCall();
    }
}


/***********************************************************************
#*  Class       :  SwFEShell
#*  Methoden    :  Sort
#*  Datum       :  ??
#*  Update      :  ??
#***********************************************************************/

BOOL SwFEShell::Sort(const SwSortOptions& rOpt)
{
    if( !HasSelection() )
        return FALSE;

    SET_CURR_SHELL( this );
    BOOL bRet;
    StartAllAction();
    if(IsTableMode())
    {
        // Tabelle sortieren
        // pruefe ob vom aktuellen Crsr der SPoint/Mark in einer Tabelle stehen
        SwFrm *pFrm = GetCurrFrm( FALSE );
        ASSERT( pFrm->FindTabFrm(), "Crsr nicht in Tabelle." );

        // lasse ueber das Layout die Boxen suchen
        SwSelBoxes  aBoxes;
        GetTblSel(*this, aBoxes);

        // die Crsr muessen noch aus dem Loesch Bereich entfernt
        // werden. Setze sie immer hinter/auf die Tabelle; ueber die
        // Dokument-Position werden sie dann immer an die alte Position gesetzt.
        while( !pFrm->IsCellFrm() )
            pFrm = pFrm->GetUpper();
        {
            ParkCrsr( SwNodeIndex( *((SwCellFrm*)pFrm)->GetTabBox()->GetSttNd() ));
        }

        // Sorting am Dokument aufrufen
        bRet = pDoc->SortTbl(aBoxes, rOpt);
    }
    else
    {
        // Text sortieren und nichts anderes
        FOREACHPAM_START(this)

            SwPaM* pPam = PCURCRSR;

            SwPosition* pStart = pPam->Start();
            SwPosition* pEnd   = pPam->End();

            SwNodeIndex aPrevIdx( pStart->nNode, -1 );
            ULONG nOffset = pEnd->nNode.GetIndex() - pStart->nNode.GetIndex();
            xub_StrLen nCntStt  = pStart->nContent.GetIndex();

            // Das Sortieren
            bRet = pDoc->SortText(*pPam, rOpt);

            // Selektion wieder setzen
            pPam->DeleteMark();
            pPam->GetPoint()->nNode.Assign( aPrevIdx.GetNode(), +1 );
            SwCntntNode* pCNd = pPam->GetCntntNode();
            xub_StrLen nLen = pCNd->Len();
            if( nLen > nCntStt )
                nLen = nCntStt;
            pPam->GetPoint()->nContent.Assign(pCNd, nLen );
            pPam->SetMark();

            pPam->GetPoint()->nNode += nOffset;
            pCNd = pPam->GetCntntNode();
            pPam->GetPoint()->nContent.Assign( pCNd, pCNd->Len() );

        FOREACHPAM_END()
    }

    EndAllAction();
    return bRet;
}

/*************************************************************************
|*
|*  SwFEShell::GetCurColNum(), _GetColNum()
|*
|*  Ersterstellung      MA 03. Feb. 95
|*  Letzte Aenderung    MA 20. Apr. 95
|
|*************************************************************************/

USHORT SwFEShell::_GetCurColNum( const SwFrm *pFrm,
                                SwGetCurColNumPara* pPara ) const
{
    USHORT nRet = 0;
    while ( pFrm )
    {
        pFrm = pFrm->GetUpper();
        if( pFrm && pFrm->IsColumnFrm() )
        {
            const SwFrm *pCurFrm = pFrm;
            do {
                ++nRet;
                pFrm = pFrm->GetPrev();
            } while ( pFrm );

            if( pPara )
            {
                // dann suche mal das Format, was diese Spaltigkeit bestimmt
                pFrm = pCurFrm->GetUpper();
                while( pFrm )
                {
                    if( ( FRM_PAGE | FRM_FLY | FRM_SECTION ) & pFrm->GetType() )
                    {
                        pPara->pFrmFmt = ((SwLayoutFrm*)pFrm)->GetFmt();
                        pPara->pPrtRect = &pFrm->Prt();
                        pPara->pFrmRect = &pFrm->Frm();
                        break;
                    }
                    pFrm = pFrm->GetUpper();
                }
                if( !pFrm )
                {
                    pPara->pFrmFmt = 0;
                    pPara->pPrtRect = 0;
                    pPara->pFrmRect = 0;
                }
            }
            break;
        }
    }
    return nRet;
}

USHORT SwFEShell::GetCurColNum( SwGetCurColNumPara* pPara ) const
{
    ASSERT( GetCurrFrm(), "Crsr geparkt?" );
    return _GetCurColNum( GetCurrFrm(), pPara );
}

USHORT SwFEShell::GetCurOutColNum( SwGetCurColNumPara* pPara ) const
{
    USHORT nRet = 0;
    SwFrm* pFrm = GetCurrFrm();
    ASSERT( pFrm, "Crsr geparkt?" );
    if( pFrm )
    {
        pFrm = pFrm->IsInTab() ? (SwFrm*)pFrm->FindTabFrm()
                               : (SwFrm*)pFrm->FindSctFrm();
        ASSERT( pFrm, "No Tab, no Sect" );
        if( pFrm )
            nRet = _GetCurColNum( pFrm, pPara );
    }
    return nRet;
}

SwFEShell::SwFEShell( SwDoc *pDoc,
        uno::Reference<linguistic::XSpellChecker1> &xSpell,
        uno::Reference<linguistic::XHyphenator> &xHyph,
                         Window *pWin,
                         SwRootFrm *pMaster, const SwViewOption *pOpt )
    : SwEditShell( pDoc, xSpell, xHyph, pWin, pMaster, pOpt ),
    pChainFrom( 0 ),
    pChainTo( 0 )
{
}

SwFEShell::SwFEShell( SwEditShell *pShell, Window *pWin )
    : SwEditShell( pShell, pWin ),
    pChainFrom( 0 ),
    pChainTo( 0 )
{
}

SwFEShell::~SwFEShell()
{
    delete pChainFrom;
    delete pChainTo;
}

void SwFEShell::CalcBoundRect( SwRect &rRect, RndStdIds nAnchorId,
    SwRelationOrient eRelOrient, BOOL bMirror, Point* pRef, Size* pPercent ) const
{
    SwFrm *pFrm;
    SwFlyFrm *pFly;
    if( pRef )
    {
        pFrm = GetCurrFrm();
        if( 0 != ( pFly = pFrm->FindFlyFrm() ) )
            pFrm = pFly->GetAnchor();
    }
    else
    {
        pFly = FindFlyFrm();
        pFrm = pFly ? pFly->GetAnchor() : GetCurrFrm();
    }
    SwPageFrm* pPage = pFrm->FindPageFrm();
    bMirror = bMirror && !( pPage->GetVirtPageNum() % 2 );

    Point aPos;
    if( FLY_PAGE == nAnchorId || FLY_AT_FLY == nAnchorId ) // LAYER_IMPL
    {
#ifdef AMA_OUT_OF_FLY
        // Falls wir uns auch ausserhalb des Rahmens aufhalten duerfen
        SwFrm *pTmp = pFrm->FindPageFrm();
        rRect = pTmp->Frm();
        if( FLY_PAGE == nAnchorId )
            pFrm = pTmp;
#else
        SwFrm *pTmp = pFrm;
        if( FLY_PAGE == nAnchorId )
            pFrm = pPage;
        else
            pFrm = pFrm->FindFlyFrm();
        if( !pFrm )
            pFrm = pTmp;
        rRect = pFrm->Frm();
        aPos = pFrm->Frm().Pos();
        if( bMirror )
        {
            switch ( eRelOrient )
            {
                case PRTAREA:
                case REL_PG_PRTAREA: aPos.X() += pFrm->Prt().Width();
                // kein break
                case REL_PG_RIGHT:
                case REL_FRM_RIGHT: aPos.X() += pFrm->Prt().Left(); break;
                default: aPos.X() += pFrm->Frm().Width();
            }
        }
        else
        {
            switch ( eRelOrient )
            {
                case REL_PG_RIGHT:
                case REL_FRM_RIGHT: aPos.X() += pFrm->Prt().Width();
                // kein break!
                case PRTAREA:
                case REL_PG_PRTAREA: aPos.X() += pFrm->Prt().Left(); break;
            }
        }
#endif
        if( pPercent )
            *pPercent = pFrm->Prt().SSize();
    }
    else
    {
        BOOL bAtCntnt = FLY_AT_CNTNT == nAnchorId ||
                        FLY_AUTO_CNTNT == nAnchorId;  // LAYER_IMPL
        if( pRef && !bAtCntnt && pFly && pFly->IsFlyInCntFrm() )
            *pRef = ( (SwFlyInCntFrm*)pFly )->GetRefPoint();

        SwFrm *pUpper = ( pFrm->IsPageFrm() || pFrm->IsFlyFrm() ) ?
                        pFrm : pFrm->GetUpper();
        rRect = pUpper->Frm();
        if( pPercent )
            *pPercent = pUpper->Prt().SSize();
        if( bAtCntnt )
        {
            while( pUpper->IsColumnFrm() || pUpper->IsSctFrm() ||
                   pUpper->IsColBodyFrm() ) // auch ein Rahmen innerhalb einer Spalte darf
                                            // ueber die ganze Seite gehen
                pUpper = pUpper->GetUpper();
        }
        if( !pUpper->IsBodyFrm() )
        {
            rRect += pUpper->Prt().Pos();
            rRect.SSize( pUpper->Prt().SSize() );
            if ( bAtCntnt )
                rRect.Top( pFrm->Frm().Top() );
            if ( pUpper->IsCellFrm() )//MA_FLY_HEIGHT
            {
                SwFrm *pTab = pUpper->FindTabFrm();
                long nBottom = pTab->GetUpper()->Frm().Top() +
                               pTab->GetUpper()->Prt().Bottom();
                rRect.Bottom( nBottom );
            }
        }
        if( bAtCntnt )
        {
            rRect.Left( pPage->Frm().Left() );
            rRect.Width( pPage->Frm().Width() );
        }
        else  // bei zeichengebundenen lieber nur 90% der Hoehe ausnutzen
            rRect.Height( (rRect.Height()*9)/10 );

        aPos = pFrm->Frm().Pos();

        if( bMirror )
        {
            switch ( eRelOrient )
            {
                case REL_FRM_RIGHT: aPos.X() += pFrm->Prt().Left(); break;
                case FRAME:
                case REL_FRM_LEFT: aPos.X() += pFrm->Frm().Width(); break;
                case PRTAREA: aPos.X() += pFrm->Prt().Right(); break;
                case REL_PG_LEFT:
                case REL_PG_FRAME: aPos.X() = pPage->Frm().Right(); break;
                case REL_PG_PRTAREA: aPos.X() = pPage->Frm().Left()
                                              + pPage->Prt().Left(); break;
            }
        }
        else
        {
            switch ( eRelOrient )
            {
                case REL_FRM_RIGHT: aPos.X() += pFrm->Prt().Width();
                // kein break!
                case PRTAREA: aPos += pFrm->Prt().Pos(); break;

                case REL_PG_RIGHT: aPos.X() = pPage->Frm().Left()
                                            + pPage->Prt().Right(); break;
                case REL_PG_PRTAREA: aPos.X() = pPage->Frm().Left()
                                              + pPage->Prt().Left(); break;
                case REL_PG_LEFT:
                case REL_PG_FRAME: aPos.X() = pPage->Frm().Left(); break;
            }
        }

    }
    if( !pRef )
    {
        rRect.Pos( rRect.Left() - aPos.X(), rRect.Top() - aPos.Y() );
        if( bMirror )
            rRect.Pos( -rRect.Right(), rRect.Top() );
    }
}

Size SwFEShell::GetGraphicDefaultSize() const
{
    Size aRet;
    SwFlyFrm *pFly = FindFlyFrm();
    if ( pFly )
    {
        aRet = pFly->GetAnchor()->Prt().SSize();

        SwRect aBound;
        CalcBoundRect( aBound, pFly->GetFmt()->GetAnchor().GetAnchorId());
        aRet.Height() = aBound.Height();
    }
    return aRet;
}


