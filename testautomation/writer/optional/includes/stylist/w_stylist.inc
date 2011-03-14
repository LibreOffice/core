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
'* owner : helge.delfs@oracle.com
'*
'* short description : Test common stylist functions and assigning styles
'*
'************************************************************************
'*
' #1 tFormatStylist1
' #1 tFormatStylist2
' #1 tFormatStylist3
' #1 tFormatStylist4
' #1 tFormatStylist5
' #1 tFormatStylist6
' #1 tFormatStylist7
' #1 tAssigningStyles1
' #1 tAssigningStyles2
' #1 tAssigningStyles3
' #1 tAssigningStyles4
' #1 tAssigningStyles5
' #1 tAssigningStyles6
' #1 tAssigningStyles7
' #1 tKeyboardShortcutToStyles
'*
'\***********************************************************************

sub w_stylist
   printLog Chr(13) + "---------   Level 1 - Format / Stylist   ----------"

   Call tFormatStylist1                ' all methods on stylist
   Call tFormatStylist2                ' interacting between documents
   Call tFormatStylist3                ' show all templates
   Call tFormatStylist4                ' search/create templates
   Call tFormatStylist5                ' select templet in stylist
   Call tFormatStylist6                ' new template from selection
   Call tFormatStylist7                ' new template from selection, modified via ContextMenu.
   Call tAssigningStyles1
   Call tAssigningStyles2
   Call tAssigningStyles3
   Call tAssigningStyles4
   Call tAssigningStyles5
   Call tAssigningStyles6
   Call tAssigningStyles7
   Call tKeyboardShortcutToStyles

end sub

' **********************************************************************

testcase tFormatStylist1
    Dim xPos as long
    Dim yPos as long
    Dim xSize%
    Dim ySize%

    printlog "- Format / Stylist : all methods with Navigator as window"
    printlog "New writerdocument"
    Call hNewDocument
    printlog "  - Call Stylist with F11"
    printlog "Call stylist with <F11> aand proof all windowmethods at stylist"
    if lcase(gPlatform) = "osx" then
        call hTypekeys "<mod1 t>"
    else
        Call wTypeKeys "<F11>"
    end if
    Kontext "Stylist"
    if Stylist.NotExists then
        Warnlog "No Stylist with <F11>"
        FormatStylist
    end if
    xPos = Stylist.GetPosX
    yPos = Stylist.GetPosY
    xSize% = Stylist.GetSizeX
    ySize% = Stylist.GetSizeY

    printlog "  - move Stylist"
    Stylist.Move 10, 10
    Sleep 1
    Stylist.Move 300, 400
    Sleep 1
    Stylist.Move 590, 790
    Sleep 1
    Stylist.Move xPos, yPos
    Sleep 1

    printlog "  - size Stylist"
    try
        Stylist.Size 300, 400
    catch
        Warnlog "Sizen Stylist doesn't work"
    endcatch    
    Sleep 1
    try
        Stylist.Size xSize%, ySize%
    catch
        Warnlog "Sizen Stylist doesn't work"
    endcatch
    Sleep 1

    printlog "  - close Stylist and reopen ( has to be the same size )"
    xPos = Stylist.GetPosX
    yPos = Stylist.GetPosY
    xSize% = Stylist.GetSizeX
    ySize% = Stylist.GetSizeY
    FormatStylist
    Sleep 1
    FormatStylist
    if xPos <> Stylist.GetPosX then Warnlog "x-Pos. has been changed : Not -> "+ xPos + "  but -> "+ Stylist.GetPosX
    if yPos <> Stylist.GetPosY then Warnlog "y-Pos. has been changed : Not -> "+ yPos + "  but -> "+ Stylist.GetPosY
    if xSize% <> Stylist.GetSizeX then Warnlog "x-Size has been changed : Not -> "+ xSize% + "  but -> "+ Stylist.GetSizeX
    if ySize% <> Stylist.GetSizeY then Warnlog "y-Size has been changed : Not -> "+ ySize% + "  but -> "+ Stylist.GetSizeY

    printlog "  - dock Stylist"
    Stylist.Dock
    Sleep 1
    if NOT Stylist.IsDocked then Warnlog "Stylist has not been docked in"
    Stylist.Undock
    Sleep 1

    Stylist.Close
    printlog "Close document"
    Call hCloseDocument

endcase

