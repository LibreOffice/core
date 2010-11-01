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
'* short description : Hide section /Protect section / Display of dialogues
'*
'************************************************************************
'*
' #1 tBookmarkSection_1
' #1 tBookmarkSection_2
' #1 tFootnoteEndnoteSection_1
' #1 tFootnoteEndnoteSection_2
' #1 tFootnoteEndnoteSection_3
' #1 tFootnoteEndnoteSection_4
' #1 tFootnoteEndnoteSection_5
' #1 tFootnoteEndnoteSection_6
' #1 tMultiColumnSection_1
' #1 tMultiColumnSection_2
' #1 tMultiColumnSection_3
' #1 tOperationSection_1
' #1 tOperationSection_2
' #1 tOperationSection_3
' #1 tPasswordProtectSection_1
' #1 tPasswordProtectSection_2
' #1 tPasswordProtectSection_3
' #1 tPasswordProtectSection_4
' #1 tWrappedSections_1
' #1 tWrappedSections_2
' #1 tWrappedSections_3
' #1 tWrappedSections_4
' #1 tWrappedSections_5
' #1 tWrappedSections_6
' #1 tWrappedSections_7
'*
'\***********************************************************************

sub w_section_3

    printLog Chr(13) + "---------   Insert bookmarks into sections   ----------"
    Call tBookmarkSection_1          'Insert bookmarks into section, jump to
    Call tBookmarkSection_2          'Insert bookmarks into a protect section, jump to
    
    printLog Chr(13) + "---------   Footnotes/Endnotes in sections   ----------"
    Call tFootnoteEndnoteSection_1       'select Text, that contains foot-/endnotes, insert section
    Call tFootnoteEndnoteSection_2       'insert footnote into text of an existing section
    Call tFootnoteEndnoteSection_3       'footnote text must be protected  in a protected section
    Call tFootnoteEndnoteSection_4       'linked sections, where source section contains footnotes , footnote text must be also protected
    Call tFootnoteEndnoteSection_5       'footnotes in hidden areas 1 footnote text must also be hidden
    Call tFootnoteEndnoteSection_6       'ALL footnotes appear before the endnotes
    
    printLog Chr(13) + "---------   Multi-column sections   ----------"
    Call tMultiColumnSection_1       'Create multi-column section directly when inserting via dialogue (TabPage "Columns")
    Call tMultiColumnSection_2       'Create multi-column section from format/section - options (TabPage "Columns")
    Call tMultiColumnSection_3       'Fill columns balanced -- Active
    
    printLog Chr(13) + "---------   Cut/copy/paste/move/delete sections   ----------"
    Call tOperationSection_1          'Cut/copy/paste/move/delete sections -- Shortcut
    Call tOperationSection_2          'Cut/copy/paste/move/delete sections -- function bar
    Call tOperationSection_3          'when inserting from clipboard the sections have to be named automatically with the correct number
    
    printLog Chr(13) + "---------   Password protect sections   ----------"
    Call tPasswordProtectSection_1          'Format - Section: check "Password protected" PW-Dialogue has to appear two times
    Call tPasswordProtectSection_2          'reload document, Format-section 1 PW-Dialog has to appear
    Call tPasswordProtectSection_3          'try wrong password ,Error massage should appear, sections-dialogue not accessible
    Call tPasswordProtectSection_4          'Remove password protection by unchecking, reload document, Format-sections 'PW-dialogue' should NOT appear
    
    printLog Chr(13) + "---------   Wrapping and Wrapped sections   ----------"
    Call tWrappedSections_1          'select Text inside a section and insert a section
    Call tWrappedSections_2          'select text across a section an insert a section
    Call tWrappedSections_3          'wrapped sections are following properties of the wrapping section - write protection
    Call tWrappedSections_4          'wrapped sections are following properties of the wrapping section - visibility
    Call tWrappedSections_5          'wrapped sections are following properties of the wrapping section - Foot-/endnote position
    Call tWrappedSections_6          'The formatting of columns of sectionWrapping and sectionWrapped can be configured separately
    Call tWrappedSections_7          'The background graphic of sectionWrapping and sectionWrapped can be configured separately

end sub

'***********************************************************************

testcase tBookmarkSection_1

    Dim sectionName as String
    Dim testText as String
    Dim bookmarkName as String

    sectionName  = "bookmarkSection"
    bookmarkName = "myBookmark"
    testText     = "Hello World"
    
    PrintLog "   - Insert bookmarks into a section, jump to"
    
    Call hNewDocument
    
    PrintLog " insert a section , and input Text"
    Call wTypeKeys testText
    Call wTypeKeys "<Shift Home>"
    fInsertSection(sectionName)
    
    PrintLog " insert a bookmark in front of Text"
    try
        fInsertBookmark(bookmarkName)
    catch
        Warnlog "   Insert a bookmark into a section failed"
    endcatch

    printlog "set the focus end of Text, make sure no text is marked."
    Call wTypeKeys "<End><SPACE><BACKSPACE>"

    PrintLog " active Navigator, double click the bookmark which be inserted,"
    PrintLog " + then the focus should be in front of $testText"
    Call wNavigatorAuswahl(6,1, true)
    Auswahlliste.TypeKeys "<return>"
    sleep(1)
'    Call wTypeKeys "<Shift End>"  'Outcommented since the bookmark also selects the text. Just like it was when it got bookmarked.
'    sleep(1)
    EditCopy
    sleep(1)
    if GetClipboardText <> testText then Warnlog "The focus is NOT in front of " +testText
    
    printlog "Close Navigator before close the document"
    fCloseNavigator
    
    Call hCloseDocument
endcase

'--------------------------------------------------

