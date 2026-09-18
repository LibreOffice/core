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

#include <controls/table/AccessibleGridControl.hxx>
#include <controls/table/TableControl.hxx>
#include <controls/table/TableFunctionSet.hxx>
#include <controls/table/defaultinputhandler.hxx>
#include <controls/table/tablemodel.hxx>

#include "tabledatawindow.hxx"
#include "tablegeometry.hxx"

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>

#include <comphelper/flagguard.hxx>
#include <tools/mapunit.hxx>
#include <vcl/toolkit/scrbar.hxx>
#include <vcl/seleng.hxx>
#include <vcl/settings.hxx>
#include <vcl/image.hxx>
#include <vcl/vclevent.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/debug.hxx>

#include <cstdlib>
#include <numeric>

#define MIN_COLUMN_WIDTH_PIXEL 4

namespace svt::table
{
using ::com::sun::star::accessibility::AccessibleTableModelChange;
using ::com::sun::star::uno::Any;
using ::com::sun::star::accessibility::XAccessible;
using ::com::sun::star::uno::Reference;

namespace AccessibleEventId = css::accessibility::AccessibleEventId;
namespace AccessibleTableModelChangeType = css::accessibility::AccessibleTableModelChangeType;

//= SuppressCursor

namespace
{
class SuppressCursor
{
private:
    TableControl& m_rTable;

public:
    explicit SuppressCursor(TableControl& _rTable)
        : m_rTable(_rTable)
    {
        m_rTable.hideCursor();
    }
    ~SuppressCursor() { m_rTable.showCursor(); }
};

//= EmptyTableModel

/** default implementation of an ->ITableModel, used as fallback when no
        real model is present

        Instances of this class are static in any way, and provide the least
        necessary default functionality for a table model.
    */
class EmptyTableModel : public ITableModel
{
public:
    EmptyTableModel() {}

