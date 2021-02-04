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

#ifndef INCLUDED_SD_INC_UNDO_UNDOOBJECTS_HXX
#define INCLUDED_SD_INC_UNDO_UNDOOBJECTS_HXX

#include <memory>
#include <svx/svdundo.hxx>
#include <pres.hxx>
#include <tools/weakbase.hxx>
#include <unotools/weakref.hxx>

class SdrObjUserCall;
class SdPage;

namespace sd
{
class UndoRemovePresObjectImpl
{
protected:
    UndoRemovePresObjectImpl(SdrObject& rObject);
    virtual ~UndoRemovePresObjectImpl();

    virtual void Undo();
    virtual void Redo();

private:
    std::unique_ptr<SfxUndoAction> mpUndoUsercall;
    std::unique_ptr<SfxUndoAction> mpUndoAnimation;
    std::unique_ptr<SfxUndoAction> mpUndoPresObj;
};

class UndoRemoveObject final : public SdrUndoRemoveObj, public UndoRemovePresObjectImpl
{
public:
    UndoRemoveObject(SdrObject& rObject);

    virtual void Undo() override;
    virtual void Redo() override;

private:
    ::tools::WeakReference<SdrObject> mxSdrObject;
};

class UndoDeleteObject final : public SdrUndoDelObj, public UndoRemovePresObjectImpl
{
public:
    UndoDeleteObject(SdrObject& rObject, bool bOrdNumDirect);

    virtual void Undo() override;
    virtual void Redo() override;

private:
    ::tools::WeakReference<SdrObject> mxSdrObject;
};

class UndoReplaceObject final : public SdrUndoReplaceObj, public UndoRemovePresObjectImpl
{
public:
    UndoReplaceObject(SdrObject& rOldObject, SdrObject& rNewObject);

    virtual void Undo() override;
    virtual void Redo() override;

private:
    ::tools::WeakReference<SdrObject> mxSdrObject;
};

class UndoObjectSetText final : public SdrUndoObjSetText
{
public:
    UndoObjectSetText(SdrObject& rNewObj, sal_Int32 nText);
    virtual ~UndoObjectSetText() override;

    virtual void Undo() override;
    virtual void Redo() override;

private:
    std::unique_ptr<SfxUndoAction> mpUndoAnimation;
    bool mbNewEmptyPresObj;
    ::tools::WeakReference<SdrObject> mxSdrObject;
};

// Undo for SdrObject::SetUserCall()

class UndoObjectUserCall final : public SdrUndoObj
{
public:
    UndoObjectUserCall(SdrObject& rNewObj);

    virtual void Undo() override;
    virtual void Redo() override;

private:
    SdrObjUserCall* mpOldUserCall;
    SdrObjUserCall* mpNewUserCall;
    ::tools::WeakReference<SdrObject> mxSdrObject;
};

// Undo for SdPage::InsertPresObj() and SdPage::RemovePresObj()

class UndoObjectPresentationKind final : public SdrUndoObj
{
public:
    UndoObjectPresentationKind(SdrObject& rObject);

    virtual void Undo() override;
    virtual void Redo() override;

private:
    PresObjKind meOldKind;
    PresObjKind meNewKind;
    ::unotools::WeakReference<SdPage> mxPage;
    ::tools::WeakReference<SdrObject> mxSdrObject;
};

// Restores correct position and size for presentation shapes with user call
// on undo

class UndoAutoLayoutPosAndSize final : public SfxUndoAction
{
public:
    UndoAutoLayoutPosAndSize(SdPage& rPage);

    virtual void Undo() override;
    virtual void Redo() override;

private:
    ::unotools::WeakReference<SdPage> mxPage;
};

class UndoGeoObject final : public SdrUndoGeoObj
{
public:
    UndoGeoObject(SdrObject& rNewObj);

    virtual void Undo() override;
    virtual void Redo() override;

private:
    ::unotools::WeakReference<SdPage> mxPage;
    ::tools::WeakReference<SdrObject> mxSdrObject;
};

class UndoAttrObject final : public SdrUndoAttrObj
{
public:
    UndoAttrObject(SdrObject& rObject, bool bStyleSheet1, bool bSaveText);

    virtual void Undo() override;
    virtual void Redo() override;

private:
    ::unotools::WeakReference<SdPage> mxPage;
    ::tools::WeakReference<SdrObject> mxSdrObject;
};

} // namespace sd

#endif // INCLUDED_SD_INC_UNDO_UNDOOBJECTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
