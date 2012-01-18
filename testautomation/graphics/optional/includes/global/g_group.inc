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
'*********************************************************************
' #1 tiGruppierung
'\********************************************************************

testcase tiGruppierung
    Dim PosX 'Variable for PositionX

    printlog "new document"
    Call hNewDocument
    printlog "Select empty layout"
    Call sSelectEmptyLayout
    sleep 3
    Kontext "DocumentImpress"
    sleep 2
    printlog "create rectangle "
    hRechteckErstellen (20,20,40,40)
    sleep 1
    printlog "create 2nd rectangle"
    hRechteckErstellen (60,60,70,10)
    sleep 1
    printlog "create 3rd rectangle"
    hRechteckErstellen (50,90,70,40)
    sleep 1
    Printlog "- Created 3 rectangles for group testing"
    printlog "Put the mouse-marker in the middle of the screen"
    gMouseClick (50,50)
    printlog "select all rectangles"
    EditSelectAll
    if (gApplication = "DRAW") then
        hOpenContextMenu
        sleep(2)
        printlog "Select Group"
        hMenuSelectNr(12)
        sleep(2)
    else
        printlog "open context menue and group rectangles"
        FormatGroupDraw
    endif
    sleep 1
    Printlog "- Get position and dimensions of elements"
    ContextPositionAndSize
    sleep 1
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    sleep 1
    PosX=PositionX.GetText
    TabPositionAndSize.OK
    sleep 1
    Kontext "DocumentImpress"
    gMouseClick 35,35
    sleep 1
    printlog "entering group using key F3"
    hTypeKeys "<F3>"
    sleep 3
    try
        printlog "cut rectangle out of document"
        EditCut
        sleep 1
        Warnlog "- Entering the group, no object within the group should have been selected"
    catch
        Printlog "- entered group, nothing selected"
    endcatch
    gMouseClick (35,35)
    gMouseMove (30,30,60,60)
    sleep 1
    printlog "leave group, compare dimensions"
    '  DocumentImpress.OpenContextMenu
    '  sleep 1
    '  hMenuSelectNr (13)
    printlog "exit group using key strg F3"
    hTypeKeys "<mod1 F3>"
    ' would be better to call the slot TBO!
    ContextPositionAndSize
    sleep 1
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    sleep 1
    if PosX = PositionX.GetText then
        Warnlog "- No change in position for X axis, even we changed position of 1 object within the group"
        TabPositionAndSize.OK
    else
        Printlog "- Moving within the group works"
        TabPositionAndSize.OK
    end if
    printlog "close document "
    Call hCloseDocument
endcase
