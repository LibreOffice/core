'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
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
