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
'* short description : Test of textframe - 1
'*
'\***********************************************************************

sub w_textframes1

    Call tTextframes_1         'Test Insert Frame with four ways
    Call tTextframes_2         'Test check Autosize
    Call tTextframes_3         'Test Uncheck Autosize
    Call tTextframes_4         'Test Uncheck Keep ratio
    Call tTextframes_5         'Test check Keep ratio
    Call tTextframes_6         'Test Anchor to page - frame doesn't move with text
    Call tTextframes_7         'Test Anchor to paragraph - text entered ahead of the paragraph ,the frame has to move with its paragraph
    Call tTextframes_8         'Test Anchor to Character - the frame follows the vertical position of the character it is anchored to
    Call tTextframes_9         'Test Anchor as Character - frame behaves as a character, moves with each text insertion ahead of it
    Call tTextframes_10        'Test Anchor in gathering pages&columns test.
    Call tTextframes_11        'Test Anchor in gathering paragraph test.
    Call tTextframes_12        'Horizontal should not be available when frame is anchored as Character
    Call tTextframes_13        'Test Horizontal
    Call tTextframes_14        'Test Vertical
    Call tTextframes_15        'Frame name can be actualized in Navigator - solution 1
    Call tTextframes_16        'Frame name can be actualized in Navigator - solution 2

end sub

'---------------------------------------------------

testcase tTextframes_1

    printlog "- Insert / Frame"
    '/// Test Insert Frame
    
    Call hNewDocument
    
    '/// 1. Insert a Frame (with Menu)
    InsertFrame
    Kontext
    try
        Active.Setpage TabType
        Kontext "TabType"
        TabType.Ok
        Wait 500
    catch
        Warnlog "Unable to insert a frame (Dialog is not up)!"
    endcatch
    
    try
        Call fFormatFrame("TabType")
        TabType.Cancel
    catch
        Warnlog "Frame has not been inserted correct with menu!"
    endcatch
    
    Call hCloseDocument
    
    '/// 2. Insert a frame over Toolbox / Insert / Frame
    Call hNewDocument
    
    hSetDocumentContext()
    Call gMouseClick (30, 50)
    
    Call hToolbarSelect("INSERT", true )
    Sleep 1
    Kontext "Insertbar"
    Rahmen.Click
    Sleep 1
    
    Call gMouseDown  ( 30,50 )
    Call gMouseMove  ( 30,50,30,60 )
    Call gMouseUp    ( 30,60 )
    Call gMouseClick ( 50,70 )
    Sleep 1
    
    'set focus to the frame
    Call wTypeKeys ( "<Shift F4>" )
    
    Sleep 2
    
    try
        Call fFormatFrame("TabType")
        TabType.Cancel
    catch
        Warnlog "Frame has not been inserted correct over toolbox !"
    endcatch
    
    Call hCloseDocument
    
    '/// Create Multi-Column frames over toolbox is not possible with testtool
    
    '/// 3. Paste a frame from clipboard
    Call hNewDocument
    
    '/// -- insert a frame first
    Call wInsertFrame
    '/// -- Copy frame to clipboard
    try
        EditCopy
    catch
        Warnlog "Unable to access 'Edit / Copy'. Maybe disabled !"
        Call hCloseDocument
        goto endsub
    endcatch
    
    Call hCloseDocument
    
    Call hNewDocument
    
    '/// -- Paste Frame into document from clipboard
    EditPaste
    Sleep 1
    
    try
        Call fFormatFrame("TabType")
        TabType.Cancel
    catch
        Warnlog "Frame has not been pasted via clipboard !"
    endcatch
    Call hCloseDocument
    
    '/// 4. Insert a frame via 'Insert / File '
    Call hNewDocument
    
    InsertFileWriter
    Kontext "OeffnenDlg"
    Dateiname.Settext Convertpath (gTesttoolpath + "writer\optional\input\textframe\frame.sxw")
    Oeffnen.Click
    Wait 500
    
    'set focus to the frame
    Call wTypeKeys ( "<Shift F4>" )
    Wait 500
    
    try
        Call fFormatFrame("TabType")
        TabType.Cancel
    catch
        Warnlog "Frame has not been inserted via insert/file !"
    endcatch
    
    Call hCloseDocument

endcase

'---------------------------------------------------

