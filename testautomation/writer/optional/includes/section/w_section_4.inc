'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_section_4.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: fredrikh $ $Date: 2008-06-18 15:03:58 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
'*
'* owner : fredrik.haegg@sun.com
'*
'* short description :
'*
'************************************************************************
'*
' #1 tSectionInTables_1
' #1 tSectionInTables_2
' #1 tSectionInTables_3
' #1 tSectionInTables_4
' #1 tSectionInTables_5
' #1 tSectionInTables_6
' #1 tSectionInTables_7
' #1 tSectionInFootnotes_1
' #1 tSectionInFootnotes_2
' #1 tEditSection_1
' #1 tEditSection_2
' #1 tEditSection_3
' #1 tEditSection_4
' #1 tEditSection_5
' #1 tParagraphSection_1
' #1 tParagraphSection_2
' #1 tParagraphSection_3
' #1 tScenarioSection_1
' #1 tScenarioSection_2
' #1 tCTLSection_1
' #1 tCTLSection_2
'*
'\***********************************************************************

sub w_section_4

   printLog Chr(13) + "- Sections in special areas -"

   printLog Chr(13) + "--- Sections in tables ---"
     Call tSectionInTables_1          'insert section in empty cell
     Call tSectionInTables_2          'select text in a cell and insert section
     Call tSectionInTables_3          'insert multicolumn-section into cell
     Call tSectionInTables_4          'insert footnote into section which is in table
     Call tSectionInTables_5          'hide section in cell, resize cell via key shortcuts, make section visible again
     Call tSectionInTables_6          'insert a section when you have a multi-selection in a table must be impossible
     Call tSectionInTables_7          'insert a section when cells are selected must be impossible

   printLog Chr(13) + "--- Sections in footnote areas  ---"
     Call tSectionInFootnotes_1       'insert a simple section over some words or lines
     Call tSectionInFootnotes_2       'insert section into footnote that spreads over several pages

   printLog Chr(13) + "- Edit sections later on (via Format - Sections...) -"
     Call tEditSection_1              'uncheck "Autowidth" and vary width manually
     Call tEditSection_2              'hide section
     Call tEditSection_3              'protect section
     Call tEditSection_4              'remove link(s), also via Edit - Links
     Call tEditSection_5              'Remove section

   printLog Chr(13) + "- section breaks (several following sections) -"
     Call tParagraphSection_1         'Keep paragraphs together
     Call tParagraphSection_2         'paragraph spacing
     Call tParagraphSection_3         'paragraph borders

   printLog Chr(13) + "- Test scenarios -"
     Call tScenarioSection_1          'create Masterdocument from another document that also contains sections
     Call tScenarioSection_2          'create Masterdocument from another document that also contains sections and footnotes

   printLog Chr(13) + "- CTL specials -"
     Call tCTLSection_1               'CTL specials -- insert/sections /Columns
     Call tCTLSection_2               'CTL specials -- format/section/options /Columns

end sub

'***********************************************************************

testcase tSectionInTables_1

  Dim tableName, columnNumber, rowNumber as String
  Dim sectionName                        as String

  tableName     = "testTable"
  columnNumber  = "4"
  rowNumber     = "2"

  sectionName   = "tableSection"

  '/// insert section in empty cell ///
   PrintLog "      - insert section in empty cell  "

    Call hNewDocument

   '/// insert a table
    Call fInsertTable(tableName , columnNumber, rowNumber)

   '/// insert section in empty cell
    try
        Call fInsertSection(sectionName)

      ' Check the section name is correct or NOT (Format/Section)
        Kontext
        FormatSections
        Kontext "BereicheBearbeiten"
        if BereichsName.GetText <> sectionName then warnlog "The section Name is wrong : " & BereichsName.Gettext
        BereicheBearbeiten.Cancel
    catch
        Warnlog "insert section in empty cell failed"
    endcatch

    Call hCloseDocument

endcase

'-------------------------------------------------------------

testcase tSectionInTables_2

  Dim tableName, columnNumber, rowNumber , testText as String
  Dim sectionName                                   as String

  tableName     = "testTable"
  columnNumber  = "4"
  rowNumber     = "2"
  testText      = "Table-Section"

  sectionName   = "tableSection"

  '/// select text in a cell and insert section  ///
   PrintLog "      - select text in a cell and insert section   "

    Call hNewDocument

   '/// insert a table
    Call fInsertTable(tableName , columnNumber, rowNumber)

   '/// input some test text in the table
    Call wDokSchreiben testText

   '/// select text
    Call wDokSchreiben "<Shift Home>"

   '/// insert section include select text
    try
        Call fInsertSection(sectionName)

      ' Check the section name is correct or NOT (Format/Section)
        Kontext
        FormatSections
        Kontext "BereicheBearbeiten"
        if BereichsName.GetText <> sectionName then warnlog "The section Name is wrong : " & BereichsName.Gettext
        BereicheBearbeiten.Cancel

      ' Check the content in section is correct or NOT
        Call wDokSchreiben "<Home>"
        Call wDokSchreiben "<Shift End>"
        EditCopy
        if GetClipBoardText <> testText then Warnlog "The content in section is NOT right : " +GetClipBoardText

    catch
        Warnlog "insert section which include text failed"
    endcatch

    Call hCloseDocument

endcase

'---------------------------------------------------------------

