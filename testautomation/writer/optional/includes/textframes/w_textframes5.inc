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
'* short description : Test of textframe - 5
'*
'\***********************************************************************

sub w_textframes5

    Call tTextframes_71         'Test negative value in Horizontal and vertical
    Call tTextframes_72         ''Vertical to' when anchor is inside frame
    Call tTextframes_73         ''Vertical to' when anchor is inside document body
    Call tTextframes_74         ''Vertical to' when anchor is inside table cell
    Call tTextframes_75         ''Vertical to' when anchor is inside header
    Call tTextframes_76         ''Vertical to' when anchor is inside footer
    Call tTextframes_77         ''Vertical to' when anchor is inside Footnote
    Call tTextframes_78         ''Vertical to' when anchor is inside endnote
    Call tTextframes_80         'Line of text - Top
    Call tTextframes_81         'Line of text - Bottom
    Call tTextframes_82         'Line of text - Center
    Call tTextframes_83         'Line of text - From Bottom (positive)
    Call tTextframes_84         'Line of text - From Bottom (negative)
    Call tTextframes_85         'Import from MS Word

end sub

'---------------------------------------------------

testcase tTextframes_71

    Dim iVertical     as Integer
    Dim sVerticalBy   as String
    Dim iHorizontal   as Integer
    Dim sHorizontalBy as String
    
    iHorizontal    = 4 'From Left
    sHorizontalBy  = "-1"+ gSeperator + "00" + gMeasurementUnit
    iVertical      = 4 'From top
    sVerticalBy    = "-1"+ gSeperator + "50" + gMeasurementUnit
    
    printlog "- Test negative value in Horizontal and vertical"
    '/// Test negative value in Horizontal and vertical
    
    Call hNewDocument
    
    '/// insert a frame
    '/// + Set Horizontal as "from top" ,
    '/// + set Horizontal by to -1cm ,
    '/// + set Vertical as "from Left" ,
    '/// + set vertical by to -1.5cm
    InsertFrame
    Kontext
    Active.Setpage TabType
    Kontext "TabType"
    Horizontal.Select    iHorizontal
    Sleep 1
    HorizontalBy.SetText sHorizontalBy
    Sleep 1
    Vertical.Select      iVertical
    Sleep 1
    VerticalBy.SetText   sVerticalBy
    Sleep 1
    TabType.Ok
    
    '/// Check if the configuration is effective
    '/// Format/Frame / Type ,
    Call fFormatFrame("TabType")
    
    if fCalculateTolerance( HorizontalBy.GetText, sHorizontalBy ) > 0.2 then
        Warnlog "Something wrong in Horizontal by !"
    end if
    if fCalculateTolerance( VerticalBy.GetText, sVerticalBy ) > 0.2 then
        Warnlog "Something wrong in Vertical by !"
    end if
    TabType.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_72

    printlog "- 'Vertical to' when anchor is inside frame"
    '/// 'Vertical to' when anchor is inside frame
    
    Call hNewDocument
    
    '/// Open a test file , which includes 2 frames ,
    '/// Frame A is anchored Frame B
    hFileOpenLocally( gTesttoolpath + "writer\optional\input\textframe\frameInFrame.sxw" )
    
    'Set focus to Frame B
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + Check if "Entire frame" and "Frame text area"
    '/// + are in Vertical to area
    Call fFormatFrame("TabType")
    if HorizontalTo.GetSelIndex <> 3 then
        Warnlog "Horizontal To should be 'Entire Frame' but get " & HorizontalTo.GetSelText
    end if
    if VerticalTo.GetSelIndex <> 2 then
        Warnlog "Vertical To should be 'Frame Text Area' but get " & VerticalTo.GetSelText
    end if
    
    TabType.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_73

    printlog "- 'Vertical to' when anchor is inside document body"
    '/// 'Vertical to' when anchor is inside document body
    
    Call hNewDocument
    
    '/// Open a test file , which includes 1 frame in the document body
    hFileOpenLocally( gTesttoolpath & "writer\optional\input\textframe\frame.sxw" )
    
    '/// Set focus to Frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + Check if "Entire page" and "Page text area"
    '/// + are in Vertical to area
    Call fFormatFrame("TabType")
    if HorizontalTo.GetSelIndex <> 7 then
        Warnlog "Horizontal To should be 'Entire Page' but get " & HorizontalTo.GetSelText
    end if
    if VerticalTo.GetSelIndex <> 4 then
        Warnlog "Vertical To should be 'Page Text Area' but get " & VerticalTo.GetSelText
    end if
    TabType.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_74

    printlog "- 'Vertical to' when anchor is inside table cell"
    '/// 'Vertical to' when anchor is inside table cell
    
    Call hNewDocument
    
    '/// Open a test file , which includes 1 frame in the document body
    hFileOpenLocally( gTesttoolpath + "writer\optional\input\textframe\frameInTable.sxw" )
    
    '/// Set focus to Frame
    Call wTypeKeys ( "<Shift F4>" )
    
    '/// Format/Frame / Type ,
    '/// + Check if "Entire page" and "Page text area"
    '/// + are in Vertical to area
    Call fFormatFrame("TabType")
    if HorizontalTo.GetSelIndex <> 7 then
        Warnlog "Horizontal To should be 'Entire Page' but get " & HorizontalTo.GetSelText
    end if
    if VerticalTo.GetSelIndex <> 4 then
        Warnlog "Vertical To should be 'Page Text Area' but get " & VerticalTo.GetSelText
    end if
    TabType.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_75

    printlog "- 'Vertical to' when anchor is inside header"
    '/// 'Vertical to' when anchor is inside header
    
    Call hNewDocument
    
    '/// Open a test file , which includes 1 frame in the header
    hFileOpenLocally( gTesttoolpath + "writer\optional\input\textframe\Header.sxw" )
    
    '/// Set focus to Frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + Check if "Entire page" and "Page text area"
    '/// + are in Vertical to area
    Call fFormatFrame("TabType")
    if HorizontalTo.GetSelIndex <> 7 then
        Warnlog "Horizontal To should be 'Entire Page' but get " & HorizontalTo.GetSelText
    end if
    if VerticalTo.GetSelIndex <> 4 then
        Warnlog "Vertical To should be 'Page Text Area' but get " & VerticalTo.GetSelText
    end if
    TabType.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_76

    printlog "- 'Vertical to' when anchor is inside footer"
    '/// 'Vertical to' when anchor is inside footer
    
    Call hNewDocument
    
    '/// Open a test file , which includes 1 frame in the footer
    hFileOpenLocally( gTesttoolpath + "writer\optional\input\textframe\Footer.sxw" )
    
    '/// Set focus to Frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + Check if "Entire page" and "Page text area"
    '/// + are in Vertical to area
    Call fFormatFrame("TabType")
    if HorizontalTo.GetSelIndex <> 7 then
        Warnlog "Horizontal To should be 'Entire Page' but get " & HorizontalTo.GetSelText
    end if
    if VerticalTo.GetSelIndex <> 4 then
        Warnlog "Vertical To should be 'Page Text Area' but get " & VerticalTo.GetSelText
    end if
    TabType.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_77

    printlog "- 'Vertical to' when anchor is inside Footnote"
    '/// 'Vertical to' when anchor is inside Footnote
    
    Call hNewDocument
    
    '/// Open a test file , which includes 1 frame in the footnote
    hFileOpen( gTesttoolpath & "writer\optional\input\textframe\frameInFootnote.sxw" )
    
    '/// Set focus to Frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + Check if "Entire page" and "Page text area"
    '/// + are in Vertical to area
    Call fFormatFrame("TabType")
    if HorizontalTo.GetSelIndex <> 7 then
        Warnlog "Horizontal To should be 'Entire Page' but get " & HorizontalTo.GetSelText
    end if
    if VerticalTo.GetSelIndex <> 4 then
        Warnlog "Vertical To should be 'Page Text Area' but get " & VerticalTo.GetSelText
    end if
    TabType.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_78

    printlog "- 'Vertical to' when anchor is inside endnote"
    '/// 'Vertical to' when anchor is inside endnote
    
    Call hNewDocument
    
    '/// Open a test file , which includes 1 frame in the endnote
    hFileOpenLocally( gTesttoolpath & "writer\optional\input\textframe\frameInEndnote.sxw" )
    
    '/// Set focus to Frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + Check if "Entire page" and "Page text area"
    '/// + are in Vertical to area
    Call fFormatFrame("TabType")
    if HorizontalTo.GetSelIndex <> 7 then
        Warnlog "Horizontal To should be 'Entire Page' but get " & HorizontalTo.GetSelText
    end if
    if VerticalTo.GetSelIndex <> 4 then
        Warnlog "Vertical To should be 'Page Text Area' but get " & VerticalTo.GetSelText
    end if
    TabType.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_80

    Dim sTestFile   as String
    Dim iVertical   as Integer
    Dim iVerticalTo as Integer
    Dim sPositionY  as String
    
    sTestFile   = Convertpath (gTesttoolpath + "writer\optional\input\textframe\lineOfText.sxw")
    iVertical   = 1    ' Top
    iVerticalTo = 6    ' Line of text'
    sPositionY  = "0"+ gSeperator + "20" + gMeasurementUnit
    
    printlog "- 'Line of text' - Top"
    '/// 'Line of text' - Top
    
    Call hNewDocument
    
    '/// Open a test file , which includes 1 frame which is
    '/// + anchored 'To Character' and a picture which is anchored
    '/// + in the frame
    hFileOpenLocally( sTestFile )
    
    '/// Set focus to Frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format/Frame / Type , select Top in Vertical ,
    '/// + select 'Line of text' in vertical to
    Call fFormatFrame("TabType")
    Vertical.Select iVertical
    Sleep 1
    VerticalTo.Select iVerticalTo
    Sleep 1
    TabType.OK
    
    '/// Check if the result is right
    'Set focus to the picture
    Call wTypeKeys "<Tab>"
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    if fCalculateTolerance( sPositionY, Verticalby.Gettext) > 0.3 then
       Warnlog "The picture's position isn't right !"
    end if
    TabPositionAndSizeWriter.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_81

    Dim sTestFile   as String
    Dim iVertical   as Integer
    Dim iVerticalTo as Integer
    Dim sPositionY  as String
    
    sTestFile   = Convertpath (gTesttoolpath + "writer\optional\input\textframe\lineOfText.sxw")
    iVertical   = 2    ' Bottom
    iVerticalTo = 6    ' Line of text'
    sPositionY  = "3"+ gSeperator + "50" + gMeasurementUnit
    
    printlog "- 'Line of text' - Bottom"
    '/// 'Line of text' - Bottom
    
    Call hNewDocument
    
    '/// Open a test file , which includes 1 frame which is
    '/// + anchored 'To Character' and a picture which is anchored
    '/// + in the frame
    hFileOpenLocally( sTestFile )
    
    '/// Set focus to Frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format/Frame / Type , select Bottom in Vertical ,
    '/// + select 'Line of text' in vertical to
    Call fFormatFrame("TabType")
    Vertical.Select iVertical
    Sleep 1
    VerticalTo.Select iVerticalTo
    Sleep 1
    TabType.OK
    
    '/// Check if the result is right
    'Set focus to the picture
    Call wTypeKeys "<Tab>"
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    if fCalculateTolerance( sPositionY, Verticalby.Gettext) > 0.3 then
       Warnlog "The picture's position isn't right !"
    end if
    TabPositionAndSizeWriter.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_82

    Dim sTestFile   as String
    Dim iVertical   as Integer
    Dim iVerticalTo as Integer
    Dim sPositionY  as String
    
    sTestFile   = Convertpath (gTesttoolpath + "writer\optional\input\textframe\lineOfText.sxw")
    iVertical   = 4    ' Center
    iVerticalTo = 6    ' Line of text'
    sPositionY  = "1"+ gSeperator + "80" + gMeasurementUnit
    
    printlog "- 'Line of text' - Center"
    '/// 'Line of text' - Center
    
    Call hNewDocument
    
    '/// Open a test file , which includes 1 frame which is
    '/// + anchored 'To Character' and a picture which is anchored
    '/// + in the frame
    hFileOpenLocally( sTestFile )
    
    '/// Set focus to Frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format/Frame / Type , select Center in Vertical ,
    '/// + select 'Line of text' in vertical to
    Call fFormatFrame("TabType")
    Vertical.Select iVertical
    Sleep 1
    VerticalTo.Select iVerticalTo
    Sleep 1
    TabType.OK
    
    '/// Check if the result is right
    'Set focus to the picture
    Call wTypeKeys "<Tab>"
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    if fCalculateTolerance( sPositionY, Verticalby.Gettext) > 0.3 then
       Warnlog "The picture's position isn't right !"
    end if
    TabPositionAndSizeWriter.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_83

    Dim sTestFile   as String
    Dim iVertical   as Integer
    Dim iVerticalTo as Integer
    Dim sPositionY  as String
    Dim sVerticalBy as String
    
    sTestFile   = Convertpath (gTesttoolpath + "writer\optional\input\textframe\lineOfText.sxw")
    iVertical   = 6    ' From Bottom
    iVerticalTo = 2    ' Line of text'
    sPositionY  = "2"+ gSeperator + "30" + gMeasurementUnit
    sVerticalBy = "1"+ gSeperator + "00" + gMeasurementUnit
    
    printlog "- 'Line of text' - From Bottom(positive)"
    '/// 'Line of text' - From Bottom(positive)
    
    Call hNewDocument
    
    '/// Open a test file , which includes 1 frame which is
    '/// + anchored 'To Character' and a picture which is anchored
    '/// + in the frame
    hFileOpenLocally( sTestFile )
    
    '/// Set focus to Frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format/Frame / Type , select From Bottom in Vertical ,
    '/// + select 'Line of text' in vertical to ,
    '/// + input a positive number in vertical by
    Call fFormatFrame("TabType")
    if Vertical.GetItemCount <> 6 then
        warnlog " Missing string in frame options"
        TabType.Cancel
        Call hCloseDocument
        goto endsub
    end if
    Vertical.Select    iVertical
    Sleep 1
    VerticalTo.Select  iVerticalTo
    Sleep 1
    VerticalBy.SetText sVerticalBy
    TabType.OK
    
    '/// Check if the result is right
    'Set focus to the picture
    Call wTypeKeys "<Tab>"
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    if fCalculateTolerance( sPositionY, Verticalby.Gettext) > 0.3 then
       Warnlog "The picture's position isn't right !"
    end if
    TabPositionAndSizeWriter.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_84

    Dim sTestFile   as String
    Dim iVertical   as Integer
    Dim iVerticalTo as Integer
    Dim sPositionY  as String
    Dim sVerticalBy as String
    
    sTestFile   = Convertpath (gTesttoolpath + "writer\optional\input\textframe\lineOfText.sxw")
    iVertical   = 6    ' From Bottom
    iVerticalTo = 2    ' Line of text'
    sPositionY  = "4"+ gSeperator + "30" + gMeasurementUnit
    sVerticalBy = "-1"+ gSeperator + "00" + gMeasurementUnit
    
    printlog "- 'Line of text' - From Bottom(negative)"
    '/// 'Line of text' - From Bottom(negative)
    
    Call hNewDocument
    
    '/// Open a test file , which includes 1 frame which is
    '/// + anchored 'To Character' and a picture which is anchored
    '/// + in the frame
    hFileOpenLocally( sTestFile )
    
    '/// Set focus to Frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format/Frame / Type , select From Bottom in Vertical ,
    '/// + select 'Line of text' in vertical to ,
    '/// + input a negative number in vertical by
    Call fFormatFrame("TabType")
    if Vertical.GetItemCount <> 6 then
        warnlog " Missing string in frame options"
        TabType.Cancel
        Call hCloseDocument
        goto endsub
    end if
    Vertical.Select    iVertical
    Sleep 1
    VerticalTo.Select  iVerticalTo
    Sleep 1
    VerticalBy.SetText sVerticalBy
    TabType.OK
    
    '/// Check if the result is right
    'Set focus to the picture
    Call wTypeKeys "<Tab>"
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    if fCalculateTolerance( sPositionY, Verticalby.Gettext) > 0.3 then
       Warnlog "The picture's position isn't right !"
    end if
    TabPositionAndSizeWriter.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_85

    Dim sTestFile       as String
    Dim iVerticalTop    as Integer
    Dim iVerticalBottom as Integer
    Dim iVerticalCenter as Integer
    Dim iVerticalTo     as Integer
    
    sTestFile       = Convertpath (gTesttoolpath + "writer\optional\input\textframe\lineOfText.doc")
    iVerticalTop    = 1    ' Top
    iVerticalBottom = 2    ' Bottom
    iVerticalCenter = 4    ' Center
    
    iVerticalTo     = 6    ' Line of text'
    
    printlog "- Import from MS Word"
    '/// Import from MS Word
    
    Call hNewDocument
    
    '/// Open a MS Word file , which includes 3 frames ,
    '/// + One is top to the line text ,
    '/// + another one is center to the line text ,
    '/// + the 3rd one is bottom to the line text
    hFileOpenLocally( sTestFile )
    
    '/// Set focus to 1st Frame
    Call wTypeKeys ( "<Shift F4>" )
    
    '/// Format/Frame / Type , check if 'To character' in anchor area
    '/// + check if 'Line of text' in vertical to ,
    '/// + check if 'Top' in Vertical
    Call fFormatFrame("TabType")
    if AnchorAtCharacter.IsChecked <> TRUE then
       Warnlog "The anchor should be To Character!"
    end if
    if Vertical.GetSelIndex <> iVerticalTop then
       Warnlog "should be Top , but get " &Vertical.GetSelText
    end if
    if VerticalTo.GetSelIndex <> iVerticalTo then
       Warnlog "Top:should be line of text , but get " &VerticalTo.GetSelText
    end if
    TabType.Cancel
    
    '/// Set focus to 2nd Frame
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    '/// Format/Frame / Type , check if 'To character' in anchor area
    '/// + check if 'Line of text' in vertical to ,
    '/// + check if 'Center' in Vertical
    Call fFormatFrame("TabType")
    if AnchorAtCharacter.IsChecked <> TRUE then
       Warnlog "The anchor should be To Character!"
    end if
    if Vertical.GetSelIndex <> iVerticalCenter then
       Warnlog "should be Center , but get " &Vertical.GetSelText
    end if
    if VerticalTo.GetSelIndex <> iVerticalTo then
       Warnlog "Center:should be line of text , but get " &VerticalTo.GetSelText
    end if
    TabType.Cancel
    
    '/// Set focus to 3rd Frame
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    '/// Format/Frame / Type , check if 'To character' in anchor area
    '/// + check if 'Line of text' in vertical to ,
    '/// + check if 'Bottom' in Vertical
    Call fFormatFrame("TabType")
    if AnchorAtCharacter.IsChecked <> TRUE then
       Warnlog "The anchor should be To Character!"
    end if
    if Vertical.GetSelIndex <> iVerticalBottom then
       Warnlog "should be Bottom , but get " &Vertical.GetSelText
    end if
    if VerticalTo.GetSelIndex <> iVerticalTo then
       Warnlog "Bottom:should be line of text , but get " &VerticalTo.GetSelText
    end if
    TabType.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

' ------------------------------------------------------------------------------
