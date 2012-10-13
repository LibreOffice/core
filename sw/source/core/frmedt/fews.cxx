/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <environmentofanchoredobject.hxx>
#include <ndtxt.hxx> // #i22341#
#include <dflyobj.hxx>
#include <dcontact.hxx> // #i26791#


using namespace com::sun::star;


TYPEINIT1(SwFEShell,SwEditShell)

/***********************************************************************
#*  Class      :  SwFEShell
#*  Method     :  EndAllActionAndCall()
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
#*  Method      :  GetCntntPos
#*  Description :  Determine the Cntnt's nearest to the point
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
                                        OSL_FAIL( "Missing Table" );
                                    }
                                    /* no break */
        case RECT_SECTION_PRT:
        case RECT_SECTION:          if( pFrm->IsInSct() )
                                        pFrm = pFrm->FindSctFrm();
                                    else {
                                        OSL_FAIL( "Missing section" );
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

    if ( bAtCrsrPos )                   // get page of Crsr
    {
        pPage = GetCurrFrm( sal_False );
        if ( pPage )
            pPage = pPage->FindPageFrm();
    }
    else if ( nYPos > -1 )              // determine page via the position
    {
        pPage = GetLayout()->Lower();
        while( pPage &&  (pPage->Frm().Bottom() < nYPos ||
                            nYPos < pPage->Frm().Top() ) )
            pPage = pPage->GetNext();
    }
    else                                // first visible page
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
                                else // only pages and frame columns
                                    nReturn |= FRMTYPE_COLUMN;
                                break;
            case FRM_PAGE:      nReturn |= FRMTYPE_PAGE;
                                if( ((SwPageFrm*)pFrm)->IsFtnPage() )
                                    nReturn |= FRMTYPE_FTNPAGE;
                                break;
            case FRM_HEADER:    nReturn |= FRMTYPE_HEADER;      break;
            case FRM_FOOTER:    nReturn |= FRMTYPE_FOOTER;      break;
            case FRM_BODY:      if( pFrm->GetUpper()->IsPageFrm() ) // not for ColumnFrms
                                    nReturn |= FRMTYPE_BODY;
                                break;
            case FRM_FTN:       nReturn |= FRMTYPE_FOOTNOTE;    break;
            case FRM_FLY:       if( ((SwFlyFrm*)pFrm)->IsFlyLayFrm() )
                                    nReturn |= FRMTYPE_FLY_FREE;
                                else if ( ((SwFlyFrm*)pFrm)->IsFlyAtCntFrm() )
                                    nReturn |= FRMTYPE_FLY_ATCNT;
                                else
                                {
                                    OSL_ENSURE( ((SwFlyFrm*)pFrm)->IsFlyInCntFrm(),
                                            "New frametype?" );
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
}

/*************************************************************************
|*
|*  SwFEShell::GetPhyPageNum()
|*  SwFEShell::GetVirtPageNum()
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
*************************************************************************/

static void lcl_SetAPageOffset( sal_uInt16 nOffset, SwPageFrm* pPage, SwFEShell* pThis )
{
    pThis->StartAllAction();
    OSL_ENSURE( pPage->FindFirstBodyCntnt(),
            "SwFEShell _SetAPageOffset() without CntntFrm" );

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
*************************************************************************/

void SwFEShell::InsertLabel( const SwLabelType eType, const String &rTxt, const String& rSeparator,
                             const String& rNumberSeparator,
                             const sal_Bool bBefore, const sal_uInt16 nId,
                             const String& rCharacterStyle,
                             const sal_Bool bCpyBrd )
{
    // get node index of cursor position, SwDoc can do everything else itself
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
                // pass down index to the startnode for flys
                nIdx = pCnt->FindFlyFrm()->
                            GetFmt()->GetCntnt().GetCntntIdx()->GetIndex();
            }
            break;
        case LTYPE_TABLE:
            if( pCnt->IsInTab() )
            {
                // pass down index to the TblNode for tables
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

                // copy marked drawing objects to
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
            OSL_ENSURE( !this, "Crsr neither in table nor in fly." );
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
#*  Method      :  Sort
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
        // Sort table
        // check if Point/Mark of current Crsr are in one table
        SwFrm *pFrm = GetCurrFrm( sal_False );
        OSL_ENSURE( pFrm->FindTabFrm(), "Crsr not in table." );

        // search boxes via the layout
        SwSelBoxes  aBoxes;
        GetTblSel(*this, aBoxes);

        // The Crsr should be removed from the deletion area.
        // Always put them behind/on the table; via the
        // document position they will always be set to the old position
        while( !pFrm->IsCellFrm() )
            pFrm = pFrm->GetUpper();
        {
            /* ParkCursor->ParkCursorTab */
            ParkCursorInTab();
        }

        // call sorting on document
        bRet = pDoc->SortTbl(aBoxes, rOpt);
    }
    else
    {
        // Sort text nothing else
        FOREACHPAM_START(this)

            SwPaM* pPam = PCURCRSR;

            SwPosition* pStart = pPam->Start();
            SwPosition* pEnd   = pPam->End();

            SwNodeIndex aPrevIdx( pStart->nNode, -1 );
            sal_uLong nOffset = pEnd->nNode.GetIndex() - pStart->nNode.GetIndex();
            xub_StrLen nCntStt  = pStart->nContent.GetIndex();

            // Sorting
            bRet = pDoc->SortText(*pPam, rOpt);

            // put selection again
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
                // now search the format, determining the columness
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
    OSL_ENSURE( GetCurrFrm(), "Crsr parked?" );
    return _GetCurColNum( GetCurrFrm(), pPara );
}

sal_uInt16 SwFEShell::GetCurOutColNum( SwGetCurColNumPara* pPara ) const
{
    sal_uInt16 nRet = 0;
    SwFrm* pFrm = GetCurrFrm();
    OSL_ENSURE( pFrm, "Crsr parked?" );
    if( pFrm )
    {
        pFrm = pFrm->IsInTab() ? (SwFrm*)pFrm->FindTabFrm()
                               : (SwFrm*)pFrm->FindSctFrm();
        OSL_ENSURE( pFrm, "No Tab, no Sect" );
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

// #i17567# - adjustments for allowing
//          negative vertical positions for fly frames anchored to paragraph/to character.
// #i22305# - adjustments for option 'Follow text flow'
//          for to frame anchored objects.
// #i22341# - adjustments for vertical alignment at top of line
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
    bool bVertic = false;
    sal_Bool bRTL = sal_False;
    bool bVerticalL2R = false;

    if ((FLY_AT_PAGE == _nAnchorId) || (FLY_AT_FLY == _nAnchorId)) // LAYER_IMPL
    {
        const SwFrm* pTmp = pFrm;
        // #i22305#
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

        if( bVert || bVertL2R )
        {
            bVertic = bVert ? true : false;
            bVerticalL2R = bVertL2R ? true : false;
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
                // no break
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
                // no break!
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
                // no break!
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA: aPos.X() += pFrm->Prt().Left(); break;
                default:break;
            }
        }

        if ( bVert && !bVertL2R )
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
        else if ( bVertL2R )
        {
            switch ( _eVertRelOrient )
            {
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA:
                {
                    aPos.X() += pFrm->GetLeftMargin();
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
        // #i17567# - allow negative positions
        // for fly frames anchor to paragraph/to character.
        if ((_nAnchorId == FLY_AT_PARA) || (_nAnchorId == FLY_AT_CHAR))
        {
            // The rectangle, the fly frame can be positioned in, is determined
            // horizontally by the frame area of the horizontal environment
            // and vertically by the printing area of the vertical environment,
            // if the object follows the text flow, or by the frame area of the
            // vertical environment, if the object doesn't follow the text flow.
            // new class <SwEnvironmentOfAnchoredObject>
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
                // #i18732# - adjust vertical 'virtual' anchor position
                // (<aPos.Y()> respectively <aPos.X()>), if object is vertical aligned
                // to page areas.
                if ( _eVertRelOrient == text::RelOrientation::PAGE_FRAME || _eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA )
                {
                    if ( bVert && !bVertL2R )
                    {
                        aPos.X() = aVertEnvironRect.Right();
                    }
                    else if ( bVertL2R )
                    {
                        aPos.X() = aVertEnvironRect.Left();
                    }
                    else
                    {
                        aPos.Y() = aVertEnvironRect.Top();
                    }
                }
            }
            else
            {
                OSL_ENSURE( rVertEnvironLayFrm.IsPageFrm(),
                        "<SwFEShell::CalcBoundRect(..)> - not following text flow, but vertical environment *not* page!" );
                aVertEnvironRect = rVertEnvironLayFrm.Frm();
                // #i18732# - adjustment vertical 'virtual' anchor position
                // (<aPos.Y()> respectively <aPos.X()>), if object is vertical aligned
                // to page areas.
                if ( _eVertRelOrient == text::RelOrientation::PAGE_FRAME || _eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA )
                {
                    if ( bVert && !bVertL2R )
                    {
                        aPos.X() = aVertEnvironRect.Right();
                        if ( _eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA )
                        {
                            aPos.X() -= rVertEnvironLayFrm.GetRightMargin();
                        }
                    }
                    else if ( bVertL2R )
                    {
                        aPos.X() = aVertEnvironRect.Left();
                        if ( _eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA )
                        {
                            aPos.X() += rVertEnvironLayFrm.GetLeftMargin();
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

            // #i22341# - adjust vertical 'virtual' anchor position
            // (<aPos.Y()> respectively <aPos.X()>), if object is anchored to
            // character and vertical aligned at character or top of line
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
                if ( bVert || bVertL2R )
                {
                    aPos.X() = nTop;
                }
                else
                {
                    aPos.Y() = nTop;
                }
            }

            // #i26945# - adjust horizontal 'virtual' anchor
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
                if ( bVert || bVertL2R )
                {
                    aPos.Y() = nLeft;
                }
                else
                {
                    aPos.X() = nLeft;
                }
            }
            if ( bVert || bVertL2R )
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
            // only use 90% of height for character bound
            {
                if( bVert || bVertL2R )
                    _orRect.Width( (_orRect.Width()*9)/10 );
                else
                    _orRect.Height( (_orRect.Height()*9)/10 );
            }
        }

        const SwTwips nBaseOfstForFly = ( pFrm->IsTxtFrm() && pFly ) ?
                                        ((SwTxtFrm*)pFrm)->GetBaseOfstForFly( !bWrapThrough ) :
                                         0;
        if( bVert || bVertL2R )
        {
            bVertic = bVert ? true : false;
            bVerticalL2R = bVertL2R ? true : false;
            _bMirror = false;

            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::FRAME_RIGHT:
                {
                    aPos.Y() += pFrm->Prt().Height();
                    aPos += (pFrm->Prt().*fnRect->fnGetPos)();
                    break;
                }
                case text::RelOrientation::PRINT_AREA:
                {
                    aPos += (pFrm->Prt().*fnRect->fnGetPos)();
                    aPos.Y() += nBaseOfstForFly;
                    break;
                }
                case text::RelOrientation::PAGE_RIGHT:
                {
                    aPos.Y() = pPage->Frm().Top() + pPage->Prt().Bottom();
                    break;
                }
                case text::RelOrientation::PAGE_PRINT_AREA:
                {
                    aPos.Y() = pPage->Frm().Top() + pPage->Prt().Top();
                    break;
                }
                case text::RelOrientation::PAGE_LEFT:
                case text::RelOrientation::PAGE_FRAME:
                {
                    aPos.Y() = pPage->Frm().Top();
                    break;
                }
                case text::RelOrientation::FRAME:
                {
                    aPos.Y() += nBaseOfstForFly;
                    break;
                }
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
                case text::RelOrientation::FRAME_RIGHT:
                    aPos.X() += pFrm->Prt().Width();
                    aPos += pFrm->Prt().Pos();
                    break;
                case text::RelOrientation::PRINT_AREA:
                    aPos += pFrm->Prt().Pos();
                    aPos.X() += nBaseOfstForFly;
                    break;
                case text::RelOrientation::PAGE_RIGHT:
                    aPos.X() = pPage->Frm().Left() + pPage->Prt().Right();
                    break;
                case text::RelOrientation::PAGE_PRINT_AREA:
                    aPos.X() = pPage->Frm().Left() + pPage->Prt().Left();
                    break;
                case text::RelOrientation::PAGE_LEFT:
                case text::RelOrientation::PAGE_FRAME:
                    aPos.X() = pPage->Frm().Left();
                    break;
                case text::RelOrientation::FRAME:
                    aPos.X() += nBaseOfstForFly;
                    break;
                default: break;
            }
        }

    }
    if( !_opRef )
    {
        if( bVertic && !bVerticalL2R )
            _orRect.Pos( aPos.X() - _orRect.Width() - _orRect.Left(), _orRect.Top() - aPos.Y() );
        else if( bVerticalL2R )
            _orRect.Pos( _orRect.Left() - aPos.X(), _orRect.Top() - aPos.Y() );
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
        // #i32951# - due to issue #i28701# no format of a
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

        SwRect aBound;
        CalcBoundRect( aBound, pFly->GetFmt()->GetAnchor().GetAnchorId());
        if ( pFly->GetAnchorFrm()->IsVertical() )
            aRet.Width() = aBound.Width();
        else
            aRet.Height() = aBound.Height();
    }
    return aRet;
}

sal_Bool SwFEShell::IsFrmVertical(const sal_Bool bEnvironment, sal_Bool& bRTL, sal_Bool& bVertL2R) const
{
    sal_Bool bVert = sal_False;
    bRTL = sal_False;
    bVertL2R = sal_False;

    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        if( rMrkList.GetMarkCount() != 1 )
            return bVert;

        SdrObject* pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
        if ( !pObj )
        {
            OSL_FAIL( "<SwFEShell::IsFrmVertical(..)> - missing SdrObject instance in marked object list -> This is a serious situation, please inform OD" );
            return bVert;
        }
        // #i26791#
        SwContact* pContact = static_cast<SwContact*>(GetUserCall( pObj ));
        if ( !pContact )
        {
            OSL_FAIL( "<SwFEShell::IsFrmVertical(..)> - missing SwContact instance at marked object -> This is a serious situation, please inform OD" );
            return bVert;
        }
        const SwFrm* pRef = pContact->GetAnchoredObj( pObj )->GetAnchorFrm();
        if ( !pRef )
        {
            OSL_FAIL( "<SwFEShell::IsFrmVertical(..)> - missing anchor frame at marked object -> This is a serious situation, please inform OD" );
            return bVert;
        }

        if ( pObj->ISA(SwVirtFlyDrawObj) && !bEnvironment )
            pRef = static_cast<const SwVirtFlyDrawObj*>(pObj)->GetFlyFrm();

        bVert = pRef->IsVertical();
        bRTL = pRef->IsRightToLeft();
        bVertL2R = pRef->IsVertLR();
    }

    return bVert;
}

void SwFEShell::MoveObjectIfActive( svt::EmbeddedObjectRef&, const Point& )
{
    // does not do anything, only avoids crash if the method is used for wrong shell
}

void SwFEShell::ToggleHeaderFooterEdit()
{
    // Clear objects selection
    if ( Imp()->GetDrawView()->AreObjectsMarked() )
    {
        Imp()->GetDrawView()->UnmarkAll();
        ClearMark();
    }

    SwCrsrShell::ToggleHeaderFooterEdit();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
