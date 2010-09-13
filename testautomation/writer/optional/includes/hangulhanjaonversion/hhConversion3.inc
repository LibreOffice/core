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
'* short description : Test of Hangul/Hanja Conversion - 3
'*
'\***********************************************************************

testcase tHHOptions_1
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile  as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\hangul.sxw")

  printlog "- User defined dictionaries"
  '/// <b> User defined dictionaries </b>

  Call hNewDocument

  '/// Open a test file , which includes some
  '/// + Korean chacters and chinese character
   hFileOpenLocally( sTestFile )

   Call wTypeKeys "<MOD1 Home>"

  '/// Tools / HangulHanjaConversion , Press Options button ,
  '/// + there should be no entries in User defined dictionaries
   ToolsLanguageHangulHanjaConversion : WaitSlot
   Kontext "HangulHanjaConversion"

     Options.Click
     Kontext "HangulHanjaOptions"
       if UserDefineDictionary.GetItemCount <> 0 then
           Warnlog "With a standard installation the will be no entry here !"
       end if
     HangulHanjaOptions.Cancel

   Kontext "HangulHanjaConversion"
   HangulHanjaConversion.Close

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHOptions_2
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile        as String
  Dim  sDictionaryName1 as String
  Dim  sDictionaryName2 as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\hangul.sxw")
  sDictionaryName1 = "myDictionary1"
  sDictionaryName2 = "myDictionary2"

  printlog "- Options - New -1"
  '/// <b> Options - New (more than 2 dictionaries) </b>

  Call hNewDocument

  '/// Open a test file , which includes some
  '/// + Korean chacters and chinese character
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<MOD1 Home>"

  '/// Tools / HangulHanjaConversion , Press Options button ,
  '/// + then press new button , enter 1 new dictionar name ,
  '/// + close the new dictionary dialog , and add another new
  '/// + dictionary name
   ToolsLanguageHangulHanjaConversion: WaitSlot
   Kontext "HangulHanjaConversion"

     Options.Click
     Kontext "HangulHanjaOptions"
       NewDictionary.Click
       Kontext "HangulHanjaNewDictionary"
       if NOT HangulHanjaNewDictionary.Exists then
           Warnlog "The window isn't up!"
           Kontext "HangulHanjaOptions"
           goto NOTest
       end if
       DictionaryName.SetText sDictionaryName1
       HangulHanjaNewDictionary.OK

     Kontext "HangulHanjaOptions"
       NewDictionary.Click
       Kontext "HangulHanjaNewDictionary"
       DictionaryName.SetText sDictionaryName2
       HangulHanjaNewDictionary.OK

    '/// Test if the new dictionaries' name exist and are checked
     Kontext "HangulHanjaOptions"
       UserDefineDictionary.typekeys "<Home>"
       if UserDefineDictionary.GetSelText <> sDictionaryName1 then
           Warnlog "The dictionary1 name should be " & sDictionaryName & " but get " & UserDefineDictionary.GetSelText
       end if
       if UserDefineDictionary.IsChecked <> TRUE then
           Warnlog "The dictionary1 entry should be checked!"
       end if

       UserDefineDictionary.typekeys "<Down>"
       if UserDefineDictionary.GetSelText <> sDictionaryName2 then
           Warnlog "The dictionary2 name should be " & sDictionaryName2 & " but get " & UserDefineDictionary.GetSelText
       end if
       if UserDefineDictionary.IsChecked <> TRUE then
           Warnlog "The dictionary2 entry should be checked!"
       end if

      'remove the dictionary you just insert .
       UserDefineDictionary.typekeys "<Home>"
       DeleteDictionary.Click
       Sleep 1
       DeleteDictionary.Click
       Sleep 1
