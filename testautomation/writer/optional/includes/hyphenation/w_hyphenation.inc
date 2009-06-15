'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_hyphenation.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: vg $ $Date: 2008-08-18 12:30:28 $
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
'* short description : hyphenation functionality test
'*
'************************************************************************
'*
' #1 tHyphenation_1
' #1 tHyphenation_2
' #1 tHyphenation_3
'*
'\***********************************************************************

testcase tHyphenation_1

  Dim testFile as String
  Dim testWord as String
  Dim firstPartOfTestWord as String
  Dim secondPartOfTestWord as String

  testFile             =  "tHyphenation.odt"
  testWord             =  "following"
  firstPartOfTestWord  =  "fol"
  secondPartOfTestWord =  "follow"

  PrintLog "- Test Hyphenation  using ctrl and -"

   printlog "Test Hyphenation  using ctrl and -"

   printlog "open a test file"
    Call hFileOpen ( gTesttoolPath + "writer\optional\input\hyphenation\" + testFile )
    Call sMakeReadOnlyDocumentEditable

    Call wTypeKeys "<End><Shift Mod1 Left>"

    EditCopy

   printlog "add '-' between 'fol' and 'lowing' , then press 3 times 'space bar' (pc). (7 times on linux)"
   printlog "+ check if fol in the end of the line"
    if GetClipboardText = testWord then
         Call wTypeKeys "<Right>" , 3
         Call wTypeKeys "<Mod1 SUBTRACT>"
         Call wTypeKeys "<Home>"
         if gPlatGroup <> "unx" then
             Call wTypeKeys "<SPACE>" ,3
         else
             Call wTypeKeys "<SPACE>" ,7
         end if

         Call wTypeKeys "<End><Left><Shift Mod1 Left>"
         EditCopy

         if  GetClipboardText <> firstPartOfTestWord then
             Warnlog "The hyphenation does NOT work well in first part!"
         end if

        printlog "add '-' between 'follow' and 'ing' , then press 6 times 'space bar'"
        printlog "+ check if follow in the end of the line ."
         Call wTypeKeys "<Mod1 z>",2
         Call wTypeKeys "<End>"
         Call wTypeKeys "<Left>",3
         Call wTypeKeys "<Mod1 SUBTRACT>"
         Call wTypeKeys "<Home>"
         if gPlatGroup <> "unx" then
             Call wTypeKeys "<SPACE>" ,3
         else
             Call wTypeKeys "<SPACE>" ,6
         end if
         Call wTypeKeys "<End><Left><Shift Mod1 Left>"
         EditCopy

         if  GetClipboardText = secondPartOfTestWord then
             Call wTypeKeys "<Home>"

         printlog "press 5 times 'space bar'"
         printlog "check if fol in the end of the line ."
             if gPlatGroup <> "unx" then
                 Call wTypeKeys "<SPACE>" ,3
             else
                 Call wTypeKeys "<SPACE>" ,5
             end if
             Call wTypeKeys "<End><Shift Mod1 Left>"
             EditCopy

             if  GetClipboardText <> firstPartOfTestWord then
                Warnlog "The hyphenation does NOT work well in first part, but it works fine in second part!"
             end if
         else
             Warnlog "The hyphenation does NOT work well in second part!"
         end if

    else
         Warnlog "The test word is wrong !!"
    end if

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tHyphenation_2

warnlog "#i102304# - outcommenting tHyphenation_2 due to bug."
goto endsub

  Dim testFile as String
  Dim testWord as String
  Dim firstPartOfTestWord as String
  Dim secondPartOfTestWord as String
  Dim testWordInHyphenationDlg as String

  testFile                 =  "tHyphenation.odt"
  testWord                 =  "following"
  testWordInHyphenationDlg =  "fol=low-ing"
  firstPartOfTestWord      =  "fol"
  secondPartOfTestWord     =  "follow"

  PrintLog "- Test Hyphenation  using Tools/Hyphenation ,test like fol-lowing"

   printlog "Test Hyphenation  using Tools/Hyphenation ,test like fol-lowing"

   printlog "open a test file"
    Call hFileOpen ( gTesttoolPath + "writer\optional\input\hyphenation\" + testFile )
    Call sMakeReadOnlyDocumentEditable

    Call wTypeKeys "<End><Shift Mod1 Left>"

    EditCopy

    if GetClipboardText <> testWord then
         Warnlog "The test word is wrong !!"
         goto endsub
    end if

   printlog "Press 3 times 'space bar'"
    Call wTypeKeys "<Home>"
         if gPlatGroup <> "unx" then
             Call wTypeKeys "<SPACE>" ,3
         else
             Call wTypeKeys "<SPACE>" ,6
         end if

   printlog "Tools/Hyphenation"
    ToolsLanguageHyphenate
    Kontext "Silbentrennung"

    if Not Silbentrennung.Exists then
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                Active.Yes

                Kontext "Active"
                if Active.Exists then
                    if Active.GetRT = 304 then Active.Ok
                end if
            end if
        end if
        Warnlog "Unable to bring up Dialog 'Hyphenation'!"
        goto endsub
    end if

   printlog "check if get 'fol=low-ing'"
    if Wort.Gettext <> testWordInHyphenationDlg then
        Warnlog "#i40561# The test can NOT be seperated two times , or something wrong!"
        Silbentrennung.Cancel
        goto NOTest
    end if

    if NOT Vor.IsEnabled then
        Warnlog "the Vor button is Not enabled, or something wrong!"
        Silbentrennung.Cancel
        goto NOTest
    end if

   printlog "add '-' between 'fol' and 'lowing' using left arrow key"
    Vor.Click
    Silbentrennung.OK

   printlog "Close all dialogue, and recover the file to default"
    Kontext "Active"
    if NOT Active.Exists then
        Kontext "Silbentrennung"
        Silbentrennung.Cancel
        Warnlog "The first message box is NOT pop up"
        goto NOTest
    end if

    Active.Yes

    Kontext "Active"
    if NOT Active.Exists then
        Warnlog "The second message box is NOT pop up"
        goto endsub
    end if

    Active.OK

   printlog "Press 3 times 'space bar' in front of the line"
    Call wTypeKeys "<BACKSPACE>",3
    Call wTypeKeys "<SPACE>",3

    Call wTypeKeys "<End><Left><Shift Mod1 Left>"
    EditCopy

   printlog "Check if get 'fol'"
    if GetClipboardText <> firstPartOfTestWord then
        Warnlog "The hyphenation does NOT work well in first part!"
    end if

NOTest:
   Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tHyphenation_3

warnlog "#i102304# - outcommenting tHyphenation_3 due to bug."
goto endsub

  Dim testFile as String
  Dim testWord as String
  Dim firstPartOfTestWord
  Dim secondPartOfTestWord as String
  Dim testWordInHyphenationDlg1 as String
  Dim testWordInHyphenationDlg2 as String

    testFile                  =  "tHyphenation.odt"
    testWord                  =  "following"
    firstPartOfTestWord       =  "fol"
    secondPartOfTestWord      =  "follow"
    testWordInHyphenationDlg1 =  "fol=low-ing"
    testWordInHyphenationDlg2 =  "fol-low=ing"

    printLog "- Test Hyphenation  using Tools/Hyphenation ,test like follow-ing"
    printlog "Test Hyphenation  using Tools/Hyphenation ,test like follow-ing"

    printlog "open a test file"
    Call hFileOpen ( gTesttoolPath + "writer\optional\input\hyphenation\" + testFile )
    Call sMakeReadOnlyDocumentEditable

    Call wTypeKeys "<End><Shift Mod1 Left>"

    EditCopy

    if GetClipboardText <> testWord then
        Warnlog "The test word is wrong !!"
        goto endsub
    end if

    printlog "Press 3 times 'space bar'"
    Call wTypeKeys "<Home>"
    if gPlatGroup <> "unx" then
        Call wTypeKeys "<SPACE>" ,3
    else
        Call wTypeKeys "<SPACE>" ,6
    end if

    printlog "Tools/Hyphenation"
    ToolsLanguageHyphenate
    Kontext "Silbentrennung"

    if Not Silbentrennung.Exists then
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                Active.Yes

                Kontext "Active"
                if Active.Exists then
                    if Active.GetRT = 304 then Active.Ok
                end if
             end if
        end if
        Warnlog "Unable to bring up Dialog 'Hyphenation'!"
        goto endsub
    end if

   printlog "check if get 'fol=low-ing'"
    if Wort.Gettext <> testWordInHyphenationDlg1 then
        Warnlog "#i40561# The test can NOT be seperated two times , or something wrong!"
        Silbentrennung.Cancel
        goto NOTest
    end if

   printlog "click left arrow key"
    if NOT Vor.IsEnabled then
        Warnlog "the Vor button is Not enabled, or something wrong!"
        Silbentrennung.Cancel
        goto NOTest
    end if
    Vor.Click

   printlog "check if get ''fol-low=ing'"
    if Wort.Gettext <> testWordInHyphenationDlg2 then
        Warnlog "The test  word should be changed after clicking Vor!"
        goto endsub
    end if

   printlog "add '-' between 'follow' and 'ing' pressing right arrow key"
    if NOT Zurueck.IsEnabled then
        Warnlog "the Zuruech button is Not enabled, or something wrong!"
        goto endsub
    end if
    Zurueck.Click

   printlog "check if get 'fol-low=ing'"
    if Wort.Gettext <> testWordInHyphenationDlg1 then
         Warnlog "The test  word should be changed after clicking Zurueck!"
         goto endsub
    end if

    Sleep 1

   printlog "close all dialogue"
    Silbentrennung.OK

    Kontext "Active"
    if NOT Active.Exists then
          Warnlog "The first message box is NOT pop up"
          goto endsub
    end if

    Active.Yes

    Kontext "Active"
    if NOT Active.Exists then
          Warnlog "The second message box is NOT pop up"
          goto endsub
    end if

    Active.OK

   printlog "recover to the default file and press 3 sparc bar in front of the line"
    Call wTypeKeys "<BACKSPACE>",3
    Call wTypeKeys "<SPACE>",3

    Call wTypeKeys "<End><Left><Shift Mod1 Left>"
    EditCopy

   printlog "check if get 'follow' in the end of the line"
    if GetClipboardText = secondPartOfTestWord then
        Call wTypeKeys "<Home>"
        Call wTypeKeys "<SPACE>" , 5

        Call wTypeKeys "<End><Shift Mod1 Left>"
           EditCopy

           if GetClipboardText <> firstPartOfTestWord then
               Warnlog "The hyphenation does NOT work well in first part, but it works fine in second part!"
           end if
    else
        Warnlog "The hyphenation does NOT work well in second part!"
    end if
NOTest:
   Call hCloseDocument

endcase
