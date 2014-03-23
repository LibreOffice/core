/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include <sfx2/opengrf.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svtools/filter.hxx>
#include <svl/stritem.hxx>
#include <vcl/msgbox.hxx>
#include <tools/urlobj.hxx>
#include <avmedia/mediawindow.hxx>
#include <vcl/svapp.hxx>

#include "fuinsert.hxx"
#include "tabvwsh.hxx"
#include "drwlayer.hxx"
#include "drawview.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "progress.hxx"
#include "sc.hrc"
#include "globstr.hrc"



////========================================================================
////    class ImportProgress
////
////  Bemerkung:
////    Diese Klasse stellt lediglich den Handler fuer den ImportProgress des
////    Grafikfilters bereit.
////========================================================================
//
//class ImportProgress
//{
//public:
//      ImportProgress( GraphicFilter& rFilter );
//      ~ImportProgress();
//
//  DECL_LINK( Update, GraphicFilter* );
//
//private:
//  ScProgress aProgress;
//};
//
////------------------------------------------------------------------------
//
//ImportProgress::ImportProgress( GraphicFilter& rFilter )
//  : aProgress( NULL, // SfxViewFrame*, NULL == alle Docs locken
//               String( ScResId(STR_INSERTGRAPHIC) ),
//               100 )
//{
//  rFilter.SetUpdatePercentHdl( LINK( this, ImportProgress, Update) );
//}
//
////------------------------------------------------------------------------
//
//__EXPORT ImportProgress::~ImportProgress()
//{
//  aProgress.SetState( 100 );
//}
//
////------------------------------------------------------------------------
//
//IMPL_LINK( ImportProgress, Update, GraphicFilter*, pGraphicFilter )
//{
//  aProgress.SetState( pGraphicFilter->GetPercent() );
//  return 0;
//}


//------------------------------------------------------------------------

void SC_DLLPUBLIC ScLimitSizeOnDrawPage(basegfx::B2DVector& rSize, basegfx::B2DPoint& rPos, const basegfx::B2DVector& rPageSize)
{
    if(rPageSize.equalZero())
        return;

    basegfx::B2DVector aPageSize(rPageSize);
    const bool bNegative(aPageSize.getX() < 0.0);

    if ( bNegative )
    {
        //  make everything positive temporarily
        aPageSize.setX(-aPageSize.getX());
        rPos.setX(-rPos.getX() - rSize.getX());
    }

    if ( rSize.getX() > aPageSize.getX() || rSize.getY() > aPageSize.getY() )
    {
        const double fX(aPageSize.getX() / rSize.getX());
        const double fY(aPageSize.getY() / rSize.getY());

        if ( fX < fY )
        {
            rSize.setX(aPageSize.getX());
            rSize.setY(rSize.getY() * fX);
        }
        else
        {
            rSize.setY(aPageSize.getY());
            rSize.setX(rSize.getX() * fY);
        }

        if(basegfx::fTools::equalZero(rSize.getX()))
            rSize.setX(1.0);

        if(basegfx::fTools::equalZero(rSize.getY()))
            rSize.setY(1.0);
    }

    if ( rPos.getX() + rSize.getX() > aPageSize.getX() )
        rPos.setX(aPageSize.getX() - rSize.getX());

    if ( rPos.getY() + rSize.getY() > aPageSize.getY() )
        rPos.setY(aPageSize.getY() - rSize.getY());

    if ( bNegative )
        rPos.setX(-rPos.getX() - rSize.getX());     // back to real position
}

//------------------------------------------------------------------------

