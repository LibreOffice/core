/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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



bool E3dUndoAction::CanRepeat(SfxRepeatTarget&) const
{
    return false;
}

/************************************************************************/

TYPEINIT1(E3dRotateUndoAction, E3dUndoAction);

/************************************************************************

                        E3dRotateUndoAction

************************************************************************/



E3dRotateUndoAction::~E3dRotateUndoAction ()
{
}



void E3dRotateUndoAction::Undo ()
{
    E3DModifySceneSnapRectUpdater aUpdater(pMy3DObj);
    pMy3DObj->SetTransform(aMyOldRotation);
}



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
    : SdrUndoAction( rModel )
    , pObject      ( pInObject )
    , aNewSet      ( rNewSet )
    , aOldSet      ( rOldSet )
{
}

E3dAttributesUndoAction::~E3dAttributesUndoAction()
{
}




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



bool E3dAttributesUndoAction::CanRepeat(SfxRepeatTarget& /*rView*/) const
{
    return false;
}



void E3dAttributesUndoAction::Repeat()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
