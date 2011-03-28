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
'\*******************************************************************

testcase tFormatStylistFlyer
    Dim xPos%
    Dim yPos%
    Dim xSize%
    Dim ySize%

    printlog "   - Format/Stylist : using all possible things to manipulate the flyer"
    printlog "open application"
    Call  hNewDocument
    Kontext "Gestalter"
    printlog "Check if stylist is there. Expected: not there"
    if Gestalter.NotExists = false Then
        qaErrorLog "Expected Stylist to be closed; But is Open. Check tests."
    else
        if lcase(gPlatform) = "osx" then
            hTypekeys "<mod1 t>"
        else
            printlog "open stylist by pressing key [F11]"
            hTypekeys "<F11>"   
        end if
        sleep 1
    end if
    Kontext "Gestalter"
    if Gestalter.NotExists then
        Warnlog "   No stylist popped up using F11; opening now via slotID"
        FormatStylist
    end if
    printlog "save stylist position and size"
    xPos% = Gestalter.GetPosX
    yPos% = Gestalter.GetPosY
    xSize% = Gestalter.GetSizeX
    ySize% = Gestalter.GetSizeY
    printlog " if Stylist is docked; undock"
    if Gestalter.IsDocked then Gestalter.UnDock
    printlog " move stylist"
    printlog "move stylist to 10,10"
    Gestalter.Move 10, 10
    Sleep 1
    printlog "move stylist to 300,400"
    Gestalter.Move 300, 400
    Sleep 1
    printlog "move stylist to 590,790"
    Gestalter.Move 590, 790
    Sleep 1
    printlog " move stylist back to saved start position"
    Gestalter.Move xPos%, yPos%
    Sleep 1

    printlog "size stylist"
    printlog "change size to 200,200"
    Gestalter.Size 200, 200
    Sleep 1
    printlog "change size to 300,400"
    Gestalter.Size 300, 400
    Sleep 1
    printlog "change size to 150,250"
    Gestalter.Size 150, 250
    Sleep 1
    printlog "change size back to saved size"
    Gestalter.Size xSize%, ySize%
    Sleep 1

    printlog "Close stylist reopen stylist, check if appearence is same as before"
    printlog "save stylist position and size"
    xPos% = Gestalter.GetPosX
    yPos% = Gestalter.GetPosY
    xSize% = Gestalter.GetSizeX
    ySize% = Gestalter.GetSizeY
    printlog "close stylist Format->Stylist"
    FormatStylist
    Sleep 1
    printlog "open stylist Format->Stylist"
    FormatStylist
    sleep (1)
    if xPos% <> Gestalter.GetPosX then Warnlog "The x-Pos. has changed : Should be -> '"+ xPos% +  "'  Is -> '"+ Gestalter.GetPosX  +"'."
    if yPos% <> Gestalter.GetPosY then Warnlog "The y-Pos. has changed : Should be -> '"+ yPos% +  "'  Is -> '"+ Gestalter.GetPosY  +"'."
    if xSize% <> Gestalter.GetSizeX then Warnlog "The x-Size has changed : Should be -> '"+ xSize% + "'  Is -> '"+ Gestalter.GetSizeX +"'."
    if ySize% <> Gestalter.GetSizeY then Warnlog "The y-Size has changed : Should be -> '"+ ySize% + "'  Is -> '"+ Gestalter.GetSizeY +"'."

    printlog "  -   dock stylist"
    printlog "dock stylist"
    Gestalter.Dock
    Sleep 1
    if (NOT Gestalter.IsDocked) then
        warnlog "   stylist not docked but should"
    endif
    printlog "UNdock stylist"
    if (Gestalter.IsDocked) Then
        Gestalter.UnDock
    endif
    Sleep 1

    printlog "  -   maximize/minimize stylist"
    printlog "minimize stylist"
    Gestalter.Minimize
    Sleep 1
    printlog "maximize stylist"
    Gestalter.Maximize
    Sleep 1
    if (NOT Gestalter.IsMax) then
        warnlog "   The stylist did not return to maximized state"
    endif
    printlog "close stylist"
    Gestalter.Close
    Call  hCloseDocument

