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

#include <svx/e3dundo.hxx>
#include <svx/e3dsceneupdater.hxx>
#include <utility>


E3dUndoAction::~E3dUndoAction ()
{
}

// Repeat does not exist

bool E3dUndoAction::CanRepeat(SfxRepeatTarget&) const
{
    return false;
}


// Undo destructor for 3D-Rotation
E3dRotateUndoAction::~E3dRotateUndoAction()
{
}

// Undo for 3D-Rotation on the Rotation matrix
void E3dRotateUndoAction::Undo()
{
    E3DModifySceneSnapRectUpdater aUpdater(&mrMy3DObj);
    mrMy3DObj.SetTransform(maMyOldRotation);
}

// Redo for 3D-Rotation on the Rotation matrix
void E3dRotateUndoAction::Redo()
{
    E3DModifySceneSnapRectUpdater aUpdater(&mrMy3DObj);
    mrMy3DObj.SetTransform(maMyNewRotation);
}

E3dAttributesUndoAction::E3dAttributesUndoAction(
    E3dObject& rInObject,
    SfxItemSet aNewSet,
    SfxItemSet aOldSet)
:   SdrUndoAction(rInObject.getSdrModelFromSdrObject())
    ,mrObject(rInObject)
    ,maNewSet(std::move(aNewSet))
    ,maOldSet(std::move(aOldSet))
{
}

E3dAttributesUndoAction::~E3dAttributesUndoAction()
{
}

// Undo() implemented through Set3DAttributes() to only maintain the attributes
// in one place

void E3dAttributesUndoAction::Undo()
{
    E3DModifySceneSnapRectUpdater aUpdater(&mrObject);
    mrObject.SetMergedItemSetAndBroadcast(maOldSet);
}

void E3dAttributesUndoAction::Redo()
{
    E3DModifySceneSnapRectUpdater aUpdater(&mrObject);
    mrObject.SetMergedItemSetAndBroadcast(maNewSet);
}

// Multiple Undo is not possible
bool E3dAttributesUndoAction::CanRepeat(SfxRepeatTarget& /*rView*/) const
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
