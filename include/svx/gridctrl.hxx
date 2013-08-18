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
#ifndef _SVX_GRIDCTRL_HXX
#define _SVX_GRIDCTRL_HXX

#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#include <com/sun/star/sdb/XRowsChangeListener.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>

#include <vcl/button.hxx>
#include <tools/ref.hxx>
#include <svtools/editbrowsebox.hxx>
#include <osl/mutex.hxx>
#include <comphelper/propmultiplex.hxx>
#include <svtools/transfer.hxx>
#include "svx/svxdllapi.h"
#include <vector>

class DbGridControl;
class CursorWrapper;

sal_Bool CompareBookmark(const ::com::sun::star::uno::Any& aLeft, const ::com::sun::star::uno::Any& aRight);

namespace svxform
{
    class DataColumn;
}
typedef ::std::vector< ::svxform::DataColumn* > DbDataColumns;

enum GridRowStatus
{
    GRS_CLEAN,
    GRS_MODIFIED,
    GRS_DELETED,
    GRS_INVALID
};

//==================================================================
// DbGridRow, description of rows
//==================================================================

class DbGridRow : public SvRefBase
{
    ::com::sun::star::uno::Any  m_aBookmark;        // ::com::sun::star::text::Bookmark of the row, can be set
    DbDataColumns               m_aVariants;
    GridRowStatus               m_eStatus;
    sal_Bool                    m_bIsNew;
                                                    // row is no longer valid
                                                    // is removed on the next positioning
public:
    DbGridRow():m_eStatus(GRS_CLEAN), m_bIsNew(sal_True) { }
    DbGridRow(CursorWrapper* pCur, sal_Bool bPaintCursor);
    void SetState(CursorWrapper* pCur, sal_Bool bPaintCursor);

    ~DbGridRow();

    sal_Bool HasField(sal_uInt32 nPos) const { return nPos < m_aVariants.size(); }
    const ::svxform::DataColumn& GetField(sal_uInt32 nPos) const { return *m_aVariants[ nPos ]; }

    void            SetStatus(GridRowStatus _eStat) { m_eStatus = _eStat; }
    GridRowStatus   GetStatus() const               { return m_eStatus; }
    void            SetNew(sal_Bool _bNew)              { m_bIsNew = _bNew; }
    sal_Bool            IsNew() const                   { return m_bIsNew; }

    const ::com::sun::star::uno::Any& GetBookmark() const { return m_aBookmark; }

    sal_Bool    IsValid() const { return m_eStatus == GRS_CLEAN || m_eStatus == GRS_MODIFIED; }
    sal_Bool    IsModified() const { return m_eStatus == GRS_MODIFIED; }
};

SV_DECL_REF(DbGridRow)

//==================================================================
// DbGridControl
//==================================================================
class DbGridColumn;
typedef ::std::vector< DbGridColumn* > DbGridColumns;

//==================================================================
class FmGridListener
{
public:
    virtual void selectionChanged() = 0;
    virtual void columnChanged() = 0;

protected:
    ~FmGridListener() {}
};

#define GRID_COLUMN_NOT_FOUND   SAL_MAX_UINT16

//==================================================================
// InitWindowFacet, describing which aspect of a column's Window to (re-)initialize
//==================================================================
enum InitWindowFacet
{
    InitFont        = 0x01,
    InitForeground  = 0x02,
    InitBackground  = 0x04,
    InitWritingMode = 0x08,
    InitAll         = 0xFF
};

//==================================================================
class FmXGridSourcePropListener;
class DisposeListenerGridBridge;
typedef ::svt::EditBrowseBox    DbGridControl_Base;
class SVX_DLLPUBLIC DbGridControl : public DbGridControl_Base
{
    friend class FmXGridSourcePropListener;
    friend class GridFieldValueListener;
    friend class DisposeListenerGridBridge;

public:
    //==================================================================
    // NavigationBar
    //==================================================================
    class NavigationBar: public Control
    {
        class AbsolutePos : public NumericField
        {
        public:
            AbsolutePos(Window* pParent, WinBits nStyle = 0);

