/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: polysc3d.cxx,v $
 * $Revision: 1.17 $
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
#include "precompiled_svx.hxx"
#include <svtools/options3d.hxx>
#include <svx/xfillit.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdogrp.hxx>
#include "svditer.hxx"
#include <svx/svdetc.hxx>

#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif
#include <vcl/svapp.hxx>
#include <svx/xoutx.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpool.hxx>
#include <svtools/style.hxx>
#include "globl3d.hxx"
#include <svx/polysc3d.hxx>
#include <goodies/base3d.hxx>
#include <goodies/b3dtex.hxx>
#include <svx/xlnclit.hxx>
#include <svtools/metitem.hxx>
#include <svx/xtable.hxx>
#include <svx/xlnwtit.hxx>

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1(E3dPolyScene, E3dScene);

E3dPolyScene::E3dPolyScene()
:   E3dScene()
{
}

E3dPolyScene::E3dPolyScene(E3dDefaultAttributes& rDefault)
:   E3dScene(rDefault)
{
}

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

UINT16 E3dPolyScene::GetObjIdentifier() const
{
    return E3D_POLYSCENE_ID;
}

/*************************************************************************
|*
|* Die Kontur fuer TextToContour
|*
\************************************************************************/

basegfx::B2DPolyPolygon E3dPolyScene::TakeContour() const
{
    return ImpTakeContour3D();
}

/*************************************************************************
|*
|* Objekt als Kontur in das Polygon einfuegen
|*
\************************************************************************/

basegfx::B2DPolyPolygon E3dPolyScene::ImpTakeContour3D() const
{
    // TransformationSet vorbereiten
    ((E3dPolyScene*)this)->InitTransformationSet();

    // Holen
    return E3dScene::ImpTakeContour3D();
}

/*************************************************************************
|*
|* Paint
|*
\************************************************************************/

sal_Bool E3dPolyScene::DoPaintObject(XOutputDevice& rOut,   const SdrPaintInfoRec& rInfoRec) const
{
    bool bOk=true;
    if(GetSubList() && GetSubList()->GetObjCount())
    {
        bOk = ((E3dPolyScene*)this)->LocalPaint3D(rOut, rInfoRec);
    }

    return bOk;
}

