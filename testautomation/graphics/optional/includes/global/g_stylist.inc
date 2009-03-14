'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: g_stylist.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:40 $
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
'*******************************************************************
'*
' #1 tFormatStylistFlyer
' #1 tFormatStylistDocuments
' #1 tFormatStylistTemplatesUse
' #1 tFormatStylistTemplateCreate
' #1 tFormatStylistTemplateSelect
' #1 tFormatStylistTemplateNew
' #1 tiFormatStylist
' #1 tFormatStylist
' #1 hWalkTheStyles
'*
'\*******************************************************************

testcase tFormatStylistFlyer
    Dim xPos%
    Dim yPos%
    Dim xSize%
    Dim ySize%

    printlog "   - Format/Stylist : using all possible things to manipulate the flyer"
    '/// open application ///'
    Call  hNewDocument
    Kontext "Gestalter"
    '/// Check if stylist is there. Expected: not there ///'
    if Gestalter.NotExists = false Then
        qaErrorLog "   - Expected Stylist to be closed; But is Open. Check tests."
    else
        hTypekeys "<F11>"   '/// open stylist by pressing key [F11] ///'
        sleep 1
    end if
    Kontext "Gestalter"
    if Gestalter.NotExists then
        Warnlog "   No stylist popped up using F11; opening now via slotID"
        FormatStylist
    end if
    '///+ save stylist position and size ///'
    xPos% = Gestalter.GetPosX
    yPos% = Gestalter.GetPosY
    xSize% = Gestalter.GetSizeX
    ySize% = Gestalter.GetSizeY
    '///+ if Stylist is docked; undock ///'
    if Gestalter.IsDocked then Gestalter.UnDock
    printlog "  -   '///move stylist ///"
    '///+ move stylist to 10,10 ///'
    Gestalter.Move 10, 10
    Sleep 1
    '///+ move stylist to 300,400 ///'
    Gestalter.Move 300, 400
    Sleep 1
    '///+ move stylist to 590,790 ///'
    Gestalter.Move 590, 790
    Sleep 1
    '///+ move stylist back to saved start position ///'
    Gestalter.Move xPos%, yPos%
    Sleep 1

    printlog "  -   '///size stylist ///"
    '///+ change size to 200,200 ///'
    Gestalter.Size 200, 200
    Sleep 1
    '///+ change size to 300,400 ///'
    Gestalter.Size 300, 400
    Sleep 1
    '///+ change size to 150,250 ///'
    Gestalter.Size 150, 250
    Sleep 1
    '///+ change size back to saved size ///'
    Gestalter.Size xSize%, ySize%
    Sleep 1

    printlog "  -   Close stylist reopen stylist, check if appearence is same as before"
    '/// save stylist position and size ///'
    xPos% = Gestalter.GetPosX
    yPos% = Gestalter.GetPosY
    xSize% = Gestalter.GetSizeX
    ySize% = Gestalter.GetSizeY
    '/// close stylist Format->Stylist ///'
    FormatStylist
    Sleep 1
    '/// open stylist Format->Stylist ///'
    FormatStylist
    sleep (1)
    if xPos% <> Gestalter.GetPosX then Warnlog "The x-Pos. has changed : Should be -> '"+ xPos% +  "'  Is -> '"+ Gestalter.GetPosX  +"'."
    if yPos% <> Gestalter.GetPosY then Warnlog "The y-Pos. has changed : Should be -> '"+ yPos% +  "'  Is -> '"+ Gestalter.GetPosY  +"'."
    if xSize% <> Gestalter.GetSizeX then Warnlog "The x-Size has changed : Should be -> '"+ xSize% + "'  Is -> '"+ Gestalter.GetSizeX +"'."
    if ySize% <> Gestalter.GetSizeY then Warnlog "The y-Size has changed : Should be -> '"+ ySize% + "'  Is -> '"+ Gestalter.GetSizeY +"'."

    printlog "  -   dock stylist"
    '/// dock stylist ///'
    Gestalter.Dock
    Sleep 1
    if (NOT Gestalter.IsDocked) then
        warnlog "   stylist not docked but should"
    endif
    '/// UNdock stylist ///'
    if (Gestalter.IsDocked) Then
        Gestalter.UnDock
    endif
    Sleep 1

    printlog "  -   maximize/minimize stylist"
    '/// minimize stylist ///'
    Gestalter.Minimize
    Sleep 1
    '/// maximize stylist ///'
    Gestalter.Maximize
    Sleep 1
    if (NOT Gestalter.IsMax) then
        warnlog "   The stylist did not return to maximized state"
    endif
    '/// close stylist ///'
    Gestalter.Close
    Call  hCloseDocument
