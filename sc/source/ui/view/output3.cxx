/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: output3.cxx,v $
 * $Revision: 1.27 $
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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <svx/eeitem.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/xoutx.hxx>
#include <vcl/svapp.hxx>

#include "output.hxx"
#include "drwlayer.hxx"
#include "document.hxx"
#include "tabvwsh.hxx"
#include "fillinfo.hxx"


#ifdef IRIX
#include <basic/sbx.hxx>
#endif
#include <svx/fmview.hxx>

// STATIC DATA -----------------------------------------------------------

SdrObject* pSkipPaintObj = NULL;

//==================================================================

// #i72502#
Point ScOutputData::PrePrintDrawingLayer(long nLogStX, long nLogStY )
{
    Rectangle aRect;
    SCCOL nCol;
    Point aOffset;
    long nLayoutSign(bLayoutRTL ? -1 : 1);

    for (nCol=0; nCol<nX1; nCol++)
        aOffset.X() -= pDoc->GetColWidth( nCol, nTab ) * nLayoutSign;
    aOffset.Y() -= pDoc->GetRowHeight( 0, nY1-1, nTab );

    long nDataWidth = 0;
    long nDataHeight = 0;
    for (nCol=nX1; nCol<=nX2; nCol++)
        nDataWidth += pDoc->GetColWidth( nCol, nTab );
    nDataHeight += pDoc->GetRowHeight( nY1, nY2, nTab );

    if ( bLayoutRTL )
        aOffset.X() += nDataWidth;

    aRect.Left() = aRect.Right()  = -aOffset.X();
    aRect.Top()  = aRect.Bottom() = -aOffset.Y();

    Point aMMOffset( aOffset );
    aMMOffset.X() = (long)(aMMOffset.X() * HMM_PER_TWIPS);
    aMMOffset.Y() = (long)(aMMOffset.Y() * HMM_PER_TWIPS);

    if (!bMetaFile)
        aMMOffset += Point( nLogStX, nLogStY );

    for (nCol=nX1; nCol<=nX2; nCol++)
        aRect.Right() += pDoc->GetColWidth( nCol, nTab );
    aRect.Bottom() += pDoc->GetRowHeight( nY1, nY2, nTab );

    aRect.Left()   = (long) (aRect.Left()   * HMM_PER_TWIPS);
    aRect.Top()    = (long) (aRect.Top()    * HMM_PER_TWIPS);
    aRect.Right()  = (long) (aRect.Right()  * HMM_PER_TWIPS);
    aRect.Bottom() = (long) (aRect.Bottom() * HMM_PER_TWIPS);

    if(pViewShell || pDrawView)
    {
        SdrView* pLocalDrawView = (pDrawView) ? pDrawView : pViewShell->GetSdrView();

        if(pLocalDrawView)
        {
            // #i76114# MapMode has to be set because BeginDrawLayers uses GetPaintRegion
            MapMode aOldMode = pDev->GetMapMode();
            if (!bMetaFile)
                pDev->SetMapMode( MapMode( MAP_100TH_MM, aMMOffset, aOldMode.GetScaleX(), aOldMode.GetScaleY() ) );

            // #i74769# work with SdrPaintWindow directly
            // #i76114# pass bDisableIntersect = true, because the intersection of the table area
            // with the Window's paint region can be empty
            Region aRectRegion(aRect);
            mpTargetPaintWindow = pLocalDrawView->BeginDrawLayers(pDev, aRectRegion, true);
            OSL_ENSURE(mpTargetPaintWindow, "BeginDrawLayers: Got no SdrPaintWindow (!)");

            if (!bMetaFile)
                pDev->SetMapMode( aOldMode );
        }
    }

    return aMMOffset;
}

