/*************************************************************************
 *
 *  $RCSfile: e3dundo.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2000-10-30 10:55:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

************************************************************************\

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
    pView        ( p3dView ),
    pObject      ( pInObject ),
    aNewSet      ( rNewSet ),
    aOldSet      ( rOldSet ),
    bUseSubObjects(bUseSubObj)
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
//-/    if(pObject->ISA(E3dObject))
//-/    {
//-/        ((E3dObject*)pObject)->SetAttrUseSubObjects(bUseSubObjects);
//-/    }

//-/    pObject->SetAttributes( aOldSet, FALSE);
//-/    SdrBroadcastItemChange aItemChange(*pObject);
    pObject->SetItemSetAndBroadcast(aOldSet);
//-/    pObject->BroadcastItemChange(aItemChange);

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
//-/    if(pObject->ISA(E3dObject))
//-/    {
//-/        ((E3dObject*)pObject)->SetAttrUseSubObjects(bUseSubObjects);
//-/    }

//-/    pObject->SetAttributes( aNewSet, FALSE);
//-/    SdrBroadcastItemChange aItemChange(*pObject);
    pObject->SetItemSetAndBroadcast(aNewSet);
//-/    pObject->BroadcastItemChange(aItemChange);

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
BOOL E3dAttributesUndoAction::CanRepeat(SfxRepeatTarget& rView) const
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

