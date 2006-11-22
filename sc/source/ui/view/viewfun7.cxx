/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewfun7.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-22 11:52:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifndef _COM_SUN_STAR_EMBED_NOVISUALAREASIZEEXCEPTION_HPP_
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#endif



// INCLUDE ---------------------------------------------------------------

#include <svx/svditer.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#include <svx/xbitmap.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xoutbmp.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif

#include "document.hxx"     // fuer MapMode Initialisierung in PasteDraw
#include "viewfunc.hxx"
#include "tabvwsh.hxx"
#include "drawview.hxx"
#include "scmod.hxx"
#include "drwlayer.hxx"
#include "drwtrans.hxx"
#include "globstr.hrc"

extern Point aDragStartDiff;

// STATIC DATA -----------------------------------------------------------

BOOL bPasteIsMove = FALSE;

using namespace com::sun::star;

//==================================================================

void lcl_AdjustInsertPos( ScViewData* pData, Point& rPos, Size& rSize )
{
//  SdrPage* pPage = pData->GetDocument()->GetDrawLayer()->GetPage( pData->GetTabNo() );
    SdrPage* pPage = pData->GetScDrawView()->GetModel()->GetPage( static_cast<sal_uInt16>(pData->GetTabNo()) );
    DBG_ASSERT(pPage,"pPage ???");
    Size aPgSize( pPage->GetSize() );
    if (aPgSize.Width() < 0)
        aPgSize.Width() = -aPgSize.Width();
    long x = aPgSize.Width() - rPos.X() - rSize.Width();
    long y = aPgSize.Height() - rPos.Y() - rSize.Height();
    // ggf. Ajustments (80/200) fuer Pixel-Rundungsfehler
    if( x < 0 )
        rPos.X() += x + 80;
    if( y < 0 )
        rPos.Y() += y + 200;
    rPos.X() += rSize.Width() / 2;          // Position bei Paste gibt Mittelpunkt an
    rPos.Y() += rSize.Height() / 2;
}

