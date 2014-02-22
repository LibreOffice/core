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
#include <IDocumentUndoRedo.hxx>
#include <wdocsh.hxx>
#include <fesh.hxx>
#include <pam.hxx>
#include <viewimp.hxx>
#include <layact.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <docfld.hxx>
#include <docufld.hxx>
#include <shellres.hxx>
#include <viewopt.hxx>
#include <printdata.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include <mdiexp.hxx>
#include <statstr.hrc>
#include <ptqueue.hxx>
#include <tabfrm.hxx>
#include <txtfrm.hxx>
#include <viscrs.hxx>
#include <fmtpdsc.hxx>
#include <globals.hrc>

using namespace ::com::sun::star;


class SwQueuedPaint
{
public:
    SwQueuedPaint *pNext;
    SwViewShell      *pSh;
    SwRect          aRect;

    SwQueuedPaint( SwViewShell *pNew, const SwRect &rRect ) :
        pNext( 0 ),
        pSh( pNew ),
        aRect( rRect )
    {}
};

SwQueuedPaint *SwPaintQueue::pQueue = 0;


class SwDrawViewSave
{
    OUString sLayerNm;
    SdrView* pDV;
    sal_Bool bPrintControls;
public:
    SwDrawViewSave( SdrView* pSdrView );
    ~SwDrawViewSave();
};

