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
#ifndef INCLUDED_SVX_GRIDCTRL_HXX
#define INCLUDED_SVX_GRIDCTRL_HXX

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
#include <svx/svxdllapi.h>
#include <vector>

class DbGridControl;
class CursorWrapper;

bool CompareBookmark(const ::com::sun::star::uno::Any& aLeft, const ::com::sun::star::uno::Any& aRight);

namespace svxform
{
    class DataColumn;
}

enum GridRowStatus
{
    GRS_CLEAN,
    GRS_MODIFIED,
    GRS_DELETED,
    GRS_INVALID
};


// DbGridRow, description of rows


class DbGridRow : public SvRefBase
{
    ::com::sun::star::uno::Any  m_aBookmark;        // Bookmark of the row, can be set
    ::std::vector< ::svxform::DataColumn* >
                                m_aVariants;
    GridRowStatus               m_eStatus;
    bool                        m_bIsNew;
                                                    // row is no longer valid
                                                    // is removed on the next positioning
public:
    DbGridRow():m_eStatus(GRS_CLEAN), m_bIsNew(true) { }
    DbGridRow(CursorWrapper* pCur, bool bPaintCursor);
    void SetState(CursorWrapper* pCur, bool bPaintCursor);

    virtual ~DbGridRow();

    bool HasField(sal_uInt32 nPos) const { return nPos < m_aVariants.size(); }
    const ::svxform::DataColumn& GetField(sal_uInt32 nPos) const { return *m_aVariants[ nPos ]; }

    void            SetStatus(GridRowStatus _eStat) { m_eStatus = _eStat; }
    GridRowStatus   GetStatus() const               { return m_eStatus; }
    void            SetNew(bool _bNew)              { m_bIsNew = _bNew; }
    bool            IsNew() const                   { return m_bIsNew; }

    const ::com::sun::star::uno::Any& GetBookmark() const { return m_aBookmark; }

    bool    IsValid() const { return m_eStatus == GRS_CLEAN || m_eStatus == GRS_MODIFIED; }
    bool    IsModified() const { return m_eStatus == GRS_MODIFIED; }
};

typedef tools::SvRef<DbGridRow> DbGridRowRef;


// DbGridControl

class DbGridColumn;
typedef ::std::vector< DbGridColumn* > DbGridColumns;


class FmGridListener
{
public:
    virtual void selectionChanged() = 0;
    virtual void columnChanged() = 0;

protected:
    ~FmGridListener() {}
};

#define GRID_COLUMN_NOT_FOUND   SAL_MAX_UINT16


// InitWindowFacet, describing which aspect of a column's Window to (re-)initialize

enum InitWindowFacet
{
    InitFontFacet   = 0x01,
    InitForeground  = 0x02,
    InitBackground  = 0x04,
    InitWritingMode = 0x08,
    InitAll         = 0xFF
};


class FmXGridSourcePropListener;
class DisposeListenerGridBridge;
typedef ::svt::EditBrowseBox    DbGridControl_Base;
class SVX_DLLPUBLIC DbGridControl : public DbGridControl_Base
{
    friend class FmXGridSourcePropListener;
    friend class GridFieldValueListener;
    friend class DisposeListenerGridBridge;

public:

    // NavigationBar

    class NavigationBar: public Control
    {
        class AbsolutePos : public NumericField
        {
        public:
            AbsolutePos(vcl::Window* pParent, WinBits nStyle = 0);

            virtual void KeyInput(const KeyEvent& rEvt) SAL_OVERRIDE;
            virtual void LoseFocus() SAL_OVERRIDE;
        };

        friend class NavigationBar::AbsolutePos;

        //  zusaetzliche Controls
        VclPtr<FixedText>    m_aRecordText;
        VclPtr<AbsolutePos>  m_aAbsolute;            // absolute positioning
        VclPtr<FixedText>    m_aRecordOf;
        VclPtr<FixedText>    m_aRecordCount;

        VclPtr<ImageButton>  m_aFirstBtn;            // ImageButton for 'go to the first record'
        VclPtr<ImageButton>  m_aPrevBtn;         // ImageButton for 'go to the previous record'
        VclPtr<ImageButton>  m_aNextBtn;         // ImageButton for 'go to the next record'
        VclPtr<ImageButton>  m_aLastBtn;         // ImageButton for 'go to the last record'
        VclPtr<ImageButton>  m_aNewBtn;          // ImageButton for 'go to a new record'
        sal_uInt16           m_nDefaultWidth;
        sal_Int32            m_nCurrentPos;

