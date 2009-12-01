'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: id_008.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:41 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
'*
'* Owner : wolfram.garten@sun.com
'*
'* short description :
'*
'******************************************************************
' #1 tiWindowNewWindow
' #1 tidWindow123       'wrn:2
'\*****************************************************************

testcase tiWindowNewWindow

    Call  hNewDocument
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    WindowNewWindow
    WaitSlot (2000)
    Call hCloseDocument
endcase

testcase tidWindow123
    goto endsub '' testing TBO: 29.03.2002
    dim iMenues as integer
    Call  hNewDocument
    Call hRechteckErstellen ( 10, 10, 20, 40 )

    Kontext "DocumentImpress"
    DocumentImpress.UseMenu
    iMenues = MenuGetItemCount
    warnlog "---- Number of Main menus: " & iMenues
    MenuSelect(Menugetitemid(8))
    sleep 1
    iMenues = MenuGetItemCount
    printlog "---- Number of Main menus: " & iMenues
    '     MenuSelect(Menugetitemid(14))
    sleep 1
    i=1
    printlog "count: " + i + "; of submenu: " + MenuGetItemCount + "; SID: " + MenuGetItemId (i) + "; Text: " + MenuGetItemText (Menugetitemid(i)) + "; Command: " + MenuGetItemCommand(Menugetitemid(i)) + "; Seperator?: " + MenuIsSeperator(i) + "; Enabled: " + MenuIsItemEnabled(Menugetitemid(i)) + "; Checked: " + MenuIsItemChecked(Menugetitemid(i)) + ";"
    i=2
    printlog "count: " + i + "; of submenu: " + MenuGetItemCount + "; SID: " + MenuGetItemId (i) + "; Text: " + MenuGetItemText (Menugetitemid(i)) + "; Command: " + MenuGetItemCommand(Menugetitemid(i)) + "; Seperator?: " + MenuIsSeperator(i) + "; Enabled: " + MenuIsItemEnabled(Menugetitemid(i)) + "; Checked: " + MenuIsItemChecked(Menugetitemid(i)) + ";"
    warnlog "Dynamic entries not accessible ? :-((((("
    '   i=3
    '   printlog "count: " + i + "; of submenue: " + MenuGetItemCount + "; SID: " + MenuGetItemId (i) + "; Text: " + MenuGetItemText (Menugetitemid(i)) + "; Command: " + MenuGetItemCommand(Menugetitemid(i)) + "; Seperator?: " + MenuIsSeperator(i) + "; Enabled: " + MenuIsItemEnabled(Menugetitemid(i)) + "; Checked: " + MenuIsItemChecked(Menugetitemid(i)) + ";"
    Call hCloseDocument
endcase

