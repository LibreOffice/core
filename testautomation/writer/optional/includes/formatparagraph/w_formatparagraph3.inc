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
'* short description : Test the functionality of Paragraph Formatting - 3
'*
'************************************************************************
'*
' #1 tParagraphTabs1         'Test Tab stops - Left
' #1 tParagraphTabs2         'Test Tab stops - Right
' #1 tParagraphTabs3         'Test Tab stops - Centered
' #1 tParagraphTabs4         'Test Tab stops - Decimal
' #1 tParagraphTabs5         'Delete a selected tab stop
' #1 tParagraphTabs6         'Delete all tab stops
' #1 tParagraphDropCaps1     'Test Drop Caps - Default
' #1 tParagraphDropCaps2     'Test Drop Caps - select a word
' #1 tParagraphDropCaps3     'Test Drop Caps - select different Number of characters
' #1 tParagraphDropCaps4     'Test Drop Caps - select different number of Lines
' #1 tParagraphDropCaps5     'Test Drop Caps - Set different 'Space to text'
' #1 tParagraphDropCaps6     'Test max 'Space to text'
' #1 tParagraphDropCaps7     'Test text in Contents "Text" field
' #1 tParagraphDropCaps8     'Test 'Character Style' with given template
' #1 tParagraphDropCaps9     'Test 'Character Style' with user-defined template
' #1 tParagraphNumbering1    'Test Numbering
' #1 tParagraphBorder1       'Test Default borders
' #1 tParagraphBorder2       'All 'spacing to contents' spin boxes are limited to 5cm
' #1 tParagraphBorder3       'Test Uncheck 'Synchronize'
' #1 tParagraphBorder4       'Test check 'Synchronize'
' #1 tParagraphBorder5       'Test default shadow color
' #1 tParagraphBorder6       'Test shadow setting
'*
'\***********************************************************************

