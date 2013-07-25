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

#ifndef _E3D_SPHERE3D_HXX
#define _E3D_SPHERE3D_HXX

#include <svx/obj3d.hxx>
#include "svx/svxdllapi.h"

/**
 * SphereObject with diameter r3DSize.
 * The count of planes depends on the horizontal and vertical segment count.
 */
class SVX_DLLPUBLIC E3dSphereObj : public E3dCompoundObject
{
private:
    basegfx::B3DPoint               aCenter;
    basegfx::B3DVector              aSize;

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();
    void SetDefaultAttributes(E3dDefaultAttributes& rDefault);

public:
    TYPEINFO();
    E3dSphereObj(E3dDefaultAttributes& rDefault, const basegfx::B3DPoint& rCenter, const basegfx::B3DVector& r3DSize);

    // FG: This constructor is only called from MakeObject from the 3d-Objectfactory
    //     when a document with a sphere is loaded.  This constructor does not call
    //     CreateSphere, or create any spheres.
    E3dSphereObj(int dummy);

    // horizontal segments:
    sal_uInt32 GetHorizontalSegments() const
        { return ((const Svx3DHorizontalSegmentsItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_HORZ_SEGS)).GetValue(); }

    // VerticalSegments:
    sal_uInt32 GetVerticalSegments() const
        { return ((const Svx3DVerticalSegmentsItem&)GetObjectItemSet().Get(SDRATTR_3DOBJ_VERT_SEGS)).GetValue(); }

    virtual sal_uInt16 GetObjIdentifier() const;
    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const;

    virtual E3dSphereObj* Clone() const;

    const basegfx::B3DPoint& Center() const { return aCenter; }
    const basegfx::B3DVector& Size() const { return aSize; }

    // set local parameters when the geometry is recreated
    void SetCenter(const basegfx::B3DPoint& rNew);
    void SetSize(const basegfx::B3DVector& rNew);

    // TakeObjName...() is for displaying in the UI, eg "3 selected frames."
    virtual OUString TakeObjNameSingul() const;
    virtual OUString TakeObjNamePlural() const;
};

#endif // _E3D_SPHERE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