testcase tTextframes_2

    Dim sText    as String
    Dim sHeight  as String
    Dim sSpacing as String

    sText    = "ABCDEFG"
    sSpacing = "0"

    printlog "- Test check Autosize"
    '/// Test check Autosize

    Call hNewDocument

    '/// insert/frame / Type , Check Autosize
    InsertFrame
    Kontext
    Active.SetPage TabType
    Kontext "TabType"
    AutoHoehe.Check
    Sleep 1
    TabType.OK

    '/// A return on a selected frame points cursor into frame
    Call wTypeKeys "<Return>"
    Wait 500

    '/// Input text full of frame
    Call wTypeKeys sText
    Wait 500

    Call wTypeKeys "<End>"
    Call wTypeKeys "<Shift Home>"
    Wait 500

    EditCopy
    Wait 500

    ' Choosed Text length because selected text depends on selected font
    if Len(GetClipboardText) = Len(sText) then Warnlog "Don't get expected text ! Autosize doesn't seem to work properly"
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_3

    Dim sText    as String
    Dim sExpect  as String
    Dim sHeight  as String
    Dim sSpacing as String

    sText    = "ABCDEFG"
    sExpect  = "ABCDEFG"
    sSpacing = "0"

    printlog "- Test Uncheck Autosize"
    '/// Test Uncheck Autosize

    Call hNewDocument

    '/// insert/frame / Type , UnCheck Autosize
    InsertFrame
    Kontext
    Active.SetPage TabType
    Kontext "TabType"
    AutoHoehe.UnCheck
    Sleep 1
    TabType.OK

    '/// A return on a selected frame points cursor into frame
    Call wTypeKeys "<Return>"
    Wait 500

    '/// Input text full of frame
    Call wTypeKeys sText
    Wait 500

    Call wTypeKeys "<End>"
    Call wTypeKeys "<Shift Home>"
    Wait 500

    EditCopy
    Wait 500

    if GetClipboardText <> sExpect then QAerrorlog "#i31690# - Don't get expected text ! Hope to get " + sExpect + " but get " + GetClipboardText
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_4

    Dim sWidth      as String
    Dim sHeight     as String
    Dim sWidthNew1  as String
    Dim sHeightNew1 as String
    Dim sWidthNew2  as String
    Dim sHeightNew2 as String
    
    sWidth      = "2"+ gSeperator + "00" + gMeasurementUnit
    sHeight     = "1"+ gSeperator + "00" + gMeasurementUnit
    
    sWidthNew1  = "4"+ gSeperator + "00" + gMeasurementUnit
    sHeightNew1 = "1"+ gSeperator + "00" + gMeasurementUnit
    
    sWidthNew2  = "4"+ gSeperator + "00" + gMeasurementUnit
    sHeightNew2 = "3"+ gSeperator + "00" + gMeasurementUnit
    
    printlog "- Test Uncheck Keep ratio"
    '/// Test Uncheck Keep ratio
    
    Call hNewDocument
    
    '/// insert a frame
    Call wInsertFrame
    
    '/// Format / Frame / Type
    Call fFormatFrame("TabType")
    
    '/// + UnCheck Keep ratio
    KeepRatio.UnCheck
    
    
    '/// + Set width and height
    Width.SetText   sWidth
    Sleep 1
    Height.SetText  sHeight
    Sleep 1
    
    if Width.GetText  <> sWidth  then Warnlog "Keep ration(Uncheck) doesn't work - width!"
    if Height.GetText <> sHeight then Warnlog "Keep ration(Uncheck) doesn't work - heigth!"
    
    '/// Check if height is changed after width is changed.
    Width.SetText   sWidthNew1
    Sleep 1
    
    if Width.GetText  <> sWidthNew1  then Warnlog "Keep ration(Uncheck) doesn't work - widthNew1 !"
    if Height.GetText <> sHeightNew1 then Warnlog "Keep ration(Uncheck) doesn't work - heigthNew1 !"
    
    '/// Check if width is changed after height is changed.
    Height.SetText sHeightNew2
    Sleep 1
    
    if Width.GetText  <> sWidthNew2  then Warnlog "Keep ration(Uncheck) doesn't work - widthNew2 !"
    if Height.GetText <> sHeightNew2 then Warnlog "Keep ration(Uncheck) doesn't work - heigthNew2 !"
    
    TabType.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_5
    
    Dim sWidth      as String
    Dim sHeight     as String
    Dim sWidthNew1  as String
    Dim sHeightNew1 as String
    Dim sWidthNew2  as String
    Dim sHeightNew2 as String
    
    sWidth      = "2"+ gSeperator + "00" + gMeasurementUnit
    sHeight     = "1"+ gSeperator + "00" + gMeasurementUnit
    
    sWidthNew1  = "4"+ gSeperator + "00" + gMeasurementUnit
    sHeightNew1 = "2"+ gSeperator + "00" + gMeasurementUnit
    
    sWidthNew2  = "6"+ gSeperator + "00" + gMeasurementUnit
    sHeightNew2 = "3"+ gSeperator + "00" + gMeasurementUnit
    
    printlog "- Test check Keep ratio"
    '/// Test check Keep ratio
    
    Call hNewDocument
    
    '/// insert a frame
    Call wInsertFrame
    
    '/// Format / Frame / Type
    Call fFormatFrame("TabType")
    
    '/// + UnCheck Keep ratio
    KeepRatio.UnCheck
    
    '/// + Set width and height
    Width.SetText sWidth
    Sleep 1
    Height.SetText sHeight
    Sleep 1
    
    '/// + Check Keep ratio
    KeepRatio.Check
    Sleep 1
    
    '/// Check if height is changed after width is changed.
    Width.SetText sWidthNew1
    Sleep 1
    
    if Width.GetText  <> sWidthNew1  then Warnlog "Keep ration(check) doesn't work - widthNew1 !"
    if Height.GetText <> sHeightNew1 then Warnlog "Keep ration(check) doesn't work - heigthNew1 !"
    
    '/// Check if width is changed after height is changed.
    Height.SetText sHeightNew2
    Sleep 1
    
    if Width.GetText  <> sWidthNew2  then Warnlog "Keep ration(check) doesn't work - widthNew2 !"
    if Height.GetText <> sHeightNew2 then Warnlog "Keep ration(check) doesn't work - heigthNew2 !"
    
    TabType.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_6

    Dim sTestFile as String
    Dim sTestText as String
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\textframe\AnchorToPage.sxw")
    sTestText = "Bottom"
    
    printlog "- Test Anchor to page - frame doesn't move with text"
    '/// Test Anchor to page - frame doesn't move with text
    
    '/// Open a test file
    '/// + This test file is designed for Anchor To Page test.
    '/// + There are 2 pages in the file , and the frame is in
    '/// + the bottom of first page
    hFileOpenLocally( sTestFile )
    
    
    Check1:
    EditSearchAndReplace
    Kontext "FindAndReplace"
    SearchFor.SetText sTestText
    Wait 500
    SearchAll.Click
    Sleep 1
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.Ok
            catch
                Active.No
            endcatch
        end if
    end if
    Kontext "FindAndReplace"
    FindAndReplace.Cancel
    
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then
        fCloseNavigator
        Wait 500
        Call wTypeKeys "<Home>"
        Wait 500
        Call wTypeKeys "<BackSpace>"
        Wait 500
        Goto Check1
    end if
    
    fCloseNavigator
    Wait 500
    
    Call wTypeKeys "<Home>"
    
    '/// Press Return before "bottom" , now the "bottom"
    '/// + should be in the second page
    Call wTypeKeys "<Return>" , 15
    Wait 500
    
    Call wTypeKeys "<Shift End>"
    Wait 500
    EditCopy
    if GetClipboardText <> sTestText then Warnlog "The test text isn't right !"
    
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "2" then Warnlog "The test text should be in second page !"
    fCloseNavigator
    Wait 500
    
    '/// Set focus to the frame  , the frame should be
    '/// + still in the first page
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then Warnlog "The test frame should be still in first page !"
    fCloseNavigator
    Wait 500
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
    
