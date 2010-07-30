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
'* short description : Chinese Dictionary test
'*
'************************************************************************
'*
' #1 tChineseTranslate_6         'Chinese dictionary : S Chinese to T Chinese
' #1 tChineseTranslate_7         'Chinese dictionary : T Chinese to S Chinese
' #1 tChineseTranslate_8         'Chinese dictionary : Reverse mapping
' #1 tChineseTranslate_9         'Chinese dictionary : Add
' #1 tChineseTranslate_10        'Chinese dictionary : Modify
' #1 tChineseTranslate_11        'Chinese dictionary : Delete
' #1 tUndoRedo                   'Undo/Redo
'*
'\***********************************************************************

testcase tChineseTranslate_6

  Dim sTerm as String
  Dim sMapping as String

  sTerm    = "蓝国"
  sMapping = "紙間"

  printlog "Chinese dictionary , S Chinese to T Chinese"

  Call hNewDocument

   fSetAsian(fGetLanguageName(86))

  printlog " Input a test term"
   Call wTypeKeys sTerm

  printlog " Tools/language / Chinese translation ,"
  printlog " Check 'Simplified Chinese to Traditional Chinese'"
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"
     SChineseToTChinese.Check

  printlog " Check 'Translate Common Terms'"
     TranslateCommonTerms.Check

  printlog " Press 'Edit Terms' button"
     EditTerms.Click
     Kontext "ChineseDictionary"

    printlog " Click radiobutton 'Simplified Chinese to Tradtional Chinese'"
       SChineseToTChinese.Check

    printlog " Add test word in Term and Mapping"
       Term.SetText    sTerm
       Mapping.SetText sMapping
       Add.Click
     ChineseDictionary.OK
     Sleep 4
   Kontext "ChineseTranslation"
   ChineseTranslation.OK

  printlog " Check if the result is correct"
   Call wTypeKeys "<Home><Shift End>"
   EditCopy
   if GetClipboardText <> sMapping then
       Warnlog "117803 - Hope to get " & sMapping & " but got " & GetClipboardText
   end if

  'Recover to default
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"
     TranslateCommonTerms.UnCheck
     EditTerms.Click
     Kontext "ChineseDictionary"
       SChineseToTChinese.Check
       SToTMappingContent.Select SToTMappingContent.GetItemCount
       Sleep 1
       Delete.Click
     ChineseDictionary.OK
     Sleep 2
   Kontext "ChineseTranslation"
   ChineseTranslation.Cancel

  Call hCloseDocument
endcase

'-----------------------------------------------------------------

testcase tChineseTranslate_7

  Dim sTerm as String
  Dim sMapping as String

  sTerm    = "紙間"
  sMapping = "蓝国"

  printlog "Chinese dictionary , T Chinese to S Chinese"

  Call hNewDocument

   fSetAsian(fGetLanguageName(88))

  printlog " Input a test term"
   Call wTypeKeys sTerm

  printlog " Tools/language / Chinese translation ,"
  printlog " Check Traditional Chinese to Simplified Chinese"
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"
     TChineseToSChinese.Check

  printlog " Check 'Translate Common Terms'"
     TranslateCommonTerms.Check

  printlog " Press 'Edit Terms' button"
     EditTerms.Click
     Kontext "ChineseDictionary"

    printlog " Click radiobutton 'Tradtional Chinese to Simplified Chinese'"
       TChineseToSChinese.Check

    printlog " Add test word in Term and Mapping"
       Term.SetText    sTerm
       Mapping.SetText sMapping
       Add.Click
     ChineseDictionary.OK
     Sleep 4
   Kontext "ChineseTranslation"
   ChineseTranslation.OK

  printlog " Check if the result is correct"
   Call wTypeKeys "<Home><Shift End>"
   EditCopy
   if GetClipboardText <> sMapping then
       Warnlog "117803 - Hope to get " & sMapping & " but get " & GetClipboardText
   end if

  'Recover to default
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"
     TranslateCommonTerms.UnCheck
     EditTerms.Click
     Kontext "ChineseDictionary"
       TChineseToSChinese.Check
       TToSMappingContent.Select TToSMappingContent.GetItemCount
       Sleep 1
       Delete.Click
     ChineseDictionary.OK
     Sleep 2
   Kontext "ChineseTranslation"
   ChineseTranslation.Cancel

  Call hCloseDocument
endcase

'-----------------------------------------------------------------

testcase tChineseTranslate_8

  Dim sTerm as String
  Dim sMapping as String

  sTerm    = "蓝国"
  sMapping = "紙間"

  printlog "Chinese dictionary , Reverse mapping"

  Call hNewDocument

   fSetAsian(fGetLanguageName(88))

  printlog " Input a test term"
   Call wTypeKeys sMapping

  printlog " Tools/language / Chinese translation ,"
  printlog " Check 'Traditional Chinese to Simplified Chinese'"
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"
     TChineseToSChinese.Check

  printlog " Check 'Translate Common Terms'"
     TranslateCommonTerms.Check

  printlog " Press 'Edit Terms' button"
     EditTerms.Click
     Kontext "ChineseDictionary"

    printlog " Click radiobutton 'Simplified Chinese to Tradtional Chinese'"
       SChineseToTChinese.Check

    printlog " Check 'Reverse Mapping'"
       ReverseMapping.Check

    printlog " Add test word in Term and Mapping"
       Term.SetText    sTerm
       Mapping.SetText sMapping
       Add.Click
     ChineseDictionary.OK
     Sleep 4
   Kontext "ChineseTranslation"
   ChineseTranslation.OK

  printlog " Check if the result is correct"
   Call wTypeKeys "<Home><Shift End>"
   EditCopy
   if GetClipboardText <> sTerm then
       Warnlog "117803 - Hope to get " & sTerm & " but get " & GetClipboardText
   end if

  'Recover to default
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"
     TranslateCommonTerms.UnCheck
     EditTerms.Click
     Kontext "ChineseDictionary"
       TChineseToSChinese.Check
       ReverseMapping.Check
       Sleep 1
       Delete.Click
     ChineseDictionary.OK
     Sleep 2
   Kontext "ChineseTranslation"
   ChineseTranslation.Cancel

  Call hCloseDocument
