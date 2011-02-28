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
'* short description : Test the functionality of Paragraph Formatting - 2
'*
'\***********************************************************************

testcase tParagraphAlignment1

  Dim sTestFile as String
  Dim sPositionX as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\alignmentTest.sxw")

  PrintLog "- Test 'Left' alignment"
  '/// Test 'Left' alignment

   Call hNewDocument

   '/// + Open the test file
   '/// + This file includes 1 paragraphs , and a pic is anchored to the character
    hFileOpenLocally( sTestFile )

   'Get the original coordinate-X
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        sPositionX = Horizontalby.Gettext
    TabPositionAndSizeWriter.Cancel
    FormatAnchorAsCharacter
    Sleep 1
    Call wTypeKeys "<ESCAPE>"

   '/// + Format/Paragraph / Alignment , choose Left/Top option
    Call fFormatParagraph("TabAusrichtungAbsatz")
        Links.Check
    TabAusrichtungAbsatz.OK

   '/// + check if the alignment is moved to left
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        if sPositionX <> Horizontalby.Gettext then warnlog "Alignment Left doesn't work !"
    TabPositionAndSizeWriter.Cancel

   Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphAlignment2

  Dim sTestFile as String
  Dim sPositionX as String

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\alignmentTestRight.sxw")
  sPositionX = "11" + gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test 'Right' alignment"
  '/// Test 'Right' alignment

   Call hNewDocument

   '/// + Open the test file
   '/// + This file includes 1 paragraphs , and a pic is anchored to the laste character
    hFileOpenLocally( sTestFile )

   '/// + Format/Paragraph / Alignment , choose Right/Bottom option
    Call fFormatParagraph("TabAusrichtungAbsatz")
        Rechts.Check
    TabAusrichtungAbsatz.OK

   '/// + check if the alignment is moved to Right
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        if fCalculateTolerance(Horizontalby.Gettext , sPositionX) > 0.02 then
            warnlog "Alignment Right doesn't work !"
        end if
    TabPositionAndSizeWriter.Cancel

   Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphAlignment3

	Dim sTestFile as String

	sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\alignmentTest.sxw")

  PrintLog "- Test 'Center' alignment"
  '/// Test 'Center' alignment

   Call hNewDocument

   '/// + Open the test file
   '/// + This file includes 1 paragraphs , and a pic is anchored to the character
    hFileOpenLocally( sTestFile )

   '/// + Format/Paragraph / Alignment , choose Center option
    Call fFormatParagraph("TabAusrichtungAbsatz")
        Zentriert.Check
    TabAusrichtungAbsatz.OK

   '/// + check if the alignment is moved to Center
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        if Int( Left(Horizontalby.Gettext , 1) ) > 7 OR Int( Left(Horizontalby.Gettext , 1) ) < 4 then
            warnlog "Alignment Center doesn't work !"
        end if
    TabPositionAndSizeWriter.Cancel

   Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphAlignment4

  Dim sTestFile as String
  Dim sPositionX1 as String
  Dim sPositionX2 as String

  sTestFile   = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\alignmentTestJustified.sxw")
  sPositionX1 = "12" + gSeperator + "84" + gMeasurementUnit

  PrintLog "- Test 'Justified' alignment - Default"
  '/// Test 'Justified' alignment - Default

   Call hNewDocument

   '/// Open the test file
   '/// + This file includes 1 paragraphs , and a pic is anchored to the last character
    hFileOpenLocally( sTestFile )

    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    FormatAnchorToPage
    Sleep 1

    Call fPositionAndSize("TabPositionAndSizeWriter")
        sPositionX2 = Horizontalby.Gettext
    TabPositionAndSizeWriter.Cancel
    Call wTypeKeys "<ESCAPE>"

   '/// Format/Paragraph / Alignment , choose Justified option , and select default in last line
    Call fFormatParagraph("TabAusrichtungAbsatz")
        Blocksatz.Check
        Sleep 1
        LetzteZeile.Select 1
    TabAusrichtungAbsatz.OK

   '/// check if the alignment is moved as Justified
    Call wTypeKeys ( "<Shift F4>" )
    Call wTypeKeys "<Tab>"
    Sleep 1
    FormatAnchorToPage
    Sleep 1

    Call fPositionAndSize("TabPositionAndSizeWriter")
        if fCalculateTolerance(Horizontalby.Gettext , sPositionX1) > 0.02 then
            if gPlatGroup = "unx" then
                Warnlog "Different font-sizes under Linux/Unix compared with windows."
            else
                Warnlog "Alignment Justified doesn't work -1 !"
            end if
        end if
    TabPositionAndSizeWriter.Cancel

    Call wTypeKeys "<Tab>"
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        if Horizontalby.Gettext <> sPositionX2 then 
            if gPlatGroup = "unx" then
                Warnlog "Different font-sizes under Linux/Unix compared with windows."
            else
                warnlog "Alignment Justified doesn't work -2 !"
            end if
        end if
    TabPositionAndSizeWriter.Cancel

   Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphAlignment5

  Dim sTestFile as String

  sTestFile   = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\alignmentTestJustified.sxw")

  PrintLog "- Test 'Justified' alignment - Centered"
  '/// Test 'Justified' alignment - Centered

   Call hNewDocument

   '/// Open the test file
   '/// + This file includes 1 paragraphs , and a pic is anchored to the last character
    hFileOpenLocally( sTestFile )

   '/// Format/Paragraph / Alignment ,
   '/// + choose Justified option , and select centered in last line
    Call fFormatParagraph("TabAusrichtungAbsatz")
        Blocksatz.Check
        Sleep 1
        LetzteZeile.Select 2
    TabAusrichtungAbsatz.OK

   '/// check if the alignment is moved as Justified
    Call wTypeKeys ( "<Shift F4>" )
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        if StrToDouble(Horizontalby.Gettext) < 7.5 OR StrToDouble(Horizontalby.Gettext) > 8.8 then
            warnlog "Alignment Justified - Centered doesn't work !"
        end if
    TabPositionAndSizeWriter.Cancel

   Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphAlignment6

  Dim sTestFile as String
  Dim sPositionX1 as String
  Dim sPositionX2 as String

  sTestFile   = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\alignmentTestJustifiedCentered.sxw")
  sPositionX2 = "11" + gSeperator + "15" + gMeasurementUnit

  PrintLog "- Test 'Justified' alignment - Justified"
  '/// Test 'Justified' alignment - Justified

   Call hNewDocument

   '/// Open the test file
   '/// + This file includes 1 paragraphs,
   '/// + and 2 pics are in 2nd line
    hFileOpenLocally( sTestFile )

    Call wTypeKeys ( "<Shift F4>" )
    Call wTypeKeys "<TAB>"
    Sleep 1
    FormatAnchorToPage
    Sleep 1

    Call fPositionAndSize("TabPositionAndSizeWriter")
        sPositionX1 = Horizontalby.Gettext
    TabPositionAndSizeWriter.Cancel
    FormatAnchorAsCharacter
    Sleep 1
    Call wTypeKeys "<ESCAPE>"

   '/// Format/Paragraph / Alignment ,
   '/// + choose Justified option , and select Justified in last line
    Call fFormatParagraph("TabAusrichtungAbsatz")
        Blocksatz.Check
        Sleep 1
        LetzteZeile.Select 3
    TabAusrichtungAbsatz.OK

   '/// check if the alignment is moved as Justified
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    FormatAnchorToPage
    Sleep 1

    Call fPositionAndSize("TabPositionAndSizeWriter")
        if Horizontalby.Gettext <> sPositionX1 then 
            if gPlatGroup = "unx" then
                Warnlog "Different font-sizes under Linux/Unix compared with windows."
            else
                warnlog "Alignment Justified doesn't work -1 !"
            end if
        end if
    TabPositionAndSizeWriter.Cancel

    Call wTypeKeys "<Tab>"
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        if fCalculateTolerance(Horizontalby.Gettext , sPositionX2) > 0.02 then Warnlog "Alignment Justified doesn't work -2 !"
    TabPositionAndSizeWriter.Cancel

   Call hCloseDocument
