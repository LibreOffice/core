/*************************************************************************
 *
 *  $RCSfile: TEditControl.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:53:02 $
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
#ifndef DBAUI_TYPEINFO_HXX
#include "TypeInfo.hxx"
#endif

class Edit;
class SfxUndoManager;
namespace dbaui
{
    class OTableDescWin;
    class OSQLNameEdit;

    class OTableEditorCtrl : public OTableRowView
    {
        enum ChildFocusState
        {
            DESCRIPTION,
            NAME,
            ROW,
            NONE
        };

        ::std::vector<OTableRow*>   m_aUndoList;
        ::std::vector<OTableRow*>*  m_pRowList;

        EEditMode                   eEditMode;

        OSQLNameEdit*               pNameCell;
        ::svt::ListBoxControl*      pTypeCell;
        Edit*                       pDescrCell;
        OTableFieldDescWin*         pDescrWin;          // properties of one column

        OTableRow*                  pActRow;

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

        virtual ::svt::CellController* GetController(long nRow, USHORT nCol);
        virtual void InitController(::svt::CellControllerRef& rController, long nRow, USHORT nCol);

        virtual void CellModified();
        virtual BOOL SaveModified();    // wird aufgerufen vor einem Zellenwechsel
                                        // return FALSE, verhindert Zellenwechsel
        virtual void Undo();
        virtual void Redo();
        virtual String GetCellText(long nRow, USHORT nColId) const;
        virtual ULONG GetTotalCellWidth(long nRow, USHORT nColId);

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

        virtual void    SetData( long nRow, USHORT nColId, const TOTypeInfoSP& _pTypeInfo );
        virtual void    SetData( long nRow, USHORT nColId, const ::com::sun::star::uno::Any& _rSaveData );
        virtual ::com::sun::star::uno::Any  GetData( long nRow, USHORT nColId );
        virtual void    SetControlText( long nRow, USHORT nColId, const String& rText );
        virtual String  GetControlText( long nRow, USHORT nColId );

        virtual OTableDesignView* GetView() const;

        ::std::vector<OTableRow*>* GetRowList(){ return m_pRowList; }

        OTableRow*      GetActRow(){ return pActRow; }
        void            CellModified( long nRow, USHORT nColId );
        void            SetReadOnly( BOOL bRead=TRUE );

        virtual void Init();
        virtual void DeactivateCell(sal_Bool bUpdate = sal_True);

        BOOL IsCutAllowed( long nRow = -1 );
        BOOL IsCopyAllowed( long nRow = -1 );
        BOOL IsPasteAllowed( long nRow = -1 );
        BOOL IsReadOnly();

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
        OFieldDescription* GetFieldDescr( long nRow );

        void SetEditMode( EEditMode eMode ){ eEditMode = eMode; }
        EEditMode GetEditMode(){ return eEditMode; }
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
    };
}
#endif // DBAUI_TABLEEDITORCONTROL_HXX



