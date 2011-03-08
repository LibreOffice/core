/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/**************************************************************************
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


#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/view/XRenderable.hpp>

#include <hintids.hxx>
#include <rtl/ustring.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/prnmon.hxx>
#include <svl/languageoptions.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/pbinitem.hxx>
#include <svx/svdview.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <tools/debug.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/syslocale.hxx>
#include <vcl/oldprintadaptor.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <fmtfsize.hxx>
#include <frmatr.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <doc.hxx>
#include <wdocsh.hxx>
#include <fesh.hxx>
#include <pam.hxx>
#include <viewimp.hxx>      // Imp->SetFirstVisPageInvalid()
#include <layact.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <docfld.hxx>       // _SetGetExpFld
#include <docufld.hxx>      // PostItFld /-Type
#include <shellres.hxx>
#include <viewopt.hxx>
#include <swprtopt.hxx>     // SwPrtOptions
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include <mdiexp.hxx>
#include <statstr.hrc>
#include <ptqueue.hxx>
#include <tabfrm.hxx>
#include <txtfrm.hxx>           // MinPrtLine
#include <viscrs.hxx>          // SwShellCrsr
#include <fmtpdsc.hxx>      // SwFmtPageDesc
#include <globals.hrc>


using namespace ::com::sun::star;

class SwQueuedPaint
{
public:
    SwQueuedPaint *pNext;
    ViewShell      *pSh;
    SwRect          aRect;

    SwQueuedPaint( ViewShell *pNew, const SwRect &rRect ) :
        pNext( 0 ),
        pSh( pNew ),
        aRect( rRect )
    {}
};

SwQueuedPaint *SwPaintQueue::pQueue = 0;

// saves some settings from the draw view
class SwDrawViewSave
{
    String sLayerNm;
    SdrView* pDV;
    sal_Bool bPrintControls;
public:
    SwDrawViewSave( SdrView* pSdrView );
    ~SwDrawViewSave();
};


void SwPaintQueue::Add( ViewShell *pNew, const SwRect &rNew )
{
    SwQueuedPaint *pPt;
    if ( 0 != (pPt = pQueue) )
    {
        while ( pPt->pSh != pNew && pPt->pNext )
            pPt = pPt->pNext;
        if ( pPt->pSh == pNew )
        {
            pPt->aRect.Union( rNew );
            return;
        }
    }
    SwQueuedPaint *pNQ = new SwQueuedPaint( pNew, rNew );
    if ( pPt )
        pPt->pNext = pNQ;
    else
        pQueue = pNQ;
}



void SwPaintQueue::Repaint()
{
    if ( !SwRootFrm::IsInPaint() && pQueue )
    {
        SwQueuedPaint *pPt = pQueue;
        do
        {   ViewShell *pSh = pPt->pSh;
            SET_CURR_SHELL( pSh );
            if ( pSh->IsPreView() )
            {
                if ( pSh->GetWin() )
                {
                    //Fuer PreView aussenherum, weil im PaintHdl (UI) die
                    //Zeilen/Spalten bekannt sind.
                    pSh->GetWin()->Invalidate();
                    pSh->GetWin()->Update();
                }
            }
            else
                pSh->Paint( pPt->aRect.SVRect() );
            pPt = pPt->pNext;
        } while ( pPt );

        do
        {   pPt = pQueue;
            pQueue = pQueue->pNext;
            delete pPt;
        } while ( pQueue );
    }
}



void SwPaintQueue::Remove( ViewShell *pSh )
{
    SwQueuedPaint *pPt;
    if ( 0 != (pPt = pQueue) )
    {
        SwQueuedPaint *pPrev = 0;
        while ( pPt && pPt->pSh != pSh )
        {
            pPrev = pPt;
            pPt = pPt->pNext;
        }
        if ( pPt )
        {
            if ( pPrev )
                pPrev->pNext = pPt->pNext;
            else if ( pPt == pQueue )
                pQueue = 0;
            delete pPt;
        }
    }
}

void SetSwVisArea( ViewShell *pSh, const SwRect &rRect, BOOL /*bPDFExport*/ )
{
    OSL_ENSURE( !pSh->GetWin(), "Drucken mit Window?" );
    pSh->aVisArea = rRect;
    pSh->Imp()->SetFirstVisPageInvalid();
    Point aPt( rRect.Pos() );

    // calculate an offset for the rectangle of the n-th page to
    // move the start point of the output operation to a position
    // such that in the output device all pages will be painted
    // at the same position
    aPt.X() = -aPt.X(); aPt.Y() = -aPt.Y();

    OutputDevice *pOut = pSh->GetOut();

    MapMode aMapMode( pOut->GetMapMode() );
    aMapMode.SetOrigin( aPt );
    pOut->SetMapMode( aMapMode );
}

