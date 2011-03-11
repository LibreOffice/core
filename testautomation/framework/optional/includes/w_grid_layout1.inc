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
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : Grid Layout for CJK test in Writer
'*
'\***********************************************************************

private const SOURCE_PATH = "framework\optional\input\CJK\"

testcase tTextGridDialog_1

    dim lsSave( 3 ) as string
    dim lbSave( 3 ) as boolean

    kontext "Navigator"
    hCloseDialog( Navigator, "close,optional" )

    '/// Check if tabpage 'Text Grid' disappears when "Asian Language support" is NOT checked
    if iSystemSprache <> 81 AND iSystemSprache <> 82 AND iSystemSprache <> 86 AND iSystemSprache <> 88 then
        Call CheckAsianLanguageSupport("Off")
        hNewDocument()
        hUseAsyncSlot( "FormatPageWriter" )
        try
            Kontext
            Active.SetPage TabGrid
            Kontext "TabGrid"
            TabGrid.Cancel
            warnlog "Shouldn't get Tabpage: Grid Text"
        catch
            Kontext
            Active.SetPage TabSeite
            Kontext "TabSeite"
            TabSeite.Cancel
        endcatch
    endif

    '/// Check if tabpage 'Text Grid' appears when "Asian Language support" is checked
    Call CheckAsianLanguageSupport("On")

    hNewDocument
    ToolsOptions
    hToolsOptions ( "StarOffice", "General" )

    printlog " - save old data"
    lsSave ( 1 ) = Zweistellig.GetText
    lbSave ( 1 ) = Aktivieren.IsChecked
    Aktivieren.Check
    if gPlatgroup = "w95" then
        lbSave (2) = StarOfficeDialogeBenutzen.IsChecked
    end if
    lbSave ( 3 ) = DruckenStatus.IsChecked
    lsSave ( 3 ) = StyleSheet.GetSelText

    printlog " - invert/change data"
    Zweistellig.SetText "1950"
    Zuruecksetzen.Click
    if gPlatgroup = "w95" then
        if lbSave (2) = TRUE then StarOfficeDialogeBenutzen.Uncheck else StarOfficeDialogeBenutzen.Check
    end if
    If lbSave ( 3 ) = TRUE then DruckenStatus.Uncheck else DruckenStatus.Check
    StyleSheet.Select 3

    Kontext "ExtrasOptionenDlg"
    hCloseDialog( ExtrasOptionenDlg, "ok" )
    hFileCloseAll()

endcase

'-------------------------------------------------------------------------

