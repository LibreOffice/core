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
'* short description : Test of textframe - 4
'*
'\***********************************************************************

sub w_textframes4
    Call tTextframes_49         ''Follow text flow' should be OFF by default
    Call tTextframes_50         ''Follow text flow' should be ON if document is older than StarOffice 8
    Call tTextframes_51         ''Follow text flow' should be OFF when wrapping mode is 'in front of text'
    Call tTextframes_52         ''Follow text flow' should be OFF when wrapping mode is 'behind text'
    Call tTextframes_53         ''Follow text flow' should be OFF when wrapping mode is 'tight'
    Call tTextframes_54         ''Follow text flow' should be ON when wrapping mode is 'square'
    Call tTextframes_55         'Exceeding layout environment at the bottom - 'Follow text flow'(ON , page area)
    Call tTextframes_56         'Exceeding layout environment at the bottom - 'Follow text flow'(OFF, page area)
    Call tTextframes_57         'Exceeding layout environment at the bottom - 'Follow text flow'(ON , header)
    Call tTextframes_58         'Exceeding layout environment at the bottom - 'Follow text flow'(OFF, header)
    Call tTextframes_59         'Exceeding layout environment at the bottom - 'Follow text flow'(ON , table)
    Call tTextframes_60         'Exceeding layout environment at the bottom - 'Follow text flow'(OFF, table)
    Call tTextframes_61         'Exceeding layout environment at the top - 'Follow text flow'(ON , footer)
    Call tTextframes_62         'Exceeding layout environment at the top - 'Follow text flow'(OFF, footer)
    Call tTextframes_63         'Exceeding layout environment at the left - 'Follow text flow'(ON , table)
    Call tTextframes_64         'Exceeding layout environment at the left - 'Follow text flow'(OFF, table)
    Call tTextframes_65         'Exceeding layout environment at the left - 'Follow text flow'(ON , Frame)
    Call tTextframes_66         'Exceeding layout environment at the left - 'Follow text flow'(OFF, Frame)
    Call tTextframes_67         'Exceeding layout environment at the right - 'Follow text flow'(ON , table)
    Call tTextframes_68         'Exceeding layout environment at the right - 'Follow text flow'(OFF, table)
    Call tTextframes_69         'Exceeding layout environment at the right - 'Follow text flow'(ON , Frame)
    Call tTextframes_70         'Exceeding layout environment at the right - 'Follow text flow'(OFF, Frame)
end sub

'---------------------------------------------------

testcase tTextframes_49

    printlog "- 'Follow text flow' should be OFF by default"
    '/// 'Follow text flow' should be OFF by default
    
    Call hNewDocument
    
    '/// insert a frame
    Call wInsertFrame
    
    '/// Format/Frame / Type ,
    '/// + Check box 'Follow text flow' should be OFF
    Call fFormatFrame("TabType")
    if FollowTextflow.IsChecked then
        Warnlog "'Follow text flow' should be turned off!"
    end if
    TabType.Cancel
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_50

    Dim sTestFile as String
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\textframe\frame.sxw")
    
    printlog "- 'Follow text flow' should be ON if document is older than StarOffice 8"
    '/// Follow text flow' should be ON if document is older than StarOffice 8
    
    Call hNewDocument
    
    '/// Open a test file , which includes a frame and
    '/// + it is StarOffice 7 format
    hFileOpenLocally( sTestFile )
    
    
    '/// Set focus to frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + Check box 'Follow text flow' should be ON
    Call fFormatFrame("TabType")
    if NOT FollowTextflow.IsChecked then
        Warnlog "'Follow text flow' should be turned on!"
    end if
    TabType.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_51

    Dim sTestFile as String
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\textframe\inFrontText.doc")
    
    printlog "- 'Follow text flow' should be OFF when wrapping mode is 'in front of text'"
    '/// 'Follow text flow' should be OFF when wrapping mode is 'in front of text'
    
    Call hNewDocument
    
    '/// Open a test file , which is a Word file and includes
    '/// + a frame that is in 'in front of text' mode
    hFileOpenLocally( sTestFile )
    
    
    '/// Set focus to frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + Check box 'Follow text flow' should be OFF
    Call fFormatFrame("TabType")
    if FollowTextflow.IsChecked then
        Warnlog "'Follow text flow' should be turned off!"
    end if
    TabType.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_52

    Dim sTestFile as  String
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\textframe\behindText.doc")
    
    printlog "- 'Follow text flow' should be OFF when wrapping mode is 'behind text'"
    '/// 'Follow text flow' should be OFF when wrapping mode is 'behind text'
    
    Call hNewDocument
    
    '/// Open a test file , which is a Word file and includes
    '/// + a frame that is in 'behind text' mode
    hFileOpenLocally( sTestFile )
    
    
    '/// Set focus to the frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + Check box 'Follow text flow' should be OFF
    Call fFormatFrame("TabType")
    if FollowTextflow.IsChecked then
        Warnlog "'Follow text flow' should be turned off!"
    end if
    TabType.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_53

    Dim sTestFile as  String
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\textframe\tight.doc")
    
    printlog "- 'Follow text flow' should be OFF when wrapping mode is 'tight'"
    '/// 'Follow text flow' should be OFF when wrapping mode is 'tight'
    
    Call hNewDocument
    
    '/// Open a test file , which is a Word file and includes
    '/// + a frame that is in 'tight' mode
    hFileOpenLocally( sTestFile )
    
    
    '/// Set focus to the frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + Check box 'Follow text flow' should be OFF
    Call fFormatFrame("TabType")
    if FollowTextflow.IsChecked then
        Warnlog "'Follow text flow' should be turned off!"
    end if
    TabType.Cancel
    
    printlog " Close active document "
    hFileCloseAll()

