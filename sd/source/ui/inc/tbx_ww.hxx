/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#ifndef _SD_TBX_WW_HXX
#define _SD_TBX_WW_HXX

#include <sdresid.hxx>
#include <sfx2/tbxctrl.hxx>

//------------------------------------------------------------------------

/*
class SdPopupWindowTbx : public SfxPopupWindow
{
private:
    SfxToolBoxManager   aTbx;
    SdResId             aSdResIdWin;
    SdResId             aSdResIdTbx;
    WindowAlign         eTbxAlign;
    Link                aSelectLink;

    DECL_LINK( TbxSelectHdl, ToolBox * );

public:
    SdPopupWindowTbx( sal_uInt16 nId, WindowAlign eAlign,
                      SdResId aRIdWin, SdResId aRIdTbx, SfxBindings& rBindings );
    ~SdPopupWindowTbx();

    virtual SfxPopupWindow* Clone() const;

    virtual void PopupModeEnd();

    void         StartSelection() { aTbx.GetToolBox().StartSelection(); }
    void         Update();

private:

    // Adapt the toolbox to whether CTL is active or not.  If CTL is not
    //    active then all toolbox items that describe CTL specific shapes are
    //    deleted from it and its size is re-calculated.
    void AdaptToCTL (void);
};
*/
//------------------------------------------------------------------------

class SdTbxControl : public SfxToolBoxControl
{
private:
    sal_Bool    IsCheckable( sal_uInt16 nSId );

public:
            SFX_DECL_TOOLBOX_CONTROL();

            SdTbxControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
            ~SdTbxControl() {}

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual void StateChanged( sal_uInt16 nSId, SfxItemState eState,
                                const SfxPoolItem* pState );
};

#endif      // _SD_TBX_WW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
