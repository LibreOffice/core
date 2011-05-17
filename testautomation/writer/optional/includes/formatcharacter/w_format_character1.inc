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
'* short description : Standard character formatting. Setting Styles.
'*
'\***********************************************************************

testcase tFormatCharacter1
    printlog "- Check if 'Pair Kerning' is enabled by default!"
    '/// Format / Character / Tab Position ///
    Call hNewDocument
    '/// Open new document
    FormatCharacter
    '/// Format / Character => Select Tabpage 'Position'
    Kontext
    Active.Setpage TabFontPosition
    Kontext "TabFontPosition"
    '/// 'Pair Kerning' should be checked by default
    if Not PairKerning.IsChecked then
        warnlog "Pair Kerning should be checked by default !"
    end if
    TabFontPosition.Cancel
    '/// Close dialog and open Stylist
    Kontext "Stylist"
    if Not Stylist.Exists then FormatStylist
    '/// Open context menu on selected style and choose 'Edit'
    Vorlagenliste.OpenContextMenu
    Call hMenuSelectNr(2)
    '///  Select Tabpage 'Position'
    Kontext
    Active.Setpage TabFontPosition
    Kontext "TabFontPosition"
    '/// 'Pair Kerning' should be checked by default
    if Not PairKerning.IsChecked then
        warnlog "Pair Kerning should be checked by default !"
    end if
    TabFontPosition.Cancel
    '/// close Stylist
    FormatStylist
    
    '/// close document
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------

testcase tFormatCharacter2
    '/// Format / Default ///
    '/// New document ///
    Call hNewDocument
    Printlog "  - adjust Font- and Paragraph-Format"
    '/// Insert a word and select it ///
    Call wTypeKeys "Format<Shift Mod1 Left>"
    '/// Select Format / Character and adjust Font- and Paragraph-Formats ///
    FormatCharacter
    Sleep 5
    Kontext
    active.SetPage TabFontEffects
    kontext "TabFontEffects"
    Outline.Check
    Shadow.Check
    Underline.Select 2
    StrikeThrough.Select 2
    TabFontEffects.OK
    FormatParagraph
    Kontext
    active.SetPage TabEinzuegeUndAbstaende
    kontext "TabEinzuegeUndAbstaende"
    Automatisch.Check
    '/// Close dialog ///
    TabEinzuegeUndAbstaende.OK
    Wait 500
    
    Printlog "  - Format / Default"
    FormatDefaultWriter
    Wait 500
    
    Printlog "  - check Font- and Paragraph-Formats"
    
    '/// Check if Font- and Paragraph-Formats have benn set to default ///
    FormatCharacter
    Kontext
    active.SetPage TabFontEffects
    kontext "TabFontEffects"
    if Outline.IsChecked then Warnlog "Outline is checked"
    if Shadow.IsChecked then Warnlog "Shadow is checked"
    if Underline.GetSelIndex = 2 then Warnlog "Wrong entry in 'Underline' is selected"
    if StrikeThrough.GetSelIndex = 2 then Warnlog "Wrong entry in 'StrikeThrough' is selected"
    TabFontEffects.OK
    
    FormatParagraph
    Kontext
    active.SetPage TabEinzuegeUndAbstaende
    kontext "TabEinzuegeUndAbstaende"
    if Automatisch.IsChecked then Warnlog "Automatic is checked"
    TabEinzuegeUndAbstaende.OK
    
    '/// Close document ///
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------