endcase

'-----------------------------------------------------------------

testcase tTextframes_7

    Dim sTestFile as String
    Dim sDummy    as String
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\textframe\AnchorToParagraph.sxw")
    sDummy    = "She heard"
    
    printlog "- Test Anchor to paragraph - text entered ahead of the paragraph ,the frame has to move with its paragraph"
    '/// Test Anchor to paragraph - text entered ahead of the paragraph ,the frame has to move with its paragraph
    
    '/// Open a test file
    '/// + This test file is designed for Anchor To Paragraph test.
    '/// + There are 2 paragraphs in the file , and the head of 2nd
    '/// + paragraph is in the bottom of the first page .
    '/// + The frame is in the first line of 2nd paragraph
    hFileOpenLocally( sTestFile )
    
    
    '/// check if the frame is in the first page
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    Check:
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then
        fCloseNavigator
        Sleep 1
    
        Call wTypeKeys "<ESCAPE>"
        Call wTypeKeys "<MOD1 Home>"
    
        EditSearchAndReplace
        Kontext "FindAndReplace"
        SearchFor.SetText sDummy
        Wait 500
        SearchAll.Click
        Sleep 1
        FindAndReplace.Cancel
    
        Call wTypeKeys "<Home>"
        Call wTypeKeys "<Backspace>"
        Goto Check
    else
        fCloseNavigator
        Wait 500
    end if
    
    Call wTypeKeys "<ESCAPE>"
    Sleep 1
    Call wTypeKeys "<MOD1 Home>"
    
    '/// Press Return before the second paragraph
    Call wTypeKeys "<Return>" , 3
    Wait 500
    
    '/// Set focus to the frame  , the frame should be in the second page
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "2" then Warnlog "The test frame should be in second page !"
    fCloseNavigator
    Wait 500
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
    
