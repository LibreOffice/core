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
#ifndef _SVX_SELCTRL_HXX
#define _SVX_SELCTRL_HXX

// include ---------------------------------------------------------------

#include <sfx2/stbitem.hxx>
#include "svx/svxdllapi.h"

// class SvxSelModeControl -----------------------------------------------

class SVX_DLLPUBLIC SvxSelectionModeControl : public SfxStatusBarControl
{
public:
    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
    virtual void    Click();
    virtual void    Paint( const UserDrawEvent& rEvt );

    SFX_DECL_STATUSBAR_CONTROL();

    SvxSelectionModeControl( sal_uInt16 nSlotId, sal_uInt16 nId, StatusBar& rStb );

    static  sal_uIntPtr GetDefItemWidth(const StatusBar& rStb);

private:
    sal_uInt16  nState;

#ifdef _SVX_SELCTRL_CXX
    SVX_DLLPRIVATE void DrawItemText_Impl();
#endif
};


#endif