endcase

'-----------------------------------------------------------------

testcase tTextframes_54

    Dim sTestFile as  String
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\textframe\square.doc")
    
    printlog "- 'Follow text flow' should be OFF when wrapping mode is 'square'"
    '/// 'Follow text flow' should be OFF when wrapping mode is 'square'
    
    Call hNewDocument
    
    '/// Open a test file , which is a Word file and includes
    '/// + a frame that is in 'square' mode
    hFileOpenLocally( sTestFile )
    
    
    '/// Set focus to the frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + Check box 'Follow text flow' should be OFF
    Call fFormatFrame("TabType")
    if FollowTextflow.IsChecked then
        Warnlog "'Follow text flow' should be turned off!"
    end if
    TabType.Cancel
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_55

    Dim iVertical   as Integer
    Dim sVerticalBy as String
    
    iVertical    = 4 'From top
    sVerticalBy  = "50"
    
    printlog "- Exceeding layout environment at the bottom - 'Follow text flow'(ON , page area)"
    '/// Exceeding layout environment at the bottom - 'Follow text flow'(ON , page area)
    
    Call hNewDocument
    
    '/// Insert/Frame / Type ,
    '/// + Select 'from top' in Vertical ,
    '/// + Set 50cm in vertical by area
    '/// + Check 'Follow text flow'
    InsertFrame
    Kontext
    Active.Setpage TabType
    Kontext "TabType"
    Vertical.Select  iVertical
    Sleep 1
    VerticalBy.SetText sVerticalBy
    Sleep 1
    FollowTextflow.Check
    TabType.Ok
    
    '/// Check if the frame is in 2nd page
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "2" then
        Warnlog "The test frame isn't in the second page!"
    end if
    
    Call fCloseNavigator
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_56

    Dim iVertical   as Integer
    Dim sVerticalBy as String
    
    iVertical    = 4 'From top
    sVerticalBy  = "50"
    
    printlog "- Exceeding layout environment at the bottom - 'Follow text flow'(OFF , page area)"
    '/// Exceeding layout environment at the bottom - 'Follow text flow'(OFF , page area)
    
    Call hNewDocument
    
    '/// Insert/Frame / Type ,
    '/// + Select 'from top' in Vertical ,
    '/// + Set 50cm in vertical by area
    '/// + UnCheck 'Follow text flow'
    InsertFrame
    Kontext
    Active.Setpage TabType
    Kontext "TabType"
    Vertical.Select  iVertical
    Sleep 1
    VerticalBy.SetText sVerticalBy
    Sleep 1
    FollowTextflow.UnCheck
    TabType.Ok
    
    '/// Check if the frame is in 1st page
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then
        Warnlog "The test frame isn't in the first page!"
    end if
    
    Call fCloseNavigator
    
    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tTextframes_57

    Dim sTestFile   as String
    Dim iVertical   as Integer
    Dim sVerticalBy as String
    Dim sPositionY1 as String
    Dim sPositionY2 as String
    
    sTestFile    = Convertpath (gTesttoolpath + "writer\optional\input\textframe\frameInHeader.sxw")
    iVertical    = 4 'From top
    sVerticalBy  = "50"
    
    printlog "- Exceeding layout environment at the bottom - 'Follow text flow'(ON , header)"
    '/// Exceeding layout environment at the bottom - 'Follow text flow'(ON , header)
    
    Call hNewDocument
    
    '/// Open a test file , which includes a frame in header and
    '/// + a picture which is anchored to frame
    hFileOpenLocally( sTestFile )
    
    
    'Set focus to the picture
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic Y position
    FormatAnchorToPage
    Sleep 1

    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionY1 = Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
    FormatAnchorToFrame
    Sleep 1
    
    '/// Set focus to the frame
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + Check box 'Follow text flow'
    '/// + set a much bigger values in from top
    Call fFormatFrame("TabType")
    FollowTextflow.Check
    Vertical.Select  iVertical
    Sleep 1
    VerticalBy.SetText sVerticalBy
    Sleep 1
    TabType.OK
    
    'Set focus to picture
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic Y position
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionY2 = Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel

    '/// Check if frame is still in the header
    if fCalculateTolerance( sPositionY1, sPositionY2) > 0.3 then
        Warnlog "Something is wrong ! Perhaps the frame isn't in the header!"
    end if
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_58

    Dim sTestFile   as String
    Dim iVertical   as Integer
    Dim sVerticalBy as String
    Dim sPositionY1 as String
    Dim sPositionY2 as String
    
    sTestFile    = Convertpath (gTesttoolpath + "writer\optional\input\textframe\frameInHeader.sxw")
    iVertical    = 4 'From top
    sVerticalBy  = "50"
    
    printlog "- Exceeding layout environment at the bottom - 'Follow text flow'(OFF , header)"
    '/// Exceeding layout environment at the bottom - 'Follow text flow'(OFF , header)
    
    Call hNewDocument
    
    '/// Open a test file , which includes a frame in header and
    '/// + a picture which is anchored to frame
    hFileOpenLocally( sTestFile )
    
    
    'Set focus to the picture
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic Y position
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionY1 = Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
    FormatAnchorToFrame
    Sleep 1
    
    '/// Set focus to the frame
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + UnCheck box 'Follow text flow'
    '/// + set a much bigger values in from top
    Call fFormatFrame("TabType")
    FollowTextflow.UnCheck
    Vertical.Select  iVertical
    Sleep 1
    VerticalBy.SetText sVerticalBy
    Sleep 1
    TabType.OK
    
    'Set focus to picture
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic Y position
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionY2 = Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
    '/// Check if frame is still in the header
    if fCalculateTolerance( sPositionY1, sPositionY2 ) < 18 then
        Warnlog "Something is wrong ! Perhaps the frame is still in the header!"
    end if
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_59

    Dim sTestFile   as String
    Dim sPositionY1 as String
    Dim sPositionY2 as String
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\textframe\frameInTable.sxw")
    
    printlog "- Exceeding layout environment at the bottom - 'Follow text flow'(ON , table)"
    '/// Exceeding layout environment at the bottom - 'Follow text flow'(ON , table)
    
    Call hNewDocument
    
    '/// Open a test file , which includes a frame in table and
    '/// + a picture which is anchored to frame
    hFileOpenLocally( sTestFile )
    
    
    'Set focus to the picture
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    Call wTypeKeys ( "<Tab>" )
    
    FormatAnchorToFrame
    Sleep 1
    
    'Get the pic Y position
    Sleep 1
    Call fFormatGraphic("TabType")
    sPositionY1 = Verticalby.Gettext
    TabType.Cancel
        
    '/// Set focus to the frame
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + Check box 'Follow text flow'
    '/// + set a much bigger values in from top
    Call fFormatFrame("TabType")
    FollowTextflow.Check
    Vertical.Select 4
    Sleep 1
    VerticalBy.SetText "50"
    Sleep 1
    TabType.OK
    
    'Set focus to picture
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic Y position
    Sleep 1
    Call fFormatGraphic("TabType")
    sPositionY2 = Verticalby.Gettext
    TabType.Cancel
    
    '/// Check if frame is still in the table
    if fCalculateTolerance( sPositionY1, sPositionY2) > 0.3 then
        Warnlog "Something is wrong ! Perhaps the frame isn't in the table!"
    end if
    
    'Set focus to frame
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'if the frame is in the table, the focus should be in the table
    Call wTypeKeys "<ESCape>"
    Sleep 1
    
    try
        Call fFormatTable("TabTabelle")
        TabTabelle.Cancel
    catch
        Warnlog "The frame isn't in the table!"
    endcatch
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_60

    Dim sTestFile   as String
    Dim sPositionY1 as String
    Dim sPositionY2 as String
    
    sTestFile    = Convertpath (gTesttoolpath + "writer\optional\input\textframe\frameInTable.sxw")
    
    printlog "- Exceeding layout environment at the bottom - 'Follow text flow'(OFF , table)"
    '/// Exceeding layout environment at the bottom - 'Follow text flow'(OFF , table)
    
    Call hNewDocument
    
    '/// Open a test file , which includes a frame in table and
    '/// + a picture which is anchored to frame
    hFileOpenLocally( sTestFile )
    
    
    'Set focus to the frame
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    
    printlog "Move the frame into the document-area."
    Call wTypeKeys "<RIGHT>",10
    Call wTypeKeys "<DOWN>",7
    
    'Set focus to the picture
    Call wTypeKeys ( "<Tab>" )
    
    'Get the pic Y position
    Sleep 1
    Call fFormatGraphic("TabType")
    sPositionY1 = Verticalby.Gettext
    TabType.Cancel
    
    FormatAnchorToFrame
    Sleep 1
    
    '/// Set focus to the frame
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + UnCheck box 'Follow text flow'
    '/// + set a much bigger values in from top
    Call fFormatFrame("TabType")
    FollowTextflow.UnCheck
    Vertical.Select 4
    Sleep 1
    VerticalBy.ToMax
    Sleep 1
    TabType.OK
    
    'Set focus to picture
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic Y position
    Sleep 1
    Call fFormatGraphic("TabType")
    sPositionY2 = Verticalby.Gettext
    TabType.Cancel
    
    '/// Check if frame is still in the table
    if fCalculateTolerance( sPositionY1, sPositionY2) <> 0 then
        Warnlog "Something is wrong ! Perhaps the frame is still in the table!"
    end if
    
    'Set focus to frame
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'if the frame is in the table, the focus should be in the table
    Call wTypeKeys "<ESCape>"
    Sleep 1
    
    try
        Call fFormatTable("TabTabelle")
        TabTabelle.Cancel
        Warnlog "The frame should be in the table!"
    catch
    endcatch
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_61

    Dim sTestFile   as String
    Dim iVertical   as Integer
    Dim sVerticalBy as String
    Dim sPositionY1 as String
    Dim sPositionY2 as String
    
    sTestFile    = Convertpath (gTesttoolpath + "writer\optional\input\textframe\frameInFooter.sxw")
    iVertical    = 4 'From top
    sVerticalBy  = "-50"
    
    printlog "- Exceeding layout environment at the top - 'Follow text flow'(ON , footer)"
    '/// Exceeding layout environment at the top - 'Follow text flow'(ON , footer)
    
    Call hNewDocument
    
    '/// Open a test file , which includes a frame in footer and
    '/// + a picture which is anchored to frame
    hFileOpenLocally( sTestFile )
    
    
    'Set focus to the picture
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic Y position
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionY1 = Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
    FormatAnchorToFrame
    Sleep 1
    
    '/// Set focus to the frame
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + Check box 'Follow text flow'
    '/// + set a much bigger values in from top
    Call fFormatFrame("TabType")
    FollowTextflow.Check
    Vertical.Select  iVertical
    Sleep 1
    VerticalBy.SetText sVerticalBy
    Sleep 1
    TabType.OK
    
    'Set focus to picture
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic Y position
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionY2 = Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
    '/// Check if frame is still in the header
    if fCalculateTolerance( sPositionY1, sPositionY2) > 0.8 then
        Warnlog "Something is wrong ! Perhaps the frame isn't in the footer!"
    end if
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_62

    Dim sTestFile   as String
    Dim iVertical   as Integer
    Dim sVerticalBy as String
    Dim sPositionY1 as String
    Dim sPositionY2 as String
    
    sTestFile    = Convertpath (gTesttoolpath + "writer\optional\input\textframe\frameInFooter.sxw")
    iVertical    = 4 'From top
    sVerticalBy  = "-50"
    
    printlog "- Exceeding layout environment at the top - 'Follow text flow'(OFF , footer)"
    '/// Exceeding layout environment at the top - 'Follow text flow'(OFF , footer)
    
    Call hNewDocument
    
    '/// Open a test file , which includes a frame in footer and
    '/// + a picture which is anchored to frame
    hFileOpenLocally( sTestFile )
    
    
    'Set focus to the picture
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic Y position
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionY1 = Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
    FormatAnchorToFrame
    Sleep 1
    
    '/// Set focus to the frame
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + UnCheck box 'Follow text flow'
    '/// + set a much bigger values in from top
    Call fFormatFrame("TabType")
    FollowTextflow.UnCheck
    Vertical.Select  iVertical
    Sleep 1
    VerticalBy.SetText sVerticalBy
    Sleep 1
    TabType.OK
    
    'Set focus to picture
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic Y position
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionY2 = Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
    '/// Check if frame is still in the header
    if fCalculateTolerance( sPositionY1, sPositionY2 ) < 18 then
        Warnlog "Something is wrong ! Perhaps the frame is still in the footer!"
    end if
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_63

    Dim sTestFile     as String
    Dim iHorizontal   as Integer
    Dim sHorizontalBy as String
    Dim sPositionX1   as String
    Dim sPositionX2   as String
    
    sTestFile      = Convertpath (gTesttoolpath + "writer\optional\input\textframe\frameInTable.sxw")
    iHorizontal    = 4 'From Left
    sHorizontalBy  = "-50"
    
    printlog "- Exceeding layout environment at the left - 'Follow text flow'(ON , table)"
    '/// Exceeding layout environment at the left - 'Follow text flow'(ON , table)
    
    Call hNewDocument
    
    '/// Open a test file , which includes a frame in table and
    '/// + a picture which is anchored to frame
    hFileOpenLocally( sTestFile )
    
    
    'Set focus to the picture
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
	Call wTypeKeys ( "<Tab>" )
    
    'Get the pic X position
    Sleep 1
    Call fFormatGraphic("TabType")
    sPositionX1 = Horizontalby.Gettext
    TabType.Cancel
   
    FormatAnchorToFrame
    Sleep 1
    
    '/// Set focus to the frame
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + Check box 'Follow text flow'
    '/// + set a much bigger values in from left
    Call fFormatFrame("TabType")
    FollowTextflow.Check
    Horizontal.Select iHorizontal
    Sleep 1
    HorizontalBy.SetText sHorizontalBy
    Sleep 1
    TabType.OK
    
    'Set focus to picture
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic X position
    Sleep 1
    Call fFormatGraphic("TabType")
    sPositionX2 = Horizontalby.Gettext
    TabType.Cancel
    
    '/// Check if frame is still in the table
    if fCalculateTolerance( sPositionX1, sPositionX2) > 0.3 then
        Warnlog "Something is wrong ! Perhaps the frame isn't in the table!"
    end if
    
    'Set focus to frame
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'if the frame is in the table, the focus should be in the table
    Call wTypeKeys "<ESCape>"
    Sleep 1
    
    try
        Call fFormatTable("TabTabelle")
        TabTabelle.Cancel
    catch
        Warnlog "The frame isn't in the table!"
    endcatch
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_64

    Dim sTestFile     as String
    Dim iHorizontal   as Integer
    Dim sHorizontalBy as String
    Dim sPositionX1   as String
    Dim sPositionX2   as String
    
    sTestFile      = Convertpath (gTesttoolpath + "writer\optional\input\textframe\frameInTable.sxw")
    iHorizontal    = 4 'From Left
    sHorizontalBy  = "-50"
    
    printlog "- Exceeding layout environment at the left - 'Follow text flow'(OFF , table)"
    '/// Exceeding layout environment at the left - 'Follow text flow'(OFF , table)
    
    Call hNewDocument
    
    '/// Open a test file , which includes a frame in table and
    '/// + a picture which is anchored to frame
    hFileOpenLocally( sTestFile )
    
    
    'Set focus to the picture
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
	Call wTypeKeys ( "<Tab>" )
    
    'Get the pic Y position
    Sleep 1
    Call fFormatGraphic("TabType")
    sPositionX1 = Horizontalby.Gettext
    TabType.Cancel
    
    FormatAnchorToFrame
    Sleep 1
    
    '/// Set focus to the frame
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + UnCheck box 'Follow text flow'
    '/// + set a much bigger values in from left
    Call fFormatFrame("TabType")
    FollowTextflow.UnCheck
    Horizontal.Select iHorizontal
    Sleep 1
    HorizontalBy.SetText sHorizontalBy
    Sleep 1
    TabType.OK
    
    'Set focus to picture
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic X position
    Sleep 1
    Call fFormatGraphic("TabType")
    sPositionX2 = Horizontalby.Gettext
    TabType.Cancel
    
    '/// Check if frame is still in the table
    if fCalculateTolerance( sPositionX1, sPositionX2) <> 0 then
        Warnlog "Something is wrong ! Perhaps the frame is still in the table!"
    end if
    
    'Set focus to frame
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'if the frame is in the table, the focus should be in the table
    Call wTypeKeys "<ESCape>"
    Sleep 1
    
    try
        Call fFormatTable("TabTabelle")
        TabTabelle.Cancel
        Warnlog "The frame shouldn't in the table!"
    catch
    endcatch
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_65

	Dim sTestFile     as String
    Dim iHorizontal   as Integer
    Dim sHorizontalBy as String
    Dim sPositionX1   as String
    Dim sPositionX2   as String
    
    sTestFile      = Convertpath (gTesttoolpath + "writer\optional\input\textframe\frameInFrame.sxw")
    iHorizontal    = 4 'From Left
    sHorizontalBy  = "-50"
    
    printlog "- Exceeding layout environment at the left - 'Follow text flow'(ON , Frame)"
    '/// Exceeding layout environment at the left - 'Follow text flow'(ON , Frame)
    
    Call hNewDocument
    
    '/// Open a test file , which includes 2 frames ,
    '/// Frame A is anchored Frame B
    '/// + a picture which is anchored to Frame A
    hFileOpenLocally( sTestFile )
    
    
    'Set focus to the picture
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic X position
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionX1 = Horizontalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
    FormatAnchorToFrame
    Sleep 1
    
    '/// Set focus to the frame
    Call wTypeKeys "<Tab>" , 2
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + Check box 'Follow text flow'
    '/// + set a much bigger values in from left
    Call fFormatFrame("TabType")
    FollowTextflow.Check
    Horizontal.Select iHorizontal
    Sleep 1
    HorizontalBy.SetText sHorizontalBy
    Sleep 1
    TabType.OK
    
    'Set focus to picture
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic X position
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionX2 = Horizontalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
    '/// Check if frame is still in the frame
    if fCalculateTolerance( sPositionX1, sPositionX2) > 0.5 then
        Warnlog "Something is wrong ! Perhaps the frame A isn't in frame B!"
    end if
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_66

    Dim sTestFile     as String
    Dim iHorizontal   as Integer
    Dim sHorizontalBy as String
    Dim sPositionX1   as String
    Dim sPositionX2   as String
    
    sTestFile      = Convertpath (gTesttoolpath + "writer\optional\input\textframe\frameInFrame.sxw")
    iHorizontal    = 4 'From Left
    sHorizontalBy  = "-50"
    
    printlog "- Exceeding layout environment at the left - 'Follow text flow'(OFF , Frame)"
    '/// Exceeding layout environment at the left - 'Follow text flow'(OFF , Frame)
    
    Call hNewDocument
    
    '/// Open a test file , which includes 2 frames ,
    '/// Frame A is anchored Frame B
    '/// + a picture which is anchored to Frame A
    hFileOpenLocally( sTestFile )
    
    
    'Set focus to the picture
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic X position
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionX1 = Horizontalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
    FormatAnchorToFrame
    Sleep 1
    
    '/// Set focus to the frame
    Call wTypeKeys "<Tab>" , 2
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + UnCheck box 'Follow text flow'
    '/// + set a much bigger values in from left
    Call fFormatFrame("TabType")
    FollowTextflow.UnCheck
    Horizontal.Select iHorizontal
    Sleep 1
    HorizontalBy.SetText sHorizontalBy
    Sleep 1
    TabType.OK
    
    'Set focus to picture
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic X position
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionX2 = Horizontalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
    '/// Check if frame is still in the frame
    if fCalculateTolerance( sPositionX1, sPositionX2) > 0.5 then
        Warnlog "Something is wrong ! Perhaps the frame A isn't in frame B!"
    end if
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_67

    Dim sTestFile     as String
    Dim iHorizontal   as Integer
    Dim sHorizontalBy as String
    Dim sPositionX1   as String
    Dim sPositionX2   as String
    
    sTestFile      = Convertpath (gTesttoolpath + "writer\optional\input\textframe\frameInTable.sxw")
    iHorizontal    = 4 'From Left
    sHorizontalBy  = "50"
    
    printlog "- Exceeding layout environment at the right - 'Follow text flow'(ON , table)"
    '/// Exceeding layout environment at the right - 'Follow text flow'(ON , table)
    
    Call hNewDocument
    
    '/// Open a test file , which includes a frame in table and
    '/// + a picture which is anchored to frame
    hFileOpenLocally( sTestFile )
    
    
    'Set focus to the picture
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
	Call wTypeKeys ( "<Tab>" )
    
    'Get the pic X position
    Sleep 1
    Call fFormatGraphic("TabType")
    sPositionX1 = Horizontalby.Gettext
    TabType.Cancel
    
    FormatAnchorToFrame
    Sleep 1
    
    '/// Set focus to the frame
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + Check box 'Follow text flow'
    '/// + set a much bigger values in from left
    Call fFormatFrame("TabType")
    FollowTextflow.Check
    Horizontal.Select iHorizontal
    Sleep 1
    HorizontalBy.SetText sHorizontalBy
    Sleep 1
    TabType.OK
    
    'Set focus to picture
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic X position
    Sleep 1
    Call fFormatGraphic("TabType")
    sPositionX2 = Horizontalby.Gettext
    TabType.Cancel
    
    '/// Check if frame is still in the table
    if fCalculateTolerance( sPositionX1, sPositionX2) > 1 then
        Warnlog "Something is wrong ! Perhaps the frame isn't in the table!"
    end if
    
    'Set focus to frame
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'if the frame is in the table, the focus should be in the table
    Call wTypeKeys "<ESCape>"
    Sleep 1
    
    try
        Call fFormatTable("TabTabelle")
        TabTabelle.Cancel
    catch
        Warnlog "The frame isn't in the table!"
    endcatch
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_68

    Dim sTestFile     as String
    Dim iHorizontal   as Integer
    Dim sHorizontalBy as String
    Dim sPositionX1   as String
    Dim sPositionX2   as String
    
    sTestFile      = Convertpath (gTesttoolpath + "writer\optional\input\textframe\frameInTable.sxw")
    iHorizontal    = 4 'From Left
    sHorizontalBy  = "50"
    
    printlog "- Exceeding layout environment at the right - 'Follow text flow'(OFF , table)"
    '/// Exceeding layout environment at the right - 'Follow text flow'(OFF , table)
    
    Call hNewDocument
    
    '/// Open a test file , which includes a frame in table and
    '/// + a picture which is anchored to frame
    hFileOpenLocally( sTestFile )
    
    
    'Set focus to the picture
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
	Call wTypeKeys ( "<Tab>" )
    
    'Get the pic Y position
    Sleep 1
    Call fFormatGraphic("TabType")
    sPositionX1 = Horizontalby.Gettext
    TabType.Cancel
    
    FormatAnchorToFrame
    Sleep 1
    
    '/// Set focus to the frame
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + UnCheck box 'Follow text flow'
    '/// + set a much bigger values in from left
    Call fFormatFrame("TabType")
    FollowTextflow.UnCheck
    Horizontal.Select iHorizontal
    Sleep 1
    HorizontalBy.SetText sHorizontalBy
    Sleep 1
    TabType.OK
    
    'Set focus to picture
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic X position
    Sleep 1
    Call fFormatGraphic("TabType")
    sPositionX2 = Horizontalby.Gettext
    TabType.Cancel
    
    '/// Check if frame is still in the table
    if fCalculateTolerance( sPositionX1, sPositionX2) <> 0 then
        Warnlog "Something is wrong ! Perhaps the frame is still in the table!"
    end if
    
    'Set focus to frame
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'if the frame is in the table, the focus should be in the table
    Call wTypeKeys "<ESCape>"
    Sleep 1
    
    try
        Call fFormatTable("TabTabelle")
        TabTabelle.Cancel
        Warnlog "The frame shouldn't in the table!"
    catch
    endcatch
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_69

    Dim sTestFile     as String
    Dim iHorizontal   as Integer
    Dim sHorizontalBy as String
    Dim sPositionX1   as String
    Dim sPositionX2   as String
    
    sTestFile      = Convertpath (gTesttoolpath + "writer\optional\input\textframe\frameInFrame.sxw")
    iHorizontal    = 4 'From Left
    sHorizontalBy  = "50"
    
    printlog "- Exceeding layout environment at the right - 'Follow text flow'(ON , Frame)"
    '/// Exceeding layout environment at the right - 'Follow text flow'(ON , Frame)
    
    Call hNewDocument
    
    '/// Open a test file , which includes 2 frames ,
    '/// Frame A is anchored Frame B
    '/// + a picture which is anchored to Frame A
    hFileOpenLocally( sTestFile )
    
    
    'Set focus to the picture
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic X position
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionX1 = Horizontalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
    FormatAnchorToFrame
    Sleep 1
    
    '/// Set focus to the frame
    Call wTypeKeys "<Tab>" , 2
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + Check box 'Follow text flow'
    '/// + set a much bigger values in from left
    Call fFormatFrame("TabType")
    FollowTextflow.Check
    Horizontal.Select iHorizontal
    Sleep 1
    HorizontalBy.SetText sHorizontalBy
    Sleep 1
    TabType.OK
    
    'Set focus to picture
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic X position
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionX2 = Horizontalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
    '/// Check if frame is still in the frame
    if fCalculateTolerance( sPositionX1, sPositionX2) > 0.5 then
        Warnlog "Something is wrong ! Perhaps the frame A isn't in frame B!"
    end if
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------

