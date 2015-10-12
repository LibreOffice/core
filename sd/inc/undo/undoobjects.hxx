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

#include <svx/svdundo.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include "pres.hxx"

class SdrObjUserCall;
class SdPage;

namespace sd
{

class UndoRemovePresObjectImpl
{
protected:
    UndoRemovePresObjectImpl( SdrObject& rObject );
    virtual ~UndoRemovePresObjectImpl();

    virtual void Undo();
    virtual void Redo();

private:
    SfxUndoAction*  mpUndoUsercall;
    SfxUndoAction*  mpUndoAnimation;
    SfxUndoAction*  mpUndoPresObj;
};

class UndoRemoveObject : public SdrUndoRemoveObj, public UndoRemovePresObjectImpl
{
public:
    UndoRemoveObject( SdrObject& rObject, bool bOrdNumDirect );

    virtual void Undo() override;
    virtual void Redo() override;

private:
    SdrObjectWeakRef mxSdrObject;
};

class UndoDeleteObject : public SdrUndoDelObj, public UndoRemovePresObjectImpl
{
public:
    UndoDeleteObject( SdrObject& rObject, bool bOrdNumDirect );

    virtual void Undo() override;
    virtual void Redo() override;

private:
    SdrObjectWeakRef mxSdrObject;
};

class UndoReplaceObject : public SdrUndoReplaceObj, public UndoRemovePresObjectImpl
{
public:
    UndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject, bool bOrdNumDirect );

    virtual void Undo() override;
    virtual void Redo() override;

private:
    SdrObjectWeakRef mxSdrObject;
};

class UndoObjectSetText : public SdrUndoObjSetText
{
public:
    UndoObjectSetText( SdrObject& rNewObj, sal_Int32 nText );
    virtual ~UndoObjectSetText();

    virtual void Undo() override;
    virtual void Redo() override;

private:
    SfxUndoAction* mpUndoAnimation;
    bool            mbNewEmptyPresObj;
    SdrObjectWeakRef mxSdrObject;
};

// Undo for SdrObject::SetUserCall()

class UndoObjectUserCall : public SdrUndoObj
{
public:
    UndoObjectUserCall(SdrObject& rNewObj);

    virtual void Undo() override;
    virtual void Redo() override;

protected:
    SdrObjUserCall* mpOldUserCall;
    SdrObjUserCall* mpNewUserCall;
    SdrObjectWeakRef mxSdrObject;
};

// Undo for SdPage::InsertPresObj() and SdPage::RemovePresObj()

class UndoObjectPresentationKind : public SdrUndoObj
{
public:
    UndoObjectPresentationKind(SdrObject& rObject);

    virtual void Undo() override;
    virtual void Redo() override;

protected:
    PresObjKind meOldKind;
    PresObjKind meNewKind;
    SdrPageWeakRef mxPage;
    SdrObjectWeakRef mxSdrObject;
};

// Restores correct position and size for presentation shapes with user call
// on undo

class UndoAutoLayoutPosAndSize : public SfxUndoAction
{
public:
    UndoAutoLayoutPosAndSize( SdPage& rPage );

    virtual void Undo() override;
    virtual void Redo() override;

protected:
    SdrPageWeakRef mxPage;
};

class UndoGeoObject : public SdrUndoGeoObj
{
public:
    UndoGeoObject( SdrObject& rNewObj );

    virtual void Undo() override;
    virtual void Redo() override;

protected:
    SdrPageWeakRef mxPage;
    SdrObjectWeakRef mxSdrObject;
};

class UndoAttrObject : public SdrUndoAttrObj
{
public:
    UndoAttrObject( SdrObject& rObject, bool bStyleSheet1, bool bSaveText );

    virtual void Undo() override;
    virtual void Redo() override;

protected:
    SdrPageWeakRef mxPage;
    SdrObjectWeakRef mxSdrObject;
};

} // namespace sd

#endif // INCLUDED_SD_INC_UNDO_UNDOOBJECTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
