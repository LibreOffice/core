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
'* short description : Test of autotext in writer
'*
'\***********************************************************************

sub w_autotext

    Call tEditAutotext_1
    Call tEditAutotext_2
    Call tEditAutotext_3
    Call tEditAutotext_4
    Call tEditAutotext_5

end sub

testcase tEditAutotext_1
    Dim j as integer
    Dim sAll as integer
    Dim i as integer
    Dim Vergleich1$
    Dim Vergleich2$
    Dim Vergleich3$
    Dim Vergleich4$
    
    Printlog "- Create a new autotext with content!"
    printlog " New Writerdocument"
    Call hNewDocument
    printlog " Create a new autotext category"
    if wCreateAutotextCategory( "AANewCategory" ) = false then
        Warnlog "Unable to create new Autotext category"
        Kontext "Autotext"
        if Autotext.Exists then Autotext.Close
        Call hCloseDocument
        goto endsub
    end if       
    wait 500
    Kontext "Autotext"
    Autotext.Close
    
    printlog " Insert some text and format text"
    Call wTypeKeys("This becomes an autotext!")
    ' Format word 'This'
    Call wTypeKeys("<Home><Right>")
    FormatCharacter
    Kontext
    Active.SetPage TabFont
    kontext "TabFont"
    if gAsianSup = True then
        FontWest.Select 5
        if StyleWest.GetItemCount >= 3 then
            StyleWest.Select 3
        else
            For i = 6 to FontWest.GetItemCount
                FontWest.Select i
                try
                    StyleWest.Select 3
                    exit for
                catch
                endcatch
            next i
        end if
        Vergleich1$ = FontWest.GetSelText                
        Vergleich2$ = StyleWest.GetSelText
        SizeWest.Select 5
        Vergleich3$ = SizeWest.GetSelText
    else
        Font.Select 5
        if Style.GetItemCount >= 3 then
            Style.Select 3
        else
            For i = 6 to Font.GetItemCount
                Font.Select i
                try
                    Style.Select 3
                    exit for
                catch
                endcatch
            next i
        end if
        Vergleich1$ = Font.GetSelText
        Vergleich2$ = Style.GetSelText
        Size.Select 5
        Vergleich3$ = Size.GetSelText
    end if
    wait 100
    Kontext
    Active.Setpage TabHyperlinkZeichen
    Kontext "TabHyperlinkZeichen"
    UrlName.Settext "http://www.irgendwas.de"
    TargetFrame.Select 2
    Vergleich4$ = TargetFrame.GetSelText
    TabHyperlinkZeichen.Ok

    Call wTypeKeys("<End>")
    Call wTypeKeys("<Shift Home>")

    printlog " Call Edit / Autotext"
    EditAutotext
    Kontext "Active"
    if Active.Exists then
        warnlog Active.Gettext
        Active.Ok
    end if
    Kontext "Autotext"
    printlog " Uncheck 'Preview and Select first group in List"
    Vorschau_anzeigen.Uncheck
    Kontext "Autotext"
    if wSelectAutotextCategory( "AANewCategory" ) = false then
        Warnlog "Unable to select Autotext category"
        Kontext "Autotext"
        if Autotext.Exists then Autotext.Close
        Call hCloseDocument
        goto endsub
    end if  
    wait 500

    printlog " Insert a name for the Autotext and a Shortcut"
    NameText.Settext "HDEAutotext"
    Kuerzel.Settext "HDE"
    printlog " Select 'New' in Menubutton 'Autotext'"
    wait 500   
    if Menue.IsEnabled then 
        Menue.Click
    else
        Warnlog "Unable to create autotext->Button 'Autotext' is disabled"
        Autotext.Close
        Call hCloseDocument
        goto endsub
    end if
    
    wait 500
    try
        Call hMenuSelectNr ( 1 )
    catch
        Warnlog "Menu Autotext->'New' disabled ! Test failed !"
        Call hMenuClose
        Kontext "Autotext"
        Autotext.Cancel
        Call hCloseDocument
        goto endsub
    endcatch
    printlog " Check if Autotext has been added to list"
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Warnlog "Unable to add autotext: " + Active.Gettext
            Active.Ok
            Kontext "Autotext"
            Autotext.Cancel
            Call hCloseDocument
            goto endsub
        end if
    end if

    if wSelectAutotextCategory("AANewCategory") = false then
        Warnlog "Created autotext category not found!"    
    else
        Liste.TypeKeys "+"

        sAll = Liste.GetItemCount
        For j = 1 to sAll
            Liste.TypeKeys "<Down>"
            if Liste.Gettext = "HDEAutotext" then
                printlog "Autotext has been found!"
                Autotext.Ok
                j = sAll + 2
            end if
        next j
        printlog " Check if content has been added to named autotext"
        Kontext "Autotext"
        if Autotext.Exists then
            Warnlog "New autotext has not been found!"
            Autotext.Close
        else
            hSetDocumentContext()
            Call wTypeKeys("<Shift Home>")
            EditCopy
            if GetClipboardtext <> "This becomes an autotext!" then
                Warnlog "Autotext has no content!"
            else
                Call wTypeKeys ("<Home><Right>")
                FormatCharacter
                Kontext
                Active.SetPage TabFont
                Kontext "TabFont"
                if gAsianSup = True then
                    if Vergleich1$ <> FontWest.GetSelText then Warnlog "Wrong Font in Autotext!"
                    if Vergleich2$ <> StyleWest.GetSelText then Warnlog "Wrong Fontstyle in Autotext!"
                    if Vergleich3$ <> SizeWest.GetSelText then Warnlog "Wrong Fontsize in Autotext!"
                else
                    if Vergleich1$ <> Font.GetSelText then Warnlog "Wrong Font in Autotext!"
                    if Vergleich2$ <> Style.GetSelText then Warnlog "Wrong Fontstyle in Autotext!"
                    if Vergleich3$ <> Size.GetSelText then Warnlog "Wrong Fontsize in Autotext!"
                end if
                wait 100
                Kontext
                Active.Setpage TabHyperlinkZeichen
                Kontext "TabHyperlinkZeichen"
                if UrlName.Gettext <> "http://www.irgendwas.de/" then Warnlog "Wrong hyperlink has been added to text!"
                if Vergleich4$ <> TargetFrame.GetSelText then warnlog "Wrong targetFrame has been added to text!"
                TabHyperlinkZeichen.Cancel
            end if
        end if
    end if

    printlog " Delete whole category with created autotext"
    Call wDeleteAutotextCategory( "AANewCategory" )
    Kontext "Autotext"
    if Autotext.Exists then Autotext.Close
    Call hCloseDocument