endcase

testcase tFormatStylistDocuments
    Dim xPos% as Integer
    Dim yPos% as Integer
    Dim xSize% as   Integer
    Dim ySize% as   Integer

    printlog "   - Format/Stylist : check methods with more than 1 document"
    '/// open application 1 ///'
    Call  hNewDocument
    printlog "  -   Open stylist, check stylist in another document"
    '/// Format->Stylist ///'
    FormatStylist
    '/// open application 2 ///'
    Call  hNewDocument
    Kontext "Gestalter"
    if (Gestalter.NotExists) then
        warnlog "   There is no stylist in the new document"
        FormatStylist
    endif

    printlog "  -   Close all documents, reopen 1"
    '/// File->Close  2 ///'
    FileClose
    Sleep 2
    '/// File->Close  1 ///'
    FileClose
    Sleep 2
    '/// open application 1 ///'
    hNewDocument
    Kontext "Gestalter"
    if (Gestalter.NotExists) then
        warnlog "   There is no stylist in the new document"
    endif

    printlog "  -   stylist in a document docked, in a new rechecked"
    '/// dock stylist ///'
    Gestalter.Dock
    Sleep 3
    '/// open application 2 ///'
    hNewDocument
    sleep 3
    Kontext "Gestalter"
    if NOT Gestalter.IsDocked then
        warnlog "   In the new document is the stylist not docked"
    endif

    printlog "  -   Close all documents, reopen one and check the docked stylist"
    '/// File->Close 2 ///'
    FileClose
    Sleep 2
    '/// File->Close 1 ///'
    FileClose
    Sleep 2
    Kontext "Gestalter"
    '/// open application 1 ///'
    hNewDocument
    Kontext "Gestalter"
    if NOT Gestalter.IsDocked then
        warnlog "   In the new document is the stylist not docked"
    endif

    printlog "  -   Close stylist using F11"
    '/// undock stylist ///'
    Gestalter.Undock
    Sleep 1
    '/// close stylist with key [F11] ///'
    hTypeKeys "<F11>"
    Kontext "Gestalter"
    if Gestalter.exists then
        warnlog "   Stylist still exists - closing again"
        Gestalter.Close
    end if
    '/// close application 1 ///'
    Call  hCloseDocument
endcase

testcase tFormatStylistTemplatesUse
    Dim i  as Integer
    Dim save as integer

    printlog "- Format/Stylist : use all templates"
    '/// open application ///'
    hNewDocument
    sleep 1
    '/// Format->Stylist ///'
    FormatStylist
    sleep 1
    Kontext "Gestalter"
    if (Gestalter.NotExists) then
        warnlog "   There is no stylist open, trying again now"
        FormatStylist
    endif
    printlog "   - Presentation-templates"
    '/// if application is impress: click on button 'Presentation Styles' ///'
    if (Lcase (gApplication) = "impress") then
        TBGestalter.Click Praesentationsvorlagen
    endif
    '/// select every item from the list 'Styles' ///'
    save = Gruppenliste.GetSelIndex
    for i=1 to Gruppenliste.GetItemCount
        Gruppenliste.Select i
        Sleep 1
    next  i

    '/// click on button 'Graphics Styles' ///'
    TBGestalter.Click Grafikvorlagen
    Sleep 2
    printlog "   - Graphic-templates"
    '/// select every item from the list 'Styles' ///'
    for i=1 to Gruppenliste.GetItemCount
        Gruppenliste.Select i
        Sleep 1
    next  i
    sleep 1
    Gruppenliste.Select save
    
    '/// close stylist ///'
    Gestalter.Close

    sleep 3
    '/// close application ///'
    Call  hCloseDocument