void lcl_InsertGraphic( const Graphic& rGraphic,
                        const String& rFileName, const String& rFilterName, sal_Bool bAsLink, sal_Bool bApi,
                        ScTabViewShell* pViewSh, Window* pWindow, SdrView* pView )
{
    ScDrawView* pDrawView = pViewSh->GetScDrawView();

    // #123922# check if an existing object is selected; if yes, evtl. replace
    // the graphic for a SdrGraphObj (including link state updates) or adapt the fill
    // style for other objects
    SdrObject* pPickObj = pDrawView ? pDrawView->getSelectedIfSingle() : 0;

    if(pPickObj)
    {
        //sal_Int8 nAction(DND_ACTION_MOVE);
        //Point aPos;
        const String aBeginUndo(ScGlobal::GetRscString(STR_UNDO_DRAGDROP));
        const String aEmpty;

        SdrObject* pResult = pDrawView->ApplyGraphicToObject(
            *pPickObj,
            rGraphic,
            aBeginUndo,
            bAsLink ? rFileName : aEmpty,
            bAsLink ? rFilterName : aEmpty);

        if(pResult)
        {
            // we are done; mark the modified/new object
            pDrawView->MarkObj(*pResult);
            return;
        }
    }

    //  #74778# set the size so the graphic has its original pixel size
    //  at 100% view scale (as in SetMarkedOriginalSize),
    //  instead of respecting the current view scale
    MapMode aSourceMap = rGraphic.GetPrefMapMode();
    MapMode aDestMap( MAP_100TH_MM );
    if ( aSourceMap.GetMapUnit() == MAP_PIXEL && pDrawView )
    {
        Fraction aScaleX, aScaleY;
        pDrawView->CalcNormScale( aScaleX, aScaleY );
        aDestMap.SetScaleX(aScaleX);
        aDestMap.SetScaleY(aScaleY);
    }

    const Size aOldLogicSize(pWindow->LogicToLogic(rGraphic.GetPrefSize(), &aSourceMap, &aDestMap));
    basegfx::B2DVector aLogicSize(aOldLogicSize.Width(), aOldLogicSize.Height());

    //  Limit size
    SdrPageView* pPV  = pView->GetSdrPageView();

    if(pPV)
    {
        SdrPage& rPage = pPV->getSdrPageFromSdrPageView();
        basegfx::B2DPoint aInsertPos(pViewSh->GetInsertPos());
        ScViewData* pData = pViewSh->GetViewData();

        if ( pData->GetDocument()->IsNegativePage( pData->GetTabNo() ) )
            aInsertPos.setX(aInsertPos.getX() - aLogicSize.getX());     // move position to left edge

        ScLimitSizeOnDrawPage( aLogicSize, aInsertPos, rPage.GetPageScale() );

        SdrGrafObj* pObj = new SdrGrafObj(
            pView->getSdrModelFromSdrView(),
            rGraphic,
            basegfx::tools::createScaleTranslateB2DHomMatrix(aLogicSize, aInsertPos));

        // #118522# calling SetGraphicLink here doesn't work
        //  #49961# Path is no longer used as name for the graphics object

        ScDrawLayer& rLayer = dynamic_cast< ScDrawLayer& >(pView->getSdrModelFromSdrView());
        String aName = rLayer.GetNewGraphicName();                  // "Grafik x"
        pObj->SetName(aName);

        //  don't select if from (dispatch) API, to allow subsequent cell operations
        sal_uLong nInsOptions = bApi ? SDRINSERT_DONTMARK : 0;
        pView->InsertObjectAtView( *pObj, nInsOptions );

        // #118522# SetGraphicLink has to be used after inserting the object,
        // otherwise an empty graphic is swapped in and the contact stuff crashes.
        // See #i37444#.
        if ( bAsLink )
            pObj->SetGraphicLink( rFileName, rFilterName );
    }
}

//------------------------------------------------------------------------