endcase

'--------------------------------------------------------------------------

testcase tParagraphAlignment7

  Dim sTestFile as String
  Dim sPositionX as String

  sTestFile   = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\testExpand.sxw")

  Printlog "- Test 'Expand single word'"
  Call hNewDocument

    '/// Open the test file
    '/// + This file includes 1 paragraphs ,
    '/// + there is only 1 word - "That" in last line ,
    '/// + and a drawring object is anchored as character after "That"
    hFileOpenLocally( sTestFile )

    printlog "Select the object."
    Sleep 1
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1

    printlog "Note the position of the drawing object"
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionX = Horizontalby.Gettext
    TabPositionAndSizeWriter.Cancel

    printlog "Deselect the object"
    Sleep 1
    Call wTypeKeys ( "<ESCAPE>" )

    '/// Format/Paragraph / Alignment ,
    '/// + choose Justified option , select Justified in 'last line'
    '/// + check 'expand single word'
    Call fFormatParagraph("TabAusrichtungAbsatz")
        Blocksatz.Check
        Sleep 1
        LetzteZeile.Select 3
        Sleep 1
        EinzelnesWortAustreiben.Check
        Sleep 1
    TabAusrichtungAbsatz.OK

    printlog "Select the drawing object , Format/Anchor / To page"
    Call wTypeKeys ( "<Shift F4>" )

    Sleep 1
    FormatAnchorToPage
    Sleep 1

    printlog "Check if the drawing object has moved from where it was before."
    Call fPositionAndSize("TabPositionAndSizeWriter")
        if Horizontalby.Gettext = sPositionX then QAErrorLog "#114314# Expand single word doesn't work"
    TabPositionAndSizeWriter.Cancel

    Call hCloseDocument
