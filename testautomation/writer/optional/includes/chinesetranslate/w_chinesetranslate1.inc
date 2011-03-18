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
'* short description : Chinese Translation test
'*
'************************************************************************
'*
' #1 tDefaultSetting_1           'Default settings for different Chinese language
' #1 tDefaultSetting_2           'Default settings
' #1 tDefaultSetting_3           'Cursor's position
' #1 tChineseTranslate_1         'Translation for selected text
' #1 tChineseTranslate_2         'Translation for NO-selected document
' #1 tChineseTranslate_3         'Translation direction : T Chinese to S Chinese
' #1 tChineseTranslate_4         'Translation direction : S Chinese to T Chinese
' #1 tChineseTranslate_5         ''Use Taiwan , Hongkong SAR...'s statue
' #0 fSetAsian
'*
'\***********************************************************************

testcase tDefaultSetting_1

  printlog "Default settings for different Chinese language"

  Call hNewDocument

  printlog " Set default language(asian) for document is 'Simplified Chinese'"
   fSetAsian(fGetLanguageName(86))

  printlog " Tools/language / Chinese translation"
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"

    printlog " 'Traditional Chinese to Simplified Chinese' should be Checked by default"
     if NOT TChineseToSChinese.IsChecked then
         Warnlog "'Traditional Chinese to Simplified Chinese' should be Checked by default !"
     end if
   ChineseTranslation.Cancel

  printlog " Set default language(asian) for document is 'Traditional Chinese'"
   fSetAsian(fGetLanguageName(88))

  printlog " Tools/language / Chinese translation"
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"

    printlog " 'Simplified Chinese to Traditional Chinese' should be Checked by default"
     if NOT SChineseToTChinese.IsChecked then
         QAErrorlog "#i96768# 'Simplified Chinese to Traditional Chinese' should be Checked by default !"
     end if
   ChineseTranslation.Cancel

  Call hCloseDocument
endcase

'-----------------------------------------------------------------

testcase tDefaultSetting_2

  printlog "Default settings "

  Call hNewDocument

   fSetAsian(fGetLanguageName(86))

  printlog " Tools/language / Chinese translation"
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"

'    printlog " Use Taiwan,Hongkong SAR , and Macao SAR character variants"
'    printlog " + should NOT be Checked by default"
'     if UseSARCharacter.IsChecked then
'         Warnlog "'Use Taiwan,Hongkong SAR , and Macao SAR character variants' should NOT be Checked by default !"
'     end if

    printlog " 'Common Terms' should NOT be Checked by default"
     if TranslateCommonTerms.IsChecked then
         Warnlog "'Common Terms' should NOT be Checked by default !"
     end if

    printlog " Click 'Edit terms' button"
     EditTerms.Click
     Kontext "ChineseDictionary"

      printlog " 'Traditional Chinese to Simplified Chinese' should be Checked by default"
       if NOT TChineseToSChinese.IsChecked then
           Warnlog "'Traditional Chinese to Simplified Chinese' should be Checked by default !"
       end if

      printlog " 'Reverse mapping' should NOT be Checked by default"
       if ReverseMapping.IsChecked then
           Warnlog "'Reverse mapping' should NOT be Checked by default !"
       end if

      printlog " 'Property' should be 'other' by default"
       if Property.GetSelIndex <> 1 then
           Warnlog "'Property' should be 'Other' by default !"
       end if
     ChineseDictionary.Cancel

   Kontext "ChineseTranslation"
   ChineseTranslation.Cancel

  Call hCloseDocument
endcase

'-----------------------------------------------------------------

testcase tDefaultSetting_3

  Dim sChinese as  String
  Dim sExpect as String

  sChinese = "红上蓝国"
  sExpect  = "紅"

  printlog "Cursor position"

  Call hNewDocument

   fSetAsian(fGetLanguageName(86))

   printlog " Input some chinese and set cursor in the beginning"
   Call wTypeKeys sChinese
   Call wTypeKeys "<Home>"

  printlog " Tools/language / Chinese translation , check"
  printlog " + 'Simplified Chinese to Traditional Chinese', press OK"
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"
     SChineseToTChinese.Check
   ChineseTranslation.OK

  printlog " Check if the cursor's position is changed -- Should NOT"
   Call wTypeKeys "<Shift Right>"
   try
       EditCopy
       if GetClipboardText <> sExpect then
           Warnlog "Hope to get " & sExpect & " but get " & GetClipboardText
       end if
   catch
       Warnlog "Cursor's position is changed - 117787"
   endcatch

  Call hCloseDocument
endcase

'-----------------------------------------------------------------

testcase tChineseTranslate_1

  Dim sChinese as String
  Dim sExpect as String

  sChinese = "红上蓝国"
  sExpect  = "紅上蓝国"

  printlog "Translation for selected text"

  Call hNewDocument

   fSetAsian(fGetLanguageName(86))

  printlog " Input some chinese and select part of input"
   Call wTypeKeys sChinese
   Call wTypeKeys "<Home>"
   Call wTypeKeys "<Shift Right>" , 2

  printlog " Tools/language / Chinese translation , check"
  printlog " + 'Simplified Chinese to Traditional Chinese', press OK"
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"
     SChineseToTChinese.Check
   ChineseTranslation.OK

  printlog " Check if the only selected text is translated -- Should be"
   Call wTypeKeys "<Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   if GetClipboardText <> sExpect then
       Warnlog "Hope to get " & sExpect & " but get " & GetClipboardText
   end if

  Call hCloseDocument
