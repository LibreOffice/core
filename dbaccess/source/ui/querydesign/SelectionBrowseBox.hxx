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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_QUERYDESIGN_SELECTIONBROWSEBOX_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_QUERYDESIGN_SELECTIONBROWSEBOX_HXX

#include <svtools/editbrowsebox.hxx>
#include "TableFieldDescription.hxx"
#include "JoinExchange.hxx"
#include "QEnumTypes.hxx"
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <svtools/transfer.hxx>

namespace connectivity
{
    class OSQLParseNode;
}

namespace dbaui
{
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

    class OQueryDesignView;
    class OSelectionBrowseBox : public ::svt::EditBrowseBox
    {
        friend class OQueryDesignView;
        ::std::vector<bool>                 m_bVisibleRow;              // at pos we find the RowId
        Timer                               m_timerInvalidate;

        long                                m_nSeekRow;
        BrowserMode                         m_nMode;                    // remember the BrowseModes
        VclPtr<Edit>                               m_pTextCell;
        VclPtr< ::svt::CheckBoxControl>             m_pVisibleCell;
        VclPtr< ::svt::ComboBoxControl>             m_pFieldCell;
        VclPtr< ::svt::ListBoxControl>              m_pFunctionCell;
        VclPtr< ::svt::ListBoxControl>              m_pTableCell;
        VclPtr< ::svt::ListBoxControl>              m_pOrderCell;

        sal_Int32                           m_nMaxColumns;              // maximum number of columns in a Select-Statement

        OUString                            m_aFunctionStrings;
        sal_uInt16                          m_nVisibleCount;            // maximum number of visible rows
        sal_uInt32                          m_nLastSortColumn;          // index of last (highest) sort column
        bool                            m_bOrderByUnRelated;
        bool                            m_bGroupByUnRelated;
        bool                            m_bStopTimer;
        bool                            m_bWasEditing;
        bool                            m_bDisableErrorBox;
        bool                            m_bInUndoMode;

        DECL_LINK_TYPED(OnInvalidateTimer, Timer*, void);
    public:
        explicit OSelectionBrowseBox( vcl::Window* pParent );
                                    virtual ~OSelectionBrowseBox();
        virtual void                dispose() override;

        void                        initialize();
        OTableFieldDescRef          InsertField( const OJoinExchangeData& jxdSource );
        OTableFieldDescRef          InsertField( const OTableFieldDescRef& rInfo, sal_uInt16 _nColumnPosition = BROWSER_INVALIDID, bool bVis=true, bool bActivate=true );
        void                        InsertColumn( const OTableFieldDescRef& pEntry, sal_uInt16& _nColumnPosition );
        void                        RemoveColumn( sal_uInt16 _nColumnId );
        void                        DeleteFields( const OUString& rAliasName );

        bool                        HasFieldByAliasName(const OUString& rFieldName, OTableFieldDescRef& rInfo) const;

        // AddGroupBy:: inserts a field with function == grouping. If the fields already exists and uses an aggregate function,
        // the flag is not set
        void                        AddGroupBy( const OTableFieldDescRef& rInfo,sal_uInt32 _nCurrentPos);
        void                        AddCondition( const OTableFieldDescRef& rInfo,
                                                  const OUString& rValue,
                                                  const sal_uInt16 nLevel,
                                                  bool _bAddOrOnOneLine  );
        void                        DuplicateConditionLevel( const sal_uInt16 nLevel);
        void                        AddOrder(const OTableFieldDescRef& rInfo, const EOrderDir eDir, sal_uInt32 _nCurrentPos);
        void                        ClearAll();
        OTableFieldDescRef          AppendNewCol( sal_uInt16 nCnt=1 );
        bool                        Save();
        OQueryDesignView*           getDesignView();
        OQueryDesignView*           getDesignView() const;
        sal_uInt16                  FieldsCount();

        void                        SetColWidth(sal_uInt16 nColId, long lNewWidth);
                                        // unlike SetColumnWidth of the base class it checks an active cell in this column

        OUString                    GetCellContents(sal_Int32 nCellIndex, sal_uInt16 nColId);
        void                        SetCellContents(sal_Int32 nCellIndex, sal_uInt16 nColId, const OUString& strNewText);
                                        // cell content (formatted as string) set/return
        sal_Int32                   GetNoneVisibleRows() const;
        void                        SetNoneVisbleRow(long nRows);
        bool                        IsRowVisible(sal_uInt16 _nWhich) const;
        void                        SetRowVisible(sal_uInt16 _nWhich, bool _bVis);