            virtual void KeyInput(const KeyEvent& rEvt);
            virtual void LoseFocus();
        };

        friend class NavigationBar::AbsolutePos;

        //  zusaetzliche Controls
        FixedText       m_aRecordText;
        AbsolutePos     m_aAbsolute;            // absolute positioning
        FixedText       m_aRecordOf;
        FixedText       m_aRecordCount;

        ImageButton     m_aFirstBtn;            // ImageButton for 'go to the first record'
        ImageButton     m_aPrevBtn;         // ImageButton for 'go to the previous record'
        ImageButton     m_aNextBtn;         // ImageButton for 'go to the next record'
        ImageButton     m_aLastBtn;         // ImageButton for 'go to the last record'
        ImageButton     m_aNewBtn;          // ImageButton for 'go to a new record'
        sal_uInt16      m_nDefaultWidth;
        sal_Int32       m_nCurrentPos;

        sal_Bool            m_bPositioning;     // protect PositionDataSource against recursion

    public:
        //  StatusIds for Controls of the Bar
        //  important for invalidation
        enum State
        {
            RECORD_TEXT = 1,
            RECORD_ABSOLUTE,
            RECORD_OF,
            RECORD_COUNT,
            RECORD_FIRST,
            RECORD_NEXT,
            RECORD_PREV,
            RECORD_LAST,
            RECORD_NEW
        };

        NavigationBar(Window* pParent, WinBits nStyle = 0);

        // Status methods for Controls
        void InvalidateAll(sal_Int32 nCurrentPos = -1, sal_Bool bAll = sal_False);
        void InvalidateState(sal_uInt16 nWhich) {SetState(nWhich);}
        void SetState(sal_uInt16 nWhich);
        sal_Bool GetState(sal_uInt16 nWhich) const;
        sal_uInt16 GetDefaultWidth() const {return m_nDefaultWidth;}

    protected:
        virtual void Resize();
        virtual void Paint(const Rectangle& rRect);
        virtual void StateChanged( StateChangedType nType );

    private:
        DECL_LINK(OnClick, Button*);
        sal_uInt16 ArrangeControls();

        void PositionDataSource(sal_Int32 nRecord);
    };

    friend class DbGridControl::NavigationBar;

public:
    // these options are or'ed and indicate, which of the single
    // features can be released, default is readonly which means 0
    enum Option
    {
        OPT_READONLY    = 0x00,
        OPT_INSERT      = 0x01,
        OPT_UPDATE      = 0x02,
        OPT_DELETE      = 0x04
    };

private:
    Font            m_aDefaultFont;
    Link            m_aMasterStateProvider;
    Link            m_aMasterSlotExecutor;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    m_xFormatter;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >    m_xContext;

    DbGridColumns   m_aColumns;         // Column description
    NavigationBar   m_aBar;
    DbGridRowRef    m_xDataRow;         // Row which can be modified
                                        // comes from the data cursor
    DbGridRowRef    m_xSeekRow,         // Row to which the iterator can set
                                        // comes from the data cursor

                    m_xEmptyRow;        // record set to insert

    sal_uInt32          m_nAsynAdjustEvent;

    // if we modify the row for the new record, we automatically insert a "new new row".
    // But if somebody else inserts a new record into the data source, we have to do the same.
    // For that reason we have to listen to some properties of our data source.
    ::comphelper::OPropertyChangeMultiplexer*       m_pDataSourcePropMultiplexer;
    FmXGridSourcePropListener*                      m_pDataSourcePropListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowsChangeListener>
                                                    m_xRowSetListener; // get notification when rows were changed

    void*                                           m_pFieldListeners;
        // property listeners for field values

    DisposeListenerGridBridge*                      m_pCursorDisposeListener;
        // need to know about the diposing of the seek cursor
        // construct analogous to the data source proplistener/multiplexer above :
        // DisposeListenerGridBridge is a bridge from FmXDisposeListener which I don't want to be derived from