void lcl_InsertMedia( const ::rtl::OUString& rMediaURL, bool bApi,
                      ScTabViewShell* pViewSh, Window* pWindow, SdrView* pView,
                      const Size& rPrefSize )
{
    SdrPageView*    pPV  = pView->GetSdrPageView();

    if(pPV)
    {
        SdrPage& rPage = pPV->getSdrPageFromSdrPageView();
        ScViewData*     pData = pViewSh->GetViewData();
        basegfx::B2DPoint aInsertPos(pViewSh->GetInsertPos());
        basegfx::B2DVector aScale(0.0, 0.0);

        if( rPrefSize.Width() && rPrefSize.Height() )
        {
            const basegfx::B2DVector aPrefScale(rPrefSize.Width(), rPrefSize.Height());

            if( pWindow )
            {
                aScale = pWindow->GetInverseViewTransformation(MapMode(MAP_100TH_MM)) * aPrefScale;
            }
            else
            {
                aScale = Application::GetDefaultDevice()->GetInverseViewTransformation(MapMode(MAP_100TH_MM)) * aPrefScale;
            }
        }
        else
        {
            aScale = basegfx::B2DVector( 5000.0, 5000.0 );
        }

        ScLimitSizeOnDrawPage( aScale, aInsertPos, rPage.GetPageScale() );

        if( pData->GetDocument()->IsNegativePage( pData->GetTabNo() ) )
            aInsertPos.setX(aInsertPos.getX() - aScale.getX());

        SdrMediaObj* pObj = new SdrMediaObj(
            pView->getSdrModelFromSdrView(),
            basegfx::tools::createScaleTranslateB2DHomMatrix(aScale, aInsertPos));

        pObj->setURL( rMediaURL );
        pView->InsertObjectAtView( *pObj, bApi ? SDRINSERT_DONTMARK : 0 );
    }
}

/*************************************************************************
|*
|* FuInsertGraphic::Konstruktor
|*
\************************************************************************/

#ifdef _MSC_VER
#pragma optimize("",off)
#endif

FuInsertGraphic::FuInsertGraphic( ScTabViewShell*   pViewSh,
                                  Window*           pWin,
                                  ScDrawView*       pViewP,
                                  SdrModel*         pDoc,
                                  SfxRequest&       rReq )
       : FuPoor(pViewSh, pWin, pViewP, pDoc, rReq)
{
    const SfxItemSet* pReqArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    if ( pReqArgs &&
         pReqArgs->GetItemState( SID_INSERT_GRAPHIC, sal_True, &pItem ) == SFX_ITEM_SET )
    {
        String aFileName = ((const SfxStringItem*)pItem)->GetValue();

        String aFilterName;
        if ( pReqArgs->GetItemState( FN_PARAM_FILTER, sal_True, &pItem ) == SFX_ITEM_SET )
            aFilterName = ((const SfxStringItem*)pItem)->GetValue();

        sal_Bool bAsLink = sal_False;
        if ( pReqArgs->GetItemState( FN_PARAM_1, sal_True, &pItem ) == SFX_ITEM_SET )
            bAsLink = ((const SfxBoolItem*)pItem)->GetValue();

        Graphic aGraphic;
        int nError = GraphicFilter::LoadGraphic( aFileName, aFilterName, aGraphic, GraphicFilter::GetGraphicFilter() );
        if ( nError == GRFILTER_OK )
        {
            lcl_InsertGraphic( aGraphic, aFileName, aFilterName, bAsLink, sal_True, pViewSh, pWindow, pView );
        }
    }
    else
    {
        SvxOpenGraphicDialog aDlg(ScResId(STR_INSERTGRAPHIC));

        if( aDlg.Execute() == GRFILTER_OK )
        {
            Graphic aGraphic;
            int nError = aDlg.GetGraphic(aGraphic);
            if( nError == GRFILTER_OK )
            {
                String aFileName = aDlg.GetPath();
                String aFilterName = aDlg.GetCurrentFilter();
                sal_Bool bAsLink = aDlg.IsAsLink();

                lcl_InsertGraphic( aGraphic, aFileName, aFilterName, bAsLink, sal_False, pViewSh, pWindow, pView );

                //  append items for recording
                rReq.AppendItem( SfxStringItem( SID_INSERT_GRAPHIC, aFileName ) );
                rReq.AppendItem( SfxStringItem( FN_PARAM_FILTER, aFilterName ) );
                rReq.AppendItem( SfxBoolItem( FN_PARAM_1, bAsLink ) );
                rReq.Done();
            }
            else
            {
                //  error is handled in SvxOpenGraphicDialog::GetGraphic

#if 0
                sal_uInt16 nRes = 0;
                switch ( nError )
                {
                    case GRFILTER_OPENERROR:    nRes = SCSTR_GRFILTER_OPENERROR;    break;
                    case GRFILTER_IOERROR:      nRes = SCSTR_GRFILTER_IOERROR;      break;
                    case GRFILTER_FORMATERROR:  nRes = SCSTR_GRFILTER_FORMATERROR;  break;
                    case GRFILTER_VERSIONERROR: nRes = SCSTR_GRFILTER_VERSIONERROR; break;
                    case GRFILTER_FILTERERROR:  nRes = SCSTR_GRFILTER_FILTERERROR;  break;
                    case GRFILTER_TOOBIG:       nRes = SCSTR_GRFILTER_TOOBIG;       break;
                }
                if ( nRes )
                {
                    InfoBox aInfoBox( pWindow, String(ScResId(nRes)) );
                    aInfoBox.Execute();
                }
                else
                {
                    sal_uLong nStreamError = GetGrfFilter()->GetLastError().nStreamError;
                    if( ERRCODE_NONE != nStreamError )
                        ErrorHandler::HandleError( nStreamError );
                }
#endif
            }
        }
    }
}