testcase tSectionInTables_3

  Dim tableName   , columnNumber   , rowNumber as String
  Dim sectionName , columnInSection            as String

  tableName       = "testTable"
  columnNumber    = "4"
  rowNumber       = "2"
  sectionName     = "tableSection"
  columnInSection = "3"

  '/// insert multicolumn-section into cell  ///
   PrintLog "      - insert multicolumn-section into cell  "

    Call hNewDocument

   '/// insert a table
    Call fInsertTable(tableName , columnNumber, rowNumber)

   '/// insert section with column = columnInSection in empty cell
    try
        InsertSection
        Sleep 1

        Kontext
        Active.Setpage TabBereiche
        Kontext "TabBereiche"
        Bereichsliste.Settext sectionName

        Kontext
        Active.Setpage TabSpalten
        Kontext "TabSpalten"
        Anzahl.Settext columnInSection
        TabSpalten.OK
        sleep 1

      ' Check the section name is correct or NOT (Format/Section)
        Kontext
        FormatSections
        Kontext "BereicheBearbeiten"
        if BereichsName.GetText <> sectionName then warnlog "The section Name is wrong : " & BereichsName.Gettext

      ' Check the section column is correct or NOT (options button)
        Optionen.Click
        Kontext
        Active.Setpage TabSpalten
        Kontext "TabSpalten"
        if Anzahl.Gettext <> columnInSection then  Warnlog "The volumn should be " +columnInSection +" but get " + Anzahl.Gettext
        TabSpalten.Cancel

        Kontext
        Kontext "BereicheBearbeiten"
        BereicheBearbeiten.Cancel
    catch
        Warnlog "insert section in empty cell failed"
    endcatch

    Call hCloseDocument

endcase

'------------------------------------------------------------------------

testcase tSectionInTables_4

  Dim tableName   , columnNumber , rowNumber as String
  Dim sectionName , footNote     , endNote   as String

  tableName     = "testTable"
  columnNumber  = "4"
  rowNumber     = "2"

  sectionName   = "tableSection"
  footNote      = "Foot"
  endNote       = "End"

  '///insert footnote into section which is in table  ///
   PrintLog "      - insert footnote into section which is in table "

   Call hNewDocument

   '/// insert a table
    Call fInsertTable(tableName , columnNumber, rowNumber)

    try
       '/// insert section
        Call fInsertSection(sectionName)

        Call wDokSchreiben "<Up>"

       '/// insert a footnote
        Call fInsertFootnote("character",footNote)

        Call wDokSchreiben "<PageUp>"

       '/// insert a endnote
        fInsertEndnote("character",endNote)

        Call wDokSchreiben "<PageUp>"

       '/// Check the section name is correct or NOT (Format/Section)
        Kontext
        FormatSections
        Kontext "BereicheBearbeiten"
        if BereichsName.GetText <> sectionName then warnlog "The section Name is wrong : " & BereichsName.Gettext
        BereicheBearbeiten.Cancel

       '/// Check the footnote and endnote are correct or NOT
        Call wDokSchreiben "<Shift Home>"
        EditCopy
        if GetClipBoardText <> footNote+endNote then Warnlog "The footnote/endnote are NOt inserted into section correctly!"
    catch
        Warnlog "insert section in empty cell failed"
    endcatch

    Call hCloseDocument

endcase

'-----------------------------------------------------------------------------

testcase tSectionInTables_5
    Dim tableName, columnNumber, rowNumber , testText as String
    Dim sectionName as String

    tableName     = "testTable"
    columnNumber  = "4"
    rowNumber     = "2"
    testText      = "Hide-Section"
    sectionName   = "tableSection"

    '/// Hide section in cell, resize cell via key shortcuts, make section visible again   ///
    printlog "      - hide section in cell, resize cell via key shortcuts, make section visible again"
    Call hNewDocument

     '/// insert a table
     Call wDokSchreiben "<RETURN>"
     Call fInsertTable(tableName , columnNumber, rowNumber)
     '/// input some test text , select text and insert a section with hide option
     Call wDokSchreiben testText
     Call wDokSchreiben "<SHIFT HOME>"
     try
         InsertSection
         Kontext
             Active.Setpage TabBereiche
             Kontext "TabBereiche"
                 Bereichsliste.Settext sectionName
                 Ausblenden.check
             TabBereiche.OK
             sleep(1)
         '/// Check the content in section is hidden or NOT
         SetClipboard = ""
         Call wDokSchreiben "<LEFT><DOWN>"
         Call wDokSchreiben "<SHIFT END>"
         try
             EditCopy
         catch
             printlog "There is no text in the document. Good."
         endcatch
         if GetClipboardText = "" then
             '/// Resize cell via key shortcuts
             Call wDokSchreiben "<MOD1 MOD2 RIGHT>" , 2
             '/// UnCheck hide option (Format/Section)
             Kontext
             FormatSections
             Kontext "BereicheBearbeiten"
                 Ausblenden.Uncheck
             BereicheBearbeiten.OK
             sleep(2)
             Call wDokSchreiben "<LEFT><HOME>"
             Call wDokSchreiben "<SHIFT END>"
             EditCopy
             if GetClipBoardText <> testText then warnlog "The content in section should be visible but get : " & GetClipBoardText
         else
             QAErrorLog "#99460# The content in section is NOT hidden!"
         end if
     catch
         warnlog "insert section which include text failed"
     endcatch

    Call hCloseDocument
endcase

'------------------------------------------------------------------

