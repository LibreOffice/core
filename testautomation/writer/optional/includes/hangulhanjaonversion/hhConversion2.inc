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
'* short description : Test of Hangul/Hanja Conversion - 2
'*
'************************************************************************
'*
' #1 tHHConversion_1         'Hanja/Hangul (Korean -> Chinese)
' #1 tHHConversion_2         'Hanja/Hangul (Chinese -> Korean)
' #1 tHHConversion_3         'Hanja(Hangul) (Korean -> Chinese)
' #1 tHHConversion_4         'Hanja(Hangul) (Chinese -> Korean)
' #1 tHHConversion_5         'Hangul(Hanja) (Korean -> Chinese)
' #1 tHHConversion_6         'Hangul(Hanja) (Chinese -> Korean)
' #1 tHHConversion_7         ''Hanja As Ruby Above Hangul' (Korean -> Chinese)
' #1 tHHConversion_8         ''Hanja As Ruby Above Hangul' (Chinese -> Korean)
' #1 tHHConversion_9         ''Hanja As Ruby Below Hangul' (Korean -> Chinese)
' #1 tHHConversion_10        ''Hanja As Ruby Below Hangul' (Chinese -> Korean)
' #1 tHHConversion_11        ''Hangul As Ruby Above Hanja' (Korean -> Chinese)
' #1 tHHConversion_12        ''Hangul As Ruby Above Hanja' (Chinese -> Korean)
' #1 tHHConversion_13        ''Hangul As Ruby Below Hanja' (Korean -> Chinese)
' #1 tHHConversion_14        ''Hangul As Ruby Below Hanja' (Chinese -> Korean)
' #1 tHHConversion_15        'Hangul Only
' #1 tHHConversion_16        'Hanja Only
' #1 tHHConversion_17        'Ignore
' #1 tHHConversion_18        'Always Ignore -1
' #1 tHHConversion_19        'Always Ignore -2
' #1 tHHConversion_20        'Replace
' #1 tHHConversion_21        'Always Replace -1
' #1 tHHConversion_22        'Always Replace -2
' #1 tHHConversion_23        'Always Replace -3
' #1 tHHConversion_24        'Replace by character -1
' #1 tHHConversion_25        'Replace by character -2(Always Ignore)
' #1 tHHConversion_26        'Replace by character -3(Always Replace)
'*
'\***********************************************************************

testcase tHHConversion_1
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String
  Dim  sResult    as String
  Dim  i          as Integer

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\KoreanToChinese.sxw") 
  sResult    = "漢字入力은該當하入力하中文信息"


  printlog "- Hanja/Hangul (Korean -> Chinese)"
  '/// <b> Simple convert from Hangul into Hanja </b>

  Call hNewDocument

  '/// Open a test file , which includes some
  '/// + Korean chacters and chinese character
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<MOD1 Home>"

  '/// Tools / HangulHanjaConversion ,
  '/// + Press Peplace button
   ToolsLanguageHangulHanjaConversion : WaitSlot
   Kontext "HangulHanjaConversion"
     For i = 1 to 15
         if NOT HangulHanjaConversion.Exists then
             Exit for
         end if
         ReplaceBtn.Click
         Sleep 1
     next i

  '/// Check if the result is correct
   Call wTypeKeys "<MOD1 Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   Sleep 1
   if GetClipboardText <> sResult then
       Warnlog "Conversion result not: " & sResult & " but: " & GetClipboardText
   end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_2
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String
  Dim  sResult    as String
  Dim  i          as Integer

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\ChineseToKorean.sxw")
  sResult    = "상산하乡황하长강력은해당"

  printlog "- Hanja/Hangul (Chinese -> Korean)"
  '/// <b> Simple convert from Hanja into Hangul </b>

  Call hNewDocument

  '/// Open a test file , which includes some
  '/// + Chinese character and Korean chacters
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<MOD1 Home>"

  '/// Tools / HangulHanjaConversion , check hanja only .
  '/// + Press Peplace button
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     HanjaOnly.Check
     For i = 1 to 15
         if NOT HangulHanjaConversion.Exists then
             Exit for
         end if
         ReplaceBtn.Click
         Sleep 1
     next i

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

