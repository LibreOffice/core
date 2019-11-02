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
                    nNewHeight = m_pHeightEdit->Normalize(nNewHeight);
                    m_pHeightEdit->SetValue(nNewHeight, FieldUnit::TWIP);
                }
            }
            else if (eState == SfxItemState::DISABLED)
            {
                m_pHeightEdit->Disable();
            }
            else
            {
                m_pHeightEdit->SetEmptyFieldValue();
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
{
    get(m_pHeightEdit, "rowheight");
    InitRowHeightToolitem();
}

TableEditPanel::~TableEditPanel() { disposeOnce(); }

void TableEditPanel::InitRowHeightToolitem()
{
    Link<Edit&, void> aLink = LINK(this, TableEditPanel, RowHeightMofiyHdl);
    m_pHeightEdit->SetModifyHdl(aLink);

    FieldUnit eFieldUnit = SW_MOD()->GetUsrPref(false)->GetMetric();
    SetFieldUnit(*m_pHeightEdit, eFieldUnit);

    m_pHeightEdit->SetMin(MINLAY, FieldUnit::TWIP);
    m_pHeightEdit->SetMax(SAL_MAX_INT32, FieldUnit::TWIP);
}

void TableEditPanel::dispose()
{
    m_pHeightEdit.clear();
    m_aRowHeightController.dispose();

    PanelLayout::dispose();
}

IMPL_LINK_NOARG(TableEditPanel, RowHeightMofiyHdl, Edit&, void)
{
    SwTwips nNewHeight = static_cast<SwTwips>(
        m_pHeightEdit->Denormalize(m_pHeightEdit->GetValue(FieldUnit::TWIP)));
    SfxUInt32Item aRowHeight(SID_ATTR_TABLE_ROW_HEIGHT);
    aRowHeight.SetValue(nNewHeight);

    m_pBindings->GetDispatcher()->ExecuteList(SID_ATTR_TABLE_ROW_HEIGHT, SfxCallMode::RECORD,
                                              { &aRowHeight });
}
}
} // end of namespace ::sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */