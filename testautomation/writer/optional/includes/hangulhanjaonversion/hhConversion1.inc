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
'* short description : Test of Hangul/Hanja Conversion - 1
'*
'************************************************************************
'*
' #1 tHHNoSelction_1         'No selection
' #1 tHHNoSelction_2         'Multi languages -1 (Korean before Chinese)
' #1 tHHNoSelction_3         'Multi languages -2 (Chinese before Korean)
' #1 tHHNoSelction_4         'Multi languages -3 (no Chinese and Korean)
' #1 tHHNoSelction_5         'Cursor's position is in the end of the document
' #1 tHHNoSelction_6         'Notconvertible Hangul character
' #1 tHHNoSelction_7         'Notconvertible Hanja character
' #1 tHHSingleSelction_1     'Single selection
' #1 tHHMultiSelction_1      'Multi selection
' #1 tHHTextBox_1            'Hangul character in text box
' #1 tHHDrawBox_1            'Hangul character in draw box
'*
'\***********************************************************************

testcase tHHNoSelction_1

    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub

    Dim  sTestFile  as String
    Dim  sResult    as String

    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\hangul.sxw")
    sResult = "漢字"

    GetClipboardText = ""

    printlog "- No Selection "
    '/// <b> If no selection is made in the document , </b>
    '/// + <b> the Hangul/Hanja conversion will start </b>
    '/// + <b> exactly at the cursor's location.  </b>

    Call hNewDocument

    '/// Open a test file , which includes some Korean characters
    '/// + and some Chinese characters
    hFileOpenLocally(sTestFile)

    '/// Set focus before the 3rd character
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Down>"

    '/// Tools / HangulHanjaConversion
    ToolsLanguageHangulHanjaConversion : WaitSlot
    Kontext "HangulHanjaConversion"

    '/// Press Peplace button
    if ReplaceBtn.IsEnabled then
        ReplaceBtn.Click
    else
        Warnlog "Replace button is disabled !"
        HangulHanjaConversion.Close
        Call hCloseDocument
        goto endsub
    end if    
    Sleep 1

    HangulHanjaConversion.Close

    '/// Check if conversion really happens at cursor's location
    Call wTypeKeys "<Home>"
    Call wTypeKeys "<Shift Right>",2
    try
        EditCopy
    catch
        Warnlog "#116346#Hangul/Hanja selection to begin of line impossible after closing hhc dialog"
        Call wTypeKeys "<MOD1 Home>"
        Call wTypeKeys "<Right>" , 4
        Call wTypeKeys "<Shift Home>"
        EditCopy
        Sleep 1
    endcatch

    if GetClipboardText <> sResult then
        Warnlog "Conversion result not: " & sResult & " but: " & GetClipboardText
    end if

    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHNoSelction_2
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


    Dim  sTestFile  as String
    Dim  sResult    as String
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\multiLanguage_K.sxw")    
    sResult = "English sèction säction ピンイン Numérico 書翰中"

    GetClipboardText = ""
    
    printlog "- Multi languages -1 (Korean before Chinese) "
    '/// <b> In multiple-script/language selections/words, </b>
    '/// + <b> only the Korean and Chinese part will be selected for conversion.</b>
    
    Call hNewDocument
    
    '/// Open a test file , which includes some English ,
    '/// + Germany , French , Korean and Chinese characters
    '/// + (Korean is before Chinese)
    Call hFileOpenLocally(sTestFile)
    
    '/// Set focus to the beginning
    Call wTypeKeys "<MOD1 Home>"
    
    '/// Tools / HangulHanjaConversion
    ToolsLanguageHangulHanjaConversion : WaitSlot
    Kontext "HangulHanjaConversion"

    '/// Press Peplace button
    if ReplaceBtn.IsEnabled then
        ReplaceBtn.Click
    else
        Ignore.Click
        Sleep 1
        ReplaceBtn.Click
    end if

    if HangulHanjaConversion.Exists then HangulHanjaConversion.Close

    '/// Check if Korean is converted
    Call wTypeKeys "<Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    Sleep 1
    if GetClipboardText <> sResult then
        Warnlog "Conversion result not: " & sResult & " but: " & GetClipboardText
    end if

    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHNoSelction_3
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


    Dim  sTestFile  as String
    Dim  sResult    as String
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\multiLanguage_C.sxw")
    sResult = "English sèction säction ピンイン Numérico 중서한"

    GetClipboardText = ""
    
    printlog "- Multi languages -2 (Chinese before Korean) "
    '/// <b> In multiple-script/language selections/words, </b>
    '/// + <b> only the Korean and Chinese part will be selected for conversion.</b>
    
    Call hNewDocument
    
    '/// Open a test file , which includes some English ,
    '/// + Germany , French , Korean and Chinese characters
    '/// + (Chinese is before Korean)
    Call hFileOpenLocally( sTestFile )
    ()
    
    '/// Set focus to the beginning
    Call wTypeKeys "<MOD1 Home>"
    
    '/// Tools / HangulHanjaConversion
    ToolsLanguageHangulHanjaConversion : WaitSlot
    Kontext "HangulHanjaConversion"
    
    '/// Press Peplace button
    if ReplaceBtn.IsEnabled then
        ReplaceBtn.Click
    else
        Sleep 1
        Ignore.Click
        Sleep 1
        ReplaceBtn.Click
        Sleep 1
        HangulHanjaConversion.Close
    end if
    
    '/// Check if Chinese is converted
    Call wTypeKeys "<Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    Sleep 1
    if GetClipboardText <> sResult then
       Warnlog "Conversion result not: " & sResult & " but: " & GetClipboardText
    end if
    
    Call hCloseDocument
    
endcase

'-----------------------------------------------------------------

testcase tHHNoSelction_4
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\noKoreanAndChinese.sxw")

  printlog "- Multi languages -3 (no Chinese and Korean) "
  '/// <b> In multiple-script/language selections/words, </b>
  '/// + <b> only the Korean and Chinese part will be selected for conversion.</b>

  Call hNewDocument

  '/// Open a test file , which includes some English ,
  '/// + Germany , French characters .
  '/// + There are no Chinese and Korean
   Call hFileOpenLocally( sTestFile )
   ()

  '/// Set focus to the beginning
   Call wTypeKeys "<MOD1 Home>"

  '/// Tools / HangulHanjaConversion ,
  '/// + nothing should happen
   ToolsLanguageHangulHanjaConversion : WaitSlot
   Kontext "HangulHanjaConversion"

   if HangulHanjaConversion.Exists then
       HangulHanjaConversion.Close
       QAErrorlog "#i39017#Conversion dialog should NOT appear !"
   end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHNoSelction_5
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String
  Dim  sResult    as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\hangul.sxw")
  sResult = "書翰"

  printlog "- Cursor's position is in the end of the document "
  '/// <b> If the cursor resides at the end of a paragraph </b>
  '/// + <b> the first available word/character in the file </b>
  '/// + <b> will be chosen. </b>

  Call hNewDocument

  '/// Open a test file , which includes some Chinese and Korean
   Call hFileOpenLocally( sTestFile )
   ()

  '/// Set focus to the end of the file
   Call wTypeKeys "<MOD1 End>"

  '/// Tools / HangulHanjaConversion ,
   ToolsLanguageHangulHanjaConversion : WaitSlot
   Kontext "HangulHanjaConversion"
     Sleep 1
     if Word.GetText <> sResult then
         Warnlog "Hope to get "  & sResult & " but get  " & Word.GetText
     end if
   HangulHanjaConversion.Close

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHNoSelction_6
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


    Dim  sTestFile  as String
    Dim  sResult    as String

    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\notConvertHangulHanja.sxw")
    sResult   = "入力를入力를入力"

    printlog "- notconvertible Hangul character"
    '/// <b> When the Hangul conversion is invoked on </b>
    '/// + <b> a notconvertible Hangul character , the </b>
    '/// + <b> dialogue will start with with an empty suggestion </b>
    '/// + <b> and the buttons "Replace" and "Always Replace" will be disabled </b>

    Call hNewDocument

    '/// Open a test file , which includes some Chinese and Korean,
    '/// + the 3rd character in 1st line isn't convertible
    Call hFileOpenLocally( sTestFile )
    ()

    '/// Set focus to the 3rd character (korean)
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Right>" , 2

    '/// Tools / HangulHanjaConversion , Replace and Always Replace
    '/// + should be disabled , "suggestion" should be empty .
    '/// + Press Ignore button will jump to the next convertible Hangul/Hanja character
    ToolsLanguageHangulHanjaConversion : WaitSlot
    Kontext "HangulHanjaConversion"
    if Suggestions.GetItemCount <> 0 then
        Warnlog "Suggestion should be empty !"
    end if
    try
        ReplaceBtn.Click
        Warnlog "Replace button should NOT work !"
        AlwaysReplace.Click
        Warnlog "Always Replace button should NOT work !"
    catch
        Ignore.Click
    endcatch
    ReplaceBtn.Click
    Sleep 1
    ReplaceBtn.Click
    Sleep 1    
    if HangulHanjaConversion.Exists then HangulHanjaConversion.Close

    '/// Check if the result is correct
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    Sleep 1
    if GetClipboardText <> sResult then
        Warnlog "Conversion doesn't work well !"
    end if

    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHNoSelction_7
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String
  Dim  sResult    as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\notConvertHangulHanja.sxw")
  sResult   = "중문奖중문奖중문"

  printlog "- notconvertible Hangul character"
  '/// <b> When the Hanja conversion is invoked on </b>
  '/// + <b> a notconvertible Hanja character the </b>
  '/// + <b> dialogue will start with with an empty suggestion </b>
  '/// + <b> and the buttons "Replace" and "Always Replace" will be disabled </b>

  Call hNewDocument

  '/// Open a test file , which includes some Chinese and Korean,
  '/// + the 3rd character in 2nd line isn't convertible
   Call hFileOpenLocally( sTestFile )
   ()

  '/// Set focus to the 3rd chinese character
   Call wTypeKeys "<MOD1 Home><Down><Home>"
   Call wTypeKeys "<Right>" , 2

  '/// Tools / HangulHanjaConversion ,
  '/// + Press Peplace button
  '/// Tools / HangulHanjaConversion , Replace and Always Replace
  '/// + should be disabled , "suggestion" should be empty .
  '/// + Press Ignore button will jump to the next convertible Hangul/Hanja character
   ToolsLanguageHangulHanjaConversion : WaitSlot
   Kontext "HangulHanjaConversion"
     if Suggestions.GetItemCount <> 0 then
         Warnlog "Suggestion should be empty !"
     end if
     try
         ReplaceBtn.Click
         Warnlog "Replace button should NOT work !"
         AlwaysReplace.Click
         Warnlog "Always Replace button should NOT work !"
     catch
         Ignore.Click
     endcatch
     ReplaceBtn.Click
     Sleep 1
     ReplaceBtn.Click
     Sleep 1
   HangulHanjaConversion.Close

  '/// Check if the result is correct
   Call wTypeKeys "<Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   Sleep 1
   if GetClipboardText <> sResult then
       Warnlog "Conversion doesn't work well !"
   end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHSingleSelction_1
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub

    
    Dim  sTestFile  as String
    Dim  sResult    as String
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\notConvertHangulHanja.sxw")
	sResult = "入力를入力를입력"

    
    printlog "- Single selection"
    '/// <b> Single selection </b>
    '/// <b> Depending on the replacement setting </b>
    '/// + <b>(by word or by character) progressing </b>
    '/// + <b>(step by step) suggestions will be made </b>
    '/// + <b> until the end of selection. </b>
    
    Call hNewDocument
    
    '/// Open a test file , which includes some Chinese and Korean
    Call hFileOpenLocally( sTestFile )
    ()
    
    '/// Select from 4th to 7th character
    Call wTypeKeys "<MOD1 Home>"
    Call wTypeKeys "<Right>" , 3
    Call wTypeKeys "<Shift Right>" , 4
    
    '/// Tools / HangulHanjaConversion ,
    '/// + Press Peplace button
    ToolsLanguageHangulHanjaConversion : WaitSlot
    Kontext "HangulHanjaConversion"
    if ReplaceBtn.IsEnabled then 
        ReplaceBtn.Click
        Sleep 1
        try
            ReplaceBtn.Click
            Sleep 1
        catch
        endcatch

       '/// Check if the result is correct
       Call wTypeKeys "<MOD1 Home>"
       Call wTypeKeys "<Shift End>"
       EditCopy
       Sleep 1
       if GetClipboardText <> sResult then
            Warnlog "Conversion result not: " & sResult & " but: " & GetClipboardText           
       end if
    else
        Warnlog "Replace-Button is disabled!"
    end if


    Call hCloseDocument

