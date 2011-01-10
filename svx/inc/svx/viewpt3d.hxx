/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _VIEWPT3D_HXX
#define _VIEWPT3D_HXX

#include "svx/svxdllapi.h"
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <tools/gen.hxx>

// predefines
namespace basegfx { class B3DRange; } // end of namespace basegfx

/*************************************************************************
|*
|* enums fuer Projektion und Seitenverhaeltnis
|*
\************************************************************************/

enum ProjectionType { PR_PARALLEL, PR_PERSPECTIVE };
enum AspectMapType  { AS_NO_MAPPING, AS_HOLD_SIZE, AS_HOLD_X, AS_HOLD_Y };


/*************************************************************************
|*
|* 3D-Viewport nach PHIGS
|*
\************************************************************************/

class SVX_DLLPUBLIC Viewport3D
{
 protected:
    basegfx::B3DHomMatrix       aViewTf;        // die eigentliche Transformationsmatrix
    basegfx::B3DPoint           aVRP;           // View Reference Point
    basegfx::B3DVector          aVPN;           // View Plane Normal
    basegfx::B3DVector          aVUV;           // View Up Vector
    basegfx::B3DPoint           aPRP;           // Projection Reference Point(View-Koordinaten)
                                    // bisher wird nur die Z-Koordinate beachtet
    double          fVPD;           // View Plane Distance
    double          fNearClipDist;  // Abstand der vorderen Clippingebene
    double          fFarClipDist;   // Abstand der hinteren Clippingebene

    ProjectionType  eProjection;    // Art der Projektion
    AspectMapType   eAspectMapping; // Flag fuer Seitenverhaeltnis-Anpassung
                                    // bei Ausgabe auf das Geraet
    Rectangle aDeviceRect;          // Position und Groesse des Ausgabebereichs

    struct
    {
        double X, Y, W, H;          // Position und Groesse des View-Windows
    } aViewWin;                     // in View-Koordinaten

    basegfx::B3DPoint       aViewPoint;     // Beobachterstandpunkt in Weltkoordinaten;
                                    // wird mit der Transformation berechnet
    sal_Bool            bTfValid;       // Flag, ob Transformation gueltig ist

    double fWRatio;                 // Device/View-Seitenverhaeltnisse
    double fHRatio;

    void MakeTransform(void);

 public:
    Viewport3D();

    void SetVRP(const basegfx::B3DPoint& rNewVRP);
    void SetVPN(const basegfx::B3DVector& rNewVPN);
    void SetVUV(const basegfx::B3DVector& rNewVUV);
    void SetPRP(const basegfx::B3DPoint& rNewPRP);
    void SetVPD(double fNewVPD);
    void SetNearClipDist(double fNewNCD);
    void SetFarClipDist(double fNewFCD);

    const basegfx::B3DPoint&    GetVRP() const  { return aVRP; }
    const basegfx::B3DVector&   GetVPN() const  { return aVPN; }
    const basegfx::B3DVector&   GetVUV() const  { return aVUV; }
    const basegfx::B3DPoint&    GetPRP() const  { return aPRP; }
    double  GetVPD() const          { return fVPD; }
    double  GetNearClipDist() const { return fNearClipDist; }
    double  GetFarClipDist() const  { return fFarClipDist; }

    void SetProjection(ProjectionType ePrj)
        { eProjection = ePrj; bTfValid = sal_False; }
    ProjectionType GetProjection() const { return(eProjection); }

    void SetAspectMapping(AspectMapType eAsp)
        { eAspectMapping = eAsp; bTfValid = sal_False; }
    AspectMapType GetAspectMapping() { return eAspectMapping; }

    void SetViewWindow(double fX, double fY, double fW, double fH);
    void GetViewWindow(double& rX, double& rY, double& rW, double& rH) const;

    void SetDeviceWindow(const Rectangle& rRect);
    const Rectangle& GetDeviceWindow() const { return aDeviceRect; }

    // Beobachterstandpunkt in Weltkoordinaten zurueckgeben
    const basegfx::B3DPoint&    GetViewPoint();

    // View-Transformationen
    const basegfx::B3DHomMatrix&    GetViewTransform();

    // Projektion und Mapping
    basegfx::B3DPoint DoProjection(const basegfx::B3DPoint& rVec) const;
    basegfx::B3DPoint   MapToDevice(const basegfx::B3DPoint& rVec) const;
};

#endif      // _VIEWPT3D_HXX
