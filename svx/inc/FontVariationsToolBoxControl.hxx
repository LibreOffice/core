/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svtools/ctrltool.hxx>
#include <svtools/popupwindowcontroller.hxx>
#include <vcl/font/Variation.hxx>

namespace svx
{
/// Drops down the variation axes of the current font, for a toolbar or the sidebar.
class FontVariationsToolBoxControl final : public svt::PopupWindowController
{
public:
    FontVariationsToolBoxControl(const css::uno::Reference<css::uno::XComponentContext>& rContext);
    virtual ~FontVariationsToolBoxControl() override;

    virtual std::unique_ptr<WeldToolbarPopup> weldPopupWindow() override;
    virtual VclPtr<vcl::Window> createVclPopupWindow(vcl::Window* pParent) override;

    // XToolbarController
    virtual void SAL_CALL execute(sal_Int16 nKeyModifier) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rArguments) override;

    const OUString& GetFontName() const { return m_aFontName; }
    const std::vector<vcl::font::VariationAxis>& GetAxes();
    std::vector<vcl::font::Variation> GetVariations();

    void Apply(const std::vector<vcl::font::Variation>& rVariations);

private:
    const FontList* GetFontList();
    void UpdateSensitivity();

    OUString m_aFontName;
    OUString m_aStyleName;
    OUString m_aAxesFontName;
    std::vector<vcl::font::VariationAxis> m_aAxes;
    std::vector<vcl::font::Variation> m_aVariations;
    bool m_bSupported = false;
    const FontList* m_pFontList = nullptr;
    std::unique_ptr<FontList> m_xOwnFontList;
};

} // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
