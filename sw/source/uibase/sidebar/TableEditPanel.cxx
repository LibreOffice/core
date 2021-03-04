/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "TableEditPanel.hxx"
#include <sal/config.h>
#include <swtypes.hxx>
#include <cmdid.h>
#include <svl/intitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/dlgctrl.hxx>
#include <swmodule.hxx>
#include <usrpref.hxx>
#include <comphelper/lok.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace sw::sidebar
{
VclPtr<PanelLayout> TableEditPanel::Create(vcl::Window* pParent,
                                           const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                           SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException(
            "no parent Window given to TableEditPanel::Create", nullptr, 0);
    if (!rxFrame.is())
        throw css::lang::IllegalArgumentException("no XFrame given to TableEditPanel::Create",
                                                  nullptr, 1);

    return VclPtr<TableEditPanel>::Create(pParent, rxFrame, pBindings);
}

void TableEditPanel::NotifyItemUpdate(const sal_uInt16 nSID, const SfxItemState eState,
                                      const SfxPoolItem* pState)
{
    switch (nSID)
    {
        case SID_ATTR_TABLE_ROW_HEIGHT:
        {
            bool bDisabled = eState == SfxItemState::DISABLED;
            m_xRowHeightEdit->set_sensitive(!bDisabled);

            if (pState && eState >= SfxItemState::DEFAULT)
            {
                const SfxUInt32Item* pItem = static_cast<const SfxUInt32Item*>(pState);
                if (pItem)
                {
                    tools::Long nNewHeight = pItem->GetValue();
                    nNewHeight = m_xRowHeightEdit->normalize(nNewHeight);
                    m_xRowHeightEdit->set_value(nNewHeight, FieldUnit::TWIP);
                }
            }
            else if (eState != SfxItemState::DISABLED)
                m_xRowHeightEdit->set_text("");

            break;
        }
        case SID_ATTR_TABLE_COLUMN_WIDTH:
        {
            bool bDisabled = eState == SfxItemState::DISABLED;
            m_xColumnWidthEdit->set_sensitive(!bDisabled);

            if (pState && eState >= SfxItemState::DEFAULT)
            {
                const SfxUInt32Item* pItem = static_cast<const SfxUInt32Item*>(pState);
                if (pItem)
                {
                    tools::Long nNewWidth = pItem->GetValue();
                    nNewWidth = m_xColumnWidthEdit->normalize(nNewWidth);
                    m_xColumnWidthEdit->set_value(nNewWidth, FieldUnit::TWIP);
                }
            }
            else if (eState != SfxItemState::DISABLED)
                m_xColumnWidthEdit->set_text("");

            break;
        }
    }
}

TableEditPanel::TableEditPanel(vcl::Window* pParent,
                               const css::uno::Reference<css::frame::XFrame>& rxFrame,
                               SfxBindings* pBindings)
    : PanelLayout(pParent, "TableEditPanel", "modules/swriter/ui/sidebartableedit.ui", rxFrame)
    , m_pBindings(pBindings)
    , m_xRowHeightEdit(
          new SvxRelativeField(m_xBuilder->weld_metric_spin_button("rowheight", FieldUnit::CM)))
    , m_xColumnWidthEdit(
          new SvxRelativeField(m_xBuilder->weld_metric_spin_button("columnwidth", FieldUnit::CM)))
    , m_xInsert(m_xBuilder->weld_toolbar("insert"))
    , m_xInsertDispatch(new ToolbarUnoDispatcher(*m_xInsert, *m_xBuilder, rxFrame))
    , m_xSelect(m_xBuilder->weld_toolbar("select"))
    , m_xSelectDispatch(new ToolbarUnoDispatcher(*m_xSelect, *m_xBuilder, rxFrame))
    , m_xRowSizing(m_xBuilder->weld_toolbar("rowsizing"))
    , m_xRowSizingDispatch(new ToolbarUnoDispatcher(*m_xRowSizing, *m_xBuilder, rxFrame))
    , m_xColumnSizing(m_xBuilder->weld_toolbar("columnsizing"))
    , m_xColumnSizingDispatch(new ToolbarUnoDispatcher(*m_xColumnSizing, *m_xBuilder, rxFrame))
    , m_xDelete(m_xBuilder->weld_toolbar("delete"))
    , m_xDeleteDispatch(new ToolbarUnoDispatcher(*m_xDelete, *m_xBuilder, rxFrame))
    , m_xSplitMerge(m_xBuilder->weld_toolbar("split_merge"))
    , m_xSplitMergeDispatch(new ToolbarUnoDispatcher(*m_xSplitMerge, *m_xBuilder, rxFrame))
    , m_xMisc(m_xBuilder->weld_toolbar("misc"))
    , m_xMiscDispatch(new ToolbarUnoDispatcher(*m_xMisc, *m_xBuilder, rxFrame))
    , m_aRowHeightController(SID_ATTR_TABLE_ROW_HEIGHT, *pBindings, *this)
    , m_aColumnWidthController(SID_ATTR_TABLE_COLUMN_WIDTH, *pBindings, *this)
    , m_aInsertRowsBeforeController(FN_TABLE_INSERT_ROW_BEFORE, *pBindings, *this)
    , m_aInsertRowsAfterController(FN_TABLE_INSERT_ROW_AFTER, *pBindings, *this)
    , m_aInsertColumnsBeforeController(FN_TABLE_INSERT_COL_BEFORE, *pBindings, *this)
    , m_aInsertColumnsAfterController(FN_TABLE_INSERT_COL_AFTER, *pBindings, *this)
    , m_aDeleteRowsController(FN_TABLE_DELETE_ROW, *pBindings, *this)
    , m_aDeleteColumnsController(FN_TABLE_DELETE_COL, *pBindings, *this)
    , m_aDeleteTableController(FN_TABLE_DELETE_TABLE, *pBindings, *this)
    , m_aSetMinimalRowHeightController(SID_TABLE_MINIMAL_ROW_HEIGHT, *pBindings, *this)
    , m_aSetOptimalRowHeightController(FN_TABLE_OPTIMAL_HEIGHT, *pBindings, *this)
    , m_aDistributeRowsController(FN_TABLE_BALANCE_ROWS, *pBindings, *this)
    , m_aSetMinimalColumnWidthController(SID_TABLE_MINIMAL_COLUMN_WIDTH, *pBindings, *this)
    , m_aSetOptimalColumnWidthController(FN_TABLE_ADJUST_CELLS, *pBindings, *this)
    , m_aDistributeColumnsController(FN_TABLE_BALANCE_CELLS, *pBindings, *this)
    , m_aMergeCellsController(FN_TABLE_MERGE_CELLS, *pBindings, *this)
{
    // tdf#130197 Give this toolbar a width as if it had 5 entries (the parent
    // grid has homogeneous width set so both columns will have the same
    // width). This will align this TableEditPanel's columns with
    // ParaPropertyPanel's columns
    padWidthForSidebar(*m_xSplitMerge, rxFrame);

    InitRowHeightToolitem();
    InitColumnWidthToolitem();

    if (comphelper::LibreOfficeKit::isActive())
        m_xMisc->set_item_visible(".uno:InsertFormula", false);

    m_pInitialFocusWidget = &m_xRowHeightEdit->get_widget();
}

TableEditPanel::~TableEditPanel() { disposeOnce(); }

void TableEditPanel::InitRowHeightToolitem()
{
    Link<weld::MetricSpinButton&, void> aLink = LINK(this, TableEditPanel, RowHeightMofiyHdl);
    m_xRowHeightEdit->connect_value_changed(aLink);

    FieldUnit eFieldUnit = SW_MOD()->GetUsrPref(false)->GetMetric();
    m_xRowHeightEdit->SetFieldUnit(eFieldUnit);

    m_xRowHeightEdit->set_min(MINLAY, FieldUnit::TWIP);
    m_xRowHeightEdit->set_max(SAL_MAX_INT32, FieldUnit::TWIP);

    limitWidthForSidebar(*m_xRowHeightEdit);
}

void TableEditPanel::InitColumnWidthToolitem()
{
    Link<weld::MetricSpinButton&, void> aLink = LINK(this, TableEditPanel, ColumnWidthMofiyHdl);
    m_xColumnWidthEdit->connect_value_changed(aLink);

    FieldUnit eFieldUnit = SW_MOD()->GetUsrPref(false)->GetMetric();
    m_xColumnWidthEdit->SetFieldUnit(eFieldUnit);

    m_xColumnWidthEdit->set_min(MINLAY, FieldUnit::TWIP);
    m_xColumnWidthEdit->set_max(SAL_MAX_INT32, FieldUnit::TWIP);

    limitWidthForSidebar(*m_xColumnWidthEdit);
}

void TableEditPanel::dispose()
{
    m_xRowHeightEdit.reset();
    m_xColumnWidthEdit.reset();

    m_xMiscDispatch.reset();
    m_xMisc.reset();

    m_xSplitMergeDispatch.reset();
    m_xSplitMerge.reset();

    m_xDeleteDispatch.reset();
    m_xDelete.reset();

    m_xColumnSizingDispatch.reset();
    m_xColumnSizing.reset();

    m_xRowSizingDispatch.reset();
    m_xRowSizing.reset();

    m_xSelectDispatch.reset();
    m_xSelect.reset();

    m_xInsertDispatch.reset();
    m_xInsert.reset();

    m_aRowHeightController.dispose();
    m_aColumnWidthController.dispose();
    m_aInsertRowsBeforeController.dispose();
    m_aInsertRowsAfterController.dispose();
    m_aInsertColumnsBeforeController.dispose();
    m_aInsertColumnsAfterController.dispose();
    m_aDeleteRowsController.dispose();
    m_aDeleteColumnsController.dispose();
    m_aDeleteTableController.dispose();
    m_aSetMinimalRowHeightController.dispose();
    m_aSetOptimalRowHeightController.dispose();
    m_aDistributeRowsController.dispose();
    m_aSetMinimalColumnWidthController.dispose();
    m_aSetOptimalColumnWidthController.dispose();
    m_aDistributeColumnsController.dispose();
    m_aMergeCellsController.dispose();

    PanelLayout::dispose();
}

IMPL_LINK_NOARG(TableEditPanel, RowHeightMofiyHdl, weld::MetricSpinButton&, void)
{
    SwTwips nNewHeight = static_cast<SwTwips>(
        m_xRowHeightEdit->denormalize(m_xRowHeightEdit->get_value(FieldUnit::TWIP)));
    SfxUInt32Item aRowHeight(SID_ATTR_TABLE_ROW_HEIGHT);
    aRowHeight.SetValue(nNewHeight);

    m_pBindings->GetDispatcher()->ExecuteList(SID_ATTR_TABLE_ROW_HEIGHT, SfxCallMode::RECORD,
                                              { &aRowHeight });
}

IMPL_LINK_NOARG(TableEditPanel, ColumnWidthMofiyHdl, weld::MetricSpinButton&, void)
{
    SwTwips nNewWidth = static_cast<SwTwips>(
        m_xColumnWidthEdit->denormalize(m_xColumnWidthEdit->get_value(FieldUnit::TWIP)));
    SfxUInt32Item aColumnWidth(SID_ATTR_TABLE_COLUMN_WIDTH);
    aColumnWidth.SetValue(nNewWidth);

    m_pBindings->GetDispatcher()->ExecuteList(SID_ATTR_TABLE_COLUMN_WIDTH, SfxCallMode::RECORD,
                                              { &aColumnWidth });
}
} // end of namespace ::sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