/******************************************************************************/

void ViewShell::InitPrt( OutputDevice *pOutDev )
{
    //Fuer den Printer merken wir uns einen negativen Offset, der
    //genau dem Offset de OutputSize entspricht. Das ist notwendig,
    //weil unser Ursprung der linken ober Ecke der physikalischen
    //Seite ist, die Ausgaben (SV) aber den Outputoffset als Urstprung
    //betrachten.
    if ( pOutDev )
    {
        aPrtOffst = Point();

        aPrtOffst += pOutDev->GetMapMode().GetOrigin();
        MapMode aMapMode( pOutDev->GetMapMode() );
        aMapMode.SetMapUnit( MAP_TWIP );
        pOutDev->SetMapMode( aMapMode );
        pOutDev->SetLineColor();
        pOutDev->SetFillColor();
    }
    else
        aPrtOffst.X() = aPrtOffst.Y() = 0;

    if ( !pWin )
        pOut = pOutDev;    //Oder was sonst?
}

void ViewShell::ChgAllPageOrientation( USHORT eOri )
{
    OSL_ENSURE( nStartAction, "missing an Action" );
    SET_CURR_SHELL( this );

    USHORT nAll = GetDoc()->GetPageDescCnt();
    BOOL bNewOri = Orientation(eOri) == ORIENTATION_PORTRAIT ? FALSE : TRUE;

    for( USHORT i = 0; i < nAll; ++ i )
    {
        const SwPageDesc& rOld =
            const_cast<const SwDoc *>(GetDoc())->GetPageDesc( i );

        if( rOld.GetLandscape() != bNewOri )
        {
            SwPageDesc aNew( rOld );
            const sal_Bool bDoesUndo( GetDoc()->DoesUndo() );
            GetDoc()->DoUndo( sal_False );
            GetDoc()->CopyPageDesc(rOld, aNew);
            GetDoc()->DoUndo( bDoesUndo );
            aNew.SetLandscape( bNewOri );
            SwFrmFmt& rFmt = aNew.GetMaster();
            SwFmtFrmSize aSz( rFmt.GetFrmSize() );
            if( bNewOri ? aSz.GetHeight() > aSz.GetWidth()
                        : aSz.GetHeight() < aSz.GetWidth() )
            {
                SwTwips aTmp = aSz.GetHeight();
                aSz.SetHeight( aSz.GetWidth() );
                aSz.SetWidth( aTmp );
                rFmt.SetFmtAttr( aSz );
            }
            GetDoc()->ChgPageDesc( i, aNew );
        }
    }
}

