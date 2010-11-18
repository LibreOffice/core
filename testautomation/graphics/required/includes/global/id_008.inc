'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/************************************************************************
'*
'* Owner : wolfram.garten@oracle.com
'*
'* short description :
'*
'\*****************************************************************
sub id_008

    printLog Chr(13) + "---------    id_008    ----------"

    Call tiWindowNewWindow
    call tidWindow123

end sub
'-------------------------------------------------------------------------------
testcase tiWindowNewWindow

    printlog "open application"
    Call  hNewDocument
    printlog "create rectangle"
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    printlog "Window->New Window"
    WindowNewWindow
    WaitSlot (2000)
    printlog "close application"
    Call hCloseDocument
endcase 'tiWindowNewWindow

'-------------------------------------------------------------------------------

testcase tidWindow123
    goto endsub '' testing TBO: 29.03.2002
    dim iMenues as integer
    printlog "open application"
    Call  hNewDocument
    printlog "create rectangle"
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
    printlog "close application"
    Call hCloseDocument
endcase 'tidWindow123