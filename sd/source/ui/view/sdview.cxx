/*************************************************************************
 *
 *  $RCSfile: sdview.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2000-10-30 11:50:43 $
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

#ifndef _E3D_OBJ3D_HXX
#include <svx/obj3d.hxx>
#endif
#ifndef _SVX_FMVIEW_HXX
#include <svx/fmview.hxx>
#endif

#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVDOGRAF_HXX
#include <svx/svdograf.hxx>
#endif
#ifndef _SVDOOLE2_HXX
#include <svx/svdoole2.hxx>
#endif
#ifndef _SVDUNDO_HXX
#include <svx/svdundo.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _IPENV_HXX //autogen
#include <so3/ipenv.hxx>
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif

#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif

#ifndef _SVDOUTL_HXX //autogen
#include <svx/svdoutl.hxx>
#endif

#pragma hdrstop

#include "app.hrc"
#include "sdwindow.hxx"
#include "sdview.hxx"
#include "sdclient.hxx"
#include "drawdoc.hxx"
#include "docshell.hxx"
#include "app.hxx"
#include "dragserv.hxx"
#include "sdpage.hxx"
#include "glob.hrc"
#include "sdresid.hxx"
#include "drviewsh.hxx"
#include "graphpro.hxx"
#include "futext.hxx"
#include "fuinsfil.hxx"
#include "fuslshow.hxx"
#include "stlpool.hxx"
#include "frmview.hxx"

#include "strings.hrc"

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif

// statisches Flag, das anzeigt, ob momentan gedropt
// werden darf
BOOL bIsDropAllowed;

TYPEINIT1( SdView, FmFormView );

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

SdView::SdView(SdDrawDocument* pDrawDoc, OutputDevice* pOutDev,
               SdViewShell* pViewShell)
  : FmFormView(pDrawDoc, pOutDev),
    pDoc(pDrawDoc),
    pDocSh( pDrawDoc->GetDocSh() ),
    pViewSh(pViewShell),
    bDragActive (FALSE),
    pDragSrcMarkList(NULL),
    nDragSrcPgNum(SDRPAGE_NOTFOUND),
    pDropMarkerObj(NULL),
    pDropMarker(NULL),
    pLockedRedraws(NULL),
    nLockRedrawSmph(0),
    eAction(DROP_NONE)
{
    EnableExtendedKeyInputDispatcher(FALSE);
    EnableExtendedMouseEventDispatcher(FALSE);
    EnableExtendedCommandEventDispatcher(FALSE);

    bIsDropAllowed = TRUE;
    SetUseIncompatiblePathCreateInterface(FALSE);
    SetMarkHdlWhenTextEdit(TRUE);
    EnableTextEditOnObjectsWithoutTextIfTextTool(TRUE);

    SetMinMoveDistancePixel(2);
    SetHitTolerancePixel(2);
    SetMeasureLayer(String(SdResId(STR_LAYER_MEASURELINES)));

    // Timer fuer verzoegertes Drop (muss fuer MAC sein)
    aDropErrorTimer.SetTimeoutHdl( LINK(this, SdView, DropErrorHdl) );
    aDropErrorTimer.SetTimeout(50);
    aDropInsertFileTimer.SetTimeoutHdl( LINK(this, SdView, DropInsertFileHdl) );
    aDropInsertFileTimer.SetTimeout(50);
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

__EXPORT SdView::~SdView()
{
    aDropErrorTimer.Stop();
    aDropInsertFileTimer.Stop();

    delete pDropMarker;

    while (GetWin(0))
    {
        // Alle angemeldeten OutDevs entfernen
        DelWin(GetWin(0));
    }

    // gespeicherte Redraws loeschen
    if (pLockedRedraws)
    {
        SdViewRedrawRec* pRec = (SdViewRedrawRec*)pLockedRedraws->First();
        while (pRec)
        {
            delete pRec;
            pRec = (SdViewRedrawRec*)pLockedRedraws->Next();
        }
        delete pLockedRedraws;
    }

}


/*************************************************************************
|*
|* Paint-Methode: das Ereignis wird an die View weitergeleitet
|*
\************************************************************************/

