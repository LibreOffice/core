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
'* short description : Test the functionality of Page Formatting / Page Styles - 1
'*
'************************************************************************
'*
' #1 tFormatPage_1       'Open format/page dialog
' #1 tFormatPage_2       'Test Portrait in tabpage Page
' #1 tFormatPage_3       'Test Landscape in tabpage Page
' #1 tFormatPage_4       'Width and Height boxes must change after changing between Portrait and Landscape
' #1 tFormatPage_5       'The sum of "Left margin" and "Right margin" must always stay below the Paper width.(Same for top/bottom and height)
' #1 tFormatPage_6       'Test predefined formats
' #1 tFormatPage_7       'Test user defined formats
' #1 tFormatPage_8       'Check Listbox Text Direction when Asian language support is enabled
' #1 tFormatPage_9       'Check Listbox Text Direction when CTL is enabled
' #1 tFormatPage_10      'when Asian support and CTL are all enabled , 3 entries should be in Text Direction listbox
' #1 TFormatPage_11      'Test Page layout - Right and left
' #1 tFormatPage_12      'Test Page layout - Mirrored
' #1 tFormatPage_13      'Test Page layout - Only right
' #1 tFormatPage_14      'Test Page layout - Only left
' #1 tFormatPage_15      'Insertfields / other / Document / Type/Page -> As page Style must be set
'*
'\***********************************************************************