endcase 'tFormatStylistFlyer
'--------------------------------------------------------
testcase tFormatStylistDocuments

    Dim xPos% as Integer
    Dim yPos% as Integer
    Dim xSize% as   Integer
    Dim ySize% as   Integer

    printlog "   - Format/Stylist : check methods with more than 1 document"
    Call  hNewDocument
    printlog "  -   Open stylist, check stylist in another document"
    FormatStylist
    Call  hNewDocument
    Kontext "Gestalter"
    if (Gestalter.NotExists) then
        warnlog "   There is no stylist in the new document"
        FormatStylist
    endif

    printlog "  -   Close all documents, reopen 1"
    hFileCloseAll()
    hNewDocument()
    
    Kontext "Gestalter"
    if (Gestalter.NotExists) then
        warnlog "   There is no stylist in the new document"
    endif

    printlog "  -   stylist in a document docked, in a new rechecked"
    Gestalter.Dock
    Sleep 3
    hNewDocument
    sleep 3
    Kontext "Gestalter"
    if NOT Gestalter.IsDocked then
        warnlog "   In the new document is the stylist not docked"
    endif

    printlog "  -   Close all documents, reopen one and check the docked stylist"
    hFileCloseAll()
    hNewDocument()

    Kontext "Gestalter"
    if NOT Gestalter.IsDocked then
        warnlog "   In the new document is the stylist not docked"
    endif

    printlog "  -   Close stylist using F11"
    Gestalter.Undock
    Sleep 1
    if lcase(gPlatform) = "osx" then
        hTypekeys "<mod1 t>"
    else
        hTypeKeys "<F11>"
    end if
    Kontext "Gestalter"
    if Gestalter.exists then
        warnlog "   Stylist still exists - closing again"
        Gestalter.Close
    end if
    Call  hCloseDocument

endcase 'tFormatStylistDocuments
'--------------------------------------------------------
testcase tFormatStylistTemplatesUse

    Dim i  as Integer
    Dim save as integer

    printlog "- Format/Stylist : use all templates"
    hNewDocument
    sleep 1
    FormatStylist
    sleep 1
    Kontext "Gestalter"
    if (Gestalter.NotExists) then
        warnlog "   There is no stylist open, trying again now"
        FormatStylist
    endif
    printlog "   - Presentation-templates"
    if ( gApplication = "IMPRESS") then
        TBGestalter.Click Praesentationsvorlagen
    endif
    save = Gruppenliste.GetSelIndex
    for i=1 to Gruppenliste.GetItemCount
        Gruppenliste.Select i
        Sleep 1
    next  i

    TBGestalter.Click Grafikvorlagen
    Sleep 2
    printlog "   - Graphic-templates"
    for i=1 to Gruppenliste.GetItemCount
        Gruppenliste.Select i
        Sleep 1
    next  i
    sleep 1
    Gruppenliste.Select save

    kontext "Gestalter"
    hCloseDialog( Gestalter, "Close" )

    hFileCloseAll()

endcase 'tFormatStylistTemplatesUse
'--------------------------------------------------------
testcase tFormatStylistTemplateCreate

    goto endsub
    dim i
    dim Zaehler

    printlog "- Format/stylist :Create template and use it"
    Call hNewDocument
    sleep 3
    printlog "   - create template for paragraph"
    FormatStylesCatalog
    Kontext "VorlagenKatalog"
    sleep 2
    printlog "   " + Vorlagen.GetItemCount
    Vorlagen.Select 1
    sleep 3
    Neu.Click
    sleep 2
    Kontext
    Active.SetPage TabVerwalten
    Kontext "TabVerwalten"
    VorlagenName.SetText "ATesttool"
    TabVerwalten.OK
    sleep 2
    Kontext "VorlagenKatalog"
    sleep 1
    VorlagenKatalog.Close
    sleep 1

    Printlog "   - New created template will be deleted now"
    FormatStylesCatalog
    sleep 1
    Kontext "Vorlagenkatalog"
    sleep 1
    Vorlagen.Select 1
    sleep 1
    zaehler=Ansicht.getItemCount
    i=0
    do
        inc i
    loop while ((Ansicht.getItemText(i) <> "ATesttool") AND (i <= zaehler))
    if i>zaehler then
        warnlog "   error"
    endif
    Ansicht.Select i
    sleep 1
    Ansicht.TypeKeys "<SPACE>"
    sleep 1
    Loeschen.Click
    Kontext "Active"
    Active.Yes
    sleep 1
    Kontext "Vorlagenkatalog"
    Vorlagenkatalog.Close
    sleep 1
    Kontext "DocumentDraw"
    gMouseClick 90,90
    hFileCloseAll()