void __EXPORT SdView::InitRedraw(OutputDevice* pOutDev, const Region& rReg)
{
    // ausfuehren ??
    if (nLockRedrawSmph == 0)
    {
        FmFormView::InitRedraw(pOutDev, rReg);

        USHORT nDemoKind =  SFX_APP()->GetDemoKind();

        if ( (nDemoKind == SFX_DEMOKIND_DEMO || nDemoKind == SFX_DEMOKIND_INVALID)
             && pOutDev->GetOutDevType() == OUTDEV_PRINTER )
        {
            SdrPageView* pPgView = GetPageViewPvNum(0);

            if (pPgView)
            {
                SdPage* pPage = (SdPage*) pPgView->GetPage();

                if (pPage)
                {
                    Rectangle aPageRect ( Point(0, 0), pPage->GetSize() );
                    SFX_APP()->SpoilDemoOutput(*pOutDev, aPageRect);
                }
            }
        }
    }
    // oder speichern?
    else
    {
        if (!pLockedRedraws)
            pLockedRedraws = new List;

        SdViewRedrawRec* pRec = new SdViewRedrawRec;
        pRec->pOut  = pOutDev;
        pRec->aRect = rReg.GetBoundRect();
        pLockedRedraws->Insert(pRec, LIST_APPEND);
    }
}


/*************************************************************************
|*
|* Selektion hat sich geaendert
|*
\************************************************************************/

void __EXPORT SdView::MarkListHasChanged()
{
    FmFormView::MarkListHasChanged();
}


/*************************************************************************
|*
|* Attribute setzen
|*
\************************************************************************/

BOOL SdView::SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll)
{
    BOOL bOk = FmFormView::SetAttributes(rSet, bReplaceAll);
    return (bOk);
}


/*************************************************************************
|*
|* Attribute holen
|*
\************************************************************************/

BOOL SdView::GetAttributes( SfxItemSet& rTargetSet, BOOL bOnlyHardAttr ) const
{
    return( FmFormView::GetAttributes( rTargetSet, bOnlyHardAttr ) );
}


/*************************************************************************
|*
|* Ist ein Praesentationsobjekt selektiert?
|*
\************************************************************************/

BOOL SdView::IsPresObjSelected(BOOL bOnPage, BOOL bOnMasterPage)
{
    /**************************************************************************
    * Ist ein Presentationsobjekt selektiert?
    **************************************************************************/
    SdrMarkList* pMarkList;

    if (nDragSrcPgNum != SDRPAGE_NOTFOUND &&
        nDragSrcPgNum != GetPageViewPvNum(0)->GetPage()->GetPageNum())
    {
        // Es laeuft gerade Drag&Drop
        // Source- und Destination-Page unterschiedlich:
        // es wird die gemerkte MarkList verwendet
        pMarkList = pDragSrcMarkList;
    }
    else
    {
        // Es wird die aktuelle MarkList verwendet
        pMarkList = new SdrMarkList(aMark);
    }

    SdrMark* pMark;
    SdPage* pPage;
    SdrObject* pObj;

    BOOL bSelected = FALSE;
    BOOL bMasterPage = FALSE;
    long nMark;
    long nMarkMax = pMarkList->GetMarkCount() - 1;

    for (nMark = nMarkMax; nMark >= 0; nMark--)
    {
        // Rueckwaerts durch die Marklist
        pMark = pMarkList->GetMark(nMark);
        pObj = pMark->GetObj();

        if ( pObj && ( pObj->IsEmptyPresObj() || pObj->GetUserCall() ) )
        {
            pPage = (SdPage*) pObj->GetPage();
            bMasterPage = pPage->IsMasterPage();

            if (bMasterPage && bOnMasterPage || !bMasterPage && bOnPage)
            {
                if ( pPage && pPage->GetPresObjList()->GetPos(pObj)
                              != LIST_ENTRY_NOTFOUND )
                {
                    bSelected = TRUE;
                }
            }
        }
    }

    if (pMarkList != pDragSrcMarkList)
    {
       delete pMarkList;
    }

    return (bSelected);
}

/*************************************************************************
|*
|* DataObject fuer Drag&Drop erzeugen
|*
\************************************************************************/