endcase

'---------------------------------------------------

testcase tEditAutotext_2
    Dim j as integer
    Dim sAll as integer
    Dim i as integer
    Dim Vergleich1$
    Dim Vergleich2$
    Dim Vergleich3$
    Dim Vergleich4$

    Printlog "- Create a new autotext with content! (Text only)"
    printlog " New Writerdocument"
    Call hNewDocument
    printlog " Create a new autotext category"
    if wCreateAutotextCategory( "AANewCategory" ) = false then
        Warnlog "Unable to create new Autotext category"
        Kontext "Autotext"
        if Autotext.Exists then Autotext.Close
        Call hCloseDocument
        goto endsub
    end if       
    wait 500
    Kontext "Autotext"
    Autotext.Close
    hSetDocumentContext()
    printlog " Insert some text and format text"
    Call wTypeKeys("This becomes an autotext!")
    ' Format word 'This'
    Call wTypeKeys("<Home><Right>")
    FormatCharacter
    Kontext
    Active.SetPage TabFont
    kontext "TabFont"
    if gAsianSup = true then
        FontWest.Select 5
        if StyleWest.GetItemCount >= 3 then
            StyleWest.Select 3
        else
            For i = 6 to FontWest.GetItemCount
                FontWest.Select i
                try
                    StyleWest.Select 3
                    exit for
                catch
                endcatch
            next i
        end if
        Vergleich1$ = FontWest.GetSelText
        Vergleich2$ = StyleWest.GetSelText
        SizeWest.Select 5
        Vergleich3$ = SizeWest.GetSelText
    else
        Font.Select 5
        if Style.GetItemCount >= 3 then
            Style.Select 3
        else
            For i = 6 to Font.GetItemCount
                Font.Select i
                try
                    Style.Select 3
                    exit for
                catch
                endcatch
            next i
        end if
        Vergleich1$ = Font.GetSelText
        Vergleich2$ = Style.GetSelText
        Size.Select 5
        Vergleich3$ = Size.GetSelText
    end if
    wait 100
    Kontext
    Active.Setpage TabHyperlinkZeichen
    Kontext "TabHyperlinkZeichen"
    UrlName.Settext "http://www.irgendwas.de"
    TargetFrame.Select 2
    Vergleich4$ = TargetFrame.GetSelText
    TabHyperlinkZeichen.Ok

    Call wTypeKeys("<End>")
    Call wTypeKeys("<Shift Home>")

    printlog " Call Edit / Autotext"
    EditAutotext
    Kontext "Active"
    if Active.Exists then
        warnlog Active.Gettext
        Active.Ok
    end if
    Kontext "Autotext"

    printlog " Uncheck 'Preview and Select first group in List"
    Vorschau_anzeigen.Uncheck
    Kontext "Autotext"
    if wSelectAutotextCategory( "AANewCategory" ) = false then
        Warnlog "Unable to select Autotext category"
        Kontext "Autotext"
        if Autotext.Exists then Autotext.Close
        Call hCloseDocument
        goto endsub
    end if  
    wait 500

    printlog " Insert a name for the Autotext and a Shortcut"
    NameText.Settext "HDEAutotext"
    Kuerzel.Settext "HDE"
    printlog " Select 'New' in Menubutton 'Autotext'"
    wait 500
    Menue.Click
    wait 500
    try
        Call hMenuSelectNr ( 2 )
    catch
        Warnlog "Menu Autotext->'New' disabled ! Test failed !"
        Call hMenuClose
        Kontext "Autotext"
        Autotext.Cancel
        Call hCloseDocument
        goto endsub
    endcatch
    printlog " Check if Autotext has been added to list"
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Warnlog "Unable to add autotext: " + Active.Gettext
            Active.Ok
            Kontext "Autotext"
            Autotext.Cancel
            Call hCloseDocument
            goto endsub
        end if
    end if

    if wSelectAutotextCategory("AANewCategory") = false then
        Warnlog "Created autotext category not found!"
    else
        Liste.TypeKeys "+"
    
        sAll = Liste.GetItemCount
        For j = 1 to sAll
            Liste.TypeKeys "<Down>"
            if Liste.Gettext = "HDEAutotext" then
                printlog "Autotext has been found!"
                Autotext.Ok
                j = sAll + 2
            end if
        next j
        printlog " Check if content has been added to named autotext"
        Kontext "Autotext"
        if Autotext.Exists then
            Warnlog "New autotext has not been found!"
            Autotext.Close
        else
            hSetDocumentContext()
            Call wTypeKeys("<Home><Up>" , 4)
            Call wTypeKeys("<Shift End>")
            EditCopy
            if GetClipboardtext <> "This becomes an autotext!" then
                Warnlog "Autotext has no content!"
            else
                Call wTypeKeys ("<Home><Right>")
                FormatCharacter
                Kontext
                Active.SetPage TabFont
                Kontext "TabFont"
                if gAsianSup = true then
                    if Vergleich1$ = FontWest.GetSelText then Warnlog "Wrong Font in Autotext!"
                    if Vergleich2$ = StyleWest.GetSelText then Warnlog "Wrong Fontstyle in Autotext!"
                    if Vergleich3$ = SizeWest.GetSelText then Warnlog "Wrong Fontsize in Autotext!"
                else
                    if Vergleich1$ = Font.GetSelText then Warnlog "Wrong Font in Autotext!"
                    if Vergleich2$ = Style.GetSelText then Warnlog "Wrong Fontstyle in Autotext!"
                    if Vergleich3$ = Size.GetSelText then Warnlog "Wrong Fontsize in Autotext!"
                end if
                wait 100
                Kontext
                Active.Setpage TabHyperlinkZeichen
                Kontext "TabHyperlinkZeichen"
                if UrlName.Gettext = "http://www.irgendwas.de/" then Warnlog "Wrong hyperlink has been added to text!"
                if Vergleich4$ = TargetFrame.GetSelText then Warnlog "Wrong targetFrame has been added to text!"
                TabHyperlinkZeichen.Cancel
            end if
        end if
    end if

    printlog " Delete whole category with created autotext"
    Call wDeleteAutotextCategory( "AANewCategory" )
    Kontext "Autotext"
    if Autotext.Exists then Autotext.Close
    Call hCloseDocument

