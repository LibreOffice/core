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

#include <cppuhelper/supportsservice.hxx>
#include <svx/verttexttbxctrl.hxx>
#include <svl/languageoptions.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/weld.hxx>
#include <rtl/ustring.hxx>

SvxCTLTextTbxCtrl::SvxCTLTextTbxCtrl(
    const css::uno::Reference<css::uno::XComponentContext>& rContext)
    : SvxVertCTLTextTbxCtrl(rContext)
{
    addStatusListener(".uno:CTLFontState");
}

OUString SvxCTLTextTbxCtrl::getImplementationName()
{
    return "com.sun.star.comp.svx.CTLToolBoxControl";
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_svx_CTLToolBoxControl_get_implementation(css::uno::XComponentContext* rContext,
                                                           css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SvxCTLTextTbxCtrl(rContext));
}

SvxVertTextTbxCtrl::SvxVertTextTbxCtrl(
    const css::uno::Reference<css::uno::XComponentContext>& rContext)
    : SvxVertCTLTextTbxCtrl(rContext)
{
    addStatusListener(".uno:VerticalTextState");
}

OUString SvxVertTextTbxCtrl::getImplementationName()
{
    return "com.sun.star.comp.svx.VertTextToolBoxControl";
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_svx_VertTextToolBoxControl_get_implementation(
    css::uno::XComponentContext* rContext, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SvxVertTextTbxCtrl(rContext));
}

SvxVertCTLTextTbxCtrl::SvxVertCTLTextTbxCtrl(
    const css::uno::Reference<css::uno::XComponentContext>& rContext)
    : SvxVertCTLTextTbxCtrl_Base(rContext, nullptr, OUString())
    , m_bVisible(false)
{
}

SvxVertCTLTextTbxCtrl::~SvxVertCTLTextTbxCtrl() {}

void SAL_CALL SvxVertCTLTextTbxCtrl::initialize(const css::uno::Sequence<css::uno::Any>& rArguments)
{
    SvxVertCTLTextTbxCtrl_Base::initialize(rArguments);
    // fdo#83320 Hide vertical text commands early
    setFastPropertyValue_NoBroadcast(1, css::uno::makeAny(true));

    if (m_pToolbar)
    {
        m_bVisible = m_pToolbar->get_item_visible(m_aCommandURL.toUtf8());
        return;
    }

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nItemId = 0;
    if (getToolboxId(nItemId, &pToolBox))
        m_bVisible = pToolBox->IsItemVisible(nItemId);
}

void SAL_CALL SvxVertCTLTextTbxCtrl::statusChanged(const css::frame::FeatureStateEvent& rEvent)
{
    ToolBox* pToolBox = nullptr;
    sal_uInt16 nItemId = 0;
    bool bVclToolBox = getToolboxId(nItemId, &pToolBox);

    bool bEnabled = false;
    if (rEvent.FeatureURL.Complete == ".uno:VerticalTextState")
    {
        SvtLanguageOptions aLangOptions;
        bEnabled = m_bVisible && aLangOptions.IsVerticalTextEnabled();
    }
    else if (rEvent.FeatureURL.Complete == ".uno:CTLFontState")
    {
        SvtLanguageOptions aLangOptions;
        bEnabled = m_bVisible && aLangOptions.IsCTLFontEnabled();
    }
    else
    {
        // normal command
        bool bValue = false;
        rEvent.State >>= bValue;

        if (m_pToolbar)
        {
            OString sId = m_aCommandURL.toUtf8();
            m_pToolbar->set_item_active(sId, bValue);
            m_pToolbar->set_item_sensitive(sId, rEvent.IsEnabled);
        }

        if (bVclToolBox)
        {
            pToolBox->CheckItem(nItemId, bValue);
            pToolBox->EnableItem(nItemId, rEvent.IsEnabled);
        }

        return;
    }

    if (m_pToolbar)
    {
        m_pToolbar->set_item_visible(m_aCommandURL.toUtf8(), bEnabled);
        return;
    }

    if (bVclToolBox)
    {
        pToolBox->ShowItem(nItemId, bEnabled);

        vcl::Window* pParent = pToolBox->GetParent();
        if (WindowType::FLOATINGWINDOW == pParent->GetType())
        {
            Size aSize(pToolBox->CalcWindowSizePixel());
            pToolBox->SetPosSizePixel(Point(), aSize);
            pParent->SetOutputSizePixel(aSize);
        }
    }
}

// XServiceInfo
sal_Bool SAL_CALL SvxVertCTLTextTbxCtrl::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> SvxVertCTLTextTbxCtrl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
