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

#ifndef _E3D_UNDO_HXX
#define _E3D_UNDO_HXX

#include <svx/svdundo.hxx>
#include <svx/scene3d.hxx>
#include "svx/svxdllapi.h"

/************************************************************************\
|*
|* Base class for all 3D undo actions.
|*
\************************************************************************/
class E3dUndoAction : public SdrUndoAction
{

    protected :
        E3dObject *pMy3DObj;

    public:
        TYPEINFO();
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
|* Undo for 3D rotation through the rotation matrices
|*
\************************************************************************/
class E3dRotateUndoAction : public E3dUndoAction
{
        basegfx::B3DHomMatrix aMyOldRotation;
        basegfx::B3DHomMatrix aMyNewRotation;

    public:
        TYPEINFO();
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
|* Undo for 3D attributes (implemented using Set3DAttributes())
|*
\************************************************************************/
class SVX_DLLPUBLIC E3dAttributesUndoAction : public SdrUndoAction
{
    using SdrUndoAction::Repeat;

    SdrObject*  pObject;

    const SfxItemSet aNewSet;
    const SfxItemSet aOldSet;

 public:
        TYPEINFO();
        E3dAttributesUndoAction( SdrModel &rModel,
            E3dObject* pInObject,
            const SfxItemSet& rNewSet,
            const SfxItemSet& rOldSet);

        virtual ~E3dAttributesUndoAction();

        virtual sal_Bool CanRepeat(SfxRepeatTarget& rView) const;
        virtual void Undo();
        virtual void Redo();
        virtual void Repeat();
};

#endif          // _E3D_CUBE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