endcase

'--------------------------------------------------------------------------

testcase tParagraphAlignment8

  Dim bAsianLanguage as Boolean
  Dim sTestFile as String
  Dim sPositionX as String
  Dim sPositionY as String

  sTestFile   = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\testExpand.sxw")
  sPositionY  = "2" + gSeperator + "80" + gMeasurementUnit

  PrintLog "- Test Uncheck 'Snap to text grid'"
  '/// <b> Test Uncheck 'Snap to text grid' </b> ///

   Call hNewDocument

   '/// Set 'Asian Language support' on ///
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)

   '/// Open the test file
   '/// + This file includes 1 paragraphs ,
   '/// + and a drawring object is anchored as character after "That"
    hFileOpenLocally( sTestFile )

   '/// Get the drawing object original position ///
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
         sPositionX = Horizontalby.Gettext
    TabPositionAndSizeWriter.Cancel
    FormatAnchorAsCharacter
    Sleep 1
    Call wTypeKeys "<ESCAPE>"

   '/// Format/Paragraph / Alignment ,
   '/// + Uncheck 'Snap to text grid'
    Call fFormatParagraph("TabAusrichtungAbsatz")
        TextrasterFangen.UnCheck
        Sleep 1
    TabAusrichtungAbsatz.OK

   '/// Format/page / Text Grid
   '/// + Check 'Grid (lines only)
   '/// + close the dialog
    Call fFormatPageWriter("TabGrid")
        LinesGrid.Check
        Sleep 1
    TabGrid.OK

   '/// Check if the drawing object is in the same position
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        if Horizontalby.Gettext <> sPositionX then warnlog "Uncheck 'Snap to text grid' doesn't work - x position!"
        if fCalculateTolerance(Verticalby.Gettext , sPositionY) > 0.1 then Warnlog "Uncheck 'Snap to text grid' doesn't work - y position!"
    TabPositionAndSizeWriter.Cancel

   '/// Set the 'Asian Language support' to default
    if bAsianLanguage = FALSE then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if

   Call hCloseDocument
endcase

'--------------------------------------------------------------------------

testcase tParagraphAlignment9

  Dim bAsianLanguage as Boolean
  Dim sTestFile as String
  Dim sPositionX as String
  Dim sPositionY as String

  sTestFile   = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\testExpand.sxw")
  sPositionY  = "3" + gSeperator + "61" + gMeasurementUnit

  PrintLog "- Test check 'Snap to text grid'"
  '/// <b> Test check 'Snap to text grid' </b> ///

   Call hNewDocument

   '/// Set 'Asian Language support' on ///
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)

   '/// Open the test file
   '/// + This file includes 1 paragraphs ,
   '/// + and a drawring object is anchored as character after "That"
    hFileOpenLocally( sTestFile )

   '/// Get the drawing object original position ///
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
         sPositionX = Horizontalby.Gettext
    TabPositionAndSizeWriter.Cancel
    FormatAnchorAsCharacter
    Sleep 1
    Call wTypeKeys "<ESCAPE>"

   '/// Format/Paragraph / Alignment ,
   '/// + check 'Snap to text grid'
    Call fFormatParagraph("TabAusrichtungAbsatz")
        TextrasterFangen.Check
        Sleep 1
    TabAusrichtungAbsatz.OK

   '/// Format/page / Text Grid
   '/// + Check 'Grid (lines only)
   '/// + close the dialog
    Call fFormatPageWriter("TabGrid")
        LinesGrid.Check
        Sleep 1
    TabGrid.OK

   '/// Check if the drawing object is in the same position
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        if Horizontalby.Gettext <> sPositionX then warnlog "check 'Snap to text grid' doesn't work - x position!"
        if fCalculateTolerance(Verticalby.Gettext , sPositionY) > 0.1 then Warnlog "check 'Snap to text grid' doesn't work - y position!"
    TabPositionAndSizeWriter.Cancel

   '/// Set the 'Asian Language support' to default
    if bAsianLanguage = FALSE then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if

   Call hCloseDocument
endcase

'--------------------------------------------------------------------------

