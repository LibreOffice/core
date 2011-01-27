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
'* short description : Test of textframe - 2
'*
'\***********************************************************************

sub w_textframes2

    Call tTextframes_17         'Test protect contents - text insertion in frame must be suppressed
    Call tTextframes_18         'Test protect position - moving frame with mouse/keyboard suppressed
    Call tTextframes_19         'Test if frame can move when protect content is checked
    Call tTextframes_20         'Test frame without wrap
    Call tTextframes_21         'Test frame with wrap ( through)
    Call tTextframes_22         'Test wrap - before/after - 1
    Call tTextframes_23         'Test wrap - before/after - 2
    Call tTextframes_24         'Set more than 1 columns for the frame
    Call tTextframes_25         'Test columns' width
    Call tTextframes_26         'Test columns' spacing
    Call tTextframes_27         'Test columns' seperator line
    Call tTextframes_28         'Test overlap with different wrapping types
    Call tTextframes_29         'Test 'synchronize' distances for border (Check Synchronize)
    Call tTextframes_30         'Test 'synchronize' distances for border (UnCheck Synchronize)
    Call tTextframes_31         'Frame should be non-editable in read-only documents
    Call tTextframes_32         'Test input text into frame in read-only document when checking "Editable in read-only document"

end sub

'---------------------------------------------------

testcase tTextframes_17

    Dim sTestText
    
    sTestText = "T"
    
    printlog "- Test protect contents - text insertion in frame must be suppressed"
    
    Call hNewDocument
    
    '/// insert a frame
    Call fInsertFrame(30,30 , 60,60)
    
    '/// Format / Frame / Options , check contents
    Call fFormatFrame("TabZusaetze")
    InhaltSchuetzen.Check
    TabZusaetze.OK
    
    Call wTypeKeys "<ESCAPE>"
    Wait 500
    
    Call gMouseClick(40,40)
    Wait 500
    
    '/// check if contents can be inserted into frame
    Call wTypeKeys sTestText
    
    Kontext "Active"
    if Active.Exists then
        Active.Ok
    else
        Warnlog "The contents should not be inserted into frame!"
    end if
    
    Call hCloseDocument

endcase

'---------------------------------------------------

testcase tTextframes_18

    Dim sTestText as String
    
    sTestText = "T"
    
    printlog "- Test protect position - moving frame with mouse/keyboard suppressed"
    
    Call hNewDocument
    
    '/// insert a frame
    Call fInsertFrame(30,30 , 70,70)
    
    '/// Format / Frame / Options , check contents
    Call fFormatFrame("TabZusaetze")
    InhaltSchuetzen.Check
    PositionSchuetzen.Check
    TabZusaetze.OK
    
    '/// Move the frame to right (Right key 4 times)
    Call wTypeKeys "<Right>" , 4
    Wait 500
    Call wTypeKeys "<ESCAPE>"
    Wait 500
    
    Call gMouseClick(40,40)
    Sleep 2
    
    '/// check if the frame is moved
    Call wTypeKeys sTestText
    
    Kontext "Active"
    if Active.Exists then
        Active.Ok
    else
        Warnlog "The contents should NOT be inserted into the frame!"
    end if
    
    Call wTypeKeys "<Shift Left>"
    try
        EditCopy
        Wait 500
        if GetClipboardText = sTestText then Warnlog "Something wrong in the test !"
    catch
    endcatch
    
    Call hCloseDocument
    
endcase

'---------------------------------------------------

