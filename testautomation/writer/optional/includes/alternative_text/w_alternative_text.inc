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
'* short description : Test Alternative Text for objects in writer
'*
'\************************************************************************

sub w_alternative_text

	Call tPictureAlternativeText
	Call tFrameAlternativeText
	Call tOLEAlternativeText	

end sub

'-------------------------------------------------------------------------

testcase tPictureAlternativeText

	printlog "- New writer document"	
	Call hNewDocument
    
	printlog "- Insert a picture"	
	InsertGraphicsFromFile
	Kontext "GrafikEinfuegenDlg"
	Dateiname.Settext ConvertPath(gTesttoolpath + "writer\optional\input\alternative_text\jolink.jpg")
	Sleep 1
	Oeffnen.Click
	Sleep 2

    printlog "- Context menu on the picture, choose 'Description'"
	try
		ContextDescriptionObject
	catch
		Warnlog "Seems picture is not selected => Description Object could not be opened"
	endcatch	
	
    printlog "- Type 'TitleText' in the Title field"
	Kontext "DescriptionObject"
	DescriptionTitle.Settext "TitleText"	
	
    printlog "- Type 'This Is A Description'"
	DescriptionText.Settext "This Is A Description"	
	
    printlog "- Close the dialog"
	DescriptionObject.Ok

    printlog "- Context menu on the picture, choose 'Description'"
	try
		ContextDescriptionObject
	catch
		Warnlog "Seems picture is not selected => Description Object could not be opened"
	endcatch	

	printlog "Check that the Title and Description fields have been saved correctly"
	Kontext "DescriptionObject"
	if DescriptionTitle.Gettext <> "TitleText" then
		Warnlog "Object title gets lost. Not 'TitleText' but " & DescriptionTitle.Gettext 	
	endif	
	if DescriptionText.Gettext <> "This Is A Description" then
		warnlog "Object description gets lost. Not 'This Is A Description' but " & DescriptionText.Gettext 	
	endif
	DescriptionObject.Cancel	
	
    printlog "- Format - Picture"
	FormatGraphics
	
	printlog "The 'Alternative' field must be filled with the content of the Title field (='TitleText')"
	Kontext
	Active.Setpage TabZusaetze
	Kontext "TabZusaetze"
	if Alternativtext.Gettext <> "TitleText" then
		Warnlog "Alternatice text in pictures options is not set"
	endif
	TabZusaetze.Cancel
	
    printlog "- Save the document"
	Call hFileSaveAsKill ( gOfficePath + "user\work\tPictureAlternativeText.odt" )
	Call hCloseDocument
	
    printlog "- Reload"
	hFileOpen(  gOfficePath + "user\work\tPictureAlternativeText.odt" )		
	' select graphic
	Call wTypeKeys ("<SHIFT F4>")
	
	printlog "Check that the Title and Description fields have been saved correctly"
	try
		ContextDescriptionObject
	catch
		Warnlog "Seems picture is not selected => Description Object could not be opened"
	endcatch	

	Kontext "DescriptionObject"
	if DescriptionTitle.Gettext <> "TitleText" then
		Warnlog "Object title gets lost. Not 'TitleText' but " & DescriptionTitle.Gettext 	
	endif	
	if DescriptionText.Gettext <> "This Is A Description" then
		warnlog "Object description gets lost. Not 'This Is A Description' but " & DescriptionText.Gettext 	
	endif
	DescriptionObject.Cancel
	
    printlog "- Format - Picture"
	FormatGraphics
	Kontext
	Active.Setpage TabZusaetze
	Kontext "TabZusaetze"
	if Alternativtext.Gettext <> "TitleText" then
		Warnlog "Alternatice text in pictures options is not set after save and reload"
	endif
	TabZusaetze.Cancel

	printlog "- close document"
 	Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tFrameAlternativeText

	printlog "- New writer document"	
	Call hNewDocument
    
	printlog "- Insert a frame"	
	Call wInsertFrame
	
    printlog "- Context menu on the frame, choose 'Description'"
	try
		ContextDescriptionObject
	catch
		Warnlog "Seems Frame is not selected => Description Object could not be opened"
	endcatch	
	
    printlog "- Type 'TitleText' in the Title field"
	Kontext "DescriptionObject"
	DescriptionTitle.Settext "TitleText"	
	
    printlog "- Type 'This Is A Description'"
	DescriptionText.Settext "This Is A Description"	
	
    printlog "- Close the dialog"
	DescriptionObject.Ok

    printlog "- Context menu on the frame, choose 'Description'"
	try
		ContextDescriptionObject
	catch
		Warnlog "Seems frame is not selected => Description Object could not be opened"
	endcatch	

	printlog "Check that the Title and Description fields have been saved correctly"
	Kontext "DescriptionObject"
	if DescriptionTitle.Gettext <> "TitleText" then
		Warnlog "Object title gets lost. Not 'TitleText' but " & DescriptionTitle.Gettext 	
	endif	
	if DescriptionText.Gettext <> "This Is A Description" then
		warnlog "Object description gets lost. Not 'This Is A Description' but " & DescriptionText.Gettext 	
	endif
	DescriptionObject.Cancel		
	
    printlog "- Save the document"
	Call hFileSaveAsKill ( gOfficePath + "user\work\tFrameAlternativeText.odt" )
	Call hCloseDocument
	
    printlog "- Reload"
	hFileOpen(  gOfficePath + "user\work\tFrameAlternativeText.odt" )		
	' select frame
	Call wTypeKeys ("<SHIFT F4>")
	
	printlog "Check that the Title and Description fields have been saved correctly"
	try
		ContextDescriptionObject
	catch
		Warnlog "Seems frame is not selected => Description Object could not be opened"
	endcatch	

	Kontext "DescriptionObject"
	if DescriptionTitle.Gettext <> "TitleText" then
		Warnlog "Object title gets lost. Not 'TitleText' but " & DescriptionTitle.Gettext 	
	endif	
	if DescriptionText.Gettext <> "This Is A Description" then
		warnlog "Object description gets lost. Not 'This Is A Description' but " & DescriptionText.Gettext 	
	endif
	DescriptionObject.Cancel
	
	printlog "- close document"
 	Call hCloseDocument


