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
#include <svl/intitem.hxx>
#include <sfx2/tbxctrl.hxx>
#include <svx/svxdllapi.h>


// - TbxImageItem -


class SVX_DLLPUBLIC TbxImageItem : public SfxUInt16Item
{
public:
                            TbxImageItem( sal_uInt16 nWhich = 0, sal_uInt16 nImage = 0 );

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;
};


// - SvxGrafToolBoxControl -


class SvxGrafToolBoxControl : public SfxToolBoxControl
{
public:
    SvxGrafToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    virtual ~SvxGrafToolBoxControl();

    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState ) override;
    virtual VclPtr<vcl::Window> CreateItemWindow( vcl::Window *pParent ) override;
};


// - SvxGrafRedToolBoxControl -


class SVX_DLLPUBLIC SvxGrafRedToolBoxControl : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafRedToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
};


// - SvxGrafGreenToolBoxControl -


class SVX_DLLPUBLIC SvxGrafGreenToolBoxControl : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafGreenToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
};


// - SvxGrafBlueToolBoxControl -


class SVX_DLLPUBLIC SvxGrafBlueToolBoxControl : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafBlueToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
};


// - SvxGrafLuminanceToolBoxControl -


class SVX_DLLPUBLIC SvxGrafLuminanceToolBoxControl : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafLuminanceToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
};


// - SvxGrafContrastToolBoxControl -


class SVX_DLLPUBLIC SvxGrafContrastToolBoxControl : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafContrastToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
};


// - SvxGrafGammaToolBoxControl -


class SVX_DLLPUBLIC SvxGrafGammaToolBoxControl : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafGammaToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
};


// - SvxGrafTransparenceToolBoxControl -


class SVX_DLLPUBLIC SvxGrafTransparenceToolBoxControl : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafTransparenceToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
};


// - SvxGrafModeToolBoxControl -


class SVX_DLLPUBLIC SvxGrafModeToolBoxControl : public SfxToolBoxControl, public SfxListener
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafModeToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
                        virtual ~SvxGrafModeToolBoxControl();

    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState ) override;
    virtual VclPtr<vcl::Window> CreateItemWindow( vcl::Window *pParent ) override;
};


// - SvxGrafAttrHelper -


class SdrView;
class SfxRequest;

class SVX_DLLPUBLIC SvxGrafAttrHelper
{
public:

    static void     ExecuteGrafAttr( SfxRequest& rReq, SdrView& rView );
    static void     GetGrafAttrState( SfxItemSet& rSet, SdrView& rView );
};

#endif // INCLUDED_SVX_GRAFCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
