/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <tools/list.hxx>
#include <svx/svdobj.hxx>
#include <init.hxx>
#include <fesh.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <cntfrm.hxx>
#include <doc.hxx>
#include <frmtool.hxx>
#include <swtable.hxx>
#include <viewimp.hxx>
#include <dview.hxx>
#include <flyfrm.hxx>
#include <node.hxx>
#include <pam.hxx>
#include <sectfrm.hxx>
#include <fmtpdsc.hxx>
#include <fmtsrnd.hxx>
#include <fmtcntnt.hxx>
#include <tabfrm.hxx>
#include <cellfrm.hxx>
#include <flyfrms.hxx>
#include <txtfrm.hxx>       // SwTxtFrm
#include <mdiexp.hxx>
#include <edimp.hxx>
#include <pagedesc.hxx>
#include <fmtanchr.hxx>
// OD 29.10.2003 #113049#
#include <environmentofanchoredobject.hxx>
// OD 12.11.2003 #i22341#
#include <ndtxt.hxx>
// OD 27.11.2003 #112045#
#include <dflyobj.hxx>
// OD 2004-03-29 #i26791#
#include <dcontact.hxx>


using namespace com::sun::star;


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

Point SwFEShell::GetCntntPos( const Point& rPoint, sal_Bool bNext ) const
{
    SET_CURR_SHELL( (ViewShell*)this );
    return GetLayout()->GetNextPrevCntntPos( rPoint, bNext );
}


const SwRect& SwFEShell::GetAnyCurRect( CurRectType eType, const Point* pPt,
                                        const uno::Reference < embed::XEmbeddedObject >& xObj ) const
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
            pFrm = pNd->getLayoutFrm( GetLayout(), pPt );
        }
        else
        {
            const bool bOldCallbackActionEnabled = GetLayout()->IsCallbackActionEnabled();
            if( bOldCallbackActionEnabled )
                GetLayout()->SetCallbackActionEnabled( sal_False );
            pFrm = GetCurrFrm();
            if( bOldCallbackActionEnabled )
                GetLayout()->SetCallbackActionEnabled( sal_True );
        }
    }

    if( !pFrm )
        return GetLayout()->Frm();

    sal_Bool  bFrm  = sal_True;
    switch ( eType )
    {
        case RECT_PAGE_PRT:         bFrm = sal_False; /* no break */
        case RECT_PAGE :            pFrm = pFrm->FindPageFrm();
                                    break;

        case RECT_PAGE_CALC:        pFrm->Calc();
                                    pFrm = pFrm->FindPageFrm();
                                    pFrm->Calc();
                                    break;

        case RECT_FLY_PRT_EMBEDDED: bFrm = sal_False; /* no break */
        case RECT_FLY_EMBEDDED:     pFrm = xObj.is() ? FindFlyFrm( xObj )
                                                : pFrm->IsFlyFrm()
                                                    ? pFrm
                                                    : pFrm->FindFlyFrm();
                                    break;

        case RECT_OUTTABSECTION_PRT:
        case RECT_OUTTABSECTION :   if( pFrm->IsInTab() )
                                        pFrm = pFrm->FindTabFrm();
                                    else {
                                        ASSERT( sal_False, "Missing Table" );
                                    }
                                    /* KEIN BREAK */
        case RECT_SECTION_PRT:
        case RECT_SECTION:          if( pFrm->IsInSct() )
                                        pFrm = pFrm->FindSctFrm();
                                    else {
                                        ASSERT( sal_False, "Missing section" );
                                    }

                                    if( RECT_OUTTABSECTION_PRT == eType ||
                                        RECT_SECTION_PRT == eType )
                                        bFrm = sal_False;
                                    break;

        case RECT_HEADERFOOTER_PRT: bFrm = sal_False; /* no break */
        case RECT_HEADERFOOTER:     if( 0 == (pFrm = pFrm->FindFooterOrHeader()) )
                                        return GetLayout()->Frm();
                                    break;

        case RECT_PAGES_AREA:       return GetLayout()->GetPagesArea();

        default:                    break;
    }
    return bFrm ? pFrm->Frm() : pFrm->Prt();
}