        bool                 m_bPositioning;     // protect PositionDataSource against recursion

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

        NavigationBar(vcl::Window* pParent, WinBits nStyle = 0);
        virtual ~NavigationBar();
        virtual void dispose() SAL_OVERRIDE;

        // Status methods for Controls
        void InvalidateAll(sal_Int32 nCurrentPos = -1, bool bAll = false);
        void InvalidateState(sal_uInt16 nWhich) {SetState(nWhich);}
        void SetState(sal_uInt16 nWhich);
        bool GetState(sal_uInt16 nWhich) const;
        sal_uInt16 GetDefaultWidth() const {return m_nDefaultWidth;}

    protected:
        virtual void Resize() SAL_OVERRIDE;
        virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) SAL_OVERRIDE;
        virtual void StateChanged( StateChangedType nType ) SAL_OVERRIDE;

    private:
        DECL_LINK_TYPED(OnClick, Button*, void);
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
    vcl::Font              m_aDefaultFont;
    Link<sal_uInt16,int>   m_aMasterStateProvider;
    Link<sal_uInt16,bool>   m_aMasterSlotExecutor;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    m_xFormatter;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >    m_xContext;

    DbGridColumns   m_aColumns;         // Column description
    VclPtr<NavigationBar>   m_aBar;
    DbGridRowRef    m_xDataRow;         // Row which can be modified
                                        // comes from the data cursor
    DbGridRowRef    m_xSeekRow,         // Row to which the iterator can set
                                        // comes from the data cursor

                    m_xEmptyRow;        // record set to insert

    ImplSVEvent *   m_nAsynAdjustEvent;

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
    ImplSVEvent *       m_nDeleteEvent;     // EventId for asychronous deletion of rows
    sal_uInt16          m_nOptions;         // What is the able to do (Insert, Update, Delete)
                                        // default readonly
    sal_uInt16          m_nOptionMask;      // the mask of options to be enabled in setDataSource
                                        // (with respect to the data source capabilities)
                                        // defaults to (insert | update | delete)
    sal_uInt16          m_nLastColId;
    long                m_nLastRowId;

    bool                m_bDesignMode : 1;      // default = sal_False
    bool                m_bRecordCountFinal : 1;
    bool                m_bMultiSelection   : 1;
    bool                m_bNavigationBar      : 1;

    bool                m_bSynchDisplay : 1;
    bool                m_bForceROController : 1;
    bool                m_bHandle : 1;
    bool                m_bFilterMode : 1;
    bool                m_bWantDestruction : 1;
    bool                m_bInAdjustDataSource : 1;
    bool                m_bPendingAdjustRows : 1;   // if an async adjust is pending, is it for AdjustRows or AdjustDataSource ?
    bool                m_bHideScrollbars : 1;

protected:
    bool                m_bUpdating : 1;            // are any updates being executed right now?

protected:
    virtual bool SeekRow(long nRow) SAL_OVERRIDE;
    virtual void VisibleRowsChanged( long nNewTopRow, sal_uInt16 nNumRows) SAL_OVERRIDE;
    virtual void PaintStatusCell(OutputDevice& rDev, const Rectangle& rRect) const SAL_OVERRIDE;
    virtual void PaintCell(OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColId) const SAL_OVERRIDE;
    virtual RowStatus GetRowStatus(long nRow) const SAL_OVERRIDE;
    virtual bool CursorMoving(long nNewRow, sal_uInt16 nNewCol) SAL_OVERRIDE;
    virtual void CursorMoved() SAL_OVERRIDE;
    virtual void ArrangeControls(sal_uInt16& nX, sal_uInt16 nY) SAL_OVERRIDE;
    virtual sal_uInt32 GetTotalCellWidth(long nRow, sal_uInt16 nColId) SAL_OVERRIDE;
    virtual void Command(const CommandEvent& rEvt) SAL_OVERRIDE;
    virtual bool PreNotify(NotifyEvent& rEvt) SAL_OVERRIDE;
    virtual void KeyInput(const KeyEvent& rEvt) SAL_OVERRIDE;
    virtual void StateChanged( StateChangedType nType ) SAL_OVERRIDE;
    virtual void DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
    virtual void Select() SAL_OVERRIDE;

    virtual ::svt::CellController* GetController(long nRow, sal_uInt16 nCol) SAL_OVERRIDE;

