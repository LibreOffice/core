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
#include <viewimp.hxx>      // Imp->SetFirstVisPageInvalid()
#include <layact.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <docfld.hxx>       // _SetGetExpFld
#include <docufld.hxx>      // PostItFld /-Type
#include <shellres.hxx>
#include <viewopt.hxx>
#include <printdata.hxx>    // SwPrintData
#include <pagedesc.hxx>
#include <poolfmt.hxx>      // fuer RES_POOLPAGE_JAKET
#include <mdiexp.hxx>       // Ansteuern der Statusleiste
#include <statstr.hrc>      //      -- " --
#include <ptqueue.hxx>
#include <tabfrm.hxx>
#include <txtfrm.hxx>       // MinPrtLine
#include <viscrs.hxx>       // SwShellCrsr
#include <fmtpdsc.hxx>      // SwFmtPageDesc
#include <globals.hrc>


using namespace ::com::sun::star;

//--------------------------------------------------------------------
//Klasse zum Puffern von Paints
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

/******************************************************************************
 *  Methode     :   void SetSwVisArea( ViewShell *pSh, Point aPrtOffset, ...
 *  Beschreibung:
 *  Erstellt    :   OK 04.11.94 16:27
 *  Aenderung   :
 ******************************************************************************/

void SetSwVisArea( ViewShell *pSh, const SwRect &rRect, sal_Bool /*bPDFExport*/ )
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

/******************************************************************************
 *  Methode     :   void ViewShell::ChgAllPageOrientation
 *  Erstellt    :   MA 08. Aug. 95
 *  Aenderung   :
 ******************************************************************************/