testcase tTextframes_19

    Dim sTestText
    
    sTestText = "T"
    
    printlog "- Test if frame can move when protect content is checked"
    
    Call hNewDocument
    
    '/// insert a frame
    Call fInsertFrame(30,30 , 70,70)
    
    '/// Format / Frame / Options , check contents
    Call fFormatFrame("TabZusaetze")
    InhaltSchuetzen.Check
    TabZusaetze.OK
    
    '/// Move the frame to right (Right key 4 times)
    Call wTypeKeys "<Right>" , 4
    Wait 500
    Call wTypeKeys "<ESCAPE>"
    Wait 500
    
    Call gMouseClick(15,15)
    Wait 500
    
    '/// check if the frame is moved
    Call wTypeKeys sTestText
    
    Kontext "Active"
    if Active.Exists then
        Warnlog "#i40348# The frame should move by keyboard!"
        Active.Ok
    end if
    
    Call wTypeKeys "<Shift Left>"
    try
        EditCopy
        Wait 500
        if GetClipboardText <> sTestText then Warnlog "Something wrong in the test . Clipboard:" +GetClipboardText
    catch
        Warnlog "#i40348#"
    endcatch
    
    Call hCloseDocument

endcase

'---------------------------------------------------

testcase tTextframes_20

    Dim sTestFile as String
    Dim sTestText as String
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\textframe\wrap.sxw")
    sTestText = "Test"
    
    printlog "- Test frame without wrap"
    
    '/// Open a test file
    '/// + This test file is designed for without wrap test
    '/// + There are 1 page in the file.
    '/// + The frame is ocupied in the whole 1st page and wraped as Through
    '/// + Some text are behind the frame
    hFileOpenLocally( sTestFile )
    
    
    '/// Set focus to frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format / Frame / Wrap , check None in setting area
    Call fFormatFrame("TabUmlauf")
    Kein.Check
    wait 500
    TabUmlauf.OK
    
    '/// Check if frame is in 1st page
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then
        Warnlog "The test frame isn't in the first page!"
    end if
    Call fCloseNavigator
    
    '/// Check if test text are in 2nd page
    Call wTypeKeys "<ESCAPE>"
    Call wTypeKeys "<Home><Shift End>"
    
    try
        EditCopy
        Wait 500
        if GetClipboardText <> sTestText then Warnlog "Test text isn't right , GetClipboardText is " +GetClipboardText
    catch
        Warnlog "EditCopy can't be executed !"
    endcatch
    
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "2" then
        Warnlog "The test text isn't in the second page!"
    end if
    
    Call fCloseNavigator
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
    
endcase

'-----------------------------------------------------------------