testcase tBookmarkSection_2

    Dim sectionName as String
    Dim testText as String
    Dim bookmarkName as String

    sectionName  = "bookmarkSection"
    bookmarkName = "myBookmark"
    testText     = "Hello World"

    PrintLog "   - Jump to for bookmarks in a protect section"

    Call hNewDocument
    sleep(2)
    PrintLog " Open options and check 'Direct-Cursor' in options"
    Call fCheckDirectCursor
    sleep(2)
    PrintLog " insert a bookmark in front of Text"
    Call fInsertBookmark(bookmarkName)
    sleep(2)
    PrintLog " insert Text"
    Call wTypeKeys testText
    Call wTypeKeys "<Shift Home>"
    sleep(1)
    PrintLog " insert a section with protect option"
    InsertSection
    sleep(2)
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    Bereichsliste.Settext sectionName
    Geschuetzt.Check
    TabBereiche.OK
    sleep(2)
    
    PrintLog " set the focus end of Text"
    Call wTypeKeys "<End>"
    sleep(2)
    PrintLog " active Navigator, double click the bookmark which be inserted,"
    PrintLog " + then the focus should be in front of Text"
    Call wNavigatorAuswahl(6,1, true)
    sleep(2)
    Auswahlliste.TypeKeys "<down><return>"
    sleep(2)
    Call wTypeKeys "<Shift End>"
    sleep(1)
    EditCopy
    sleep(2)
    if GetClipboardText <> testText then Warnlog "The focus is NOT in front of " & testText
    printlog "Close Navigator before close the document"
    Call fCloseNavigator
    sleep(1)
    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tFootnoteEndnoteSection_1

    Dim sectionName as String
    Dim testText as String
    Dim preMessage as String
    
    sectionName  = "FootnoteEndnoteSection"
    testText     = "This is footnotes/endnotes test"

    PrintLog "   - Insert a section that contains foot-/endnotes"

    Call hNewDocument

    PrintLog " insert footnote in front of Text"
    fInsertFootnote("automatic")
    Call wTypeKeys "<PageUp>"

    PrintLog " insert some test words"
    Call wTypeKeys testText

    PrintLog " insert endnote at end of Text"
    fInsertEndnote("automatic")
    Call wTypeKeys "<PageUp>"

    EditSelectAll
    sleep(1)
    EditCopy
    sleep(1)
    preMessage = GetClipBoardtext

    PrintLog " insert a section which include Text, foot-/endnotes"
    try
       fInsertSection(sectionName)
       EditSelectAll
       sleep(1)
       EditCopy
       sleep(1)
       if GetClipboardText <> preMessage then Warnlog "There is something changed aftering inserting the section which include Text, foot-/endnotes"
    catch
       Warnlog "   Could NOT insert a section which include Text, foot-/endnotes"
    endcatch
    
    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tFootnoteEndnoteSection_2

    Dim sectionName as String
    Dim testText as String
    Dim preMessage as String

    sectionName  = "FootnoteEndnoteSection"
    testText     = "This is footnotes/endnotes test"

    PrintLog "   - insert footnote/endnote into text of an existing section "

    Call hNewDocument

    PrintLog " insert some test words"
    Call wTypeKeys testText
    Call wTypeKeys "<Shift Home>"

    PrintLog " insert a section which include Text"
    fInsertSection(sectionName)

    Call wTypeKeys "<Home>"

    PrintLog " insert footnote into the section"
    try
       fInsertFootnote("automatic")
    catch
       Warnlog "   Could NOT insert a footnote into the section"
    endcatch

    Call wTypeKeys "<PageUp><End>"

    PrintLog " insert endnote into the section"
    try
       fInsertEndnote("automatic")
    catch
       Warnlog "   Could NOT insert a endnote into the section"
    endcatch

    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tFootnoteEndnoteSection_3

    Dim sectionName as String
    Dim preMessage as String

    sectionName  = "FootnoteEndnoteSection"

    PrintLog "   - footnote text must be protected  in a protected section"

    Call hNewDocument

    PrintLog " Open options and check 'Direct-Cursor' in options"
    fCheckDirectCursor

    PrintLog " insert a footnote"
    fInsertFootnote("automatic")
    Call wTypeKeys "<PageUp>"

    PrintLog " insert a endnote"
    fInsertEndnote("automatic")
    Call wTypeKeys "<PageUp>"

    EditSelectAll
    sleep(1)
    EditCopy
    sleep(1)

    preMessage = GetClipBoardtext

    PrintLog " insert a section with protect option"
    InsertSection
    sleep 1
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    Bereichsliste.Settext sectionName
    Geschuetzt.Check
    TabBereiche.OK
    sleep 2

    EditSelectAll
    sleep(1)
    EditCopy
    sleep(1)

    if GetClipBoardtext = preMessage then
        Call wTypeKeys "<Space>"
        Kontext "Active"
        if Active.Exists then
            Active.ok
        else
            Warnlog "-  No warning when trying to write in protected area!"
        end if
    else
        Warnlog "Testtool couldn't find the protected area!"
    end if

    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tFootnoteEndnoteSection_4

    Dim sectionName as String
    Dim sourceSectionName as String
    Dim testText as String
    Dim preMessage as String

    sectionName       = "FootnoteEndnoteSection"
    sourceSectionName = "NormalSection"
    testText          = "The test is in source section"

    if Dir (gOfficePath + "user\work\tFootnoteEndnoteSection_4.odt") <> "" then kill (gOfficePath + "user\work\tFootnoteEndnoteSection_4.odt")

        PrintLog "   - linked sections, where source section contains footnotes , footnote text must be also protected "

    PrintLog " insert a footnote in a section, then save the file"
    Call hNewDocument

    fInsertFootnote("automatic")
    Call wTypeKeys "<PageUp>"

    Call wTypeKeys testText

    fInsertEndnote("automatic")
    Call wTypeKeys "<PageUp>"

    Call wTypeKeys "<Shift Home>"
    fInsertSection(sourceSectionName)

    EditSelectAll
    sleep(1)
    EditCopy
    sleep(1)
    preMessage = GetClipBoardtext

    Call hFileSaveAsKill (gOfficePath + "user\work\tFootnoteEndnoteSection_4.odt")
    Call hCloseDocument

    Call hNewDocument

    PrintLog " Open options and check 'Direct-Cursor' in options"
    fCheckDirectCursor

    PrintLog " insert a section with link option"
    InsertSection
    sleep 1
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    Bereichsliste.Settext sectionName
    Verknuepfung.Check

    DateiVerknuepfung.Settext ConvertPath(gOfficePath + "user\work\tFootnoteEndnoteSection_4.odt")
    TabBereiche.OK
    sleep 2

    EditSelectAll
    sleep(1)
    EditCopy
    sleep(1)

    PrintLog " the footnote should be protected"
    if GetClipBoardtext = preMessage then
        Call wTypeKeys "<Space>"
        Kontext "Active"
        if Active.Exists then
            Active.ok
        else
            Warnlog "-  No warning when trying to write in protected area!"
        end if
    else
        Warnlog "Testtool couldn't find the protected area!"
    end if

    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tFootnoteEndnoteSection_5
    Dim sectionName as String
    sectionName  = "FootnoteEndnoteSection"
    printlog "   - Footnotes in hidden areas 1 footnote text must also be hidden"
    Call hNewDocument

    PrintLog " Insert a footnote"
    fInsertFootnote("automatic")
    Call wTypeKeys "<PAGEUP>"
    PrintLog " Insert a endnote"
    fInsertEndnote("automatic")
    Call wTypeKeys "<PAGEUP>"
    Call wTypeKeys "<HOME>"
    Call wTypeKeys "<RETURN>"
    Call wTypeKeys "<SHIFT END>"
    PrintLog " Insert a section with hide option"
    InsertSection
    sleep(1)
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
        Bereichsliste.Settext sectionName
        Ausblenden.Check
    TabBereiche.OK
    sleep(1)
    try
        EditSelectAll
        sleep(1)
        EditCopy
        sleep(1)
        warnlog "'Edit / Copy' is usable but if there is nothing in the document (hidden section in footnote) it should be disabled!"
    catch
        printlog "Clipboard should be and is empty."
    endcatch

    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tFootnoteEndnoteSection_6

    Dim sectionName as String
    
    sectionName  = "FootnoteEndnoteSection"
    
    PrintLog "   - ALL footnotes appear before the endnotes "
    
    Call hNewDocument
    
    fInsertSection(sectionName)
    Call wTypeKeys "<Up>"
    
    PrintLog " insert the first footnote"
    fInsertFootnote("automatic")
    Call wNavigatorAuswahl(7,1, true)
    if Seitennummer.GetText <> "1" then warnlog "The first footnote should be in the first page"
    fCloseNavigator
    Call wTypeKeys "<PageUp>"
    
    PrintLog " insert a endnote"
    fInsertEndnote("automatic")
    Call wNavigatorAuswahl(7,1, true)
    if Seitennummer.GetText <> "2" then warnlog "The first endnote should be in the second page"
    fCloseNavigator
    Call wTypeKeys "<PageUp>"
    
    PrintLog " insert the second footnote"
    fInsertFootnote("automatic")
    Call wNavigatorAuswahl(7,1, true)
    if Seitennummer.GetText <> "1" then warnlog "The second footnote should be in the first page"
    fCloseNavigator
    
    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tMultiColumnSection_1

    Dim columnNumber as String

    columnNumber  =  "3"

    PrintLog "   - Create multi-column section directly when inserting via dialogue (TabPage 'Columns')"

    Call hNewDocument

    PrintLog " insert a section(insert/section with TabPage 'Columns',"
    PrintLog " + set column to $columnNumber"
    InsertSection
    sleep 1
    
    Kontext
    Active.Setpage TabSpalten
    Kontext "TabSpalten"
    Anzahl.Settext columnNumber
    TabSpalten.OK
    sleep 1
    
    Kontext
    FormatSections
    Kontext "BereicheBearbeiten"
    Optionen.Click
    
    Kontext
    Active.Setpage TabSpalten
    Kontext "TabSpalten"
    
    'Check volumn number
    if Anzahl.Gettext <> columnNumber then  Warnlog "The volumn we hope is " +columnNumber +" but get " + Anzahl.Gettext
    
    'Check volumn width
    if Spaltenbreite1.Gettext <> Spaltenbreite2.Gettext OR Spaltenbreite1.Gettext <> Spaltenbreite3.Gettext then Warnlog "column width is NOT Balance"
    
    TabSpalten.Cancel
    
    Kontext
    Kontext "BereicheBearbeiten"
    BereicheBearbeiten.Cancel
    
    Call hCloseDocument