endcase

testcase tFormatStylistTemplateCreate
    goto endsub
    dim i
    dim Zaehler

    printlog "- Format/stylist :Create template and use it"
    '/// open application ///'
    Call hNewDocument
    sleep 3
    printlog "   - create template for paragraph"
    '/// Format->Styles->Catalog... ///'
    FormatStylesCatalog
        Kontext "VorlagenKatalog"
        sleep 2
        printlog "   " + Vorlagen.GetItemCount
        '/// select first template ///'
        Vorlagen.Select 1
        sleep 3
        '/// click button 'New...' ///'
        Neu.Click
            sleep 2
            Kontext
            '/// select tabpage 'Organzier' ///'
            Active.SetPage TabVerwalten
            Kontext "TabVerwalten"
            '/// set textfield 'Name' to 'ATesttool' ///'
            VorlagenName.SetText "ATesttool"
            '/// close dialog 'Graphic styles' with OK ///'
            TabVerwalten.OK
        sleep 2
        Kontext "VorlagenKatalog"
        sleep 1
        '/// close dialog 'Style Catalog' ///'
        VorlagenKatalog.Close
    sleep 1

    Printlog "   - New created template will be deleted now"
    '/// Format->Styles->Catalog... ///'
    FormatStylesCatalog
        sleep 1
        Kontext "Vorlagenkatalog"
        sleep 1
        Vorlagen.Select 1
        sleep 1
        '/// search for the created entry 'ATesttool' ///'
        zaehler=Ansicht.getItemCount
        i=0
        do
            inc i
        loop while ((Ansicht.getItemText(i) <> "ATesttool") AND (i <= zaehler))
        if i>zaehler then
            warnlog "   error"
        endif
        '/// select the entry ///'
        Ansicht.Select i
        sleep 1
        Ansicht.TypeKeys "<SPACE>"
        sleep 1
        '/// click button 'Delete...' ///'
        Loeschen.Click
            '/// say YES to 'Do you realy want to delete Style ...' ///'
            Kontext "Active"
            Active.Yes
            sleep 1
         Kontext "Vorlagenkatalog"
        '/// close dialog 'Style Catalog' ///'
         Vorlagenkatalog.Close
    sleep 1
    Kontext "DocumentDraw"
    gMouseClick 90,90
    sleep 1
    '/// close application ///'
    Call hCloseDocument
    sleep 1
endcase

testcase tFormatStylistTemplateSelect
    dim i as integer
    dim sListDown(21) as string
    dim sListUp(21) as string
    dim sTemp as string

    printlog "   - Format/Stylist : select template"
    '/// open application ///'
    Call  hNewDocument
    '/// insert a textframe with text and select the text ///'
    hTextrahmenErstellen  ("This is a formatted text <Shift Home>",20,20,80,30)
    printlog "   -   format text"
    '/// Format->Character ///'
    FormatCharacter
        Kontext
        '/// select tabpage 'Font' ///'
        Active.SetPage TabFont
        Kontext "TabFont"
        '/// close dialog 'Font' with OK ///'
        TabFont.OK

    printlog "  -   choose format using stylist"
    '/// Format->Stylist ///'
    FormatStylist
        '/// compare, if every entry can be selected from the list 'templates' ///'
        '///+ if there are double entries, then you can't go throgh the list with just pressin [down],  ///'
        '///+ but it works going from the bottom [up] -> count the uniqe entries for both directions and compare! ///'
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
        '/// doubleclick in list onto the entry 'Default' ///'
        ' here this method has to stay, because it will also find some errors;
        ' the method, that works even with an error in the list is shown in testcase tFormatStylistTemplateNew
        hDoubleClickInList ( Vorlagenliste, glLocale (3) )
        Gestalter.Close

    printlog "  -   check Format"
    '/// Format->Area ///'
    FormatArea
        Kontext
        '/// select tabpage 'Area' ///'
        Active.SetPage TabArea
        Kontext "TabArea"
        'TODO: TBO check criteria, that right template is used....
        '  ???? if Farbe.IsChecked then Printlog "Standard template is used"
        TabArea.OK
    sleep 2
    '/// close application ///'
    hCloseDocument