testcase tTextframes_21

    Dim sTestFile as String
    Dim sTestText as String
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\textframe\through.sxw")
    sTestText = "Test"
    
    printlog "- Test frame with wrap "
    
    '/// Open a test file
    '/// + This test file is designed for with through wrap test
    '/// + There are 2 page in the file.
    '/// + The frame is ocupied in the whole 1st page and wraped as None
    '/// + Some text are in the 2nd page
    
    hFileOpenLocally( sTestFile )
    
    
    '/// Set focus to frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format / Frame / Wrap , check through in setting area
    Call fFormatFrame("TabUmlauf")
    Durchlauf.Check
    wait 500
    TabUmlauf.OK
    
    '/// Check if frame are in 1st page
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then
        Warnlog "The test frame isn't in the first page!"
    end if
    Call fCloseNavigator
    
    '/// Check if test text are in 1st page
    Call wTypeKeys "<ESCAPE>"
    Call wTypeKeys "<Home><Shift End>"
    
    try
        EditCopy
        Wait 500
        if GetClipboardText <> sTestText then Warnlog "Test text isn't right , GetClipboardText is " +GetClipboardText
    catch
        Warnlog "EditCopy can't be executed !"
    endcatch
    
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then
        Warnlog "The test text isn't in the first page!"
    end if
    
    Call fCloseNavigator
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_22

    Dim sTestFile as String
    Dim sTestText as String
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\textframe\before.sxw")
    sTestText = "Test"
    
    printlog "- Test wrap - before/after - 1"
    
    '/// Open a test file
    '/// + This test file is designed for with before/after wrap test
    '/// + There are 2 page in the file.
    '/// + The frame is ocupied in the whole 1st page and wraped as None
    '/// + there are some space in the left of the frame.
    '/// + Some text are in the 2nd page
    
    hFileOpenLocally( sTestFile )
    
    
    printlog "- Test wrap - before"
    
    '/// Set focus to frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format / Frame / Wrap , check before in setting area
    Call fFormatFrame("TabUmlauf")
    Links.Check
    wait 500
    TabUmlauf.OK
    
    '/// Check if frame are in 1st page
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then
        Warnlog "The test frame isn't in the first page!"
    end if
    Call fCloseNavigator
    
    '/// Check if test text are in 1st page
    Call wTypeKeys "<ESCAPE>"
    Sleep 1
    Call wTypeKeys "<Home><Shift End>"
    
    try
        EditCopy
        Wait 500
        if GetClipboardText <> sTestText then Warnlog "Test text isn't right , GetClipboardText is " +GetClipboardText
    catch
        Warnlog "EditCopy can't be executed !"
    endcatch
    
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then
        Warnlog "The test text isn't in the first page!"
    end if
    
    Call fCloseNavigator
    
    printlog "- Test wrap - after"
    
    'Set focus to frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format / Frame / Wrap , check after in setting area
    Call fFormatFrame("TabUmlauf")
    Rechts.Check
    wait 500
    TabUmlauf.OK
    
    '/// Check if frame are in 1st page
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then
        Warnlog "The test frame isn't in the first page!"
    end if
    Call fCloseNavigator
    
    '/// Check if test text are in 2nd page
    Call wTypeKeys "<ESCAPE>"
    Sleep 1
    Call wTypeKeys "<Home><Shift End>"
    
    try
        EditCopy
        Wait 500
        if GetClipboardText <> sTestText then Warnlog "Test text isn't right , GetClipboardText is " +GetClipboardText
    catch
        Warnlog "EditCopy can't be executed !"
    endcatch
    
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "2" then
        Warnlog "The test text isn't in the first page!"
    end if
    
    Call fCloseNavigator
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_23

    Dim sTestFile as String
    Dim sTestText as String
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\textframe\after.sxw")
    sTestText = "Test"
    
    printlog "- Test wrap - before/after - 2"
    
    '/// Open a test file
    '/// + This test file is designed for with before/after wrap test
    '/// + There are 2 page in the file.
    '/// + The frame is ocupied in the whole 1st page and wraped as None
    '/// + there are some space in the right of the frame.
    '/// + Some text are in the 2nd page
    
    hFileOpenLocally( sTestFile )
    
    
    printlog "- Test wrap - before"
    
    '/// Set focus to frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format / Frame / Wrap , check before in setting area
    Call fFormatFrame("TabUmlauf")
    Links.Check
    wait 500
    TabUmlauf.OK
    
    '/// Check if frame are in 1st page
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then
        Warnlog "The test frame isn't in the first page!"
    end if
    Call fCloseNavigator
    
    '/// Check if test text are in 1st page
    Call wTypeKeys "<ESCAPE>"
    Sleep 1
    Call wTypeKeys "<Home><Shift End>"
    
    try
        EditCopy
        Wait 500
        if GetClipboardText <> sTestText then Warnlog "Test text isn't right , GetClipboardText is " +GetClipboardText
    catch
        Warnlog "EditCopy can't be executed !"
    endcatch
    
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "2" then
        Warnlog "The test text isn't in the first page!"
    end if
    
    Call fCloseNavigator
    
    printlog "- Test wrap - after"
    
    '/// Set focus to frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format / Frame / Wrap , check after in setting area
    Call fFormatFrame("TabUmlauf")
    Rechts.Check
    wait 500
    TabUmlauf.OK
    
    '/// Check if frame are in 1st page
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then
        Warnlog "The test frame isn't in the first page!"
    end if
    Call fCloseNavigator
    
    '/// Check if test text are in 2nd page
    Call wTypeKeys "<ESCAPE>"
    Sleep 1
    Call wTypeKeys "<Home><Shift End>"
    
    try
        EditCopy
        Wait 500
        if GetClipboardText <> sTestText then Warnlog "Test text isn't right , GetClipboardText is " +GetClipboardText
    catch
        Warnlog "EditCopy can't be executed !"
    endcatch
    
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then
        Warnlog "The test text isn't in the first page!"
    end if
    
    Call fCloseNavigator
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_24

    Dim sColumn as String
    
    sColumn = "3"
    
    printlog "- Set more than 1 columns for the frame"
    
    Call hNewDocument
    
    '/// Insert a frame
    Call wInsertFrame
    
    '/// Format / Frame / columns , set columns to 3 , close the dialogue
    Call fFormatFrame("TabSpalten")
    Anzahl.SetText sColumn
    wait 500
    TabSpalten.OK
    
    '/// Check if the column is set correctly
    Call fFormatFrame("TabSpalten")
    if Anzahl.GetText <> sColumn then Warnlog "the column isn't set correctly ! The column you get is:Anzahl.GetText"
    TabSpalten.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_25

    Dim sWidth            as String
    Dim sSpacingToContent as String
    Dim sColumn           as String
    Dim sWidthColumn1     as String
    Dim sWidthColumn2     as String
    Dim sWidthColumn3     as String
    
    sWidth            = "6"+ gSeperator + "00" + gMeasurementUnit
    sSpacingToContent = "0"+ gSeperator + "00"
    
    sColumn = "3"
    
    sWidthColumn1 = "1"+ gSeperator + "00" + gMeasurementUnit
    sWidthColumn2 = "2"+ gSeperator + "00" + gMeasurementUnit
    sWidthColumn3 = "3"+ gSeperator + "00" + gMeasurementUnit
    
    printlog "- Test columns' width"
    
    Call hNewDocument
    
    '/// Insert a frame
    Call wInsertFrame
    
    'Set frame's width
    Call fFormatFrame("TabType")
    Width.SetText sWidth
    wait 500
    TabType.OK
    
    'Set frame's Spacing to Content
    Call fFormatFrame("TabUmrandung")
    Synchronisieren.Check
    wait 500
    TabUmrandung.TypeKeys "<SHIFT TAB>"
    Links.SetText sSpacingToContent
    Rechts.SetText sSpacingToContent
    Oben.SetText sSpacingToContent
    Unten.SetText sSpacingToContent
    wait 500
    TabUmrandung.TypeKeys "<SHIFT TAB>"
    TabUmrandung.OK
    
    '/// Format / Frame / columns , set columns to 3
    Call fFormatFrame("TabSpalten")
    Anzahl.SetText sColumn
    wait 500
    TabSpalten.OK
    
    '/// + Set 3 columns to different width
    Call fFormatFrame("TabSpalten")
    AutomBreite.Uncheck
    Sleep 1
    TabSpalten.TypeKeys "<SHIFT TAB>"
    Spaltenbreite1.SetText sWidthColumn1
    wait 500
    TabSpalten.TypeKeys "<SHIFT TAB>"
    Spaltenbreite2.SetText sWidthColumn2
    wait 500
    TabSpalten.TypeKeys "<SHIFT TAB>"
    Spaltenbreite3.SetText sWidthColumn3
    wait 500
    TabSpalten.TypeKeys "<SHIFT TAB>"
    TabSpalten.OK
    
    '/// Check if the columns' width is set correctly
    Call fFormatFrame("TabSpalten")
    if Spaltenbreite1.GetText <> sWidthColumn1 then Warnlog "the column1's width isn't set correctly !"
    if Spaltenbreite2.GetText <> sWidthColumn2 then Warnlog "the column2's width isn't set correctly !"
    if Spaltenbreite3.GetText <> sWidthColumn3 then Warnlog "the column3's width isn't set correctly !"
    TabSpalten.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_26
    
    Dim sWidth            as String
    Dim sSpacingToContent as String
    Dim sColumn           as String
    Dim sWidthColumn1     as String
    Dim sWidthColumn2     as String
    Dim sWidthColumn3     as String
    Dim sSpacing1         as String
    Dim sSpacing2         as String
    Dim sNewWidthColumn1  as String
    Dim sNewWidthColumn2  as String
    Dim sNewWidthColumn3  as String
    Dim sNewWidthColumn4  as String
    
    sWidth            = "6"+ gSeperator + "00" + gMeasurementUnit
    sSpacingToContent = "0"+ gSeperator + "00"
    
    sColumn = "3"
    
    sWidthColumn1     = "1"+ gSeperator + "00" + gMeasurementUnit
    sWidthColumn2     = "2"+ gSeperator + "00" + gMeasurementUnit
    sWidthColumn3     = "3"+ gSeperator + "00" + gMeasurementUnit
    
    sSpacing1         = "0"+ gSeperator + "10" + gMeasurementUnit
    sSpacing2         = "0"+ gSeperator + "20" + gMeasurementUnit
    
    sNewWidthColumn1  = "1"+ gSeperator + "00" + gMeasurementUnit
    sNewWidthColumn2  = "1"+ gSeperator + "90" + gMeasurementUnit
    sNewWidthColumn3  = "2"+ gSeperator + "80" + gMeasurementUnit
    sNewWidthColumn4  = "2"+ gSeperator + "81" + gMeasurementUnit
    
    printlog "- Test columns' spacing"
    
    Call hNewDocument
    
    '/// Insert a frame
    Call wInsertFrame
    
    'Set frame's width
    Call fFormatFrame("TabType")
    Width.SetText sWidth
    wait 500
    TabType.TypeKeys "<SHIFT TAB>"
    TabType.OK
    
    'Set frame's Spacing to Content
    Call fFormatFrame("TabUmrandung")
    Synchronisieren.Check
    wait 500
    Links.SetText sSpacingToContent
    Rechts.SetText sSpacingToContent
    Oben.SetText sSpacingToContent
    Unten.SetText sSpacingToContent
    wait 500
    TabUmrandung.TypeKeys "<SHIFT TAB>"
    TabUmrandung.OK
    
    '/// Format / Frame / columns , set columns to 3
    Call fFormatFrame("TabSpalten")
    Anzahl.SetText sColumn
    wait 500
    TabSpalten.OK
    
    '/// + Set 3 columns to different width
    Call fFormatFrame("TabSpalten")
    AutomBreite.Uncheck
    Sleep 1
    TabSpalten.TypeKeys "<SHIFT TAB>"
    Spaltenbreite1.SetText sWidthColumn1
    wait 500
    TabSpalten.TypeKeys "<SHIFT TAB>"
    Spaltenbreite2.SetText sWidthColumn2
    wait 500
    TabSpalten.TypeKeys "<SHIFT TAB>"
    Spaltenbreite3.SetText sWidthColumn3
    wait 500
    TabSpalten.TypeKeys "<SHIFT TAB>"
    TabSpalten.OK
    
    '/// + Set Spacing1 and Spacing2
    Call fFormatFrame("TabSpalten")
    Abstand12.SetText sSpacing1
    Sleep 2
    Abstand23.SetText sSpacing2
    Sleep 3
    Art.Select 1
    Sleep 1
    TabSpalten.OK
    Sleep 2
    
    '/// Check if the columns' width is set correctly
    Call fFormatFrame("TabSpalten")
    if Spaltenbreite1.GetText <> sNewWidthColumn1 then Warnlog "the column1's width isn't set correctly !"
    if Spaltenbreite2.GetText <> sNewWidthColumn2 then Warnlog "the column2's width isn't set correctly !"
    if (Spaltenbreite3.GetText <> sNewWidthColumn3) then
        if (Spaltenbreite3.GetText <> sNewWidthColumn4) then
            Warnlog "the column3's width isn't set correctly ! (not" & Spaltenbreite3.GetText & " but " & sNewWidthColumn3 & ")"
        end if
    end if
    if Abstand12.GetText <> sSpacing1 then Warnlog "the 1st spacing isn't set correctly !"
    if Abstand23.GetText <> sSpacing2 then Warnlog "the 2nd spacing isn't set correctly !"
    TabSpalten.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_27
    
    Dim iLine     as Integer
    Dim iPosition as Integer
    Dim sHeight   as String
    
    iLine     = 3
    iPosition = 2
    sHeight   = "50%"
    
    printlog "- Test columns' seperator line"
    
    Call hNewDocument
    
    '/// Insert a frame
    Call wInsertFrame
    
    '/// Format / Frame / columns , set columns to 4
    '/// + default : column is 1 , Line and Height and Position isn't enabled
    Call fFormatFrame("TabSpalten")
    if Anzahl.GetText <> "1" then  Warnlog "the default column number is 1 , but get " +Anzahl.GetText
    
    try
        Art.Select      iLine
        Warnlog "Line : should be inactived !"
        Hoehe.SetText   sHeight
        Warnlog "Height : should be inactived !"
        Position.Select iPosition
        Warnlog "Position : should be inactived !"
    catch
    endcatch
    
    Anzahl.SetText "2"
    Wait 500
    TabSpalten.OK
    
    '/// Check if the columns' width is set correctly
    Call fFormatFrame("TabSpalten")
    Art.Select      iLine
    Sleep 2
    Hoehe.SetText   sHeight
    Sleep 2
    Position.Select iPosition
    Sleep 2
    TabSpalten.OK
    
    Call fFormatFrame("TabSpalten")
    if Art.GetSelIndex      <> iLine      then Warnlog "Line isn't correct!"
    Sleep 1
    if Hoehe.GetText        <> sHeight    then Warnlog "Height isn't correct!"
    Sleep 1
    if Position.GetSelIndex <> iPosition  then Warnlog "Position isn't correct!"
    Sleep 1
    TabSpalten.Cancel
    
    Call hCloseDocument
    