void ViewShell::ChgAllPageOrientation( sal_uInt16 eOri )
{
    OSL_ENSURE( nStartAction, "missing an Action" );
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
            // Groesse anpassen.
            // PORTRAIT  -> Hoeher als Breit
            // LANDSCAPE -> Breiter als Hoch
            // Hoehe ist die VarSize, Breite ist die FixSize (per Def.)
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

/******************************************************************************
 *  Methode     :   void ViewShell::ChgAllPageOrientation
 *  Erstellt    :   MA 08. Aug. 95
 *  Aenderung   :
 ******************************************************************************/


void ViewShell::ChgAllPageSize( Size &rSz )
{
    OSL_ENSURE( nStartAction, "missing an Action" );
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


void ViewShell::CalcPagesForPrint( sal_uInt16 nMax )
{
    SET_CURR_SHELL( this );

    SwRootFrm* pMyLayout = GetLayout();
    // ULONG nStatMax = pLayout->GetPageNum();

    const SwFrm *pPage = pMyLayout->Lower();
    SwLayAction aAction( pMyLayout, Imp() );

    pMyLayout->StartAllAction();
    for ( sal_uInt16 i = 1; pPage && i <= nMax; pPage = pPage->GetNext(), ++i )
    {
        pPage->Calc();
        SwRect aOldVis( VisArea() );
        aVisArea = pPage->Frm();
        Imp()->SetFirstVisPageInvalid();
        aAction.Reset();
        aAction.SetPaint( sal_False );
        aAction.SetWaitAllowed( sal_False );
        aAction.SetReschedule( sal_True );

        aAction.Action();

        aVisArea = aOldVis;             //Zuruecksetzen wg. der Paints!
        Imp()->SetFirstVisPageInvalid();
//       SwPaintQueue::Repaint();
    }

    pMyLayout->EndAllAction();
}

/******************************************************************************/

SwDoc * ViewShell::FillPrtDoc( SwDoc *pPrtDoc, const SfxPrinter* pPrt)
{
    OSL_ENSURE( this->IsA( TYPE(SwFEShell) ),"ViewShell::Prt for FEShell only");
    SwFEShell* pFESh = (SwFEShell*)this;
    // Wir bauen uns ein neues Dokument
//    SwDoc *pPrtDoc = new SwDoc;
//    pPrtDoc->acquire();
//    pPrtDoc->SetRefForDocShell( (SvEmbeddedObjectRef*)&(long&)rDocShellRef );
    pPrtDoc->LockExpFlds();

    // Der Drucker wird uebernommen
    //! Make a copy of it since it gets destroyed with the temporary document
    //! used for PDF export
    if (pPrt)
        pPrtDoc->setPrinter( new SfxPrinter(*pPrt), true, true );

    const SfxPoolItem* pCpyItem;
    const SfxItemPool& rPool = GetAttrPool();
    for( sal_uInt16 nWh = POOLATTR_BEGIN; nWh < POOLATTR_END; ++nWh )
        if( 0 != ( pCpyItem = rPool.GetPoolDefaultItem( nWh ) ) )
            pPrtDoc->GetAttrPool().SetPoolDefaultItem( *pCpyItem );

    // JP 29.07.99 - Bug 67951 - set all Styles from the SourceDoc into
    //                              the PrintDoc - will be replaced!
    pPrtDoc->ReplaceStyles( *GetDoc() );

    SwShellCrsr *pActCrsr = pFESh->_GetCrsr();
    SwShellCrsr *pFirstCrsr = dynamic_cast<SwShellCrsr*>(pActCrsr->GetNext());
    if( !pActCrsr->HasMark() ) // bei Multiselektion kann der aktuelle Cursor leer sein
    {
        pActCrsr = dynamic_cast<SwShellCrsr*>(pActCrsr->GetPrev());
    }

    // Die Y-Position der ersten Selektion
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

    // get page descriptor - fall back to the first one if pPage could not be found
    const SwPageDesc* pPageDesc = pPage ? pPrtDoc->FindPageDescByName(
        pPage->GetPageDesc()->GetName() ) : &pPrtDoc->GetPageDesc( (sal_uInt16)0 );

    if( !pFESh->IsTableMode() && pActCrsr->HasMark() )
    {   // Am letzten Absatz die Absatzattribute richten:
        SwNodeIndex aNodeIdx( *pPrtDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
        SwTxtNode* pTxtNd = pPrtDoc->GetNodes().GoNext( &aNodeIdx )->GetTxtNode();
        SwCntntNode *pLastNd =
            pActCrsr->GetCntntNode( (*pActCrsr->GetMark()) <= (*pActCrsr->GetPoint()) );
        // Hier werden die Absatzattribute des ersten Absatzes uebertragen
        if( pLastNd && pLastNd->IsTxtNode() )
            ((SwTxtNode*)pLastNd)->CopyCollFmt( *pTxtNd );
    }

    // es wurde in der CORE eine neu angelegt (OLE-Objekte kopiert!)
//      if( aDocShellRef.Is() )
//          SwDataExchange::InitOle( aDocShellRef, pPrtDoc );
    // und fuellen es mit dem selektierten Bereich
    pFESh->Copy( pPrtDoc );

    //Jetzt noch am ersten Absatz die Seitenvorlage setzen
    {
        SwNodeIndex aNodeIdx( *pPrtDoc->GetNodes().GetEndOfContent().StartOfSectionNode() );
        SwCntntNode* pCNd = pPrtDoc->GetNodes().GoNext( &aNodeIdx ); // gehe zum 1. ContentNode
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
                    // Hier werden die Absatzattribute des ersten Absatzes uebertragen
                    if( pFirstNd && pFirstNd->IsTxtNode() )
                        ((SwTxtNode*)pFirstNd)->CopyCollFmt( *pTxtNd );
                }
            }
        }
    }
    return pPrtDoc;
}

// TODO: there is already a GetPageByPageNum, but it checks some physical page
// number; unsure if we want that here, should find out what that is...
SwPageFrm const*
sw_getPage(SwRootFrm const& rLayout, sal_Int32 const nPage)
{
    // yes this is O(n^2) but at least it does not crash...
    SwPageFrm const* pPage = dynamic_cast<const SwPageFrm*>(rLayout.Lower());
    for (sal_Int32 i = nPage; pPage && (i > 0); --i)
    {
        if (1 == i) { // note: nPage is 1-based, i.e. 0 is invalid!
            return pPage;
        }
        pPage = dynamic_cast<SwPageFrm const*>(pPage->GetNext());
    }
    OSL_ENSURE(pPage, "ERROR: SwPageFrm expected");
    OSL_FAIL("non-existent page requested");
    return 0;
}

