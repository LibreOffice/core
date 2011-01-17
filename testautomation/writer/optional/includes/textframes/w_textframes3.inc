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
'* short description : Test of textframe - 3
'*
'\***********************************************************************

sub w_textframes3

    Call tTextframes_33         'Test attribute 'AutoSize' in linked frames
    Call tTextframes_34         'Insert text into original frame must flow into target frame, when first one is full
    Call tTextframes_35         'Show link - under Format.Frame.Options (previous link, next link)
    Call tTextframes_36         'Test link frames which are placed on different pages
    Call tTextframes_37         'Test link frames which are placed on header
    Call tTextframes_38         'Test link frames which are placed on footer
    Call tTextframes_39         'Test drag&drop with a linked frame - keyboard
    Call tTextframes_40         'Test Undo on drag&drop of frames (keyboard)
    Call tTextframes_41         'Test Undo on removing the link
    Call tTextframes_42         'Link frame in header with frame in bodytext should be forbidden
    Call tTextframes_43         'Link frame in footer with frame in bodytext should be forbidden
    Call tTextframes_44         'Link frame when target frame is not empty  should be forbidden
    Call tTextframes_45         'Recursed links should be forbidden
    Call tTextframes_46         'Frames which couldn't be linked with the current one is part of another 'chain'
    Call tTextframes_47         'Frames couldn't be linked with the current one is in protected section
    Call tTextframes_48         'Frames could be linked with the current one is set to "editable in protected documents" even it is in protected section

end sub

'---------------------------------------------------

