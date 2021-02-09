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

#pragma once

#include <config_options.h>
#include <svx/obj3d.hxx>
#include <svx/svxdllapi.h>

class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) E3dPolygonObj final : public E3dCompoundObject
{
    // parameters
    basegfx::B3DPolyPolygon aPolyPoly3D;
    basegfx::B3DPolyPolygon aPolyNormals3D;
    basegfx::B2DPolyPolygon aPolyTexture2D;
    bool bLineOnly;

    SVX_DLLPRIVATE void CreateDefaultNormals();
    SVX_DLLPRIVATE void CreateDefaultTexture();

    virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;

    // protected destructor
    virtual ~E3dPolygonObj() override;

public:
    void SetPolyPolygon3D(const basegfx::B3DPolyPolygon& rNewPolyPoly3D);
    void SetPolyNormals3D(const basegfx::B3DPolyPolygon& rNewPolyPoly3D);
    void SetPolyTexture2D(const basegfx::B2DPolyPolygon& rNewPolyPoly2D);

    E3dPolygonObj(SdrModel& rSdrModel, const basegfx::B3DPolyPolygon& rPolyPoly3D);
    E3dPolygonObj(SdrModel& rSdrModel);
    E3dPolygonObj(SdrModel& rSdrModel, E3dPolygonObj const& rSource);

    const basegfx::B3DPolyPolygon& GetPolyPolygon3D() const { return aPolyPoly3D; }
    const basegfx::B3DPolyPolygon& GetPolyNormals3D() const { return aPolyNormals3D; }
    const basegfx::B2DPolyPolygon& GetPolyTexture2D() const { return aPolyTexture2D; }

    virtual SdrObjKind GetObjIdentifier() const override;
    virtual SdrObjectUniquePtr DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

    virtual E3dPolygonObj* CloneSdrObject(SdrModel& rTargetModel) const override;

    // LineOnly?
    bool GetLineOnly() const { return bLineOnly; }
    void SetLineOnly(bool bNew);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