testcase tParagraphTextFlow1
    
    Dim sTestFile as String
    Dim sTestText as String
    Dim i as Integer
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\hyphenation.sxw")
    sTestText = "h"
    
    PrintLog "- Hyphenation must be processed while entering text WITHOUT inquiry"
    '/// Hyphenation must be processed while entering text WITHOUT inquiry
    
    Call hNewDocument
    
    '/// Open the test file
    '/// + This file includes 2 words : AAA and Kilometerhigh
    hFileOpenLocally( sTestFile )
    
    'the focus is in front of Kilometerhigh
    Call wTypeKeys "<Home>"
    
    '/// 1. Test there is no Hyphenation configuratioin
    For i = 1 to 10
        Call wTypeKeys "M"
        Call wTypeKeys "<End>"
        Call wTypeKeys "<Shift Left>"
        EditCopy
        if GetClipboardText <> sTestText then
            if GetClipboardText = "A" then
                Goto GoON
            else
                warnlog "no Hyphenation configuratioin doesn't work !"
            end if
        end if
        Call wTypeKeys "<Home>"
    next i
    
    warnlog "1.Something wrong in Hyphenation test !"
    
    GoON:
    
    Call hCloseDocument
    
    Call hNewDocument
    hFileOpenLocally( sTestFile )
    
    '/// 2. Test there is the Hyphenation configuratioin
    '/// + Format/Paragraph / Text Flow ,
    '/// + Check Hyphenation 'Automatically'
    Call fFormatParagraph("TabTextfluss")
    Automatisch.Check
    TabTextfluss.OK
    
    Call wTypeKeys "<Home>"
    
    For i = 1 to 10
        Call wTypeKeys "M"
        Call wTypeKeys "<End>"
        Call wTypeKeys "<Shift Left>"
        EditCopy
        if GetClipboardText <> sTestText then
            if GetClipboardText = "t" then
                Goto GoON1
            else
                warnlog "Hyphenation configuratioin doesn't work !"
            end if
        end if
        Call wTypeKeys "<Home>"
    next i
    warnlog "2.Something wrong in Hyphenation test !"
    
    GoON1:
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphTextFlow2

    Dim sTestFile as String
    Dim sTestText as String
    Dim i as Integer
    
    sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\hyphenation.sxw")
    sTestText  = "Kilomet"
    
    PrintLog "- Hyphenation - Characters at line end"
    '/// Hyphenation - Characters at line end
    
    Call hNewDocument
    
    '/// Open the test file
    '/// + This file includes 2 words : AAA and Kilometerhigh
    hFileOpenLocally( sTestFile )
    
    '/// 1.Test Characters at line end ==> 6
    '/// + Format/Paragraph / Text Flow ,
    '/// + Check Hyphenation 'Automatically'
    '/// + Set 6 in 'Characters at line end'
    Call fFormatParagraph("TabTextfluss")
    Automatisch.Check
    Sleep 1
    AbZeichenende.SetText "6"
    TabTextfluss.OK
    
    'the focus is in front of Kilometerhigh
    Call wTypeKeys "<Home>"
    
    For i = 1 to 10
        Call wTypeKeys "M"
        Call wTypeKeys "<End>"
        Call wTypeKeys "<Shift Left>" , 7

        EditCopy
        if GetClipboardText = sTestText then Goto GoON1
        Call wTypeKeys "<Home>"
    next i
    warnlog "Something wrong in Characters at line end = > 6"
    
    GoON1:
    Call hCloseDocument
    
    Call hNewDocument
    
    hFileOpenLocally( sTestFile )
    
    '/// 2.Test Characters at line end ==> 8
    '/// + Format/Paragraph / Text Flow ,
    '/// + Check Hyphenation 'Automatically'
    '/// + Set 8 in 'Characters at line end'
    Call fFormatParagraph("TabTextfluss")
    Automatisch.Check
    Sleep 1
    AbZeichenende.SetText "8"
    TabTextfluss.OK
    
    'the focus is in front of Kilometerhigh
    Call wTypeKeys "<Home>"
    
    For i = 1 to 10
        Call wTypeKeys "M"
        Call wTypeKeys "<End>"
        Call wTypeKeys "<Shift Left>" , 6
        EditCopy
        if GetClipboardText = sTestText then  warnlog "Something wrong in Characters at line end = > 8"
        Call wTypeKeys "<Home>"
    next i
    
    Call hCloseDocument
endcase

'--------------------------------------------------------------------------

