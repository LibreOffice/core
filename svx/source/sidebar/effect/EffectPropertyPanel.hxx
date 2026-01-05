/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svx/sidebar/EffectPropertyPanelBase.hxx>

class ColorListBox;

namespace svx::sidebar
{
class EffectPropertyPanel : public EffectPropertyPanelBase
{
public:
    EffectPropertyPanel(weld::Widget* pParent, SfxBindings* pBindings);
    ~EffectPropertyPanel() override;

    static std::unique_ptr<PanelLayout> Create(weld::Widget* pParent, SfxBindings* pBindings);

    void setGlowRadius(const SdrMetricItem& rGlowRadius) override;
    void setGlowColor(const XColorItem& rGlowColor) override;
    void setGlowTransparency(const SdrPercentItem& rGlowTransparency) override;
    void setSoftEdgeRadius(const SdrMetricItem& rSoftEdgeRadius) override;

private:
    SfxBindings* mpBindings;
    sfx2::sidebar::ControllerItem maGlowColorController;
    sfx2::sidebar::ControllerItem maGlowRadiusController;
    sfx2::sidebar::ControllerItem maGlowTransparencyController;
    sfx2::sidebar::ControllerItem maSoftEdgeRadiusController;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