endcase 'tFormatStylistTemplateCreate
'--------------------------------------------------------
testcase tFormatStylistTemplateSelect

    dim i as integer
    dim sListDown(21) as string
    dim sListUp(21) as string
    dim sTemp as string

    printlog "   - Format/Stylist : select template"
    Call  hNewDocument
    hTextrahmenErstellen  ("This is a formatted text <Shift Home>",20,20,80,30)
    printlog "   -   format text"
    FormatCharacter
    Kontext
    Active.SetPage TabFont
    Kontext "TabFont"
    TabFont.OK

    printlog "  -   choose format using stylist"
    FormatStylist
    Kontext "Gestalter"
    sleep 1
    ' going from bottom to top
    Vorlagenliste.TypeKeys "<End>"
    for i = 1 to 20
        ListAppend(sListUp(), Vorlagenliste.GetText)
        Vorlagenliste.TypeKeys "<Up>"
    next i
    ' going from top to bottom
    Vorlagenliste.TypeKeys "<Home>"
    for i = 1 to 20
        ListAppend(sListDown(), Vorlagenliste.GetText)
        Vorlagenliste.TypeKeys "<Down>"
    next i
    ' delete double entries in both lists (up/down)
    sTemp = sListUp(20)
    for i = 19 to 1 step -1
        if (sListUp(i) = sTemp) then
            printlog i
            ListDelete(sListUp(), i+1)
        endif
        sTemp = sListUp(i)
    next i
    sTemp = sListDown(20)
    for i = 19 to 1 step -1
        if (sListDown(i) = sTemp) then
            ListDelete(sListDown(), i+1)
        endif
        sTemp = sListDown(i)
    next i
    ' if there are double entries...
    if (ListCount(sListUp()) <> ListCount(sListDown())) then
        if (iSprache = 82) then  ' WorkAround BUG: #111652#
            warnLog "There are double entries in the list! Up: " + ListCount(sListUp()) + "; Down: " +ListCount(sListDown())
        elseif (iSprache = 07) then
            warnlog "There are missing entries in the list. Will be fixed in CWS the 06.02.2006. - FHA"     'Russian TODO -FHA
        else
            warnlog "There are double entries in the list! Up: " + ListCount(sListUp()) + "; Down: " +ListCount(sListDown())
        endif
        for i = ListCount(sListUp()) to 1 step -1
            printlog "* " + i + ": '" + sListUp(i) + "'"
        next i
    endif
    ' here this method has to stay, because it will also find some errors;
    ' the method, that works even with an error in the list is shown in testcase tFormatStylistTemplateNew
    hDoubleClickInList ( Vorlagenliste, glLocale (3) )
    Gestalter.Close

    printlog "  -   check Format"
    FormatArea
    Kontext
    Active.SetPage TabArea
    Kontext "TabArea"
    'TODO: TBO check criteria, that right template is used....
    '  ???? if Farbe.IsChecked then Printlog "Standard template is used"
    TabArea.OK
    sleep 2
    hCloseDocument

