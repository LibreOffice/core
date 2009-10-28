'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_grid_layout1.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: jsk $ $Date: 2008-06-17 07:50:24 $
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
'* owner : thorsten.bosbach@sun.com
'*
'* short description : Grid Layout for CJK test in Writer
'*
'\***********************************************************************

private const SOURCE_PATH = "framework\optional\input\CJK\"

testcase tTextGridDialog_1

<<<<<<< local
    '/// Check if tabpage 'Text Grid' disappears when "Asian Language support" is NOT checked
    if iSystemSprache <> 81 AND iSystemSprache <> 82 AND iSystemSprache <> 86 AND iSystemSprache <> 88 then
        Call CheckAsianLanguageSupport("Off")
        hNewDocument()
        FormatPageWriter
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
=======
 if iSystemSprache <> 81 AND iSystemSprache <> 82 AND iSystemSprache <> 86 AND iSystemSprache <> 88 then
   Call CheckAsianLanguageSupport("Off")
   hNewDocument()
   FormatPageWriter
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
 end if
>>>>>>> other

<<<<<<< local
    '/// Check if tabpage 'Text Grid' appears when "Asian Language support" is checked
    Call CheckAsianLanguageSupport("On")
=======
   Call CheckAsianLanguageSupport("On")
>>>>>>> other

    sleep 5
    FormatPageWriter
    Kontext
    if Active.exists (10) then
        try
            Kontext
            Active.SetPage TabGrid
            Kontext "TabGrid"
            TabGrid.Cancel
        catch
            Kontext
            Active.SetPage TabSeite
            Kontext "TabSeite"
            TabSeite.Cancel
            warnlog "Couldn't get Tabpage: Grid Text"
        endcatch
    else
        warnlog "FormatPageWriter didn't bring up ndialog"
    endif
    hCloseDocument()
endcase

'-------------------------------------------------------------------------

testcase tTextGridDialog_2
<<<<<<< local
    '/// No grid is active, no changes to existing version
    Dim testFile , sCorrectResult as String
=======
   Dim testFile , sCorrectResult as String
>>>>>>> other

    testFile       = "gridtest.sxw"
    sCorrectResult = "1"

    Call hNewDocument

<<<<<<< local
    '/// open a test file , and check No Grid checkbox
    Call hFileOpen ( gTesttoolPath + SOURCE_PATH + testFile )
    sMakeReadOnlyDocumentEditable

=======
   Call hFileOpen ( gTesttoolPath + SOURCE_PATH + testFile )
   sMakeReadOnlyDocumentEditable
   
>>>>>>> other
    FormatPageWriter
    Kontext
    if active.exists(5) then
        Active.SetPage TabGrid
    endif
    Kontext "TabGrid"
    NoGrid.Check
    Sleep 1
    TabGrid.OK
    Sleep 1

<<<<<<< local
    '/// Active navigator dialogue , set page to '2'  and wait for a while
=======
>>>>>>> other
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"

    Seitennummer.SetText "2"
    Sleep 5
    hCloseNavigator

<<<<<<< local
    '/// In No Grid mode, there should be only 1 page, the focus is before the first line
=======
>>>>>>> other
    Kontext "DocumentWriter"
    Call DocumentWriter.TypeKeys  "<End>"
    Sleep 1
    Call DocumentWriter.TypeKeys  "<Shift Left>"
    hUseAsyncSlot( "EditCopy" )

    if GetClipboardText <> sCorrectResult then Warnlog "Something wrong when choosing NoGrid option!"

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tTextGridDialog_3
<<<<<<< local
    '/// Grid (lines only): The page is set up with a defined number of lines
    Dim testFile  , sCorrectResult , sLinesPerPage as String
=======
   Dim testFile  , sCorrectResult , sLinesPerPage as String
>>>>>>> other

    testFile       = "gridtest.sxw"
    sCorrectResult = "G"
    sLinesPerPage  = "10"

    Call hNewDocument

<<<<<<< local
    '/// open a test file and check Grid(lines onle) checkbox ,
    '/// + set 10 line/page
    Call hFileOpen ( gTesttoolPath + SOURCE_PATH + testFile )
    sMakeReadOnlyDocumentEditable