SdDataObjectRef SdView::CreateDataObject(SdView* pView, const Point& rDragPos)
{
    SdDataObjectRef pDataObject = new SdDataObject(pDoc, pView);
    SD_MOD()->pDragData = pDataObject;

    SdrOle2Obj* pSdrOleObj = NULL;

    if (aMark.GetMarkCount() == 1)
    {
        // Genau ein OLE-Objekt vorhanden
        SdrObject* pObj = aMark.GetMark(0)->GetObj();

        if (pObj && pObj->ISA(SdrOle2Obj) && ((SdrOle2Obj*) pObj)->GetObjRef().Is())
        {
            pSdrOleObj = (SdrOle2Obj*) pObj;
        }
    }
    // ObjectDescriptor aufbauen
    SvObjectDescriptor* pObjDesc = NULL;

    if (pSdrOleObj)
    {
        // ObjectDescriptor fuer das OLE-Objekt erzeugen
        pObjDesc = new SvObjectDescriptor(pSdrOleObj->GetObjRef(), rDragPos);
    }
    else
    {
        // Standard-ObjectDescriptor erzeugen
        pObjDesc = new SvObjectDescriptor(pDocSh, rDragPos);
    }

    pObjDesc->SetSize( GetAllMarkedRect().GetSize() );

    String aDisplayName;

    if (pDocSh)
    {
        const INetURLObject& rURLObj =  pDocSh->GetMedium()->GetURLObject();
        aDisplayName = rURLObj.GetURLNoPass();
    }

    pObjDesc->SetDisplayName(aDisplayName);

    // Wenn das Dokument, nicht aber der selektierte Bereich gelinkt werden kann
    pObjDesc->SetCanLink(FALSE);

    // Eigentuemeruebergang
    pDataObject->SetObjectDescriptor(pObjDesc);

    pDataObject->pSdView = pView;
    pDataObject->aStartPos = rDragPos;

    return (pDataObject);
}


/*************************************************************************
|*
|* DataObject fuers Clipboard erzeugen
|*
\************************************************************************/

SdDataObjectRef SdView::CreateDataObject()
{
    // DataObject aufbauen
    SdDataObjectRef pDataObject = new SdDataObject(pDoc);
    SD_MOD()->pClipboardData = pDataObject;
    pDoc->CreatingDataObj(TRUE);
    SdDrawDocument* pDocument = (SdDrawDocument*) GetAllMarkedModel();
    pDoc->CreatingDataObj(FALSE);
    pDataObject->SetDocument(pDocument);

    // Groesse der Source-Seite uebernehmen
    SdrPageView* pPgView = GetPageViewPvNum(0);
    SdPage* pPage = (SdPage*) pPgView->GetPage();
    SdPage* pNewPage = (SdPage*) pDocument->GetPage(0);
    pNewPage->SetSize( pPage->GetSize() );
    pNewPage->SetLayoutName( pPage->GetLayoutName() );

    // ObjectDescriptor aufbauen
    SdrOle2Obj* pSdrOleObj = NULL;

    if (aMark.GetMarkCount() == 1)
    {
        // Genau ein OLE-Objekt vorhanden
        SdrObject* pObj = aMark.GetMark(0)->GetObj();

        if (pObj && pObj->ISA(SdrOle2Obj) && ((SdrOle2Obj*) pObj)->GetObjRef().Is())
        {
            pSdrOleObj = (SdrOle2Obj*) pObj;
        }
    }
    // ObjectDescriptor aufbauen
    SvObjectDescriptor* pObjDesc = NULL;

    if (pSdrOleObj)
    {
        // ObjectDescriptor fuer das OLE-Objekt erzeugen
        pObjDesc = new SvObjectDescriptor(pSdrOleObj->GetObjRef(), Point());
    }
    else
    {
        // Standard-ObjectDescriptor erzeugen
        SdDrawDocShell* pDocShell = pDocument->GetDocSh();
        pObjDesc = new SvObjectDescriptor(pDocShell, Point() );
    }

    Rectangle aMarkRect(GetAllMarkedRect());
    pObjDesc->SetSize(aMarkRect.GetSize());

    // Position des MarkRects
    pDataObject->aStartPos = aMarkRect.TopLeft();

    String aDisplayName;

    if (pDocSh)
    {
        const INetURLObject& rURLObj =  pDocSh->GetMedium()->GetURLObject();
        aDisplayName = rURLObj.GetURLNoPass();
    }

    pObjDesc->SetDisplayName(aDisplayName);

    // Wenn das Dokument, nicht aber der selektierte Bereich gelinkt werden kann
    pObjDesc->SetCanLink(FALSE);

    // Eigentuemeruebergang
    pDataObject->SetObjectDescriptor(pObjDesc);

    pDataObject->CopyClipboard();

    return (pDataObject);
}