void ScViewFunc::PasteDraw( const Point& rLogicPos, SdrModel* pModel,
        BOOL bGroup, BOOL bSameDocClipboard )
{
    MakeDrawLayer();
    Point aPos( rLogicPos );

    //  #64184# MapMode am Outliner-RefDevice muss stimmen (wie in FuText::MakeOutliner)
    //! mit FuText::MakeOutliner zusammenfassen?
    MapMode aOldMapMode;
    OutputDevice* pRef = GetViewData()->GetDocument()->GetDrawLayer()->GetRefDevice();
    if (pRef)
    {
        aOldMapMode = pRef->GetMapMode();
        pRef->SetMapMode( MapMode(MAP_100TH_MM) );
    }

    BOOL bNegativePage = GetViewData()->GetDocument()->IsNegativePage( GetViewData()->GetTabNo() );

    SdrView* pDragEditView = NULL;
    ScModule* pScMod = SC_MOD();
    const ScDragData& rData = pScMod->GetDragData();
    ScDrawTransferObj* pDrawTrans = rData.pDrawTransfer;
    if (pDrawTrans)
    {
        pDragEditView = pDrawTrans->GetDragSourceView();

        aPos -= aDragStartDiff;
        if ( bNegativePage )
        {
            if (aPos.X() > 0) aPos.X() = 0;
        }
        else
        {
            if (aPos.X() < 0) aPos.X() = 0;
        }
        if (aPos.Y() < 0) aPos.Y() = 0;
    }

    ScDrawView* pDrawView = GetScDrawView();
    if (bGroup)
        pDrawView->BegUndo( ScGlobal::GetRscString( STR_UNDO_PASTE ) );

    BOOL bSameDoc = ( pDragEditView && pDragEditView->GetModel() == pDrawView->GetModel() );
    if (bSameDoc)
    {
            // lokal kopieren - incl. Charts

        Point aSourceStart = pDragEditView->GetAllMarkedRect().TopLeft();
        long nDiffX = aPos.X() - aSourceStart.X();
        long nDiffY = aPos.Y() - aSourceStart.Y();

            // innerhalb einer Page verschieben?

        if ( bPasteIsMove &&
                pDrawView->GetSdrPageView()->GetPage() ==
                pDragEditView->GetSdrPageView()->GetPage() )
        {
            if ( nDiffX != 0 || nDiffY != 0 )
                pDragEditView->MoveAllMarked(Size(nDiffX,nDiffY), FALSE);
        }
        else
        {
            SdrModel* pModel = pDragEditView->GetModel();
            SdrPage* pDestPage = pModel->GetPage( static_cast<sal_uInt16>(GetViewData()->GetTabNo()) );
            DBG_ASSERT(pDestPage,"nanu, Page?");

            SdrMarkList aMark = pDragEditView->GetMarkedObjectList();
            aMark.ForceSort();
            ULONG nMarkAnz=aMark.GetMarkCount();
            for (ULONG nm=0; nm<nMarkAnz; nm++) {
                const SdrMark* pM=aMark.GetMark(nm);
                const SdrObject* pObj=pM->GetMarkedSdrObj();

                // #116235#
                SdrObject* pNeuObj=pObj->Clone();
                //SdrObject* pNeuObj=pObj->Clone(pDestPage,pModel);

                if (pNeuObj!=NULL)
                {
                    pNeuObj->SetModel(pModel);
                    pNeuObj->SetPage(pDestPage);

                    //  #68787# copy graphics within the same model - always needs new name
                    if ( pNeuObj->ISA(SdrGrafObj) && !bPasteIsMove )
                        pNeuObj->SetName(((ScDrawLayer*)pModel)->GetNewGraphicName());

                    if (nDiffX!=0 || nDiffY!=0)
                        pNeuObj->NbcMove(Size(nDiffX,nDiffY));
                    pDestPage->InsertObject( pNeuObj );
                    pDrawView->AddUndo(new SdrUndoInsertObj( *pNeuObj ));

                    //  Chart braucht nicht mehr getrennt behandelt zu werden,
                    //  weil es seine Daten jetzt selber hat
                }
            }

            if (bPasteIsMove)
                pDragEditView->DeleteMarked();
        }
    }
    else
    {
        bPasteIsMove = FALSE;       // kein internes Verschieben passiert

        SdrView aView(pModel);      // #i71529# never create a base class of SdrView directly!
        SdrPageView* pPv = aView.ShowSdrPage(aView.GetModel()->GetPage(0));
        aView.MarkAllObj(pPv);
        Size aSize = aView.GetAllMarkedRect().GetSize();
        lcl_AdjustInsertPos( GetViewData(), aPos, aSize );

        //  #41333# Markierung nicht aendern, wenn Ole-Objekt aktiv
        //  (bei Drop aus Ole-Objekt wuerde sonst mitten im ExecuteDrag deaktiviert!)

        ULONG nOptions = 0;
        SfxInPlaceClient* pClient = GetViewData()->GetViewShell()->GetIPClient();
        if ( pClient && pClient->IsObjectInPlaceActive() )
            nOptions |= SDRINSERT_DONTMARK;

        // #89247# Set flag for ScDocument::UpdateChartListeners() which is
        // called during paste.
        if ( !bSameDocClipboard )
            GetViewData()->GetDocument()->SetPastingDrawFromOtherDoc( TRUE );

        pDrawView->Paste( *pModel, aPos, NULL, nOptions );

        if ( !bSameDocClipboard )
            GetViewData()->GetDocument()->SetPastingDrawFromOtherDoc( FALSE );

        // #68991# Paste puts all objects on the active (front) layer
        // controls must be on SC_LAYER_CONTROLS

        SCTAB nTab = GetViewData()->GetTabNo();
        SdrPage* pPage = pDrawView->GetModel()->GetPage(static_cast<sal_uInt16>(nTab));
        DBG_ASSERT(pPage,"Page?");
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->ISA(SdrUnoObj) && pObject->GetLayer() != SC_LAYER_CONTROLS )
                    pObject->NbcSetLayer(SC_LAYER_CONTROLS);
                pObject = aIter.Next();
            }
        }

        // #75299# all graphics objects must have names
        GetViewData()->GetDocument()->EnsureGraphicNames();
    }

    if (bGroup)
    {
        pDrawView->GroupMarked();
        pDrawView->EndUndo();
    }

    if (pRef)
        pRef->SetMapMode( aOldMapMode );

    // GetViewData()->GetViewShell()->SetDrawShell( TRUE );
    // #99759# It is not sufficient to just set the DrawShell if we pasted, for
    // example, a chart.  SetDrawShellOrSub() would only work for D&D in the
    // same document but not if inserting from the clipboard, therefore
    // MarkListHasChanged() is what we need.
    pDrawView->MarkListHasChanged();

}

