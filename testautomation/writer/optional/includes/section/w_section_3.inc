'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************

**
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
