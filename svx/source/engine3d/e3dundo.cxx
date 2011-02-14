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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/e3dundo.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/outlobj.hxx>
#include <svx/view3d.hxx>
#include <svx/scene3d.hxx>
#include <svx/e3dsceneupdater.hxx>

/************************************************************************/

TYPEINIT1(E3dUndoAction, SfxUndoAction);

/************************************************************************\
|*
|* Destruktor der Basisklasse
|*
\************************************************************************/
E3dUndoAction::~E3dUndoAction ()
{
}

/************************************************************************\
|*
|* Repeat gibt es nicht
|*
\************************************************************************/
sal_Bool E3dUndoAction::CanRepeat(SfxRepeatTarget&) const
{
    return sal_False;
}

/************************************************************************/

TYPEINIT1(E3dRotateUndoAction, E3dUndoAction);

/************************************************************************

                        E3dRotateUndoAction

************************************************************************/

/************************************************************************\
|*
|* Undodestruktor fuer 3D-Rotation
|*
\************************************************************************/
E3dRotateUndoAction::~E3dRotateUndoAction ()
{
}

/************************************************************************\
|*
|* Undo fuer 3D-Rotation ueber die Rotationsmatrizen
|*
\************************************************************************/
void E3dRotateUndoAction::Undo ()
{
    E3DModifySceneSnapRectUpdater aUpdater(pMy3DObj);
    pMy3DObj->SetTransform(aMyOldRotation);
}

/************************************************************************\
|*
|* Undo fuer 3D-Rotation ueber die Rotationsmatrizen
|*
\************************************************************************/
void E3dRotateUndoAction::Redo ()
{
    E3DModifySceneSnapRectUpdater aUpdater(pMy3DObj);
    pMy3DObj->SetTransform(aMyNewRotation);
}

/*************************************************************************
|*
|* E3dAttributesUndoAction
|*
\************************************************************************/

TYPEINIT1(E3dAttributesUndoAction, SdrUndoAction);

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/
E3dAttributesUndoAction::E3dAttributesUndoAction( SdrModel &rModel,
    E3dView*    p3dView,
    E3dObject*  pInObject,
    const SfxItemSet& rNewSet,
    const SfxItemSet& rOldSet,
    sal_Bool bUseSubObj)
:   SdrUndoAction( rModel ),
    pObject      ( pInObject ),
    pView        ( p3dView ),
    bUseSubObjects(bUseSubObj),
    aNewSet      ( rNewSet ),
    aOldSet      ( rOldSet )
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/
E3dAttributesUndoAction::~E3dAttributesUndoAction()
{
}

/*************************************************************************
|*
|* Undo()
|* Implementiert ueber Set3DAttributes(), um die Attribute nur an einer
|* Stelle pflegen zu muessen!
|*
\************************************************************************/
void E3dAttributesUndoAction::Undo()
{
    E3DModifySceneSnapRectUpdater aUpdater(pObject);
    pObject->SetMergedItemSetAndBroadcast(aOldSet);
}

/*************************************************************************
|*
|* Redo()
|*
\************************************************************************/
void E3dAttributesUndoAction::Redo()
{
    E3DModifySceneSnapRectUpdater aUpdater(pObject);
    pObject->SetMergedItemSetAndBroadcast(aNewSet);
}

/*************************************************************************
|*
|* Mehrfaches Undo nicht moeglich
|*
\************************************************************************/
sal_Bool E3dAttributesUndoAction::CanRepeat(SfxRepeatTarget& /*rView*/) const
{
    return sal_False;
}

/*************************************************************************
|*
|* Mehrfaches Undo nicht moeglich
|*
\************************************************************************/
void E3dAttributesUndoAction::Repeat()
{
}

