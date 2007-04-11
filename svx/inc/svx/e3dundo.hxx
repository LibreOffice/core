/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: e3dundo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:46:00 $
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

#ifndef _E3D_UNDO_HXX
#define _E3D_UNDO_HXX

#ifndef _SVDUNDO_HXX
#include <svx/svdundo.hxx>
#endif
class E3DObject;
#ifndef _E3D_SCENE3D_HXX
#include <svx/scene3d.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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
        TYPEINFO();
        E3dUndoAction (SdrModel  *pModel,
                       E3dObject *p3DObj) :
            SdrUndoAction (*pModel),
            pMy3DObj (p3DObj)
            {
            }

        virtual ~E3dUndoAction ();

        virtual BOOL CanRepeat(SfxRepeatTarget&) const;
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
|* Undo fuer 3D-Attribute (Implementiert ueber Set3DAttributes())
|*
\************************************************************************/
class SVX_DLLPUBLIC E3dAttributesUndoAction : public SdrUndoAction
{
    using SdrUndoAction::Repeat;

    SdrObject*  pObject;
    E3dView*    pView;
    BOOL        bUseSubObjects;

    const SfxItemSet aNewSet;
    const SfxItemSet aOldSet;

 public:
        TYPEINFO();
        E3dAttributesUndoAction( SdrModel &rModel, E3dView* pView,
            E3dObject* pInObject,
            const SfxItemSet& rNewSet,
            const SfxItemSet& rOldSet,
            BOOL bUseSubObj);

        virtual ~E3dAttributesUndoAction();

        virtual BOOL CanRepeat(SfxRepeatTarget& rView) const;
        virtual void Undo();
        virtual void Redo();
        virtual void Repeat();
};

#endif          // _E3D_CUBE3D_HXX