testcase tParagraphTabs1

  Dim sTestFile as String
  Dim sTabs      as String
  Dim sPosition as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\TabsTest.sxw")
  sTabs     = "3" + gSeperator + "00" + gMeasurementUnit
  sPosition = "5" + gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test Tab stops - Left"
  '/// Test Tab stops - Left

   Call hNewDocument

   '/// Open the test file
   '/// + There is 1 pic which is anchored as character in the file
    hFileOpenLocally( sTestFile )
    

   '/// Format/Paragraph / Tabs ,
   '/// + check Left in the type area , and input 3cm in Position
    Call fFormatParagraph("TabTabulatoren")
        TypLinks.Check
        Position.SetText sTabs
    TabTabulatoren.OK

    Call wTypeKeys "<MOD1 Home>"

   '/// Press TAB
    Call wTypeKeys "<TAB>"

   '/// Check if TAB is configurated correctly
    Call wTypeKeys ( "<Shift F4>" )
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        if fCalculateTolerance(Horizontalby.Gettext , sPosition) > 0.02 then Warnlog "Tab type (left) doesn't work !"
    TabPositionAndSizeWriter.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphTabs2

  Dim sTestFile  as String
  Dim sTabs       as String
  Dim sPosition  as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\TabsTest.sxw")
  sTabs     = "3" + gSeperator + "00" + gMeasurementUnit
  sPosition = "4" + gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test Tab stops - Right"
  '/// Test Tab stops - Right

   Call hNewDocument

   '/// Open the test file
   '/// + There is 1 pic which is anchored as character in the file
    hFileOpenLocally( sTestFile )
    

   '/// Format/Paragraph / Tabs ,
   '/// + check Right in the type area , and input 3cm in Position
    Call fFormatParagraph("TabTabulatoren")
        TypRechts.Check
        Position.SetText sTabs
    TabTabulatoren.OK

    Call wTypeKeys "<MOD1 Home>"

   '/// Press TAB
    Call wTypeKeys "<TAB>"

   '/// Check if TAB is configurated correctly
    Call wTypeKeys ( "<Shift F4>" )
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        if fCalculateTolerance(Horizontalby.Gettext , sPosition) > 0.02 then Warnlog "Tab type (Right) doesn't work !"
    TabPositionAndSizeWriter.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphTabs3

  Dim sTestFile    as String
  Dim sTabs        as String
  Dim sPosition    as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\TabsTest.sxw")
  sTabs     = "3" + gSeperator + "00" + gMeasurementUnit
  sPosition = "4" + gSeperator + "50" + gMeasurementUnit

  PrintLog "- Test Tab stops - Centered"
  '/// Test Tab stops - Centered

   Call hNewDocument

   '/// Open the test file
   '/// + There is 1 pic which is anchored as character in the file
    hFileOpenLocally( sTestFile )
    

   '/// Format/Paragraph / Tabs ,
   '/// + check Centered in the type area , and input 3cm in Position
    Call fFormatParagraph("TabTabulatoren")
        TypZentriert.Check
        Position.SetText sTabs
    TabTabulatoren.OK

    Call wTypeKeys "<MOD1 Home>"

   '/// Press TAB
    Call wTypeKeys "<TAB>"

   '/// Check if TAB is configurated correctly
    Call wTypeKeys ( "<Shift F4>" )
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        if fCalculateTolerance(Horizontalby.Gettext , sPosition) > 0.02 then Warnlog "Tab type (Centered) doesn't work !"
    TabPositionAndSizeWriter.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphTabs4

  Dim sTestFile    as String
  Dim sTabs        as String
  Dim sPosition    as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\TabsDecimal.sxw")
  sTabs     = "5" + gSeperator + "00" + gMeasurementUnit
  sPosition = "7" + gSeperator + "11" + gMeasurementUnit

  PrintLog "- Test Tab stops - Decimal"
  '/// Test Tab stops - Decimal

   Call hNewDocument

   '/// Open the test file
   '/// + There is 1 pic which is anchored as character in the file ,
   '/// + and there are 1 "." seperator in front of the pic
    hFileOpenLocally( sTestFile )
    

   '/// Format/Paragraph / Tabs ,
   '/// + check Decimal in the type area ,
   '/// + input "." in 'character' and input 5cm in 'Position'
    Call fFormatParagraph("TabTabulatoren")
        TypDezimal.Check
        Sleep 1
        TypZeichen.SetText "."   'I use seperator as '.' in this case
        Position.SetText sTabs
    TabTabulatoren.OK

    Call wTypeKeys "<MOD1 Home>"

   '/// Press TAB
    Call wTypeKeys "<TAB>"

   '/// Check if TAB is configurated correctly
   '/// + [Check . should be at distance 5.00cm]
    Call wTypeKeys ( "<Shift F4>" )
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        if fCalculateTolerance(Horizontalby.Gettext , sPosition) > 0.03 then Warnlog "Tab type (Decimal) doesn't work !"
    TabPositionAndSizeWriter.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphTabs5

  Dim sTab1 , sTab2 , sTab3   as String

  sTab1 = "1" + gSeperator + "00" + gMeasurementUnit
  sTab2 = "2" + gSeperator + "00" + gMeasurementUnit
  sTab3 = "3" + gSeperator + "00" + gMeasurementUnit

  PrintLog "- Delete a selected tab stop"
  '/// Delete a selected tab stop

   Call hNewDocument

   '/// Format/Paragraph / Tabs ,
   '/// + insert 3 new tab stops
    Call fFormatParagraph("TabTabulatoren")
        Position.SetText sTab1
        Neu.Click
        Sleep 1
        Position.SetText sTab2
        Neu.Click
        Sleep 1
        Position.SetText sTab3
        Neu.Click
        Sleep 1
    TabTabulatoren.OK

   '/// Delete the selected tab stop
   '/// + Select 2.00cm , and press delete key
    Call fFormatParagraph("TabTabulatoren")
        Position.Select 2
        Sleep 2
        Loeschen.Click
        Sleep 2
        Position.Select 1
        if Position.GetSelText   <>  sTab1 then warnlog "should get " &sTab1 & " but get " &Position.GetSelText
        Position.Select 2
        if Position.GetSelText   <>  sTab3 then warnlog "should get " &sTab3 & " but get " &Position.GetSelText

        if Position.GetItemCount <>  2     then warnlog "should only be 2 items in the listbox !"
    TabTabulatoren.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphTabs6

  Dim sTab1 , sTab2 , sTab3   as String

  sTab1 = "1" + gSeperator + "00" + gMeasurementUnit
  sTab2 = "2" + gSeperator + "00" + gMeasurementUnit
  sTab3 = "3" + gSeperator + "00" + gMeasurementUnit

  PrintLog "- Delete all tab stops"
  '/// Delete all tab stops

   Call hNewDocument

   '/// Format/Paragraph / Tabs ,
   '/// + insert 3 new tab stops
    Call fFormatParagraph("TabTabulatoren")
        Position.SetText sTab1
        Neu.Click
        Sleep 1
        Position.SetText sTab2
        Neu.Click
        Sleep 1
        Position.SetText sTab3
        Neu.Click
        Sleep 1
    TabTabulatoren.OK

   '/// Delete all tab stops
   '/// + Press 'Delete All' Key
    Call fFormatParagraph("TabTabulatoren")
        AlleLoeschen.Click
        Sleep 2
        if Position.GetItemCount <>  0  then warnlog "should be no items in the listbox !"
    TabTabulatoren.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphDropCaps1

  Dim sTestFile     as String
  Dim sPositionX    as String
  Dim sPositionY    as String

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\DropCapsTest.sxw")
  sPositionX = "4" + gSeperator + "00" + gMeasurementUnit
  sPositionY = "3" + gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test Drop Caps - Default"
  '/// Test Drop Caps - Default

   Call hNewDocument

   '/// Open the test file
   '/// + There is 1 pic which is anchored as character in the file
    hFileOpenLocally( sTestFile )
    

   '/// Format/Paragraph / Drop Caps ,
   '/// + Check 'Display Drop Caps' , close the dialog
    Call fFormatParagraph("TabInitialen")
        Anzeigen.Check
        Sleep 1
        if Initialentext.GetText <> "H" then
            warnlog "The text is wrong : should be H , but get " &Initialentext.GetText
            TabInitialen.OK
            goto endsub
        end if
        if Zeilen.GetText <> "3" then
            warnlog "The default number should be 3 lines , but get " &Zeilen.GetText
            TabInitialen.OK
            goto endsub
        end if
    TabInitialen.OK

   '/// Set focus to the pic , check if its position is changed correctly
    Call wTypeKeys ( "<Shift F4>" )
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        if fCalculateTolerance(Horizontalby.Gettext , sPositionX) > 0.4 then Warnlog "X position isn't right !"
        if fCalculateTolerance(Verticalby.Gettext , sPositionY) > 0.4 then Warnlog "Y position isn't right !"
    TabPositionAndSizeWriter.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphDropCaps2

  Dim sTestFile     as String
  Dim sPositionX    as String
  Dim sPositionY    as String

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\DropCapsTest.sxw")
  sPositionX = "6" + gSeperator + "00" + gMeasurementUnit
  sPositionY = "3" + gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test Drop Caps - select a word"
  '/// Test Drop Caps - select a word

   Call hNewDocument

   '/// Open the test file
   '/// + There is 1 pic which is anchored as character in the file
    hFileOpenLocally( sTestFile )
    

   '/// Format/Paragraph / Drop Caps ,
   '/// + Check 'Display Drop Caps' , then check 'whole word' , then close the dialog
    Call fFormatParagraph("TabInitialen")
        Anzeigen.Check
        Sleep 1
        GanzesWort.Check
        Sleep 1
        if Initialentext.GetText <> "He" then
            warnlog "The text is wrong : should be He , but get " &Initialentext.GetText
            TabInitialen.OK
            goto endsub
        end if
        if Zeilen.GetText <> "3" then
            warnlog "The default number should be 3 lines , but get " &Zeilen.GetText
            TabInitialen.OK
            goto endsub
        end if
    TabInitialen.OK

   '/// Set focus to the pic , check if its position is changed correctly
    Call wTypeKeys ( "<Shift F4>" )
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        if fCalculateTolerance(Horizontalby.Gettext , sPositionX) > 0.4 then Warnlog "X position isn't right !"
        if fCalculateTolerance(Verticalby.Gettext , sPositionY) > 0.4 then Warnlog "Y position isn't right !"
    TabPositionAndSizeWriter.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphDropCaps3

  Dim sTestFile     as String
  Dim sPositionX    as String
  Dim sPositionY    as String

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\DropCapsTest.sxw")
  sPositionX = "4" + gSeperator + "50" + gMeasurementUnit
  sPositionY = "4" + gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test Drop Caps - select different Number of characters"
  '/// Test Drop Caps - select different Number of characters

   Call hNewDocument

   '/// Open the test file
   '/// + There is 1 pic which is anchored as character in the file
    hFileOpenLocally( sTestFile )
    

   '/// Format/Paragraph / Drop Caps ,
   '/// + Check 'Display Drop Caps' , then set '6' in 'number of characters' ,then close the dialog
    Call fFormatParagraph("TabInitialen")
        Anzeigen.Check
        Sleep 1
        Anzahl.SetText "6"
        Sleep 1
        if Initialentext.GetText <> "He hea" then
            warnlog "The text is wrong : should be 'He hea' , but get " &Initialentext.GetText
            TabInitialen.OK
            goto endsub
        end if
        if Zeilen.GetText <> "3" then
            warnlog "The default number should be 3 lines , but get " &Zeilen.GetText
            TabInitialen.OK
            goto endsub
        end if
    TabInitialen.OK

   '/// Set focus to the pic , check if its position is changed correctly
    Call wTypeKeys ( "<Shift F4>" )
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        if fCalculateTolerance(Horizontalby.Gettext , sPositionX) > 0.4 then Warnlog "X position isn't right !"
        if fCalculateTolerance(Verticalby.Gettext , sPositionY) > 0.4 then Warnlog "Y position isn't right !"
    TabPositionAndSizeWriter.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphDropCaps4

  Dim sTestFile     as String
  Dim sPositionX    as String
  Dim sPositionY    as String

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\DropCapsTest.sxw")
  sPositionX = "7" + gSeperator + "50" + gMeasurementUnit
  sPositionY = "2" + gSeperator + "80" + gMeasurementUnit

  PrintLog "- Test Drop Caps - select different number of Lines"
  '/// Test Drop Caps - select different number of Lines

   Call hNewDocument

   '/// Open the test file
   '/// + There is 1 pic which is anchored as character in the file
    hFileOpenLocally( sTestFile )
    

   '/// Format/Paragraph / Drop Caps ,
   '/// + Check 'Display Drop Caps' , then set '6' in 'Lines' ,then close the dialog
    Call fFormatParagraph("TabInitialen")
        Anzeigen.Check
        Sleep 1
        Zeilen.SetText "6"
        Sleep 1
        if Initialentext.GetText <> "H" then
            warnlog "The text is wrong : should be 'H' , but get " &Initialentext.GetText
            TabInitialen.OK
            goto endsub
        end if
    TabInitialen.OK

   '/// Set focus to the pic , check if its position is changed correctly
    Call wTypeKeys ( "<Shift F4>" )
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        if fCalculateTolerance(Horizontalby.Gettext , sPositionX) > 0.5 then Warnlog "X position isn't right !"
        if fCalculateTolerance(Verticalby.Gettext , sPositionY) > 0.5 then Warnlog "Y position isn't right !"
    TabPositionAndSizeWriter.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphDropCaps5

  Dim sTestFile     as String
  Dim sSpace        as String
  Dim sPositionX    as String
  Dim sPositionY    as String

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\DropCapsTest.sxw")
  sSpace     = "1" + gSeperator + "00" + gMeasurementUnit
  sPositionX = "6" + gSeperator + "00" + gMeasurementUnit
  sPositionY = "3" + gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test Drop Caps - Set different 'Space to text'"
  '/// Test Drop Caps - Set different 'Space to text'

   Call hNewDocument

   '/// Open the test file
   '/// + There is 1 pic which is anchored as character in the file
    hFileOpenLocally( sTestFile )
    

   '/// Format/Paragraph / Drop Caps ,
   '/// +  Check 'Display Drop Caps' ,then set 'Space to text' to be 1cm , close the dialog
    Call fFormatParagraph("TabInitialen")
        Anzeigen.Check
        Sleep 1
        AbstandZumText.SetText sSpace
        if Initialentext.GetText <> "H" then
            warnlog "The text is wrong : should be H , but get " &Initialentext.GetText
            TabInitialen.OK
            goto endsub
        end if
        if Zeilen.GetText <> "3" then
            warnlog "The default number should be 3 lines , but get " &Zeilen.GetText
            TabInitialen.OK
            goto endsub
        end if
    TabInitialen.OK

   '/// Set focus to the pic , check if its position is changed correctly
    Call wTypeKeys ( "<Shift F4>" )
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        if fCalculateTolerance(Horizontalby.Gettext , sPositionX) > 0.4 then Warnlog "X position isn't right !"
        if fCalculateTolerance(Verticalby.Gettext , sPositionY) > 0.4 then Warnlog "Y position isn't right !"
    TabPositionAndSizeWriter.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphDropCaps6

  Dim sSpace  , sMaxSpace   as String

  sSpace  = "3" + gSeperator + "00" + gMeasurementUnit
  sSpace  = "2" + gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test max 'Space to text'"
  '/// Test max 'Space to text'

   Call hNewDocument

   '/// Format/Paragraph / Drop Caps ,
   '/// + Check 'Display Drop Caps' ,then set 'Space to text' to be 3cm , close the dialog
    Call fFormatParagraph("TabInitialen")
        Anzeigen.Check
        Sleep 1
        AbstandZumText.SetText sSpace
        Sleep 1
    TabInitialen.OK

   '/// Reopen Format/Paragraph / Drop Caps ,
   '/// + 'Space to text' should be 2cm
    Call fFormatParagraph("TabInitialen")
        if AbstandZumText.GetText <> sSpace then Warnlog "Max 'Space to text' should be 2cm!"
    TabInitialen.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphDropCaps7

  Dim sTestFile     as String
  Dim sTestText     as String
  Dim sHopeResult   as String

  sTestFile   = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\DropCapsTest.sxw")
  sTestText   = "Xyz"
  sHopeResult = "Xyzhe"

  PrintLog "- Test text in Contents 'Text' field"
  '/// Test text in Contents "Text" field

   Call hNewDocument

   '/// Open the test file
   '/// + There is 1 pic which is anchored as character in the file
    hFileOpenLocally( sTestFile )
    

   '/// Format/Paragraph / Drop Caps ,
   '/// + Check 'Display Drop Caps' ,then set Xyz in Contents 'Text' field , close the dialog
    Call fFormatParagraph("TabInitialen")
        Anzeigen.Check
        Sleep 1
        Initialentext.SetText sTestText
        Sleep 1
    TabInitialen.OK

   '/// Check if the head of the artile is changed to be "Xyzhe"
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift Right>" , 5

    EditCopy
    if GetClipboardText <> sHopeResult then
        warnlog "Get the wrong result : " &  GetClipboardText
    end if

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphDropCaps8

  Dim iCharacterStyle  as Integer

  iCharacterStyle = 3

  PrintLog "- Test 'Character Style' with given template"
  '/// Test 'Character Style' with given template

   Call hNewDocument

   '/// Format/Paragraph / Drop Caps ,
   '/// + Check 'Display Drop Caps' ,then select 3rd in Contents 'Character Style' field , close the dialog
    Call fFormatParagraph("TabInitialen")
        Anzeigen.Check
        Sleep 1
        Zeichenvorlage.Select  iCharacterStyle
        Sleep 1
    TabInitialen.OK

   '/// Check if the set is OK
    Call fFormatParagraph("TabInitialen")
        if Zeichenvorlage.GetSelIndex <> iCharacterStyle then
            warnlog "Get the wrong result : " & iCharacterStyle
        end if
    TabInitialen.OK

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphDropCaps9

    Dim sCharacterStyle  as String
    Dim i                as Integer

    sCharacterStyle = "aNewTestStyle"

    PrintLog "- Test 'Character Style' with user-defined template"
    '/// Test 'Character Style' with user-defined template

    Call hNewDocument

    '/// Make a user-defined template
    '/// Stylist , select 'Character Styles'
    '/// Press New button to add a new character style
    Call wStyleCreate ( sCharacterStyle, "Character" )

    '/// Format/Paragraph / Drop Caps ,
    '/// + Check 'Display Drop Caps' ,then select the character style which you just inserted
    '/// + in Contents 'Character Style' field , close the dialog
    Call fFormatParagraph("TabInitialen")
    Anzeigen.Check
    Sleep 1
    Zeichenvorlage.Select  sCharacterStyle
    Sleep 1
    TabInitialen.OK

   '/// Check if the set is OK
    Call fFormatParagraph("TabInitialen")
    if Zeichenvorlage.GetSelText <> sCharacterStyle then
        Warnlog "Get the wrong result : " & Zeichenvorlage.GetSelText
    end if
    TabInitialen.Cancel

    '/// Delete Style
    Call wStyleDelete( sCharacterStyle, "Character" )    

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphNumbering1

  Dim sTestFile                 as String
  Dim sTest1 , sTest2           as String
  Dim iNumberingStyle1          as Integer
  Dim sNumberingStartWith1      as String
  Dim sLineNumberingStartWith1  as String
  Dim iNumberingStyle2          as Integer
  Dim sNumberingStartWith2      as String
  Dim sLineNumberingStartWith2  as String

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\paragraphNumbering.sxw")
  sTest1                    = "He1"
  iNumberingStyle1          = 3
  sNumberingStartWith1      = "2"
  sLineNumberingStartWith1  = "3"
  sTest2                    = "He2"
  iNumberingStyle2          = 8
  sNumberingStartWith2      = "4"
  sLineNumberingStartWith2  = "5"

  PrintLog "- Test Numbering"
  '/// Test Numbering

   Call hNewDocument

   '/// Open the test file ,
   '/// + there are 2 paragraphes in the file
    hFileOpenLocally( sTestFile )
    

   '/// Find the test word , set the focus to 1st paragraph
    Call fFindWord( sTest1 )

   '/// Format/Paragraph / Numbering , select 3rd in Numbering Style ,
   '/// + Check "Restart at this paragraph" ,
   '/// + check "Start with" in Numbering area , input 2
   '/// + check "include this paragraph in line numbering" ,
   '/// + check "Restart at this paragraph" in Line numbering area ,
   '/// + check "Start with" in Line Numbering area , input 3
    Call fFormatParagraph("TabNumerierungAbsatz")
        Vorlage.Select iNumberingStyle1
        Sleep 1
        NumerierungNeuBeginnen.Check
        Sleep 1
        StartWith.Check
        Sleep 1
        NumerierungBeginnenBei.SetText sNumberingStartWith1
        ZeilenumerierungZeilenMitzaehlen.Check
        Sleep 1
        ZeilenumerierungNeuBeginnen.Check
        Sleep 1
        ZeilenumerierungBeginnenBei.SetText sLineNumberingStartWith1
    TabNumerierungAbsatz.OK

   '/// Find the test word , set the focus to 2nd paragraph
    Call fFindWord( sTest2 )

   '/// Format/Paragraph / Numbering , select 8th in Numbering Style ,
   '/// + Check "Restart at this paragraph" ,
   '/// + check "Start with" in Numbering area , input 4
   '/// + check "include this paragraph in line numbering" ,
   '/// + check "Restart at this paragraph" in Line numbering area ,
   '/// + check "Start with" in Line Numbering area , input 5
    Call fFormatParagraph("TabNumerierungAbsatz")
        Vorlage.Select iNumberingStyle2
        Sleep 1
        NumerierungNeuBeginnen.Check
        Sleep 1
        StartWith.Check
        Sleep 1
        NumerierungBeginnenBei.SetText sNumberingStartWith2
        ZeilenumerierungZeilenMitzaehlen.Check
        Sleep 1
        ZeilenumerierungNeuBeginnen.Check
        Sleep 1
        ZeilenumerierungBeginnenBei.SetText sLineNumberingStartWith2
    TabNumerierungAbsatz.OK

   '/// Check if the configuration in paragraph1 is correct
    PrintLog "- Check if the configuration in paragraph1 is correct"
    Call fFindWord( sTest1 )

    Call fFormatParagraph("TabNumerierungAbsatz")
        if Vorlage.GetSelIndex                         <> iNumberingStyle1 then
            Warnlog "The numbering Style is wrong , get " & Vorlage.GetSelIndex
        end if
        if NumerierungNeuBeginnen.IsChecked            <> TRUE then
            Warnlog "'Restart at this paragraph' should be checked !"
        end if
        if StartWith.IsChecked                         <> TRUE then
            Warnlog "'Start with' in Numbering area should be checked !"
        end if
        if NumerierungBeginnenBei.GetText              <> sNumberingStartWith1 then
            Warnlog "'Start with' in Numbering area is wrong , get " & NumerierungBeginnenBei.GetText
        end if
        if ZeilenumerierungZeilenMitzaehlen.IsChecked  <> TRUE  then
            Warnlog "'include this paragraph in line numbering' should be checked!"
        end if
        if ZeilenumerierungNeuBeginnen.IsChecked       <> TRUE  then
            Warnlog "'Restart at this paragraph' in Line numbering area should be checked!"
        end if
        if ZeilenumerierungBeginnenBei.GetText         <> sLineNumberingStartWith1 then
            Warnlog "'Restart at this paragraph' in Line numbering area is wrong , get " & ZeilenumerierungBeginnenBei.GetText
        end if
    TabNumerierungAbsatz.Cancel

   '/// Check if the configuration in paragraph2 is correct
    PrintLog "- Check if the configuration in paragraph2 is correct"
    Call fFindWord( sTest2 )

    Call fFormatParagraph("TabNumerierungAbsatz")
        if Vorlage.GetSelIndex                         <> iNumberingStyle2 then
            Warnlog "The numbering Style is wrong , get " & Vorlage.GetSelIndex
        end if
        if NumerierungNeuBeginnen.IsChecked            <> TRUE then
            Warnlog "'Restart at this paragraph' should be checked !"
        end if
        if StartWith.IsChecked                         <> TRUE then
            Warnlog "'Start with' in Numbering area should be checked !"
        end if
        if NumerierungBeginnenBei.GetText              <> sNumberingStartWith2 then
            Warnlog "'Start with' in Numbering area is wrong , get " & NumerierungBeginnenBei.GetText
        end if
        if ZeilenumerierungZeilenMitzaehlen.IsChecked  <> TRUE  then
            Warnlog "'include this paragraph in line numbering' should be checked!"
        end if
        if ZeilenumerierungNeuBeginnen.IsChecked       <> TRUE  then
            Warnlog "'Restart at this paragraph' in Line numbering area should be checked!"
        end if
        if ZeilenumerierungBeginnenBei.GetText         <> sLineNumberingStartWith2 then
            Warnlog "'Restart at this paragraph' in Line numbering area is wrong , get " & ZeilenumerierungBeginnenBei.GetText
        end if
    TabNumerierungAbsatz.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphBorder1

    Dim sSpacingToContentsNoBorder   as String   'no border in the page
    Dim sSpacingToContentsYesBorder  as String   'border exists  in the page

    sSpacingToContentsNoBorder  = "0" + gSeperator + "00" + gMeasurementUnit
    sSpacingToContentsYesBorder = "0" + gSeperator + "05" + gMeasurementUnit

    PrintLog "Test Default borders"
    '/// <b> Test Default borders </b> ///

    PrintLog "- when no border is set, 'spacing to contents'"
    '/// 1. When no border is set ///

    Call hNewDocument

    '/// Check if "spacing to contents" is unable ///
    '/// format/Paragraph , tabpage borders
     Call fFormatParagraph("TabUmrandung")
         if Links.GetText  <> sSpacingToContentsNoBorder then
             Warnlog "Default of left Spacing to contents is wrong !"
         end if
         if Rechts.GetText <> sSpacingToContentsNoBorder then
             Warnlog "Default of right Spacing to contents is wrong !"
         end if
         if Oben.GetText   <> sSpacingToContentsNoBorder then
             Warnlog "Default of up Spacing to contents is wrong !"
         end if
         if Unten.GetText  <> sSpacingToContentsNoBorder then
             Warnlog "Default of bottom Spacing to contents is wrong !"
         end if
     TabUmrandung.Cancel

    PrintLog "Check if 'spacing to contents' is 0.05cm when there are borders in the paragraph"
    '/// 2. Check if "spacing to contents" is 0.05cm when there are borders in the paragraph ///
    '/// format/Paragraph , tabpage borders ///
    '/// + select 2nd in Default area ///
     Call fFormatParagraph("TabUmrandung")
         Vorgaben.TypeKeys "<Right>" , 2
         Sleep 2

         if Links.GetText  <> sSpacingToContentsYesBorder then
             Warnlog "Default of left Spacing to contents is wrong !"
         end if
         if Rechts.GetText <> sSpacingToContentsYesBorder then
             Warnlog "Default of right Spacing to contents is wrong !"
         end if
         if Oben.GetText   <> sSpacingToContentsYesBorder then
             Warnlog "Default of up Spacing to contents is wrong !"
         end if
         if Unten.GetText  <> sSpacingToContentsYesBorder then
             Warnlog "Default of bottom Spacing to contents is wrong !"
         end if
     TabUmrandung.Cancel

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphBorder2

    Dim sSpacingToContents   as String   'the max spacing to content
    Dim sSpacingToContentsL  as String
    Dim sSpacingToContentsR  as String
    Dim sSpacingToContentsU  as String
    Dim sSpacingToContentsB  as String

    sSpacingToContents  = "5" + gSeperator + "00" + gMeasurementUnit
    sSpacingToContentsL = "6" + gSeperator + "00" + gMeasurementUnit
    sSpacingToContentsR = "7" + gSeperator + "00" + gMeasurementUnit
    sSpacingToContentsU = "8" + gSeperator + "00" + gMeasurementUnit
    sSpacingToContentsB = "9" + gSeperator + "00" + gMeasurementUnit

    PrintLog "- All 'spacing to contents' spin boxes are limited to 5cm"
    '///<b> All "spacing to contents" spin boxes are limited to 5cm </b>///

    Call hNewDocument

    '/// format/Page , tabpage borders ,  ///
    '/// + set border exists in the paragraph ///
    '/// + set all spacing to content's size ///
    '/// + more than 5.00cm ///
     Call fFormatParagraph("TabUmrandung")
         Vorgaben.TypeKeys "<Right>" , 2
         Sleep 2
         Synchronisieren.Uncheck
         Sleep 1
         Links.SetText  sSpacingToContentsL
         Rechts.SetText sSpacingToContentsR
         Oben.SetText   sSpacingToContentsU
         Unten.SetText  sSpacingToContentsB
     TabUmrandung.OK

    '/// Check if all "spacing to contents" is 5.00cm ///
     Call fFormatParagraph("TabUmrandung")
         if Links.GetText  <> sSpacingToContents then
             Warnlog "left Spacing to contents is wrong !"
         end if
         if Rechts.GetText <> sSpacingToContents then
             Warnlog "right Spacing to contents is wrong !"
         end if
         if Oben.GetText   <> sSpacingToContents then
             Warnlog "up Spacing to contents is wrong !"
         end if
         if Unten.GetText  <> sSpacingToContents then
             Warnlog "bottom Spacing to contents is wrong !"
         end if
     TabUmrandung.Cancel

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphBorder3

    Dim sSpacingToContentsL  as String
    Dim sSpacingToContentsR  as String
    Dim sSpacingToContentsU  as String
    Dim sSpacingToContentsB  as String

    sSpacingToContentsL = "0" + gSeperator + "50" + gMeasurementUnit
    sSpacingToContentsR = "0" + gSeperator + "60" + gMeasurementUnit
    sSpacingToContentsU = "0" + gSeperator + "70" + gMeasurementUnit
    sSpacingToContentsB = "0" + gSeperator + "80" + gMeasurementUnit

    PrintLog "- Test Uncheck 'Synchronize'"
    '///<b> Test Uncheck 'Synchronize' </b>///

    Call hNewDocument

    '/// format/Page , tabpage borders ,  ///
    '/// + set border exists in the paragraph ///
    '/// + Uncheck synchronize ///
    '/// + Set all spacing to contents ///
     Call fFormatParagraph("TabUmrandung")
         Vorgaben.TypeKeys "<Right>" , 2
         Sleep 2
         Synchronisieren.Uncheck
         Sleep 1
         Links.SetText  sSpacingToContentsL
         Rechts.SetText sSpacingToContentsR
         Oben.SetText   sSpacingToContentsU
         Unten.SetText  sSpacingToContentsB
     TabUmrandung.OK

    '/// + Check if all "spacing to contents" is correct
     Call fFormatParagraph("TabUmrandung")
         if Links.GetText  <> sSpacingToContentsL then
             Warnlog "left Spacing to contents is wrong !"
         end if
         if Rechts.GetText <> sSpacingToContentsR then
             Warnlog "right Spacing to contents is wrong !"
         end if
         if Oben.GetText   <> sSpacingToContentsU then
             Warnlog "up Spacing to contents is wrong !"
         end if
         if Unten.GetText  <> sSpacingToContentsB then
             Warnlog "bottom Spacing to contents is wrong !"
         end if
     TabUmrandung.Cancel

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphBorder4

    Dim sSpacingToContentsL  as String
    Dim sSpacingToContentsR  as String
    Dim sSpacingToContentsU  as String
    Dim sSpacingToContentsB  as String

    sSpacingToContentsL = "0" + gSeperator + "50" + gMeasurementUnit
    sSpacingToContentsR = "0" + gSeperator + "60" + gMeasurementUnit
    sSpacingToContentsU = "0" + gSeperator + "70" + gMeasurementUnit
    sSpacingToContentsB = "0" + gSeperator + "80" + gMeasurementUnit

    PrintLog "- Test Check 'Synchronize'"
    '/// <b> Test Check 'Synchronize' </b> ///

    Call hNewDocument

    '/// format/Page , tabpage borders ,  ///
    '/// + set border exists in the paragraph ///
    '/// + check synchronize ///
    '/// + Set all spacing to contents ///
     Call fFormatParagraph("TabUmrandung")
         Vorgaben.TypeKeys "<Right>" , 2
         Sleep 2
         Synchronisieren.Check
         Sleep 1
         Links.SetText  sSpacingToContentsL
         Rechts.SetText sSpacingToContentsR
         Oben.SetText   sSpacingToContentsU
         Unten.SetText  sSpacingToContentsB
     TabUmrandung.OK

    '/// + Check if all "spacing to contents" is correct
     Call fFormatParagraph("TabUmrandung")
         if Links.GetText  <> sSpacingToContentsB then
             Warnlog "left Spacing to contents is wrong !"
         end if
         if Rechts.GetText <> sSpacingToContentsB then
             Warnlog "right Spacing to contents is wrong !"
         end if
         if Oben.GetText   <> sSpacingToContentsB then
             Warnlog "up Spacing to contents is wrong !"
         end if
         if Unten.GetText  <> sSpacingToContentsB then
             Warnlog "bottom Spacing to contents is wrong !"
         end if
     TabUmrandung.Cancel

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphBorder5

    Dim sDefaultShadowColor as String
    
    Select case iSprache
        case 01   : sDefaultShadowColor = "Gray"
        case 31:    sDefaultShadowColor = "Grijs"
        case 33   : sDefaultShadowColor = "Gris"
        case 34   : sDefaultShadowColor = "Gris"
        case 39   : sDefaultShadowColor = "Grigio"
        case 46   : sDefaultShadowColor = "Grått"
        case 49   : sDefaultShadowColor = "Grau"
        case 55   : sDefaultShadowColor = "Cinza"
        case 81   : sDefaultShadowColor = "灰色"
        case 82   : sDefaultShadowColor = "회색"
        case 86   : sDefaultShadowColor = "灰色"
        case 88   : sDefaultShadowColor = "灰色"
        case else : QAErrorLog "The test does not support the language " + iSprache
                    Goto endsub
    end select
    
    PrintLog "- Test default shadow color"
    '/// <b> Test default shadow color </b>///
    
    Call hNewDocument
    
    '/// + format/Paragraph , tabpage borders
    '/// + Check if the default shadow color is gray
    Call fFormatParagraph("TabUmrandung")
    if SchattenFarbe.GetSelText <> sDefaultShadowColor then
        Warnlog "Shadow default color is wrong , get " & SchattenFarbe.GetSelText
    end if
    TabUmrandung.Cancel
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphBorder6

    Dim iBorderColor , iShadowColor   as Integer
    Dim sShadowDistance               as String

    iBorderColor = 5 : iShadowColor = 4
    sShadowDistance     = "0" + gSeperator + "40" + gMeasurementUnit

    PrintLog "- Test shadow setting"
    '/// <b> Test shadow setting </b>///

    Call hNewDocument

    '/// Format/Paragraph , tabpage borders , do some configuration => ///
    '/// + select 2nd border in default , select 5th border color ,///
    '/// + select 5th shadow , select 4th shadow color , set shadow distance to 0.40cm ///
    '/// + and close the dialog ///
     Call fFormatParagraph("TabUmrandung")
         Vorgaben.TypeKeys "<Right>" , 2
         Sleep 2
         StilFarbe.Select iBorderColor            'set border color
         Position.TypeKeys "<Right>" , 4
         Sleep 2
         Groesse.SetText      sShadowDistance
         SchattenFarbe.Select iShadowColor        'set shadow color
         Sleep 1
     TabUmrandung.OK

    '/// + format/Page , check if the configuration is all right
     Call fFormatParagraph("TabUmrandung")
         if StilFarbe.GetSelIndex     <> iBorderColor     then Warnlog "border color is wrong !"
         if Groesse.GetText           <> sShadowDistance  then Warnlog "shadow distance is wrong !"
         if SchattenFarbe.GetSelindex <> iShadowColor     then Warnlog "shadow color is wrong !"
     TabUmrandung.Cancel

    Call hCloseDocument

endcase