NOTest:
     HangulHanjaOptions.Cancel

   Kontext "HangulHanjaConversion"
   HangulHanjaConversion.Close

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHOptions_3
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile       as String
  Dim  sDictionaryName as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\hangul.sxw")
  sDictionaryName = "myDictionary"

  printlog "- Options - New -2"
  '/// <b> Options - New (duplicate dictionaries) </b>

  Call hNewDocument

  '/// Open a test file , which includes some
  '/// + Korean chacters and chinese characters
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<MOD1 Home>"

  '/// Tools / HangulHanjaConversion , Press Options button ,
  '/// + then press new button , enter 1 new dictionary name ,
  '/// + close the new dictionary dialog , and add this dictionary again
  '/// + there should be a error message dialog pop up
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"

     Options.Click
     Kontext "HangulHanjaOptions"
       NewDictionary.Click
       Kontext "HangulHanjaNewDictionary"
       DictionaryName.SetText sDictionaryName
       HangulHanjaNewDictionary.OK

     Kontext "HangulHanjaOptions"
       NewDictionary.Click
       Kontext "HangulHanjaNewDictionary"
       DictionaryName.SetText sDictionaryName
       HangulHanjaNewDictionary.OK
       Kontext "Active"
         if Active.Exists then
             if Active.GetRT = 304 then
                 Active.Ok
             end if
         else
             QAErrorLog "#i39920# The warning message box doesn't pup up ! "
         end if
     Kontext "HangulHanjaOptions"
      'remove the dictionary you just insert .
       UserDefineDictionary.typekeys "<Home>"
       DeleteDictionary.Click
       Sleep 1
     HangulHanjaOptions.Cancel

   Kontext "HangulHanjaConversion"
   HangulHanjaConversion.Close

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHOptions_4
warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub



  Dim  sTestFile       as String
  Dim  sDictionaryName as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\hangul.sxw")
  sDictionaryName = "myDictionary"

  printlog "- Options - New -3"
  '/// <b> Options - New (checkbox) </b>

  Call hNewDocument

  '/// Open a test file , which includes some
  '/// + Korean chacters and chinese character
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<MOD1 Home>"

  '/// Tools / HangulHanjaConversion , Press Options button ,
  '/// + then press new button , enter 1 new dictionar name ,
  '/// + close the new dictionary dialog
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"

     Options.Click
     Kontext "HangulHanjaOptions"
       NewDictionary.Click
       Kontext "HangulHanjaNewDictionary"
       DictionaryName.SetText sDictionaryName
       HangulHanjaNewDictionary.OK

     Kontext "HangulHanjaOptions"

    '/// Uncheck the new dictionary and close the dialog
     Kontext "HangulHanjaOptions"
       UserDefineDictionary.typekeys "<Home>"
       UserDefineDictionary.UnCheck
     HangulHanjaOptions.OK

  '/// Press options button , check if the dictionary isn't checked
   Kontext "HangulHanjaConversion"
     Options.Click
     Kontext "HangulHanjaOptions"
       if UserDefineDictionary.IsChecked = TRUE then
           Warnlog "The dictionary entry should NOT be checked!"
       end if

      'remove the dictionary you just insert .
       DeleteDictionary.Click
       Sleep 1

     HangulHanjaOptions.Cancel

   Kontext "HangulHanjaConversion"
   HangulHanjaConversion.Close

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHOptions_5
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile        as String
  Dim  sDictionaryName1 as String
  Dim  sDictionaryName2 as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\hangul.sxw")
  sDictionaryName1 = "nDictionary1"
  sDictionaryName2 = "mDictionary2"

  printlog "- Options - Edit (Book)"
  '/// <b> Options - Edit </b>

  Call hNewDocument

  '/// Open a test file , which includes some
  '/// + Korean chacters and chinese character
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<MOD1 Home>"

  '/// Tools / HangulHanjaConversion , Press Options button ,
  '/// + then press new button , insert 2 new dictionaries
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"

     Options.Click
     Kontext "HangulHanjaOptions"
       NewDictionary.Click
       Kontext "HangulHanjaNewDictionary"
       DictionaryName.SetText sDictionaryName1
       HangulHanjaNewDictionary.OK

     Kontext "HangulHanjaOptions"
       NewDictionary.Click
       Kontext "HangulHanjaNewDictionary"
       DictionaryName.SetText sDictionaryName2
       HangulHanjaNewDictionary.OK

    '/// Press Edit button
     Kontext "HangulHanjaOptions"
       EditDictionary.Click
       Kontext "HangulHanjaEditDictionary"
         if Book.GetItemText(1) <> sDictionaryName1 then
             Warnlog "The dictionary name should be " & sDictionaryName1 & " but get " & Book.GetItemText(1)
         end if
         if Book.GetItemText(2) <> sDictionaryName2 then
             Warnlog "The dictionary name should be " & sDictionaryName2 & " but get " & Book.GetItemText(2)
         end if
       HangulHanjaEditDictionary.Close

     Kontext "HangulHanjaOptions"
      'remove the dictionary you just insert .
       DeleteDictionary.Click
       Sleep 1
       DeleteDictionary.Click
       Sleep 1
     HangulHanjaOptions.Cancel

   Kontext "HangulHanjaConversion"
   HangulHanjaConversion.Close

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHOptions_6
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile                      as String
  Dim  sDictionaryName                as String
  Dim  sHangul1 , sHanja1a , sHanja1b as String
  Dim  sHangul2 , sHanja2a , sHanja2b as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\hangul.sxw")
  sDictionaryName = "myDictionary"
  sHangul1 = "에" : sHanja1a = "里" : sHanja1b = "梨"
  sHangul2 = "를" : sHanja2a = "力" : sHanja2b = "求"

  printlog "- Options - Edit (Original) -1"
  '/// <b> Options - Edit (Original) -2 </b>
  '/// <b> insert 2 entries in original, </b>
  '/// <b> check if the result is correct when inputing </b>
  '/// <b> the hangul character in original </b>

  Call hNewDocument

  '/// Open a test file , which includes some
  '/// + Korean chacters and chinese character
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<MOD1 Home>"

  '/// Tools / HangulHanjaConversion , Press Options button ,
  '/// + then press new button , insert 1 new dictionary
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"

     Options.Click
     Kontext "HangulHanjaOptions"
       NewDictionary.Click
       Kontext "HangulHanjaNewDictionary"
       DictionaryName.SetText sDictionaryName
       HangulHanjaNewDictionary.OK

    '/// Press Edit button
     Kontext "HangulHanjaOptions"
       EditDictionary.Click
       Kontext "HangulHanjaEditDictionary"
        '/// Test new button . It should NOT active when no entry in suggestion
         try
             NewEntry.Click
             Warnlog "New button should NOT be active when no entry in suggestion !"
         catch
         endcatch

        '/// Input 1 Hangul character in original
        '/// + and 2 Hanja character in suggestion , then click new
         Original.SetText    sHangul1
         Sleep 1
         Suggestion1.SetText sHanja1a
         Sleep 1
         Suggestion2.SetText sHanja1b
         Sleep 1
         NewEntry.Click
         Sleep 1
        '/// Input another Hangul character in original
        '/// + and 2 Hanja character in suggestion , then click new
         Original.SetText    sHangul2
         Sleep 1
         Suggestion1.SetText sHanja2a
         Sleep 1
         Suggestion2.SetText sHanja2b
         Sleep 1
         NewEntry.Click
         Sleep 1

        '/// Input 1st Hangul character in original ,
        '/// + check if 2 Hanja character are in suggestion
         Original.SetText sHangul1
         Sleep 1
         if Suggestion1.GetText <> sHanja1a then
             Warnlog "Don't get " & sHanja1a
         end if
         if Suggestion2.GetText <> sHanja1b then
             Warnlog "Don't get " & sHanja1b
         end if

        '/// Input 2nd Hangul character in original ,
        '/// + check if 2 Hanja character are in suggestion
         Original.SetText sHangul2
         Sleep 1
         if Suggestion1.GetText <> sHanja2a then
             Warnlog "Don't get " & sHanja2a
         end if
         if Suggestion2.GetText <> sHanja2b then
             Warnlog "Don't get " & sHanja2b
         end if

       HangulHanjaEditDictionary.Close

     Kontext "HangulHanjaOptions"
      'remove the dictionary you just insert .
       DeleteDictionary.Click
       Sleep 1
     HangulHanjaOptions.Cancel

   Kontext "HangulHanjaConversion"
   HangulHanjaConversion.Close

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHOptions_7
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile                      as String
  Dim  sDictionaryName                as String
  Dim  sHangul1 , sHanja1a , sHanja1b as String
  Dim  sHangul2 , sHanja2a , sHanja2b as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\hangul.sxw")
  sDictionaryName = "myDictionary"
  sHangul1 = "에" : sHanja1a = "里" : sHanja1b = "梨"
  sHangul2 = "를" : sHanja2a = "力" : sHanja2b = "求"

  printlog "- Options - Edit (Original) -2"
  '/// <b> Options - Edit (Original) -2 </b>
  '/// <b> insert 2 entries in original, </b>
  '/// <b> check if the result is correct when selecting </b>
  '/// <b> the hangul character in original </b>

  Call hNewDocument

  '/// Open a test file , which includes some
  '/// + Korean chacters and chinese character
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<MOD1 Home>"

  '/// Tools / HangulHanjaConversion , Press Options button ,
  '/// + then press new button , insert 1 new dictionary
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"

     Options.Click
     Kontext "HangulHanjaOptions"
       NewDictionary.Click
       Kontext "HangulHanjaNewDictionary"
       DictionaryName.SetText sDictionaryName
       HangulHanjaNewDictionary.OK

    '/// Press Edit button
     Kontext "HangulHanjaOptions"
       EditDictionary.Click
       Kontext "HangulHanjaEditDictionary"

        '/// Input 1 Hangul character in original
        '/// + and 2 Hanja character in suggestion , then click new
         Original.SetText    sHangul1
         Sleep 1
         Suggestion1.SetText sHanja1a
         Sleep 1
         Suggestion2.SetText sHanja1b
         Sleep 1
         NewEntry.Click
         Sleep 1
        '/// Input another Hangul character in original
        '/// + and 2 Hanja character in suggestion , then click new
         Original.SetText    sHangul2
         Sleep 1
         Suggestion1.SetText sHanja2a
         Sleep 1
         Suggestion2.SetText sHanja2b
         Sleep 1
         NewEntry.Click
         Sleep 1

         if Original.GetItemCount <> 2 then
             Warnlog "#i39936# There should be only 2 items , but get " & Original.GetItemCount
             Goto NoTest
         end if

        '/// Select 1st Hangul character in original ,
        '/// + check if 2 Hanja character are in suggestion
         Original.Select sHangul1
         Sleep 1
         if Suggestion1.GetText <> sHanja1a then
             Warnlog "Don't get " & sHanja1a
         end if
         if Suggestion2.GetText <> sHanja1b then
             Warnlog "Don't get " & sHanja1b
         end if

        '/// Input 2nd Hangul character in original ,
        '/// + check if 2 Hanja character are in suggestion
         Original.Select sHangul2
         Sleep 1
         if Suggestion1.GetText <> sHanja2a then
             Warnlog "Don't get " & sHanja2a
         end if
         if Suggestion2.GetText <> sHanja2b then
             Warnlog "Don't get " & sHanja2b
         end if
