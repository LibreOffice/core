/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_TABLEEDITPANEL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_TABLEEDITPANEL_HXX

#include <com/sun/star/frame/XFrame.hpp>
#include <svx/sidebar/PanelLayout.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>

namespace sw
{
namespace sidebar
{
class TableEditPanel : public PanelLayout,
                       public sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
    friend class VclPtr<TableEditPanel>;

public:
    static VclPtr<vcl::Window> Create(vcl::Window* pParent,
                                      const css::uno::Reference<css::frame::XFrame>& rxFrame);

    virtual void NotifyItemUpdate(const sal_uInt16 nSId, const SfxItemState eState,
                                  const SfxPoolItem* pState) override;

private:
    TableEditPanel(vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rxFrame);
    virtual ~TableEditPanel() override;
};
}
} // end of namespace sw::sidebar

#endif // INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_TABLEEDITPANEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */