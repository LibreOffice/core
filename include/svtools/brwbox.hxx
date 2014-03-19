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
#ifndef INCLUDED_SVTOOLS_BRWBOX_HXX
#define INCLUDED_SVTOOLS_BRWBOX_HXX

#include <svtools/svtdllapi.h>
#include <vcl/scrbar.hxx>
#include <vcl/ctrl.hxx>
#include <tools/multisel.hxx>
#include <svtools/headbar.hxx>
#include <svtools/transfer.hxx>
#include <svtools/AccessibleBrowseBoxObjType.hxx>
#include <svtools/accessibletableprovider.hxx>
#include <vector>

#include <limits.h>
#include <memory>

class BrowserColumn;
class BrowserDataWin;
class MultiSelection;
class BrowserHeader;

typedef ::std::vector< BrowserColumn* > BrowserColumns;

namespace svt {
    class BrowseBoxImpl;
    class IAccessibleFactory;
}
namespace utl {
    class AccessibleStateSetHelper;
}


// - BrowseBox-Types -


#define BROWSER_INVALIDID           SAL_MAX_UINT16
#define BROWSER_ENDOFSELECTION      (static_cast<long>(SFX_ENDOFSELECTION))

typedef sal_uLong BrowserMode;

#define BROWSER_COLUMNSELECTION      0x0001
#define BROWSER_MULTISELECTION       0x0002
#define BROWSER_THUMBDRAGGING        0x0004
#define BROWSER_KEEPHIGHLIGHT        0x0008
#define BROWSER_KEEPSELECTION        BROWSER_KEEPHIGHLIGHT  // old, don't use!
#define BROWSER_HLINES               0x0010
#define BROWSER_VLINES               0x0020
#define BROWSER_HLINESFULL           BROWSER_HLINES // old, don't use!
#define BROWSER_VLINESFULL           BROWSER_VLINES // old, don't use!
#define BROWSER_HLINESDOTS           0x0000 // old => don't use!
#define BROWSER_VLINESDOTS           0x0000 // old => don't use!

#define BROWSER_HIDESELECT           0x0100 // old => don't use!
#define BROWSER_HIDECURSOR           0x0200

#define BROWSER_NO_HSCROLL           0x0400
#define BROWSER_NO_SCROLLBACK        0x0800

#define BROWSER_AUTO_VSCROLL         0x1000
#define BROWSER_AUTO_HSCROLL         0x2000

#define BROWSER_TRACKING_TIPS        0x4000

#define BROWSER_NO_VSCROLL           0x8000

#define BROWSER_HIGHLIGHT_NONE       0x0100 // == BROWSER_HIDESELECT

#define BROWSER_HEADERBAR_NEW    0x00040000
#define BROWSER_AUTOSIZE_LASTCOL 0x00080000
#define BROWSER_OWN_DATACHANGED  0x00100000

#define BROWSER_CURSOR_WO_FOCUS  0x00200000
    // Allows a cursor which is shown even if the control does not have the focus. This does not affect other
    // situations which require to temporarily hide the cursor (such as scrolling).

#define BROWSER_SMART_HIDECURSOR 0x00400000
    // is an enhanced version of BROWSER_HIDECURSOR.
    // When set, BROWSER_HIDECURSOR is overruled, and the cursor is hidden as long as no selection exists,
    // but shown otherwise. This does not affect other situations which require to temporarily hide the
    // cursor (such as scrolling).

#define BROWSER_NONE                      0
#define BROWSER_SELECT                  720
#define BROWSER_ENHANCESELECTION        722
#define BROWSER_SELECTDOWN              724
#define BROWSER_SELECTUP                725
#define BROWSER_CURSORDOWN              731
#define BROWSER_CURSORUP                732
#define BROWSER_CURSORLEFT              733
#define BROWSER_CURSORRIGHT             734
#define BROWSER_CURSORPAGEDOWN          735
#define BROWSER_CURSORPAGEUP            736
#define BROWSER_CURSORENDOFFILE         741
#define BROWSER_CURSORTOPOFFILE         742
#define BROWSER_CURSORENDOFSCREEN       743
#define BROWSER_CURSORTOPOFSCREEN       744
#define BROWSER_CURSORHOME              745
#define BROWSER_CURSOREND               746
#define BROWSER_SCROLLDOWN              751
#define BROWSER_SCROLLUP                752
#define BROWSER_SELECTHOME              753
#define BROWSER_SELECTEND               754
#define BROWSER_SELECTCOLUMN            755
#define BROWSER_MOVECOLUMNLEFT          756
#define BROWSER_MOVECOLUMNRIGHT         757


