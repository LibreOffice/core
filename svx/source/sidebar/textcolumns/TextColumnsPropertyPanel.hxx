/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>

namespace svx::sidebar
{
class TextColumnsPropertyPanel : public PanelLayout,
                                 public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    TextColumnsPropertyPanel(weld::Widget* pParent, SfxBindings* pBindings);
    virtual ~TextColumnsPropertyPanel() override;

    static std::unique_ptr<PanelLayout> Create(weld::Widget* pParent, SfxBindings* pBindings);

    virtual void NotifyItemUpdate(const sal_uInt16 nSId, const SfxItemState eState,
                                  const SfxPoolItem* pState) override;

    virtual void GetControlState(const sal_uInt16 /*nSId*/,
                                 boost::property_tree::ptree& /*rState*/) override{};

private:
    SfxBindings* mpBindings;

    std::unique_ptr<weld::SpinButton> m_xColumnsNumber;
    std::unique_ptr<weld::MetricSpinButton> m_xColumnsSpacing;

    sfx2::sidebar::ControllerItem maColumnsNumberController;
    sfx2::sidebar::ControllerItem maColumnsSpacingController;

    DECL_LINK(ModifyColumnsNumberHdl, weld::SpinButton&, void);
    DECL_LINK(ModifyColumnsSpacingHdl, weld::MetricSpinButton&, void);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