endcase

'----------------------------------------------------------------

testcase tHHMultiSelction_1
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String
  Dim  sResult    as String
  Dim  sMultiWord as String

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\multiSelection.sxw")
  sResult    = "를入力를에서한자와기호를入力"
  sMultiWord =  "입력"

  printlog "- Multi selection"
  '/// <b> Multi selection </b>
  '/// <b> conversion will jump after finishing </b>
  '/// + <b> selection to next selection until end. </b>

  Call hNewDocument

  '/// Open a test file , which includes some Korean chacters
   Call hFileOpenLocally( sTestFile )
   ()

  '/// Do a multi Selectiong to the charcters -- 입력
   Call fFindWord(sMultiWord)

  '/// Tools / HangulHanjaConversion ,
  '/// + Press Peplace button
   ToolsLanguageHangulHanjaConversion : WaitSlot
   Kontext "HangulHanjaConversion"
     ReplaceBtn.Click
     Sleep 1
     ReplaceBtn.Click
     Sleep 1

  '/// Check if the result is correct
   Call wTypeKeys "<MOD1 Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   Sleep 1
   if GetClipboardText <> sResult then
       Warnlog "Conversion doesn't work well !"
   end if

  Call hCloseDocument

endcase

'----------------------------------------------------------------

testcase tHHTextBox_1
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\textBox.sxw")

  printlog "- Hangul character in text box"
  '/// <b> Hangul character in text box </b>

  Call hNewDocument

  '/// Open a test file , which includes a text box and
  '/// + some Korean chacters in the text box
   Call hFileOpenLocally( sTestFile )
   ()

  '/// Tools / HangulHanjaConversion , the below 4 checkboxes ralated
  '/// + ruby should be disabled .
  '/// -> 'Hanja As Ruby Above Hangul'
  '/// -> 'Hanja As Ruby Below Hangul'
  '/// -> 'Hangul As Ruby Above Hanja'
  '/// -> 'Hangul As Ruby Below Hanja'
   try
       ToolsLanguageHangulHanjaConversion : WaitSlot
       Kontext "HangulHanjaConversion"
       if NOT HangulHanjaConversion.Exists then
           Warnlog "Hangul/Hanja Conversion dialogue isn't poped up !"
           Call hCloseDocument
           goto endsub
       end if
   catch
       Warnlog "- Format / HangulHanjaConversion is disabled or hidden!"
       Call hCloseDocument
       goto endsub
   endcatch

   try
       HanjaAsRubyAbove.Check
       Warnlog "'Hanja As Ruby Above Hangul' should be disabled!"
   catch
   endcatch

   try
       HanjaAsRubyBelow.Check
       Warnlog "'Hanja As Ruby Below Hangul' should be disabled!"
   catch
   endcatch

   try
       HangulAsRubyAbove.Check
       Warnlog "'Hangul As Ruby Above Hanja' should be disabled!"
   catch
   endcatch

   try
       HangulAsRubyBelow.Check
       Warnlog "'Hangul As Ruby Below Hanja' should be disabled!"
   catch
   endcatch

   HangulHanjaConversion.Close
   Sleep 1

  Call hCloseDocument

