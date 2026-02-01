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
#include <ChartGradientPalettes.hxx>

namespace chart
{
class ChartGradientPaletteHelper;

namespace sidebar
{
struct IGradientPaletteHandler
{
    virtual ~IGradientPaletteHandler() = default;

    virtual void createDiagramSnapshot() = 0;
    virtual void restoreOriginalDiagram() = 0;

    virtual void select(ChartGradientVariation eVariant, ChartGradientType eType) = 0;
    virtual void apply(const ChartGradientPalette& rGradientPalette) = 0;
    virtual void setPreview(bool bFlag) = 0;
    [[nodiscard]] virtual std::shared_ptr<ChartGradientPaletteHelper> getHelper() const = 0;
    [[nodiscard]] virtual ChartGradientVariation getVariation() const = 0;
    [[nodiscard]] virtual ChartGradientType getType() const = 0;
};

class ChartGradientPaletteControl final : public svt::PopupWindowController
{
    std::shared_ptr<IGradientPaletteHandler> mpHandler;
    ChartGradientVariation meGradientVariation;
    ChartGradientType mnGradientType;

public:
    explicit ChartGradientPaletteControl(
        const css::uno::Reference<css::uno::XComponentContext>& rContext);

    // XInitialization
    void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rArguments) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    void SAL_CALL execute(sal_Int16 nKeyModifier) override;
    void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

    ~ChartGradientPaletteControl() override;

    void
    setGradientPaletteHandler(std::shared_ptr<IGradientPaletteHandler> rGradientPaletteHandler);
    std::shared_ptr<ChartGradientPaletteHelper> getGradientPaletteHelper() const;
    ChartGradientVariation getGradientVariation() const;
    ChartGradientType getGradientType() const;
    void dispatchGradientPaletteCommand(ChartGradientVariation eVariation,
                                        ChartGradientType eType) const;
    void applyGradientPalette(ChartGradientVariation eVariation, ChartGradientType eType,
                              bool bIsPreview = false) const;
    void updateStatus(bool bForce = false);

    void createDiagramSnapshot() const;
    void restoreOriginalDiagram() const;

private:
    std::unique_ptr<WeldToolbarPopup> weldPopupWindow() override;
    VclPtr<vcl::Window> createVclPopupWindow(vcl::Window* pParent) override;

    void renderSelectedGradientPalette(const VclPtr<VirtualDevice>& pDev) const;
};

class ChartGradientPalettePopup final : public WeldToolbarPopup
{
    rtl::Reference<ChartGradientPaletteControl> mxControl;
    std::unique_ptr<ChartGradientPalettes> mxLightPalettes;
    std::unique_ptr<ChartGradientPalettes> mxDarkPalettes;

    ChartGradientVariation meHighlightedItemVariation;
    ChartGradientType mnHighlightedItemType;
    bool mbItemSelected;

    DECL_LINK(SelectLightPaletteHdl, weld::IconView&, bool);
    DECL_LINK(SelectDarkPaletteHdl, weld::IconView&, bool);
    sal_uInt32 GetSelectedItem(const std::unique_ptr<ChartGradientPalettes>& xPalettes);

    DECL_LINK(LightPaletteMouseMoveHdl, const MouseEvent&, bool);
    DECL_LINK(DarkPaletteMouseMoveHdl, const MouseEvent&, bool);
    void MouseMoveHdl(const std::unique_ptr<ChartGradientPalettes>& xPalettes,
                      ChartGradientVariation eHlItemVariation);

    void GrabFocus() override;

    void selectItem(ChartGradientVariation eVariation, ChartGradientType eType) const;
    void initGradientPalettes() const;

public:
    ChartGradientPalettePopup(ChartGradientPaletteControl* pControl, weld::Widget* pParent);
    ~ChartGradientPalettePopup() override;
};
} // end namespace sidebar
} // end namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
