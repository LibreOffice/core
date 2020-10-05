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
#include <memory>

struct SwUndoGroupObjImpl;
class SdrMark;
class SdrMarkList;
class SdrObject;
class SdrObjGroup;
class SdrUndoAction;
class SwDrawFrameFormat;
class SwDoc;

// Undo for Draw Objects
class SwSdrUndo : public SwUndo
{
    std::unique_ptr<SdrUndoAction> m_pSdrUndo;
    std::unique_ptr<SdrMarkList> m_pMarkList; // MarkList for all selected SdrObjects

public:
    SwSdrUndo( std::unique_ptr<SdrUndoAction> , const SdrMarkList* pMarkList, const SwDoc& rDoc );

    virtual ~SwSdrUndo() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    virtual OUString GetComment() const override;
};

class SwUndoDrawGroup : public SwUndo
{
    std::unique_ptr<SwUndoGroupObjImpl[]> m_pObjArray;
    sal_uInt16 m_nSize;
    bool m_bDeleteFormat;

public:
    SwUndoDrawGroup( sal_uInt16 nCnt, const SwDoc& rDoc );

    virtual ~SwUndoDrawGroup() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void AddObj( sal_uInt16 nPos, SwDrawFrameFormat*, SdrObject* );
    void SetGroupFormat( SwDrawFrameFormat* );
};

// Action "ungroup drawing object" is now split into three parts - see
// method <SwDoc::UnGroupSelection(..)>:
// - creation for <SwDrawFrameFormat> instances for the group members of the
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
    std::unique_ptr<SwUndoGroupObjImpl[]> m_pObjArray;
    sal_uInt16 m_nSize;
    bool m_bDeleteFormat;

public:
    SwUndoDrawUnGroup( SdrObjGroup*, const SwDoc& rDoc );

    virtual ~SwUndoDrawUnGroup() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void AddObj( sal_uInt16 nPos, SwDrawFrameFormat* );
};

class SwUndoDrawUnGroupConnectToLayout : public SwUndo
{
private:
    std::vector< std::pair< SwDrawFrameFormat*, SdrObject* > > m_aDrawFormatsAndObjs;

public:
    SwUndoDrawUnGroupConnectToLayout(const SwDoc& rDoc);

    virtual ~SwUndoDrawUnGroupConnectToLayout() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void AddFormatAndObj( SwDrawFrameFormat* pDrawFrameFormat,
                       SdrObject* pDrawObject );
};

class SwUndoDrawDelete : public SwUndo
{
    std::unique_ptr<SwUndoGroupObjImpl[]> m_pObjArray;
    std::unique_ptr<SdrMarkList> m_pMarkList;  // MarkList for all selected SdrObjects
    bool m_bDeleteFormat;

public:
    SwUndoDrawDelete( sal_uInt16 nCnt, const SwDoc& rDoc );

    virtual ~SwUndoDrawDelete() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void AddObj( SwDrawFrameFormat*, const SdrMark& );
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDODRAW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
