/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: editwin.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _EDITWIN_HXX
#define _EDITWIN_HXX

#if OSL_DEBUG_LEVEL > 1

#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif

class ImpWorkWindow;

class EditWindow
{
protected:
    ImpWorkWindow   *pImpWorkWindow;
    BOOL check();

    WorkWindow  *pMemParent;
    String      aMemName;
    WinBits     iMemWstyle;

    String      aMemPreWinText;
    BOOL        bShowWin;

    xub_StrLen      nTextLen;   // aus Performanzgründen eigene Länge mitführen

public:
    EditWindow( WorkWindow *pParent, const UniString &rName = UniString( RTL_CONSTASCII_USTRINGPARAM ( "Debug" ) ), WinBits iWstyle = WB_HSCROLL | WB_VSCROLL );
    virtual ~EditWindow();

    void Clear();
    void Show();
    void Hide();
    void AddText( const String &rNew );
    void AddText( const sal_Char* rNew );

    virtual BOOL Close(); // derived
    BOOL bQuiet;
    BOOL Check();
};

#endif
#endif