BOOL ScViewFunc::PasteObject( const Point& rPos, const uno::Reference < embed::XEmbeddedObject >& xObj,
                                const Size* pDescSize, const Graphic* pReplGraph, const ::rtl::OUString& aMediaType, sal_Int64 nAspect )
{
    MakeDrawLayer();
    if ( xObj.is() )
    {
        ::rtl::OUString aName;
        //TODO/MBA: is that OK?
        comphelper::EmbeddedObjectContainer& aCnt = GetViewData()->GetViewShell()->GetObjectShell()->GetEmbeddedObjectContainer();
        if ( !aCnt.HasEmbeddedObject( xObj ) )
            aCnt.InsertEmbeddedObject( xObj, aName );
        else
            aName = aCnt.GetEmbeddedObjectName( xObj );

        svt::EmbeddedObjectRef aObjRef( xObj, nAspect );
        if ( pReplGraph )
            aObjRef.SetGraphic( *pReplGraph, aMediaType );

        Size aSize;
        if ( nAspect == embed::Aspects::MSOLE_ICON )
        {
            MapMode aMapMode( MAP_100TH_MM );
            aSize = aObjRef.GetSize( &aMapMode );
        }
        else
        {
            // working with visual area can switch object to running state
            MapUnit aMapObj = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
            MapUnit aMap100 = MAP_100TH_MM;

            if ( pDescSize && pDescSize->Width() && pDescSize->Height() )
            {
                // use size from object descriptor if given
                aSize = OutputDevice::LogicToLogic( *pDescSize, aMap100, aMapObj );
                awt::Size aSz;
                aSz.Width = aSize.Width();
                aSz.Height = aSize.Height();
                xObj->setVisualAreaSize( nAspect, aSz );
            }

            awt::Size aSz;
            try
            {
                aSz = xObj->getVisualAreaSize( nAspect );
            }
            catch ( embed::NoVisualAreaSizeException& )
            {
                // the default size will be set later
            }

            aSize = Size( aSz.Width, aSz.Height );
            aSize = OutputDevice::LogicToLogic( aSize, aMapObj, aMap100 );  // fuer SdrOle2Obj

            if( aSize.Height() == 0 || aSize.Width() == 0 )
            {
                DBG_ERROR("SvObjectDescriptor::GetSize == 0");
                aSize.Width() = 5000;
                aSize.Height() = 5000;
                aSize = OutputDevice::LogicToLogic( aSize, aMap100, aMapObj );
                aSz.Width = aSize.Width();
                aSz.Height = aSize.Height();
                xObj->setVisualAreaSize( nAspect, aSz );
            }
        }

        // don't call AdjustInsertPos
        Point aInsPos = rPos;
        if ( GetViewData()->GetDocument()->IsNegativePage( GetViewData()->GetTabNo() ) )
            aInsPos.X() -= aSize.Width();
        Rectangle aRect( aInsPos, aSize );

        ScDrawView* pDrView = GetScDrawView();
        SdrOle2Obj* pSdrObj = new SdrOle2Obj( aObjRef, aName, aRect );

        SdrPageView* pPV = pDrView->GetSdrPageView();
        pDrView->InsertObjectSafe( pSdrObj, *pPV );             // nicht markieren wenn Ole
        GetViewData()->GetViewShell()->SetDrawShell( TRUE );
        return TRUE;
    }
    else
        return FALSE;
}

BOOL ScViewFunc::PasteBitmap( const Point& rPos, const Bitmap& rBmp )
{
    String aEmpty;
    Graphic aGraphic(rBmp);
    return PasteGraphic( rPos, aGraphic, aEmpty, aEmpty );
}

BOOL ScViewFunc::PasteMetaFile( const Point& rPos, const GDIMetaFile& rMtf )
{
    String aEmpty;
    Graphic aGraphic(rMtf);
    return PasteGraphic( rPos, aGraphic, aEmpty, aEmpty );
}

