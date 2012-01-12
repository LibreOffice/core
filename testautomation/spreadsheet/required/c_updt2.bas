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
'* short description : Resource-Test Spreadsheet Application Part II
'*
'\***********************************************************************

sub main

    use "spreadsheet\required\includes\c_upd_formatmenu.inc"
    use "spreadsheet\required\includes\c_upd_formatmenu2.inc"
    use "spreadsheet\required\includes\c_upd_formatmenu3.inc"
    use "spreadsheet\required\includes\c_upd_toolsmenu.inc"
    use "spreadsheet\required\includes\c_upd_toolsmenu2.inc"
    use "spreadsheet\required\includes\c_upd_datamenu.inc"
    use "spreadsheet\required\includes\c_upd_windowmenu.inc"
    use "spreadsheet\required\includes\c_upd_helpmenu.inc"
    use "spreadsheet\tools\includes\c_cell_tools.inc"
    use "spreadsheet\tools\includes\c_select_tools.inc"

    Printlog "-----------------------------------------------------"
    Printlog "--- Resource-Test Spreadsheet Application Part II ---"
    Printlog "-----------------------------------------------------"   

    call hStatusIn("Spreadsheet", "c_updt2.bas", "Resource Test for Spreadsheet Part II")

    call c_upd_formatmenu    'FORMAT Menu Part I
    call c_upd_formatmenu2   'FORMAT Menu Part II
    call c_upd_formatmenu3   'FORMAT Menu Part III    
    call c_upd_toolsmenu     'TOOLS  Menu Part I
    call c_upd_toolsmenu2    'TOOLS  Menu Part II
    call c_upd_datamenu      'DATA   Menu
    call c_upd_windowmenu    'WINDOW Menu
    call c_upd_helpmenu      'HELP   Menu
    
    call hStatusOut

end sub

' ********************************************
' ** - global start routines
' ********************************************
sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\tools\includes\optional\t_locale_tools.inc"
    gApplication   = "CALC"
    GetUseFiles ()
end sub