endcase

'----------------------------------------------------------

testcase tMultiColumnSection_2

    Dim columnNumber , sectionName as String

    sectionName   =  "MultiColumnSection"
    columnNumber  =  "3"

    PrintLog "   - Create multi-column section from format/section - options (TabPage Columns)"

    Call hNewDocument

    PrintLog " insert a section with only 1 culumn, that means a normal section"
    fInsertSection(sectionName)

    PrintLog " change the volumn number (format/section) with"
    PrintLog " + TabPage 'Columns', set column to $columnNumber"
    Kontext
    FormatSections
    Kontext "BereicheBearbeiten"
    Optionen.Click

    Kontext
    Active.Setpage TabSpalten
    Kontext "TabSpalten"

    'Set volumn number
    Anzahl.Settext columnNumber
    TabSpalten.OK

    Kontext
    Kontext "BereicheBearbeiten"
    BereicheBearbeiten.OK

    PrintLog " Check the volumn number"
    Kontext
    FormatSections
    Kontext "BereicheBearbeiten"
    Optionen.Click

    Kontext
    Active.Setpage TabSpalten
    Kontext "TabSpalten"

    'Check volumn number
    if Anzahl.Gettext <> columnNumber then  Warnlog "The volumn we hope is " +columnNumber +" but get " + Anzahl.Gettext

    TabSpalten.Cancel

    Kontext
    Kontext "BereicheBearbeiten"
    BereicheBearbeiten.Cancel

    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tMultiColumnSection_3

    Dim columnNumber as String
    Dim testText1 as String
    Dim testText2 as String
    Dim i as Integer

    columnNumber  =  "2"
    testText1     =  "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUV"
    testText2     =  "-"

    PrintLog " fill columns balanced -- Active"

    PrintLog " when text inserted into a section, the columns have"
    PrintLog " + to be filled equally"

    PrintLog "   - fill columns balanced -- Active"

    Call hNewDocument

    PrintLog " input some text , less 1 line"
    Call wTypeKeys testText1
    EditSelectAll

    PrintLog " insert a section ,set column to $columnNumber"
    InsertSection
    sleep 1

    Kontext
    Active.Setpage TabSpalten
    Kontext "TabSpalten"
    Anzahl.Settext columnNumber
    TabSpalten.OK
    sleep 1

    Call wTypeKeys "<Escape>"

    PrintLog " input '-' in the end of the text check if"
    PrintLog " + the '-' will appare to second colum"
    For i = 0 to 100
        Call wTypeKeys testText2
        Call wTypeKeys "<Shift Home>"
        sleep(1)
        EditCopy
        sleep(1)
        if GetClipboardText = testText2 then goto GoON
        Call wTypeKeys "<Escape>"
        Call wTypeKeys "<End>"
    next i
    Warnlog "We can't get " +hopeResult2 +" in second column"
    