    virtual void CellModified() SAL_OVERRIDE;
    virtual bool SaveModified() SAL_OVERRIDE;
    virtual bool IsModified() const SAL_OVERRIDE;

    virtual sal_uInt16 AppendColumn(const OUString& rName, sal_uInt16 nWidth = 0, sal_uInt16 nPos = HEADERBAR_APPEND, sal_uInt16 nId = (sal_uInt16)-1) SAL_OVERRIDE;
    void RemoveColumn(sal_uInt16 nId);
    DbGridColumn* CreateColumn(sal_uInt16 nId) const;
    virtual void ColumnMoved(sal_uInt16 nId) SAL_OVERRIDE;
    virtual bool SaveRow() SAL_OVERRIDE;
    virtual bool IsTabAllowed(bool bForward) const SAL_OVERRIDE;

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

    void DataSourcePropertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException, std::exception);

    void FieldValueChanged(sal_uInt16 _nId, const ::com::sun::star::beans::PropertyChangeEvent& _evt);
    void FieldListenerDisposing(sal_uInt16 _nId);

    void disposing(sal_uInt16 _nId, const ::com::sun::star::lang::EventObject& _rEvt);

    // own overridables
    /// called when the current row changed
    virtual void onRowChange();
    /// called when the current column changed
    virtual void onColumnChange();

    // DragSourceHelper overridables
    virtual void StartDrag( sal_Int8 nAction, const Point& rPosPixel ) SAL_OVERRIDE;

    void    executeRowContextMenu( long _nRow, const Point& _rPreferredPos );

