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
#ifndef _SVX_VERT_TEXT_TBXCTRL_HXX
#define _SVX_VERT_TEXT_TBXCTRL_HXX

#include <sfx2/tbxctrl.hxx>
#include "svx/svxdllapi.h"

/*
  control to remove/insert cjk settings dependent vertical text toolbox item
 */
class SvxVertCTLTextTbxCtrl : public SfxToolBoxControl
{
    sal_Bool bCheckVertical; //determines whether vertical mode or CTL mode has to be checked
public:
    SvxVertCTLTextTbxCtrl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SvxVertCTLTextTbxCtrl();

    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    void    SetVert(sal_Bool bSet) {bCheckVertical = bSet;}

};

class SVX_DLLPUBLIC SvxCTLTextTbxCtrl : public SvxVertCTLTextTbxCtrl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxCTLTextTbxCtrl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
};

class SVX_DLLPUBLIC SvxVertTextTbxCtrl : public SvxVertCTLTextTbxCtrl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxVertTextTbxCtrl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