testcase tHHConversion_3
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String
  Dim  sResult    as String
  Dim  i          as Integer

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\KoreanToChinese.sxw")
  sResult = "漢字(한자)入力(입력)은該當(해당)하入力(입력)하中文信息"


  printlog "- Hanja(Hangul) (Korean -> Chinese)"
  '/// <b> Test Hanja(Hangul) - Simple convert from Hangul into Hanja </b>

  Call hNewDocument

  '/// Open a test file , which includes some
  '/// + Korean chacters and chinese character
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<MOD1 Home>"

  '/// Tools / HangulHanjaConversion , check Hanja(Hangul)
  '/// + Press Peplace button
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     HanjaHangul.Check
     Sleep 1
     For i = 1 to 15
         if NOT HangulHanjaConversion.Exists then
             Exit for
         end if
         ReplaceBtn.Click
         Sleep 1
     next i

  '/// Check if the result is correct
   Call wTypeKeys "<MOD1 Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   Sleep 1
   if GetClipboardText <> sResult then
        Warnlog "Conversion result not: " & sResult & " but: " & GetClipboardText
   end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_4
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String
  Dim  sResult    as String
  Dim  i          as Integer

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\ChineseToKorean.sxw")
  sResult = "上山下(상산하)乡黄河(황하)长江(강)력은該當(해당)"


  printlog "- Hanja(Hangul) (Chinese -> Korean)"
  '/// <b> Test Hanja(Hangul) - Simple convert from Hanja into Hangul </b>

  Call hNewDocument

  '/// Open a test file , which includes some
  '/// + Chinese character and Korean chacters
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<MOD1 Home>"

  '/// Tools / HangulHanjaConversion , check Hanja(Hangul).
  '/// + Press Peplace button
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     HanjaHangul.Check
     Sleep 1
     For i = 1 to 15
         if NOT HangulHanjaConversion.Exists then
             Exit for
         end if
         ReplaceBtn.Click
         Sleep 1
     next i

  '/// Check if the result is correct
   Call wTypeKeys "<MOD1 Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   Sleep 1
   if GetClipboardText <> sResult then
        Warnlog "Conversion result not: " & sResult & " but: " & GetClipboardText
   end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_5
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String
  Dim  sResult    as String
  Dim  i          as Integer

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\KoreanToChinese.sxw")
  sResult    = "한자(漢字)입력(入力)은해당(該當)하입력(入力)하中文信息"


  printlog "- Hangul(Hanja) (Korean -> Chinese)"
  '/// <b> Test Hangul(Hanja) - Simple convert from Hangul into Hanja </b>

  Call hNewDocument

  '/// Open a test file , which includes some
  '/// + Korean chacters and chinese character
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<MOD1 Home>"

  '/// Tools / HangulHanjaConversion , check Hangul(Hanja) ,
  '/// + Press Peplace button
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     HangulHanja.Check
     Sleep 1
     For i = 1 to 15
         if NOT HangulHanjaConversion.Exists then
             Exit for
         end if
         ReplaceBtn.Click
         Sleep 1
     next i

  '/// Check if the result is correct
   Call wTypeKeys "<MOD1 Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   Sleep 1
   if GetClipboardText <> sResult then
        Warnlog "Conversion result not: " & sResult & " but: " & GetClipboardText
   end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_6
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String
  Dim  sResult    as String
  Dim  i          as Integer

  sTestFile  = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\ChineseToKorean.sxw")
  sResult = "상산하(上山下)乡황하(黄河)长강(江)력은해당(該當)"



  printlog "- Hangul(Hanja) (Chinese -> Korean)"
  '/// <b> Test Hangul(Hanja) - Simple convert from Hanja into Hangul </b>

  Call hNewDocument

  '/// Open a test file , which includes some
  '/// + Chinese character and Korean chacters
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<MOD1 Home>"

  '/// Tools / HangulHanjaConversion , check Hangul(Hanja),
  '/// + Press Peplace button
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     HangulHanja.Check
     Sleep 1
     For i = 1 to 15
         if NOT HangulHanjaConversion.Exists then
             Exit for
         end if
         ReplaceBtn.Click
         Sleep 1
     next i

  '/// Check if the result is correct
   Call wTypeKeys "<MOD1 Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   Sleep 1
   if GetClipboardText <> sResult then
        Warnlog "Conversion result not: " & sResult & " but: " & GetClipboardText
   end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_7
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile                as String
  Dim  sHangulText , sHanjaText as String
  Dim  iAlignment  , iPosition  as Integer

  sHangulText =  "한자"
  sHanjaText  =  "漢字"
  iAlignment  =  2  'Center
  iPosition   =  1  'Top
  sTestFile   =  Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\KoreanToChinese.sxw")

  printlog "- 'Hanja As Ruby Above Hangul' (Korean -> Chinese)"
  '/// <b> Test 'Hanja As Ruby Above Hangul' - Simple convert from Hangul into Hanja </b>

  Call hNewDocument

  '/// Open a test file , which includes some Korean chacters
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion , check 'Hanja As Ruby Above Hangul' ,
  '/// + press Peplace button
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     HanjaAsRubyAbove.Check
     Sleep 1
     ReplaceBtn.Click
     Sleep 1
   HangulHanjaConversion.Close

   Call wTypeKeys "<Home>"
  '/// Format/Ruby , check if the result is correct
   FormatRuby
   Kontext "RubyDialog"
     Sleep 4
     if BasisText1.GetText <> sHangulText then
         Warnlog "Base text : hope to get " & sHangulText & " but get " & BasisText1.GetText
     end if
     if RubyText1.GetText  <> sHanjaText then
         Warnlog "Ruby text : hope to get " & sHanjaText & " but get " & RubyText1.GetText
     end if
     if Adjust.GetSelIndex <> iAlignment  then
         Warnlog "Alignment : hope to get " & Adjust.GeTSelText(iAlignment) & " but get " & Adjust.GeTSelText
     end if
     if RubyPosition.GetSelIndex <> iPosition then
         Warnlog "Position: hope to get " & RubyPosition.GeTSelText(iPosition) & " but get " & RubyPosition.GeTSelText
     end if
   RubyDialog.Close
   Sleep 2

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_8
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile                as String
  Dim  sHangulText , sHanjaText as String
  Dim  iAlignment  , iPosition  as Integer

  sHangulText =  "상산하"
  sHanjaText  =  "上山下"
  iAlignment  =  2  'Center
  iPosition   =  1  'Top
  sTestFile   =  Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\ChineseToKorean.sxw")

  printlog "- 'Hanja As Ruby Above Hangul' (Chinese -> Korean)"
  '/// <b> Test 'Hanja As Ruby Above Hangul' - Simple convert from Hanja into Hangul </b>

  Call hNewDocument

  '/// Open a test file , which includes some Chinese chacters
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion , check 'Hanja As Ruby Above Hangul' ,
  '/// + press Peplace button
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     HanjaAsRubyAbove.Check
     Sleep 1
     ReplaceBtn.Click
     Sleep 1
   HangulHanjaConversion.Close

   Call wTypeKeys "<Home>"
  '/// Format/Ruby , check if the result is correct
   FormatRuby
   Kontext "RubyDialog"
     Sleep 4
     if BasisText1.GetText <> sHangulText then
         Warnlog "#i39186# Base text : hope to get " & sHangulText & " but get " & BasisText1.GetText
     end if
     if RubyText1.GetText  <> sHanjaText then
         Warnlog "#i39186# Ruby text : hope to get " & sHanjaText & " but get " & RubyText1.GetText
     end if
     if Adjust.GetSelIndex <> iAlignment  then
         Warnlog "Alignment : hope to get " & Adjust.GeTSelText(iAlignment) & " but get " & Adjust.GeTSelText
     end if
     if RubyPosition.GetSelIndex <> iPosition then
         Warnlog "Position: hope to get " & RubyPosition.GeTSelText(iPosition) & " but get " & RubyPosition.GeTSelText
     end if
   RubyDialog.Close
   Sleep 2

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_9
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile                as String
  Dim  sHangulText , sHanjaText as String
  Dim  iAlignment  , iPosition  as Integer

  sHangulText =  "한자"
  sHanjaText  =  "漢字"
  iAlignment  =  2  'Center
  iPosition   =  2  'Bottom
  sTestFile   =  Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\KoreanToChinese.sxw")

  printlog "- 'Hanja As Ruby Below Hangul' (Korean -> Chinese)"
  '/// <b> Test 'Hanja As Ruby Below Hangul' - Simple convert from Hangul into Hanja </b>

  Call hNewDocument

  '/// Open a test file , which includes some Korean chacters
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion , check 'Hanja As Ruby Below Hangul' ,
  '/// + press Peplace button
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     HanjaAsRubyBelow.Check
     Sleep 1
     ReplaceBtn.Click
     Sleep 1
   HangulHanjaConversion.Close

   Call wTypeKeys "<Home>"
  '/// Format/Ruby , check if the result is correct
   FormatRuby
   Kontext "RubyDialog"
     Sleep 4
     if BasisText1.GetText <> sHangulText then
         Warnlog "Base text : hope to get " & sHangulText & " but get " & BasisText1.GetText
     end if
     if RubyText1.GetText  <> sHanjaText then
         Warnlog "Ruby text : hope to get " & sHanjaText & " but get " & RubyText1.GetText
     end if
     if Adjust.GetSelIndex <> iAlignment  then
         Warnlog "Alignment : hope to get " & Adjust.GeTSelText(iAlignment) & " but get " & Adjust.GeTSelText
     end if
     if RubyPosition.GetSelIndex <> iPosition then
         Warnlog "Position: hope to get " & RubyPosition.GeTSelText(iPosition) & " but get " & RubyPosition.GeTSelText
     end if
   RubyDialog.Close
   Sleep 2

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_10
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile                as String
  Dim  sHangulText , sHanjaText as String
  Dim  iAlignment  , iPosition  as Integer

  sHangulText =  "상산하"
  sHanjaText  =  "上山下"
  iAlignment  =  2  'Center
  iPosition   =  2  'Bottom
  sTestFile   =  Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\ChineseToKorean.sxw")

  printlog "- 'Hanja As Ruby Below Hangul' (Chinese -> Korean)"
  '/// <b> Test 'Hanja As Ruby Below Hangul' - Simple convert from Hanja into Hangul </b>

  Call hNewDocument

  '/// Open a test file , which includes some Chinese chacters
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion , check 'Hanja As Ruby Below Hangul' ,
  '/// + press Peplace button
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     HanjaAsRubyBelow.Check
     Sleep 1
     ReplaceBtn.Click
     Sleep 1
   HangulHanjaConversion.Close

   Call wTypeKeys "<Home>"
  '/// Format/Ruby , check if the result is correct
   FormatRuby
   Kontext "RubyDialog"
     Sleep 4
     if BasisText1.GetText <> sHangulText then
         Warnlog "#i39186# Base text : hope to get " & sHangulText & " but get " & BasisText1.GetText
     end if
     if RubyText1.GetText  <> sHanjaText then
         Warnlog "#i39186# Ruby text : hope to get " & sHanjaText & " but get " & RubyText1.GetText
     end if
     if Adjust.GetSelIndex <> iAlignment  then
         Warnlog "Alignment : hope to get " & Adjust.GeTSelText(iAlignment) & " but get " & Adjust.GeTSelText
     end if
     if RubyPosition.GetSelIndex <> iPosition then
         Warnlog "Position: hope to get " & RubyPosition.GeTSelText(iPosition) & " but get " & RubyPosition.GeTSelText
     end if
   RubyDialog.Close
   Sleep 2

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_11
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile                as String
  Dim  sHangulText , sHanjaText as String
  Dim  iAlignment  , iPosition  as Integer

  sHangulText =  "한자"
  sHanjaText  =  "漢字"
  iAlignment  =  2  'Center
  iPosition   =  1  'Top
  sTestFile   =  Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\KoreanToChinese.sxw")

  printlog "- 'Hangul As Ruby Above Hanja' (Korean -> Chinese)"
  '/// <b> Test 'Hangul As Ruby Above Hanja' - Simple convert from Hangul into Hanja </b>

  Call hNewDocument

  '/// Open a test file , which includes some Korean chacters
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion , check 'Hangul As Ruby Above Hanja' ,
  '/// + press Peplace button
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     HangulAsRubyAbove.Check
     Sleep 1
     ReplaceBtn.Click
     Sleep 1
   HangulHanjaConversion.Close

   Call wTypeKeys "<Home>"
  '/// Format/Ruby , check if the result is correct
   FormatRuby
   Kontext "RubyDialog"
     Sleep 4
     if BasisText1.GetText <> sHanjaText then
         Warnlog "Base text : hope to get " & sHangulText & " but get " & BasisText1.GetText
     end if
     if RubyText1.GetText  <> sHangulText then
         Warnlog "Ruby text : hope to get " & sHanjaText & " but get " & RubyText1.GetText
     end if
     if Adjust.GetSelIndex <> iAlignment  then
         Warnlog "Alignment : hope to get " & Adjust.GeTSelText(iAlignment) & " but get " & Adjust.GeTSelText
     end if
     if RubyPosition.GetSelIndex <> iPosition then
         Warnlog "Position: hope to get " & RubyPosition.GeTSelText(iPosition) & " but get " & RubyPosition.GeTSelText
     end if
   RubyDialog.Close
   Sleep 2

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_12
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile                as String
  Dim  sHangulText , sHanjaText as String
  Dim  iAlignment  , iPosition  as Integer

  sHangulText =  "상산하"
  sHanjaText  =  "上山下"
  iAlignment  =  2  'Center
  iPosition   =  1  'Top
  sTestFile   =  Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\ChineseToKorean.sxw")

  printlog "- 'Hangul As Ruby Above Hanja' (Chinese -> Korean)"
  '/// <b> Test 'Hangul As Ruby Above Hanja' - Simple convert from Hanja into Hangul </b>

  Call hNewDocument

  '/// Open a test file , which includes some Chinese chacters
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion , check 'Hangul As Ruby Above Hanja' ,
  '/// + press Peplace button
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     HangulAsRubyAbove.Check
     Sleep 1
     ReplaceBtn.Click
     Sleep 1
   HangulHanjaConversion.Close

   Call wTypeKeys "<Home>"
  '/// Format/Ruby , check if the result is correct
   FormatRuby
   Kontext "RubyDialog"
     Sleep 4
     if BasisText1.GetText <> sHanjaText then
         Warnlog "#i39186# Base text : hope to get " & sHangulText & " but get " & BasisText1.GetText
     end if
     if RubyText1.GetText  <> sHangulText then
         Warnlog "#i39186# Ruby text : hope to get " & sHanjaText & " but get " & RubyText1.GetText
     end if
     if Adjust.GetSelIndex <> iAlignment  then
         Warnlog "Alignment : hope to get " & Adjust.GeTSelText(iAlignment) & " but get " & Adjust.GeTSelText
     end if
     if RubyPosition.GetSelIndex <> iPosition then
         Warnlog "Position: hope to get " & RubyPosition.GeTSelText(iPosition) & " but get " & RubyPosition.GeTSelText
     end if
   RubyDialog.Close
   Sleep 2

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_13
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile                as String
  Dim  sHangulText , sHanjaText as String
  Dim  iAlignment  , iPosition  as Integer

  sHangulText =  "한자"
  sHanjaText  =  "漢字"
  iAlignment  =  2  'Center
  iPosition   =  2  'Bottom
  sTestFile   =  Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\KoreanToChinese.sxw")

  printlog "- 'Hangul As Ruby Below Hanja' (Korean -> Chinese)"
  '/// <b> Test 'Hangul As Ruby Below Hanja' - Simple convert from Hangul into Hanja </b>

  Call hNewDocument

  '/// Open a test file , which includes some Korean chacters
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion , check 'Hangul As Ruby Below Hanja' ,
  '/// + press Peplace button
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     HangulAsRubyBelow.Check
     Sleep 1
     ReplaceBtn.Click
     Sleep 1
   HangulHanjaConversion.Close

   Call wTypeKeys "<Home>"
  '/// Format/Ruby , check if the result is correct
   FormatRuby
   Kontext "RubyDialog"
     Sleep 4
     if BasisText1.GetText <> sHanjaText then
         Warnlog "Base text : hope to get " & sHangulText & " but get " & BasisText1.GetText
     end if
     if RubyText1.GetText  <> sHangulText then
         Warnlog "Ruby text : hope to get " & sHanjaText & " but get " & RubyText1.GetText
     end if
     if Adjust.GetSelIndex <> iAlignment  then
         Warnlog "Alignment : hope to get " & Adjust.GeTSelText(iAlignment) & " but get " & Adjust.GeTSelText
     end if
     if RubyPosition.GetSelIndex <> iPosition then
         Warnlog "Position: hope to get " & RubyPosition.GeTSelText(iPosition) & " but get " & RubyPosition.GeTSelText
     end if
   RubyDialog.Close
   Sleep 2

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_14
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile                as String
  Dim  sHangulText , sHanjaText as String
  Dim  iAlignment  , iPosition  as Integer

  sHangulText =  "상산하"
  sHanjaText  =  "上山下"
  iAlignment  =  2  'Center
  iPosition   =  2  'Bottom
  sTestFile   =  Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\ChineseToKorean.sxw")

  printlog "- 'Hangul As Ruby Below Hanja' (Chinese -> Korean)"
  '/// <b> Test 'Hangul As Ruby Below Hanja' - Simple convert from Hanja into Hangul </b>

  Call hNewDocument

  '/// Open a test file , which includes some Chinese chacters
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion , check 'Hangul As Ruby Below Hanja' ,
  '/// + press Peplace button
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     HangulAsRubyBelow.Check
     Sleep 1
     ReplaceBtn.Click
     Sleep 1
   HangulHanjaConversion.Close

   Call wTypeKeys "<Home>"
  '/// Format/Ruby , check if the result is correct
   FormatRuby
   Kontext "RubyDialog"
     Sleep 4
     if BasisText1.GetText <> sHanjaText then
         Warnlog "#i39186# Base text : hope to get " & sHangulText & " but get " & BasisText1.GetText
     end if
     if RubyText1.GetText  <> sHangulText then
         Warnlog "#i39186# Ruby text : hope to get " & sHanjaText & " but get " & RubyText1.GetText
     end if
     if Adjust.GetSelIndex <> iAlignment  then
         Warnlog "Alignment : hope to get " & Adjust.GeTSelText(iAlignment) & " but get " & Adjust.GeTSelText
     end if
     if RubyPosition.GetSelIndex <> iPosition then
         Warnlog "Position: hope to get " & RubyPosition.GeTSelText(iPosition) & " but get " & RubyPosition.GeTSelText
     end if
   RubyDialog.Close
   Sleep 2

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_15
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String

  sTestFile   =  Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\KoreanToChinese.sxw")

  printlog "- Hangul Only"
  '/// <b> Test 'Hangul Only' </b>

  Call hNewDocument

  '/// Open a test file , which includes some Korean chacters .
  '/// + Set focus before Korean character
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion ,
  '/// + 'Hangul Only' should be active and Unchecked,
  '/// + 'Hanja Only' should be inactive
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     if HangulOnly.IsChecked = TRUE then
         Warnlog "The default of Hangul Only should be Unchecked ! Bug#116320"
     end if

     try
         HangulOnly.UnCheck
         HangulOnly.Check
     catch
         Warnlog "'Hangul Only' Should be active !"
     endcatch

     try
         HanjaOnly.Check
         Warnlog "'Hanja Only' should be inactive !"
     catch
     endcatch
   HangulHanjaConversion.Close

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_16
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String

  sTestFile   =  Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\ChineseToKorean.sxw")

  printlog "- Hanja Only"
  '/// <b> Test 'Hanja Only' </b>

  Call hNewDocument

  '/// Open a test file , which includes some Chinese chacters .
  '/// + Set focus before Chinese character
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion ,
  '/// + 'Hanja Only' should be active and Unchecked,
  '/// + 'Hangul Only' should be inactive
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     if HanjaOnly.IsChecked = TRUE then
         Warnlog "The default of Hanja Only should be Unchecked ! Bug#116320"
     end if

     try
         HanjaOnly.UnCheck
         HanjaOnly.Check
     catch
         Warnlog "'Hanja Only' Should be active !"
     endcatch

     try
         HangulOnly.Check
         Warnlog "'Hangul Only' should be inactive !"
     catch
     endcatch
   HangulHanjaConversion.Close

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_17
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile            as String
  Dim  sResult   , sWord    as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\KoreanToChinese.sxw")
  sResult   = "한자入力"
  sWord     = "入力"
  GetClipboardText = ""

  printlog "- Ignore"
  '/// <b> Test 'Ignore' </b>

  Call hNewDocument

  '/// Open a test file , which includes some Korean chacters .
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion , press Ignore ,
  '/// + next word should be selected for conversion.
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     Ignore.Click
     Sleep 1
     if Word.GetText <> sWord then
         Warnlog "Hope get " & sWord & " in Word" & " but get " & Word.GetText
     end if
     ReplaceBtn.Click
     Sleep 1
   HangulHanjaConversion.Close

   Call wTypeKeys "<Home>"
   Call wTypeKeys "<Shift Right>" , 4
   EditCopy
   if GetClipboardText <> sResult then
       Warnlog "Should get " & sResult & " but get " & GetClipboardText
   end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_18
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile   as String
  Dim  sResult     as String
  Dim  sWord       as String
  Dim  i           as Integer

  sTestFile    = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\alwayIgnore.sxw")
  GetClipboardText = "" 
  sWord = "산하"
  sResult = "산하  황하 상산하乡   산하"

  printlog "- Always Ignore -1"
  '/// <b> Test 'Always Ignore' -1 </b>

  Call hNewDocument

  '/// Open a test file , which includes some Chinese chacters =>.
  '/// + 山下  上山下乡   山下
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion , press Always Ignore ,
  '/// + (here, 山下 should be selected) , then press Replace
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     AlwaysIgnore.Click
     Sleep 1
     For i = 1 to 4
         if NOT HangulHanjaConversion.Exists then
             Exit for
         end if
         ReplaceBtn.Click
         Sleep 1
     next i

  '/// The result should be only 黄河 and 上山下 are converted
   Call wTypeKeys "<Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   if GetClipboardText <> sResult then
       Warnlog "Should get " & sResult & " but get " & GetClipboardText
   end if

  '/// Close this active document
  Call hCloseDocument

  '/// So far , there is still a StarOffice session exists .

  '/// Reopen the test file
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion ,
  '/// + ,(here,  黄河 should be selected)
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     if Word.GetText <>  sWord then
         Warnlog "#i39308# Don't get " &  sWord & " but get " & Word.GetText
     end if

     For i = 1 to 4
         if NOT HangulHanjaConversion.Exists then
             Exit for
         end if
         ReplaceBtn.Click
         Sleep 1
     next i

  '/// The result should be only 黄河 and 上山下 are converted
   Call wTypeKeys "<Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   if GetClipboardText <> sResult then
       QAErrorlog "#i39308# Should get " & sResult & " but get " & GetClipboardText
   end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_19
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String
  Dim  sWord      as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\alwayIgnore.sxw")
  sWord     = "산하"

  printlog "- Always Ignore -2"
  '/// <b> Test 'Always Ignore' -2 </b>

  Call hNewDocument

  '/// Open a test file , which includes some Chinese chacters =>.
  '/// + 山下  上山下乡   山下
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion , press Always Ignore ,
  '/// + (here, 山下 should be selected)
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     AlwaysIgnore.Click
     Sleep 1
   HangulHanjaConversion.Close

  Call hCloseDocument

 '/// Close all Office session
  try
      Call hCloseDocument
      Call hCloseDocument
  catch
  endcatch
  Sleep 2

  Call hNewDocument
  '/// Reopen the test file
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion , 山下 should be selcted
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     if Word.GetText <>  sWord then
         Warnlog "Don't get " &  sWord & " but get " & Word.GetText
     end if
   HangulHanjaConversion.Close

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_20
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String
  Dim  sWord      as String
  Dim  sResult    as String
  Dim  i          as Integer

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\Replace.sxw")
  sWord     = "書翰"
  sResult   = "書翰자  와"

  GetClipboardText = ""

  printlog "- Replace"
  '/// <b> Test Replace </b>

  Call hNewDocument

  '/// Open a test file , which includes some Korean chacters
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion , select a word in selection
  '/// + then press Replace button
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"

    try
        Suggestions.Select   sWord
    catch
        Warnlog "No suggestion made in Hangul/Hanja converter"
        HangulHanjaConversion.Close
        Call hCloseDocument
        goto endsub
    endcatch
        
    Sleep 1
    For i = 1 to 4
        if NOT HangulHanjaConversion.Exists then
            Exit for
        end if
        ReplaceBtn.Click
        Sleep 1
    next i

    '/// Check if the result is right
    Call wTypeKeys "<Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    if GetClipboardText <> sResult then
        Warnlog "Should get " & sResult & " but get " & GetClipboardText
    end if

    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_21
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String
  Dim  sWord1      as String
  Dim  sWord2      as String
  Dim  sResult    as String
  Dim  i          as Integer

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\Replace.sxw")
  sWord1     = "書翰"  :    sWord2     = "書翰"
  sResult = "書翰자  와"
  GetClipboardText = ""
  



  printlog "- Replace All -1"
  '/// <b> Test 'Replace All' -1</b>
  '/// <b> replace list will be active for the office session  </b>

  Call hNewDocument

  '/// Open a test file , which includes some Korean chacters
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion , select a word in selection,
  '/// + press 'always replace' , then press Replace button
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
    try
        Suggestions.Select   sWord1
    catch
        Warnlog "No suggestion made in Hangul/Hanja converter"
        HangulHanjaConversion.Close
        Call hCloseDocument
        goto endsub
    endcatch

    Sleep 2
    AlwaysReplace.Click
    Sleep 1
    For i = 1 to 2
        if NOT HangulHanjaConversion.Exists then
            Exit for
        end if
        ReplaceBtn.Click
        Sleep 1
    next i

    '/// Check if the result is right
    Call wTypeKeys "<Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    Sleep 1
    if GetClipboardText <> sResult then
        Warnlog "1st:Should get " & sResult & " but get " & GetClipboardText
    end if

    '/// Close the file , so far , there is still a StarOffice session
    Call hCloseDocument

    Call hNewDocument

    '/// Reopen this test file
    hFileOpenLocally( sTestFile )
    Call wTypeKeys "<Home>"

    '/// Tools / HangulHanjaConversion
    ToolsLanguageHangulHanjaConversion
    Kontext "HangulHanjaConversion"
    if Word.GetText <> sWord2 then
        Warnlog "#i39308# Should get " & sWord2 & " but get " & Word.GetText
        HangulHanjaConversion.Close
        Call hCloseDocument
        goto endsub
    end if
    For i = 1 to 2
        if NOT HangulHanjaConversion.Exists then
            Exit for
        end if
        ReplaceBtn.Click
        Sleep 1
    next i

    '/// Check if the result is right
    Call wTypeKeys "<Home>"
    Call wTypeKeys "<Shift End>"
    GetClipboardText = ""
    EditCopy
    Sleep 1
    if GetClipboardText <> sResult then
        Warnlog "2nd:Should get " & sResult & " but get " & GetClipboardText
    end if
    
    Call hCloseDocument
    
