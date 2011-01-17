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
#ifndef _SWSYNCBTN_HXX
#define _SWSYNCBTN_HXX
#include <sfx2/childwin.hxx>

#ifdef _SYNCDLG
#include <sfx2/basedlgs.hxx>

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

class SwSyncBtnDlg : public SfxFloatingWindow
{
    PushButton              aSyncBtn;

    DECL_LINK( BtnHdl,      PushButton* pBtn = 0 );

public:
    SwSyncBtnDlg(SfxBindings*, SfxChildWindow*, Window *pParent);
    ~SwSyncBtnDlg();
};
#endif

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

class SwSyncChildWin : public SfxChildWindow
{
public:
    SwSyncChildWin( Window* ,
                    sal_uInt16 nId,
                    SfxBindings*,
                    SfxChildWinInfo*  );

    SFX_DECL_CHILDWINDOW( SwSyncChildWin );
};


#endif

