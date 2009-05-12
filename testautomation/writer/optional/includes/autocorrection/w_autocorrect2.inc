'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_autocorrect2.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: vg $ $Date: 2008-08-18 12:23:31 $
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
'* short description : Test AutoCorrect/AutoFormat/Word Completion
'*
'************************************************************************
'*
' #1 tToolsAutocorrectWordCompletion_1
' #1 tToolsAutocorrectWordCompletion_2
' #1 tToolsAutocorrectWordCompletion_3
' #1 tToolsAutocorrectWordCompletion_4
' #1 tToolsAutocorrectWordCompletion_5
'*
'\***********************************************************************

sub w_autocorrect_autoformat_2

    printLog Chr(13) + "- Test the AutoCorrect/AutoFormat Word Completion features"    
    Call tToolsAutocorrectWordCompletion_1     'Check the default settings
    Call tToolsAutocorrectWordCompletion_2     'Check entries
    Call tToolsAutocorrectWordCompletion_3     'Check list of suggestions can be "scrolled"
    Call tToolsAutocorrectWordCompletion_4     'Test Max entries
    Call Ttoolsautocorrectwordcompletion_5     'Check 'When closing a document, save the list for later use in other documents'
    
end sub

'-------------------------------------------------------------------------

testcase tToolsAutocorrectWordCompletion_1

  Dim iAcceptWith as String
  Dim iEntriesNumber as Integer
  Dim sMinWordLength as String
  Dim sMaxEntries as String
  Dim sMinMaxEntries as String

  iAcceptWith     = 2
  iEntriesNumber  = 0
  sMinWordLength  = "10"
  sMaxEntries     = "500"
  sMinMaxEntries  = "50"

 'Delete all entries in the list
  fDeleteAllEntries()

  printLog "Check the default settings"
  '/// Check the default settings
  '/// - Check box "Enable word completion" (default: checked)
  '/// - Check box "Append Space" (default: unchecked)
  '/// - Check box "Show as tip" (default: unchecked)
  '/// - Check box "Collect Words" (default: checked)
  '/// - Check box "When closing a document, save the list for later use in other documents" (default: checked)
  '/// - List box "Accept with" (Default = "Return")
  '/// - Spin box "minimal number of characters" default = 10
  '/// - Spin box "Max. entries" (default =500, minimum = 50)
  '/// - Box: "List of words collected" -- should be empty after Office restart.

   Call hNewDocument

   ToolsAutocorrect
   Active.Setpage TabWortergaenzung
   Kontext "TabWortergaenzung"

      if WoerterErgaenzen.IsChecked   <> TRUE           then Warnlog "Complete words should be checked ."
      if AppendSpace.IsChecked        =  TRUE           then Warnlog "Append space should be UNchecked ."
      if AlsTipphilfe.IsChecked       =  TRUE           then Warnlog "Show as tip should be UNchecked ."
      if VorschlaegeSammeln.IsChecked <> TRUE           then Warnlog "Collect suggestions should be checked ."
      if KeepList.IsChecked           <> TRUE           then Warnlog "When closing a document, save the list for later use in other documents should be checked ."
      if AnnehmenMit.GetSelIndex      <> iAcceptWith    then Warnlog "The Accept with should be 'Enter' ."
      if MinWortlaenge.GetText        <> sMinWordLength then Warnlog "#i40540# Min word length should be " + sMinWordLength
      if MaxEintraege.GetText         <> sMaxEntries    then Warnlog "Max entries should be " + sMaxEntries
      if GesammelteWorte.GetItemCount <> iEntriesNumber then Warnlog "Entries number  should be " + iEntriesNumber

      MaxEintraege.SetText "49"

     TabWortergaenzung.OK

     ToolsAutocorrect
     Active.Setpage TabWortergaenzung
     Kontext "TabWortergaenzung"
      if MaxEintraege.GetText <> sMinMaxEntries then Warnlog "Max entries should be " + sMinMaxEntries
      MaxEintraege.SetText sMaxEntries
     TabWortergaenzung.OK

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tToolsAutocorrectWordCompletion_2

  Dim iDefaultEntriesNumber as Integer
  Dim iEntriesNumber as Integer
  Dim i as Integer
  Dim sMinWordLength as String

  iDefaultEntriesNumber  = 0
  iEntriesNumber         = 3
  sMinWordLength         = "5"

 'Delete all entries in the list
  fDeleteAllEntries()

  printLog "Check the default settings"
  '/// Check entries
  '/// delete all entries first
  '/// input some test words
  '/// check if some words should are remembered in the entries
  '/// delete all entries
  '/// close and reopen dialog, check entries again

   Call hNewDocument

   ToolsAutocorrect
   Active.Setpage TabWortergaenzung
   Kontext "TabWortergaenzung"
   if GesammelteWorte.GetItemCount <> iDefaultEntriesNumber then
       Warnlog "Entries number should be " + iDefaultEntriesNumber
       Warnlog "No test for this case"
       TabWortergaenzung.Cancel
       Goto NoTest
   end if
   MinWortlaenge.SetText sMinWordLength
   TabWortergaenzung.OK

    Call wTypeKeys "Hello World ! This is a StarOffice Test .<Return>"
    Sleep 3

    ToolsAutocorrect
    Active.Setpage TabWortergaenzung
    Kontext "TabWortergaenzung"
       if GesammelteWorte.GetItemCount <> iEntriesNumber then Warnlog "Entries number should be " + iEntriesNumber
       for i = 1 to GesammelteWorte.GetItemCount
           GesammelteWorte.Select 1
           if EintragLoeschen.IsEnabled then
               EintragLoeschen.Click
           else
               i = GesammelteWorte.GetItemCount + 1
           end if
       next i
     TabWortergaenzung.OK
     Sleep 1

    ToolsAutocorrect
    Active.Setpage TabWortergaenzung
    Kontext "TabWortergaenzung"
    if GesammelteWorte.GetItemCount <> iDefaultEntriesNumber then Warnlog "Entries number should be " + iDefaultEntriesNumber
    TabWortergaenzung.Cancel