BOOL E3dPolyScene::LocalPaint3D(XOutputDevice& rOut,
    const SdrPaintInfoRec& rInfoRec)
{
    Time aStartTime;
    Rectangle aBound(GetSnapRect());
    OutputDevice* pOut = rOut.GetOutDev();

    sal_uInt16 nShadeMode = GetShadeMode();
    Base3D *pBase3D = Base3D::Create(pOut, nShadeMode > 2);

    DBG_ASSERT(pBase3D, "Habe keinen Base3D Kontext bekommen!!");
    pBase3D->SetDisplayQuality(GetDisplayQuality());

    // #i71618#
    // copy local PolygonOffset to base3d
    pBase3D->setPolygonOffset(getPolygonOffset());

    if(nShadeMode == 0)
        pBase3D->SetShadeModel(Base3DFlat);
    else if(nShadeMode == 1)
        pBase3D->SetShadeModel(Base3DPhong);
    else
        pBase3D->SetShadeModel(Base3DSmooth);

    Rectangle aVisible(Point(0,0), pOut->GetOutputSizePixel());
    aVisible = pOut->PixelToLogic(aVisible);

    // #101867# If rendering to metafile
    if(pOut && pOut->GetConnectMetaFile() && !pOut->IsOutputEnabled())
    {
        aVisible = aBound;
    }

    // Feststellen, ob transparente Teile enthalten sind
    pBase3D->SetTransparentPartsContainedHint(AreThereTransparentParts());

    // eventuell ScissorRegion setzen
    pBase3D->ActivateScissorRegion(FALSE);
    if(aBound != aVisible)
    {
        Rectangle aClipBound = aBound.GetIntersection(aVisible);
        if(aClipBound != aBound)
        {
            pBase3D->SetScissorRegion(aClipBound);

#ifdef DBG_UTIL     // draw aClipBound for testing
            static BOOL bDoDrawClipBoundForTesting(FALSE);
            if(bDoDrawClipBoundForTesting)
            {
                OutputDevice* pTempOut = rOut.GetOutDev();

                // red
                pTempOut->SetLineColor(Color(COL_RED));
                pTempOut->SetFillColor();
                pTempOut->DrawRect(aClipBound);

                Rectangle aClipBoundPixel = pTempOut->LogicToPixel(aClipBound);
                BOOL bWasEnabled = pTempOut->IsMapModeEnabled();
                pTempOut->EnableMapMode(FALSE);

                // GREEN
                pTempOut->SetLineColor(Color(COL_GREEN));
                pTempOut->SetFillColor();
                pTempOut->DrawRect(aClipBoundPixel);

                pTempOut->EnableMapMode(bWasEnabled);
            }
#endif
        }
    }

    // GeometricSet reset und mit pBase3D assoziieren
    B3dCamera& rSet = GetCameraSet();
    pBase3D->SetTransformationSet(&rSet);

    // Dithering
    pBase3D->SetDither(GetDither() && SvtOptions3D().IsDithering());

    // Licht setzen, vor Object->World setzen, um Lichter im
    // WKS zu positionieren
    pBase3D->SetLightGroup(&(GetLightGroup()));

    // Transformation initialisieren
    InitTransformationSet();

    // Viewport-Grenzen eintragen
    rSet.SetViewportRectangle(aBound, aVisible);

    // Matritzen aktualisieren (OpenGL)
    pBase3D->SetTransformationSet(&rSet);

    if(!pBase3D->GetDisplayQuality())
    {
        // Extrem verminderte Darstellungsqualitaet, zeichne als
        // WireFrame, voellig OHNE renderer
        DrawWireframe(pBase3D, rOut);
    }
    else
    {
        // Schatten zeichnen
        if(pBase3D->GetDisplayQuality() > 128)
        {
            Volume3D aVolume(rSet.GetDeviceVolume());
            DrawAllShadows(pBase3D, rOut, aBound, aVolume, rInfoRec);
        }

        // Szene mit clipping zeichnen
        DrawPolySceneClip(rOut, this, pBase3D, rInfoRec);
    }

    // Labels darueber zeichnen. Da es auch Wertebeschriftungen
    // im Chart gibt, ist es besser, die Labels hinterher zu zeichnen.
    // Leider kann es auch dabei zu Zeichenfehlern kommen. Die
    // LabelObjekte muessen irgendwann mal auf echte 3D-Objekte
    // umgestellt werden.
    SdrObjList* pSubList = GetSubList();
    if(pSubList)
    {
        SdrObjListIter a3DIterator(*pSubList, IM_DEEPWITHGROUPS);
        while ( a3DIterator.IsMore() )
        {
            E3dObject* pObj = (E3dObject*) a3DIterator.Next();
            DBG_ASSERT(pObj->ISA(E3dObject), "In Szenen sind nur 3D-Objekte erlaubt!");

            if(pObj->ISA(E3dLabelObj))
            {
                E3dLabelObj* pLabelObject = (E3dLabelObj*)pObj;
                SdrObject* pLabel = (SdrObject*)pLabelObject->Get2DLabelObj();
                const basegfx::B3DPoint aPos(rSet.WorldToViewCoor(pLabelObject->GetTransPosition()));
                Point a2DPos((long)(aPos.getX() + 0.5), (long)(aPos.getY() + 0.5));
                pLabel->NbcSetAnchorPos(a2DPos);
                pLabel->SingleObjectPainter(rOut, rInfoRec); // #110094#-17
            }
        }
    }

    // Zeichenzeit bestimmen
    aPaintTime = Time().GetTime() - aStartTime.GetTime();

    return TRUE;
}

/*************************************************************************
|*
|* Geometrie zeichnen mit clipping Beruecksichtigung
|*
\************************************************************************/