    // ITableModel overridables
    virtual TableSize getColumnCount() const override { return 0; }
    virtual TableSize getRowCount() const override { return 0; }
    virtual bool hasColumnHeaders() const override { return false; }
    virtual bool hasRowHeaders() const override { return false; }
    virtual PColumnModel getColumnModel(sal_Int32) override
    {
        OSL_FAIL("EmptyTableModel::getColumnModel: invalid call!");
        return PColumnModel();
    }
    virtual PTableRenderer getRenderer() const override { return PTableRenderer(); }
    virtual PTableInputHandler getInputHandler() const override { return PTableInputHandler(); }
    virtual TableMetrics getRowHeight() const override { return 5 * 100; }
    virtual TableMetrics getColumnHeaderHeight() const override { return 0; }
    virtual TableMetrics getRowHeaderWidth() const override { return 0; }
    virtual ScrollbarVisibility getVerticalScrollbarVisibility() const override
    {
        return ScrollbarShowNever;
    }
    virtual ScrollbarVisibility getHorizontalScrollbarVisibility() const override
    {
        return ScrollbarShowNever;
    }
    virtual void addTableModelListener(const VclPtr<TableControl>&) override {}
    virtual void removeTableModelListener(const VclPtr<TableControl>&) override {}
    virtual ::std::optional<::Color> getLineColor() const override
    {
        return ::std::optional<::Color>();
    }
    virtual ::std::optional<::Color> getHeaderBackgroundColor() const override
    {
        return ::std::optional<::Color>();
    }
    virtual ::std::optional<::Color> getHeaderTextColor() const override
    {
        return ::std::optional<::Color>();
    }
    virtual ::std::optional<::Color> getActiveSelectionBackColor() const override
    {
        return ::std::optional<::Color>();
    }
    virtual ::std::optional<::Color> getInactiveSelectionBackColor() const override
    {
        return ::std::optional<::Color>();
    }
    virtual ::std::optional<::Color> getActiveSelectionTextColor() const override
    {
        return ::std::optional<::Color>();
    }
    virtual ::std::optional<::Color> getInactiveSelectionTextColor() const override
    {
        return ::std::optional<::Color>();
    }
    virtual ::std::optional<::Color> getTextColor() const override
    {
        return ::std::optional<::Color>();
    }
    virtual ::std::optional<::Color> getTextLineColor() const override
    {
        return ::std::optional<::Color>();
    }
    virtual ::std::optional<::std::vector<::Color>> getRowBackgroundColors() const override
    {
        return ::std::optional<::std::vector<::Color>>();
    }
    virtual css::style::VerticalAlignment getVerticalAlign() const override
    {
        return css::style::VerticalAlignment(0);
    }
    virtual ITableDataSort* getSortAdapter() override { return nullptr; }
    virtual bool isEnabled() const override { return true; }
    virtual void getCellContent(sal_Int32 const, sal_Int32 const,
                                css::uno::Any& o_cellContent) override
    {
        o_cellContent.clear();
    }
    virtual void getCellToolTip(sal_Int32 const, sal_Int32 const, css::uno::Any&) override {}
    virtual Any getRowHeading(sal_Int32 const) const override { return Any(); }
};
}

TableControl::TableControl(vcl::Window* pParent, WinBits nStyle)
    : Control(pParent, nStyle)
    , m_pModel(std::make_shared<EmptyTableModel>())
    , m_pInputHandler()
    , m_nRowHeightPixel(15)
    , m_nColHeaderHeightPixel(0)
    , m_nRowHeaderWidthPixel(0)
    , m_nColumnCount(0)
    , m_nRowCount(0)
    , m_nCurColumn(COL_INVALID)
    , m_nCurRow(ROW_INVALID)
    , m_nLeftColumn(0)
    , m_nTopRow(0)
    , m_nCursorHidden(1)
    , m_pDataWindow(VclPtr<TableDataWindow>::Create(*this))
    , m_pVScroll(nullptr)
    , m_pHScroll(nullptr)
    , m_pScrollCorner(nullptr)
    , m_aSelectedRows()
    , m_pTableFunctionSet(new TableFunctionSet(*this))
    , m_nAnchor(-1)
    , m_bUpdatingColWidths(false)
{
    m_pSelEngine.reset(new SelectionEngine(m_pDataWindow.get(), m_pTableFunctionSet.get()));
    m_pSelEngine->SetSelectionMode(SelectionMode::Single);
    m_pDataWindow->SetPosPixel(Point(0, 0));
    m_pDataWindow->Show();

    m_pDataWindow->SetSelectHdl(LINK(this, TableControl, ImplSelectHdl));

    // by default, use the background as determined by the style settings
    const Color aWindowColor(GetSettings().GetStyleSettings().GetFieldColor());
    SetBackground(Wallpaper(aWindowColor));
    GetOutDev()->SetFillColor(aWindowColor);

    SetCompoundControl(true);
}

TableControl::~TableControl() { disposeOnce(); }

void TableControl::dispose()
{
    CallEventListeners(VclEventId::ObjectDying);

    SetModel(PTableModel());
    disposeAccessible();

    m_pVScroll.disposeAndClear();
    m_pHScroll.disposeAndClear();
    m_pScrollCorner.disposeAndClear();
    m_pDataWindow.disposeAndClear();
    m_pTableFunctionSet.reset();
    m_pSelEngine.reset();

    Control::dispose();
}

void TableControl::GetFocus()
{
    showCursor();

    Control::GetFocus();
}

void TableControl::LoseFocus()
{
    hideCursor();

    Control::LoseFocus();
}

void TableControl::KeyInput(const KeyEvent& rKEvt)
{
    bool bHandled = false;

    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
    sal_uInt16 nKeyCode = rKeyCode.GetCode();

    struct ActionMapEntry
    {
        sal_uInt16 nKeyCode;
        sal_uInt16 nKeyModifier;
        TableControlAction eAction;
    } static const aKnownActions[]
        = { { KEY_DOWN, 0, TableControlAction::cursorDown },
            { KEY_UP, 0, TableControlAction::cursorUp },
            { KEY_LEFT, 0, TableControlAction::cursorLeft },
            { KEY_RIGHT, 0, TableControlAction::cursorRight },
            { KEY_HOME, 0, TableControlAction::cursorToLineStart },
            { KEY_END, 0, TableControlAction::cursorToLineEnd },
            { KEY_PAGEUP, 0, TableControlAction::cursorPageUp },
            { KEY_PAGEDOWN, 0, TableControlAction::cursorPageDown },
            { KEY_PAGEUP, KEY_MOD1, TableControlAction::cursorToFirstLine },
            { KEY_PAGEDOWN, KEY_MOD1, TableControlAction::cursorToLastLine },
            { KEY_HOME, KEY_MOD1, TableControlAction::cursorTopLeft },
            { KEY_END, KEY_MOD1, TableControlAction::cursorBottomRight },
            { KEY_SPACE, KEY_MOD1, TableControlAction::cursorSelectRow },
            { KEY_UP, KEY_SHIFT, TableControlAction::cursorSelectRowUp },
            { KEY_DOWN, KEY_SHIFT, TableControlAction::cursorSelectRowDown },
            { KEY_END, KEY_SHIFT, TableControlAction::cursorSelectRowAreaBottom },
            { KEY_HOME, KEY_SHIFT, TableControlAction::cursorSelectRowAreaTop } };
    for (const ActionMapEntry& rAction : aKnownActions)
    {
        if ((rAction.nKeyCode == nKeyCode) && (rAction.nKeyModifier == rKeyCode.GetModifier()))
        {
            bHandled = dispatchAction(rAction.eAction);
            break;
        }
    }

    if (!bHandled)
        Control::KeyInput(rKEvt);
    else
    {
        commitCellEvent(AccessibleEventId::STATE_CHANGED,
                        Any(css::accessibility::AccessibleStateType::FOCUSED), Any());
        // Huh? What the heck? Why do we unconditionally notify a STATE_CHANGE/FOCUSED after each and every
        // (handled) key stroke?

        commitTableEvent(AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, Any(), Any());
        // ditto: Why do we notify this unconditionally? We should find the right place to notify the
        // ACTIVE_DESCENDANT_CHANGED event.
        // Also, we should check if STATE_CHANGED/FOCUSED is really necessary: finally, the children are
        // transient, aren't they?
    }
}

void TableControl::StateChanged(StateChangedType i_nStateChange)
{
    Control::StateChanged(i_nStateChange);

    // forward certain settings to the data window
    switch (i_nStateChange)
    {
        case StateChangedType::ControlFocus:
            invalidateSelectedRows();
            break;

        case StateChangedType::ControlBackground:
            if (IsControlBackground())
                getDataWindow().SetControlBackground(GetControlBackground());
            else
                getDataWindow().SetControlBackground();
            break;

        case StateChangedType::ControlForeground:
            if (IsControlForeground())
                getDataWindow().SetControlForeground(GetControlForeground());
            else
                getDataWindow().SetControlForeground();
            break;

        case StateChangedType::ControlFont:
            if (IsControlFont())
                getDataWindow().SetControlFont(GetControlFont());
            else
                getDataWindow().SetControlFont();
            break;
        default:;
    }
}

void TableControl::Resize()
{
    Control::Resize();
    onResize();
}

void TableControl::SetModel(const PTableModel& _pModel)
{
    SuppressCursor aHideCursor(*this);

    if (m_pModel)
        m_pModel->removeTableModelListener(this);

    m_pModel = _pModel;
    if (!m_pModel)
        m_pModel = std::make_shared<EmptyTableModel>();

    m_pModel->addTableModelListener(this);

    m_nCurRow = ROW_INVALID;
    m_nCurColumn = COL_INVALID;

    // recalc some model-dependent cached info
    impl_ni_updateCachedModelValues();
    impl_ni_relayout();

    // completely invalidate
    Invalidate();

    // reset cursor to (0,0)
    if (m_nRowCount)
        m_nCurRow = 0;
    if (m_nColumnCount)
        m_nCurColumn = 0;
}

namespace
{
bool lcl_adjustSelectedRows(::std::vector<sal_Int32>& io_selectionIndexes,
                            sal_Int32 const i_firstAffectedRowIndex, TableSize const i_offset)
{
    bool didChanges = false;
    for (auto& selectionIndex : io_selectionIndexes)
    {
        if (selectionIndex < i_firstAffectedRowIndex)
            continue;
        selectionIndex += i_offset;
        didChanges = true;
    }
    return didChanges;
}
}

void TableControl::rowsInserted(sal_Int32 i_first, sal_Int32 i_last)
{
    OSL_PRECOND(i_last >= i_first, "TableControl::rowsInserted: invalid row indexes!");

    TableSize const insertedRows = i_last - i_first + 1;

    // adjust selection, if necessary
    bool const selectionChanged = lcl_adjustSelectedRows(m_aSelectedRows, i_first, insertedRows);

    // adjust our cached row count
    m_nRowCount = m_pModel->getRowCount();

    // if the rows have been inserted before the current row, adjust this
    if (i_first <= m_nCurRow)
        GoToCell(m_nCurColumn, m_nCurRow + insertedRows);

    // relayout, since the scrollbar need might have changed
    impl_ni_relayout();

    // notify A1YY events
    impl_commitAccessibleEvent(
        AccessibleEventId::TABLE_MODEL_CHANGED,
        Any(AccessibleTableModelChange(AccessibleTableModelChangeType::ROWS_INSERTED, i_first,
                                       i_last, -1, -1)));

    // schedule repaint
    invalidateRowRange(i_first, ROW_INVALID);

    // call selection handlers, if necessary
    if (selectionChanged)
        Select();
}

void TableControl::rowsRemoved(sal_Int32 i_first, sal_Int32 i_last)
{
    sal_Int32 firstRemovedRow = i_first;
    sal_Int32 lastRemovedRow = i_last;

    // adjust selection, if necessary
    bool selectionChanged = false;
    if (i_first == -1)
    {
        selectionChanged = markAllRowsAsDeselected();

        firstRemovedRow = 0;
        lastRemovedRow = m_nRowCount - 1;
    }
    else
    {
        ENSURE_OR_RETURN_VOID(i_last >= i_first, "TableControl::rowsRemoved: illegal indexes!");

        for (sal_Int32 row = i_first; row <= i_last; ++row)
        {
            if (markRowAsDeselected(row))
                selectionChanged = true;
        }

        if (lcl_adjustSelectedRows(m_aSelectedRows, i_last + 1, i_first - i_last - 1))
            selectionChanged = true;
    }

    // adjust cached row count
    m_nRowCount = m_pModel->getRowCount();

    // adjust the current row, if it is larger than the row count now
    if (m_nCurRow >= m_nRowCount)
    {
        if (m_nRowCount > 0)
            GoToCell(m_nCurColumn, m_nRowCount - 1);
        else
        {
            m_nCurRow = ROW_INVALID;
            m_nTopRow = 0;
        }
    }
    else if (m_nRowCount == 0)
    {
        m_nTopRow = 0;
    }

    // relayout, since the scrollbar need might have changed
    impl_ni_relayout();

    // notify A11Y events
    commitTableEvent(AccessibleEventId::TABLE_MODEL_CHANGED,
                     Any(AccessibleTableModelChange(AccessibleTableModelChangeType::ROWS_REMOVED,
                                                    firstRemovedRow, lastRemovedRow, -1, -1)),
                     Any());

    // schedule a repaint
    invalidateRowRange(firstRemovedRow, ROW_INVALID);

    // call selection handlers, if necessary
    if (selectionChanged)
        Select();
}

void TableControl::columnInserted()
{
    m_nColumnCount = m_pModel->getColumnCount();
    impl_ni_relayout();

    Invalidate();
}

void TableControl::columnRemoved()
{
    m_nColumnCount = m_pModel->getColumnCount();

    // adjust the current column, if it is larger than the column count now
    if (m_nCurColumn >= m_nColumnCount)
    {
        if (m_nColumnCount > 0)
            GoToCell(m_nCurColumn - 1, m_nCurRow);
        else
            m_nCurColumn = COL_INVALID;
    }

    impl_ni_relayout();

    Invalidate();
}

void TableControl::allColumnsRemoved()
{
    m_nColumnCount = m_pModel->getColumnCount();
    impl_ni_relayout();

    Invalidate();
}

void TableControl::cellsUpdated(sal_Int32 const i_firstRow, sal_Int32 const i_lastRow)
{
    invalidateRowRange(i_firstRow, i_lastRow);
}

void TableControl::tableMetricsChanged()
{
    impl_ni_updateCachedTableMetrics();
    impl_ni_relayout();
    Invalidate();
}

void TableControl::impl_invalidateColumn(sal_Int32 const i_column)
{
    tools::Rectangle const aAllCellsArea(impl_getAllVisibleCellsArea());

    const TableColumnGeometry aColumn(*this, aAllCellsArea, i_column);
    if (aColumn.isValid())
        Invalidate(aColumn.getRect());
}

void TableControl::columnChanged(sal_Int32 const i_column,
                                 ColumnAttributeGroup const i_attributeGroup)
{
    ColumnAttributeGroup nGroup(i_attributeGroup);
    if (nGroup & ColumnAttributeGroup::APPEARANCE)
    {
        impl_invalidateColumn(i_column);
        nGroup &= ~ColumnAttributeGroup::APPEARANCE;
    }

    if (nGroup & ColumnAttributeGroup::WIDTH)
    {
        if (!m_bUpdatingColWidths)
        {
            impl_ni_relayout(i_column);
            invalidate(TableArea::All);
        }

        nGroup &= ~ColumnAttributeGroup::WIDTH;
    }

    OSL_ENSURE((nGroup == ColumnAttributeGroup::NONE)
                   || (i_attributeGroup == ColumnAttributeGroup::ALL),
               "TableControl::columnChanged: don't know how to handle this change!");
}

tools::Rectangle TableControl::impl_getAllVisibleCellsArea() const
{
    tools::Rectangle aArea(Point(0, 0), Size(0, 0));

    // determine the right-most border of the last column which is
    // at least partially visible
    aArea.SetRight(m_nRowHeaderWidthPixel);
    if (!m_aColumnWidths.empty())
    {
        // the number of pixels which are scrolled out of the left hand
        // side of the window
        const tools::Long nScrolledOutLeft
            = m_nLeftColumn == 0 ? 0 : m_aColumnWidths[m_nLeftColumn - 1].getEnd();

        ColumnPositions::const_reverse_iterator loop = m_aColumnWidths.rbegin();
        do
        {
            aArea.SetRight(loop->getEnd() - nScrolledOutLeft);
            ++loop;
        } while ((loop != m_aColumnWidths.rend())
                 && (loop->getEnd() - nScrolledOutLeft >= aArea.Right()));
    }
    // so far, aArea.Right() denotes the first pixel *after* the cell area
    aArea.AdjustRight(-1);

    // determine the last row which is at least partially visible
    aArea.SetBottom(m_nColHeaderHeightPixel + impl_getVisibleRows(true) * m_nRowHeightPixel - 1);

    return aArea;
}

tools::Rectangle TableControl::impl_getAllVisibleDataCellArea() const
{
    tools::Rectangle aArea(impl_getAllVisibleCellsArea());
    aArea.SetLeft(m_nRowHeaderWidthPixel);
    aArea.SetTop(m_nColHeaderHeightPixel);
    return aArea;
}

void TableControl::impl_ni_updateCachedTableMetrics()
{
    m_nRowHeightPixel
        = LogicToPixel(Size(0, m_pModel->getRowHeight()), MapMode(MapUnit::MapAppFont)).Height();

    m_nColHeaderHeightPixel = 0;
    if (m_pModel->hasColumnHeaders())
        m_nColHeaderHeightPixel
            = LogicToPixel(Size(0, m_pModel->getColumnHeaderHeight()), MapMode(MapUnit::MapAppFont))
                  .Height();

    m_nRowHeaderWidthPixel = 0;
    if (m_pModel->hasRowHeaders())
        m_nRowHeaderWidthPixel
            = LogicToPixel(Size(m_pModel->getRowHeaderWidth(), 0), MapMode(MapUnit::MapAppFont))
                  .Width();
}

void TableControl::impl_ni_updateCachedModelValues()
{
    m_pInputHandler = m_pModel->getInputHandler();
    if (!m_pInputHandler)
        m_pInputHandler = std::make_shared<DefaultInputHandler>();

    m_nColumnCount = m_pModel->getColumnCount();
    if (m_nLeftColumn >= m_nColumnCount)
        m_nLeftColumn = (m_nColumnCount > 0) ? m_nColumnCount - 1 : 0;

    m_nRowCount = m_pModel->getRowCount();
    if (m_nTopRow >= m_nRowCount)
        m_nTopRow = (m_nRowCount > 0) ? m_nRowCount - 1 : 0;

    impl_ni_updateCachedTableMetrics();
}

namespace
{
/// determines whether a scrollbar is needed for the given values
bool lcl_determineScrollbarNeed(tools::Long const i_position,
                                ScrollbarVisibility const i_visibility,
                                tools::Long const i_availableSpace, tools::Long const i_neededSpace)
{
    if (i_visibility == ScrollbarShowNever)
        return false;
    if (i_visibility == ScrollbarShowAlways)
        return true;
    if (i_position > 0)
        return true;
    if (i_availableSpace >= i_neededSpace)
        return false;
    return true;
}

void lcl_setButtonRepeat(vcl::Window& _rWindow)
{
    AllSettings aSettings = _rWindow.GetSettings();
    MouseSettings aMouseSettings = aSettings.GetMouseSettings();

    aMouseSettings.SetButtonRepeat(0);
    aSettings.SetMouseSettings(aMouseSettings);

    _rWindow.SetSettings(aSettings, true);
}

bool lcl_updateScrollbar(vcl::Window& _rParent, VclPtr<ScrollBar>& _rpBar, bool const i_needBar,
                         tools::Long _nVisibleUnits, tools::Long _nPosition, tools::Long _nRange,
                         bool _bHorizontal, const Link<ScrollBar*, void>& _rScrollHandler)
{
    // do we currently have the scrollbar?
    bool bHaveBar = _rpBar != nullptr;

    // do we need to correct the scrollbar visibility?
    if (bHaveBar && !i_needBar)
    {
        if (_rpBar->IsTracking())
            _rpBar->EndTracking();
        _rpBar.disposeAndClear();
    }
    else if (!bHaveBar && i_needBar)
    {
        _rpBar = VclPtr<ScrollBar>::Create(

            &_rParent, WB_DRAG | (_bHorizontal ? WB_HSCROLL : WB_VSCROLL));
        _rpBar->SetScrollHdl(_rScrollHandler);
        // get some speed into the scrolling...
        lcl_setButtonRepeat(*_rpBar);
    }

    if (_rpBar)
    {
        _rpBar->SetRange(Range(0, _nRange));
        _rpBar->SetVisibleSize(_nVisibleUnits);
        _rpBar->SetPageSize(_nVisibleUnits);
        _rpBar->SetLineSize(1);
        _rpBar->SetThumbPos(_nPosition);
        _rpBar->Show();
    }

    return (bHaveBar != i_needBar);
}

/** returns the number of rows fitting into the given range,
            for the given row height. Partially fitting rows are counted, too, if the
            respective parameter says so.
        */
TableSize lcl_getRowsFittingInto(tools::Long _nOverallHeight, tools::Long _nRowHeightPixel,
                                 bool _bAcceptPartialRow)
{
    return _bAcceptPartialRow ? (_nOverallHeight + (_nRowHeightPixel - 1)) / _nRowHeightPixel
                              : _nOverallHeight / _nRowHeightPixel;
}

/** returns the number of columns fitting into the given area,
            with the first visible column as given. Partially fitting columns are counted, too,
            if the respective parameter says so.
        */
TableSize lcl_getColumnsVisibleWithin(const tools::Rectangle& _rArea,
                                      sal_Int32 _nFirstVisibleColumn, const TableControl& _rControl,
                                      bool _bAcceptPartialRow)
{
    TableSize visibleColumns = 0;
    TableColumnGeometry aColumn(_rControl, _rArea, _nFirstVisibleColumn);
    while (aColumn.isValid())
    {
        if (!_bAcceptPartialRow)
            if (aColumn.getRect().Right() > _rArea.Right())
                // this column is only partially visible, and this is not allowed
                break;

        aColumn.moveRight();
        ++visibleColumns;
    }
    return visibleColumns;
}
}

tools::Long
TableControl::impl_ni_calculateColumnWidths(sal_Int32 const i_assumeInflexibleColumnsUpToIncluding,
                                            bool const i_assumeVerticalScrollbar,
                                            ::std::vector<tools::Long>& o_newColWidthsPixel) const
{
    // the available horizontal space
    tools::Long gridWidthPixel = GetOutputSizePixel().Width();
    ENSURE_OR_RETURN(!!m_pModel,
                     "TableControl::impl_ni_calculateColumnWidths: not allowed without a model!",
                     gridWidthPixel);
    if (m_pModel->hasRowHeaders() && (gridWidthPixel != 0))
    {
        gridWidthPixel -= m_nRowHeaderWidthPixel;
    }

    if (i_assumeVerticalScrollbar
        && (m_pModel->getVerticalScrollbarVisibility() != ScrollbarShowNever))
    {
        tools::Long nScrollbarMetrics = GetSettings().GetStyleSettings().GetScrollBarSize();
        gridWidthPixel -= nScrollbarMetrics;
    }

    // no need to do anything without columns
    TableSize const colCount = m_pModel->getColumnCount();
    if (colCount == 0)
        return gridWidthPixel;

    // collect some meta data for our columns:
    // - their current (pixel) metrics
    tools::Long accumulatedCurrentWidth = 0;
    ::std::vector<tools::Long> currentColWidths;
    currentColWidths.reserve(colCount);
    typedef ::std::vector<::std::pair<tools::Long, long>> ColumnLimits;
    ColumnLimits effectiveColumnLimits;
    effectiveColumnLimits.reserve(colCount);
    tools::Long accumulatedMinWidth = 0;
    tools::Long accumulatedMaxWidth = 0;
    // - their relative flexibility
    ::std::vector<::sal_Int32> columnFlexibilities;
    columnFlexibilities.reserve(colCount);
    tools::Long flexibilityDenominator = 0;
    size_t flexibleColumnCount = 0;
    for (sal_Int32 col = 0; col < colCount; ++col)
    {
        PColumnModel const pColumn = m_pModel->getColumnModel(col);
        ENSURE_OR_THROW(!!pColumn, "invalid column returned by the model!");

        // current width
        tools::Long const currentWidth = appFontWidthToPixel(pColumn->getWidth());
        currentColWidths.push_back(currentWidth);

        // accumulated width
        accumulatedCurrentWidth += currentWidth;

        // flexibility
        ::sal_Int32 flexibility = pColumn->getFlexibility();
        OSL_ENSURE(flexibility >= 0, "TableControl::impl_ni_calculateColumnWidths: a column's "
                                     "flexibility should be non-negative.");
        if ((flexibility < 0) // normalization
            || (!pColumn->isResizable()) // column not resizable => no auto-resize
            || (col
                <= i_assumeInflexibleColumnsUpToIncluding) // column shall be treated as inflexible => respect this
        )
            flexibility = 0;

        // min/max width
        tools::Long effectiveMin = currentWidth, effectiveMax = currentWidth;
        // if the column is not flexible, it will not be asked for min/max, but we assume the current width as limit then
        if (flexibility > 0)
        {
            tools::Long const minWidth = appFontWidthToPixel(pColumn->getMinWidth());
            if (minWidth > 0)
                effectiveMin = minWidth;
            else
                effectiveMin = MIN_COLUMN_WIDTH_PIXEL;

            tools::Long const maxWidth = appFontWidthToPixel(pColumn->getMaxWidth());
            OSL_ENSURE(minWidth <= maxWidth, "TableControl::impl_ni_calculateColumnWidths: "
                                             "pretty undecided 'bout its width limits, this "
                                             "column!");
            if ((maxWidth > 0) && (maxWidth >= minWidth))
                effectiveMax = maxWidth;
            else
                effectiveMax = gridWidthPixel; // TODO: any better guess here?

            if (effectiveMin == effectiveMax)
                // if the min and the max are identical, this implies no flexibility at all
                flexibility = 0;
        }

        columnFlexibilities.push_back(flexibility);
        flexibilityDenominator += flexibility;
        if (flexibility > 0)
            ++flexibleColumnCount;

        effectiveColumnLimits.emplace_back(effectiveMin, effectiveMax);
        accumulatedMinWidth += effectiveMin;
        accumulatedMaxWidth += effectiveMax;
    }

    o_newColWidthsPixel = currentColWidths;
    if (flexibilityDenominator == 0)
    {
        // no column is flexible => don't adjust anything
    }
    else if (gridWidthPixel > accumulatedCurrentWidth)
    { // we have space to give away ...
        tools::Long distributePixel = gridWidthPixel - accumulatedCurrentWidth;
        if (gridWidthPixel > accumulatedMaxWidth)
        {
            // ... but the column's maximal widths are still less than we have
            // => set them all to max
            for (svt::table::TableSize i = 0; i < colCount; ++i)
            {
                o_newColWidthsPixel[i] = effectiveColumnLimits[i].second;
            }
        }
        else
        {
            bool startOver = false;
            do
            {
                startOver = false;
                // distribute the remaining space amongst all columns with a positive flexibility
                for (size_t i = 0; i < o_newColWidthsPixel.size() && !startOver; ++i)
                {
                    tools::Long const columnFlexibility = columnFlexibilities[i];
                    if (columnFlexibility == 0)
                        continue;

                    tools::Long newColWidth
                        = currentColWidths[i]
                          + columnFlexibility * distributePixel / flexibilityDenominator;

                    if (newColWidth > effectiveColumnLimits[i].second)
                    { // that was too much, we hit the col's maximum
                        // set the new width to exactly this maximum
                        newColWidth = effectiveColumnLimits[i].second;
                        // adjust the flexibility denominator ...
                        flexibilityDenominator -= columnFlexibility;
                        columnFlexibilities[i] = 0;
                        --flexibleColumnCount;
                        // ... and the remaining width ...
                        tools::Long const difference = newColWidth - currentColWidths[i];
                        distributePixel -= difference;
                        // ... this way, we ensure that the width not taken up by this column is consumed by the other
                        // flexible ones (if there are some)

                        // and start over with the first column, since there might be earlier columns which need
                        // to be recalculated now
                        startOver = true;
                    }

                    o_newColWidthsPixel[i] = newColWidth;
                }
            } while (startOver);

            // are there pixels left (might be caused by rounding errors)?
            distributePixel
                = gridWidthPixel
                  - ::std::accumulate(o_newColWidthsPixel.begin(), o_newColWidthsPixel.end(), 0);
            while ((distributePixel > 0) && (flexibleColumnCount > 0))
            {
                // yes => ignore relative flexibilities, and subsequently distribute single pixels to all flexible
                // columns which did not yet reach their maximum.
                for (size_t i = 0; (i < o_newColWidthsPixel.size()) && (distributePixel > 0); ++i)
                {
                    if (columnFlexibilities[i] == 0)
                        continue;

                    OSL_ENSURE(o_newColWidthsPixel[i] <= effectiveColumnLimits[i].second,
                               "TableControl::impl_ni_calculateColumnWidths: inconsistency!");
                    if (o_newColWidthsPixel[i] >= effectiveColumnLimits[i].first)
                    {
                        columnFlexibilities[i] = 0;
                        --flexibleColumnCount;
                        continue;
                    }

                    ++o_newColWidthsPixel[i];
                    --distributePixel;
                }
            }
        }
    }
    else if (gridWidthPixel < accumulatedCurrentWidth)
    { // we need to take away some space from the columns which allow it ...
        tools::Long takeAwayPixel = accumulatedCurrentWidth - gridWidthPixel;
        if (gridWidthPixel < accumulatedMinWidth)
        {
            // ... but the column's minimal widths are still more than we have
            // => set them all to min
            for (svt::table::TableSize i = 0; i < colCount; ++i)
            {
                o_newColWidthsPixel[i] = effectiveColumnLimits[i].first;
            }
        }
        else
        {
            bool startOver = false;
            do
            {
                startOver = false;
                // take away the space we need from the columns with a positive flexibility
                for (size_t i = 0; i < o_newColWidthsPixel.size() && !startOver; ++i)
                {
                    tools::Long const columnFlexibility = columnFlexibilities[i];
                    if (columnFlexibility == 0)
                        continue;

                    tools::Long newColWidth
                        = currentColWidths[i]
                          - columnFlexibility * takeAwayPixel / flexibilityDenominator;

                    if (newColWidth < effectiveColumnLimits[i].first)
                    { // that was too much, we hit the col's minimum
                        // set the new width to exactly this minimum
                        newColWidth = effectiveColumnLimits[i].first;
                        // adjust the flexibility denominator ...
                        flexibilityDenominator -= columnFlexibility;
                        columnFlexibilities[i] = 0;
                        --flexibleColumnCount;
                        // ... and the remaining width ...
                        tools::Long const difference = currentColWidths[i] - newColWidth;
                        takeAwayPixel -= difference;

                        // and start over with the first column, since there might be earlier columns which need
                        // to be recalculated now
                        startOver = true;
                    }

                    o_newColWidthsPixel[i] = newColWidth;
                }
            } while (startOver);

            // are there pixels left (might be caused by rounding errors)?
            takeAwayPixel
                = ::std::accumulate(o_newColWidthsPixel.begin(), o_newColWidthsPixel.end(), 0)
                  - gridWidthPixel;
            while ((takeAwayPixel > 0) && (flexibleColumnCount > 0))
            {
                // yes => ignore relative flexibilities, and subsequently take away pixels from all flexible
                // columns which did not yet reach their minimum.
                for (size_t i = 0; (i < o_newColWidthsPixel.size()) && (takeAwayPixel > 0); ++i)
                {
                    if (columnFlexibilities[i] == 0)
                        continue;

                    OSL_ENSURE(o_newColWidthsPixel[i] >= effectiveColumnLimits[i].first,
                               "TableControl::impl_ni_calculateColumnWidths: inconsistency!");
                    if (o_newColWidthsPixel[i] <= effectiveColumnLimits[i].first)
                    {
                        columnFlexibilities[i] = 0;
                        --flexibleColumnCount;
                        continue;
                    }

                    --o_newColWidthsPixel[i];
                    --takeAwayPixel;
                }
            }
        }
    }

    return gridWidthPixel;
}

void TableControl::impl_ni_relayout(sal_Int32 const i_assumeInflexibleColumnsUpToIncluding)
{
    ENSURE_OR_RETURN_VOID(!m_bUpdatingColWidths,
                          "TableControl::impl_ni_relayout: recursive call detected!");

    m_aColumnWidths.resize(0);
    if (!m_pModel)
        return;

    ::comphelper::FlagRestorationGuard const aWidthUpdateFlag(m_bUpdatingColWidths, true);
    SuppressCursor aHideCursor(*this);

    // layouting steps:

    // 1. adjust column widths, leaving space for a vertical scrollbar
    // 2. determine need for a vertical scrollbar
    //    - V-YES: all fine, result from 1. is still valid
    //    - V-NO: result from 1. is still under consideration

    // 3. determine need for a horizontal scrollbar
    //   - H-NO: all fine, result from 2. is still valid
    //   - H-YES: reconsider need for a vertical scrollbar, if result of 2. was V-NO
    //     - V-YES: all fine, result from 1. is still valid
    //     - V-NO: redistribute the remaining space (if any) amongst all columns which allow it

    ::std::vector<tools::Long> newWidthsPixel;
    tools::Long gridWidthPixel = impl_ni_calculateColumnWidths(
        i_assumeInflexibleColumnsUpToIncluding, true, newWidthsPixel);

    // the width/height of a scrollbar, needed several times below
    tools::Long const nScrollbarMetrics = GetSettings().GetStyleSettings().GetScrollBarSize();

    // determine the playground for the data cells (excluding headers)
    // TODO: what if the control is smaller than needed for the headers/scrollbars?
    tools::Rectangle aDataCellPlayground(Point(0, 0), GetOutputSizePixel());
    aDataCellPlayground.SetLeft(m_nRowHeaderWidthPixel);
    aDataCellPlayground.SetTop(m_nColHeaderHeightPixel);

    OSL_ENSURE((m_nRowCount == m_pModel->getRowCount())
                   && (m_nColumnCount == m_pModel->getColumnCount()),
               "TableControl::impl_ni_relayout: how is this expected to work with invalid data?");
    tools::Long const nAllColumnsWidth
        = ::std::accumulate(newWidthsPixel.begin(), newWidthsPixel.end(), 0);

    ScrollbarVisibility const eVertScrollbar = m_pModel->getVerticalScrollbarVisibility();
    ScrollbarVisibility const eHorzScrollbar = m_pModel->getHorizontalScrollbarVisibility();

    // do we need a vertical scrollbar?
    bool bNeedVerticalScrollbar
        = lcl_determineScrollbarNeed(m_nTopRow, eVertScrollbar, aDataCellPlayground.GetHeight(),
                                     m_nRowHeightPixel * m_nRowCount);
    bool bFirstRoundVScrollNeed = false;
    if (bNeedVerticalScrollbar)
    {
        aDataCellPlayground.AdjustRight(-nScrollbarMetrics);
        bFirstRoundVScrollNeed = true;
    }

    // do we need a horizontal scrollbar?
    bool const bNeedHorizontalScrollbar = lcl_determineScrollbarNeed(
        m_nLeftColumn, eHorzScrollbar, aDataCellPlayground.GetWidth(), nAllColumnsWidth);
    if (bNeedHorizontalScrollbar)
    {
        aDataCellPlayground.AdjustBottom(-nScrollbarMetrics);

        // now that we just found that we need a horizontal scrollbar,
        // the need for a vertical one may have changed, since the horizontal
        // SB might just occupy enough space so that not all rows do fit
        // anymore
        if (!bFirstRoundVScrollNeed)
        {
            bNeedVerticalScrollbar = lcl_determineScrollbarNeed(m_nTopRow, eVertScrollbar,
                                                                aDataCellPlayground.GetHeight(),
                                                                m_nRowHeightPixel * m_nRowCount);
            if (bNeedVerticalScrollbar)
            {
                aDataCellPlayground.AdjustRight(-nScrollbarMetrics);
            }
        }
    }

    // the initial call to impl_ni_calculateColumnWidths assumed that we need a vertical scrollbar. If, by now,
    // we know that this is not the case, re-calculate the column widths.
    if (!bNeedVerticalScrollbar)
        gridWidthPixel = impl_ni_calculateColumnWidths(i_assumeInflexibleColumnsUpToIncluding,
                                                       false, newWidthsPixel);

    // update the column objects with the new widths we finally calculated
    TableSize const colCount = m_pModel->getColumnCount();
    m_aColumnWidths.reserve(colCount);
    tools::Long accumulatedWidthPixel = m_nRowHeaderWidthPixel;
    bool anyColumnWidthChanged = false;
    for (sal_Int32 col = 0; col < colCount; ++col)
    {
        const tools::Long columnStart = accumulatedWidthPixel;
        const tools::Long columnEnd = columnStart + newWidthsPixel[col];
        m_aColumnWidths.emplace_back(columnStart, columnEnd);
        accumulatedWidthPixel = columnEnd;

        // and don't forget to forward this to the column models
        PColumnModel const pColumn = m_pModel->getColumnModel(col);
        ENSURE_OR_THROW(!!pColumn, "invalid column returned by the model!");

        tools::Long const oldColumnWidthAppFont = pColumn->getWidth();
        tools::Long const newColumnWidthAppFont = pixelWidthToAppFont(newWidthsPixel[col]);
        pColumn->setWidth(newColumnWidthAppFont);

        anyColumnWidthChanged |= (oldColumnWidthAppFont != newColumnWidthAppFont);
    }

    // if the column widths changed, ensure everything is repainted
    if (anyColumnWidthChanged)
        invalidate(TableArea::All);

    // if the column resizing happened to leave some space at the right, but there are columns
    // scrolled out to the left, scroll them in
    while ((m_nLeftColumn > 0)
           && (accumulatedWidthPixel - m_aColumnWidths[m_nLeftColumn - 1].getStart()
               <= gridWidthPixel))
    {
        --m_nLeftColumn;
    }

    // now adjust the column metrics, since they currently ignore the horizontal scroll position
    if (m_nLeftColumn > 0)
    {
        const tools::Long offsetPixel
            = m_aColumnWidths[0].getStart() - m_aColumnWidths[m_nLeftColumn].getStart();
        for (auto& columnWidth : m_aColumnWidths)
        {
            columnWidth.move(offsetPixel);
        }
    }

    // show or hide the scrollbars as needed, and position the data window
    impl_ni_positionChildWindows(aDataCellPlayground, bNeedVerticalScrollbar,
                                 bNeedHorizontalScrollbar);
}

void TableControl::impl_ni_positionChildWindows(tools::Rectangle const& i_dataCellPlayground,
                                                bool const i_verticalScrollbar,
                                                bool const i_horizontalScrollbar)
{
    tools::Long const nScrollbarMetrics = GetSettings().GetStyleSettings().GetScrollBarSize();

    // create or destroy the vertical scrollbar, as needed
    lcl_updateScrollbar(
        *this, m_pVScroll, i_verticalScrollbar,
        lcl_getRowsFittingInto(i_dataCellPlayground.GetHeight(), m_nRowHeightPixel, false),
        // visible units
        m_nTopRow, // current position
        m_nRowCount, // range
        false, // vertical
        LINK(this, TableControl, OnScroll) // scroll handler
    );

    // position it
    if (m_pVScroll)
    {
        tools::Rectangle aScrollbarArea(Point(i_dataCellPlayground.Right() + 1, 0),
                                        Size(nScrollbarMetrics, i_dataCellPlayground.Bottom() + 1));
        m_pVScroll->SetPosSizePixel(aScrollbarArea.TopLeft(), aScrollbarArea.GetSize());
    }

    // create or destroy the horizontal scrollbar, as needed
    lcl_updateScrollbar(
        *this, m_pHScroll, i_horizontalScrollbar,
        lcl_getColumnsVisibleWithin(i_dataCellPlayground, m_nLeftColumn, *this, false),
        // visible units
        m_nLeftColumn, // current position
        m_nColumnCount, // range
        true, // horizontal
        LINK(this, TableControl, OnScroll) // scroll handler
    );

    // position it
    if (m_pHScroll)
    {
        TableSize const nVisibleUnits
            = lcl_getColumnsVisibleWithin(i_dataCellPlayground, m_nLeftColumn, *this, false);
        TableMetrics const nRange = m_nColumnCount;
        if (m_nLeftColumn + nVisibleUnits == nRange - 1)
        {
            if (m_aColumnWidths[nRange - 1].getStart() - m_aColumnWidths[m_nLeftColumn].getEnd()
                    + m_aColumnWidths[nRange - 1].getWidth()
                > i_dataCellPlayground.GetWidth())
            {
                m_pHScroll->SetVisibleSize(nVisibleUnits - 1);
                m_pHScroll->SetPageSize(nVisibleUnits - 1);
            }
        }
        tools::Rectangle aScrollbarArea(Point(0, i_dataCellPlayground.Bottom() + 1),
                                        Size(i_dataCellPlayground.Right() + 1, nScrollbarMetrics));
        m_pHScroll->SetPosSizePixel(aScrollbarArea.TopLeft(), aScrollbarArea.GetSize());
    }

    // the corner window connecting the two scrollbars in the lower right corner
    bool bHaveScrollCorner = nullptr != m_pScrollCorner;
    bool bNeedScrollCorner = (nullptr != m_pHScroll) && (nullptr != m_pVScroll);
    if (bHaveScrollCorner && !bNeedScrollCorner)
    {
        m_pScrollCorner.disposeAndClear();
    }
    else if (!bHaveScrollCorner && bNeedScrollCorner)
    {
        m_pScrollCorner = VclPtr<ScrollBarBox>::Create(this);
        m_pScrollCorner->SetSizePixel(Size(nScrollbarMetrics, nScrollbarMetrics));
        m_pScrollCorner->SetPosPixel(
            Point(i_dataCellPlayground.Right() + 1, i_dataCellPlayground.Bottom() + 1));
        m_pScrollCorner->Show();
    }
    else if (bHaveScrollCorner && bNeedScrollCorner)
    {
        m_pScrollCorner->SetPosPixel(
            Point(i_dataCellPlayground.Right() + 1, i_dataCellPlayground.Bottom() + 1));
        m_pScrollCorner->Show();
    }

    // resize the data window
    m_pDataWindow->SetSizePixel(Size(i_dataCellPlayground.GetWidth() + m_nRowHeaderWidthPixel,
                                     i_dataCellPlayground.GetHeight() + m_nColHeaderHeightPixel));
}

void TableControl::onResize()
{
    impl_ni_relayout();
    checkCursorPosition();
}

void TableControl::Select()
{
    ImplCallEventListenersAndHandler(VclEventId::TableRowSelect, nullptr);
    commitAccessibleEvent(AccessibleEventId::SELECTION_CHANGED);

    commitTableEvent(AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, Any(), Any());
    // TODO: why do we notify this when the *selection* changed? Shouldn't we find a better place for this,
    // actually, when the active descendant, i.e. the current cell, *really* changed?
}

void TableControl::doPaintContent(vcl::RenderContext& rRenderContext,
                                  const tools::Rectangle& _rUpdateRect)
{
    if (!GetModel())
        return;
    PTableRenderer pRenderer = GetModel()->getRenderer();
    DBG_ASSERT(!!pRenderer, "TableDataWindow::doPaintContent: invalid renderer!");
    if (!pRenderer)
        return;

    // our current style settings, to be passed to the renderer
    const StyleSettings& rStyle = rRenderContext.GetSettings().GetStyleSettings();
    m_nRowCount = m_pModel->getRowCount();
    // the area occupied by all (at least partially) visible cells, including
    // headers
    tools::Rectangle const aAllCellsWithHeaders(impl_getAllVisibleCellsArea());

    // draw the header column area
    if (m_pModel->hasColumnHeaders())
    {
        TableRowGeometry const aHeaderRow(
            *this, tools::Rectangle(Point(0, 0), aAllCellsWithHeaders.BottomRight()),
            ROW_COL_HEADERS);
        tools::Rectangle const aColRect(aHeaderRow.getRect());
        pRenderer->PaintHeaderArea(rRenderContext, aColRect, true, false, rStyle);
        // Note that strictly, aHeaderRow.getRect() also contains the intersection between column
        // and row header area. However, below we go to paint this intersection, again,
        // so this hopefully doesn't hurt if we already paint it here.

        for (TableCellGeometry aCell(aHeaderRow, m_nLeftColumn); aCell.isValid(); aCell.moveRight())
        {
            if (_rUpdateRect.GetIntersection(aCell.getRect()).IsEmpty())
                continue;

            pRenderer->PaintColumnHeader(aCell.getColumn(), rRenderContext, aCell.getRect(),
                                         rStyle);
        }
    }
    // the area occupied by the row header, if any
    tools::Rectangle aRowHeaderArea;
    if (m_pModel->hasRowHeaders())
    {
        aRowHeaderArea = aAllCellsWithHeaders;
        aRowHeaderArea.SetRight(m_nRowHeaderWidthPixel - 1);

        TableSize const nVisibleRows = impl_getVisibleRows(true);
        TableSize nActualRows = nVisibleRows;
        if (m_nTopRow + nActualRows > m_nRowCount)
            nActualRows = m_nRowCount - m_nTopRow;
        aRowHeaderArea.SetBottom(m_nColHeaderHeightPixel + m_nRowHeightPixel * nActualRows - 1);

        pRenderer->PaintHeaderArea(rRenderContext, aRowHeaderArea, false, true, rStyle);
        // Note that strictly, aRowHeaderArea also contains the intersection between column
        // and row header area. However, below we go to paint this intersection, again,
        // so this hopefully doesn't hurt if we already paint it here.

        if (m_pModel->hasColumnHeaders())
        {
            TableCellGeometry const aIntersection(
                *this, tools::Rectangle(Point(0, 0), aAllCellsWithHeaders.BottomRight()),
                COL_ROW_HEADERS, ROW_COL_HEADERS);
            tools::Rectangle const aInters(aIntersection.getRect());
            pRenderer->PaintHeaderArea(rRenderContext, aInters, true, true, rStyle);
        }
    }

    // draw the table content row by row
    TableSize colCount = GetModel()->getColumnCount();

    // paint all rows
    tools::Rectangle const aAllDataCellsArea(impl_getAllVisibleDataCellArea());
    for (TableRowGeometry aRowIterator(*this, aAllCellsWithHeaders, getTopRow());
         aRowIterator.isValid(); aRowIterator.moveDown())
    {
        if (_rUpdateRect.GetIntersection(aRowIterator.getRect()).IsEmpty())
            continue;

        bool const isControlFocused = HasControlFocus();
        bool const isSelectedRow = IsRowSelected(aRowIterator.getRow());

        tools::Rectangle const aRect = aRowIterator.getRect().GetIntersection(aAllDataCellsArea);

        // give the renderer a chance to prepare the row
        pRenderer->PrepareRow(aRowIterator.getRow(), isControlFocused, isSelectedRow,
                              rRenderContext, aRect, rStyle);

        // paint the row header
        if (m_pModel->hasRowHeaders())
        {
            const tools::Rectangle aCurrentRowHeader(
                aRowHeaderArea.GetIntersection(aRowIterator.getRect()));
            pRenderer->PaintRowHeader(rRenderContext, aCurrentRowHeader, rStyle);
        }

        if (!colCount)
            continue;

        // paint all cells in this row
        for (TableCellGeometry aCell(aRowIterator, m_nLeftColumn); aCell.isValid();
             aCell.moveRight())
        {
            pRenderer->PaintCell(aCell.getColumn(), isSelectedRow, isControlFocused, rRenderContext,
                                 aCell.getRect(), rStyle);
        }
    }
}

void TableControl::hideCursor()
{
    if (++m_nCursorHidden == 1)
        impl_ni_doSwitchCursor(false);
}

void TableControl::showCursor()
{
    DBG_ASSERT(m_nCursorHidden > 0, "TableControl::showCursor: cursor not hidden!");
    if (--m_nCursorHidden == 0)
        impl_ni_doSwitchCursor(true);
}

bool TableControl::dispatchAction(TableControlAction _eAction)
{
    bool bSuccess = false;
    bool selectionChanged = false;

    switch (_eAction)
    {
        case TableControlAction::cursorDown:
            if (m_pSelEngine->GetSelectionMode() == SelectionMode::Single)
            {
                //if other rows already selected, deselect them
                if (!m_aSelectedRows.empty())
                {
                    invalidateSelectedRows();
                    m_aSelectedRows.clear();
                }
                if (m_nCurRow < m_nRowCount - 1)
                {
                    ++m_nCurRow;
                    m_aSelectedRows.push_back(m_nCurRow);
                }
                else
                    m_aSelectedRows.push_back(m_nCurRow);
                invalidateRow(m_nCurRow);
                ensureVisible(m_nCurColumn, m_nCurRow);
                selectionChanged = true;
                bSuccess = true;
            }
            else
            {
                if (m_nCurRow < m_nRowCount - 1)
                    bSuccess = GoToCell(m_nCurColumn, m_nCurRow + 1);
            }
            break;

        case TableControlAction::cursorUp:
            if (m_pSelEngine->GetSelectionMode() == SelectionMode::Single)
            {
                if (!m_aSelectedRows.empty())
                {
                    invalidateSelectedRows();
                    m_aSelectedRows.clear();
                }
                if (m_nCurRow > 0)
                {
                    --m_nCurRow;
                    m_aSelectedRows.push_back(m_nCurRow);
                    invalidateRow(m_nCurRow);
                }
                else
                {
                    m_aSelectedRows.push_back(m_nCurRow);
                    invalidateRow(m_nCurRow);
                }
                ensureVisible(m_nCurColumn, m_nCurRow);
                selectionChanged = true;
                bSuccess = true;
            }
            else
            {
                if (m_nCurRow > 0)
                    bSuccess = GoToCell(m_nCurColumn, m_nCurRow - 1);
            }
            break;
        case TableControlAction::cursorLeft:
            if (m_nCurColumn > 0)
                bSuccess = GoToCell(m_nCurColumn - 1, m_nCurRow);
            else if ((m_nCurColumn == 0) && (m_nCurRow > 0))
                bSuccess = GoToCell(m_nColumnCount - 1, m_nCurRow - 1);
            break;

        case TableControlAction::cursorRight:
            if (m_nCurColumn < m_nColumnCount - 1)
                bSuccess = GoToCell(m_nCurColumn + 1, m_nCurRow);
            else if ((m_nCurColumn == m_nColumnCount - 1) && (m_nCurRow < m_nRowCount - 1))
                bSuccess = GoToCell(0, m_nCurRow + 1);
            break;

        case TableControlAction::cursorToLineStart:
            bSuccess = GoToCell(0, m_nCurRow);
            break;

        case TableControlAction::cursorToLineEnd:
            bSuccess = GoToCell(m_nColumnCount - 1, m_nCurRow);
            break;

        case TableControlAction::cursorToFirstLine:
            bSuccess = GoToCell(m_nCurColumn, 0);
            break;

        case TableControlAction::cursorToLastLine:
            bSuccess = GoToCell(m_nCurColumn, m_nRowCount - 1);
            break;

        case TableControlAction::cursorPageUp:
        {
            sal_Int32 nNewRow = ::std::max(sal_Int32(0), m_nCurRow - impl_getVisibleRows(false));
            bSuccess = GoToCell(m_nCurColumn, nNewRow);
        }
        break;

        case TableControlAction::cursorPageDown:
        {
            sal_Int32 nNewRow = ::std::min(m_nRowCount - 1, m_nCurRow + impl_getVisibleRows(false));
            bSuccess = GoToCell(m_nCurColumn, nNewRow);
        }
        break;

        case TableControlAction::cursorTopLeft:
            bSuccess = GoToCell(0, 0);
            break;

        case TableControlAction::cursorBottomRight:
            bSuccess = GoToCell(m_nColumnCount - 1, m_nRowCount - 1);
            break;

        case TableControlAction::cursorSelectRow:
        {
            if (m_pSelEngine->GetSelectionMode() == SelectionMode::NONE)
                return false;
            //pos is the position of the current row in the vector of selected rows, if current row is selected
            int pos = getRowSelectedNumber(m_aSelectedRows, m_nCurRow);
            //if current row is selected, it should be deselected, when ALT+SPACE are pressed
            if (pos > -1)
            {
                m_aSelectedRows.erase(m_aSelectedRows.begin() + pos);
                if (m_aSelectedRows.empty() && m_nAnchor != -1)
                    m_nAnchor = -1;
            }
            //else select the row->put it in the vector
            else
                m_aSelectedRows.push_back(m_nCurRow);
            invalidateRow(m_nCurRow);
            selectionChanged = true;
            bSuccess = true;
        }
        break;
        case TableControlAction::cursorSelectRowUp:
        {
            if (m_pSelEngine->GetSelectionMode() == SelectionMode::NONE)
                return false;
            else if (m_pSelEngine->GetSelectionMode() == SelectionMode::Single)
            {
                //if there are other selected rows, deselect them
                return false;
            }
            else
            {
                //there are other selected rows
                if (!m_aSelectedRows.empty())
                {
                    //the anchor wasn't set -> a region is not selected, that's why clear all selection
                    //and select the current row
                    if (m_nAnchor == -1)
                    {
                        invalidateSelectedRows();
                        m_aSelectedRows.clear();
                        m_aSelectedRows.push_back(m_nCurRow);
                        invalidateRow(m_nCurRow);
                    }
                    else
                    {
                        //a region is already selected, prevRow is last selected row and the row above - nextRow - should be selected
                        int prevRow = getRowSelectedNumber(m_aSelectedRows, m_nCurRow);
                        int nextRow = getRowSelectedNumber(m_aSelectedRows, m_nCurRow - 1);
                        if (prevRow > -1)
                        {
                            //if m_nCurRow isn't the upper one, can move up, otherwise not
                            if (m_nCurRow > 0)
                                m_nCurRow--;
                            else
                                return true;
                            //if nextRow already selected, deselect it, otherwise select it
                            if (nextRow > -1 && m_aSelectedRows[nextRow] == m_nCurRow)
                            {
                                m_aSelectedRows.erase(m_aSelectedRows.begin() + prevRow);
                                invalidateRow(m_nCurRow + 1);
                            }
                            else
                            {
                                m_aSelectedRows.push_back(m_nCurRow);
                                invalidateRow(m_nCurRow);
                            }
                        }
                        else
                        {
                            if (m_nCurRow > 0)
                            {
                                m_aSelectedRows.push_back(m_nCurRow);
                                m_nCurRow--;
                                m_aSelectedRows.push_back(m_nCurRow);
                                invalidateSelectedRegion(m_nCurRow + 1, m_nCurRow);
                            }
                        }
                    }
                }
                else
                {
                    //if nothing is selected and the current row isn't the upper one
                    //select the current and one row above
                    //otherwise select only the upper row
                    if (m_nCurRow > 0)
                    {
                        m_aSelectedRows.push_back(m_nCurRow);
                        m_nCurRow--;
                        m_aSelectedRows.push_back(m_nCurRow);
                        invalidateSelectedRegion(m_nCurRow + 1, m_nCurRow);
                    }
                    else
                    {
                        m_aSelectedRows.push_back(m_nCurRow);
                        invalidateRow(m_nCurRow);
                    }
                }
                m_pSelEngine->SetAnchor(true);
                m_nAnchor = m_nCurRow;
                ensureVisible(m_nCurColumn, m_nCurRow);
                selectionChanged = true;
                bSuccess = true;
            }
        }
        break;
        case TableControlAction::cursorSelectRowDown:
        {
            if (m_pSelEngine->GetSelectionMode() == SelectionMode::NONE)
                bSuccess = false;
            else if (m_pSelEngine->GetSelectionMode() == SelectionMode::Single)
            {
                bSuccess = false;
            }
            else
            {
                if (!m_aSelectedRows.empty())
                {
                    //the anchor wasn't set -> a region is not selected, that's why clear all selection
                    //and select the current row
                    if (m_nAnchor == -1)
                    {
                        invalidateSelectedRows();
                        m_aSelectedRows.clear();
                        m_aSelectedRows.push_back(m_nCurRow);
                        invalidateRow(m_nCurRow);
                    }
                    else
                    {
                        //a region is already selected, prevRow is last selected row and the row beneath - nextRow - should be selected
                        int prevRow = getRowSelectedNumber(m_aSelectedRows, m_nCurRow);
                        int nextRow = getRowSelectedNumber(m_aSelectedRows, m_nCurRow + 1);
                        if (prevRow > -1)
                        {
                            //if m_nCurRow isn't the last one, can move down, otherwise not
                            if (m_nCurRow < m_nRowCount - 1)
                                m_nCurRow++;
                            else
                                return true;
                            //if next row already selected, deselect it, otherwise select it
                            if (nextRow > -1 && m_aSelectedRows[nextRow] == m_nCurRow)
                            {
                                m_aSelectedRows.erase(m_aSelectedRows.begin() + prevRow);
                                invalidateRow(m_nCurRow - 1);
                            }
                            else
                            {
                                m_aSelectedRows.push_back(m_nCurRow);
                                invalidateRow(m_nCurRow);
                            }
                        }
                        else
                        {
                            if (m_nCurRow < m_nRowCount - 1)
                            {
                                m_aSelectedRows.push_back(m_nCurRow);
                                m_nCurRow++;
                                m_aSelectedRows.push_back(m_nCurRow);
                                invalidateSelectedRegion(m_nCurRow - 1, m_nCurRow);
                            }
                        }
                    }
                }
                else
                {
                    //there wasn't any selection, select current and row beneath, otherwise only row beneath
                    if (m_nCurRow < m_nRowCount - 1)
                    {
                        m_aSelectedRows.push_back(m_nCurRow);
                        m_nCurRow++;
                        m_aSelectedRows.push_back(m_nCurRow);
                        invalidateSelectedRegion(m_nCurRow - 1, m_nCurRow);
                    }
                    else
                    {
                        m_aSelectedRows.push_back(m_nCurRow);
                        invalidateRow(m_nCurRow);
                    }
                }
                m_pSelEngine->SetAnchor(true);
                m_nAnchor = m_nCurRow;
                ensureVisible(m_nCurColumn, m_nCurRow);
                selectionChanged = true;
                bSuccess = true;
            }
        }
        break;

        case TableControlAction::cursorSelectRowAreaTop:
        {
            if (m_pSelEngine->GetSelectionMode() == SelectionMode::NONE)
                bSuccess = false;
            else if (m_pSelEngine->GetSelectionMode() == SelectionMode::Single)
                bSuccess = false;
            else
            {
                //select the region between the current and the upper row
                sal_Int32 iter = m_nCurRow;
                invalidateSelectedRegion(m_nCurRow, 0);
                //put the rows in vector
                while (iter >= 0)
                {
                    if (!IsRowSelected(iter))
                        m_aSelectedRows.push_back(iter);
                    --iter;
                }
                m_nCurRow = 0;
                m_nAnchor = m_nCurRow;
                m_pSelEngine->SetAnchor(true);
                ensureVisible(m_nCurColumn, 0);
                selectionChanged = true;
                bSuccess = true;
            }
        }
        break;

        case TableControlAction::cursorSelectRowAreaBottom:
        {
            if (m_pSelEngine->GetSelectionMode() == SelectionMode::NONE)
                return false;
            else if (m_pSelEngine->GetSelectionMode() == SelectionMode::Single)
                return false;
            //select the region between the current and the last row
            sal_Int32 iter = m_nCurRow;
            invalidateSelectedRegion(m_nCurRow, m_nRowCount - 1);
            //put the rows in the vector
            while (iter <= m_nRowCount)
            {
                if (!IsRowSelected(iter))
                    m_aSelectedRows.push_back(iter);
                ++iter;
            }
            m_nCurRow = m_nRowCount - 1;
            m_nAnchor = m_nCurRow;
            m_pSelEngine->SetAnchor(true);
            ensureVisible(m_nCurColumn, m_nRowCount - 1);
            selectionChanged = true;
            bSuccess = true;
        }
        break;
        default:
            OSL_FAIL("TableControl::dispatchAction: unsupported action!");
            break;
    }

    if (bSuccess && selectionChanged)
    {
        Select();
    }

    return bSuccess;
}

void TableControl::impl_ni_doSwitchCursor(bool _bShow)
{
    PTableRenderer pRenderer = m_pModel ? m_pModel->getRenderer() : PTableRenderer();
    if (pRenderer)
    {
        tools::Rectangle aCellRect = calcCellRect(m_nCurRow, m_nCurColumn);
        if (_bShow)
            pRenderer->ShowCellCursor(*m_pDataWindow, aCellRect);
        else
            pRenderer->HideCellCursor(*m_pDataWindow);
    }
}

sal_Int32 TableControl::getRowAtPoint(const Point& rPoint) const
{
    return impl_getRowForAbscissa(rPoint.Y());
}

sal_Int32 TableControl::getColAtPoint(const Point& rPoint) const
{
    return impl_getColumnForOrdinate(rPoint.X());
}

TableCell TableControl::hitTest(Point const& i_point) const
{
    TableCell aCell(getColAtPoint(i_point), getRowAtPoint(i_point));
    if (aCell.nColumn > COL_ROW_HEADERS)
    {
        PColumnModel const pColumn = m_pModel->getColumnModel(aCell.nColumn);
        MutableColumnMetrics const& rColInfo(m_aColumnWidths[aCell.nColumn]);
        if ((rColInfo.getEnd() - 3 <= i_point.X()) && (rColInfo.getEnd() >= i_point.X())
            && pColumn->isResizable())
        {
            aCell.eArea = ColumnDivider;
        }
    }
    return aCell;
}

ColumnMetrics TableControl::getColumnMetrics(sal_Int32 const i_column) const
{
    ENSURE_OR_RETURN((i_column >= 0) && (i_column < m_pModel->getColumnCount()),
                     "TableControl::getColumnMetrics: illegal column index!", ColumnMetrics());
    return m_aColumnWidths[i_column];
}

PTableModel TableControl::GetModel() const { return m_pModel; }

sal_Int32 TableControl::GetCurrentColumn() const { return m_nCurColumn; }

sal_Int32 TableControl::GetCurrentRow() const { return m_nCurRow; }

::Size TableControl::getTableSizePixel() const { return m_pDataWindow->GetOutputSizePixel(); }

void TableControl::setPointer(PointerStyle i_pointer) { m_pDataWindow->SetPointer(i_pointer); }

void TableControl::captureMouse() { m_pDataWindow->CaptureMouse(); }

void TableControl::releaseMouse() { m_pDataWindow->ReleaseMouse(); }

void TableControl::invalidate(TableArea const i_what)
{
    switch (i_what)
    {
        case TableArea::ColumnHeaders:
            m_pDataWindow->Invalidate(calcHeaderRect(true));
            break;

        case TableArea::RowHeaders:
            m_pDataWindow->Invalidate(calcHeaderRect(false));
            break;

        case TableArea::All:
            m_pDataWindow->Invalidate();
            m_pDataWindow->GetParent()->Invalidate(InvalidateFlags::Transparent);
            break;
    }
}

tools::Long TableControl::pixelWidthToAppFont(tools::Long const i_pixels) const
{
    return m_pDataWindow->PixelToLogic(Size(i_pixels, 0), MapMode(MapUnit::MapAppFont)).Width();
}

tools::Long TableControl::appFontWidthToPixel(tools::Long const i_appFontUnits) const
{
    return m_pDataWindow->LogicToPixel(Size(i_appFontUnits, 0), MapMode(MapUnit::MapAppFont))
        .Width();
}

void TableControl::hideTracking() { m_pDataWindow->HideTracking(); }

void TableControl::showTracking(tools::Rectangle const& i_location, ShowTrackFlags const i_flags)
{
    m_pDataWindow->ShowTracking(i_location, i_flags);
}

void TableControl::activateCell(sal_Int32 const i_col, sal_Int32 const i_row)
{
    GoToCell(i_col, i_row);
}

void TableControl::invalidateSelectedRegion(sal_Int32 _nPrevRow, sal_Int32 _nCurRow)
{
    // get the visible area of the table control and set the Left and right border of the region to be repainted
    tools::Rectangle const aAllCells(impl_getAllVisibleCellsArea());

    tools::Rectangle aInvalidateRect;
    aInvalidateRect.SetLeft(aAllCells.Left());
    aInvalidateRect.SetRight(aAllCells.Right());
    // if only one row is selected
    if (_nPrevRow == _nCurRow)
    {
        tools::Rectangle aCellRect = calcCellRect(_nCurRow, m_nCurColumn);
        aInvalidateRect.SetTop(aCellRect.Top());
        aInvalidateRect.SetBottom(aCellRect.Bottom());
    }
    //if the region is above the current row
    else if (_nPrevRow < _nCurRow)
    {
        tools::Rectangle aCellRect = calcCellRect(_nPrevRow, m_nCurColumn);
        aInvalidateRect.SetTop(aCellRect.Top());
        aCellRect = calcCellRect(_nCurRow, m_nCurColumn);
        aInvalidateRect.SetBottom(aCellRect.Bottom());
    }
    //if the region is beneath the current row
    else
    {
        tools::Rectangle aCellRect = calcCellRect(_nCurRow, m_nCurColumn);
        aInvalidateRect.SetTop(aCellRect.Top());
        aCellRect = calcCellRect(_nPrevRow, m_nCurColumn);
        aInvalidateRect.SetBottom(aCellRect.Bottom());
    }

    invalidateRect(aInvalidateRect);
}

void TableControl::invalidateRect(const tools::Rectangle& rInvalidateRect)
{
    m_pDataWindow->Invalidate(rInvalidateRect, m_pDataWindow->GetControlBackground().IsTransparent()
                                                   ? InvalidateFlags::Transparent
                                                   : InvalidateFlags::NONE);
}

void TableControl::invalidateSelectedRows()
{
    for (auto const& selectedRow : m_aSelectedRows)
    {
        invalidateRow(selectedRow);
    }
}

void TableControl::invalidateRowRange(sal_Int32 const i_firstRow, sal_Int32 const i_lastRow)
{
    sal_Int32 const firstRow = i_firstRow < m_nTopRow ? m_nTopRow : i_firstRow;
    sal_Int32 const lastVisibleRow = m_nTopRow + impl_getVisibleRows(true) - 1;
    sal_Int32 const lastRow
        = ((i_lastRow == ROW_INVALID) || (i_lastRow > lastVisibleRow)) ? lastVisibleRow : i_lastRow;

    tools::Rectangle aInvalidateRect;

    tools::Rectangle const aVisibleCellsArea(impl_getAllVisibleCellsArea());
    TableRowGeometry aRow(*this, aVisibleCellsArea, firstRow, true);
    while (aRow.isValid() && (aRow.getRow() <= lastRow))
    {
        aInvalidateRect.Union(aRow.getRect());
        aRow.moveDown();
    }

    if (i_lastRow == ROW_INVALID)
        aInvalidateRect.SetBottom(m_pDataWindow->GetOutputSizePixel().Height());

    invalidateRect(aInvalidateRect);
}

void TableControl::checkCursorPosition()
{
    TableSize nVisibleRows = impl_getVisibleRows(true);
    TableSize nVisibleCols = impl_getVisibleColumns(true);
    if ((m_nTopRow + nVisibleRows > m_nRowCount) && (m_nRowCount >= nVisibleRows))
    {
        --m_nTopRow;
    }
    else
    {
        m_nTopRow = 0;
    }

    if ((m_nLeftColumn + nVisibleCols > m_nColumnCount) && (m_nColumnCount >= nVisibleCols))
    {
        --m_nLeftColumn;
    }
    else
    {
        m_nLeftColumn = 0;
    }

    m_pDataWindow->Invalidate();
}

TableSize TableControl::impl_getVisibleRows(bool _bAcceptPartialRow) const
{
    DBG_ASSERT(m_pDataWindow, "TableControl::impl_getVisibleRows: no data window!");

    return lcl_getRowsFittingInto(m_pDataWindow->GetOutputSizePixel().Height()
                                      - m_nColHeaderHeightPixel,
                                  m_nRowHeightPixel, _bAcceptPartialRow);
}

TableSize TableControl::impl_getVisibleColumns(bool _bAcceptPartialCol) const
{
    DBG_ASSERT(m_pDataWindow, "TableControl::impl_getVisibleColumns: no data window!");

    return lcl_getColumnsVisibleWithin(
        tools::Rectangle(Point(0, 0), m_pDataWindow->GetOutputSizePixel()), m_nLeftColumn, *this,
        _bAcceptPartialCol);
}

bool TableControl::GoToCell(sal_Int32 _nColumn, sal_Int32 _nRow)
{
    // TODO: give veto listeners a chance

    if ((_nColumn < 0) || (_nColumn >= m_nColumnCount) || (_nRow < 0) || (_nRow >= m_nRowCount))
    {
        OSL_ENSURE(false, "TableControl::GoToCell: invalid row or column index!");
        return false;
    }

    SuppressCursor aHideCursor(*this);
    m_nCurColumn = _nColumn;
    m_nCurRow = _nRow;

    // ensure that the new cell is visible
    ensureVisible(m_nCurColumn, m_nCurRow);
    return true;
}

void TableControl::ensureVisible(sal_Int32 _nColumn, sal_Int32 _nRow)
{
    DBG_ASSERT((_nColumn >= 0) && (_nColumn < m_nColumnCount) && (_nRow >= 0)
                   && (_nRow < m_nRowCount),
               "TableControl::ensureVisible: invalid coordinates!");

    SuppressCursor aHideCursor(*this);

    if (_nColumn < m_nLeftColumn)
        impl_scrollColumns(_nColumn - m_nLeftColumn);
    else
    {
        TableSize nVisibleColumns = impl_getVisibleColumns(false /*bAcceptPartialVisibility*/);
        if (_nColumn > m_nLeftColumn + nVisibleColumns - 1)
        {
            impl_scrollColumns(_nColumn - (m_nLeftColumn + nVisibleColumns - 1));
            // TODO: since not all columns have the same width, this might in theory result
            // in the column still not being visible.
        }
    }

    if (_nRow < m_nTopRow)
        impl_scrollRows(_nRow - m_nTopRow);
    else
    {
        TableSize nVisibleRows = impl_getVisibleRows(false /*_bAcceptPartialVisibility*/);
        if (_nRow > m_nTopRow + nVisibleRows - 1)
            impl_scrollRows(_nRow - (m_nTopRow + nVisibleRows - 1));
    }
}

OUString TableControl::GetAccessibleCellText(sal_Int32 const i_row, sal_Int32 const i_col)
{
    Any aCellValue;
    m_pModel->getCellContent(i_col, i_row, aCellValue);

    OUString sCellStringContent;
    m_pModel->getRenderer()->GetFormattedCellString(aCellValue, sCellStringContent);

    return sCellStringContent;
}

TableSize TableControl::impl_ni_ScrollRows(TableSize _nRowDelta)
{
    // compute new top row
    sal_Int32 nNewTopRow = ::std::max(::std::min(static_cast<sal_Int32>(m_nTopRow + _nRowDelta),
                                                 static_cast<sal_Int32>(m_nRowCount - 1)),
                                      sal_Int32(0));

    sal_Int32 nOldTopRow = m_nTopRow;
    m_nTopRow = nNewTopRow;

    // if updates are enabled currently, scroll the viewport
    if (m_nTopRow != nOldTopRow)
    {
        SuppressCursor aHideCursor(*this);
        // TODO: call an onStartScroll at our listener (or better an own onStartScroll,
        // which hides the cursor and then calls the listener)
        // Same for onEndScroll

        // scroll the view port, if possible
        tools::Long nPixelDelta = m_nRowHeightPixel * (m_nTopRow - nOldTopRow);

        tools::Rectangle aDataArea(Point(0, m_nColHeaderHeightPixel),
                                   m_pDataWindow->GetOutputSizePixel());

        if (m_pDataWindow->GetBackground().IsScrollable()
            && std::abs(nPixelDelta) < aDataArea.GetHeight())
        {
            m_pDataWindow->Scroll(0, static_cast<tools::Long>(-nPixelDelta), aDataArea,
                                  ScrollFlags::Clip | ScrollFlags::Update | ScrollFlags::Children);
        }
        else
        {
            m_pDataWindow->Invalidate(InvalidateFlags::Update);
            m_pDataWindow->GetParent()->Invalidate(InvalidateFlags::Transparent);
        }

        // update the position at the vertical scrollbar
        if (m_pVScroll != nullptr)
            m_pVScroll->SetThumbPos(m_nTopRow);
    }

    // The scroll bar availability might change when we scrolled.
    // For instance, imagine a view with 10 rows, if which 5 fit into the window, numbered 1 to 10.
    // Now let
    // - the user scroll to row number 6, so the last 5 rows are visible
    // - somebody remove the last 4 rows
    // - the user scroll to row number 5 being the top row, so the last two rows are visible
    // - somebody remove row number 6
    // - the user scroll to row number 1
    // => in this case, the need for the scrollbar vanishes immediately.
    if (m_nTopRow == 0)
        PostUserEvent(LINK(this, TableControl, OnUpdateScrollbars));

    return static_cast<TableSize>(m_nTopRow - nOldTopRow);
}

TableSize TableControl::impl_scrollRows(TableSize const i_rowDelta)
{
    return impl_ni_ScrollRows(i_rowDelta);
}

TableSize TableControl::impl_ni_ScrollColumns(TableSize _nColumnDelta)
{
    // compute new left column
    const sal_Int32 nNewLeftColumn
        = ::std::max(::std::min(static_cast<sal_Int32>(m_nLeftColumn + _nColumnDelta),
                                static_cast<sal_Int32>(m_nColumnCount - 1)),
                     sal_Int32(0));

    const sal_Int32 nOldLeftColumn = m_nLeftColumn;
    m_nLeftColumn = nNewLeftColumn;

    // if updates are enabled currently, scroll the viewport
    if (m_nLeftColumn != nOldLeftColumn)
    {
        SuppressCursor aHideCursor(*this);
        // TODO: call an onStartScroll at our listener (or better an own onStartScroll,
        // which hides the cursor and then calls the listener)
        // Same for onEndScroll

        // scroll the view port, if possible
        const tools::Rectangle aDataArea(Point(m_nRowHeaderWidthPixel, 0),
                                         m_pDataWindow->GetOutputSizePixel());

        tools::Long nPixelDelta = m_aColumnWidths[nOldLeftColumn].getStart()
                                  - m_aColumnWidths[m_nLeftColumn].getStart();

        // update our column positions
        // Do this *before* scrolling, as ScrollFlags::Update will trigger a paint, which already needs the correct
        // information in m_aColumnWidths
        for (auto& columnWidth : m_aColumnWidths)
        {
            columnWidth.move(nPixelDelta);
        }

        // scroll the window content (if supported and possible), or invalidate the complete window
        if (m_pDataWindow->GetBackground().IsScrollable()
            && std::abs(nPixelDelta) < aDataArea.GetWidth())
        {
            m_pDataWindow->Scroll(nPixelDelta, 0, aDataArea,
                                  ScrollFlags::Clip | ScrollFlags::Update);
        }
        else
        {
            m_pDataWindow->Invalidate(InvalidateFlags::Update);
            m_pDataWindow->GetParent()->Invalidate(InvalidateFlags::Transparent);
        }

        // update the position at the horizontal scrollbar
        if (m_pHScroll != nullptr)
            m_pHScroll->SetThumbPos(m_nLeftColumn);
    }

    // The scroll bar availability might change when we scrolled. This is because we do not hide
    // the scrollbar when it is, in theory, unnecessary, but currently at a position > 0. In this case, it will
    // be auto-hidden when it's scrolled back to pos 0.
    if (m_nLeftColumn == 0)
        PostUserEvent(LINK(this, TableControl, OnUpdateScrollbars));

    return static_cast<TableSize>(m_nLeftColumn - nOldLeftColumn);
}

TableSize TableControl::impl_scrollColumns(TableSize const i_columnDelta)
{
    return impl_ni_ScrollColumns(i_columnDelta);
}

SelectionEngine* TableControl::getSelEngine() { return m_pSelEngine.get(); }

sal_Int32 TableControl::GetRowCount() const { return m_pModel->getRowCount(); }

sal_Int32 TableControl::GetColumnCount() const { return m_pModel->getColumnCount(); }

OUString TableControl::GetRowName(sal_Int32 nIndex) const
{
    OUString sRowName;
    m_pModel->getRowHeading(nIndex) >>= sRowName;
    return sRowName;
}

OUString TableControl::GetColumnName(sal_Int32 nIndex) const
{
    return m_pModel->getColumnModel(nIndex)->getName();
}

bool TableControl::HasRowHeader() { return m_pModel->hasRowHeaders(); }

bool TableControl::HasColumnHeader() { return m_pModel->hasColumnHeaders(); }

bool TableControl::IsRowSelected(sal_Int32 i_row) const
{
    return ::std::find(m_aSelectedRows.begin(), m_aSelectedRows.end(), i_row)
           != m_aSelectedRows.end();
}

sal_Int32 TableControl::GetSelectedRowIndex(size_t const i_selectionIndex) const
{
    if (i_selectionIndex < m_aSelectedRows.size())
        return m_aSelectedRows[i_selectionIndex];
    return ROW_INVALID;
}

int TableControl::getRowSelectedNumber(const ::std::vector<sal_Int32>& selectedRows,
                                       sal_Int32 current)
{
    std::vector<sal_Int32>::const_iterator it
        = ::std::find(selectedRows.begin(), selectedRows.end(), current);
    if (it != selectedRows.end())
    {
        return it - selectedRows.begin();
    }
    return -1;
}

void TableControl::SelectRow(sal_Int32 nRowIndex, bool bSelect)
{
    ENSURE_OR_RETURN_VOID((nRowIndex >= 0) && (nRowIndex < m_pModel->getRowCount()),
                          "TableControl::SelectRow: invalid row index!");

    if (bSelect)
    {
        if (!markRowAsSelected(nRowIndex))
            // nothing to do
            return;
    }
    else
    {
        markRowAsDeselected(nRowIndex);
    }

    invalidateRowRange(nRowIndex, nRowIndex);
    Select();
}

void TableControl::SelectAllRows(bool bSelect)
{
    if (bSelect)
    {
        if (!markAllRowsAsSelected())
            // nothing to do
            return;
    }
    else
    {
        if (!markAllRowsAsDeselected())
            // nothing to do
            return;
    }

    Invalidate();
    // TODO: can't we do better than this, and invalidate only the rows which changed?
    Select();
}

sal_Int32 TableControl::impl_getColumnForOrdinate(tools::Long const i_ordinate) const
{
    if ((m_aColumnWidths.empty()) || (i_ordinate < 0))
        return COL_INVALID;

    if (i_ordinate < m_nRowHeaderWidthPixel)
        return COL_ROW_HEADERS;

    ColumnPositions::const_iterator lowerBound = ::std::lower_bound(
        m_aColumnWidths.begin(), m_aColumnWidths.end(),
        MutableColumnMetrics(i_ordinate + 1, i_ordinate + 1), ColumnInfoPositionLess());
    if (lowerBound == m_aColumnWidths.end())
    {
        // point is *behind* the start of the last column ...
        if (i_ordinate < m_aColumnWidths.rbegin()->getEnd())
            // ... but still before its end
            return m_nColumnCount - 1;
        return COL_INVALID;
    }
    return lowerBound - m_aColumnWidths.begin();
}

sal_Int32 TableControl::impl_getRowForAbscissa(tools::Long const i_abscissa) const
{
    if (i_abscissa < 0)
        return ROW_INVALID;

    if (i_abscissa < m_nColHeaderHeightPixel)
        return ROW_COL_HEADERS;

    tools::Long const abscissa = i_abscissa - m_nColHeaderHeightPixel;
    tools::Long const row = m_nTopRow + abscissa / m_nRowHeightPixel;
    return row < m_pModel->getRowCount() ? row : ROW_INVALID;
}

bool TableControl::markRowAsDeselected(sal_Int32 const i_rowIndex)
{
    ::std::vector<sal_Int32>::iterator selPos
        = ::std::find(m_aSelectedRows.begin(), m_aSelectedRows.end(), i_rowIndex);
    if (selPos == m_aSelectedRows.end())
        return false;

    m_aSelectedRows.erase(selPos);
    return true;
}

bool TableControl::markRowAsSelected(sal_Int32 const i_rowIndex)
{
    if (IsRowSelected(i_rowIndex))
        return false;

    SelectionMode const eSelMode = getSelEngine()->GetSelectionMode();
    switch (eSelMode)
    {
        case SelectionMode::Single:
            if (!m_aSelectedRows.empty())
            {
                OSL_ENSURE(m_aSelectedRows.size() == 1, "TableControl::markRowAsSelected: "
                                                        "SingleSelection with more than one "
                                                        "selected element?");
                m_aSelectedRows[0] = i_rowIndex;
                break;
            }
            [[fallthrough]];

        case SelectionMode::Multiple:
            m_aSelectedRows.push_back(i_rowIndex);
            break;

        default:
            OSL_ENSURE(false, "TableControl::markRowAsSelected: unsupported selection mode!");
            return false;
    }

    return true;
}

bool TableControl::markAllRowsAsDeselected()
{
    if (m_aSelectedRows.empty())
        return false;

    m_aSelectedRows.clear();
    return true;
}

bool TableControl::markAllRowsAsSelected()
{
    SelectionMode const eSelMode = getSelEngine()->GetSelectionMode();
    ENSURE_OR_RETURN_FALSE(eSelMode == SelectionMode::Multiple,
                           "TableControl::markAllRowsAsSelected: unsupported selection mode!");

    if (m_aSelectedRows.size() == size_t(m_pModel->getRowCount()))
    {
#if OSL_DEBUG_LEVEL > 0
        for (TableSize row = 0; row < m_pModel->getRowCount(); ++row)
        {
            OSL_ENSURE(IsRowSelected(row),
                       "TableControl::markAllRowsAsSelected: inconsistency in the selected rows!");
        }
#endif
        // already all rows marked as selected
        return false;
    }

    m_aSelectedRows.clear();
    for (sal_Int32 i = 0; i < m_pModel->getRowCount(); ++i)
        m_aSelectedRows.push_back(i);

    return true;
}

void TableControl::commitAccessibleEvent(sal_Int16 const i_eventID)
{
    impl_commitAccessibleEvent(i_eventID, Any());
}

void TableControl::commitCellEvent(sal_Int16 const i_eventID, const Any& i_newValue,
                                   const Any& i_oldValue)
{
    if (m_xAccessibleTable.is())
        m_xAccessibleTable->commitCellEvent(i_eventID, i_newValue, i_oldValue);
}

void TableControl::commitTableEvent(sal_Int16 const i_eventID, const Any& i_newValue,
                                    const Any& i_oldValue)
{
    if (m_xAccessibleTable.is())
        m_xAccessibleTable->commitTableEvent(i_eventID, i_newValue, i_oldValue);
}

bool TableControl::ConvertPointToCellAddress(sal_Int32& rRow, sal_Int32& rColPos,
                                             const Point& rPoint)
{
    rRow = getRowAtPoint(rPoint);
    rColPos = getColAtPoint(rPoint);
    return rRow >= 0;
}

tools::Rectangle TableControl::calcHeaderRect(bool bColHeader)
{
    tools::Rectangle const aRectTableWithHeaders(impl_getAllVisibleCellsArea());
    Size const aSizeTableWithHeaders(aRectTableWithHeaders.GetSize());
    if (bColHeader)
        return tools::Rectangle(aRectTableWithHeaders.TopLeft(),
                                Size(aSizeTableWithHeaders.Width(), m_nColHeaderHeightPixel));
    else
        return tools::Rectangle(aRectTableWithHeaders.TopLeft(),
                                Size(m_nRowHeaderWidthPixel, aSizeTableWithHeaders.Height()));
}

tools::Rectangle TableControl::calcHeaderCellRect(bool bColHeader, sal_Int32 nPos)
{
    tools::Rectangle const aHeaderRect = calcHeaderRect(bColHeader);
    TableCellGeometry const aGeometry(*this, aHeaderRect, bColHeader ? nPos : COL_ROW_HEADERS,
                                      bColHeader ? ROW_COL_HEADERS : nPos);
    return aGeometry.getRect();
}

tools::Rectangle TableControl::calcTableRect() const { return impl_getAllVisibleDataCellArea(); }

tools::Rectangle TableControl::calcCellRect(sal_Int32 nRow, sal_Int32 nCol) const
{
    if (!m_pModel || (nRow == ROW_INVALID) || (nCol == COL_INVALID))
        return tools::Rectangle();

    TableCellGeometry aCell(*this, impl_getAllVisibleCellsArea(), nCol, nRow);
    return aCell.getRect();
}

IMPL_LINK_NOARG(TableControl, OnUpdateScrollbars, void*, void)
{
    // TODO: can't we simply use lcl_updateScrollbar here, so the scrollbars ranges are updated, instead of
    // doing a complete re-layout?
    impl_ni_relayout();
}

IMPL_LINK(TableControl, OnScroll, ScrollBar*, _pScrollbar, void)
{
    DBG_ASSERT((_pScrollbar == m_pVScroll) || (_pScrollbar == m_pHScroll),
               "TableControl::OnScroll: where did this come from?");

    if (_pScrollbar == m_pVScroll)
        impl_ni_ScrollRows(_pScrollbar->GetDelta());
    else
        impl_ni_ScrollColumns(_pScrollbar->GetDelta());
}

IMPL_LINK_NOARG(TableControl, ImplSelectHdl, LinkParamNone*, void) { Select(); }

rtl::Reference<comphelper::OAccessible> TableControl::CreateAccessible()
{
    rtl::Reference<comphelper::OAccessible> pParent = GetAccessibleParent();
    return getAccessible(pParent);
}

const rtl::Reference<accessibility::AccessibleGridControl>&
TableControl::getAccessible(const rtl::Reference<comphelper::OAccessible>& rpParent)
{
    if (m_xAccessibleTable.is())
        return m_xAccessibleTable;

    DBG_TESTSOLARMUTEX();
    if (rpParent.is())
    {
        m_xAccessibleTable = new accessibility::AccessibleGridControl(rpParent, *this);
    }

    return m_xAccessibleTable;
}

void TableControl::disposeAccessible()
{
    if (m_xAccessibleTable.is())
        m_xAccessibleTable->dispose();
    m_xAccessibleTable.clear();
}

sal_Int32 TableControl::GetAccessibleControlCount() const
{
    // TC_TABLE is always defined, no matter whether empty or not
    sal_Int32 count = 1;
    if (m_pModel->hasRowHeaders())
        ++count;
    if (m_pModel->hasColumnHeaders())
        ++count;
    return count;
}

OUString TableControl::GetAccessibleObjectName(AccessibleTableControlObjType eObjType,
                                               sal_Int32 nRow, sal_Int32 nCol) const
{
    OUString sRetText;
    //Window* pWin;
    switch (eObjType)
    {
        case AccessibleTableControlObjType::GRIDCONTROL:
            sRetText = "Grid control";
            break;
        case AccessibleTableControlObjType::TABLE:
            sRetText = "Grid control";
            break;
        case AccessibleTableControlObjType::ROWHEADERBAR:
            sRetText = "RowHeaderBar";
            break;
        case AccessibleTableControlObjType::COLUMNHEADERBAR:
            sRetText = "ColumnHeaderBar";
            break;
        case AccessibleTableControlObjType::TABLECELL:
            //the name of the cell consists of column name and row name if defined
            //if the name is equal to cell content, it'll be read twice
            if (m_pModel->hasColumnHeaders())
            {
                sRetText = GetColumnName(nCol) + " , ";
            }
            if (m_pModel->hasRowHeaders())
            {
                sRetText += GetRowName(nRow) + " , ";
            }
            //aRetText = GetAccessibleCellText(_nRow, _nCol);
            break;
        case AccessibleTableControlObjType::ROWHEADERCELL:
            sRetText = GetRowName(nRow);
            break;
        case AccessibleTableControlObjType::COLUMNHEADERCELL:
            sRetText = GetColumnName(nCol);
            break;
        default:
            OSL_FAIL("GridControl::GetAccessibleName: invalid enum!");
    }
    return sRetText;
}

OUString TableControl::GetAccessibleObjectDescription(AccessibleTableControlObjType eObjType) const
{
    OUString sRetText;
    switch (eObjType)
    {
        case AccessibleTableControlObjType::GRIDCONTROL:
            sRetText = "Grid control description";
            break;
        case AccessibleTableControlObjType::TABLE:
            sRetText = "TABLE description";
            break;
        case AccessibleTableControlObjType::ROWHEADERBAR:
            sRetText = "ROWHEADERBAR description";
            break;
        case AccessibleTableControlObjType::COLUMNHEADERBAR:
            sRetText = "COLUMNHEADERBAR description";
            break;
        case AccessibleTableControlObjType::TABLECELL:
            // the description of the cell consists of column name and row name if defined
            // if the name is equal to cell content, it'll be read twice
            if (m_pModel->hasColumnHeaders())
            {
                sRetText = GetColumnName(GetCurrentColumn()) + " , ";
            }
            if (m_pModel->hasRowHeaders())
            {
                sRetText += GetRowName(GetCurrentRow());
            }
            break;
        case AccessibleTableControlObjType::ROWHEADERCELL:
            sRetText = "ROWHEADERCELL description";
            break;
        case AccessibleTableControlObjType::COLUMNHEADERCELL:
            sRetText = "COLUMNHEADERCELL description";
            break;
    }
    return sRetText;
}

void TableControl::FillAccessibleStateSet(sal_Int64& rStateSet,
                                          AccessibleTableControlObjType eObjType) const
{
    switch (eObjType)
    {
        case AccessibleTableControlObjType::GRIDCONTROL:
        case AccessibleTableControlObjType::TABLE:

            rStateSet |= css::accessibility::AccessibleStateType::FOCUSABLE;

            if (m_pSelEngine->GetSelectionMode() == SelectionMode::Multiple)
                rStateSet |= css::accessibility::AccessibleStateType::MULTI_SELECTABLE;

            if (HasChildPathFocus())
                rStateSet |= css::accessibility::AccessibleStateType::FOCUSED;

            if (IsActive())
                rStateSet |= css::accessibility::AccessibleStateType::ACTIVE;

            if (getDataWindow().IsEnabled())
            {
                rStateSet |= css::accessibility::AccessibleStateType::ENABLED;
                rStateSet |= css::accessibility::AccessibleStateType::SENSITIVE;
            }

            if (IsReallyVisible())
                rStateSet |= css::accessibility::AccessibleStateType::VISIBLE;

            if (eObjType == AccessibleTableControlObjType::TABLE)
                rStateSet |= css::accessibility::AccessibleStateType::MANAGES_DESCENDANTS;
            break;

        case AccessibleTableControlObjType::COLUMNHEADERBAR:
        case AccessibleTableControlObjType::ROWHEADERBAR:
            rStateSet |= css::accessibility::AccessibleStateType::VISIBLE;
            rStateSet |= css::accessibility::AccessibleStateType::MANAGES_DESCENDANTS;
            break;

        case AccessibleTableControlObjType::TABLECELL:
        {
            rStateSet |= css::accessibility::AccessibleStateType::FOCUSABLE;
            if (HasChildPathFocus())
                rStateSet |= css::accessibility::AccessibleStateType::FOCUSED;
            rStateSet |= css::accessibility::AccessibleStateType::ACTIVE;
            rStateSet |= css::accessibility::AccessibleStateType::TRANSIENT;
            rStateSet |= css::accessibility::AccessibleStateType::SELECTABLE;
            rStateSet |= css::accessibility::AccessibleStateType::VISIBLE;
            rStateSet |= css::accessibility::AccessibleStateType::SHOWING;
            if (IsRowSelected(GetCurrentRow()))
                // Hmm? Wouldn't we expect the affected row to be a parameter to this function?
                rStateSet |= css::accessibility::AccessibleStateType::SELECTED;
        }
        break;

        case AccessibleTableControlObjType::ROWHEADERCELL:
            rStateSet |= css::accessibility::AccessibleStateType::VISIBLE;
            rStateSet |= css::accessibility::AccessibleStateType::TRANSIENT;
            break;

        case AccessibleTableControlObjType::COLUMNHEADERCELL:
            rStateSet |= css::accessibility::AccessibleStateType::VISIBLE;
            break;
    }
}

void TableControl::FillAccessibleStateSetForCell(sal_Int64& rStateSet, sal_Int32 nRow,
                                                 sal_uInt16) const
{
    if (IsRowSelected(nRow))
        rStateSet |= css::accessibility::AccessibleStateType::SELECTED;
    if (HasChildPathFocus())
        rStateSet |= css::accessibility::AccessibleStateType::FOCUSED;
    else // only transient when column is not focused
        rStateSet |= css::accessibility::AccessibleStateType::TRANSIENT;

    rStateSet |= css::accessibility::AccessibleStateType::VISIBLE;
    rStateSet |= css::accessibility::AccessibleStateType::SHOWING;
    rStateSet |= css::accessibility::AccessibleStateType::ENABLED;
    rStateSet |= css::accessibility::AccessibleStateType::SENSITIVE;
    rStateSet |= css::accessibility::AccessibleStateType::ACTIVE;
}

void TableControl::impl_commitAccessibleEvent(sal_Int16 const i_eventID, Any const& i_newValue)
{
    if (m_xAccessibleTable.is())
        m_xAccessibleTable->commitEvent(i_eventID, i_newValue, css::uno::Any());
}
} // namespace svt::table

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
