/*************************************************************************
 *
 *  $RCSfile: TEditControl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-22 07:54:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_TABLEEDITORCONTROL_HXX
#define DBAUI_TABLEEDITORCONTROL_HXX

#ifndef DBAUI_TABLEDESIGNCONTROL_HXX
#include "TableDesignControl.hxx"
#endif
#ifndef DBAUI_TABLEDESIGNVIEW_HXX
#include "TableDesignView.hxx"
#endif
#ifndef DBAUI_TABLEFIELDDESCRIPTION_HXX
#include "TableFieldDescWin.hxx"
#endif
#ifndef DBAUI_TABLEROW_HXX
#include "TableRow.hxx"
#endif
#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif

class Edit;
class SfxUndoManager;
namespace dbaui
{
    class OTableDescWin;

    class OTableEditorCtrl : public OTableRowView
    {
        ::std::vector<OTableRow*>   m_aUndoList;
        ::std::vector<OTableRow*>*  m_pRowList;

        EEditMode                   eEditMode;

        Edit*                       pNameCell;
        DbListBoxCtrl*              pTypeCell;
        Edit*                       pDescrCell;
        OTableFieldDescWin*         pDescrWin;          // properties of one column

        OTableRow*                  pActRow;

        ULONG nIndexEvent;
        ULONG nCutEvent;
        ULONG nPasteEvent;
        ULONG nDeleteEvent;
        ULONG nInsNewRowsEvent;
        ULONG nInvalidateTypeEvent;
        ULONG nEntryNotFoundEvent;

        long nOldDataPos;

        BOOL bSaveOnMove;
        BOOL bSaveFormatter;
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

        virtual DbCellController* GetController(long nRow, USHORT nCol);
        virtual void InitController(DbCellControllerRef& rController, long nRow, USHORT nCol);

        virtual void CellModified();
        virtual BOOL SaveModified();    // wird aufgerufen vor einem Zellenwechsel
                                        // return FALSE, verhindert Zellenwechsel
        virtual void Undo();
        virtual void Redo();
        virtual String GetCellText(long nRow, USHORT nColId) const;
        virtual ULONG GetTotalCellWidth(long nRow, USHORT nColId) const;

        virtual void CopyRows();
        virtual void InsertRows( long nRow );
        virtual void DeleteRows();
        virtual void InsertNewRows( long nRow );

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
        void            SwitchType( const OTypeInfo* _pType );

        void            DisplayData( long nRow, BOOL bGrabFocus = TRUE );
            // erzwingt das Anzeigen der genannten Zeile (selbst wenn es eigentlich schon die aktuelle ist)

        virtual void    SetData( long nRow, USHORT nColId, const OTypeInfo* _pTypeInfo );
        virtual void    SetData( long nRow, USHORT nColId, const String& _rSaveData );
        virtual String  GetData( long nRow, USHORT nColId );
        virtual void    SetControlText( long nRow, USHORT nColId, const String& rText );
        virtual String  GetControlText( long nRow, USHORT nColId );

        virtual OTableDesignView* GetView() const;

        ::std::vector<OTableRow*>* GetRowList(){ return m_pRowList; }

        OTableRow*      GetActRow(){ return pActRow; }
        void            CellModified( long nRow, USHORT nColId );
        void            SetReadOnly( BOOL bRead=TRUE );

        virtual void Init();

        BOOL IsCutAllowed( long nRow = -1 );
        BOOL IsCopyAllowed( long nRow = -1 );
        BOOL IsPasteAllowed( long nRow = -1 );
        BOOL IsReadOnly();

        virtual void Cut();
        virtual void Copy();
        virtual void Paste();
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
        OFieldDescription* GetFieldDescr( long nRow );
        BOOL IsPrimaryKeyAllowed( long nRow );
        BOOL IsInsertNewAllowed( long nRow );
        BOOL IsDeleteAllowed( long nRow );
        void SetEditMode( EEditMode eMode ){ eEditMode = eMode; }
        EEditMode GetEditMode(){ return eEditMode; }
        BOOL SaveData(long nRow, USHORT nColumnId);
    };
}
#endif // DBAUI_TABLEEDITORCONTROL_HXX