void E3dPolyScene::DrawPolySceneClip(XOutputDevice& rOut,
    const E3dObject* /*p3DObj*/, Base3D* pBase3D, const SdrPaintInfoRec& rInfoRec)
{
    // spezielles Clipping fuer OpenGL, um keine floating windows ueberzumalen
    OutputDevice* pOut = rOut.GetOutDev();

    if(pBase3D->GetBase3DType() == BASE3D_TYPE_OPENGL
        && pOut->GetOutDevType() == OUTDEV_WINDOW
        && pBase3D->GetTransformationSet())
    {
        Window* pWin = (Window*)pOut;
        RegionHandle aRegionHandle;
        Rectangle aClipRect;
        Rectangle aSystemClipRect;
        BOOL bClippingWasActive(FALSE);
        Region aClipRegion;

#ifdef DBG_UTIL     // ClipRegions zum testen zeichnen
        static BOOL bDoDrawRegionsToMakeThemVisible = FALSE;
        if(bDoDrawRegionsToMakeThemVisible)
        {
            // Rot
            aClipRegion = pWin->GetWindowClipRegionPixel();

            // Trick von TH, um das Umrechnen auf logische Koordinaten zu vermeiden
            BOOL bMapModeWasSet = pWin->IsMapModeEnabled();
            pWin->EnableMapMode(FALSE);

            aClipRegion.Intersect(pWin->GetActiveClipRegion());
            pWin->EnableMapMode(bMapModeWasSet);

            aRegionHandle = aClipRegion.BeginEnumRects();

            while(aClipRegion.GetEnumRects(aRegionHandle, aClipRect))
            {
                pOut->SetLineColor(Color(COL_RED));
                pOut->SetFillColor();
                pOut->DrawRect(aClipRect);
            }
            aClipRegion.EndEnumRects(aRegionHandle);
        }
#endif

        // Eventuell bereits gesetzte ClipRegion sichern und zur Verwendung
        // vorbereiten
        if(pBase3D->IsScissorRegionActive())
        {
            aSystemClipRect = pBase3D->GetScissorRegionPixel();
            bClippingWasActive = TRUE;
        }

        // ClipRegion holen und Durchlauf vorbereiten
        aClipRegion = pWin->GetWindowClipRegionPixel();

        // Trick von TH, um das Umrechnen auf logische Koordinaten zu vermeiden
        BOOL bMapModeWasSet = pWin->IsMapModeEnabled();
        pWin->EnableMapMode(FALSE);

        aClipRegion.Intersect(pWin->GetActiveClipRegion());
        pWin->EnableMapMode(bMapModeWasSet);

        aRegionHandle = aClipRegion.BeginEnumRects();
        Rectangle aBase3DRect = pWin->LogicToPixel(
            pBase3D->GetTransformationSet()->GetLogicalViewportBounds());

        // Lauf ueber die ClipRegions
        while(aClipRegion.GetEnumRects(aRegionHandle, aClipRect))
        {
            if(aClipRect.IsOver(aBase3DRect))
            {
                // ClipRegion setzen
                if(bClippingWasActive)
                {
                    pBase3D->SetScissorRegionPixel(aClipRect.GetIntersection(aSystemClipRect), TRUE);
                }
                else
                {
                    pBase3D->SetScissorRegionPixel(aClipRect, TRUE);
                }

                // Beginn szene
                pBase3D->StartScene();

                // Ausgabe aller 3D Objekte
                Paint3D(rOut, pBase3D, rInfoRec, 0);

                // Ende der Szene
                UINT32 nWasDrawMode = rOut.GetOutDev()->GetDrawMode();
                rOut.GetOutDev()->SetDrawMode(rInfoRec.nOriginalDrawMode);
                pBase3D->EndScene();
                rOut.GetOutDev()->SetDrawMode(nWasDrawMode);
            }
        }
        aClipRegion.EndEnumRects(aRegionHandle);
    }
    else
    {
        // Ohne clipping ausgeben
        // Beginn szene
        pBase3D->StartScene();

        // Ausgabe aller 3D Objekte
        ((E3dPolyScene *)this)->Paint3D(rOut, pBase3D, rInfoRec, 0);

        // Ende der Szene
        UINT32 nWasDrawMode = rOut.GetOutDev()->GetDrawMode();
        rOut.GetOutDev()->SetDrawMode(rInfoRec.nOriginalDrawMode);
        pBase3D->EndScene();
        rOut.GetOutDev()->SetDrawMode(nWasDrawMode);
    }
}

