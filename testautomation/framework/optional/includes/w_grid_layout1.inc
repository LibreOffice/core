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

   Call CheckAsianLanguageSupport("On")

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
   Dim testFile , sCorrectResult as String

   testFile       = "gridtest.sxw"
   sCorrectResult = "1"

   Call hNewDocument

   Call hFileOpen ( gTesttoolPath + SOURCE_PATH + testFile )
   sMakeReadOnlyDocumentEditable
   
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

    Kontext
    ViewNavigator
     Kontext "NavigatorWriter"

     Seitennummer.SetText "2"
     Sleep 5
    fCloseNavigator

    Kontext "DocumentWriter"
    Call DocumentWriter.TypeKeys  "<End>"
    Sleep 1
    Call DocumentWriter.TypeKeys  "<Shift Left>"
    Editcopy

    if GetClipboardText <> sCorrectResult then Warnlog "Something wrong when choosing NoGrid option!"

   Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tTextGridDialog_3
   Dim testFile  , sCorrectResult , sLinesPerPage as String

   testFile       = "gridtest.sxw"
   sCorrectResult = "G"
   sLinesPerPage  = "10"

   Call hNewDocument

   Call hFileOpen ( gTesttoolPath + SOURCE_PATH + testFile )
   sMakeReadOnlyDocumentEditable
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

    Kontext
    ViewNavigator
     Kontext "NavigatorWriter"

     Seitennummer.SetText "2"
     Sleep 5
    fCloseNavigator

    Kontext "DocumentWriter"
    Call DocumentWriter.TypeKeys  "<End>"
    Sleep 1
    Call DocumentWriter.TypeKeys  "<Shift Left>"
    Editcopy

    if GetClipboardText <> sCorrectResult then Warnlog "Something wrong when choosing Grid(lines only) option!"

   Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tTextGridDialog_4
   Dim testFile      , sCorrectResult as String
   Dim sLinesPerPage , sCharsPerLine  as String

   testFile       = "gridtest.sxw"
   sCorrectResult = "E"
   sLinesPerPage  = "10"
   sCharsPerLine  = "9"

   Call hNewDocument

   Call hFileOpen ( gTesttoolPath + SOURCE_PATH + testFile )
   sMakeReadOnlyDocumentEditable
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

    Kontext
    ViewNavigator
     Kontext "NavigatorWriter"

     Seitennummer.SetText "2"
     Sleep 5
    fCloseNavigator

    Kontext "DocumentWriter"
    Call DocumentWriter.TypeKeys  "<End>"
    Sleep 1
    Call DocumentWriter.TypeKeys  "<Shift Left>"
    Editcopy

    if GetClipboardText <> sCorrectResult then Warnlog "Something wrong when choosing Grid(lines and characters) option!"

   Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase  tGridDisplay_1
   Call hNewDocument

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

    FilePrint
    Kontext "Active"
    if Active.Exists(5) then
      if Active.GetRT = 304 then
        Active.Ok
        QAErrorLog "No Default-Printer!"
        Kontext "DruckenDlg"
        Sleep 1
        DruckenDlg.Cancel
      end if
    else
      Kontext "DruckenDlg"
      Sleep 1
      DruckenDlg.OK
      Sleep 5
      kontext
      if active.exists(5) then
          qaErrorLog active.getText
          active.ok
      end if
    end if

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

    FilePrint
    Kontext "Active"
    if Active.Exists(5) then
      if Active.GetRT = 304 then
        Active.Ok
        QAErrorLog "No Default-Printer!"
        Kontext "DruckenDlg"
        Sleep 1
        DruckenDlg.Cancel
      end if
    else
      Kontext "DruckenDlg"
      Sleep 1
      DruckenDlg.OK
      Sleep 5
      kontext
      if active.exists(5) then
          qaErrorLog active.getText
          active.ok
      end if
    end if

   Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tUserDefineLayout_1
   Dim sLinesPerPage       , sCharsPerLine          as String
   Dim sExpectLinesPerPage , sExpectLinesPerPage1   as String

   sLinesPerPage  = "20"
   sCharsPerLine  = "2"
   sExpectLinesPerPage   = "2"
   sExpectLinesPerPage1  = "3"

  Call hNewDocument

   FormatPageWriter
   Kontext
   if active.exists(5) then
       Active.SetPage TabGrid
   endif
   Kontext "TabGrid"
   CharsGrid.Check

    LinesPerPage.SetText sLinesPerPage
    Sleep 2
    CharsPerLine.SetText sCharsPerLine
    Sleep 2

   'This step is no use, just let the LinesPerPage change the number
    CharsGrid.Check

    if LinesPerPage.GetText <> sExpectLinesPerPage AND LinesPerPage.GetText <> sExpectLinesPerPage1 then
         Warnlog "The lines per page should be"  + sExpectLinesPerPage + " OR " + sExpectLinesPerPage1 +" but get " +LinesPerPage.GetText
    end if

   TabGrid.Cancel

  Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tFontSizeChanges_1
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
    fCloseNavigator

    Kontext "DocumentWriter"
    sleep 3
    Call DocumentWriter.TypeKeys  "<Shift Right>"
    sleep 3
    Editcopy
    sTemp = GetClipboardText
    if sTemp <> sCorrectResult then 
        Warnlog "Don't get the expected result , hope to be: '" + sCorrectResult + "'; but get: '" + sTemp + "'"
    endif

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tRubyText_1
   Dim testFile , sRubyText as String

   testFile   = "rubytest.sxw"
   sRubyText  = "Ruby Test Text ....................."

   Call hNewDocument

   Call hFileOpen ( gTesttoolPath + SOURCE_PATH + testFile )
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
    Editcopy

    if NOT(GetClipboardText = "5" OR GetClipboardText = "6") then
        Warnlog "Don't get the correct result after inserting ruby text! Get " + GetClipboardText
    end if

   Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tIndentsTest_1
   Dim testFile , sCorrectPage1 , sCorrectPage2 as String

   testFile       = "IndentsTest.sxw"
   sCorrectPage1  = "1"
   sCorrectPage2  = "2"

   Call hNewDocument

   Call hFileOpen ( gTesttoolPath + SOURCE_PATH + testFile )
    sMakeReadOnlyDocumentEditable
   Kontext "DocumentWriter"
   Call DocumentWriter.TypeKeys  "<Down>"
   Sleep 1
   Call DocumentWriter.TypeKeys  "<Tab>"
   Sleep 1
   Call DocumentWriter.TypeKeys  "<End>"
   Sleep 1

    Kontext
    ViewNavigator
     Kontext "NavigatorWriter"
     if Seitennummer.GetText <> sCorrectPage1 then Warnlog "First test :Something wrong in indents test!"
    fCloseNavigator

    Kontext "DocumentWriter"
    Call DocumentWriter.TypeKeys  "<Home>"
    Sleep 1
    Call DocumentWriter.TypeKeys  "<Tab>"
    Sleep 1
    Call DocumentWriter.TypeKeys  "<End>"
    Sleep 1

    Kontext
    ViewNavigator
     Kontext "NavigatorWriter"
     if Seitennummer.GetText <> sCorrectPage2 then Warnlog "Second test :Something wrong in indents test!"
    fCloseNavigator

   Call hCloseDocument