' **********************************************************************

testcase tFormatStylist2
    Dim xPos%
    Dim yPos%
    Dim xSize%
    Dim ySize%

    printlog "- Format/Stylist : Methods from Stylist as Window between more than one document"
    printlog " New document"
    Call hNewDocument
    printlog "check all methods of Stylist as Window between more than one document"
    Call wTypeKeys "Test the stylist functions"
    printlog "  - Call Stylist in first document and check in a second document"
    Sleep 2
    Kontext "Stylist"
    If Not Stylist.Exists then FormatStylist
    Sleep 2
    Call hNewDocument
    Call wTypeKeys "Test the stylist functions"
    Sleep 2
    Kontext "Stylist"
    if Stylist.NotExists then Warnlog "No Stylist in new document"

    printlog "  - Close all documents and reopen"
    Sleep 2
    Call hCloseDocument
    Sleep 2
    Call hCloseDocument
    Sleep 2
    Call hNewDocument
    Sleep 2
    Call wTypeKeys "Test the stylist functions"
    Kontext "Stylist"
    if Stylist.NotExists then Warnlog "No Stylist in a new document"

    printlog "  - Dock Stylist in first document and check in a new if docked"
    Sleep 2
    Stylist.Dock
    Sleep 1
    Call hNewDocument
    Call wTypeKeys "Test the stylist functions"
    Kontext "Stylist"
    if NOT Stylist.IsDocked then Warnlog "Stylist is not docked in new document"
    
    printlog "  - Close all documents, reopen one and check docked Stylist"
    Call hCloseDocument
    Sleep 2
    Call hCloseDocument
    Sleep 2
    Call hNewDocument
    Kontext "Stylist"
    if NOT Stylist.IsDocked then Warnlog "Stylist is not docked in new document"
    
    printlog "  - Close Stylist with <F11>"
    Stylist.Undock
    Sleep 1
    if lcase(gPlatform) = "osx" then
        call hTypekeys "<mod1 t>"
    else
        Call wTypeKeys "<F11>"
    end if
    Kontext "Stylist"
    if NOT Stylist.NotExists then
        Warnlog "Stylist is still open"
        Stylist.Close
    end if
    printlog "Close document"
    Call hCloseDocument
    
endcase

' **********************************************************************

testcase tFormatStylist3
    Dim i as Integer
    Dim AllStyles as integer
    Dim j as integer
    Dim RemString as string
    printlog "- Format / Stylist : select all Style properties"

    Call hNewDocument
    printlog " New writerdocument"
    FormatStylist
    printlog "Select all style properties in stylist"
    Kontext "Stylist"
    if Stylist.NotExists then FormatStylist
    printlog "   - Paragraph Styles"
    Absatzvorlagen.Click

    for i=1 to Gruppenliste.GetItemCount
        Gruppenliste.Select i
        wait 500
        if i = 2 then  ' All Styles
            Vorlagenliste.TypeKeys "<Home>"
            AllStyles = 1
            For j = 1 to 200
                RemString = Vorlagenliste.Gettext
                'Include this, if you want to see the styles in writerdocument
                'SetClipboard Vorlagenliste.Gettext
                'Kontext "DocumentWriter"
                'DocumentWriter.TypeKeys GetClipboardtext + "<Return>"
                'Kontext "Stylist"
                Vorlagenliste.TypeKeys "<Down>"
                if Vorlagenliste.Gettext <> RemString then
                    AllStyles = AllStyles + 1
                else
                    j = 202
                    if AllStyles <> "122" then
                        Select Case iSprache
                            Case 07
                                warnlog "Double styles in stylist"
                            Case 55
                                warnlog "Missing Style in Stylist"
                            Case 31
                                warnlog "Missing Style in Stylist"
                            Case else
                                Warnlog "There are " + AllStyles + " styles in this category and not 122!"
                        end select
                    else
                        printlog "There are " + AllStyles + " styles in this category!"
                    end if
                end if
            next j
        end if
        Sleep 1
    next i

    Zeichenvorlagen.Click
    Sleep 2
    printlog "   - Chapter Styles"
    for i=1 to Gruppenliste.GetItemCount
        Gruppenliste.Select i
        AllStyles = AllStyles + 1
        Sleep 1
    next i

    Rahmenvorlagen.Click
    Sleep 2
    printlog "   - Frame Styles"
    for i=1 to Gruppenliste.GetItemCount
        Gruppenliste.Select i
        AllStyles = AllStyles + 1
        Sleep 1
    next i

    Seitenvorlagen.Click
    Sleep 2
    printlog "   - Page Styles"
    for i=1 to Gruppenliste.GetItemCount
        Gruppenliste.Select i
        AllStyles = AllStyles + 1
        Sleep 1
    next i
    Stylist.Close
    printlog "Close document"
    Call hCloseDocument
