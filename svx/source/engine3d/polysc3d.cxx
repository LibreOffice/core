/*************************************************************************
 *
 *  $RCSfile: polysc3d.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-10 15:21:31 $
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

#ifndef INCLUDED_SVTOOLS_OPTIONS3D_HXX
#include <svtools/options3d.hxx>
#endif

#ifndef _SVX_FILLITEM_HXX
#include "xfillit.hxx"
#endif

#ifndef _SVDOPATH_HXX //autogen
#include "svdopath.hxx"
#endif

#ifndef _SVDOGRP_HXX //autogen
#include "svdogrp.hxx"
#endif

#ifndef _SVDITER_HXX //autogen
#include "svditer.hxx"
#endif

#ifndef _SVDETC_HXX //autogen
#include "svdetc.hxx"
#endif

#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _XOUTX_HXX
#include "xoutx.hxx"
#endif

#ifndef _SVDPAGE_HXX
#include "svdpage.hxx"
#endif

#ifndef _SVDPOOL_HXX
#include "svdpool.hxx"
#endif

#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif

#ifndef _E3D_GLOBL3D_HXX
#include "globl3d.hxx"
#endif

#ifndef _E3D_POLYSC3D_HXX
#include "polysc3d.hxx"
#endif

#ifndef _E3D_PLIGHT3D_HXX
#include "plight3d.hxx"
#endif

#ifndef _E3D_DLIGHT3D_HXX
#include "dlight3d.hxx"
#endif

#ifndef _B3D_BASE3D_HXX
#include <goodies/base3d.hxx>
#endif

#ifndef _B3D_B3DTEX_HXX
#include <goodies/b3dtex.hxx>
#endif

#ifndef _SVX_XLNCLIT_HXX
#include "xlnclit.hxx"
#endif

#ifndef _SFXMETRICITEM_HXX
#include <svtools/metitem.hxx>
#endif

#ifndef _XTABLE_HXX
#include "xtable.hxx"
#endif

#ifndef _SVX_XLNWTIT_HXX
#include "xlnwtit.hxx"
#endif

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

void E3dPolyScene::TakeContour(XPolyPolygon& rPoly) const
{
    ((E3dPolyScene*)this)->TakeContour3D(rPoly);
}

/*************************************************************************
|*
|* Objekt als Kontur in das Polygon einfuegen
|*
\************************************************************************/

void E3dPolyScene::TakeContour3D(XPolyPolygon& rPoly)
{
    // TransformationSet vorbereiten
    InitTransformationSet();

    // Holen
    E3dScene::TakeContour3D(rPoly);
}

/*************************************************************************
|*
|* Paint
|*
\************************************************************************/

FASTBOOL E3dPolyScene::Paint(ExtOutputDevice& rOut,
    const SdrPaintInfoRec& rInfoRec) const
{
    FASTBOOL bOk=TRUE;
    if(GetSubList() && GetSubList()->GetObjCount())
    {
        bOk = ((E3dPolyScene*)this)->LocalPaint3D(rOut, rInfoRec);
    }
    else
    {
        // Leere Szene, zeichne genau wie leere Gruppe
        if (!rInfoRec.bPrinter && rInfoRec.aPaintLayer.IsSet(nLayerId)) {
            OutputDevice* pOutDev=rOut.GetOutDev();
            pOutDev->SetLineColor(Color(COL_LIGHTGRAY));
            pOutDev->SetFillColor();
            pOutDev->DrawRect(aOutRect);
        }
    }
    if (bOk && (rInfoRec.nPaintMode & SDRPAINTMODE_GLUEPOINTS) !=0) {
        bOk=PaintGluePoints(rOut,rInfoRec);
    }
    return bOk;
}

BOOL E3dPolyScene::LocalPaint3D(ExtOutputDevice& rOut,
    const SdrPaintInfoRec& rInfoRec)
{
    Time aStartTime;
    Rectangle aBound(GetSnapRect());
    OutputDevice* pOut = rOut.GetOutDev();

//-/    Base3D *pBase3D = Base3D::Create(pOut, bForceDraftShadeModel);
    sal_uInt16 nShadeMode = GetShadeMode();
    Base3D *pBase3D = Base3D::Create(pOut, nShadeMode > 2);

    DBG_ASSERT(pBase3D, "Habe keinen Base3D Kontext bekommen!!");
    pBase3D->SetDisplayQuality(GetDisplayQuality());

//-/    pBase3D->SetShadeModel(eShadeModel);
    if(nShadeMode == 0)
        pBase3D->SetShadeModel(Base3DFlat);
    else if(nShadeMode == 1)
        pBase3D->SetShadeModel(Base3DPhong);
    else
        pBase3D->SetShadeModel(Base3DSmooth);

    Rectangle aVisible(Point(0,0), pOut->GetOutputSizePixel());
    aVisible = pOut->PixelToLogic(aVisible);

    // Bei GDIMetaFiles falsches aVisible korrigieren
    if((aVisible.Left() == aVisible.Right() && aVisible.Top() == aVisible.Bottom())
        || (aVisible.GetWidth() == 0L || aVisible.GetHeight() == 0L))
        aVisible = aBound;

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
                OutputDevice* pOut = rOut.GetOutDev();

                // red
                pOut->SetLineColor(Color(COL_RED));
                pOut->SetFillColor();
                pOut->DrawRect(aClipBound);

                Rectangle aClipBoundPixel = pOut->LogicToPixel(aClipBound);
                BOOL bWasEnabled = pOut->IsMapModeEnabled();
                pOut->EnableMapMode(FALSE);

                // GREEN
                pOut->SetLineColor(Color(COL_GREEN));
                pOut->SetFillColor();
                pOut->DrawRect(aClipBoundPixel);

                pOut->EnableMapMode(bWasEnabled);
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
            B3dVolume aVol = rSet.GetDeviceVolume();
            Volume3D aVolume(aVol.MinVec(), aVol.MaxVec());
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
                const Vector3D aPos = rSet.WorldToViewCoor(pLabelObject->GetTransPosition());
                Point a2DPos((long)(aPos.X() + 0.5), (long)(aPos.Y() + 0.5));
                pLabel->NbcSetAnchorPos(a2DPos);
                pLabel->Paint(rOut, rInfoRec);
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

void E3dPolyScene::DrawPolySceneClip(ExtOutputDevice& rOut,
    const E3dObject* p3DObj, Base3D* pBase3D, const SdrPaintInfoRec& rInfoRec)
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

void E3dPolyScene::Paint3D(ExtOutputDevice& rOut, Base3D* pBase3D,
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

void E3dPolyScene::DrawAllShadows(Base3D* pBase3D, ExtOutputDevice& rXOut,
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

void E3dPolyScene::DrawWireframe(Base3D* pBase3D, ExtOutputDevice& rXOut)
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
                Matrix4D mTransform = pCompObj->GetFullTransform();
                GetCameraSet().SetObjectTrans(mTransform);
                pBase3D->SetTransformationSet(&(GetScene()->GetCameraSet()));

                // WireFrame eines einzelnen Objektes
                pCompObj->DrawObjectWireframe(rXOut);
            }
        }
    }
}


