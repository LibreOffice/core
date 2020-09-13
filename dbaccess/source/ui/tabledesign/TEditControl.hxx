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

#include <TableDesignControl.hxx>
#include <TableDesignView.hxx>
#include "TableFieldDescWin.hxx"
#include <TableRow.hxx>
#include <TypeInfo.hxx>

class Edit;
class SfxUndoManager;
namespace dbaui
{
    class OSQLNameEditControl;

    class OTableEditorCtrl : public OTableRowView
    {
        enum ChildFocusState
        {
            HELPTEXT,
            DESCRIPTION,
            NAME,
            ROW,
            NONE
        };

        std::vector< std::shared_ptr<OTableRow> >*    m_pRowList;

        VclPtr<OTableDesignView>           m_pView;
        VclPtr<OSQLNameEditControl>        pNameCell;
        VclPtr<::svt::ListBoxControl>      pTypeCell;
        VclPtr<::svt::EditControl>         pHelpTextCell;
        VclPtr<::svt::EditControl>         pDescrCell;
        OTableFieldDescWin*                pDescrWin;          // properties of one column

         std::shared_ptr<OTableRow> pActRow;

        ImplSVEvent *               nCutEvent;
        ImplSVEvent *               nPasteEvent;
        ImplSVEvent *               nDeleteEvent;
        ImplSVEvent *               nInsNewRowsEvent;
        ImplSVEvent *               nInvalidateTypeEvent;
        ChildFocusState             m_eChildFocus;

        long nOldDataPos;

        bool bReadOnly;

        // helper class
        class ClipboardInvalidator final
        {
        private:
            AutoTimer m_aInvalidateTimer;
            VclPtr<OTableEditorCtrl> m_pOwner;

        public:
            explicit ClipboardInvalidator(OTableEditorCtrl*);
            ~ClipboardInvalidator();
            void Stop();

        private:
            DECL_LINK(OnInvalidate, Timer*, void);
        };

        friend class OTableEditorCtrl::ClipboardInvalidator;

        ClipboardInvalidator m_aInvalidate;

    protected:
        virtual void Command( const CommandEvent& rEvt ) override;
        virtual bool SeekRow(long nRow) override;
        virtual void PaintCell(OutputDevice& rDev, const tools::Rectangle& rRect,
                               sal_uInt16 nColumnId ) const override;

        virtual void CursorMoved() override;
        virtual RowStatus GetRowStatus(long nRow) const override;

        virtual ::svt::CellController* GetController(long nRow, sal_uInt16 nCol) override;
        virtual void InitController(::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol) override;

        virtual void CellModified() override;
        virtual bool SaveModified() override; // is called before changing a cell (false prevents change)

        virtual OUString GetCellText(long nRow, sal_uInt16 nColId) const override;
        virtual sal_uInt32 GetTotalCellWidth(long nRow, sal_uInt16 nColId) override;

        virtual void CopyRows() override;
        virtual void InsertRows( long nRow ) override;
        virtual void DeleteRows() override;
        virtual void InsertNewRows( long nRow ) override;

        virtual bool IsPrimaryKeyAllowed() override;
        virtual bool IsInsertNewAllowed( long nRow ) override;
        virtual bool IsDeleteAllowed() override;

        void ClearModified();

        void SetPrimaryKey( bool bSet );
        bool IsPrimaryKey();

    public:
        explicit OTableEditorCtrl(vcl::Window* pParentWin, OTableDesignView* pView);
        virtual ~OTableEditorCtrl() override;
        virtual void dispose() override;
        virtual bool CursorMoving(long nNewRow, sal_uInt16 nNewCol) override;
        SfxUndoManager& GetUndoManager() const;

        void SetDescrWin( OTableFieldDescWin* pWin )
        {
            pDescrWin = pWin;
            if (pDescrWin && pActRow)
                pDescrWin->DisplayData(pActRow->GetActFieldDescr());
        }
        void SaveCurRow();
        void SwitchType( const TOTypeInfoSP& _pType );

        /// force displaying of the given row
        void DisplayData( long nRow );

        virtual void SetCellData( long nRow, sal_uInt16 nColId, const TOTypeInfoSP& _pTypeInfo ) override;
        virtual void SetCellData( long nRow, sal_uInt16 nColId, const css::uno::Any& _rSaveData ) override;
        virtual css::uno::Any  GetCellData( long nRow, sal_uInt16 nColId ) override;
        virtual void SetControlText( long nRow, sal_uInt16 nColId, const OUString& rText ) override;

        virtual OTableDesignView* GetView() const override;

        std::vector< std::shared_ptr<OTableRow> >* GetRowList(){ return m_pRowList; }

        const std::shared_ptr<OTableRow>& GetActRow() const { return pActRow; }
        void CellModified( long nRow, sal_uInt16 nColId );
        void SetReadOnly( bool bRead );

        virtual void Init() override;
        virtual void DeactivateCell(bool bUpdate = true) override;

        bool IsCutAllowed();
        bool IsCopyAllowed();
        bool IsPasteAllowed() const;
        bool IsReadOnly() const { return bReadOnly;}
        OFieldDescription* GetFieldDescr( long nRow );

        // Window overrides
        virtual bool PreNotify( NotifyEvent& rNEvt ) override;

        // IClipboardTest
        virtual bool isCutAllowed() override { return IsCutAllowed(); }
        virtual bool isCopyAllowed() override { return IsCopyAllowed(); }
        virtual bool isPasteAllowed() override { return IsPasteAllowed(); }

        virtual void cut() override;
        virtual void copy() override;
        virtual void paste() override;

    private:
        DECL_LINK( DelayedCut, void*, void );
        DECL_LINK( DelayedPaste, void*, void );
        DECL_LINK( DelayedDelete, void*, void );
        DECL_LINK( DelayedInsNewRows, void*, void );
        DECL_LINK( InvalidateFieldType, void*, void );

        void InitCellController();
        sal_Int32 HasFieldName( const OUString& rFieldName );
        OUString GenerateName( const OUString& rName );
        bool SetDataPtr( long nRow );

        void SaveData(long nRow, sal_uInt16 nColumnId);
        /** AdjustFieldDescription set the needed values for the description
            @param  _pFieldDesc     the field description where to set the values
            @param  _rMultiSel      contains the positions which changed for undo/redo
            @param  _nPos           the current position
            @param  _bSet           should a key be set
            @param  _bPrimaryKey    which value should the pkey have
        */
        void AdjustFieldDescription( OFieldDescription* _pFieldDesc,
                                     MultiSelection& _rMultiSel,
                                     sal_Int32 _nPos,
                                     bool _bSet,
                                     bool _bPrimaryKey);
        /** InvalidateFeatures invalidates the slots SID_UNDO | SID_REDO | SID_SAVEDOC
        */
        void InvalidateFeatures();

        void resetType();
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