    FmGridListener*                                 m_pGridListener;

protected:
    CursorWrapper*  m_pDataCursor;      // Cursor for Updates
    CursorWrapper*  m_pSeekCursor;      // Cursor for Seeking

private:
    // iteration variables
    DbGridRowRef        m_xCurrentRow;
    DbGridRowRef        m_xPaintRow;        // Row to be displayed
    sal_Int32           m_nSeekPos;         // Position of the SeekCursor
    sal_Int32           m_nTotalCount;      // is set when the data cursor finished counting the
                                            // records. Initial value is -1
    osl::Mutex          m_aDestructionSafety;
    osl::Mutex          m_aAdjustSafety;

    com::sun::star::util::Date
                        m_aNullDate;        // NullDate of the Numberformatter;

    BrowserMode         m_nMode;
    sal_Int32           m_nCurrentPos;      // Current position;
    sal_uInt32          m_nDeleteEvent;     // EventId for asychronous deletion of rows
    sal_uInt16          m_nOptions;         // What is the able to do (Insert, Update, Delete)
                                        // default readonly
    sal_uInt16          m_nOptionMask;      // the mask of options to be enabled in setDataSource
                                        // (with respect to the data source capabilities)
                                        // defaults to (insert | update | delete)
    sal_uInt16              m_nLastColId;
    long                m_nLastRowId;

    sal_Bool            m_bDesignMode : 1;      // default = sal_False
    sal_Bool            m_bRecordCountFinal : 1;
    sal_Bool            m_bMultiSelection   : 1;
    sal_Bool            m_bNavigationBar      : 1;

    sal_Bool            m_bSynchDisplay : 1;
    sal_Bool            m_bForceROController : 1;
    sal_Bool            m_bHandle : 1;
    sal_Bool            m_bFilterMode : 1;
    sal_Bool            m_bWantDestruction : 1;
    sal_Bool            m_bInAdjustDataSource : 1;
    sal_Bool            m_bPendingAdjustRows : 1;   // if an async adjust is pending, is it for AdjustRows or AdjustDataSource ?
    sal_Bool            m_bHideScrollbars : 1;

protected:
    sal_Bool            m_bUpdating : 1;            // are any updates being executed right now?

protected:
    virtual sal_Bool SeekRow(long nRow);
    virtual void VisibleRowsChanged( long nNewTopRow, sal_uInt16 nNumRows);
    virtual void PaintStatusCell(OutputDevice& rDev, const Rectangle& rRect) const;
    virtual void PaintCell(OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColId) const;
    virtual RowStatus GetRowStatus(long nRow) const;
    virtual sal_Bool CursorMoving(long nNewRow, sal_uInt16 nNewCol);
    virtual void CursorMoved();
    virtual void ArrangeControls(sal_uInt16& nX, sal_uInt16 nY);
    virtual sal_uInt32 GetTotalCellWidth(long nRow, sal_uInt16 nColId);
    virtual void Command(const CommandEvent& rEvt);
    virtual long PreNotify(NotifyEvent& rEvt);
    virtual void KeyInput(const KeyEvent& rEvt);
    virtual void StateChanged( StateChangedType nType );
    virtual void DataChanged( const DataChangedEvent& rDCEvt );
    virtual void Select();

    virtual ::svt::CellController* GetController(long nRow, sal_uInt16 nCol);

    virtual void CellModified();
    virtual sal_Bool SaveModified();
    virtual sal_Bool IsModified() const;

    virtual sal_uInt16 AppendColumn(const OUString& rName, sal_uInt16 nWidth = 0, sal_uInt16 nPos = HEADERBAR_APPEND, sal_uInt16 nId = (sal_uInt16)-1);
    virtual void RemoveColumn(sal_uInt16 nId);
    virtual DbGridColumn* CreateColumn(sal_uInt16 nId) const;
    virtual void ColumnMoved(sal_uInt16 nId);
    virtual sal_Bool SaveRow();
    virtual sal_Bool IsTabAllowed(sal_Bool bForward) const;