testcase tFormatCharacter3
    Dim Vergleich1 as integer
    Dim Vergleich2 as string
    Dim Vergleich3 as string
    Dim Vergleich4 as string
    Dim vLink as integer

    printlog "New document"
    Call hNewDocument
	printlog "add some text to document => 	'formatted text with hyperlink'"
	Call wTypeKeys ("formatted text with hyperlink")
	printlog "select text"
	Call wTypeKeys ("<Shift Home>")

    FormatCharacter
    WaitSlot (5)
    Kontext
    printlog "On Tabpage 'Font' adjust settings"
    active.SetPage TabFont
    kontext "TabFont"
    try
        FontWest.Select 1
        Vergleich2 = FontWest.GetSelText
        Vergleich1 = StyleWest.GetItemCount
        StyleWest.Select Vergleich1
        Vergleich3 = StyleWest.GetSelText
        SizeWest.Select 24
    catch
        Font.Select 1
        Vergleich2 = Font.GetSelText
        Vergleich1 = Style.GetItemCount
        Style.Select Vergleich1
        Vergleich3 = Style.GetSelText
        Size.Select 24
    endcatch
    wait 100
    Kontext
    printlog "On Tabpage 'Font Effects' adjust settings"
    active.SetPage TabFontEffects
    kontext "TabFontEffects"
    Blinking.Check
    Kontext
    Active.Setpage TabFontPosition
    printlog "On Tabpage 'Font Position ' adjust settings"
    Kontext "TabFontPosition"
    Spacing.Select 2
    SpacingBy.More 3
    Vergleich4 = SpacingBy.GetText
    if Right ( Vergleich4, 2 ) <> "pt" then Warnlog "Wrong Unit at Spacing by"
    Kontext
    printlog "On Tabpage 'Hyperlink' adjust settings"
    active.SetPage TabHyperlinkZeichen
    kontext "TabHyperlinkZeichen"
    URLName.SetText "www.mikeportnoy.com"
    NameText.SetText "A hyperlink with testtool"
    TargetFrame.Select 2
    Select Case iSprache
        Case 36
            vLink = 2
        Case else
            vLink = 6
    end select
    UnbesuchterLink.Select vLink
    kontext
    active.SetPage TabHintergrund
    kontext "TabHintergrund"
    Sleep 1
    Hintergrundfarbe.Typekeys "<Down><Left>", 4
    TabHintergrund.OK
    printlog "Close dialog"

    printlog "Reopen with Format / Character and check settings made"
    Call wTypeKeys "<Home><Right>"
    FormatCharacter
    Kontext
    active.SetPage TabFont
    kontext "TabFont"
    try
        if FontWest.GetSelIndex <> 1 then Warnlog "Font should "+ Vergleich2 +" is "+ FontWest.GetSelText
        if StyleWest.GetSelText <> Vergleich3 then Warnlog "FontStyle should " +Vergleich3+" is "+ StyleWest.GetSelText
        if SizeWest.GetSelIndex <> 24 then Warnlog "Wrong Font Size : " + SizeWest.GetSelText
    catch
        if Font.GetSelIndex <> 1 then Warnlog "Font should "+ Vergleich2 +" is "+ Font.GetSelText
        if Style.GetSelText <> Vergleich3 then Warnlog "FontStyle should " + Vergleich3 + " is "+ Style.GetSelText
        if Size.GetSelIndex <> 24 then Warnlog "Wrong Font Size : " + Size.GetSelText
    endcatch
    Kontext
    active.SetPage TabFontEffects
    kontext "TabFontEffects"
    if NOT Blinking.IsChecked then Warnlog "Blinking is not checked"
    Kontext
    Active.Setpage TabFontPosition
    Kontext "TabFontPosition"
    if Spacing.GetSelIndex <> 2 then Warnlog "Wrong Spacing activated, is not the 2. but " + Spacing.GetSelText
    if SpacingBy.GetText <> Vergleich4 then Warnlog "Wrong Value selected. Not: " + Vergleich4 +" but " + SpacingBy.GetText

    Kontext
    active.SetPage TabHyperlinkZeichen
    kontext "TabHyperlinkZeichen"
    if URLName.GetText <> "http://www.mikeportnoy.com/" then Warnlog "#i105855#Wrong URL : " + URLName.GetText
    SetClipboard UrlName.Gettext
    if NameText.GetText <> "A hyperlink with testtool" then Warnlog "#i105855#Wrong Linkname : " + NameText.GetText
    if TargetFrame.GetSelIndex <> 2 then Warnlog "#i105855#Wrong Target selected : " + TargetFrame.GetSelText
    if UnbesuchterLink.GetSelIndex <> vLink then Warnlog "#i105855#Wrong Style for unused Link :" + UnbesuchterLink.GetSelIndex
    TabHyperlinkZeichen.OK

    printlog "Save document as '../user/work/format1.sxw' and close it"
    if gApplication = "WRITER" then
        Call hFileSaveAsWithFilterKill ( gOfficePath + "user\work\format1.odt", "writer8")
    else
        Call hFileSaveAsWithFilterKill ( gOfficePath + "user\work\format1.odm", "writer8")  'gMasterDocFilter
    end if    

    printlog "Close document"
    Call hCloseDocument
    printlog "reopen previous saved document and check saved character format"    
    if gApplication = "WRITER" then
        hFileOpen( gOfficePath & "user\work\format1.odt" )
    else
        hFileOpen( gOfficePath & "user\work\format1.odm" )
    end if
    Call wTypeKeys "<Mod1 Home><Right>"
    Wait 500
    FormatCharacter
    WaitSlot (3)
    Kontext
    active.SetPage TabFont
    kontext "TabFont"
    try
        if FontWest.GetSelIndex <> 1 then Warnlog "Font should "+ Vergleich2 +" is "+ FontWest.GetSelText
        if StyleWest.GetSelText <> Vergleich3 then Warnlog "FontStyle should " + Vergleich3 + " is "+ StyleWest.GetSelText
        if SizeWest.GetSelIndex <> 24 then Warnlog "Wrong Font Size : " + SizeWest.GetSelText
    catch
        if Font.GetSelIndex <> 1 then Warnlog "Font should "+ Vergleich2 +" is "+ Font.GetSelText
        if Style.GetSelText <> Vergleich3 then Warnlog "FontStyle should " +Vergleich3+" is "+ Style.GetSelText
        if Size.GetSelIndex <> 24 then Warnlog "Wrong Font Size : " + Size.GetSelText
    endcatch
    Kontext
    active.SetPage TabFontEffects
    kontext "TabFontEffects"
    if NOT Blinking.IsChecked then Warnlog "Blinking is not checked"
    Kontext
    Active.Setpage TabFontPosition
    Kontext "TabFontPosition"
    if Spacing.GetSelIndex <> 2 then Warnlog "Wrong Spacing selected. Not 2. but " + Spacing.GetSelText
    if SpacingBy.GetText <> Vergleich4 then Warnlog "Wrong value selectzed. Not " + Vergleich4 +" but " + SpacingBy.GetText
    Kontext
    active.SetPage TabHyperlinkZeichen
    kontext "TabHyperlinkZeichen"
    if URLName.GetText <> "http://www.mikeportnoy.com/" then Warnlog "#i105855#Wrong URL : " + URLName.GetText
    if NameText.GetText <> "A hyperlink with testtool" then Warnlog "#i105855#Wrong Linkname : " + NameText.GetText
    if TargetFrame.GetSelIndex <> 2 then Warnlog "#i105855#Wrong Target selected : " + TargetFrame.GetSelText
    TabHyperlinkZeichen.OK
    '/// Close document ///
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------