testcase tTextframes_33

    printlog "- Test attribute 'AutoSize' in linked frames"
    '/// Test attribute 'AutoSize' in linked frames
    
    Call hNewDocument
    
    '/// insert a frame
    Call fInsertFrame(30,30 , 50,50)
    
    Call wTypeKeys "<ESCAPE>"
    WaitSlot (5)
    
    '/// insert another frame
    Call fInsertFrame(50,50 , 80,80)
    
    printlog "Move the second frame away from the first one."
    Call wTypeKeys "<RIGHT>",10
    
    '/// Set focus to first frame
    Call wTypeKeys "<TAB>"
    Wait 500
    
    '/// Active frame bar
    Call hToolbarSelect("FRAME", true )
    Wait 500
    
    '/// Click link frame button
    Verketten.Click
    Wait 500
    
    '/// Target the second frame
    Call gMouseClick(60,70)
    Wait 500
    
    '/// Format/Frame / Type ,
    '/// + 'AutoSize' should be turned off in 1st frame (origination)
    Call fFormatFrame("TabType")
    try
        AutoHoehe.Check
        Warnlog "'AutoSize' should be turned off!"
    catch
    endcatch
    TabType.Cancel
    
    '/// Set focus to second frame
    Call wTypeKeys "<TAB>"
    Wait 500
    
    '/// + 'AutoSize' should be actived in 2nd frame (target)
    Call fFormatFrame("TabType")
    try
        AutoHoehe.Check
    catch
        Warnlog "'AutoSize' should be actived!"
    endcatch
    TabType.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_34

    Dim sText         as String
    Dim sOriginalName as String
    Dim sTargetName   as String
    
    sOriginalName = "TEST1"
    sTargetName   = "TEST2"
    
    printlog "- Test attribute 'AutoSize' in linked frames"
    '/// Test attribute 'AutoSize' in linked frames
    
    Call hNewDocument
    
    '/// insert a frame , set frame name
    Call fInsertFrame(30,30 , 50,50)
    
    Call fFormatFrame("TabZusaetze")
    ObjektName.SetText sOriginalName
    TabZusaetze.OK
    
    Call wTypeKeys "<ESCAPE>"
    Wait 500
    
    '/// insert another frame , set frame name
    Call fInsertFrame(50,50 , 80,80)
    
    printlog "Move second frame away from the first frame."
    Call wTypeKeys "<RIGHT>", 12
    
    printlog "Name second object."
    Call fFormatFrame("TabZusaetze")
    ObjektName.SetText sTargetName
    TabZusaetze.OK
    
    '/// Set focus to first frame
    Call wTypeKeys "<TAB>"
    Wait 500
    
    '/// Active frame bar
    Call hToolbarSelect("FRAME", true )
    Wait 500
    
    '/// Click link frame button
    Verketten.Click
    Wait 500
    
    '/// Target the second frame
    Call gMouseClick(60,70)
    Wait 500
    
    Call wTypeKeys "<RETURN>"
    Wait 500
    
    Call wBlindtextEinfuegen
    Wait 500
    
    '/// set focus to the frame which the focus is in
    Call wTypeKeys "<ESCAPE>"
    Sleep 1
    
    '/// Check if the link is showed correctly (Format.Frame.Options)
    Call fFormatFrame("TabZusaetze")
    if ObjektName.GetText <> sTargetName then Warnlog "The focus should be in 2nd frame!"
    TabZusaetze.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_35

    Dim iNoLink as Integer
    Dim iLink   as Integer
    
    iNoLink = 1
    iLink   = 2
    
    printlog "- Show link - under Format.Frame.Options (previous link, next link)"
    '/// Show link - under Format.Frame.Options (previous link, next link)
    
    Call hNewDocument
    
    '/// insert a frame
    Call fInsertFrame(30,30 , 50,50)
    
    Call wTypeKeys "<ESCAPE>"
    Wait 500
    
    '/// insert another frame
    Call fInsertFrame(50,50 , 80,80)
    
    printlog "Move the second frame away from the first one."
    Call wTypeKeys "<RIGHT>",10
    
    '/// Set focus to first frame
    Call wTypeKeys "<TAB>"
    Wait 500
    
    '/// Active frame bar
    Call hToolbarSelect("FRAME", true )
    Wait 500
    
    '/// Click link frame button
    Verketten.Click
    Wait 500
    
    '/// Target the second frame
    Call gMouseClick(60,70)
    Wait 500
    
    '/// Check if the link is showed correctly (Format.Frame.Options)
    Call fFormatFrame("TabZusaetze")
    if PreviousLink.GetSelIndex <> iNoLink then Warnlog "1st frame: privious link should be none"
    if NextLink.GetSelIndex     <> iLink   then Warnlog "1st frame: next link isn't right!"
    TabZusaetze.Cancel
    
    '/// Set focus to second frame
    Call wTypeKeys "<TAB>"
    Wait 500
    
    '/// Check if the link is showed correctly (Format.Frame.Options)
    Call fFormatFrame("TabZusaetze")
    if PreviousLink.GetSelIndex <> iLink   then Warnlog "2nd frame: privious link isn't right!"
    if NextLink.GetSelIndex     <> iNoLink then Warnlog "2nd frame: next link should be none"
    TabZusaetze.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_36

    Dim sTestFile as String
    Dim sTestText as String
    Dim iNoLink   as Integer
    Dim iLink     as Integer
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\textframe\twopages.sxw")
    iNoLink   = 1
    iLink     = 2
    
    printlog "- Test link frames which are placed on different pages"
    '/// Test link frames which are placed on different pages
    
    Call hNewDocument
    
    '/// Open a test file
    '/// + This test file is designed for testing link
    '/// + frames which are placed on different pages
    '/// + There are 2 pages in the file and 1 frame/page
    hFileOpenLocally( sTestFile )
    
    
    'Set focus to first frame
    Call wTypeKeys ( "<Shift F4>" )
    Wait 500
    
    'Check if frame is in 1st page
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then
        Warnlog "The test frame isn't in the first page!"
    end if
    Call fCloseNavigator
    
    'Set focus to second frame
    Call wTypeKeys "<TAB>"
    Wait 500
    
    'Check if frame is in 2nd page
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "2" then
        Warnlog "The test frame isn't in the second page!"
    end if
    Call fCloseNavigator
    
    '/// Set focus to first frame
    Call wTypeKeys "<TAB>"
    Wait 500
    
    '/// Set link in Format/Frame/Options
    Call fFormatFrame("TabZusaetze")
    try
        NextLink.Select iLink
        Sleep 1
    catch
        Warnlog "Can't link 2 frames which are placed on different pages!"
    endcatch
    TabZusaetze.OK
    
    '/// Check if the link is correctly
    Call fFormatFrame("TabZusaetze")
    if PreviousLink.GetSelIndex <> iNoLink then Warnlog "1st frame: privious link should be none"
    if NextLink.GetSelIndex     <> iLink   then Warnlog "1st frame: next link isn't right!"
    TabZusaetze.Cancel
    
    '/// Set focus to second frame
    Call wTypeKeys "<TAB>"
    Wait 500
    
    '/// Check if the link is correctly
    Call fFormatFrame("TabZusaetze")
    if PreviousLink.GetSelIndex <> iLink   then Warnlog "2nd frame: privious link isn't right!"
    if NextLink.GetSelIndex     <> iNoLink then Warnlog "2nd frame: next link should be none"
    TabZusaetze.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_37

    Dim sTestFile as String
    Dim sTestText as String
    Dim iNoLink   as String
    Dim iLink     as  Integer
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\textframe\Header.sxw")
    iNoLink   = 1
    iLink     = 2
    
    printlog "- Test link frames which are placed on header"
    '/// Test link frames which are placed on header
    
    Call hNewDocument
    
    '/// Open a test file
    '/// + This test file is designed for testing link
    '/// + frames which are placed on header
    '/// + There are 2 frames in the header
    
    hFileOpenLocally( sTestFile )
    
    
    '/// Set focus to first frame
    Call wTypeKeys ( "<Shift F4>" )
    
    Wait 500
    
    '/// Set link in Format.Frame.Options
    Call fFormatFrame("TabZusaetze")
    try
        NextLink.Select iLink
        Sleep 1
    catch
        Warnlog "Can't link 2 frames which are in the header"
    endcatch
    TabZusaetze.OK
    
    '/// Check if the link is correctly
    Call fFormatFrame("TabZusaetze")
    if PreviousLink.GetSelIndex <> iNoLink then Warnlog "1st frame: privious link should be none"
    if NextLink.GetSelIndex     <> iLink   then Warnlog "1st frame: next link isn't right!"
    TabZusaetze.Cancel
    
    '/// Set focus to second frame
    Call wTypeKeys "<TAB>"
    Wait 500
    
    '/// Check if the link is correctly
    Call fFormatFrame("TabZusaetze")
    if PreviousLink.GetSelIndex <> iLink   then Warnlog "2nd frame: privious link isn't right!"
    if NextLink.GetSelIndex     <> iNoLink then Warnlog "2nd frame: next link should be none"
    TabZusaetze.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_38

    Dim sTestFile as String
    Dim sTestText as  String
    Dim iNoLink   as String
    Dim iLink     as  Integer
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\textframe\Footer.sxw")
    iNoLink   = 1
    iLink     = 2
    
    printlog "- Test link frames which are placed on footer"
    '/// Test link frames which are placed on footer
    
    Call hNewDocument
    
    '/// Open a test file
    '/// + This test file is designed for testing link
    '/// + frames which are placed on header
    '/// + There are 2 frames in the header
    hFileOpenLocally( sTestFile )
    
    
    '/// Set focus to first frame
    Call wTypeKeys ( "<Shift F4>" )
    Wait 500
    
    '/// Set link in Format.Frame.Options
    Call fFormatFrame("TabZusaetze")
    try
        NextLink.Select iLink
        Sleep 1
    catch
        Warnlog "Can't link 2 frames which are in the header"
    endcatch
    TabZusaetze.OK
    
    '/// Check if the link is correctly
    Call fFormatFrame("TabZusaetze")
    if PreviousLink.GetSelIndex <> iNoLink then Warnlog "1st frame: privious link should be none"
    if NextLink.GetSelIndex     <> iLink   then Warnlog "1st frame: next link isn't right!"
    TabZusaetze.Cancel
    
    '/// Set focus to second frame
    Call wTypeKeys "<TAB>"
    Wait 500
    
    '/// Check if the link is correctly
    Call fFormatFrame("TabZusaetze")
    if PreviousLink.GetSelIndex <> iLink   then Warnlog "2nd frame: privious link isn't right!"
    if NextLink.GetSelIndex     <> iNoLink then Warnlog "2nd frame: next link should be none"
    TabZusaetze.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_39

    Dim sHorizontalBy1    as String
    Dim sHorizontalBy2    as String
    Dim sVerticalBy1      as String
    Dim sVerticalBy2      as String
    Dim sHorizontalByNew1 as String
    Dim sHorizontalByNew2 as String
    Dim sVerticalByNew1   as String
    Dim sVerticalByNew2   as String
    
    sHorizontalBy1    = "2"+ gSeperator + "00" + gMeasurementUnit
    sVerticalBy1      = "1"+ gSeperator + "00" + gMeasurementUnit
    sHorizontalBy2    = "7"+ gSeperator + "00" + gMeasurementUnit
    sVerticalBy2      = "4"+ gSeperator + "00" + gMeasurementUnit
    sHorizontalByNew1 = "3"+ gSeperator + "00" + gMeasurementUnit
    sVerticalByNew1   = "2"+ gSeperator + "00" + gMeasurementUnit
    sHorizontalByNew2 = "7"+ gSeperator + "00" + gMeasurementUnit
    sVerticalByNew2   = "4"+ gSeperator + "00" + gMeasurementUnit
    
    printlog "- Test drag&drop with a linked frame - keyboard"
    '/// Test drag&drop with a linked frame - keyboard
    
    Call hNewDocument
    
    '/// insert a frame and set Horizontal and Vertical
    Call fInsertFrame(30,30 , 49,49)
    
    Call fFormatFrame("TabType")
    HorizontalBy.SetText sHorizontalBy1
    wait 500
    VerticalBy.SetText sVerticalBy1
    wait 500
    TabType.OK
    
    Call wTypeKeys "<ESCAPE>"
    Wait 500
    
    '/// insert another frame and set Horizontal and Vertical
    Call fInsertFrame(51,51 , 80,80)
    
    Call fFormatFrame("TabType")
    HorizontalBy.SetText sHorizontalBy2
    wait 500
    VerticalBy.SetText sVerticalBy2
    wait 500
    TabType.OK
    
    Call wTypeKeys "<Tab>"
    
    '/// link two frames
    Call fFormatFrame("TabZusaetze")
    NextLink.Select 2
    Sleep 1
    TabZusaetze.OK
    
    '/// Move 1st frame with keyboard (right and down)
    Call wTypeKeys "<Right>"
    Sleep 1
    Call wTypeKeys "<Down>"
    Sleep 1
    
    Call fFormatFrame("TabType")
    if HorizontalBy.GetText <> sHorizontalByNew1 then QAErrorLog "#110726#1st frame: Horizontal by isn't right!"
    if VerticalBy.GetText   <> sVerticalByNew1   then QAErrorLog "#110726#1st frame: Vertical by isn't right!"
    TabType.Cancel
    
    Call wTypeKeys "<Tab>"
    
    '/// Move 2nd frame with keyboard (right and down)
    Call wTypeKeys "<Right>"
    Sleep 1
    Call wTypeKeys "<Down>"
    Sleep 1
    
    '/// Check the result
    Call fFormatFrame("TabType")
    if HorizontalBy.GetText <> sHorizontalByNew2 then QAErrorLog "#110726# 2nd frame: Horizontal by isn't right! "
    if VerticalBy.GetText   <> sVerticalByNew2   then QAErrorLog "#110726# 2nd frame: Vertical by isn't right! "
    TabType.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_40

    Dim sHorizontalBy as String
    
    sHorizontalBy = "2"+ gSeperator + "00" + gMeasurementUnit
    
    printlog "- Test Undo on drag&drop of frames (keyboard)"
    '/// Test Undo on drag&drop of frames (keyboard)
    
    Call hNewDocument
    
    '/// insert a frame
    Call fInsertFrame(30,30 , 49,49)
    
    Call fFormatFrame("TabType")
    HorizontalBy.SetText sHorizontalBy
    wait 500
    TabType.OK
    
    Call wTypeKeys "<ESCAPE>"
    Wait 500
    
    '/// insert another frame
    Call fInsertFrame(51,51 , 80,80)
    
    printlog "Move the second frame away from the first one."
    Call wTypeKeys "<RIGHT>",10