    /// hide a column
    virtual void    HideColumn(sal_uInt16 nId);
    /// show a column
    virtual void    ShowColumn(sal_uInt16 nId);

    /** This is called before executing a context menu for a row. rMenu contains the initial entries
        handled by this base class' method (which always has to be called).
        Derived classes may alter the menu in any way and handle any additional entries in
        PostExecuteColumnContextMenu.
        All disabled entries will be removed before executing the menu, so be careful with separators
        near entries you probably wish to disable ...
    */
    virtual void PreExecuteRowContextMenu(sal_uInt16 nRow, PopupMenu& rMenu);
    /** After executing the context menu for a row this method is called.
    */
    virtual void PostExecuteRowContextMenu(sal_uInt16 nRow, const PopupMenu& rMenu, sal_uInt16 nExecutionResult);

    virtual void DataSourcePropertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);

    virtual void FieldValueChanged(sal_uInt16 _nId, const ::com::sun::star::beans::PropertyChangeEvent& _evt);
    virtual void FieldListenerDisposing(sal_uInt16 _nId);

    virtual void disposing(sal_uInt16 _nId, const ::com::sun::star::lang::EventObject& _rEvt);

    // own overridables
    /// called when the current row changed
    virtual void onRowChange();
    /// called when the current column changed
    virtual void onColumnChange();

    // DragSourceHelper overridables
    virtual void StartDrag( sal_Int8 nAction, const Point& rPosPixel );

    void    executeRowContextMenu( long _nRow, const Point& _rPreferredPos );

