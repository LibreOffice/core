/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

        ULONG                       nIndexEvent;
        ULONG                       nCutEvent;
        ULONG                       nPasteEvent;
        ULONG                       nDeleteEvent;
        ULONG                       nInsNewRowsEvent;
        ULONG                       nInvalidateTypeEvent;
        ULONG                       nEntryNotFoundEvent;
        ChildFocusState             m_eChildFocus;

        long nOldDataPos;

        BOOL bSaveOnMove;
        BOOL bReadOnly;
        //------------------------------------------------------------------
        // Hilfsklasse
        class ClipboardInvalidator
        {
        private:
            AutoTimer   m_aInvalidateTimer;
            OTableEditorCtrl* m_pOwner;

        public:
            ClipboardInvalidator(ULONG nTimeout,OTableEditorCtrl*);
            ~ClipboardInvalidator();

        protected:
            DECL_LINK(OnInvalidate, void*);
        };
        friend class OTableEditorCtrl::ClipboardInvalidator;

        ClipboardInvalidator    m_aInvalidate;

    protected:
        virtual void Command( const CommandEvent& rEvt );
        virtual BOOL SeekRow(long nRow);
        virtual void PaintCell(OutputDevice& rDev, const Rectangle& rRect,
                               USHORT nColumnId ) const;

        virtual void CursorMoved();
        virtual RowStatus GetRowStatus(long nRow) const;

        virtual ::svt::CellController* GetController(long nRow, USHORT nCol);
        virtual void InitController(::svt::CellControllerRef& rController, long nRow, USHORT nCol);

        virtual void CellModified();
        virtual BOOL SaveModified();    // wird aufgerufen vor einem Zellenwechsel
                                        // return FALSE, verhindert Zellenwechsel
        virtual void Undo();
        virtual void Redo();
        virtual String GetCellText(long nRow, sal_uInt16 nColId) const;
        virtual sal_uInt32 GetTotalCellWidth(long nRow, sal_uInt16 nColId);

        virtual void CopyRows();
        virtual void InsertRows( long nRow );
        virtual void DeleteRows();
        virtual void InsertNewRows( long nRow );

        virtual sal_Bool IsPrimaryKeyAllowed( long nRow );
        virtual sal_Bool IsInsertNewAllowed( long nRow );
        virtual sal_Bool IsDeleteAllowed( long nRow );

        void         ClearModified();

        void         SetPrimaryKey( BOOL bSet );
        BOOL         IsPrimaryKey();

        DECL_LINK(ControlPreNotifyHdl, NotifyEvent*);

    public:
        //  TYPEINFO();
        OTableEditorCtrl(Window* pParentWin);
        virtual         ~OTableEditorCtrl();
        virtual BOOL    CursorMoving(long nNewRow, USHORT nNewCol);
        virtual void    UpdateAll();
        SfxUndoManager* GetUndoManager() const;

        void            SetDescrWin( OTableFieldDescWin* pWin ){ pDescrWin = pWin; if (pDescrWin && pActRow) pDescrWin->DisplayData(pActRow->GetActFieldDescr()); }
        BOOL            SaveCurRow();
        void            SwitchType( const TOTypeInfoSP& _pType );

        void            DisplayData( long nRow, BOOL bGrabFocus = TRUE );
            // erzwingt das Anzeigen der genannten Zeile (selbst wenn es eigentlich schon die aktuelle ist)

        virtual void    SetCellData( long nRow, sal_uInt16 nColId, const TOTypeInfoSP& _pTypeInfo );
        virtual void    SetCellData( long nRow, sal_uInt16 nColId, const ::com::sun::star::uno::Any& _rSaveData );
        virtual ::com::sun::star::uno::Any  GetCellData( long nRow, sal_uInt16 nColId );
        virtual void    SetControlText( long nRow, sal_uInt16 nColId, const String& rText );
        virtual String  GetControlText( long nRow, sal_uInt16 nColId );

        virtual OTableDesignView* GetView() const;

        ::std::vector< ::boost::shared_ptr<OTableRow> >* GetRowList(){ return m_pRowList; }

        ::boost::shared_ptr<OTableRow>         GetActRow(){ return pActRow; }
        void            CellModified( long nRow, sal_uInt16 nColId );
        void            SetReadOnly( BOOL bRead=TRUE );

        virtual void Init();
        virtual void DeactivateCell(sal_Bool bUpdate = sal_True);

        BOOL IsCutAllowed( long nRow = -1 );
        BOOL IsCopyAllowed( long nRow = -1 );
        BOOL IsPasteAllowed( long nRow = -1 );
        BOOL IsReadOnly();
        OFieldDescription* GetFieldDescr( long nRow );

        // window overloads
        virtual long            PreNotify( NotifyEvent& rNEvt );

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
        DECL_LINK( EntryNotFound, void* );

        void InitCellController();
        INT32 HasFieldName( const String& rFieldName );
        String GenerateName( const String& rName );
        BOOL SetDataPtr( long nRow );

        BOOL SaveData(long nRow, USHORT nColumnId);
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
