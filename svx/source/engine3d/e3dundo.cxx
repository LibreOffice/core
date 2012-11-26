/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/e3dundo.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/outlobj.hxx>
#include <svx/view3d.hxx>
#include <svx/scene3d.hxx>
#include <svx/e3dsceneupdater.hxx>

/************************************************************************/

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
    pMy3DObj->SetB3DTransform(aMyOldRotation);
}

/************************************************************************\
|*
|* Undo fuer 3D-Rotation ueber die Rotationsmatrizen
|*
\************************************************************************/
void E3dRotateUndoAction::Redo ()
{
    E3DModifySceneSnapRectUpdater aUpdater(pMy3DObj);
    pMy3DObj->SetB3DTransform(aMyNewRotation);
}

/*************************************************************************
|*
|* E3dAttributesUndoAction
|*
\************************************************************************/

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

