'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_contextmenu1.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: vg $ $Date: 2008-08-18 12:25:37 $
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
'* owner : fredrik.haegg@sun.com
'*
'* short description : Pick context for context menus
'*
'\***********************************************************************



sub w_contextmenu1

	Call tPickSimpleText
	Call tPickSimpleTextMultiselection
	Call tPickGraphic
	Call tPickHyperlink
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
    printlog "   Shift + F8 changes selection mode in writer"
	Call wTypeKeys "<Shift F8>"
	Call wTypeKeys "<Mod1 Left>", 3
	Call wTypeKeys "<Right>"
	Call wTypeKeys "<Shift F8>"
	Call DocumentWriter.MouseMove 36, 38
	Call wOpenContextMenu(true)
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
	Call DocumentWriter.MouseMove 32, 38
	Call wOpenContextMenu(true)
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

	Call DocumentWriter.MouseMove 50,30
	Printlog "   Open context menu upon graphic with mouse"
	Call wOpenContextMenu(true)
	
	printlog "   Select 'ImageMap' in context menu"
	try
		Call hMenuSelectNr(7)
		wait 500
	catch
		Warnlog "Unable to open context menu"
		Call hCloseDocument
		goto endsub
	endcatch
	Printlog "   The graphic has to be selected and the context menu has to show graphics related commands"
	Kontext "ImageMapEditor"
	if Not ImageMapEditor.Exists then
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
	Call wTypeKeys "www.sun.com"
	Call wTypeKeys " "

    printlog "   Open context menu on hyperlink"
	Call DocumentWriter.MouseMove 30, 38
	Call wOpenContextMenu(true)
    printlog "   Last entry in menu has to be 'Open hyperlink' ...."
	if MenuGetItemCount = 12 then
		Warnlog "Missing menu entry for hyperlinks to open"
	else
		if MenuGetItemCount = 13 then Call hMenuSelectNr(13)
	end if

    printlog "   Close text document"
	Call hCloseDocument

endcase

' -----------------------------------------------------------------------------

testcase tPickDrawingObjects

	printlog "Context menu on drawing objects"
    printlog "   Open a new text document"
	Call hNewDocument
    printlog "   Set zoom to 100%"
	Call wSetZoom
    printlog "   empty clipboard"
	SetClipBoard = ""
    printlog "   Insert 2 drawing objects (rectangle)"

    Call wZeichenobjektEinfuegen ( "Rechteck", 30, 30, 40, 40 )
	Sleep 2
    Call wZeichenobjektEinfuegen ( "Rechteck", 30, 45, 40, 55 )

    printlog "   Select both objects"
    Call wObjektSelektieren ( 25, 25, 60, 60 )

    printlog "   Open context menu on 1. drawing object"
	Call DocumentWriter.MouseMove 35, 35
	Call wOpenContextMenu(true)
    printlog "   Normally 'copy' is last menu entry"
	Call hMenuSelectNr(12)

    printlog "   Both drawing objects have to leave selected"
	try
        FormatGroupGroup
        Sleep 1
	catch
		Warnlog "Drawing objects did not leave selected after context menu opened"
    endcatch

    printlog "   Close text document"
	Call hCloseDocument

endcase