testcase tTextGridDialog_2
    '/// No grid is active, no changes to existing version
    Dim testFile , sCorrectResult as String

    testFile       = "gridtest.sxw"
    sCorrectResult = "1"

    Call hNewDocument

    '/// open a test file , and check No Grid checkbox
    Call hFileOpenLocally( gTesttoolPath + SOURCE_PATH + testFile )

    hUseAsyncSlot( "FormatPageWriter" )
    Kontext
    if active.exists(5) then
        Active.SetPage TabGrid
    endif
    Kontext "TabGrid"
    NoGrid.Check
    hCloseDialog( TabGrid, "ok" )


    '/// Active navigator dialogue , set page to '2'  and wait for a while
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Seitennummer.SetText "2" : Wait( 2000 )
    kontext "Navigator"
    hCloseDialog( Navigator, "close" )

    '/// In No Grid mode, there should be only 1 page, the focus is before the first line
    Kontext "DocumentWriter"
    Call DocumentWriter.TypeKeys  "<End>"
    wait( 200 ) ' sleep 1
    Call DocumentWriter.TypeKeys  "<Shift Left>"
    hUseAsyncSlot( "EditCopy" )

    if GetClipboardText <> sCorrectResult then Warnlog "Something wrong when choosing NoGrid option!"

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tTextGridDialog_3
    '/// Grid (lines only): The page is set up with a defined number of lines
    Dim testFile  , sCorrectResult , sLinesPerPage as String

    testFile       = "gridtest.sxw"
    sCorrectResult = "G"
    sLinesPerPage  = "10"

    Call hNewDocument

    '/// open a test file and check Grid(lines onle) checkbox ,
    '/// + set 10 line/page
    Call hFileOpenLocally( gTesttoolPath + SOURCE_PATH + testFile )
    hUseAsyncSlot( "FormatPageWriter" )
    Kontext
    if active.exists(5) then
        Active.SetPage TabGrid
    endif
    Kontext "TabGrid"
    LinesGrid.Check
    LinesPerPage.SetText sLinesPerPage
    hCloseDialog( TabGrid, "ok" )

    '/// Active navigator dialogue , set page to '2'  and wait for a while
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Seitennummer.SetText "2" : Wait( 2000 )
    kontext "Navigator"
    hCloseDialog( Navigator, "close" )

    Kontext "DocumentWriter"
    Call DocumentWriter.TypeKeys  "<End>"
    wait( 200 ) ' sleep 1
    Call DocumentWriter.TypeKeys  "<Shift Left>"
    hUseAsyncSlot( "EditCopy" )

    if GetClipboardText <> sCorrectResult then Warnlog "Something wrong when choosing Grid(lines only) option!"

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tTextGridDialog_4
    '/// Grid (lines and characters): The page is set up with a defined number of lines and a defined number of characters in each line
    Dim testFile      , sCorrectResult as String
    Dim sLinesPerPage , sCharsPerLine  as String

    testFile       = "gridtest.sxw"
    sCorrectResult = "E"
    sLinesPerPage  = "10"
    sCharsPerLine  = "9"

    Call hNewDocument

    '/// open a test file and check Grid(lines and characters) checkbox
    '/// + set 10 line/page and 9 characters/line
    Call hFileOpenLocally( gTesttoolPath + SOURCE_PATH + testFile )
    hUseAsyncSlot( "FormatPageWriter" )
    Kontext
    if active.exists(5) then
        Active.SetPage TabGrid
    endif
    Kontext "TabGrid"
    CharsGrid.Check
    LinesPerPage.SetText sLinesPerPage
    CharsPerLine.SetText sCharsPerLine
    hCloseDialog( TabGrid, "ok" )

    '/// Active navigator dialogue , set page to '2'  and wait for a while
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    Seitennummer.SetText "2" : Wait( 2000 )
    kontext "Navigator"
    hCloseDialog( Navigator, "close" )

    Kontext "DocumentWriter"
    Call DocumentWriter.TypeKeys  "<End>"
    wait( 200 ) ' sleep 1
    Call DocumentWriter.TypeKeys  "<Shift Left>"
    hUseAsyncSlot( "EditCopy" )

    if GetClipboardText <> sCorrectResult then Warnlog "Something wrong when choosing Grid(lines and characters) option!"

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase  tGridDisplay_1
    '/// Print grid: The grid lines will be printed. Inactive if "Display grid" is inactive.
    Call hNewDocument

    '/// format / page  / tabpage Text grid
    '/// + check "display grid" and check "print grid"
    hUseAsyncSlot( "FormatPageWriter" )
    Kontext
    if active.exists(5) then
        Active.SetPage TabGrid
    endif
    Kontext "TabGrid"
    CharsGrid.Check
    Display.Check
    PrintGrid.Check
    hCloseDialog( TabGrid, "ok" )


    '/// Print to printer
    hUseAsyncSlot( "FilePrint" )
    Kontext "Active"
    if ( Active.Exists( 5 ) ) then
        if Active.GetRT() = 304 then
            Active.Ok()
            QAErrorLog "No Default-Printer!"
            Kontext "Printing"
            if ( Printing.exists( 1 )) then
                Printing.Cancel
            endif
        endif
    else
        Kontext "Printing"
        if ( Printing.exists( 2 )) then
            Printing.OK()
            Printing.notExists( 5 )
        endif

        kontext "Active"
        if active.exists(5) then
            qaErrorLog active.getText()
            active.ok()
        endif
    endif

    '/// format / page  / tabpage Text grid
    '/// + check "display grid" and uncheck "print grid"
    FormatPageWriter
    Kontext
    if active.exists(5) then
        Active.SetPage TabGrid
    endif
    Kontext "TabGrid"
    CharsGrid.Check
    Display.Check
    PrintGrid.UnCheck
    hCloseDialog( TabGrid, "ok" )

    '/// Print to printer
    FilePrint
    Kontext "Active"
    if ( Active.Exists( 5 ) )then
        if Active.GetRT() = 304 then
            Active.Ok()
            QAErrorLog "No Default-Printer!"
            Kontext "Printing"
            if ( Printing.exists( 1 )) then
                Printing.Cancel()
            endif
        endif
    else
        Kontext "Printing"
        hCloseDialog( Printing, "ok" )

        kontext
        if ( active.exists( 2 ) ) then
            qaErrorLog active.getText
            active.ok
        endif
    endif

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tUserDefineLayout_1
    '/// Decrease characters per line will cause lines per page decrease accordingly
    Dim sLinesPerPage       , sCharsPerLine          as String
    Dim sExpectLinesPerPage , sExpectLinesPerPage1   as String

    sLinesPerPage  = "20"
    sCharsPerLine  = "2"
    sExpectLinesPerPage   = "2"
    sExpectLinesPerPage1  = "3"

    Call hNewDocument

    '/// Format/page , set lines per page to 20, and set charsets per line to 2
    '/// + wait for a moment , then check the the number  of lines per page.
    hUseAsyncSlot( "FormatPageWriter" )
    Kontext
    if active.exists(5) then
        Active.SetPage TabGrid
    endif
    Kontext "TabGrid"
    CharsGrid.Check

    LinesPerPage.SetText sLinesPerPage
    CharsPerLine.SetText sCharsPerLine

    'This step is no use, just let the LinesPerPage change the number
    CharsGrid.Check

    if LinesPerPage.GetText <> sExpectLinesPerPage AND LinesPerPage.GetText <> sExpectLinesPerPage1 then
        Warnlog "The lines per page should be"  + sExpectLinesPerPage + " OR " + sExpectLinesPerPage1 +" but get " +LinesPerPage.GetText
    endif

    TabGrid.Cancel

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tFontSizeChanges_1
    '/// Change the font size to bigger enough will change the layout
    Dim testFile      , sCorrectResult as String
    Dim sLinesPerPage , sCharsPerLine  as String
    dim sTemp as string

    testFile       = "FontSizeChanges_1.sxw"
    sCorrectResult = "1"
    sLinesPerPage  = "2"
    sCharsPerLine  = "9"

    Call hFileOpenLocally( gTesttoolPath + SOURCE_PATH + testFile )
    Kontext "DocumentWriter"
    Call DocumentWriter.TypeKeys  "<Shift Right>"

    Kontext "FormatObjectbar"
    Schriftgroesse.Select   "60"
    Schriftgroesse.TypeKeys "<Return>"

    Kontext "DocumentWriter"
    hUseAsyncSlot( "FormatPageWriter" )
    Kontext
    if active.exists(5) then
        Active.SetPage TabGrid
    endif
    Kontext "TabGrid"
    if TabGrid.exists(5) then
        sTemp = LinesPerPage.GetText
        if sTemp <> sLinesPerPage then
            Warnlog "The lines per page should be: '" + sLinesPerPage + "'; but get: '" + sTemp + "'"
        endif
        sTemp = CharsPerLine.GetText
        if sTemp <> sCharsPerLine then
            Warnlog "The chars per line should be: '" + sCharsPerLine + "'; but get: '" +sTemp + "'"
        endif
        TabGrid.Cancel
    else
        warnlog "TabGrid is not available."
    endif

    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    if NavigatorWriter.exists(5) then
        Seitennummer.SetText "2" : Wait( 2000 )
        kontext "Navigator"
        hCloseDialog( Navigator, "close" )
    else
        warnlog( "Navigator not open" )
    endif

    Kontext "DocumentWriter"
    wait( 200 ) ' sleep 3
    Call DocumentWriter.TypeKeys  "<Shift Right>"

    hUseAsyncSlot( "EditCopy" )
    sTemp = GetClipboardText
    if sTemp <> sCorrectResult then
        Warnlog "Don't get the expected result , hope to be: '" + sCorrectResult + "'; but get: '" + sTemp + "'"
    endif

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tRubyText_1

    '/// If Ruby Text is applied and longer than its Base Text it will stretch over more grid cells than the base text itself.
    const TEST_FILE = "rubytest.sxw"
    const RUBY_TEXT = "Ruby Test Text ....................."

    printlog( "Create a new document" )
    Call hNewDocument

    printlog( "Open test file" )
    hFileOpenLocally( gTesttoolPath + SOURCE_PATH + TEST_FILE )

    Kontext "DocumentWriter"
    printlog( "Mark character to the right of current cursor position" )
    Call DocumentWriter.TypeKeys  "<Shift Right>"

    printlog( "Open <Format Ruby> dialog" )
    hUseAsyncSlot( "FormatRuby" )

    Kontext "RubyDialog"
    if ( RubyDialog.exists( 3 ) ) then
        RubyText1.setText( RUBY_TEXT )
        RubyDialog.OK
        DialogClose.Click
    else
        warnlog( "Ruby dialog not open" )
    endif

    Kontext "DocumentWriter"
    printlog( "Apply Autoformat" )
    hUseAsyncSlot( "FormatAutoformatApply" )

    printlog( "Select last character in current line" )
    Call DocumentWriter.TypeKeys  "<End>"
    Wait( 200 )
    Call DocumentWriter.TypeKeys  "<Shift Left>"
    Wait( 200 )

    printlog( "<EditCopy>" )
    hUseAsyncSlot( "EditCopy" )

    if NOT(GetClipboardText = "5" OR GetClipboardText = "6") then
        Warnlog "Don't get the correct result after inserting ruby text! Get " + GetClipboardText
    endif

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tIndentsTest_1
    '/// Indents should work and always indenting as close as possible to their measurements by snapping to the appropriate grid cell. Tabs therefore will be ignored.

    const TEST_FILE      = "IndentsTest.sxw"
    const CORRECT_PAGE_1 = "1"
    const CORRECT_PAGE_2 = "2"

    Call hNewDocument

    '/// open a test file , and press TAB in second line
    hFileOpenLocally( gTesttoolPath + SOURCE_PATH + TEST_FILE )

    Kontext "DocumentWriter"
    Call DocumentWriter.TypeKeys  "<Down>"
    wait( 200 ) ' sleep 1
    Call DocumentWriter.TypeKeys  "<Tab>"
    wait( 200 ) ' sleep 1
    Call DocumentWriter.TypeKeys  "<End>"
    wait( 200 ) ' sleep 1

    '/// Active navigator dialogue , check if the page number is 1
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    if ( Seitennummer.GetText <> CORRECT_PAGE_1 ) then
        Warnlog "First test :Something wrong in indents test!"
    endif
    kontext "Navigator"
    hCloseDialog( Navigator, "close" )

    '/// press TAB in second line again
    Kontext "DocumentWriter"
    Call DocumentWriter.TypeKeys  "<Home>"
    wait( 200 ) ' sleep 1
    Call DocumentWriter.TypeKeys  "<Tab>"
    wait( 200 ) ' sleep 1
    Call DocumentWriter.TypeKeys  "<End>"
    wait( 200 ) ' sleep 1

    '/// Active navigator dialogue , check if the page number is 2
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    if ( Seitennummer.GetText <> CORRECT_PAGE_2 ) then
        Warnlog "Second test :Something wrong in indents test!"
    endif
    kontext "Navigator"
    hCloseDialog( Navigator, "close" )

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tObjectsTest_1
    '/// If objects are anchored to text they will "wander" with its surrounding text.
    '/// If they are anchored as character they will snap to the appropriate grid cell (since they are a character).

    const TEST_FILE      = "objectsTest.sxw"
    const CORRECT_PAGE_1 = "1"
    const CORRECT_PAGE_2 = "2"

    Call hNewDocument

    '/// open a test file , and press TAB in second line
    Call hFileOpen ( gTesttoolPath + SOURCE_PATH + TEST_FILE )

    '/// If object is anchored as Character , check if it will snap to the appropriate grid cell
    fStartupNavigator( "OLEObject" , 1 )
    wait( 200 ) ' sleep 1
    Auswahlliste.TypeKeys "<Return>"
    wait( 200 ) ' sleep 1
    kontext "Navigator"
    hCloseDialog( Navigator, "close" )

    Kontext
    FormatAnchorAsCharacter
    WaitSlot()


    ViewNavigator
    Kontext "NavigatorWriter"
    Seitennummer.SetText( CORRECT_PAGE_1 )
    WaitSlot( 5000 ) ' wait( 200 ) ' sleep 5
    if ( Seitennummer.GetText <> CORRECT_PAGE_1 ) then
        Warnlog "Something wrong when object is anchored as Character!"
    endif
    kontext "Navigator"
    hCloseDialog( Navigator, "close" )

    '/// If object is anchored to Character , check if it will NOT snap to the appropriate grid cell
    fStartupNavigator( "OLEObject" , 1 )
    Auswahlliste.TypeKeys "<Return>"
    kontext "Navigator"
    hCloseDialog( Navigator, "close" )

    Kontext
    FormatAnchorToCharacter

    ViewNavigator
    Kontext "NavigatorWriter"
    Seitennummer.SetText( CORRECT_PAGE_2 )
    WaitSlot( 5000 ) ' wait( 200 ) ' sleep 5
    if ( Seitennummer.GetText <> CORRECT_PAGE_1 ) then
        Warnlog "Something wrong when object is anchored to Character!"
    endif
    kontext "Navigator"
    hCloseDialog( Navigator, "close" )

    Call hCloseDocument
endcase