=======
   Call hFileOpen ( gTesttoolPath + SOURCE_PATH + testFile )
   sMakeReadOnlyDocumentEditable
>>>>>>> other
    FormatPageWriter
    Kontext
    if active.exists(5) then
        Active.SetPage TabGrid
    endif
    Kontext "TabGrid"
    LinesGrid.Check
    LinesPerPage.SetText sLinesPerPage
    Sleep 1
    TabGrid.OK
    Sleep 1

<<<<<<< local
    '/// Active navigator dialogue , set page to '2'  and wait for a while
=======
>>>>>>> other
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"

    Seitennummer.SetText "2"
    Sleep 5
    hCloseNavigator

    Kontext "DocumentWriter"
    Call DocumentWriter.TypeKeys  "<End>"
    Sleep 1
    Call DocumentWriter.TypeKeys  "<Shift Left>"
    hUseAsyncSlot( "EditCopy" )

    if GetClipboardText <> sCorrectResult then Warnlog "Something wrong when choosing Grid(lines only) option!"

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tTextGridDialog_4
<<<<<<< local
    '/// Grid (lines and characters): The page is set up with a defined number of lines and a defined number of characters in each line
    Dim testFile      , sCorrectResult as String
    Dim sLinesPerPage , sCharsPerLine  as String
=======
   Dim testFile      , sCorrectResult as String
   Dim sLinesPerPage , sCharsPerLine  as String
>>>>>>> other

    testFile       = "gridtest.sxw"
    sCorrectResult = "E"
    sLinesPerPage  = "10"
    sCharsPerLine  = "9"

    Call hNewDocument

<<<<<<< local
    '/// open a test file and check Grid(lines and characters) checkbox
    '/// + set 10 line/page and 9 characters/line
    Call hFileOpen ( gTesttoolPath + SOURCE_PATH + testFile )
    sMakeReadOnlyDocumentEditable
=======
   Call hFileOpen ( gTesttoolPath + SOURCE_PATH + testFile )
   sMakeReadOnlyDocumentEditable
>>>>>>> other
    FormatPageWriter
    Kontext
    if active.exists(5) then
        Active.SetPage TabGrid
    endif
    Kontext "TabGrid"
    CharsGrid.Check
    LinesPerPage.SetText sLinesPerPage
    CharsPerLine.SetText sCharsPerLine
    Sleep 1
    TabGrid.OK
    Sleep 1

<<<<<<< local
    '/// Active navigator dialogue , set page to '2'  and wait for a while
=======
>>>>>>> other
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"

    Seitennummer.SetText "2"
    Sleep 5
    hCloseNavigator

    Kontext "DocumentWriter"
    Call DocumentWriter.TypeKeys  "<End>"
    Sleep 1
    Call DocumentWriter.TypeKeys  "<Shift Left>"
    hUseAsyncSlot( "EditCopy" )

    if GetClipboardText <> sCorrectResult then Warnlog "Something wrong when choosing Grid(lines and characters) option!"

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase  tGridDisplay_1
<<<<<<< local
    '/// Print grid: The grid lines will be printed. Inactive if "Display grid" is inactive.
    Call hNewDocument
=======
   Call hNewDocument
>>>>>>> other

<<<<<<< local
    '/// format / page  / tabpage Text grid
    '/// + check "display grid" and check "print grid"
=======
>>>>>>> other
    FormatPageWriter
    Kontext
    if active.exists(5) then
        Active.SetPage TabGrid
    endif
    Kontext "TabGrid"
    CharsGrid.Check
    Display.Check
    Sleep 1
    PrintGrid.Check
    Sleep 1
    TabGrid.OK
    Sleep 1

<<<<<<< local
    '/// Print to printer
=======
>>>>>>> other
    FilePrint
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

<<<<<<< local
    '/// format / page  / tabpage Text grid
    '/// + check "display grid" and uncheck "print grid"
=======
>>>>>>> other
    FormatPageWriter
    Kontext
    if active.exists(5) then
        Active.SetPage TabGrid
    endif
    Kontext "TabGrid"
    CharsGrid.Check
    Display.Check
    Sleep 1
    PrintGrid.UnCheck
    Sleep 1
    TabGrid.OK
    Sleep 1