NoTest:
   Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tToolsAutocorrectWordCompletion_3

  Dim aTestWord() as String
  Dim sPartOfTestWord as String
  Dim i as Integer
  Dim j as Integer

  aTestWord = Array( _
                   "accessibility" , _
                   "accomplish"    , _
                   "account"       , _
                   )
  j = UBound(aTestWord)

  sPartOfTestWord = "acc"

  printLog "Check list of suggestions can be 'scrolled'"
  '/// Check list of suggestions can be 'scrolled'

  '/// input some test words with first 3 letters are same
  '/// forward with <CTRL-Tab>
  '/// backwards with <SHIFT-CTRL-Tab>
  '/// endless loop test

 'Delete all entries in the list
  fDeleteAllEntries()

   Call hNewDocument

   'forward with <CTRL-Tab>
    For i = 0 to j
        Call wTypeKeys aTestWord(i)
        Call wTypeKeys "<Return>"
    next i
    Sleep 2

    For i = 0 to j
        Call wTypeKeys sPartOfTestWord

        if i > 0 then Call wTypeKeys "<MOD1 TAB>", i
        Sleep 2

        Call wTypeKeys "<Return>"
        Sleep 2
        Call wTypeKeys "<Shift Home>"
        Sleep 2
        EditCopy
        Sleep 2
        if GetClipboardText <> aTestWord(i) then Warnlog "#102535# Should be " +aTestWord(i) +" but get " +GetClipboardText
        Call wTypeKeys "<Delete>"
    next i

   'endless loop test
    Call wTypeKeys sPartOfTestWord
    Call wTypeKeys "<MOD1 TAB>", j+1
    Sleep 2
    Call wTypeKeys "<Return>"
    Sleep 2
    Call wTypeKeys "<Shift Home>"
    Sleep 2
    EditCopy
    Sleep 2
    if GetClipboardText <> aTestWord(0) then Warnlog "#102535# Should be " +aTestWord(0) +" but get " +GetClipboardText
    Call wTypeKeys "<Delete>"

   'backwards with <SHIFT-CTRL-Tab>
    For i = j to 0 step -1
        Call wTypeKeys sPartOfTestWord

        Call wTypeKeys "<MOD1 TAB>", j

        if i > 0 then Call wTypeKeys "<Shift MOD1 TAB>", i

        Sleep 2
        Call wTypeKeys "<Return>"
        Sleep 2
        Call wTypeKeys "<Shift Home>"
        Sleep 2
        EditCopy
        Sleep 2
        if GetClipboardText <> aTestWord(j-i) then Warnlog "#102535# Should be " +aTestWord(j-i) +" but get " +GetClipboardText
        Call wTypeKeys "<Delete>"
    next i

   'endless loop test
    Call wTypeKeys sPartOfTestWord
    Call wTypeKeys "<Shift MOD1 TAB>"
    Sleep 2
    Call wTypeKeys "<Return>"
    Sleep 2
    Call wTypeKeys "<Shift Home>"
    Sleep 2
    EditCopy
    Sleep 2
    if GetClipboardText <> aTestWord(j) then Warnlog "#102535# Should be " +aTestWord(j) +" but get " +GetClipboardText

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tToolsAutocorrectWordCompletion_4

  Dim sDefaultMaxEntries as String
  Dim sTestMaxEntries as String
  Dim sTestFile as String
  Dim iTestMaxEntries as Integer

  sDefaultMaxEntries  =  "500"
  sTestMaxEntries     =  "50"
  sTestFile           =  "tToolsAutocorrectWordCompletion_4.sxw"
  iTestMaxEntries     =  50

  printLog "Test Max entries"
  '/// Test Max entries

  '/// Delete all entries in the list
  '/// set Max entries to 50
  '/// Open the test file ( The entries number is > 50 when Max entries is 500)
  '/// check the number of entries.

 'Delete all entries in the list
  fDeleteAllEntries()

   Call hNewDocument

    ToolsAutocorrect
    Active.Setpage TabWortergaenzung
    Kontext "TabWortergaenzung"
    MaxEintraege.SetText sTestMaxEntries
    TabWortergaenzung.OK

    Call hFileOpen ( gTesttoolPath + "writer\optional\input\advanced\" + sTestFile )
    Sleep 4

    ToolsAutocorrect
    Active.Setpage TabWortergaenzung
    Kontext "TabWortergaenzung"
    if GesammelteWorte.GetItemCount > iTestMaxEntries then Warnlog "Entries number should be less than " + iTestMaxEntries

     'Recover to default
      MaxEintraege.SetText sDefaultMaxEntries
     TabWortergaenzung.OK

   Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tToolsAutocorrectWordCompletion_5

  Dim sTestWord1 as String
  Dim sTestWord2 as String
  Dim sTestWord3 as String
  Dim iEntriesNumber_Checked as Integer
  Dim iEntriesNumber_UnChecked as Integer

  sTestWord1               = "kilometer "
  sTestWord2               = "entertainment "
  sTestWord3               = "education "
  iEntriesNumber_Checked   = 3
  iEntriesNumber_UnChecked = 1

  printLog "Check 'When closing a document, save the list for later use in other documents'"
  '/// Check 'When closing a document, save the list for later use in other documents'
  '/// delete all entries first.
  '/// 1) check when the checkbox is checked
  '///    a) Open a file, type "kilometer"
  '///    b) Open 2nd file, type "entertainment" and "education"
  '///    c) Close 2nd file, the number of entries in first file should be 3
  '/// 2) check when the checkbox is Unchecked
  '///    delete all entries , and Uncheck the testing checkbox
  '///    a) Open a file, type "kilometer"
  '///    b) Open 2nd file, type "entertainment" and "education"
  '///    c) Close 2nd file, the number of entries in first file should be 1

  'Test when checkbox is checkec -- default

  'Open 1st file
   Call hNewDocument

    'Delete all entries in the list
     fDeleteAllEntries()

     Call wTypeKeys sTestWord1

    'Open 2nd file
     Call hNewDocument

       Call wTypeKeys sTestWord2
       Call wTypeKeys sTestWord3

    ToolsAutocorrect
    Active.Setpage TabWortergaenzung
    Kontext "TabWortergaenzung"
    if GesammelteWorte.GetItemCount <> iEntriesNumber_Checked then
        Warnlog "Checked:Entries number should be equal " + iEntriesNumber_Checked + " in 2nd file ,but get " +GesammelteWorte.GetItemCount
    end if
    TabWortergaenzung.Cancel

    'Close 2nd file
     Call hCloseDocument

    ToolsAutocorrect
    Active.Setpage TabWortergaenzung
    Kontext "TabWortergaenzung"
      if GesammelteWorte.GetItemCount <> iEntriesNumber_Checked then
          Warnlog "Checked:Entries number should be equal " + iEntriesNumber_Checked + " in 1st file ,but get " +GesammelteWorte.GetItemCount
      end if
     TabWortergaenzung.Cancel

   Call hCloseDocument

  'Test when checkbox is Unchecked
  'Open 1st file
   Call hNewDocument

    'Delete all entries in the list
     fDeleteAllEntries()

    ToolsAutocorrect
    Active.Setpage TabWortergaenzung
    Kontext "TabWortergaenzung"

       KeepList.Uncheck
     TabWortergaenzung.OK

     Call wTypeKeys sTestWord1

    'Open 2nd file
     Call hNewDocument

       Call wTypeKeys sTestWord2
       Call wTypeKeys sTestWord3

    ToolsAutocorrect
    Active.Setpage TabWortergaenzung
    Kontext "TabWortergaenzung"
    if GesammelteWorte.GetItemCount <> iEntriesNumber_Checked then
        Warnlog "UnChecked:Entries number should be equal " + iEntriesNumber_Checked + " in 2nd file ,but get " +GesammelteWorte.GetItemCount
    end if
    TabWortergaenzung.Cancel

    'Close 2nd file
     Call hCloseDocument

    ToolsAutocorrect
    Active.Setpage TabWortergaenzung
    Kontext "TabWortergaenzung"
    if GesammelteWorte.GetItemCount <> iEntriesNumber_UnChecked then
        Warnlog "UnChecked:Entries number should be equal " + iEntriesNumber_UnChecked + " in 2nd file ,but get " +GesammelteWorte.GetItemCount
    end if
    TabWortergaenzung.Cancel

    'Recover to the default
    ToolsAutocorrect
    Active.Setpage TabWortergaenzung
    Kontext "TabWortergaenzung"
    KeepList.Check
    TabWortergaenzung.OK

  'Close 1st file
   Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tToolsAutocorrectWordCompletion_x

  Dim sTestWord as String
  Dim sPartOfTestWord as String

  sTestWord       = "StarOffice"
  sPartOfTestWord = "Sta"

  printLog "Check Show as Tip"
  '/// Check Show as Tip

  '/// When uncheck ( default) , the left word will be highlight
  '/// When check  , the left word will NOT be appeared,  <--- can test
  '/// and a help tip will appear       <--- no way to test yet

  'Show as Tip -- uncheck (default)
   Call hNewDocument

     Call wTypeKeys sTestWord
     Call wTypeKeys "<Return>"
     Sleep 1
     Call wTypeKeys sPartOfTestWord
     Sleep 4
     Call wTypeKeys "<Return>"
     Call wTypeKeys "<Shift Home>"

     Sleep 1
     Call wTypeKeys "StaS"
     Sleep 4
     Call wTypeKeys "<Return>"

   Call hCloseDocument

endcase
