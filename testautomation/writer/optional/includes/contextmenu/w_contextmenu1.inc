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
'* short description : Pick context for context menus
'*
'\***********************************************************************

sub w_contextmenu1

    Call tPickSimpleText
    Call tPickSimpleTextMultiselection
    Call tPickGraphic
    Call tPickHyperlink
    Call tHyperlinkEdit
    Call tHyperlinkRemove
    Call tHyperlinkCopy
    Call tPickDrawingObjects

end sub

' -----------------------------------------------------------------------------

testcase tPickSimpleText

    Dim SetThisFontSize as string
    Dim GetThisFontSize as string
    printlog "Open context menu on unselected text"
    printlog "   Open a new text document"
        Call hNewDocument
    printlog "   Set zoom to 100%"
        Call wSetZoom
    printlog "   Write down 'This is a test'"
        Call wTypeKeys "This is a test"
    printlog "   In text document select word 'test'"
        Call wTypeKeys "<End>"
        Call wTypeKeys "<Mod1 Shift Left>"
    printlog "   Open context menu in word 'This'"
        Call wTypeKeys "<Mod1 Left>", 3
        Call wTypeKeys "<Right>"
        Call wTypeKeys "<Shift F10>"
    printlog "   Change font size for word 'This'"
	try
            Call hMenuSelectNr(3)
            wait 500
            SetThisFontSize = hMenuItemGetText(MenuGetItemCount)
            Call hMenuSelectNr(MenuGetItemCount)
	catch
            Warnlog "Unable to open context menu"
	endcatch

	Call wTypeKeys "<Home>"
    printlog "   Check if font size has changed for word 'This' and not for word 'test'"
	Call wTypeKeys "<Right>"
	FormatCharacter
	Kontext
	Active.Setpage TabFont
	Kontext "TabFont"
	if gAsianSup = True or gCTLSup = True then
            GetThisFontSize = SizeWest.GetSelText
	else
            GetThisFontSize = Size.GetSelText
	end if
	TabFont.Ok

	if GetThisFontSize <> SetThisFontSize then Warnlog "Fontsize for word 'This' has not correctly been set"

	Call wTypeKeys "<Mod1 Right>", 3
	Call wTypeKeys "<Right>"
	FormatCharacter
	Kontext
	Active.Setpage TabFont
	Kontext "TabFont"
	if gAsianSup = True or gCTLSup = True then
            GetThisFontSize = SizeWest.GetSelText
	else
            GetThisFontSize = Size.GetSelText
	end if
	TabFont.Ok

	if GetThisFontSize = SetThisFontSize then Warnlog "Fontsize for word 'test' has been set"
	Call hCloseDocument
endcase

' -----------------------------------------------------------------------------

testcase tPickSimpleTextMultiselection
    Dim SetThisFontSize as string
    Dim GetThisFontSize as string
	printlog "   Open context menu on unselected text"
	Call hNewDocument
	Printlog "   Set zoom to 100%"
	Call wSetZoom
	Printlog "   Write down 'This is a test'"
	Call wTypeKeys "This is a test"

	Printlog "   In text document select word 'test' and 'This'"
	Call wTypeKeys "<End>"
	Call wTypeKeys "<Mod1 Shift Left>"	
	Printlog "   Open context menu in word 'This'"
	Printlog "   Shift + F8 changes selection mode in writer"
	Call wTypeKeys "<Shift F8>"
	Call wTypeKeys "<Mod1 Left>", 3
	Call wTypeKeys "<Mod1 Shift Right>"
	Call wTypeKeys "<Shift F8>"
	Call wTypeKeys "<Shift F10>"

	Printlog "   Change font size"
	try
            Call hMenuSelectNr(3)
            wait 500
            SetThisFontSize = hMenuItemGetText(MenuGetItemCount)
            Call hMenuSelectNr(MenuGetItemCount)
	catch
            Warnlog "Unable to open context menu"
	endcatch

	Call wTypeKeys "<Home>"
	Printlog "Check if font size has changed for words 'This' and 'test'"
	Call wTypeKeys "<Right>"
	FormatCharacter
	Kontext
	Active.Setpage TabFont
	Kontext "TabFont"
	if gAsianSup = True or gCTLSup = True then
		GetThisFontSize = SizeWest.GetSelText
	else
		GetThisFontSize = Size.GetSelText
	end if	
	TabFont.Ok

	if GetThisFontSize <> SetThisFontSize then Warnlog "Fontsize for word 'This' has not correctly been set"

	Call wTypeKeys "<Mod1 Right>", 3
	Call wTypeKeys "<Right>"
	FormatCharacter
	Kontext
	Active.Setpage TabFont
	Kontext "TabFont"
	if gAsianSup = True or gCTLSup = True then
            GetThisFontSize = SizeWest.GetSelText
	else
            GetThisFontSize = Size.GetSelText
	end if
	TabFont.Ok

	if GetThisFontSize <> SetThisFontSize then Warnlog "Fontsize for word 'test' has not correctly been set"
    printlog "   Close text document"
    Call hCloseDocument
