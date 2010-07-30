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
'* short description : general option test (test for language-group)
'*
'\******************************************************************************

sub opt_lan1
  Dim bOld as Boolean

   bOld = ActiveDeactivateAsianSupport ( FALSE )

   Call tLanguageSettingsLanguages
   Call tLanguageSettingsWritingAids

   printlog "++ activate the asian support in StarOffice ++"
   ActiveDeactivateAsianSupport ( TRUE )
   Call tLanguageSettingsJapaneseFind
   Call tLanguageSettingsAsianLayout

   printlog "++ reset asian support in StarOffice to default ++"
   if bOld = TRUE then
      ActiveDeactivateAsianSupport ( TRUE )
   else
      ActiveDeactivateAsianSupport ( FALSE )
   end if

end sub

'*******************************************************************************

testcase tLanguageSettingsLanguages

   Dim lsSave ( 5 ) as String, lsSave2 (3) as String
   Dim bChanged as Boolean, bSave as Boolean, bSave2 as Boolean

   bChanged = FALSE



   hNewDocument
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "Languages" )

   printlog "- check dependences in this dialog"
   printlog "    - asian support"
   if Aktivieren.IsChecked() = FALSE then
      if Asiatisch.IsEnabled() = TRUE then
         Warnlog "Asian support is not active, the listbox is in wrong state:"
         printlog "...the language for asian is active => Perhaps #108220#"
         end if
   if Aktivieren.isEnabled then
       Aktivieren.Check
   else
       qaErrorLog("Asian locale setting is selected: '"+Gebietsschema.getSelText()+"'")
       printlog Aktivieren.isEnabled 
   endif
      if Asiatisch.IsEnabled() <> TRUE then
         Warnlog "Asian support is active, the listbox is in wrong state:"
         printlog "...language for asian is not active => BUG!"
         end if
   if Aktivieren.isEnabled then
       Aktivieren.unCheck
   else
       qaErrorLog("Asian locale setting is selected: '"+Gebietsschema.getSelText()+"'")
       printlog Aktivieren.isEnabled 
   endif
   else
      if Asiatisch.IsEnabled() <> TRUE then
         Warnlog "Asian support is not active, the listbox is in wrong state:"
         printlog "...language for asian is not active => BUG!"
         end if
   if Aktivieren.isEnabled then
       Aktivieren.unCheck
   else
       qaErrorLog("Asian locale setting is selected: '"+Gebietsschema.getSelText()+"'")
       printlog Aktivieren.isEnabled 
   endif
      if Asiatisch.IsEnabled() = TRUE then
         Warnlog "Asian support is active, the listbox is in wrong state:"
         printlog "...language for asian is active => BUG!"
         end if
   if Aktivieren.isEnabled then
       Aktivieren.Check
   else
       qaErrorLog("Asian locale setting is selected: '"+Gebietsschema.getSelText()+"'")
       printlog Aktivieren.isEnabled 
   endif
   end if

   printlog "    - CTL support"
   if ComplexScriptEnabled.IsChecked() = FALSE then
      if LanguageComplexScript.IsEnabled() = TRUE then
         Warnlog "CTL support is not active, the listbox is in wrong state:"
         printlog "...language for CTL is active => BUG!"
      end if
      ComplexScriptEnabled.Check()
      if LanguageComplexScript.IsEnabled() <> TRUE then
         Warnlog "CTL support is active, the listbox is in wrong state:"
         printlog "...language for CTL is not active => BUG!"
      end if
      ComplexScriptEnabled.UnCheck()
   else
      if LanguageComplexScript.IsEnabled() <> TRUE then
         Warnlog "CTL support is active, the listbox is in wrong state:"
         printlog "...language for CTL is not active => BUG!"
      end if
      ComplexScriptEnabled.UnCheck()
      if LanguageComplexScript.IsEnabled() = TRUE then
         Warnlog "CTL support is not active, the listbox is in wrong state:"
         printlog "...the language for CTL is active => BUG!"
      end if
      ComplexScriptEnabled.Check()
   end if

   printlog "- save raw data"
   lsSave (1) = Gebietsschema.GetSelText()
   lsSave (2) = StandardWaehrung.GetSelText()
   lsSave (3) = Westlich.GetSelText()
   bSave = Aktivieren.IsChecked()
   if Aktivieren.isEnabled then
       Aktivieren.Check
   else
       qaErrorLog("Asian locale setting is selected: '"+Gebietsschema.getSelText()+"'")
       printlog Aktivieren.isEnabled 
   endif
   lsSave (4) = Asiatisch.GetSelText()

   if lsSave (4) = "" then
      Warnlog "Empty entry is selected for 'Asian'! => Bug 102046"
      endif

   bSave2 = ComplexScriptEnabled.IsChecked()
   ComplexScriptEnabled.Check()
   lsSave (5) = LanguageComplexScript.GetSelText()

   if lsSave (5) = "" then
      Warnlog "Empty entry is selected for 'Complex script'! => Bug 102046"
      endif

   printlog "- change settings"
   select case iSprache
      case 34   : Gebietsschema.Select( 40 )
      case 39   : Gebietsschema.Select( 17 )
      case 81   : Gebietsschema.Select( 22 )
      case 88   : Gebietsschema.Select( 19 )
      case else : Gebietsschema.Select( 15 )
   end select
   StandardWaehrung.Select( 20 )
   Westlich.Select( 10 )
   lsSave2(1) = Westlich.GetSelText()

   if Aktivieren.isEnabled then
       Aktivieren.Check
   else
       qaErrorLog("Asian locale setting is selected: '"+Gebietsschema.getSelText()+"'")
       printlog Aktivieren.isEnabled 
   endif
   Asiatisch.Select( 5 )
   lsSave2(2) = Asiatisch.GetSelText()
   AktuellesDokument.Uncheck()

   ComplexScriptEnabled.Check
   LanguageComplexScript.Select( 7 )
   lsSave2 (3) = LanguageComplexScript.GetSelText()

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK()
   Sleep ( 3 )

   hCloseDocument

   printlog "- exit/restart StarOffice"
   ExitRestartTheOffice
   hNewDocument
   printlog "- check changes"
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "Languages" )

   select case iSprache
      case 34   : if Gebietsschema.GetSelIndex <> 40 then
                  Warnlog "locale settings => changes not saved!"
                     endif
      case 39   : if Gebietsschema.GetSelIndex <> 17 then
                  Warnlog "locale settings => changes not saved!"
                     endif
      case 81   : if Gebietsschema.GetSelIndex <> 22 then
                  Warnlog "locale settings => changes not saved!"
                     endif
      case 88   : if Gebietsschema.GetSelIndex <> 19 then
                  Warnlog "locale settings => changes not saved!"
                     endif
      case else : if Gebietsschema.GetSelIndex <> 15 then
                  Warnlog "locale settings => changes not saved!"
                     endif
   end select

   if StandardWaehrung.GetSelIndex <> 20 then
      Warnlog "default currency => changes not saved!"
      endif

   if Westlich.GetSelText <> lsSave2(1) then
      Warnlog "western => changes not saved! "
      endif

   if Aktivieren.IsChecked <> TRUE then
      Warnlog "Enabled asian support => changes not saved!"
      endif

   if Asiatisch.GetSelText <> lsSave2(2) then
      Warnlog  "asian => changes not saved! "
      endif

   if ComplexScriptEnabled.IsChecked <> TRUE then
      Warnlog "CTL support => changes not saved!"
      endif

   if LanguageComplexScript.GetSelText <> lsSave2 (3) then
      Warnlog  "CTL language => changes not saved! "
      endif

   printlog "- set to default"
   Gebietsschema.Select lsSave(1)
   StandardWaehrung.Select lsSave (2)
   Westlich.Select lsSave (3)
   AktuellesDokument.Uncheck()
   if Aktivieren.isEnabled then
       Aktivieren.Check
   else
       qaErrorLog("Asian locale setting is selected: '"+Gebietsschema.getSelText()+"'")
       printlog Aktivieren.isEnabled 
   endif

   if lsSave (4) <> "" then
      Asiatisch.Select lsSave (4)
   else
      Asiatisch.Select 1
   end if

   if Aktivieren.isEnabled then
   if bSave = TRUE then 
       Aktivieren.Check
   else 
       Aktivieren.Uncheck
   endif
   else
       qaErrorLog("Asian locale setting is selected: '"+Gebietsschema.getSelText()+"'")
       printlog Aktivieren.isEnabled 
   endif
   ComplexScriptEnabled.Check()

   if lsSave (5) <> "" then
      LanguageComplexScript.Select( lsSave(5) )
   else
      LanguageComplexScript.Select( 1 )
   end if

   if bSave2 = TRUE then
      ComplexScriptEnabled.Check()
   else
      ComplexScriptEnabled.Uncheck()
   endif

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK()
   Sleep (3)

   printlog "- check the data"
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "Languages" )

   if Gebietsschema.GetSelText() <> lsSave(1) then
      Warnlog "locale settings => changes not saved! => BugID : 87977"
      endif

   if StandardWaehrung.GetSelText() <> lsSave(2) then
      Warnlog "default currency => changes not saved!"
      endif

   if Westlich.GetSelText() <> lsSave (3) then
      Warnlog  "western => changes not saved!"
      endif

   if Aktivieren.IsChecked() <> bSave then
      Warnlog "Enabled asian support => changes not saved!"
      endif

   if Aktivieren.isEnabled then
       Aktivieren.Check
   else
       qaErrorLog("Asian locale setting is selected: '"+Gebietsschema.getSelText()+"'")
       printlog Aktivieren.isEnabled 
   endif
   if lsSave (4) <> "" then
      if Asiatisch.GetSelText() <> lsSave (4) then
         Warnlog "asian => changes not saved!"
      endif
   end if

   if ComplexScriptEnabled.IsChecked() <> bSave2 then
      Warnlog "CTL support => changes not saved!"
      endif

   ComplexScriptEnabled.Check()

   if lsSave (5) <> "" then
      if LanguageComplexScript.GetSelText() <> lsSave (5) then
      Warnlog  "CTL language => changes not saved! "
      endif
   end if
   if Aktivieren.isEnabled then
   if bSave = TRUE then 
       Aktivieren.Check
   else 
       Aktivieren.Uncheck
   endif
   else
       qaErrorLog("Asian locale setting is selected: '"+Gebietsschema.getSelText()+"'")
       printlog Aktivieren.isEnabled 
   endif
   if bSave2 = TRUE then
      ComplexScriptEnabled.Check()
   else
      ComplexScriptEnabled.Uncheck()
   endif

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK()
   Sleep (3)
   hclosedocument
