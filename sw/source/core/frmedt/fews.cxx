/*************************************************************************
 *
 *  $RCSfile: fews.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:48:06 $
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
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
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
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>       // SwTxtFrm
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
// OD 29.10.2003 #113049#
#ifndef _ENVIRONMENTOFANCHOREDOBJECT_HXX
#include <environmentofanchoredobject.hxx>
#endif
// OD 12.11.2003 #i22341#
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
// OD 27.11.2003 #112045#
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
// OD 2004-03-29 #i26791#
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif

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
    return GetLayout()->GetNextPrevCntntPos( rPoint, bNext );
}


const SwRect& SwFEShell::GetAnyCurRect( CurRectType eType, const Point* pPt,
                                        const SvEmbeddedObject *pObj ) const
{
    const SwFrm *pFrm = Imp()->HasDrawView()
                ? ::GetFlyFromMarked( &Imp()->GetDrawView()->GetMarkedObjectList(),
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
        const SvxNumberType& rNum = ((const SwPageFrm*)pPage)->GetPageDesc()->GetNumType();
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
|*  SwFEShell::IsDirectlyInSection()
|*
|*  Hack for OS:
|*
*************************************************************************/

bool SwFEShell::IsDirectlyInSection() const
{
    SwFrm* pFrm = GetCurrFrm( FALSE );
    return pFrm && pFrm->GetUpper() && pFrm->GetUpper()->IsSctFrm();
}

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
    const SwFrm *pFrm;
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
            pFrm = ((SwFlyFrm*)pFrm)->GetAnchorFrm();
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
        if ( Imp()->GetDrawView()->AreObjectsMarked() )
            FrameNotify( this, FLY_DRAG_START );
    }
}

