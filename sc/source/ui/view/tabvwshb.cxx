/*************************************************************************
 *
 *  $RCSfile: tabvwshb.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-05 16:48:23 $
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

//------------------------------------------------------------------

#ifdef WNT
#pragma optimize ("", off)
#endif

// INCLUDE ---------------------------------------------------------------

#include <svx/pfiledlg.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdview.hxx>
#include <svx/linkmgr.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <so3/ipenv.hxx>
#include <so3/ipobj.hxx>
#include <so3/linkdlg.hxx>
#include <so3/svstor.hxx>
#include <so3/soerr.hxx>
#include <svtools/rectitem.hxx>
#include <svtools/whiter.hxx>
#include <sch/schdll.hxx>
#include <sch/memchrt.hxx>
#include <sch/schdll0.hxx>



#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif
#ifndef SO2_DECL_SVSTORAGE_DEFINED
#define SO2_DECL_SVSTORAGE_DEFINED
SO2_DECL_REF(SvStorage)
#endif

#include "tabvwsh.hxx"
#include "globstr.hrc"
#include "scmod.hxx"
#include "document.hxx"
#include "sc.hrc"
#include "client.hxx"
#include "fuinsert.hxx"
#include "docsh.hxx"

// STATIC DATA -----------------------------------------------------------

void ScTabViewShell::ConnectObject( SdrOle2Obj* pObj )
{
    //  wird aus dem Paint gerufen

    SvInPlaceObjectRef xIPObj = pObj->GetObjRef();
    Window* pWin = GetActiveWin();

    //  #41412# wenn schon connected ist, nicht nochmal SetObjArea/SetSizeScale

    SfxInPlaceClientRef xClient = FindIPClient( xIPObj, pWin );
    if ( !xClient.Is() )
    {
        xClient = new ScClient( this, pWin, GetSdrView()->GetModel() );

        ErrCode nErr = xIPObj->DoConnect( xClient );
        if (nErr != ERRCODE_NONE)
            ErrorHandler::HandleError(nErr);

        Rectangle aRect = pObj->GetLogicRect();
        Size aDrawSize = aRect.GetSize();
        Size aOleSize = xIPObj->GetVisArea().GetSize();

                // sichtbarer Ausschnitt wird nur inplace veraendert!
        aRect.SetSize( aOleSize );
        xClient->GetEnv()->SetObjArea( aRect );

        Fraction aScaleWidth (aDrawSize.Width(),  aOleSize.Width() );
        Fraction aScaleHeight(aDrawSize.Height(), aOleSize.Height() );
        aScaleWidth.ReduceInaccurate(10);       // kompatibel zum SdrOle2Obj
        aScaleHeight.ReduceInaccurate(10);
        xClient->GetEnv()->SetSizeScale(aScaleWidth,aScaleHeight);

        ((ScClient*)(SfxInPlaceClient*)xClient)->SetGrafEdit( NULL );
    }
}

BOOL ScTabViewShell::ActivateObject( SdrOle2Obj* pObj, long nVerb )
{
    // #41081# Gueltigkeits-Hinweisfenster nicht ueber dem Objekt stehenlassen
    RemoveHintWindow();

    SvInPlaceObjectRef xIPObj = pObj->GetObjRef();
    Window* pWin = GetActiveWin();
    ErrCode nErr = ERRCODE_NONE;
    BOOL bErrorShown = FALSE;

    if ( xIPObj->IsLink() )
        nErr = xIPObj->DoVerb(nVerb);           // gelinkt -> ohne Client etc.
    else
    {
        SfxInPlaceClientRef xClient = FindIPClient( xIPObj, pWin );
        if ( !xClient.Is() )
            xClient = new ScClient( this, pWin, GetSdrView()->GetModel() );
        nErr = xIPObj->DoConnect( xClient );

        if ( !(nErr & ERRCODE_ERROR_MASK) )
        {
            Rectangle aRect = pObj->GetLogicRect();
            Size aDrawSize = aRect.GetSize();
            Size aOleSize = xIPObj->GetVisArea().GetSize();
            aOleSize = OutputDevice::LogicToLogic( aOleSize,
                                                   xIPObj->GetMapUnit(), MAP_100TH_MM );

                    // sichtbarer Ausschnitt wird nur inplace veraendert!
            aRect.SetSize( aOleSize );
            xClient->GetEnv()->SetObjArea( aRect );

            Fraction aScaleWidth (aDrawSize.Width(),  aOleSize.Width() );
            Fraction aScaleHeight(aDrawSize.Height(), aOleSize.Height() );
            aScaleWidth.ReduceInaccurate(10);       // kompatibel zum SdrOle2Obj
            aScaleHeight.ReduceInaccurate(10);
            xClient->GetEnv()->SetSizeScale(aScaleWidth,aScaleHeight);

            ((ScClient*)(SfxInPlaceClient*)xClient)->SetGrafEdit( NULL );

            //  Link fuer Daten-Highlighting im Chart setzen
            if ( SFX_APP()->HasFeature(SFX_FEATURE_SCHART) )
            {
                SvGlobalName aObjClsId = *xIPObj->GetSvFactory();
                if (SchModuleDummy::HasID( aObjClsId ))
                {
                    SchMemChart* pMemChart = SchDLL::GetChartData(xIPObj);
                    if (pMemChart)
                        pMemChart->SetSelectionHdl( LINK( GetViewData()->GetDocShell(),
                                                    ScDocShell, ChartSelectionHdl ) );
                }
            }

            nErr = SfxViewShell::DoVerb( xClient, nVerb );
            bErrorShown = TRUE;
            // SfxViewShell::DoVerb zeigt seine Fehlermeldungen selber an
        }
    }
    if (nErr != ERRCODE_NONE && !bErrorShown)
        ErrorHandler::HandleError(nErr);

    //! SetDocumentName sollte schon im Sfx passieren ???
    xIPObj->SetDocumentName( GetViewData()->GetDocShell()->GetTitle() );

    return ( !(nErr & ERRCODE_ERROR_MASK) );
}

ErrCode __EXPORT ScTabViewShell::DoVerb(long nVerb)
{
    SdrView* pView = GetSdrView();
    if (!pView)
        return ERRCODE_SO_NOTIMPL;          // soll nicht sein

    SdrOle2Obj* pOle2Obj = NULL;
    SdrGrafObj* pGrafObj = NULL;
    SdrObject* pObj = NULL;
    ErrCode nErr = ERRCODE_NONE;

    const SdrMarkList& rMarkList = pView->GetMarkList();
    if (rMarkList.GetMarkCount() == 1)
    {
        pObj = rMarkList.GetMark(0)->GetObj();
        if (pObj->GetObjIdentifier() == OBJ_OLE2)
            pOle2Obj = (SdrOle2Obj*) pObj;
        else if (pObj->GetObjIdentifier() == OBJ_GRAF)
        {
            pGrafObj = (SdrGrafObj*) pObj;
        }
    }

    if (pOle2Obj)
    {
        ActivateObject( pOle2Obj, nVerb );
    }
#if 0
    //! remove this along with pGrafEdit member of ScClient
    else if ( pGrafObj && SFX_APP()->HasFeature(SFX_FEATURE_SIMAGE) )
    {
        /**********************************************************
        * OLE-Objekt erzeugen, StarImage starten
        * Grafik-Objekt loeschen (durch OLE-Objekt ersetzt)
        **********************************************************/

        pView->HideMarkHdl(NULL);

        String aEmtpyString;
        SvStorageRef aStor = new SvStorage(String());
        SvInPlaceObjectRef aNewIPObj =
