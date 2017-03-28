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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_PARAGRAPH_PARASPACINGCONTROL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_PARAGRAPH_PARASPACINGCONTROL_HXX

#include <sfx2/tbxctrl.hxx>
#include <svx/svxdllapi.h>
#include <vcl/window.hxx>
#include <com/sun/star/ui/XContextChangeEventListener.hpp>
#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
#include <com/sun/star/ui/XContextChangeEventMultiplexer.hpp>

namespace svx {

class SVX_DLLPUBLIC ParaULSpacingControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    ParaULSpacingControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx);
    virtual ~ParaULSpacingControl() override;

    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState) override;
    virtual VclPtr<vcl::Window> CreateItemWindow(vcl::Window* pParent) override = 0;
};

class SVX_DLLPUBLIC ParaAboveSpacingControl : public ParaULSpacingControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    ParaAboveSpacingControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx);
    virtual VclPtr<vcl::Window> CreateItemWindow(vcl::Window* pParent) override;
};

class SVX_DLLPUBLIC ParaBelowSpacingControl : public ParaULSpacingControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    ParaBelowSpacingControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx);
    virtual VclPtr<vcl::Window> CreateItemWindow(vcl::Window* pParent) override;
};

class SVX_DLLPUBLIC ParaLRSpacingControl : public SfxToolBoxControl,
                                    public css::ui::XContextChangeEventListener
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    ParaLRSpacingControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx);
    virtual ~ParaLRSpacingControl() override;

    virtual void SAL_CALL dispose() override;

    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState) override;
    virtual VclPtr<vcl::Window> CreateItemWindow(vcl::Window* pParent) override = 0;

    // XContextChangeEventListener
    virtual void SAL_CALL notifyContextChangeEvent(const css::ui::ContextChangeEventObject& rEvent) override;

    virtual ::css::uno::Any SAL_CALL queryInterface(const ::css::uno::Type& aType) override;

    virtual void SAL_CALL acquire() throw () override;

    virtual void SAL_CALL disposing(const ::css::lang::EventObject&) override;

    virtual void SAL_CALL release() throw () override;

private:
    css::uno::Reference<css::ui::XContextChangeEventMultiplexer> m_xMultiplexer;
};

class SVX_DLLPUBLIC ParaLeftSpacingControl : public ParaLRSpacingControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    ParaLeftSpacingControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx);
    virtual VclPtr<vcl::Window> CreateItemWindow(vcl::Window* pParent) override;
};

class SVX_DLLPUBLIC ParaRightSpacingControl : public ParaLRSpacingControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    ParaRightSpacingControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx);
    virtual VclPtr<vcl::Window> CreateItemWindow(vcl::Window* pParent) override;
};

class SVX_DLLPUBLIC ParaFirstLineSpacingControl : public ParaLRSpacingControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    ParaFirstLineSpacingControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx);
    virtual VclPtr<vcl::Window> CreateItemWindow(vcl::Window* pParent) override;
};

}

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
