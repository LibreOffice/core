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
#ifndef _TBXDRAW_HXX
#define _TBXDRAW_HXX

// include ---------------------------------------------------------------

#include <sfx2/tbxctrl.hxx>

// class SvxPopupWindowTbxMgr --------------------------------------------

class SvxPopupWindowTbxMgr : public SfxPopupWindow
{
    void*               pNiemalsDenPointerAufDieErzeugendeToolBoxMerken; // MI!
    ResId               aRIdWinTemp;
    ResId               aRIdTbxTemp;

    Link                aTbxClickHdl;


    DECL_LINK( TbxSelectHdl, ToolBox*);
    DECL_LINK( TbxClickHdl, ToolBox*);

protected:
    using SfxPopupWindow::StateChanged;

public:
    SvxPopupWindowTbxMgr(   sal_uInt16 nId,
                            SfxToolBoxControl* pTbxCtl,
                            ResId aRIdWin,
                            ResId aRIdTbx );
    SvxPopupWindowTbxMgr(   sal_uInt16 nId,
                            WindowAlign eAlign,
                            ResId aRIdWin,
                            ResId aRIdTbx );
    ~SvxPopupWindowTbxMgr();

    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState);
    virtual SfxPopupWindow*     Clone() const;
    virtual void                PopupModeEnd();

    void                        StartSelection();

    // toolbox click or execute may chage the default selection in the tools toolbox
    virtual void                UserEvent( sal_uIntPtr nEvent, void* pEventData );
};


#endif      // _TBX_DRAW_HXX