testcase tFormatHyperlink
    ' /// This testcase has been written due to bug #109488
    '/// Open a new writerdocument
    Call hNewDocument
    '/// Enter a URL (www.sun.com) in document
    Call wTypeKeys "Autocorrect my URL www.sun.com"
    '/// Enter a space after URL
    Call wTypeKeys " "
    '/// URL should have been autocompleted to http://www.sun.com
    Call wTypeKeys "<Left>",7
    FormatCharacter
    Kontext
    Active.Setpage TabHyperlinkZeichen
    Kontext "TabHyperlinkZeichen"
    if URLName.GetText <> "http://www.sun.com/" then
        Warnlog "Autocorrection for hyperlink does not work!"
    else
        printlog "Hyperlink has been auto-corrected"
    end if
    TabHyperlinkZeichen.Cancel

    '/// Change URL to www.sauna.com
    Call wTypeKeys "a"
    Call wTypeKeys "<Left>" ,2
    Call wTypeKeys "a"

    '/// Check whether URL has been changed
    FormatCharacter
    Kontext
    Active.Setpage TabHyperlinkZeichen
    Kontext "TabHyperlinkZeichen"
    if URLName.GetText <> "http://www.sun.com/" then
        Warnlog "Autocorrection for hyperlink should not have been affected to URL!"
    else
        printlog "Hyperlink has not been auto-corrected"
    end if
    TabHyperlinkZeichen.Cancel
    '/// Close document
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------