endcase

'-------------------------------------------------------------------------

testcase tObjectsTest_1
   Dim testFile , sCorrectPage1 , sCorrectPage2 as String

   testFile       = "objectsTest.sxw"
   sCorrectPage1  = "1"
   sCorrectPage2  = "2"

   Call hNewDocument

   Call hFileOpen ( gTesttoolPath + SOURCE_PATH + testFile )

    fStartupNavigator( "OLEObject" , 1 )
    Sleep 1
    Auswahlliste.TypeKeys "<Return>"
    Sleep 1
    fCloseNavigator
    Sleep 2

    Kontext
    FormatAnchorAsCharacter
    Sleep 1

    ViewNavigator
     Kontext "NavigatorWriter"
     Seitennummer.SetText   sCorrectPage2
     Sleep 5
     if Seitennummer.GetText <> sCorrectPage2 then Warnlog "Something wrong when object is anchored as Character!"
    fCloseNavigator

    fStartupNavigator( "OLEObject" , 1 )
    Auswahlliste.TypeKeys "<Return>"
    fCloseNavigator

    Kontext
    FormatAnchorToCharacter

    ViewNavigator
     Kontext "NavigatorWriter"
     Seitennummer.SetText   sCorrectPage2
     Sleep 5
     if Seitennummer.GetText <> sCorrectPage1 then Warnlog "Something wrong when object is anchored to Character!"
    fCloseNavigator
   Kontext

   Call hCloseDocument
endcase