endcase

testcase tFormatStylistTemplateNew
    dim sListDown(21) as string
    dim sListUp(21) as string
    dim sTemp as string
    dim i as integer

    printlog "- Format/Stylist : New template from selection"
    '/// open application ///'
    Call  hNewDocument
    '/// create a textframe with content ///'
    hTextrahmenErstellen ("This text will soon be formatted and thereafter made as template",20,20,80,40)
    sleep 1
    '/// select some text in the frame ///'
    hTypeKeys "<Return><Up><Shift End>"
    '/// call Format->Character ///'
    FormatCharacter
    Kontext
        '/// select tabpage 'Font' ///'
        Active.SetPage TabFont
        Kontext "TabFont"
        '/// cloce dialog 'character' with OK ///'
        TabFont.OK

    printlog "  -   Create template from selection"
    '/// call Format->Stylist ///'
    FormatStylist
        Kontext "Gestalter"
        '/// click button 'Graphics Styles' ///'
        TBGestalter.Click Grafikvorlagen
        Sleep 1
        '/// Click button 'New from selection' ///'
        TBGestalterRight.Click AusSelektion
            Kontext "VorlageErzeugen"
            '/// in textfield 'Style name' type 'Testtool' ///'
            VorlagenName.SetText "Testtool"
        '/// click button 'OK' on dialog 'Create Style' ///'
        VorlageErzeugen.OK
    printlog "   - assign new graphic template to selected text"
    '/// create a textframe with content ///'
    hTextrahmenErstellen ("This is another text",40,40,70,50)
    '/// select some text in the frame ///'
    hTypeKeys "<Return><Up><Shift End>"
        Kontext "Gestalter"
        '/// click button 'Graphics Styles' in Stylist///'
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
        '/// go through list again from bottom and stop on the entry 'Testtool' ///'
        Vorlagenliste.TypeKeys "<End>"
        i = 0
        do
            inc i
            sTemp = Vorlagenliste.GetText
            Vorlagenliste.TypeKeys "<Up>"
        loop while (("Testtool" <> sTemp) AND (i<20))
        '/// press key [Return] ///'
        Vorlagenliste.TypeKeys "<Return>"
        Kontext "Gestalter"
        '/// close stylist ///'
        Gestalter.Close

    Printlog "   - check template"
    '/// call Format->Character ///'
    FormatCharacter
    Kontext
        '/// select tabpage 'Font' ///'
        Active.SetPage TabFont
        Kontext "TabFont"
    '/// close dialog 'character' with OK ///'
    TabFont.OK
    '/// exit textframe by pressing Key 'Escape' two times ///'
    hTypeKeys ("<escape>",2)
    '/// close application ///'
    hCloseDocument
endcase

testcase tiFormatStylist
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

    '/// open application ///'
    Call hNewDocument
    sleep 2
    '/// if stylist is open, close it ///'
    Kontext "Gestalter"
    if Gestalter.Exists Then
        Warnlog "- Stylist exists, perhaps not closed last time a Impress doc was open"
        Gestalter.Close
    end if
    sleep 1

    '/// Format->Stylist ///'
    FormatStylist
        Kontext "Gestalter"
        '/// click button 'graphics styles' ///'
        Grafikvorlagen.Click
            sleep 1
            '/// select entry 'First Line Indent' from list ///'
            Vorlagenliste.Select Num1        ' num1
            sleep 1
            Printlog "- open Grafic template Dialogs using the context menue , checking if all resources exist"
            '/// open kontext menue select 2nd entry from top: 'Modify' ///'
            Vorlagenliste.OpenContextMenu
            sleep 1
            hMenuSelectNr (2)
                sleep 1
                '///+ call every tabpage and cklick a little bit -> see 'sub hWalkTheStyles' ///'
                call hWalkTheStyles
                Kontext
                '/// switch to tabpage 'text' ///'
                Messagebox.SetPage TabText
                Kontext "TabText"
                sleep 1
                '///+ cancel dialog 'graphics styles' ///'
                TabText.Cancel
                sleep 1

    '/// if Impress then do the following / in Draw this is not avialable ///'
    if (Ucase(gApplication) = "IMPRESS") AND (iSprache <> 07) then  'Russian TODO -FHA
        Kontext "Gestalter"
        Printlog "- presentation templates"
        sleep 1
        '/// click button 'Presentation Styles' in Stylist ///'
        Praesentationsvorlagen.Click

        '/// select entry 'Background' from list ///'
        Vorlagenliste.Select Num2
        sleep 1
        '/// open kontext menue select 1st entry from top: 'Modify' ///'
        Vorlagenliste.OpenContextMenu
        sleep 1
        hMenuSelectNr (1)
            sleep 1
            Kontext
            if Messagebox.exists (5) then
                try
                    '/// only one tabpage is avilable : Area ///'
