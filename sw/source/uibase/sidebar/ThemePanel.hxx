/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <com/sun/star/frame/XFrame.hpp>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <svtools/valueset.hxx>
#include <svx/ColorSets.hxx>

namespace sw::sidebar {

class ThemePanel : public PanelLayout,
                       public sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
    friend class VclPtr<ThemePanel>;
public:
    static VclPtr<PanelLayout> Create(vcl::Window* pParent,
                                  const css::uno::Reference<css::frame::XFrame>& rxFrame);

    virtual void NotifyItemUpdate(const sal_uInt16 nSId,
                                  const SfxItemState eState,
                                  const SfxPoolItem* pState) override;

    virtual void GetControlState(
        const sal_uInt16 /*nSId*/,
        boost::property_tree::ptree& /*rState*/) override {};

private:
    ThemePanel(vcl::Window* pParent,
                   const css::uno::Reference<css::frame::XFrame>& rxFrame);
    virtual ~ThemePanel() override;

    virtual void dispose() override;

    std::unique_ptr<weld::TreeView> mxListBoxFonts;
    std::unique_ptr<ValueSet> mxValueSetColors;
    std::unique_ptr<weld::CustomWeld> mxValueSetColorsWin;
    std::unique_ptr<weld::Button> mxApplyButton;

    svx::ColorSets maColorSets;

    DECL_LINK(ClickHdl, weld::Button&, void);
    DECL_LINK(DoubleClickHdl, weld::TreeView&, bool);
    DECL_LINK(DoubleClickValueSetHdl, ValueSet*, void);
    void DoubleClickHdl();

};

} // end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