GoON:
    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tOperationSection_1
    Dim testText1 as String
    Dim testText2 as String
    Dim testText3 as String
    Dim sectionName as String

    sectionName   =  "OperationSection"
    testText1     =  "The operation CUT  for Section test"
    testText2     =  "The operation COPY for Section test"
    testText3     =  "The operation DELETE for Section test"

    PrintLog "   - Cut/copy/paste/move/delete sections - Via key shortcuts"

    Call hNewDocument

    PrintLog " insert a section"
    Call wTypeKeys "<Return>"
    fInsertSection(sectionName)
    Call wTypeKeys "<Up>"
    Call wTypeKeys testText1
    Call wTypeKeys "<Up>"
    EditSelectAll

    PrintLog " Cut section"
    Call wTypeKeys "<Mod1 x>"

    try
        FormatSections
        Kontext "BereicheBearbeiten"
        BereicheBearbeiten.Cancel
        Warnlog "Cut section is failed!"
    catch
   endcatch

    PrintLog " Paste section"
    Call wTypeKeys "<Mod1 v>"

    'start up Navigator
    Call wNavigatorAuswahl(7,1, true)
    PrintLog " 1)check section name"
    if Auswahlliste.GetSelText = sectionName then
        PrintLog " 2)check the content in section"
        Call wTypeKeys "<home><Shift End>"
        sleep(1)
        EditCopy
        sleep(1)
        if GetClipboardText <> testText1 then  Warnlog "The content is the section is NOT right in cut/paste section!"
    else
        warnlog "The Section Name is wrong, should be " + sectionName +" but get " +Auswahlliste.GetSelText
    end if

    'Close Navigator before close the document
    Kontext
    Call fCloseNavigator

    Call hCloseDocument

    Call hNewDocument

    PrintLog " insert a section"
    Call wTypeKeys "<Return>"
    Call fInsertSection(sectionName)
    Call wTypeKeys "<Up>"
    Call wTypeKeys testText2
    Call wTypeKeys "<Up>"
    EditSelectAll

    PrintLog " Copy section"
    Call wTypeKeys "<Mod1 c>"

    Call hCloseDocument

    Call hNewDocument

    'Paste section
    Call wTypeKeys "<Mod1 v>"

    'start up Navigator
    Call wNavigatorAuswahl(7,1, true)

    PrintLog " 1)check section name"
    if Auswahlliste.GetSelText = sectionName then
        PrintLog " 2)check the content in section"
        sleep 1
        Call wTypeKeys "<home><Shift End>"
        sleep(1)
        EditCopy
        sleep(1)
        if GetClipboardText <> testText2 then  Warnlog "The content is the section is NOT right in copy/paste section!"
    else
        warnlog "The Section Name is wrong, should be " + sectionName +" but get " +Auswahlliste.GetSelText
    end if

    printlog "'Close Navigator before close the document"
    Call fCloseNavigator

    Call hCloseDocument

    Call hNewDocument

    PrintLog " insert a section"
    Call wTypeKeys "<Return>"
    Call fInsertSection(sectionName)
    Call wTypeKeys "<Up>"
    Call wTypeKeys testText3
    Call wTypeKeys "<Up>"
    EditSelectAll

    PrintLog " delete the section"
    Call wTypeKeys "<Delete>"

    PrintLog " check if section exists or not"
    try
        FormatSections
        Kontext "BereicheBearbeiten"
        BereicheBearbeiten.Cancel
        Warnlog "Delete section is failed!"
    catch
    endcatch

    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tOperationSection_2

    Dim testText1 as String
    Dim testText2 as String
    Dim testText3 as String
    Dim sectionName as String

    sectionName   =  "OperationSection"
    testText1     =  "The operation CUT  for Section test"
    testText2     =  "The operation COPY for Section test"
    testText3     =  "The operation DELETE for Section test"

    PrintLog "   - Cut/copy/paste/move/delete sections - Via function bar"

    Call hNewDocument

    PrintLog " insert a section"
    Call wTypeKeys "<Return>"
    Call fInsertSection(sectionName)
    Call wTypeKeys "<Up>"
    Call wTypeKeys testText1
    Call wTypeKeys "<Up>"
    EditSelectAll
    sleep 1
    PrintLog " Cut section"
    Kontext "Standardbar"
    Ausschneiden.Click

    try
        FormatSections
        Kontext "BereicheBearbeiten"
        BereicheBearbeiten.Cancel
        Warnlog "Cut section is failed!"
    catch
    endcatch

    PrintLog " Paste section"
    Kontext "Standardbar"
    Einfuegen.Click

    'start up Navigator
    Call wNavigatorAuswahl(7,1, true)

    PrintLog " 1)check section name"
    if Auswahlliste.GetSelText = sectionName then
        PrintLog " 2)check the content in section"
        Call wTypeKeys "<home><Shift End>"
        sleep(1)
        EditCopy
        sleep(1)
        if GetClipboardText <> testText1 then  Warnlog "The content is the section is NOT right in cut/paste section!"
    else
        warnlog "The Section Name is wrong, should be " + sectionName +" but get " +Auswahlliste.GetSelText
    end if

    printlog "Close Navigator before close the document"
    Call fCloseNavigator
    Call hCloseDocument
    Call hNewDocument

    PrintLog " insert a section"
    Call wTypeKeys "<Return>"
    Call fInsertSection(sectionName)
    Call wTypeKeys "<Up>"
    Call wTypeKeys testText2
    Call wTypeKeys "<Up>"
    EditSelectAll
    sleep 1

    PrintLog " Copy section"
    Kontext "Standardbar"
    Kopieren.Click

    Call hCloseDocument

    Call hNewDocument

    PrintLog " Paste section"
    Kontext "Standardbar"
    Einfuegen.Click

    'start up Navigator
    Call wNavigatorAuswahl(7,1, true)

    PrintLog " 1)check section name"
    if Auswahlliste.GetSelText = sectionName then
        PrintLog " 2)check the content in section"
        sleep 1
        Call wTypeKeys "<home><Shift End>"
        sleep(1)
        EditCopy
        sleep(1)
        if GetClipboardText <> testText2 then  Warnlog "The content is the section is NOT right in copy/paste section!"
    else
        warnlog "The Section Name is wrong, should be " + sectionName +" but get " +Auswahlliste.GetSelText
    end if

    Call fCloseNavigator
    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tOperationSection_3

    Dim sectionName  as String
    
    PrintLog " when inserting from clipboard the sections have to be named automatically"
    PrintLog " + with the correct number, means:"
    PrintLog " + when copying 1 section inside a document, the new section must be"
    PrintLog " + named 'Section2' automatically, if the copied section is named Section1"
    
    PrintLog "   - when inserting from clipboard the sections have to be named automatically with the correct number"
    
    sectionName = gDefaultSectionName + 2
    
    PrintLog " New document"
    Call hNewDocument
    Call wTypeKeys "<Return>"
    
    PrintLog " Insert a section with default name"
    InsertSection
    sleep 1
    Kontext
    Kontext "TabBereiche"
    TabBereiche.OK
    sleep 1
    
    PrintLog " Copy and Paste the section into the document"
    EditSelectAll
    sleep(1)
    EditCopy
    sleep(1)
    Call wTypeKeys "<Escape>"
    sleep(1)
    EditPaste
    
    Kontext
    FormatSections
    sleep(1)
    Kontext "BereicheBearbeiten"
    Bereich.TypeKeys "<Up>",2
    Bereich.TypeKeys "<Down>"
    
    if  BereichsName.GetText <> sectionName then warnlog "We hope the section name is " & sectionName & " but get " & BereichsName.GetText
    BereicheBearbeiten.Cancel
    
    PrintLog " Close document"
    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tPasswordProtectSection_1

    Dim Password as String
    Dim newPassword as String

    Password    = "protectedarea"
    newPassword = "newProtectedArea"

    PrintLog "   - Format - Section: check 'Password protected' 1 PW-Dialogue has to appear two times "

    Call hNewDocument

    PrintLog " Insert a section with password"
    InsertSection
    sleep 1
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    Geschuetzt.Check
    sleep 1
    Passwort.Check
    
    Kontext "BereicheSchuetzen"
     PasswortName.Settext Password
     PasswortBestaetigen.Settext Password
    BereicheSchuetzen.Ok
    
    Kontext
    Kontext "TabBereiche"
    TabBereiche.OK
    
    FormatSections
    Kontext "BereicheBearbeiten"
    
    EnterPasswort.Click
    
    Kontext
    Kontext "BereicheSchuetzen"
    
    PrintLog " PassWord-Dialogue should  appear first time"
    if BereicheSchuetzen.NotExists then
        Warnlog "Unable to enter passwort for section, because dialog is not up!"
    else
        PasswortName.Settext Password
        BereicheSchuetzen.Ok
        sleep 1

       PrintLog " + PW-Dialogue should  appear second time"
        if BereicheSchuetzen.NotExists then
             Warnlog "Unable to enter passwort for section, because dialog is not up!"
        else
             PasswortName.Settext newPassword
             PasswortBestaetigen.Settext newPassword
             BereicheSchuetzen.Ok
    
             Kontext
             Kontext "BereicheBearbeiten"
             PrintLog " + 'Password protected' checkbox must be checked after inputing new password"
             if Passwort.IsChecked <> TRUE then Warnlog "The 'password protected' checkbox should be checked"
        end if
    end if

    Kontext
    Kontext "BereicheBearbeiten"
    BereicheBearbeiten.OK

    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tPasswordProtectSection_2

    Dim Password as String
    
    Password = "protectedarea"
    
    if Dir (gOfficePath + "user\work\tPasswordProtectSection_2.odt") <> "" then
        kill (gOfficePath + "user\work\tPasswordProtectSection_2.odt")
    end if
    PrintLog "   - reload document, Format-section 1 PW-Dialog has to appear "
    
    Call hNewDocument
    
    PrintLog " Insert a section with password , and save the file"
    InsertSection
    sleep 1
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    Geschuetzt.Check
    sleep 1
    Passwort.Check
    
    Kontext "BereicheSchuetzen"
    PasswortName.Settext Password
    PasswortBestaetigen.Settext Password
    BereicheSchuetzen.Ok
    
    Kontext "TabBereiche"
    TabBereiche.OK
    
    Call hFileSaveAsKill (gOfficePath + "user\work\tPasswordProtectSection_2.odt")
    
    Call hCloseDocument
    
    PrintLog " Reload the file"
    hFileOpen( gOfficepath + "user\work\tPasswordProtectSection_2.odt")
    
    FormatSections
    Kontext "BereicheBearbeiten"
    
    Optionen.Click
    
    Kontext "BereicheSchuetzen"
    PrintLog " PassWord-Dialogue should appear when clicking 'option' button"
    if BereicheSchuetzen.NotExists then
        Warnlog "Unable to enter passwort for section, because dialog is not up!"
    else
        BereicheSchuetzen.Cancel
    end if

    Kontext "BereicheBearbeiten"
    BereicheBearbeiten.Cancel

    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tPasswordProtectSection_3

    Dim Password as String
    Dim wrongPassword as String
    
    Password       = "protectedarea"
    wrongPassword  = "wrongPW"
    
    PrintLog "   - Input wrong password ,Error massage should appear, sections-dialogue not accessible "
    
    Call hNewDocument
    
    PrintLog " Insert a section with password"
    InsertSection
    sleep 1
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    Geschuetzt.Check
    sleep 1
    Passwort.Check
    
    Kontext "BereicheSchuetzen"
     PasswortName.Settext Password
     PasswortBestaetigen.Settext Password
    BereicheSchuetzen.Ok
    
    Kontext "TabBereiche"
    TabBereiche.OK
    
    FormatSections
    Kontext "BereicheBearbeiten"
    
    EnterPasswort.Click
    
    Kontext "BereicheSchuetzen"
    
    PasswortName.Settext wrongPassword
    
    BereicheSchuetzen.Ok
    sleep 1
    
    Kontext
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT <> 304 then  warnlog "Error message box is NOT up! The active box is " +Active.GetRT
        Active.OK
    else
        Warnlog "Error message box is NOT up!"
    end if
    
    Kontext
    Kontext "BereicheBearbeiten"
    BereicheBearbeiten.Cancel
    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tPasswordProtectSection_4

    Dim Password as String
    
    Password = "protectedarea"
    
    if Dir (gOfficePath + "user\work\tPasswordProtectSection_4.odt") <> "" then
        kill (gOfficePath + "user\work\tPasswordProtectSection_4.odt")
    end if
    
    PrintLog "   - Remove password protection by unchecking, reload document, Format-sections 'PW-dialogue' may not appear"
    
    Call hNewDocument
    
    PrintLog " Insert a section with password"
    InsertSection
    sleep 1
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    Geschuetzt.Check
    sleep 1
    Passwort.Check
    
    Kontext "BereicheSchuetzen"
    PasswortName.Settext Password
    PasswortBestaetigen.Settext Password
    BereicheSchuetzen.Ok
    
    Kontext "TabBereiche"
    TabBereiche.OK
    
    PrintLog " Remove the password protection and save the file"
    FormatSections
    Kontext "BereicheBearbeiten"
    
    Passwort.Uncheck
    Kontext "BereicheSchuetzen"    
    PrintLog " PassWord-Dialogue should appear when clicking 'option' button"
    if BereicheSchuetzen.NotExists then
        Warnlog "Unable to uncheck password for section, because dialog is not up!"
    else
        PasswortName.Settext Password
        BereicheSchuetzen.OK
    end if
    
    Kontext "BereicheBearbeiten"
    BereicheBearbeiten.OK
    
    Call hFileSaveAsKill (gOfficePath + "user\work\tPasswordProtectSection_4.odt")
    
    Call hCloseDocument
    
    PrintLog " Reload the file"
    hFileOpen( gOfficepath + "user\work\tPasswordProtectSection_4.odt")
    
    PrintLog " In format/section, PW-Dialogue should appear when clicking 'option' button"
    FormatSections
    Kontext "BereicheBearbeiten"
    
    Optionen.Click
    
    Kontext "BereicheSchuetzen"
    
    if BereicheSchuetzen.Exists then
        Warnlog "Unable to enter passwort for section, because dialog is not up!"
        PasswortName.Settext Password
        BereicheSchuetzen.OK
    end if
    
    Kontext
    Active.Setpage TabSpalten
    Kontext "TabSpalten"
    TabSpalten.Cancel
    
    Kontext "BereicheBearbeiten"
    BereicheBearbeiten.Cancel
    
    Call hCloseDocument

