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

#include <svx/e3dundo.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/outlobj.hxx>
#include <svx/view3d.hxx>
#include <svx/scene3d.hxx>
#include <svx/e3dsceneupdater.hxx>

/************************************************************************/

TYPEINIT1(E3dUndoAction, SfxUndoAction);

E3dUndoAction::~E3dUndoAction ()
{
}

// Repeat does not exist

sal_Bool E3dUndoAction::CanRepeat(SfxRepeatTarget&) const
{
    return sal_False;
}

/************************************************************************/

TYPEINIT1(E3dRotateUndoAction, E3dUndoAction);

/************************************************************************

                        E3dRotateUndoAction

************************************************************************/

// Undo destructor for 3D-Rotation

E3dRotateUndoAction::~E3dRotateUndoAction ()
{
}

// Undo for 3D-Rotation on the Rotation matrix

void E3dRotateUndoAction::Undo ()
{
    E3DModifySceneSnapRectUpdater aUpdater(pMy3DObj);
    pMy3DObj->SetTransform(aMyOldRotation);
}

// Redo for 3D-Rotation on the Rotation matrix

void E3dRotateUndoAction::Redo ()
{
    E3DModifySceneSnapRectUpdater aUpdater(pMy3DObj);
    pMy3DObj->SetTransform(aMyNewRotation);
}

TYPEINIT1(E3dAttributesUndoAction, SdrUndoAction);

E3dAttributesUndoAction::E3dAttributesUndoAction( SdrModel &rModel,
    E3dObject*  pInObject,
    const SfxItemSet& rNewSet,
    const SfxItemSet& rOldSet)
:   SdrUndoAction( rModel ),
    pObject      ( pInObject ),
    aNewSet      ( rNewSet ),
    aOldSet      ( rOldSet )
{
}

E3dAttributesUndoAction::~E3dAttributesUndoAction()
{
}

// Undo() implemented through Set3DAttributes() to only maintain the attributes
// in one place

void E3dAttributesUndoAction::Undo()
{
    E3DModifySceneSnapRectUpdater aUpdater(pObject);
    pObject->SetMergedItemSetAndBroadcast(aOldSet);
}

void E3dAttributesUndoAction::Redo()
{
    E3DModifySceneSnapRectUpdater aUpdater(pObject);
    pObject->SetMergedItemSetAndBroadcast(aNewSet);
}

// Multiple Undo is not possible

sal_Bool E3dAttributesUndoAction::CanRepeat(SfxRepeatTarget& /*rView*/) const
{
    return sal_False;
}

// Multiple Undo is not possible

void E3dAttributesUndoAction::Repeat()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