testcase tSectionInTables_6

  Dim tableName, columnNumber, rowNumber , testText , selectedString as String
  Dim sectionName                                                    as String

  tableName       =  "testTable"
  columnNumber    =  "4"
  rowNumber       =  "2"
  testText        =  "Mul-s-Mul"
  selectedString  =  "Mul"

  sectionName     =  "tableSection"

  '/// insert a section when you have a multi-selection in a table must be impossible  ///
   PrintLog "      - insert a section when you have a multi-selection in a table must be impossible"

    Call hNewDocument

   '/// insert a table
    Call fInsertTable(tableName , columnNumber, rowNumber)

   '/// input some words and do multi-selection
    Call wDokSchreiben testText
    fMultiSelection(selectedString)

   '/// insert section
    try
        Call fInsertSection(sectionName)
        Warnlog "Create section should be impossible!"
    catch
    endcatch

    Call hCloseDocument

endcase

'---------------------------------------------------------------

testcase tSectionInTables_7

  Dim tableName, columnNumber, rowNumber  as String
  Dim sectionName                         as String

  tableName       =  "testTable"
  columnNumber    =  "4"
  rowNumber       =  "2"

  sectionName     =  "tableSection"

  '/// insert a section when cells are selected must be impossible  ///
   PrintLog "      - insert a section when cells are selected must be impossible"

    Call hNewDocument

   '/// insert a table
    Call fInsertTable(tableName , columnNumber, rowNumber)

   '/// more than one cell are selected
    Call wDokSchreiben "<Shift Right>",2

   '/// insert section
    try
        Call fInsertSection(sectionName)
        Warnlog "Create section should be impossible!"
    catch
    endcatch

    Call hCloseDocument

endcase

'----------------------------------------------------------------

testcase tSectionInFootnotes_1

  Dim sectionName  as String

  sectionName = "footnoteSection"

  '/// insert a simple section over some words or lines ///
   PrintLog "      - insert a simple section over some words or lines "

   Call hNewDocument

   '/// + insert footnote
    Call fInsertFootnote("automatic")

   '/// + insert some dummy text in the footnote area
    wBlindtextEinfuegen

    EditSelectAll

   '/// + insert a section for selected words
    try
        Call fInsertSection(sectionName)

      ' Check the section name is correct or NOT (Format/Section)
        Kontext
        FormatSections
        Kontext "BereicheBearbeiten"
        if BereichsName.GetText <> sectionName then warnlog "The section Name is wrong : " & BereichsName.Gettext
        BereicheBearbeiten.Cancel
    catch
        Warnlog "insert a section failed !"
    endcatch

   Call hCloseDocument

endcase

'--------------------------------------------------------------------

testcase tSectionInFootnotes_2

  Dim sectionName  as String

  sectionName = "footnoteSection"

  '/// insert section into footnote that spreads over several pages ///
   PrintLog "      - insert section into footnote that spreads over several pages "

   Call hNewDocument

   '/// insert footnote
    Call fInsertFootnote("automatic")
    sleep(2)
   '/// insert some dummy text that spreads over several pages in the footnote area
    Call wBlindtextEinfuegen
    Call wBlindtextEinfuegen
    Call wBlindtextEinfuegen
    Call wBlindtextEinfuegen
    Call wBlindtextEinfuegen
    Call wBlindtextEinfuegen
    Call wBlindtextEinfuegen

    Kontext
    EditNavigator
    sleep(1)
    Kontext "NavigatorWriter"

    '/// Check if dummy text is enough in footnote area
     if Seitennummer.GetText > "1" then
        EditSelectAll
        sleep(1)
        '/// + insert a section for selected words
         try
             Call fInsertSection(sectionName)
             ' Check the section name is correct or NOT (Format/Section)
               Kontext
               FormatSections
               Kontext "BereicheBearbeiten"
               if BereichsName.GetText <> sectionName then warnlog "The section Name is wrong : " & BereichsName.Gettext
               BereicheBearbeiten.Cancel
        catch
               Warnlog "insert a section failed !"
        endcatch
     else
         QAErrorLog "Not enough dummy text in the footnote area!"
     endif
    Call fCloseNavigator
   Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tEditSection_1

  Dim columnNumber ,  sectionName  as  String
  Dim width1       ,  width2       as  String

  sectionName   =  "MultiColumnSection"
  columnNumber  =  "3"

  '/// uncheck "Autowidth" and vary width manually ///
   PrintLog "   - Create multi-column section from format/section - options (TabPage Columns)"

   Call hNewDocument

  '/// insert a section with only 1 culumn, that means a normal section
   Call fInsertSection(sectionName)
   Call wDokSchreiben "<Up>"

  '/// change the column number (format/section) with
  '/// + TabPage "Columns", set column to $columnNumber
   Kontext
   FormatSections
   Kontext "BereicheBearbeiten"
   Optionen.Click

    Kontext
    Active.Setpage TabSpalten
    Kontext "TabSpalten"

   '/// + Set column number
    Anzahl.Settext columnNumber
    Sleep 1

   'This step is useless , but must be done ! Because after the aboved step, all control are NOT actived !
    Kontext
    Active.Setpage TabHintergrund
    Kontext "TabHintergrund"

    Kontext
    Active.Setpage TabSpalten
    Kontext "TabSpalten"

   '/// uncheck "Autowidth" and vary width manually
    AutomBreite.UnCheck

    width1 = "4" + gSeperator + "00" + gMeasurementUnit
    width2 = "3" + gSeperator + "00" + gMeasurementUnit

    Spaltenbreite1.Settext width1
    Sleep 1
    Spaltenbreite2.Settext width2
    Sleep 1
    TabSpalten.TypeKeys "<Tab>"
    Sleep 1

    TabSpalten.OK

    Kontext
    Kontext "BereicheBearbeiten"
    BereicheBearbeiten.OK

    Call wDokSchreiben "<Up>"
   '/// Check the column's width
    Kontext
    FormatSections
    Kontext "BereicheBearbeiten"
    Optionen.Click

    Kontext
    Active.Setpage TabSpalten
    Kontext "TabSpalten"

    if Spaltenbreite1.Gettext <> width1 then Warnlog "column1 width is NOT correct , hope to get " + width1 + " but get " +Spaltenbreite1.Gettext
    if Spaltenbreite2.Gettext <> width2 then Warnlog "column2 width is NOT correct , hope to get " + width2 + " but get " +Spaltenbreite2.Gettext

    TabSpalten.Cancel

    Kontext
    Kontext "BereicheBearbeiten"
    BereicheBearbeiten.Cancel

    Call hCloseDocument