endcase

' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
testcase tLanguageSettingsWritingAids

   Dim lbSave1 ( 5 ) as Boolean
   Dim lbSave2 ( 5 ) as Boolean
   Dim lbSave3 ( 15 ) as Boolean
   dim iCounter as Integer
   Dim i as Integer, iCounter1 as Integer, iCounter2 as Integer, iCounter3 as Integer



   hNewDocument
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "WritingAids" )

   printlog " - save raw data / and inverted settings"
   iCounter1 = Sprachmodule.GetItemCount
   Sprachmodule.TypeKeys "<PageUp>", 6

   for i=1 to iCounter1
      lbSave1 (i) = Sprachmodule.IsChecked
      if lbSave1 (i) = TRUE then Sprachmodule.Uncheck else Sprachmodule.Check
      Sprachmodule.TypeKeys "<Down>"
   next i

   iCounter2 = Benutzerwoerterbuch.GetItemCount
   Benutzerwoerterbuch.TypeKeys "<PageUp>", 6

   for i=1 to iCounter2 - 1
      lbSave2 (i) = Benutzerwoerterbuch.IsChecked
      if lbSave2 (i) = TRUE then Benutzerwoerterbuch.Uncheck else Benutzerwoerterbuch.Check
      Benutzerwoerterbuch.TypeKeys "<Down>"
   next i

   if Benutzerwoerterbuch.IsChecked <> TRUE then
      Warnlog "Last entry in 'User-defined dictionaries' should be checked ( internal Ignore-All List ), but it isn't => BUG?!"
      endif

   iCounter3 = Optionen.GetItemCount
   Optionen.TypeKeys "<PageUp>", 6
   for i=1 to iCounter3
      if i<>9 AND i<>10 AND i<>11 then
         lbSave3 (i) = Optionen.IsChecked
         if lbSave3 (i) = TRUE then Optionen.Uncheck else Optionen.Check
      end if
      Optionen.TypeKeys "<Down>"
   next i

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)
   hCloseDocument

   printlog " - exit/restart StarOffice"
   ExitRestartTheOffice

   printlog " - check changes"

   hNewDocument

   ToolsOptions
   hToolsOptions ( "LanguageSettings", "WritingAids" )
   printlog "    'available language modules'"
   Sprachmodule.TypeKeys "<PageUp>", 6

   for i=1 to iCounter1
      if Sprachmodule.Ischecked =  lbSave1 (i) then
          Warnlog "entry no. " + i + " => changes not saved!  perhaps #i15841!"
      end if
      Sprachmodule.TypeKeys "<Down>"
   next i

   printlog "    'user-definded dictonaries'"
   Benutzerwoerterbuch.TypeKeys "<PageUp>", 6

   for i=1 to iCounter2 - 1
      if Benutzerwoerterbuch.Ischecked = lbSave2 (i) then Warnlog "entry no. " + i + " => changes not saved!"
      Benutzerwoerterbuch.TypeKeys "<Down>"
   next i

   if Benutzerwoerterbuch.IsChecked <> TRUE then
      Warnlog "Last entry in 'User-defined dictionaries' should be checked ( internal Ignore-All List ), but it isn't => BUG?!"
      endif

   printlog "    'options'"
   Optionen.TypeKeys "<PageUp>", 6
   for i=1 to iCounter3
      if i<>9 AND i<>10 AND i<>11 then
         if Optionen.IsChecked = lbSave3 (i) then Warnlog "entry no. " + i + " => changes not saved!"
         end if
      Optionen.TypeKeys "<Down>"
   next i

   printlog " - make second changes"
   Optionen.TypeKeys "<PageUp>", 6

   for i=1 to iCounter3
      if i=1 then Optionen.Uncheck
      if i=3 then Optionen.Check
      if i=5 then Optionen.Uncheck
      if i=6 then Optionen.Check
      if i=7 then Optionen.Uncheck
      if i=12 then Optionen.Check
      Optionen.TypeKeys "<Down>"
   next i

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

   hCloseDocument

   printlog " - check second changes"
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "WritingAids" )
   Optionen.TypeKeys "<PageUp>", 6

   for i=1 to iCounter
      select case i
         case 1  : if Optionen.IsChecked <> FALSE then
                      Warnlog "entry no. 1  => changes not saved!"
                      endif
         case 3  : if Optionen.IsChecked <> TRUE  then
                      Warnlog "entry no. 3  => changes not saved!"
                      endif
         case 5  : if Optionen.IsChecked <> FALSE then
                      Warnlog "entry no. 5  => changes not saved!"
                      endif
         case 6  : if Optionen.IsChecked <> TRUE  then
                      Warnlog "entry no. 6  => changes not saved!"
                      endif
         case 7  : if Optionen.IsChecked <> FALSE then
                      printlog "entry no. 7  => not saved! => it is only for documents"
                      endif
         case 12 : if Optionen.IsChecked <> TRUE  then
                      Warnlog "entry no. 12  => changes not saved!"
                      endif
      end select
      Optionen.TypeKeys "<Down>"
   next i

   printlog " - reset settings"
   Sprachmodule.TypeKeys "<PageUp>", 6

   for i=1 to iCounter1
      if lbSave1 (i) = TRUE then Sprachmodule.Check else Sprachmodule.UnCheck
      Sprachmodule.TypeKeys "<Down>"
   next i

   Benutzerwoerterbuch.TypeKeys "<PageUp>", 6

   for i=1 to iCounter2 - 1
      if lbSave2 (i) = TRUE then Benutzerwoerterbuch.Check else Benutzerwoerterbuch.UnCheck
      Benutzerwoerterbuch.TypeKeys "<Down>"
   next i

   Optionen.TypeKeys "<PageUp>", 6

   for i=1 to iCounter3
      if i<>9 AND i<>10 AND i<>11 then
         if lbSave3 (i) = TRUE then Optionen.Check else Optionen.UnCheck
      end if
      Optionen.TypeKeys "<Down>"
   next i

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

   printlog " - check raw data"
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "WritingAids" )

   printlog "    'available language modules'"
   Sprachmodule.TypeKeys "<PageUp>", 6

   for i=1 to iCounter1
      if Sprachmodule.Ischecked <> lbSave1 (i) then Warnlog "entry no. " + i + " => changes not saved!"
      Sprachmodule.TypeKeys "<Down>"
   next i

   printlog "    'user-definded dictonaries'"
   Benutzerwoerterbuch.TypeKeys "<PageUp>", 6
   for i=1 to iCounter2 - 1
      if Benutzerwoerterbuch.Ischecked <> lbSave2 (i) then Warnlog "entry no. " + i + " => changes not saved!"
      Benutzerwoerterbuch.TypeKeys "<Down>"
   next i

   printlog "    'options'"
   Optionen.TypeKeys "<PageUp>", 6
   for i=1 to iCounter3
      if i<>9 AND i<>10 AND i<>11 then
         if Optionen.IsChecked <> lbSave3 (i) then Warnlog "entry no. " + i + " => changes not saved!"
      end if
      Optionen.TypeKeys "<Down>"
   next i

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)
endcase

' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
testcase tLanguageSettingsJapaneseFind
 Dim bSave ( 20 ) as Boolean


   hNewDocument
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "SearchingInJapanese" )

 printlog " - save data"
   bSave (1 ) = MatchCase.IsChecked
   bSave (2 ) = MatchFullHalf.IsChecked
   bSave (3 ) = MatchHiraganaKatakana.IsChecked
   bSave (4 ) = MatchContractions.IsChecked
   bSave (5 ) = MatchMinusDashChoOn.IsChecked
   bSave (6 ) = MatchRepeatcharacterMarks.IsChecked
   bSave (7 ) = MatchVariantFormKanji.IsChecked
   bSave (8 ) = MatchOldKanaForms.IsChecked
   bSave (9 ) = MatchDiZiDuZu.IsChecked
   bSave (10) = MatchBaVaHaFa.IsChecked
   bSave (11) = MatchTsiThiChiDhiZi.IsChecked
   bSave (12) = MatchHyuFyuByuVyu.IsChecked
   bSave (13) = MatchDeSheZeJe.IsChecked
   bSave (14) = MatchIaIya.IsChecked
   bSave (15) = MatchKiKu.IsChecked
   bSave (16) = PunctuationCharacters.IsChecked
   bSave (17) = WhitespaceCharacters.IsChecked
   bSave (18) = ProlongedSoundmarks.IsChecked
   bSave (19) = MiddleDots.IsChecked

 printlog " - invert data"
   if bSave (1 ) = TRUE then MatchCase.Uncheck                 else MatchCase.Check
   if bSave (2 ) = TRUE then MatchFullHalf.Uncheck             else MatchFullHalf.Check
   if bSave (3 ) = TRUE then MatchHiraganaKatakana.Uncheck     else MatchHiraganaKatakana.Check
   if bSave (4 ) = TRUE then MatchContractions.Uncheck         else MatchContractions.Check
   if bSave (5 ) = TRUE then MatchMinusDashChoOn.Uncheck       else MatchMinusDashChoOn.Check
   if bSave (6 ) = TRUE then MatchRepeatcharacterMarks.Uncheck else MatchRepeatcharacterMarks.Check
   if bSave (7 ) = TRUE then MatchVariantFormKanji.Uncheck     else MatchVariantFormKanji.Check
   if bSave (8 ) = TRUE then MatchOldKanaForms.Uncheck         else MatchOldKanaForms.Check
   if bSave (9 ) = TRUE then MatchDiZiDuZu.Uncheck             else MatchDiZiDuZu.Check
   if bSave (10) = TRUE then MatchBaVaHaFa.Uncheck             else MatchBaVaHaFa.Check
   if bSave (11) = TRUE then MatchTsiThiChiDhiZi.Uncheck       else MatchTsiThiChiDhiZi.Check
   if bSave (12) = TRUE then MatchHyuFyuByuVyu.Uncheck         else MatchHyuFyuByuVyu.Check
   if bSave (13) = TRUE then MatchDeSheZeJe.Uncheck            else MatchDeSheZeJe.Check
   if bSave (14) = TRUE then MatchIaIya.Uncheck                else MatchIaIya.Check
   if bSave (15) = TRUE then MatchKiKu.Uncheck                 else MatchKiKu.Check
   if bSave (16) = TRUE then PunctuationCharacters.Uncheck     else PunctuationCharacters.Check
   if bSave (17) = TRUE then WhitespaceCharacters.Uncheck      else WhitespaceCharacters.Check
   if bSave (18) = TRUE then ProlongedSoundmarks.Uncheck       else ProlongedSoundmarks.Check
   if bSave (19) = TRUE then MiddleDots.Uncheck                else MiddleDots.Check

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)
   hCloseDocument

 printlog " - exit/restart StarOffice"
   ExitRestartTheOffice

 printlog " - check changes"
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "SearchingInJapanese" )

   if MatchCase.IsChecked                 = bSave (1 ) then Warnlog "Match Case => changes not saved!"
   if MatchFullHalf.IsChecked             = bSave (2 ) then Warnlog "Match Full Half => changes not saved!"
   if MatchHiraganaKatakana.IsChecked     = bSave (3 ) then Warnlog "Match Hiragana Katakana => changes not saved!"
   if MatchContractions.IsChecked         = bSave (4 ) then Warnlog "Match Contractions => changes not saved!"
   if MatchMinusDashChoOn.IsChecked       = bSave (5 ) then Warnlog "Match Minus Dash Cho On => changes not saved!"
   if MatchRepeatcharacterMarks.IsChecked = bSave (6 ) then Warnlog "Match Repeatcharacter Marks => changes not saved!"
   if MatchVariantFormKanji.IsChecked     = bSave (7 ) then Warnlog "Match Variant Form Kanji => changes not saved!"
   if MatchOldKanaForms.IsChecked         = bSave (8 ) then Warnlog "Match Old Kana Forms => changes not saved!"
   if MatchDiZiDuZu.IsChecked             = bSave (9 ) then Warnlog "Match Di Zi Du Zu => changes not saved!"
   if MatchBaVaHaFa.IsChecked             = bSave (10) then Warnlog "Match Ba Va Ha Fa => changes not saved!"
   if MatchTsiThiChiDhiZi.IsChecked       = bSave (11) then Warnlog "Match Tsi Thi Chi Dhi Zi => changes not saved!"
   if MatchHyuFyuByuVyu.IsChecked         = bSave (12) then Warnlog "Match Hyu Iyu Byu Vyu => changes not saved!"
   if MatchDeSheZeJe.IsChecked            = bSave (13) then Warnlog "Match De She Ze Je => changes not saved!"
   if MatchIaIya.IsChecked                = bSave (14) then Warnlog "Match Ia Iya => changes not saved!"
   if MatchKiKu.IsChecked                 = bSave (15) then Warnlog "Match Ki Ku => changes not saved!"
   if PunctuationCharacters.IsChecked     = bSave (16) then Warnlog "Punctuation Characters => changes not saved!"
   if WhitespaceCharacters.IsChecked      = bSave (17) then Warnlog "Whitespace Characters => changes not saved!"
   if ProlongedSoundmarks.IsChecked       = bSave (18) then Warnlog "Prolonged Soundmarks => changes not saved!"
   if MiddleDots.IsChecked                = bSave (19) then Warnlog "Middle Dots => changes not saved!"

 printlog " - second changes"
   MatchCase.Check
   MatchFullHalf.Uncheck
   MatchHiraganaKatakana.Check
   MatchContractions.Check
   MatchMinusDashChoOn.Uncheck
   MatchRepeatcharacterMarks.Uncheck
   MatchVariantFormKanji.Check
   MatchOldKanaForms.Check
   MatchDiZiDuZu.Uncheck
   MatchBaVaHaFa.Check
   MatchTsiThiChiDhiZi.Uncheck
   MatchHyuFyuByuVyu.Check
   MatchDeSheZeJe.Uncheck
   MatchIaIya.Check
   MatchKiKu.Uncheck
   PunctuationCharacters.Uncheck
   WhitespaceCharacters.Uncheck
   ProlongedSoundmarks.Uncheck
   MiddleDots.Check

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - check changes"
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "SearchingInJapanese" )

   if MatchCase.IsChecked                 <> TRUE  then Warnlog "Match Case => changes not saved!"
   if MatchFullHalf.IsChecked             <> FALSE then Warnlog "Match Full Half => changes not saved!"
   if MatchHiraganaKatakana.IsChecked     <> TRUE  then Warnlog "Match Hiragana Katakana => changes not saved!"
   if MatchContractions.IsChecked         <> TRUE  then Warnlog "Match Contractions => changes not saved!"
   if MatchMinusDashChoOn.IsChecked       <> FALSE then Warnlog "Match Minus Dash Cho On => changes not saved!"
   if MatchRepeatcharacterMarks.IsChecked <> FALSE then Warnlog "Match Repeatcharacter Marks => changes not saved!"
   if MatchVariantFormKanji.IsChecked     <> TRUE  then Warnlog "Match Variant Form Kanji => changes not saved!"
   if MatchOldKanaForms.IsChecked         <> TRUE  then Warnlog "Match Old Kana Forms => changes not saved!"
   if MatchDiZiDuZu.IsChecked             <> FALSE then Warnlog "Match Di Zi Du Zu => changes not saved!"
   if MatchBaVaHaFa.IsChecked             <> TRUE  then Warnlog "Match Ba Va Ha Fa => changes not saved!"
   if MatchTsiThiChiDhiZi.IsChecked       <> FALSE then Warnlog "Match Tsi Thi Chi Dhi Zi => changes not saved!"
   if MatchHyuFyuByuVyu.IsChecked         <> TRUE  then Warnlog "Match Hyu Iyu Byu Vyu => changes not saved!"
   if MatchDeSheZeJe.IsChecked            <> FALSE then Warnlog "Match De She Ze Je => changes not saved!"
   if MatchIaIya.IsChecked                <> TRUE  then Warnlog "Match Ia Iya => changes not saved!"
   if MatchKiKu.IsChecked                 <> FALSE then Warnlog "Match Ki Ku => changes not saved!"
   if PunctuationCharacters.IsChecked     <> FALSE then Warnlog "Punctuation Characters => changes not saved!"
   if WhitespaceCharacters.IsChecked      <> FALSE then Warnlog "Whitespace Characters => changes not saved!"
   if ProlongedSoundmarks.IsChecked       <> FALSE then Warnlog "Prolonged Soundmarks => changes not saved!"
   if MiddleDots.IsChecked                <> TRUE  then Warnlog "Middle Dots => changes not saved!"

 printlog " - reset to saved data"
   if bSave (1 ) = TRUE then MatchCase.Check                 else MatchCase.UnCheck
   if bSave (2 ) = TRUE then MatchFullHalf.Check             else MatchFullHalf.UnCheck
   if bSave (3 ) = TRUE then MatchHiraganaKatakana.Check     else MatchHiraganaKatakana.UnCheck
   if bSave (4 ) = TRUE then MatchContractions.Check         else MatchContractions.UnCheck
   if bSave (5 ) = TRUE then MatchMinusDashChoOn.Check       else MatchMinusDashChoOn.UnCheck
   if bSave (6 ) = TRUE then MatchRepeatcharacterMarks.Check else MatchRepeatcharacterMarks.UnCheck
   if bSave (7 ) = TRUE then MatchVariantFormKanji.Check     else MatchVariantFormKanji.UnCheck
   if bSave (8 ) = TRUE then MatchOldKanaForms.Check         else MatchOldKanaForms.UnCheck
   if bSave (9 ) = TRUE then MatchDiZiDuZu.Check             else MatchDiZiDuZu.UnCheck
   if bSave (10) = TRUE then MatchBaVaHaFa.Check             else MatchBaVaHaFa.UnCheck
   if bSave (11) = TRUE then MatchTsiThiChiDhiZi.Check       else MatchTsiThiChiDhiZi.UnCheck
   if bSave (12) = TRUE then MatchHyuFyuByuVyu.Check         else MatchHyuFyuByuVyu.UnCheck
   if bSave (13) = TRUE then MatchDeSheZeJe.Check            else MatchDeSheZeJe.UnCheck
   if bSave (14) = TRUE then MatchIaIya.Check                else MatchIaIya.UnCheck
   if bSave (15) = TRUE then MatchKiKu.Check                 else MatchKiKu.UnCheck
   if bSave (16) = TRUE then PunctuationCharacters.Check     else PunctuationCharacters.UnCheck
   if bSave (17) = TRUE then WhitespaceCharacters.Check      else WhitespaceCharacters.UnCheck
   if bSave (18) = TRUE then ProlongedSoundmarks.Check       else ProlongedSoundmarks.UnCheck
   if bSave (19) = TRUE then MiddleDots.Check                else MiddleDots.UnCheck

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - check saved data"
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "SearchingInJapanese" )

   if MatchCase.IsChecked                 <> bSave (1 ) then Warnlog "Match Case => changes not saved!"
   if MatchFullHalf.IsChecked             <> bSave (2 ) then Warnlog "Match Full Half => changes not saved!"
   if MatchHiraganaKatakana.IsChecked     <> bSave (3 ) then Warnlog "Match Hiragana Katakana => changes not saved!"
   if MatchContractions.IsChecked         <> bSave (4 ) then Warnlog "Match Contractions => changes not saved!"
   if MatchMinusDashChoOn.IsChecked       <> bSave (5 ) then Warnlog "Match Minus Dash Cho On => changes not saved!"
   if MatchRepeatcharacterMarks.IsChecked <> bSave (6 ) then Warnlog "Match Repeatcharacter Marks => changes not saved!"
   if MatchVariantFormKanji.IsChecked     <> bSave (7 ) then Warnlog "Match Variant Form Kanji => changes not saved!"
   if MatchOldKanaForms.IsChecked         <> bSave (8 ) then Warnlog "Match Old Kana Forms => changes not saved!"
   if MatchDiZiDuZu.IsChecked             <> bSave (9 ) then Warnlog "Match Di Zi Du Zu => changes not saved!"
   if MatchBaVaHaFa.IsChecked             <> bSave (10) then Warnlog "Match Ba Va Ha Fa => changes not saved!"
   if MatchTsiThiChiDhiZi.IsChecked       <> bSave (11) then Warnlog "Match Tsi Thi Chi Dhi Zi => changes not saved!"
   if MatchHyuFyuByuVyu.IsChecked         <> bSave (12) then Warnlog "Match Hyu Fyu Byu Vyu => changes not saved!"
   if MatchDeSheZeJe.IsChecked            <> bSave (13) then Warnlog "Match De She Ze Je => changes not saved!"
   if MatchIaIya.IsChecked                <> bSave (14) then Warnlog "Match Ia Iya => changes not saved!"
   if MatchKiKu.IsChecked                 <> bSave (15) then Warnlog "Match Ki Ku => changes not saved!"
   if PunctuationCharacters.IsChecked     <> bSave (16) then Warnlog "Punctuation Characters => changes not saved!"
   if WhitespaceCharacters.IsChecked      <> bSave (17) then Warnlog "Whitespace Characters => changes not saved!"
   if ProlongedSoundmarks.IsChecked       <> bSave (18) then Warnlog "Prolonged Soundmarks => changes not saved!"
   if MiddleDots.IsChecked                <> bSave (19) then Warnlog "Middle Dots => changes not saved!"

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