<<<<<<< local
    '/// Print to printer
=======
>>>>>>> other
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
        if( Printing.exists( 1 )) then
            Printing.OK
            Printing.notExists( 5 )
        endif
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
<<<<<<< local
    '/// Decrease characters per line will cause lines per page decrease accordingly
    Dim sLinesPerPage       , sCharsPerLine          as String
    Dim sExpectLinesPerPage , sExpectLinesPerPage1   as String
=======
   Dim sLinesPerPage       , sCharsPerLine          as String
   Dim sExpectLinesPerPage , sExpectLinesPerPage1   as String
>>>>>>> other

    sLinesPerPage  = "20"
    sCharsPerLine  = "2"
    sExpectLinesPerPage   = "2"
    sExpectLinesPerPage1  = "3"

    Call hNewDocument

<<<<<<< local
    '/// Format/page , set lines per page to 20, and set charsets per line to 2
    '/// + wait for a moment , then check the the number  of lines per page.
    FormatPageWriter
    Kontext
    if active.exists(5) then
        Active.SetPage TabGrid
    endif
    Kontext "TabGrid"
    CharsGrid.Check
=======
   FormatPageWriter
   Kontext
   if active.exists(5) then
       Active.SetPage TabGrid
   endif
   Kontext "TabGrid"
   CharsGrid.Check
>>>>>>> other

    LinesPerPage.SetText sLinesPerPage
    Sleep 2
    CharsPerLine.SetText sCharsPerLine
    Sleep 2

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
<<<<<<< local
    '/// Change the font size to bigger enough will change the layout
=======
>>>>>>> other
    Dim testFile      , sCorrectResult as String
    Dim sLinesPerPage , sCharsPerLine  as String
    dim sTemp as string

    testFile       = "FontSizeChanges_1.sxw"
    sCorrectResult = "1"
    sLinesPerPage  = "2"
    sCharsPerLine  = "9"

    Call hFileOpen ( gTesttoolPath + SOURCE_PATH + testFile )
    sMakeReadOnlyDocumentEditable
    Kontext "DocumentWriter"
    Call DocumentWriter.TypeKeys  "<Shift Right>"

    Kontext "FormatObjectbar"
    Schriftgroesse.Select   "60"
    Schriftgroesse.TypeKeys "<Return>"
    Sleep 1

    Kontext "DocumentWriter"
    FormatPageWriter
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
        Seitennummer.SetText "2"
        Sleep 5
    else
        warnlog "Navigator not available"
    endif
    hCloseNavigator

    Kontext "DocumentWriter"
    sleep 3
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
<<<<<<< local
    '/// If Ruby Text is applied and longer than its Base Text it will stretch over more grid cells than the base text itself.
    Dim testFile , sRubyText as String
=======
   Dim testFile , sRubyText as String
>>>>>>> other

    testFile   = "rubytest.sxw"
    sRubyText  = "Ruby Test Text ....................."

    Call hNewDocument

<<<<<<< local
    '/// open a test file , and insert  long ruby test text
    Call hFileOpen ( gTesttoolPath + SOURCE_PATH + testFile )
=======
   Call hFileOpen ( gTesttoolPath + SOURCE_PATH + testFile )
>>>>>>> other
    sMakeReadOnlyDocumentEditable
    Kontext "DocumentWriter"
    Call DocumentWriter.TypeKeys  "<Shift Right>"
    Sleep 1

    FormatRuby
    Kontext "RubyDialog"
    Sleep 3
    RubyText1.setText sRubyText
    Sleep 2
    RubyDialog.OK
    Sleep 2
    DialogClose.Click
    Sleep 1

    Kontext "DocumentWriter"
    FormatAutoformatApply
    Sleep 2
    Call DocumentWriter.TypeKeys  "<End>"
    Sleep 2
    Call DocumentWriter.TypeKeys  "<Shift Left>"
    hUseAsyncSlot( "EditCopy" )

    if NOT(GetClipboardText = "5" OR GetClipboardText = "6") then
        Warnlog "Don't get the correct result after inserting ruby text! Get " + GetClipboardText
    endif

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tIndentsTest_1
<<<<<<< local
    '/// Indents should work and always indenting as close as possible to their measurements by snapping to the appropriate grid cell. Tabs therefore will be ignored.
    Dim testFile , sCorrectPage1 , sCorrectPage2 as String