sal_Bool ViewShell::PrintOrPDFExport(
    OutputDevice *pOutDev,
    SwPrintData const& rPrintData,
    sal_Int32 nRenderer     /* the index in the vector of pages to be printed */ )
{
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//Immer die Druckroutinen in viewpg.cxx (PrintProspect) mitpflegen!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    const sal_Int32 nMaxRenderer = rPrintData.GetRenderData().GetPagesToPrint().size() - 1;
    OSL_ENSURE( 0 <= nRenderer && nRenderer <= nMaxRenderer, "nRenderer out of bounds");
    if (!pOutDev || nMaxRenderer < 0 || nRenderer < 0 || nRenderer > nMaxRenderer)
        return sal_False;

    // save settings of OutputDevice (should be done always since the
    // output device is now provided by a call from outside the Writer)
    pOutDev->Push();

    // Print/PDF export for (multi-)selection has already generated a
    // temporary document with the selected text.
    // (see XRenderable implementation in unotxdoc.cxx)
    // It is implemented this way because PDF export calls this Prt function
    // once per page and we do not like to always have the temporary document
    // to be created that often here.
    ViewShell *pShell = new ViewShell( *this, 0, pOutDev );

    SdrView *pDrawView = pShell->GetDrawView();
    if (pDrawView)
    {
        pDrawView->SetBufferedOutputAllowed( false );
        pDrawView->SetBufferedOverlayAllowed( false );
    }

    {   //Zusaetzlicher Scope, damit die CurrShell vor dem zerstoeren der
        //Shell zurueckgesetzt wird.

        SET_CURR_SHELL( pShell );

        //JP 01.02.99: das ReadOnly Flag wird NIE mitkopiert; Bug 61335
        if( pOpt->IsReadonly() )
            pShell->pOpt->SetReadonly( sal_True );

        // save options at draw view:
        SwDrawViewSave aDrawViewSave( pShell->GetDrawView() );

        pShell->PrepareForPrint( rPrintData );

        const sal_Int32 nPage = rPrintData.GetRenderData().GetPagesToPrint()[ nRenderer ];
        OSL_ENSURE( nPage < 0 ||
            rPrintData.GetRenderData().GetValidPagesSet().count( nPage ) == 1,
            "ViewShell::PrintOrPDFExport: nPage not valid" );
        ViewShell *const pViewSh2 = (nPage < 0)
                ? rPrintData.GetRenderData().m_pPostItShell.get()// post-it page
                : pShell; // a 'regular' page, not one from the post-it doc

        SwPageFrm const*const pStPage =
            sw_getPage(*pViewSh2->GetLayout(), abs(nPage));
        OSL_ENSURE( pStPage, "failed to get start page" );
        if (!pStPage)
        {
            return sal_False;
        }

        //!! applying view options and formatting the document should now only be done in getRendererCount!

        ::SetSwVisArea( pViewSh2, pStPage->Frm() );

// FIXME disabled because rPrintData.aOffset is always (0,0)
#if 0
        //  wenn wir einen Umschlag drucken wird ein Offset beachtet
        if( pStPage->GetFmt()->GetPoolFmtId() == RES_POOLPAGE_JAKET )
        {
            Point aNewOrigin = pOutDev->GetMapMode().GetOrigin();
            aNewOrigin += rPrintData.aOffset;
            MapMode aTmp( pOutDev->GetMapMode() );
            aTmp.SetOrigin( aNewOrigin );
            pOutDev->SetMapMode( aTmp );
        }
#endif

        pShell->InitPrt( pOutDev );

        ::SetSwVisArea( pViewSh2, pStPage->Frm() );

        pStPage->GetUpper()->Paint( pStPage->Frm(), &rPrintData );

        SwPaintQueue::Repaint();
    }  //Zus. Scope wg. CurShell!

    delete pShell;

    // restore settings of OutputDevice (should be done always now since the
    // output device is now provided by a call from outside the Writer)
    pOutDev->Pop();

    return sal_True;
}

/******************************************************************************
 *  Methode     :   PrtOle2()
 *  Beschreibung:
 *  Erstellt    :   PK 07.12.94
 *  Aenderung   :   MA 16. Feb. 95
 ******************************************************************************/