endcase

'-----------------------------------------------------------------

testcase tTextframes_28

    Dim sFrameName1 as String
    Dim sFrameName2 as String
    
    sFrameName1 = "Test1"
    sFrameName2 = "Test2"
    
    printlog "- Test overlap with different wrapping types"
    
    Call hNewDocument
    
    printlog "Insert a frame"
    Call fInsertFrame(30, 30, 60, 60)
    Sleep 2
    printlog "Naming the frame"
    Call fFormatFrame("TabZusaetze")
    ObjektName.SetText sFrameName1
    TabZusaetze.OK
    
    printlog "Set the frame's wrap"
    Call fFormatFrame("TabUmlauf")
    Kein.Check
    Sleep 1
    TabUmlauf.OK
    Sleep 2
    Call wTypeKeys "<ESCAPE>"
    Sleep 2
    
    printlog "Insert another frame"
    Call fInsertFrame(50, 50, 80, 80)
    Sleep 2
    printlog "Naming the frame"
    Call fFormatFrame("TabZusaetze")
    ObjektName.SetText sFrameName2
    TabZusaetze.OK
    
    printlog "Set the frame's wrap"
    Call fFormatFrame("TabUmlauf")
    Sleep 1
    Durchlauf.Check
    Sleep 1
    TabUmlauf.OK
    
    Call wTypeKeys "<ESCAPE>"
    Sleep 2
    
    printlog "Set focus to second frame"
    Call gMouseClick(60,60) ' Click into frame
    wait 500
    Call wTypeKeys "<ESCAPE>" ' Selects frame holding the cursor
    wait 500
    
    printlog "Check second frame's name"
    Call fFormatFrame("TabZusaetze")
    if ObjektName.GetText <> sFrameName2 then Warnlog "The 2nd frame's name is wrong ! Get " +ObjektName.GetText
    TabZusaetze.Cancel
    
    printlog "Check second frame's wrap status"
    Call fFormatFrame("TabUmlauf")
    if Durchlauf.IsChecked <> TRUE then Warnlog "The 2nd frame's wrap status is wrong ! "
    TabUmlauf.Cancel
    
    printlog "Set focus to first frame"
    Call wTypeKeys "<TAB>"
    Sleep 2
    
    printlog "Check first frame's name"
    Call fFormatFrame("TabZusaetze")
    if ObjektName.GetText <> sFrameName1 then Warnlog "The 1st frame's name is wrong ! Get " +ObjektName.GetText
    TabZusaetze.Cancel
    
    printlog "Check first frame's wrap status"
    Call fFormatFrame("TabUmlauf")
    if Kein.IsChecked <> TRUE then Warnlog "The 1st frame's wrap status is wrong ! "
    TabUmlauf.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_29

    Dim sLeft1   as String
    Dim sRight1  as String
    Dim sTop1    as String
    Dim sBottom1 as String
    
    sLeft1   =  "0"+ gSeperator + "1"
    sRight1  =  "0"+ gSeperator + "2"
    sTop1    =  "0"+ gSeperator + "3"
    sBottom1 =  "0"+ gSeperator + "4"
    
    printlog "- Test 'synchronize' distances for border (Check Synchronize)"
    
    Call hNewDocument
    
    '/// insert a frame
    Call wInsertFrame
    
    '/// Format / Frame / Borders , check Synchronize ,
    '/// + then set spacing to contents
    '/// + to differect number (left, right, top, bottom)
    Call fFormatFrame("TabUmrandung")
    Synchronisieren.Check
    wait 500
    Links.SetText    sLeft1
    wait 500
    TabUmrandung.TypeKeys "<SHIFT TAB>"
    Rechts.SetText   sRight1
    wait 500
    TabUmrandung.TypeKeys "<SHIFT TAB>"
    Oben.SetText     sTop1
    wait 500
    TabUmrandung.TypeKeys "<SHIFT TAB>"
    Unten.SetText    sBottom1
    wait 500
    TabUmrandung.TypeKeys "<SHIFT TAB>"
    TabUmrandung.OK
    
    '/// Check if the spacing to contents is right
    Call fFormatFrame("TabUmrandung")
    if Synchronisieren.IsChecked <> TRUE     then Warnlog "checkbox synchronize should be checked!"
    if Instr(Links.Gettext, sBottom1) = false then Warnlog "spacing to contents (left) isn't right!"
    if Instr(Rechts.GetText, sBottom1) = false then Warnlog "spacing to contents (right) isn't right!"
    if Instr(Oben.GetText, sBottom1) = false then Warnlog "spacing to contents (top) isn't right!"
    if Instr(Unten.GetText, sBottom1) = false then Warnlog "spacing to contents (bottom) isn't right!"
    TabUmrandung.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_30

    Dim sLeft1   as String
    Dim sRight1  as String
    Dim sTop1    as String
    Dim sBottom1 as String
    
    sLeft1   =  "0"+ gSeperator + "1"
    sRight1  =  "0"+ gSeperator + "2"
    sTop1    =  "0"+ gSeperator + "3"
    sBottom1 =  "0"+ gSeperator + "4"
    
    printlog "- Test 'synchronize' distances for border (UnCheck Synchronize)"
    
    Call hNewDocument
    
    '/// insert a frame
    Call wInsertFrame
    
    '/// Format / Frame / Borders , Uncheck Synchronize ,
    '/// + then set spacing to contents
    '/// + to differect number (left, right, top, bottom)
    Call fFormatFrame("TabUmrandung")
    Synchronisieren.UnCheck
    wait 500
    Links.SetText    sLeft1
    wait 500
    TabUmrandung.TypeKeys "<SHIFT TAB>"
    TabUmrandung.TypeKeys "<SHIFT TAB>"
    Rechts.SetText   sRight1
    wait 500
    TabUmrandung.TypeKeys "<SHIFT TAB>"
    Oben.SetText     sTop1
    wait 500
    TabUmrandung.TypeKeys "<SHIFT TAB>"
    Unten.SetText    sBottom1
    wait 500
    TabUmrandung.TypeKeys "<SHIFT TAB>"
    TabUmrandung.OK
    
    '/// Check if the spacing to contents is right
    Call fFormatFrame("TabUmrandung")
    if Synchronisieren.IsChecked = TRUE then Warnlog "checkbox synchronize should be checked!"
    if Instr(Links.Gettext, sLeft1) = false  then Warnlog "spacing to contents (left) isn't right!"  '<> sLeft1
    if Instr(Rechts.GetText, sRight1) = false then Warnlog "spacing to contents (right) isn't right!"
    if Instr(Oben.GetText, sTop1) = false then Warnlog "spacing to contents (top) isn't right!"
    if Instr(Unten.GetText, sBottom1) = false then Warnlog "spacing to contents (bottom) isn't right!"
    TabUmrandung.Cancel
    
    Call hCloseDocument
    
