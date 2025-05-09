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
#include <ChartColorPalettes.hxx>

namespace chart
{
class ChartColorPaletteHelper;

namespace sidebar
{
struct IColorPaletteHandler
{
    virtual ~IColorPaletteHandler() = default;

    virtual void createDiagramSnapshot() = 0;
    virtual void restoreOriginalDiagram() = 0;

    virtual void select(ChartColorPaletteType eType, sal_uInt32 nIndex) = 0;
    virtual void apply(const ChartColorPalette* pColorPalette) = 0;
    [[nodiscard]] virtual std::shared_ptr<ChartColorPaletteHelper> getHelper() const = 0;
    [[nodiscard]] virtual ChartColorPaletteType getType() const = 0;
    [[nodiscard]] virtual sal_uInt32 getIndex() const = 0;
};

class ChartColorPaletteControl final : public svt::PopupWindowController
{
    std::shared_ptr<IColorPaletteHandler> mpHandler;
    ChartColorPaletteType meColorPaletteType;
    sal_uInt32 mnColorPaletteIndex;

public:
    explicit ChartColorPaletteControl(
        const css::uno::Reference<css::uno::XComponentContext>& rContext);

    // XInitialization
    void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rArguments) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    void SAL_CALL execute(sal_Int16 nKeyModifier) override;
    void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

    ~ChartColorPaletteControl() override;

    void setColorPaletteHandler(std::shared_ptr<IColorPaletteHandler> rColorPaletteHandler);
    std::shared_ptr<ChartColorPaletteHelper> getColorPaletteHelper() const;
    ChartColorPaletteType getColorPaletteType() const;
    sal_uInt32 getColorPaletteIndex() const;
    void dispatchColorPaletteCommand(ChartColorPaletteType eType, sal_uInt32 nIndex) const;
    void applyColorPalette(const ChartColorPalette* pColorPalette) const;
    void updateStatus(bool bForce = false);

    void createDiagramSnapshot() const;
    void restoreOriginalDiagram() const;

private:
    std::unique_ptr<WeldToolbarPopup> weldPopupWindow() override;
    VclPtr<vcl::Window> createVclPopupWindow(vcl::Window* pParent) override;

    void renderSelectedColorPalette(const VclPtr<VirtualDevice>& pDev) const;
};

class ChartColorPalettePopup final : public WeldToolbarPopup
{
    rtl::Reference<ChartColorPaletteControl> mxControl;
    std::unique_ptr<ChartColorPalettes> mxColorfulValueSet;
    std::unique_ptr<weld::CustomWeld> mxColorfulValueSetWin;
    std::unique_ptr<ChartColorPalettes> mxMonoValueSet;
    std::unique_ptr<weld::CustomWeld> mxMonoValueSetWin;

    ChartColorPaletteType meHighlightedItemType;
    sal_uInt16 mnHighlightedItemId;
    bool mbItemSelected;

    DECL_LINK(SelectColorfulValueSetHdl, ValueSet*, void);
    DECL_LINK(SelectMonoValueSetHdl, ValueSet*, void);
    sal_uInt32 SelectValueSetHdl(const std::unique_ptr<ChartColorPalettes>& xValueSet);

    DECL_LINK(ColorfulMouseMoveHdl, const MouseEvent&, void);
    DECL_LINK(MonoMouseMoveHdl, const MouseEvent&, void);
    void MouseMoveHdl(const std::unique_ptr<ChartColorPalettes>& xValueSet,
                      ChartColorPaletteType eHlItemType);

    void GrabFocus() override;

    void selectItem(ChartColorPaletteType eType, sal_uInt32 nIndex) const;
    void initColorPalettes() const;

public:
    ChartColorPalettePopup(ChartColorPaletteControl* pControl, weld::Widget* pParent);
    ~ChartColorPalettePopup() override;
};
} // end namespace sidebar
} // end namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