endcase

' **********************************************************************

testcase tFormatStylist4
    Dim i as Integer
    Dim AlterWert$
    Dim NeuerWert$

    printlog "- Format / Stylist : check created Styles"
    printlog "New writerdocument"
    Call hNewDocument
    printlog "   - create Paragraph Style in Stylist"
    Call wStyleCreate ( "ATesttool", "Paragraph", true )
    Kontext "Stylist"
    printlog "Check if Style has been added in stylist"
    if wStyleSelect ( "ATesttool", "Paragraph" ) = false then Warnlog "Paragraph Style has not been found"

    printlog "   - Create Character Style in Stylist"
    Call wStyleCreate ( "BTesttool", "Character", true )
    if wStyleSelect ( "BTesttool", "Character" ) = false then Warnlog "Character Style has not been found!"
    printlog "   - Create Frame Style in Stylist"
    Call wStyleCreate ( "CTesttool", "Frame", true )
    if wStyleSelect ( "CTesttool", "Frame" ) = false then Warnlog "Frame Style has not been found!"

    printlog "   - Create Page Style in Stylist"
    Call wStyleCreate ( "DTesttool", "Page", true )
    if wStyleSelect ( "DTesttool", "Page" ) = false then Warnlog "Page Style has not been found"

    printlog "   - creating Styles should not have been affecting to new documents"
    ' To avoid focus problems on unix first close stylist
    Kontext "Stylist"
    if Stylist.Exists then FormatStylist   
    Call hNewDocument
    Kontext "Stylist"
    if Not Stylist.Exists then FormatStylist

    if wStyleSelect ( "ATesttool", "Paragraph" ) = true then Warnlog "Paragraph Style has been found in new document"
    if wStyleSelect ( "BTesttool", "Character" ) = true then Warnlog "Character Style has been found in new document"
    'if wStyleSelect ( "CTesttool", "Frame" ) = true then Warnlog "Frame Style has been found in new document"
    if wStyleSelect ( "DTesttool", "Page" ) = true then Warnlog "Page Style has been found in new document"

    Call hCloseDocument
    printlog "Close document"
    Call hCloseDocument
endcase

' **********************************************************************

testcase tFormatStylist5
    Dim SuchMal as string, i as integer
    Dim bStyleFound as boolean

    printlog "- Format / Stylist : select Style"
    printlog "New writerdocument"
    if bAsianLan = True then
        printlog "- Not in Asian languages!"
        goto endsub
    end if
    Call hNewDocument
    Call wTypeKeys "Dies ist ein formatierter Text <Shift Home>"
    printlog "  - format Text"
    FormatCharacter
    Kontext
    Active.SetPage TabFontEffects
    Kontext "TabFontEffects"
    StrikeThrough.Select 2
    Outline.Check
    TabFontEffects.OK

    printlog "  - adjust to Standard-Format through the Stylist"
    printlog "adjust to Standard-Format through the Stylist"
    FormatStylist
    Kontext "Stylist"
    if Stylist.NotExists then FormatStylist
    'Get name for 'default' template
    Kontext "TextObjectbar"
    SuchMal = Vorlage.GetSelText
    printlog "Using " & SuchMal & " as 'default'" 
    Kontext "Stylist"
   
    Vorlagenliste.TypeKeys "<Home>"
    bStyleFound = false
    for i=1 to Vorlagenliste.GetItemCount
        Vorlagenliste.Select i
        if Vorlagenliste.GetText = Suchmal then
            Vorlagenliste.TypeKeys "<Return>"
            bStyleFound = true
            exit for
        end if
    next i
    Stylist.Close
    
    if bStyleFound = false then
        Warnlog "Style: " & Suchmal & " has not been found in Stylist"
        Call hCloseDocument
        goto endsub
    end if      

    printlog "  - check format"
    FormatCharacter
    Kontext
    Active.SetPage TabFontEffects
    Kontext "TabFontEffects"
    if StrikeThrough.GetSelIndex = 2 then Warnlog "Wrong entry selected in StrikeThrough"
    if Outline.IsChecked then Warnlog "Format Outline is still checked"
    TabFontEffects.OK

    printlog "Close document"
    Call hCloseDocument
