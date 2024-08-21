/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_EFFECT_TEXTEFFECTPROPERTYPANEL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_EFFECT_TEXTEFFECTPROPERTYPANEL_HXX

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>

class ColorListBox;

namespace svx::sidebar
{
class TextEffectPropertyPanel : public PanelLayout,
                                public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    TextEffectPropertyPanel(weld::Widget* pParent, SfxBindings* pBindings);
    virtual ~TextEffectPropertyPanel() override;

    static std::unique_ptr<PanelLayout> Create(weld::Widget* pParent, SfxBindings* pBindings);

    virtual void NotifyItemUpdate(const sal_uInt16 nSId, const SfxItemState eState,
                                  const SfxPoolItem* pState) override;

    virtual void GetControlState(const sal_uInt16 /*nSId*/,
                                 boost::property_tree::ptree& /*rState*/) override{};

private:
    sfx2::sidebar::ControllerItem maTGlowColorController;
    sfx2::sidebar::ControllerItem maTGlowRadiusController;
    sfx2::sidebar::ControllerItem maTGlowTransparencyController;
    std::unique_ptr<weld::Label> mxFTTTransparency;

    SfxBindings* mpBindings;

    std::unique_ptr<weld::MetricSpinButton> mxTGlowRadius;
    std::unique_ptr<ColorListBox> mxLBTGlowColor;
    std::unique_ptr<weld::MetricSpinButton> mxTGlowTransparency;
    std::unique_ptr<weld::Label> mxFTTColor;

    void Initialize();
    void UpdateControls();

    DECL_LINK(ModifyTGlowColorHdl, ColorListBox&, void);
    DECL_LINK(ModifyTGlowRadiusHdl, weld::MetricSpinButton&, void);
    DECL_LINK(ModifyTGlowTransparencyHdl, weld::MetricSpinButton&, void);
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