/*************************************************************************
|*
|* FuInsertGraphic::Destruktor
|*
\************************************************************************/

FuInsertGraphic::~FuInsertGraphic()
{
}

/*************************************************************************
|*
|* FuInsertGraphic::Function aktivieren
|*
\************************************************************************/

void FuInsertGraphic::Activate()
{
    FuPoor::Activate();
}

/*************************************************************************
|*
|* FuInsertGraphic::Function deaktivieren
|*
\************************************************************************/

void FuInsertGraphic::Deactivate()
{
    FuPoor::Deactivate();
}

/*************************************************************************
|*
|* FuInsertMedia::Konstruktor
|*
\************************************************************************/

FuInsertMedia::FuInsertMedia( ScTabViewShell*   pViewSh,
                              Window*           pWin,
                              ScDrawView*       pViewP,
                              SdrModel*         pDoc,
                              SfxRequest&       rReq ) :
    FuPoor(pViewSh, pWin, pViewP, pDoc, rReq)
{
    ::rtl::OUString     aURL;
    const SfxItemSet*   pReqArgs = rReq.GetArgs();
    bool                bAPI = false;

    if( pReqArgs )
    {
        const SfxStringItem* pStringItem = dynamic_cast< const SfxStringItem* >( &pReqArgs->Get( rReq.GetSlot() ) );

        if( pStringItem )
        {
            aURL = pStringItem->GetValue();
            bAPI = aURL.getLength();
        }
    }

    if( bAPI || ::avmedia::MediaWindow::executeMediaURLDialog( pWindow, aURL ) )
    {
        Size aPrefSize;

        if( pWin )
            pWin->EnterWait();

        if( !::avmedia::MediaWindow::isMediaURL( aURL, true, &aPrefSize ) )
        {
            if( pWin )
                pWin->LeaveWait();

            if( !bAPI )
                ::avmedia::MediaWindow::executeFormatErrorBox( pWindow );
        }
        else
        {
            lcl_InsertMedia( aURL, bAPI, pViewSh, pWindow, pView, aPrefSize );

            if( pWin )
                pWin->LeaveWait();
        }
    }
}

/*************************************************************************
|*
|* FuInsertMedia::Destruktor
|*
\************************************************************************/

FuInsertMedia::~FuInsertMedia()
{
}

/*************************************************************************
|*
|* FuInsertMedia::Function aktivieren
|*
\************************************************************************/

void FuInsertMedia::Activate()
{
    FuPoor::Activate();
}

/*************************************************************************
|*
|* FuInsertMedia::Function deaktivieren
|*
\************************************************************************/

void FuInsertMedia::Deactivate()
{
    FuPoor::Deactivate();
}
