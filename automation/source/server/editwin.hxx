/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: editwin.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:23:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    ~EditWindow();

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

