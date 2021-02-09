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

#ifndef INCLUDED_SVX_CUBE3D_HXX
#define INCLUDED_SVX_CUBE3D_HXX

#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <svx/obj3d.hxx>
#include <svx/svxdllapi.h>

namespace sdr::contact { class ViewContact; }

class E3dDefaultAttributes;

/*************************************************************************
|*
|*                                                              |
|* Create a 3D cuboid; aPos: Center or left, bottom, behind     |__
|*                           (depending on bPosIsCenter)       /
|* nSideFlags indicates, if only some of the cuboid surfaces can
|* be created; the corresponding bits are defined in the enum.
|* The flag bDblSided indicates whether the created surfaces are
|* two-sided (which only makes sense if not all of the surfaces were
|* created).
|*
\************************************************************************/

class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC E3dCubeObj final : public E3dCompoundObject
{
    // Parameter
    basegfx::B3DPoint                   aCubePos;
    basegfx::B3DVector                  aCubeSize;

    // BOOLeans
    bool                                bPosIsCenter : 1;

    void SetDefaultAttributes(const E3dDefaultAttributes& rDefault);
    virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;

private:
    // protected destructor - due to final, make private
    virtual ~E3dCubeObj() override;

public:
    E3dCubeObj(SdrModel& rSdrModel,
        const E3dDefaultAttributes& rDefault,
        const basegfx::B3DPoint& aPos,
        const basegfx::B3DVector& r3DSize);
    E3dCubeObj(SdrModel& rSdrModel, E3dCubeObj const &);
    E3dCubeObj(SdrModel& rSdrModel);

    virtual SdrObjKind GetObjIdentifier() const override;
    virtual SdrObjectUniquePtr DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

    virtual E3dCubeObj* CloneSdrObject(SdrModel& rTargetModel) const override;

    // Set local parameters with geometry recreation
    void SetCubePos(const basegfx::B3DPoint& rNew);
    const basegfx::B3DPoint& GetCubePos() const { return aCubePos; }

    void SetCubeSize(const basegfx::B3DVector& rNew);
    const basegfx::B3DVector& GetCubeSize() const { return aCubeSize; }

    void SetPosIsCenter(bool bNew);
    bool GetPosIsCenter() const { return bPosIsCenter; }

    // TakeObjName...() is for the display in the UI, for example "3 frames selected".
    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;
};

#endif // INCLUDED_SVX_CUBE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
