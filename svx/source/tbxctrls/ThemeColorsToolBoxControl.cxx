/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/ThemeColorsToolBoxControl.hxx>
#include <vcl/weld/IconView.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/propertyvalue.hxx>
#include <vcl/weld/Builder.hxx>
#include <svx/dialog/ThemeColorsPaneBase.hxx>

ThemeColorsToolBoxControl::ThemeColorsToolBoxControl() {}

ThemeColorsToolBoxControl::~ThemeColorsToolBoxControl() {}

void SAL_CALL ThemeColorsToolBoxControl::dispose()
{
    SolarMutexGuard aSolarMutexGuard;
    m_xVclBox.disposeAndClear();
    svt::ToolboxController::dispose();
}

void SAL_CALL
ThemeColorsToolBoxControl::initialize(const css::uno::Sequence<css::uno::Any>& rArguments)
{
    svt::ToolboxController::initialize(rArguments);

    // Register to listen for AddTheme command changes
    addStatusListener(".uno:AddTheme");
}

void SAL_CALL ThemeColorsToolBoxControl::statusChanged(const css::frame::FeatureStateEvent& rEvent)
{
    SolarMutexGuard aSolarMutexGuard;

    if (m_xVclBox && rEvent.FeatureURL.Complete == ".uno:AddTheme")
        m_xVclBox->refreshThemeColors();

    svt::ToolboxController::statusChanged(rEvent);
}

css::uno::Reference<css::awt::XWindow>
ThemeColorsToolBoxControl::createItemWindow(const css::uno::Reference<css::awt::XWindow>& rParent)
{
    css::uno::Reference<css::awt::XWindow> xItemWindow;

    VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow(rParent);
    if (pParent)
    {
        SolarMutexGuard aSolarMutexGuard;

        m_xVclBox = VclPtr<ThemeColorsPaneWrapper>::Create(pParent, m_xFrame);
        xItemWindow = VCLUnoHelper::GetInterface(m_xVclBox.get());
    }
    return xItemWindow;
}

void SAL_CALL ThemeColorsToolBoxControl::update() {}

OUString SAL_CALL ThemeColorsToolBoxControl::getImplementationName()
{
    return u"com.sun.star.comp.svx.ThemeColorsToolBoxControl"_ustr;
}

sal_Bool SAL_CALL ThemeColorsToolBoxControl::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL ThemeColorsToolBoxControl::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.ToolbarController"_ustr };
}

// Export function for service registration
extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_svx_ThemeColorsToolBoxControl_get_implementation(
    css::uno::XComponentContext* /*rxContext*/, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ThemeColorsToolBoxControl());
}

ThemeColorsPaneWrapper::ThemeColorsPaneWrapper(
    vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rxFrame)
    : InterimItemWindow(pParent, u"svx/ui/themeselectorpanel.ui"_ustr, u"ThemeSelectorPanel"_ustr,
                        true, reinterpret_cast<sal_uInt64>(SfxViewShell::Current()))
    , svx::ThemeColorsPaneBase(m_xBuilder->weld_icon_view(u"iconview_theme_colors"_ustr))
    , m_xFrame(rxFrame)
{
    // Override selection handler to apply theme on single click
    if (mxIconViewThemeColors)
        mxIconViewThemeColors->connect_selection_changed(
            LINK(this, ThemeColorsPaneWrapper, SelectionChangedHdl));

    initColorSets();
    SetOptimalSize();
}

ThemeColorsPaneWrapper::~ThemeColorsPaneWrapper() { disposeOnce(); }

void ThemeColorsPaneWrapper::SetOptimalSize() { SetSizePixel(GetOptimalSize()); }

void ThemeColorsPaneWrapper::refreshThemeColors() { initColorSets(); }

void ThemeColorsPaneWrapper::dispose()
{
    mxIconViewThemeColors.reset();
    InterimItemWindow::dispose();
}

void ThemeColorsPaneWrapper::onColorSetActivated()
{
    if (!mpCurrentColorSet)
        return;

    css::uno::Sequence<css::beans::PropertyValue> aArgs{ comphelper::makePropertyValue(
        u"ThemeName"_ustr, mpCurrentColorSet->getName()) };

    comphelper::dispatchCommand(u".uno:ApplyTheme"_ustr, m_xFrame, aArgs);
}

IMPL_LINK_NOARG(ThemeColorsPaneWrapper, SelectionChangedHdl, weld::IconView&, void)
{
    // Call base class selection handler first to update mpCurrentColorSet
    svx::ThemeColorsPaneBase::SelectionChangedHdl(*mxIconViewThemeColors);

    // Apply theme on single-click
    onColorSetActivated();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
