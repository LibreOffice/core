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



#ifndef _E3D_UNDO_HXX
#define _E3D_UNDO_HXX

#include <svx/svdundo.hxx>
class E3DObject;
#include <svx/scene3d.hxx>
#include "svx/svxdllapi.h"

class E3dView;

/************************************************************************\
|*
|* Basisklasse fuer alle 3D-Undo-Aktionen.
|*
\************************************************************************/
class E3dUndoAction : public SdrUndoAction
{

    protected :
        E3dObject *pMy3DObj;

    public:
        E3dUndoAction (SdrModel  *pModel,
                       E3dObject *p3DObj) :
            SdrUndoAction (*pModel),
            pMy3DObj (p3DObj)
            {
            }

        virtual ~E3dUndoAction ();

        virtual sal_Bool CanRepeat(SfxRepeatTarget&) const;
};

/************************************************************************\
|*
|* Undo fuer 3D-Rotation ueber die Rotationsmatrizen (ob das wohl klappt ?)
|*
\************************************************************************/
class E3dRotateUndoAction : public E3dUndoAction
{
        basegfx::B3DHomMatrix aMyOldRotation;
        basegfx::B3DHomMatrix aMyNewRotation;

    public:
        E3dRotateUndoAction (SdrModel       *pModel,
                             E3dObject      *p3DObj,
                             const basegfx::B3DHomMatrix &aOldRotation,
                             const basegfx::B3DHomMatrix &aNewRotation) :
            E3dUndoAction (pModel, p3DObj),
            aMyOldRotation (aOldRotation),
            aMyNewRotation (aNewRotation)
            {
            }

        virtual ~E3dRotateUndoAction ();

        virtual void Undo();
        virtual void Redo();

};

/************************************************************************\
|*
|* Undo fuer 3D-Attribute (Implementiert ueber Set3DAttributes())
|*
\************************************************************************/
class SVX_DLLPUBLIC E3dAttributesUndoAction : public SdrUndoAction
{
    using SdrUndoAction::Repeat;

    SdrObject*  pObject;
    E3dView*    pView;
    sal_Bool        bUseSubObjects;

    const SfxItemSet aNewSet;
    const SfxItemSet aOldSet;

 public:
        E3dAttributesUndoAction( SdrModel &rModel, E3dView* pView,
            E3dObject* pInObject,
            const SfxItemSet& rNewSet,
            const SfxItemSet& rOldSet,
            sal_Bool bUseSubObj);

        virtual ~E3dAttributesUndoAction();

        virtual sal_Bool CanRepeat(SfxRepeatTarget& rView) const;
        virtual void Undo();
        virtual void Redo();
        virtual void Repeat();
};

#endif          // _E3D_CUBE3D_HXX
