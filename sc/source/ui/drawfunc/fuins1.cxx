/*************************************************************************
 *
 *  $RCSfile: fuins1.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:56 $
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
#include <svx/svdograf.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svtools/filter.hxx>
#include <svtools/stritem.hxx>
#include <vcl/msgbox.hxx>
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

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
    SvxImportGraphicDialog* pDlg = NULL;
    Graphic aImportGraphic;
    String  aPath;
    String  aFilterName;
    BOOL    bError = FALSE;
    BOOL    bAsLink = FALSE;
    Graphic* pInsGraphic = NULL;

    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    if ( pArgs && pArgs->GetItemState( SID_INSERT_GRAPHIC, TRUE, &pItem ) == SFX_ITEM_SET )
    {
        DBG_ASSERT( pItem->ISA(SfxStringItem), "falsches Item" );
        aPath = ((const SfxStringItem*) pItem)->GetValue();

        if ( pArgs->GetItemState( FN_PARAM_FILTER, TRUE, &pItem ) == SFX_ITEM_SET )
        {
            DBG_ASSERT( pItem->ISA(SfxStringItem), "falsches Item" );
            aFilterName = ((const SfxStringItem*) pItem)->GetValue();
        }
        if ( pArgs->GetItemState( FN_PARAM_1, TRUE, &pItem ) == SFX_ITEM_SET )
        {
            DBG_ASSERT( pItem->ISA(SfxBoolItem), "falsches Item" );
            bAsLink = ((const SfxBoolItem*) pItem)->GetValue();
        }

        if ( GRFILTER_OK == ::LoadGraphic( aPath, aFilterName, aImportGraphic ) )
            pInsGraphic = &aImportGraphic;
        else
            bError = TRUE;
    }

    if ( !pInsGraphic && !bError )                  // noch keine geladen -> Dialog
    {
        pDlg = new SvxImportGraphicDialog( pWindow,
                                      String( ScResId(STR_INSERTGRAPHIC) ),
                                      ENABLE_STANDARD | ENABLE_LINK );

        if ( pDlg->Execute() == RET_OK )
        {
            aPath       = pDlg->GetPath();
            aFilterName = pDlg->GetCurFilter();
            bAsLink     = pDlg->AsLink();
            pInsGraphic = pDlg->GetGraphic();           // per Preview schon geladen?

            if ( !pInsGraphic )
            {
                // Grafik laden, wenn Dialog es nicht bereits getan hat

                GraphicFilter&  rFilter  = pDlg->GetFilter();
                ImportProgress  aImportProgress( rFilter );
                INetURLObject aURL;
                aURL.SetSmartURL( aPath );
                USHORT nError = rFilter.ImportGraphic( aImportGraphic, aURL );
                if ( nError == GRFILTER_OK )
                    pInsGraphic = &aImportGraphic;
                else
                {
                    //! merge error strings with Writer and Impress!!!
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
                }
            }

            rReq.AppendItem( SfxStringItem( SID_INSERT_GRAPHIC, aPath       ) );
            rReq.AppendItem( SfxStringItem( FN_PARAM_FILTER,    aFilterName ) );
            rReq.AppendItem( SfxBoolItem(   FN_PARAM_1,         bAsLink     ) );
        }
    }

    if ( pInsGraphic )              // irgendwo eine Grafik gefunden -> einfuegen
    {
        //  #74778# set the size so the graphic has its original pixel size
        //  at 100% view scale (as in SetMarkedOriginalSize),
        //  instead of respecting the current view scale

        ScDrawView* pDrawView = pViewSh->GetScDrawView();
        MapMode aSourceMap = pInsGraphic->GetPrefMapMode();
        MapMode aDestMap( MAP_100TH_MM );
        if ( aSourceMap.GetMapUnit() == MAP_PIXEL && pDrawView )
        {
            Fraction aScaleX, aScaleY;
            pDrawView->CalcNormScale( aScaleX, aScaleY );
            aDestMap.SetScaleX(aScaleX);
            aDestMap.SetScaleY(aScaleY);
        }
        Size aLogicSize = pWindow->LogicToLogic(
                                pInsGraphic->GetPrefSize(), &aSourceMap, &aDestMap );

        //  Groesse begrenzen

        SdrPageView* pPV  = pView->GetPageViewPvNum(0);
        SdrPage* pPage = pPV->GetPage();
        Point aInsertPos = pViewSh->GetInsertPos();
        LimitSizeOnDrawPage( aLogicSize, aInsertPos, pPage->GetSize() );

        Rectangle aRect ( aInsertPos, aLogicSize );

        SdrGrafObj* pObj = new SdrGrafObj( *pInsGraphic, aRect );

        if ( bAsLink )
            pObj->SetGraphicLink( aPath, aFilterName );

        //  #49961# Pfad wird nicht mehr als Name der Grafik gesetzt

        ScDrawLayer* pLayer = (ScDrawLayer*) pView->GetModel();
        String aName = pLayer->GetNewGraphicName();                 // "Grafik x"
        pObj->SetName(aName);

        pView->InsertObject( pObj, *pPV );

        rReq.Done();
    }

    rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), pInsGraphic != NULL ) );

    delete pDlg;        // Dialog erst am Ende loeschen
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