endcase 'tFormatStylistTemplateSelect
'---------------------------------------------------------
testcase tFormatStylistTemplateNew

    dim sListDown(21) as string
    dim sListUp(21) as string
    dim sTemp as string
    dim i as integer

    printlog "- Format/Stylist : New template from selection"
    Call  hNewDocument
    hTextrahmenErstellen ("This text will soon be formatted and thereafter made as template",20,20,80,40)
    sleep 1
    hTypeKeys "<Return><Up><Shift End>"
    FormatCharacter
    Kontext
    Active.SetPage TabFont
    Kontext "TabFont"
    TabFont.OK

    printlog "  -   Create template from selection"
    FormatStylist
    Kontext "Gestalter"
    TBGestalter.Click Grafikvorlagen
    Sleep 1
    TBGestalterRight.Click AusSelektion
    Kontext "VorlageErzeugen"
    VorlagenName.SetText "Testtool"
    VorlageErzeugen.OK
    printlog "   - assign new graphic template to selected text"
    hTextrahmenErstellen ("This is another text",40,40,70,50)
    hTypeKeys "<Return><Up><Shift End>"
    Kontext "Gestalter"
    TBGestalter.Click Grafikvorlagen
    sleep 1
    ' test BUG: #111652# ' find illegal double entries in template list
    ' going from bottom to top
    Vorlagenliste.TypeKeys "<End>"
    for i = 1 to 20
        sTemp = Vorlagenliste.GetText
        ListAppend(sListUp(), sTemp)
        Vorlagenliste.TypeKeys "<Up>"
    next i
    ' going from top to bottom
    Vorlagenliste.TypeKeys "<Home>"
    for i = 1 to 20
        ListAppend(sListDown(), Vorlagenliste.GetText)
        Vorlagenliste.TypeKeys "<Down>"
    next i
    ' delete double entries in both lists (up/down)
    sTemp = sListUp(20)
    for i = 19 to 1 step -1
        if (sListUp(i) = sTemp) then
            ListDelete(sListUp(), i+1)
        endif
        sTemp = sListUp(i)
    next i
    sTemp = sListDown(20)
    for i = 19 to 1 step -1
        if (sListDown(i) = sTemp) then
            ListDelete(sListDown(), i+1)
        endif
        sTemp = sListDown(i)
    next i
    ' if any double entries...
    if (ListCount(sListUp()) <> ListCount(sListDown())) then
        if (iSprache = 82) then         ' WorkAround BUG: #111652#
            warnLog "82: There are double entries in the list! Up: " + ListCount(sListUp()) + "; Down: " +ListCount(sListDown())
        elseif (iSprache = 07) then
            warnlog "There are missing entries in the list. Will be fixed in CWS the 06.02.2006. - FHA"   'Russian TODO -FHA
        else
            warnlog "There are double entries in the list! Up: " + ListCount(sListUp()) + "; Down: " +ListCount(sListDown())
            for i = ListCount(sListDown()) to 1 step -1
                printlog " - " + i + " " + sListDown(i)
            next i
            for i = ListCount(sListUp()) to 1 step -1
                printlog " - " + i + " " + sListUp(i)
            next i
        endif
    endif
    Vorlagenliste.TypeKeys "<End>"
    i = 0
    do
        inc i
        sTemp = Vorlagenliste.GetText
        Vorlagenliste.TypeKeys "<Up>"
    loop while (("Testtool" <> sTemp) AND (i<20))
    Vorlagenliste.TypeKeys "<Return>"
    Kontext "Gestalter"
    Gestalter.Close

    Printlog "   - check template"
    FormatCharacter
    Kontext
    Active.SetPage TabFont
    Kontext "TabFont"
    TabFont.OK
    hTypeKeys ("<escape>",2)
    hCloseDocument

