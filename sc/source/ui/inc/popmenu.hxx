/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: popmenu.hxx,v $
 * $Revision: 1.4 $
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

#ifndef SC_POPMENU_HXX
#define SC_POPMENU_HXX

#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#include "scdllapi.h"

class SC_DLLPUBLIC ScPopupMenu : public PopupMenu
{
private:
    USHORT  nSel;
    BOOL    bHit;
protected:
    virtual void Select();
public:
    ScPopupMenu() : nSel(0),bHit(FALSE) {}
    ScPopupMenu(const ResId& rRes) : PopupMenu(rRes),nSel(0),bHit(FALSE) {}
    USHORT  GetSelected() const { return nSel; }
    BOOL    WasHit() const      { return bHit; }
};



#endif

