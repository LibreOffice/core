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
#ifndef _SFX_OBJMNCTL_HXX
#define _SFX_OBJMNCTL_HXX


#include <sfx2/mnuitem.hxx>

class SfxBindings;
class PopupMenu;
class Menu;

class SfxObjectVerbsControl: public SfxMenuControl
{
    PopupMenu*          pMenu;
    Menu&               rParent;

private:
    void                FillMenu();
    DECL_LINK( MenuSelect, Menu * );
    virtual             void StateChanged( sal_uInt16, SfxItemState, const SfxPoolItem* pState );

public:
                        SfxObjectVerbsControl(sal_uInt16, Menu&, SfxBindings&);
                        ~SfxObjectVerbsControl();
    virtual PopupMenu*  GetPopup() const;
    SFX_DECL_MENU_CONTROL();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
