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
'* short description : Testing of impress tables
'*
'\***********************************************************************

sub main
    Printlog "------------------- Impress Tables Test -------------------------"
    call hStatusIn ("Graphics","g_tables.bas")

    use "graphics\optional\includes\global\g_tables.inc"

    PrintLog "-------------------------" + gApplication + "-------------------"

    call tiInsertTableUsingMenu
    call tiInsertTableUsingButton
    call tiInsertTableUsingToolbarbutton
    call tiTableObjectBar
    gApplication = "DRAW"

    PrintLog "-------------------------" + gApplication + "-------------------"

    call tiInsertTableUsingMenu
    call tiInsertTableUsingToolbarbutton
    call tiTableObjectBar

    call hStatusOut

end sub

'----------------------------------------------
sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    gApplication = "IMPRESS"
    Call getUseFiles
end sub

