/*************************************************************************
 *
 *  $RCSfile: SelectionBrowseBox.hxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 14:05:35 $
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
#ifndef DBAUI_QUERYDESIGN_OSELECTIONBROWSEBOX_HXX
#define DBAUI_QUERYDESIGN_OSELECTIONBROWSEBOX_HXX

#ifndef _SVTOOLS_EDITBROWSEBOX_HXX_
#include <svtools/editbrowsebox.hxx>
#endif
//#ifndef DBAUI_QUERYCONTROLLER_HXX
//#include "querycontroller.hxx"
//#endif
#ifndef DBAUI_TABLEFIELDDESC_HXX
#include "TableFieldDescription.hxx"
#endif
#ifndef DBAUI_JOINEXCHANGE_HXX
#include "JoinExchange.hxx"
#endif
#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

namespace connectivity
{
    class OSQLParseNode;
}

namespace dbaui
{
    //==================================================================
#define SIZ_NONE        0
#define SIZ_TOP         1
#define SIZ_BOTTOM      2
#define SIZ_LEFT        4
#define SIZ_RIGHT       8

#define BROW_FIELD_ROW          0
#define BROW_COLUMNALIAS_ROW    1
#define BROW_TABLE_ROW          2
#define BROW_ORDER_ROW          3
#define BROW_VIS_ROW            4
#define BROW_FUNCTION_ROW       5
#define BROW_CRIT1_ROW          6
#define BROW_CRIT2_ROW          7
#define BROW_CRIT3_ROW          8
#define BROW_CRIT4_ROW          9
#define BROW_CRIT5_ROW          10
#define BROW_CRIT6_ROW          11
#define BROW_ROW_CNT            12

    //==================================================================
    class OQueryDesignView;
    class OSelectionBrowseBox : public ::svt::EditBrowseBox
    {
        friend class OQueryDesignView;
        ::std::vector<FASTBOOL>             m_bVisibleRow;  // an Pos steht die RowId
        Timer                               m_timerInvalidate;

        long                                m_nSeekRow;
        BrowserMode                         m_nMode;                        // Merken des BrowseModes
        Edit*                               m_pTextCell;
        ::svt::CheckBoxControl*             m_pVisibleCell;
        ::svt::ComboBoxControl*             m_pFieldCell;
        ::svt::ListBoxControl*              m_pFunctionCell;
        ::svt::ListBoxControl*              m_pTableCell;
        ::svt::ListBoxControl*              m_pOrderCell;

        OTableFieldDescRef                  m_pEmptyEntry;      // default entry in the list may reference more than once

        sal_Int32                           m_nMaxColumns;              // maximale Anzahl der Spalten in einem Select-Statement

        String                              m_aFunctionStrings;
        sal_uInt16                          m_nVisibleCount;                // Anzahl der max sichtbaren Zeilen
        sal_Bool                            m_bOrderByUnRelated;
        sal_Bool                            m_bGroupByUnRelated;
        sal_Bool                            m_bStopTimer;
        sal_Bool                            m_bWasEditing;

        DECL_LINK(OnInvalidateTimer, void*);
    public:                         OSelectionBrowseBox( Window* pParent );
                                    ~OSelectionBrowseBox();

        void                        initialize();
        OTableFieldDescRef          InsertField( const OJoinExchangeData& jxdSource, USHORT _nColumnPostion = BROWSER_INVALIDID, sal_Bool bVis=sal_True, sal_Bool bActivate=sal_True );
        OTableFieldDescRef          InsertField( const OTableFieldDescRef& rInfo, USHORT _nColumnPostion = BROWSER_INVALIDID, sal_Bool bVis=sal_True, sal_Bool bActivate=sal_True );
        void                        InsertColumn( OTableFieldDescRef pEntry, USHORT& _nColumnPostion );
        void                        RemoveColumn( USHORT _nColumnId );
        void                        DeleteFields( const String& rAliasName );
        // AddGroupBy:: F"ugt ein Feld mit Funktion == Grupierung. Falls das Feld schon vorhanden ist und ein Aggregate Funktion
        // benutzt, wird das Flag nicht gesetzt
        void                        AddGroupBy( const OTableFieldDescRef& rInfo );
        void                        AddCondition( const OTableFieldDescRef& rInfo,
                                                  const String& rValue,
                                                  const sal_uInt16 nLevel, const char* pOp=0 );
        void                        AddOrder(const OTableFieldDescRef& rInfo, const EOrderDir eDir, sal_uInt16& nPos);
        void                        ClearAll();
        OTableFieldDescRef          AppendNewCol( sal_uInt16 nCnt=1 );
        sal_Bool                    Save();
        OQueryDesignView*           getDesignView();
        OQueryDesignView*           getDesignView() const;
        void                        SetColWidth();
        sal_uInt16                  FieldsCount();

        void                        SetColWidth(sal_uInt16 nColId, long lNewWidth);
                                    // beachtet im Gegensatz zum SetColumnWidth der Basisklasse auch eine eventuell aktive Zelle in dieser Spalte

        String                      GetCellContents(sal_Int32 nCellIndex, USHORT nColId);
        void                        SetCellContents(sal_Int32 nCellIndex, USHORT nColId, const String& strNewText);
                                        // Zelleninhalt (als String formatiert) setzen/liefern
        sal_Int32                   GetNoneVisibleRows() const;
        void                        SetNoneVisbleRow(long nRows);
        sal_Bool                    IsRowVisible(sal_uInt16 _nWhich) const;
        void                        SetRowVisible(sal_uInt16 _nWhich, sal_Bool _bVis);

        void                        SetReadOnly(sal_Bool bRO);
        // calculate an optimal size. Basically, this takes into account the number of visible rows.
        Size                        CalcOptimalSize( const Size& _rAvailable );

        // can the current content be cut
        sal_Bool                    isPasteAllowed();
        sal_Bool                    isCutAllowed();
        sal_Bool                    isCopyAllowed();
        void                        cut();
        void                        paste();
        void                        copy();

        virtual void                GetFocus();
        virtual void                DeactivateCell(sal_Bool bUpdate = sal_True);
        virtual void                ColumnMoved( USHORT nColId ) { ColumnMoved(nColId,TRUE); }
        void                        ColumnMoved( USHORT nColId,BOOL _bCreateUndo);

        void                        Fill();
        void                        PreFill();

        /** GetCellText returns the text at the given position
            @param  _nRow
                the number of the row
            @param  _nColId
                the ID of the column
            @return
                the text out of the cell
        */
        virtual String              GetCellText(long _nRow, USHORT _nColId) const;

        /** returns the description of the row.
            @param  _nRow
                The row number.
            @return
                The header text of the specified row.
        */
        virtual ::rtl::OUString     GetRowDescription( sal_Int32 _nRow ) const;

        /** return the name of the specified object.
            @param  eObjType
                The type to ask for
            @param  _nPosition
                The position of a tablecell (index position), header bar  colum/row cell
            @return
                The name of the specified object.
        */
        virtual ::rtl::OUString     GetAccessibleName( ::svt::AccessibleBrowseBoxObjType eObjType,sal_Int32 _nPosition = -1) const;

    protected:
        virtual sal_Bool            SeekRow( long nRow );

        virtual void                PaintStatusCell(OutputDevice& rDev, const Rectangle& rRect) const;
        virtual void                PaintCell(OutputDevice& rDev, const Rectangle& rRect,
                                              sal_uInt16 nColumnId ) const;

        virtual sal_Int8            AcceptDrop( const BrowserAcceptDropEvent& rEvt );
        virtual sal_Int8            ExecuteDrop( const BrowserExecuteDropEvent& rEvt );
        virtual void                MouseButtonDown( const BrowserMouseEvent& rEvt );
        virtual void                MouseButtonUp( const BrowserMouseEvent& rEvt );
        virtual void                KeyInput( const KeyEvent& rEvt );
        virtual void                Command(const CommandEvent& rEvt);
        virtual void                ArrangeControls(sal_uInt16& nX, sal_uInt16 nY);

        virtual ::svt::CellController*  GetController(long nRow, sal_uInt16 nCol);
        virtual void                InitController(::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol);
        virtual void                CellModified();
        virtual sal_Bool            SaveModified();
        virtual void                Init();
        virtual sal_uInt32          GetTotalCellWidth(long nRow, sal_uInt16 nColId) const;
        virtual void                ColumnResized( sal_uInt16 nColId );

        virtual sal_uInt32          GetTotalCellWidth(long nRow, sal_uInt16 nColId);

        virtual sal_uInt16          GetDefaultColumnWidth(const String& rName) const;
        // if you want to have an own header ...
        virtual BrowserHeader*      imp_CreateHeaderBar(BrowseBox* pParent);

        void                        stopTimer();
        void                        startTimer();

    private:
        OTableFieldDescRef          FindFirstFreeCol(USHORT& _rColumnPosition);
            // rCol enthaelt die Nummer (in pOTableFieldDescList) der ersten Spalte, die von sich sagt, dass sie leer ist
            // wenn es keine solche gibt, ist rCol undefiniert und der Rueckgabewert NULL
        void                        CheckFreeColumns(USHORT& _rColumnPosition);
            // testet, ob es noch freie Spalten gibt, wenn nicht, wird ein neuer Packen angefuegt
            // rCol enthaelt die Nummer der ersten freien Spalte (in pOTableFieldDescList)

        void            RemoveField( sal_uInt16 nId, sal_Bool bActivate = sal_True);
        Rectangle       GetInvalidRect( sal_uInt16 nColId );
        long            GetRealRow(long nRow) const;
        long            GetBrowseRow(long nRowId) const;
        sal_Bool        GetFunctionName(sal_uInt32 _nFunctionTokenId,String& rFkt);
        void            appendUndoAction(const String& _rOldValue,const String& _rNewValue,sal_Int32 _nRow,sal_Bool& _bListAction);
        void            appendUndoAction(const String& _rOldValue,const String& _rNewValue,sal_Int32 _nRow);
        OTableFields&   getFields() const;
        void            enableControl(const OTableFieldDescRef& _rEntry,Window* _pControl);
        void            setTextCellContext(const OTableFieldDescRef& _rEntry,const String& _sText,ULONG _nHelpId);
        void            invalidateUndoRedo();
        OTableFieldDescRef getEntry(OTableFields::size_type _nPos);

        void            adjustSelectionMode( sal_Bool _bClickedOntoHeader, sal_Bool _bClickedOntoHandleCol );

        /** save the filed change in save modified
            @param  _sFieldName
                The field name inserted by the user.
            @param  _pEntry
                The entry which will contain the nescessary entries.
            @param  _bListAction
                Will be set to <TRUE/> when we are in a list action otherwise <FALSE/>
            @return
                <TRUE/> if an error occured otherwise <FALSE/>
        */
        sal_Bool        saveField(const String& _sFieldName,OTableFieldDescRef& _pEntry,sal_Bool& _bListAction);

        /** sets the table window at the _pEntry
            @param  _pEntry
                The entry where the window should be set.
            @param  _sTableName
                The table name to search for.
            @return
                <TRUE/> if the table name was set otherwise <FALSE/>
        */
        sal_Bool        fillEntryTable(OTableFieldDescRef& _pEntry,const ::rtl::OUString& _sTableName);

        /** uses the parse node to fill all information into the field
            @param  _pColumnRef
                The parse node used to fill the info into the field.
            @param  _xMetaData
                Use to parse the node to a string.
            @param  _pEntry
                The entry which will contain the nescessary entries.
            @param  _bListAction
                Will be set to <TRUE/> when we are in a list action otherwise <FALSE/>
            @return
                <TRUE/> if an error occured otherwise <FALSE/>
        */
        sal_Bool        fillColumnRef(  const ::connectivity::OSQLParseNode* _pColumnRef,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _xMetaData,
                                        OTableFieldDescRef& _pEntry,
                                        sal_Bool& _bListAction);
        sal_Bool        fillColumnRef(  const ::rtl::OUString& _sColumnName,
                                        const ::rtl::OUString& _sTableRange,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _xMetaData,
                                        OTableFieldDescRef& _pEntry,
                                        sal_Bool& _bListAction);


        /** append an undo action for the table field
            @param  _sOldAlias
                The old table alias.
            @param  _sAlias
                The new alias name.
            @param  _bListAction
                Will be set to <TRUE/> when we are in a list action otherwise <FALSE/>
        */
        void            notifyTableFieldChanged(const String& _sOldAlias,const String& _sAlias,sal_Bool& _bListAction,USHORT _nColumnId);

        /** append an undo action for the function field
            @param  _sOldFunctionName
                The old value.
            @param  _sFunctionName
                The new function name.
            @param  _bListAction
                Will be set to <TRUE/> when we are in a list action otherwise <FALSE/>
        */
        void            notifyFunctionFieldChanged(const String& _sOldFunctionName,const String& _sFunctionName,sal_Bool& _bListAction,USHORT _nColumnId);

        /** clears the function fields of the submitted entry if it doesn't match the SQL standard and append an undo action.
            E.q. AGGREGATE functions are only valid when the field name isn't an asterix
            @param  _sFieldName
                The field name.
            @param  _pEntry
                The entry to be cleared
            @param  _bListAction
                When <TRUE/> an list action will be created.
        */
        void            clearEntryFunctionField(const String& _sFieldName,OTableFieldDescRef& _pEntry,sal_Bool& _bListAction,USHORT _nColumnId);

        /** remove or insert the necessary function types
            @param  _pEntry
                The currently edited entry.
        */
        void            setFunctionCell(OTableFieldDescRef& _pEntry);
    };
}
#endif // DBAUI_QUERYDESIGN_OSELECTIONBROWSEBOX_HXX