endcase

' -----------------------------------------------------------------------------

testcase tPickGraphic

	printlog "Open context menu on not focussed graphic"
	Call hNewDocument
	Printlog "   Set zoom to 100%"
	Call wSetZoom
	Printlog "   Insert a graphic into writerdocument"
	Call hGrafikEinfuegen ( gTesttoolPath + "writer\optional\input\menu\drums.jpg" )
	Printlog "   Deselect graphic with 'Escape' and insert some text in text document"
	Call wTypeKeys "<Escape>"
	Call wTypeKeys "Don't be afraid !"

	Call gMouseClick ( 50,30 )
	Printlog "   Open context menu upon graphic with mouse"
	Call wOpenContextMenu(true)

	printlog "   Select 'ImageMap' in context menu"
	try
            CALL hMenuFindSelect (".uno:ImageMapDialog", true, 11, true)   'Find "ImageMap" and call the slot.
            wait 500
	catch
            Warnlog "Unable to open context menu"
            Call hCloseDocument
            goto endsub
	endcatch
	Printlog "   The graphic has to be selected and the context menu has to show graphics related commands"
	Kontext "ImageMapEditor"
	if Not ImageMapEditor.Exists(3) then
            Warnlog "Unfortunately not the context menu from graphics has been used!"
            Kontext
            try
            	Active.Setpage TabFont
            	Kontext "TabFont"
            	TabFont.Cancel
            catch
            endcatch
	else
            ImageMapEditor.Close
            Kontext "Active"
            if Active.Exists then
            	if Active.GetRT = 304 then Active.No
            end if
	end if

	Kontext "DocumentWriter"
	try
            FormatGraphics
            Kontext
            Active.Setpage TabType
            Kontext "TabType"
            TabType.Close
	catch
            Warnlog "Graphic has not been selected!"
	endcatch
    printlog "   Close text document"
    Call hCloseDocument
endcase

' -----------------------------------------------------------------------------

testcase tPickHyperlink

    printlog "   Context menu on hyperlinks"
    printlog "   Open a new text document"
	Call hNewDocument
    printlog "   Set zoom to 100%"
	Call wSetZoom
    printlog "   Empty clipboard"
	SetClipBoard = ""
    printlog "   Insert a hyperlink in document"
	Call wTypeKeys "www.oracle.com"
	Call wTypeKeys " "
	Call wTypeKeys "<HOME>"
    printlog "   Open context menu on hyperlink"
        Call wTypeKeys "<SHIFT F10>"
    printlog "   Last entry in menu has to be 'Open hyperlink' ...."
        CALL hMenuFindSelect (10955, false, 17, true)   'Find "Hyperlink-dialog" and call the slot.
    kontext "TabHyperlinkInternet"
    if TabHyperlinkInternet.Exists(3) then
        Warnlog "Hyperlink-Dialog opened!"
	TabHyperlinkInternet.Close
    end if
    printlog "   Close text document"
    Call hCloseDocument
endcase

' -----------------------------------------------------------------------------

testcase tHyperlinkEdit

    printlog "Context menu on hyperlinks"
    printlog "   Open a new text document"
	CALL hNewDocument
    printlog "   Set zoom to 100%"
	Call wSetZoom
    printlog "   Empty clipboard"
	SetClipBoard = ""
    printlog "   Insert a hyperlink in document"
	Call wTypeKeys "www.openoffice.org"
	Call wTypeKeys " "
	Call wTypeKeys "<HOME>"
    printlog "   Open context menu on hyperlink"
    Call wTypeKeys "<SHIFT F10>"

    printlog "   Open contextmenu, find the entry 'Edit Hyperlink', and call the slot..."
    CALL hMenuFindSelect (21835, true, 18, true)   'Find "Edit Hyperlink" and call the slot.
    printlog "   Check if the Hyperlink-dialogue comes up correctly."
    kontext "HyperlinkDialog"
    if NOT HyperlinkDialog.Exists(3) then
        Warnlog "No Hyperlink-Dialog visible!"
	else
	    HyperlinkDialog.Close
    end if
    printlog "   Close text document"
    CALL hCloseDocument
endcase

' -----------------------------------------------------------------------------

