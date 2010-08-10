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
'**************************************************************************************

testcase tdInsertObjectOleObjects

    Dim Zaehler as Integer
    Dim i as integer
    Dim a as integer
    Dim Liste (100) as String

    Liste (0) = 0
    printlog " open application "
    Call hNewDocument
    printlog " Insert->Object->OLE Object... "
    InsertObjectOLEObject
    Kontext "OLEObjektInsert"
    Zaehler = Objekttyp.GetItemCount

    printlog " - there are: --"+Zaehler + "-- possible items to insert"

    ' on lin & sol there are no additional OLE-Types, like on win ??!?!?!?!.
    ' first letter is usuallay 'S' from 'Star...', but on windows exists 'Further Objects' as last type. this won't be used.
    if (left(Objekttyp.GetItemText (Zaehler),1) <> left (Objekttyp.GetItemText (1),1)) then
        PrintLog "    "+Zaehler +": '"+Objekttyp.GetItemText (Zaehler) + "' WONT be inserted"
        a = Zaehler -1
    else
        a = Zaehler
    endif

    printlog " insert every object type, except the one, that is the last with the first letter being different from the firts' object' letter "
    for i = 1 to a
        printlog "+ select object entry in list "
        Objekttyp.Select i
        sleep 2
        PrintLog "    "+i+": '"+Objekttyp.GetSelText + "' will be inserted"
        printlog "+ leave dialog 'Insert OLE Object' with OK "
        OLEObjektInsert.OK
        sleep 2
        printlog "+ leave edit mode by typing key [Escape] "
        hTypeKeys "<escape>"
        printlog "+ Edit -> Select All "
        EditSelectAll
        sleep 2
        printlog "+ Delete object by typing key [Delete] "
        hTypeKeys "<DELETE>"
        sleep 2
        printlog "+ Insert->Object->OLE Object... "
        InsertObjectOLEObject
        Kontext "OLEObjektInsert"
    next i

    printlog "+ leave dialog 'Insert OLE Object' with CANCEL "
    OLEObjektInsert.Cancel
    sleep 1
    printlog " close application "
    Call hCloseDocument

endcase 'tdInsertObjectOleObjects

'-------------------------------------------------------------------------------
testcase tInsertFloatingFrame

    dim iWaitIndex as integer

    if (gApplication = "IMPRESS") then
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if

    printlog " open application "
    Call hNewDocument
    printlog " Insert->Floating Frame... "
    InsertFloatingFrame
    Kontext "TabEigenschaften"
    printlog " set 'name' to something "
    FrameName.SetText "StarDivision Home"
    printlog " set 'Contents' to 'graphics\\input\\leer.od?' "
    Inhalt.SetText ConvertPath (gTesttoolPath + "graphics\required\input\leer." + ExtensionString)
    printlog " close dialog 'Floating Frames Properties' with OK "
    TabEigenschaften.OK
    iWaitIndex = 0
    do while TabEigenschaften.Exists AND iWaitIndex < 10
        sleep (1)
        iWaitIndex = iWaitIndex + 1
    loop
    '   gMouseMove 10,10,90,90
    printlog " the object should be selected by default now "
    try
        printlog " Format->Position And Size "
        ContextPositionAndSize
        Kontext
        Active.SetPage TabPositionAndSize
        Kontext "TabPositionAndSize"
        If (PositionY.GetText = "") Then
            Warnlog "No position for frame available ?!"
        endif
        printlog " close dialog with OK "
        TabPositionAndSize.OK
    catch
        Warnlog "- Insert Floating Frame did not work, there is nothing in the document?"
    endcatch
    printlog " delete object by typing Key [Delete] "
    hTypekeys "<DELETE>"
    sleep 2
    kontext
    if (messagebox.exists (5)) then
        printlog "messageb: " + messagebox.gettext
        messagebox.cancel
    else
        qaErrorLog "Messagebox missing about changes in inserted document! #103343# 6.y"
    endif
    sleep 2
    printlog " close application "
    Call hCloseDocument

endcase 'tInsertFloatingFrame