testcase tFormatPage_1

	PrintLog "- Open format/page dialog"
	'/// <b> Open format/page dialog </b> ///

	Call hNewDocument

	'/// 1. via Mouse clicks in Menu: Format-Page
	try
		fFormatPageWriter("TabSeite")
		TabSeite.Cancel
	catch
	endcatch

	'/// 2. via shortcuts in menu  (manual test is needed)

	'/// 3. via Context menu when Cursor is in the document
	try
		hMenuSelectNr(5)
		Sleep 2
		hMenuSelectNr(5)
		Sleep 2
		Kontext
		Active.SetPage TabSeite
		Kontext "TabSeite"
		TabSeite.Cancel
	catch
	endcatch

	Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_2

  	Dim sTestString   as String
	Dim iRepectLength as Integer
	Dim iFormat       as Integer

	sTestString   =  "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQ"
	iRepectLength =  26
	iFormat       =  2   'A4 paper

	PrintLog "- Test Portrait in tabpage Page"
	'/// <b> Test Portrait in tabpage Page </b> ///

	Call hNewDocument

	'/// Input a long test string
    Call wTypeKeys sTestString

   '/// Format/Page , tabpage page , check 'Portrait'
    fFormatPageWriter("TabSeite")
    Papierformat.Select iFormat
    Wait 500
    Hoch.Check
    Wait 500
    TabSeite.OK

	'/// Check if Checkbox -- Portrait works fine
    Call wTypeKeys "<Shift Home>"
    EditCopy
    wait 500
    if Len(GetClipboardText) < iRepectLength then Warnlog "Checkbox Portrait doesn't work fine!"

	Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_3

    Dim iTextLength as integer

	PrintLog "- Test Landscape in tabpage Page"
	'/// Test Landscape in tabpage Page

	Call hNewDocument

	Call wOptionsUndo( "Grundschriften" )

	'/// Input a long test string
	Call wTypeKeys "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQ"
    Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    iTextLength = Len(GetClipBoardtext)

	'/// Format/Page , tabpage page , check 'Landscape'
	fFormatPageWriter("TabSeite")
	Papierformat.Select 2 ' A4 Paper
	Wait 500
	Quer.Check
	Wait 500
	TabSeite.OK

	'/// Check if Checkbox -- Landscape works fine
	Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys "<Shift End>"
	EditCopy
	wait 500
	if Len(GetClipboardText) <= iTextLength then Warnlog "Checkbox Landscape doesn't work fine!"

	Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_4

	Dim iFormat as Integer
	Dim sWidth  as String
    Dim sHeight as String

	iFormat = 2   'A4 paper

	PrintLog "- Width and Height boxes must change after changing between Portrait and Landscape"
	'/// Width and Height boxes must change after changing between Portrait and Landscape

	Call hNewDocument

	'/// Format/Page  tabpage page ,
	'/// + check 'Portrait' first and get width and height ,
	'/// + then change from Portrait to Landscape
	'/// + check if the change is effective
	fFormatPageWriter("TabSeite")
	Papierformat.Select iFormat
	Wait 500
	Hoch.Check
	Sleep 1
	sWidth  = Breite.GetText
	sHeight = Hoehe.GetText

	Quer.Check
	Sleep 1
	if Breite.GetText <> sHeight  then Warnlog "Width isn't changed when changing from Portrait to Landscape !"
	if Hoehe.GetText  <> sWidth   then Warnlog "Height isn't changed when changing from Portrait to Landscape !"
	TabSeite.Cancel

	'/// Format/Page  tabpage page ,
	'/// + check 'Landscape' first and get width and height ,
	'/// + then change from Landscape to Portrait
	'/// + check if the change is effective
	fFormatPageWriter("TabSeite")
	Papierformat.Select iFormat
	Wait 500
	Quer.Check
	Sleep 1
	sWidth  = Breite.GetText
	sHeight = Hoehe.GetText

	Hoch.Check
	Sleep 1
	if Breite.GetText <> sHeight  then Warnlog "Width isn't changed when changing from Landscape to Portrait !"
	if Hoehe.GetText  <> sWidth   then Warnlog "Height isn't changed when changing from Landscape to Portrait !"
	TabSeite.Cancel

	Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_5

	Dim iFormat as Integer
	Dim sWidth  as String , sHeight as String
	Dim sLeft   as String , sRight as String   , sUp as String  , sBottom as String  ' set page size
	Dim sLeft1 as String  , sRight1 as String  ,  sUp1 as String  , sBottom1 as String  ' get page size

	iFormat    = 2   'A4 paper
	sWidth     = "20" + gSeperator + "00" + gMeasurementUnit
	sHeight    = "25" + gSeperator + "00" + gMeasurementUnit

	sLeft      = "15" + gSeperator + "00" + gMeasurementUnit
	sRight     = "15" + gSeperator + "00" + gMeasurementUnit
	sUp        = "15" + gSeperator + "00" + gMeasurementUnit
	sBottom    = "15" + gSeperator + "00" + gMeasurementUnit

	sLeft1     = "15" + gSeperator + "00" + gMeasurementUnit
	sRight1    = "4"  + gSeperator + "50" + gMeasurementUnit
	sUp1       = "15" + gSeperator + "00" + gMeasurementUnit
	sBottom1   = "9"  + gSeperator + "50" + gMeasurementUnit

	PrintLog "- The sum of 'Left margin' and 'Right margin' must always stay below the Paper width."
	PrintLog "  (Same for top/bottom and height)"
	'/// The sum of "Left margin" and "Right margin" must always stay below the Paper width.
	'/// (Same for top/bottom and height)

	Call hNewDocument

	'/// Format/Page , tabpage Page , set width and height , close the dialog
	fFormatPageWriter("TabSeite")
	Papierformat.Select iFormat
	Wait 500
	Hoch.Check
	Sleep 1
	Breite.SetText sWidth
	Wait 500
	Hoehe.SetText sHeight
	Wait 500
	TabSeite.OK

	'/// Format/Page , tabpage Page ,
	'/// + Set the sum of "Left margin" and "Right margin" more than Width
	fFormatPageWriter("TabSeite")
	Links.SetText   sLeft
	Wait 500
	Rechts.SetText  sRight
	Wait 500
	TabSeite.OK

	'/// Check if the setting is OK
	fFormatPageWriter("TabSeite")
	if Left$(Links.GetText,5)   <>  Left$(sLeft1,5)   then Warnlog "The Margin(left) is wrong!"
	if Left$(Rechts.GetText,4)  <>  Left$(sRight1,4)  then Warnlog "The Margin(right) is wrong!"
	TabSeite.Cancel

	'/// Format/Page , tabpage Page ,
	'/// + Set the sum of "Up margin" and "Bottom margin" more than Heigth
	fFormatPageWriter("TabSeite")
	Oben.SetText   sUp
	Wait 500
	Unten.SetText  sBottom
	Wait 500
	TabSeite.OK

	'/// Check if the setting is OK
	fFormatPageWriter("TabSeite")
	if Left$(Oben.GetText,5)   <>  Left$(sUp1,5)      then Warnlog "The Margin(Up) is wrong!"
	if Left$(Unten.GetText,4)  <>  Left$(sBottom1,4)  then Warnlog "The Margin(Bottom) is wrong!"
	TabSeite.Cancel

	Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_6

	Dim iPaperTray as Integer  ,  iPageLayout as Integer
	Dim iFormatLayout as Integer  ,  iReferenceStyle as Integer
	Dim sWidth as String, sHeight as String
	Dim sLeft as String  ,  sRight as String  ,  sUp as String  ,  sBottom as String

	iPaperTray    = 1 :  iPageLayout     = 2
	iFormatLayout = 2 :  iReferenceStyle = 5

	sWidth        = "20" + gSeperator + "00" + gMeasurementUnit
	sHeight       = "25" + gSeperator + "00" + gMeasurementUnit
	sLeft         = "2"  + gSeperator + "00" + gMeasurementUnit
	sRight        = "2"  + gSeperator + "00" + gMeasurementUnit
	sUp           = "2"  + gSeperator + "00" + gMeasurementUnit
	sBottom       = "2"  + gSeperator + "00" + gMeasurementUnit

	PrintLog "- Test Pre-defined formats"
	'/// Test Pre-defined formats

	Call hNewDocument

	'/// Format/Page , tabpage page , do some configuration
	fFormatPageWriter("TabSeite")
	Hoch.Check
	Wait 500
	Breite.SetText       sWidth
	Wait 500
	Hoehe.SetText        sHeight
	Wait 500
	Schacht.Select       iPaperTray
	Wait 500
	Links.SetText        sLeft
	Wait 500
	Rechts.SetText       sRight
	Wait 500
	Oben.SetText         sUp
	Wait 500
	Unten.SetText        sBottom
	Wait 500
	Seitenlayout.Select  iPageLayout
	Wait 500
	Numerierung.Select   iFormatLayout
	Wait 500
	Einschalten.Check
	Sleep 1
	AbsatzVorlage.Select iReferenceStyle
	Wait 500
	TabSeite.OK

	'/// Save the file  , close the file and reload the file
    Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\tFormatPage_6.odt", "writer8")
	Call hCloseDocument

	Call hNewDocument

	hFileOpen( gOfficepath + "user\work\tFormatPage_6.odt")

	'/// Check if Pre-defined formats are correct
	fFormatPageWriter("TabSeite")
	if Hoch.IsChecked            <> TRUE             then Warnlog "Portrait should be checked!"
	if Left$(Breite.GetText,5)   <> Left$(sWidth,5)  then Warnlog "The width is wrong!"
	if Left$(Hoehe.GetText,5)    <> Left$(sHeight,5) then Warnlog "The height is wrong!"
	if Schacht.GetSelIndex       <> iPaperTray       then Warnlog "The Paper tray is wrong!"
	if Left$(Links.GetText,4)    <> Left$(sLeft,4)   then Warnlog "The Margin(left) is wrong!"
	if Left$(Rechts.GetText,4)   <> Left$(sRight,4)  then Warnlog "The Margin(right) is wrong!"
	if Left$(Oben.GetText,4)     <> Left$(sUp,4)     then Warnlog "The Margin(top) is wrong!"
	if Left$(Unten.GetText,4)    <> Left$(sBottom,4) then Warnlog "The Margin(bottom) is wrong!"
	if Seitenlayout.GetSelIndex  <> iPageLayout      then Warnlog "The paper layout is wrong!"
	if Numerierung.GetSelIndex   <> iFormatLayout    then Warnlog "The layout settings(format) is wrong!"
	if Einschalten.IsChecked     <> TRUE             then Warnlog "Portrait should be checked!"
	if AbsatzVorlage.GetSelIndex <> iReferenceStyle  then Warnlog "The reference style is wrong!"
	TabSeite.Cancel

	Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_7

	Dim iPaperTray as Integer  ,  iPageLayout as Integer
	Dim iFormatLayout as Integer  ,  iReferenceStyle as Integer
	Dim sWidth as String  ,  sHeight as String
	Dim sLeft as String  ,  sRight as String  ,  sUp as String  ,  sBottom as String

	iPaperTray    = 1 :  iPageLayout     = 2
	iFormatLayout = 2 :  iReferenceStyle = 5

	sWidth        = "20" + gSeperator + "00" + gMeasurementUnit
	sHeight       = "25" + gSeperator + "00" + gMeasurementUnit
	sLeft         = "2"  + gSeperator + "00" + gMeasurementUnit
	sRight        = "2"  + gSeperator + "00" + gMeasurementUnit
	sUp           = "2"  + gSeperator + "00" + gMeasurementUnit
	sBottom       = "2"  + gSeperator + "00" + gMeasurementUnit

	PrintLog "- Test user defined formats"
	'/// Test user defined formats

	Call hNewDocument

	'/// Format/Page , tabpage page , do some configuration
	fFormatPageWriter("TabSeite")
	Hoch.Check
	Wait 500
	Breite.SetText       sWidth
	Wait 500
	Hoehe.SetText        sHeight
	Wait 500
	Schacht.Select       iPaperTray
	Wait 500
	Links.SetText        sLeft
	Wait 500
	Rechts.SetText       sRight
	Wait 500
	Oben.SetText         sUp
	Wait 500
	Unten.SetText        sBottom
	Wait 500
	Seitenlayout.Select  iPageLayout
	Wait 500
	Numerierung.Select   iFormatLayout
	Wait 500
	Einschalten.Check
	Sleep 1
	AbsatzVorlage.Select iReferenceStyle
	Wait 500
	TabSeite.OK

	'/// Check if user defined formats are correct
	fFormatPageWriter("TabSeite")
	if Hoch.IsChecked            <> TRUE                then Warnlog "Portrait should be checked!"
	if Left$(Breite.GetText,5)   <> Left$(sWidth,5)     then Warnlog "The width is wrong!"
	if left$(Hoehe.GetText,5)    <> Left$(sHeight,5)    then Warnlog "The height is wrong!"
	if Schacht.GetSelIndex       <> iPaperTray          then Warnlog "The Paper tray is wrong!"
	if Left$(Links.GetText,4)    <> Left$(sLeft,4)      then Warnlog "The Margin(left) is wrong!"
	if Left$(Rechts.GetText,4)   <> Left$(sRight,4)     then Warnlog "The Margin(right) is wrong!"
	if Left$(Oben.GetText,4)     <> Left$(sUp,4)        then Warnlog "The Margin(top) is wrong!"
	if Left$(Unten.GetText,4)    <> Left$(sBottom,4)    then Warnlog "The Margin(bottom) is wrong!"
	if Seitenlayout.GetSelIndex  <> iPageLayout         then Warnlog "The paper layout is wrong!"
	if Numerierung.GetSelIndex   <> iFormatLayout       then Warnlog "The layout settings(format) is wrong!"
	if Einschalten.IsChecked     <> TRUE                then Warnlog "Portrait should be checked!"
	if AbsatzVorlage.GetSelIndex <> iReferenceStyle     then Warnlog "The reference style is wrong!"
	TabSeite.Cancel

	Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_8
    Dim RecheckCTLSupport   as string
    Dim RecheckAsianSupport as string
    RecheckCTLSupport = gCTLSup
    RecheckAsianSupport = gAsianSup

    Select Case iSystemSprache
        Case 81, 82, 86, 88
            printlog "No test with asian lovale, because checkbox 'Asian language support' is disabled by default!"
            goto endsub
    end select

    PrintLog "- Check Listbox Text Direction when Asian language support is enabled"
    '/// Check Listbox Text Direction when Asian language support is enabled

    Call hNewDocument

   '/// Set CTL unable
    Call ActiveDeactivateCTLSupport(FALSE)
    '/// Enable Asian Support
    Call ActiveDeactivateAsianSupport(TRUE)
   '/// - the Listbox Text Direction should be visible when Asian language support is enabled
    Call fFormatPageWriter("TabSeite")
    if Not Textfluss.IsVisible then
        Warnlog "Listbox 'Text Direction' isn't visible though asian support is enabled"
    end if
    TabSeite.Cancel

    '/// Disable Asian Support
    Call ActiveDeactivateAsianSupport(FALSE)
    '/// - the Listbox Text Direction should be invisible when Asian language support is unabled
    Call fFormatPageWriter("TabSeite")
    if Textfluss.IsVisible then
        Warnlog "Listbox 'Text Direction' is visible though asian support is disabled"
    end if
    TabSeite.Cancel

    if RecheckCTLSupport = True then
        Call ActiveDeactivateCTLSupport(TRUE)
    else
        Call ActiveDeactivateCTLSupport(FALSE)
    end if
    if RecheckAsianSupport = True then
        Call ActiveDeactivateAsianSupport(TRUE)
    else
        Call ActiveDeactivateAsianSupport(FALSE)
    end if

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_9

    Dim RecheckCTLSupport as string
    Dim RecheckAsianSupport as string
    RecheckCTLSupport = gCTLSup
    RecheckAsianSupport = gAsianSup

    PrintLog "- Check Listbox Text Direction when CTL is enabled"
    '/// Check Listbox Text Direction when CTL is enabled

    Call hNewDocument

    '/// Set CTL enabled
    Call ActiveDeactivateAsianSupport(FALSE)
    Call ActiveDeactivateCTLSupport(TRUE)
    '/// Check if 'Text direction' appears
    Call fFormatPageWriter("TabSeite")
    if Textfluss.IsVisible = False then
        Warnlog "Right to Left (horizontal) should appear when CTL is checked!"
    end if
    TabSeite.Cancel

    '/// Set CTL unable
    Call ActiveDeactivateCTLSupport(FALSE)

    '/// Check if 'Text direction' appears
    Call fFormatPageWriter("TabSeite")
    if Textfluss.IsVisible = TRUE then
        Warnlog "Left to Right (horizontal) appears though CTL is disabled"
    end if
    TabSeite.Cancel

    if RecheckCTLSupport = True then
        Call ActiveDeactivateCTLSupport(TRUE)
    else
        Call ActiveDeactivateCTLSupport(FALSE)
    end if
    if RecheckAsianSupport = True then
        Call ActiveDeactivateAsianSupport(TRUE)
    else
        Call ActiveDeactivateAsianSupport(FALSE)
    end if

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_10

    Dim RecheckCTLSupport   as string
    Dim RecheckAsianSupport as string
    RecheckCTLSupport = gCTLSup
    RecheckAsianSupport = gAsianSup

    PrintLog "- when Asian support and CTL are all enabled , 3 entries should be in Text Direction listbox"
    '/// when Asian support and CTL are all enabled , 3 entries should be in Text Direction listbox

    Call hNewDocument

    '/// Set Asian support enabled and Set CTL enabled
    Call ActiveDeactivateAsianSupport(TRUE)
    Call ActiveDeactivateCTLSupport(TRUE)

    '/// Check if there are 3 entires in Text Direction listbox
    fFormatPageWriter("TabSeite")
    if Textfluss.GetItemCount <> 3 then
        Warnlog "There should be 3 entries in Text direction listbox , but get " & Textfluss.GetItemCount
    end if
    TabSeite.Cancel

    if RecheckCTLSupport = True then
        Call ActiveDeactivateCTLSupport(TRUE)
    else
        Call ActiveDeactivateCTLSupport(FALSE)
    end if
    if RecheckAsianSupport = True then
        Call ActiveDeactivateAsianSupport(TRUE)
    else
        Call ActiveDeactivateAsianSupport(FALSE)
    end if

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_11

    Dim sTestFile    as String
    Dim sMarginsLeft as String
    Dim iPageLayout  as Integer

    sTestFile    = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\pageLayout.sxw")
    sMarginsLeft = "10" + gSeperator + "00" + gMeasurementUnit
    iPageLayout  = 1

    PrintLog "- Test Page layout - Right and left"
    '/// Test Page layout - Right and left

     Call hNewDocument

    '/// Open a test file ,
    '/// + There are 2 pictures in 2 pages separately ,
    '/// + and they are anchored as character
     hFileOpenLocally( sTestFile )
     

    '/// FormatPage / tabpage Page , Set left margins to 10cm ,
    '/// + and set page layout to "Right to left"
     fFormatPageWriter("TabSeite")
     Links.SetText  sMarginsLeft
     Sleep 1
     Seitenlayout.Select iPageLayout
     Sleep 1
     TabSeite.OK

    '/// Set focus to first picture , format/anchor to Page
    Call wTypeKeys ( "<Shift F4>" )
    Call wTypeKeys "<Tab>"

    FormatAnchorToPage
    Sleep 1

    '/// Check its position
     fPositionAndSize("TabPositionAndSizeWriter")
     if Left$(Horizontalby.Gettext,5) <> Left$(sMarginsLeft,5) then  Warnlog "1-The position X isn't right , should be " &sMarginsLeft & " but get " & Horizontalby.Gettext
     TabPositionAndSizeWriter.Cancel

    '/// Set focus to second picture , format/anchor to Page
     Call wTypeKeys "<Tab>"

     FormatAnchorToPage
     Sleep 1

    '/// Check its position
     fPositionAndSize("TabPositionAndSizeWriter")
     if Left$(Horizontalby.Gettext,5) <> Left$(sMarginsLeft,5) then  Warnlog "2-The position X isn't right , should be " &sMarginsLeft & " but get " & Horizontalby.Gettext
     TabPositionAndSizeWriter.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_12

    Dim sTestFile    as String
    Dim sMarginsLeft as String
    Dim sMarginsRight as String
    Dim iPageLayout  as Integer

    sTestFile     = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\pageLayout.sxw")
    sMarginsLeft  = "10" + gSeperator + "00" + gMeasurementUnit
    sMarginsRight = "4"  + gSeperator + "00" + gMeasurementUnit
    iPageLayout   = 2

    PrintLog "- Test Page layout - Mirrored"
    '/// Test Page layout - Mirrored

     Call hNewDocument

    '/// Open a test file ,
    '/// + there are 2 pictures in 2 pages separately , and they are anchored as character
     hFileOpenLocally( sTestFile )
     

    '/// FormatPage / tabpage Page , Set left margins to 10cm ,
    '/// + and set page layout to "Mirrored"
     fFormatPageWriter("TabSeite")
     Links.SetText  sMarginsLeft
     Sleep 1
     Rechts.SetText sMarginsRight
     Sleep 1
     Seitenlayout.Select iPageLayout
     Sleep 1
     TabSeite.OK

    '/// Set focus to first picture , format/anchor to Page
    Call wTypeKeys ( "<Shift F4>" )
    Call wTypeKeys "<Tab>"

     FormatAnchorToPage
     Sleep 1

    '/// Check its position
     fPositionAndSize("TabPositionAndSizeWriter")
     if Left$(Horizontalby.Gettext,5) <> Left$(sMarginsLeft,5) then  Warnlog "1-The position X isn't right , should be " &sMarginsLeft & " but get " & Horizontalby.Gettext
     TabPositionAndSizeWriter.Cancel

    '/// Set focus to second picture , format/anchor to Page
     Call wTypeKeys "<Tab>"

     FormatAnchorToPage
     Sleep 1

    '/// Check its position
     fPositionAndSize("TabPositionAndSizeWriter")
     if Left$(Horizontalby.Gettext,4) <> Left$(sMarginsRight,4) then  Warnlog "2-The position X isn't right , should be " &sMarginsRight & " but get " & Horizontalby.Gettext
     TabPositionAndSizeWriter.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_13

    Dim sTestFile as String
    Dim sPage1 as String
    Dim sPage2 as String
    Dim iPageLayout as Integer

    sTestFile     = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\pageLayout.sxw")
    sPage1        = "1"
    sPage2        = "3"
    iPageLayout   = 3

    PrintLog "- Test Page layout - Only right"
    '/// Test Page layout - Only right

     Call hNewDocument

    '/// Open a test file ,
    '/// + there are 2 pictures in 2 pages separately
     hFileOpenLocally( sTestFile )
     

    '/// FormatPage / tabpage Page , Set left margins to 10cm ,
    '/// + and set page layout to "Only right"
     fFormatPageWriter("TabSeite")
     Seitenlayout.Select iPageLayout
     Sleep 1
     TabSeite.OK

    '/// Set focus to first picture , Check if the picture is in first page
    Call wTypeKeys ( "<Shift F4>" )
    Call wTypeKeys "<Tab>"

	Kontext "Navigator"
	if Not Navigator.Exists then ViewNavigator
	Kontext "NavigatorWriter"
	Sleep 1
	if Seitennummer.GetText <> sPage1 then Warnlog "1st pic isn't in page 1 !"
	fCloseNavigator
	Wait 500

    '/// Set focus to second picture , Check if the picture is in third page
     Call wTypeKeys "<Tab>"

     Kontext "Navigator"
     if Not Navigator.Exists then ViewNavigator
     Kontext "NavigatorWriter"
     Sleep 1
     if Seitennummer.GetText <> sPage2 then Warnlog "#i40333# 2nd pic isn't in page 3 !"
     fCloseNavigator
     Wait 500

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_14
    Dim sTestFile as String
    Dim sPage1 as String
    Dim sPage2 as String
    Dim iPageLayout as Integer

    sTestFile     = Convertpath (gTesttoolpath + "writer\optional\input\formatpage\pageLayout.sxw")
    sPage1        = "2"
    sPage2        = "4"
    iPageLayout   = 4

    PrintLog "- Test Page layout - Only left"
    '/// Test Page layout - Only left

     Call hNewDocument

    '/// Open a test file ,
    '/// + there are 2 pictures in 2 pages separately
     hFileOpenLocally( sTestFile )
     

    '/// FormatPage / tabpage Page , Set left margins to 10cm ,
    '/// + and set page layout to "Only left"
     fFormatPageWriter("TabSeite")
     Seitenlayout.Select iPageLayout
     Sleep 1
     TabSeite.OK

    '/// Set focus to first picture , Check if the picture is in first page
    Call wTypeKeys ( "<Shift F4>" )
    Call wTypeKeys "<Tab>"

     Kontext "Navigator"
     if Not Navigator.Exists then ViewNavigator
     Kontext "NavigatorWriter"
     Sleep 1
     if Seitennummer.GetText <> sPage1 then Warnlog "1st pic isn't in page 2 !"
     fCloseNavigator
     Wait 500

    '/// Set focus to second picture , Check if the picture is in third page
     Call wTypeKeys "<Tab>"

     Kontext "Navigator"
     if Not Navigator.Exists then ViewNavigator
     Kontext "NavigatorWriter"
     Sleep 1
     if Seitennummer.GetText <> sPage2 then Warnlog "#i40333# 2nd pic isn't in page 4 !"
     fCloseNavigator
     Wait 500

    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-------------------------------------------------------------------------------------------

