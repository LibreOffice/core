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
#include <strings.hrc>
#include <svl/intitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/dlgctrl.hxx>
#include <swmodule.hxx>
#include <usrpref.hxx>
#include <comphelper/lok.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>

using namespace ::com::sun::star;
namespace sw::sidebar
{
std::unique_ptr<PanelLayout>
TableEditPanel::Create(weld::Widget* pParent,
                       const css::uno::Reference<css::frame::XFrame>& rxFrame,
                       SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException(
            u"no parent Window given to TableEditPanel::Create"_ustr, nullptr, 0);
    if (!rxFrame.is())
        throw css::lang::IllegalArgumentException(u"no XFrame given to TableEditPanel::Create"_ustr,
                                                  nullptr, 1);

    return std::make_unique<TableEditPanel>(pParent, rxFrame, pBindings);
}

void TableEditPanel::NotifyItemUpdate(const sal_uInt16 nSID, const SfxItemState eState,
                                      const SfxPoolItem* pState)
{
    switch (nSID)
    {
        case SID_ATTR_TABLE_ROW_HEIGHT:
        {
            bool bDisabled = eState == SfxItemState::DISABLED;
            m_aRowHeightEdit.set_sensitive(!bDisabled);

            if (pState && eState >= SfxItemState::DEFAULT)
            {
                const SfxUInt32Item* pItem = static_cast<const SfxUInt32Item*>(pState);
                if (pItem)
                {
                    tools::Long nNewHeight = pItem->GetValue();
                    nNewHeight = m_aRowHeightEdit.normalize(nNewHeight);
                    m_aRowHeightEdit.set_value(nNewHeight, FieldUnit::TWIP);
                }
            }
            else if (eState != SfxItemState::DISABLED)
                m_aRowHeightEdit.set_text(u""_ustr);

            break;
        }
        case SID_ATTR_TABLE_COLUMN_WIDTH:
        {
            bool bDisabled = eState == SfxItemState::DISABLED;
            m_aColumnWidthEdit.set_sensitive(!bDisabled);

            if (pState && eState >= SfxItemState::DEFAULT)
            {
                const SfxUInt32Item* pItem = static_cast<const SfxUInt32Item*>(pState);
                if (pItem)
                {
                    tools::Long nNewWidth = pItem->GetValue();
                    nNewWidth = m_aColumnWidthEdit.normalize(nNewWidth);
                    m_aColumnWidthEdit.set_value(nNewWidth, FieldUnit::TWIP);
                }
            }
            else if (eState != SfxItemState::DISABLED)
                m_aColumnWidthEdit.set_text(u""_ustr);

            break;
        }
        case SID_ATTR_TABLE_ALIGNMENT:
            if (pState && eState >= SfxItemState::DEFAULT)
            {
                const SfxUInt16Item* pItem = static_cast<const SfxUInt16Item*>(pState);
                if (pItem)
                {
                    sal_uInt16 nAlignment = pItem->GetValue();
                    m_xAlignment->set_active_id(OUString::number(nAlignment));
                    EnableLeftRight(nAlignment);
                }
            }
            break;
        case SID_ATTR_TABLE_LEFT_SPACE:
        case SID_ATTR_TABLE_RIGHT_SPACE:
        {
            if (pState && eState >= SfxItemState::DEFAULT)
            {
                const SfxInt32Item* pItem = static_cast<const SfxInt32Item*>(pState);
                if (pItem)
                {
                    if (SID_ATTR_TABLE_LEFT_SPACE == nSID)
                    {
                        m_aLeftSpacingEdit.set_value(
                            m_aLeftSpacingEdit.normalize(pItem->GetValue()), FieldUnit::TWIP);
                        m_aLeftSpacingEdit.save_value();
                    }
                    else
                    {
                        m_aRightSpacingEdit.set_value(
                            m_aRightSpacingEdit.normalize(pItem->GetValue()), FieldUnit::TWIP);
                        m_aRightSpacingEdit.save_value();
                    }
                }
            }
            break;
        }
    }
}

TableEditPanel::TableEditPanel(weld::Widget* pParent,
                               const css::uno::Reference<css::frame::XFrame>& rxFrame,
                               SfxBindings* pBindings)
    : PanelLayout(pParent, u"TableEditPanel"_ustr, u"modules/swriter/ui/sidebartableedit.ui"_ustr)
    , m_pBindings(pBindings)
    , m_aRowHeightEdit(m_xBuilder->weld_metric_spin_button(u"rowheight"_ustr, FieldUnit::CM))
    , m_aColumnWidthEdit(m_xBuilder->weld_metric_spin_button(u"columnwidth"_ustr, FieldUnit::CM))
    , m_xInsert(m_xBuilder->weld_toolbar(u"insert"_ustr))
    , m_xInsertDispatch(new ToolbarUnoDispatcher(*m_xInsert, *m_xBuilder, rxFrame))
    , m_xSelect(m_xBuilder->weld_toolbar(u"select"_ustr))
    , m_xSelectDispatch(new ToolbarUnoDispatcher(*m_xSelect, *m_xBuilder, rxFrame))
    , m_xRowSizing(m_xBuilder->weld_toolbar(u"rowsizing"_ustr))
    , m_xRowSizingDispatch(new ToolbarUnoDispatcher(*m_xRowSizing, *m_xBuilder, rxFrame))
    , m_xColumnSizing(m_xBuilder->weld_toolbar(u"columnsizing"_ustr))
    , m_xColumnSizingDispatch(new ToolbarUnoDispatcher(*m_xColumnSizing, *m_xBuilder, rxFrame))
    , m_xDelete(m_xBuilder->weld_toolbar(u"delete"_ustr))
    , m_xDeleteDispatch(new ToolbarUnoDispatcher(*m_xDelete, *m_xBuilder, rxFrame))
    , m_xSplitMerge(m_xBuilder->weld_toolbar(u"split_merge"_ustr))
    , m_xSplitMergeDispatch(new ToolbarUnoDispatcher(*m_xSplitMerge, *m_xBuilder, rxFrame))
    , m_xAlignment(m_xBuilder->weld_combo_box(u"alignmentLB"_ustr))
    , m_aLeftSpacingEdit(m_xBuilder->weld_metric_spin_button(u"leftspace"_ustr, FieldUnit::CM))
    , m_aRightSpacingEdit(m_xBuilder->weld_metric_spin_button(u"rightspace"_ustr, FieldUnit::CM))
    , m_xMisc(m_xBuilder->weld_toolbar(u"misc"_ustr))
    , m_xMiscDispatch(new ToolbarUnoDispatcher(*m_xMisc, *m_xBuilder, rxFrame))
    , m_aRowHeightController(SID_ATTR_TABLE_ROW_HEIGHT, *pBindings, *this)
    , m_aColumnWidthController(SID_ATTR_TABLE_COLUMN_WIDTH, *pBindings, *this)
    , m_aAlignmentController(SID_ATTR_TABLE_ALIGNMENT, *pBindings, *this)
    , m_aLeftSpacingController(SID_ATTR_TABLE_LEFT_SPACE, *pBindings, *this)
    , m_aRightSpacingController(SID_ATTR_TABLE_RIGHT_SPACE, *pBindings, *this)
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
    InitAlignmentControls();
    if (comphelper::LibreOfficeKit::isActive())
        m_xMisc->set_item_visible(u".uno:InsertFormula"_ustr, false);
}

