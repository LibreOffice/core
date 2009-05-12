/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tbxctl.hxx,v $
 * $Revision: 1.10 $
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
#ifndef _BASIDE_TBXCTL_HXX
#define _BASIDE_TBXCTL_HXX

#include <sfx2/tbxctrl.hxx>
#include <com/sun/star/frame/XLayoutManager.hpp>

/*
#ifdef _BASIDE_POPUPWINDOWTBX

// class PopupWindowTbx --------------------------------------------------

class PopupWindowTbx : public SfxPopupWindow
{
private:
    SfxToolBoxManager   aTbx;
    Link                aSelectLink;

    DECL_LINK( SelectHdl, void* );

public:
    PopupWindowTbx( USHORT nId, WindowAlign eAlign,
                    ResId aRIdWin, ResId aRIdTbx, SfxBindings& rBind );
    ~PopupWindowTbx();

    void                    StartSelection()
                                { aTbx.GetToolBox().StartSelection(); }
    void                    Update();

    virtual SfxPopupWindow* Clone() const;
    virtual void            PopupModeEnd();
};
#endif
*/
//-------------------
// class TbxControls
//-------------------
class TbxControls : public SfxToolBoxControl
{
private:

    struct StateChangedInfo
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
        bool bDisabled;
    };

    USHORT                  nLastSlot;

protected:
    virtual void            StateChanged( USHORT nSID, SfxItemState eState,
                                          const SfxPoolItem* pState );
public:
    SFX_DECL_TOOLBOX_CONTROL();

    TbxControls(USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~TbxControls() {}

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();

    using                       SfxToolBoxControl::Select;
    void                        Select( USHORT nModifier );
};


#endif // _BASIDE_TBXCTL_HXX

