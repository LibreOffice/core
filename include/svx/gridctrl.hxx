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
#include <com/sun/star/uno/XComponentContext.hpp>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>

#include <vcl/button.hxx>
#include <tools/ref.hxx>
#include <svtools/editbrowsebox.hxx>
#include <osl/mutex.hxx>
#include <comphelper/propmultiplex.hxx>
#include <vcl/transfer.hxx>
#include <svx/svxdllapi.h>
#include <o3tl/typed_flags_set.hxx>
#include <memory>
#include <vector>

class DbGridControl;
class CursorWrapper;

bool CompareBookmark(const css::uno::Any& aLeft, const css::uno::Any& aRight);

namespace svxform
{
    class DataColumn;
}

enum class GridRowStatus
{
    Clean,
    Modified,
    Deleted,
    Invalid
};


// DbGridRow, description of rows


class DbGridRow : public SvRefBase
{
    css::uno::Any  m_aBookmark;        // Bookmark of the row, can be set
    ::std::vector< std::unique_ptr<::svxform::DataColumn> >
                                m_aVariants;
    GridRowStatus               m_eStatus;
    bool                        m_bIsNew;
                                                    // row is no longer valid
                                                    // is removed on the next positioning
public:
    DbGridRow();
    DbGridRow(CursorWrapper* pCur, bool bPaintCursor);
    void SetState(CursorWrapper* pCur, bool bPaintCursor);

    virtual ~DbGridRow() override;

    bool HasField(sal_uInt32 nPos) const { return nPos < m_aVariants.size(); }
    const ::svxform::DataColumn& GetField(sal_uInt32 nPos) const { return *m_aVariants[ nPos ]; }

    void            SetStatus(GridRowStatus _eStat) { m_eStatus = _eStat; }
    GridRowStatus   GetStatus() const               { return m_eStatus; }
    void            SetNew(bool _bNew)              { m_bIsNew = _bNew; }
    bool            IsNew() const                   { return m_bIsNew; }

    const css::uno::Any& GetBookmark() const { return m_aBookmark; }

    bool    IsValid() const { return m_eStatus == GridRowStatus::Clean || m_eStatus == GridRowStatus::Modified; }
    bool    IsModified() const { return m_eStatus == GridRowStatus::Modified; }
};

typedef tools::SvRef<DbGridRow> DbGridRowRef;


// DbGridControl

class DbGridColumn;

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

enum class InitWindowFacet
{
    Font        = 0x01,
    Foreground  = 0x02,
    Background  = 0x04,
    WritingMode = 0x08,
    All         = 0x0F
};
namespace o3tl
{
    template<> struct typed_flags<InitWindowFacet> : is_typed_flags<InitWindowFacet, 0x0f> {};
}


// these options are or'ed and indicate, which of the single
// features can be released, default is readonly which means 0
enum class DbGridControlOptions
{
    Readonly    = 0x00,
    Insert      = 0x01,
    Update      = 0x02,
    Delete      = 0x04
};
namespace o3tl
{
    template<> struct typed_flags<DbGridControlOptions> : is_typed_flags<DbGridControlOptions, 0x07> {};
}

//  StatusIds for Controls of the Bar
//  important for invalidation
enum class DbGridControlNavigationBarState
{
    NONE,
    Text,
    Absolute,
    Of,
    Count,
    First,
    Next,
    Prev,
    Last,
    New,
    Undo // related to SID_FM_RECORD_UNDO
};