endcase

'------------------------------------------------------

testcase tEditSection_2

    Dim sectionName  , testText  as String

    sectionName   =  "HideSection"
    testText      =  "Hide Section"

    '/// hide section in format/section ///
    printlog "   - hide section in format/section"

    Call hNewDocument

     '/// insert a normal section, and input some words
     Call wDokSchreiben "<RETURN>"
     Call wDokSchreiben testText
     Call WDokSchreiben "<SHIFT HOME>"
     Call fInsertSection(sectionName)
     '/// Edit section with Format/Section and choose 'Hide'
     Kontext
         FormatSections
         Kontext "BereicheBearbeiten"
             Ausblenden.check
             BereicheBearbeiten.OK
             sleep(1)
     '/// check the section and its contents
     printlog "  -- check the section and its contents "
     try
         EditSelectAll
         EditCopy
         warnlog "'Edit / Copy' is usable but if there is nothing in the document (hidden section) it should be disabled!"
     catch
         printlog "Clipboard should be and is empty."
     endcatch
    Call hCloseDocument
endcase

'-------------------------------------------------------------------

testcase tEditSection_3

  Dim sectionName  , testText  as String

  sectionName   =  "ProtectSection"
  testText      =  "Protect Section"

  '/// protect section in format/section ///
   PrintLog "   - protect section in format/section"

    Call hNewDocument

  '/// Open options and check 'Direct-Cursor' in options ///
    fCheckDirectCursor

  '/// insert a normal section, and input some words
    Call wDokSchreiben testText
    Call WDokSchreiben "<Shift Home>"

    Call fInsertSection(sectionName)

  '/// Edit section with Format/Section and choose 'Protect'
    Kontext
    FormatSections
    Kontext "BereicheBearbeiten"
    Geschuetzt.Check
    BereicheBearbeiten.OK
    sleep 1

    Call wDokSchreiben "<Home>"
    Call wDokSchreiben "<Space>"
    Kontext "Active"
    If Active.Exists then
      Active.ok
    else
      Warnlog "-  No warning when trying to write in protected area!"
    endif

    Call hCloseDocument

endcase

'--------------------------------------------------------

testcase tEditSection_4

  Dim sectionName  , testText  as String

  sectionName   =  "LinkSection"
  testText      =  "Link Section"

  if Dir (gOfficePath + "user\work\tEditSection_4.odt") <> "" then kill (gOfficePath + "user\work\tEditSection_4.odt")

  '/// remove link(s), also via Edit - Links ///
   PrintLog "   - remove link(s), also via Edit - Links "

  '/// edit a test file and save it
   Call hNewDocument
   Call wDokSchreiben testText
   Call hFileSaveAsKill (gOfficePath + "user\work\tEditSection_4")
   Call hCloseDocument

   Call hNewDocument

  '/// insert a section and link the test file
   InsertSection
   Sleep 1
   Kontext
   Active.Setpage TabBereiche
   Kontext "TabBereiche"
   Bereichsliste.Settext sectionName
   Verknuepfung.Check

   DateiVerknuepfung.Settext ConvertPath(gOfficePath + "user\work\tEditSection_4.odt")
   TabBereiche.OK
   Sleep 1

  '/// Edit section with Format/Section and unCheck 'link'
   Kontext
   FormatSections
   Kontext "BereicheBearbeiten"
   Verknuepfung.UnCheck
   BereicheBearbeiten.OK
   sleep 1

  '/// the test file's content should be reserved after
  '/// + unlinking the test file
   EditSelectAll
   EditCopy
   if GetClipBoardText <> testText then Warnlog "the test file's content should be reserved after unlinking the test file"

  '/// check if the unlink is taken effect
   Kontext
   FormatSections
   Kontext "BereicheBearbeiten"
   If Verknuepfung.IsChecked = TRUE then Warnlog "The link option should be UnChecked!"
   BereicheBearbeiten.Cancel

   Call hCloseDocument

endcase

'-------------------------------------------------------

