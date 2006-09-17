/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: e3dundo.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:55:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _E3D_UNDO_HXX
#include "e3dundo.hxx"
#endif

#ifndef _SVDMODEL_HXX
#include "svdmodel.hxx"
#endif

#ifndef _OUTLOBJ_HXX
#include "outlobj.hxx"
#endif

#ifndef _E3D_VIEW3D_HXX
#include "view3d.hxx"
#endif

#ifndef _E3D_SCENE3D_HXX
#include "scene3d.hxx"
#endif

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

