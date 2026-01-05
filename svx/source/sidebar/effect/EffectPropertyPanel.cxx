/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "EffectPropertyPanel.hxx"

#include <sfx2/dispatch.hxx>
#include <svx/colorbox.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/sdprcitm.hxx>
#include <svx/svddef.hxx>
#include <svx/svxids.hrc>
#include <svx/xcolit.hxx>
#include <svl/itemset.hxx>

namespace svx::sidebar
{
EffectPropertyPanel::EffectPropertyPanel(weld::Widget* pParent, SfxBindings* pBindings)
    : EffectPropertyPanelBase(pParent)
    , mpBindings(pBindings)
    , maGlowColorController(SID_ATTR_GLOW_COLOR, *pBindings, *this)
    , maGlowRadiusController(SID_ATTR_GLOW_RADIUS, *pBindings, *this)
    , maGlowTransparencyController(SID_ATTR_GLOW_TRANSPARENCY, *pBindings, *this)
    , maSoftEdgeRadiusController(SID_ATTR_SOFTEDGE_RADIUS, *pBindings, *this)
{
}

EffectPropertyPanel::~EffectPropertyPanel()
{
    maGlowColorController.dispose();
    maGlowRadiusController.dispose();
    maGlowTransparencyController.dispose();
    maSoftEdgeRadiusController.dispose();
}

void EffectPropertyPanel::setGlowRadius(const SdrMetricItem& rGlowRadius)
{
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_GLOW_RADIUS, SfxCallMode::RECORD,
                                             { &rGlowRadius });
}

void EffectPropertyPanel::setGlowColor(const XColorItem& rGlowColor)
{
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_GLOW_COLOR, SfxCallMode::RECORD,
                                             { &rGlowColor });
}

void EffectPropertyPanel::setGlowTransparency(const SdrPercentItem& rGlowTransparency)
{
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_GLOW_TRANSPARENCY, SfxCallMode::RECORD,
                                             { &rGlowTransparency });
}

void EffectPropertyPanel::setSoftEdgeRadius(const SdrMetricItem& rSoftEdgeRadius)
{
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_SOFTEDGE_RADIUS, SfxCallMode::RECORD,
                                             { &rSoftEdgeRadius });
}

std::unique_ptr<PanelLayout> EffectPropertyPanel::Create(weld::Widget* pParent,
                                                         SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException(
            u"no parent Window given to EffectPropertyPanel::Create"_ustr, nullptr, 0);
    if (pBindings == nullptr)
        throw css::lang::IllegalArgumentException(
            u"no SfxBindings given to EffectPropertyPanel::Create"_ustr, nullptr, 2);

    return std::make_unique<EffectPropertyPanel>(pParent, pBindings);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
