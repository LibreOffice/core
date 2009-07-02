'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: g_clipboard.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:39 $
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
'**************************************************************************************
' #1 tiEditUndo
' #1 tiEditRedo
' #1 tiEditClipboard
' #1 tiEditPasteSpecial_HTML
' #1 tdEditPasteSpecial_Text
' #1 tdEditPasteSpecial_Graphic
' #1 tdEditPasteSpecial_Circle
' #1 tiEditPasteSpecial
'\*************************************************************************************

testcase tiEditUndo

    Dim sDatei as string
    Dim Ueber_Text_1 as string

    if (gApplication = "IMPRESS") then
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if

    sDatei = gTesttoolPath + "graphics\required\input\leer."+ExtensionString

    hFileOpen ConvertPath (sDatei)
    sleep 5
    printlog "Checking for readonly state. If so, make it editable."
    sMakeReadOnlyDocumentEditable

    hTypeKeys "<TAB><TAB>"
    sleep 3
    ContextPositionAndSize
    Kontext
    if Messagebox.exists (5) then
        Messagebox.SetPage TabPositionAndSize
    else
        print "what"
    endif
    Kontext "TabPositionAndSize"
    TabPositionAndSize.Cancel
    sleep 2
    Kontext "Toolbar"
    Auswahl.Click
    sleep 1
    sleep 1
    gMouseClick 60,60
    sleep 1
    Kontext "Toolbar"
    Auswahl.Click
    sleep 1
    hTypeKeys "<TAB><TAB>"
    Kontext "Toolbar"
    Auswahl.Click
    sleep 1
    Kontext
    Messagebox.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    if PositionX=0 Then Warnlog "  Command was not completely executed, and it could not be undone."
        TabPositionAndSize.Cancel
        sleep 2
        gMouseClick 60,60
        sleep 1

        Sleep 2
        gMouseClick 60,60
        sleep 1
        Kontext "Toolbar"
        Auswahl.Click
        hTypeKeys "<TAB><TAB>"
        sleep 1
        Kontext "Toolbar"
        Auswahl.Click
        kontext
        Messagebox.SetPage TabPositionAndSize
        kontext "TabPositionAndSize"
        if PositionX.GetText = Ueber_Text_1 then
            printlog "OK, last order could be redone"
        else
            warnlog "Last order could not be redone"
        end if
        TabPositionAndSize.Cancel


endcase

'-------------------------------------------------------------------------------
testcase tiEditRedo

    Dim Datei$
    Dim Ueber_Text_1

    if (gApplication = "IMPRESS") then
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if

    Datei$ = gTesttoolPath + "graphics\required\input\leer."+ExtensionString

    sleep 3
    ' check if the document is writable
    if fIsDocumentWritable = false then
        ' make the document writable and check if it's succesfull
        if fMakeDocumentWritable = false then
            warnlog "The document can't be make writeable. Test stopped."
            goto endsub
        endif
    endif
    sleep 1
    gMouseClick 30,10
    sleep 1
    hTypeKeys "<TAB>"
    sleep 2
    ContextPositionAndSize
    sleep 1
    Kontext
    Messagebox.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    Ueber_Text_1=PositionX.GetText
    TabPositionAndSize.Cancel
    sleep 1
    hTypeKeys "<TAB>"
    sleep 1
    sleep 1
    gMouseClick 60,60
    sleep 1
    hTypeKeys "<TAB>"
    sleep 1
    ContextPositionAndSize
    sleep 1
    Kontext
    Kontext "TabPositionAndSize"
    if PositionX.GetText = Ueber_Text_1 Then Warnlog "  Nothing changed, so nothing can be undone"
        TabPositionAndSize.Cancel
        sleep 1
        gMouseClick 60,60
        sleep 1
        hTypeKeys "<TAB>"
        sleep 1
        ContextPositionAndSize
        sleep 1
        kontext
        kontext "TabPositionAndSize"
        if PositionX.GetText <> Ueber_Text_1 Then Warnlog "  Undo didn't worked, so repeat last action wont work either"
            TabPositionAndSize.Cancel
            sleep 2
            sleep 2
            gMouseClick 60,60
            sleep 2
            hTypeKeys "<TAB>"
            sleep 2
            ContextPositionAndSize
            sleep 1
            kontext
            Messagebox.SetPage TabPositionAndSize
            kontext "TabPositionAndSize"
            printlog "Action Undo repeated"
        else
            warnlog "Action Undo not repeated"
        end if
        TabPositionAndSize.OK
        sleep 1
        sleep 1
endcase

'-------------------------------------------------------------------------------
testcase tiEditClipboard

    Dim Ueber_Text_4 as string
    dim sTemp as string

    Call hNewDocument
    sleep 1
    SetClipboard ""
    Call hRechteckErstellen ( 65, 65, 80, 80 )
    sleep 1
    gMouseClick 70,70
    sleep 1
    Sleep 1
    EditCopy
    hTypeKeys "<DELETE>"
    EditPaste
    sleep 1
    EditSelectAll
    sTemp = fGetPositionX()
    printlog "OK   EditCopy and EditPaste"