endcase

'-----------------------------------------------------------------

testcase tTextframes_31

    Dim sTestFile as String
    
    sTestFile = ConvertPath (gOfficePath + "user\work\testFileReadOnly.odt" )
    
    if Dir(sTestFile) <> "" then
        setattr sTestFile , 0
        Sleep 2
        kill sTestFile
    end if
    
    printlog "- Frame should be non-editable in read-only documents"
    
    Call hNewDocument
    
    '/// insert a frame
    Call wInsertFrame
    
    '/// Save the file and set the file's property to Read-only
    Call hFileSaveAs (sTestFile ,true)
    
    Call hCloseDocument
    Sleep 1
    
    setattr sTestFile , 1
    Sleep 2
    
    '/// Check if the frame is non-editable
    hFileOpen( sTestFile )
    
    Call wTypeKeys ( "<Shift F4>" )
    
    try
        FormatFrame
        Kontext
        Active.SetPage TabType
        Kontext "TabType"
        TabType.Cancel
        Warnlog "The frame should be non-editable!"
    catch
    endcatch
    
    Call hCloseDocument
    
endcase

'-----------------------------------------------------------------

testcase tTextframes_32

    Dim sTestFile as String
    Dim sText     as String
    
    sText     = "Test"
    sTestFile = ConvertPath (gOfficePath + "user\work\testFileReadOnly.odt" )
    
    if Dir(sTestFile) <> "" then
        setattr sTestFile , 0
        Sleep 2
        kill sTestFile
    end if
    
    printlog "- Test input text into frame in read-only document when checking 'Editable in read-only document'"
    
    Call hNewDocument
    
    '/// insert a frame
    Call fInsertFrame(30,30 , 70,70)
    
    '/// Format / Frame / Options , check "Editable in read-only document"
    Call fFormatFrame("TabZusaetze")
    Editierbar.Check
    wait 500
    TabZusaetze.OK
    Call wTypeKeys "<ESCAPE>"
    
    '/// Save the file and set the file's property to Read-only
    Call hFileSaveAs (sTestFile ,true)
    
    Call hCloseDocument
    
    setattr sTestFile , 1
    Sleep 2
    
    '/// Check if the text can be inputed into frame
    hFileOpen( sTestFile )
    
    Call wTypeKeys "<SHIFT F4><RETURN>"
    Wait 500
    
    try
        Call wTypeKeys sText
        Sleep 1
    catch
        Warnlog "Can't input text into frame !"
    endcatch
    
    Call wTypeKeys "<Shift Home>"
    Sleep 1
    
    EditCopy
    Sleep 1
    
    if GetClipboardText <> sText then Warnlog "The inputed text isn't right!"
    
    Call hCloseDocument
    
endcase

' ------------------------------------------------------------------------------