testcase tFormatPage_15
    Dim sType as String
    Dim sFormat as String

    Select case iSprache
        case 01:    sType   = "Page"
                    sFormat = "As Page Style"
        case 03:    sType   = "Página"
                    sFormat = "Como estilo de página"
        case 31:    sType = "Pagina"
                    sFormat = "Zoals paginaopmaakprofiel"
        case 33   : sType   = "Page"
                    sFormat = "D'après le style de page"
        case 34   : sType   = "Página"
                    sFormat = "Como estilo de página"
        case 39   : sType   = "Pagina"
                    sFormat = "Come modello di pagina"
        case 46:    sType   = "Sida"
                    sFormat = "Som sidformatmall"
        case 49:    sType   = "Seite"
                    sFormat = "Wie Seitenvorlage"
        case 50:    sType   = "Stran"
                    sFormat = "Kot predloga za stran"
        case 55   : sType   = "Estatísticas"
                    sFormat = "Como Estilo de Página"
        case 81   : sType   = "ページ"
                    sFormat = "ページスタイル�?�設定"
        case 82   : sType   = "통계"
                    sFormat = "페�?�지 스타�?� 설정"
        case 86   : sType   = "页"
                    sFormat = "和页�?�样�?相�?�"
        case 88   : sType   = "�?"
                    sFormat = "和�?�?�樣�?相�?�"
        case else : QAErrorLog "The test does not support the language " + iSprache
        Goto endsub
    end select

    PrintLog "- Insertfields / other / Document / Type/Page -> As page Style must be set."
    '/// Insertfield / other / Document / Type/Page -> As page Style must be set.

    Call hNewDocument

    '/// InsertFields / other , tabpage document , select page ,
    '/// + check if Page format is right
    Call fInsertFieldsOther("TabDokumentFeldbefehle")
    Feldtyp.Select sType
    Sleep 3
    if Formatliste.GetSelText <> sFormat then
        warnlog "the format setting should be " & sFormat & " but get " &Formatliste.GetSelText
    end if
    TabDokumentFeldbefehle.Close

    Call hCloseDocument
endcase
