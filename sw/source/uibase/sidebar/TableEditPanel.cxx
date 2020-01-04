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
#include <svtools/unitconv.hxx>
#include <swmodule.hxx>
#include <usrpref.hxx>
#include <svx/svxids.hrc>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace sw
{
namespace sidebar
{
VclPtr<vcl::Window> TableEditPanel::Create(vcl::Window* pParent,
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
            if (pState && eState >= SfxItemState::DEFAULT)
            {
                const SfxUInt32Item* pItem = static_cast<const SfxUInt32Item*>(pState);
                if (pItem)
                {
                    long nNewHeight = pItem->GetValue();
                    nNewHeight = m_pRowHeightEdit->Normalize(nNewHeight);
                    m_pRowHeightEdit->SetValue(nNewHeight, FieldUnit::TWIP);
                }
            }
            else if (eState == SfxItemState::DISABLED)
            {
                m_pRowHeightEdit->Disable();
            }
            else
            {
                m_pRowHeightEdit->SetEmptyFieldValue();
            }
            break;
        }
        case SID_ATTR_TABLE_COLUMN_WIDTH:
        {
            if (pState && eState >= SfxItemState::DEFAULT)
            {
                const SfxUInt32Item* pItem = static_cast<const SfxUInt32Item*>(pState);
                if (pItem)
                {
                    long nNewWidth = pItem->GetValue();
                    nNewWidth = m_pColumnWidthEdit->Normalize(nNewWidth);
                    m_pColumnWidthEdit->SetValue(nNewWidth, FieldUnit::TWIP);
                }
            }
            else if (eState == SfxItemState::DISABLED)
            {
                m_pColumnWidthEdit->Disable();
            }
            else
            {
                m_pColumnWidthEdit->SetEmptyFieldValue();
            }
            break;
        }
    }
}

TableEditPanel::TableEditPanel(vcl::Window* pParent,
                               const css::uno::Reference<css::frame::XFrame>& rxFrame,
                               SfxBindings* pBindings)
    : PanelLayout(pParent, "TableEditPanel", "modules/swriter/ui/sidebartableedit.ui", rxFrame)
    , m_pBindings(pBindings)
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
    get(m_pRowHeightEdit, "rowheight");
    get(m_pColumnWidthEdit, "columnwidth");

    InitRowHeightToolitem();
    InitColumnWidthToolitem();
}

TableEditPanel::~TableEditPanel() { disposeOnce(); }

void TableEditPanel::InitRowHeightToolitem()
{
    Link<Edit&, void> aLink = LINK(this, TableEditPanel, RowHeightMofiyHdl);
    m_pRowHeightEdit->SetModifyHdl(aLink);

    FieldUnit eFieldUnit = SW_MOD()->GetUsrPref(false)->GetMetric();
    SetFieldUnit(*m_pRowHeightEdit, eFieldUnit);

    m_pRowHeightEdit->SetMin(MINLAY, FieldUnit::TWIP);
    m_pRowHeightEdit->SetMax(SAL_MAX_INT32, FieldUnit::TWIP);
}

void TableEditPanel::InitColumnWidthToolitem()
{
    Link<Edit&, void> aLink = LINK(this, TableEditPanel, ColumnWidthMofiyHdl);
    m_pColumnWidthEdit->SetModifyHdl(aLink);

    FieldUnit eFieldUnit = SW_MOD()->GetUsrPref(false)->GetMetric();
    SetFieldUnit(*m_pColumnWidthEdit, eFieldUnit);

    m_pColumnWidthEdit->SetMin(MINLAY, FieldUnit::TWIP);
    m_pColumnWidthEdit->SetMax(SAL_MAX_INT32, FieldUnit::TWIP);
}

void TableEditPanel::dispose()
{
    m_pRowHeightEdit.clear();
    m_pColumnWidthEdit.clear();
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

IMPL_LINK_NOARG(TableEditPanel, RowHeightMofiyHdl, Edit&, void)
{
    SwTwips nNewHeight = static_cast<SwTwips>(
        m_pRowHeightEdit->Denormalize(m_pRowHeightEdit->GetValue(FieldUnit::TWIP)));
    SfxUInt32Item aRowHeight(SID_ATTR_TABLE_ROW_HEIGHT);
    aRowHeight.SetValue(nNewHeight);

    m_pBindings->GetDispatcher()->ExecuteList(SID_ATTR_TABLE_ROW_HEIGHT, SfxCallMode::RECORD,
                                              { &aRowHeight });
}

IMPL_LINK_NOARG(TableEditPanel, ColumnWidthMofiyHdl, Edit&, void)
{
    SwTwips nNewWidth = static_cast<SwTwips>(
        m_pColumnWidthEdit->Denormalize(m_pColumnWidthEdit->GetValue(FieldUnit::TWIP)));
    SfxUInt32Item aColumnWidth(SID_ATTR_TABLE_COLUMN_WIDTH);
    aColumnWidth.SetValue(nNewWidth);

    m_pBindings->GetDispatcher()->ExecuteList(SID_ATTR_TABLE_COLUMN_WIDTH, SfxCallMode::RECORD,
                                              { &aColumnWidth });
}
}
} // end of namespace ::sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */