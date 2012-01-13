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

************
' #1 tEis1549
' #1 tEis1523
' #1 tEis1581
'\*************************************************************************************

testcase tEis1549
    call hNewDocument
    call sSelectEmptyLayout
    call hTextrahmenErstellen ("Sun",20,20,70,20)

    hTypeKeys ("<escape><escape>")
    hTypeKeys ("<end><shift home>")
    try
        EditCopy
        if GetClipboardText <> "" then
            warnlog "Leaving edit mode didn't work :-( '" + GetClipboardText + "'"
        endif
    catch
        Printlog "Left text edit mode :-)"
    endcatch
    EditSelectAll
    hTypeKeys (" ")
    sleep 1
    hTypeKeys ("is")
    hTypeKeys ("<shift home>")
    try
        EditCopy
        if GetClipboardText = "Sun is" then
            Printlog "Activating a textobject with 'printable' characters works :-)"
        else
            warnlog "Activating edit mode didn't work :-( '" + GetClipboardText + "' #99393#"
        endif
    catch
        warnlog "Entering edit mode didn't work :-( '" + GetClipboardText + "'"
    endcatch

    hTypeKeys ("<escape><escape>")
    hTypeKeys ("<end><shift home>")
    try
        EditCopy
        if GetClipboardText <> "" then
            warnlog "Leaving edit mode didn't work :-( '" + GetClipboardText + "'"
        endif
    catch
        Printlog "Left text edit mode :-)"
    endcatch
    hTypeKeys ("<Tab>")
    hTypeKeys ("<F2>")
    '   hTypeKeys ("<end><shift home>")
    hTypeKeys ("<shift home>")
    try
        EditCopy
        if GetClipboardText = "Sun is" then
            Printlog "Activating a textobject with <F2> works :-)"
        else
            warnlog "Activating edit mode didn't work :-( '" + GetClipboardText + "' #99393#"
        endif
    catch
        warnlog "Entering edit mode didn't work :-( '" + GetClipboardText + "'"
    endcatch

    hTypeKeys ("<escape><escape>")
    hTypeKeys ("<end><shift home>")
    try
        EditCopy
        if GetClipboardText <> "" then
            warnlog "Leaving edit mode didn't work :-( '" + GetClipboardText + "'"
        endif
    catch
        Printlog "Left text edit mode :-)"
    endcatch
    hTypeKeys ("<Tab>")
    hTypeKeys ("<Return>")
    '   hTypeKeys ("<end><shift home>")
    hTypeKeys ("<shift home>")
    try
        EditCopy
        if GetClipboardText = "Sun is" then
            Printlog "Activating a textobject with <Return> works :-)"
        else
            warnlog "Activating edit mode didn't work :-( '" + GetClipboardText + "' #99393#"
        endif
    catch
        warnlog "Entering edit mode didn't work :-( '" + GetClipboardText + "'"
    endcatch

    call hCloseDocument
endcase 'tEis1549

testcase tEis1523
    call hNewDocument

    printlog "Switching between Draw/Impress"
    if (UCase(gApplication)) = "IMPRESS" then
        call sSelectEmptyLayout
    
    endif
    call hTextrahmenErstellen ("A",20,20,30,30)
    hTypeKeys ("<escape><escape>")
    call hTextrahmenErstellen ("B",10,10,15,15)
    hTypeKeys ("<escape><escape>")
    call hTextrahmenErstellen ("C",40,40,50,50)
    hTypeKeys ("<escape><escape>")

    hTypeKeys ("<mod1 Return>")
    '   hTypeKeys ("<end><shift home>")
    hTypeKeys ("<shift home>")
    try
        EditCopy
        if GetClipboardText <> "A" then
            warnlog "First textbox not activated :-( instead '" + GetClipboardText + "'"
        endif
    catch
        warnlog "Unexpected error :-("
    endcatch
    hTypeKeys ("<mod1 Return>")
    '   hTypeKeys ("<end><shift home>")
    hTypeKeys ("<shift home>")
    try
        EditCopy
        if GetClipboardText <> "B" then
            warnlog "First textbox not activated :-( instead '" + GetClipboardText + "'"
        endif
    catch
        warnlog "Unexpected error :-("
    endcatch
    hTypeKeys ("<mod1 Return>")
    hTypeKeys ("<end><shift home>")
    try
        EditCopy
        if GetClipboardText <> "C" then
            warnlog "First textbox not activated :-( Instead '" + GetClipboardText + "'"
        endif
    catch
        warnlog "Unexpected error :-("
    endcatch

    hTypeKeys ("<mod1 Return>")
    hTypeKeys ("<PageDown>")
    call hTextrahmenErstellen ("D",20,20,30,30)
    hTypeKeys ("<escape><escape>")
    hTypeKeys ("<mod1 Return>")
    hTypeKeys ("<end><shift home>")
    try
        EditCopy
        if GetClipboardText <> "D" then
            warnlog "First textbox not activated :-( instead '" + GetClipboardText + "'"
        endif
    catch
        warnlog "Unexpected error :-("
    endcatch

    if (gApplication = "IMPRESS") then
        hTypeKeys ("<mod1 Return>")
        FormatModifyPage ' ..Layout
        sleep 2
        Kontext "Slides"
        hTypeKeys ("<PageDown>")
        hTypeKeys ("<PageDown>")
        kontext "Tasks"
        LayoutsPreview.TypeKeys "<home>"
        LayoutsPreview.TypeKeys "<down>"
        LayoutsPreview.TypeKeys "<Return>"
        hTypeKeys ("<mod1 Return>")
        hTypeKeys ("<end><shift home>")
        try
            EditCopy
            if GetClipboardText <> "" then
                warnlog "No new layout :-( '" + GetClipboardText + "'"
            endif
        catch
            Printlog "Layout accepted :-)"
        endcatch
        hTypeKeys "Great"
        hTypeKeys ("<end><shift home>")
        try
            EditCopy
            if GetClipboardText <> "Great" then
                warnlog "No new layout 2:-( '" + GetClipboardText + "'"
            endif
        catch
            Printlog "Layout accepted :-)"
        endcatch
    endif

    call hCloseDocument
endcase 'tEis1523

testcase tEis1581
    dim i as integer
    call hNewDocument

    if (gApplication = "IMPRESS") then
        FormatModifyPage
        sleep 1
        kontext "Tasks"
        LayoutsPreview.TypeKeys "<home>"
        kontext "Pagelayout_UndoDeleteWarning"
        if Pagelayout_UndoDeleteWarning.exists then
            Pagelayout_UndoDeleteWarning.ok
        endif
        for i = 1 to 9
            kontext "Tasks"
            LayoutsPreview.TypeKeys "<RIGHT>"
            kontext "Pagelayout_UndoDeleteWarning"
            if Pagelayout_UndoDeleteWarning.exists then
                warnlog "   Undo/Delete-Warning showed up. But it shouldn't have."
                Pagelayout_UndoDeleteWarning.ok
            endif
        next i
        kontext "Tasks"
        LayoutsPreview.TypeKeys "<Return>"
        sleep (1)
        hTypeKeys ("<tab><tab>")

        DocumentImpress.UseMenu
        hMenuSelectNr(4)
        hMenuSelectNr(13)
        hMenuSelectNr(1)
        sleep (2)
        kontext "GeneralFileDialog"
        if GeneralFileDialog.exists then
            sleep 1
            GeneralFileDialog.cancel
        else
            warnlog "Graphic wasn't selected :-("
        endif
        kontext "Tasks"
        LayoutsPreview.TypeKeys "<home>"
        kontext "Pagelayout_UndoDeleteWarning"
        if Pagelayout_UndoDeleteWarning.exists then
            warnlog "   Undo/Delete-Warning showed up. But it shouldn't have."
            Pagelayout_UndoDeleteWarning.ok
        endif
        sleep (1)
        for i = 1 to 3
            kontext "Tasks"
            LayoutsPreview.TypeKeys "<RIGHT>"
            kontext "Pagelayout_UndoDeleteWarning"
            if Pagelayout_UndoDeleteWarning.exists then
                warnlog "   Undo/Delete-Warning showed up. But it shouldn't have."
                Pagelayout_UndoDeleteWarning.ok
            endif
        next i
        kontext "Tasks"
        LayoutsPreview.TypeKeys "<Return>"
        sleep (1)
        hTypeKeys ("<tab><tab>")
        sleep (1)
        DocumentImpress.UseMenu
        hMenuSelectNr(4)
        hMenuSelectNr(16)
        hMenuSelectNr(1)
        try
            sleep 2
            '         InsertTitle *OLEObjektEinfuegen/OLEObjektInsert   SID_INSERT_OBJECT
            kontext "OLEObjektInsert"
            sleep 1
            OLEObjektInsert.cancel
        catch
            warnlog "Ole wasn't selected :-("
        endcatch
    endif
    call hCloseDocument
endcase 'tEis1581