void TableEditPanel::InitRowHeightToolitem()
{
    Link<weld::MetricSpinButton&, void> aLink = LINK(this, TableEditPanel, RowHeightMofiyHdl);
    m_aRowHeightEdit.connect_value_changed(aLink);

    FieldUnit eFieldUnit = SW_MOD()->GetUsrPref(false)->GetMetric();
    m_aRowHeightEdit.SetFieldUnit(eFieldUnit);

    m_aRowHeightEdit.set_min(MINLAY, FieldUnit::TWIP);
    m_aRowHeightEdit.set_max(SAL_MAX_INT32, FieldUnit::TWIP);

    limitWidthForSidebar(m_aRowHeightEdit);
}

void TableEditPanel::InitColumnWidthToolitem()
{
    Link<weld::MetricSpinButton&, void> aLink = LINK(this, TableEditPanel, ColumnWidthMofiyHdl);
    m_aColumnWidthEdit.connect_value_changed(aLink);

    FieldUnit eFieldUnit = SW_MOD()->GetUsrPref(false)->GetMetric();
    m_aColumnWidthEdit.SetFieldUnit(eFieldUnit);

    m_aColumnWidthEdit.set_min(MINLAY, FieldUnit::TWIP);
    m_aColumnWidthEdit.set_max(SAL_MAX_INT32, FieldUnit::TWIP);

    limitWidthForSidebar(m_aColumnWidthEdit);
}

void TableEditPanel::InitAlignmentControls()
{
    m_xAlignment->connect_changed(LINK(this, TableEditPanel, AlignmentHdl));
    m_aLeftSpacingEdit.connect_value_changed(LINK(this, TableEditPanel, SpacingHdl));
    m_aRightSpacingEdit.connect_value_changed(LINK(this, TableEditPanel, SpacingHdl));
    m_xAlignment->append(OUString::number(text::HoriOrientation::FULL),
                         SwResId(STR_TABLE_PANEL_ALIGN_AUTO));
    m_xAlignment->append(OUString::number(text::HoriOrientation::LEFT),
                         SwResId(STR_TABLE_PANEL_ALIGN_LEFT));
    m_xAlignment->append(OUString::number(text::HoriOrientation::LEFT_AND_WIDTH),
                         SwResId(STR_TABLE_PANEL_ALIGN_FROM_LEFT));
    m_xAlignment->append(OUString::number(text::HoriOrientation::RIGHT),
                         SwResId(STR_TABLE_PANEL_ALIGN_RIGHT));
    m_xAlignment->append(OUString::number(text::HoriOrientation::CENTER),
                         SwResId(STR_TABLE_PANEL_ALIGN_CENTER));
    m_xAlignment->append(OUString::number(text::HoriOrientation::NONE),
                         SwResId(STR_TABLE_PANEL_ALIGN_MANUAL));

    FieldUnit eFieldUnit = SW_MOD()->GetUsrPref(false)->GetMetric();
    m_aLeftSpacingEdit.SetFieldUnit(eFieldUnit);
    m_aRightSpacingEdit.SetFieldUnit(eFieldUnit);
}

