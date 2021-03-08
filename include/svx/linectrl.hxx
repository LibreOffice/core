/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVX_LINECTRL_HXX
#define INCLUDED_SVX_LINECTRL_HXX

#include <sfx2/tbxctrl.hxx>
#include <svtools/popupwindowcontroller.hxx>
#include <svx/svxdllapi.h>
#include <memory>

namespace svx {
    class ToolboxButtonLineStyleUpdater;
}

class XLineStyleItem;
class XLineDashItem;

typedef std::function<bool(const OUString&, const css::uno::Any&)> LineStyleSelectFunction;
typedef std::function<void(bool)> LineStyleIsNoneFunction;

// SvxLineStyleController:
class SVXCORE_DLLPUBLIC SvxLineStyleToolBoxControl final : public svt::PopupWindowController
{
private:
    std::unique_ptr<svx::ToolboxButtonLineStyleUpdater> m_xBtnUpdater;

    LineStyleSelectFunction m_aLineStyleSelectFunction;
    LineStyleIsNoneFunction m_aLineStyleIsNoneFunction;

public:
    SvxLineStyleToolBoxControl( const css::uno::Reference<css::uno::XComponentContext>& rContext );

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence<css::uno::Any>& rArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    virtual void SAL_CALL execute(sal_Int16 nKeyModifier) override;
    virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

    virtual ~SvxLineStyleToolBoxControl() override;

    // called when the user selects a line style
    void setLineStyleSelectFunction(const LineStyleSelectFunction& aLineStyleSelectFunction);
    // called when the line style changes, can be used to trigger disabling the arrows if the none line style is selected
    void setLineStyleIsNoneFunction(const LineStyleIsNoneFunction& aLineStyleIsNoneFunction);
    void dispatchLineStyleCommand(const OUString& rCommand, const css::uno::Sequence<css::beans::PropertyValue>& rArgs);

private:
    virtual std::unique_ptr<WeldToolbarPopup> weldPopupWindow() override;
    virtual VclPtr<vcl::Window> createVclPopupWindow( vcl::Window* pParent ) override;

};

// SvxLineWidthController:

class SVX_DLLPUBLIC SvxLineWidthToolBoxControl final : public SfxToolBoxControl
{
    static MapUnit GetCoreMetric();

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxLineWidthToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx );
    virtual ~SvxLineWidthToolBoxControl() override;

    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                      const SfxPoolItem* pState ) override;
    virtual VclPtr<InterimItemWindow> CreateItemWindow( vcl::Window *pParent ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