NoTest:
       HangulHanjaEditDictionary.Close

     Kontext "HangulHanjaOptions"
      'remove the dictionary you just insert .
       DeleteDictionary.Click
       Sleep 1
     HangulHanjaOptions.Cancel

   Kontext "HangulHanjaConversion"
   HangulHanjaConversion.Close

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHOptions_8
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile           as String
  Dim  sDictionaryName     as String
  Dim  sHangul1 , sHanja1  as String
  Dim  sHangul2 , sHanja2  as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\hangul.sxw")
  sDictionaryName = "myDictionary"
  sHangul1 = "에" : sHanja1 = "里"
  sHangul2 = "를" : sHanja2 = "力"

  printlog "- Options - Edit (Original) -3"
  '/// <b> Options - Edit (Original)-3 </b>
  '/// <b> Test delele in Edit custom Dictionary dialog </b>

  Call hNewDocument

  '/// Open a test file , which includes some
  '/// + Korean chacters and chinese character
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<MOD1 Home>"

  '/// Tools / HangulHanjaConversion , Press Options button ,
  '/// + then press new button , insert 1 new dictionary
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"

     Options.Click
     Kontext "HangulHanjaOptions"
       NewDictionary.Click
       Kontext "HangulHanjaNewDictionary"
       DictionaryName.SetText sDictionaryName
       HangulHanjaNewDictionary.OK

    '/// Press Edit button
     Kontext "HangulHanjaOptions"
       EditDictionary.Click
       Kontext "HangulHanjaEditDictionary"

        '/// Insert 2 entries
         Original.SetText sHangul1
         Sleep 1
         Suggestion1.SetText sHanja1
         Sleep 1
         NewEntry.Click
         Sleep 1
         Original.SetText sHangul2
         Sleep 1
         Suggestion1.SetText sHanja2
         Sleep 1
         NewEntry.Click

        '/// Select 2nd Hangul character in original
         Original.Select sHangul2
         Sleep 1

        '/// Press Delete button
         DeleteEntry.Click
         Sleep 1

        '/// Check if 2nd entry is really deleted
         if Original.GetItemCount <> 1 then
             Warnlog "There should be only 1 item , but get " & Original.GetItemCount
         end if

         Original.SetText sHangul1
         Sleep 1
         if Suggestion1.GetText <> sHanja1 then
             Warnlog "Don't get " & sHanja1
         end if

       HangulHanjaEditDictionary.Close

     Kontext "HangulHanjaOptions"
      'remove the dictionary you just insert .
       DeleteDictionary.Click
       Sleep 1
     HangulHanjaOptions.Cancel

   Kontext "HangulHanjaConversion"
   HangulHanjaConversion.Close

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHOptions_9
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile                     as String
  Dim  sDictionaryName               as String
  Dim  sHangul , sHanja1 , sHanja2   as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\hangul.sxw")
  sDictionaryName = "myDictionary"
  sHangul = "에" : sHanja1 = "里" : sHanja2 = "力"

  printlog "- Options - Edit (Original) -4"
  '/// <b> Options - Edit (Original)-4 </b>
  '/// <b> Add a suggestion </b>

  Call hNewDocument

  '/// Open a test file , which includes some
  '/// + Korean chacters and chinese character
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<MOD1 Home>"

  '/// Tools / HangulHanjaConversion , Press Options button ,
  '/// + then press new button , insert 1 new dictionary
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"

     Options.Click
     Kontext "HangulHanjaOptions"
       NewDictionary.Click
       Kontext "HangulHanjaNewDictionary"
       DictionaryName.SetText sDictionaryName
       HangulHanjaNewDictionary.OK

    '/// Press Edit button
     Kontext "HangulHanjaOptions"
       EditDictionary.Click
       Kontext "HangulHanjaEditDictionary"

        '/// Insert 1 original and 1 suggestion
         Original.SetText sHangul
         Sleep 1
         Suggestion1.SetText sHanja1
         Sleep 1
         NewEntry.Click
         Sleep 1

        '/// Input 1st Hangul character in original
         Original.SetText sHangul
         Sleep 1

        '/// Add a new suggestion , and press NEW again
         Suggestion2.SetText sHanja2
         Sleep 1
         NewEntry.Click
         Sleep 1

        '/// Check if 2nd entry is really added
         Original.SetText sHangul
         Sleep 1
         if Suggestion1.GetText <> sHanja1 then
             Warnlog "Don't get " & sHanja1
         end if
         if Suggestion2.GetText <> sHanja2 then
             Warnlog "Don't get " & sHanja2
         end if

       HangulHanjaEditDictionary.Close

     Kontext "HangulHanjaOptions"
      'remove the dictionary you just insert .
       DeleteDictionary.Click
       Sleep 1
     HangulHanjaOptions.Cancel

   Kontext "HangulHanjaConversion"
   HangulHanjaConversion.Close

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHOptions_10
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile                     as String
  Dim  sDictionaryName               as String
  Dim  sHangul , sHanja1 , sHanja2   as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\hangul.sxw")
  sDictionaryName = "myDictionary"
  sHangul = "에" : sHanja1 = "里" : sHanja2 = "力"

  printlog "- Options - Edit (Original) -5"
  '/// <b> Options - Edit (Original)-5 </b>
  '/// <b> Delete a suggestion </b>

  Call hNewDocument

  '/// Open a test file , which includes some
  '/// + Korean chacters and chinese character
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<MOD1 Home>"

  '/// Tools / HangulHanjaConversion , Press Options button ,
  '/// + then press new button , insert 1 new dictionary
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"

     Options.Click
     Kontext "HangulHanjaOptions"
       NewDictionary.Click
       Kontext "HangulHanjaNewDictionary"
       DictionaryName.SetText sDictionaryName
       HangulHanjaNewDictionary.OK

    '/// Press Edit button
     Kontext "HangulHanjaOptions"
       EditDictionary.Click
       Kontext "HangulHanjaEditDictionary"

        '/// Insert 1 original and 2 suggestions , click NEW
         Original.SetText sHangul
         Sleep 1
         Suggestion1.SetText sHanja1
         Sleep 1
         Suggestion2.SetText sHanja2
         Sleep 1
         NewEntry.Click
         Sleep 1

        '/// Input 1st Hangul character in original
         Original.SetText sHangul
         Sleep 1

        '/// Delete 2nd suggestion , and press NEW again
         Suggestion2.SetText ""
         Sleep 1
         NewEntry.Click
         Sleep 1

        '/// Check if 2nd suggestion is really deleted
         Original.SetText sHangul
         Sleep 1
         if Suggestion1.GetText <> sHanja1 then
             Warnlog "Don't get " & sHanja1
         end if
         if Suggestion2.GetText <> "" then
             Warnlog "#i39319# Something wrong in 2nd suggestion , get " & Suggestion2.GetText
         end if

       HangulHanjaEditDictionary.Close

     Kontext "HangulHanjaOptions"
      'remove the dictionary you just insert .
       DeleteDictionary.Click
       Sleep 1
     HangulHanjaOptions.Cancel

   Kontext "HangulHanjaConversion"
   HangulHanjaConversion.Close

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHOptions_11
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile                     as String
  Dim  sDictionaryName               as String
  Dim  sHangul , sHanja1 , sHanja2   as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\hangul.sxw")
  sDictionaryName = "myDictionary"
  sHangul = "에" : sHanja1 = "里" : sHanja2 = "力"

  printlog "- Options - Edit (Original) -6"
  '/// <b> Options - Edit (Original)-6 </b>
  '/// <b> Update a suggestion </b>

  Call hNewDocument

  '/// Open a test file , which includes some
  '/// + Korean chacters and chinese character
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<MOD1 Home>"

  '/// Tools / HangulHanjaConversion , Press Options button ,
  '/// + then press new button , insert 1 new dictionary
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"

     Options.Click
     Kontext "HangulHanjaOptions"
       NewDictionary.Click
       Kontext "HangulHanjaNewDictionary"
       DictionaryName.SetText sDictionaryName
       HangulHanjaNewDictionary.OK

    '/// Press Edit button
     Kontext "HangulHanjaOptions"
       EditDictionary.Click
       Kontext "HangulHanjaEditDictionary"

        '/// Insert 1 original and 1 suggestion , click NEW
         Original.SetText sHangul
         Sleep 1
         Suggestion1.SetText sHanja1
         Sleep 1
         NewEntry.Click
         Sleep 1

        '/// Input 1st Hangul character in original
         Original.SetText sHangul
         Sleep 1

        '/// Update 1st suggestion , and press NEW again
         Suggestion1.SetText sHanja2
         Sleep 1
         NewEntry.Click
         Sleep 1

        '/// Check if 1st suggestion is really updated
         Original.SetText sHangul
         Sleep 1
         if Suggestion1.GetText <> sHanja2 then
             Warnlog "#i39319# Don't get " & sHanja2 & " but get " & Suggestion1.GetText
         end if

       HangulHanjaEditDictionary.Close

     Kontext "HangulHanjaOptions"
      'remove the dictionary you just insert .
       DeleteDictionary.Click
       Sleep 1
     HangulHanjaOptions.Cancel

   Kontext "HangulHanjaConversion"
   HangulHanjaConversion.Close

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