// - BrowseEvent -


class BrowseEvent
{
    Window*             pWin;
    long                nRow;
    Rectangle           aRect;
    sal_uInt16              nCol;
    sal_uInt16              nColId;

public:
                        BrowseEvent();
                        BrowseEvent( Window* pWindow,
                                     long nAbsRow,
                                     sal_uInt16 nColumn, sal_uInt16 nColumnId,
                                     const Rectangle& rRect );

    Window*             GetWindow() const { return pWin; }
    long                GetRow() const { return nRow; }
    sal_uInt16              GetColumn() const { return nCol; }
    sal_uInt16              GetColumnId() const { return nColId; }
    const Rectangle&    GetRect() const { return aRect; }
};


// - BrowserMouseEvent -


class BrowserMouseEvent: public MouseEvent, public BrowseEvent
{
public:
    BrowserMouseEvent();
    BrowserMouseEvent( BrowserDataWin* pWin, const MouseEvent& rEvt );
    BrowserMouseEvent( Window* pWin, const MouseEvent& rEvt,
                       long nAbsRow, sal_uInt16 nColumn, sal_uInt16 nColumnId,
                       const Rectangle& rRect );
};


// - BrowserAcceptDropEvent -


class BrowserAcceptDropEvent : public AcceptDropEvent, public BrowseEvent
{
public:
    BrowserAcceptDropEvent();
    BrowserAcceptDropEvent( BrowserDataWin* pWin, const AcceptDropEvent& rEvt );
};


// - BrowserExecuteDropEvent -


class BrowserExecuteDropEvent : public ExecuteDropEvent, public BrowseEvent
{
public:
    BrowserExecuteDropEvent();
    BrowserExecuteDropEvent( BrowserDataWin* pWin, const ExecuteDropEvent& rEvt );
};


// - BrowseBox -


// TODO
// The whole selection thingie in this class is somewhat .... suspicious to me.
// some oddities:
// * method parameters named like members (and used in both semantics within the method!)
// * the multi selection flag is sometimes used as if it is for row selection, sometimes as if
//   it's for column selection, too (and sometimes in an even stranger way :)
// * it is not really defined like all these hundreds selection related flags/methods work together
//   and influence each other. I do not understand it very well, but this may be my fault :)
// * There is a GetColumnSelection, but it can't be used to determine the selected columns (at least
//   not without a const_cast)
//
// We should clearly define this somewhere in the future. Or, even better, we should re-implement this
// whole class, which is planned for a long time :)
//
// sorry for the ranting. could not resist