testcase tEditSection_5

  Dim sectionName  , testText  as String

  sectionName   =  "RemoveSection"
  testText      =  "Remove Section Test"

  '/// remove section via Edit - undo ///
   PrintLog "   - remove section via Edit - undo"

   Call hNewDocument

  '/// insert a normal section, and input some words
    Call wDokSchreiben testText
    Call WDokSchreiben "<Shift Home>"
    Call fInsertSection(sectionName)

  '/// remove section with Format/Section and click undo button
   Kontext
   FormatSections
   Kontext "BereicheBearbeiten"

  'click undo button
   Aufheben.Click
   Sleep 1
   BereicheBearbeiten.OK
   sleep 1

  'check if section is still existed
   try
       Kontext
       FormatSections
       Kontext "BereicheBearbeiten"
       BereicheBearbeiten.Cancel
       Warnlog "There are should be no section existed!"
   catch
   endcatch

  '/// the test file's content should be reserved after removing the section
   EditSelectAll
   EditCopy
   if GetClipBoardText <> testText then Warnlog "the test file's content should be reserved after removing the section"

   Call hCloseDocument

endcase

'------------------------------------------------

testcase tParagraphSection_1

  Dim documentName , sectionName , hopeResult as String

   documentName  =  ConvertPath(gTesttoolpath + "writer\optional\input\section\tParagraphSection_1.sxw")
   sectionName   =  "ParagraphSection"
   hopeResult    =  "He "

  '/// Keep paragraphs together ///
   PrintLog "   - Keep paragraphs together"

  '/// Open a file with 4 paragraphs, the 4th paragraph is crossing the page
   Call hFileOpen(documentName)

   EditSelectAll
   Call fInsertSection(sectionName)
   Call WDokSchreiben "<MOD1 End>"
   Call WDokSchreiben "<Up>",3

   FormatParagraph
   Kontext
   active.SetPage TabTextfluss
   kontext "TabTextfluss"

  '/// Check option "Do not split paragraphs"
   AbsatzNichtTrennen.Check

   TabTextfluss.OK

   Call WDokSchreiben "<MOD1 Home>"

  'Set the focus to the head of second page
   Kontext
   EditNavigator
   Kontext "NavigatorWriter"
   Seitennummer.SetText "2"
   Sleep 2
   fCloseNavigator

   Call WDokSchreiben "<Shift MOD1 Right>"
   EditCopy

   if GetClipBoardText <> hopeResult then Warnlog "Something wrong in Keep paragraphs together test!"

   Call hCloseDocument

endcase

'-------------------------------------------------------

testcase tParagraphSection_2

    Dim  sectionName                                                                          as String
    Dim  beforeTextIntent1,afterTextIntent1,firstTextIntent1,beforeTextSpace1,afterTextSpace1 as String
    Dim  beforeTextIntent2,afterTextIntent2,firstTextIntent2,beforeTextSpace2,afterTextSpace2 as String
    
    sectionName   =  "ParagraphSection"
    
    '/// paragraph spacing  ///
    PrintLog "   - paragraph spacing "
    
    Call hNewDocument
    
    '/// insert first paragraph , and do some configurations
    if wBlindtextEinfuegen() = false then
        Call hCloseDocument
        goto endsub
    endif
       
    if iSprache = 82 then
        Call wDokSchreiben "<Return>"
    endif
    Call WDokSchreiben "<Up>"
    
    FormatParagraph
    Kontext
    active.SetPage TabEinzuegeUndAbstaende
    kontext "TabEinzuegeUndAbstaende"
    
    beforeTextIntent1 = "0" + gSeperator + "01" + gMeasurementUnit
    afterTextIntent1  = "0" + gSeperator + "02" + gMeasurementUnit
    firstTextIntent1  = "0" + gSeperator + "03" + gMeasurementUnit
    beforeTextSpace1  = "0" + gSeperator + "04" + gMeasurementUnit
    afterTextSpace1   = "0" + gSeperator + "05" + gMeasurementUnit
    
    Vonlinks.SetText   beforeTextIntent1
    VonRechts.SetText  afterTextIntent1
    ErsteZeile.SetText firstTextIntent1
    Oben.SetText       beforeTextSpace1
    Unten.SetText      afterTextSpace1
    Automatisch.Check
    Registerhaltigkeit.UnCheck
    TabEinzuegeUndAbstaende.OK
    
    Call WDokSchreiben "<Down>"
    
    '/// insert second paragraph , and do some configurations which is different from the first one
    wBlindtextEinfuegen
    Call WDokSchreiben "<Up>"
    
    FormatParagraph
    Kontext
    active.SetPage TabEinzuegeUndAbstaende
    kontext "TabEinzuegeUndAbstaende"
    
    beforeTextIntent2 = "0" + gSeperator + "06" + gMeasurementUnit
    afterTextIntent2  = "0" + gSeperator + "07" + gMeasurementUnit
    firstTextIntent2  = "0" + gSeperator + "08" + gMeasurementUnit
    beforeTextSpace2  = "0" + gSeperator + "09" + gMeasurementUnit
    afterTextSpace2   = "0" + gSeperator + "10" + gMeasurementUnit
    
    Vonlinks.SetText   beforeTextIntent2
    VonRechts.SetText  afterTextIntent2
    Automatisch.UnCheck
    ErsteZeile.SetText firstTextIntent2
    Oben.SetText       beforeTextSpace2
    Unten.SetText      afterTextSpace2
    Registerhaltigkeit.Check
    TabEinzuegeUndAbstaende.OK
    
    EditSelectAll
    Call fInsertSection(sectionName)
    
    'Check the first paragraph's configuration
    Call WDokSchreiben "<MOD1 Home>"
    FormatParagraph
    Kontext
    active.SetPage TabEinzuegeUndAbstaende
    kontext "TabEinzuegeUndAbstaende"
    
    if Automatisch.IsChecked <> TRUE OR Registerhaltigkeit.IsChecked = TRUE then
        Warnlog "Something wrong in checkbox configuration in first paragraph!"
    endif
    
    if Vonlinks.GetText   <>  beforeTextIntent1 OR _
        VonRechts.GetText  <>  afterTextIntent1  OR _
        ErsteZeile.GetText <>  firstTextIntent1  OR _
        Oben.GetText       <>  beforeTextSpace1  OR _
        Unten.GetText      <>  afterTextSpace1         then
        Warnlog "Something wrong in intent and sparcing area in first paragraph"
    endif
    
    TabEinzuegeUndAbstaende.Cancel
    
    Call WDokSchreiben "<MOD1 End>"
    Call WDokSchreiben "<Up>",3
    
    'Check the second paragraph's configuration
    FormatParagraph
    Kontext
    active.SetPage TabEinzuegeUndAbstaende
    kontext "TabEinzuegeUndAbstaende"
    
    if Automatisch.IsChecked = TRUE OR Registerhaltigkeit.IsChecked <> TRUE then
        Warnlog "Something wrong in checkbox configuration in second paragraph!"
    endif
    
    if Vonlinks.GetText   <>  beforeTextIntent2 OR _
        VonRechts.GetText  <>  afterTextIntent2  OR _
        ErsteZeile.GetText <>  firstTextIntent2  OR _
        Oben.GetText       <>  beforeTextSpace2  OR _
        Unten.GetText      <>  afterTextSpace2         then
        Warnlog "Something wrong in intent and sparcing area in second paragraph!"
    endif
    
    TabEinzuegeUndAbstaende.Cancel
    
    Call hCloseDocument

