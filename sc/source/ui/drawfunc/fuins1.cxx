/*************************************************************************
 *
 *  $RCSfile: fuins1.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2002-09-02 13:56:29 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include <svx/impgrf.hxx>
#include <svx/opengrf.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svtools/filter.hxx>
#include <svtools/stritem.hxx>
#include <vcl/msgbox.hxx>
#include <tools/urlobj.hxx>

#include "fuinsert.hxx"
#include "tabvwsh.hxx"
#include "drwlayer.hxx"
#include "drawview.hxx"
#include "scresid.hxx"
#include "progress.hxx"
#include "sc.hrc"



//========================================================================
//  class ImportProgress
//
//  Bemerkung:
//  Diese Klasse stellt lediglich den Handler fuer den ImportProgress des
//  Grafikfilters bereit.
//========================================================================

class ImportProgress
{
public:
        ImportProgress( GraphicFilter& rFilter );
        ~ImportProgress();

    DECL_LINK( Update, GraphicFilter* );

private:
    ScProgress aProgress;
};

//------------------------------------------------------------------------

ImportProgress::ImportProgress( GraphicFilter& rFilter )
    : aProgress( NULL, // SfxViewFrame*, NULL == alle Docs locken
                 String( ScResId(STR_INSERTGRAPHIC) ),
                 100 )
{
    rFilter.SetUpdatePercentHdl( LINK( this, ImportProgress, Update) );
}

//------------------------------------------------------------------------

__EXPORT ImportProgress::~ImportProgress()
{
    aProgress.SetState( 100 );
}

//------------------------------------------------------------------------

IMPL_LINK( ImportProgress, Update, GraphicFilter*, pGraphicFilter )
{
    aProgress.SetState( pGraphicFilter->GetPercent() );
    return 0;
}


//------------------------------------------------------------------------

void LimitSizeOnDrawPage( Size& rSize, Point& rPos, const Size& rPage )
{
    if ( !rPage.Width() || !rPage.Height() )
        return;

    if ( rSize.Width() > rPage.Width() || rSize.Height() > rPage.Height() )
    {
        double fX = rPage.Width()  / (double) rSize.Width();
        double fY = rPage.Height() / (double) rSize.Height();

        if ( fX < fY )
        {
            rSize.Width()  = rPage.Width();
            rSize.Height() = (long) ( rSize.Height() * fX );
        }
        else
        {
            rSize.Height() = rPage.Height();
            rSize.Width()  = (long) ( rSize.Width() * fY );
        }

        if (!rSize.Width())
            rSize.Width() = 1;
        if (!rSize.Height())
            rSize.Height() = 1;
    }

    if ( rPos.X() + rSize.Width() > rPage.Width() )
        rPos.X() = rPage.Width() - rSize.Width();
    if ( rPos.Y() + rSize.Height() > rPage.Height() )
        rPos.Y() = rPage.Height() - rSize.Height();
}

//------------------------------------------------------------------------

void lcl_InsertGraphic( const Graphic& rGraphic,
                        const String& rFileName, const String& rFilterName, BOOL bAsLink, BOOL bApi,
                        ScTabViewShell* pViewSh, Window* pWindow, SdrView* pView )
{
    //  #74778# set the size so the graphic has its original pixel size
    //  at 100% view scale (as in SetMarkedOriginalSize),
    //  instead of respecting the current view scale

    ScDrawView* pDrawView = pViewSh->GetScDrawView();
    MapMode aSourceMap = rGraphic.GetPrefMapMode();
    MapMode aDestMap( MAP_100TH_MM );
    if ( aSourceMap.GetMapUnit() == MAP_PIXEL && pDrawView )
    {
        Fraction aScaleX, aScaleY;
        pDrawView->CalcNormScale( aScaleX, aScaleY );
        aDestMap.SetScaleX(aScaleX);
        aDestMap.SetScaleY(aScaleY);
    }
    Size aLogicSize = pWindow->LogicToLogic(
                            rGraphic.GetPrefSize(), &aSourceMap, &aDestMap );

    //  Limit size

    SdrPageView* pPV  = pView->GetPageViewPvNum(0);
    SdrPage* pPage = pPV->GetPage();
    Point aInsertPos = pViewSh->GetInsertPos();
    LimitSizeOnDrawPage( aLogicSize, aInsertPos, pPage->GetSize() );

    Rectangle aRect ( aInsertPos, aLogicSize );

    SdrGrafObj* pObj = new SdrGrafObj( rGraphic, aRect );

    if ( bAsLink )
        pObj->SetGraphicLink( rFileName, rFilterName );

    //  #49961# Path is no longer used as name for the graphics object

    ScDrawLayer* pLayer = (ScDrawLayer*) pView->GetModel();
    String aName = pLayer->GetNewGraphicName();                 // "Grafik x"
    pObj->SetName(aName);

    //  don't select if from (dispatch) API, to allow subsequent cell operations
    ULONG nInsOptions = bApi ? SDRINSERT_DONTMARK : 0;
    pView->InsertObject( pObj, *pPV, nInsOptions );
}

/*************************************************************************
|*
|* FuInsertGraphic::Konstruktor
|*
\************************************************************************/

#ifdef WNT
#pragma optimize("",off)
#endif

FuInsertGraphic::FuInsertGraphic( ScTabViewShell*   pViewSh,
                                  Window*           pWin,
                                  SdrView*          pView,
                                  SdrModel*         pDoc,
                                  SfxRequest&       rReq )
       : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    const SfxItemSet* pReqArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    if ( pReqArgs &&
         pReqArgs->GetItemState( SID_INSERT_GRAPHIC, TRUE, &pItem ) == SFX_ITEM_SET )
    {
        String aFileName = ((const SfxStringItem*)pItem)->GetValue();

        String aFilterName;
        if ( pReqArgs->GetItemState( FN_PARAM_FILTER, TRUE, &pItem ) == SFX_ITEM_SET )
            aFilterName = ((const SfxStringItem*)pItem)->GetValue();

        BOOL bAsLink = FALSE;
        if ( pReqArgs->GetItemState( FN_PARAM_1, TRUE, &pItem ) == SFX_ITEM_SET )
            bAsLink = ((const SfxBoolItem*)pItem)->GetValue();

        Graphic aGraphic;
        USHORT nError = ::LoadGraphic( aFileName, aFilterName, aGraphic, ::GetGrfFilter() );
        if ( nError == GRFILTER_OK )
        {
            lcl_InsertGraphic( aGraphic, aFileName, aFilterName, bAsLink, TRUE, pViewSh, pWindow, pView );
        }
    }
    else
    {
        SvxOpenGraphicDialog aDlg(ScResId(STR_INSERTGRAPHIC));

        if( aDlg.Execute() == GRFILTER_OK )
        {
            Graphic aGraphic;
            USHORT nError = aDlg.GetGraphic(aGraphic);
            if( nError == GRFILTER_OK )
            {
                String aFileName = aDlg.GetPath();
                String aFilterName = aDlg.GetCurrentFilter();
                BOOL bAsLink = aDlg.IsAsLink();

                lcl_InsertGraphic( aGraphic, aFileName, aFilterName, bAsLink, FALSE, pViewSh, pWindow, pView );

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
                USHORT nRes = 0;
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
                    ULONG nStreamError = GetGrfFilter()->GetLastError().nStreamError;
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