testcase tParagraphTextFlow3

    Dim sTestFile      as String
    Dim sTestText     as String
    Dim i                   as Integer

    sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\hyphenation.sxw")
    sTestText  = "lomet"
    
    PrintLog "- Hyphenation - Characters at line begin"
    '/// Hyphenation - Characters at line begin
    
    Call hNewDocument
    
    '/// Open the test file
    '/// + This file includes 2 words : AAA and Kilometerhigh
    hFileOpenLocally( sTestFile )
    
    '/// 1.Test Characters at line begin ==> 4
    '/// + Format/Paragraph / Text Flow ,
    '/// + Check Hyphenation 'Automatically'
    '/// + Set 4 in 'Characters at line begin'
    Call fFormatParagraph("TabTextfluss")
    Automatisch.Check
    Sleep 1
    AbZeichenAnfang.SetText "4"
    TabTextfluss.OK
    
    'the focus is in front of Kilometerhigh
    Call wTypeKeys "<Home>"
    
    For i = 1 to 10
        Call wTypeKeys "M"
        Call wTypeKeys "<End>"
        Call wTypeKeys "<Shift Left>" , 5
        EditCopy
        if GetClipboardText = sTestText then Goto GoON1
        Call wTypeKeys "<Home>"
    next i
    warnlog "Something wrong in Characters at line end = > 4"
    
    GoON1:
    Call hCloseDocument
    
    Call hNewDocument
    
    hFileOpenLocally( sTestFile )

    
    '/// 2.Test Characters at line end ==> 6
    '/// + Format/Paragraph / Text Flow ,
    '/// + Check Hyphenation 'Automatically'
    '/// + Set 8 in 'Characters at line end'
    Call fFormatParagraph("TabTextfluss")
    Automatisch.Check
    Sleep 1
    AbZeichenAnfang.SetText "6"
    TabTextfluss.OK
    
    'the focus is in front of Kilometerhigh
    Call wTypeKeys "<Home>"
    
    For i = 1 to 10
        Call wTypeKeys "M"
        Call wTypeKeys "<End>"
        Call wTypeKeys "<Shift Left>" , 5

        EditCopy
        if GetClipboardText = sTestText then Goto GoON2
        Call wTypeKeys "<Home>"

    next i
    warnlog "Something wrong in Characters at line end = > 6"
    
    GoON2:
    Call hCloseDocument
endcase

'--------------------------------------------------------------------------

testcase tParagraphTextFlow4

  Dim sTestFile as String
  Dim i as Integer
  Dim j as Integer
  Dim sTestText()  as Variant

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\hyphenationNew.odt")

  PrintLog "- Hyphenation - maxinum number of consecutive typhens"
  '/// Hyphenation - maxinum number of consecutive typhens

   Call hNewDocument

   '/// Open the test file
   '/// + This file includes 4 words : KiloAmterhigh1 kiloAmeterhigh2 kiloAmeterhigh3 kiloAmeterhigh4
   '/// + the words' length is longer then page length
    hFileOpenLocally( sTestFile )

   '/// 1.Test maxinum number of consecutive typhens (default => 0)
   '/// + Format/Paragraph / Text Flow ,
   '/// + Check Hyphenation 'Automatically'
   '/// + Set 0 in 'maxinum number of consecutive typhens'
    sTestText = Array( _
                      "Amet"    , _
                      "1 kilo"    , _
                      "erhigh"    , _
                      "Amet"    , _
                      )

    j = UBound(sTestText)

    Call fFormatParagraph("TabTextfluss")
        Automatisch.Check
        Sleep 1
        Trennstellen.SetText "0"
    TabTextfluss.OK

    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Down>"
    For i = 0 to j
        Call wTypeKeys "<Down>" , i*2
        Call wTypeKeys "<Shift End>"
        EditCopy
        if GetClipboardText <> sTestText(i) then
            warnlog " Something wrong in Consecutive typhens 0 : " &i
            exit for
        end if
        Call wTypeKeys "<MOD1 Home>"
        Call wTypeKeys "<Down>"
    Next i

   '/// 2.Test maxinum number of consecutive typhens = >1
   '/// + Format/Paragraph / Text Flow ,
   '/// + Check Hyphenation 'Automatically'
   '/// + Set 0 in 'maxinum number of consecutive typhens'
    sTestText = Array( _
                      "Amete"    , _
                      "kilo"         , _
                      "rhigh2"    , _
                      "Amete"    , _
                      )

    Call fFormatParagraph("TabTextfluss")
        Automatisch.Check
        Sleep 1
        Trennstellen.SetText "1"
    TabTextfluss.OK

    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Down>"
    For i = 0 to j
        Call wTypeKeys "<Down>" , i*2
        Call wTypeKeys "<Shift End>"
        EditCopy
        if GetClipboardText <> sTestText(i) then 
            warnlog " Something wrong in Consecutive typhens 1 : " &i
            exit for
        end if
        Call wTypeKeys "<MOD1 Home>"
        Call wTypeKeys "<Down>"
    Next i

   '/// 3.Test maxinum number of consecutive typhens = >2
   '/// + Format/Paragraph / Text Flow ,
   '/// + Check Hyphenation 'Automatically'
   '/// + Set 0 in 'maxinum number of consecutive typhens'
    sTestText = Array( _
                      "Amet"      , _
                      "1 kilo"      , _
                      "erhigh"     , _
                      "Amet"      , _
                      )

    Call fFormatParagraph("TabTextfluss")
        Automatisch.Check
        Sleep 1
        Trennstellen.SetText "2"
    TabTextfluss.OK

    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Down>"
    For i = 0 to j
        Call wTypeKeys "<Down>" , i*2
        Call wTypeKeys "<Shift End>"
        EditCopy
        if GetClipboardText <> sTestText(i) then
            warnlog " Something wrong in Consecutive typhens 2 : " &i
            exit for
        end if
        Call wTypeKeys "<MOD1 Home>"
        Call wTypeKeys "<Down>"
    Next i

   Call hCloseDocument
