/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewpt3d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:47:36 $
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

#ifndef _VIEWPT3D_HXX
#define _VIEWPT3D_HXX

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

#ifndef _BGFX_POINT_B3DPOINT_HXX
#include <basegfx/point/b3dpoint.hxx>
#endif

#ifndef _BGFX_VECTOR_B3DVECTOR_HXX
#include <basegfx/vector/b3dvector.hxx>
#endif

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif

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
    basegfx::B3DVector      aVPN;           // View Plane Normal
    basegfx::B3DVector      aVUV;           // View Up Vector
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
    BOOL            bTfValid;       // Flag, ob Transformation gueltig ist

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
        { eProjection = ePrj; bTfValid = FALSE; }
    ProjectionType GetProjection() const { return(eProjection); }

    void SetAspectMapping(AspectMapType eAsp)
        { eAspectMapping = eAsp; bTfValid = FALSE; }
    AspectMapType GetAspectMapping() { return eAspectMapping; }

    void SetViewWindow(double fX, double fY, double fW, double fH);
    void GetViewWindow(double& rX, double& rY, double& rW, double& rH) const;

    // View-Window genau um das mit rTransform transformierte Volumen legen
    void FitViewToVolume(const basegfx::B3DRange& rVolume, const basegfx::B3DHomMatrix& rTransform);

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
