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

#ifndef INCLUDED_SVX_LBOXCTRL_HXX
#define INCLUDED_SVX_LBOXCTRL_HXX

#include <sfx2/tbxctrl.hxx>
#include <rtl/ustring.hxx>
#include <vector>
#include <svx/svxdllapi.h>

class ToolBox;
class SvxPopupWindowListBox;



class SvxListBoxControl : public SfxToolBoxControl
{
protected:
    OUString                aActionStr;
    VclPtr<SvxPopupWindowListBox> pPopupWin;

    void    Impl_SetInfo( sal_Int32 nCount );

    DECL_LINK_TYPED( PopupModeEndHdl, FloatingWindow*, void );
    DECL_LINK_TYPED( SelectHdl, ListBox&, void );

public:
    SvxListBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    virtual ~SvxListBoxControl();

    virtual VclPtr<SfxPopupWindow> CreatePopupWindow() override;
    virtual void                StateChanged( sal_uInt16 nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState ) override;
};



class SVX_DLLPUBLIC SvxUndoRedoControl : public SvxListBoxControl
{
    std::vector< OUString > aUndoRedoList;
    OUString                aDefaultText;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxUndoRedoControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );

    virtual ~SvxUndoRedoControl();
    virtual void StateChanged( sal_uInt16 nSID,
                               SfxItemState eState,
                               const SfxPoolItem* pState ) override;

    virtual VclPtr<SfxPopupWindow> CreatePopupWindow() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