endcase

'----------------------------------------------------------------

testcase tHHDrawBox_1
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\drawBox.sxw")

  printlog "- Hangul character in draw box"
  '/// <b> Hangul character in draw box </b>

  Call hNewDocument

  '/// Open a test file , which includes a draw box and
  '/// + some Korean chacters in the text box
   Call hFileOpenLocally( sTestFile )
   ()
  '/// Tools / HangulHanjaConversion , the below 4 checkboxes ralated
  '/// + ruby should be disabled .
  '/// -> 'Hanja As Ruby Above Hangul'
  '/// -> 'Hanja As Ruby Below Hangul'
  '/// -> 'Hangul As Ruby Above Hanja'
  '/// -> 'Hangul As Ruby Below Hanja'
   try
       ToolsLanguageHangulHanjaConversion : WaitSlot
       Kontext "HangulHanjaConversion"
       if NOT HangulHanjaConversion.Exists then
           Warnlog "Hangul/Hanja Conversion dialogue isn't poped up !"
           Call hCloseDocument
           goto endsub
       end if
   catch
       Warnlog "- Format / HangulHanjaConversion is disabled or hidden!"
       Call hCloseDocument
       goto endsub
   endcatch

   try
       HanjaAsRubyAbove.Check
       Warnlog "'Hanja As Ruby Above Hangul' should be disabled!"
   catch
   endcatch

   try
       HanjaAsRubyBelow.Check
       Warnlog "'Hanja As Ruby Below Hangul' should be disabled!"
   catch
   endcatch

   try
       HangulAsRubyAbove.Check
       Warnlog "'Hangul As Ruby Above Hanja' should be disabled!"
   catch
   endcatch

   try
       HangulAsRubyBelow.Check
       Warnlog "'Hangul As Ruby Below Hanja' should be disabled!"
   catch
   endcatch

   HangulHanjaConversion.Close
   Sleep 1

  Call hCloseDocument

endcase

