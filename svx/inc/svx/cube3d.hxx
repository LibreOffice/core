/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier) const;

    virtual E3dCubeObj* Clone() const;

    // Set local parameters with geometry recreation
    void SetCubePos(const basegfx::B3DPoint& rNew);
    const basegfx::B3DPoint& GetCubePos() { return aCubePos; }

    void SetCubeSize(const basegfx::B3DVector& rNew);
    const basegfx::B3DVector& GetCubeSize() { return aCubeSize; }

    void SetPosIsCenter(sal_Bool bNew);
    sal_Bool GetPosIsCenter() { return (sal_Bool)bPosIsCenter; }

    void SetSideFlags(sal_uInt16 nNew);
    sal_uInt16 GetSideFlags() { return nSideFlags; }

    // TakeObjName...() is for the display in the UI, for example "3 frames selected".
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;
};

#endif          // _E3D_CUBE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