endcase 'tFormatStylistTemplateNew
'--------------------------------------------------------
testcase tiFormatStylist
warnlog "#i116803# - Crash when closing stylist with closer"
goto endsub

    Dim Num1 as integer '  graphics : first line indent ; the one with 0,6cm in the summary
    Dim Num2 as integer '  presentations : background   ; just one tabpage pops up: Area
    Dim Num3 as integer '                : background objects ; Line pops up
    Dim Num4 as integer '                : notes   ; font 20
    Dim Num5 as integer '                : outline 1  ; where 9 items exist of

    select case iSprache
    case 01: Num1 = 3  : Num2 = 1  : Num3 = 2  : Num4 = 3  : Num5 = 4
    case 07: Num1 = 3  : Num2 = 4  : Num3 = 2  : Num4 = 3  : Num5 = 1    'Russian TODO -FHA
    case 31: Num1 = 1  : Num2 = 2  : Num3 = 3  : Num4 = 1  : Num5 = 5
    case 33: Num1 = 3  : Num2 = 1  : Num3 = 3  : Num4 = 2  : Num5 = 5
    case 34: Num1 = 2  : Num2 = 10 : Num3 = 12 : Num4 = 3  : Num5 = 4
    case 36: Num1 = 2  : Num2 = 3  : Num3 = 4  : Num4 = 10 : Num5 = 6
    case 39: Num1 = 2  : Num2 = 3  : Num3 = 2  : Num4 = 1  : Num5 = 5
    case 45: Num1 = 11 : Num2 = 2  : Num3 = 2  : Num4 = 13 : Num5 = 5
    case 46: Num1 = 2  : Num2 = 2  : Num3 = 3  : Num4 = 1  : Num5 = 4
    case 48: Num1 = 11 : Num2 = 13 : Num3 = 11 : Num4 = 1  : Num5 = 5
    case 49: Num1 = 1  : Num2 = 10 : Num3 = 11 : Num4 = 14 : Num5 = 1
    case 55: Num1 = 4  : Num2 = 12 : Num3 = 11 : Num4 = 10 : Num5 = 1
        '      case 81: Num1 = 9  : Num2 = 13 : Num3 = 14 : Num4 = 12 : Num5 = 1 Asian languages not testable, because locale dependant :-(
        '      case 82: Num1 = 6  : Num2 = 11 : Num3 = 12 : Num4 = 10 : Num5 = 1
        '      case 86: Num1 = 15 : Num2 = 13 : Num3 = 14 : Num4 = 12 : Num5 = 2
        '      case 88: Num1 = 4  : Num2 = 13 : Num3 = 14 : Num4 = 3  : Num5 = 4
    case else :
        if (bAsianLan = FALSE) then
            Warnlog "For the language  " + iSprache +" nothing is prepared yet: find numbers of entries in stylist and insert here"
        else
            qaErrorLog "The Asian-office-languages aren't supported yet."
        endif
        goto endsub
    end select

    printlog "Open new doc"
    Call hNewDocument
    sleep 2
    printlog "call the navigator"
    Kontext "Gestalter"
    if Gestalter.Exists Then
        Warnlog "- Stylist exists, perhaps not closed last time a Impress doc was open"
        Gestalter.Close
    end if
    sleep 1



    FormatStylist
    Kontext "Gestalter"
    printlog "opening graphics styles"
    Grafikvorlagen.Click
    sleep 1
    printlog "Select number 1"
    Vorlagenliste.Select Num1        ' num1
    sleep 1
    Printlog "- open Grafic template Dialogs using the context menue , checking if all resources exist"
    printlog "Opening context menu"
    Vorlagenliste.OpenContextMenu
    sleep 1
    hMenuSelectNr (2)
    sleep 1
    call hWalkTheStyles
    Kontext
    Messagebox.SetPage TabText
    Kontext "TabText"
    sleep 1
    printlog "Canceling dialog"
    TabText.Cancel
    sleep 1

    if (Ucase(gApplication) = "IMPRESS") AND (iSprache <> 07) then  'Russian TODO -FHA
        Kontext "Gestalter"
        Printlog "- presentation templates"
        sleep 1
        printlog "presentation styles"
        Praesentationsvorlagen.Click
        printlog "Select number 2"
        Vorlagenliste.Select Num2
        sleep 1
        printlog "opening context menu"
        Vorlagenliste.OpenContextMenu
        sleep 1
        printlog "selecting modify"
        hMenuSelectNr (1)
        sleep 1
        Kontext
        if Messagebox.exists (5) then
            try
                printlog "Messagebox.SetPage TabArea"
                Kontext "TabArea"
                if TabArea.Exists = false then
                    warnlog "Where is TabArea?"
                else
                    Printlog "TabArea exists, good."
                    TabArea.Cancel
                endif
            catch
                warnlog "there might be another systemlocale -> I can't select the entries in the stylist via numbers :-("
            endcatch
        else
            warnlog "tab area/Messagebox doesn't exist :-("
        endif
        Kontext "Gestalter"
        sleep 1

        printlog "Select number 3"
        Vorlagenliste.Select Num3
        sleep 1
        printlog "opening context menu of number 3"
        Vorlagenliste.OpenContextMenu
        sleep 1
        hMenuSelectNr (1)
        sleep 1
        printlog "Check the styles..."
        call hWalkTheStyles (1,1)
        Kontext
        printlog "setting tabpage indents and spacing"
        Messagebox.SetPage TabEinzuegeUndAbstaende
        Kontext "TabEinzuegeUndAbstaende"
        sleep 1
        printlog "Canceling tabpage"
        TabEinzuegeUndAbstaende.Cancel
        sleep 1
        Kontext "Gestalter"
        sleep 1

        printlog "Select number 4"
        Vorlagenliste.Select Num4
        sleep 1
        printlog "opening context menu of number 4"
        Vorlagenliste.OpenContextMenu
        sleep 1
        hMenuSelectNr (1)
        sleep 1
        Kontext
        printlog "setting tabpage TabFont"
        Messagebox.SetPage TabFont
        kontext "TabFont"
        Kontext
        printlog "setting tabpage TabFontEffects"
        Messagebox.SetPage TabFontEffects
        kontext "TabFontEffects"
        Kontext
        printlog "setting tabpage TabIndents and Spacing"
        Messagebox.SetPage TabEinzuegeUndAbstaende
        kontext "TabEinzuegeUndAbstaende"
        sleep 1
        printlog "canceling TabIndents and Spacing"
        TabEinzuegeUndAbstaende.Cancel
        sleep 1
        Kontext "Gestalter"
        sleep 1

        printlog "opening context menu of number 5"
        Vorlagenliste.Select Num5
        sleep 1
        printlog "opening context menu of number 5"
        Vorlagenliste.OpenContextMenu
        sleep 1
        hMenuSelectNr (1)
        sleep 1
        Kontext
        printlog "setting tabpage Tab Font"
        Messagebox.SetPage TabFont
        kontext "TabFont"
        Kontext
        printlog "setting tabpage Tab FontEffects"
        Messagebox.SetPage TabFontEffects
        kontext "TabFontEffects"
        Kontext
        printlog "setting tabpage TabIndents and Spacing"
        Messagebox.SetPage TabEinzuegeUndAbstaende
        kontext "TabEinzuegeUndAbstaende"
        sleep 1
        Kontext
        printlog "setting tabpage Tab Bullets"
        Messagebox.SetPage TabBullet
        Kontext "TabBullet"
        sleep 1
        Kontext
        printlog "setting tabpage Tab Numbering Type"
        Messagebox.SetPage TabNumerierungsart
        Kontext "TabNumerierungsart"
        sleep 1
        Kontext
        printlog "setting tabpage Tab Graphics"
        Messagebox.SetPage TabGrafiken
        Kontext "TabGrafiken"
        sleep 1
        Kontext
        printlog "setting tabpage Tab Numbering options"
        Messagebox.SetPage TabOptionenNumerierung
        Kontext "TabOptionenNumerierung"
        sleep 1
        printlog "Canceling..."
        TabOptionenNumerierung.Cancel
        sleep 1
    endif




    kontext "DocumentImpress"
    printlog "Closing Stylist"
    if lcase(gPlatform) = "osx" then
        hTypekeys "<mod1 t>"
    else
        Kontext "Gestalter"
        Gestalter.Close 
    end if
    sleep 1
    Kontext "Gestalter"
    if Gestalter.Exists Then Warnlog "- Stylist could not be closed pressing F11"
    printlog "Closing doc."
    Call hCloseDocument