endcase

'---------------------------------------------------

testcase tEditAutotext_3

    Dim i as Integer
    Dim j as Integer
    Dim AlterWert as String
    Dim hauMatsch as integer

    printlog " Edit / Autotext"
    PrintLog "- Edit / Autotext"
    printlog " New document"
    Call  hNewDocument
    printlog " Select Edit / Autotext"
    EditAutotext
    Kontext "Active"
    if Active.Exists then
        warnlog Active.Gettext
        Active.Ok
    end if

    printlog "  - Select and Insert all Autotexts "
    printlog " Select and Insert all autotexts"
    Sleep 1
    Kontext "AutoText"
    Vorschau_anzeigen.Uncheck
    Liste.Select 1
    for j=1 to 10
        Liste.TypeKeys "-"
        Liste.TypeKeys "<Down>"
        wait 500
    next j

    hauMatsch = Liste.GetItemCount

    for i = 1 to hauMatsch
        Liste.Select 1
        for j=1 to Liste.GetItemCount
            Liste.TypeKeys "-"
            Liste.TypeKeys "<Down>"
            wait 500
        next j
        Vorschau_anzeigen.Check

        Liste.Select i
        Liste.TypeKeys "+"

        Call tCheckAutotextInGroup(Liste.GetItemCount-hauMatsch,i)

    next i

    Kontext "Autotext"
    if Autotext.Exists then Autotext.Close
    
    Call hCloseDocument
