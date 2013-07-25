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

#ifndef _E3D_CUBE3D_HXX
#define _E3D_CUBE3D_HXX

#include <svx/obj3d.hxx>
#include "svx/svxdllapi.h"

/*************************************************************************
|*
|*                                                              |
|* Create a 3D cuboid; aPos: Center oder left, bottom, behind   |__
|*                           (depending on bPosIsCenter)       /
|* nSideFlags indicates, if only some of the cuboid surfaces can
|* be created; the corresponding bits are defined in the enum.
|* The flag bDblSided indicates whether the created surfaces are
|* two-sided (which only makes sense if not all of the surfaces were
|* created).
|*
\************************************************************************/

enum { CUBE_BOTTOM = 0x0001, CUBE_BACK = 0x0002, CUBE_LEFT = 0x0004,
       CUBE_TOP = 0x0008, CUBE_RIGHT = 0x0010, CUBE_FRONT = 0x0020,
       CUBE_FULL = 0x003F, CUBE_OPEN_TB = 0x0036, CUBE_OPEN_LR = 0x002B,
       CUBE_OPEN_FB = 0x001D };

class SVX_DLLPUBLIC E3dCubeObj : public E3dCompoundObject
{
private:
    // Parameter
    basegfx::B3DPoint                   aCubePos;
    basegfx::B3DVector                  aCubeSize;
    sal_uInt16                              nSideFlags;

    // BOOLeans
    unsigned                            bPosIsCenter : 1;

protected:
    void SetDefaultAttributes(E3dDefaultAttributes& rDefault);
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

public:
    TYPEINFO();
    E3dCubeObj(E3dDefaultAttributes& rDefault, basegfx::B3DPoint aPos, const basegfx::B3DVector& r3DSize);
    E3dCubeObj();

    virtual sal_uInt16 GetObjIdentifier() const;
    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const;

    virtual E3dCubeObj* Clone() const;

    // Set local parameters with geometry recreation
    void SetCubePos(const basegfx::B3DPoint& rNew);
    const basegfx::B3DPoint& GetCubePos() { return aCubePos; }

    void SetCubeSize(const basegfx::B3DVector& rNew);
    const basegfx::B3DVector& GetCubeSize() { return aCubeSize; }

    void SetPosIsCenter(sal_Bool bNew);
    sal_Bool GetPosIsCenter() { return (sal_Bool)bPosIsCenter; }

    sal_uInt16 GetSideFlags() { return nSideFlags; }

    // TakeObjName...() is for the display in the UI, for example "3 frames selected".
    virtual OUString TakeObjNameSingul() const;
    virtual OUString TakeObjNamePlural() const;
};

#endif          // _E3D_CUBE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
