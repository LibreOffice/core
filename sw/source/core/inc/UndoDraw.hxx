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

#ifndef SW_UNDO_DRAW_HXX
#define SW_UNDO_DRAW_HXX

#include <undobj.hxx>

#include <svx/svdundo.hxx>


struct SwUndoGroupObjImpl;
class SdrMark;
class SdrMarkList;
class SdrObject;
class SdrObjGroup;
class SdrUndoAction;
class SwDrawFrmFmt;

//--------------------------------------------------------------------
// ----------- Undo for Draw Objects ---------------------------------

class SwSdrUndo : public SwUndo
{
    SdrUndoAction* pSdrUndo;
    SdrMarkList* pMarkList; // MarkList for all selected SdrObjects
public:
    SwSdrUndo( SdrUndoAction* , const SdrMarkList* pMarkList );

    virtual ~SwSdrUndo();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    String GetComment() const;
};

class SwUndoDrawGroup : public SwUndo
{
    SwUndoGroupObjImpl* pObjArr;
    sal_uInt16 nSize;
    sal_Bool bDelFmt;

public:
    SwUndoDrawGroup( sal_uInt16 nCnt );

    virtual ~SwUndoDrawGroup();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    void AddObj( sal_uInt16 nPos, SwDrawFrmFmt*, SdrObject* );
    void SetGroupFmt( SwDrawFrmFmt* );
};

// Action "ungroup drawing object" is now splitted into three parts - see
// method <SwDoc::UnGroupSelection(..)>:
// - creation for <SwDrawFrmFmt> instances for the group members of the
//   selected group objects
// - intrinsic ungroup of the selected group objects
// - creation of <SwDrawContact> instances for the former group members and
//   connection to the Writer layout.
// Thus, two undo actions (instances of <SwUndo>) are needed:
// - Existing class <SwUndoDrawUnGroup> takes over the part for the formats.
// - New class <SwUndoDrawUnGroupConnectToLayout> takes over the part for
//   contact object.
class SwUndoDrawUnGroup : public SwUndo
{
    SwUndoGroupObjImpl* pObjArr;
    sal_uInt16 nSize;
    sal_Bool bDelFmt;

public:
    SwUndoDrawUnGroup( SdrObjGroup* );

    virtual ~SwUndoDrawUnGroup();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    void AddObj( sal_uInt16 nPos, SwDrawFrmFmt* );
};

class SwUndoDrawUnGroupConnectToLayout : public SwUndo
{
    private:
        std::vector< std::pair< SwDrawFrmFmt*, SdrObject* > > aDrawFmtsAndObjs;

    public:
        SwUndoDrawUnGroupConnectToLayout();

        virtual ~SwUndoDrawUnGroupConnectToLayout();

        virtual void UndoImpl( ::sw::UndoRedoContext & );
        virtual void RedoImpl( ::sw::UndoRedoContext & );

        void AddFmtAndObj( SwDrawFrmFmt* pDrawFrmFmt,
                           SdrObject* pDrawObject );
};

class SwUndoDrawDelete : public SwUndo
{
    SwUndoGroupObjImpl* pObjArr;
    SdrMarkList* pMarkLst;  // MarkList for all selected SdrObjects
    sal_uInt16 nSize;
    sal_Bool bDelFmt;

public:
    SwUndoDrawDelete( sal_uInt16 nCnt );

    virtual ~SwUndoDrawDelete();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    void AddObj( sal_uInt16 nPos, SwDrawFrmFmt*, const SdrMark& );
};

#endif // SW_UNDO_DRAW_HXX