endcase

'-----------------------------------------------------------------

testcase tChineseTranslate_2

  Dim sChinese as String
  Dim sExpect as String

  sChinese = "红上蓝国"
  sExpect  = "紅上藍國"

  printlog "Translation for NO-selected document"

  Call hNewDocument

   fSetAsian(fGetLanguageName(86))

  printlog " Input some chinese"
   Call wTypeKeys sChinese

  printlog " Tools/language / Chinese translation , check"
  printlog " + 'Simplified Chinese to Traditional Chinese' , press OK"
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"
     SChineseToTChinese.Check
   ChineseTranslation.OK

  printlog " Check if all document is translated -- Should"
   Call wTypeKeys "<Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   if GetClipboardText <> sExpect then
       Warnlog "Hope to get " & sExpect & " but get " & GetClipboardText
   end if

  Call hCloseDocument
endcase

'-----------------------------------------------------------------

testcase tChineseTranslate_3

  Dim sChinese as String
  Dim sExpect as String

  sChinese = "红上藍國"
  sExpect  = "红上蓝国"

  printlog "Translation direction : T Chinese to S Chinese"

  Call hNewDocument

   fSetAsian(fGetLanguageName(88))

  printlog " Input some chinese , include some"
  printlog " + tradition chinese and simplified chinese"
   Call wTypeKeys sChinese

  printlog " Tools/language / Chinese translation , check"
  printlog " + 'Traditional Chinese to Simplified Chinese' , press OK"
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"
     TChineseToSChinese.Check
   ChineseTranslation.OK

  printlog " Check if only traditional chinese is translated -- Should"
   Call wTypeKeys "<Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   if GetClipboardText <> sExpect then
       Warnlog "Hope to get " & sExpect & " but get " & GetClipboardText
   end if

  Call hCloseDocument
endcase

'-----------------------------------------------------------------

testcase tChineseTranslate_4

  Dim sChinese as String
  Dim sExpect as String

  sChinese = "红上藍國"
  sExpect  = "紅上藍國"

  printlog "Translation direction : S Chinese to T Chinese"

  Call hNewDocument

   fSetAsian(fGetLanguageName(86))

  printlog " Input some chinese , include some"
  printlog " + tradition chinese and simplified chinese"
   Call wTypeKeys sChinese

  printlog " Tools/language / Chinese translation , check"
  printlog " + 'Simplified Chinese to Traditional Chinese' , press OK"
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"
     SChineseToTChinese.Check
   ChineseTranslation.OK

  printlog " Check if only simplified chinese is translated -- Should"
   Call wTypeKeys "<Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   if GetClipboardText <> sExpect then
       Warnlog "Hope to get " & sExpect & " but get " & GetClipboardText
   end if

  Call hCloseDocument
endcase

'-----------------------------------------------------------------

testcase tChineseTranslate_5
    if bAsianLan <> True then
        'This testcase might only run with 'Use Taiwan , Hongkong SAR...'s status.
        QaErrorLog "Testing with Asian languages only"
        goto endsub
    end if

  printlog "'Use Taiwan , Hongkong SAR...'s statue"

  Call hNewDocument

   fSetAsian(fGetLanguageName(88))

  printlog " Tools/language / Chinese translation"
  printlog " Check Traditional Chinese to Simplified Chinese"
  printlog " + 'Use Taiwan , Hongkong SAR...' should be disabled"
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"
     TChineseToSChinese.Check
     try
         UseSARCharacter.Check
         Warnlog "'Use Taiwan , Hongkong SAR and ...' should be disabled !"
     catch
     endcatch

  printlog " Check Simplified Chinese to Traditional Chinese"
  printlog " + 'Use Taiwan , Hongkong SAR...' should be enabled"
     SChineseToTChinese.Check
     try
         UseSARCharacter.Check
     catch
         Warnlog "'Use Taiwan , Hongkong SAR and ...' should be enabled !"
     endcatch

  printlog " UnCheck 'Use Taiwan , Hongkong SAR...'"
     UseSARCharacter.UnCheck

  printlog " Check Translate Common Terms"
     TranslateCommonTerms.Check

  printlog " 'Use Taiwan , Hongkong SAR...' should be checked automatically"
     if NOT UseSARCharacter.IsChecked then
         Warnlog "'Use Taiwan , Hongkong SAR and ...' should be checked !"
     end if
   ChineseTranslation.OK

  printlog " Close the dialog and reopen it"
  printlog " 'Use Taiwan , Hongkong SAR...' and 'Translate Common Terms'"
  printlog " + should be checked"
   ToolsLanguageChineseTranslation
   Kontext "ChineseTranslation"
     if NOT UseSARCharacter.IsChecked then
         Warnlog "'Use Taiwan , Hongkong SAR and ...' should be checked ! - 117802 "
     end if
     if NOT TranslateCommonTerms.IsChecked then
         Warnlog "'Translate common terms' should be checked ! - 117802 "
     end if
   ChineseTranslation.Cancel

  Call hCloseDocument
endcase

'-----------------------------------------------------------------

function fSetAsian(sAsian as String)  as String

    'This function is set CJK language in Tools/Options/Language Settings / Languages
    ToolsOptions
    Call hToolsOptions("LANGUAGESETTINGS","LANGUAGES")
    fSetAsian = Asiatisch.GetSelText
    if sAsian = "Null" then
        Asiatisch.Select  1
        goto GoON
    else
        Asiatisch.Select  sAsian
    end if
    
    GoON:
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

end function