endcase

' **********************************************************************

testcase tFormatStylist6
    Dim i as integer

    printlog "- Format / Stylist : new Style from selection"
    printlog "New writerdocument"
    Call hNewDocument
    Call wTypeKeys "Creating a style from selection <Return><Up><Shift End>"
    printlog "create a new style from selection"
    FormatCharacter
        Kontext
        Active.SetPage TabFontEffects
        Kontext "TabFontEffects"
            Shadow.Check
            StrikeThrough.Select 2
            Overline.Select 2
            OverlineColor.Select 6
            Outline.Check
            Effects.Select 3
            Blinking.Check
        Kontext
        Active.Setpage TabHyperlinkZeichen
        Kontext "TabHyperlinkZeichen"
            URLName.SetText "www.mikeportnoy.com"
    TabHyperlinkZeichen.OK

    printlog "  - Create Character Style from selection"
    Kontext "Stylist"
    if not Stylist.Exists then FormatStylist
        Kontext "Stylist"
        TBGestalter.Click ZeichenVorlagen
        Sleep 1

    AusSelektion.OpenMenu
    Sleep 1
    hMenuSelectNr(1)
    Sleep 1

    Kontext "VorlageErzeugen"
    if VorlageErzeugen.Exists then
        VorlagenName.SetText "Testtool"
        VorlageErzeugen.OK
    else
        Warnlog "Creating Style did not work. Dialog not up!"
    end if

    printlog "   - assign new Character Style to a new text"
    wTypeKeys "<Home><Down>",3
    wTypeKeys "This is some new text <Return><Up><Up><Shift End>"
    Kontext "Stylist"
        TBGestalter.Click ZeichenVorlagen
        if Vorlagenliste.GetSeltext <> "Testtool" then  'Go find the entry.
            Vorlagenliste.TypeKeys "<Home>"
        For i = 1 to 100
                Vorlagenliste.TypeKeys "<Down>"
                if Vorlagenliste.GetSeltext = "Testtool" then  'Found it!
                    i = 102
                end if
            next i
        end if
        Vorlagenliste.TypeKeys "<Return>"  'Assign to text.
    Stylist.Close
    if i = 101 then
        Warnlog "Created Style could not be found in Stylist!"
    else
        Printlog "   - check Style"
        FormatCharacter
        Kontext
        Active.SetPage TabFontEffects
        Kontext "TabFontEffects"
            if NOT Shadow.IsChecked then Warnlog "Shadow is not checked anymore"
            if StrikeThrough.GetSelIndex <> 2 then Warnlog "Wrong entry selection in StrikeThrough"
            if Overline.GetSelIndex <> 2 then Warnlog "Overline is not checked anymore"
        if OverlineColor.GetSelIndex <> 6 then Warnlog "Wrong Overline Color selected"
            if NOT Outline.IsChecked then Warnlog "Outline is not checked anymore"
            if Effects.GetSelIndex <> 3 then Warnlog "Wrong effect selected"
            if NOT Blinking.IsChecked then Warnlog "Blinking is not checked anymore"
        Kontext
        Active.Setpage TabHyperlinkZeichen
        Kontext "TabHyperlinkZeichen"
            if URLName.GetText <> "http://www.mikeportnoy.com/" then Warnlog "TabHyperlink: URL is wrong " + URLName.Gettext
        TabHyperlinkZeichen.OK
    end if

    printlog "Close document"
    Call hCloseDocument
endcase

' **********************************************************************