endcase

'-----------------------------------------------------------------

testcase tTextframes_8

    Dim sTestFile as String
    Dim sDummy    as String
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\textframe\AnchorToCharacter.sxw")
    sDummy    = "He heard"
    
    printlog "- Test Anchor to Character - the frame follows the vertical position of the character it is anchored to"
    '/// Test Anchor to Character - the frame follows the vertical position of the character it is anchored to
    
    '/// Open a test file
    '/// + This test file is designed for Anchor To Character test.
    '/// + The frame is anchored to the character 't' of That .
    '/// + The frame is in the bottom of the first page
    
    hFileOpenLocally( sTestFile )
    
    
    '/// check if the frame is in the first page
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    Check1:
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then
        fCloseNavigator
        Sleep 1
    
        Call wTypeKeys "<ESCAPE>"
        Call wTypeKeys "<MOD1 Home>"
    
        EditSearchAndReplace
        Kontext "FindAndReplace"
        SearchFor.SetText sDummy
        Wait 500
        SearchAll.Click
        Sleep 1
        FindAndReplace.Cancel
    
        Call wTypeKeys "<Home>"
        Call wTypeKeys "<Backspace>"
        Goto Check1
    else
        fCloseNavigator
        Wait 500
    end if
    
    Call wTypeKeys "<ESCAPE>"
    Sleep 1
    Call wTypeKeys "<End>"
    
    '/// Press Return behind the character which the frame is anchored.
    Call wTypeKeys "<Return>" , 8
    Wait 500
    
    '/// Set focus to the frame  , the frame should be still in the first page
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then Warnlog "The test frame should be in first page !"
    fCloseNavigator
    Wait 500
    
    Call hCloseDocument
    
    hFileOpenLocally( sTestFile )
    
    
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    Check2:
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then
        fCloseNavigator
        Sleep 1
    
        Call wTypeKeys "<ESCAPE>"
        Call wTypeKeys "<MOD1 Home>"
    
        EditSearchAndReplace
        Kontext "FindAndReplace"
        SearchFor.SetText sDummy
        Wait 500
        SearchAll.Click
        Sleep 1
        FindAndReplace.Cancel
    
        Call wTypeKeys "<Home>"
        Call wTypeKeys "<Backspace>"
        Goto Check2
    else
        fCloseNavigator
        Wait 500
    end if
    
    '/// Press Return  in front of the character which the frame is anchored.
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Return>" , 8
    Wait 500
    
    '/// Set focus to the frame  , the frame should be in the second page
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "2" then Warnlog "The test frame should be in second page !"
    fCloseNavigator
    Wait 500
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_9

    Dim sTestFile as String
    Dim sDummy    as String
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\textframe\AnchorAsCharacter.sxw")
    sDummy    = "He heard"
    
    printlog "- Test Anchor as Character - frame behaves as a character, moves with each text insertion ahead of it"
    '/// Test Anchor as Character - frame behaves as a character, moves with each text insertion ahead of it
    
    '/// Open a test file
    '/// + This test file is designed for Anchor as Character test.
    '/// + The frame is anchored as the character.
    '/// + The frame is in the right and bottom of the first page
    hFileOpenLocally( sTestFile )
    
    
    'check if the frame is in the first page
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    Check1:
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then
        fCloseNavigator
        Sleep 1
    
        Call wTypeKeys "<ESCAPE>"
        Call wTypeKeys "<MOD1 Home>"
    
        EditSearchAndReplace
        Kontext "FindAndReplace"
        SearchFor.SetText sDummy
        Wait 500
        SearchAll.Click
        Sleep 1
        FindAndReplace.Cancel
    
        Call wTypeKeys "<Home>"
        Call wTypeKeys "<Backspace>"
        Goto Check1
    else
        fCloseNavigator
        Wait 500
    end if
    
    Call wTypeKeys "<ESCAPE>"
    Sleep 1
    Call wTypeKeys "<End>"
    
    '/// Press Return behind the the frame.
    Call wTypeKeys "<Return>" , 8
    Wait 500
    
    '/// Set focus to the frame  , the frame should be still in the first page
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then Warnlog "The test frame should be in first page !"
    fCloseNavigator
    Wait 500
    
    Call hCloseDocument
    
    hFileOpenLocally( sTestFile )
    
    
    Check2:
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then
        fCloseNavigator
        Sleep 1
    
        Call wTypeKeys "<ESCAPE>"
        Call wTypeKeys "<MOD1 Home>"
    
        EditSearchAndReplace
        Kontext "FindAndReplace"
        SearchFor.SetText sDummy
        Wait 500
        SearchAll.Click
        Sleep 1
        FindAndReplace.Cancel
    
        Call wTypeKeys "<Home>"
        Call wTypeKeys "<Backspace>"
        Goto Check2
    else
        fCloseNavigator
        Wait 500
    end if
    
    '/// Press Return  in front of the frame
    Call wTypeKeys "<ESCAPE>"
    Sleep 1
    Call wTypeKeys "<Home>"
    Call wTypeKeys "<Return>" , 8
    Wait 500
    
    '/// Set focus to the frame  , the frame should be in the second page
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "2" then Warnlog "The test frame should be in second page !"
    fCloseNavigator
    Wait 500
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_10

    Dim sColumn1   as String
    Dim sColumn2   as String
    Dim sTestFile1 as String
    Dim sTestFile2 as String
    
    sColumn1   = "1"
    sColumn2   = "2"
    sTestFile1 = Convertpath (gTesttoolpath + "writer\optional\input\textframe\pageColumn1.sxw")
    sTestFile2 = Convertpath (gTesttoolpath + "writer\optional\input\textframe\pageColumn2.sxw")
    
    printlog "- Test Anchor in gathering pages&columns test."
    '/// Test Anchor in gathering pages&columns test.
    
    '/// Open a test file
    '/// + This test file is designed for Anchor in gathering pages&columns test.
    '/// + There are 2 columns in the page. And the frame is in the second column.
    hFileOpenLocally( sTestFile1,false)
    
    
    'Press return to ensure the frame is in 2nd column
    Call wTypeKeys "<MOD1 Home>"
    Sleep 1
    Call wTypeKeys "<Return>" , 8
    Sleep 1
    
    '/// Set the column to 1
    fFormatPageWriter("TabSpalten")
    Anzahl.SetText sColumn1
    Sleep 1
    TabSpalten.OK
    
    '/// Check if the frame is in the second page
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "2" then Warnlog "The test frame should be in second page -1!"
    fCloseNavigator
    Wait 500
    
    Call hCloseDocument
    
    printlog "- Test Anchor in seperating pages&columns test"
    '/// Test Anchor in seperating pages&columns test
    
    '/// Open a test file2
    '/// + This test file is designed for Anchor in seperating
    '/// + pages&columns test.
    '/// + There are 2 pages. And the frame is in the second page.
    hFileOpenLocally( sTestFile2,false)
    
    
    'check if the frame is in the 2nd page
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "2" then
        Call wTypeKeys "<ESCAPE>"
        Call wTypeKeys "<MOD1 Home>"
        Sleep 1
        Call wTypeKeys "<Return>" , 8
        Sleep 1
    end if
    fCloseNavigator
    Wait 500
    
    Call wTypeKeys "<ESCAPE>"
    Sleep 1
    
    '/// Set the column to 2
    fFormatPageWriter("TabSpalten")
    Anzahl.SetText sColumn2
    Sleep 1
    TabSpalten.OK
    
    '/// Check if the frame is in the first page
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then Warnlog "The test frame should be in first page ! -2"
    fCloseNavigator
    Wait 500
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_11

    Dim sTestFile as String
    Dim sDummy1   as String
    Dim sDummy2   as String
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\textframe\pageParagraph.sxw")
    sDummy1   = "He heard"
    sDummy2   = "after he"
    
    printlog "- Test Anchor in gathering paragraph test"
    '/// Test Anchor in gathering paragraph test
    
    '/// Open a test file
    '/// + This test file is designed for Anchor in gathering paragraph test.
    '/// + There are 2 paragraph in the file .
    '/// + The 1st paragraph is in 1st page and the 2nd paragraph is in 2nd page
    '/// + Two paragraphs, a frame is anchored to the second one. When deleting the paragraph mark, the anchor of the frame has to move the the first paragraph.
    hFileOpenLocally( sTestFile )
    

