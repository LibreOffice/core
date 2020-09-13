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
#pragma once

#include <GeneralUndo.hxx>
#include <strings.hrc>
#include "SelectionBrowseBox.hxx"
#include <osl/diagnose.h>

namespace dbaui
{
   // OQueryDesignFieldUndoAct - Basisclass for undo's in the fieldlist of a query design

    class OQueryDesignFieldUndoAct : public OCommentUndoAction
    {
    protected:
        VclPtr<OSelectionBrowseBox>    pOwner;
        sal_uInt16                  m_nColumnPosition;

        virtual void    Undo() override = 0;
        virtual void    Redo() override = 0;

    public:
        OQueryDesignFieldUndoAct(OSelectionBrowseBox* pSelBrwBox, const char* pCommentID);
        virtual ~OQueryDesignFieldUndoAct() override;

        void SetColumnPosition(sal_uInt16 _nColumnPosition)
        {
            m_nColumnPosition = _nColumnPosition;
            OSL_ENSURE(m_nColumnPosition != BROWSER_INVALIDID,"Column position was not set add the undo action!");
            OSL_ENSURE(m_nColumnPosition < pOwner->GetColumnCount(),"Position outside the column count!");
        }
    };

    // OTabFieldCellModifiedUndoAct - undo class to change a line of the column description

    class OTabFieldCellModifiedUndoAct final : public OQueryDesignFieldUndoAct
    {
        OUString    m_strNextCellContents;
        sal_Int32   m_nCellIndex;

    public:
        explicit OTabFieldCellModifiedUndoAct(OSelectionBrowseBox* pSelBrwBox)
            : OQueryDesignFieldUndoAct(pSelBrwBox, STR_QUERY_UNDO_MODIFY_CELL)
            ,m_nCellIndex(BROWSER_INVALIDID){ }

        void SetCellContents(const OUString& str)  { m_strNextCellContents = str; }
        void SetCellIndex(sal_Int32 nIndex)      { m_nCellIndex = nIndex; }

        virtual void Undo() override;
        virtual void Redo() override { Undo(); }
    };

    // OTabFieldSizedUndoAct - undo class to change the column width

    class OTabFieldSizedUndoAct final : public OQueryDesignFieldUndoAct
    {
        long        m_nNextWidth;

    public:
        explicit OTabFieldSizedUndoAct(OSelectionBrowseBox* pSelBrwBox) : OQueryDesignFieldUndoAct(pSelBrwBox, STR_QUERY_UNDO_SIZE_COLUMN), m_nNextWidth(0) { }

        void SetOriginalWidth(long nWidth) { m_nNextWidth = nWidth; }

        virtual void Undo() override;
        virtual void Redo() override { Undo(); }
    };

    // OTabFieldUndoAct - base class for undos in the fieldlist of a query design, which are used to change complete field descriptions

    class OTabFieldUndoAct : public OQueryDesignFieldUndoAct
    {
    protected:
        OTableFieldDescRef      pDescr;     // the deleted column description

    public:
        OTabFieldUndoAct(OSelectionBrowseBox* pSelBrwBox, const char* pCommentID) : OQueryDesignFieldUndoAct(pSelBrwBox, pCommentID) { }

        void SetTabFieldDescr(OTableFieldDescRef const & pDescription) { pDescr = pDescription; }
    };

    // OTabFieldDelUndoAct - undo class to delete a field

    class OTabFieldDelUndoAct : public OTabFieldUndoAct
    {
    protected:
        virtual void Undo() override { pOwner->EnterUndoMode();pOwner->InsertColumn(pDescr, m_nColumnPosition);pOwner->LeaveUndoMode(); }
        virtual void Redo() override { pOwner->EnterUndoMode();pOwner->RemoveColumn(pDescr->GetColumnId());pOwner->LeaveUndoMode(); }

    public:
        explicit OTabFieldDelUndoAct(OSelectionBrowseBox* pSelBrwBox) : OTabFieldUndoAct(pSelBrwBox, STR_QUERY_UNDO_TABFIELDDELETE) { }
    };

    // OTabFieldCreateUndoAct - undo class for creating a field

    class OTabFieldCreateUndoAct : public OTabFieldUndoAct
    {
    protected:
        virtual void Undo() override { pOwner->EnterUndoMode();pOwner->RemoveColumn(pDescr->GetColumnId());pOwner->LeaveUndoMode();}
        virtual void Redo() override { pOwner->EnterUndoMode();pOwner->InsertColumn(pDescr, m_nColumnPosition);pOwner->LeaveUndoMode();}

    public:
        explicit OTabFieldCreateUndoAct(OSelectionBrowseBox* pSelBrwBox) : OTabFieldUndoAct(pSelBrwBox, STR_QUERY_UNDO_TABFIELDCREATE) { }
    };

    // OTabFieldMovedUndoAct - Undo class when a field was moved inside the selection

    class OTabFieldMovedUndoAct : public OTabFieldUndoAct
    {
    protected:
        virtual void Undo() override;
        virtual void Redo() override
        {
            Undo();
        }

    public:
        explicit OTabFieldMovedUndoAct(OSelectionBrowseBox* pSelBrwBox) : OTabFieldUndoAct(pSelBrwBox, STR_QUERY_UNDO_TABFIELDMOVED) { }
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