endcase

'-------------------------------------------------------------------------

testcase tOLEAlternativeText

	printlog "- New writer document"	
	Call hNewDocument
    
	printlog "- Insert a OLE"	
    InsertObjectOLEObject
    Kontext "OLEObjektEinfuegen"
    NeuErstellen.Check
    OLEObjektEinfuegen.Ok
	Sleep 2	
	Call gMouseClick (10,10)	
	Call wTypeKeys "<SHIFT F4>"
	
    printlog "- Context menu on the OLE, choose 'Description'"
	try
		ContextDescriptionObject
	catch
		Warnlog "Seems OLE is not selected => Description Object could not be opened"
	endcatch	
	
    printlog "- Type 'TitleText' in the Title field"
	Kontext "DescriptionObject"
	DescriptionTitle.Settext "TitleText"	
	
    printlog "- Type 'This Is A Description'"
	DescriptionText.Settext "This Is A Description"	
	
    printlog "- Close the dialog"
	DescriptionObject.Ok

    printlog "- Context menu on the OLE, choose 'Description'"
	try
		ContextDescriptionObject
	catch
		Warnlog "Seems OLE is not selected => Description Object could not be opened"
	endcatch	

	printlog "Check that the Title and Description fields have been saved correctly"
	Kontext "DescriptionObject"
	if DescriptionTitle.Gettext <> "TitleText" then
		Warnlog "Object title gets lost. Not 'TitleText' but " & DescriptionTitle.Gettext 	
	endif	
	if DescriptionText.Gettext <> "This Is A Description" then
		warnlog "Object description gets lost. Not 'This Is A Description' but " & DescriptionText.Gettext 	
	endif
	DescriptionObject.Cancel		
	
    printlog "- Save the document"
	Call hFileSaveAsKill ( gOfficePath + "user\work\tOLEAlternativeText.odt" )
	Call hCloseDocument
	
    printlog "- Reload"
	hFileOpen(  gOfficePath + "user\work\tOLEAlternativeText.odt" )		
	' select OLE
	Call wTypeKeys ("<SHIFT F4>")
	
	printlog "Check that the Title and Description fields have been saved correctly"
	try
		ContextDescriptionObject
	catch
		Warnlog "Seems OLE is not selected => Description Object could not be opened"
	endcatch	

	Kontext "DescriptionObject"
	if DescriptionTitle.Gettext <> "TitleText" then
		Warnlog "Object title gets lost. Not 'TitleText' but " & DescriptionTitle.Gettext 	
	endif	
	if DescriptionText.Gettext <> "This Is A Description" then
		warnlog "Object description gets lost. Not 'This Is A Description' but " & DescriptionText.Gettext 	
	endif
	DescriptionObject.Cancel
	
	printlog "- close document"
 	Call hCloseDocument

endcase