/*************************************************************************
|*
|* Alles selektieren
|*
\************************************************************************/

void SdView::SelectAll()
{
    if ( IsTextEdit() )
    {
        OutlinerView* pOLV = GetTextEditOutlinerView();
        const Outliner* pOutliner = GetTextEditOutliner();
        pOLV->SelectRange( 0, pOutliner->GetParagraphCount() );
    }
    else
    {
        MarkAll();
    }
}


/*************************************************************************
|*
|* Dokument hat sich geaendert
|*
\************************************************************************/

void __EXPORT SdView::ModelHasChanged()
{
    // Erst SdrView benachrichtigen
    FmFormView::ModelHasChanged();
}

/*************************************************************************
|*
|* StyleSheet setzen
|*
\************************************************************************/

BOOL SdView::SetStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr)
{
    // weiter an SdrView
    return FmFormView::SetStyleSheet(pStyleSheet, bDontRemoveHardAttr);
}


/*************************************************************************
|*
|* Texteingabe beginnen
|*
\************************************************************************/

BOOL SdView::BegTextEdit(SdrObject* pObj, SdrPageView* pPV, Window* pWin,
                         BOOL bIsNewObj, SdrOutliner* pGivenOutliner,
                         OutlinerView* pGivenOutlinerView, BOOL bDontDeleteOutliner,
                         BOOL bOnlyOneView)
{
    BOOL bReturn = FmFormView::BegTextEdit(pObj, pPV, pWin, bIsNewObj, pGivenOutliner,
                                        pGivenOutlinerView, bDontDeleteOutliner,
                                        bOnlyOneView);

    if (bReturn)
    {
        // UndoManager an der obersten Shell (SdDrawTextObjectBar) setzen
        Outliner* pOL = GetTextEditOutliner();
        SfxUndoManager& rUndoMgr = pOL->GetUndoManager();
        SdViewShell* pViewShell = pDocSh->GetViewShell();
        DBG_ASSERT(pViewShell, "ViewShell nicht gefunden");
        if (pViewShell)
        {
            pViewShell->GetViewFrame()->GetDispatcher()->
                GetShell(0)->SetUndoManager(&rUndoMgr);
        }
    }

    return(bReturn);
}

/*************************************************************************
|*
|* Texteingabe beenden
|*
\************************************************************************/

SdrEndTextEditKind SdView::EndTextEdit(BOOL bDontDeleteReally)
{
    BOOL bIsTextEdit = IsTextEdit();

    SdrEndTextEditKind eKind;

    SdViewShell* pViewShell= pDocSh->GetViewShell();

    if ( pViewShell && pViewShell->ISA(SdDrawViewShell) )
    {
        FuPoor* pFunc = ( (SdDrawViewShell*) pViewShell)->GetActualFunction();

        if ( !pFunc || !pFunc->ISA(FuText) )
            pFunc = ( (SdDrawViewShell*) pViewShell)->GetOldFunction();

        if ( pFunc && pFunc->ISA(FuText) )
        {
            BOOL bDefaultTextRestored = ( (FuText*) pFunc)->RestoreDefaultText();

            eKind = FmFormView::EndTextEdit(bDontDeleteReally);

            SdrTextObj* pTextObj = ( (FuText*) pFunc)->GetTextObj();

            if (pViewShell && pTextObj)
            {
                FuSlideShow* pFuSlideShow = pViewShell->GetSlideShow();

                if (pFuSlideShow)
                {
                    pFuSlideShow->EndTextEdit(pTextObj);
                }
            }

            if (eKind == SDRENDTEXTEDIT_CHANGED && !bDefaultTextRestored)
            {
                ( (FuText*) pFunc)->ObjectChanged();
            }
        }
        else
        {
            eKind = FmFormView::EndTextEdit(bDontDeleteReally);
        }
    }
    else
    {
        eKind = FmFormView::EndTextEdit(bDontDeleteReally);
    }

    if (bIsTextEdit)
    {
        // UndoManager an der obersten Shell setzen
        SfxUndoManager* pUndoMgr = pDocSh->GetUndoManager();
        SdViewShell* pViewShell = pDocSh->GetViewShell();
        DBG_ASSERT(pViewShell, "ViewShell nicht gefunden");
        if (pViewShell)
        {
            pViewShell->GetViewFrame()->GetDispatcher()->
                GetShell(0)->SetUndoManager(pUndoMgr);
        }
    }

    return(eKind);
}