endcase

'-----------------------------------------------------------------

testcase tChineseTranslate_9

  Dim sTerm1 as String
  Dim sMapping1 as String
  Dim sMapping2 as String
  Dim i as  Integer

  sTerm1    = "Term1"
  sMapping1 = "Mapping1" :  sMapping2 = "Mapping2"

  printlog "Chinese dictionary : Add"

  Call hNewDocument

  printlog " Tools/language / Chinese translation ,"
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"

  printlog " Press 'Edit Terms' button"
     EditTerms.Click
     Kontext "ChineseDictionary"
       TChineseToSChinese.Check

    printlog " Check if term is empty , Add button should be inactive"
       try
           Add.Click
           Warnlog "- term is empty , Add button should be inactive !"
       catch
       endcatch

    printlog " Input test word in Term(A) and Mapping(B) , press Add"
       Term.SetText    sTerm1
       Mapping.SetText sMapping1
       Add.Click

    printlog " Input test word in Term(A) and Mapping(C) , press Add"
       Term.SetText    sTerm1
       Mapping.SetText sMapping2

    printlog " Check if term text is unaltered , Add button should be inactive"
       try
           Add.Click
           Warnlog "- term text is unaltered , Add button should be inactive !"
       catch
       endcatch

     ChineseDictionary.Cancel
   Kontext "ChineseTranslation"
   ChineseTranslation.Cancel

  Call hCloseDocument
endcase

'-----------------------------------------------------------------

testcase tChineseTranslate_10

  Dim sTerm1 as String
  Dim sMapping1 as String
  Dim sMapping2 as String
  Dim i as Integer

  sTerm1    = "Term1"
  sMapping1 = "Mapping1" :  sMapping2 = "Mapping2"

  printlog "Chinese dictionary : Modify"

  Call hNewDocument

  printlog " Tools/language / Chinese translation ,"
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"

  printlog " Press 'Edit Terms' button"
     EditTerms.Click
     Kontext "ChineseDictionary"
       TChineseToSChinese.Check

    printlog " Input test word in Term(A) and Mapping(B) , press Add"
       Term.SetText    sTerm1
       Mapping.SetText sMapping1
       Add.Click

    printlog " Input test word in Mapping(C) and select another property ,"
    printlog " + then press Modify"
       Mapping.SetText sMapping2
       Property.Select 3
       Sleep 1
       Modify.Click

    printlog " Check if modification is correct"
       TToSMappingContent.Select TToSMappingContent.GetItemCount
       if Term.GetText <> sTerm1 then
           Warnlog "Should get " & sTerm1 & " but get " & Term.GetText
       end if
       if Mapping.GetText <> sMapping2 then
           Warnlog "Should get " & sMapping2 & " but get " & Mapping.GetText
       end if
       if Property.GetSelIndex <> 3 then
           Warnlog "Property is NOT correct!"
       end if
     ChineseDictionary.Cancel
   Kontext "ChineseTranslation"
   ChineseTranslation.Cancel

  Call hCloseDocument
endcase

'-----------------------------------------------------------------

testcase tChineseTranslate_11

  Dim sTerm1 as String
  Dim sMapping1 as String
  Dim sTerm2 as String
  Dim i as Integer

  sTerm1    = "Term1"    : sTerm2 = "Term2"
  sMapping1 = "Mapping1"

  printlog "Chinese dictionary : Delete"

  Call hNewDocument

  printlog " Tools/language / Chinese translation ,"
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"

  printlog " Press 'Edit Terms' button"
     EditTerms.Click
     Kontext "ChineseDictionary"
       TChineseToSChinese.Check

    printlog " Input test word in Term(A) and Mapping(B) , press Add"
       Term.SetText    sTerm1
       Mapping.SetText sMapping1
       Add.Click

    printlog " Input test word in Term(C), check if Delete button is active"
       Term.SetText sTerm2
       try
           Delete.Click
           Warnlog "Delete button should be inactive !"
       catch
       endcatch
     ChineseDictionary.Cancel

   Kontext "ChineseTranslation"
   ChineseTranslation.Cancel

  Call hCloseDocument
endcase

'-----------------------------------------------------------------

testcase tUndoRedo

  Dim sChinese as String
  Dim sExpect as String

  sChinese = "红上蓝国"
  sExpect  = "紅上藍國"

  printlog "Undo/Redo"

  Call hNewDocument

   fSetAsian(fGetLanguageName(86))

   printlog " Input some simplified chinese"
   Call wTypeKeys sChinese

  printlog " Tools/language / Chinese translation , check"
  printlog " + 'Simplified Chinese to Traditional Chinese' , press OK"
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"
     SChineseToTChinese.Check
   ChineseTranslation.OK

  printlog " Edit/Undo"
   EditUndo
   Call wTypeKeys "<Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   if GetClipboardText <> sChinese then
       Warnlog "#117810# Hope to get " & sChinese & " but get " & GetClipboardText
   end if

  printlog " Edit/Redo"
   EditRedo
   Call wTypeKeys "<Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   if GetClipboardText <> sExpect then
       Warnlog "#117810# Hope to get " & sExpect & " but get " & GetClipboardText
   end if

  Call hCloseDocument
endcase

'-----------------------------------------------------------------