public:
    DbGridControl(
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >,
        Window* pParent,
        WinBits nBits = WB_BORDER);

    virtual ~DbGridControl();

    virtual void Init();
    virtual void InitColumnsByFields(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xFields) = 0;
    virtual void RemoveRows();

    /** GetCellText returns the text at the given position
        @param  _nRow
            the number of the row
        @param  _nColId
            the ID of the column
        @return
            the text out of the cell
    */
    virtual OUString  GetCellText(long _nRow, sal_uInt16 _nColId) const;

    void RemoveRows(sal_Bool bNewCursor);

    void InvalidateStatus();

    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& getNumberFormatter() const {return m_xFormatter;}

    // the data source
    // the options can restrict but not extend the update abilities
    virtual void setDataSource(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& rCursor,
        sal_uInt16 nOpts = OPT_INSERT | OPT_UPDATE | OPT_DELETE);
    virtual void Dispatch(sal_uInt16 nId);

    CursorWrapper* getDataSource() const {return m_pDataCursor;}
    const DbGridColumns& GetColumns() const {return m_aColumns;}

    void EnableHandle(sal_Bool bEnable);
    sal_Bool HasHandle() const {return m_bHandle;}
    void InsertHandleColumn();

    // which position does the column with the id in the ::com::sun::star::sdbcx::View have, the handle column doesn't count
    sal_uInt16 GetViewColumnPos( sal_uInt16 nId ) const { sal_uInt16 nPos = GetColumnPos(nId); return (nPos==BROWSER_INVALIDID) ? GRID_COLUMN_NOT_FOUND : nPos-1; }

    // which position does the column with the id in m_aColumns have, that means the ::com::sun::star::sdbcx::Container
    // returned from the GetColumns (may be different from the position returned by GetViewColumnPos
    // if there are hidden columns)
    sal_uInt16 GetModelColumnPos( sal_uInt16 nId ) const;

    // the number of columns in the model
    sal_uInt16 GetViewColCount() const { return ColCount() - 1; }
    sal_uInt16 GetModelColCount() const { return (sal_uInt16)m_aColumns.size(); }
    // reverse to GetViewColumnPos: Id of position, the first non-handle column has position 0
    sal_uInt16 GetColumnIdFromViewPos( sal_uInt16 nPos ) const { return GetColumnId(nPos + 1); }
    sal_uInt16 GetColumnIdFromModelPos( sal_uInt16 nPos ) const;

    virtual void SetDesignMode(sal_Bool bMode);
    sal_Bool IsDesignMode() const {return m_bDesignMode;}
    sal_Bool IsOpen() const {return m_pSeekCursor != NULL;}

    virtual void SetFilterMode(sal_Bool bMode);
    sal_Bool IsFilterMode() const {return m_bFilterMode;}
    sal_Bool IsFilterRow(long nRow) const {return m_bFilterMode && nRow == 0;}

    void EnableNavigationBar(sal_Bool bEnable);
    sal_Bool HasNavigationBar() const {return m_bNavigationBar;}

    sal_uInt16 GetOptions() const {return m_nOptions;}
    NavigationBar& GetNavigationBar() {return m_aBar;}
    sal_uInt16 SetOptions(sal_uInt16 nOpt);
        // The new options are interpreted with respect to the current data source. If it is unable
        // to update, to insert or to restore, the according options are ignored. If the grid isn't
        // connected to a data source, all options except OPT_READONLY are ignored.

    const com::sun::star::util::Date&   getNullDate() const {return m_aNullDate;}

    // positioning
    void MoveToPosition(sal_uInt32 nPos);
    void MoveToFirst();
    void MoveToNext();
    void MoveToPrev();
    void MoveToLast();
    void AppendNew();

    // adjustment of the cursors in case the data cursor has been
    // moved from the outside.
    // the flag indicates if an adjustment of the row count should be
    // done as well
    void AdjustDataSource(sal_Bool bFull = sal_False);
    void Undo();

    virtual void BeginCursorAction();
    virtual void EndCursorAction();

    // is the current line being updated
    sal_Bool IsUpdating() const {return m_bUpdating;}

    virtual void RowRemoved( long nRow, long nNumRows = 1, sal_Bool bDoPaint = sal_True );
    virtual void RowInserted( long nRow, long nNumRows = 1, sal_Bool bDoPaint = sal_True, sal_Bool bKeepSelection = sal_False );
    virtual void RowModified( long nRow, sal_uInt16 nColId = USHRT_MAX );

    void resetCurrentRow();

    sal_Bool getDisplaySynchron() const { return m_bSynchDisplay; }
    void setDisplaySynchron(sal_Bool bSync);
        // when set to sal_False, the display is no longer in sync with the current cursor position
        // (means that in AdjustDataSource we are jumping to a row not belonging to CursorPosition)
        // when using this, you should know what you are doing, because for example entering data
        // in a row in the display that is not in sync with the position of the cursor can be very critical

    const DbGridRowRef& GetCurrentRow() const {return m_xCurrentRow;}

    void SetStateProvider(const Link& rProvider) { m_aMasterStateProvider = rProvider; }
        // if this link is set the given provider will be asked for the state of my items.
        // the return values are interpreted as follows :
        // <0 -> not specified (use default mechanism to determine the state)
        // ==0 -> the item is disabled
        // >0 -> the item is enabled
    void SetSlotExecutor(const Link& rExecutor) { m_aMasterSlotExecutor = rExecutor; }
        // analogous : if this link is set, all nav-bar slots will be routed through it when executed
        // if the handler returns nonzero, no further handling of the slot occurs

    void EnablePermanentCursor(sal_Bool bEnable);
    sal_Bool IsPermanentCursorEnabled() const;

    /** forces both scrollbars to be hidden

        For the horizontal srollbar, this is overruled by enabling the navigation bar: A navigation
        bar <b>always</b> implies a horizontal scroll bar
        @seealso EnableNavigationBar
    */
    void        ForceHideScrollbars( sal_Bool _bForce );

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        getContext() const { return m_xContext; }

    /// returns <TRUE/> if the text of the given cell can be copied into the clipboard
    sal_Bool    canCopyCellText(sal_Int32 _nRow, sal_Int16 _nColId);
    /// copies the text of the given cell into the clipboard
    void        copyCellText(sal_Int32 _nRow, sal_Int16 _nColId);

    // selectin listener handling
    FmGridListener*             getGridListener() const { return m_pGridListener; }
    void                        setGridListener( FmGridListener* _pListener ) { m_pGridListener = _pListener; }

    // helper class to grant access to selected methods from within the DbCellControl class
    struct GrantControlAccess
    {
        friend class DbCellControl;
        friend class RowSetEventListener;
    protected:
        GrantControlAccess() { }
    };

    /// called when a controller needs to be re-initialized
    void refreshController(sal_uInt16 _nColId, GrantControlAccess _aAccess);

    CursorWrapper* GetSeekCursor(GrantControlAccess /*_aAccess*/) const    { return m_pSeekCursor; }
    const DbGridRowRef& GetSeekRow(GrantControlAccess /*_aAccess*/) const  { return m_xSeekRow;    }
    void  SetSeekPos(sal_Int32 nPos,GrantControlAccess /*_aAccess*/) {m_nSeekPos = nPos;}

    /**
        @return
            The count of additional controls of the control area.
    */
    virtual sal_Int32 GetAccessibleControlCount() const;

    /** Creates the accessible object of an additional control.
        @param _nIndex
            The 0-based index of the control.
        @return
            The XAccessible interface of the specified control.
    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
    CreateAccessibleControl( sal_Int32 _nIndex );

    // IAccessibleTableProvider
    /** Creates the accessible object of a data table cell.
        @param nRow  The row index of the cell.
        @param nColumnId  The column ID of the cell.
        @return  The XAccessible interface of the specified cell. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
    CreateAccessibleCell( sal_Int32 nRow, sal_uInt16 nColumnId );

protected:
    void RecalcRows(long nNewTopRow, sal_uInt16 nLinesOnScreen, sal_Bool bUpdateCursor);
    sal_Bool SeekCursor(long nRow, sal_Bool bAbsolute = sal_False);
    void RemoveColumns();       // cleaning of own structures
    void AdjustRows();
    sal_Int32 AlignSeekCursor();
    sal_Bool SetCurrent(long nNewRow);

    OUString GetCurrentRowCellText(DbGridColumn* pCol,const DbGridRowRef& _rRow) const;
    virtual void DeleteSelectedRows();
    sal_Bool IsValid(const DbGridRowRef& _xRow) const {return _xRow && _xRow->IsValid();}

    // row which is currently being appended
    sal_Bool IsCurrentAppending() const;

    // empty row for insertion
    sal_Bool IsInsertionRow(long nRow) const;

    void  SetSeekPos(sal_Int32 nPos) {m_nSeekPos = nPos;}
    sal_Int32 GetCurrentPos() const {return m_nCurrentPos;}
    sal_Int32 GetSeekPos() const {return m_nSeekPos;}
    sal_Int32 GetTotalCount() const {return m_nTotalCount;}

    const DbGridRowRef& GetEmptyRow() const { return m_xEmptyRow;   }
    const DbGridRowRef& GetSeekRow() const  { return m_xSeekRow;    }
    const DbGridRowRef& GetPaintRow() const { return m_xPaintRow;   }
    CursorWrapper* GetSeekCursor() const    { return m_pSeekCursor; }


    void ConnectToFields();
    void DisconnectFromFields();

    void implAdjustInSolarThread(sal_Bool _bRows);
        // calls AdjustRows or AdjustDataSource, synchron if the caller is running in the solar thread, else asynchron

protected:
    virtual void InitController(::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol);
    void Construct();
    void ImplInitWindow( const InitWindowFacet _eInitWhat );
    DECL_LINK(OnDelete, void*);

    DECL_LINK(OnAsyncAdjust, void*);
        // if the param is != NULL, AdjustRows will be called, else AdjustDataSource

private:
    using BrowseBox::InsertHandleColumn;
};


SV_IMPL_REF(DbGridRow);


#endif // _SVX_GRIDCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