endcase

'---------------------------------------------------

testcase tEditAutotext_4

    Call hNewDocument

    printlog " Create a new categorie"
    printlog "  - Create new categorie"
    if wCreateAutotextCategory("AAAAAA") = false then
        Warnlog "Unable to create category"
        Kontext "Autotext"
        if Autotext.Exists then Autotext.Close
        Call hCloseDocument
        goto endsub
    end if

    printlog " Close dialog and check if categorie has been created"
    Kontext "Autotext"
    if Autotext.Exists then Autotext.Close
    if wSelectAutotextCategory("AAAAAA") = false then
        Warnlog "New categorie has been deleted ( after closing and reopening dialog )"
        Kontext "Autotext"
        if Autotext.Exists then Autotext.Close
        Call hCloseDocument
        goto endsub
    end if

    Bereiche.Click
    Kontext "BereicheBearbeitenAutotext"
    printlog " Rename the new categorie"
    printlog "  -  Rename new categorie!"
    Bereich.Settext "AAAAAArrghh"
    Wait 500
    printlog " In dialog click 'rename'"
    if Umbenennen.IsEnabled then
        Umbenennen.Click
        BereicheBearbeitenAutoText.OK
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                warnlog Active.Gettext
                Active.Ok
            end if
        end if
    else
        Warnlog "Categorie couldn't be renamed (Button is disabled), no further test!"
        Kontext "BereicheBearbeitenAutotext"
        BereicheBearbeitenAutoText.Close
        if wDeleteAutotextCategory("AAAAAA") = false then Warnlog "Unable to delete category"
        Kontext "Autotext"
        AutoText.Close
        Call hCloseDocument
        goto endsub
    end if

    Kontext "BereicheBearbeitenAutotext"
    if BereicheBearbeitenAutotext.Exists then BereicheBearbeitenAutotext.Ok 
    printlog " Close dialog and check if Categorie has been renamed"
    Kontext "Autotext"
    if Autotext.Exists then Autotext.Close
    Sleep 1
    if wSelectAutotextCategory("AAAAAArrghh") = false then
        Warnlog "Categorie hasn't been correctly renamed"
        AlterWert=Liste.Gettext
    end if

    printlog " Delete new categorie"
    printlog "  - Delete new categorie"
    printlog " Select 'Categories'"
    Bereiche.Click
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            warnlog Active.Gettext
            Active.Ok
        end if
    end if
    Kontext "BereicheBearbeitenAutoText"
    Bereich.SetText "AAAAAArrghh"
    printlog " Select 'Delete', close document and check if categorie has been deleted"
    if Loeschen.IsEnabled then
        Loeschen.Click
    else
        Warnlog "Categorie couldn't be deleted (Button is disabled)!"
    end if

    BereicheBearbeitenAutoText.OK
    Kontext "MessageBox"
    if Not MessageBox.Exists(2) then
        Warnlog "No messsage after deleting Categorie and closing the Dialog !"
    else
        MessageBox.No
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                warnlog Active.Gettext
                Active.Ok
            end if
        end if
        if wSelectAutotextCategory("AAAAAArrghh") = false then
            Warnlog "After denying the 'Deletemessage' the categorie has been deleted!"
        else
            Kontext "Autotext"
            Bereiche.Click
            Kontext "Active"
            if Active.Exists then
                if Active.GetRT = 304 then
                    warnlog Active.Gettext
                    Active.Ok
                end if
            end if
            Kontext "BereicheBearbeitenAutoText"
            Bereich.SetText "AAAAAArrghh"
            if Loeschen.IsEnabled then
                Loeschen.Click
                BereicheBearbeitenAutotext.Ok
                Kontext "MessageBox"
                if MessageBox.Exists(2) then MessageBox.Yes
                Kontext "Active"
                if Active.Exists then
                    if Active.GetRT = 304 then
                        QAErrorlog Active.Gettext
                        Active.Ok
                    end if
                end if
                Sleep 2
                if wSelectAutotextCategory("AAAAAArrghh") = true then
                    Warnlog "Though the categorie has been deleted it is always there!"
                end if
            else
                Warnlog "Area couldn't be deleted (Button is disabled)!"
            end if
        end if
    end if

    Kontext "Autotext"
    if Autotext.Exists then Autotext.Close
    
    printlog " Close document"
    Call hCloseDocument
endcase

'---------------------------------------------------

testcase tEditAutotext_5
    printlog "  - Insert Autotext with ShortCuts"
    Call hNewDocument
    printlog " try to insert  autotexts with Shotcut"

    Call wTypeKeys "<Return>", 3
    Call wBlindtextEinfuegen

    Kontext "Active"
    if Active.Exists then
        warnlog Active.GetText
        Active.OK
    end if
    Sleep 1
    Call wTypeKeys "<Mod1 A>"
    Sleep 1
    EditCopy
    printlog " Check if autotext has been inserted"
    if GetClipboardText = "" then Warnlog "Autotext couldn't be inserted with Short-Cut!"

    printlog " Close document"
    Call hCloseDocument
endcase