#ifndef SO3
                    &SvInPlaceObject::ClassFactory()->CreateAndInit(
                    *SIM_MOD()->pSimDrawDocShellFactory, aStor );
#else
                    &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                    *SIM_MOD()->pSimDrawDocShellFactory, aStor );
#endif
        if ( aNewIPObj.Is() )
        {
            SdrGrafObj* pTempSdrGrafObj = (SdrGrafObj*) pObj->Clone ();
            pView->BegUndo( ScGlobal::GetRscString( STR_UNDO_GRAFEDIT ) );

            SvEmbeddedInfoObject* pInfo =
                    GetViewFrame()->GetObjectShell()->
                                   InsertObject( aNewIPObj, String() );
            String aName;
            if( pInfo )
                aName = pInfo->GetObjName();

            Rectangle aRect = pObj->GetLogicRect();
            SdrOle2Obj* pSdrOle2Obj = new SdrOle2Obj( aNewIPObj, aName, aRect );

            SdrPageView* pPV = pView->GetPageViewPvNum(0);
            pView->ReplaceObject(pObj, *pPV, pSdrOle2Obj);
            pSdrOle2Obj->SetLogicRect(aRect);   // erst nach InsertObject !!!
            aNewIPObj->SetVisAreaSize( aRect.GetSize() );

            const Graphic& rGraphic = pTempSdrGrafObj->GetGraphic();
            SimDLL::Update(aNewIPObj, rGraphic, GetActiveWin() );

//                          pView->EndUndo();
            //  passendes EndUndo in ScDrawView::MarkListHasChanged

            ActivateObject( pSdrOle2Obj, SVVERB_SHOW );

            ScClient* pClient = (ScClient*) GetIPClient();
            pClient->SetGrafEdit( pTempSdrGrafObj );
        }
    }
#endif
    else
        DBG_ERROR("kein Objekt fuer Verb gefunden");

    return nErr;
}