class FmXGridSourcePropListener;
class DisposeListenerGridBridge;
class SVX_DLLPUBLIC DbGridControl : public svt::EditBrowseBox
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
            AbsolutePos(vcl::Window* pParent, WinBits nStyle);

            virtual void KeyInput(const KeyEvent& rEvt) override;
            virtual void LoseFocus() override;
        };

        friend class NavigationBar::AbsolutePos;

        //  additional controls
        VclPtr<FixedText>    m_aRecordText;
        VclPtr<AbsolutePos>  m_aAbsolute;            // absolute positioning
        VclPtr<FixedText>    m_aRecordOf;
        VclPtr<FixedText>    m_aRecordCount;

        VclPtr<ImageButton>  m_aFirstBtn;            // ImageButton for 'go to the first record'
        VclPtr<ImageButton>  m_aPrevBtn;         // ImageButton for 'go to the previous record'
        VclPtr<ImageButton>  m_aNextBtn;         // ImageButton for 'go to the next record'
        VclPtr<ImageButton>  m_aLastBtn;         // ImageButton for 'go to the last record'
        VclPtr<ImageButton>  m_aNewBtn;          // ImageButton for 'go to a new record'
        sal_Int32            m_nCurrentPos;

        bool                 m_bPositioning;     // protect PositionDataSource against recursion

    public:
        NavigationBar(vcl::Window* pParent);
        virtual ~NavigationBar() override;
        virtual void dispose() override;

        // Status methods for Controls
        void InvalidateAll(sal_Int32 nCurrentPos, bool bAll = false);
        void InvalidateState(DbGridControlNavigationBarState nWhich) {SetState(nWhich);}
        void SetState(DbGridControlNavigationBarState nWhich);
        bool GetState(DbGridControlNavigationBarState nWhich) const;
        sal_uInt16 ArrangeControls();

    protected:
        virtual void Resize() override;
        virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
        virtual void StateChanged( StateChangedType nType ) override;

    private:
        DECL_LINK(OnClick, Button*, void);

        void PositionDataSource(sal_Int32 nRecord);
    };

    friend class DbGridControl::NavigationBar;

private:
    Link<DbGridControlNavigationBarState,int>    m_aMasterStateProvider;
    Link<DbGridControlNavigationBarState,bool>   m_aMasterSlotExecutor;

    css::uno::Reference< css::util::XNumberFormatter >    m_xFormatter;
    css::uno::Reference< css::uno::XComponentContext >    m_xContext;

    std::vector< std::unique_ptr<DbGridColumn> > m_aColumns;         // Column description
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
    rtl::Reference<::comphelper::OPropertyChangeMultiplexer>  m_pDataSourcePropMultiplexer;
    FmXGridSourcePropListener*                      m_pDataSourcePropListener;
    css::uno::Reference< css::sdb::XRowsChangeListener>
                                                    m_xRowSetListener; // get notification when rows were changed

    void*                                           m_pFieldListeners;
        // property listeners for field values

    std::unique_ptr<DisposeListenerGridBridge>      m_pCursorDisposeListener;
        // need to know about the disposing of the seek cursor
        // construct analogous to the data source proplistener/multiplexer above :
        // DisposeListenerGridBridge is a bridge from FmXDisposeListener which I don't want to be derived from

    FmGridListener*                                 m_pGridListener;

protected:
    std::unique_ptr<CursorWrapper> m_pDataCursor;      // Cursor for Updates
    std::unique_ptr<CursorWrapper> m_pSeekCursor;      // Cursor for Seeking

private:
    // iteration variables
    DbGridRowRef        m_xCurrentRow;
    DbGridRowRef        m_xPaintRow;        // Row to be displayed
    sal_Int32           m_nSeekPos;         // Position of the SeekCursor
    sal_Int32           m_nTotalCount;      // is set when the data cursor finished counting the
                                            // records. Initial value is -1
    osl::Mutex          m_aDestructionSafety;
    osl::Mutex          m_aAdjustSafety;

    css::util::Date
                        m_aNullDate;        // NullDate of the Numberformatter;

    BrowserMode         m_nMode;
    sal_Int32           m_nCurrentPos;      // Current position;
    ImplSVEvent *       m_nDeleteEvent;     // EventId for asychronous deletion of rows
    DbGridControlOptions m_nOptions;        // What is the able to do (Insert, Update, Delete)
                                            // default readonly
    DbGridControlOptions m_nOptionMask;     // the mask of options to be enabled in setDataSource
                                            // (with respect to the data source capabilities)
                                            // defaults to (insert | update | delete)
    sal_uInt16          m_nLastColId;
    long                m_nLastRowId;

    bool                m_bDesignMode : 1;      // default = sal_False
    bool                m_bRecordCountFinal : 1;
    bool                m_bNavigationBar      : 1;

    bool                m_bSynchDisplay : 1;
    bool                m_bHandle : 1;
    bool                m_bFilterMode : 1;
    bool                m_bWantDestruction : 1;
    bool                m_bPendingAdjustRows : 1;   // if an async adjust is pending, is it for AdjustRows or AdjustDataSource ?
    bool                m_bHideScrollbars : 1;

