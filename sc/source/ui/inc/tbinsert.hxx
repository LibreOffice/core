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

#ifndef SC_TBINSERT_HXX
#define SC_TBINSERT_HXX

#include <sfx2/tbxctrl.hxx>

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

class ScTbxInsertCtrl : public SfxToolBoxControl
{
    sal_uInt16                  nLastSlotId;

    using SfxToolBoxControl::Select;
    virtual void            Select( sal_Bool bMod1 = sal_False );

public:
    SFX_DECL_TOOLBOX_CONTROL();

    ScTbxInsertCtrl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rBox );
    ~ScTbxInsertCtrl();

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual void                StateChanged( sal_uInt16 nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState );
};

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
/*
class ScTbxInsertPopup : public SfxPopupWindow
{
    SfxToolBoxManager   aTbx;
    ResId               aRIdWinTemp;
    ResId               aRIdTbxTemp;

    Link                aTbxClickHdl;

    DECL_LINK( TbxSelectHdl, ToolBox* );
    DECL_LINK( TbxClickHdl, ToolBox* );

protected:
    virtual void        PopupModeEnd();

public:
    ScTbxInsertPopup( sal_uInt16 nId, WindowAlign eAlign,
                        const ResId& rRIdWin, const ResId& rRIdTbx,
                        SfxBindings& rBindings );
    ~ScTbxInsertPopup();

    virtual SfxPopupWindow* Clone() const;
    void StartSelection();
};
*/


#endif