testcase tFormatStylist7
    Dim i as integer

    printlog "- Format / Stylist : new Style from selection, then modified via the ContextMenu."
    printlog "New writerdocument"
    Call hNewDocument
    Call wTypeKeys "Creating a style from selection <Return><Up><Shift End>"
    printlog "create a new style from selection"

    printlog "  - Create Character Style from selection"
    Kontext "Stylist"
    if not Stylist.Exists then FormatStylist
        Kontext "Stylist"
        TBGestalter.Click ZeichenVorlagen
        Sleep 1

    AusSelektion.OpenMenu
    Sleep 1
    hMenuSelectNr(1)
    Sleep 1

    Kontext "VorlageErzeugen"
    if VorlageErzeugen.Exists then
        VorlagenName.SetText "Testtool"
        VorlageErzeugen.OK
    else
        Warnlog "Creating Style did not work. Dialog not up!"
    end if

    printlog " Open ContextMenu over the 'Testtool' Style."
    Kontext "Stylist"
    if not Stylist.Exists then FormatStylist
    Kontext "Stylist"
        TBGestalter.Click ZeichenVorlagen
        if Vorlagenliste.GetSeltext <> "Testtool" then
            Vorlagenliste.TypeKeys "<Home>"
            For i = 1 to 100
                Vorlagenliste.TypeKeys "<Down>"
                if Vorlagenliste.GetSeltext = "Testtool" then
                    i = 102
                end if
            next i
        if i = 101 then
           warnlog "No Testtool -Stylist found. Check if new stylists can be created. Test ends."
           goto endsub
        end if
        end if
        Vorlagenliste.TypeKeys "<SHIFT F10>"  'Open ContextMenu
        Sleep 1
        printlog "  Select 'Modify'"
        Call hMenuSelectNr(2)  'Modify
        Sleep 1

    printlog "  Goto Tabpage 'Font Effects' and change some settings."
    Kontext
    Active.SetPage TabFontEffects
    Kontext "TabFontEffects"
        Shadow.Check
        StrikeThrough.Select 2
        Overline.Select 2
        OverlineColor.Select 6
        Outline.Check
        Effects.Select 3
        Blinking.Check
    TabFontEffects.OK

    printlog "   - assign new Character Style to a new text"
    wTypeKeys "<Home><Down>",3
    wTypeKeys "This is some new text <Return><Up><Up><Shift End>"
    Kontext "Stylist"
        TBGestalter.Click ZeichenVorlagen
        
        if Vorlagenliste.GetSeltext <> "Testtool" then
        Vorlagenliste.TypeKeys "<Home>"
            For i = 1 to 100
                Vorlagenliste.TypeKeys "<Down>"
                if Vorlagenliste.GetSeltext = "Testtool" then 'Found it!
                    i = 102
                end if
            next i
        end if
        Vorlagenliste.TypeKeys "<Return>"  'Assign Style to Text.
    Stylist.Close
    if i = 101 then
        Warnlog "Created Style could not be found in Stylist!"
    else
        Printlog "   - check Style"
        FormatCharacter
        Kontext
        Active.SetPage TabFontEffects
        Kontext "TabFontEffects"
            if NOT Shadow.IsChecked then Warnlog "Shadow is not checked anymore"
            if StrikeThrough.GetSelIndex <> 2 then Warnlog "Wrong entry selection in StrikeThrough"
            if Overline.GetSelIndex <> 2 then Warnlog "Overline is not checked anymore"
            if OverlineColor.GetSelIndex <> 6 then Warnlog "Wrong Overline Color selected"
            if NOT Outline.IsChecked then Warnlog "Outline is not checked anymore"
            if Effects.GetSelIndex <> 3 then Warnlog "Wrong effect selected"
            if NOT Blinking.IsChecked then Warnlog "Blinking is not checked anymore"
        TabFontEffects.OK
    end if

    printlog "Close document"
    Call hCloseDocument
endcase

' **********************************************************************