endcase

'--------------------------------------------------------------------------

testcase tParagraphTextFlow5

  Dim sTestFile as String

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\columnBreakParagraphBefore.sxw")

  PrintLog "- Paragraph break before Column"
  '/// Paragraph break before Column

   Call hNewDocument

   '/// Open the test file
   '/// + There are 2 columns/page ,includes  2 paragraph
   '/// + the 1st paragraph is in 1st and 2nd column
   '/// + the 2nd paragraph is is 2nd and 3rd column
    hFileOpenLocally( sTestFile )

   '/// Set the focus to 2nd paragraph
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
        SearchFor.SetText "She"
        Sleep 1
        SearchAll.Click
        Sleep 1
    FindAndReplace.Close

   '/// Format/Paragraph / Text Flow
   '/// + Set 'Breaks' enable
   '/// + Select column in Type' and select Before in 'Position'
    Call fFormatParagraph("TabTextfluss")
        Umbruch.Check
        Sleep 1
        UmbruchTyp.Select 2
        Sleep 1
        UmbruchPosition.Select 1
    TabTextfluss.OK

   '/// + check if the 2nd paragraph is in 2nd page
    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "2" then warnlog "Something wrong in 'Paragraph break before Column'!"
    fCloseNavigator

   Call hCloseDocument
endcase

'--------------------------------------------------------------------------

testcase tParagraphTextFlow6

  Dim sTestFile as String

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\columnBreakParagraphAfter.sxw")

  PrintLog "- Paragraph break after Column"
  '/// Paragraph break after Column

   Call hNewDocument

   '/// Open the test file
   '/// + There are 2 columns/page ,includes  1 paragraph
   '/// + the paragraph is in 1st and 2nd column
   '/// + and it is NOT full of 2nd column
    hFileOpenLocally( sTestFile )

   '/// Format/Paragraph / Text Flow
   '/// + Set 'Breaks' enable
   '/// + Select column in Type' and select After in 'Position'
    Call fFormatParagraph("TabTextfluss")
        Umbruch.Check
        Sleep 1
        UmbruchTyp.Select 2
        Sleep 1
        UmbruchPosition.Select 2
    TabTextfluss.OK

   '/// Set the focus to the end of the paragraph
    Call wTypeKeys "<MOD1 End>"
    Call wTypeKeys "<Return>"

   '/// + check if the 2nd paragraph is in 2nd page
    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "2" then warnlog "Something wrong in 'Paragraph break after Column'! "
    fCloseNavigator

   Call hCloseDocument
endcase

'--------------------------------------------------------------------------

testcase tParagraphTextFlow7

  Dim sTestFile as String

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\pageBreakBefore.sxw")

  PrintLog "- Paragraph break before Page"
  '/// Paragraph break before Page

   Call hNewDocument

   '/// Open the test file
   '/// + There are 2 paragraphs in 1 page
    hFileOpenLocally( sTestFile )

   '/// Set the focus to 2nd paragraph
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
        SearchFor.SetText "She"
        Sleep 1
        SearchAll.Click
        Sleep 1
    FindAndReplace.Close

   '/// Format/Paragraph / Text Flow
   '/// + Set 'Breaks' enable
   '/// + Select page in 'Type' and select Before in 'Position'
   '/// + Select 3rd in 'with page style' and set 4 in 'page number'
    Call fFormatParagraph("TabTextfluss")
        Umbruch.Check
        Sleep 1
        UmbruchTyp.Select 1
        Sleep 1
        UmbruchPosition.Select 1
        Sleep 1
        MitSeitenvorlage.Check
        Sleep 1
        Vorlage.Select 3
        Seitenzahl.SetText "4"
    TabTextfluss.OK

   '/// + check if the 2nd paragraph is in 4th page
    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "2" then warnlog "Something wrong in ' Column break after paragraph'! "
    fCloseNavigator

    Call fFormatParagraph("TabTextfluss")
        if Vorlage.GetSelIndex <> 3   then  warnlog "Should select the 3rd in with page style , but get " &Vorlage.GetSelIndex
        if Seitenzahl.GetText  <> "4" then  warnlog "Should get 4 in with page number , but get " &Seitenzahl.GetText
    TabTextfluss.OK

   Call hCloseDocument
endcase

'--------------------------------------------------------------------------

