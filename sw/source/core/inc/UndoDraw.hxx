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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNDODRAW_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNDODRAW_HXX

#include <undobj.hxx>
#include <svx/svdundo.hxx>

struct SwUndoGroupObjImpl;
class SdrMark;
class SdrMarkList;
class SdrObject;
class SdrObjGroup;
class SdrUndoAction;
class SwDrawFrmFmt;

// Undo for Draw Objects
class SwSdrUndo : public SwUndo
{
    SdrUndoAction* pSdrUndo;
    SdrMarkList* pMarkList; // MarkList for all selected SdrObjects

public:
    SwSdrUndo( SdrUndoAction* , const SdrMarkList* pMarkList );

    virtual ~SwSdrUndo();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;
};

class SwUndoDrawGroup : public SwUndo
{
    SwUndoGroupObjImpl* pObjArr;
    sal_uInt16 nSize;
    bool bDelFmt;

public:
    SwUndoDrawGroup( sal_uInt16 nCnt );

    virtual ~SwUndoDrawGroup();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;

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
    bool bDelFmt;

public:
    SwUndoDrawUnGroup( SdrObjGroup* );

    virtual ~SwUndoDrawUnGroup();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;

    void AddObj( sal_uInt16 nPos, SwDrawFrmFmt* );
};

class SwUndoDrawUnGroupConnectToLayout : public SwUndo
{
private:
    std::vector< std::pair< SwDrawFrmFmt*, SdrObject* > > aDrawFmtsAndObjs;

public:
    SwUndoDrawUnGroupConnectToLayout();

    virtual ~SwUndoDrawUnGroupConnectToLayout();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;

    void AddFmtAndObj( SwDrawFrmFmt* pDrawFrmFmt,
                       SdrObject* pDrawObject );
};

class SwUndoDrawDelete : public SwUndo
{
    SwUndoGroupObjImpl* pObjArr;
    SdrMarkList* pMarkLst;  // MarkList for all selected SdrObjects
    sal_uInt16 nSize;
    bool bDelFmt;

public:
    SwUndoDrawDelete( sal_uInt16 nCnt );

    virtual ~SwUndoDrawDelete();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;

    void AddObj( sal_uInt16 nPos, SwDrawFrmFmt*, const SdrMark& );
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDODRAW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