'                    Messagebox.SetPage TabArea
                    Kontext "TabArea"
                    if TabArea.Exists = false then
                       warnlog "Where is TabArea?"
                    else
                       Printlog "TabArea exists, good."
                       TabArea.Cancel
                    endif
                    '/// close dialog 'Background' ///'
                catch
                    warnlog "there might be another systemlocale -> I can't select the entries in the stylist via numbers :-("
                endcatch
            else
            warnlog "tab area/Messagebox doesn't exist :-("
            endif
        Kontext "Gestalter"
        sleep 1

        '/// select entry 'Background objects' from list in Stylist ///'
        Vorlagenliste.Select Num3
        '///+ open kontext menue select 1nd entry from top: 'Modify' ///'
        sleep 1
        Vorlagenliste.OpenContextMenu
        sleep 1
        hMenuSelectNr (1)
            sleep 1
            '///+ call every tabpage and cklick a little bit -> see 'sub hWalkTheStyles' ///'
            call hWalkTheStyles (1,1)
            Kontext
            Messagebox.SetPage TabEinzuegeUndAbstaende
            Kontext "TabEinzuegeUndAbstaende"
            sleep 1
            '///+ cancel dialog 'Background objects' ///'
            TabEinzuegeUndAbstaende.Cancel
        sleep 1
        Kontext "Gestalter"
        sleep 1

        '/// select entry 'Notes' from list in Stylist ///'
        Vorlagenliste.Select Num4
        sleep 1
        '///+ open kontext menue select 1st entry from top: 'Modify' ///'
        Vorlagenliste.OpenContextMenu
        sleep 1
        hMenuSelectNr (1)
            sleep 1
            Kontext
            '///+ switch to tabpage 'Font' ///'
            Messagebox.SetPage TabFont
            kontext "TabFont"
            Kontext
            '///+ switch to tabpage 'Font Effect' ///'
            Messagebox.SetPage TabFontEffects
            kontext "TabFontEffects"
            Kontext
            '///+ switch to tabpage 'indents & spacing' ///'
            Messagebox.SetPage TabEinzuegeUndAbstaende
            kontext "TabEinzuegeUndAbstaende"
            sleep 1
            '///+ cancel dialog 'Notes' ///'
            TabEinzuegeUndAbstaende.Cancel
        sleep 1
        Kontext "Gestalter"
        sleep 1

        '/// select entry 'Outline 1' from list in Stylist ///'
        Vorlagenliste.Select Num5
        '///+ open kontext menue select 1st entry from top: 'Modify' ///'
        sleep 1
        Vorlagenliste.OpenContextMenu
        sleep 1
        hMenuSelectNr (1)
            sleep 1
            Kontext
            '///+ switch to tabpage 'Font' ///'
            Messagebox.SetPage TabFont
            kontext "TabFont"
            Kontext
            '///+ switch to tabpage 'Font Effect' ///'
            Messagebox.SetPage TabFontEffects
            kontext "TabFontEffects"
            Kontext
            '///+ switch to tabpage 'indents & spacing' ///'
            Messagebox.SetPage TabEinzuegeUndAbstaende
            kontext "TabEinzuegeUndAbstaende"
            sleep 1
            Kontext
            '/// switch to tabpage 'Bullets' ///'
            Messagebox.SetPage TabBullet
            Kontext "TabBullet"
            sleep 1
            Kontext
            '/// switch to tabpage 'Numbering Type' ///'
            Messagebox.SetPage TabNumerierungsart
            Kontext "TabNumerierungsart"
            sleep 1
            Kontext
            '/// switch to tabpage 'Graphics' ///'
            Messagebox.SetPage TabGrafiken
            Kontext "TabGrafiken"
            sleep 1
            Kontext
            '/// switch to tabpage 'Customize' ///'
            Messagebox.SetPage TabOptionenNumerierung
            Kontext "TabOptionenNumerierung"
            sleep 1
            '///+ cancel dialog 'Outline 1' ///'
            TabOptionenNumerierung.Cancel
        sleep 1
    endif
    '/// close Stylist with key 'F11' ///'
    hTypeKeys "<F11>"
    sleep 1
    Kontext "Gestalter"
    if Gestalter.Exists Then Warnlog "- Stylist could not be closed pressing F11"
    '/// close application ///'
    Call hCloseDocument