=======
   Dim testFile , sCorrectPage1 , sCorrectPage2 as String
>>>>>>> other

    testFile       = "IndentsTest.sxw"
    sCorrectPage1  = "1"
    sCorrectPage2  = "2"

    Call hNewDocument

<<<<<<< local
    '/// open a test file , and press TAB in second line
    Call hFileOpen ( gTesttoolPath + SOURCE_PATH + testFile )
=======
   Call hFileOpen ( gTesttoolPath + SOURCE_PATH + testFile )
>>>>>>> other
    sMakeReadOnlyDocumentEditable
    Kontext "DocumentWriter"
    Call DocumentWriter.TypeKeys  "<Down>"
    Sleep 1
    Call DocumentWriter.TypeKeys  "<Tab>"
    Sleep 1
    Call DocumentWriter.TypeKeys  "<End>"
    Sleep 1

<<<<<<< local
    '/// Active navigator dialogue , check if the page number is 1
=======
>>>>>>> other
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    if Seitennummer.GetText <> sCorrectPage1 then Warnlog "First test :Something wrong in indents test!"
    hCloseNavigator

<<<<<<< local
    '/// press TAB in second line again
=======
>>>>>>> other
    Kontext "DocumentWriter"
    Call DocumentWriter.TypeKeys  "<Home>"
    Sleep 1
    Call DocumentWriter.TypeKeys  "<Tab>"
    Sleep 1
    Call DocumentWriter.TypeKeys  "<End>"
    Sleep 1

<<<<<<< local
    '/// Active navigator dialogue , check if the page number is 2
=======
>>>>>>> other
    Kontext
    ViewNavigator
    Kontext "NavigatorWriter"
    if Seitennummer.GetText <> sCorrectPage2 then Warnlog "Second test :Something wrong in indents test!"
    hCloseNavigator

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tObjectsTest_1
<<<<<<< local
    '/// If objects are anchored to text they will "wander" with its surrounding text.
    '/// If they are anchored as character they will snap to the appropriate grid cell (since they are a character).
    Dim testFile , sCorrectPage1 , sCorrectPage2 as String
=======
   Dim testFile , sCorrectPage1 , sCorrectPage2 as String
>>>>>>> other

    testFile       = "objectsTest.sxw"
    sCorrectPage1  = "1"
    sCorrectPage2  = "2"

    Call hNewDocument

<<<<<<< local
    '/// open a test file , and press TAB in second line
    Call hFileOpen ( gTesttoolPath + SOURCE_PATH + testFile )
=======
   Call hFileOpen ( gTesttoolPath + SOURCE_PATH + testFile )
>>>>>>> other

<<<<<<< local
    '/// If object is anchored as Character , check if it will snap to the appropriate grid cell
=======
>>>>>>> other
    fStartupNavigator( "OLEObject" , 1 )
    Sleep 1
    Auswahlliste.TypeKeys "<Return>"
    Sleep 1
    hCloseNavigator

    Kontext
    FormatAnchorAsCharacter 
    WaitSlot()


    ViewNavigator
    Kontext "NavigatorWriter"
    Seitennummer.SetText   sCorrectPage2
    Sleep 5
    if Seitennummer.GetText <> sCorrectPage2 then Warnlog "Something wrong when object is anchored as Character!"
    hCloseNavigator

<<<<<<< local
    '/// If object is anchored to Character , check if it will NOT snap to the appropriate grid cell
=======
>>>>>>> other
    fStartupNavigator( "OLEObject" , 1 )
    Auswahlliste.TypeKeys "<Return>"
    hCloseNavigator

    Kontext
    FormatAnchorToCharacter

    ViewNavigator
    Kontext "NavigatorWriter"
    Seitennummer.SetText   sCorrectPage2
    Sleep 5
    if Seitennummer.GetText <> sCorrectPage1 then Warnlog "Something wrong when object is anchored to Character!"
    hCloseNavigator
    Kontext

    Call hCloseDocument
endcase