BOOL ScViewFunc::PasteGraphic( const Point& rPos, const Graphic& rGraphic,
                                const String& rFile, const String& rFilter )
{
    MakeDrawLayer();
    ScDrawView* pDrawView = GetScDrawView();

    Point aPos( rPos );
    Window* pWin = GetActiveWin();
    MapMode aSourceMap = rGraphic.GetPrefMapMode();
    MapMode aDestMap( MAP_100TH_MM );

    if (aSourceMap.GetMapUnit() == MAP_PIXEL)
    {
            //  Pixel-Korrektur beruecksichtigen, damit Bitmap auf dem Bildschirm stimmt

        Fraction aScaleX, aScaleY;
        pDrawView->CalcNormScale( aScaleX, aScaleY );
        aDestMap.SetScaleX(aScaleX);
        aDestMap.SetScaleY(aScaleY);
    }

    Size aSize = pWin->LogicToLogic( rGraphic.GetPrefSize(), &aSourceMap, &aDestMap );
//  lcl_AdjustInsertPos( GetViewData(), aPos, aSize );
    if ( GetViewData()->GetDocument()->IsNegativePage( GetViewData()->GetTabNo() ) )
        aPos.X() -= aSize.Width();

    GetViewData()->GetViewShell()->SetDrawShell( TRUE );

    Rectangle aRect(aPos, aSize);
    SdrGrafObj* pGrafObj = new SdrGrafObj(rGraphic, aRect);

    // #118522# calling SetGraphicLink here doesn't work

    //  #49961# Pfad wird nicht mehr als Name der Grafik gesetzt

    ScDrawLayer* pLayer = (ScDrawLayer*) pDrawView->GetModel();
    String aName = pLayer->GetNewGraphicName();                 // "Grafik x"
    pGrafObj->SetName(aName);

    // nicht markieren wenn Ole
    pDrawView->InsertObjectSafe(pGrafObj, *pDrawView->GetSdrPageView());

    // #118522# SetGraphicLink has to be used after inserting the object,
    // otherwise an empty graphic is swapped in and the contact stuff crashes.
    // See #i37444#.
    if (rFile.Len())
        pGrafObj->SetGraphicLink( rFile, rFilter );

    return TRUE;
}

BOOL ScViewFunc::ApplyGraphicToObject( SdrObject* pPickObj, const Graphic& rGraphic )
{
    BOOL bRet = FALSE;
    SdrGrafObj* pNewGrafObj = NULL;

    ScDrawView* pDrawView = GetScDrawView();
    if ( pDrawView && pPickObj )
    {
        /**********************************************************************
        * Objekt neu attributieren
        **********************************************************************/
        SdrPageView* pPV = pDrawView->GetSdrPageView();
        if (pPickObj->ISA(SdrGrafObj))
        {
            /******************************************************************
            * Das Graphik-Objekt bekommt eine neue Graphik
            ******************************************************************/
            pNewGrafObj = (SdrGrafObj*) pPickObj->Clone();
            pNewGrafObj->SetGraphic(rGraphic);

            pDrawView->BegUndo(ScGlobal::GetRscString(STR_UNDO_DRAGDROP));
            pDrawView->ReplaceObjectAtView(pPickObj, *pPV, pNewGrafObj);
            pDrawView->EndUndo();

            bRet = TRUE;
        }
        else if (pPickObj->IsClosedObj() && !pPickObj->ISA(SdrOle2Obj))
        {
            /******************************************************************
            * Das Objekt wird mit der Graphik gefuellt
            ******************************************************************/
            //pDrawView->BegUndo(ScGlobal::GetRscString(STR_UNDO_DRAGDROP));
            pDrawView->AddUndo(new SdrUndoAttrObj(*pPickObj));
            //pDrawView->EndUndo();

            XOBitmap aXOBitmap( rGraphic.GetBitmap() );
            SfxItemSet aSet( pDrawView->GetModel()->GetItemPool(),
                                XATTR_FILLSTYLE, XATTR_FILLBITMAP );
            aSet.Put(XFillStyleItem(XFILL_BITMAP));
            aSet.Put(XFillBitmapItem(String(), aXOBitmap));

            pPickObj->SetMergedItemSetAndBroadcast(aSet);

            bRet = TRUE;
        }
    }
    return bRet;
}


