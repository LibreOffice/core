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
'* short description : Test the functionality of Paragraph Formatting - 1
'*
'************************************************************************
'*
' #1 tFormatParagraph1       'Open format/paragraph dialog
' #1 tParagraphIndent1       'Set values for Indent - Before text
' #1 tParagraphIndent2       'Set values for Indent - after text
' #1 tParagraphIndent3       'Set values for Indent - first line
' #1 tParagraphIndent4       'Test checkbox - "automatic"
' #1 tParagraphIndent5       'Set values for "Above paraghraph"
' #1 tParagraphIndent6       'Set values for "Below paraghraph"
' #1 tParagraphIndent7       'Test above and below spacing of successive paragraphs are not added up, the biggest one should be displayed
' #1 tParagraphIndent8       'Test above and below spacing of successive paragraphs are added up
' #1 tParagraphIndent9       'Test Line spacing - Single
' #1 tParagraphIndent10      'Test Line spacing - 1.5 lines
' #1 tParagraphIndent11      'Test Line spacing - Double lines
' #1 tParagraphIndent12      'Test Line spacing - Proportional
' #1 tParagraphIndent13      'Test Line spacing - At least
' #1 tParagraphIndent14      'Test Line spacing - Leading
' #1 tParagraphIndent15      'Test Line spacing - Fixed
' #1 tParagraphIndent16      'Test Regester-true Active
' #1 tParagraphIndent17      'Test "Add spacing between paragarph and talbes" when importing from MS-Word
' #1 tParagraphIndent18      'Test Negativ indent
'*
'\***********************************************************************