void ViewShell::ChgAllPageSize( Size &rSz )
{
    OSL_ENSURE( nStartAction, "missing an Action" );
    SET_CURR_SHELL( this );

    SwDoc* pMyDoc = GetDoc();
    USHORT nAll = pMyDoc->GetPageDescCnt();

    for( USHORT i = 0; i < nAll; ++i )
    {
        const SwPageDesc &rOld = const_cast<const SwDoc *>(pMyDoc)->GetPageDesc( i );
        SwPageDesc aNew( rOld );
        const sal_Bool bDoesUndo( GetDoc()->DoesUndo() );
        GetDoc()->DoUndo( sal_False );
        GetDoc()->CopyPageDesc( rOld, aNew );
        GetDoc()->DoUndo( bDoesUndo );
        SwFrmFmt& rPgFmt = aNew.GetMaster();
        Size aSz( rSz );
        const BOOL bOri = aNew.GetLandscape();
        if( bOri  ? aSz.Height() > aSz.Width()
                  : aSz.Height() < aSz.Width() )
        {
            SwTwips aTmp = aSz.Height();
            aSz.Height() = aSz.Width();
            aSz.Width()  = aTmp;
        }

        SwFmtFrmSize aFrmSz( rPgFmt.GetFrmSize() );
        aFrmSz.SetSize( aSz );
        rPgFmt.SetFmtAttr( aFrmSz );
        pMyDoc->ChgPageDesc( i, aNew );
    }
}


void ViewShell::CalcPagesForPrint( USHORT nMax )
{
    SET_CURR_SHELL( this );

    SwRootFrm* pLayout = GetLayout();

    const SwFrm *pPage = pLayout->Lower();
    SwLayAction aAction( pLayout, Imp() );

    pLayout->StartAllAction();
    for ( USHORT i = 1; pPage && i <= nMax; pPage = pPage->GetNext(), ++i )
    {
        pPage->Calc();
        SwRect aOldVis( VisArea() );
        aVisArea = pPage->Frm();
        Imp()->SetFirstVisPageInvalid();
        aAction.Reset();
        aAction.SetPaint( FALSE );
        aAction.SetWaitAllowed( FALSE );
        aAction.SetReschedule( TRUE );

        aAction.Action();

        aVisArea = aOldVis;
        Imp()->SetFirstVisPageInvalid();

    }
    pLayout->EndAllAction();
}

/******************************************************************************/

