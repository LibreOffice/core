/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_VIEWPT3D_HXX
#define INCLUDED_SVX_VIEWPT3D_HXX

#include <svx/svxdllapi.h>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <tools/gen.hxx>

// predefines
namespace basegfx { class B3DRange; } // end of namespace basegfx

/*************************************************************************
|*
|* enums for projection and aspect ratio
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
    basegfx::B3DHomMatrix       aViewTf;        // the real transformations matrix
    basegfx::B3DPoint           aVRP;           // View Reference Point
    basegfx::B3DVector          aVPN;           // View Plane Normal
    basegfx::B3DVector          aVUV;           // View Up Vector
    basegfx::B3DPoint           aPRP;           // Projection Reference Point(View-coordinates)
                                    // up to now only the z-coordinate is considered
    double          fVPD;           // View Plane Distance

    ProjectionType  eProjection;    // kind of the projection
    AspectMapType   eAspectMapping; // flag for the acclimatization of the aspect ratio
                                    // for display on the device
    Rectangle aDeviceRect;          // position and size of the output area

    struct
    {
        double X, Y, W, H;          // position and size of the view window
    } aViewWin;                     // in view coordinates

    basegfx::B3DPoint       aViewPoint;     //observers position in world coordinates;
                                    // is calculated by the transformation
    bool            bTfValid;       // flag, if transformation is valid

    double fWRatio;                 // device/view aspect ratio
    double fHRatio;

    void MakeTransform();

 public:
    Viewport3D();

    void SetVRP(const basegfx::B3DPoint& rNewVRP);
    void SetVPN(const basegfx::B3DVector& rNewVPN);
    void SetVUV(const basegfx::B3DVector& rNewVUV);
    void SetPRP(const basegfx::B3DPoint& rNewPRP);
    void SetVPD(double fNewVPD);

    const basegfx::B3DPoint&    GetVRP() const  { return aVRP; }
    const basegfx::B3DVector&   GetVUV() const  { return aVUV; }

    void SetProjection(ProjectionType ePrj)
        { eProjection = ePrj; bTfValid = false; }
    ProjectionType GetProjection() const { return eProjection; }

    AspectMapType GetAspectMapping() { return eAspectMapping; }

    void SetViewWindow(double fX, double fY, double fW, double fH);

    void SetDeviceWindow(const Rectangle& rRect);
    const Rectangle& GetDeviceWindow() const { return aDeviceRect; }

    // returns observers position in world coordinates
    const basegfx::B3DPoint&    GetViewPoint();
};

#endif // INCLUDED_SVX_VIEWPT3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