testcase tFormatStyles1
    Dim iMenuEntry as integer

    if hIsExtensionAlreadyInstalled("SunSearchToolbar.oxt") = true then
        iMenuEntry = 6
    else
        iMenuEntry = 5
    end if
    '/// All format styles in context-menu ///
    '/// Open a new document ///
    Call hNewDocument
    printlog "- Style 'Bold' "
    '/// Write 'Style Bold' - execute Context menu 'Style Bold'
    Call wTypeKeys "Style Bold"
    Call wTypeKeys "<Shift Home>"
    Kontext "DocumentWriter"
    DocumentWriter.OpenContextMenu
    Call hMenuFindSelect (".uno:StyleMenu", true, 5, false)   'Find 'Style' and call the slot.
    if hMenuFindSelect (".uno:Bold", true, 1, false) = true then  'Find 'Bold' and call the slot.
        '/// Check if changes to style have been made ///
        Kontext "TextObjectbar"
        if Fett.Getstate(2) = 0 then
            Warnlog "Style 'Bold' with context menu has not been set!"
        end if

        Call wTypeKeys "<End><Mod1 Shift X>"
        wait 500
        Call wTypeKeys "<Return>"
    endif

    printlog "- Style 'Italic' "
    '/// Write 'Style Italic' - execute Context menu 'Style Italic'
    Call wTypeKeys "Style Italic"
    Call wTypeKeys "<Shift Home>"
    Kontext "DocumentWriter"
    DocumentWriter.OpenContextMenu
    Call hMenuFindSelect (".uno:StyleMenu", true, 5, false)   'Find 'Style' and call the slot.
    if hMenuFindSelect (".uno:Italic", true, 2, false) = true then  'Find 'Italic' and call the slot.
        ' /// Check if changes to style have been made ///
        Kontext "TextObjectbar"
        if Kursiv.Getstate(2) = 0 then
            Warnlog "#58722#Style 'Italic' with context menu has not been set!"
        end if

        Call wTypeKeys "<End><Right>"
        Call wTypeKeys "<Return>"
    end if

    printlog "- Style 'Underline' "
    '/// Write 'Style Underline' - execute Context menu 'Style Underline'
    Call wTypeKeys "Style Underline"
    Call wTypeKeys "<Shift Home>"
    Kontext "DocumentWriter"
    DocumentWriter.OpenContextMenu
    Call hMenuFindSelect (".uno:StyleMenu", true, 5, false)   'Find 'Style' and call the slot.
    if hMenuFindSelect (".uno:Underline", true, 4, false) = true then   'Find 'Underline' and call the slot.
        ' /// Check if changes to style have been made ///
        FormatCharacter
        Kontext
        Active.Setpage TabFontEffects
        Kontext "TabFontEffects"
        if Underline.GetSelIndex <> 2 then Warnlog "Style 'Underline' with context menu has not been set!"
        TabFontEffects.Cancel
        Call wTypeKeys "<End><Right>"
        Call wTypeKeys "<Return>"
    end if

    printlog "- Style 'Overline' "
    '/// Write 'Style Overline' - execute Context menu 'Style Strikethrough'
    Call wTypeKeys "Style Overline"
    Call wTypeKeys "<Shift Home>"
    Kontext "DocumentWriter"
    DocumentWriter.OpenContextMenu
    Call hMenuFindSelect (".uno:StyleMenu", true, 5, false)   'Find 'Style' and call the slot.
    if hMenuFindSelect (".uno:Overline", true, 3, false) = true then  'Find 'Overline' and call the slot.
        ' /// Check if changes to style have been made ///
        FormatCharacter
        Kontext
        Active.Setpage TabFontEffects
        Kontext "TabFontEffects"
        if Overline.GetSelIndex <> 2 then Warnlog "Style 'Overline' with context menu has not been set!"
        TabFontEffects.Cancel
        Call wTypeKeys "<End><Right>"
        Call wTypeKeys "<Return>"
    end if

    printlog "- Style 'StrikeThrough' "
    '/// Write 'Style Strikethrough' - execute Context menu 'Style Strikethrough'
    Call wTypeKeys "Style Strikethrough"
    Call wTypeKeys "<Shift Home>"
    Kontext "DocumentWriter"
    DocumentWriter.OpenContextMenu
    Call hMenuFindSelect (".uno:StyleMenu", true, 5, false)   'Find 'Style' and call the slot.
    if hMenuFindSelect (".uno:Strikeout", true, 5, false) = true then  'Find 'Strikethrough' and call the slot.
        ' /// Check if changes to style have been made ///
        FormatCharacter
        Kontext
        Active.Setpage TabFontEffects
        Kontext "TabFontEffects"
        if StrikeThrough.GetSelIndex <> 2 then Warnlog "Style 'StrikeThrough' with context menu has not been set!"
        TabFontEffects.Cancel
        Call wTypeKeys "<End>"
        Call wTypeKeys "<Right><Return>"
    end if

    printlog "- Style 'Shadow' "
    '/// Write 'Style Shadow' - execute Context menu 'Style Shadow'
    Call wTypeKeys "Style Shadow"
    Call wTypeKeys "<Shift Home>"
    Kontext "DocumentWriter"
    DocumentWriter.OpenContextMenu
    Call hMenuFindSelect (".uno:StyleMenu", true, 5, false)   'Find 'Style' and call the slot.
    if hMenuFindSelect (".uno:Shadowed", true, 6, false) = true then  'Find 'Shadowed' and call the slot.
        ' /// Check if changes to style have been made ///
        FormatCharacter
        Kontext
        Active.Setpage TabFontEffects
        Kontext "TabFontEffects"
        if Shadow.IsChecked = False then Warnlog "Style 'Shadow' with context menu has not been set!"
        TabFontEffects.Cancel
        Call wTypeKeys "<End><Right>"
        Call wTypeKeys "<Return>"
    end if

    printlog "- Style 'Outline' "
    '/// Write 'Style Outline' - execute Context menu 'Style Outline'
    Call wTypeKeys "Style Outline"
    Call wTypeKeys "<Shift Home>"
    Kontext "DocumentWriter"
    DocumentWriter.OpenContextMenu
    Call hMenuFindSelect (".uno:StyleMenu", true, 5, false)   'Find 'Style' and call the slot.
    if hMenuFindSelect (".uno:OutlineFont", true, 7, false) = true then   'Find 'Outline' and call the slot.
        ' /// Check if changes to style have been made ///
        FormatCharacter
        Kontext
        Active.Setpage TabFontEffects
        Kontext "TabFontEffects"
        if Outline.IsChecked = false then Warnlog "Style 'Outline' with context menu has not been set!"
        TabFontEffects.Cancel
    end if
    
    printlog "Close Document."
    Call hCloseDocument