endcase

sub hWalkTheStyles (optional a as integer,optional b as integer)
    ' TODO: TBO DialogTest should go to Recource test!
    dim i as integer

    if isMissing (a) then a=1
    if isMissing (b) then b=2
    i=1
    if a <= i AND i <= b then
    Kontext
    '/// switch to tabpage 'Line' ///'
    Messagebox.SetPage TabLinie
      kontext "TabLinie"
      Call DialogTest ( TabLinie )
    Kontext
    '/// switch to tabpage 'Area' ///'
    Messagebox.SetPage TabArea
      kontext "TabArea"
      Call DialogTest ( TabArea, 1 )
    '/// select radio button 'none' ///'
      FillOptions.Select 1  '  NoFill.Check
      Call DialogTest ( TabArea, 1 )
    '/// select radio button 'color' ///'
      FillOptions.Select 2
      Call DialogTest ( TabArea, 2 )
    '/// select radio button 'gradient' ///'
      FillOptions.Select 3
      Call DialogTest ( TabArea, 3 )
    '/// select radio button 'hatching' ///'
      FillOptions.Select 4
      Call DialogTest ( TabArea, 4 )
    '/// select radio button 'bitmap' ///'
      FillOptions.Select 5
      Call DialogTest ( TabArea, 5 )
    Kontext
    '/// switch to tabpage 'shadowing' ///'
    Messagebox.SetPage TabSchatten
      kontext "TabSchatten"
    '/// check 'use shadow' ///'
      Anzeigen.check
      Call DialogTest ( TabSchatten )
    Kontext
    '/// switch to tabpage 'Transparency' ///'
    Messagebox.SetPage TabTransparenz
      kontext "TabTransparenz"
    '/// check 'No transparency' ///'
      KeineTransparenz.Check
    '/// check 'Transparency' ///'
      LineareTransparenz.Check
    '/// check 'Gradient' ///'
      Transparenzverlauf.Check
    Kontext
    '/// switch to tabpage 'Font' ///'
    Messagebox.SetPage TabFont
      kontext "TabFont"
      Call DialogTest ( TabFont )
    Kontext
    '/// switch to tabpage 'Font Effect' ///'
    Messagebox.SetPage TabFontEffects
      kontext "TabFontEffects"
    Kontext
    '/// switch to tabpage 'indents & spacing' ///'
    Messagebox.SetPage TabEinzuegeUndAbstaende
      kontext "TabEinzuegeUndAbstaende"
      Call DialogTest ( TabEinzuegeUndAbstaende )
    endif
    i=2
    if a <= i AND i <= b then
    Kontext
    '/// switch to tabpage 'Organize' ///'
    Messagebox.SetPage TabVerwalten
      kontext "TabVerwalten"
      Call DialogTest ( TabVerwalten )
    Kontext
    '/// switch to tabpage 'text' ///'
    Messagebox.SetPage TabText
      Kontext "TabText"
      Call DialogTest ( TabText )
    Kontext
    '/// switch to tabpage 'text animation' ///'
    Messagebox.SetPage TabLauftext
      Kontext "TabLauftext"
      Call DialogTest ( TabLauftext )
    Kontext
    '/// switch to tabpage 'dimension' ///'
    Messagebox.SetPage TabBemassung
      Kontext "TabBemassung"
      Call DialogTest ( TabBemassung )
    Kontext
    '/// switch to tabpage 'connector' ///'
    Messagebox.setpage TabVerbinder
      Kontext "TabVerbinder"
      Call Dialogtest ( TabVerbinder )
    Kontext
    '/// switch to tabpage 'alignment' ///'
    Messagebox.setpage TabAusrichtungAbsatz
      Kontext "TabAusrichtungAbsatz"
         Links.Check
         Rechts.Check
         Zentriert.Check
         Blocksatz.Check
    Kontext
    '/// switch to tabpage 'Tabs' ///'
    Messagebox.setpage TabTabulator
      Kontext "TabTabulator"
    '/// click 'new' ///'
         Neu.click
    '///                                    ' MAYBE CHECK COUNT OF THIS ?? Position               svx:MetricBox:RID_SVXPAGE_TABULATOR:ED_TABPOS ///'
    '/// click 'delete all' ///'
         AlleLoeschen.click
    '/// click 'new' ///'
         Neu.click
    '/// click 'delete' ///'
         Loeschen.click
    endif
    i=3
    if a <= i AND i <= b then
      Kontext
    '/// switch to tabpage 'Bullets' ///'
      Messagebox.SetPage TabBullet
      Kontext "TabBullet"
      sleep 1
      Call DialogTest (TabBullet)
      sleep 1
      Kontext
    '/// switch to tabpage 'Numbering Type' ///'
      Messagebox.SetPage TabNumerierungsart
      Kontext "TabNumerierungsart"
      sleep 1
      Call DialogTest (TabNumerierungsart)
      sleep 1
      Kontext
    '/// switch to tabpage 'Graphics' ///'
      Messagebox.SetPage TabGrafiken
      Kontext "TabGrafiken"
      sleep 1
      Call DialogTest (TabGrafiken)
      sleep 1
      Kontext
    '/// switch to tabpage 'Customize' ///'
      Messagebox.SetPage TabOptionenNumerierung
      Kontext "TabOptionenNumerierung"
      sleep 1
      Call DialogTest (TabOptionenNumerierung)
      sleep 1
    endif