testcase tformatparagraph1

  PrintLog "- 'Open format/paragraph dialog"
  '/// <b>Open format/paragraph dialog</b>

   Call hNewDocument

   '/// via Mouse clicks in Menu: Format-paragraph
    try
        Call fFormatParagraph("TabTextfluss")
        TabTextfluss.Cancel
    catch
    endcatch

   '/// via shortcuts in menu  (manual test is needed)

   '/// via Context menu when Cursor is in the document
    try
        hMenuSelectNr(8)
        Sleep 2
        Kontext
        Active.SetPage TabTextfluss
        Kontext "TabTextfluss"
        TabTextfluss.Cancel
    catch
    endcatch

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphIndent1

  Dim sTest                               as String
  Dim iLenTest                            as Integer
  Dim iLenHalf                            as Integer
  Dim sPageWidth                          as String
  Dim sMarginLeft                         as String
  Dim sMarginRight                        as String
  Dim sIndentLeft                         as String

  sTest        = "lllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll"
  sPageWidth   = "8" + gSeperator + "00" + gMeasurementUnit
  sMarginLeft  = "2" + gSeperator + "00" + gMeasurementUnit
  sMarginRight = "2" + gSeperator + "00" + gMeasurementUnit
  sIndentLeft  = "2" + gSeperator + "00" + gMeasurementUnit

  PrintLog "- Set values for Indent - Before text"
  '/// <b>Set values for Indent - Before text</b>

   Call hNewDocument

   'Format/Page , tabpage Page , set page size and margin size
    Call fFormatPageWriter("TabSeite")
        Breite.SetText  sPageWidth
        Links.SetText   sMarginLeft
        Rechts.SetText  sMarginRight
    TabSeite.OK

   '/// Input some test words which should be longer than one line
    Call wTypeKeys sTest
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    iLenTest = Len(getclipboardtext)

   '/// Format/Paragraph / Indents & Spacing , set 2cm in "before Text"
    Call fFormatParagraph("TabEinzuegeUndAbstaende")
        Vonlinks.SetText sIndentLeft
    TabEinzuegeUndAbstaende.OK

    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    iLenHalf = Len(getclipboardtext)

    if iLenHalf <> Int(iLenTest/2) AND iLenHalf <> Int(iLenTest/2 +1) AND iLenHalf <> Int(iLenTest/2 -1) then
        Warnlog "1.Indents - before Text is not right !"
    end if

    Call wTypeKeys "<Right><Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    iLenHalf = Len(getclipboardtext)

    if iLenHalf <> Int(iLenTest/2) AND iLenHalf <> Int(iLenTest/2 +1) AND iLenHalf <> Int(iLenTest/2 -1) then
        Warnlog "2.Indents - before Text is not right !"
    end if

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphIndent2

  Dim sTest                               as String
  Dim iLenTest                            as Integer
  Dim iLenHalf                            as Integer
  Dim sPageWidth                          as String
  Dim sMarginLeft                         as String
  Dim sMarginRight                        as String
  Dim sIndentRight                        as String

  sTest        = "lllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll"
  sPageWidth   = "8" + gSeperator + "00" + gMeasurementUnit
  sMarginLeft  = "2" + gSeperator + "00" + gMeasurementUnit
  sMarginRight = "2" + gSeperator + "00" + gMeasurementUnit
  sIndentRight = "2" + gSeperator + "00" + gMeasurementUnit

  PrintLog "- Set values for Indent - after text"
  '/// <b>Set values for Indent - after text</b>

   Call hNewDocument

   'Format/Page , tabpage Page , set page size and margin size
    Call fFormatPageWriter("TabSeite")
        Breite.SetText  sPageWidth
        Links.SetText   sMarginLeft
        Rechts.SetText  sMarginRight
    TabSeite.OK

   '/// Input some test words which should be longer than one line
    Call wTypeKeys sTest
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    iLenTest = Len(getclipboardtext)

   '/// Format/Paragraph / Indents & Spacing , set 2cm in "after Text"
    Call fFormatParagraph("TabEinzuegeUndAbstaende")
        VonRechts.SetText sIndentRight
    TabEinzuegeUndAbstaende.OK

    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    iLenHalf = Len(getclipboardtext)

    if iLenHalf <> Int(iLenTest/2) AND iLenHalf <> Int(iLenTest/2 +1) AND iLenHalf <> Int(iLenTest/2 -1) then
        Warnlog "1.Indents - after Text is not right !"
    end if

    Call wTypeKeys "<Right><Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    iLenHalf = Len(getclipboardtext)

    if iLenHalf <> Int(iLenTest/2) AND iLenHalf <> Int(iLenTest/2 +1) AND iLenHalf <> Int(iLenTest/2 -1) then
        Warnlog "2.Indents - after Text is not right !"
    end if

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphIndent3

  Dim sTest                             as String
  Dim iLenTest                          as Integer
  Dim iLenNew                           as Integer
  Dim sPageWidth                        as String
  Dim sMarginLeft                       as String
  Dim sMarginRight                      as String
  Dim sIndentRight                      as String
  Dim sIndentFirst                      as String

  sTest        = "lllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll"
  sPageWidth   = "8" + gSeperator + "00" + gMeasurementUnit
  sMarginLeft  = "2" + gSeperator + "00" + gMeasurementUnit
  sMarginRight = "2" + gSeperator + "00" + gMeasurementUnit
  sIndentFirst = "2" + gSeperator + "00" + gMeasurementUnit

  PrintLog "- Set values for Indent - First line"
  '/// <b>Set values for Indent - First line</b>

   Call hNewDocument

   'Format/Page , tabpage Page , set page size and margin size
    Call fFormatPageWriter("TabSeite")
        Breite.SetText  sPageWidth
        Links.SetText   sMarginLeft
        Rechts.SetText  sMarginRight
    TabSeite.OK

   '/// Input some test words which should be longer than one line
    Call wTypeKeys sTest
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    iLenTest = Len(getclipboardtext)

   '/// Format/Paragraph / Indents & Spacing , set 2cm in "first line"
    Call fFormatParagraph("TabEinzuegeUndAbstaende")
        ErsteZeile.SetText sIndentFirst
    TabEinzuegeUndAbstaende.OK

    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    iLenNew = Len(getclipboardtext)

    if iLenNew <> Int(iLenTest/2) AND iLenNew <> Int(iLenTest/2 +1) AND iLenNew <> Int(iLenTest/2 -1) then
        Warnlog "1.Indents - First line is not right !"
    end if

    Call wTypeKeys "<Right><Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    iLenNew = Len(getclipboardtext)

    if iLenNew <> iLenTest then
        Warnlog "2.Indents - first line is not right !"
    end if

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphIndent4

  Dim sTest                             as String
  Dim iLenTest                          as Integer
  Dim iLenNew                           as Integer
  Dim sPageWidth                        as String
  Dim sMarginLeft                       as String
  Dim sMarginRight                      as String
  Dim sIndentRight                      as String
  Dim sIndentFirst                      as String

  sTest        = "lllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll"
  sPageWidth   = "8" + gSeperator + "00" + gMeasurementUnit
  sMarginLeft  = "2" + gSeperator + "00" + gMeasurementUnit
  sMarginRight = "2" + gSeperator + "00" + gMeasurementUnit
  sIndentFirst = "2" + gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test checkbox - 'automatic'"
  '/// <b>Test checkbox - "automatic"</b>

   Call hNewDocument

   'Format/Page , tabpage Page , set page size and margin size
    Call fFormatPageWriter("TabSeite")
        Breite.SetText  sPageWidth
        Links.SetText   sMarginLeft
        Rechts.SetText  sMarginRight
    TabSeite.OK

   '/// Input some test words which should be longer than one line
    Call wTypeKeys sTest
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    iLenTest = Len(getclipboardtext)

   '/// Format/Paragraph / Indents & Spacing , Check "automatic"
    Call fFormatParagraph("TabEinzuegeUndAbstaende")
        Automatisch.Check
        Sleep 2
        if ErsteZeile.IsEnabled = TRUE then Warnlog "The first line shouldn't be enabled when checking automatic !"
    TabEinzuegeUndAbstaende.OK

    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    iLenNew = Len(getclipboardtext)

   'the first line should be shorter than before and longer then 1/2 previous length
    if iLenNew >= iLenTest OR iLenNew < Int(iLenTest/2 -1) then
        Warnlog "The 1st line length isn't right after checking 'automatic'!"
    end if

    Call wTypeKeys "<Right><Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    iLenNew = Len(getclipboardtext)

   'the 2nd line should be equal the previous length
    if iLenNew <> iLenTest then
        Warnlog "The 2nd line length isn't right after checking 'automatic'!"
    end if

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphIndent5

  Dim sTestFile          as String
  Dim sTest1             as String
  Dim sTest2             as String  'the words in 2nd paragraph (sTest1 is head , sTest2 is end)
  Dim sSpacing           as String

  sTest1    = "He2" : sTest2 = "endof2ndparagraph"
  sSpacing  = "10" + gSeperator + "00" + gMeasurementUnit
  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\indent.sxw")

  PrintLog "- Set values for 'Above paraghraph'"
  '/// <b>Set values for "Above paraghraph"</b>

   Call hNewDocument

   '/// Open the test file
   '/// This file includes 2 paragraphs , and the end of 2nd paragraph is in 1st page
    hFileOpenLocally( sTestFile )
    

   'Set the focus to 2nd paragraph
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
        SearchFor.SetText sTest1
        Sleep 1
        SearchAll.Click
        Sleep 1
    FindAndReplace.Close

   '/// Format/Paragraph / Indents & Spacing , set 10cm for "Above paraghraph"
    Call fFormatParagraph("TabEinzuegeUndAbstaende")
        Oben.SetText sSpacing
    TabEinzuegeUndAbstaende.OK

   'Set the focus to the end of the 2nd paragraph
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
        SearchFor.SetText sTest2
        Sleep 1
        SearchAll.Click
        Sleep 1
    FindAndReplace.Close

   '/// check if the 2nd paragraph is in 2nd page
    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "2" then warnlog "Something wrong in 'Above paraghraph'! The end of 2nd paragraph should be 2nd page !"
    fCloseNavigator

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphIndent6

  Dim sTestFile          as String
  Dim sTest1             as String
  Dim sTest2             as String  '(sTest1 is head of paragraph 1 , sTest2 is end of paragraph2)
  Dim sSpacing           as String

  sTest1    = "He1" : sTest2 = "endof2ndparagraph"
  sSpacing  = "10" + gSeperator + "00" + gMeasurementUnit
  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\indent.sxw")

  PrintLog "- Set values for 'Below paraghraph'"
  '/// <b>Set values for "Below paraghraph"</b>

   Call hNewDocument

   '/// Open the test file
   '/// This file includes 2 paragraphs , and the end of 2nd paragraph is in 1st page
    hFileOpenLocally( sTestFile )
    

   'Set the focus to 1st paragraph
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
        SearchFor.SetText sTest1
        Sleep 1
        SearchAll.Click
        Sleep 1
    FindAndReplace.Close

   '/// Format/Paragraph / Indents & Spacing , set 10cm for "Below paraghraph"
    Call fFormatParagraph("TabEinzuegeUndAbstaende")
        Unten.SetText sSpacing
    TabEinzuegeUndAbstaende.OK

   'Set the focus to the end of the 2nd paragraph
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
        SearchFor.SetText sTest2
        Sleep 1
        SearchAll.Click
        Sleep 1
    FindAndReplace.Close

   '/// check if the 2nd paragraph is in 2nd page
    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "2" then warnlog "Something wrong in 'Above paraghraph'! The end of 2nd paragraph should be 2nd page !"
    fCloseNavigator

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphIndent7

  Dim sTestFile                as String
  Dim sEndOfPara1              as String
  Dim sStartPara2              as String
  Dim sEndPara2                as String
  Dim sParagraph2              as String

  sTestFile   = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\spacing.sxw")
  sEndOfPara1 = "endpara1"
  sStartPara2 = "StartPara2" : sEndPara2 = "endofpara2"
  sParagraph2 = "StartPara2mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmendofpara2"

  PrintLog "- Test above and below spacing of successive paragraphs are not added up, the biggest one should be displayed"
  '/// <b>Test above and below spacing of successive paragraphs are not added up, the biggest one should be displayed"</b>

   Call hNewDocument

   '/// Open the test file
   '/// This file includes 1 paragraph which is design like
   '/// spacing above : 1cm , spacing below : 2cm
   '/// the checkbox ��Add spacing between paragraphs and tables�� in
   '///      Tools -> Options... -> Text Document -> Compatibility  is checked
   '/// if the spacing is adopted 1cm ,2nd paragrahp will be in 1st page
   '/// if the spacing is adopted 2cm ,2nd paragrahp part will be in 1st page , part will be in 2nd page
   '/// if the spacing is adopted 3cm ,2nd paragrahp will be in 2nd page
    hFileOpenLocally( sTestFile )
    

   'Set the focus to the end of the 1st paragraph
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
        SearchFor.SetText sEndOfPara1
        Sleep 1
        SearchAll.Click
        Sleep 1
    FindAndReplace.Close

    Call wTypeKeys "<End>"
    Sleep 1

    Call wTypeKeys "<Return>"
    Sleep 1

   '/// insert 2nd paragraph
    Call wTypeKeys sParagraph2
    Sleep 1

   '/// check if the head of paragraph2 is in 1st page and
   '/// + the end of paragraph2 is in 2nd page
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
        SearchFor.SetText sStartPara2
        Sleep 1
        SearchAll.Click
        Sleep 1
    FindAndReplace.Close

    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "1" then warnlog "The start of paragraph2 should be in 1st page !"
    fCloseNavigator

    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
        SearchFor.SetText sEndPara2
        Sleep 1
        SearchAll.Click
        Sleep 1
    FindAndReplace.Close

    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "2" then warnlog "The end of paragraph2 should be in 2nd page !"
    fCloseNavigator

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphIndent8

  Dim sTestFile                as String
  Dim sEndOfPara1              as String
  Dim sStartPara2              as String
  Dim sParagraph2              as String

  sTestFile   = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\spacing1.sxw")
  sEndOfPara1 = "endpara1"
  sStartPara2 = "StartPara2"
  sParagraph2 = "StartPara2mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmendofpara2"

  PrintLog "- Test above and below spacing of successive paragraphs are added up"
  '/// <b>Test above and below spacing of successive paragraphs are added up</b>

   Call hNewDocument

   '/// Open the test file
   '/// This file includes 1 paragraph which is design like
   '/// spacing above : 1cm , spacing below : 2cm
   '/// the checkbox "Add spacing between paragraphs and tables" in
   '///      Tools -> Options... -> Text Document -> Compatibility  is Unchecked
   '/// if the spacing is adopted 1cm ,2nd paragrahp will be in 1st page
   '/// if the spacing is adopted 2cm ,2nd paragrahp part will be in 1st page , part will be in 2nd page
   '/// if the spacing is adopted 3cm ,2nd paragrahp will be in 2nd page
    hFileOpenLocally( sTestFile )
    

   'Set the focus to the end of the 1st paragraph
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
        SearchFor.SetText sEndOfPara1
        Sleep 1
        SearchAll.Click
        Sleep 1
    FindAndReplace.Close

    Call wTypeKeys "<End>"
    Sleep 1

    Call wTypeKeys "<Return>"
    Sleep 1

   '/// insert 2nd paragraph
    Call wTypeKeys sParagraph2
    Sleep 1

   '/// check if the paragraph2 is in 2nd page
    Kontext
    EditSearchAndReplace
    Kontext "FindAndReplace"
        SearchFor.SetText sStartPara2
        Sleep 1
        SearchAll.Click
        Sleep 1
    FindAndReplace.Close

    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "2" then warnlog "The start of paragraph2 should be in 2nd page !"
    fCloseNavigator

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphIndent9

  Dim sTestFile as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\eightline.sxw")

  PrintLog "- Test Line spacing - Single"
  '/// <b>Test Line spacing - Single</b>

   Call hNewDocument

   '/// Open the test file
   '/// This file includes 8 lines ,
   '/// + when the spacing is single line , the 8 lines should be in 1 page as design.
    hFileOpenLocally( sTestFile )
    

   '/// Format/Paragraph / Indents & Spacing , select single line in Line spacing
    Call fFormatParagraph("TabEinzuegeUndAbstaende")
        Zeilenabstand.Select 1
    TabEinzuegeUndAbstaende.OK

   'Set the focus to the end of the paragraph
    Call wTypeKeys "<MOD1 End>"
    Sleep 1

   '/// check if the last line is in 1 page (it should be)
    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "1" then warnlog "The end of paragraph should be in 1st page !"
    fCloseNavigator

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphIndent10

  Dim sTestFile as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\eightline.sxw")

  PrintLog "- Test Line spacing - 1.5 lines"
  '/// <b>Test Line spacing - 1.5 lines</b> ///

   Call hNewDocument

   '/// Open the test file
   '/// This file includes 8 lines
   '/// when the spacing is 1.5 lines ,
   '/// + the last 2(or 3) lines should be in 2nd page.
    hFileOpenLocally( sTestFile )
    

   '/// Format/Paragraph / Indents & Spacing , select 1.5 lines in Line spacing
    Call fFormatParagraph("TabEinzuegeUndAbstaende")
        Zeilenabstand.Select 2
        Sleep 1
    TabEinzuegeUndAbstaende.OK

   'Set the focus to the end of the paragraph
    Call wTypeKeys "<MOD1 End>"
    Sleep 1

    Call wTypeKeys "<Home>"
    Sleep 1

   '/// check if the last 2 lines(7th and 8th) is in 2nd page (it should be)
    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "2" then warnlog "The 8th line should be in 2nd page !"
    fCloseNavigator

    Call wTypeKeys "<Up>"
    Sleep 1
    Call wTypeKeys "<Home>"
    Sleep 1

    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "2" then warnlog "The 7th line should be in 2nd page !"
    fCloseNavigator

    Call wTypeKeys "<Up>" , 2
    Sleep 1
    Call wTypeKeys "<Home>"
    Sleep 1

   '/// check if the 5th lines is in 1st page (it should be)
    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "1" then warnlog "The 5th line should be in 1st page !"
    fCloseNavigator

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphIndent11

    Dim sTestFile as String
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\eightline.sxw")
    
    PrintLog "- Test Line spacing - Double lines"
    '/// <b>Test Line spacing - Double lines</b>
    
    Call hNewDocument
    Call wOptionsUndo ( "Grundschriften" )
    '/// Open the test file
    '/// This file includes 8 lines
    '/// when the spacing is Double lines , the first 4 lines should be in 1st page
    '/// + the last 4 lines should be in 2nd page.
    hFileOpenLocally( sTestFile )
    
    
    '/// Format/Paragraph / Indents & Spacing , select Double lines in Line spacing
    Call fFormatParagraph("TabEinzuegeUndAbstaende")
    Zeilenabstand.Select 3
    TabEinzuegeUndAbstaende.OK
    
    'Set the focus to the end of the paragraph
    Call wTypeKeys "<MOD1 End>"
    Sleep 1
    
    Call wTypeKeys "<Home>"
    Sleep 1
    
    '/// check if the last 4 lines is in 2nd page (it should be)
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "2" then warnlog "The 8th line should be in 2nd page !"
    fCloseNavigator
    
    Call wTypeKeys "<Up>" , 3
    Sleep 1
    
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "2" then warnlog "The 5th line should be in 2nd page !"
    fCloseNavigator
    
    Call wTypeKeys "<Up>"
    Sleep 1
    
    '/// check if the first 4 lines is in 1st page (it should be)
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "1" then warnlog "The 4th line should be in 1st page !"
    fCloseNavigator
    
    'Set the focus to the end of the paragraph
    Call wTypeKeys "<MOD1 End>"
    Sleep 1
    
    Call wTypeKeys "m"
    Kontext "Navigator"
    if Not Navigator.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    Sleep 1
    if Seitennummer.GetText <> "3" then warnlog "The 9th line should be in 3rd page !"
    fCloseNavigator
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphIndent12

  Dim sTestFile     as String
  Dim sProportional as String

  sTestFile     = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\eightline.sxw")
  sProportional = "130%"

  PrintLog "- Test Line spacing - Proportional"
  '/// <b>Test Line spacing - Proportional</b>

   Call hNewDocument

   '/// 1.Test the max of Proportional is 200%
   '/// Open the test file
   '/// This file includes 8 lines
   '/// when the spacing Proportional is more than 200% ,
   '/// + then it will change to be Double line automatically
    hFileOpenLocally( sTestFile )
    

   '/// Format/Paragraph / Indents & Spacing ,
   '/// + select Proportional in Line spacing , and input 300%
    Call fFormatParagraph("TabEinzuegeUndAbstaende")
        Zeilenabstand.Select 4
        Sleep 2
        Um.SetText "300%"
    TabEinzuegeUndAbstaende.OK

    Call fFormatParagraph("TabEinzuegeUndAbstaende")
        if Zeilenabstand.GetSelIndex <> 3 then warnlog "should get double line, but get " & Zeilenabstand.GetSelText(Zeilenabstand.GetSelIndex)
    TabEinzuegeUndAbstaende.Cancel

   '/// 2.Test when Proportional is 130%

   '/// Format/Paragraph / Indents & Spacing ,
   '/// + select Proportional in Line spacing , and input 130%
    Call fFormatParagraph("TabEinzuegeUndAbstaende")
        Zeilenabstand.Select 4
        Sleep 2
        Um.SetText sProportional
    TabEinzuegeUndAbstaende.OK

   'Set the focus to the end of the paragraph
    Call wTypeKeys "<MOD1 End>"
    Sleep 1

    Call wTypeKeys "<Home>"
    Sleep 1

   '/// check if the last 2 lines is in 2nd page (it should be)
    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "2" then warnlog "The 8th line should be in 2nd page !"
    fCloseNavigator

    Call wTypeKeys "<Up>" , 2
    Sleep 1

    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "1" then warnlog "The 6th line should be in 2nd page !"
    fCloseNavigator

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphIndent13

  Dim sTestFile  as String
  Dim sAtLeast   as String
  Dim sPositionY as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\eightLineAdd1Pic.sxw")
  sAtLeast  = "4" + gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test Line spacing - At least"
  '/// <b>Test Line spacing - At least</b>

   Call hNewDocument

   '/// Open the test file
   '/// This file includes 8 lines and 1 pic is anchored as character in 1st line
   '/// when the spacing is At least to 4cm, there should be 1 line/page
   '/// + and the pic should move down
    hFileOpenLocally( sTestFile )
    

   'Get the original coordinate-Y
   Call wTypeKeys ( "<Shift F4>" )
   Sleep 1
   FormatAnchorToPage
   Sleep 1
   Call fPositionAndSize("TabPositionAndSizeWriter")
   sPositionY = Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel
    FormatAnchorAsCharacter
    Sleep 1
    Call wTypeKeys "<ESCAPE>"
    Sleep 1

   '/// Format/Paragraph / Indents & Spacing , select At least in Line spacing and set 4cm
    Call fFormatParagraph("TabEinzuegeUndAbstaende")
        Zeilenabstand.Select 5
        Sleep 1
        Von.SetText sAtLeast
    TabEinzuegeUndAbstaende.OK

   'Set the focus to the end of the paragraph
    Call wTypeKeys "<MOD1 End>"
    Sleep 1

   '/// check if it is 1 line /page
    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "8" then Warnlog "The 8th line should be in 8th page !"
    fCloseNavigator

   '/// check if the pic moves
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    FormatAnchorToPage
    Sleep 1

    Call fPositionAndSize("TabPositionAndSizeWriter")
        if sPositionY = Verticalby.Gettext then warnlog "The pic should move !"
    TabPositionAndSizeWriter.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphIndent14

  Dim sTestFile  as String
  Dim sAtLeast   as String
  Dim sPositionY as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\eightLineAdd1Pic.sxw")
  sAtLeast  = "4" + gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test Line spacing - leading"
  '/// <b>Test Line spacing - leading</b>

   Call hNewDocument

   '/// Open the test file
   '/// This file includes 8 lines and 1 pic is anchored as character in 1st line
   '/// when the spacing is leading to 4cm, there should be 1 line/page
   '/// + and the pic should NOT move
    hFileOpenLocally( sTestFile )
    

   'Get the original coordinate-Y
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        sPositionY = Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel
    FormatAnchorAsCharacter
    Sleep 1
    Call wTypeKeys "<ESCAPE>"
    Sleep 1

   '/// Format/Paragraph / Indents & Spacing , select leading in Line spacing and set 4cm
    Call fFormatParagraph("TabEinzuegeUndAbstaende")
        Zeilenabstand.Select 6
        Sleep 1
        Von.SetText sAtLeast
    TabEinzuegeUndAbstaende.OK

   'Set the focus to the end of the paragraph
    Call wTypeKeys "<MOD1 End>"
    Sleep 1

   '/// check if it is 1 line /page
    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "8" then Warnlog "The 8th line should be in 8th page !"
    fCloseNavigator

   '/// check if the pic moves
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    FormatAnchorToPage
    Sleep 1

    Call fPositionAndSize("TabPositionAndSizeWriter")
        if sPositionY <> Verticalby.Gettext then warnlog "The pic should NOT move !"
    TabPositionAndSizeWriter.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphIndent15

  Dim sTestFile  as String
  Dim sFixed     as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\eightline.sxw")
  sFixed    = "0" + gSeperator + "50" + gMeasurementUnit

  PrintLog "- Test Line spacing - Fixed lines"
  '/// <b>Test Line spacing - Fixed lines</b>

   Call hNewDocument

   '/// Open the test file
   '/// + This file includes 8 lines , and font size is 12
    hFileOpenLocally( sTestFile )
    

   '/// Format/Paragraph / Indents & Spacing ,
   '/// + select Fixed lines in Line spacing
   '/// + set 0.5cm
    Call fFormatParagraph("TabEinzuegeUndAbstaende")
        Zeilenabstand.Select 7
        sleep 1
        Von.SetText sFixed
    TabEinzuegeUndAbstaende.OK

   '/// Format/Character / Font ,Change font size to 24
    Call wTypeKeys "<MOD1 a>"
    Sleep 1
    Call fFormatCharacter("TabFont")
        try
            SizeWest.Select "24"
        catch
            Size.Select "24"
        endcatch
    TabFont.OK

   '/// check if there are still 8 lines/page and there
   '/// + should be 2 pages
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Down>" , 7

    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "1" then warnlog "The 8th line should be in 1st page !"
    Call fCloseNavigator

    Call wTypeKeys "<Down>"

    Kontext "Navigator"
        if Not Navigator.Exists then ViewNavigator
        Kontext "NavigatorWriter"
        Sleep 1
        if Seitennummer.GetText <> "2" then warnlog "The 9th line should be in 2nd page !"
    Call fCloseNavigator

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphIndent16

  Dim sTestFile    as String
  Dim sPositionX as String
  Dim sPositionY as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\registerTrue.sxw")

  PrintLog "- Test Regester-true Active"
  '/// <b>Test Regester-true Active</b>

   Call hNewDocument

   '/// Open the test file .
   '/// This file includes 2 paragraphs ,
   '/// + and 2 columns , in format/page / page ,
   '/// + regester-true has been checked as text body.
   '/// + You will find out the "He2 heard quiet steps"
   '/// + isn't at the same line with the context left it .
   '/// + There is a pic anchored "He2" .
    hFileOpenLocally( sTestFile )
    

   'Get the original location for the pic
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        sPositionX = Horizontalby.Gettext
        sPositionY = Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel

    FormatAnchorAsCharacter
    Sleep 1
    Call wTypeKeys "<ESCAPE>"
    Sleep 1
    Call wTypeKeys "<MOD1 A>"

   '/// Format/Paragraph / Indents & Spacing ,
   '/// + check Regester-true Active
    Call fFormatParagraph("TabEinzuegeUndAbstaende")
        Registerhaltigkeit.Check
        Sleep 1
    TabEinzuegeUndAbstaende.OK

   '/// Check if  "He2 heard quiet steps" is at the same line with the context left it.
    Call wTypeKeys ( "<Shift F4>" )
    Sleep 1
    FormatAnchorToPage
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
        if sPositionX <> Horizontalby.Gettext then warnlog "Position X should NOT be changed !"
        if sPositionY =  Verticalby.Gettext then warnlog "Position Y should be changed !"
    TabPositionAndSizeWriter.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphIndent17

	Dim sTestFile1 as String
	Dim sTestFile2 as String
	
	sTestFile1 = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\AddSpacingShouldCheck.doc")
	sTestFile2 = Convertpath (gTesttoolpath + "writer\optional\input\formatparagraph\AddSpacingShouldNotCheck.doc")
	
	PrintLog "- Test 'Add spacing between paragarph and talbes' when importing from MS-Word"
	'/// <b>Test 'Add spacing between paragarph and talbes' when importing from MS-Word</b>
	
	Call hNewDocument
	
	'/// 1.Open the test file - AddSpacingShouldCheck.doc
	'///   In Tools/Optionis/Text Document/Compatibility,
	'/// + the 'Add spacing between paragarph and talbes'
	'/// + should be checked
	hFileOpenLocally( sTestFile1)
	
	
	ToolsOptions
	Call hToolsOptions("WRITER","COMPATIBILITY")
	Kontext "TabCompatibility"
	Sleep 2
	Options.TypeKeys "<Down>"
	if Options.IsChecked <> TRUE then
		warnlog "'Add spacing between paragarph and talbes' should be checked"
	end if
	Kontext "ExtrasOptionenDlg"
	ExtrasOptionenDlg.OK
	
	Call hCloseDocument
	
	Call hNewDocument
	
	'/// 2.Open the test file - AddSpacingShouldNotCheck.doc
	'///   In Tools/Optionis/Text Document/Compatibility,
	'/// + the 'Add spacing between paragarph and talbes'
	'/// + should NOT be checked
	hFileOpenLocally( sTestFile2)
	
	
	ToolsOptions
	Call hToolsOptions("WRITER","COMPATIBILITY")
	Kontext "TabCompatibility"
	Sleep 2
	Options.TypeKeys "<Down>"
	if Options.IsChecked = TRUE then
		warnlog "'Add spacing between paragarph and talbes' should NOT be checked"
	end if
	Kontext "ExtrasOptionenDlg"
	ExtrasOptionenDlg.OK
	
	Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

