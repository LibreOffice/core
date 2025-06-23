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

#include <svtools/popupwindowcontroller.hxx>
#include <svtools/toolbarmenu.hxx>

namespace chart
{
class ChartColorPaletteHelper;

namespace sidebar
{
struct IThemeHandler
{
    virtual ~IThemeHandler() = default;
    virtual void select(sal_uInt32 nIndex) = 0;
    [[nodiscard]] virtual VclPtr<VirtualDevice> makePictureFromThemedChart(sal_uInt32 nIndex) = 0;
};

class ChartThemeControl final : public svt::PopupWindowController
{
public:
    std::shared_ptr<IThemeHandler> mpHandler;

public:
    explicit ChartThemeControl(const css::uno::Reference<css::uno::XComponentContext>& rContext);

    // XInitialization
    void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rArguments) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    void SAL_CALL execute(sal_Int16 nKeyModifier) override;
    void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

    ~ChartThemeControl() override;

    void setThemeHandler(std::shared_ptr<IThemeHandler> rThemeHandler);

    void updateStatus(bool bForce = false);

private:
    std::unique_ptr<WeldToolbarPopup> weldPopupWindow() override;
    VclPtr<vcl::Window> createVclPopupWindow(vcl::Window* pParent) override;
};

class ChartThemePopup final : public WeldToolbarPopup
{
    rtl::Reference<ChartThemeControl> mxControl;
    std::unique_ptr<weld::IconView> mxThemesIconView;

    DECL_LINK(ThemeSelectedHdl, weld::IconView&, bool);

    void GrabFocus() override;

    VclPtr<VirtualDevice> makeImage(int nIndex);

public:
    ChartThemePopup(ChartThemeControl* pControl, weld::Widget* pParent);
    ~ChartThemePopup() override;
};

} // end namespace sidebar
} // end namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