class SVT_DLLPUBLIC BrowseBox
        :public Control
        ,public DragSourceHelper
        ,public DropTargetHelper
        ,public svt::IAccessibleTableProvider
{
    friend class BrowserDataWin;
    friend class ::svt::BrowseBoxImpl;

#ifdef DBG_UTIL
    friend const char* BrowseBoxCheckInvariants( const void * pVoid );
#endif

public:
    static const sal_uInt16 HandleColumnId = 0;

private:
    Window*         pDataWin;       // window to display data rows
    ScrollBar*      pVScroll;       // vertical scrollbar
    ScrollBar       aHScroll;       // horizontal scrollbar

    long            nDataRowHeight; // height of a single data-row
    sal_uInt16      nTitleLines;    // number of lines in title row
    sal_uLong       nControlAreaWidth; // width of fixed area beneeth hscroll
    bool            bThumbDragging; // handle thumb dragging
    bool            bColumnCursor;  // single columns and fields selectable
    bool            bMultiSelection;// allow multiple selected rows
    bool            bKeepHighlight; // don't hide selection on LoseFocus

    bool            bHLines;        // draw lines between rows
    bool            bVLines;        // draw lines between columns
    bool            bHDots;         // draw lines between rows dotted
    bool            bVDots;         // draw lines between columns dotted
    Color           aGridLineColor;     // color for lines, default dark grey
    bool            bBootstrapped;  // child windows resized etc.
    long            nTopRow;        // no. of first visible row (0...)
    long            nCurRow;        // no. of row with cursor
    long            nRowCount;      // total number of rows in model
    sal_uInt16      nFirstCol;      // no. of first visible scrollable column
    sal_uInt16      nCurColId;      // column id of cursor

    bool            bSelecting;
    bool            bRowDividerDrag;
    bool            bHit;
    bool            mbInteractiveRowHeight;
    Point           a1stPoint;
    Point           a2ndPoint;

    long            nResizeX;       // mouse position at start of resizing
    long            nMinResizeX;    // never drag more left
    long            nDragX;         // last dragged column (MouseMove)
    sal_uInt16      nResizeCol;     // resize this column in MouseMove
    bool            bResizing;      // mouse captured for column resizing

    bool            bSelect;        /// select or deselect
    bool            bSelectionIsVisible; // depending on focus
    bool            bScrolling;     // hidden cursor while scrolling
    bool            bNotToggleSel;  // set while in ToggleSelection() etc.
    bool            bHasFocus;      // set/unset in Get/LoseFocus
    bool            bHideSelect;    // hide selection (highlight)
    TriState        bHideCursor;    // hide cursor (frame)
    Range           aSelRange;      // for selection expansion

    BrowserColumns* pCols;          // array of column-descriptions
    union
    {
        MultiSelection* pSel;       // selected rows for multi-selection
        long            nSel;       // selected row for single-selection
    }               uRow;
    MultiSelection* pColSel;        // selected column-ids

    ::std::auto_ptr< ::svt::BrowseBoxImpl >  m_pImpl;       // impl structure of the BrowseBox object

    bool            m_bFocusOnlyCursor; // hide cursor if we don't have the focus
    Color           m_aCursorColor;     // special color for cursor, COL_TRANSPARENT for usual (VCL-painted) "inverted" cursor
    BrowserMode     m_nCurrentMode;     // last argument of SetMode (redundant, as our other members represent the current settings, too)

private:
    SVT_DLLPRIVATE void            ConstructImpl(BrowserMode nMode);
    SVT_DLLPRIVATE void            ExpandRowSelection( const BrowserMouseEvent& rEvt );
    SVT_DLLPRIVATE void            ToggleSelection( bool bForce = false );

    SVT_DLLPRIVATE void            UpdateScrollbars();
    SVT_DLLPRIVATE void            AutoSizeLastColumn();

    SVT_DLLPRIVATE long            ImpGetDataRowHeight() const;
    SVT_DLLPRIVATE Rectangle       ImplFieldRectPixel( long nRow, sal_uInt16 nColId ) const;
    SVT_DLLPRIVATE sal_uInt16          FrozenColCount() const;

    SVT_DLLPRIVATE void            ColumnInserted( sal_uInt16 nPos );

    DECL_DLLPRIVATE_LINK(       ScrollHdl, ScrollBar * );
    DECL_DLLPRIVATE_LINK(       EndScrollHdl, void * );
    DECL_DLLPRIVATE_LINK(       StartDragHdl, HeaderBar * );

    SVT_DLLPRIVATE long            GetFrozenWidth() const;

    bool            GoToRow(long nRow, bool bRowColMove, bool bDoNotModifySelection = false );

    bool            GoToColumnId( sal_uInt16 nColId, bool bMakeVisible, bool bRowColMove = false);
    void            SelectColumnPos( sal_uInt16 nCol, bool _bSelect, bool bMakeVisible);
    void            SelectColumnId( sal_uInt16 nColId, bool _bSelect, bool bMakeVisible)
                        { SelectColumnPos( GetColumnPos(nColId), _bSelect, bMakeVisible); }

    void            ImplPaintData(OutputDevice& _rOut, const Rectangle& _rRect, bool _bForeignDevice, bool _bDrawSelections);

    bool            PaintCursorIfHiddenOnce() const { return !m_bFocusOnlyCursor && !HasFocus(); }

    sal_uInt16      ToggleSelectedColumn();
    void            SetToggledSelectedColumn(sal_uInt16 _nSelectedColumnId);

protected:
    /// retrieves the XAccessible implementation associated with the BrowseBox instance
    ::svt::IAccessibleFactory&   getAccessibleFactory();

protected:
    sal_uInt16          ColCount() const;

    // software plug for database access
    // Der RowCount wird jetzt intern automatisch gezaehlt
    // (ueber RowInserted und RowRemoved), daher ist das Ueberladen
    // dieser Methode ueberfluessig!
public:
    virtual long    GetRowCount() const;

protected:
    // fuer Anzeige im VScrollBar z.B. auf "?" oder setzen
    void            SetRealRowCount( const OUString &rRealRowCount );

    // Return Value muss immer sal_True sein - SeekRow *muss* klappen!
    // (sonst ASSERT) MI: wer hat das eingebaut? Das darf nicht so sein!

    /** seeks for the given row position
        @param nRow
            nRow starts at 0
    */
    virtual bool    SeekRow( long nRow ) = 0;
    virtual void    DrawCursor();
    virtual void    PaintRow( OutputDevice &rDev, const Rectangle &rRect );
    virtual void    PaintData( Window& rWin, const Rectangle& rRect );
    virtual void    PaintField( OutputDevice& rDev, const Rectangle& rRect,
                                sal_uInt16 nColumnId ) const = 0;
    // Benachrichtigung an die abgeleitete Klasse, dass sich der sichtbare
    // Bereich von Rows geaendert hat. Aus dieser Methode heraus darf
    // die abgeleitete Klasse Aenderungen des Model mit Hilfe der Methoden
    // RowInserted und RowRemoved bekanntgeben. Mit sich daraus ergebenden
    // neuen Zustand wird anschliessend ein Paint veranlasst (und entsprechend
    // SeekRow etc. gerufen).
    //
    // Parameter: nNewTopRow: Nr. der neuen TopRow (kann von VisibleRowsChanged
    // durch Aufruf von RowInserted und RowDeleted noch veraendert werden).
    // nNumRows: Anzahl der sichtbaren Rows (auch eine teilweise sichtbare Row
    // wird mitgezaehlt).
    //
    // Moegliche Ursachen fuer die Aenderung des sichtbaren Bereiches:
    // - Vor dem sichtbaren Bereich sind Rows eingefuegt oder geloescht worden,
    //   dadurch aendert sich nur die Numerierung der sichtbaren Rows
    // - Scrollen (und daraus resultierend eine andere erste sichtbare Row)
    // - Resize des Fensters
    virtual void    VisibleRowsChanged( long nNewTopRow, sal_uInt16 nNumRows);

    // Anzahl sichtbarer Rows in dem Fenster (inkl. "angeschnittener" Rows)
    sal_uInt16          GetVisibleRows()
                        { return (sal_uInt16)((pDataWin->GetOutputSizePixel().Height() - 1 )/ GetDataRowHeight() + 1); }
    long            GetTopRow() { return nTopRow; }
    sal_uInt16          GetFirstVisibleColNumber() const { return nFirstCol; }

    // Focus-Rect ein-/ausschalten
    void            DoShowCursor( const char *pWhoLog );
    void            DoHideCursor( const char *pWhoLog );
    short           GetCursorHideCount() const;

    virtual BrowserHeader*  CreateHeaderBar( BrowseBox* pParent );

    // HACK(virtuelles Create wird im Ctor nicht gerufen)
    void            SetHeaderBar( BrowserHeader* );

    long            CalcReverseZoom(long nVal);

    inline const DataFlavorExVector&
                    GetDataFlavors() const;

    bool            IsDropFormatSupported( SotFormatStringId nFormat );     // need this because the base class' IsDropFormatSupported is not const ...
    bool            IsDropFormatSupported( SotFormatStringId nFormat ) const;

    bool            IsDropFormatSupported( const css::datatransfer::DataFlavor& _rFlavor );        // need this because the base class' IsDropFormatSupported is not const ...
    bool            IsDropFormatSupported( const css::datatransfer::DataFlavor& _rFlavor ) const;

private:
    void*           implGetDataFlavors() const;
        // with this we can make GetDataFlavors() inline, which is strongly needed as SVTOOLS does not export
        // any sysbols containing an "_STL", so a non-inlined method would not be exported ....

protected:
    // callbacks for the data window
    virtual void    ImplStartTracking();
    virtual void    ImplTracking();
    virtual void    ImplEndTracking();

public:
                    BrowseBox( Window* pParent, WinBits nBits = 0,
                               BrowserMode nMode = 0 );
                    BrowseBox( Window* pParent, const ResId& rId,
                               BrowserMode nMode = 0 );
                    ~BrowseBox();

    // ererbte ueberladene Handler
    virtual void    StateChanged( StateChangedType nStateChange );
    virtual void    MouseButtonDown( const MouseEvent& rEvt );
    virtual void    MouseMove( const MouseEvent& rEvt );
    virtual void    MouseButtonUp( const MouseEvent& rEvt );
    virtual void    KeyInput( const KeyEvent& rEvt );
    virtual void    LoseFocus();
    virtual void    GetFocus();
    virtual void    Resize();
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    Command( const CommandEvent& rEvt );
    virtual void    StartDrag( sal_Int8 _nAction, const Point& _rPosPixel );

    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt );     // will forward everything got to the second AcceptDrop method
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt );   // will forward everything got to the second ExecuteDrop method

    virtual sal_Int8 AcceptDrop( const BrowserAcceptDropEvent& rEvt );
    virtual sal_Int8 ExecuteDrop( const BrowserExecuteDropEvent& rEvt );

    // neue Handler
    virtual void    MouseButtonDown( const BrowserMouseEvent& rEvt );
    virtual void    MouseMove( const BrowserMouseEvent& rEvt );
    virtual void    MouseButtonUp( const BrowserMouseEvent& rEvt );
    virtual void    StartScroll();
    virtual void    EndScroll();
    virtual void    Select();
    virtual void    DoubleClick( const BrowserMouseEvent& rEvt );
    virtual bool    IsCursorMoveAllowed( long nNewRow, sal_uInt16 nNewColId ) const;
    virtual void    CursorMoved();
    virtual void    ColumnMoved( sal_uInt16 nColId );
    virtual void    ColumnResized( sal_uInt16 nColId );
    virtual long    QueryColumnResize( sal_uInt16 nColId, long nWidth );
    /// called when the row height has been changed interactively
    virtual void    RowHeightChanged();
    virtual long    QueryMinimumRowHeight();

    // Window-Control (pass to DataWindow)
    void            SetUpdateMode( bool bUpdate );
    bool            GetUpdateMode() const;

    // map-mode and font control
    void            SetFont( const Font& rNewFont );
    const Font&     GetFont() const { return pDataWin->GetFont(); }
    void            SetTitleFont( const Font& rNewFont )
                        { Control::SetFont( rNewFont ); }
    const Font&     GetTitleFont() const { return Control::GetFont(); }

    // color for line painting
    void            SetGridLineColor(const Color& rColor) {aGridLineColor = rColor;}
    const Color&    GetGridLineColor() const {return aGridLineColor;}

    // inserting, changing, removing and freezing of columns
    void            InsertHandleColumn( sal_uLong nWidth );
    void            InsertDataColumn( sal_uInt16 nItemId, const OUString& rText,
                                    long nSize, HeaderBarItemBits nBits = HIB_STDSTYLE,
                                    sal_uInt16 nPos = HEADERBAR_APPEND );
    void            SetColumnTitle( sal_uInt16 nColumnId, const OUString &rTitle );
    void            SetColumnWidth( sal_uInt16 nColumnId, sal_uLong nWidth );
    void            SetColumnPos( sal_uInt16 nColumnId, sal_uInt16 nPos );
    void            FreezeColumn( sal_uInt16 nColumnId, bool bFreeze = true );
    void            UnfreezeColumns();
    void            RemoveColumn( sal_uInt16 nColumnId );
    void            RemoveColumns();

    // control of title and data row height
    void            SetDataRowHeight( long nPixel );
    long            GetDataRowHeight() const;
    void            SetTitleLines( sal_uInt16 nLines );
    sal_uInt16          GetTitleLines() const { return nTitleLines; }
    virtual long    GetTitleHeight() const;

    // access to dynamic values of cursor row
    OUString        GetColumnTitle( sal_uInt16 nColumnId ) const;
    Rectangle       GetFieldRect( sal_uInt16 nColumnId ) const;
    sal_uLong       GetColumnWidth( sal_uInt16 nColumnId ) const;
    sal_uInt16      GetColumnId( sal_uInt16 nPos ) const;
    sal_uInt16      GetColumnPos( sal_uInt16 nColumnId ) const;
    bool            IsFrozen( sal_uInt16 nColumnId ) const;

    // movement of visible area
    void            ResetScroll();
    long            ScrollColumns( long nColumns );
    long            ScrollRows( long nRows );
    bool            MakeFieldVisible( long nRow, sal_uInt16 nColId, bool bComplete = false );

    // access and movement of cursor
    long            GetCurRow() const { return nCurRow; }
    sal_uInt16      GetCurColumnId() const { return nCurColId; }
    bool            GoToRow( long nRow );
    bool            GoToColumnId( sal_uInt16 nColId );
    bool            GoToRowColumnId( long nRow, sal_uInt16 nColId );

    // selections
    virtual void    SetNoSelection();
    virtual void    SelectAll();
    virtual void    SelectRow( long nRow, bool _bSelect = true, bool bExpand = true );
    void            SelectColumnPos( sal_uInt16 nCol, bool _bSelect = true )
                        { SelectColumnPos( nCol, _bSelect, true); }
    void            SelectColumnId( sal_uInt16 nColId, bool _bSelect = true )
                        { SelectColumnPos( GetColumnPos(nColId), _bSelect, true); }
    long            GetSelectRowCount() const;
    sal_uInt16          GetSelectColumnCount() const;
    virtual bool    IsRowSelected( long nRow ) const;
    bool            IsColumnSelected( sal_uInt16 nColumnId ) const;
    long            FirstSelectedRow( bool bInverse = false );
    long            LastSelectedRow();
    long            NextSelectedRow();
    const MultiSelection* GetColumnSelection() const { return pColSel; }
    const MultiSelection* GetSelection() const
                    { return bMultiSelection ? uRow.pSel : 0; }

    long            FirstSelectedColumn( ) const;

    bool            IsResizing() const { return bResizing; }

    // access to positions of fields, column and rows
    Window&         GetDataWindow() const { return *pDataWin; }
    Rectangle       GetRowRectPixel( long nRow,
                                     bool bRelToBrowser = true ) const;
    Rectangle       GetFieldRectPixel( long nRow, sal_uInt16 nColId,
                                       bool bRelToBrowser = true) const;
    bool            IsFieldVisible( long nRow, sal_uInt16 nColId,
                                    bool bComplete = false ) const;
    long            GetRowAtYPosPixel( long nY,
                                        bool bRelToBrowser = true  ) const;
    sal_uInt16          GetColumnAtXPosPixel( long nX,
                                          bool bRelToBrowser = true  ) const;

    // invalidations
    void            Clear();
    void            RowRemoved( long nRow, long nNumRows = 1, bool bDoPaint = true );
    void            RowModified( long nRow, sal_uInt16 nColId = BROWSER_INVALIDID );
    void            RowInserted( long nRow, long nNumRows = 1, bool bDoPaint = true, bool bKeepSelection = false );

    // miscellanous
    void            ReserveControlArea( sal_uInt16 nWidth = USHRT_MAX );
    Rectangle       GetControlArea() const;
    bool            ProcessKey( const KeyEvent& rEvt );
    void            Dispatch( sal_uInt16 nId );
    void            SetMode( BrowserMode nMode = 0 );
    BrowserMode     GetMode( ) const { return m_nCurrentMode; }

    void            SetCursorColor(const Color& _rCol);
    Color           GetCursorColor() const { return m_aCursorColor; }
    void            ResetSelecting() { bSelecting = false; }

    /** specifies that the user is allowed to interactively change the height of a row,
        by simply dragging an arbitrary row separator.

        Note that this works only if there's a handle column, since only in this case,
        there *is* something for the user to click onto
    */
    void            EnableInteractiveRowHeight( bool _bEnable = true ) { mbInteractiveRowHeight = _bEnable; }
    bool            IsInteractiveRowHeightEnabled( ) const { return mbInteractiveRowHeight; }

    /// access to selected methods, to be granted to the BrowserColumn
    struct BrowserColumnAccess { friend class BrowserColumn; private: BrowserColumnAccess() { } };
    /** public version of PaintField, with selected access rights for the BrowserColumn
    */
    void            DoPaintField( OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColumnId, BrowserColumnAccess ) const
                    { PaintField( rDev, rRect, nColumnId ); }

    /** suggests a default width for a column containing a given text

        The width is calculated so that the text fits completely, plus som margin.
    */
    sal_uLong         GetDefaultColumnWidth( const OUString& _rText ) const;

    /** GetCellText returns the text at the given position
        @param  _nRow
            the number of the row
        @param  _nColId
            the ID of the column
        @return
            the text out of the cell
    */
    virtual OUString  GetCellText(long _nRow, sal_uInt16 _nColId) const;

    /** @return
            the current column count
    */
    sal_uInt16 GetColumnCount() const { return ColCount(); }

    /** commitBrowseBoxEvent commit the event at all listeners of the browsebox
        @param nEventId
            the event id
        @param rNewValue
            the new value
        @param rOldValue
            the old value
    */
    void commitBrowseBoxEvent(sal_Int16 nEventId,
            const css::uno::Any& rNewValue,
            const css::uno::Any& rOldValue);

    /** commitTableEvent commit the event at all listeners of the table
        @param nEventId
            the event id
        @param rNewValue
            the new value
        @param rOldValue
            the old value
    */
    void commitTableEvent(sal_Int16 nEventId,
            const css::uno::Any& rNewValue,
            const css::uno::Any& rOldValue);

    /** fires an AccessibleEvent relative to a header bar AccessibleContext

        @param nEventId
            the event id
        @param rNewValue
            the new value
        @param rOldValue
            the old value
    */
    void commitHeaderBarEvent(sal_Int16 nEventId,
            const css::uno::Any& rNewValue,
            const css::uno::Any& rOldValue,
            bool _bColumnHeaderBar
         );

    /** returns the Rectangle for either the column header bar ot the row header bar
        @param  _bIsColumnBar
            <TRUE/> when column header bar is used
        @param  _bOnScreen
            <TRUE/> when the rectangle should be calculated OnScreen
        @return
            the Rectangle
    */
    virtual Rectangle calcHeaderRect(bool _bIsColumnBar, bool _bOnScreen = true);

    /** calculates the Rectangle of the table
        @param  _bOnScreen
            <TRUE/> when the rectangle should be calculated OnScreen
        @return
            the Rectangle
    */
    virtual Rectangle calcTableRect(bool _bOnScreen = true);

    /**
        @param  _nRowId
            the current row
        @param  _nColId
            the column id
        @param  _bOnScreen
            <TRUE/> when the rectangle should be calculated OnScreen
        @return
            the Rectangle
    */
    virtual Rectangle GetFieldRectPixelAbs(sal_Int32 _nRowId, sal_uInt16 _nColId, bool _bIsHeader, bool _bOnScreen = true);

    /// return <TRUE/> if and only if the accessible object for this instance has been created and is alive
    bool isAccessibleAlive( ) const;

    // ACCESSIBILITY ==========================================================