testcase tTextframes_70

    Dim sTestFile     as String
    Dim iHorizontal   as Integer
    Dim sHorizontalBy as String
    Dim sPositionX1   as String
    Dim sPositionX2   as String
    
    sTestFile      = Convertpath (gTesttoolpath + "writer\optional\input\textframe\frameInFrame.sxw")
    iHorizontal    = 4 'From Left
    sHorizontalBy  = "50"
    
    printlog "- Exceeding layout environment at the left - 'Follow text flow'(OFF , Frame)"
    '/// Exceeding layout environment at the left - 'Follow text flow'(OFF , Frame)
    
    Call hNewDocument
    
    '/// Open a test file , which includes 2 frames ,
    '/// Frame A is anchored Frame B
    '/// + a picture which is anchored to Frame A
    hFileOpenLocally( sTestFile )
    
    
    'Set focus to the picture
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic X position
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionX1 = Horizontalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
    FormatAnchorToFrame
    Sleep 1
    
    '/// Set focus to the frame
    Call wTypeKeys "<Tab>" , 2
    Sleep 1
    
    '/// Format/Frame / Type ,
    '/// + UnCheck box 'Follow text flow'
    '/// + set a much bigger values in from left
    Call fFormatFrame("TabType")
    FollowTextflow.UnCheck
    Horizontal.Select iHorizontal
    Sleep 1
    HorizontalBy.SetText sHorizontalBy
    Sleep 1
    TabType.OK
    
    'Set focus to picture
    Call wTypeKeys "<Tab>"
    Sleep 1
    
    'Get the pic X position
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionX2 = Horizontalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
    '/// Check if frame is still in the table
    if fCalculateTolerance( sPositionX1, sPositionX2) < 3 then
        Warnlog "Something is wrong ! Perhaps the frame A isn't in frame B!"
    end if
    
    printlog " Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-----------------------------------------------------------------