end sub

testcase tFormatStylist
    Dim sTemp as String
    dim sSettings(20,3) ' control_name; control_type; value
    dim i as integer
    dim abctemp

    '/// open application ///'
    Call  hNewDocument
    '/// create a textframe with content ///'
    sleep 5

    hTextrahmenErstellen ("I hate mondays...",20,20,80,40)
    sleep 1
    '/// open stylist with: Format->Stylist ///'
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
    '/// open kontext menue select 1st entry from top: 'New' ///'
    sleep 1
    Vorlagenliste.OpenContextMenu
    sleep 1
    hMenuSelectNr 1
    sleep 1
    Kontext
    if Messagebox.exists (5) then
        '/// switch to tabpage Organizer ///'
        Messagebox.SetPage TabVerwalten
        Kontext "TabVerwalten"
        VorlagenName.SetText "1Newtheme"
        sTemp = VorlagenName.getText
        VerknuepftMit.getSelText
        Bereich.getSelText
        '/// close dialog 'Graphics Styles' ///'
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
        '/// switch to tabpage Organizer ///'
        Messagebox.SetPage TabVerwalten
        Kontext "TabVerwalten"
        VorlagenName.setText("1Test")
        '/// close dialog 'Graphics Styles' ///'
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
       '/// Closing stylist ///'
       hTypekeys "<F11>"
       Kontext "Stylist"
       if (Stylist.Exists) then
          ErrorLog "The Stylist should be closed now."
       endif
    endif
    Call hCloseDocument
endcase