testcase tHyperlinkRemove

    printlog "Context menu on hyperlinks"
    printlog "   Open a new text document"
	CALL hNewDocument
        kontext "TabHyperlinkInternet"
        if TabHyperlinkInternet.Exists then
	    TabHyperlinkInternet.Close
        end if
        Kontext "DocumentWriter"
    printlog "   Set zoom to 100%"
	Call wSetZoom
    printlog "   Empty clipboard"
	SetClipBoard = ""
    printlog "   Insert a hyperlink in document"
	Call wTypeKeys "www.openoffice.org"
	Call wTypeKeys " "
	Call wTypeKeys "<HOME>"
    printlog "   Open context menu on hyperlink"
        Call wTypeKeys "<SHIFT F10>"
    printlog "   Find the entry 'Remove hyperlink' in the menu... Check the position of the entry, and call it."
        CALL hMenuFindSelect (21839, true, 20, true)   'Find "Remove hyperlink" and call the slot.
    Printlog "   Check that the hyperlink formatting has been removed and 'Format - Character - Hyperlink - URL' is empty."
	Call wTypeKeys "<HOME>"
    printlog "   Open context menu on the hyperlink."
        Call wTypeKeys "<SHIFT F10>"
    printlog "   Check that the Menu has less than 18 entries, otherwise the pasted content is wrong."
        if MenuGetItemCount > 18 then 
	    Warnlog "   Either error or the contextmenu has changed - contact testowner."
            CALL hMenuFindSelect (21839, false, 20, false)   'Check if "Remove hyperlink" exists in the menu.
            kontext "TabHyperlinkInternet"
            if TabHyperlinkInternet.Exists(3) then
                Warnlog "   ERROR. The Hyperlink-Status of the text wasn't removed. BUG!"
	        TabHyperlinkInternet.Close
            end if
	end if
    printlog "   Close text document"
        CALL hCloseDocument
endcase

' -----------------------------------------------------------------------------

testcase tHyperlinkCopy

    printlog "Context menu on hyperlinks"
    printlog "   Open a new text document"
	Call hNewDocument
    printlog "   If the Hyperlink-Dialog is open - Close it."
        kontext "TabHyperlinkInternet"
        if TabHyperlinkInternet.Exists then
            TabHyperlinkInternet.Close
        end if
        kontext "DocumentWriter"
    printlog "   Set zoom to 100%"
	Call wSetZoom
    printlog "   Empty clipboard"
	SetClipBoard = ""
    printlog "   Insert a hyperlink in document"
	Call wTypeKeys "www.openoffice.org"
	Call wTypeKeys " "
	Call wTypeKeys "<HOME><RIGHT RIGHT RIGHT>"
    printlog "   Open context menu on hyperlink"
        Call wTypeKeys "<SHIFT F10>"
    printlog "    Select 'Copy Hyperlink Location'..."
        CALL hMenuFindSelect (21840, true, 19, true)   'Find "Copy Hyperlink Location" and call the slot.
    printlog "   Check that the Clipboard contains something/the string we wanted to copy."
        if GetClipboard = "" then
	    warnlog "   Clipboard empty, notify testowner and check if the Hyperlink Copy-function works."
	end if
    printlog "   Set the cursor somwhere else in the document."
	Call wTypeKeys "<END>"
	Call wTypeKeys "<RETURN>", 10
    printlog "   Paste."
        EditPaste
    printlog "   Check that the hyperlink URL is pasted as plain text."
    printlog "   Mark the text we just pasted."
	Call wTypeKeys "<HOME><RIGHT RIGHT RIGHT>"
    printlog "   We do this by opening the context menu on the text-string."
        Call wTypeKeys "<SHIFT F10>"
    printlog "   Check if the contextmenu contains anything about 'Hyperlink'."
        CALL hMenuFindSelect (10955, true, 17, false) 'Try to find and call "Open Hyperlink".
        kontext "TabHyperlinkInternet"
        if TabHyperlinkInternet.Exists(3) then
            Warnlog "   Seemingly, the copy wasn't plaintext, but also a hyperlink. BUG!"
        end if
    printlog "   Close text document"
        Call hCloseDocument
endcase

' -----------------------------------------------------------------------------

testcase tPickDrawingObjects

    printlog "Context menu on drawing objects"
    printlog "   Open a new text document"
	Call hNewDocument
    printlog "   If the Hyperlink-Dialog is open - Close it."
        kontext "TabHyperlinkInternet"
        if TabHyperlinkInternet.Exists then
            TabHyperlinkInternet.Close
        end if
        kontext "DocumentWriter"
    printlog "   Set zoom to 100%"
	Call wSetZoom
    printlog "   empty clipboard"
	SetClipBoard = ""
    printlog "   Insert 2 drawing objects (rectangle)"

        Call wZeichenobjektEinfuegen ( "Rechteck", 35, 40, 52, 52 )
        Call wTypeKeys "<Escape>"
        Sleep 1
        Call wZeichenobjektEinfuegen ( "Rechteck", 35, 35, 50, 50 )
        Call wTypeKeys "<Escape>"
        Sleep 1
    printlog "   Select both objects"
        Call wObjektSelektieren ( 32, 32, 60, 60 )
        Sleep 1

    printlog "   Open context menu on 1. drawing object"
	Call DocumentWriter.MouseMove 51, 51
	Call wOpenContextMenu(true)
    printlog "   Normally 'copy' is last menu entry"
        CALL hMenuFindSelect (5711, true, 15, true)   'Find "Copy" and call the slot.

    printlog "   Both drawing objects have to leave selected"
    try
        FormatGroupGroup
        Sleep 1
    catch
        Warnlog "Drawing objects did not leave selected after context menu opened"
    endcatch

    printlog "   Close text document"
    CALL hCloseDocument
endcase

' -----------------------------------------------------------------------------
