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

#ifndef INCLUDED_SVX_GRAFCTRL_HXX
#define INCLUDED_SVX_GRAFCTRL_HXX

#include <svl/lstner.hxx>
#include <sfx2/tbxctrl.hxx>
#include <svx/svxdllapi.h>


class SvxGrafToolBoxControl : public SfxToolBoxControl
{
public:
    SvxGrafToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx );
    virtual ~SvxGrafToolBoxControl() override;

    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState ) override;
    virtual VclPtr<InterimItemWindow> CreateItemWindow( vcl::Window *pParent ) override;
};


class SVX_DLLPUBLIC SvxGrafRedToolBoxControl final : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafRedToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx );
};


class SVX_DLLPUBLIC SvxGrafGreenToolBoxControl final : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafGreenToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx );
};


class SVX_DLLPUBLIC SvxGrafBlueToolBoxControl final : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafBlueToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx );
};


class SVX_DLLPUBLIC SvxGrafLuminanceToolBoxControl final : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafLuminanceToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx );
};


class SVX_DLLPUBLIC SvxGrafContrastToolBoxControl final : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafContrastToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx );
};


class SVX_DLLPUBLIC SvxGrafGammaToolBoxControl final : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafGammaToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx );
};


class SVX_DLLPUBLIC SvxGrafTransparenceToolBoxControl final : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafTransparenceToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx );
};


class SVX_DLLPUBLIC SvxGrafModeToolBoxControl final : public SfxToolBoxControl, public SfxListener
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafModeToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx );
                        virtual ~SvxGrafModeToolBoxControl() override;

    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState ) override;
    virtual VclPtr<InterimItemWindow> CreateItemWindow( vcl::Window *pParent ) override;
};


class SdrView;
class SfxRequest;


class SVX_DLLPUBLIC SvxGrafAttrHelper
{
public:

    static void     ExecuteGrafAttr( SfxRequest& rReq, SdrView& rView );
    static void     GetGrafAttrState( SfxItemSet& rSet, SdrView const & rView );
};

#endif // INCLUDED_SVX_GRAFCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