// #i72502#
void ScOutputData::PostPrintDrawingLayer(const Point& rMMOffset) // #i74768#
{
    // #i74768# just use offset as in PrintDrawingLayer() to also get the form controls
    // painted with offset
    MapMode aOldMode = pDev->GetMapMode();

    if (!bMetaFile)
    {
        pDev->SetMapMode( MapMode( MAP_100TH_MM, rMMOffset, aOldMode.GetScaleX(), aOldMode.GetScaleY() ) );
    }

    if(pViewShell || pDrawView)
    {
        SdrView* pLocalDrawView = (pDrawView) ? pDrawView : pViewShell->GetSdrView();

        if(pLocalDrawView)
        {
            // #i74769# work with SdrPaintWindow directly
            pLocalDrawView->EndDrawLayers(*mpTargetPaintWindow);
            mpTargetPaintWindow = 0;
        }
    }

    // #i74768#
    if (!bMetaFile)
    {
        pDev->SetMapMode( aOldMode );
    }
}

// #i72502#
void ScOutputData::PrintDrawingLayer(const sal_uInt16 nLayer, const sal_uInt16 nPaintMode, const Point& rMMOffset)
{
    // #109985#
    const sal_uInt16 nPaintModeHideAll(SDRPAINTMODE_SC_HIDE_OLE|SDRPAINTMODE_SC_HIDE_CHART|SDRPAINTMODE_SC_HIDE_DRAW);

    // #109985#
    if((nPaintModeHideAll == nPaintMode) || (!pDoc->GetDrawLayer()))
    {
        return;
    }

    MapMode aOldMode = pDev->GetMapMode();

    if (!bMetaFile)
    {
        pDev->SetMapMode( MapMode( MAP_100TH_MM, rMMOffset, aOldMode.GetScaleX(), aOldMode.GetScaleY() ) );
    }

    // #109985#
    DrawSelectiveObjects( nLayer, nPaintMode);

    if (!bMetaFile)
    {
        pDev->SetMapMode( aOldMode );
    }
}

// #109985#
void ScOutputData::DrawSelectiveObjects(const sal_uInt16 nLayer, const sal_uInt16 nPaintMode)
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return;

    //  #i46362# high contrast mode (and default text direction) must be handled
    //  by the application, so it's still needed when using DrawLayer().

    SdrOutliner& rOutl = pModel->GetDrawOutliner();
    rOutl.EnableAutoColor( bUseStyleColor );
    rOutl.SetDefaultHorizontalTextDirection(
                (EEHorizontalTextDirection)pDoc->GetEditTextDirection( nTab ) );

    //  #i69767# The hyphenator must be set (used to be before drawing a text shape with hyphenation).
    //  LinguMgr::GetHyphenator (EditEngine) uses a wrapper now that creates the real hyphenator on demand,
    //  so it's not a performance problem to call UseHyphenator even when it's not needed.

    pModel->UseHyphenator();

    ULONG nOldDrawMode = pDev->GetDrawMode();
    if ( bUseStyleColor && Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        pDev->SetDrawMode( nOldDrawMode | DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL |
                            DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT );
    }

    // #109985#
    if(pViewShell || pDrawView)
    {
        SdrView* pLocalDrawView = (pDrawView) ? pDrawView : pViewShell->GetSdrView();

        if(pLocalDrawView)
        {
            SdrPageView* pPageView = pLocalDrawView->GetSdrPageView();

            if(pPageView)
            {
                // Region aDrawRegion(rRect);
                // pPageView->DrawLayer(nLayer, aDrawRegion, pDev, nPaintMode);
                pPageView->DrawLayer(sal::static_int_cast<SdrLayerID>(nLayer), pDev, nPaintMode);
            }
        }
    }

    pDev->SetDrawMode(nOldDrawMode);

    // #109985#
    return;

