/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>
#include "TableEditPanel.hxx"

#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace sw
{
namespace sidebar
{
VclPtr<vcl::Window> TableEditPanel::Create(vcl::Window* pParent,
                                           const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException(
            "no parent Window given to TableEditPanel::Create", nullptr, 0);
    if (!rxFrame.is())
        throw css::lang::IllegalArgumentException("no XFrame given to TableEditPanel::Create",
                                                  nullptr, 1);

    return VclPtr<TableEditPanel>::Create(pParent, rxFrame);
}

void TableEditPanel::NotifyItemUpdate(const sal_uInt16 /*nSId*/, const SfxItemState /*eState*/,
                                      const SfxPoolItem* /*pState*/)
{
}

TableEditPanel::TableEditPanel(vcl::Window* pParent,
                               const css::uno::Reference<css::frame::XFrame>& rxFrame)
    : PanelLayout(pParent, "TableEditPanel", "modules/swriter/ui/sidebartableedit.ui", rxFrame)
{
}

TableEditPanel::~TableEditPanel() { disposeOnce(); }
}
} // end of namespace ::sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */