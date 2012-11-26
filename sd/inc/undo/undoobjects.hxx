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



#ifndef _SD_UNDOOBJECTS_HXX
#define _SD_UNDOOBJECTS_HXX

#include <svx/svdundo.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include "pres.hxx"

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
    SfxUndoAction*  mpUndoSdPage;
    SfxUndoAction*  mpUndoAnimation;
    SfxUndoAction*  mpUndoPresObj;
};

///////////////////////////////////////////////////////////////////////

class UndoRemoveObject : public SdrUndoRemoveObj, public UndoRemovePresObjectImpl
{
public:
    UndoRemoveObject( SdrObject& rObject );

    virtual void Undo();
    virtual void Redo();

private:
    SdrObjectWeakRef mxSdrObject;
};

///////////////////////////////////////////////////////////////////////

class UndoDeleteObject : public SdrUndoDelObj, public UndoRemovePresObjectImpl
{
public:
    UndoDeleteObject( SdrObject& rObject );

    virtual void Undo();
    virtual void Redo();

private:
    SdrObjectWeakRef mxSdrObject;
};

///////////////////////////////////////////////////////////////////////

class UndoReplaceObject : public SdrUndoReplaceObj, public UndoRemovePresObjectImpl
{
public:
    UndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject );

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
// Undo for connecting SdrObject to SdPage

class UndoConnectionToSdrObject : public SdrUndoObj
{
public:
    UndoConnectionToSdrObject(SdrObject& rNewObj);

    virtual void Undo();
    virtual void Redo();

protected:
    SdPage* mpOldSdPage;
    SdPage* mpNewSdPage;
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