else
    warnlog "- Edit paste did not work; is: '" + sTemp + "' ; should: '" + Ueber_Text_4 + "'"
end if
sleep 1
hTypeKeys "<escape>"
EditSelectAll
sleep 2
EditCut
sleep 1
hTypeKeys "<TAB>"
try
    ContextPositionAndSize
    kontext
    Messagebox.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    if TabPositionAndSize.NOTexists then
        warnlog "TabPositionAndSize isn't up ?!!"
    endif
    TabPositionAndSize.Ok
    warnlog "TabPositionAndSize IS up :-( Doesn't have to be, because object should be deleted !!!"
catch
    Printlog " - There is no object in the document anymore :-)"
endcatch
sleep 1
EditPaste
sleep 2
sTemp = fGetPositionX()
if LiberalMeasurement(sTemp, Ueber_Text_4) then
    printlog "OK   Edit paste"
else
    warnlog "- Edit paste did not work; is: '" + sTemp + "' ; should: '" + Ueber_Text_4 + "'"
end if
sleep 3
Call hCloseDocument
endcase

'-------------------------------------------------------------------------------
testcase tiEditPasteSpecial_HTML

    dim Zaehler as integer
    Kontext "Standardbar"
    sleep 1
    sleep 5
    autoexecute=false
    Kontext "DocumentWriter"
    DocumentWriter.MouseDown 10,10
    DocumentWriter.MouseUp 10,10
    autoexecute=true
    EditSelectAll
    sleep 3
    Sleep 2
    sleep 3
    sleep 1
    sleep 1
    Kontext "InhaltEinfuegen"
    For Zaehler = 1 to Auswahl.GetItemCount
        Auswahl.select Zaehler
        printlog Zaehler, ". Selection = ", Auswahl.GetSelText
    next Zaehler

    warnlog "formats for inserting Html <> 4"
else
    printlog "OK   4 formats for HTML-Document"
end if

sleep 3
sleep 3
try
catch
    Warnlog "-  Nothing inserted into document"
endcatch
sleep 3
sleep 3
endcase

'-------------------------------------------------------------------------------
testcase tdEditPasteSpecial_Text

    printlog "Insert Text"
    sleep 1
    Kontext "InhaltEinfuegen"
    printlog "Selection = ", Auswahl.GetSelText
    sleep 1
    if Auswahl.GetItemCount <> 1 then
        warnlog "Selectionformat <> 1"
    else
        printlog "Format Text OK"
    end if
    sleep 1

endcase

'-------------------------------------------------------------------------------
testcase tdEditPasteSpecial_Graphic

    dim Zaehler as integer

    printlog "Insert graphic file"

    Kontext "GrafikEinfuegenDlg"
    Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\enter.bmp")
    Oeffnen.Click
    sleep 1
    sleep 1
    Kontext "InhaltEinfuegen"
    Auswahl.select Zaehler
    printlog Zaehler, ". Selection = ", Auswahl.GetSelText
next Zaehler

warnlog "Format for insert graphic files should be 4, but is: " + Auswahl.GetItemCount
else
printlog "There are " + Auswahl.GetItemCount + " formats for graphic files"
end if
gMouseClick 90,90
sleep 1
endcase

'-------------------------------------------------------------------------------
testcase tdEditPasteSpecial_Circle

    dim Zaehler as integer
    sleep 5
    Kontext "Toolbar"
    Ellipsen.Click
    sleep 3
    gMouseMove (20,20,80,80)
    sleep 1
    hTypeKeys "<escape>"
    sleep 2
    sleep 5
    sleep 1
    sleep 1
    Kontext "InhaltEinfuegen"
    Auswahl.select Zaehler
    printlog Zaehler, ". Selection = ", Auswahl.GetSelText
next Zaehler
warnlog "Number of possible formats <> 4"
else
printlog "There are 4 formats for circle"
end if
sleep 1
sleep 1
endcase

'-------------------------------------------------------------------------------
testcase tiEditPasteSpecial

    dim i as integer
    dim x as integer
    '   warnlog "TODOTBO: from draw, impress, math..."
    gApplication   = "WRITER"
    call hNewDocument
    kontext "DocumentWriter"
    DocumentWriter.TypeKeys "abc<shift home>"
    editcut
    call hCloseDocument
    gApplication   = "IMPRESS"
    call hNewDocument
    sleep 1

    Printlog " get count of possibilities: "
    EditPasteSpecial
    sleep 1
    Kontext "InhaltEinfuegen"
    x = Auswahl.GetItemCount
    for i = 1 to x
        printlog " - " + i + " - " + Auswahl.GetItemText (i)
    next i
    InhaltEinfuegen.Cancel

    Printlog " insert every possibility"
    for i = 1 to x
        Printlog " - processing number: "+i
        EditPasteSpecial
        sleep 1
        Kontext "InhaltEinfuegen"
        Auswahl.Select i
        sleep 1
        InhaltEinfuegen.OK
        sleep 1
    next i
endcase

'-------------------------------------------------------------------------------
