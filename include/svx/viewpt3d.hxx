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

/*************************************************************************
|*
|* enums for projection
|*
\************************************************************************/

enum class ProjectionType { Parallel, Perspective };


/*************************************************************************
|*
|* 3D viewport according to PHIGS
|*
\************************************************************************/

class SVXCORE_DLLPUBLIC Viewport3D
{
 protected:
    basegfx::B3DHomMatrix       m_aViewTf;        // the real transformations matrix
    basegfx::B3DPoint           m_aVRP;           // View Reference Point
    basegfx::B3DVector          m_aVPN;           // View Plane Normal
    basegfx::B3DVector          m_aVUV;           // View Up Vector
    basegfx::B3DPoint           m_aPRP;           // Projection Reference Point(View-coordinates)
                                                // up to now only the z-coordinate is considered

    ProjectionType  m_eProjection;    // kind of the projection
    tools::Rectangle m_aDeviceRect;          // position and size of the output area

    struct
    {
        double X, Y, W, H;          // position and size of the view window
    } m_aViewWin;                     // in view coordinates

    basegfx::B3DPoint       m_aViewPoint;     //observers position in world coordinates;
                                    // is calculated by the transformation
    bool            m_bTfValid;       // flag, if transformation is valid

 public:
    Viewport3D();

    void SetVRP(const basegfx::B3DPoint& rNewVRP);
    void SetVPN(const basegfx::B3DVector& rNewVPN);
    void SetVUV(const basegfx::B3DVector& rNewVUV);
    void SetPRP(const basegfx::B3DPoint& rNewPRP);

    const basegfx::B3DPoint&    GetVRP() const  { return m_aVRP; }
    const basegfx::B3DVector&   GetVUV() const  { return m_aVUV; }

    void SetProjection(ProjectionType ePrj)
        { m_eProjection = ePrj; m_bTfValid = false; }
    ProjectionType GetProjection() const { return m_eProjection; }

    void SetViewWindow(double fX, double fY, double fW, double fH);

    void SetDeviceWindow(const tools::Rectangle& rRect);
    const tools::Rectangle& GetDeviceWindow() const { return m_aDeviceRect; }

    // returns observers position in world coordinates
    const basegfx::B3DPoint&    GetViewPoint();
};

#endif // INCLUDED_SVX_VIEWPT3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