public:
    /** Creates and returns the accessible object of the whole BrowseBox. */
    virtual css::uno::Reference<
        css::accessibility::XAccessible > CreateAccessible();

    // Children ---------------------------------------------------------------

    /** Creates the accessible object of a data table cell.
        @param nRow  The row index of the cell.
        @param nColumnId  The column pos of the cell.
        @return  The XAccessible interface of the specified cell. */
    virtual css::uno::Reference<
        css::accessibility::XAccessible >
    CreateAccessibleCell( sal_Int32 nRow, sal_uInt16 nColumnPos );

    /** Creates the accessible object of a row header.
        @param nRow  The row index of the header.
        @return  The XAccessible interface of the specified row header. */
    virtual css::uno::Reference<
        css::accessibility::XAccessible >
    CreateAccessibleRowHeader( sal_Int32 nRow );

    /** Creates the accessible object of a column header.
        @param nColumnId  The column ID of the header.
        @return  The XAccessible interface of the specified column header. */
    virtual css::uno::Reference<
        css::accessibility::XAccessible >
    CreateAccessibleColumnHeader( sal_uInt16 nColumnPos );

    /** @return  The count of additional controls of the control area. */
    virtual sal_Int32 GetAccessibleControlCount() const;

    /** Creates the accessible object of an additional control.
        @param nIndex  The 0-based index of the control.
        @return  The XAccessible interface of the specified control. */
    virtual css::uno::Reference<
        css::accessibility::XAccessible >
    CreateAccessibleControl( sal_Int32 nIndex );

    // Conversions ------------------------------------------------------------

    /** Converts a point relative to the data window origin to a cell address.
        @param rnRow  Out-parameter that takes the row index.
        @param rnColumnId  Out-parameter that takes the column ID.
        @param rPoint  The position in pixels relative to the data window.
        @return <TRUE/>, if the point could be converted to a valid address. */
    virtual bool ConvertPointToCellAddress(
        sal_Int32& rnRow, sal_uInt16& rnColumnId, const Point& rPoint );

    /** Converts a point relative to the row header bar origin to a row header
        index.
        @param rnRow  Out-parameter that takes the row index.
        @param rPoint  The position in pixels relative to the header bar.
        @return <TRUE/>, if the point could be converted to a valid index. */
    virtual bool ConvertPointToRowHeader( sal_Int32& rnRow, const Point& rPoint );

    /** Converts a point relative to the column header bar origin to a column
        header index.
        @param rnColumnId  Out-parameter that takes the column ID.
        @param rPoint  The position in pixels relative to the header bar.
        @return <TRUE/>, if the point could be converted to a valid index. */
    virtual bool ConvertPointToColumnHeader( sal_uInt16& rnColumnPos, const Point& rPoint );

    /** Converts a point relative to the BrowseBox origin to the index of an
        existing control.
        @param rnRow  Out-parameter that takes the 0-based control index.
        @param rPoint  The position in pixels relative to the BrowseBox.
        @return <TRUE/>, if the point could be converted to a valid index. */
    virtual bool ConvertPointToControlIndex( sal_Int32& rnIndex, const Point& rPoint );

    // Object data and state --------------------------------------------------

    /** return the name of the specified object.
        @param  eObjType
            The type to ask for
        @param  _nPosition
            The position of a tablecell (index position), header bar  colum/row cell
        @return
            The name of the specified object.
    */
    virtual OUString GetAccessibleObjectName( ::svt::AccessibleBrowseBoxObjType eObjType,sal_Int32 _nPosition = -1) const;

    /** return the description of the specified object.
        @param  eObjType
            The type to ask for
        @param  _nPosition
            The position of a tablecell (index position), header bar  column/row cell
        @return
            The description of the specified object.
    */
    virtual OUString GetAccessibleObjectDescription( ::svt::AccessibleBrowseBoxObjType eObjType,sal_Int32 _nPosition = -1) const;

    /** @return  The header text of the specified row. */
    virtual OUString GetRowDescription( sal_Int32 nRow ) const;

    /** @return  The header text of the specified column. */
    virtual OUString GetColumnDescription( sal_uInt16 _nColumn ) const;

    /** Fills the StateSet with all states (except DEFUNC and SHOWING, done by
        the accessible object), depending on the specified object type. */
    virtual void FillAccessibleStateSet(
            ::utl::AccessibleStateSetHelper& rStateSet,
            ::svt::AccessibleBrowseBoxObjType eObjType ) const;

    /** Fills the StateSet with all states for one cell (except DEFUNC and SHOWING, done by
        the accessible object). */
    virtual void FillAccessibleStateSetForCell(
            ::utl::AccessibleStateSetHelper& _rStateSet,
            sal_Int32 _nRow, sal_uInt16 _nColumn ) const;

    /** Sets focus to current cell of the data table. */
    virtual void GrabTableFocus();

    // IAccessibleTableProvider
    virtual sal_Int32               GetCurrRow() const;
    virtual sal_uInt16              GetCurrColumn() const;
    virtual bool                    HasRowHeader() const;
    virtual bool                    IsCellFocusable() const;
    virtual bool                    GoToCell( sal_Int32 _nRow, sal_uInt16 _nColumn );
    virtual void                    SelectColumn( sal_uInt16 _nColumn, bool _bSelect = true );
    virtual bool                    IsColumnSelected( long _nColumn ) const;
    virtual sal_Int32               GetSelectedRowCount() const;
    virtual sal_Int32               GetSelectedColumnCount() const;
    virtual void                    GetAllSelectedRows( css::uno::Sequence< sal_Int32 >& _rRows ) const;
    virtual void                    GetAllSelectedColumns( css::uno::Sequence< sal_Int32 >& _rColumns ) const;
    virtual bool                    IsCellVisible( sal_Int32 _nRow, sal_uInt16 _nColumn ) const;
    virtual OUString                GetAccessibleCellText(long _nRow, sal_uInt16 _nColPos) const;
    virtual bool                    GetGlyphBoundRects( const Point& rOrigin, const OUString& rStr, int nIndex, int nLen, int nBase, MetricVector& rVector );
    virtual Rectangle               GetWindowExtentsRelative( Window *pRelativeWindow ) const;
    virtual void                    GrabFocus();
    virtual css::uno::Reference< css::accessibility::XAccessible > GetAccessible( bool bCreate = true );
    virtual Window*                 GetAccessibleParentWindow() const;
    virtual Window*                 GetWindowInstance();

private:
    // the following declares some Window/OutputDevice methods private. This happened in the course
    // of CWS warnings01, which pointed out nameclashs in those methods. If the build breaks in some
    // upper module, you should investigate whether you really wanted to call base class methods,
    // or the versions at this class. In the latter case, use the renamed versions above.

    // Set/GetLineColor - superseded by Set/GetGridLineColor
    using OutputDevice::SetLineColor;
    using OutputDevice::GetLineColor;

    // ToTop/ToBottom were never property implemented. If you currently call it, this is most probably wrong
    // and not doing as intended
    using Window::ToTop;
};


inline const DataFlavorExVector& BrowseBox::GetDataFlavors() const
{
    return *reinterpret_cast<DataFlavorExVector*>(implGetDataFlavors());
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
