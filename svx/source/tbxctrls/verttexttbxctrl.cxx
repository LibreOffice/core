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


#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include <svx/verttexttbxctrl.hxx>
#include <svl/languageoptions.hxx>
#include <sfx2/app.hxx>
#include <svl/eitem.hxx>
#include <vcl/toolbox.hxx>
#include <rtl/ustring.hxx>

SFX_IMPL_TOOLBOX_CONTROL(SvxCTLTextTbxCtrl, SfxBoolItem);
SFX_IMPL_TOOLBOX_CONTROL(SvxVertTextTbxCtrl, SfxBoolItem);

SvxCTLTextTbxCtrl::SvxCTLTextTbxCtrl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SvxVertCTLTextTbxCtrl( nSlotId, nId, rTbx )
{
    addStatusListener( ".uno:CTLFontState");
}

SvxVertTextTbxCtrl::SvxVertTextTbxCtrl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SvxVertCTLTextTbxCtrl( nSlotId, nId, rTbx )
{
    addStatusListener( ".uno:VerticalTextState");
}

SvxVertCTLTextTbxCtrl::SvxVertCTLTextTbxCtrl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
}

SvxVertCTLTextTbxCtrl::~SvxVertCTLTextTbxCtrl( )
{
}

void SvxVertCTLTextTbxCtrl::StateChanged(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState )
{
    SvtLanguageOptions aLangOptions;
    bool bCalc = false;
    bool bEnabled = false;
    if ( nSID == SID_VERTICALTEXT_STATE )
        bEnabled = aLangOptions.IsVerticalTextEnabled();
    else if ( nSID == SID_CTLFONT_STATE )
        bEnabled = aLangOptions.IsCTLFontEnabled();
    else
    {
        SfxToolBoxControl::StateChanged(nSID, eState, pState);
        return;
    }

    if(!bEnabled)
    {
        // always hide if either IsVerticalTextEnabled or IsCTLFontEnabled
        // is false
        GetToolBox().HideItem( GetId() );
        bCalc = true;
    }
    if(bCalc)
    {
        ToolBox& rTbx = GetToolBox();
        vcl::Window* pParent = rTbx.GetParent();
        if(WindowType::FLOATINGWINDOW == pParent->GetType())
        {
            Size aSize(rTbx.CalcWindowSizePixel());
            rTbx.SetPosSizePixel( Point(), aSize );
            pParent->SetOutputSizePixel( aSize );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