'-------------------------------------------------------------------------------
testcase tInsertFields

    dim sTemp as string
    dim sFile as string
    dim i as integer

    if (gApplication = "IMPRESS") then
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if

    Call hNewDocument
    printlog " new document "

    printlog " open 'tools->options' and set first/last name and email adress on the page 'StarOffice->UserData' "
    ToolsOptions
    hToolsOptions ("STAROFFICE","USERDATA")
    VorName.SetText "Garfield"
    ZuName.SetText  "Peanuts"
    EMail.SetText   "recumbent@oracle.com"
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog " every item from menue 'Insert->Fields' is inserted, and the keys [Strg]+[A], [Delete] are pressed "
    InsertFieldsDateFix
    printlog "  (date fix) "
    EditSelectAll
    hTypeKeys "<DELETE>"

    InsertFieldsDateVariable
    printlog "  (date variable) "
    EditSelectAll
    hTypeKeys "<DELETE>"

    InsertFieldsTimeFix
    printlog "  (time fix)"
    EditSelectAll
    hTypeKeys "<DELETE>"

    InsertFieldsTimeVariable
    printlog "  (time variable) "
    EditSelectAll
    hTypeKeys "<DELETE>"

    InsertFieldsPageCountDraw
    printlog "  (page count)"
    EditSelectAll
    hTypeKeys "<DELETE>"

    InsertFieldsAuthorDraw
    printlog "  (author) "
    printlog "+ additional test for author "
    printlog "+ it has to tbe 'Garfield Peanuts' "
    hTypeKeys "<F2>"
    EditSelectAll
    EditCopy
    sTemp = GetClipboard()
    if (sTemp <> "Garfield Peanuts") then
        Warnlog "wrong author; expected: 'Garfield Peanuts' is: '" + sTemp + "'"
    else
        Printlog "Author is: '" + sTemp + "'"
    endif
    hTypeKeys "<Escape><Escape>"
    printlog "deleting content"
    EditSelectAll
    hTypeKeys "<DELETE>"

    printlog "+ additional test for pagenumber  "
    for i = 1 to 2
        InsertFieldsPageNumberDraw
        printlog "Inserting page number. It has to be '1' and should remain the same when we insert a new slide "
        hTypeKeys "<F2>"
        EditSelectAll
        EditCopy
        sTemp = GetClipboard()
        if (sTemp <> right(str(i),1) ) then
            if sTemp = 2 then
                warnlog "Wrong number; expected it to still be: '1' but was: '" + sTemp + "'"
            else
                printlog "Good. Number is: '" + sTemp + "'"
            endif
        else
            printlog "Good. Number is: '" + sTemp + "'"
        endif
        hTypeKeys "<Escape><Escape>"
        EditSelectAll
        hTypeKeys "<DELETE>"
        printlog " Insert->Slide "
        InsertSlide
        sleep 2
    next i

    printlog "And one try when we switch slides manually, deleting slides to start."
    EditDeleteSlide
    EditDeleteSlide

    for i = 1 to 2
        InsertFieldsPageNumberDraw
        printlog "  (page number) "
        printlog "+ additional test for pagenumber  "
        printlog "+ it has to change when we insert a new slide and press pagedown "
        hTypeKeys "<F2>"
        EditSelectAll
        EditCopy
        sTemp = GetClipboard()
        if i > 1 then
            if (sTemp <> right(str(i),1) ) then
                warnlog "Wrong number; expected: '" + i + "' and it was: '" + sTemp + "'"
            else
                printlog "Good. We expected number: '" + i + "' and it was number: '" + sTemp + "'"
            endif
        else
            if (sTemp <> right(str(i),1) ) then
                warnlog "Wrong number; expected: '" + i + "' and it was: '" + sTemp + "'"
            else
                printlog "Good. We expected number: '" + i + "' and it was number: '" + sTemp + "'"
            endif
        endif
        hTypeKeys "<Escape><Escape>"
        EditSelectAll
        hTypeKeys "<DELETE>"
        printlog " Insert->Slide "
        InsertSlide
        hTypeKeys "<PAGEDOWN>"
        sleep 2
    next i
    printlog "Additional test for Page Count:"
    InsertFieldsPageCountDraw
    hTypeKeys "<F2>"
    EditSelectAll
    EditCopy
    sTemp = GetClipboard()
    if sTemp <> "3" then
        warnlog "Wrong page count detected!"
    else
        printlog "Page count " & sTemp & " was expected."
    endif
    hTypeKeys "<Escape><Escape>"

    printlog "checking of page count changes if slide is inserted"
    InsertSlide
    hTypeKeys "<PAGEUP>"
    EditSelectAll
    hTypeKeys "<F2>"
    EditSelectAll
    EditCopy
    sTemp = GetClipboard()
    if sTemp <> "4" then
        warnlog "Wrong page count detected!"
    else
        printlog "Page count " & sTemp & " was expected and updated correctly."
    endif

    hTypeKeys "<Escape><Escape>"
    printlog "deleting all"
    EditSelectAll
    hTypeKeys "<DELETE>"

    InsertFieldsFileName
    printlog "  (file name) "
    printlog "+ additional test for filename "
    printlog "+ save file as " + gOfficePath + "user\\work\\fields.od ?"
    sFile = ConvertPath (gOfficePath + "user\work\fields"
    if FileExists(sFile + "." + ExtensionString) then
        kill (sFile + "." + ExtensionString)
    end if

    hFileSaveAsKill (sFile)
    sleep 5
    hTypeKeys "<F2>"
    EditSelectAll
    EditCopy
    sTemp = GetClipboard()

    if (sTemp <> sFile) then
        Warnlog "wrong author; expected: '" + sFile + "' is: '" + sTemp + "'"
    else
        Printlog "Author is: '" + sTemp + "'"
    end if

    hTypeKeys "<Escape><Escape>"
    EditSelectAll
    hTypeKeys "<DELETE>"

    printlog " close application "
    hCloseDocument

endcase 'tInsertFields

'-------------------------------------------------------------------------------
testcase tdInsertSnapPoint_Line
    qaerrorlog "Disabled SnappPointLine test, because of unpredictable mouse moves (07/2002 TBO)"
    goto endsub

    Dim Wert1 as string
    Dim Wert2 as string
    dim sGridArea as string ' holds the old value from the options
    dim i as integer
    dim z as integer
    dim itemp as integer
    dim iState as integer
    dim iPosition as integer

    printlog " new document "
    Call hNewDocument

    printlog " the snap depends on the snap-grid in tools->options->draw->grid ...->snap range "
    printlog "+  so i will modify the snap range to the maximum of 50 pixel, to be safe, i hope :-) "
    printlog " Tools->Options "
    ToolsOptions
    printlog " select Grid tab page in application "
    Select Case Ucase(gApplication)
    case "DRAW"         :    hToolsOptions ("Drawing","GRID")
    case "IMPRESS"     :    hToolsOptions ("IMPRESS","GRID")
    end select
    sGridArea = fangenFangBereich.GetText
    printlog " set metric field 'Snap Range' to Maximum "
    fangenFangBereich.ToMax
    Kontext "ExtrasOptionenDlg"
    printlog " close options dialog with OK "
    ExtrasOptionenDlg.OK

    for i = 1 to 3
        printlog " new document "
        Call hNewDocument

        printlog " ContextMenu: Insert SnapPointLine "
        InsertSnapPointLine
        Kontext "NeuesFangobjekt"
        printlog "+ check checkbox 'Type: Point' "
        select case i
        case 1: printlog "------- vertical Snap Object"
            Vertikal.Check
        case 2: printlog "------- horizontal Snap Object"
            Horizontal.Check
        case 3: printlog "------- Point Snap Object"
            Punkt.Check
        end select
        printlog "+ set Metric Field 'Position X' to 5 and get entry from field for further reference "
        if (1 AND i) then
            X.SetText "5"
            Wert1=X.GetText
        endif
        printlog "+ set Metric Field 'Position Y' to 5 and get entry from field for further reference "
        if (2 AND i) then
            Y.SetText "5"
            Wert2=Y.GetText
        else
            Wert2=Wert1
        endif
        sleep 2
        printlog "+ close dialog 'New Snap Object' with OK "
        NeuesFangobjekt.OK
        Printlog "SnapPoint x: '"+Wert1+"', y: '"+Wert2+"' created"
        sleep 2

        printlog " view->Zoom... "
        ViewZoom
        Kontext "Massstab"
        printlog "+ check radiobutton '75%'  "
        Vergroesserung75.Check
        printlog "+ close dialog 'Zoom' with OK "
        Massstab.OK

        printlog " create a rectangle "
        Call hRechteckErstellen (27,27,80,80)
        sleep 3
        printlog " Moove rectangle to test snapping: "
        printlog "+ Format->Position and SIze...  "
        ContextPositionAndSize
        Kontext
        printlog "+ select tabpage 'Position and Size' on dialog 'Position and Size' "
        Active.SetPage TabPositionAndSize
        Kontext "TabPositionAndSize"
        printlog "+ enter saved text into metricfields into 'Position X' and 'Position Y' "
        printlog "+ press more button on metric fields 'Position X' and 'Position Y' "
        if (1 AND i) then
            PositionX.SetText Wert1
            sleep 1
            PositionX.More
        endif
        if (2 AND i) then
            PositionY.SetText Wert2
            sleep 1
            PositionY.More
        endif
        printlog "+ close dialog 'Position and Size' with OK "
        TabPositionAndSize.OK
        sleep 2

        printlog " move rectangle  "
        iState = 0
        for z = 1 to 100
            gmousemove2 z,z
            try
                iTemp = getmousestyle
            catch
                qaerrorlog "OOo-Application was not on top all applications - please move the testtool to the back!"
            endcatch
            if ((0 = iState) and (6 = iTemp)) then
                iState = 1 ' found object
                iPosition = z
                printlog "found: " + z
            endif
            if ((1 = iState) and (6 <> iTemp)) then
                iState = 2 ' left object
                iPosition = (fix (z - iPosition) / 2) + iPosition
                printlog "found: " + z
            endif
        next z
        if (2 <> iState) then
            qaerrorlog "couldn't find object - the following action may fail..."
            iPosition = 60
        endif
        gMousemove (iPosition,iPosition,iPosition + 1,iPosition + 1)
        sleep 4
        printlog " check if object snapped back to snap point "
        ContextPositionAndSize
        Kontext
        if active.exists (5) then
            Active.SetPage TabPositionAndSize
        else
            warnlog "Couldn't switch tabpage (2), because object was deselected"
            hTypeKeys "<TAB>"
            ContextPositionAndSize
            Kontext
            Active.SetPage TabPositionAndSize
        endif
        Kontext "TabPositionAndSize"
        if (1 AND i) then
            if (PositionX.GetText <> Wert1) Then
                WarnLog "(1) Rectangle not aligned to snap point. Soll X= "+ Wert1 + "  IST= " + PositionX.GetText
            else
                PrintLog "(1) Rectangle aligned to snap point. Soll= " + Wert1 + "  Ist= " + PositionX.GetText
            end if
        end if
        if (2 AND i) then
            if (PositionY.GetText <> Wert2) Then
                WarnLog "(1) Rectangle not aligned to snap point. Soll Y= "+ Wert2 + "  IST= " + PositionY.GetText
            else
                PrintLog "(1) Rectangle aligned to snap point. Soll= " + Wert2 + "  Ist= " + PositionY.GetText
            end if
        end if
        TabPositionAndSize.OK
        sleep 1

        printlog " close application "
        Call hCloseDocument
    next i

    ToolsOptions
    Select Case Ucase(gApplication)
    case "DRAW"         :    hToolsOptions ("Drawing","GRID")
    case "IMPRESS"      :    hToolsOptions ("IMPRESS","GRID")
    end select
    fangenFangBereich.SetText sGridArea
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog " close application "
    Call hCloseDocument

endcase 'tdInsertSnapPoint_Line

'-------------------------------------------------------------------------------
testcase tdInsertFile

    dim sTemp as string
    dim sFile as string
    dim i as integer
    dim iCut as integer

    if (gApplication = "IMPRESS") then
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if

    printlog " open application "
    Call  hNewDocument
    sleep 1
    printlog "<b> take a text file without .txt extension, to test file regcognition </b>"
    printlog " Insert->File "
    InsertFileDraw
    sleep 1
    Kontext "OeffnenDlg"
    printlog " use file: graphics\\optional\\input\\einfg.ini"
    sFile = ConvertPath (gTesttoolPath + "graphics\required\input\einfg.ini")
    Printlog "file:     '" + sFile + "'"
    Dateiname.SetText (sFile)
    printlog " click button 'Insert' "
    Oeffnen.Click
    Kontext "SeitenObjekteEinfuegen"
    sleep 3
    printlog " expected: dialog 'Insert Slides/Objects' "
    if (SeitenObjekteEinfuegen.Exists (5) = False) Then
        warnlog "1  - Dialog 'Insert Slides/Objects' does not exist :-("
        kontext
        ' Text File Path - do it again :-(
        if active.exists (5) then
            printlog "ACTIVE: "+active.gettext
            active.ok
            ' will be wrong next time, bzw support it tooo
            printlog "Have to select the text filter manually, to get file loaded"
            InsertFileDraw
            sleep 1
            Kontext "OeffnenDlg"
            Dateiname.SetText ConvertPath (gTesttoolPath + "graphics\required\input\einfg.ini")
            Select Case Ucase(gApplication)
            case "DRAW"
                DateiTyp.Select 6 ' hopefully TEXT!
            case "IMPRESS"
                DateiTyp.Select 11 ' hopefully TEXT!
            end select
            printlog "selected filter: "+DateiTyp.GetSelText
            Oeffnen.Click
            kontext
            if active.exists (5) then
                printlog "ACTIVE: "+active.gettext
                active.ok
                warnlog "failed to open file? :-("
            endif
            ' Text File Path - dialog is there :-(
        else
            Kontext "AsciiFilterOptionen"
            If AsciiFilterOptionen.Exists(3) then
                printlog "NO, not loaded (yet): we are in ASCII filter dialog :-("
                AsciiFilterOptionen.OK
            else
                warnlog "Now there is a problem :-( 1"
            endif
        endif
        sleep 3
        Kontext "SeitenObjekteEinfuegen"
        if SeitenObjekteEinfuegen.Exists (5) <> TRUE then
            warnlog "Now there is a problem :-( 2"
        else
            Printlog "InsertFile Text-dialogue came up :-) (finaly)"
            SeitenObjekteEinfuegen.OK
            sleep 3
        endif
        ' as expected, file got regcognized and i can select the slides
    else ' the good way with    ' little strange dialog
        '*SeitenObjekteEinfuegen   DLG_INSERT_PAGES_OBJS
        'DeleteUnusedBackgrounds sd:CheckBox:DLG_INSERT_PAGES_OBJS:CBX_CHECK_MASTERS
        Printlog "1: '" + InsertObjectsList.GetSelText + "'"
        printlog " leave dialog 'Insert Slides/Objects' with OK "
        SeitenObjekteEinfuegen.OK
        sleep 3
    end if
    printlog " select all inserted text, by typing [Strg]+[A] "
    EditSelectAll
    try
        printlog " Edit->Cut "
        EditCut
    catch
        Warnlog "It seems there is nothing inserted :-("
    endcatch
    sleep 1

    printlog "<b> use a presentation file to test extended capability of dialog </b>"
    printlog " Insert->File "
    InsertFileDraw
    sleep 1
    Kontext "OeffnenDlg"
    printlog " use file: graphics\required\input\diashow.odp"
    sFile = ConvertPath (gTesttoolPath + "graphics\required\input\diashow.odp")
    Printlog "file used : '" + sFile + "'"
    Dateiname.SetText (sFile)
    printlog " click button 'Insert' "
    Oeffnen.Click
    Kontext "SeitenObjekteEinfuegen"
    sleep 3
    printlog " expected: dialog 'Insert Slides/Objects' "
    if (SeitenObjekteEinfuegen.Exists (5) = False) Then
        warnlog "unexpected error :-("
    else ' the good way with    ' little strange dialog
        '*SeitenObjekteEinfuegen   DLG_INSERT_PAGES_OBJS
        'DeleteUnusedBackgrounds sd:CheckBox:DLG_INSERT_PAGES_OBJS:CBX_CHECK_MASTERS
        InsertObjectsList.TypeKeys "<up>"
        sTemp = InsertObjectsList.GetSelText
        if (gPlatgroup <> "unx") then ' to get rid of file://[/] on unix the last '/' belongs too the file path :-(
            iCut = 8
        else
            iCut = 7
        endif
        if (fConvertBackslashToSlash (sFile) = right(sTEmp, len(sTemp)-(iCut))) then
            Printlog " 0:  '" + sTemp + "'"
        else
            Warnlog "different : '" + fConvertBackslashToSlash  (sFile) + "' than '" + right(sTEmp, len(sTemp)-(iCut)) + "'"
        endif
        InsertObjectsList.TypeKeys "<+>"
        sleep 10
        for i = 1 to 4
            InsertObjectsList.TypeKeys "<down>"
            sTemp = InsertObjectsList.GetSelText
            if (right(sTemp,1) <> right (str(i),1)) then
                warnlog " " + i + ": '" + sTemp + "' is not as expected :-("
            else
                Printlog " " + i + ": '" + sTemp + "'"
            endif
        next i
        printlog " leave dialog 'Insert Slides/Objects' with OK "
        SeitenObjekteEinfuegen.OK
        sleep 3
    end if
    kontext "Messagebox"
    printlog " in draw pops a mesbox up, about adopting format for objects, too [yes no cancel] -> say NO printlog "
    if messagebox.exists (5) then
        printlog "active: '" + messagebox.gettext + "'"
        messagebox.no
    endif
    printlog " select the inserted slide content on the last slide, by typing [End] [Strg]+[A] "
    hTypeKeys "<end>"
    EditSelectAll
    try
        printlog " Edit->Cut "
        EditCut
    catch
        Warnlog "It seems there is nothing inserted :-("
    endcatch
    sleep 1

    printlog " close application "
    Call hCloseDocument

endcase 'tdInsertFile

'-------------------------------------------------------------------------------
testcase tInsertGraphics

    dim i as integer

    printlog " open application "
    Call hNewDocument
    printlog " Insert->Graphics "
    InsertGraphicsFromFile
    Kontext "GrafikEinfuegenDlg"
    sleep 2
    printlog " check number of graphics filter in dialog, should be 27 "
    i = Dateityp.GetItemCount
    if (i <> 27) Then
        Warnlog "- Number of filters changed. Should be = 27 / but it is = '" + i + "'"
    endif
    sleep 1
    printlog " set file name to global\\input\\graf_inp\\enter.bmp"
    Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\enter.bmp")
    sleep 3
    printlog " check checkbox 'Preview' "
    Preview.Check
    sleep 5
    printlog " click button 'open' "
    Oeffnen.Click
    sleep 5
    Kontext "Active"
    if Active.Exists Then
        Warnlog Active.GetText
        Active.OK
    end if
    try
        printlog " to check if something was inserted, try Edit->Cut and Edit->Paste "
        EditCut
        sleep 3
        EditPaste
        sleep 3
    catch
        WarnLog "  Nothing was created, so EditCut does not work ?"
    endcatch
    sleep 2
    printlog " close application "
    Call hCloseDocument
    'TODO: TBO: Check if link graphic would work!"

endcase 'tInsertGraphics

'-------------------------------------------------------------------------------

testcase tInsertObjectFormula

    printlog " open application "
    Call hNewDocument
    printlog " put '1+2+3+4+5+6+7+8+9=x' into the Clipboard "
    SetClipboard "1+2+3+4+5+6+7+8+9=x"
    printlog " Insert->Object->Formula "
    InsertObjectFormulaDraw
    sleep 2
    printlog " check if window 'Selection' exists -> if not ERROR & open window with View->Selection "
    Kontext "AuswahlMath"
    if AuswahlMath.Exists Then
        printlog "  - selection is visible :-) "
    else
        if gApplication = "IMPRESS" then
            Kontext "DocumentImpress"
            DocumentImpress.UseMenu
            hMenuSelectNr(3)
            hMenuItemCheck(6)
        else
            Kontext "DocumentDraw"
            DocumentDraw.UseMenu
            hMenuSelectNr(3)
            hMenuItemCheck(6)
        endif
    endif
    'ViewSelection   deactivated due to problems with Slot
    printlog "  - selection wasn't visible :-( "
    Kontext "AuswahlMath"
    if (AuswahlMath.Exists = FALSE) Then
        warnlog "  - selection is STILL not visible :-) "
    endif
    sleep 5
    Kontext "AuswahlMath"
    sleep 3
    if AuswahlMath.Exists Then
        AuswahlMath.Close
        printlog " put Clipboard into window 'Commands' with Edit->Paste "
        if gApplication = "IMPRESS" then
            Kontext "DocumentImpress"
            DocumentImpress.UseMenu
            hMenuSelectNr(2)
            hMenuSelectNr(5)
        else
            Kontext "DocumentDraw"
            DocumentDraw.UseMenu
            hMenuSelectNr(2)
            hMenuSelectNr(5)
        endif
        'EditPaste   deactivated due to problems with Slot

        PrintLog "Formel: " + GetClipboardText + " placed into document"
    else
        Warnlog " Selection-window doesn't exist"
    end if
    gMouseClick   10,10
    printlog " close application "
    hCloseDocument

endcase 'tInsertObjectFormula

'-------------------------------------------------------------------------------
testcase tInsertChart
    qaerrorlog "Not yet ready."
    goto endsub

    dim sTemp as string

    printlog " open application "
    Call hNewDocument
    printlog " Insert->Chart "
    InsertChart
    sleep 5
    printlog " now to check we are in Chart mode, trie Insert->Title "
    InsertTitle
    Kontext "InsertTitleDialog"
    printlog " type something in field 'Main Title' "
    MainTitle.SetText "Koelner Dom Koelsch"
    PrintLog MainTitle.GetText
    printlog " Close dialog 'InsertTitleDialog' "
    InsertTitleDialog.OK
    printlog " click outside the chart, to get out of chart mode "
    gMouseClick 10,10
    printlog " check if the InsertTitle -entry is available, if yes, then it's a bug. If not, then it's ok "
    try
        InsertTitle
        Kontext "InsertTitleDialog"
        Warnlog "Edit mode of chart was not left :-("
        InsertTitleDialog.Cancel
    catch
        Printlog "Edit mode of chart left :-)"
    endcatch
    printlog " go into edit mode of chart again, by typing [Tab][Return] "
    hTypeKeys "<TAB><RETURN>"
    printlog " now to check we are in Chart mode, trie Insert->Title "
    InsertTitle
    Kontext "InsertTitleDialog"
    sTemp = MainTitle.GetText
    if (sTemp <> "Koelner Dom Koelsch") then
        Warnlog "Title changed :-( is: '" + sTemp + "'; should be: '" + "Koelner Dom Koelsch" + "'"
    endif
    InsertTitleDialog.Cancel
    printlog " click outside the chart, to get out of chart mode "
    gMouseClick 10,10


    printlog " go into edit mode of chart again, by typing [Tab][Return] "
    hTypeKeys "<TAB><RETURN>"
    printlog " now to check we are in Chart mode, try Insert->Title "
    InsertTitle
    Kontext "InsertTitleDialog"
    SubtitleCheck.Check
    sTemp = MainTitle.GetText
    if (sTemp <> "Koelner Dom Koelsch") then
        Warnlog "Title changed :-( is: '" + sTemp + "'; should be: '" + "Koelner Dom Koelsch" + "'"
    endif

    printlog " type something in field 'Main Title' "
    SubtitleTitle.SetText "This is the Subtitle-Title"
    PrintLog SubtitleTitle.GetText
    printlog " Close dialog 'InsertTitleDialog' "
    InsertTitleDialog.OK
    printlog " click outside the chart, to get out of chart mode "
    gMouseClick 10,10


    printlog " go into edit mode of chart again, by typing [Tab][Return] "
    hTypeKeys "<TAB><RETURN>"
    printlog " now to check we are in Chart mode, trie Insert->Title "
    InsertTitle
    Kontext "InsertTitleDialog"
    XaxisCheck.Check
    sTemp = SubtitleTitle.GetText
    if (sTemp <> "This is the Subtitle-Title") then
        Warnlog "Title changed :-( is: '" + sTemp + "'; should be: '" + "This is the Subtitle-Title" + "'"
    endif

    sTemp = MainTitle.GetText
    if (sTemp <> "Koelner Dom Koelsch") then
        Warnlog "Title changed :-( is: '" + sTemp + "'; should be: '" + "Koelner Dom Koelsch" + "'"
    endif

    printlog " type something in field 'X Axis Title' "
    XaxisTitle.SetText "This is the X Axis -Title"
    PrintLog XaxisTitle.GetText
    printlog " Close dialog 'InsertTitleDialog' "
    InsertTitleDialog.OK
    printlog " click outside the chart, to get out of chart mode "
    gMouseClick 10,10


    printlog " go into edit mode of chart again, by typing [Tab][Return] "
    hTypeKeys "<TAB><RETURN>"
    printlog " now to check we are in Chart mode, try Insert->Title "
    InsertTitle
    Kontext "InsertTitleDialog"
    YaxisCheck.Check
    sTemp = XaxisTitle.GetText
    if (sTemp <> "This is the X Axis -Title") then
        Warnlog "Title changed :-( is: '" + sTemp + "'; should be: '" + "This is the X Axis -Title" + "'"
    endif

    sTemp = SubtitleTitle.GetText
    if (sTemp <> "This is the Subtitle-Title") then
        Warnlog "Title changed :-( is: '" + sTemp + "'; should be: '" + "This is the Subtitle-Title" + "'"
    endif

    sTemp = MainTitle.GetText
    if (sTemp <> "Koelner Dom Koelsch") then
        Warnlog "Title changed :-( is: '" + sTemp + "'; should be: '" + "Koelner Dom Koelsch" + "'"
    endif

    if ZaxisCheck.IsChecked then
        Warnlog "ZaxisTitle should not have been checked. Check why."
    else
        try
            ZaxisTitle.SetText "This is the Z Axis -Title"
            Warnlog "ZaxisTitle should not be writable at this moment."
        catch
            printlog "ZaxisTitle was not writable. Good."
        endcatch
    end if

    printlog " type something in field 'Main Title' "
    YaxisTitle.SetText "This is the Y Axis -Title"
    PrintLog YaxisTitle.GetText
    printlog " Close dialog 'InsertTitleDialog' "
    InsertTitleDialog.OK
    printlog " click outside the chart, to get out of chart mode "
    gMouseClick 10,10

    hTypeKeys "<TAB>"


    printlog " go into edit mode of chart again, by typing [Tab][Return] "
    hTypeKeys "<TAB><RETURN>"
    printlog " now to check we are in Chart mode, trie Insert->Title "
    InsertTitle
    Kontext "InsertTitleDialog"
    '      ZaxisCheck.Check
    sTemp = YaxisTitle.GetText
    if (sTemp <> "This is the Y Axis -Title") then
        Warnlog "Title changed :-( is: '" + sTemp + "'; should be: '" + "This is the Y Axis -Title" + "'"
    endif

    sTemp = XaxisTitle.GetText
    if (sTemp <> "This is the X Axis -Title") then
        Warnlog "Title changed :-( is: '" + sTemp + "'; should be: '" + "This is the X Axis -Title" + "'"
    endif

    sTemp = SubtitleTitle.GetText
    if (sTemp <> "This is the Subtitle-Title") then
        Warnlog "Title changed :-( is: '" + sTemp + "'; should be: '" + "This is the Subtitle-Title" + "'"
    endif

    sTemp = MainTitle.GetText
    if (sTemp <> "Koelner Dom Koelsch") then
        Warnlog "Title changed :-( is: '" + sTemp + "'; should be: '" + "Koelner Dom Koelsch" + "'"
    endif

    printlog " type something in field 'X Axis Title' "
    '      ZaxisTitle.SetText "This is the Z Axis -Title"
    '      PrintLog ZaxisTitle.GetText
    printlog " Close dialog 'InsertTitleDialog' "
    InsertTitleDialog.OK
    printlog " click outside the chart, to get out of chart mode "
    gMouseClick 10,10


    printlog " go into edit mode of chart again, by typing [Tab][Return] "
    hTypeKeys "<TAB><RETURN>"
    printlog " now to check we are in Chart mode, trie Insert->Title "
    InsertTitle
    Kontext "InsertTitleDialog"
    sTemp = YaxisTitle.GetText
    if (sTemp <> "This is the Y Axis -Title") then
        Warnlog "Title changed :-( is: '" + sTemp + "'; should be: '" + "This is the Y Axis -Title" + "'"
    endif

    sTemp = XaxisTitle.GetText
    if (sTemp <> "This is the X Axis -Title") then
        Warnlog "Title changed :-( is: '" + sTemp + "'; should be: '" + "This is the X Axis -Title" + "'"
    endif

    sTemp = SubtitleTitle.GetText
    if (sTemp <> "This is the Subtitle-Title") then
        Warnlog "Title changed :-( is: '" + sTemp + "'; should be: '" + "This is the Subtitle-Title" + "'"
    endif

    sTemp = MainTitle.GetText
    if (sTemp <> "Koelner Dom Koelsch") then
        Warnlog "Title changed :-( is: '" + sTemp + "'; should be: '" + "Koelner Dom Koelsch" + "'"
    endif

    '      sTemp = ZaxisTitle.GetText
    '      if (sTemp <> "This is the Z Axis -Title") then
    '         Warnlog "Title changed :-( is: '" + sTemp + "'; should be: '" + "This is the Z Axis -Title" + "'"
    '      endif

    printlog " Close dialog 'InsertTitleDialog' "
    InsertTitleDialog.OK

    printlog " click outside the chart, to get out of chart mode "
    gMouseClick 10,10
    printlog " close application "
    Call hCloseDocument

endcase 'tInsertChart

'----------------------------------------------------------------------------------------------------------------------------------------------------------
testcase tInsertObjectPlugIn

    dim i as integer
    dim x as integer
    dim bAble as boolean
    dim sTemp as string

    printlog " open application "
    call hNewDocument
    sleep 2
    printlog " Insert->Object->Plug-in"
    InsertObjectPlugIn
    sleep 2
    Kontext "PlugInEinfuegen"
    printlog " click on button 'Browse...' "
    Durchsuchen.Click
    sleep 2
    kontext "OeffnenDLG"
    sleep 1
    bAble = FALSE
    x = Dateityp.GetItemCount
    Printlog "-------TBO: print all entries from File-Type-List: "
    for i = 1 to x
        sTemp = Dateityp.GetItemText (i)
        printlog "- - - - -: '"+ sTemp +"'"
        if (inStr(sTemp, ".mov") > 0) then
            bAble = True
            Printlog "^^^^^^^^^^^^^^^^^^ contains a Apple QT plugin .mov :-) "
        endif
    next i
    Printlog "---------------------------------------------------"
    OeffnenDLG.cancel
    Kontext "PlugInEinfuegen" ' :-)
    printlog " if .mov files are regcognized, insert the folowing one; else cancel dialog!"
    if bAble then
        printlog "+ set field 'File/URL' to 'graphics\\input\\sample.mov' "
        DateiUrl.SetText ConvertPath (gTesttoolPath + "graphics\required\input\sample.mov")
        printlog "+ press button 'OK' to close dialog "
        PlugInEinfuegen.OK
        sleep 2
        Kontext "Active"
        if Active.Exists then
            printlog Active.GetText
            Active.OK
            sleep 1
        end if
        printlog "+ click outside the inserted frame "
        gMouseClick 30,30
        try
            printlog "+ check with 'Edit->Select all' and key [delete] if something was inserted "
            EditSelectAll
            hTypeKeys "<DELETE>"
        catch
            Warnlog " Nothing inserted which could be deleted"
        endcatch
    else
        qaErrorLog "Not able to test the insertion of a plugin, because no ability to play .mov files is available :-("
        PlugInEinfuegen.Cancel
    endif
    printlog " close application "
    hCloseDocument

endcase 'tInsertObjectPlugIn

'-------------------------------------------------------------------------------
testcase tInsertSpecialCharacter
    qaerrorlog "tInsertSpecialCharacter outcommented due to unpredictable results"
    goto endsub

    Dim i as integer
    Dim j as integer
    Dim Zaehler as integer
    Dim sSchriftType
    Dim Zufall as Integer
    dim sTemp as string

    printlog " open application "
    Call hNewDocument

    printlog " insert a text frame with content "
    hTextRahmenErstellen ("Who will read this?",30,30,80,80)
    printlog "<b> Insert->Special Character... </b>"
    InsertSpecialCharacterDraw
    Kontext "Sonderzeichen"
    printlog " select a character for every 'font' "
    j = Schrift.GetItemCount
    For i = 1 to j
        Schrift.Select j
        Zufall = Int((12*Rnd)+1)
        printlog "+ to insert a character, randomize the type keys [Cursor Right] and press [Space] "
        SchriftListe.TypeKeys "<RIGHT>" , Zufall
        SchriftListe.TypeKeys "<SPACE>"
    next i
    sleep 1
    printlog " press button [Delete] "
    Loeschen.Click '----------------------------------- > NOTHING is inserted !!
    sleep 1
    try
        printlog " tries to close dialog 'Special Characters' with OK "
        Sonderzeichen.OK
        Warnlog "Dialogue need not to be closeable, because nothing is selected :-("
    catch
        printlog " close dialog with 'Cancel' "
        Sonderzeichen.Cancel
    endcatch
    sleep 1
    printlog " select something with keys [Shift]+[Home] "
    hTypeKeys "<SHIFT HOME>"
    printlog " Edit->Copy "
    EditCopy
    sTemp = GetClipboardText
    printlog " Compare the selected text with the  one typed at the beginning - it has to be the same "
    '   if  (sTemp <> "Who will read this?") then
    '      Warnlog "Hmm, there is something wrong! But what? :-( "
    '      printlog "is: '" + sTemp + "'; should: '" + "Who will read this?" + "'"
    '   endif
    sleep 2
    hTypeKeys "<end>"

    '   printlog "<b> select a character for every 'font' </b>"
    '   j = Schrift.GetItemCount
    For i = 1 to j
        printlog " Insert->Special Character... "
        InsertSpecialCharacterDraw
        Kontext "Sonderzeichen"
        sleep 1
        try
            Schrift.Select i
        catch
            qaErrorLog("Sometimes this Timeout happens: " + i )
        endcatch
        Printlog "(" + i + "/" + j + "): '" + Schrift.GetSelText + "'"
        sleep 1
        Zufall = Int((256*Rnd)+1)
        printlog "+ to insert a character, randomize the type keys [Cursor Right] and press [Space] "
        SchriftListe.TypeKeys "<home>"
        SchriftListe.TypeKeys "<RIGHT>" , Zufall
        SchriftListe.TypeKeys "<SPACE>"
        sleep 1
        try
            printlog " trie to close dialog 'Special Characters' with OK "
            Sonderzeichen.OK
        catch
            printlog " close dialog with 'Cancel' "
            Warnlog "Dialog was not closeable with OK :-("
            Sonderzeichen.Cancel
        endcatch
        sleep 2
    next i
    printlog " select the inserted chars with keys with keys [Strg]+[End] + j-times [shift]+[right] "
    hTypeKeys "<Mod1 end>"
    hTypeKeys ("<shift left>", j)
    printlog " Edit->Copy "
    EditCut
    printlog " now select the rest with keys [Shift]+[Home] "
    hTypeKeys "<SHIFT HOME>"
    printlog " Edit->Copy "
    EditCopy
    sTemp = GetClipboardText
    printlog " Compare the selected text with the  one typed at the beginning - it has to be the same "
    if  (sTemp <> "Who will read this?") then
        qaerrorlog "is: '" + sTemp + "'; should: '" + "Who will read this?" + "'"
    endif
    sleep 2
    hTypeKeys "<end>"

    printlog "<b> Insert->Special Character... </b>"
    InsertSpecialCharacterDraw
    Kontext "Sonderzeichen"
    sleep 3
    '   printlog " select a character for every 'font' "
    '   j = Schrift.GetItemCount
    For i = 1 to j
        Zufall = Int((256*Rnd)+1)
        Schrift.Select i
        sleep 5
        Printlog "(" + i + "/" + j + "): '" + Schrift.GetSelText + "'; " + Zufall
        sleep 3
        printlog "+ to insert a character, randomize the type keys [Cursor Right] and press [Space] "
        SchriftListe.TypeKeys "<RIGHT>" , Zufall
        SchriftListe.TypeKeys "<SPACE>"
        sleep 3
    next i
    sleep 1
    try
        printlog " tries to close dialog 'Special Characters' with OK "
        Sonderzeichen.OK
    catch
        printlog " close dialog with 'Cancel' "
        Warnlog "Dialogue was not closeable with OK :-("
        Sonderzeichen.Cancel
    endcatch
    sleep 2
    printlog " close application "
    Call hCloseDocument

endcase 'tInsertSpecialCharacter

'-------------------------------------------------------------------------------
testcase tInsertSpreadsheet

    printlog " open application "
    Call hNewDocument
    sleep 3
    printlog " Insert->Spreadsheet "
    InsertSpreadsheetDraw
    sleep 2
    try
        printlog " Spreadsheet is in edit mode per default "
        printlog " Format->Sheet->Rename "
        FormatSheetRename
        Kontext "TabelleUmbenennen"
        printlog " close dialog 'Rename Sheet' with OK "
        TabelleUmbenennen.OK
    catch
        WarnLog "Spreadsheet is not in editmode :-( "
    endcatch
    printlog " delete spreadsheet OLE by typing [DELETE] (first click outside the spreadsheet!) "
    gMouseClick (1,1)
    hTypeKeys "<tab>"
    sleep 3
    hTypeKeys "<DELETE>"
    sleep 2
    printlog " close application "
    hCloseDocument

endcase 'tInsertSpreadsheet

'-------------------------------------------------------------------------------

