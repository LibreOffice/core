/*************************************************************************
 *
 *  $RCSfile: drawview.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-25 14:57:47 $
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

// INCLUDE ---------------------------------------------------------------

#include <svx/svditer.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdvmark.hxx>
#include <svx/xoutx.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/ipfrm.hxx>
#include <so3/ipobj.hxx>
#include <so3/pseudo.hxx>

#include "drawview.hxx"
#include "global.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "drawutil.hxx"
#include "globstr.hrc"
#include "tabvwsh.hxx"
#include "client.hxx"
#include "scmod.hxx"
#include "drwlayer.hxx"

#include "sc.hrc"

// -----------------------------------------------------------------------

#define SC_HANDLESIZE_BIG       9
#define SC_HANDLESIZE_SMALL     7

// -----------------------------------------------------------------------


#ifdef WNT
#pragma optimize ( "", off )
#endif


void ScDrawView::Construct()
{
    EnableExtendedKeyInputDispatcher(FALSE);
    EnableExtendedMouseEventDispatcher(FALSE);
    EnableExtendedCommandEventDispatcher(FALSE);

    SetFrameDragSingles(TRUE);
//  SetSolidMarkHdl(TRUE);              // einstellbar -> UpdateUserViewOptions

    SetMinMoveDistancePixel( 2 );
    SetHitTolerancePixel( 2 );

    if (pViewData)
    {
        USHORT nTab = pViewData->GetTabNo();
        ShowPagePgNum( nTab, Point() );

        BOOL bEx = pViewData->GetViewShell()->IsDrawSelMode();
        BOOL bProt = pDoc->IsTabProtected( nTab ) ||
                     pViewData->GetSfxDocShell()->IsReadOnly();

        SdrLayer* pLayer;
        SdrLayerAdmin& rAdmin = GetModel()->GetLayerAdmin();
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_BACK);
        if (pLayer)
            SetLayerLocked( pLayer->GetName(), bProt || !bEx );
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_INTERN);
        if (pLayer)
            SetLayerLocked( pLayer->GetName(), TRUE );
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_FRONT);
        if (pLayer)
        {
            SetLayerLocked( pLayer->GetName(), bProt );
            SetActiveLayer( pLayer->GetName() );        // FRONT als aktiven Layer setzen
        }
        pLayer = rAdmin.GetLayerPerID(SC_LAYER_CONTROLS);
        if (pLayer)
            SetLayerLocked( pLayer->GetName(), bProt );

        SetSwapAsynchron(TRUE);
    }
    else
        ShowPagePgNum( nTab, Point() );

    UpdateUserViewOptions();
    RecalcScale();
    UpdateWorkArea();

    bInConstruct = FALSE;
}

__EXPORT ScDrawView::~ScDrawView()
{
    delete pDropMarker;
}

void ScDrawView::AddCustomHdl()
{
    const SdrMarkList &rMrkList = GetMarkList();
    UINT32 nCount = rMrkList.GetMarkCount();
    for(UINT32 nPos=0; nPos<nCount; nPos++ )
    {
        const SdrObject* pObj = rMrkList.GetMark(nPos)->GetObj();
        if(ScDrawLayer::GetAnchor(pObj) == SCA_CELL)
        {
            const INT32 nDelta = 1;

            Point aPos = pObj->GetBoundRect().TopLeft();
            long nPosX = (long) (aPos.X() / HMM_PER_TWIPS) + nDelta;
            long nPosY = (long) (aPos.Y() / HMM_PER_TWIPS) + nDelta;

            UINT16 nCol;
            INT32 nWidth = 0;

            for(nCol=0; nCol<=MAXCOL && nWidth<=nPosX; nCol++)
                nWidth += pDoc->GetColWidth(nCol,nTab);

            if(nCol)
                --nCol;

            UINT16 nRow;
            INT32 nHeight = 0;

            for(nRow=0; nRow<=MAXROW && nHeight<=nPosY; nRow++)
                nHeight += pDoc->FastGetRowHeight(nRow,nTab);

            if(nRow)
                --nRow;

            ScTabView* pView = pViewData->GetView();
            ScAddress aScAddress(nCol, nRow, nTab);
            pView->CreateAnchorHandles(aHdl, aScAddress);
        }
    }
}

void ScDrawView::InvalidateAttribs()
{
    if (!pViewData) return;
    SfxBindings& rBindings = pViewData->GetBindings();

        // echte Statuswerte:

    rBindings.Invalidate( SID_ATTR_FILL_STYLE );
    rBindings.Invalidate( SID_ATTR_FILL_COLOR );

    rBindings.Invalidate( SID_ATTR_LINE_STYLE );
    rBindings.Invalidate( SID_ATTR_LINE_WIDTH );
    rBindings.Invalidate( SID_ATTR_LINE_COLOR );

    rBindings.Invalidate( SID_ANCHOR_PAGE );
    rBindings.Invalidate( SID_ANCHOR_CELL );

    rBindings.Invalidate( SID_OLE_OBJECT );

    rBindings.Invalidate( SID_HYPERLINK_GETLINK );

        // Funktionen, die nicht immer moeglich sind:

    rBindings.Invalidate( SID_ATTRIBUTES_LINE );
    rBindings.Invalidate( SID_ATTRIBUTES_AREA );
    rBindings.Invalidate( SID_ATTR_LINEEND_STYLE );     // Tbx-Controller

    rBindings.Invalidate( SID_GROUP );
    rBindings.Invalidate( SID_UNGROUP );
    rBindings.Invalidate( SID_ENTER_GROUP );
    rBindings.Invalidate( SID_LEAVE_GROUP );
    rBindings.Invalidate( SID_MIRROR_HORIZONTAL );
    rBindings.Invalidate( SID_MIRROR_VERTICAL );

    rBindings.Invalidate( SID_IMAP_EXEC );

    rBindings.Invalidate( SID_FRAME_UP );
    rBindings.Invalidate( SID_FRAME_DOWN );
    rBindings.Invalidate( SID_FRAME_TO_TOP );
    rBindings.Invalidate( SID_FRAME_TO_BOTTOM );
    rBindings.Invalidate( SID_OBJECT_HEAVEN );
    rBindings.Invalidate( SID_OBJECT_HELL );

    rBindings.Invalidate( SID_OBJECT_ALIGN_LEFT );
    rBindings.Invalidate( SID_OBJECT_ALIGN_CENTER );
    rBindings.Invalidate( SID_OBJECT_ALIGN_RIGHT );
    rBindings.Invalidate( SID_OBJECT_ALIGN_UP );
    rBindings.Invalidate( SID_OBJECT_ALIGN_MIDDLE );
    rBindings.Invalidate( SID_OBJECT_ALIGN_DOWN );

    rBindings.Invalidate( SID_DELETE );
    rBindings.Invalidate( SID_DELETE_CONTENTS );
    rBindings.Invalidate( SID_CUT );
    rBindings.Invalidate( SID_COPY );

    rBindings.Invalidate( SID_ANCHOR_TOGGLE );
    rBindings.Invalidate( SID_ORIGINALSIZE );

    rBindings.Invalidate( SID_ATTR_TRANSFORM );

    rBindings.Invalidate( SID_ATTR_GRAF_MODE );
    rBindings.Invalidate( SID_ATTR_GRAF_RED );
    rBindings.Invalidate( SID_ATTR_GRAF_GREEN );
    rBindings.Invalidate( SID_ATTR_GRAF_BLUE );
    rBindings.Invalidate( SID_ATTR_GRAF_LUMINANCE );
    rBindings.Invalidate( SID_ATTR_GRAF_CONTRAST );
    rBindings.Invalidate( SID_ATTR_GRAF_GAMMA );
    rBindings.Invalidate( SID_ATTR_GRAF_TRANSPARENCE );

    rBindings.Invalidate( SID_GRFFILTER );
    rBindings.Invalidate( SID_GRFFILTER_INVERT );
    rBindings.Invalidate( SID_GRFFILTER_SMOOTH );
    rBindings.Invalidate( SID_GRFFILTER_SHARPEN );
    rBindings.Invalidate( SID_GRFFILTER_REMOVENOISE );
    rBindings.Invalidate( SID_GRFFILTER_SOBEL );
    rBindings.Invalidate( SID_GRFFILTER_MOSAIC );
    rBindings.Invalidate( SID_GRFFILTER_EMBOSS );
    rBindings.Invalidate( SID_GRFFILTER_POSTER );
    rBindings.Invalidate( SID_GRFFILTER_POPART );
    rBindings.Invalidate( SID_GRFFILTER_SEPIA );
    rBindings.Invalidate( SID_GRFFILTER_SOLARIZE );
}

void ScDrawView::InvalidateDrawTextAttrs()
{
    if (!pViewData) return;
    SfxBindings& rBindings = pViewData->GetBindings();

    //  cjk/ctl font items have no configured slots,
    //  need no invalidate

    rBindings.Invalidate( SID_ATTR_CHAR_FONT );
    rBindings.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
    rBindings.Invalidate( SID_ATTR_CHAR_WEIGHT );
    rBindings.Invalidate( SID_ATTR_CHAR_POSTURE );
    rBindings.Invalidate( SID_ATTR_CHAR_UNDERLINE );
    rBindings.Invalidate( SID_ULINE_VAL_NONE );
    rBindings.Invalidate( SID_ULINE_VAL_SINGLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOUBLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOTTED );
    rBindings.Invalidate( SID_ATTR_CHAR_COLOR );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_LEFT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_CENTER );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_RIGHT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_BLOCK );
    rBindings.Invalidate( SID_ATTR_PARA_LINESPACE_10 );
    rBindings.Invalidate( SID_ATTR_PARA_LINESPACE_15 );
    rBindings.Invalidate( SID_ATTR_PARA_LINESPACE_20 );
    rBindings.Invalidate( SID_SET_SUPER_SCRIPT );
    rBindings.Invalidate( SID_SET_SUB_SCRIPT );
}

void ScDrawView::DrawMarks( OutputDevice* pOut ) const
{
//  if (IsMarkHdlShown())
//      DrawMarkHdl(pOut,FALSE);

    USHORT nWinNum = ((ScDrawView*)this)->FindWin(pOut);        //! DrawMarks nicht-const
    if (nWinNum!=SDRVIEWWIN_NOTFOUND)
    {
        ((ScDrawView*)this)->AfterInitRedraw(nWinNum);          //! DrawMarks nicht-const

        if (IsShownXorVisibleWinNum(nWinNum))
            ((ScDrawView*)this)->ToggleShownXor(pOut,NULL);     //! DrawMarks nicht-const
    }
}

void ScDrawView::SetMarkedToLayer( BYTE nLayerNo )
{
    //
    //!         Undo !!!
    //

    if (HasMarkedObj())
    {
        const SdrMarkList& rMark = GetMarkList();
        ULONG nCount = rMark.GetMarkCount();
        for (ULONG i=0; i<nCount; i++)
        {
            SdrObject* pObj = rMark.GetMark(i)->GetObj();
            if ( !pObj->ISA(SdrUnoObj) )
                pObj->SetLayer( nLayerNo );
        }

        //  Paint passiert beim SetLayer
    }
}

BOOL ScDrawView::HasMarkedControl() const
{
    if (HasMarkedObj())
    {
        const SdrMarkList& rMark = GetMarkList();
        ULONG nCount = rMark.GetMarkCount();
        for (ULONG i=0; i<nCount; i++)
        {
            SdrObject* pObj = rMark.GetMark(i)->GetObj();
            if ( pObj->ISA(SdrUnoObj) )
                return TRUE;
            else if ( pObj->ISA(SdrObjGroup) )
            {
                SdrObjListIter aIter( *pObj, IM_DEEPWITHGROUPS );
                SdrObject* pSubObj = aIter.Next();
                while (pSubObj)
                {
                    if ( pSubObj->ISA(SdrUnoObj) )
                        return TRUE;
                    pSubObj = aIter.Next();
                }
            }

        }
    }
    return FALSE;       // war nix
}

void ScDrawView::UpdateWorkArea()
{
    SdrPage* pPage = GetModel()->GetPage(nTab);
    if (pPage)
        SetWorkArea( Rectangle( Point(), pPage->GetSize() ) );
    else
        DBG_ERROR("Page nicht gefunden");
}

void ScDrawView::DoCut()
{
    DoCopy();
    BegUndo( ScGlobal::GetRscString( STR_UNDO_CUT ) );
    DeleteMarked();     // auf dieser View - von der 505f Umstellung nicht betroffen
    EndUndo();
}

void ScDrawView::GetScale( Fraction& rFractX, Fraction& rFractY ) const
{
    rFractX = aScaleX;
    rFractY = aScaleY;
}

void ScDrawView::RecalcScale()
{
    double nPPTX;
    double nPPTY;
    Fraction aZoomX(1,1);
    Fraction aZoomY(1,1);

    if (pViewData)
    {
        nTab = pViewData->GetTabNo();
        nPPTX = pViewData->GetPPTX();
        nPPTY = pViewData->GetPPTY();
        aZoomX = pViewData->GetZoomX();
        aZoomY = pViewData->GetZoomY();
    }
    else
    {
        Point aLogic = pDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
        nPPTX = aLogic.X() / 1000.0;
        nPPTY = aLogic.Y() / 1000.0;
                                            //! Zoom uebergeben ???
    }

    USHORT nEndCol = 0;
    USHORT nEndRow = 0;
    pDoc->GetTableArea( nTab, nEndCol, nEndRow );
    if (nEndCol<20)
        nEndCol = 20;
    if (nEndRow<20)
        nEndRow = 20;

    ScDrawUtil::CalcScale( pDoc, nTab, 0,0, nEndCol,nEndRow, pDev,aZoomX,aZoomY,nPPTX,nPPTY,
                            aScaleX,aScaleY );
}

void ScDrawView::PaintObject( SdrObject* pObject, OutputDevice* pDev ) const
{
    pXOut->SetOutDev( pDev );
    SdrPaintInfoRec aInfoRec;
    pObject->Paint( *pXOut, aInfoRec );
}

void __EXPORT ScDrawView::MarkListHasChanged()
{
    FmFormView::MarkListHasChanged();

    UpdateBrowser();

    ScTabViewShell* pViewSh = pViewData->GetViewShell();

    if (!bInConstruct)          // nicht wenn die View gerade angelegt wird
    {
        pViewSh->Unmark();      // Selektion auff'm Doc entfernen

        //  #65379# end cell edit mode if drawing objects are selected
        if ( GetMarkList().GetMarkCount() )
            SC_MOD()->InputEnterHandler();
    }

    //  IP deaktivieren

    ScClient* pClient = (ScClient*) pViewSh->GetIPClient();
    if ( pClient && pClient->IsInPlaceActive() )
    {
        //  #41730# beim ViewShell::Activate aus dem Reset2Open nicht die Handles anzeigen
        bDisableHdl = TRUE;

        pClient->GetProtocol().Reset2Open();
        SFX_APP()->SetViewFrame(pViewSh->GetViewFrame());

        bDisableHdl = FALSE;

        //  Image-Ole wieder durch Grafik ersetzen passiert jetzt in ScClient::UIActivate
    }

    //  Ole-Objekt selektiert?

    SdrOle2Obj* pOle2Obj = NULL;
    SdrGrafObj* pGrafObj = NULL;

    const SdrMarkList& rMarkList = GetMarkList();
    ULONG nMarkCount = rMarkList.GetMarkCount();

    if ( nMarkCount == 0 && !pViewData->GetViewShell()->IsDrawSelMode() && !bInConstruct )
    {
        //  re-lock background layer if it was unlocked in SelectObject
        SdrLayer* pLayer = GetModel()->GetLayerAdmin().GetLayerPerID(SC_LAYER_BACK);
        if ( pLayer && !IsLayerLocked( pLayer->GetName() ) )
            SetLayerLocked( pLayer->GetName(), TRUE );
    }

    BOOL bSubShellSet = FALSE;
    if (nMarkCount == 1)
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
        if (pObj->GetObjIdentifier() == OBJ_OLE2)
        {
            pOle2Obj = (SdrOle2Obj*) pObj;
            if (!pDoc->IsChart(pObj) )
                pViewSh->SetOleObjectShell(TRUE);
            else
                pViewSh->SetChartShell(TRUE);
            bSubShellSet = TRUE;
        }
        else if (pObj->GetObjIdentifier() == OBJ_GRAF)
        {
            pGrafObj = (SdrGrafObj*) pObj;
            pViewSh->SetGraphicShell(TRUE);
            bSubShellSet = TRUE;
        }
        else if (pObj->GetObjIdentifier() != OBJ_TEXT   // Verhindern, das beim Anlegen
                    || !pViewSh->IsDrawTextShell())     // eines TextObjekts auf die
        {                                               // DrawShell umgeschaltet wird.
            pViewSh->SetDrawShell(TRUE);                //@#70206#
        }
    }

    if ( nMarkCount && !bSubShellSet )
    {
        BOOL bOnlyControls = TRUE;
        BOOL bOnlyGraf     = TRUE;
        for (ULONG i=0; i<nMarkCount; i++)
        {
            SdrObject* pObj = rMarkList.GetMark(i)->GetObj();
            if ( pObj->ISA( SdrObjGroup ) )
            {
                const SdrObjList *pLst = ((SdrObjGroup*)pObj)->GetSubList();
                for ( USHORT j = 0; j < pLst->GetObjCount(); ++j )
                {
                    SdrObject *pSubObj = pLst->GetObj( j );

                    if (!pSubObj->ISA(SdrUnoObj))
                        bOnlyControls = FALSE;
                    if (pSubObj->GetObjIdentifier() != OBJ_GRAF)
                        bOnlyGraf = FALSE;

                    if ( !bOnlyControls && !bOnlyGraf ) break;
                }
            }
            else
            {
                if (!pObj->ISA(SdrUnoObj))
                    bOnlyControls = FALSE;
                if (pObj->GetObjIdentifier() != OBJ_GRAF)
                    bOnlyGraf = FALSE;
            }

            if ( !bOnlyControls && !bOnlyGraf ) break;
        }

        if(bOnlyControls)
        {
            pViewSh->SetDrawFormShell(TRUE);            // jetzt UNO-Controls
        }
        else if(bOnlyGraf)
        {
            pViewSh->SetGraphicShell(TRUE);
        }
        else if(nMarkCount>1)
        {
            pViewSh->SetDrawShell(TRUE);
        }
    }



    //  Verben anpassen

    BOOL bOle = pViewSh->GetViewFrame()->ISA(SfxInPlaceFrame);
    if ( pOle2Obj && !bOle )
    {
        SvInPlaceObject* pIPObj = pOle2Obj->GetObjRef();
        if (pIPObj)
            pViewSh->SetVerbs( &pIPObj->GetVerbList() );
        else
        {
            DBG_ERROR("SdrOle2Obj ohne ObjRef");
            pViewSh->SetVerbs( 0 );
        }
    }
    else
        pViewSh->SetVerbs( 0 );

    //  Image-Map Editor

    if ( pOle2Obj )
        UpdateIMap( pOle2Obj );
    else if ( pGrafObj )
        UpdateIMap( pGrafObj );

    InvalidateAttribs();                // nach dem IMap-Editor Update
    InvalidateDrawTextAttrs();

    USHORT nWinCount = GetWinCount();
    for (USHORT i=0; i<nWinCount; i++)
    {
        OutputDevice* pDev = GetWin(i);
        if (pDev->GetOutDevType() == OUTDEV_WINDOW)
            ((Window*)pDev)->Update();
    }
}

void __EXPORT ScDrawView::ModelHasChanged()
{
    FmFormView::ModelHasChanged();
}

void __EXPORT ScDrawView::UpdateUserViewOptions()
{
    if (pViewData)
    {
        const ScViewOptions&    rOpt = pViewData->GetOptions();
        const ScGridOptions&    rGrid = rOpt.GetGridOptions();

        BOOL bBigHdl = rOpt.GetOption( VOPT_BIGHANDLES );

        SetDragStripes( rOpt.GetOption( VOPT_HELPLINES ) );
        SetSolidMarkHdl( rOpt.GetOption( VOPT_SOLIDHANDLES ) );
        SetMarkHdlSizePixel( bBigHdl ? SC_HANDLESIZE_BIG : SC_HANDLESIZE_SMALL );

        SetGridVisible( rGrid.GetGridVisible() );
        SetSnapEnabled( rGrid.GetUseGridSnap() );
        SetGridSnap( rGrid.GetUseGridSnap() );
        SetSnapGrid( Size( rGrid.GetFldSnapX(), rGrid.GetFldSnapY() ) );

        SetGridCoarse( Size( rGrid.GetFldDrawX(), rGrid.GetFldDrawY() ) );
        SetGridFine( Size( rGrid.GetFldDrawX() / (rGrid.GetFldDivisionX() + 1),
                           rGrid.GetFldDrawY() / (rGrid.GetFldDivisionY() + 1) ) );
    }
}

#ifdef WNT
#pragma optimize ( "", on )
#endif

BOOL ScDrawView::SelectObject( const String& rName )
{
    UnmarkAll();

    USHORT nObjectTab = 0;
    SdrObject* pFound = NULL;

    SfxObjectShell* pShell = pDoc->GetDocumentShell();
    if (pShell)
    {
        SdrModel* pDrawLayer = GetModel();
        USHORT nTabCount = pDoc->GetTableCount();
        for (USHORT i=0; i<nTabCount && !pFound; i++)
        {
            SdrPage* pPage = pDrawLayer->GetPage(i);
            DBG_ASSERT(pPage,"Page ?");
            if (pPage)
            {
                SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
                SdrObject* pObject = aIter.Next();
                while (pObject && !pFound)
                {
                    if (pObject->GetName() == rName)
                    {
                        pFound = pObject;
                        nObjectTab = i;
                    }
                    pObject = aIter.Next();
                }
            }
        }
    }

    if ( pFound )
    {
        ScTabView* pView = pViewData->GetView();
        if ( nObjectTab != nTab )                               // Tabelle umschalten
            pView->SetTabNo( nObjectTab );

        DBG_ASSERT( nTab == nObjectTab, "Tabellen umschalten hat nicht geklappt" );

        pView->ScrollToObject( pFound );

        //  #61585# to select an object on the background layer, the layer has to
        //  be unlocked even if exclusive drawing selection mode is not active
        //  (this is reversed in MarkListHasChanged when nothing is selected)

        if ( pFound->GetLayer() == SC_LAYER_BACK &&
                !pViewData->GetViewShell()->IsDrawSelMode() &&
                !pDoc->IsTabProtected( nTab ) &&
                !pViewData->GetSfxDocShell()->IsReadOnly() )
        {
            SdrLayer* pLayer = GetModel()->GetLayerAdmin().GetLayerPerID(SC_LAYER_BACK);
            if (pLayer)
                SetLayerLocked( pLayer->GetName(), FALSE );
        }

        SdrPageView* pPV = GetPageViewPvNum(0);
        MarkObj( pFound, pPV );
    }

    return ( pFound != NULL );
}

String ScDrawView::GetSelectedChartName() const
{
    const SdrMarkList& rMarkList = GetMarkList();
    if (rMarkList.GetMarkCount() == 1)
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
        if (pObj->GetObjIdentifier() == OBJ_OLE2)
            if ( pDoc->IsChart(pObj) )
                return pObj->GetName();
    }

    return EMPTY_STRING;        // nichts gefunden
}

FASTBOOL ScDrawView::InsertObjectSafe(SdrObject* pObj, SdrPageView& rPV, ULONG nOptions)
{
    //  Markierung nicht aendern, wenn Ole-Objekt aktiv
    //  (bei Drop aus Ole-Objekt wuerde sonst mitten im ExecuteDrag deaktiviert!)

    if (pViewData)
    {
        SfxInPlaceClient* pClient = pViewData->GetViewShell()->GetIPClient();
        if ( pClient && pClient->IsInPlaceActive() )
            nOptions |= SDRINSERT_DONTMARK;
    }

    return InsertObject( pObj, rPV, nOptions );
}

void __EXPORT ScDrawView::MakeVisible( const Rectangle& rRect, Window& rWin )
{
    //! rWin richtig auswerten
    //! ggf Zoom aendern

    if ( pViewData && pViewData->GetActiveWin() == &rWin )
        pViewData->GetView()->MakeVisible( rRect );
}

SdrEndTextEditKind ScDrawView::ScEndTextEdit()
{
    BOOL bIsTextEdit = IsTextEdit();
    SdrEndTextEditKind eKind = EndTextEdit();

    if ( bIsTextEdit && pViewData )
        pViewData->GetViewShell()->SetDrawTextUndo(NULL);   // "normaler" Undo-Manager

    return eKind;
}

void ScDrawView::MarkDropObj( SdrObject* pObj )
{
    if ( pObj )
    {
        if ( !pDropMarker )
            pDropMarker = new SdrViewUserMarker(this);
        if ( pDropMarkObj != pObj )
        {
            pDropMarkObj = pObj;
            pDropMarker->SetXPolyPolygon(pDropMarkObj, GetPageViewPvNum(0));
            pDropMarker->Show();
        }
    }
    else                // Markierung aufheben
    {
        if (pDropMarker)
        {
            pDropMarker->Hide();
            pDropMarkObj = NULL;
        }
    }
}



