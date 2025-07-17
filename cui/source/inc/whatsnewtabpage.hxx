/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/customweld.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/graph.hxx>

class BrandGraphic : public weld::CustomWidgetController
{
private:
    bool m_bIsFirstStart;
    Graphic m_aGraphic;
    Size m_aGraphicSize;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;

public:
    BrandGraphic();
    const Size& GetGraphicSize() const { return m_aGraphicSize; };
    void SetIsFirstStart(const bool bIsFirstStart) { m_bIsFirstStart = bIsFirstStart; };
};

class WhatsNewTabPage : public SfxTabPage
{
private:
    BrandGraphic m_aBrand;
    std::unique_ptr<weld::CustomWeld> m_pBrand;

    virtual void ActivatePage(const SfxItemSet&) override;
    virtual void Reset(const SfxItemSet* rSet) override;

public:
    WhatsNewTabPage(weld::Container* pPage, weld::DialogController* pController,
                    const SfxItemSet& rSet);
    static std::unique_ptr<SfxTabPage>
    Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
