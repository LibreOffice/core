/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <A11yObjectName.hxx>

#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svdoashp.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdobjkind.hxx>

namespace sd
{
OUString GetA11yObjectName(const SdrObject& rObject)
{
    OUString aName = rObject.GetName();
    if (!aName.isEmpty())
        return aName;

    if (rObject.GetObjIdentifier() == SdrObjKind::Graphic)
        return SvxResId(STR_ObjNameSingulGRAF);

    if (rObject.GetObjIdentifier() == SdrObjKind::CustomShape)
    {
        const SdrObjCustomShape& rCustomShape = static_cast<const SdrObjCustomShape&>(rObject);
        if (rCustomShape.IsTextPath())
            return SvxResId(STR_ObjNameSingulFONTWORK);

        OUString aShapeName = rCustomShape.GetCustomShapeName();
        if (!aShapeName.isEmpty())
            return aShapeName;
    }

    return rObject.TakeObjNameSingul();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