protected:
    bool                m_bUpdating : 1;            // are any updates being executed right now?

protected:
    virtual bool SeekRow(long nRow) override;
    virtual void VisibleRowsChanged( long nNewTopRow, sal_uInt16 nNumRows) override;
    virtual void PaintCell(OutputDevice& rDev, const tools::Rectangle& rRect, sal_uInt16 nColId) const override;
    virtual RowStatus GetRowStatus(long nRow) const override;
    virtual bool CursorMoving(long nNewRow, sal_uInt16 nNewCol) override;
    virtual void CursorMoved() override;
    virtual void ArrangeControls(sal_uInt16& nX, sal_uInt16 nY) override;
    virtual sal_uInt32 GetTotalCellWidth(long nRow, sal_uInt16 nColId) override;
    virtual void Command(const CommandEvent& rEvt) override;
    virtual bool PreNotify(NotifyEvent& rEvt) override;
    virtual void KeyInput(const KeyEvent& rEvt) override;
    virtual void StateChanged( StateChangedType nType ) override;
    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void Select() override;

    virtual ::svt::CellController* GetController(long nRow, sal_uInt16 nCol) override;

    virtual void CellModified() override;
    virtual bool SaveModified() override;
    virtual bool IsModified() const override;

    virtual sal_uInt16 AppendColumn(const OUString& rName, sal_uInt16 nWidth, sal_uInt16 nPos = HEADERBAR_APPEND, sal_uInt16 nId = sal_uInt16(-1)) override;
    void RemoveColumn(sal_uInt16 nId);
    std::unique_ptr<DbGridColumn> CreateColumn(sal_uInt16 nId) const;
    virtual void ColumnMoved(sal_uInt16 nId) override;
    virtual bool SaveRow() override;
    virtual bool IsTabAllowed(bool bForward) const override;

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

    /// @throws css::uno::RuntimeException
    void DataSourcePropertyChanged(const css::beans::PropertyChangeEvent& evt);

    void FieldValueChanged(sal_uInt16 _nId);
    void FieldListenerDisposing(sal_uInt16 _nId);

    void disposing(sal_uInt16 _nId);

    // own overridables
    /// called when the current row changed
    virtual void onRowChange();
    /// called when the current column changed
    virtual void onColumnChange();

    // DragSourceHelper overridables
    virtual void StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;

    void    executeRowContextMenu( long _nRow, const Point& _rPreferredPos );

