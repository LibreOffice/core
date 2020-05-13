/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_SOFTEDGE_SOFTEDGEPROPERTYPANEL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_SOFTEDGE_SOFTEDGEPROPERTYPANEL_HXX

#include <vcl/vclptr.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>

class ColorListBox;

namespace svx::sidebar
{
class SoftEdgePropertyPanel : public PanelLayout,
                              public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    SoftEdgePropertyPanel(vcl::Window* pParent,
                          const css::uno::Reference<css::frame::XFrame>& rxFrame,
                          SfxBindings* pBindings);
    virtual ~SoftEdgePropertyPanel() override;
    virtual void dispose() override;

    static VclPtr<vcl::Window> Create(vcl::Window* pParent,
                                      const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                      SfxBindings* pBindings);

    virtual void NotifyItemUpdate(const sal_uInt16 nSId, const SfxItemState eState,
                                  const SfxPoolItem* pState) override;

    virtual void GetControlState(const sal_uInt16 /*nSId*/,
                                 boost::property_tree::ptree& /*rState*/) override{};

private:
    sfx2::sidebar::ControllerItem maSoftEdgeRadiusController;

    SfxBindings* mpBindings;

    std::unique_ptr<weld::MetricSpinButton> mxSoftEdgeRadius;
    std::unique_ptr<weld::Label> mxFTRadius;

    void Initialize();

    DECL_LINK(ModifySoftEdgeRadiusHdl, weld::MetricSpinButton&, void);
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