/*************************************************************************
|*
|* Zeichenroutine fuer 3D
|*
\************************************************************************/

void E3dPolyScene::Paint3D(XOutputDevice& rOut, Base3D* pBase3D,
    const SdrPaintInfoRec& rInfoRec, UINT16 nDrawFlags)
{
    if(GetScene() != this)
    {
        // Szene in einer Szene, durchreichen
        E3dObject::Paint3D(rOut, pBase3D, rInfoRec, nDrawFlags);
    }
    else
    {
        // Dies ist die Root-Szene
        if(pBase3D->GetTransparentPartsContainedHint())
        {
            // Nicht-Transparente Flaechen
            E3dObject::Paint3D(rOut, pBase3D, rInfoRec, E3D_DRAWFLAG_FILLED);

            // Nicht-Transparente Linien
            E3dObject::Paint3D(rOut, pBase3D, rInfoRec, E3D_DRAWFLAG_OUTLINE);

            // Transparente Flaechen
            E3dObject::Paint3D(rOut, pBase3D, rInfoRec, E3D_DRAWFLAG_FILLED|E3D_DRAWFLAG_TRANSPARENT);

            // Transparente Linien
            E3dObject::Paint3D(rOut, pBase3D, rInfoRec, E3D_DRAWFLAG_OUTLINE|E3D_DRAWFLAG_TRANSPARENT);
        }
        else
        {
            // call parent
            E3dObject::Paint3D(rOut, pBase3D, rInfoRec, E3D_DRAWFLAG_FILLED|E3D_DRAWFLAG_OUTLINE);
        }
    }
}

/*************************************************************************
|*
|* Schatten aller Objekte zeichnen
|*
\************************************************************************/

void E3dPolyScene::DrawAllShadows(Base3D* pBase3D, XOutputDevice& rXOut,
    const Rectangle& rBound, const Volume3D& rVolume,
    const SdrPaintInfoRec& rInfoRec)
{
    // TransformationSet vorbereiten
    InitTransformationSet();

    // Schatten Zeichnen
    E3dScene::DrawShadows(pBase3D, rXOut, rBound, rVolume, rInfoRec);
}

/*************************************************************************
|*
|* Verminderte Darstellungsqualitaet, zeichne als WireFrame OHNE renderer
|*
\************************************************************************/

void E3dPolyScene::DrawWireframe(Base3D* pBase3D, XOutputDevice& rXOut)
{
    // Farben setzen
    rXOut.GetOutDev()->SetLineColor(Color(COL_BLACK));
    rXOut.GetOutDev()->SetFillColor();

    // Unterobjekte darstellen
    SdrObjList* pSubList = GetSubList();
    if(pSubList)
    {
        SdrObjListIter a3DIterator(*pSubList, IM_DEEPWITHGROUPS);
        while ( a3DIterator.IsMore() )
        {
            E3dObject* pObj = (E3dObject*) a3DIterator.Next();
            DBG_ASSERT(pObj->ISA(E3dObject), "In Szenen sind nur 3D-Objekte erlaubt!");
            if(pObj->ISA(E3dCompoundObject))
            {
                E3dCompoundObject* pCompObj = (E3dCompoundObject*)pObj;

                // ObjectTrans setzen
                basegfx::B3DHomMatrix mTransform = pCompObj->GetFullTransform();
                GetCameraSet().SetObjectTrans(mTransform);
                pBase3D->SetTransformationSet(&(GetScene()->GetCameraSet()));

                // WireFrame eines einzelnen Objektes
                pCompObj->DrawObjectWireframe(rXOut);
            }
        }
    }
}