/*************************************************************************
|*
|* Originalgroesse der markierten Objekte setzen
|*
\************************************************************************/

void SdView::SetMarkedOriginalSize()
{
    SdrUndoGroup* pUndoGroup = new SdrUndoGroup(*pDoc);

    BOOL bOK = FALSE;
    ULONG nCount = aMark.GetMarkCount();

    for (ULONG i=0; i<nCount; i++)
    {
        SdrObject* pObj = aMark.GetMark(i)->GetObj();

        if (pObj->GetObjInventor() == SdrInventor)
        {
            if (pObj->GetObjIdentifier() == OBJ_OLE2)
            {
                SvInPlaceObjectRef xIPObj = ((SdrOle2Obj*)pObj)->GetObjRef();
                if (xIPObj.Is())
                {
                    bOK = TRUE;
                    Size aOleSize = OutputDevice::LogicToLogic(
                                        xIPObj->GetVisArea().GetSize(),
                                        xIPObj->GetMapUnit(), MAP_100TH_MM);
                    Rectangle aDrawRect = pObj->GetLogicRect();

                    pUndoGroup->AddAction( new SdrUndoGeoObj( *pObj ) );
                    pObj->Resize( aDrawRect.TopLeft(), Fraction( aOleSize.Width(), aDrawRect.GetWidth() ),
                                                       Fraction( aOleSize.Height(), aDrawRect.GetHeight() ) );
                }
            }
            else if (pObj->GetObjIdentifier() == OBJ_GRAF)
            {
                bOK = TRUE;

                // Falls der PrefMapMode der Graphic MAP_PIXEL ist,
                // machen wir die logische Umrechnung ueber das Fenster,
                // um eine 1:1-Abbildung auf Pixel zu erreichen
                // (KA 19.12.95)
                Size aSize;
                if (((SdrGrafObj*) pObj)->GetGrafPrefMapMode().GetMapUnit() == MAP_PIXEL && pViewSh)
                {
                    SdWindow* pWindow = pViewSh->GetActiveWindow();
                    const MapMode&   rWinMap = pWindow->GetMapMode();
                    const Fraction&  rWinScaleX = rWinMap.GetScaleX();
                    const Fraction&  rWinScaleY = rWinMap.GetScaleY();

                    aSize = pWindow->PixelToLogic( ((SdrGrafObj*) pObj)->GetGrafPrefSize() );
                    aSize.Width() = ( aSize.Width() * rWinScaleX.GetNumerator() /
                                      rWinScaleX.GetDenominator() );
                    aSize.Height() = ( aSize.Height() * rWinScaleY.GetNumerator() /
                                       rWinScaleY.GetDenominator() );
                }
                else
                {
                    aSize = OutputDevice::LogicToLogic(((SdrGrafObj*) pObj)->GetGrafPrefSize(),
                                                       ((SdrGrafObj*) pObj)->GetGrafPrefMapMode(),
                                                       MapMode( MAP_100TH_MM ));
                }

                // Groesse ggf. auf Seitengroesse begrenzen
                SdrPage* pPage = GetPageViewPvNum(0)->GetPage();
                Size aPageSize = pPage->GetSize();
                aPageSize.Width() -= pPage->GetLftBorder() + pPage->GetRgtBorder();
                aPageSize.Height() -= pPage->GetUppBorder() + pPage->GetLwrBorder();

                // Falls Grafik zu gross, wird die Grafik
                // in die Seite eingepasst
                if ((aSize.Height() > aPageSize.Height()) ||
                    (aSize.Width()  > aPageSize.Width()) &&
                    aSize.Height() && aPageSize.Height())
                {
                    float fGrfWH =  (float)aSize.Width() /
                                    (float)aSize.Height();
                    float fWinWH =  (float)aPageSize.Width() /
                                    (float)aPageSize.Height();

                    // Grafik an Pagesize anpassen (skaliert)
                    if ( fGrfWH < fWinWH )
                    {
                        aSize.Width() = (long)(aPageSize.Height() * fGrfWH);
                        aSize.Height()= aPageSize.Height();
                    }
                    else if ( fGrfWH > 0.F )
                    {
                        aSize.Width() = aPageSize.Width();
                        aSize.Height()= (long)(aPageSize.Width() / fGrfWH);
                    }
                }

                pUndoGroup->AddAction(new SdrUndoGeoObj(*pObj));
                Rectangle aRect = pObj->GetLogicRect();
                aRect.SetSize(aSize);
                pObj->SetLogicRect(aRect);
            }
        }
    }

    if (bOK)
    {
        pUndoGroup->SetComment( String(SdResId(STR_UNDO_ORIGINALSIZE)) );
        pDocSh->GetUndoManager()->AddUndoAction(pUndoGroup);
    }
    else
        delete pUndoGroup;
}