endcase

'-----------------------------------------------------------------

testcase tHHConversion_22
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


    Dim  sTestFile  as String
    Dim  i          as Integer

    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\Replace.sxw")
    GetClipboardText = ""

    printlog "- Replace All -2"
    '/// <b> Test 'Replace All' -2</b>
    '/// <b> replace list will NOT be active  when office session is deleted </b>

    Call hNewDocument

    '/// Open a test file , which includes some Korean chacters
    hFileOpenLocally( sTestFile )
    Call wTypeKeys "<Home>"

    '/// Tools / HangulHanjaConversion , select a word in selection,
    '/// + press 'always replace' , then press Replace button
    ToolsLanguageHangulHanjaConversion
    Kontext "HangulHanjaConversion"
    try
        Suggestions.Select 1
    catch
        Warnlog "No suggestion made in Hangul/Hanja converter"
        HangulHanjaConversion.Close
        Call hCloseDocument
        goto endsub
    endcatch
    
    Sleep 1
    AlwaysReplace.Click
    if HangulHanjaConversion.Exists then HangulHanjaConversion.Close

    '/// Close all documents
    For i = 1 to GetDocumentCount - 1
        Call hCloseDocument
    next i

    '/// Restart the office
    Call hStartTheOffice
    Call hNewDocument
  	'/// Reopen this test file
   	hFileOpenLocally( sTestFile )
   	Call wTypeKeys "<Home>"

	'/// Tools / HangulHanjaConversion
    ToolsLanguageHangulHanjaConversion
    Kontext "HangulHanjaConversion"
	if HangulHanjaConversion.Exists then
	   	HangulHanjaConversion.Close
	else
		Warnlog "Replace list is active after office-restart"	
	end if

    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_23
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


    Dim  sTestFile  as String
    Dim  sWord      as String
    Dim  sResult    as String
    Dim  i          as Integer
    
    sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\replaceAll.sxw")
	sResult = "은력銀河"
    GetClipboardText = ""
    
    printlog "- Replace All -3"
    '/// <b> Test 'Replace All' -3 </b>
    '/// <b> Replacing AF must not lead to replace the AF within CAFE </b>
    
    Call hNewDocument
    
    '/// Open a test file , which includes some Korean chacters
    hFileOpenLocally( sTestFile )
    Call wTypeKeys "<Home>"

    '/// Tools / HangulHanjaConversion , select a word in selection,
    '/// + press 'always replace' , then press Replace button
    ToolsLanguageHangulHanjaConversion
    Kontext "HangulHanjaConversion"
	for i = 1 to 2
	    if ReplaceBtn.IsEnabled then    
		    try
 	  	 	    Suggestions.Select 1
		    catch
    		    Warnlog "No suggestion made in Hangul/Hanja converter"
        		HangulHanjaConversion.Close
		        Call hCloseDocument
    		    goto endsub
	    	endcatch
		else
			Ignore.Click
		end if
	next i
    Sleep 1
    AlwaysReplace.Click
    Sleep 1
    For i = 1 to 2
        if NOT HangulHanjaConversion.Exists then
            Exit for
        end if
        ReplaceBtn.Click
        Sleep 1
    next i

    '/// Check if the result is right
    Call wTypeKeys "<Home>"
    Call wTypeKeys "<Shift End>"
    EditCopy
    if GetClipboardText <> sResult then
        Warnlog "Should get " & sResult & " but get " & GetClipboardText
    end if

    Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_24
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String
  Dim  sResult    as String
  Dim  i          as Integer

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\replaceAll.sxw")
  sResult   = "垠力垠下"
  GetClipboardText = ""

  printlog "- Replace by character -1"
  '/// <b> Test 'Replace by character' -1 </b>

  Call hNewDocument

  '/// Open a test file , which includes some Korean chacters
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion , the default for 'Replace by character'
  '/// + is OFF . Check 'Replace by character' , then press Replace button
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     if ReplaceByCharacter.IsChecked = TRUE then
         Warnlog "The default for 'Replace By Character' should be OFF !"
     end if
     ReplaceByCharacter.Check
     Sleep 1
     For i = 1 to 4
         if NOT HangulHanjaConversion.Exists then
             Exit for
         end if
         ReplaceBtn.Click
         Sleep 1
     next i

  '/// Check if the result is right
   Call wTypeKeys "<Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   if GetClipboardText <> sResult then
       Warnlog "Should get " & sResult & " but get " & GetClipboardText
   end if

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHConversion_25
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile   as String
  Dim  sResult     as String
  Dim  i           as Integer

  sTestFile    = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\alwayIgnore.sxw")
  sResult      = "山하  황하 상山하乡   山하"
  GetClipboardText = ""

  printlog "- Replace by character -2"
  '/// <b> Test 'Replace by character' -2 </b>
  '/// <b> Test Always Ignore when 'Replace by character' is checked </b>

  Call hNewDocument

  '/// Open a test file , which includes some Chinese chacters =>.
  '/// + 山下  上山下乡   山下
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion , Check 'Replace by character' ,
  '/// + press Always Ignore ,  then press Replace
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     ReplaceByCharacter.Check
     Sleep 1
     AlwaysIgnore.Click
     Sleep 1
     For i = 1 to 10
         if NOT HangulHanjaConversion.Exists then
             Exit for
         end if
         ReplaceBtn.Click
         Sleep 1
     next i

  '/// The result should be only 黄河 and 上山下 are converted
   Call wTypeKeys "<Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   if GetClipboardText <> sResult then
       Warnlog "Should get " & sResult & " but get " & GetClipboardText
   end if

  '/// Close this active document
  Call hCloseDocument

 'The purpose is to delete Office session
  try
      Call hCloseDocument
      Call hCloseDocument
  catch
  endcatch
  Sleep 2