void ViewShell::PrtOle2( SwDoc *pDoc, const SwViewOption *pOpt, const SwPrintData& rOptions,
                         OutputDevice* pOleOut, const Rectangle& rRect )
{
  //Wir brauchen eine Shell fuer das Drucken. Entweder hat das Doc schon
    //eine, dann legen wir uns eine neue Sicht an, oder das Doc hat noch
    //keine, dann erzeugen wir die erste Sicht.
    ViewShell *pSh;
    if( pDoc->GetCurrentViewShell() )
        pSh = new ViewShell( *pDoc->GetCurrentViewShell(), 0, pOleOut,VSHELLFLAG_SHARELAYOUT );//swmod 080129
    else    //swmod 071108//swmod 071225
        pSh = new ViewShell( *pDoc, 0, pOpt, pOleOut);//swmod 080129

    {
        SET_CURR_SHELL( pSh );
        pSh->PrepareForPrint( rOptions );
        pSh->SetPrtFormatOption( sal_True );

        SwRect aSwRect( rRect );
        pSh->aVisArea = aSwRect;

        if ( pSh->GetViewOptions()->getBrowseMode() &&
             pSh->GetNext() == pSh )
        {
            pSh->CheckBrowseView( sal_False );
            pSh->GetLayout()->Lower()->InvalidateSize();
        }

        // CalcPagesForPrint() should not be necessary here. The pages in the
        // visible area will be formatted in SwRootFrm::Paint().
        // Removing this gives us a performance gain during saving the
        // document because the thumbnail creation will not trigger a complete
        // formatting of the document.
        // Seiten fuers Drucken formatieren
        // pSh->CalcPagesForPrint( SHRT_MAX );
        //#39275# jetzt will der Meyer doch ein Clipping
        pOleOut->Push( PUSH_CLIPREGION );
        pOleOut->IntersectClipRegion( aSwRect.SVRect() );
        pSh->GetLayout()->Paint( aSwRect );
//      SFX_APP()->SpoilDemoOutput( *pOleOut, rRect );

        pOleOut->Pop();
        // erst muss das CurrShell Object zerstoert werden!!
    }
    delete pSh;
}

/******************************************************************************
 *  Methode     :   IsAnyFieldInDoc()
 *  Beschreibung:   Stellt fest, ob im DocNodesArray Felder verankert sind
 *  Erstellt    :   JP 27.07.95
 *  Aenderung   :   JP 10.12.97
 ******************************************************************************/



sal_Bool ViewShell::IsAnyFieldInDoc() const
{
    const SfxPoolItem* pItem;
    sal_uInt32 nMaxItems = pDoc->GetAttrPool().GetItemCount2( RES_TXTATR_FIELD );
    for( sal_uInt32 n = 0; n < nMaxItems; ++n )
        if( 0 != (pItem = pDoc->GetAttrPool().GetItem2( RES_TXTATR_FIELD, n )))
        {
            const SwFmtFld* pFmtFld = (SwFmtFld*)pItem;
            const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
            //#i101026# mod: do not include postits in field check
            const SwField* pFld = pFmtFld->GetFld();
            if( pTxtFld && pTxtFld->GetTxtNode().GetNodes().IsDocNodes() && (pFld->Which() != RES_POSTITFLD))
                return sal_True;
        }
    return sal_False;
}



/******************************************************************************
 *  SwDrawViewSave
 *
 *  Saves some settings at the draw view
 ******************************************************************************/

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


// OD 09.01.2003 #i6467# - method also called for page preview
void ViewShell::PrepareForPrint( const SwPrintData &rOptions )
{
    pOpt->SetGraphic ( sal_True == rOptions.bPrintGraphic );
    pOpt->SetTable   ( sal_True == rOptions.bPrintTable );
    pOpt->SetDraw    ( sal_True == rOptions.bPrintDraw  );
    pOpt->SetControl ( sal_True == rOptions.bPrintControl );
    pOpt->SetPageBack( sal_True == rOptions.bPrintPageBackground );
    pOpt->SetBlackFont( sal_True == rOptions.bPrintBlackFont );

    if ( HasDrawView() )
    {
        SdrView *pDrawView = GetDrawView();
        String sLayerNm;
        sLayerNm.AssignAscii(RTL_CONSTASCII_STRINGPARAM("Controls" ));
        // OD 09.01.2003 #i6467# - consider, if view shell belongs to page preview
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