Check1:
    'First check
    EditSearchAndReplace
    Kontext "FindAndReplace"

    SearchFor.SetText sDummy1
    Wait 500

    SearchAll.Click

    Sleep 1
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.Ok
            catch
                Active.No
            endcatch
        end if
    end if
    Kontext "FindAndReplace"
    FindAndReplace.Cancel

    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then
        fCloseNavigator
        Wait 500
        Call wTypeKeys "<Home>"
        Wait 500
        Call wTypeKeys "<BackSpace>"
        Wait 500
        Goto Check1
    end if

    fCloseNavigator
    Wait 500

Check2:
    'Second check
    EditSearchAndReplace
    Kontext "FindAndReplace"

    SearchFor.SetText sDummy2
    Wait 500

    SearchAll.Click
    Sleep 1
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.Ok
            catch
                Active.No
            endcatch
        end if
    end if
    Kontext "FindAndReplace"
    FindAndReplace.Close

    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "2" then
        fCloseNavigator
        Wait 500
        Call wTypeKeys "<MOD1 Home>"
        Wait 500
        Call wTypeKeys "<Return>"
        Wait 500
        Goto Check2
    end if

    fCloseNavigator
    Wait 500

    '/// Delete the 1st paragraph mark
    Call wTypeKeys "<Home>"
    Wait 500
    Call wTypeKeys "<Backspace>"
    Sleep 1

    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1

    '/// Check if frame is in 1st page
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then
        Warnlog "The test frame should be in the first page! "
    end if
    fCloseNavigator
    Wait 500

    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_12

    printlog "- Horizontal should not be available when frame is anchored as Character"
    '/// Horizontal should not be available when frame is anchored as Character
    
    Call hNewDocument
    
    '/// insert a frame
    Call wInsertFrame
    
    '/// format / frame / type
    Call fFormatFrame("TabType")
    
    '/// + Check Anchor as Character
    AnchorAsCharacter.Check
    wait 500
    
    '/// + Check if Horizontal is available
    try
        Horizontal.Select 1
        Warnlog "Horizontal Should NOT be available when anchor as Character!"
    catch
    endcatch
    
    try
        HorizontalBy.SetText "1"
        Warnlog "HorizontalBy Should NOT be available when anchor as Character!"
    catch
    endcatch
    
    try
        HorizontalTo.Select 1
        Warnlog "HorizontalTo Should NOT be available when anchor as Character!"
    catch
    endcatch
    
    TabType.Cancel
    
    Call hCloseDocument
    