'    Call wTypeKeys "<DOWN>",7
    
    Call wTypeKeys "<Tab>"
    
    '/// Active frame bar
    Call hToolbarSelect("FRAME", true )
    Wait 500
    
    '/// Click link frame button
    Verketten.Click
    Wait 500
    
    '/// Target the second frame
    Call gMouseClick(75,30)
    Wait 500
    
    '/// Move 1st frame with keyboard (Left)
    Call wTypeKeys "<Left>"
    Sleep 1
    
    '/// Edit / Undo
    EditUndo
    Wait 500
    
    '/// Check if the frame is moved
    Call fFormatFrame("TabType")
    if HorizontalBy.GetText <> sHorizontalBy then Warnlog "Undo on drag&drop of frames doesn't work!"
    TabType.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_41

    Dim iLink as String
    Dim iNolink as Integer
    
    iLink   =  2
    iNolink =  1
    
    printlog "- Test Undo on removing the link"
    '/// Test Undo on removing the link
    
    Call hNewDocument
    
    '/// insert a frame
    Call fInsertFrame(30,30 , 49,49)
    
    Call wTypeKeys "<ESCAPE>"
    WaitSlot (5)
    
    '/// insert another frame
    Call fInsertFrame(51,51 , 80,80)
    
    printlog "Move the second frame away from the first one."
    Call wTypeKeys "<RIGHT>",10
    
    '/// Set focus to 1st frame
    Call wTypeKeys "<Tab>"
    
    '/// Active frame bar
    Call hToolbarSelect("FRAME", true )
    Wait 500
    
    '/// Click link frame button
    Verketten.Click
    Wait 500
    
    '/// Target the second frame
    Call gMouseClick(60,70)
    Wait 500
    
    '/// Remove link
    Kontext "FrameObjectbar"
    VerkettungLoesen.Click
    Wait 500
    
    Call fFormatFrame("TabZusaetze")
    if NextLink.GetSelIndex  <> iNoLink then Warnlog "Remove link doesn't work!"
    TabZusaetze.Cancel
    
    '/// Edit / Undo
    EditUndo
    Wait 500
    
    '/// check if the frame is moved
    Call fFormatFrame("TabZusaetze")
    if NextLink.GetSelIndex  <> iLink then Warnlog "Undo on removing link doesn't work!"
    TabZusaetze.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_42

    Dim iNoLink as Integer
    
    iNoLink  =  1
    
    printlog "- Link frame in header with frame in bodytext should be forbidden"
    '/// Link frame in header with frame in bodytext should be forbidden
    
    Call hNewDocument
    
    '/// insert a frame in header
    DocumentWriter.UseMenu
    MenuSelect(Menugetitemid(4))
    Sleep 2
    MenuSelect(Menugetitemid(9))
    Sleep 2
    MenuSelect(Menugetitemid(1))
    Sleep 2
    
    Call wInsertFrame
    
    Call wTypeKeys "<ESCAPE>"
    Wait 500
    
    '/// insert another frame in bodytext
    Call fInsertFrame(50,50 , 70,65)
    
    Call wTypeKeys "<Tab>"
    
    '/// Try to link two frames
    Call hToolbarSelect("FRAME", true )
    Wait 500
    Verketten.Click
    Wait 500
    Call gMouseClick(60,60)
    Wait 500
    
    Call fFormatFrame("TabZusaetze")
    if NextLink.GetSelIndex  <> iNoLink then Warnlog "There should be no link between 2 frames!"
    TabZusaetze.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_43

    Dim iLink as Integer
    
    iLink = 2
    
    printlog "- Link frame in footer with frame in bodytext should be forbidden"
    '/// Link frame in footer with frame in bodytext should be forbidden
    
    Call hNewDocument
    
    '/// insert a frame in footer
    DocumentWriter.UseMenu
    MenuSelect(Menugetitemid(4))
    Sleep 2
    MenuSelect(Menugetitemid(10))
    Sleep 2
    MenuSelect(Menugetitemid(1))
    Sleep 2
    
    Call wInsertFrame
    
    Call wTypeKeys "<ESCAPE>"
    Wait 500
    
    '/// insert another frame in bodytext
    Call fInsertFrame(50,50 , 70,70)
    
    Call wTypeKeys "<Tab>"
    
    '/// Try to link two frames
    Call fFormatFrame("TabZusaetze")
    Sleep 1
    try
        NextLink.Select iLink
        Sleep 1
        Warnlog "There should be no link between 2 frames!"
    catch
    endcatch
    TabZusaetze.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_44

    Dim sText   as String
    Dim iNoLink as Integer
    
    sText    =  "Test"
    iNoLink  =  1
    
    printlog "- Link frame in footer with frame in bodytext should be forbidden"
    '/// Link frame in footer with frame in bodytext should be forbidden
    
    Call hNewDocument
    
    '/// insert a frame
    Call fInsertFrame(50,30 , 50,40)
    
    Call wTypeKeys "<ESCAPE>"
    Wait 500
    
    '/// insert another frame
    Call fInsertFrame(60,50 , 70,65)
    Call wTypeKeys "<ESCAPE>"
    
    '/// input some text in the target frame(2nd frame)
    Call gMouseClick(65,60)
    Call wTypeKeys sText
    
    Call wTypeKeys ( "<Shift F4>" )
    
    '/// link two frames
    Call hToolbarSelect("FRAME", true )
    Wait 500
    Verketten.Click
    Wait 500
    Call gMouseClick(50,60)
    Wait 500
    
    Call fFormatFrame("TabZusaetze")
    if NextLink.GetSelIndex  <> iNoLink then Warnlog "There should be no link between 2 frames!"
    TabZusaetze.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_45

    Dim iNoLink as Integer
    
    iNoLink  =  1
    
    printlog "- Recursed links should be forbidden"
    '/// Recursed links should be forbidden
    
    Call hNewDocument
    
    '/// insert a frame
    Call fInsertFrame(30,30 , 45,45)
    
    Call wTypeKeys "<ESCAPE>"
    Wait 500
    
    '/// insert another frame
    Call fInsertFrame(50,50 , 70,70)
    
    printlog "Move the second frame away from the first one."
    Call wTypeKeys "<RIGHT>",1
    
    '/// set focus to 1st frame
    Call wTypeKeys "<TAB>"
    Sleep 1
    
    '/// link two frames (2nd frame is target frame)
    Kontext "FrameObjectbar"
    Sleep 2
    Verketten.Click
    Wait 500
    Call gMouseClick(60,60)
    Wait 500
    
    '/// link two frames (1st frame is target frame)
    Call wTypeKeys "<TAB>"
    Call hToolbarSelect("FRAME", true )
    Wait 500
    Verketten.Click
    Wait 500
    Call gMouseClick(35,35)
    Wait 500
    
    Call fFormatFrame("TabZusaetze")
    if NextLink.GetItemCount <> iNoLink then Warnlog "There should be no link between 2 frames!" & NextLink.GetItemCount 
    TabZusaetze.Cancel
    Wait 500
    
    Call hCloseDocument
