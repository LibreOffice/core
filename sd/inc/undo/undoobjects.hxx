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

#ifndef _SD_UNDOOBJECTS_HXX
#define _SD_UNDOOBJECTS_HXX

#include <svx/svdundo.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include "pres.hxx"

class SdrObjUserCall;
class SdPage;

namespace sd
{
    class UndoManager;

///////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////

class UndoRemoveObject : public SdrUndoRemoveObj, public UndoRemovePresObjectImpl
{
public:
    UndoRemoveObject( SdrObject& rObject, bool bOrdNumDirect );

    virtual void Undo();
    virtual void Redo();

private:
    SdrObjectWeakRef mxSdrObject;
};

///////////////////////////////////////////////////////////////////////

class UndoDeleteObject : public SdrUndoDelObj, public UndoRemovePresObjectImpl
{
public:
    UndoDeleteObject( SdrObject& rObject, bool bOrdNumDirect );

    virtual void Undo();
    virtual void Redo();

private:
    SdrObjectWeakRef mxSdrObject;
};

///////////////////////////////////////////////////////////////////////

class UndoReplaceObject : public SdrUndoReplaceObj, public UndoRemovePresObjectImpl
{
public:
    UndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject, bool bOrdNumDirect );

    virtual void Undo();
    virtual void Redo();

private:
    SdrObjectWeakRef mxSdrObject;
};

///////////////////////////////////////////////////////////////////////

class UndoObjectSetText : public SdrUndoObjSetText
{
public:
    UndoObjectSetText( SdrObject& rNewObj, sal_Int32 nText );
    virtual ~UndoObjectSetText();

    virtual void Undo();
    virtual void Redo();

private:
    SfxUndoAction* mpUndoAnimation;
    bool            mbNewEmptyPresObj;
    SdrObjectWeakRef mxSdrObject;
};

//////////////////////////////////////////////////////////////////////////////
// Undo for SdrObject::SetUserCall()

class UndoObjectUserCall : public SdrUndoObj
{
public:
    UndoObjectUserCall(SdrObject& rNewObj);

    virtual void Undo();
    virtual void Redo();

protected:
    SdrObjUserCall* mpOldUserCall;
    SdrObjUserCall* mpNewUserCall;
    SdrObjectWeakRef mxSdrObject;
};

//////////////////////////////////////////////////////////////////////////////
// Undo for SdPage::InsertPresObj() and SdPage::RemovePresObj()

class UndoObjectPresentationKind : public SdrUndoObj
{
public:
    UndoObjectPresentationKind(SdrObject& rObject);

    virtual void Undo();
    virtual void Redo();

protected:
    PresObjKind meOldKind;
    PresObjKind meNewKind;
    SdrPageWeakRef mxPage;
    SdrObjectWeakRef mxSdrObject;
};

//////////////////////////////////////////////////////////////////////////////
// Restores correct position and size for presentation shapes with user call
// on undo

class UndoAutoLayoutPosAndSize : public SfxUndoAction
{
public:
    UndoAutoLayoutPosAndSize( SdPage& rPage );

    virtual void Undo();
    virtual void Redo();

protected:
    SdrPageWeakRef mxPage;
};

//////////////////////////////////////////////////////////////////////////////

class UndoGeoObject : public SdrUndoGeoObj
{
public:
    UndoGeoObject( SdrObject& rNewObj );

    virtual void Undo();
    virtual void Redo();

protected:
    SdrPageWeakRef mxPage;
    SdrObjectWeakRef mxSdrObject;
};

//////////////////////////////////////////////////////////////////////////////

class UndoAttrObject : public SdrUndoAttrObj
{
public:
    UndoAttrObject( SdrObject& rObject, bool bStyleSheet1, bool bSaveText );

    virtual void Undo();
    virtual void Redo();

protected:
    SdrPageWeakRef mxPage;
    SdrObjectWeakRef mxSdrObject;
};

} // namespace sd

#endif     // _SD_UNDOOBJECTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