endcase

'---------------------------------------------------------------------------------------------------------------------

testcase tFormatCharacter
  	Dim Vergleich1%, Vergleich2$, Vergleich3$, Vergleich4$
    	
	PrintLog "- Format / Character"
	Call hNewDocument
	
	printlog "add some text to document => 	'formatted text with hyperlink'"
	Call wTypeKeys ("formatted text with hyperlink")
	printlog "select text"
	Call wTypeKeys ("<Shift Home>")
	
	printlog "  - adjust character"
	printlog "Format->Character"
	FormatCharacter
	Sleep 5
	Kontext
	printlog "On Tabpage 'Font' adjust settings"
	active.SetPage TabFont
	kontext "TabFont"
	try
    	FontWest.Select 1
		Vergleich2$ = FontWest.GetSelText
		Vergleich1% = StyleWest.GetItemCount
		StyleWest.Select Vergleich1%
		Vergleich3$ = StyleWest.GetSelText
		SizeWest.Select 24
	catch
    	Font.Select 1
		Vergleich2$ = Font.GetSelText
		Vergleich1% = Style.GetItemCount
		Style.Select Vergleich1%
		Vergleich3$ = Style.GetSelText
		Size.Select 24
	endcatch
	wait 100
	Kontext
	printlog "On Tabpage 'Font Effects' adjust settings"
	active.SetPage TabFontEffects
	kontext "TabFontEffects"
	Blinking.Check
	Kontext
	Active.Setpage TabFontPosition
	printlog "On Tabpage 'Font Position ' adjust settings"
	Kontext "TabFontPosition"
	Spacing.Select 2
	SpacingBy.More 3
	Vergleich4$ = SpacingBy.GetText
	if Right ( Vergleich4$, 2 ) <> "pt" then Warnlog "Wrong Unit at Spacing by"
	Kontext
	printlog "On Tabpage 'Hyperlink' adjust settings"
	active.SetPage TabHyperlinkZeichen
	kontext "TabHyperlinkZeichen"
	URLName.SetText "www.mikeportnoy.com"
	NameText.SetText "Link created by testtool"
	TargetFrame.Select 2

	UnbesuchterLink.Select 6
	Kontext
	active.SetPage TabHintergrund
	kontext "TabHintergrund"
	Sleep 1
	Hintergrundfarbe.Typekeys "<Down><Left>", 4
	TabHintergrund.OK
	printlog "Close dialog"

	printlog " Type Home- and Right-Key"
	Call wTypeKeys "<Home><Right>"	
	
	printlog "recheck settings made with opening Format->Character"
	FormatCharacter
	Kontext
	active.SetPage TabFont
	kontext "TabFont"
	try
    	if FontWest.GetSelText <> Vergleich2$ then Warnlog "Font should "+ Vergleich2$ +" is "+ FontWest.GetSelText
		if StyleWest.GetSelIndex <> Vergleich1% then Warnlog "FontStyle should " +Vergleich3$+" is "+ StyleWest.GetSelText
		if SizeWest.GetSelIndex <> 24 then Warnlog "Wrong Font Size : " + SizeWest.GetSelText
	catch
    	if Font.GetSelText <> Vergleich2$ then Warnlog "Font should "+ Vergleich2$ +" is "+ Font.GetSelText
		if Style.GetSelIndex <> Vergleich1% then Warnlog "FontStyle should " +Vergleich3$+" is "+ Style.GetSelText
		if Size.GetSelIndex <> 24 then Warnlog "Wrong Font Size : " + Size.GetSelText
	endcatch
	Kontext
	active.SetPage TabFontEffects
	kontext "TabFontEffects"
	if NOT Blinking.IsChecked then Warnlog "Blinking is not checked"
	Kontext
	Active.Setpage TabFontPosition
	Kontext "TabFontPosition"
	if Spacing.GetSelIndex <> 2 then Warnlog "Wrong Spacing activated, is not the 2. but " + Spacing.GetSelText
	if SpacingBy.GetText <> Vergleich4$ then Warnlog "Wrong Value selected. Not: " + Vergleich4$ +" but " + SpacingBy.GetText

	Kontext
	active.SetPage TabHyperlinkZeichen
	kontext "TabHyperlinkZeichen"
	if URLName.GetText <> "http://www.mikeportnoy.com/" then Warnlog "#i105855#Wrong URL : " + URLName.GetText + " (#BugID:90982)"
	SetClipboard UrlName.Gettext
	if NameText.GetText <> "Link created by testtool" then Warnlog "#i105855#Wrong Linkname : " + NameText.GetText
	if TargetFrame.GetSelIndex <> 2 then Warnlog "#i105855#Wrong Target selected : " + TargetFrame.GetSelText
	if UnbesuchterLink.GetSelIndex <> 6 then Warnlog "#i105855#Wrong Style for unused Link :" + UnbesuchterLink.GetSelIndex
	TabHyperlinkZeichen.OK

	printlog "Save document as '../user/work/format1.sxw' and close it"
	if gApplication = "WRITER" then
    	Call hFileSaveAsWithFilterKill ( gOfficePath + "user\work\format1.odt", "writer8" )
	else
    	Call hFileSaveAsWithFilterKill ( gOfficePath + "user\work\format1.odm", "writer8" ) 'gMasterDocFilter
	end if	
	Call hCloseDocument
	printlog "re-open previous saved document"
	if gApplication = "WRITER" then
    	hFileOpen( gOfficePath & "user\work\format1.odt" )
	else
  	    hFileOpen( gOfficePath & "user\work\format1.odm" )
	end if
   	Call wTypeKeys "<Mod1 Home><Right>"
	Wait 500

	printlog "Execute Format->Character and re-check settings made"
	FormatCharacter

	Sleep 3
	Kontext
	active.SetPage TabFont
	kontext "TabFont"
	try
		if FontWest.GetSelText <> Vergleich2$ then Warnlog "Font should "+ Vergleich2$ +" is "+ FontWest.GetSelText
		if StyleWest.GetSelIndex <> Vergleich1% then Warnlog "FontStyle should " +Vergleich3$+" is "+ StyleWest.GetSelText
		if SizeWest.GetSelIndex <> 24 then Warnlog "Wrong Font Size : " + SizeWest.GetSelText
	catch
		if Font.GetSelText <> Vergleich2$ then Warnlog "Font should "+ Vergleich2$ +" is "+ Font.GetSelText
		if Style.GetSelIndex <> Vergleich1% then Warnlog "FontStyle should " +Vergleich3$+" is "+ Style.GetSelText
		if Size.GetSelIndex <> 24 then Warnlog "Wrong Font Size : " + Size.GetSelText
	endcatch
	Kontext
	active.SetPage TabFontEffects
	kontext "TabFontEffects"
	if NOT Blinking.IsChecked then Warnlog "Blinking is not checked"
	Kontext
	Active.Setpage TabFontPosition
	Kontext "TabFontPosition"
	if Spacing.GetSelIndex <> 2 then Warnlog "Wrong Spacing selected. Not 2. but " + Spacing.GetSelText
	if SpacingBy.GetText <> Vergleich4$ then Warnlog "Wrong value selectzed. Not " + Vergleich4$ +" but " + SpacingBy.GetText
	Kontext
	active.SetPage TabHyperlinkZeichen
	kontext "TabHyperlinkZeichen"
	if URLName.GetText <> "http://www.mikeportnoy.com/" then Warnlog "#i105855#Wrong URL : " + URLName.GetText
	if NameText.GetText <> "Link created by testtool" then Warnlog "#i105855#Wrong Linkname : " + NameText.GetText
	if TargetFrame.GetSelIndex <> 2 then Warnlog "#i105855#Wrong Target selected : " + TargetFrame.GetSelText
	TabHyperlinkZeichen.OK
	printlog "Close document"
	Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------