endcase

'-------------------------------------------------------------------------------------------

testcase tParagraphIndent18

  Dim sPageWidth                          as String
  Dim sMarginLeft                         as String
  Dim sMarginRight                        as String
  Dim sBeforeText                         as String
  Dim sAfterText                          as String

  sPageWidth   = "8"   + gSeperator + "00" + gMeasurementUnit
  sMarginLeft  = "2"   + gSeperator + "00" + gMeasurementUnit
  sMarginRight = "2"   + gSeperator + "00" + gMeasurementUnit
  sBeforeText  = "-10" + gSeperator + "00" + gMeasurementUnit
  sAfterText   = "-12" + gSeperator + "00" + gMeasurementUnit

  PrintLog "- Test Negativ indent"
  '/// <b>Test Negativ indent</b> ///

   Call hNewDocument

   '/// Format/Page , tabpage Page , set page size and margin size ///
    Call fFormatPageWriter("TabSeite")
        Breite.SetText  sPageWidth
        Links.SetText   sMarginLeft
        Rechts.SetText  sMarginRight
    TabSeite.OK

   '/// Format/Paragraph / Indents & Spacing , ///
   '/// + set -10cm in 'before text' ///
   '/// + set -12cm in 'after text'  ///
    Call fFormatParagraph("TabEinzuegeUndAbstaende")
        Vonlinks.SetText  sBeforeText
        VonRechts.SetText sAfterText
    TabEinzuegeUndAbstaende.OK

   '/// Check if the configuration is right ///
    Call fFormatParagraph("TabEinzuegeUndAbstaende")
        if Vonlinks.GetText  <> sBeforeText then warnlog "Before text is wrong , get :" &Vonlinks.GetText
        if VonRechts.GetText <> sAfterText  then warnlog "After text is wrong , get :" &VonRechts.SetText
    TabEinzuegeUndAbstaende.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------------------------