/*************************************************************************
|*
|* virt. Device mit Seiteninhalt erzeugen, Device gehoert dem Caller
|*
\************************************************************************/

VirtualDevice* SdView::CreatePageVDev(USHORT nSdPage, PageKind ePageKind,
                                      ULONG nWidthPixel)
{
    SdViewShell*    pViewShell = pDocSh->GetViewShell();
    SdWindow*       pWin = pViewShell->GetActiveWindow();
    VirtualDevice*  pVDev = new VirtualDevice( *pWin );
    MapMode         aMM( MAP_100TH_MM );

    SdPage* pPage = pDoc->GetSdPage(nSdPage, ePageKind);
    DBG_ASSERT(pPage, "Seite nicht gefunden");

    Size aPageSize(pPage->GetSize());
    aPageSize.Width()  -= pPage->GetLftBorder();
    aPageSize.Width()  -= pPage->GetRgtBorder();
    aPageSize.Height() -= pPage->GetUppBorder();
    aPageSize.Height() -= pPage->GetLwrBorder();

    // use scaling?
    if( nWidthPixel )
    {
        const Fraction aFrac( (long) nWidthPixel, pVDev->LogicToPixel( aPageSize, aMM ).Width() );

        aMM.SetScaleX( aFrac );
        aMM.SetScaleY( aFrac );
    }

    pVDev->SetMapMode( aMM );
    BOOL bAbort = !pVDev->SetOutputSize(aPageSize);
    DBG_ASSERT(!bAbort, "virt. Device nicht korrekt erzeugt");

    SdrView* pView = new SdrView(pDoc, pVDev);
    pView->ShowPage(pPage, Point(-pPage->GetLftBorder(), -pPage->GetUppBorder()));
    SdrPageView* pPageView  = pView->GetPageView(pPage);
    DBG_ASSERT(pViewShell, "ViewShell nicht gefunden");
    FrameView* pFrameView   = pViewShell->GetFrameView();
    pPageView->SetVisibleLayers( pFrameView->GetVisibleLayers() );
    pPageView->SetLockedLayers( pFrameView->GetLockedLayers() );
    pPageView->SetPrintableLayers( pFrameView->GetPrintableLayers() );

    Point aPoint( pPage->GetLftBorder(), pPage->GetUppBorder() );
    Region aRegion (Rectangle( aPoint, aPageSize ) );
    pView->InitRedraw(pVDev, aRegion);
    delete pView;
    return pVDev;
}



/*************************************************************************
|*
|* OLE-Obj am Client connecten
|*
\************************************************************************/