testcase tFormatCharacterOverline1
    Dim iCounter as integer
    Dim CompareValue1 as string
    Dim CompareValue2 as string
    Dim CompareValue3 as string
    Dim CompareValue4 as string
    
    Printlog " New Writer document."
    Call hNewDocument
    Printlog " Format Character."
    FormatCharacter
    Printlog " Tab 'Font Effects'."
    Kontext
    active.SetPage TabFontEffects
    Kontext "TabFontEffects"
    Printlog " Select Overline Listbox."
    Printlog " Default (first) entry is 'Without'."
    Overline.Select (1)
    
    Printlog " Select Colour Listbox."
    Printlog " Verify that Colour List box is DISABLED."
    try
        OverlineColor.Select (1)
        warnlog "Color-listbox was falsely enabled."
    catch
        printlog "Color-listbox is correctly disabled by default."
    endcatch
    
    Printlog " Select Overline Listbox again."
    Printlog " Select second entry 'Single'."
    Overline.Select (2)
    
    Printlog " Step down all 14 entries (Last: 'Double Wave' in English UI)."
    for iCounter = 2 to Overline.GetItemCount
        Overline.Select (iCounter)
	wait (500)
    next iCounter
    
    Printlog " Select Color Listbox."
    Printlog " Verify that Color List box is ENABLED."
    try
        OverlineColor.Select (OverlineColor.GetItemCount)
        printlog "Color-listbox is correctly enabled."
    catch
        warnlog "Color-listbox was falsely disabled."
    endcatch
    
    'CompareValue2 = OverlineColor.GetSelIndex
    
    Printlog " Close dialog."
    TabFontEffects.Ok
    Printlog " Close document."
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------