function hSelectInList (window, sEntry as String) as Boolean
'/// alternativ method to 'hDoubleClickInList' (without mouse) ///'
'///+ window: name of list ///'
'///+ sEntry: string to find in list ///'
'///+ ReturnValue: if found: TRUE; else FALSE ///'
    Dim i as Integer
    Dim sTemp as String
    Dim sLastTemp as String

    '/// go through list from bottom and stop on the entry sEntry ///'
    window.TypeKeys "<End>"
    sTemp = ""
    do
        sLastTemp = sTemp
        sTemp = window.GetText
        window.TypeKeys "<Up>"
    loop while ((sEntry <> sTemp) AND (sLastTemp <> sTemp))
    '/// press key [Return] ///'
    if (sEntry = sTemp) then
        window.TypeKeys "<Return>"
        hSelectInList = TRUE
    else
        hSelectInList = FALSE
    endif
end function

function hWalkTheStyles2 (atemp)
'function hWalkTheStyles2 (bSet as boolean, aSettings(), atemp as variant) as string
    dim i as integer
    dim x as integer
    dim itemp
    dim bSet
    dim aSettings(5,5)
'    dim atemp

    '/// Organizer ///'
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
'/// Line ///'
    i=2
    Kontext
    Messagebox.SetPage TabLinie
        kontext "TabLinie"
        'Context: *Line; Line Styles; Arrow Styles
'/// Area ///'
    i=3
    Kontext
    Messagebox.SetPage TabArea
        kontext "TabArea"
        'Context: *Area; *Shadow; Transparency; Colors; Gradients; Hatching; Bitmaps
'/// Shadowing ///'
    i=4
    Kontext
    Messagebox.SetPage TabSchatten
        kontext "TabSchatten"
'/// Transparency ///'
    i=5
    Kontext
    Messagebox.SetPage TabTransparenz
        kontext "TabTransparenz"
'/// Font ///'
    i=6
    Kontext
    Messagebox.SetPage TabFont
        kontext "TabFont"
        'Context: *Font; *Font Effect; Position
'/// Font Effect ///'
    i=7
    Kontext
    Messagebox.SetPage TabFontEffects
        kontext "TabFontEffects"
'/// Indents & Spacing ///'
    i=8
    Kontext
    Messagebox.SetPage TabEinzuegeUndAbstaende
        kontext "TabEinzuegeUndAbstaende"
        'Context: *Indents & Spacing; *Alignment; *Tabs
'/// Text ///'
    i=9
    Kontext
    Messagebox.SetPage TabText
        Kontext "TabText"
        'Context: *Text; *Text Animation
'/// Text Animation ///'
    i=10
    Kontext
    Messagebox.SetPage TabLauftext
        Kontext "TabLauftext"
'/// Dimensioning ///'
    i=11
    Kontext
    Messagebox.SetPage TabBemassung
        Kontext "TabBemassung"
'/// Connector ///'
    i=12
    Kontext
    Messagebox.setpage TabVerbinder
        Kontext "TabVerbinder"
'/// Alignment ///'
    i=13
    Kontext
    Messagebox.setpage TabAusrichtungAbsatz
        Kontext "TabAusrichtungAbsatz"
'/// Tabs ///'
    i=14
    Kontext
    Messagebox.setpage TabTabulator
        Kontext "TabTabulator"

'    '/// switch to tabpage 'Bullets' ///'
'      Messagebox.SetPage TabBullet
'      Kontext "TabBullet"
'      sleep 1
'      Call DialogTest (TabBullet)
'      sleep 1
'      Kontext
'    '/// switch to tabpage 'Numbering Type' ///'
'      Messagebox.SetPage TabNumerierungsart
'      Kontext "TabNumerierungsart"
'      sleep 1
'      Call DialogTest (TabNumerierungsart)
'      sleep 1
'      Kontext
'    '/// switch to tabpage 'Graphics' ///'
'      Messagebox.SetPage TabGrafiken
'      Kontext "TabGrafiken"
'      sleep 1
'      Call DialogTest (TabGrafiken)
'      sleep 1
'      Kontext
'    '/// switch to tabpage 'Customize' ///'
'      Messagebox.SetPage TabOptionenNumerierung
'      Kontext "TabOptionenNumerierung"
'      sleep 1
'      Call DialogTest (TabOptionenNumerierung)
'      sleep 1
end function