SwDoc * ViewShell::CreatePrtDoc( SfxObjectShellRef &rDocShellRef)
{
    OSL_ENSURE( this->IsA( TYPE(SwFEShell) ),"ViewShell::Prt for FEShell only");
    SwFEShell* pFESh = (SwFEShell*)this;

    SwDoc *pPrtDoc = new SwDoc;
    pPrtDoc->acquire();
    pPrtDoc->SetRefForDocShell( (SfxObjectShellRef*)&(long&)rDocShellRef );
    pPrtDoc->LockExpFlds();

    const SfxPoolItem* pCpyItem;
    const SfxItemPool& rPool = GetAttrPool();
    for( USHORT nWh = POOLATTR_BEGIN; nWh < POOLATTR_END; ++nWh )
        if( 0 != ( pCpyItem = rPool.GetPoolDefaultItem( nWh ) ) )
            pPrtDoc->GetAttrPool().SetPoolDefaultItem( *pCpyItem );

    pPrtDoc->ReplaceStyles( *GetDoc() );

    SwShellCrsr *pActCrsr = pFESh->_GetCrsr();
    SwShellCrsr *pFirstCrsr = dynamic_cast<SwShellCrsr*>(pActCrsr->GetNext());
    if( !pActCrsr->HasMark() )
    {
        pActCrsr = dynamic_cast<SwShellCrsr*>(pActCrsr->GetPrev());
    }

    Point aSelPoint;
    if( pFESh->IsTableMode() )
    {
        SwShellTableCrsr* pShellTblCrsr = pFESh->GetTableCrsr();

        const SwCntntNode* pCntntNode = pShellTblCrsr->GetNode()->GetCntntNode();
        const SwCntntFrm *pCntntFrm = pCntntNode ? pCntntNode->GetFrm( 0, pShellTblCrsr->Start() ) : 0;
        if( pCntntFrm )
        {
            SwRect aCharRect;
            SwCrsrMoveState aTmpState( MV_NONE );
            pCntntFrm->GetCharRect( aCharRect, *pShellTblCrsr->Start(), &aTmpState );
            aSelPoint = Point( aCharRect.Left(), aCharRect.Top() );
        }
    }
    else
    {
       aSelPoint = pFirstCrsr->GetSttPos();
    }

    const SwPageFrm* pPage = GetLayout()->GetPageAtPos( aSelPoint );
    OSL_ENSURE( pPage, "no page found!" );

    // get page descriptor - fall back to the first one if pPage could not be found
    const SwPageDesc* pPageDesc = pPage ? pPrtDoc->FindPageDescByName(
        pPage->GetPageDesc()->GetName() ) : &pPrtDoc->_GetPageDesc( (sal_uInt16)0 );

    if( !pFESh->IsTableMode() && pActCrsr->HasMark() )
    {
        SwNodeIndex aNodeIdx( *pPrtDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
        SwTxtNode* pTxtNd = pPrtDoc->GetNodes().GoNext( &aNodeIdx )->GetTxtNode();
        SwCntntNode *pLastNd =
            pActCrsr->GetCntntNode( (*pActCrsr->GetMark()) <= (*pActCrsr->GetPoint()) );

        if( pLastNd && pLastNd->IsTxtNode() )
            ((SwTxtNode*)pLastNd)->CopyCollFmt( *pTxtNd );
    }


    pFESh->Copy( pPrtDoc );

    {
        SwNodeIndex aNodeIdx( *pPrtDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
        SwCntntNode* pCNd = pPrtDoc->GetNodes().GoNext( &aNodeIdx );
        if( pFESh->IsTableMode() )
        {
            SwTableNode* pTNd = pCNd->FindTableNode();
            if( pTNd )
                pTNd->GetTable().GetFrmFmt()->SetFmtAttr( SwFmtPageDesc( pPageDesc ) );
        }
        else
        {
            pCNd->SetAttr( SwFmtPageDesc( pPageDesc ) );
            if( pFirstCrsr->HasMark() )
            {
                SwTxtNode *pTxtNd = pCNd->GetTxtNode();
                if( pTxtNd )
                {
                    SwCntntNode *pFirstNd =
                        pFirstCrsr->GetCntntNode( (*pFirstCrsr->GetMark()) > (*pFirstCrsr->GetPoint()) );
                    if( pFirstNd && pFirstNd->IsTxtNode() )
                        ((SwTxtNode*)pFirstNd)->CopyCollFmt( *pTxtNd );
                }
            }
        }
    }
    return pPrtDoc;
}

SwDoc * ViewShell::FillPrtDoc( SwDoc *pPrtDoc, const SfxPrinter* pPrt)
{
    OSL_ENSURE( this->IsA( TYPE(SwFEShell) ),"ViewShell::Prt for FEShell only");
    SwFEShell* pFESh = (SwFEShell*)this;
    pPrtDoc->LockExpFlds();

    // Make a copy of it since it gets destroyed with the temporary document
    // used for PDF export
    if (pPrt)
        pPrtDoc->setPrinter( new SfxPrinter(*pPrt), true, true );

    const SfxPoolItem* pCpyItem;
    const SfxItemPool& rPool = GetAttrPool();
    for( USHORT nWh = POOLATTR_BEGIN; nWh < POOLATTR_END; ++nWh )
        if( 0 != ( pCpyItem = rPool.GetPoolDefaultItem( nWh ) ) )
            pPrtDoc->GetAttrPool().SetPoolDefaultItem( *pCpyItem );

    pPrtDoc->ReplaceStyles( *GetDoc() );

    SwShellCrsr *pActCrsr = pFESh->_GetCrsr();
    SwShellCrsr *pFirstCrsr = dynamic_cast<SwShellCrsr*>(pActCrsr->GetNext());
    if( !pActCrsr->HasMark() )
    {
        pActCrsr = dynamic_cast<SwShellCrsr*>(pActCrsr->GetPrev());
    }

    Point aSelPoint;
    if( pFESh->IsTableMode() )
    {
        SwShellTableCrsr* pShellTblCrsr = pFESh->GetTableCrsr();

        const SwCntntNode* pCntntNode = pShellTblCrsr->GetNode()->GetCntntNode();
        const SwCntntFrm *pCntntFrm = pCntntNode ? pCntntNode->GetFrm( 0, pShellTblCrsr->Start() ) : 0;
        if( pCntntFrm )
        {
            SwRect aCharRect;
            SwCrsrMoveState aTmpState( MV_NONE );
            pCntntFrm->GetCharRect( aCharRect, *pShellTblCrsr->Start(), &aTmpState );
            aSelPoint = Point( aCharRect.Left(), aCharRect.Top() );
        }
    }
    else
    {
       aSelPoint = pFirstCrsr->GetSttPos();
    }

    const SwPageFrm* pPage = GetLayout()->GetPageAtPos( aSelPoint );
    OSL_ENSURE( pPage, "no page found!" );

    // get page descriptor - fall back to the first one if pPage could not be found
    const SwPageDesc* pPageDesc = pPage ? pPrtDoc->FindPageDescByName(
        pPage->GetPageDesc()->GetName() ) : &pPrtDoc->_GetPageDesc( (sal_uInt16)0 );

    if( !pFESh->IsTableMode() && pActCrsr->HasMark() )
    {
        SwNodeIndex aNodeIdx( *pPrtDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
        SwTxtNode* pTxtNd = pPrtDoc->GetNodes().GoNext( &aNodeIdx )->GetTxtNode();
        SwCntntNode *pLastNd =
            pActCrsr->GetCntntNode( (*pActCrsr->GetMark()) <= (*pActCrsr->GetPoint()) );

        if( pLastNd && pLastNd->IsTxtNode() )
            ((SwTxtNode*)pLastNd)->CopyCollFmt( *pTxtNd );
    }

    pFESh->Copy( pPrtDoc );

    {
        SwNodeIndex aNodeIdx( *pPrtDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
        SwCntntNode* pCNd = pPrtDoc->GetNodes().GoNext( &aNodeIdx );
        if( pFESh->IsTableMode() )
        {
            SwTableNode* pTNd = pCNd->FindTableNode();
            if( pTNd )
                pTNd->GetTable().GetFrmFmt()->SetFmtAttr( SwFmtPageDesc( pPageDesc ) );
        }
        else
        {
            pCNd->SetAttr( SwFmtPageDesc( pPageDesc ) );
            if( pFirstCrsr->HasMark() )
            {
                SwTxtNode *pTxtNd = pCNd->GetTxtNode();
                if( pTxtNd )
                {
                    SwCntntNode *pFirstNd =
                        pFirstCrsr->GetCntntNode( (*pFirstCrsr->GetMark()) > (*pFirstCrsr->GetPoint()) );

                    if( pFirstNd && pFirstNd->IsTxtNode() )
                        ((SwTxtNode*)pFirstNd)->CopyCollFmt( *pTxtNd );
                }
            }
        }
    }
    return pPrtDoc;
}


sal_Bool ViewShell::PrintOrPDFExport(
    OutputDevice *pOutDev,
    const SwPrtOptions &rPrintData,
    sal_Int32 nRenderer     /* the index in the vector of pages to be printed */ )
{

    const sal_Int32 nMaxRenderer = rPrintData.GetRenderData().GetPagesToPrint().size() - 1;
#if OSL_DEBUG_LEVEL > 1
    DBG_ASSERT( 0 <= nRenderer && nRenderer <= nMaxRenderer, "nRenderer out of bounds");
#endif
    if (!pOutDev || nMaxRenderer < 0 || nRenderer < 0 || nRenderer > nMaxRenderer)
        return sal_False;

    // save settings of OutputDevice (should be done always since the
    // output device is now provided by a call from outside the Writer)
    pOutDev->Push();

    ViewShell *pShell;
    SwDoc *pOutDevDoc;

    SfxObjectShellRef aDocShellRef;

    //! Note: Since for PDF export of (multi-)selection a temporary
    //! document is created that contains only the selects parts,
    //! and thus that document is to printed in whole the,
    //! rPrintData.bPrintSelection parameter will be false.
    BOOL bSelection = rPrintData.bPrintSelection;

    // PDF export for (multi-)selection has already generated a temporary document
    // with the selected text. (see XRenderable implementation in unotxdoc.cxx)
    // Thus we like to go in the 'else' part here in that case.
    // Is is implemented this way because PDF export calls this Prt function
    // once per page and we do not like to always have the temporary document
    // to be created that often here in the 'then' part.
    if ( bSelection )
    {
        pOutDevDoc = CreatePrtDoc( aDocShellRef );

        pShell = new ViewShell( *pOutDevDoc, 0, pOpt, pOutDev );
        pOutDevDoc->SetRefForDocShell( 0 );
    }
    else
    {
        pOutDevDoc = GetDoc();
        pShell = new ViewShell( *this, 0, pOutDev );
    }

    SdrView *pDrawView = pShell->GetDrawView();
    if (pDrawView)
    {
        pDrawView->SetBufferedOutputAllowed( false );
        pDrawView->SetBufferedOverlayAllowed( false );
    }

    {
        SET_CURR_SHELL( pShell );


        if( pOpt->IsReadonly() )
            pShell->pOpt->SetReadonly( TRUE );

        // save options at draw view:
        SwDrawViewSave aDrawViewSave( pShell->GetDrawView() );

        pShell->PrepareForPrint( rPrintData );

        const sal_Int32 nPage = rPrintData.GetRenderData().GetPagesToPrint()[ nRenderer ];
#if OSL_DEBUG_LEVEL > 1
        DBG_ASSERT( nPage == 0 || rPrintData.GetRenderData().GetValidPagesSet().count( nPage ) == 1, "nPage not valid" );
#endif
        const SwPageFrm *pStPage = 0;
        if (nPage > 0)  // a 'regular' page, not one from the post-it document
        {
            const SwRenderData::ValidStartFramesMap_t &rFrms = rPrintData.GetRenderData().GetValidStartFrames();
            SwRenderData::ValidStartFramesMap_t::const_iterator aIt( rFrms.find( nPage ) );
            DBG_ASSERT( aIt != rFrms.end(), "failed to find start frame" );
            if (aIt == rFrms.end())
                return sal_False;
            pStPage = aIt->second;
        }
        else    // a page from the post-its document ...
        {
            DBG_ASSERT( nPage == 0, "unexpected page number. 0 for post-it pages expected" );
            pStPage = rPrintData.GetRenderData().GetPostItStartFrames()[ nRenderer ];
        }
        DBG_ASSERT( pStPage, "failed to get start page" );

        //!! applying view options and formatting the dcoument should now only be done in getRendererCount!

        ViewShell *pViewSh2 = nPage == 0 ? /* post-it page? */
                rPrintData.GetRenderData().m_pPostItShell : pShell;
        ::SetSwVisArea( pViewSh2, pStPage->Frm() );


        if( pStPage->GetFmt()->GetPoolFmtId() == RES_POOLPAGE_JAKET )
        {
            Point aNewOrigin = pOutDev->GetMapMode().GetOrigin();
            aNewOrigin += rPrintData.aOffset;
            MapMode aTmp( pOutDev->GetMapMode() );
            aTmp.SetOrigin( aNewOrigin );
            pOutDev->SetMapMode( aTmp );
        }

        pShell->InitPrt( pOutDev );

        pViewSh2 = nPage == 0 ? /* post-it page? */
                rPrintData.GetRenderData().m_pPostItShell : pShell;
        ::SetSwVisArea( pViewSh2, pStPage->Frm() );

        pStPage->GetUpper()->Paint( pStPage->Frm(), &rPrintData );

        SwPaintQueue::Repaint();
    }

    delete pShell;

    if (bSelection )
    {
        if ( !pOutDevDoc->release() )
            delete pOutDevDoc;
    }

    // restore settings of OutputDevice (should be done always now since the
    // output device is now provided by a call from outside the Writer)
    pOutDev->Pop();

    return sal_True;
}

void ViewShell::PrtOle2( SwDoc *pDoc, const SwViewOption *pOpt, const SwPrintData& rOptions,
                         OutputDevice* pOleOut, const Rectangle& rRect )
{
    ViewShell *pSh;
    if( pDoc->GetRootFrm() && pDoc->GetRootFrm()->GetCurrShell() )
        pSh = new ViewShell( *pDoc->GetRootFrm()->GetCurrShell(), 0, pOleOut );
    else
        pSh = new ViewShell( *pDoc, 0, pOpt, pOleOut );

    {
        SET_CURR_SHELL( pSh );
        pSh->PrepareForPrint( rOptions );
        pSh->SetPrtFormatOption( TRUE );

        SwRect aSwRect( rRect );
        pSh->aVisArea = aSwRect;

        if ( pSh->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) &&
             pSh->GetNext() == pSh )
        {
            pSh->CheckBrowseView( FALSE );
            pDoc->GetRootFrm()->Lower()->InvalidateSize();
        }

        // CalcPagesForPrint() should not be necessary here. The pages in the
        // visible area will be formatted in SwRootFrm::Paint().
        // Removing this gives us a performance gain during saving the
        // document because the thumbnail creation will not trigger a complete
        // formatting of the document.
        // Seiten fuers Drucken formatieren
        // pSh->CalcPagesForPrint( SHRT_MAX );
        pOleOut->Push( PUSH_CLIPREGION );
        pOleOut->IntersectClipRegion( aSwRect.SVRect() );
        pSh->GetLayout()->Paint( aSwRect );

        pOleOut->Pop();

    }
    delete pSh;
}

BOOL ViewShell::IsAnyFieldInDoc() const
{
    const SfxPoolItem* pItem;
    USHORT nMaxItems = pDoc->GetAttrPool().GetItemCount( RES_TXTATR_FIELD );
    for( USHORT n = 0; n < nMaxItems; ++n )
        if( 0 != (pItem = pDoc->GetAttrPool().GetItem( RES_TXTATR_FIELD, n )))
        {
            const SwFmtFld* pFmtFld = (SwFmtFld*)pItem;
            const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
            //#i101026# do not include postits in field check
            const SwField* pFld = pFmtFld->GetFld();
            if( pTxtFld && pTxtFld->GetTxtNode().GetNodes().IsDocNodes() && (pFld->Which() != RES_POSTITFLD))
                return TRUE;
        }
    return FALSE;
}

SwDrawViewSave::SwDrawViewSave( SdrView* pSdrView )
    : pDV( pSdrView )
{
    if ( pDV )
    {
        sLayerNm.AssignAscii( RTL_CONSTASCII_STRINGPARAM("Controls" ) );
        bPrintControls = pDV->IsLayerPrintable( sLayerNm );
    }
}

SwDrawViewSave::~SwDrawViewSave()
{
    if ( pDV )
    {
        pDV->SetLayerPrintable( sLayerNm, bPrintControls );
    }
}


// #i6467# - method also called for page preview
void ViewShell::PrepareForPrint( const SwPrintData &rOptions )
{
    pOpt->SetGraphic ( TRUE == rOptions.bPrintGraphic );
    pOpt->SetTable   ( TRUE == rOptions.bPrintTable );
    pOpt->SetDraw    ( TRUE == rOptions.bPrintDraw  );
    pOpt->SetControl ( TRUE == rOptions.bPrintControl );
    pOpt->SetPageBack( TRUE == rOptions.bPrintPageBackground );
    pOpt->SetBlackFont( TRUE == rOptions.bPrintBlackFont );

    if ( HasDrawView() )
    {
        SdrView *pDrawView = GetDrawView();
        String sLayerNm;
        sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("Controls" ));
        // #i6467# - consider, if view shell belongs to page preview
        if ( !IsPreView() )
        {
            pDrawView->SetLayerPrintable( sLayerNm, rOptions.bPrintControl );
        }
        else
        {
            pDrawView->SetLayerVisible( sLayerNm, rOptions.bPrintControl );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
