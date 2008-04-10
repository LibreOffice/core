/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: e3dundo.cxx,v $
 * $Revision: 1.9 $
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
#include <svx/outlobj.hxx>
#include <svx/view3d.hxx>
#include <svx/scene3d.hxx>

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
BOOL E3dUndoAction::CanRepeat(SfxRepeatTarget&) const
{
    return FALSE;
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
    pMy3DObj->SetTransform(aMyOldRotation);
    pMy3DObj->GetScene()->CorrectSceneDimensions();
}

/************************************************************************\
|*
|* Undo fuer 3D-Rotation ueber die Rotationsmatrizen
|*
\************************************************************************/
void E3dRotateUndoAction::Redo ()
{
    pMy3DObj->SetTransform(aMyNewRotation);
    pMy3DObj->GetScene()->CorrectSceneDimensions();
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
    BOOL bUseSubObj)
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
    //pObject->SetItemSetAndBroadcast(aOldSet);
    pObject->SetMergedItemSetAndBroadcast(aOldSet);

    if(pObject->ISA(E3dObject))
    {
        E3dScene* pScene = ((E3dObject*)pObject)->GetScene();
        if(pScene)
            pScene->CorrectSceneDimensions();
    }
}

/*************************************************************************
|*
|* Redo()
|*
\************************************************************************/
void E3dAttributesUndoAction::Redo()
{
    //pObject->SetItemSetAndBroadcast(aNewSet);
    pObject->SetMergedItemSetAndBroadcast(aNewSet);

    if(pObject->ISA(E3dObject))
    {
        E3dScene* pScene = ((E3dObject*)pObject)->GetScene();
        if(pScene)
            pScene->CorrectSceneDimensions();
    }
}

/*************************************************************************
|*
|* Mehrfaches Undo nicht moeglich
|*
\************************************************************************/
BOOL E3dAttributesUndoAction::CanRepeat(SfxRepeatTarget& /*rView*/) const
{
    return FALSE;
}

/*************************************************************************
|*
|* Mehrfaches Undo nicht moeglich
|*
\************************************************************************/
void E3dAttributesUndoAction::Repeat()
{
}