/*  SdrOutliner& rOutl = pModel->GetDrawOutliner();
    rOutl.EnableAutoColor( bUseStyleColor );
    rOutl.SetDefaultHorizontalTextDirection(
                (EEHorizontalTextDirection)pDoc->GetEditTextDirection( nTab ) );

    XOutputDevice* pXOut = new XOutputDevice( pDev );
    pXOut->SetOutDev( pDev );
    SdrPaintInfoRec aInfoRec;

    if ( pViewShell )
    {
        SdrView* pDrawView = pViewShell->GetSdrView();
        if (pDrawView)
            aInfoRec.pPV = pDrawView->GetPageViewByIndex(0);
    }

    BOOL bDidDummy = FALSE;

    SdrPage* pPage = pModel->GetPage(nTab);
    DBG_ASSERT(pPage,"Page nicht gefunden");
    if (!pPage)
        return;

    if ( aInfoRec.pPV && aInfoRec.pPV->GetObjList() == pPage )
        aInfoRec.bNotActive = FALSE;

    //  handle high contrast draw modes in addition to the group draw modes,
    //  aInfoRec.nOriginalDrawMode must include the high contrast bits if used
    ULONG nOldDrawMode = pDev->GetDrawMode();
    if ( bUseStyleColor && Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        pDev->SetDrawMode( nOldDrawMode | DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL |
                            DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT );
    }

    //  DrawMode handling copied from SdrObjList::Paint
    UINT32 nWasDrawMode = pDev->GetDrawMode();
    if(!aInfoRec.bOriginalDrawModeSet)
    {
        // Original-Paintmode retten
        aInfoRec.bOriginalDrawModeSet = TRUE;
        aInfoRec.nOriginalDrawMode = pDev->GetDrawMode();
    }
    if(aInfoRec.pPV && aInfoRec.bNotActive)
    {
        if(aInfoRec.pPV->GetView().DoVisualizeEnteredGroup())
        {
            // Darstellung schmal
            pDev->SetDrawMode(nWasDrawMode | (
                DRAWMODE_GHOSTEDLINE|DRAWMODE_GHOSTEDFILL|DRAWMODE_GHOSTEDTEXT|DRAWMODE_GHOSTEDBITMAP|DRAWMODE_GHOSTEDGRADIENT));
        }
    }
    else
    {
        // Darstellung normal
        pDev->SetDrawMode(aInfoRec.nOriginalDrawMode);
    }

    //  Paint ueber SdrObjListIter geht bei 3D-Objekten schief

    ULONG nObjCount = pPage->GetObjCount();
    for (ULONG nObjNum = 0; nObjNum<nObjCount; nObjNum++)
    {
        SdrObject* pObject = pPage->GetObj(nObjNum);

        if ( pObject->GetLayer()==(SdrLayerID) nLayer && pObject != pSkipPaintObj )
        {
            // #62107# nur das anfassen, was ueberhaupt gepainted werden koennte,
            // damit nicht unnoetig durch Cache-Miss nachgeladen wird

            Rectangle aObjRect = pObject->GetBoundRect();
            if ( aObjRect.IsOver( rRect ) )
            {
                BOOL bDraw;
                BOOL bDummy;
                UINT16 nSdrObjKind = pObject->GetObjIdentifier();
                if (nSdrObjKind == OBJ_OLE2)
                {
                    if ( pDoc->IsChart( pObject ) )
                    {
                        bDraw = (nObjectFlags & SC_OBJECTS_CHARTS) != 0;
                        bDummy = (nDummyFlags & SC_OBJECTS_CHARTS) != 0;
                    }
                    else
                    {
                        bDraw = (nObjectFlags & SC_OBJECTS_OLE) != 0;
                        bDummy = (nDummyFlags & SC_OBJECTS_OLE) != 0;
                    }
                }
                else if (nSdrObjKind == OBJ_GRAF)       // Grafiken zusammen mit Ole-Objekten
                {
                    bDraw = (nObjectFlags & SC_OBJECTS_OLE) != 0;
                    bDummy = (nDummyFlags & SC_OBJECTS_OLE) != 0;
                }
                else
                {
                    bDraw = (nObjectFlags & SC_OBJECTS_DRAWING) != 0;
                    bDummy = (nDummyFlags & SC_OBJECTS_DRAWING) != 0;
                }

                if (bDraw || bDummy)
                {
                    BOOL bClip = !rRect.IsInside( aObjRect );
                    BOOL bPush = FALSE;

                    if (bClip && bDraw && nSdrObjKind == OBJ_GRAF)
                    {
                        //  #80136# don't call GetGraphic here
                        //  (would swap the graphic in and reschedule)
                        //  GetGraphicType / IsAnimated don't swap in

                        SdrGrafObj* pGrafObj = (SdrGrafObj*)pObject;
                        GraphicType eType = pGrafObj->GetGraphicType();
                        if ( eType == GRAPHIC_GDIMETAFILE || pGrafObj->IsAnimated() )
                            bClip = FALSE;
                    }

                    //  #36427#/#37790#: VC-Objekte pfuschen mit den ClipRegions rum,
                    //  darum fuer Metafiles Push/Pop aussenherum.
                    //! Hier wieder raus, wenn die VC-Objekte richtig funktionieren!
//                  if ( bMetaFile && bDraw && !bClip && pObject->ISA(VCSbxDrawObject) )
//                      bPush = TRUE;

                    if (bClip)
                    {
                        if (bMetaFile)
                        {
                            pDev->Push();
                            pDev->IntersectClipRegion( rRect );
                        }
                        else
                        {
                            if ( nLayer != SC_LAYER_BACK )
                            {   // #29660# HACK:
                                // untere Gitterlinie blieb manchmal stehen,
                                // anscheinend Rundungsfehler in ClipRegion
                                // vom Grafiktreiber
                                Rectangle aPix = pDev->LogicToPixel(rRect);
                                aPix.Bottom() += 1;
                                Rectangle aNew = pDev->PixelToLogic(aPix);
                                pDev->SetClipRegion( aNew );
                            }
                            else
                                pDev->SetClipRegion( rRect );
                        }
                    }
                    else if (bPush)         // nur Push/Pop
                        pDev->Push();

                    if (bDraw)
                    {
                        //  set model's hyphenator on demand
                        if ( ((const SfxBoolItem&)pObject->GetItemSet().Get(EE_PARA_HYPHENATE)).GetValue() )
                            pModel->UseHyphenator();

                        if (pObject == pEditObj)
                        {
                            aInfoRec.nPaintMode|=SDRPAINTMODE_TEXTEDIT;
                            pObject->Paint( *pXOut, aInfoRec );
                            aInfoRec.nPaintMode&=~SDRPAINTMODE_TEXTEDIT;
                        }
                        else
                            pObject->Paint( *pXOut, aInfoRec );
                        bDidDummy = FALSE;

                            // Plugins connecten, wenn sichtbar:

                        if ( pViewShell && pObject->ISA(SdrOle2Obj) )
                        {
                            SdrOle2Obj* pOleObj = (SdrOle2Obj*)pObject;
                            ConnectObject( pOleObj->GetObjRef(), pOleObj );
                        }
                    }
                    else
                    {
                        if (!bDidDummy)
                        {
                            pDev->SetFillColor(COL_LIGHTGRAY);
                            pDev->SetLineColor(COL_BLACK);
                            bDidDummy = TRUE;
                        }
                        pDev->DrawRect( aObjRect );
                    }

                    if (bClip || bPush)     // bei bPush ist auch bMetaFile=TRUE
                    {
                        if (bMetaFile)
                            pDev->Pop();
                        else
                            pDev->SetClipRegion();
                    }
                }
            }
        }
    }

//  pDev->SetDrawMode(aInfoRec.nOriginalDrawMode);
    pDev->SetDrawMode(nOldDrawMode);

    delete pXOut;
    */
}

//  Teile nur fuer Bildschirm

// #109985#
void ScOutputData::DrawingSingle(const sal_uInt16 nLayer, const sal_uInt16 nPaintMode)
{
    BOOL    bHad    = FALSE;
    long    nPosY   = nScrY;
    SCSIZE  nArrY;
    for (nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];

        if ( pThisRowInfo->bChanged )
        {
            if (!bHad)
            {
                bHad = TRUE;
            }
        }
        else if (bHad)
        {
            DrawSelectiveObjects( nLayer, nPaintMode);
            bHad = FALSE;
        }
        nPosY += pRowInfo[nArrY].nHeight;
    }

    if (bHad)
        DrawSelectiveObjects( nLayer, nPaintMode);
}