endcase

'-------------------------------------------------------

testcase tParagraphSection_3

    Dim  sectionName                                as String
    Dim  left1, right1 , top1 , bottom1 , distance1 as String
    Dim  left2,                           distance2 as String
    
    sectionName   =  "ParagraphSection"
    
    '/// paragraph borders   ///
    PrintLog "   - paragraph borders  "
    
    Call hNewDocument
    
    '/// insert first paragraph , and do some configurations
    if wBlindtextEinfuegen() = false then
        Call hCloseDocument
        goto endsub
    endif
    if iSprache = 82 then
        Call wDokSchreiben "<Return>"
    endif   
    Call WDokSchreiben "<Up>"
    
    FormatParagraph
    Kontext
    active.SetPage TabUmrandung
    kontext "TabUmrandung"
    
    Vorgaben.Typekeys "<home><right>"        ' 2nd from the left
    synchronisieren.UnCheck
    
    left1     =  "0" + gSeperator + "01" + gMeasurementUnit
    right1    =  "0" + gSeperator + "02" + gMeasurementUnit
    top1      =  "0" + gSeperator + "03" + gMeasurementUnit
    bottom1   =  "0" + gSeperator + "04" + gMeasurementUnit
    distance1 =  "0" + gSeperator + "10" + gMeasurementUnit
    
    Links.SetText  left1
    Rechts.SetText right1
    Oben.SetText   top1
    Unten.SetText  bottom1
    
    Position.TypeKeys "<home><right>"        ' 2nd. from the left
    Groesse.SetText distance1
    SchattenFarbe.Select 2
    TabUmrandung.OK
    
    Call WDokSchreiben "<Down>",2
    
    '/// insert second paragraph , and do some configurations
    '/// + which is different from the first one
    wBlindtextEinfuegen
    Call WDokSchreiben "<Up>"
    
    FormatParagraph
    Kontext
    active.SetPage TabUmrandung
    kontext "TabUmrandung"
    
    Vorgaben.Typekeys "<home><right><right>"       ' 3nd from the left
    synchronisieren.Check
    
    left2     =  "0" + gSeperator + "06" + gMeasurementUnit
    distance2 =  "0" + gSeperator + "15" + gMeasurementUnit
    
    Links.SetText  left2
    
    Position.TypeKeys "<home><right><right>"       ' 3nd. from the left
    Groesse.SetText distance2
    SchattenFarbe.Select 5
    TabUmrandung.OK
    
    EditSelectAll
    Call fInsertSection(sectionName)
    
    '<<< Check the FIRST paragraph's configuration >>>
    Call WDokSchreiben "<MOD1 Home>"
    
    FormatParagraph
    Kontext
    active.SetPage TabUmrandung
    kontext "TabUmrandung"
    
    '   if synchronisieren.IsChecked = TRUE then Warnlog "The Synchronize option should NOT be checked in first paragraph!"
    
    if Links.GetText   <>  left1  OR _
        Rechts.GetText  <>  right1 OR _
        Oben.GetText    <>  top1   OR _
        Unten.GetText   <>  bottom1     then
        Warnlog "Something wrong in space to contents area in first paragraph"
    endif
    
    if Groesse.GetText           <> distance1 OR _
        SchattenFarbe.GetSelIndex <> 2         then
        Warnlog "Something wrong in shadow style area in the first paragraph"
    endif
    
    TabUmrandung.Cancel
    
    Call WDokSchreiben "<MOD1 End>"
    Call WDokSchreiben "<Up>",3
    
    '<<<Check the SECOND paragraph's configuration>>>
    FormatParagraph
    Kontext
    active.SetPage TabUmrandung
    kontext "TabUmrandung"
    
    '   if synchronisieren.IsChecked <> TRUE then Warnlog "The Synchronize option should be checked in second paragraph!"
    
    if Links.GetText   <>  left2 OR _
        Rechts.GetText  <>  left2 OR _
        Oben.GetText    <>  left2 OR _
        Unten.GetText   <>  left2      then        
        Warnlog "Something wrong in space to contents area in second paragraph"
    endif
    
    if Groesse.GetText           <> distance2   OR _
        SchattenFarbe.GetSelIndex <> 5           then
        Warnlog "Something wrong in shadow style area in the first paragraph"
    endif
    
    TabUmrandung.Cancel
    
    Call hCloseDocument

