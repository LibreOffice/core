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
#ifndef DBAUI_TABLEEDITORCONTROL_HXX
#define DBAUI_TABLEEDITORCONTROL_HXX

#include "TableDesignControl.hxx"
#include "TableDesignView.hxx"
#include "TableFieldDescWin.hxx"
#include "TableRow.hxx"
#include "QEnumTypes.hxx"
#include "TypeInfo.hxx"

class Edit;
class SfxUndoManager;
namespace dbaui
{
    class OSQLNameEdit;

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

        ::std::vector< ::boost::shared_ptr<OTableRow> > m_aUndoList;
        ::std::vector< ::boost::shared_ptr<OTableRow> >*    m_pRowList;

        OSQLNameEdit*               pNameCell;
        ::svt::ListBoxControl*      pTypeCell;
        Edit*                       pHelpTextCell;
        Edit*                       pDescrCell;
        OTableFieldDescWin*         pDescrWin;          // properties of one column

         ::boost::shared_ptr<OTableRow> pActRow;

        sal_uLong                       nIndexEvent;
        sal_uLong                       nCutEvent;
        sal_uLong                       nPasteEvent;
        sal_uLong                       nDeleteEvent;
        sal_uLong                       nInsNewRowsEvent;
        sal_uLong                       nInvalidateTypeEvent;
        ChildFocusState             m_eChildFocus;

        long nOldDataPos;

        sal_Bool bSaveOnMove;
        sal_Bool bReadOnly;

        // helper class
        class ClipboardInvalidator
        {
        private:
            AutoTimer m_aInvalidateTimer;
            OTableEditorCtrl* m_pOwner;

        public:
            ClipboardInvalidator(sal_uLong nTimeout,OTableEditorCtrl*);
            ~ClipboardInvalidator();

        protected:
            DECL_LINK(OnInvalidate, void*);
        };

        friend class OTableEditorCtrl::ClipboardInvalidator;

        ClipboardInvalidator m_aInvalidate;

    protected:
        virtual void Command( const CommandEvent& rEvt );
        virtual sal_Bool SeekRow(long nRow);
        virtual void PaintCell(OutputDevice& rDev, const Rectangle& rRect,
                               sal_uInt16 nColumnId ) const;

        virtual void CursorMoved();
        virtual RowStatus GetRowStatus(long nRow) const;

        virtual ::svt::CellController* GetController(long nRow, sal_uInt16 nCol);
        virtual void InitController(::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol);

        virtual void CellModified();
        virtual sal_Bool SaveModified(); // is called before changing a cell (false prevents change)

        virtual void Undo();
        virtual void Redo();
        virtual OUString GetCellText(long nRow, sal_uInt16 nColId) const;
        virtual sal_uInt32 GetTotalCellWidth(long nRow, sal_uInt16 nColId);

        virtual void CopyRows();
        virtual void InsertRows( long nRow );
        virtual void DeleteRows();
        virtual void InsertNewRows( long nRow );

        virtual sal_Bool IsPrimaryKeyAllowed( long nRow );
        virtual sal_Bool IsInsertNewAllowed( long nRow );
        virtual sal_Bool IsDeleteAllowed( long nRow );

        void ClearModified();

        void SetPrimaryKey( sal_Bool bSet );
        sal_Bool IsPrimaryKey();

        DECL_LINK(ControlPreNotifyHdl, NotifyEvent*);

    public:
        OTableEditorCtrl(Window* pParentWin);
        virtual ~OTableEditorCtrl();
        virtual sal_Bool CursorMoving(long nNewRow, sal_uInt16 nNewCol);
        virtual void UpdateAll();
        SfxUndoManager& GetUndoManager() const;

        void SetDescrWin( OTableFieldDescWin* pWin )
        {
            pDescrWin = pWin;
            if (pDescrWin && pActRow)
                pDescrWin->DisplayData(pActRow->GetActFieldDescr());
        }
        sal_Bool SaveCurRow();
        void SwitchType( const TOTypeInfoSP& _pType );

        /// force displaying of the given row
        void DisplayData( long nRow, sal_Bool bGrabFocus = sal_True );

        virtual void SetCellData( long nRow, sal_uInt16 nColId, const TOTypeInfoSP& _pTypeInfo );
        virtual void SetCellData( long nRow, sal_uInt16 nColId, const ::com::sun::star::uno::Any& _rSaveData );
        virtual ::com::sun::star::uno::Any  GetCellData( long nRow, sal_uInt16 nColId );
        virtual void SetControlText( long nRow, sal_uInt16 nColId, const String& rText );
        virtual String GetControlText( long nRow, sal_uInt16 nColId );

        virtual OTableDesignView* GetView() const;

        ::std::vector< ::boost::shared_ptr<OTableRow> >* GetRowList(){ return m_pRowList; }

        ::boost::shared_ptr<OTableRow>         GetActRow(){ return pActRow; }
        void CellModified( long nRow, sal_uInt16 nColId );
        void SetReadOnly( sal_Bool bRead=sal_True );

        virtual void Init();
        virtual void DeactivateCell(sal_Bool bUpdate = sal_True);

        sal_Bool IsCutAllowed( long nRow = -1 );
        sal_Bool IsCopyAllowed( long nRow = -1 );
        sal_Bool IsPasteAllowed( long nRow = -1 );
        sal_Bool IsReadOnly();
        OFieldDescription* GetFieldDescr( long nRow );

        // window overloads
        virtual long PreNotify( NotifyEvent& rNEvt );

        // IClipboardTest
        virtual sal_Bool isCutAllowed() { return IsCutAllowed(); }
        virtual sal_Bool isCopyAllowed() { return IsCopyAllowed(); }
        virtual sal_Bool isPasteAllowed() { return IsPasteAllowed(); }
        virtual sal_Bool hasChildPathFocus() { return HasChildPathFocus(); }

        virtual void cut();
        virtual void copy();
        virtual void paste();

    private:
        DECL_LINK( StartIndexing, void* );
        DECL_LINK( DelayedCut, void* );
        DECL_LINK( DelayedPaste, void* );
        DECL_LINK( DelayedDelete, void* );
        DECL_LINK( DelayedInsNewRows, void* );
        DECL_LINK( InvalidateFieldType, void* );

        void InitCellController();
        sal_Int32 HasFieldName( const String& rFieldName );
        String GenerateName( const String& rName );
        sal_Bool SetDataPtr( long nRow );

        sal_Bool SaveData(long nRow, sal_uInt16 nColumnId);
        /** AdjustFieldDescription set the needed values for the description
            @param  _pFieldDesc     the field description where to set the values
            @param  _rMultiSel      contains the postions which changed for undo/redo
            @param  _nPos           the current position
            @param  _bSet           should a key be set
            @param  _bPrimaryKey    which value should the pkey have
        */
        void AdjustFieldDescription( OFieldDescription* _pFieldDesc,
                                     MultiSelection& _rMultiSel,
                                     sal_Int32 _nPos,
                                     sal_Bool _bSet,
                                     sal_Bool _bPrimaryKey);
        /** InvalidateFeatures invalidates the slots SID_UNDO | SID_REDO | SID_SAVEDOC
        */
        void InvalidateFeatures();

        void resetType();
    };
}
#endif // DBAUI_TABLEEDITORCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