void ScTabViewShell::ExecDrawIns(SfxRequest& rReq)
{
    USHORT nSlot = rReq.GetSlot();
    if (nSlot != SID_OBJECTRESIZE )
    {
        SC_MOD()->InputEnterHandler();
        UpdateInputHandler();
    }

    //  Rahmen fuer Chart einfuegen wird abgebrochen:
    FuPoor* pPoor = GetDrawFuncPtr();
    if ( pPoor && pPoor->GetSlotID() == SID_DRAW_CHART )
        GetViewData()->GetDispatcher().Execute(SID_DRAW_CHART, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);

    MakeDrawLayer();

    SfxBindings& rBindings = GetViewFrame()->GetBindings();
    ScTabView*   pTabView  = GetViewData()->GetView();
    Window*      pWin      = pTabView->GetActiveWin();
    SdrView*     pView     = pTabView->GetSdrView();
    ScDocShell*  pDocSh    = GetViewData()->GetDocShell();
    ScDocument*  pDoc      = pDocSh->GetDocument();
//  SdrModel*    pDrModel  = pDocSh->MakeDrawLayer();
    SdrModel*    pDrModel  = pView->GetModel();

    switch ( nSlot )
    {
        case SID_INSERT_GRAPHIC:
            FuInsertGraphic(this, pWin, pView, pDrModel, rReq);
            // shell is set in MarkListHasChanged
            break;

        case SID_INSERT_DIAGRAM:
            FuInsertChart(this, pWin, pView, pDrModel, rReq);
//?         SC_MOD()->SetFunctionDlg( NULL );//XXX
            break;

        case SID_INSERT_OBJECT:
        case SID_INSERT_PLUGIN:
        case SID_INSERT_SOUND:
        case SID_INSERT_VIDEO:
        case SID_INSERT_APPLET:
        case SID_INSERT_SIMAGE:
        case SID_INSERT_SMATH:
        case SID_INSERT_FLOATINGFRAME:
            FuInsertOLE(this, pWin, pView, pDrModel, rReq);
            break;

        case SID_OBJECTRESIZE:
            {
                //          Der Server moechte die Clientgrosse verandern

                SfxInPlaceClient* pIPClient = GetIPClient();

                if ( pIPClient && pIPClient->IsInPlaceActive() )
                {
                    const SfxRectangleItem& rRect =
                        (SfxRectangleItem&)rReq.GetArgs()->Get(SID_OBJECTRESIZE);
                    Rectangle aRect( pWin->PixelToLogic( rRect.GetValue() ) );

                    if ( pView->HasMarkedObj() )
                    {
                        const SdrMarkList& rMarkList = pView->GetMarkList();

                        if (rMarkList.GetMarkCount() == 1)
                        {
                            SdrMark* pMark = rMarkList.GetMark(0);
                            SdrObject* pObj = pMark->GetObj();

                            UINT16 nSdrObjKind = pObj->GetObjIdentifier();

                            if (nSdrObjKind == OBJ_OLE2)
                            {
                                SvInPlaceObjectRef aIPObj =
                                ( (SdrOle2Obj*) pObj)->GetObjRef();

                                if ( aIPObj.Is() )
                                {
                                    pObj->SetLogicRect(aRect);
                                }
                            }
                        }
                    }
                }
            }
            break;

        case SID_LINKS:
            {
                SvBaseLinksDialog( pWin, pDoc->GetLinkManager() ).Execute();
                rBindings.Invalidate( nSlot );
                SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );     // Navigator
                rReq.Done();
            }
    }
}

void ScTabViewShell::GetDrawInsState(SfxItemSet &rSet)
{
    BOOL bOle = GetViewFrame()->ISA(SfxInPlaceFrame);
    BOOL bTabProt = GetViewData()->GetDocument()->IsTabProtected(GetViewData()->GetTabNo());
    SfxApplication* pSfxApp = SFX_APP();

    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_INSERT_DIAGRAM:
            case SID_OPENDLG_CHART:
                if ( bOle || bTabProt || !pSfxApp->HasFeature(SFX_FEATURE_SCHART) )
                    rSet.DisableItem( nWhich );
                break;

            case SID_INSERT_SIMAGE:
                if ( bOle || bTabProt || !pSfxApp->HasFeature(SFX_FEATURE_SIMAGE) )
                    rSet.DisableItem( nWhich );
                break;

            case SID_INSERT_SMATH:
                if ( bOle || bTabProt || !pSfxApp->HasFeature(SFX_FEATURE_SMATH) )
                    rSet.DisableItem( nWhich );
                break;

            case SID_INSERT_OBJECT:
            case SID_INSERT_PLUGIN:
            case SID_INSERT_FLOATINGFRAME:
                if (bOle || bTabProt)
                    rSet.DisableItem( nWhich );
                break;

            case SID_INSERT_SOUND:
            case SID_INSERT_VIDEO:
                if (bOle || bTabProt || !SvxPluginFileDlg::IsAvailable(nWhich))
                    rSet.DisableItem( nWhich );
                break;

            case SID_INSERT_APPLET:
                //  wenn SOLAR_JAVA nicht definiert ist, immer disablen
#ifdef SOLAR_JAVA
                if (bOle || bTabProt)
#endif
                    rSet.DisableItem( nWhich );
                break;

            case SID_INSERT_GRAPHIC:
                if (bTabProt)
                    rSet.DisableItem( nWhich );
                break;

            case SID_LINKS:
                {
                    if (GetViewData()->GetDocument()->GetLinkManager()->GetLinks().Count() == 0 )
                        rSet.DisableItem( SID_LINKS );
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}