endcase

'-------------------------------------------------------

testcase tScenarioSection_1

   Dim SectionName() , tempFile          as String
   DIM sectionInMasterDoc , MasterDoc    as String
   DIM ContentInMasterDoc()              as String
   Dim i , j                            as Integer
   Dim temp1,temp2,LinkFile            as String

  '/// create Masterdocument from another document that also contains sections  ///
   PrintLog "   - create Masterdocument from another document that also contains sections"

   MasterDoc = Convertpath( gOfficePath + "user\work\tScenarioSection_1.odm" )

   SectionName = Array( _
                       "tScenarioSection_11.odt", _
                       "tScenarioSection_12.odt", _
                       "tScenarioSection_13.odt", _
                       )

   ContentInMasterDoc = Array( _
                              "This is scenario Section test" , _
                              "The content in hide section"   , _
                              "The content in protect section", _
                             )


   j = UBound(SectionName)

   if hFileExists ( MasterDoc ) then app.kill ( MasterDoc )

   For i = 0 to j
       tempFile =  Convertpath( gOfficePath + "user\work\" + SectionName(i) )
       if hFileExists ( tempFile ) then app.kill ( tempFile )
   next i

  '/// open testdocument (tScenarioSection_1.sxw) ///
   Call hFileOpen ( gTesttoolpath + "writer\optional\input\section\tScenarioSection_1.sxw")

  '/// Select 'File /Send -> Create Masterdocument ///
   FileSendCreateMasterdocument

  '/// the 'Save as...' dialog has to appear ///
   Kontext "SendMasterDocument"
   if SendMasterDocument.NotExists then Warnlog "Save As - Dialog didn't appear!"

  '/// Select as template 'Default' ///
   Vorlage.Select 1

  '/// Save document as 'tScenarioSection_1.odm' ///
   Dateiname.SetText Convertpath( gOfficePath + "user\work\tScenarioSection_1" )

   Speichern.Click
   Sleep 3

   Kontext "Active"
   if Active.Exists then
      Active.yes
   else
      Warnlog "- No messagebox asking for the actualizationing of the master document !"
   endif
   Sleep 1

   Kontext "Navigator"
   if Navigator.Exists then
      Navigator.Close
   else
      Warnlog "Navigator in Master document isn't up!"
   end if

   Call WDokSchreiben "<Up>" , 3

  '/// Check the sections in MasterDoc
  '/// 1) check the content is section
  '/// 2) check the section name
  '/// 3) check if the section is hidden    (should NOT be hidden)
  '/// 4) check if the section is protected (should be protected)
  '/// 5) check if the section is linked and linked file is correct or not   (should be linked)

   For i = 0 to j

      Call WDokSchreiben "<Home><Shift End>"
      EditCopy

      if GetClipboardText = ContentInMasterDoc(i) then
         Kontext
         FormatSections
         Kontext "BereicheBearbeiten"

         if BereichsName.GetText <> SectionName(i) then
            Warnlog "The first section's name is wrong !"
            Warnlog "We hope it is " + SectionName(i) + " but get: " +BereichsName.GetText
         end if
         if Ausblenden.IsChecked   =   TRUE then Warnlog "The section should NOT be hidden"
         if Geschuetzt.IsChecked   <>  TRUE then Warnlog "The section should be protected"
         if Verknuepfung.IsChecked =   TRUE then
             temp1 = LCase("user/work/" & SectionName(i))
             temp2 = LCase(Dateiname.GetText)

             if Instr(temp2, temp1) = 0 then Warnlog "The linked file is NOT correct!"
         else
             Warnlog "The section should be linked"
         end if
         BereicheBearbeiten.Cancel
      else
         Warnlog "The content in first section in Master file is wrong!"
      end if

      Call WDokSchreiben "<Down>"

   next i

   Call hCloseDocument
endcase

'-------------------------------------------------------

testcase tScenarioSection_2

   Dim SectionName() , tempFile          as String
   DIM sectionInMasterDoc , MasterDoc    as String
   DIM ContentInMasterDoc()              as String
   Dim i , j  , k                        as Integer
   Dim sValue as String

  '/// create Masterdocument from another document that also contains sections and footnotes
   PrintLog "   - create Masterdocument from another document that also contains sections and footnotes"

   MasterDoc = Convertpath( gOfficePath + "user\work\tScenarioSection_2.sxg" )

   SectionName = Array( _
                       "tScenarioSection_21.odt", _
                       "tScenarioSection_22.odt", _
                       "tScenarioSection_23.odt", _
                       )

   ContentInMasterDoc = Array( _
                              "This is scenario Section test"                  , _
                              "FootNote1The content in hide sectionEndNote1"   , _
                              "FootNote2The content in protect sectionEndNote2", _
                             )

   j = UBound(SectionName)

   if hFileExists ( MasterDoc ) then app.kill ( MasterDoc )

   For i = 0 to j
       tempFile =  Convertpath( gOfficePath + "user\work\" + SectionName(i) )
       if hFileExists ( tempFile ) then app.kill ( tempFile )
   next i

  '/// open testdocument (tScenarioSection_2.sxw) ///
   Call hFileOpen ( gTesttoolpath + "writer\optional\input\section\tScenarioSection_2.sxw")

  '/// Select 'File -> Send -> Create Masterdocument ///
   FileSendCreateMasterdocument

  '/// the 'Save as...' dialog has to appear ///
   Kontext "SendMasterDocument"
   if SendMasterDocument.NotExists then Warnlog "Save As - Dialog didn't appear!"

  '/// Select as template 'Default' ///
   Vorlage.Select 1

  '/// Save document as 'masterdoc.odm' ///
   Dateiname.SetText Convertpath( gOfficePath + "user\work\tScenarioSection_2" )

   Speichern.Click
   Sleep 3

   Kontext "Active"
   if Active.Exists then
      Active.yes
   else
      Warnlog "- No messagebox asking for the actualizationing of the master document !"
   endif
   Sleep 1

   Kontext "Navigator"
   if Navigator.Exists then
      Navigator.Close
   else
      Warnlog "Navigator in Master document isn't up!"
   end if

   Call WDokSchreiben "<Up>" , 3

  '/// Check the sections in MasterDoc
  '/// 1) check the content is section
  '/// 2) check the section name
  '/// 3) check if the section is hidden    (should NOT be hidden)
  '/// 4) check if the section is protected (should be protected)
  '/// 5) check if the section is linked and linked file is correct or not (should be linked)

   For i = 0 to j

      Call wDokSchreiben "<Home><Shift End>"
      EditCopy

      if GetClipboardText = ContentInMasterDoc(i) then
         Kontext
         FormatSections
         Kontext "BereicheBearbeiten"

         if BereichsName.GetText <> SectionName(i) then
            Warnlog "The first section's name is wrong !"
            Warnlog "We hope it is " + SectionName(i) + " but get: " +BereichsName.GetText
         end if

		if Ausblenden.IsChecked   =   TRUE then Warnlog "The section should NOT be hidden"
		if Geschuetzt.IsChecked   <>  TRUE then Warnlog "The section should be protected"
		if Verknuepfung.IsChecked =   TRUE then
			sValue = ConvertPath ( gOfficePath & "user/work/" + SectionName(i) )
			if Instr( ConvertPath(Dateiname.Gettext), sValue) = false then
			 	Warnlog "The linked file is NOT correct!"
			endif				
		else
        	Warnlog "The section should be linked"
        end if
        BereicheBearbeiten.Cancel
      else
         Warnlog "The content in first section in Master file is wrong!"
      end if

      Call wDokSchreiben "<Down>"

   next i

   Call hCloseDocument

endcase

'----------------------------------------------

testcase tCTLSection_1

  '/// CTL specials Check  -- insert/sections  ///
   PrintLog "   - CTL specials -- insert/sections  "

   Call hNewDocument

  '/// 1)Switch on "Complex scripts support" in Tools/Options/Language Settings/Languages
   ActiveDeactivateCTLSupport(True)

  '/// 2)Insert/sections / Columns
   InsertSection
   Sleep 1
   Kontext
   Active.Setpage TabSpalten
   Kontext "TabSpalten"

  '/// 3)Check if text direction listbox is visible
   if (TextDirection.Exists) then

      '/// 4)Check the default name
       if TextDirection.GetSelIndex <> 3 then
          Warnlog "The default name we hope is :" + TextDirection.GetItemText(3) +" but get: "+TextDirection.GetSelText
       endif

      '/// 5)Check the available values ( Should 3 items )
          if TextDirection.GetItemCount <> 3 then
              Warnlog "The values in TextDirection should be 3, but get " +TextDirection.GetItemCount
          endif

   else
       Warnlog "the textdirection listbox doesn't exist!"
   endif

   TabSpalten.Cancel

  'Recover to default
   ActiveDeactivateCTLSupport(False)

   Call hCloseDocument

endcase

'-------------------------------------------------------

testcase tCTLSection_2

   Dim SectionName as String

   SectionName =  "CTLSection"

  '/// CTL specials -- format/section/options /Columns ///
   PrintLog "   - CTL specials -- format/section/options /Columns "

   Call hNewDocument

  '/// 1)Switch on "Complex scripts support" in Tools/Options/Language Settings/Languages
   ActiveDeactivateCTLSupport(True)

  '/// 2)Insert a section
   Call fInsertSection(SectionName)

  '/// 3)format/section/options /Columns
   Kontext
   FormatSections
   Kontext "BereicheBearbeiten"
   Optionen.Click

   Kontext
   Active.Setpage TabSpalten
   Kontext "TabSpalten"

  '/// 4)Check if text direction listbox is visible
   if (TextDirection.Exists) then

      '/// 5)Check the default name
       if TextDirection.GetSelIndex <> 3 then
          Warnlog "The default name we hope is :" + TextDirection.GetItemText(3) +" but get: "+TextDirection.GetSelText
       endif

      '/// 6)Check the available values ( Should 3 items )
       if TextDirection.GetItemCount <> 3 then
           Warnlog "The values in TextDirection should be 3, but get " +TextDirection.GetItemCount
       endif

   else
       Warnlog "the textdirection listbox doesn't exist!"
   endif

   TabSpalten.Cancel

   Kontext
   Kontext "BereicheBearbeiten"
   BereicheBearbeiten.Cancel

  'Recover to default
   ActiveDeactivateCTLSupport(False)

   Call hCloseDocument

endcase