testcase tHHOptions_12
    warnlog( "#i111911# - Test is broken beyond repait, please rewrite" )
    goto endsub


  Dim  sTestFile             as String
  Dim  sDictionaryName       as String
  Dim  sHangul , sHanja      as String

  sTestFile = Convertpath (gTesttoolpath + "writer\optional\input\hangulhanjaonversion\hangul.sxw")
  sDictionaryName = "myDictionary"
  sHangul = "에" : sHanja = "里力"

  printlog "- Options - Edit (Original) -7"
  '/// <b> Options - Edit (Original)-7 </b>
  '/// <b> Original length isn't same as suggestion length </b>

  Call hNewDocument

  '/// Open a test file , which includes some
  '/// + Korean chacters and chinese character
   hFileOpenLocally( sTestFile )
   Call wTypeKeys "<MOD1 Home>"

  '/// Tools / HangulHanjaConversion , Press Options button ,
  '/// + then press new button , insert 1 new dictionary
   ToolsLanguageHangulHanjaConversion
   Kontext "HangulHanjaConversion"

     Options.Click
     Kontext "HangulHanjaOptions"
       NewDictionary.Click
       Kontext "HangulHanjaNewDictionary"
       DictionaryName.SetText sDictionaryName
       HangulHanjaNewDictionary.OK

    '/// Press Edit button
     Kontext "HangulHanjaOptions"
       EditDictionary.Click
       Kontext "HangulHanjaEditDictionary"

        '/// Insert 1 original and 1 suggestion which is 2
        '/// + hanja character , click NEW
         Original.SetText sHangul
         Sleep 1
         Suggestion1.SetText sHanja
         Sleep 1
         NewEntry.Click
         Sleep 1

        '/// Check if a warning message appears
         Kontext "Active"
         if Active.Exists then
             if Active.GetRT = 304 then
                 Active.Ok
             else
                Warnlog "The warning message box doesn't pop up ! "
             end if
         end if

       Kontext "HangulHanjaEditDictionary"
       HangulHanjaEditDictionary.Close

     Kontext "HangulHanjaOptions"
       EditDictionary.Click

       Kontext "HangulHanjaEditDictionary"

        '/// Check if the suggestion is added (should NOT)
        '/// Input 1st Hangul character in original
         Original.SetText sHangul
         Sleep 1

         if Suggestion1.GetText <> "" then
             Warnlog "Should get nothing , but get " & Suggestion1.GetText
         end if

       HangulHanjaEditDictionary.Close

     Kontext "HangulHanjaOptions"
      'remove the dictionary you just insert .
       DeleteDictionary.Click
       Sleep 1
     HangulHanjaOptions.Cancel

   Kontext "HangulHanjaConversion"
   HangulHanjaConversion.Close

  Call hCloseDocument

endcase

'-----------------------------------------------------------------