testcase tAssigningStyles1
    Dim i as integer
    Dim sStyles() as string
    Dim sTemp as string
    Dim j as integer
    Dim bStyleInList as boolean

    printlog "UI and Strings check"
    Call hNewDocument
    printlog "Open a new writerdocument"
    printlog "Check if the following strings and only those strings are present:"
    printlog "Clear formatting"
    printlog "Heading 1"
    printlog "Heading 2"
    printlog "Heading 3"
    printlog "Default"
    printlog "Text body"
    printlog "More..."
    Select Case iSprache
        Case 01 : sStyles = Array("Clear formatting", "Default", "Heading 1", "Heading 2", "Heading 3", "Text body", "More...")
        Case 07 : sStyles = Array("Отменить форматирование", "Базовый", "Заголовок 1", "Заголовок 2", "Заголовок 3", "О�?новной тек�?т", "Дополнительно...")
        Case 31 : sStyles = Array("Opmaak wissen", "Kop 1", "Kop 2", "Kop 3", "Standaard", "Tekstblok", "Meer...")
        Case 33 : sStyles = Array("Effacer le formatage", "Corps de texte", "Par défaut", "Titre 1", "Titre 2", "Titre 3", "Autres...")
        Case 34 : sStyles = Array("Borrar formato", "Cuerpo de texto", "Encabezado 1", "Encabezado 2", "Encabezado 3", "Predeterminado", "Más...")
        Case 36 : sStyles = Array("Formázás törlése", "Alapértelmezett", "Címsor 1", "Címsor 2", "Címsor 3", "Szövegtörzs", "További...")
        Case 39 : sStyles = Array("Cancella formattazione", "Corpo testo", "Intestazione 1", "Intestazione 2", "Intestazione 3", "Standard", "Extra...")
        Case 46 : sStyles = Array("Radera formatering", "Brödtext", "Överskrift 1", "Överskrift 2", "Överskrift 3", "Standard", "Mer...")
        Case 48 : sStyles = Array("Wyczyść formatowanie", "Domyślnie", "Nagłówek 1", "Nagłówek 2", "Nagłówek 3", "Treść tekstu", "Więcej...")
        Case 49 : sStyles = Array("Formatierung löschen", "Standard", "Textkörper", "Überschrift 1", "Überschrift 2", "Überschrift 3", "Weitere...")
        Case 55 : sStyles = Array("Limpar formatação", "Corpo do texto", "Padrão", "Título 1", "Título 2", "Título 3", "Mais...")
        Case 81 : sStyles = Array("書�?設定�?�解除", "本文", "標準", "見出�?� 1", "見出�?� 2", "見出�?� 3", "詳細...")
        Case 82 : sStyles = Array("서�? 지우기", "기본값", "제목 1", "제목 2", "제목 3", "�?스트 본문", "�?세히...")
        Case 86 : sStyles = Array("清除格�?", "默认", "标题 1", "标题 2", "标题 3", "正文", "更多(~M)...")
        Case 88 : sStyles = Array("清除格�?", "內文", "標題 1", "標題 2", "標題 3", "�?設", "更多...")
        Case 90 : sStyles = Array("Biçimlendirmeyi temizle", "Başlık 1", "Başlık 2", "Başlık 3", "Metin gövdesi", "Varsayılan", "Diğer...")
        Case else : Warnlog "Current language not supported. Please add styles to list!"
                    Kontext "Textobjectbar"
                    printlog "- Current Styles: "
                    For i = 1 to Vorlage.GetItemCount
                        Vorlage.Select i
                        printlog Vorlage.GetSelText
                    next i
                    Call hCloseDocument
                    exit sub
    end select

    'Check number of entrys....have to be seven
    Kontext "Textobjectbar"
    if Vorlage.GetItemCount <> 7 then
        Select case iSprache
            case 82     : warnlog "Korean: Number of styles in styleslistbox wrong"
            case else   : Warnlog "Not 7 entrys in Styles & Formatting combobox but: " & Vorlage.GetItemCount
        end select
    end if

    For i = 1 to Vorlage.GetItemCount
        Vorlage.Select i
        sTemp = Vorlage.GetSelText
        printlog "- check style: " & sTemp
        bStyleInList = false
        For j = 0 to 6
            if sStyles(j) = sTemp then
                bStyleInList = true
            end if
        next j
        
        if bStyleInList = true then
            printlog "-  style " & sTemp & " found in list"
        else
            if iSprache <> 82 then Warnlog "-  style " & sTemp & " wasn't found in list!"
        end if
    next i

    Call hCloseDocument
endcase

' **********************************************************************

testcase tAssigningStyles2
    Dim i as integer
    Dim sTemp as string

    printlog "- Apply Style"
    printlog "New writerdocument"
    Call hNewDocument
    printlog "Insert dummytext"
    if wBlindtextEinfuegen = false then
        Call hCloseDocument
        exit sub
    end if
    
    printlog "Select whole dummytext"
    Call wTypeKeys "<Mod1 A>"
    printlog "Apply style 'Heading 1' through Styles & Formatting combobox"
    Kontext "Textobjectbar"
    for i = 1 to Vorlage.GetItemCount
        Vorlage.Select i
        sTemp = Vorlage.GetSelText
        if Instr(sTemp, "1") then
            Vorlage.TypeKeys "<Return>"            
            exit for
        end if
    next i

    printlog "Check if style has been applied to paragraph"
    Call wTypeKeys "<Mod1 Home><End>"
    Call wTypeKeys "<Left>", 2
    
    Kontext "Textobjectbar"
    if Vorlage.GetSeltext <> sTemp then
        Warnlog "Seems style has not been completely applied to paragraph!"
    end if

    Call hCloseDocument
