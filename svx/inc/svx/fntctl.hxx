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
#ifndef _SVX_FNTCTL_HXX
#define _SVX_FNTCTL_HXX

// include ---------------------------------------------------------------

#include <sfx2/mnuitem.hxx>
#include <svl/lstner.hxx>
#include "svx/svxdllapi.h"



class FontNameMenu;
class SfxBindings;

// class SvxFontMenuControl ----------------------------------------------

class SVX_DLLPUBLIC SvxFontMenuControl : public SfxMenuControl, public SfxListener
{
private:
    FontNameMenu*   pMenu;
    Menu&           rParent;

    SVX_DLLPRIVATE void         FillMenu();
    DECL_LINK( MenuSelect, FontNameMenu * );
    SVX_DLLPRIVATE virtual void StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
    SVX_DLLPRIVATE virtual void     Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

public:
    SvxFontMenuControl( sal_uInt16 nId, Menu&, SfxBindings& );
    ~SvxFontMenuControl();

    virtual PopupMenu*  GetPopup() const;

    SFX_DECL_MENU_CONTROL();
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