public:
    DbGridControl(
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >,
        vcl::Window* pParent,
        WinBits nBits = WB_BORDER);

    virtual ~DbGridControl();
    virtual void dispose() SAL_OVERRIDE;

    virtual void Init() SAL_OVERRIDE;
    virtual void InitColumnsByFields(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xFields) = 0;
    virtual void RemoveRows() SAL_OVERRIDE;

    /** GetCellText returns the text at the given position
        @param  _nRow
            the number of the row
        @param  _nColId
            the ID of the column
        @return
            the text out of the cell
    */
    virtual OUString  GetCellText(long _nRow, sal_uInt16 _nColId) const SAL_OVERRIDE;

    void RemoveRows(bool bNewCursor);

    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& getNumberFormatter() const {return m_xFormatter;}

    // the data source
    // the options can restrict but not extend the update abilities
    void setDataSource(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& rCursor,
        sal_uInt16 nOpts = OPT_INSERT | OPT_UPDATE | OPT_DELETE);
    virtual void Dispatch(sal_uInt16 nId) SAL_OVERRIDE;

    CursorWrapper* getDataSource() const {return m_pDataCursor;}
    const DbGridColumns& GetColumns() const {return m_aColumns;}

    void EnableHandle(bool bEnable);
    bool HasHandle() const {return m_bHandle;}
    void InsertHandleColumn();

    // which position does the column with the id in the View have, the handle column doesn't count
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

    virtual void SetDesignMode(bool bMode);
    bool IsDesignMode() const {return m_bDesignMode;}
    bool IsOpen() const {return m_pSeekCursor != NULL;}

    void SetFilterMode(bool bMode);
    bool IsFilterMode() const {return m_bFilterMode;}
    bool IsFilterRow(long nRow) const {return m_bFilterMode && nRow == 0;}

    void EnableNavigationBar(bool bEnable);
    bool HasNavigationBar() const {return m_bNavigationBar;}

    sal_uInt16 GetOptions() const {return m_nOptions;}
    NavigationBar& GetNavigationBar() {return *m_aBar.get();}
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
    void AdjustDataSource(bool bFull = false);
    void Undo();

    virtual void BeginCursorAction();
    virtual void EndCursorAction();

    // is the current line being updated
    bool IsUpdating() const {return m_bUpdating;}

    void RowRemoved( long nRow, long nNumRows = 1, bool bDoPaint = true );
    void RowInserted( long nRow, long nNumRows = 1, bool bDoPaint = true, bool bKeepSelection = false );
    void RowModified( long nRow, sal_uInt16 nColId = USHRT_MAX );

    void resetCurrentRow();

    bool getDisplaySynchron() const { return m_bSynchDisplay; }
    void setDisplaySynchron(bool bSync);
        // when set to sal_False, the display is no longer in sync with the current cursor position
        // (means that in AdjustDataSource we are jumping to a row not belonging to CursorPosition)
        // when using this, you should know what you are doing, because for example entering data
        // in a row in the display that is not in sync with the position of the cursor can be very critical

    const DbGridRowRef& GetCurrentRow() const {return m_xCurrentRow;}

    void SetStateProvider(const Link<sal_uInt16,int>& rProvider) { m_aMasterStateProvider = rProvider; }
        // if this link is set the given provider will be asked for the state of my items.
        // the return values are interpreted as follows :
        // <0 -> not specified (use default mechanism to determine the state)
        // ==0 -> the item is disabled
        // >0 -> the item is enabled
    void SetSlotExecutor(const Link<sal_uInt16,bool>& rExecutor) { m_aMasterSlotExecutor = rExecutor; }
        // analogous : if this link is set, all nav-bar slots will be routed through it when executed
        // if the handler returns nonzero, no further handling of the slot occurs

    void EnablePermanentCursor(bool bEnable);
    bool IsPermanentCursorEnabled() const;

    /** forces both scrollbars to be hidden

        For the horizontal scrollbar, this is overruled by enabling the navigation bar: A navigation
        bar <b>always</b> implies a horizontal scroll bar
        @seealso EnableNavigationBar
    */
    void        ForceHideScrollbars( bool _bForce );

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        getContext() const { return m_xContext; }

    /// returns <TRUE/> if the text of the given cell can be copied into the clipboard
    bool    canCopyCellText(sal_Int32 _nRow, sal_Int16 _nColId);
    /// copies the text of the given cell into the clipboard
    void        copyCellText(sal_Int32 _nRow, sal_Int16 _nColId);

    // select in listener handling
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
    virtual sal_Int32 GetAccessibleControlCount() const SAL_OVERRIDE;

    /** Creates the accessible object of an additional control.
        @param _nIndex
            The 0-based index of the control.
        @return
            The XAccessible interface of the specified control.
    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
    CreateAccessibleControl( sal_Int32 _nIndex ) SAL_OVERRIDE;

    // IAccessibleTableProvider
    /** Creates the accessible object of a data table cell.
        @param nRow  The row index of the cell.
        @param nColumnId  The column ID of the cell.
        @return  The XAccessible interface of the specified cell. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
    CreateAccessibleCell( sal_Int32 nRow, sal_uInt16 nColumnId ) SAL_OVERRIDE;

protected:
    void RecalcRows(long nNewTopRow, sal_uInt16 nLinesOnScreen, bool bUpdateCursor);
    bool SeekCursor(long nRow, bool bAbsolute = false);
    void RemoveColumns();       // cleaning of own structures
    void AdjustRows();
    sal_Int32 AlignSeekCursor();
    bool SetCurrent(long nNewRow);

    OUString GetCurrentRowCellText(DbGridColumn* pCol,const DbGridRowRef& _rRow) const;
    virtual void DeleteSelectedRows();
    static bool IsValid(const DbGridRowRef& _xRow) { return _xRow && _xRow->IsValid(); }

    // row which is currently being appended
    bool IsCurrentAppending() const;

    // empty row for insertion
    bool IsInsertionRow(long nRow) const;

    void  SetSeekPos(sal_Int32 nPos) {m_nSeekPos = nPos;}
    sal_Int32 GetCurrentPos() const {return m_nCurrentPos;}
    sal_Int32 GetSeekPos() const {return m_nSeekPos;}
    sal_Int32 GetTotalCount() const {return m_nTotalCount;}

    const DbGridRowRef& GetEmptyRow() const { return m_xEmptyRow;   }
    const DbGridRowRef& GetSeekRow() const  { return m_xSeekRow;    }
    const DbGridRowRef& GetPaintRow() const { return m_xPaintRow;   }

    void ConnectToFields();
    void DisconnectFromFields();

    void implAdjustInSolarThread(bool _bRows);
        // calls AdjustRows or AdjustDataSource, synchron if the caller is running in the solar thread, else asynchron

protected:
    void ImplInitWindow( const InitWindowFacet _eInitWhat );
    DECL_LINK_TYPED(OnDelete, void*, void);

    DECL_LINK_TYPED(OnAsyncAdjust, void*, void);
        // if the param is != NULL, AdjustRows will be called, else AdjustDataSource

private:
    using BrowseBox::InsertHandleColumn;
};

#endif // INCLUDED_SVX_GRIDCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