endcase

' **********************************************************************

testcase tAssigningStyles3

    printlog "- Create a new style"
    printlog "Open a new writerdocument"
    Call hNewDocument
    printlog "Insert a dummy text"
    if wBlindtextEinfuegen = false then
        Call hCloseDocument
        exit sub
    end if
    
    printlog "Select the whole dummy text"
    Call wTypeKeys "<Mod1 A>"
    printlog "Format / Character" 
    FormatCharacter
    Kontext
    Active.Setpage TabFont
    Kontext "TabFont" 
    printlog "Apply first font in list and select 2nd Typeface"
    if gAsianSup = True then
        FontWest.Select 1
        StyleWest.Select 2
    else
        Font.Select 1
        Style.Select 1
    end if
    TabFont.Ok
    
    printlog "Enter 'MyStyle' in Stylelistbox"
    Kontext "Textobjectbar"
    Vorlage.TypeKeys "<Mod1 A>"
    Vorlage.TypeKeys "MyStyle<Return>"
    
    printlog "Check if style has been applied to the combobox"
    Call wTypeKeys "<Mod1 End>"
    
    wait 500
    Kontext "Textobjectbar"
    try
        Vorlage.Select "MyStyle"
    catch
        Warnlog "Seems style has not been applied to combobox"
    endcatch
        
    printlog "Check if style has been applied to paragraph"
    Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys "<Right>", 2
    
    Kontext "Textobjectbar"
    if Vorlage.GetSelText <> "MyStyle" then
        Warnlog "Seems style has not been applied to paragraph"
    end if

    printlog "Close Document"
    Call hCloseDocument
endcase

' **********************************************************************

testcase tAssigningStyles4
    Dim i as integer
    Dim sTemp as string
    Dim sStyle as integer
    
    printlog "- Clear formatting: whole paragraph"
    printlog "Open a new writer document"
    Call hNewDocument
    printlog "Insert a dummytext"
    if wBlindtextEinfuegen = false then
        Call hCloseDocument
        exit sub
    end if
    
    printlog "Select whole dummytext"
    Call wTypeKeys "<Mod1 A>"
    printlog "Apply style 'Heading 1' through Styles & Formatting combobox"
    Kontext "Textobjectbar"
    ' remeber default style
    sStyle = Vorlage.GetSelIndex
    for i = 1 to Vorlage.GetItemCount
        Vorlage.Select i
        sTemp = Vorlage.GetSelText
        if Instr(sTemp, "1") then
            Vorlage.TypeKeys "<Return>"
            exit for
        end if
    next i

    printlog "choose 'Clear formatting' in combobox" 
    Kontext "Textobjectbar"
    Vorlage.Select 1
    Vorlage.TypeKeys "<Return>"
    wait 500
    printlog "Check if style has been set to default"
    Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys "<Right>", 3

    wait 500    
    Kontext "Textobjectbar"
    if Vorlage.GetSelIndex <> sStyle then
        warnlog "Seems paragraph has not been set to default! " & Vorlage.GetSelText & "(" & Vorlage.GetSelIndex & ")" 
    end if

    printlog "Close Document"
    Call hCloseDocument 

endcase

' **********************************************************************

testcase tAssigningStyles5

    printlog "- Clear formatting: whole paragraph with hard formatting of some characters"
    printlog " - 1. Cursor outside of hard formatted string"
    printlog "Open a new writer document"
    Call hNewDocument
    printlog "Insert a dummytext"
    if wBlindtextEinfuegen = false then
        Call hCloseDocument
        exit sub
    end if

    printlog "Highlight first line in dummy text"
    Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys "<Shift End>" 
    
    printlog "Format / Bold"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    Fett.Click
    Wait 500

    printlog "Set cursor outside of bold formatted text"
    Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys "<Down><Right>"
    
    printlog "Select 'Clear Formatting' in Style-Listbox"
    Kontext "Textobjectbar"
    Vorlage.Select 1
    Vorlage.TypeKeys "<Return>"
    wait 500

    printlog "Check that first line is still bold and has not been reset"
    Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys "<Right>"

    Kontext "TextObjectbar"
    if Fett.Getstate(2) = 0 then
        Warnlog "Style 'Bold' has been reset!"
    end if

    printlog "Close document"
    Call hCloseDocument