endcase

'-----------------------------------------------------------------

testcase tTextframes_46

    Dim iNoLink as Integer
    
    iNoLink = 1
    
    printlog "- Frames couldn't be linked with the current one is part of another 'chain'"
    '/// Frames couldn't be linked with the current one is part of another 'chain'
    
    Call hNewDocument
    
    '/// insert a frame
    Call fInsertFrame(30,30 , 49,49)
    
    Call wTypeKeys "<ESCAPE>"
    Wait 500
    
    '/// insert another frame
    Call fInsertFrame(51,51 , 65,65)
    
    printlog "Move the frame away from the position of the first one."
    Call wTypeKeys "<RIGHT>",4
    Call wTypeKeys "<DOWN>",4
    
    Call wTypeKeys "<ESCAPE>"
    Wait 500
    
    '/// insert 3rd frame
    Call fInsertFrame(70,70 , 80,80)
    Wait 500
    
    printlog "Move the frame away from the position of the first one."
    Call wTypeKeys "<LEFT>",8
    Call wTypeKeys "<UP>",8
    
    '/// set focus to 1st frame
    Call wTypeKeys "<SHIFT F4>" 'make sure object is selected.
    Call wTypeKeys "<TAB>"
    Sleep 1
    
    '/// link two frames 1 & 2 (2nd frame is target frame)
    '/// Active frame bar
    Call hToolbarSelect("FRAME", true )
    Sleep 1
    Verketten.Click
    Sleep 1
    Call gMouseClick(70,70)
    Wait 500
    
    '/// set focus to 3rd frame
    Call wTypeKeys "<TAB>" , 2
    
    '/// link two frames 3 & 2 (2nd frame is target frame)
    Kontext "FrameObjectbar"
    Sleep 2
    Verketten.Click
    Sleep 1
    Call gMouseClick(70,70)
    Wait 500
    
    Call fFormatFrame("TabZusaetze")
    if NextLink.GetSelIndex  <> iNoLink then Warnlog "Shouldn't link to 2nd frame ! "
    TabZusaetze.Cancel
    Wait 500

    Call hCloseDocument