endcase

'-----------------------------------------------------------------

testcase tTextframes_13

    Dim sHorizontalBy       as String
    Dim iHorizontalLeft     as Integer
    Dim iHorizontalRight    as Integer
    Dim iHorizontalCenter   as Integer
    Dim iHorizontalFromLeft as Integer
    
    sHorizontalBy       = "1"+ gSeperator + "00" + gMeasurementUnit
    
    iHorizontalLeft     = 1
    iHorizontalRight    = 2
    iHorizontalCenter   = 3
    iHorizontalFromLeft = 4
    
    printlog "Test Horizontal"
    '/// Test Horizontal
    
    Call hNewDocument
    
    '/// insert a frame
    Call wInsertFrame
    
    '/// format / frame / type
    Call fFormatFrame("TabType")
    '/// + Select Horizontal as Left
    printlog "- Horizontal as Left"
    
    Horizontal.Select iHorizontalLeft
    
    try
        HorizontalBy.SetText   sHorizontalBy
        Warnlog "HorizontalBy Should NOT be available !"
    catch
    endcatch
    
    HorizontalTo.Select 1
    wait 500
    TabType.OK
    
    Call fFormatFrame("TabType")
    if Horizontal.GetSelIndex   <> iHorizontalLeft then Warnlog "Horizontal option is not correct !"
    if HorizontalTo.GetSelIndex <> 1               then Warnlog "HorizontalTo option is not correct !"
    TabType.Cancel
    
    '/// + Select Horizontal as Right
    printlog "- Horizontal as Right"
    
    Call fFormatFrame("TabType")
    Horizontal.Select iHorizontalRight
    
    try
        HorizontalBy.SetText   sHorizontalBy
        Warnlog "HorizontalBy Should NOT be available !"
    catch
    endcatch
    
    HorizontalTo.Select 2
    TabType.OK
    
    Call fFormatFrame("TabType")
    if Horizontal.GetSelIndex   <> iHorizontalRight then Warnlog "Horizontal option is not correct !"
    if HorizontalTo.GetSelIndex <> 2                then Warnlog "HorizontalTo option is not correct !"
    TabType.Cancel
    
    Call fFormatFrame("TabType")
    '/// + Select Horizontal as Center
    printlog "- Horizontal as Center"
    
    Horizontal.Select iHorizontalCenter
    
    try
        HorizontalBy.SetText   sHorizontalBy
        Warnlog "HorizontalBy Should NOT be available !"
    catch
    endcatch
    
    HorizontalTo.Select 3
    TabType.OK
    
    Call fFormatFrame("TabType")
    if Horizontal.GetSelIndex   <> iHorizontalCenter then Warnlog "Horizontal option is not correct !"
    if HorizontalTo.GetSelIndex <> 3                 then Warnlog "HorizontalTo option is not correct !"
    TabType.Cancel
    
    Call fFormatFrame("TabType")
    '/// + Select Horizontal as From left
    printlog "- Horizontal as From left "
    
    Horizontal.Select iHorizontalFromLeft
    
    try
        HorizontalBy.SetText   sHorizontalBy
    catch
        Warnlog "HorizontalBy Should be available !"
    endcatch
    
    HorizontalTo.Select 3
    wait 500
    TabType.OK
    
    Call fFormatFrame("TabType")
    if Horizontal.GetSelIndex   <> iHorizontalFromLeft then Warnlog "Horizontal option is not correct !"
    if HorizontalBy.GetText     <> sHorizontalBy       then Warnlog "HorizontalBy option is not correct !"
    if HorizontalTo.GetSelIndex <> 3                   then Warnlog "HorizontalTo option is not correct !"
    TabType.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_14

    Dim sVerticalBy      as String
    Dim iVerticalTop     as Integer
    Dim iVerticalBottom  as Integer
    Dim iVerticalCenter  as Integer
    Dim iVerticalFromTop as Integer
    
    sVerticalBy      = "1"+ gSeperator + "00" + gMeasurementUnit
    
    iVerticalTop     = 1
    iVerticalBottom  = 2
    iVerticalCenter  = 3
    iVerticalFromTop = 4
    
    printlog "Test Vertical"
    '/// Test Vertical
    
    Call hNewDocument
    
    '/// insert a frame
    Call wInsertFrame
    
    '/// format / frame / type
    Call fFormatFrame("TabType")
    '/// + Select Vertical as Top
    printlog "- Vertical as Top"
    
    Vertical.Select iVerticalTop
    
    try
        VerticalBy.SetText  sVerticalBy
        Warnlog "VerticalBy Should NOT be available !"
    catch
    endcatch
    
    VerticalTo.Select 1
    wait 500
    TabType.OK
    
    Call fFormatFrame("TabType")
    if Vertical.GetSelIndex   <> iVerticalTop then Warnlog "Vertical option is not correct !"
    if VerticalTo.GetSelIndex <> 1            then Warnlog "VerticalTo option is not correct !"
    TabType.Cancel
    
    '/// + Select Vertical as Right
    printlog "- Vertical as Right"
    
    Call fFormatFrame("TabType")
    Vertical.Select iVerticalBottom
    
    try
        VerticalBy.SetText  sVerticalBy
        Warnlog "VerticalBy Should NOT be available !"
    catch
    endcatch
    
    VerticalTo.Select 2
    TabType.OK
    
    Call fFormatFrame("TabType")
    if Vertical.GetSelIndex   <> iVerticalBottom then Warnlog "Vertical option is not correct !"
    if VerticalTo.GetSelIndex <> 2               then Warnlog "VerticalTo option is not correct !"
    TabType.Cancel
    
    Call fFormatFrame("TabType")
    '/// + Select Vertical as Center
    printlog "- Vertical as Center"
    
    Vertical.Select iVerticalCenter
    
    try
        VerticalBy.SetText  sVerticalBy
        Warnlog "VerticalBy Should NOT be available !"
    catch
    endcatch
    
    VerticalTo.Select 1
    TabType.OK
    
    Call fFormatFrame("TabType")
    if Vertical.GetSelIndex   <> iVerticalCenter then Warnlog "Vertical option is not correct !"
    if VerticalTo.GetSelIndex <> 1               then Warnlog "VerticalTo option is not correct !"
    TabType.Cancel
    
    Call fFormatFrame("TabType")
    '/// + Select Vertical as From top
    printlog "- Vertical as From top"
    
    Vertical.Select iVerticalFromTop
    
    try
        VerticalBy.SetText  sVerticalBy
    catch
        Warnlog "VerticalBy Should be available !"
    endcatch
    
    VerticalTo.Select 2
    wait 500
    TabType.OK
    
    Call fFormatFrame("TabType")
    if Vertical.GetSelIndex   <> iVerticalFromTop then Warnlog "Vertical option is not correct !"
    if VerticalBy.GetText     <> sVerticalBy      then Warnlog "VerticalBy option is not correct !"
    if VerticalTo.GetSelIndex <> 2                then Warnlog "VerticalTo option is not correct !"
    TabType.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_15

    Dim sOriginalName as String
    Dim sNewName      as String
    
    sOriginalName = "TEST1"
    sNewName      = "TEST2"
    
    printlog "- Frame name can be actualized in Navigator - solution 1"
    '/// Frame name can be actualized in Navigator - solution 1
    
    Call hNewDocument
    
    '/// insert a frame and name as "TEST1"
    Call wInsertFrame
    
    Call fFormatFrame("TabZusaetze")
    ObjektName.SetText sOriginalName
    TabZusaetze.OK
    
    '/// Open Navigator,
    Call fStartupNavigator("TextFrame" , 1)
    Auswahlliste.TypeKeys "<Down>"
    Auswahlliste.OpenContextMenu
    
    '/// + ContextMenu  TextFrame / Edit,
    Call hMenuSelectNr(4)
    Sleep 2
    Call hMenuSelectNr(1)
    Sleep 1
    Kontext "TabZusaetze"   
    '/// + Change the frame name in the opened dialogue ,
    try
        ObjektName.SetText sNewName
        TabZusaetze.OK
    catch
        Warnlog "format/grame dialogue isn't opened correctly!"
        Call fCloseNavigator
        goto endsub
    endcatch
    
    '/// + close the navigator
    Call fCloseNavigator
    
    Call wTypeKeys ( "<Shift F4>" )
    
    '/// Check if the name is changed correctly
    Call fFormatFrame("TabZusaetze")
    if ObjektName.GetText <> sNewName then Warnlog "frame name isn't renamed correctly!"
    TabZusaetze.Cancel
    
    Call hCloseDocument