endcase

' **********************************************************************

testcase tAssigningStyles6

    Dim iStandard as integer

    printlog "- Clear formatting: whole paragraph with hard formatting of some characters"
    printlog " - 2. Cursor inside of hard formatted string"  
    printlog "Open a new writer document"
    Call hNewDocument
    printlog "Insert a dummytext"
    if wBlindtextEinfuegen = false then
        Call hCloseDocument
        exit sub
    end if

    printlog "Highlight first line in dummy text"
    Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys "<Shift End>" 
    
    printlog "Format / Bold"
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    Fett.Click
    Wait 500

    printlog "Point cursor to first highlighted word of bold formatted text"
    Call wTypeKeys "<Mod1 Home>"    
    Call wTypeKeys "<Right>"
    
    printlog "Select 'Format / Default Formatting' in Style-Listbox"
    FormatDefaultWriter
    wait 500

    printlog "first highlighted word should be set to 'Standard'"
    Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys "<Right>"
    Kontext "TextObjectbar"
    if Fett.Getstate(2) = 1 then
        Warnlog "Style 'Bold' has not been reset!"
    end if
    
    printlog "Check that rest of first line is still bold and has not been reset"
    Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys "<End><Left>"

    Kontext "TextObjectbar"
    if Fett.Getstate(2) = 1 then
        Warnlog "Style 'Bold' has not been reset!"
    end if

    printlog "Close document"
    Call hCloseDocument
endcase

' **********************************************************************

testcase tAssigningStyles7

    printlog "More ..."  
    printlog "Open a new writer document"
    Call hNewDocument
    
    printlog "Select 'More...' in Style-Listbox"
    Kontext "Textobjectbar"
    Vorlage.Select Vorlage.GetItemCount
    Vorlage.TypeKeys "<Return>"
    wait 500

    printlog "The Stylist should have been opened"
    Kontext "Stylist"
    if Not Stylist.Exists then
        Warnlog "'Styles and Formatting' doesn't open if 'More...' is selected in Style-Listbox"
    else
        printlog "In Style group listbox 'Automatic' should be selected"
        if Gruppenliste.GetSelIndex <> 5 then
            Warnlog "Seems 'Automatic' is not selected per default in 'Styles and Formatting'"
        end if
        Stylist.Close
    end if

    printlog "Close document"
    Call hCloseDocument
endcase

' **********************************************************************

testcase tKeyboardShortcutToStyles()
    dim sTempa as string
    dim sTempb as string
    dim sTempc as string
    dim i as integer
    dim x as integer

    printlog "open new application document"
    call hNewDocument()

    printlog "get current default style from TextObjectBar"
    kontext "TextObjectBar"
    sTempa = Vorlage.getSelText
    sTempc = sTempa
    for i = 3 to 0 step -1
        printlog "Perform the Keycombination <CTRL>+3 (2,1,0) and check, if Style in TextObjectBar changed"
        printlog "- <Ctrl>+<"+i+"> -> Heading "+i+" style"
        Call hTypeKeys("<Mod1 "+i+">")
        sleep 1
        kontext "TextObjectBar"
        sTempb = sTempa
        sTempa = Vorlage.getSelText
        if (sTempa <> sTempb) then
            printlog "OK: Heading changed: '" + sTempa + "'"
        else
            warnlog "Heading didn't change: '" + sTempa + "'"
        end if
    next i
    kontext "DocumentWriter"
    DocumentWriter.TypeKeys("<MOD1 SHIFT 0>")
    sleep (2)
    kontext "TextObjectBar"
    sTempa = Vorlage.getSelText

    printlog "should now be the default from the beginning"
    if (uCase(gApplication)<>"HTML") then
        if (sTempa = sTempc) then
            printlog "OK: Heading changed to default: '" + sTempa + "'"
        else
            warnlog "Heading didn't change to default. Should have been: '" + sTempc + "' but was: '" + sTempa + "'"
        end if
    end if

    printlog "close document"
    call hCloseDocument()
endcase

