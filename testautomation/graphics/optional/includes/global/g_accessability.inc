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
'* short description : Accessability
'*
'**************************************************************************************
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