endcase 'tiFormatStylist
'---------------------------------------------------------
sub hWalkTheStyles (optional a as integer,optional b as integer)
    ' TODO: TBO DialogTest should go to Recource test!
    dim i as integer

    if isMissing (a) then a=1
    if isMissing (b) then b=2
    i=1
    if a <= i AND i <= b then
        Kontext
        Messagebox.SetPage TabLinie
        kontext "TabLinie"
        Call DialogTest ( TabLinie )
        Kontext
        Messagebox.SetPage TabArea
        kontext "TabArea"
        Call DialogTest ( TabArea, 1 )
        FillOptions.Select 1  '  NoFill.Check
        Call DialogTest ( TabArea, 1 )
        FillOptions.Select 2
        Call DialogTest ( TabArea, 2 )
        FillOptions.Select 3
        Call DialogTest ( TabArea, 3 )
        FillOptions.Select 4
        Call DialogTest ( TabArea, 4 )
        FillOptions.Select 5
        Call DialogTest ( TabArea, 5 )
        Kontext
        Messagebox.SetPage TabSchatten
        kontext "TabSchatten"
        Anzeigen.check
        Call DialogTest ( TabSchatten )
        Kontext
        Messagebox.SetPage TabTransparenz
        kontext "TabTransparenz"
        KeineTransparenz.Check
        LineareTransparenz.Check
        Transparenzverlauf.Check
        Kontext
        Messagebox.SetPage TabFont
        kontext "TabFont"
        Call DialogTest ( TabFont )
        Kontext
        Messagebox.SetPage TabFontEffects
        kontext "TabFontEffects"
        Kontext
        Messagebox.SetPage TabEinzuegeUndAbstaende
        kontext "TabEinzuegeUndAbstaende"
        Call DialogTest ( TabEinzuegeUndAbstaende )
    endif
    i=2
    if a <= i AND i <= b then
        Kontext
        Messagebox.SetPage TabVerwalten
        kontext "TabVerwalten"
        Call DialogTest ( TabVerwalten )
        Kontext
        Messagebox.SetPage TabText
        Kontext "TabText"
        Call DialogTest ( TabText )
        Kontext
        Messagebox.SetPage TabLauftext
        Kontext "TabLauftext"
        Call DialogTest ( TabLauftext )
        Kontext
        Messagebox.SetPage TabBemassung
        Kontext "TabBemassung"
        Call DialogTest ( TabBemassung )
        Kontext
        Messagebox.setpage TabVerbinder
        Kontext "TabVerbinder"
        Call Dialogtest ( TabVerbinder )
        Kontext
        Messagebox.setpage TabAusrichtungAbsatz
        Kontext "TabAusrichtungAbsatz"
        Links.Check
        Rechts.Check
        Zentriert.Check
        Blocksatz.Check
        Kontext
        Messagebox.setpage TabTabulator
        Kontext "TabTabulator"
        Neu.click
        AlleLoeschen.click
        Neu.click
        Loeschen.click
    endif
    i=3
    if a <= i AND i <= b then
        Kontext
        Messagebox.SetPage TabBullet
        Kontext "TabBullet"
        sleep 1
        Call DialogTest (TabBullet)
        sleep 1
        Kontext
        Messagebox.SetPage TabNumerierungsart
        Kontext "TabNumerierungsart"
        sleep 1
        Call DialogTest (TabNumerierungsart)
        sleep 1
        Kontext
        Messagebox.SetPage TabGrafiken
        Kontext "TabGrafiken"
        sleep 1
        Call DialogTest (TabGrafiken)
        sleep 1
        Kontext
        Messagebox.SetPage TabOptionenNumerierung
        Kontext "TabOptionenNumerierung"
        sleep 1
        Call DialogTest (TabOptionenNumerierung)
        sleep 1
    endif

