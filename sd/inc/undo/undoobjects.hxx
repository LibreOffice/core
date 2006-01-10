/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: undoobjects.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-01-10 14:24:25 $
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

#ifndef _SD_UNDOOBJECTS_HXX
#define _SD_UNDOOBJECTS_HXX

#ifndef _SVDUNDO_HXX
#include <svx/svdundo.hxx>
#endif
#ifndef _SDR_OBJECTUSER_HXX
#include <svx/sdrobjectuser.hxx>
#endif

#ifndef _PRESENTATION_HXX
#include "pres.hxx"
#endif

class SdrObjUserCall;
class SdPage;

namespace sd
{
    class UndoManager;

///////////////////////////////////////////////////////////////////////

class UndoShapeWatcher : public sdr::ObjectUser
{
public:
    UndoShapeWatcher( SdrObject& rObject );
    ~UndoShapeWatcher();

protected:
    SdrObject*  mpObjectSafe;

private:
    virtual void ObjectInDestruction(const SdrObject& rObject);
};

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

class UndoRemoveObject : public SdrUndoRemoveObj, public UndoRemovePresObjectImpl, public UndoShapeWatcher
{
public:
    UndoRemoveObject( SdrObject& rObject, bool bOrdNumDirect );

    virtual void Undo();
    virtual void Redo();
};

///////////////////////////////////////////////////////////////////////

class UndoDeleteObject : public SdrUndoDelObj, public UndoRemovePresObjectImpl, public UndoShapeWatcher
{
public:
    UndoDeleteObject( SdrObject& rObject, bool bOrdNumDirect );

    virtual void Undo();
    virtual void Redo();
};

///////////////////////////////////////////////////////////////////////

class UndoReplaceObject : public SdrUndoReplaceObj, public UndoRemovePresObjectImpl, public UndoShapeWatcher
{
public:
    UndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject, bool bOrdNumDirect );

    virtual void Undo();
    virtual void Redo();
};

///////////////////////////////////////////////////////////////////////

class UndoObjectSetText : public SdrUndoObjSetText, public UndoShapeWatcher
{
public:
    UndoObjectSetText( SdrObject& rNewObj );
    virtual ~UndoObjectSetText();

    virtual void Undo();
    virtual void Redo();

private:
    SfxUndoAction* mpUndoAnimation;
    bool            mbNewEmptyPresObj;
};

//////////////////////////////////////////////////////////////////////////////
// Undo for SdrObject::SetUserCall()

class UndoObjectUserCall : public SdrUndoObj, public UndoShapeWatcher
{
public:
    UndoObjectUserCall(SdrObject& rNewObj);

    virtual void Undo();
    virtual void Redo();

protected:
    SdrObjUserCall* mpOldUserCall;
    SdrObjUserCall* mpNewUserCall;
};

//////////////////////////////////////////////////////////////////////////////
// Undo for SdPage::InsertPresObj() and SdPage::RemovePresObj()

class UndoObjectPresentationKind : public SdrUndoObj, public UndoShapeWatcher
{
public:
    UndoObjectPresentationKind(SdrObject& rObject);

    virtual void Undo();
    virtual void Redo();

protected:
    PresObjKind meOldKind;
    PresObjKind meNewKind;
    SdPage*     mpPage;
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
    SdPage& mrPage;
};

//////////////////////////////////////////////////////////////////////////////

class UndoGeoObject : public SdrUndoGeoObj, public UndoShapeWatcher
{
public:
    UndoGeoObject( SdrObject& rNewObj );

    virtual void Undo();
    virtual void Redo();

protected:
    SdPage* mpPage;
};

//////////////////////////////////////////////////////////////////////////////

class UndoAttrObject : public SdrUndoAttrObj, public UndoShapeWatcher
{
public:
    UndoAttrObject( SdrObject& rObject, bool bStyleSheet1, bool bSaveText );

    virtual void Undo();
    virtual void Redo();

protected:
    SdPage* mpPage;
};

} // namespace sd

#endif     // _SD_UNDOOBJECTS_HXX
