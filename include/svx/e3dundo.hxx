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

#ifndef INCLUDED_SVX_E3DUNDO_HXX
#define INCLUDED_SVX_E3DUNDO_HXX

#include <svl/itemset.hxx>
#include <svx/svdundo.hxx>
#include <svx/obj3d.hxx>
#include <utility>
#include <svx/svxdllapi.h>

/************************************************************************\
|*
|* Base class for all 3D undo actions.
|*
\************************************************************************/
class SAL_WARN_UNUSED E3dUndoAction : public SdrUndoAction
{
    protected:
        E3dObject&      mrMy3DObj;

    public:
        E3dUndoAction(E3dObject &r3DObj)
        :   SdrUndoAction(r3DObj.getSdrModelFromSdrObject()),
            mrMy3DObj(r3DObj)
        {
        }

        virtual ~E3dUndoAction () override;

        virtual bool CanRepeat(SfxRepeatTarget&) const override;
};

/************************************************************************\
|*
|* Undo for 3D rotation through the rotation matrices
|*
\************************************************************************/
class SAL_WARN_UNUSED E3dRotateUndoAction final : public E3dUndoAction
{
private:
    basegfx::B3DHomMatrix   maMyOldRotation;
    basegfx::B3DHomMatrix   maMyNewRotation;

public:
    E3dRotateUndoAction(
        E3dObject& r3DObj,
        basegfx::B3DHomMatrix aOldRotation,
        basegfx::B3DHomMatrix aNewRotation)
    :   E3dUndoAction(r3DObj),
        maMyOldRotation(std::move(aOldRotation)),
        maMyNewRotation(std::move(aNewRotation))
    {
    }

    virtual ~E3dRotateUndoAction () override;

    virtual void Undo() override;
    virtual void Redo() override;
};

/************************************************************************\
|*
|* Undo for 3D attributes (implemented using Set3DAttributes())
|*
\************************************************************************/
class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC E3dAttributesUndoAction final : public SdrUndoAction
{
private:
    using SdrUndoAction::Repeat;

    SdrObject&          mrObject;
    const SfxItemSet    maNewSet;
    const SfxItemSet    maOldSet;

 public:
        E3dAttributesUndoAction(
            E3dObject& rInObject,
            SfxItemSet aNewSet,
            SfxItemSet aOldSet);

        virtual ~E3dAttributesUndoAction() override;

        virtual bool CanRepeat(SfxRepeatTarget& rView) const override;
        virtual void Undo() override;
        virtual void Redo() override;
};

#endif // INCLUDED_SVX_E3DUNDO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