endcase

'-----------------------------------------------------------------

testcase tHHConversion_26
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String
  Dim  sWord      as String
  Dim  sResult    as String
  Dim  i          as Integer

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\replaceAll.sxw")
  sWord     = "恩"
  sResult   = "恩力恩下"
  GetClipboardText = ""

  printlog "- Replace by character -3"
  '/// <b> Test 'Replace by character' -3 </b>
  '/// <b> Test Always Replace when 'Replace by character' is checked </b>

  Call hNewDocument

  '/// Open a test file , which includes some Korean chacters
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<Home>"

  '/// Tools / HangulHanjaConversion ,  Check 'Replace by character' ,
  '/// + select a word in selection, press 'always replace' ,
  '/// + then press Replace button
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"
     ReplaceByCharacter.Check
     Sleep 1
     Grid.Typekeys "<Right>"
     Sleep 1
     AlwaysReplace.Click
     Sleep 1
     For i = 1 to 4
         if NOT HangulHanjaConversion.Exists then
             Exit for
         end if
         ReplaceBtn.Click
         Sleep 1
     next i

  '/// Check if the result is right
   Call wTypeKeys "<Home>"
   Call wTypeKeys "<Shift End>"
   EditCopy
   if GetClipboardText <> sResult then
       Warnlog "Should get " & sResult & " but get " & GetClipboardText
   end if

  Call hCloseDocument

 'Close all Office session
  try
      Call hCloseDocument
      Call hCloseDocument
  catch
  endcatch

endcase

'-----------------------------------------------------------------

