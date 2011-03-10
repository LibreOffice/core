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

        sal_uLong                       nIndexEvent;
        sal_uLong                       nCutEvent;
        sal_uLong                       nPasteEvent;
        sal_uLong                       nDeleteEvent;
        sal_uLong                       nInsNewRowsEvent;
        sal_uLong                       nInvalidateTypeEvent;
        sal_uLong                       nEntryNotFoundEvent;
        ChildFocusState             m_eChildFocus;

        long nOldDataPos;

        sal_Bool bSaveOnMove;
        sal_Bool bReadOnly;
        //------------------------------------------------------------------
        // Hilfsklasse
        class ClipboardInvalidator
        {
        private:
            AutoTimer   m_aInvalidateTimer;
            OTableEditorCtrl* m_pOwner;

        public:
            ClipboardInvalidator(sal_uLong nTimeout,OTableEditorCtrl*);
            ~ClipboardInvalidator();

        protected:
            DECL_LINK(OnInvalidate, void*);
        };
        friend class OTableEditorCtrl::ClipboardInvalidator;

        ClipboardInvalidator    m_aInvalidate;

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
        virtual sal_Bool SaveModified();    // wird aufgerufen vor einem Zellenwechsel
                                        // return sal_False, verhindert Zellenwechsel
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

        void         SetPrimaryKey( sal_Bool bSet );
        sal_Bool         IsPrimaryKey();

        DECL_LINK(ControlPreNotifyHdl, NotifyEvent*);

    public:
        //  TYPEINFO();
        OTableEditorCtrl(Window* pParentWin);
        virtual         ~OTableEditorCtrl();
        virtual sal_Bool    CursorMoving(long nNewRow, sal_uInt16 nNewCol);
        virtual void    UpdateAll();
        SfxUndoManager& GetUndoManager() const;

        void            SetDescrWin( OTableFieldDescWin* pWin ){ pDescrWin = pWin; if (pDescrWin && pActRow) pDescrWin->DisplayData(pActRow->GetActFieldDescr()); }
        sal_Bool            SaveCurRow();
        void            SwitchType( const TOTypeInfoSP& _pType );

        void            DisplayData( long nRow, sal_Bool bGrabFocus = sal_True );
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
        void            SetReadOnly( sal_Bool bRead=sal_True );

        virtual void Init();
        virtual void DeactivateCell(sal_Bool bUpdate = sal_True);

        sal_Bool IsCutAllowed( long nRow = -1 );
        sal_Bool IsCopyAllowed( long nRow = -1 );
        sal_Bool IsPasteAllowed( long nRow = -1 );
        sal_Bool IsReadOnly();
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