endcase


'****************************************************************************
'** After the Wrapping section and Wrapped section are generated, the display
'** is like
'**
'** This is                          <----|--------------------
'** wrapped   <---- Wrapped section  <----| Wrapping section
'** sections test                    <----|--------------------
'*****************************************************************************
testcase tWrappedSections_1

    Dim wrappingSection as String
    Dim wrappedSection as String
    Dim testText as String

    wrappingSection =  "wrappingSection"
    wrappedSection  =  "wrappedSection"
    testText        =  "This is wrapped section test"

    PrintLog "   - select Text inside a section and insert a section "

    Call hNewDocument

    PrintLog " insert wrapping section"
    Call wTypeKeys testText
    EditSelectAll
    fInsertSection(wrappingSection)

    Call wTypeKeys "<Home>"
    Call wTypeKeys "<Mod1 Right>",2
    Call wTypeKeys "<Mod1 Shift Right>"

    try
        PrintLog " insert wrapped section"
        fInsertSection(wrappedSection)

        Kontext
        FormatSections
        Kontext "BereicheBearbeiten"
        Bereich.TypeKeys "<Up>"

        if  BereichsName.GetText <> wrappingSection then Warnlog "The wrapping section names is wrong : " +BereichsName.GetText
        Bereich.TypeKeys "<Down>"
        if  BereichsName.GetText <> wrappedSection then Warnlog "The wrapped section names is wrong : " +BereichsName.GetText

        BereicheBearbeiten.Cancel
    catch
        Warnlog "Can't generate wrapped section"
    endcatch

    Call hCloseDocument

