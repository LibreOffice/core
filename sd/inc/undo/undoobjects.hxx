/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: undoobjects.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:23:25 $
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