TableEditPanel::~TableEditPanel()
{
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
}

void TableEditPanel::EnableLeftRight(sal_uInt16 nAlignment)
{
    bool enableLeft = true;
    bool enableRight = true;
    switch (nAlignment)
    {
        case text::HoriOrientation::FULL:
            enableLeft = false;
            enableRight = false;
            break;
        case text::HoriOrientation::LEFT:
            enableLeft = false;
            break;
        case text::HoriOrientation::CENTER:
        case text::HoriOrientation::RIGHT:
            enableRight = false;
            break;
        case text::HoriOrientation::LEFT_AND_WIDTH:
            enableRight = false;
            break;
        default:
            break;
    }
    m_aLeftSpacingEdit.set_sensitive(enableLeft);
    m_aRightSpacingEdit.set_sensitive(enableRight);
}

IMPL_LINK_NOARG(TableEditPanel, RowHeightMofiyHdl, weld::MetricSpinButton&, void)
{
    SwTwips nNewHeight = static_cast<SwTwips>(
        m_aRowHeightEdit.denormalize(m_aRowHeightEdit.get_value(FieldUnit::TWIP)));
    SfxUInt32Item aRowHeight(SID_ATTR_TABLE_ROW_HEIGHT);
    aRowHeight.SetValue(nNewHeight);

    m_pBindings->GetDispatcher()->ExecuteList(SID_ATTR_TABLE_ROW_HEIGHT, SfxCallMode::RECORD,
                                              { &aRowHeight });
}

IMPL_LINK_NOARG(TableEditPanel, ColumnWidthMofiyHdl, weld::MetricSpinButton&, void)
{
    SwTwips nNewWidth = static_cast<SwTwips>(
        m_aColumnWidthEdit.denormalize(m_aColumnWidthEdit.get_value(FieldUnit::TWIP)));
    SfxUInt32Item aColumnWidth(SID_ATTR_TABLE_COLUMN_WIDTH);
    aColumnWidth.SetValue(nNewWidth);

    m_pBindings->GetDispatcher()->ExecuteList(SID_ATTR_TABLE_COLUMN_WIDTH, SfxCallMode::RECORD,
                                              { &aColumnWidth });
}

IMPL_LINK_NOARG(TableEditPanel, AlignmentHdl, weld::ComboBox&, void) { AlignmentModify(true); }

IMPL_LINK(TableEditPanel, SpacingHdl, weld::MetricSpinButton&, rField, void)
{
    if (!rField.get_value_changed_from_saved())
        return;
    rField.save_value();
    AlignmentModify(false);
}

void TableEditPanel::AlignmentModify(bool alignmentChanged)
{
    sal_uInt16 nAlign = m_xAlignment->get_active_id().toUInt32();
    SwTwips nLeft = static_cast<SwTwips>(
        m_aLeftSpacingEdit.denormalize(m_aLeftSpacingEdit.get_value(FieldUnit::TWIP)));
    SwTwips nRight = static_cast<SwTwips>(
        m_aRightSpacingEdit.denormalize(m_aRightSpacingEdit.get_value(FieldUnit::TWIP)));
    if (alignmentChanged)
    {
        EnableLeftRight(nAlign);
        switch (nAlign)
        {
            case text::HoriOrientation::CENTER:
                nLeft = nRight = (nLeft + nRight) / 2;
                break;
            case text::HoriOrientation::LEFT:
                nRight = nLeft + nRight;
                nLeft = 0;
                break;
            case text::HoriOrientation::RIGHT:
                nLeft = nLeft + nRight;
                nRight = 0;
                break;
            default:
                if (!m_aLeftSpacingEdit.get_sensitive())
                    nLeft = 0;
                if (!m_aRightSpacingEdit.get_sensitive())
                    nRight = 0;
                break;
        }
    }

    SfxUInt16Item aAlign(SID_ATTR_TABLE_ALIGNMENT, nAlign);
    SfxInt32Item aLeft(SID_ATTR_TABLE_LEFT_SPACE, nLeft);
    SfxInt32Item aRight(SID_ATTR_TABLE_RIGHT_SPACE, nRight);
    m_pBindings->GetDispatcher()->ExecuteList(SID_ATTR_TABLE_ALIGNMENT, SfxCallMode::RECORD,
                                              { &aAlign, &aLeft, &aRight });
}

} // end of namespace ::sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
