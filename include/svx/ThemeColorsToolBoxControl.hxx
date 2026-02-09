/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svx/svxdllapi.h>
#include <svtools/toolboxcontroller.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <vcl/vclptr.hxx>
#include <vcl/InterimItemWindow.hxx>
#include <svx/dialog/ThemeColorsPaneBase.hxx>

// Used to put theme colors pane to the notebookbar

class SVX_DLLPUBLIC ThemeColorsPaneWrapper final : public InterimItemWindow,
                                                   public svx::ThemeColorsPaneBase
{
private:
    css::uno::Reference<css::frame::XFrame> m_xFrame;

public:
    ThemeColorsPaneWrapper(vcl::Window* pParent,
                           const css::uno::Reference<css::frame::XFrame>& rxFrame);
    virtual ~ThemeColorsPaneWrapper() override;
    virtual void dispose() override;
    void SetOptimalSize();
    void refreshThemeColors();

    DECL_LINK(SelectionChangedHdl, weld::IconView&, void);

protected:
    void onColorSetActivated() override;
};

class SVX_DLLPUBLIC ThemeColorsToolBoxControl final
    : public cppu::ImplInheritanceHelper<svt::ToolboxController, css::lang::XServiceInfo>
{
    VclPtr<ThemeColorsPaneWrapper> m_xVclBox;

public:
    ThemeColorsToolBoxControl();
    virtual ~ThemeColorsToolBoxControl() override;

    // XStatusListener
    virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

    // XToolbarController
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rArguments) override;
    virtual css::uno::Reference<css::awt::XWindow>
        SAL_CALL createItemWindow(const css::uno::Reference<css::awt::XWindow>& rParent) override;

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XUpdatable
    virtual void SAL_CALL update() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