endcase

' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
' > * > * > * > * > * > * > * > * > * > * > * > * > * > * > * > *
testcase tLanguageSettingsAsianLayout
 Dim bSave ( 20 ) as Boolean
 Dim sSave ( 20 ) as String
 Dim i as Integer, iLan as Integer


   hNewDocument
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "AsianLayout" )

 printlog " - save raw data"
   bSave ( 1 ) = WesternCharactersOnly.IsChecked
   bSave ( 2 ) = WesternTextAndPunctuation.IsChecked
   bSave ( 3 ) = NoCompression.IsChecked
   bSave ( 4 ) = CompressPunctuationOnly.IsChecked
   bSave ( 5 ) = CompressPunctuationAndJapaneseKana.IsChecked

   iLan = Language.GetItemCount
   if iLan > 4 then
      Warnlog "There are more than 4 languages in the list. The test break here, because it can't work with more than 4 entries. Bug?!?"
      Kontext "ExtrasOptionenDlg"
      ExtrasOptionenDlg.OK
      hclosedocument
      goto endsub
   end if

   for i=1 to iLan
      Language.Select i
      sSave ( i ) = Language.GetSelText
      bSave ( 5+i ) = Standard.IsChecked
      Standard.Uncheck
      sSave ( 5+i  ) = NoStartOfLine.GetText
      sSave ( 10+i ) = NoEndOfLine.GetText
      if bSave ( 5+1 ) = TRUE then
         Standard.Check
      else
         Standard.Uncheck
      end if
   next i

 printlog " - change data"
   WesternTextAndPunctuation.Check
   CompressPunctuationOnly.Check
   for i=1 to Language.GetItemCount
      Language.Select i
      Standard.UnCheck
      select case i
        case 1 : NoStartOfLine.SetText "avc" : NoEndOfLine.SetText "vca"
        case 2 : NoStartOfLine.SetText ">" : NoEndOfLine.SetText "<"
        case 3 : NoStartOfLine.SetText "!d?$" : NoEndOfLine.SetText "!d?$"
        case 4 : NoStartOfLine.SetText "aS$%6" : NoEndOfLine.SetText "aS$$6"
      end select
   next i

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)
   hCloseDocument

 printlog " - exit/restart StarOffice"
   ExitRestartTheOffice

 printlog " - check changes"
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "AsianLayout" )

   if WesternTextAndPunctuation.IsChecked <> TRUE then Warnlog "Western Text And Punctuation => changes not saved!"
   if CompressPunctuationOnly.IsChecked <> TRUE then Warnlog "Compress Punctuation Only => changes not saved!"
   for i=1 to iLan
      Language.Select i
      if Language.GetSelText <> sSave (i) then Warnlog "The language-entries are changed!"
      if Standard.IsChecked then
         Warnlog sSave(i) + " :  Standard is not unchecked"
         Standard.Uncheck
      end if
      select case i
        case 1 : if NoStartOfLine.GetText <> "avc"   then Warnlog sSave (i) + " :  NoStartOfLine -> changes not saved"
                 if NoEndOfLine.GetText   <> "vca"   then Warnlog sSave (i) + " :  NoEndOfLine -> changes not saved"
        case 2 : if NoStartOfLine.GetText <> ">"     then Warnlog sSave (i) + " :  NoStartOfLine -> changes not saved"
                 if NoEndOfLine.GetText   <> "<"     then Warnlog sSave (i) + " :  NoEndOfLine -> changes not saved"
        case 3 : if NoStartOfLine.GetText <> "!d?$"  then Warnlog sSave (i) + " :  NoStartOfLine -> changes not saved"
                 if NoEndOfLine.GetText   <> "!d?$"  then Warnlog sSave (i) + " :  NoEndOfLine -> changes not saved"
        case 4 : if NoStartOfLine.GetText <> "aS$%6" then Warnlog sSave (i) + " :  NoStartOfLine -> changes not saved"
                 if NoEndOfLine.GetText   <> "aS$$6" then Warnlog sSave (i) + " :  NoEndOfLine -> changes not saved"
      end select
   next i

 printlog " - second changes"
   WesternCharactersOnly.Check
   CompressPunctuationAndJapaneseKana.Check

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - check changes"
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "AsianLayout" )

   if WesternCharactersOnly.IsChecked <> TRUE then Warnlog "Western Characters Only => changes not saved!"
   if CompressPunctuationAndJapaneseKana.IsChecked <> TRUE then Warnlog "Compress Punctuation And Japanese Kana => changes not saved!"

 printlog " - reset to saved data"
   if bSave ( 1 ) = TRUE then WesternCharactersOnly.Check
   if bSave ( 2 ) = TRUE then WesternTextAndPunctuation.Check
   if bSave ( 3 ) = TRUE then NoCompression.Check
   if bSave ( 4 ) = TRUE then CompressPunctuationOnly.Check
   if bSave ( 5 ) = TRUE then CompressPunctuationAndJapaneseKana.Check

   for i=1 to iLan
      Language.Select sSave(i)
      Standard.Uncheck
      NoStartOfLine.SetText sSave ( 5+i )
      NoEndOfLine.SetText sSave ( 10+i )
      if bSave ( 5+i ) = TRUE then Standard.Check else Standard.Uncheck
   next i

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - check reset"
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "AsianLayout" )

   if WesternCharactersOnly.IsChecked              <> bSave ( 1 ) then Warnlog "Western Characters Only => changes not saved!"
   if WesternTextAndPunctuation.IsChecked          <> bSave ( 2 ) then Warnlog "Western Text And Punctuation => changes not saved!"
   if NoCompression.IsChecked                      <> bSave ( 3 ) then Warnlog "No Compression => changes not saved!"
   if CompressPunctuationOnly.IsChecked            <> bSave ( 4 ) then Warnlog "Compress Punctuation Only => changes not saved!"
   if CompressPunctuationAndJapaneseKana.IsChecked <> bSave ( 5 ) then Warnlog "Compress Punctuation And Japanese Kana => changes not saved!"
   for i=1 to iLan
      Language.Select sSave(i)
      if Standard.IsChecked <> bSave ( 5+i ) then Warnlog sSave(i) + " : Standard -> changes not saved"
      Standard.Uncheck
      if NoStartOfLine.GetText <> sSave ( 5+i ) then Warnlog sSave(i) + " : NoStartOfLine -> changes not saved"
      if NoEndOfLine.GetText <> sSave ( 10+i )  then Warnlog sSave(i) + " : NoEndOfLine -> changes not saved"
      if bSave ( 5+i ) = TRUE then Standard.Check else Standard.Uncheck
   next i

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)
endcase