public:
    DbGridControl(
        css::uno::Reference< css::uno::XComponentContext > const & _rxContext,
        vcl::Window* pParent,
        WinBits nBits);

    virtual ~DbGridControl() override;
    virtual void dispose() override;

    virtual void Init() override;
    virtual void InitColumnsByFields(const css::uno::Reference< css::container::XIndexAccess >& xFields) = 0;
    virtual void RemoveRows() override;

    /** GetCellText returns the text at the given position
        @param  _nRow
            the number of the row
        @param  _nColId
            the ID of the column
        @return
            the text out of the cell
    */
    virtual OUString  GetCellText(long _nRow, sal_uInt16 _nColId) const override;

    void RemoveRows(bool bNewCursor);

    const css::uno::Reference< css::util::XNumberFormatter >& getNumberFormatter() const {return m_xFormatter;}

    // the data source
    // the options can restrict but not extend the update abilities
    void setDataSource(const css::uno::Reference< css::sdbc::XRowSet >& rCursor,
        DbGridControlOptions nOpts = DbGridControlOptions::Insert | DbGridControlOptions::Update | DbGridControlOptions::Delete);
    virtual void Dispatch(sal_uInt16 nId) override;

    CursorWrapper* getDataSource() const {return m_pDataCursor.get();}
    const std::vector< std::unique_ptr<DbGridColumn> >& GetColumns() const {return m_aColumns;}

    void EnableHandle(bool bEnable);
    bool HasHandle() const {return m_bHandle;}
    void InsertHandleColumn();

    // which position does the column with the id in the View have, the handle column doesn't count
    sal_uInt16 GetViewColumnPos( sal_uInt16 nId ) const { sal_uInt16 nPos = GetColumnPos(nId); return (nPos==BROWSER_INVALIDID) ? GRID_COLUMN_NOT_FOUND : nPos-1; }

    // which position does the column with the id in m_aColumns have, that means the css::sdbcx::Container
    // returned from the GetColumns (may be different from the position returned by GetViewColumnPos
    // if there are hidden columns)
    sal_uInt16 GetModelColumnPos( sal_uInt16 nId ) const;

    // the number of columns in the model
    sal_uInt16 GetViewColCount() const { return ColCount() - 1; }
    sal_uInt16 GetModelColCount() const { return static_cast<sal_uInt16>(m_aColumns.size()); }
    // reverse to GetViewColumnPos: Id of position, the first non-handle column has position 0
    sal_uInt16 GetColumnIdFromViewPos( sal_uInt16 nPos ) const { return GetColumnId(nPos + 1); }
    sal_uInt16 GetColumnIdFromModelPos( sal_uInt16 nPos ) const;

    virtual void SetDesignMode(bool bMode);
    bool IsDesignMode() const {return m_bDesignMode;}
    bool IsOpen() const {return m_pSeekCursor != nullptr;}

    void SetFilterMode(bool bMode);
    bool IsFilterMode() const {return m_bFilterMode;}
    bool IsFilterRow(long nRow) const {return m_bFilterMode && nRow == 0;}

    void EnableNavigationBar(bool bEnable);
    bool HasNavigationBar() const {return m_bNavigationBar;}

    DbGridControlOptions GetOptions() const {return m_nOptions;}
    NavigationBar& GetNavigationBar() {return *m_aBar.get();}
    DbGridControlOptions SetOptions(DbGridControlOptions nOpt);
        // The new options are interpreted with respect to the current data source. If it is unable
        // to update, to insert or to restore, the according options are ignored. If the grid isn't
        // connected to a data source, all options except OPT_READONLY are ignored.

    const css::util::Date&   getNullDate() const {return m_aNullDate;}

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
    void RowInserted( long nRow, long nNumRows = 1, bool bDoPaint = true );
    void RowModified( long nRow );

    void resetCurrentRow();

    bool getDisplaySynchron() const { return m_bSynchDisplay; }
    void setDisplaySynchron(bool bSync);
        // when set to sal_False, the display is no longer in sync with the current cursor position
        // (means that in AdjustDataSource we are jumping to a row not belonging to CursorPosition)
        // when using this, you should know what you are doing, because for example entering data
        // in a row in the display that is not in sync with the position of the cursor can be very critical

    const DbGridRowRef& GetCurrentRow() const {return m_xCurrentRow;}

    void SetStateProvider(const Link<DbGridControlNavigationBarState,int>& rProvider) { m_aMasterStateProvider = rProvider; }
        // if this link is set the given provider will be asked for the state of my items.
        // the return values are interpreted as follows :
        // <0 -> not specified (use default mechanism to determine the state)
        // ==0 -> the item is disabled
        // >0 -> the item is enabled
    void SetSlotExecutor(const Link<DbGridControlNavigationBarState,bool>& rExecutor) { m_aMasterSlotExecutor = rExecutor; }
        // analogous : if this link is set, all nav-bar slots will be routed through it when executed
        // if the handler returns nonzero, no further handling of the slot occurs

    void EnablePermanentCursor(bool bEnable);
    bool IsPermanentCursorEnabled() const;

    /** forces both scrollbars to be hidden

        For the horizontal scrollbar, this is overruled by enabling the navigation bar: A navigation
        bar <b>always</b> implies a horizontal scroll bar
        @seealso EnableNavigationBar
    */
    void        ForceHideScrollbars();

    const css::uno::Reference< css::uno::XComponentContext >&
        getContext() const { return m_xContext; }

    /// returns <TRUE/> if the text of the given cell can be copied into the clipboard
    bool    canCopyCellText(sal_Int32 _nRow, sal_uInt16 _nColId);
    /// copies the text of the given cell into the clipboard
    void        copyCellText(sal_Int32 _nRow, sal_uInt16 _nColId);

    // select in listener handling
    void                        setGridListener( FmGridListener* _pListener ) { m_pGridListener = _pListener; }

    // helper class to grant access to selected methods from within the DbCellControl class
    struct GrantControlAccess final
    {
        friend class DbCellControl;
        friend class RowSetEventListener;
        GrantControlAccess() { }
    };

    /// called when a controller needs to be re-initialized
    void refreshController(sal_uInt16 _nColId, GrantControlAccess _aAccess);

    CursorWrapper* GetSeekCursor(GrantControlAccess /*_aAccess*/) const    { return m_pSeekCursor.get(); }
    const DbGridRowRef& GetSeekRow(GrantControlAccess /*_aAccess*/) const  { return m_xSeekRow;    }
    void  SetSeekPos(sal_Int32 nPos,GrantControlAccess /*_aAccess*/) {m_nSeekPos = nPos;}

    /**
        @return
            The count of additional controls of the control area.
    */
    virtual sal_Int32 GetAccessibleControlCount() const override;

    /** Creates the accessible object of an additional control.
        @param _nIndex
            The 0-based index of the control.
        @return
            The XAccessible interface of the specified control.
    */
    virtual css::uno::Reference<
        css::accessibility::XAccessible >
    CreateAccessibleControl( sal_Int32 _nIndex ) override;

    // IAccessibleTableProvider
    /** Creates the accessible object of a data table cell.
        @param nRow  The row index of the cell.
        @param nColumnId  The column ID of the cell.
        @return  The XAccessible interface of the specified cell. */
    virtual css::uno::Reference<
        css::accessibility::XAccessible >
    CreateAccessibleCell( sal_Int32 nRow, sal_uInt16 nColumnId ) override;

protected:
    void RecalcRows(long nNewTopRow, sal_uInt16 nLinesOnScreen, bool bUpdateCursor);
    bool SeekCursor(long nRow, bool bAbsolute = false);
    void RemoveColumns();       // cleaning of own structures
    void AdjustRows();
    sal_Int32 AlignSeekCursor();
    bool SetCurrent(long nNewRow);

    OUString GetCurrentRowCellText(DbGridColumn const * pCol,const DbGridRowRef& _rRow) const;
    virtual void DeleteSelectedRows();
    static bool IsValid(const DbGridRowRef& _xRow) { return _xRow.is() && _xRow->IsValid(); }

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
    DECL_LINK(OnDelete, void*, void);

    DECL_LINK(OnAsyncAdjust, void*, void);
        // if the param is != NULL, AdjustRows will be called, else AdjustDataSource

private:
    using BrowseBox::InsertHandleColumn;
};

#endif // INCLUDED_SVX_GRIDCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