        void                        SetReadOnly(bool bRO);
        // calculate an optimal size. Basically, this takes into account the number of visible rows.
        Size                        CalcOptimalSize( const Size& _rAvailable );

        // can the current content be cut
        bool                        isPasteAllowed();
        bool                        isCutAllowed();
        bool                        isCopyAllowed();
        void                        cut();
        void                        paste();
        void                        copy();

        virtual void                GetFocus() override;
        virtual void                DeactivateCell(bool bUpdate = true) override;
        virtual void                ColumnMoved( sal_uInt16 nColId ) override { ColumnMoved(nColId,true); }
        void                        ColumnMoved( sal_uInt16 nColId, bool _bCreateUndo);

        void                        Fill();
        void                        PreFill();

        /** Disables the generation of undo actions
        */
        inline void                 EnterUndoMode() { m_bInUndoMode = true; }
        /** Enables the generation of undo actions
        */
        inline void                 LeaveUndoMode() { m_bInUndoMode = false; }

        /** GetCellText returns the text at the given position
            @param  _nRow
                the number of the row
            @param  _nColId
                the ID of the column
            @return
                the text out of the cell
        */
        virtual OUString            GetCellText(long _nRow, sal_uInt16 _nColId) const override;

        /** returns the description of the row.
            @param  _nRow
                The row number.
            @return
                The header text of the specified row.
        */
        virtual OUString     GetRowDescription( sal_Int32 _nRow ) const override;

        /** return the name of the specified object.
            @param  eObjType
                The type to ask for
            @param  _nPosition
                The position of a tablecell (index position), header bar  column/row cell
            @return
                The name of the specified object.
        */
        virtual OUString     GetAccessibleObjectName( ::svt::AccessibleBrowseBoxObjType eObjType,sal_Int32 _nPosition = -1) const override;

        // IAccessibleTableProvider
        /** Creates the accessible object of a data table cell.
        @param nRow  The row index of the cell.
        @param nColumnId  The column ID of the cell.
        @return  The XAccessible interface of the specified cell. */
        virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessibleCell( sal_Int32 nRow, sal_uInt16 nColumnId ) override;

    protected:
        virtual bool                SeekRow( long nRow ) override;

        virtual void                PaintStatusCell(OutputDevice& rDev, const Rectangle& rRect) const override;
        virtual void                PaintCell(OutputDevice& rDev, const Rectangle& rRect,
                                              sal_uInt16 nColumnId ) const override;

        virtual sal_Int8            AcceptDrop( const BrowserAcceptDropEvent& rEvt ) override;
        virtual sal_Int8            ExecuteDrop( const BrowserExecuteDropEvent& rEvt ) override;
        virtual void                MouseButtonDown( const BrowserMouseEvent& rEvt ) override;
        virtual void                MouseButtonUp( const BrowserMouseEvent& rEvt ) override;
        virtual void                KeyInput( const KeyEvent& rEvt ) override;
        virtual void                Command(const CommandEvent& rEvt) override;
        virtual void                ArrangeControls(sal_uInt16& nX, sal_uInt16 nY) override;

        virtual ::svt::CellController*  GetController(long nRow, sal_uInt16 nCol) override;
        virtual void                InitController(::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol) override;
        virtual void                CellModified() override;
        virtual bool                SaveModified() override;
        virtual void                Init() override;
        virtual void                ColumnResized( sal_uInt16 nColId ) override;

        virtual sal_uInt32          GetTotalCellWidth(long nRow, sal_uInt16 nColId) override;

        // if you want to have an own header ...
        virtual VclPtr<BrowserHeader> imp_CreateHeaderBar(BrowseBox* pParent) override;

        void                        stopTimer();
        void                        startTimer();

    private:
        OTableFieldDescRef          FindFirstFreeCol(sal_uInt16& _rColumnPosition);

            // rCol contains the Nummer (in pOTableFieldDescList) of the first column, which itself tells it is empty
            // if there are none, rCol is undefined and the returnvalue NULL
        void                        CheckFreeColumns(sal_uInt16& _rColumnPosition);

            // check if empty columns are available, if not,  a new Packen is appended
            // rCol contains the Nummer of the first empty column (in pOTableFieldDescList)