sal_uInt16 SwFEShell::GetPageNumber( const Point &rPoint ) const
{
    const SwFrm *pPage = GetLayout()->Lower();
    while ( pPage && !pPage->Frm().IsInside( rPoint ) )
        pPage = pPage->GetNext();
    if ( pPage )
        return ((const SwPageFrm*)pPage)->GetPhyPageNum();
    else
        return 0;
}


sal_Bool SwFEShell::GetPageNumber( long nYPos, sal_Bool bAtCrsrPos, sal_uInt16& rPhyNum, sal_uInt16& rVirtNum, String &rDisplay) const
{
    const SwFrm *pPage;

    if ( bAtCrsrPos )                   //Seite vom Crsr besorgen
    {
        pPage = GetCurrFrm( sal_False );
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

/*************************************************************************
|*
|*  SwFEShell::IsDirectlyInSection()
|*
|*  Hack for OS:
|*
*************************************************************************/

bool SwFEShell::IsDirectlyInSection() const
{
    SwFrm* pFrm = GetCurrFrm( sal_False );
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

sal_uInt16 SwFEShell::GetFrmType( const Point *pPt, sal_Bool bStopAtFly ) const
{
    sal_uInt16 nReturn = FRMTYPE_NONE;
    const SwFrm *pFrm;
    if ( pPt )
    {
        SwPosition aPos( *GetCrsr()->GetPoint() );
        Point aPt( *pPt );
        GetLayout()->GetCrsrOfst( &aPos, aPt );
        SwCntntNode *pNd = aPos.nNode.GetNode().GetCntntNode();
        pFrm = pNd->getLayoutFrm( GetLayout(), pPt );
    }
    else
        pFrm = GetCurrFrm( sal_False );
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

void SwFEShell::ShGetFcs( sal_Bool bUpdate )
{
    ::SetShell( this );
    SwCrsrShell::ShGetFcs( bUpdate );

    if ( HasDrawView() )
    {
        Imp()->GetDrawView()->showMarkHandles();
        if ( Imp()->GetDrawView()->AreObjectsMarked() )
            FrameNotify( this, FLY_DRAG_START );
    }
}

void SwFEShell::ShLooseFcs()
{
    SwCrsrShell::ShLooseFcs();

    if ( HasDrawView() && Imp()->GetDrawView()->AreObjectsMarked() )
    {
        Imp()->GetDrawView()->hideMarkHandles();
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

sal_uInt16 SwFEShell::GetPhyPageNum()
{
    SwFrm *pFrm = GetCurrFrm();
    if ( pFrm )
        return pFrm->GetPhyPageNum();
    return 0;
}

sal_uInt16 SwFEShell::GetVirtPageNum( const sal_Bool bCalcFrm )
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
|*  sal_uInt16 SwFEShell::GetPageOffset() const
|*
|*  Ersterstellung      OK 07.07.93 08:20
|*  Letzte Aenderung    MA 30. Mar. 95
|*
*************************************************************************/

void lcl_SetAPageOffset( sal_uInt16 nOffset, SwPageFrm* pPage, SwFEShell* pThis )
{
    pThis->StartAllAction();
    ASSERT( pPage->FindFirstBodyCntnt(),
            "SwFEShell _SetAPageOffset() ohne CntntFrm" );

    SwFmtPageDesc aDesc( pPage->GetPageDesc() );
    aDesc.SetNumOffset( nOffset );

    SwFrm *pFrm = pThis->GetCurrFrm( sal_False );
    if ( pFrm->IsInTab() )
        pThis->GetDoc()->SetAttr( aDesc, *pFrm->FindTabFrm()->GetFmt() );
    else
    {
        pThis->GetDoc()->InsertPoolItem( *pThis->GetCrsr(), aDesc, 0 );
    }

    pThis->EndAllAction();
}

void SwFEShell::SetNewPageOffset( sal_uInt16 nOffset )
{
    GetLayout()->SetVirtPageNum( sal_True );
    const SwPageFrm *pPage = GetCurrFrm( sal_False )->FindPageFrm();
    lcl_SetAPageOffset( nOffset, (SwPageFrm*)pPage, this );
}

void SwFEShell::SetPageOffset( sal_uInt16 nOffset )
{
    const SwPageFrm *pPage = GetCurrFrm( sal_False )->FindPageFrm();
    const SwRootFrm* pDocLayout = GetLayout();
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
                pDocLayout->SetVirtPageNum( sal_True );
                lcl_SetAPageOffset( nOffset, (SwPageFrm*)pPage, this );
                break;
            }
        }
        pPage = (SwPageFrm*)pPage->GetPrev();
    }
}

sal_uInt16 SwFEShell::GetPageOffset() const
{
    const SwPageFrm *pPage = GetCurrFrm()->FindPageFrm();
    while ( pPage )
    {
        const SwFrm *pFlow = pPage->FindFirstBodyCntnt();
        if ( pFlow )
        {
            if ( pFlow->IsInTab() )
                pFlow = pFlow->FindTabFrm();
            const sal_uInt16 nOffset = pFlow->GetAttrSet()->GetPageDesc().GetNumOffset();
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

void SwFEShell::InsertLabel( const SwLabelType eType, const String &rTxt, const String& rSeparator,
                             const String& rNumberSeparator,
                             const sal_Bool bBefore, const sal_uInt16 nId,
                             const String& rCharacterStyle,
                             const sal_Bool bCpyBrd )
{
    //NodeIndex der CrsrPosition besorgen, den Rest kann das Dokument
    //selbst erledigen.
    SwCntntFrm *pCnt = LTYPE_DRAW==eType ? 0 : GetCurrFrm( sal_False );
    if( LTYPE_DRAW==eType || pCnt )
    {
        StartAllAction();

        sal_uLong nIdx = 0;
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
                    for ( sal_uInt16 i = 0; i < rMrkList.GetMarkCount(); ++i )
                    {
                        SdrObject* pDrawObj = rMrkList.GetMark(i)->GetMarkedSdrObj();
                        if( pDrawObj )
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
                            GetDoc()->InsertDrawLabel( rTxt, rSeparator, rNumberSeparator, nId, rCharacterStyle, *pDrawObj );
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
            pFlyFmt = GetDoc()->InsertLabel( eType, rTxt, rSeparator, rNumberSeparator, bBefore, nId,
                                             nIdx, rCharacterStyle, bCpyBrd );

        SwFlyFrm* pFrm;
        const Point aPt( GetCrsrDocPos() );
        if( pFlyFmt && 0 != ( pFrm = pFlyFmt->GetFrm( &aPt )))
            SelectFlyFrm( *pFrm, sal_True );

        EndAllActionAndCall();
    }
}


/***********************************************************************
#*  Class       :  SwFEShell
#*  Methoden    :  Sort
#*  Datum       :  ??
#*  Update      :  ??
#***********************************************************************/

sal_Bool SwFEShell::Sort(const SwSortOptions& rOpt)
{
    if( !HasSelection() )
        return sal_False;

    SET_CURR_SHELL( this );
    sal_Bool bRet;
    StartAllAction();
    if(IsTableMode())
    {
        // Tabelle sortieren
        // pruefe ob vom aktuellen Crsr der SPoint/Mark in einer Tabelle stehen
        SwFrm *pFrm = GetCurrFrm( sal_False );
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
            sal_uLong nOffset = pEnd->nNode.GetIndex() - pStart->nNode.GetIndex();
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

sal_uInt16 SwFEShell::_GetCurColNum( const SwFrm *pFrm,
                                SwGetCurColNumPara* pPara ) const
{
    sal_uInt16 nRet = 0;
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

sal_uInt16 SwFEShell::GetCurColNum( SwGetCurColNumPara* pPara ) const
{
    ASSERT( GetCurrFrm(), "Crsr geparkt?" );
    return _GetCurColNum( GetCurrFrm(), pPara );
}

sal_uInt16 SwFEShell::GetCurOutColNum( SwGetCurColNumPara* pPara ) const
{
    sal_uInt16 nRet = 0;
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

SwFEShell::SwFEShell( SwDoc& rDoc, Window *pWindow, const SwViewOption *pOptions )
    : SwEditShell( rDoc, pWindow, pOptions ),
    pChainFrom( 0 ), pChainTo( 0 ), bCheckForOLEInCaption( sal_False )
{
}

SwFEShell::SwFEShell( SwEditShell& rShell, Window *pWindow )
    : SwEditShell( rShell, pWindow ),
    pChainFrom( 0 ), pChainTo( 0 ), bCheckForOLEInCaption( sal_False )
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
                               const sal_Int16 _eHoriRelOrient,
                               const sal_Int16 _eVertRelOrient,
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
    sal_Bool bVertic = sal_False;
    sal_Bool bRTL = sal_False;

    if ((FLY_AT_PAGE == _nAnchorId) || (FLY_AT_FLY == _nAnchorId)) // LAYER_IMPL
    {
        const SwFrm* pTmp = pFrm;
        // OD 06.11.2003 #i22305#
        if ((FLY_AT_PAGE == _nAnchorId) ||
            ((FLY_AT_FLY == _nAnchorId) && !_bFollowTextFlow))
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
            bVertic = sal_True;
            _bMirror = false; // no mirroring in vertical environment
            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::PAGE_RIGHT:
                case text::RelOrientation::FRAME_RIGHT: aPos.Y() += pFrm->Prt().Height();
                // no break!
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA: aPos.Y() += pFrm->Prt().Top(); break;
                default: break;
            }
        }
        else if ( _bMirror )
        {
            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA: aPos.X() += pFrm->Prt().Width();
                // kein break
                case text::RelOrientation::PAGE_RIGHT:
                case text::RelOrientation::FRAME_RIGHT: aPos.X() += pFrm->Prt().Left(); break;
                default: aPos.X() += pFrm->Frm().Width();
            }
        }
        else if ( bRTL )
        {
            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA: aPos.X() += pFrm->Prt().Width();
                // kein break!
                case text::RelOrientation::PAGE_LEFT:
                case text::RelOrientation::FRAME_LEFT: aPos.X() += pFrm->Prt().Left() -
                                               pFrm->Frm().Width(); break;
                default: break;
            }
        }
        else
        {
            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::PAGE_RIGHT:
                case text::RelOrientation::FRAME_RIGHT:   aPos.X() += pFrm->Prt().Width();
                // kein break!
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA: aPos.X() += pFrm->Prt().Left(); break;
                default:break;
            }
        }
        // --> OD 2006-12-12 #i67221# - proposed patch
        if( bVert )
        {
            switch ( _eVertRelOrient )
            {
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA:
                {
                    aPos.X() -= pFrm->GetRightMargin();
                }
                break;
            }
        }
        else
        {
            switch ( _eVertRelOrient )
            {
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA:
                {
                    if ( pFrm->IsPageFrm() )
                    {
                        aPos.Y() =
                            static_cast<const SwPageFrm*>(pFrm)->PrtWithoutHeaderAndFooter().Top();
                    }
                    else
                    {
                        aPos.Y() += pFrm->Prt().Top();
                    }
                }
                break;
            }
        }
        // <--
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
        if ((_nAnchorId == FLY_AT_PARA) || (_nAnchorId == FLY_AT_CHAR))
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
                                aEnvOfObj.GetHoriEnvironmentLayoutFrm( *pFrm );
            const SwLayoutFrm& rVertEnvironLayFrm =
                                aEnvOfObj.GetVertEnvironmentLayoutFrm( *pFrm );
            SwRect aHoriEnvironRect( rHoriEnvironLayFrm.Frm() );
            SwRect aVertEnvironRect;
            if ( _bFollowTextFlow )
            {
                aVertEnvironRect = rVertEnvironLayFrm.Prt();
                aVertEnvironRect.Pos() += rVertEnvironLayFrm.Frm().Pos();
                // OD 19.09.2003 #i18732# - adjust vertical 'virtual' anchor position
                // (<aPos.Y()> respectively <aPos.X()>), if object is vertical aligned
                // to page areas.
                if ( _eVertRelOrient == text::RelOrientation::PAGE_FRAME || _eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA )
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
                if ( _eVertRelOrient == text::RelOrientation::PAGE_FRAME || _eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA )
                {
                    if ( bVert )
                    {
                        aPos.X() = aVertEnvironRect.Right();
                        if ( _eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA )
                        {
                            aPos.X() -= rVertEnvironLayFrm.GetRightMargin();
                        }
                    }
                    else
                    {
                        aPos.Y() = aVertEnvironRect.Top();
                        if ( _eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA )
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
            // --> OD 2005-12-29 #125800#
            // <pFrm>, which is the anchor frame or the proposed anchor frame,
            // doesn't have to be a text frame (e.g. edit a to-page anchored
            // fly frame). Thus, assure this.
            const SwTxtFrm* pTxtFrm( dynamic_cast<const SwTxtFrm*>(pFrm) );
            if ( pTxtFrm &&
                 (_nAnchorId == FLY_AT_CHAR) &&
                 ( _eVertRelOrient == text::RelOrientation::CHAR ||
                   _eVertRelOrient == text::RelOrientation::TEXT_LINE ) )
            {
                SwTwips nTop = 0L;
                if ( _eVertRelOrient == text::RelOrientation::CHAR )
                {
                    SwRect aChRect;
                    if ( _pToCharCntntPos )
                    {
                        pTxtFrm->GetAutoPos( aChRect, *_pToCharCntntPos );
                    }
                    else
                    {
                        // No content position provided. Thus, use a default one.
                        SwPosition aDefaultCntntPos( *(pTxtFrm->GetTxtNode()) );
                        pTxtFrm->GetAutoPos( aChRect, aDefaultCntntPos );
                    }
                    nTop = (aChRect.*fnRect->fnGetBottom)();
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

            // --> OD 2004-10-05 #i26945# - adjust horizontal 'virtual' anchor
            // position (<aPos.X()> respectively <aPos.Y()>), if object is
            // anchored to character and horizontal aligned at character.
            if ( pTxtFrm &&
                 (_nAnchorId == FLY_AT_CHAR) &&
                 _eHoriRelOrient == text::RelOrientation::CHAR )
            {
                SwTwips nLeft = 0L;
                SwRect aChRect;
                if ( _pToCharCntntPos )
                {
                    pTxtFrm->GetAutoPos( aChRect, *_pToCharCntntPos );
                }
                else
                {
                    // No content position provided. Thus, use a default one.
                    SwPosition aDefaultCntntPos( *(pTxtFrm->GetTxtNode()) );
                    pTxtFrm->GetAutoPos( aChRect, aDefaultCntntPos );
                }
                nLeft = (aChRect.*fnRect->fnGetLeft)();
                if ( bVert )
                {
                    aPos.Y() = nLeft;
                }
                else
                {
                    aPos.X() = nLeft;
                }
            }
            // <--

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
            bVertic = sal_True;
            _bMirror = false;

            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::FRAME_RIGHT: aPos.Y() += pFrm->Prt().Height();
                                    aPos += (pFrm->Prt().*fnRect->fnGetPos)();
                                    break;
                case text::RelOrientation::PRINT_AREA: aPos += (pFrm->Prt().*fnRect->fnGetPos)();
                              aPos.Y() += nBaseOfstForFly;
                              break;
                case text::RelOrientation::PAGE_RIGHT: aPos.Y() = pPage->Frm().Top()
                                            + pPage->Prt().Bottom(); break;
                case text::RelOrientation::PAGE_PRINT_AREA: aPos.Y() = pPage->Frm().Top()
                                              + pPage->Prt().Top(); break;
                case text::RelOrientation::PAGE_LEFT:
                case text::RelOrientation::PAGE_FRAME: aPos.Y() = pPage->Frm().Top(); break;
                case text::RelOrientation::FRAME: aPos.Y() += nBaseOfstForFly; break;
                default: break;
            }
        }
        else if( _bMirror )
        {
            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::FRAME_RIGHT:   aPos.X() += pFrm->Prt().Left(); break;
                case text::RelOrientation::FRAME:
                case text::RelOrientation::FRAME_LEFT: aPos.X() += pFrm->Frm().Width(); break;
                case text::RelOrientation::PRINT_AREA: aPos.X() += pFrm->Prt().Right(); break;
                case text::RelOrientation::PAGE_LEFT:
                case text::RelOrientation::PAGE_FRAME: aPos.X() = pPage->Frm().Right(); break;
                case text::RelOrientation::PAGE_PRINT_AREA: aPos.X() = pPage->Frm().Left()
                                              + pPage->Prt().Left(); break;
                default: break;
            }
        }
        else if ( bRTL )
        {
            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::FRAME_LEFT:
                    aPos.X() = pFrm->Frm().Left() +
                               pFrm->Prt().Left();
                    break;

                case text::RelOrientation::PRINT_AREA:
                    aPos.X() = pFrm->Frm().Left() + pFrm->Prt().Left() +
                               pFrm->Prt().Width();
                    aPos.X() += nBaseOfstForFly;
                    break;

                case text::RelOrientation::PAGE_LEFT:
                    aPos.X() = pPage->Frm().Left() + pPage->Prt().Left();
                    break;

                case text::RelOrientation::PAGE_PRINT_AREA:
                    aPos.X() = pPage->Frm().Left() + pPage->Prt().Left() +
                               pPage->Prt().Width() ;
                    break;

                case text::RelOrientation::PAGE_RIGHT:
                case text::RelOrientation::PAGE_FRAME:
                    aPos.X() = pPage->Frm().Right();
                    break;

                case text::RelOrientation::FRAME:
                    aPos.X() += nBaseOfstForFly;
                    break;
                default: break;
            }
        }
        else
        {
            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::FRAME_RIGHT:   aPos.X() += pFrm->Prt().Width();
                                    aPos += pFrm->Prt().Pos();
                                    break;
                case text::RelOrientation::PRINT_AREA: aPos += pFrm->Prt().Pos();
                              aPos.X() += nBaseOfstForFly;
                              break;
                case text::RelOrientation::PAGE_RIGHT: aPos.X() = pPage->Frm().Left()
                                            + pPage->Prt().Right(); break;
                case text::RelOrientation::PAGE_PRINT_AREA: aPos.X() = pPage->Frm().Left()
                                              + pPage->Prt().Left(); break;
                case text::RelOrientation::PAGE_LEFT:
                case text::RelOrientation::PAGE_FRAME: aPos.X() = pPage->Frm().Left(); break;
                case text::RelOrientation::FRAME: aPos.X() += nBaseOfstForFly; break;
                default: break;
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
        // --> OD 2004-09-24 #i32951# - due to issue #i28701# no format of a
        // newly inserted Writer fly frame or its anchor frame is performed
        // any more. Thus, it could be possible (e.g. on insert of a horizontal
        // line) that the anchor frame isn't formatted and its printing area
        // size is (0,0). If this is the case the printing area of the upper
        // of the anchor frame is taken.
        const SwFrm* pAnchorFrm = pFly->GetAnchorFrm();
        aRet = pAnchorFrm->Prt().SSize();
        if ( aRet.Width() == 0 && aRet.Height() == 0 &&
             pAnchorFrm->GetUpper() )
        {
            aRet = pAnchorFrm->GetUpper()->Prt().SSize();
        }
        // <--

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
sal_Bool SwFEShell::IsFrmVertical(sal_Bool bEnvironment, sal_Bool& bRTL) const
{
    sal_Bool bVert = sal_False;
    bRTL = sal_False;

    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        if( rMrkList.GetMarkCount() != 1 )
            return bVert;

        SdrObject* pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
        // --> OD 2006-01-06 #123831# - make code robust:
        if ( !pObj )
        {
            ASSERT( false,
                    "<SwFEShell::IsFrmVertical(..)> - missing SdrObject instance in marked object list -> This is a serious situation, please inform OD" );
            return bVert;
        }
        // <--
        // OD 2004-03-29 #i26791#
        SwContact* pContact = static_cast<SwContact*>(GetUserCall( pObj ));
        // --> OD 2006-01-06 #123831# - make code robust:
        if ( !pContact )
        {
            ASSERT( false,
                    "<SwFEShell::IsFrmVertical(..)> - missing SwContact instance at marked object -> This is a serious situation, please inform OD" );
            return bVert;
        }
        // <--
        const SwFrm* pRef = pContact->GetAnchoredObj( pObj )->GetAnchorFrm();
        // --> OD 2006-01-06 #123831# - make code robust:
        if ( !pRef )
        {
            ASSERT( false,
                    "<SwFEShell::IsFrmVertical(..)> - missing anchor frame at marked object -> This is a serious situation, please inform OD" );
            return bVert;
        }
        // <--

        if ( pObj->ISA(SwVirtFlyDrawObj) && !bEnvironment )
            pRef = static_cast<const SwVirtFlyDrawObj*>(pObj)->GetFlyFrm();

        bVert = pRef->IsVertical();
        bRTL = pRef->IsRightToLeft();
    }

    return bVert;
}

void SwFEShell::MoveObjectIfActive( svt::EmbeddedObjectRef&, const Point& )
{
    // does not do anything, only avoids crash if the method is used for wrong shell
}

