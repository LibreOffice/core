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
#ifndef _SVX_INSCTRL_HXX
#define _SVX_INSCTRL_HXX

#include <sfx2/stbitem.hxx>
#include "svx/svxdllapi.h"

// class SvxInsertToolBoxControl -----------------------------------------

class SVX_DLLPUBLIC SvxInsertStatusBarControl : public SfxStatusBarControl
{
public:
    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
    virtual void    Click();
    virtual void    Paint( const UserDrawEvent& rEvt );

    SFX_DECL_STATUSBAR_CONTROL();

    SvxInsertStatusBarControl( sal_uInt16 nSlotId, sal_uInt16 nId, StatusBar& rStb );
    ~SvxInsertStatusBarControl();

private:
    sal_Bool    bInsert;

#ifdef _SVX_INSCTRL_CXX
    SVX_DLLPRIVATE void DrawItemText_Impl();
#endif
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