        void            RemoveField( sal_uInt16 nId );
        Rectangle       GetInvalidRect( sal_uInt16 nColId );
        long            GetRealRow(long nRow) const;
        long            GetBrowseRow(long nRowId) const;
        bool            GetFunctionName(sal_uInt32 _nFunctionTokenId, OUString& rFkt);
        void            appendUndoAction(const OUString& _rOldValue,const OUString& _rNewValue,sal_Int32 _nRow, bool& _bListAction);
        void            appendUndoAction(const OUString& _rOldValue,const OUString& _rNewValue,sal_Int32 _nRow);
        OTableFields&   getFields() const;
        static void     enableControl(const OTableFieldDescRef& _rEntry,Window* _pControl);
        void            setTextCellContext(const OTableFieldDescRef& _rEntry,const OUString& _sText,const OString& _sHelpId);
        void            invalidateUndoRedo();
        OTableFieldDescRef getEntry(OTableFields::size_type _nPos);

        void            adjustSelectionMode( bool _bClickedOntoHeader, bool _bClickedOntoHandleCol );

        /** save the filed change in save modified
            @param  _sFieldName
                The field name inserted by the user.
            @param  _pEntry
                The entry which will contain the necessary entries.
            @param  _bListAction
                Will be set to <TRUE/> when we are in a list action otherwise <FALSE/>
            @return
                <TRUE/> if an error occurred otherwise <FALSE/>
        */
        bool            saveField(OUString& _sFieldName, OTableFieldDescRef& _pEntry, bool& _bListAction);

        /** sets the table window at the _pEntry
            @param  _pEntry
                The entry where the window should be set.
            @param  _sTableName
                The table name to search for.
            @return
                <TRUE/> if the table name was set otherwise <FALSE/>
        */
        bool            fillEntryTable(OTableFieldDescRef& _pEntry,const OUString& _sTableName);

        /** uses the parse node to fill all information into the field
            @param  _pColumnRef
                The parse node used to fill the info into the field.
            @param  _xMetaData
                Use to parse the node to a string.
            @param  _pEntry
                The entry which will contain the necessary entries.
            @param  _bListAction
                Will be set to <TRUE/> when we are in a list action otherwise <FALSE/>
            @return
                <TRUE/> if an error occurred otherwise <FALSE/>
        */
        bool            fillColumnRef(  const ::connectivity::OSQLParseNode* _pColumnRef,
                                        const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
                                        OTableFieldDescRef& _pEntry,
                                        bool& _bListAction);
        bool            fillColumnRef(  const OUString& _sColumnName,
                                        const OUString& _sTableRange,
                                        const css::uno::Reference< css::sdbc::XDatabaseMetaData >& _xMetaData,
                                        OTableFieldDescRef& _pEntry,
                                        bool& _bListAction);

        /** append an undo action for the table field
            @param  _sOldAlias
                The old table alias.
            @param  _sAlias
                The new alias name.
            @param  _bListAction
                Will be set to <TRUE/> when we are in a list action otherwise <FALSE/>
        */
        void            notifyTableFieldChanged(const OUString& _sOldAlias,const OUString& _sAlias, bool& _bListAction,sal_uInt16 _nColumnId);

        /** append an undo action for the function field
            @param  _sOldFunctionName
                The old value.
            @param  _sFunctionName
                The new function name.
            @param  _bListAction
                Will be set to <TRUE/> when we are in a list action otherwise <FALSE/>
        */
        void            notifyFunctionFieldChanged(const OUString& _sOldFunctionName,const OUString& _sFunctionName, bool& _bListAction,sal_uInt16 _nColumnId);

        /** clears the function fields of the submitted entry if it doesn't match the SQL standard and append an undo action.
            E.q. AGGREGATE functions are only valid when the field name isn't an asterisk
            @param  _sFieldName
                The field name.
            @param  _pEntry
                The entry to be cleared
            @param  _bListAction
                When <TRUE/> an list action will be created.
        */
        void            clearEntryFunctionField(const OUString& _sFieldName,OTableFieldDescRef& _pEntry, bool& _bListAction,sal_uInt16 _nColumnId);

        /** remove or insert the necessary function types
            @param  _pEntry
                The currently edited entry.
        */
        void            setFunctionCell(OTableFieldDescRef& _pEntry);

    private:
        using ::svt::EditBrowseBox::AcceptDrop;
        using ::svt::EditBrowseBox::ExecuteDrop;
        using ::svt::EditBrowseBox::MouseButtonDown;
        using ::svt::EditBrowseBox::MouseButtonUp;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_QUERYDESIGN_SELECTIONBROWSEBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