void SwPaintQueue::Add( SwViewShell *pNew, const SwRect &rNew )
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
        {   SwViewShell *pSh = pPt->pSh;
            SET_CURR_SHELL( pSh );
            if ( pSh->IsPreview() )
            {
                if ( pSh->GetWin() )
                {
                    
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

void SwPaintQueue::Remove( SwViewShell *pSh )
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

void SetSwVisArea( SwViewShell *pSh, const SwRect &rRect )
{
    OSL_ENSURE( !pSh->GetWin(), "Drucken mit Window?" );
    pSh->maVisArea = rRect;
    pSh->Imp()->SetFirstVisPageInvalid();
    Point aPt( rRect.Pos() );

    
    
    
    
    aPt.X() = -aPt.X(); aPt.Y() = -aPt.Y();

    OutputDevice *pOut = pSh->GetOut();

    MapMode aMapMode( pOut->GetMapMode() );
    aMapMode.SetOrigin( aPt );
    pOut->SetMapMode( aMapMode );
}

void SwViewShell::InitPrt( OutputDevice *pOutDev )
{
    
    
    
    if ( pOutDev )
    {
        maPrtOffst = Point();

        maPrtOffst += pOutDev->GetMapMode().GetOrigin();
        MapMode aMapMode( pOutDev->GetMapMode() );
        aMapMode.SetMapUnit( MAP_TWIP );
        pOutDev->SetMapMode( aMapMode );
        pOutDev->SetLineColor();
        pOutDev->SetFillColor();
    }
    else
        maPrtOffst.X() = maPrtOffst.Y() = 0;

    if ( !mpWin )
        mpOut = pOutDev;
}

void SwViewShell::ChgAllPageOrientation( sal_uInt16 eOri )
{
    OSL_ENSURE( mnStartAction, "missing an Action" );
    SET_CURR_SHELL( this );

    sal_uInt16 nAll = GetDoc()->GetPageDescCnt();
    sal_Bool bNewOri = Orientation(eOri) == ORIENTATION_PORTRAIT ? sal_False : sal_True;

    for( sal_uInt16 i = 0; i < nAll; ++ i )
    {
        const SwPageDesc& rOld = GetDoc()->GetPageDesc( i );

        if( rOld.GetLandscape() != bNewOri )
        {
            SwPageDesc aNew( rOld );
            {
                ::sw::UndoGuard const ug(GetDoc()->GetIDocumentUndoRedo());
                GetDoc()->CopyPageDesc(rOld, aNew);
            }
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

void SwViewShell::ChgAllPageSize( Size &rSz )
{
    OSL_ENSURE( mnStartAction, "missing an Action" );
    SET_CURR_SHELL( this );

    SwDoc* pMyDoc = GetDoc();
    sal_uInt16 nAll = pMyDoc->GetPageDescCnt();

    for( sal_uInt16 i = 0; i < nAll; ++i )
    {
        const SwPageDesc &rOld = pMyDoc->GetPageDesc( i );
        SwPageDesc aNew( rOld );
        {
            ::sw::UndoGuard const ug(GetDoc()->GetIDocumentUndoRedo());
            GetDoc()->CopyPageDesc( rOld, aNew );
        }
        SwFrmFmt& rPgFmt = aNew.GetMaster();
        Size aSz( rSz );
        const sal_Bool bOri = aNew.GetLandscape();
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

void SwViewShell::CalcPagesForPrint( sal_uInt16 nMax )
{
    SET_CURR_SHELL( this );

    SwRootFrm* pMyLayout = GetLayout();

    const SwFrm *pPage = pMyLayout->Lower();
    SwLayAction aAction( pMyLayout, Imp() );

    pMyLayout->StartAllAction();
    for ( sal_uInt16 i = 1; pPage && i <= nMax; pPage = pPage->GetNext(), ++i )
    {
        pPage->Calc();
        SwRect aOldVis( VisArea() );
        maVisArea = pPage->Frm();
        Imp()->SetFirstVisPageInvalid();
        aAction.Reset();
        aAction.SetPaint( sal_False );
        aAction.SetWaitAllowed( sal_False );
        aAction.SetReschedule( sal_True );

        aAction.Action();

        maVisArea = aOldVis; 
        Imp()->SetFirstVisPageInvalid();
    }

    pMyLayout->EndAllAction();
}

SwDoc * SwViewShell::FillPrtDoc( SwDoc *pPrtDoc, const SfxPrinter* pPrt)
{
    OSL_ENSURE( this->IsA( TYPE(SwFEShell) ),"SwViewShell::Prt for FEShell only");
    SwFEShell* pFESh = (SwFEShell*)this;
    pPrtDoc->LockExpFlds();

    
    
    
    if (pPrt)
        pPrtDoc->setPrinter( new SfxPrinter(*pPrt), true, true );

    const SfxPoolItem* pCpyItem;
    const SfxItemPool& rPool = GetAttrPool();
    for( sal_uInt16 nWh = POOLATTR_BEGIN; nWh < POOLATTR_END; ++nWh )
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
        const SwCntntFrm *pCntntFrm = pCntntNode ? pCntntNode->getLayoutFrm( GetLayout(), 0, pShellTblCrsr->Start() ) : 0;
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

    
    const SwPageDesc* pPageDesc = pPage ? pPrtDoc->FindPageDescByName(
        pPage->GetPageDesc()->GetName() ) : &pPrtDoc->GetPageDesc( (sal_uInt16)0 );

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



SwPageFrm const*
sw_getPage(SwRootFrm const& rLayout, sal_Int32 const nPage)
{
    
    SwPageFrm const* pPage = dynamic_cast<const SwPageFrm*>(rLayout.Lower());
    for (sal_Int32 i = nPage; pPage && (i > 0); --i)
    {
        if (1 == i) { 
            return pPage;
        }
        pPage = dynamic_cast<SwPageFrm const*>(pPage->GetNext());
    }
    OSL_ENSURE(pPage, "ERROR: SwPageFrm expected");
    OSL_FAIL("non-existent page requested");
    return 0;
}

sal_Bool SwViewShell::PrintOrPDFExport(
    OutputDevice *pOutDev,
    SwPrintData const& rPrintData,
    sal_Int32 nRenderer     /* the index in the vector of pages to be printed */ )
{
    

    const sal_Int32 nMaxRenderer = rPrintData.GetRenderData().GetPagesToPrint().size() - 1;
    OSL_ENSURE( 0 <= nRenderer && nRenderer <= nMaxRenderer, "nRenderer out of bounds");
    if (!pOutDev || nMaxRenderer < 0 || nRenderer < 0 || nRenderer > nMaxRenderer)
        return sal_False;

    
    
    pOutDev->Push();

    
    
    
    
    
    
    SwViewShell *pShell = new SwViewShell( *this, 0, pOutDev );

    SdrView *pDrawView = pShell->GetDrawView();
    if (pDrawView)
    {
        pDrawView->SetBufferedOutputAllowed( false );
        pDrawView->SetBufferedOverlayAllowed( false );
    }

    {   

        SET_CURR_SHELL( pShell );

        
        if( mpOpt->IsReadonly() )
            pShell->mpOpt->SetReadonly( sal_True );

        
        SwDrawViewSave aDrawViewSave( pShell->GetDrawView() );

        pShell->PrepareForPrint( rPrintData );

        const sal_Int32 nPage = rPrintData.GetRenderData().GetPagesToPrint()[ nRenderer ];
        OSL_ENSURE( nPage < 0 ||
            rPrintData.GetRenderData().GetValidPagesSet().count( nPage ) == 1,
            "SwViewShell::PrintOrPDFExport: nPage not valid" );
        SwViewShell *const pViewSh2 = (nPage < 0)
                ? rPrintData.GetRenderData().m_pPostItShell.get()
                : pShell; 

        SwPageFrm const*const pStPage =
            sw_getPage(*pViewSh2->GetLayout(), abs(nPage));
        OSL_ENSURE( pStPage, "failed to get start page" );
        if (!pStPage)
        {
            return sal_False;
        }

        

        ::SetSwVisArea( pViewSh2, pStPage->Frm() );

        pShell->InitPrt( pOutDev );

        ::SetSwVisArea( pViewSh2, pStPage->Frm() );

        pStPage->GetUpper()->Paint( pStPage->Frm(), &rPrintData );

        SwPaintQueue::Repaint();
    }

    delete pShell;

    
    
    pOutDev->Pop();

    return sal_True;
}

void SwViewShell::PrtOle2( SwDoc *pDoc, const SwViewOption *pOpt, const SwPrintData& rOptions,
                         OutputDevice* pOleOut, const Rectangle& rRect )
{
    
    
    SwViewShell *pSh;
    if( pDoc->GetCurrentViewShell() )
        pSh = new SwViewShell( *pDoc->GetCurrentViewShell(), 0, pOleOut,VSHELLFLAG_SHARELAYOUT );
    else
        pSh = new SwViewShell( *pDoc, 0, pOpt, pOleOut);

    {
        SET_CURR_SHELL( pSh );
        pSh->PrepareForPrint( rOptions );
        pSh->SetPrtFormatOption( sal_True );

        SwRect aSwRect( rRect );
        pSh->maVisArea = aSwRect;

        if ( pSh->GetViewOptions()->getBrowseMode() &&
             pSh->GetNext() == pSh )
        {
            pSh->CheckBrowseView( sal_False );
            pSh->GetLayout()->Lower()->InvalidateSize();
        }

        
        
        
        
        

        pOleOut->Push( PUSH_CLIPREGION );
        pOleOut->IntersectClipRegion( aSwRect.SVRect() );
        pSh->GetLayout()->Paint( aSwRect );

        pOleOut->Pop();
        
    }
    delete pSh;
}


sal_Bool SwViewShell::IsAnyFieldInDoc() const
{
    const SfxPoolItem* pItem;
    sal_uInt32 nMaxItems = mpDoc->GetAttrPool().GetItemCount2( RES_TXTATR_FIELD );
    for( sal_uInt32 n = 0; n < nMaxItems; ++n )
    {
        if( 0 != (pItem = mpDoc->GetAttrPool().GetItem2( RES_TXTATR_FIELD, n )))
        {
            const SwFmtFld* pFmtFld = (SwFmtFld*)pItem;
            const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
            if( pTxtFld && pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            {
                return sal_True;
            }
        }
    }

    nMaxItems = mpDoc->GetAttrPool().GetItemCount2( RES_TXTATR_INPUTFIELD );
    for( sal_uInt32 n = 0; n < nMaxItems; ++n )
    {
        if( 0 != (pItem = mpDoc->GetAttrPool().GetItem2( RES_TXTATR_INPUTFIELD, n )))
        {
            const SwFmtFld* pFmtFld = (SwFmtFld*)pItem;
            const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
            if( pTxtFld && pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            {
                return sal_True;
            }
        }
    }

    return sal_False;
}


SwDrawViewSave::SwDrawViewSave( SdrView* pSdrView )
    : pDV( pSdrView )
{
    if ( pDV )
    {
        sLayerNm = "Controls";
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


void SwViewShell::PrepareForPrint( const SwPrintData &rOptions )
{
    mpOpt->SetGraphic ( sal_True == rOptions.bPrintGraphic );
    mpOpt->SetTable   ( sal_True == rOptions.bPrintTable );
    mpOpt->SetDraw    ( sal_True == rOptions.bPrintDraw  );
    mpOpt->SetControl ( sal_True == rOptions.bPrintControl );
    mpOpt->SetPageBack( sal_True == rOptions.bPrintPageBackground );
    mpOpt->SetBlackFont( sal_True == rOptions.bPrintBlackFont );

    if ( HasDrawView() )
    {
        SdrView *pDrawView = GetDrawView();
        OUString sLayerNm;
        sLayerNm = "Controls";
        
        if ( !IsPreview() )
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