testcase tParagraphTextFlow8

  Dim sTestFile as String

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\pageBreakAfter.sxw")

  PrintLog "-  Paragraph break after page"
  '/// Paragraph break after page

   Call hNewDocument

   '/// Open the test file
   '/// + There is 1 paragraph in the file
    hFileOpenLocally( sTestFile )

   '/// Format/Paragraph / Text Flow
   '/// + Set 'Breaks' enable
   '/// + Select page in Type' and select After in 'Position'
    Call fFormatParagraph("TabTextfluss")
        Umbruch.Check
        Sleep 1
        UmbruchTyp.Select 1
        Sleep 1
        UmbruchPosition.Select 2
    TabTextfluss.OK

   '/// Set the focus to the end of the paragraph
    Call wTypeKeys "<MOD1 End>"
    Call wTypeKeys "<Return>"

   '/// + check if the 2nd paragraph is in 2nd page
    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "2" then warnlog "Something wrong in 'Paragraph break after page'! "
    fCloseNavigator

   Call hCloseDocument
endcase

'--------------------------------------------------------------------------

testcase tParagraphTextFlow9

  Dim sTestFile as String
  Dim sPage1   as String
  Dim sPage2   as String
  Dim i              as Integer

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\textFlowOptions.sxw")

  PrintLog "-  Don't split paragraph"
  '/// Don't split paragraph

   Call hNewDocument

   '/// Open the test file
   '/// + There is 1 paragraph which cross 2 pages
    hFileOpenLocally( sTestFile )
    Call wTypeKeys "<MOD1 Home>"

    Call fFindWord("She")
    Call wTypeKeys "<Home>"

    PrintLog "-  Uncheck Don't split paragraph"
   '/// 1. Test unchecking "Don't split paragraph"
   '/// Format/Paragraph / Text Flow
   '/// + UnCheck "Don't split paragraph"
    Call fFormatParagraph("TabTextfluss")
        AbsatzNichtTrennen.UnCheck
    TabTextfluss.OK

    Kontext "Navigator"
       if Not Navigator.Exists then ViewNavigator
       Kontext "NavigatorWriter"
       Sleep 1
       if Seitennummer.GetText <> "1" then warnlog "The head of the paragraph should be in 1st page !"
    fCloseNavigator

    Call wTypeKeys "<MOD1 End>"
    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "2" then warnlog "The end of the paragraph should be in 2nd page !"
    fCloseNavigator

   Call hCloseDocument

   PrintLog "-  Check Don't split paragraph"
   Call hNewDocument
    hFileOpenLocally( sTestFile )

    Call wTypeKeys "<MOD1 Home>"

    Call fFindWord("She")
    Call wTypeKeys "<Home>"

   '/// 2. Test checking "Don't split paragraph"
   '/// Format/Paragraph / Text Flow
   '/// + Check "Don't split paragraph"
    Call fFormatParagraph("TabTextfluss")
        AbsatzNichtTrennen.Check
    TabTextfluss.OK

    Kontext "Navigator"
       if Not Navigator.Exists then ViewNavigator
       Kontext "NavigatorWriter"
       Sleep 1
       if Seitennummer.GetText <> "2" then warnlog "The head of the paragraph should be in 2nd page !"
    fCloseNavigator

    Call wTypeKeys "<MOD1 End>"
    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "2" then warnlog "The end of the paragraph should be in 2nd page !"
    fCloseNavigator

   Call hCloseDocument
endcase

'--------------------------------------------------------------------------

testcase tParagraphTextFlow10

  Dim sTestFile as String
  Dim sPage1 as String
  Dim sPage2 as String
  Dim i as Integer

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\textFlowOptions1.sxw")

  PrintLog "-  Keep with next paragraph"
  '/// Keep with next paragraph

   Call hNewDocument

   '/// Open the test file
   '/// There are 2 paragraphes in the file , 1st one crosses
   '/// + 1st and 2nd page , 2nd one is in 2nd page
    hFileOpenLocally( sTestFile )
    EditSearchAndReplace
    Kontext "FindAndReplace"
        SearchFor.SetText "He1"
        Sleep 1
        SearchAll.Click
        Sleep 1
    FindAndReplace.Close
    Call wTypeKeys "<Home>"

    PrintLog "-  Uncheck Keep with next paragraph"
   '/// 1. Test unchecking "Keep with next paragraph"
   '/// Format/Paragraph / Text Flow
   '/// + UnCheck "Keep with next paragraph"
    Call fFormatParagraph("TabTextfluss")
        AbsaetzeZusammenhalten.UnCheck
    TabTextfluss.OK

    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "1" then warnlog "The head of the 1st paragraph should be in 1st page ! == " &i
    fCloseNavigator

    EditSearchAndReplace
    Kontext "FindAndReplace"
        SearchFor.SetText "He2"
        Sleep 1
        SearchAll.Click
        Sleep 1
    FindAndReplace.Close
    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "2" then warnlog "The head of the 2nd paragraph should be in 2nd page !"
    fCloseNavigator

   Call hCloseDocument

   Call hNewDocument

    hFileOpenLocally( sTestFile )

    PrintLog "-  Check Keep with next paragraph"
   '/// 2. Test checking "Keep with next paragraph"
   '/// Set focus to first paragraph ,Format/Paragraph / Text Flow
   '/// + Check "Keep with next paragraph"
    EditSearchAndReplace
    Kontext "FindAndReplace"
        SearchFor.SetText "He1"
        Sleep 1
        SearchAll.Click
        Sleep 1
    FindAndReplace.Close
    Call wTypeKeys "<Home>"
    Call fFormatParagraph("TabTextfluss")
        AbsaetzeZusammenhalten.Check
    TabTextfluss.OK

    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "2" then warnlog "The head of the 1st paragraph should be in 2nd page !"
    fCloseNavigator

   'Get the head of 2nd paragarph's location
    EditSearchAndReplace
    Kontext "FindAndReplace"
        SearchFor.SetText "He2"
        Sleep 1
        SearchAll.Click
        Sleep 1
    FindAndReplace.Close

    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "2" then warnlog "The head of the 2nd paragraph should be in 2nd page !"
    fCloseNavigator

   Call hCloseDocument