void SwFEShell::ShLooseFcs()
{
    SwCrsrShell::ShLooseFcs();

    if ( HasDrawView() && Imp()->GetDrawView()->AreObjectsMarked() )
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
                const SdrMarkList& rMrkList = pDView->GetMarkedObjectList();
                StartUndo();

                // OD 27.11.2003 #112045# - copy marked drawing objects to
                // local list to perform the corresponding action for each object
                std::vector<SdrObject*> aDrawObjs;
                {
                    for ( USHORT i = 0; i < rMrkList.GetMarkCount(); ++i )
                    {
                        SdrObject* pDrawObj = rMrkList.GetMark(i)->GetObj();
                        aDrawObjs.push_back( pDrawObj );
                    }
                }
                // loop on marked drawing objects
                while ( !aDrawObjs.empty() )
                {
                    SdrObject* pDrawObj = aDrawObjs.back();
                    if ( !pDrawObj->ISA(SwVirtFlyDrawObj) &&
                         !pDrawObj->ISA(SwFlyDrawObj) )
                    {
                        SwFlyFrmFmt *pFmt =
                            GetDoc()->InsertDrawLabel( rTxt, nId, *pDrawObj );
                        if( !pFlyFmt )
                            pFlyFmt = pFmt;
                    }

                    aDrawObjs.pop_back();
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
            /* #107993# ParkCursor->ParkCursorTab */
            ParkCursorInTab();
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

SwFEShell::SwFEShell( SwDoc& rDoc, Window *pWin,
                     SwRootFrm *pMaster, const SwViewOption *pOpt )
    : SwEditShell( rDoc, pWin, pMaster, pOpt ),
    pChainFrom( 0 ), pChainTo( 0 ), bCheckForOLEInCaption( FALSE )
{
}

SwFEShell::SwFEShell( SwEditShell& rShell, Window *pWin )
    : SwEditShell( rShell, pWin ),
    pChainFrom( 0 ), pChainTo( 0 ), bCheckForOLEInCaption( FALSE )
{
}

SwFEShell::~SwFEShell()
{
    delete pChainFrom;
    delete pChainTo;
}

// OD 18.09.2003 #i17567#, #108749#, #110354# - adjustments for allowing
//          negative vertical positions for fly frames anchored to paragraph/to character.
// OD 06.11.2003 #i22305# - adjustments for option 'Follow text flow'
//          for to frame anchored objects.
// OD 12.11.2003 #i22341# - adjustments for vertical alignment at top of line
//          for to character anchored objects.
void SwFEShell::CalcBoundRect( SwRect& _orRect,
                               const RndStdIds _nAnchorId,
                               const SwRelationOrient _eHoriRelOrient,
                               const SwRelationOrient _eVertRelOrient,
                               const SwPosition* _pToCharCntntPos,
                               const bool _bFollowTextFlow,
                               bool _bMirror,
                               Point* _opRef,
                               Size* _opPercent ) const
{
    const SwFrm* pFrm;
    const SwFlyFrm* pFly;
    if( _opRef )
    {
        pFrm = GetCurrFrm();
        if( 0 != ( pFly = pFrm->FindFlyFrm() ) )
            pFrm = pFly->GetAnchorFrm();
    }
    else
    {
        pFly = FindFlyFrm();
        pFrm = pFly ? pFly->GetAnchorFrm() : GetCurrFrm();
    }

    sal_Bool bWrapThrough = sal_False;
    if ( pFly )
    {
        SwFlyFrmFmt* pFmt = (SwFlyFrmFmt*)pFly->GetFmt();
        const SwFmtSurround& rSurround = pFmt->GetSurround();
        bWrapThrough = rSurround.GetSurround() == SURROUND_THROUGHT;
    }

    const SwPageFrm* pPage = pFrm->FindPageFrm();
    _bMirror = _bMirror && !pPage->OnRightPage();

    Point aPos;
    BOOL bVertic = FALSE;
    BOOL bRTL = FALSE;

    if( FLY_PAGE == _nAnchorId || FLY_AT_FLY == _nAnchorId ) // LAYER_IMPL
    {
        const SwFrm* pTmp = pFrm;
        // OD 06.11.2003 #i22305#
        if ( FLY_PAGE == _nAnchorId ||
             ( FLY_AT_FLY == _nAnchorId && !_bFollowTextFlow ) )
        {
            pFrm = pPage;
        }
        else
        {
            pFrm = pFrm->FindFlyFrm();
        }
        if ( !pFrm )
            pFrm = pTmp;
        _orRect = pFrm->Frm();
        SWRECTFN( pFrm )
        bRTL = pFrm->IsRightToLeft();
        if ( bRTL )
            aPos = pFrm->Frm().TopRight();
        else
            aPos = (pFrm->Frm().*fnRect->fnGetPos)();

        if( bVert )
        {
            bVertic = TRUE;
            _bMirror = false; // no mirroring in vertical environment
            switch ( _eHoriRelOrient )
            {
                case REL_PG_RIGHT:
                case REL_FRM_RIGHT: aPos.Y() += pFrm->Prt().Height();
                // no break!
                case PRTAREA:
                case REL_PG_PRTAREA: aPos.Y() += pFrm->Prt().Top(); break;
            }
        }
        else if ( _bMirror )
        {
            switch ( _eHoriRelOrient )
            {
                case PRTAREA:
                case REL_PG_PRTAREA: aPos.X() += pFrm->Prt().Width();
                // kein break
                case REL_PG_RIGHT:
                case REL_FRM_RIGHT: aPos.X() += pFrm->Prt().Left(); break;
                default: aPos.X() += pFrm->Frm().Width();
            }
        }
        else if ( bRTL )
        {
            switch ( _eHoriRelOrient )
            {
                case PRTAREA:
                case REL_PG_PRTAREA: aPos.X() += pFrm->Prt().Width();
                // kein break!
                case REL_PG_LEFT:
                case REL_FRM_LEFT: aPos.X() += pFrm->Prt().Left() -
                                               pFrm->Frm().Width(); break;
            }
        }
        else
        {
            switch ( _eHoriRelOrient )
            {
                case REL_PG_RIGHT:
                case REL_FRM_RIGHT: aPos.X() += pFrm->Prt().Width();
                // kein break!
                case PRTAREA:
                case REL_PG_PRTAREA: aPos.X() += pFrm->Prt().Left(); break;
            }
        }
        if ( _opPercent )
            *_opPercent = pFrm->Prt().SSize();
    }
    else
    {
        const SwFrm* pUpper = ( pFrm->IsPageFrm() || pFrm->IsFlyFrm() ) ?
                              pFrm : pFrm->GetUpper();
        SWRECTFN( pUpper );
        if ( _opPercent )
            *_opPercent = pUpper->Prt().SSize();

        bRTL = pFrm->IsRightToLeft();
        if ( bRTL )
            aPos = pFrm->Frm().TopRight();
        else
            aPos = (pFrm->Frm().*fnRect->fnGetPos)();

        // OD 08.09.2003 #i17567#, #108749#, #110354# - allow negative positions
        // for fly frames anchor to paragraph/to character.
        if ( _nAnchorId == FLY_AT_CNTNT || _nAnchorId == FLY_AUTO_CNTNT )
        {
            // The rectangle, the fly frame can be positioned in, is determined
            // horizontally by the frame area of the horizontal environment
            // and vertically by the printing area of the vertical environment,
            // if the object follows the text flow, or by the frame area of the
            // vertical environment, if the object doesn't follow the text flow.
            // OD 29.10.2003 #113049# - new class <SwEnvironmentOfAnchoredObject>
            objectpositioning::SwEnvironmentOfAnchoredObject aEnvOfObj(
                                                            _bFollowTextFlow );
            const SwLayoutFrm& rHoriEnvironLayFrm =
                                aEnvOfObj.GetHoriEnvironmentLayoutFrm( *pFrm, false );
            const SwLayoutFrm& rVertEnvironLayFrm =
                                aEnvOfObj.GetVertEnvironmentLayoutFrm( *pFrm, false );
            SwRect aHoriEnvironRect( rHoriEnvironLayFrm.Frm() );
            SwRect aVertEnvironRect;
            if ( _bFollowTextFlow )
            {
                aVertEnvironRect = rVertEnvironLayFrm.Prt();
                aVertEnvironRect.Pos() += rVertEnvironLayFrm.Frm().Pos();
                // OD 19.09.2003 #i18732# - adjust vertical 'virtual' anchor position
                // (<aPos.Y()> respectively <aPos.X()>), if object is vertical aligned
                // to page areas.
                if ( _eVertRelOrient == REL_PG_FRAME || _eVertRelOrient == REL_PG_PRTAREA )
                {
                    if ( bVert )
                    {
                        aPos.X() = aVertEnvironRect.Right();
                    }
                    else
                    {
                        aPos.Y() = aVertEnvironRect.Top();
                    }
                }
            }
            else
            {
                ASSERT( rVertEnvironLayFrm.IsPageFrm(),
                        "<SwFEShell::CalcBoundRect(..)> - not following text flow, but vertical environment *not* page!" );
                aVertEnvironRect = rVertEnvironLayFrm.Frm();
                // OD 19.09.2003 #i18732# - adjustment vertical 'virtual' anchor position
                // (<aPos.Y()> respectively <aPos.X()>), if object is vertical aligned
                // to page areas.
                if ( _eVertRelOrient == REL_PG_FRAME || _eVertRelOrient == REL_PG_PRTAREA )
                {
                    if ( bVert )
                    {
                        aPos.X() = aVertEnvironRect.Right();
                        if ( _eVertRelOrient == REL_PG_PRTAREA )
                        {
                            aPos.X() -= rVertEnvironLayFrm.GetRightMargin();
                        }
                    }
                    else
                    {
                        aPos.Y() = aVertEnvironRect.Top();
                        if ( _eVertRelOrient == REL_PG_PRTAREA )
                        {
                            aPos.Y() += rVertEnvironLayFrm.GetTopMargin();
                            // add height of page header
                            const SwFrm* pTmpFrm = rVertEnvironLayFrm.Lower();
                            if ( pTmpFrm->IsHeaderFrm() )
                            {
                                aPos.Y() += pTmpFrm->Frm().Height();
                            }
                        }
                    }
                }
            }

            // OD 12.11.2003 #i22341# - adjust vertical 'virtual' anchor position
            // (<aPos.Y()> respectively <aPos.X()>), if object is anchored to
            // character and vertical aligned at character or top of line
            if ( _nAnchorId == FLY_AUTO_CNTNT &&
                 ( _eVertRelOrient == REL_CHAR ||
                   _eVertRelOrient == REL_VERT_LINE ) )
            {
                ASSERT( pFrm->ISA(SwTxtFrm),
                        "<SwFEShell::CalcBoundRect(..)> - wrong anchor frame." )
                const SwTxtFrm* pTxtFrm = static_cast<const SwTxtFrm*>(pFrm);
                SwTwips nTop = 0L;
                if ( _eVertRelOrient == REL_CHAR )
                {
                    SwRect aCharRect;
                    if ( _pToCharCntntPos )
                    {
                        pTxtFrm->GetAutoPos( aCharRect, *_pToCharCntntPos );
                    }
                    else
                    {
                        // No content position provided. Thus, use a default one.
                        SwPosition aDefaultCntntPos( *(pTxtFrm->GetTxtNode()) );
                        pTxtFrm->GetAutoPos( aCharRect, aDefaultCntntPos );
                    }
                    nTop = (aCharRect.*fnRect->fnGetBottom)();
                }
                else
                {
                    if ( _pToCharCntntPos )
                    {
                        pTxtFrm->GetTopOfLine( nTop, *_pToCharCntntPos );
                    }
                    else
                    {
                        // No content position provided. Thus, use a default one.
                        SwPosition aDefaultCntntPos( *(pTxtFrm->GetTxtNode()) );
                        pTxtFrm->GetTopOfLine( nTop, aDefaultCntntPos );
                    }
                }
                if ( bVert )
                {
                    aPos.X() = nTop;
                }
                else
                {
                    aPos.Y() = nTop;
                }
            }

            if ( bVert )
            {
                _orRect = SwRect( aVertEnvironRect.Left(),
                                  aHoriEnvironRect.Top(),
                                  aVertEnvironRect.Width(),
                                  aHoriEnvironRect.Height() );
            }
            else
            {
                _orRect = SwRect( aHoriEnvironRect.Left(),
                                  aVertEnvironRect.Top(),
                                  aHoriEnvironRect.Width(),
                                  aVertEnvironRect.Height() );
            }
        }
        else
        {
            if( _opRef && pFly && pFly->IsFlyInCntFrm() )
                *_opRef = ( (SwFlyInCntFrm*)pFly )->GetRefPoint();

            _orRect = pUpper->Frm();
            if( !pUpper->IsBodyFrm() )
            {
                _orRect += pUpper->Prt().Pos();
                _orRect.SSize( pUpper->Prt().SSize() );
                if ( pUpper->IsCellFrm() )//MA_FLY_HEIGHT
                {
                    const SwFrm* pTab = pUpper->FindTabFrm();
                    long nBottom = (pTab->GetUpper()->*fnRect->fnGetPrtBottom)();
                    (_orRect.*fnRect->fnSetBottom)( nBottom );
                }
            }
            // bei zeichengebundenen lieber nur 90% der Hoehe ausnutzen
            {
                if( bVert )
                    _orRect.Width( (_orRect.Width()*9)/10 );
                else
                    _orRect.Height( (_orRect.Height()*9)/10 );
            }
        }

        const SwTwips nBaseOfstForFly = ( pFrm->IsTxtFrm() && pFly ) ?
                                        ((SwTxtFrm*)pFrm)->GetBaseOfstForFly( !bWrapThrough ) :
                                         0;
        if( bVert )
        {
            bVertic = TRUE;
            _bMirror = false;

            switch ( _eHoriRelOrient )
            {
                case REL_FRM_RIGHT: aPos.Y() += pFrm->Prt().Height();
                                    aPos += (pFrm->Prt().*fnRect->fnGetPos)();
                                    break;
                case PRTAREA: aPos += (pFrm->Prt().*fnRect->fnGetPos)();
                              aPos.Y() += nBaseOfstForFly;
                              break;
                case REL_PG_RIGHT: aPos.Y() = pPage->Frm().Top()
                                            + pPage->Prt().Bottom(); break;
                case REL_PG_PRTAREA: aPos.Y() = pPage->Frm().Top()
                                              + pPage->Prt().Top(); break;
                case REL_PG_LEFT:
                case REL_PG_FRAME: aPos.Y() = pPage->Frm().Top(); break;
                case FRAME: aPos.Y() += nBaseOfstForFly; break;
            }
        }
        else if( _bMirror )
        {
            switch ( _eHoriRelOrient )
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
        else if ( bRTL )
        {
            switch ( _eHoriRelOrient )
            {
                case REL_FRM_LEFT:
                    aPos.X() = pFrm->Frm().Left() +
                               pFrm->Prt().Left();
                    break;

                case PRTAREA:
                    aPos.X() = pFrm->Frm().Left() + pFrm->Prt().Left() +
                               pFrm->Prt().Width();
                    aPos.X() += nBaseOfstForFly;
                    break;

                case REL_PG_LEFT:
                    aPos.X() = pPage->Frm().Left() + pPage->Prt().Left();
                    break;

                case REL_PG_PRTAREA:
                    aPos.X() = pPage->Frm().Left() + pPage->Prt().Left() +
                               pPage->Prt().Width() ;
                    break;

                case REL_PG_RIGHT:
                case REL_PG_FRAME:
                    aPos.X() = pPage->Frm().Right();
                    break;

                case FRAME:
                    aPos.X() += nBaseOfstForFly;
                    break;
            }
        }
        else
        {
            switch ( _eHoriRelOrient )
            {
                case REL_FRM_RIGHT: aPos.X() += pFrm->Prt().Width();
                                    aPos += pFrm->Prt().Pos();
                                    break;
                case PRTAREA: aPos += pFrm->Prt().Pos();
                              aPos.X() += nBaseOfstForFly;
                              break;
                case REL_PG_RIGHT: aPos.X() = pPage->Frm().Left()
                                            + pPage->Prt().Right(); break;
                case REL_PG_PRTAREA: aPos.X() = pPage->Frm().Left()
                                              + pPage->Prt().Left(); break;
                case REL_PG_LEFT:
                case REL_PG_FRAME: aPos.X() = pPage->Frm().Left(); break;
                case FRAME: aPos.X() += nBaseOfstForFly; break;
            }
        }

    }
    if( !_opRef )
    {
        if( bVertic )
            _orRect.Pos( aPos.X() - _orRect.Width() - _orRect.Left(), _orRect.Top() - aPos.Y() );
        else if ( bRTL )
            _orRect.Pos( - ( _orRect.Right() - aPos.X() ), _orRect.Top() - aPos.Y() );
        else
            _orRect.Pos( _orRect.Left() - aPos.X(), _orRect.Top() - aPos.Y() );
        if( _bMirror )
            _orRect.Pos( -_orRect.Right(), _orRect.Top() );
    }
}

Size SwFEShell::GetGraphicDefaultSize() const
{
    Size aRet;
    SwFlyFrm *pFly = FindFlyFrm();
    if ( pFly )
    {
        aRet = pFly->GetAnchorFrm()->Prt().SSize();

        SwRect aBound;
        CalcBoundRect( aBound, pFly->GetFmt()->GetAnchor().GetAnchorId());
        if ( pFly->GetAnchorFrm()->IsVertical() )
            aRet.Width() = aBound.Width();
        else
            aRet.Height() = aBound.Height();
    }
    return aRet;
}
/* -----------------------------12.08.2002 12:51------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwFEShell::IsFrmVertical(BOOL bEnvironment, BOOL& bRTL) const
{
    BOOL bVert = FALSE;
    bRTL = FALSE;

    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkList();
        if( rMrkList.GetMarkCount() != 1 )
            return bVert;

        SdrObject* pObj = rMrkList.GetMark( 0 )->GetObj();
        // OD 2004-03-29 #i26791#
        SwContact* pContact = static_cast<SwContact*>(GetUserCall( pObj ));
        const SwFrm* pRef = pContact->GetAnchoredObj( pObj )->GetAnchorFrm();

        if ( pObj->ISA(SwVirtFlyDrawObj) && !bEnvironment )
            pRef = static_cast<const SwVirtFlyDrawObj*>(pObj)->GetFlyFrm();

        bVert = pRef->IsVertical();
        bRTL = pRef->IsRightToLeft();
    }

    return bVert;
}