endcase

'---------------------------------------------------

testcase tTextframes_16

    Dim sOriginalName as String
    Dim sNewName      as String
    
    sOriginalName = "TEST1"
    sNewName      = "TEST2"
    
    printlog "- Frame name can be actualized in Navigator - solution 2"
    '/// Frame name can be actualized in Navigator - solution 2
    
    Call hNewDocument
    
    '/// insert a frame and name as "TEST1"
    Call wInsertFrame
    
    Call fFormatFrame("TabZusaetze")
    ObjektName.SetText sOriginalName
    TabZusaetze.OK
    
    '/// + Open Navigator ,
    Call fStartupNavigator("TextFrame" , 1)
    
    Auswahlliste.TypeKeys "<Down>"
    Auswahlliste.OpenContextMenu
    
    '/// + ContextMenu  TextFrame / Rename
    Call hMenuSelectNr(4)
    Sleep 2
    Call hMenuSelectNr(3)
    Sleep 1
    
    Kontext "ObjekteUmbenennen"
    
    '/// + Change the frame name in the opened dialogue ,
    try
        ObjektName.SetText sNewName
        ObjekteUmbenennen.OK
    catch
        Warnlog "Frame rename dialogue isn't opened correctly!"
        Call fCloseNavigator
        goto endsub
    endcatch
    
    '/// + close navigaotr
    Call fCloseNavigator
    
    Call wTypeKeys ( "<Shift F4>" )
    
    '/// Check if the name is changed correctly
    Call fFormatFrame("TabZusaetze")
    if ObjektName.GetText <> sNewName then Warnlog "frame name isn't renamed correctly!"
    TabZusaetze.Cancel
    
    Call hCloseDocument

endcase

'---------------------------------------------------