end sub 'hWalkTheStyles
'---------------------------------------------------------
testcase tFormatStylist

    Dim sTemp as String
    dim sSettings(20,3) ' control_name; control_type; value
    dim i as integer
    dim abctemp

    Call  hNewDocument
    sleep 5

    hTextrahmenErstellen ("I hate mondays...",20,20,80,40)
    sleep 1
    FormatStylist
    sleep 1
    Kontext "Stylist"
    if (Stylist.NotExists) then
        qaErrorLog "There is no stylist open, trying again now"
        FormatStylist
    endif
    sleep 1
    Vorlagenliste.TypeKeys "<End>"
    Vorlagenliste.TypeKeys "<Up>"
    Vorlagenliste.TypeKeys "<Up>"
    sleep 1
    Vorlagenliste.OpenContextMenu
    sleep 1
    hMenuSelectNr 1
    sleep 1
    Kontext
    if Messagebox.exists (5) then
        Messagebox.SetPage TabVerwalten
        Kontext "TabVerwalten"
        VorlagenName.SetText "1Newtheme"
        sTemp = VorlagenName.getText
        VerknuepftMit.getSelText
        Bereich.getSelText
        TabVerwalten.OK
    endif
    sleep 1
    Kontext "Stylist"
    hSelectInList(Vorlagenliste,sTemp)
    sleep 1
    Vorlagenliste.OpenContextMenu
    sleep 1
    hMenuSelectNr 2
    sleep 1
    Kontext
    if Messagebox.exists (5) then
        Messagebox.SetPage TabVerwalten
        Kontext "TabVerwalten"
        VorlagenName.setText("1Test")
        TabVerwalten.OK
    endif
    sleep 1

    '    hTextrahmenErstellen ("I hate mondays..., too",60,60,80,60)
    '    sleep 1
    '    FormatArea
    '        i = 1
    '       kontext "tabArea"
    ''    temp = Hatching
    '    abctemp = 666666
    '    printlog isobject(abctemp)
    '    printlog isnumeric(abctemp)
    '    sSettings(i,1) = Hatching
    '    printlog isobject(sSettings(i,1))
    '    printlog isnumeric(sSettings(i,1))
    '    sSettings(i,2) = "CheckBox"
    '    sSettings(i,3) = True
    '        inc (i)
    ''        printlog " " + i
    '    dim asgc as boolean
    '    asgc = true
    ''    hWalkTheStyles2(asgc, sSettings(), temp)
    ' '   hWalkTheStyles2(abctemp)
    '    Kontext "TabArea"
    '    TabArea.OK
    Kontext "Stylist"
    if (Stylist.NotExists) then
        ErrorLog "There is no stylist open, should be."
    else
        if lcase(gPlatform) = "osx" then
            hTypekeys "<mod1 t>"
        else
            hTypekeys "<F11>"
        end if
        Kontext "Stylist"
        if (Stylist.Exists) then
            ErrorLog "The Stylist should be closed now."
        endif
    endif
    Call hCloseDocument