testcase tFormatCharacterOverline2
    Dim CompareValue1 as string
    Dim CompareValue2 as string
    Dim CompareValue3 as string
    Dim CompareValue4 as string
    Dim SaveFilePathAndName as String
    
    SaveFilePathAndName = gOfficePath + "user\work\OverlineTest2.odt"
    
    Printlog " New Writer document."
    Call hNewDocument
    Printlog " Type 'Overline Single Red' (without quotes)."
    Call wTypeKeys "Overline Single Red"
    Printlog " Select entire string."
    Call wTypeKeys "<HOME>"
    Call wTypeKeys "<SHIFT END>"
    Printlog " Format Character."
    FormatCharacter
    Printlog " Tab 'Font Effects'."
    Kontext
    active.SetPage TabFontEffects
    Kontext "TabFontEffects"
    Printlog " Select Overline Listbox."
    Printlog " Select second entry 'Single'."
    Overline.Select (2)
    
    Printlog " Select Color Listbox."
    Printlog " In Color List box, select Color 'Red', OK."
    OverlineColor.Select (6)
    
    CompareValue1 = Overline.GetSelIndex
    CompareValue2 = OverlineColor.GetSelIndex
    
    Printlog " Close dialog."
    TabFontEffects.Ok
    
    Printlog " Save document."
    Call hFileSaveAsKill ( SaveFilePathAndName )
    Printlog " Reload document."
    FileReload
    Printlog " Select entire string."
    Call wTypeKeys "<HOME><SHIFT END>"
    Printlog " Format Character."
    FormatCharacter
    Printlog " Tab 'Font Effects'."
    Kontext
    active.SetPage TabFontEffects
    Kontext "TabFontEffects"
    Printlog " Verify that 'Overline Single' and Overline Colour 'Red' is set."
    
    If Overline.GetSelIndex <> CompareValue1 then warnlog "The Overline-Value wasn't saved after closing the options-dialogue."
    If OverlineColor.GetSelIndex <> CompareValue2 then warnlog "The Color-Value wasn't saved after closing the options-dialogue."
    
    Printlog " Close dialog."
    TabFontEffects.Ok
    Printlog " Close document."
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------