endcase

'----------------------------------------------------------

testcase tWrappedSections_2

    Dim wrappingSection as String
    Dim wrappedSection as String
    Dim testTextInFirstline as String
    Dim testTextInSecondline as String

    wrappingSection      =  "wrappingSection"
    wrappedSection       =  "wrappedSection"
    testTextInFirstline  =  "This is wrapped section test in first line"
    testTextInSecondline =  "This is wrapped section test in second line"

    printlog "   - select text across a section an insert a section "
    Call hNewDocument

    PrintLog " Insert wrapped section"
    Call wTypeKeys "<Return>"
    Call wTypeKeys(testTextInFirstline)
    Call wTypeKeys "<Return>"
    Call wTypeKeys(testTextInSecondline)
    Call wTypeKeys "<MOD1 HOME><SHIFT END>"
    Call fInsertSection(wrappedSection)

    EditSelectAll
    try
        PrintLog " Insert wrapping section"
        Call fInsertSection(wrappingSection)
        Kontext
        FormatSections
        Kontext "BereicheBearbeiten"
        Bereich.TypeKeys "<UP>"
        if BereichsName.GetText <> wrappingSection then
            QAErrorLog "#102552# - The wrapping section names is wrong."
        end if
        Bereich.TypeKeys "<Down>"
        if BereichsName.GetText <> wrappedSection then
            QAErrorLog "#102552# - The wrapped section names is wrong."
        end if
        BereicheBearbeiten.Cancel
     catch
         Warnlog "Can't generate wrapping section"
     endcatch

    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tWrappedSections_3

    Dim wrappingSection as String
    Dim wrappedSection as String
    Dim testText as String
    Dim contentInWrappedSection as String
    
    wrappingSection =  "wrappingSection"
    wrappedSection  =  "wrappedSection"
    testText        =  "This is wrapped section test"
    contentInWrappedSection = "wrapped "
    
    PrintLog "   - wrapped sections are following properties of the wrapping section - write protection"
    
    Call hNewDocument
    
    PrintLog " Open options and check 'Direct-Cursor' in options"
    fCheckDirectCursor
    
    PrintLog " insert wrapping section"
    Call wTypeKeys testText
    EditSelectAll
    fInsertSection(wrappingSection)
    
    Call wTypeKeys "<Home>"
    Call wTypeKeys "<Mod1 Right>",2
    Call wTypeKeys "<Mod1 Shift Right>"
    
    PrintLog " insert wrapped section"
    fInsertSection(wrappedSection)
    
    PrintLog " format/section, choose protect in wrappingSection"
    Kontext
    FormatSections
    Kontext "BereicheBearbeiten"
    Bereich.TypeKeys "<Up>"
    Geschuetzt.Check
    BereicheBearbeiten.OK
    
    Call wTypeKeys "<Home><Shift End>"
    sleep(1)
    EditCopy
    sleep(1)
    if GetClipBoardtext = contentInWrappedSection then
        Call wTypeKeys "<Space>"
        Kontext "Active"
        if Active.Exists then
            Active.ok
        else
            Warnlog "-  No warning when trying to write in protected area!"
        end if
    else
        Warnlog "The focus is NOT in the wrapped section!"
    end if
    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tWrappedSections_4

    Dim wrappingSection as String
    Dim wrappedSection as String
    Dim testText as String
    
    wrappingSection =  "wrappingSection"
    wrappedSection  =  "wrappedSection"
    testText        =  "This is wrapped section test"
    
    printlog "   - wrapped sections are following properties of the wrapping section - visibility"
    
    Call hNewDocument
    
    PrintLog " insert wrapping section"
    Call wTypeKeys "<RETURN>"
    Call wTypeKeys testText
    Call wTypeKeys "<SHIFT HOME>"
    Call fInsertSection(wrappingSection)
    Call wTypeKeys "<HOME>"
    Call wTypeKeys "<MOD1 RIGHT>" , 2
    Call wTypeKeys "<MOD1 SHIFT RIGHT>"
    PrintLog " insert wrapped section"
    Call fInsertSection(wrappedSection)
    PrintLog " format/section, choose hide checkbox in wrappingSection"
    Kontext
    FormatSections
    Kontext "BereicheBearbeiten"
    Bereich.TypeKeys "<UP>"
    Ausblenden.check
    BereicheBearbeiten.OK
    
    try
        EditSelectAll
        sleep(1)
        EditCopy
        sleep(1)
        warnlog "There should be nothing visible in the document but 'Edit / Copy' is available!"
        warnlog "Clipboard: " & GetClipBoardtext
    catch
        printlog "All sections are invisible."
    endcatch
    
    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tWrappedSections_5

    Dim wrappingSection as String
    Dim wrappedSection as String
    Dim testText as String
    
    wrappingSection =  "wrappingSection"
    wrappedSection  =  "wrappedSection"
    testText        =  "This is wrapped section test"

    PrintLog "   - wrapped sections are following properties of the wrapping section - Foot-/endnote position"

    Call hNewDocument
    PrintLog " insert wrapping section"
    Call wTypeKeys testText
    EditSelectAll
    Call fInsertSection(wrappingSection)
    
    Call wTypeKeys "<Home>"
    Call wTypeKeys "<Mod1 Right>",2
    Call wTypeKeys "<Mod1 Shift Right>"
    
    PrintLog " insert wrapped section"
    fInsertSection(wrappedSection)
    
    PrintLog " format/section, choose wrappingSection , then press options button,"
    PrintLog " + then tabpage footnotes/endnotes ,"
    PrintLog " + check collect at end of text in footnotes area and"
    PrintLog " + check collect at end of text in endnotes area"
    Kontext
    FormatSections
    Kontext "BereicheBearbeiten"
    Bereich.TypeKeys "<Up>"
    'press options button
    Optionen.Click
    
    'tabpage footnotes/endnotes
    Kontext
    Active.Setpage TabFussEndnoten
    Kontext "TabFussEndnoten"
    
    'check collect at end of text in footnotes area and
    'check collect at end of text in endnotes area
    FussnotenAmTextendeSammeln.Check
    EndnotenAmBereichsendeSammeln.Check
    
    TabFussEndnoten.OK
    Kontext "BereicheBearbeiten"
    BereicheBearbeiten.OK
    
    PrintLog " format/section again, choose wrappedSection , then press options button,"
    PrintLog " + then tabpage footnotes/endnotes ,"
    PrintLog " + check if collect at end of text in footnotes area and"
    PrintLog " + collect at end of text in endnotes area are checked"
    FormatSections
    Kontext "BereicheBearbeiten"
    Bereich.TypeKeys "<Down>"
    'press options button
    Optionen.Click
    
    'tabpage footnotes/endnotes
    Kontext
    Active.Setpage TabFussEndnoten
    Kontext "TabFussEndnoten"
    
    PrintLog " check if collect at end of text in footnotes area is checked"
    if FussnotenAmTextendeSammeln.IsChecked <> TRUE then
        Warnlog "The footnotes check option is NOT checked in wrappedSection"
    end if
    
    PrintLog " check if collect at end of text in endnotes area  is checked"
    if EndnotenAmBereichsendeSammeln.IsChecked  <> TRUE then
        Warnlog "The endnotes check option is NOT checked in wrappedSection"
    end if
    
    TabFussEndnoten.Cancel
   
    Kontext "BereicheBearbeiten"
    BereicheBearbeiten.Cancel
    
    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tWrappedSections_6

    Dim wrappingSection as String
    Dim wrappedSection as String
    Dim testText as String
    Dim columnInwrappingSection as String
    Dim columnInwrappedSection as String

    wrappingSection         =  "wrappingSection"
    wrappedSection          =  "wrappedSection"
    testText                =  "This is wrapped section test"
    columnInwrappingSection =  "2"
    columnInwrappedSection  =  "3"
    
    PrintLog "   - The formatting of columns of sectionWrapping and sectionWrapped can be configured separately"
    
    Call hNewDocument
    
    PrintLog " insert wrapping section"
    Call wTypeKeys testText
    EditSelectAll
    fInsertSection(wrappingSection)
    
    Call wTypeKeys "<Home>"
    Call wTypeKeys "<Mod1 Right>",2
    Call wTypeKeys "<Mod1 Shift Right>"
    
    PrintLog " insert wrapped section"
    fInsertSection(wrappedSection)
    
    PrintLog " format/section, choose wrappingSection , then press options button,"
    PrintLog " + then tabpage columns, set columns to $columnInwrappingSection"
    FormatSections
    Kontext "BereicheBearbeiten"
    Bereich.TypeKeys "<Up>"
    'press options button
    Optionen.Click
    
    Kontext
    Active.Setpage TabSpalten
    Kontext "TabSpalten"
    
    'Set column to $columnInwrappingSection
    Anzahl.Settext columnInwrappingSection
    TabSpalten.OK
    
    PrintLog " format/section, choose wrappedSection , then press options button,"
    PrintLog " + then tabpage columns, set columns to $columnInwrappedSection"
    Kontext "BereicheBearbeiten"
    Bereich.TypeKeys "<Down>"
    'press options button
    Optionen.Click
    
    Kontext
    Active.Setpage TabSpalten
    Kontext "TabSpalten"
    
    'Set column to $columnInwrappedSection
    Anzahl.Settext columnInwrappedSection
    TabSpalten.OK
    
    Kontext "BereicheBearbeiten"
    BereicheBearbeiten.OK
    
    PrintLog " format/section again, then check if columns is configured separately"
    'check columns in wrappingSection
    Kontext
    FormatSections
    Kontext "BereicheBearbeiten"
    Bereich.TypeKeys "<Up>"
    Optionen.Click
    
    Kontext
    Active.Setpage TabSpalten
    Kontext "TabSpalten"
    if Anzahl.Gettext <> columnInwrappingSection then
        Warnlog "The volumn in wrappingSection should be " +columnInwrappingSection +" but get " + Anzahl.Gettext
    end if
    TabSpalten.Cancel

    'check columns in wrappedSection
    Kontext "BereicheBearbeiten"
    Bereich.TypeKeys "<Down>"
    Optionen.Click

    Kontext
    Active.Setpage TabSpalten
    Kontext "TabSpalten"
    if Anzahl.Gettext <> columnInwrappedSection then
        Warnlog "The volumn in wrappedSection should be " +columnInwrappedSection +" but get " + Anzahl.Gettext
    end if
    TabSpalten.Cancel

    Kontext "BereicheBearbeiten"
    BereicheBearbeiten.Cancel
    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tWrappedSections_7

    Dim wrappingSection as String
    Dim wrappedSection as String
    Dim testText as String
    Dim testFileWrapping , testFileWrapped    as String
    
    wrappingSection  =  "wrappingSection"
    wrappedSection   =  "wrappedSection"
    testText         =  "This is wrapped section test"
    testFileWrapping =   ConvertPath ( gTesttoolPath + "writer\optional\input\graphics\jolink.jpg")
    testFileWrapped  =   ConvertPath ( gTesttoolPath + "writer\optional\input\graphics\bild.jpg")

    PrintLog "   - The background graphic of sectionWrapping and sectionWrapped can be configured separately"

    Call hNewDocument    
    PrintLog " insert wrapping section"
    Call wTypeKeys testText
    EditSelectAll
    Call fInsertSection(wrappingSection)

    Call wTypeKeys "<Home>"
    Call wTypeKeys "<Mod1 Right>",2
    Call wTypeKeys "<Mod1 Shift Right>"
    
    PrintLog " insert wrapped section"
    Call fInsertSection(wrappedSection)
    
    PrintLog " format/section,"
    PrintLog " + choose wrappingSection , then press options button ,"
    PrintLog " + then tabpage backgroud / graphic, browse a picture ,"
    PrintLog " + choose 'Position' in type ."
    FormatSections
    Kontext "BereicheBearbeiten"
    Bereich.TypeKeys "<Up>"
    'press options button
    Optionen.Click

    Kontext
    Active.SetPage TabHintergrund
    sleep 1
    Kontext "TabHintergrund"

    Als.Select (2)
    sleep 1
    Durchsuchen.click

    Kontext "GrafikEinfuegenDlg"
    Dateiname.SetText testFileWrapping
    sleep 1
    Oeffnen.Click
    sleep 2
    
    Kontext "TabHintergrund"
    Position.Check
    
    TabHintergrund.OK
    PrintLog " choose wrappingSection , then press options button ,"
    PrintLog " + then tabpage backgroud / graphic, browse a picture ,"
    PrintLog " + choose 'area' in type ."
    Kontext "BereicheBearbeiten"
    Bereich.TypeKeys "<Down>"
    'press options button
    Optionen.Click

    Kontext
    Active.SetPage TabHintergrund
    sleep 1
    Kontext "TabHintergrund"

    Als.Select (2)
    sleep 1
    Durchsuchen.click

    Kontext "GrafikEinfuegenDlg"
    Dateiname.SetText testFileWrapped
    sleep 1
    Oeffnen.Click
    sleep 2

    Kontext "TabHintergrund"
    Flaeche.Check
    TabHintergrund.OK

    Kontext "BereicheBearbeiten"
    BereicheBearbeiten.OK

    PrintLog " format/section again,"
    PrintLog " + choose wrappingSection , then press options button,"
    PrintLog " + then tabpage background / graphic,"
    PrintLog " + check if 'Position' is checked in type"
    FormatSections
    Kontext "BereicheBearbeiten"
    Bereich.TypeKeys "<Up>"
    'press options button
    Optionen.Click
    
    Kontext
    Active.SetPage TabHintergrund
    sleep 1
    Kontext "TabHintergrund"
    Als.Select (2)
    sleep 1
    
    if Position.IsChecked <> TRUE then
        Warnlog "'Position' is NOT checked in type"
    end if
    
    TabHintergrund.Cancel
    
    PrintLog " choose wrappedSection , then press options button,"
    PrintLog " + then tabpage backgroud / graphic,"
    PrintLog " + check if 'Area' is checked in type ."
    Kontext "BereicheBearbeiten"
    Bereich.TypeKeys "<Down>"
    'press options button
    Optionen.Click
    
    Kontext
    Active.SetPage TabHintergrund
    sleep 1
    Kontext "TabHintergrund"
    Als.Select (2)
    sleep 1
    
    if Flaeche.IsChecked <> TRUE then
        Warnlog "'Area' is NOT checked in type"
    end if
    TabHintergrund.Cancel
    
    Kontext "BereicheBearbeiten"
    BereicheBearbeiten.Cancel
    
    Call hCloseDocument
endcase