endcase 'tFormatStylist
'--------------------------------------------------------
function hSelectInList (window, sEntry as String) as Boolean

    Dim i as Integer
    Dim sTemp as String
    Dim sLastTemp as String

    window.TypeKeys "<End>"
    sTemp = ""
    do
        sLastTemp = sTemp
        sTemp = window.GetText
        window.TypeKeys "<Up>"
    loop while ((sEntry <> sTemp) AND (sLastTemp <> sTemp))
    if (sEntry = sTemp) then
        window.TypeKeys "<Return>"
        hSelectInList = TRUE
    else
        hSelectInList = FALSE
    endif

end function 'hSelectInList
'---------------------------------------------------------
function hWalkTheStyles2 (atemp)

    'function hWalkTheStyles2 (bSet as boolean, aSettings(), atemp as variant) as string
    dim i as integer
    dim x as integer
    dim itemp
    dim bSet
    dim aSettings(5,5)
    '    dim atemp

    i=1
    Kontext
    printlog aSettings(i,3)
    printlog aSettings(i,2)
    printlog val(aSettings(i,1))
    printlog atemp
    Messagebox.SetPage TabArea
    kontext "TabArea"
    'press button standard :-))))))
    atemp = Hatching
    printlog atemp

    if aSettings(i,3) then
        itemp = val(aSettings(i,1))
        printlog isobject(atemp)
        printlog isNumeric(atemp)
        Hatching.check
        atemp.check

    else
        '        aSettings(i,1).Uncheck
    endif
    'print "baeh"
    i=2
    Kontext
    Messagebox.SetPage TabSchatten
    kontext "TabSchatten"

    Kontext
    Messagebox.SetPage TabVerwalten
    kontext "TabVerwalten"
    i=2
    Kontext
    Messagebox.SetPage TabLinie
    kontext "TabLinie"
    'Context: *Line; Line Styles; Arrow Styles
    i=3
    Kontext
    Messagebox.SetPage TabArea
    kontext "TabArea"
    'Context: *Area; *Shadow; Transparency; Colors; Gradients; Hatching; Bitmaps
    i=4
    Kontext
    Messagebox.SetPage TabSchatten
    kontext "TabSchatten"
    i=5
    Kontext
    Messagebox.SetPage TabTransparenz
    kontext "TabTransparenz"
    i=6
    Kontext
    Messagebox.SetPage TabFont
    kontext "TabFont"
    'Context: *Font; *Font Effect; Position
    i=7
    Kontext
    Messagebox.SetPage TabFontEffects
    kontext "TabFontEffects"
    i=8
    Kontext
    Messagebox.SetPage TabEinzuegeUndAbstaende
    kontext "TabEinzuegeUndAbstaende"
    'Context: *Indents & Spacing; *Alignment; *Tabs
    i=9
    Kontext
    Messagebox.SetPage TabText
    Kontext "TabText"
    'Context: *Text; *Text Animation
    i=10
    Kontext
    Messagebox.SetPage TabLauftext
    Kontext "TabLauftext"
    i=11
    Kontext
    Messagebox.SetPage TabBemassung
    Kontext "TabBemassung"
    i=12
    Kontext
    Messagebox.setpage TabVerbinder
    Kontext "TabVerbinder"
    i=13
    Kontext
    Messagebox.setpage TabAusrichtungAbsatz
    Kontext "TabAusrichtungAbsatz"
    i=14
    Kontext
    Messagebox.setpage TabTabulator
    Kontext "TabTabulator"

    '      Messagebox.SetPage TabBullet
    '      Kontext "TabBullet"
    '      sleep 1
    '      Call DialogTest (TabBullet)
    '      sleep 1
    '      Kontext
    '      Messagebox.SetPage TabNumerierungsart
    '      Kontext "TabNumerierungsart"
    '      sleep 1
    '      Call DialogTest (TabNumerierungsart)
    '      sleep 1
    '      Kontext
    '      Messagebox.SetPage TabGrafiken
    '      Kontext "TabGrafiken"
    '      sleep 1
    '      Call DialogTest (TabGrafiken)
    '      sleep 1
    '      Kontext
    '      Messagebox.SetPage TabOptionenNumerierung
    '      Kontext "TabOptionenNumerierung"
    '      sleep 1
    '      Call DialogTest (TabOptionenNumerierung)
    '      sleep 1
end function 'hWalkTheStyles2