endcase

'-----------------------------------------------------------------

testcase tTextframes_47

    Dim iLink as Integer
    
    iLink = 2
    
    printlog "- Frames couldn't be linked with the current one is in protected section"
    '/// Frames couldn't be linked with the current one is in protected section
    
    Call hNewDocument
    
    '/// Insert a section
    InsertSection
    wait 500
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    wait 500
    TabBereiche.OK
    wait 500
    
    Call wTypeKeys "<UP>" , 1
    Call wTypeKeys "<RETURN>", 6
    
    '/// Insert a frame (A) in the section
    Call wInsertFrame
    Call wTypeKeys "<UP>" , 2
    Call wTypeKeys "<ESCAPE>"
    
    '/// Set the section to be protected
    FormatSections
    Kontext "BereicheBearbeiten"
    Geschuetzt.Check
    sleep 1
    BereicheBearbeiten.OK
    
    Call wTypeKeys "<DOWN>" , 10
        
    '/// Insert another frame (B) out of the section
    Call fInsertFrame(30,70 , 60,90)
    
    Call wTypeKeys "<RIGHT>" , 8
    Call wTypeKeys "<DOWN>" , 8
    
    '/// Link B->A (A is target)
    Call fFormatFrame("TabZusaetze")
    try
        NextLink.Select iLink
        Sleep 1
        QAErrorLog "#110862# Should NOT link 2 frames when target frame is in the protected section!"
    catch
    endcatch
    TabZusaetze.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_48

    Dim iLink as Integer
    
    iLink = 2
    
    printlog "- Frames could be linked with the current one is set to 'editable in protected documents' even it is in protected section "
    
    Call hNewDocument
    
    printlog "Insert a section"
    InsertSection
    wait 500
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    wait 500
    TabBereiche.OK
    wait 500
    
    Call wTypeKeys "<UP>" , 1
    Call wTypeKeys "<RETURN>", 6
    
    printlog "Insert a frame (A) in the section , and check 'editable in protected documents'"
    Call wInsertFrame
    Call wTypeKeys "<UP>" , 2          
       
    Call fFormatFrame("TabZusaetze")
    Editierbar.Check
    wait 500
    TabZusaetze.OK
    
    Call wTypeKeys "<ESCAPE>"
    
    printlog "Set the section to be protected"
    FormatSections
    Kontext "BereicheBearbeiten"
    Geschuetzt.Check
    sleep 1
    BereicheBearbeiten.OK
    
    Call wTypeKeys "<DOWN>" , 10
    printlog "nsert another frame (B) out of the section"
    Call fInsertFrame(50,50 , 60,60)
    
    printlog "Link B->A (A is target)"
    Call fFormatFrame("TabZusaetze")
    try
        NextLink.Select iLink
        Sleep 1
    catch
        Warnlog "link 2 frames should work!"
    endcatch
    TabZusaetze.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------
