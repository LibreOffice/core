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

#ifndef _SVX_LBOXCTRL_HXX_
#define _SVX_LBOXCTRL_HXX_

#include <sfx2/tbxctrl.hxx>
#include <rtl/ustring.hxx>
#include <vector>
#include "svx/svxdllapi.h"

class ToolBox;
class SvxPopupWindowListBox;

/////////////////////////////////////////////////////////////////

class SvxListBoxControl : public SfxToolBoxControl
{
protected:
    String                  aActionStr;
    SvxPopupWindowListBox * pPopupWin;

    void    Impl_SetInfo( sal_uInt16 nCount );

    DECL_LINK( PopupModeEndHdl, void * );
    DECL_LINK( SelectHdl, void * );

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxListBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    virtual ~SvxListBoxControl();

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual void                StateChanged( sal_uInt16 nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState );
};

/////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SvxUndoRedoControl : public SvxListBoxControl
{
    std::vector< ::rtl::OUString > aUndoRedoList;
    ::rtl::OUString                aDefaultText;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxUndoRedoControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );

    virtual ~SvxUndoRedoControl();
    virtual void StateChanged( sal_uInt16 nSID,
                               SfxItemState eState,
                               const SfxPoolItem* pState );

    virtual SfxPopupWindow*  CreatePopupWindow();
};

#endif