endcase

'--------------------------------------------------------------------------

testcase tParagraphTextFlow11

  Dim sTestFile as String
  Dim sLines as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\eightline.sxw")
  sLines    = "3"

  PrintLog "- Orphan control"
  '/// 'Orphan control

   Call hNewDocument

   '/// Open the test file
   '/// + There is 1 paragraph (8 lines) in the file
    hFileOpenLocally( sTestFile )

   '/// Format/Paragraph / Text Flow
   '/// + Check 'Orphan control' , set 3 in 'Lines'
    Call fFormatParagraph("TabTextfluss")
        Hurenkinderregelung.Check
        Sleep 1
        HurenkinderZeilen.SetText sLines
    TabTextfluss.OK

   '/// Press return 5 times to let only 3 lines are in 1st page
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Return>" , 5

   '/// + check if the 1st line is in 1st page
    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "1" then warnlog "The 1st line should be in 1st page!"
    fCloseNavigator

   '/// Press Return again
    Call wTypeKeys "<Return>"

   '/// + check if the 1st line is in 2nd page
    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "2" then warnlog "The 1st line should be in 2nd page!"
    fCloseNavigator

   Call hCloseDocument
endcase

'--------------------------------------------------------------------------

testcase tParagraphTextFlow12

  Dim sTestFile as String
  Dim sLines as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\eightline.sxw")
  sLines    = "3"

  PrintLog "- Widow control"
  '/// 'Widow control

   Call hNewDocument

   '/// Open the test file
   '/// + There is 1 paragraph (8 lines) in the file
    hFileOpenLocally( sTestFile )

   '/// Format/Paragraph / Text Flow
   '/// + Check 'Widow control' , set 3 in 'Lines'
    Call fFormatParagraph("TabTextfluss")
        Schusterjungenregelung.Check
        Sleep 1
        SchusterZeilen.SetText sLines
    TabTextfluss.OK

   '/// Press return to let the lastest line into 2nd page
    Call wTypeKeys "<MOD1 Home><Return>"

   '/// + check if the lastest 3rd line(6th line) is in 2nd page
    Call wTypeKeys "<Down>"  , 5
    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "2" then warnlog "The 6th line should be in 2nd page!"
    fCloseNavigator

   '/// + check if the 5th line is in 1st page
    Call wTypeKeys "<Up>"
    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "1" then warnlog "The 5th line should be in 1st page!"
    fCloseNavigator

   Call hCloseDocument
endcase

'--------------------------------------------------------------------------

testcase tParagraphTextFlow13

  Dim sTestFile as String
  Dim i as Integer

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\specialHyphenation.sxw")

  PrintLog "- Special test for Hyphenation"
  '/// <b> 'Widow control </b> ///

   Call hNewDocument

   '/// Open the test file
   '/// + There is 1 word TABC in the end of the first line
    hFileOpenLocally( sTestFile )

   '/// Press spaces in the head of the first line , we hope
   '/// + the whole word - 'TABC' will go to 2nd line at one time
    For i = 1 to 30
        Call wTypeKeys "<Home>"
        Call wTypeKeys " "
        Call wTypeKeys "<End>"
        Call wTypeKeys "<Shift Left>"
        EditCopy
        if GetClipboardText = "B" then
            QAErrorLog "#114094#"
            Exit for
        else
            if GetClipboardText <> "C" then
                Exit for
            end if
        end if
    next i

   Call hCloseDocument
endcase

'--------------------------------------------------------------------------