void SdView::DoConnect(SdrOle2Obj* pObj)
{
    if (pViewSh)
    {
        const SvInPlaceObjectRef& rIPObjRef = pObj->GetObjRef();

        if (!rIPObjRef->IsLink())
        {
            SfxInPlaceClientRef pSdClient = (SdClient*) rIPObjRef->GetIPClient();

            if ( !pSdClient.Is() )
            {
                SdWindow* pWindow = pViewSh->GetActiveWindow();
                pSdClient = new SdClient(pObj, pViewSh, pWindow);

                rIPObjRef->DoConnect(pSdClient);
                Rectangle aRect = pObj->GetLogicRect();
                SvClientData* pClientData = pSdClient->GetEnv();

                if (pClientData)
                {
                    Size aDrawSize = aRect.GetSize();
                    Size aObjAreaSize = rIPObjRef->GetVisArea().GetSize();
                    aObjAreaSize = OutputDevice::LogicToLogic( aObjAreaSize,
                                                           rIPObjRef->GetMapUnit(),
                                                           pDoc->GetScaleUnit() );

                    // sichtbarer Ausschnitt wird nur inplace veraendert!
                    aRect.SetSize(aObjAreaSize);
                    pClientData->SetObjArea(aRect);

                    Fraction aScaleWidth (aDrawSize.Width(),  aObjAreaSize.Width() );
                    Fraction aScaleHeight(aDrawSize.Height(), aObjAreaSize.Height() );
                    aScaleWidth.ReduceInaccurate(10);       // kompatibel zum SdrOle2Obj
                    aScaleHeight.ReduceInaccurate(10);
                    pClientData->SetSizeScale(aScaleWidth, aScaleHeight);
                }
            }
        }
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL SdView::IsMorphingAllowed() const
{
    const SdrMarkList&  rMarkList = GetMarkList();
    BOOL                bRet = FALSE;

    if ( rMarkList.GetMarkCount() == 2 )
    {
        const SdrObject*    pObj1 = rMarkList.GetMark( 0 )->GetObj();
        const SdrObject*    pObj2 = rMarkList.GetMark( 1 )->GetObj();
        const UINT16        nKind1 = pObj1->GetObjIdentifier();
        const UINT16        nKind2 = pObj2->GetObjIdentifier();

        if ( ( nKind1 != OBJ_TEXT && nKind2 != OBJ_TEXT ) &&
             ( nKind1 != OBJ_TITLETEXT && nKind2 != OBJ_TITLETEXT ) &&
             ( nKind1 != OBJ_OUTLINETEXT && nKind2 != OBJ_OUTLINETEXT ) &&
             ( nKind1 != OBJ_GRUP && nKind2 != OBJ_GRUP ) &&
             ( nKind1 != OBJ_LINE && nKind2 != OBJ_LINE ) &&
             ( nKind1 != OBJ_PLIN && nKind2 != OBJ_PLIN ) &&
             ( nKind1 != OBJ_PATHLINE && nKind2 != OBJ_PATHLINE ) &&
             ( nKind1 != OBJ_FREELINE && nKind2 != OBJ_FREELINE ) &&
             ( nKind1 != OBJ_PATHPLIN && nKind2 != OBJ_PATHPLIN ) &&
             ( nKind1 != OBJ_MEASURE && nKind2 != OBJ_MEASURE ) &&
             ( nKind1 != OBJ_EDGE && nKind2 != OBJ_EDGE ) &&
             ( nKind1 != OBJ_GRAF && nKind2 != OBJ_GRAF ) &&
             ( nKind1 != OBJ_OLE2 && nKind2 != OBJ_OLE2 ) &&
             ( nKind1 != OBJ_CAPTION && nKind2 !=  OBJ_CAPTION ) &&
             !pObj1->ISA( E3dObject) && !pObj2->ISA( E3dObject) )
        {
            SfxItemSet      aSet1( pDoc->GetPool(), XATTR_FILLSTYLE, XATTR_FILLSTYLE );
            SfxItemSet      aSet2( pDoc->GetPool(), XATTR_FILLSTYLE, XATTR_FILLSTYLE );

//-/            pObj1->TakeAttributes( aSet1, TRUE, FALSE );
            aSet1.Put(pObj1->GetItemSet());

//-/            pObj2->TakeAttributes( aSet2, TRUE, FALSE );
            aSet2.Put(pObj2->GetItemSet());

            const XFillStyle    eFillStyle1 = ( (const XFillStyleItem&) aSet1.Get( XATTR_FILLSTYLE ) ).GetValue();
            const XFillStyle    eFillStyle2 = ( (const XFillStyleItem&) aSet2.Get( XATTR_FILLSTYLE ) ).GetValue();

            if( ( eFillStyle1 == XFILL_NONE || eFillStyle1 == XFILL_SOLID ) &&
                ( eFillStyle2 == XFILL_NONE || eFillStyle2 == XFILL_SOLID ) )
                bRet = TRUE;
        }
    }

    return bRet;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL SdView::IsVectorizeAllowed() const
{
    const SdrMarkList&  rMarkList = GetMarkList();
    BOOL                bRet = FALSE;

    if( rMarkList.GetMarkCount() == 1 )
    {
        const SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();

        if( pObj->ISA( SdrGrafObj ) && ( (SdrGrafObj*) pObj )->GetGraphicType() == GRAPHIC_BITMAP )
            bRet = TRUE;
    }

    return bRet;
}


