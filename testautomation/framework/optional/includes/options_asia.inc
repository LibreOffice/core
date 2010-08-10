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
'* short description : functionality test for language/settings
'*
'\*****************************************************************

testcase func_LanguageSettings_Language
  Dim bSave as Boolean

   ToolsOptions
   hToolsOptions ( "LanguageSettings", "Languages" )
   bSave = Aktivieren.IsChecked
 Printlog "asian support = TRUE"
   if Aktivieren.isEnabled then
       Aktivieren.unCheck
   else
       qaErrorLog("Asian locale setting is selected: '"+Gebietsschema.getSelText()+"'")
       printlog Aktivieren.isEnabled 
   endif

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

   hTestAsianForWriter    ( TRUE )
   hTestAsianForCalc      ( TRUE )
   hTestAsianForImpress   ( TRUE )
   hTestAsianForDraw      ( TRUE )

   ToolsOptions
   hToolsOptions ( "LanguageSettings", "Languages" )
 Printlog Chr(13) + "asian support = FALSE"
   Aktivieren.Uncheck

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

   hTestAsianForWriter    ( FALSE )
   hTestAsianForCalc      ( FALSE )
   hTestAsianForImpress   ( FALSE )
   hTestAsianForDraw      ( FALSE )

   ToolsOptions
   hToolsOptions ( "LanguageSettings", "Languages" )
 Printlog Chr(13) + "asian support = TRUE"
   if Aktivieren.isEnabled then
       Aktivieren.unCheck
   else
       qaErrorLog("Asian locale setting is selected: '"+Gebietsschema.getSelText()+"'")
       printlog Aktivieren.isEnabled 
   endif

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

   hTestAsianForWriter    ( TRUE )
   hTestAsianForCalc      ( TRUE )
   hTestAsianForImpress   ( TRUE )
   hTestAsianForDraw      ( TRUE )

   ToolsOptions
   hToolsOptions ( "LanguageSettings", "Languages" )
   if bSave = TRUE then Aktivieren.Check else Aktivieren.UnCheck
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)
endcase

' *********************************************
' **
' **
sub hTestAsianForWriter ( bAsianTrue as Boolean )
  Dim iTabCounter as Integer

   gApplication = "WRITER"
   hNewDocument
 Printlog "- test in writer"

   FormatCharacter

   if bAsianTrue = TRUE then
      Kontext
      iTabCounter = active.GetPageCount
      if iTabCounter <> 6 then Warnlog "There are not 6 pages on the dialog, there are " + iTabCounter + "!"
   else
      Kontext
      iTabCounter = active.GetPageCount
      if iTabCounter <> 5 then Warnlog "There are not 5 pages on the dialog, there are " + iTabCounter + "!"
   end if

 printlog "  - check tab-dialog for FormatCharacter"
   Kontext
   active.SetPage TabFont
   Kontext "TabFont"
   if bAsianTrue = TRUE then
      if FontWest.Exists then
         if FontWest.IsVisible = FALSE then Warnlog "'Western Font' is not visible!"
      else
         Warnlog "'Western Font' does not exists!"
      end if
      if StyleWest.Exists then
         if StyleWest.IsVisible = FALSE then Warnlog "'Western Style' is not visible!"
      else
         Warnlog "'Western Style' does not exists!"
      end if
      if SizeWest.Exists then
         if SizeWest.IsVisible = FALSE then Warnlog "'Western Size' is not visible!"
      else
         Warnlog "'Western Size' does not exists!"
      end if
      if LanguageWest.Exists then
         if LanguageWest.IsVisible = FALSE then Warnlog "'Western Language' is not visible!"
      else
         Warnlog "'Western Language' does not exists=> Bug!"
      end if
      if FontEast.Exists then
         if FontEast.IsVisible = FALSE then Warnlog "'Eastern Font' is not visible!"
      else
         Warnlog "'Eastern Font' does not exists!"
      end if
      if StyleEast.Exists then
         if StyleEast.IsVisible = FALSE then Warnlog "'Eastern Style' is not visible!"
      else
         Warnlog "'Eastern Style' does not exists!"
      end if
      if SizeEast.Exists then
         if SizeEast.IsVisible = FALSE then Warnlog "'Eastern Size' is not visible!"
      else
         Warnlog "'Eastern Size' does not exists!"
      end if
      if LanguageEast.Exists then
         if LanguageEast.IsVisible = FALSE then Warnlog "'Eastern Language' is not visible!"
      else
         Warnlog "'Eastern Language' does not exists!"
      end if
   else
      if FontEast.Exists then
         if FontEast.IsVisible then Warnlog "'Eastern Font' is visible!"
      end if
      if StyleEast.Exists then
         if StyleEast.IsVisible then Warnlog "'Eastern Style' is visible!"
      end if
      if SizeEast.Exists then
         if SizeEast.IsVisible then Warnlog "'Eastern Size' is visible!"
      end if
      if LanguageEast.Exists then
         if LanguageEast.IsVisible then Warnlog "'Eastern Language' is visible!"
      end if
      if FontWest.Exists then
         if FontWest.IsVisible then Warnlog "'Eastern Font' is visible!"
      end if
      if StyleWest.Exists then
         if StyleWest.IsVisible then Warnlog "'Eastern Style' is visible!"
      end if
      if SizeWest.Exists then
         if SizeWest.IsVisible then Warnlog "'Eastern Size' is visible!"
      end if
      if LanguageWest.Exists then
         if LanguageWest.IsVisible then Warnlog "'Eastern Language' is visible!"
      end if
   end if

   Kontext
   active.SetPage TabFontEffects
   Kontext "TabFontEffects"
   if bAsianTrue = TRUE then
      if Emphasis.Exists then
         if Emphasis.IsVisible = FALSE then
            Warnlog "'Emphasis mark' is not visible!"
         else
            Emphasis.Select 2
         end if
      else
         Warnlog "'Emphasis mark' does not exists!"
      end if
      if Position.Exists then
         if Position.IsVisible = FALSE then Warnlog "'Position' is not visible!"
      else
         Warnlog "'Position' does not exists!"
      end if
   else
      if Emphasis.Exists then
         if Emphasis.IsVisible then Warnlog "'Emphasis mark' is visible!"
      end if
      if Position.Exists then
         if Position.IsVisible then Warnlog "'Position' is visible!"
      end if
   end if

   Kontext
   Active.SetPage TabFontPosition

   if bAsianTrue = TRUE then
      try
         Kontext
         Active.SetPage TabAsianLayout
      catch
         Warnlog "The tabpage 'Asian Layout' does not exists!"
      endcatch
   else
      try
         Kontext
         Active.SetPage TabAsianLayout
         Warnlog "The tabpage 'Asian Layout' exists!"
      catch
      endcatch
   end if

   Kontext
   active.SetPage TabHyperlinkZeichen

   Kontext
   active.SetPage TabHintergrund
   kontext "TabHintergrund"
   TabHintergrund.Close

 printlog "  - check ruby-dialog"
   if bAsianTrue = TRUE then
      try
         FormatRuby
         Kontext "RubyDialog"
         RubyDialog.Close
      catch
         Warnlog "Ruby-Dialog can't be opened!"
      endcatch
   else
      try
         FormatRuby
         Kontext "RubyDialog"
         RubyDialog.Close
         Warnlog "Ruby-Dialog can be opened!"
      catch
      endcatch
   end if

 printlog "  - check menu items for format/change case"
   if bAsianTrue = TRUE then
      try
         FormatChangeCaseHalfWidth
         Wait 500
      catch
         Warnlog "Format/Case/Half width is not active!"
      endcatch

      try
         FormatChangeCaseFullWidth
         Wait 500
      catch
         Warnlog "Format/Case/Full width is not active!"
      endcatch

      try
         FormatChangeCaseHiragana
         Wait 500
      catch
         Warnlog "Format/Case/Hiragana is not active!"
      endcatch

      try
         FormatChangeCaseKatagana
         Wait 500
      catch
         Warnlog "Format/Case/Katagana is not active!"
      endcatch
   else
      try
         FormatChangeCaseHalfWidth
         Wait 500
         Warnlog "Format/Case/Half width is active!"
      catch
      endcatch

      try
         FormatChangeCaseFullWidth
         Wait 500
         Warnlog "Format/Case/Full width is active!"
      catch
      endcatch

      try
         FormatChangeCaseHiragana
         Wait 500
         Warnlog "Format/Case/Hiragana is active!"
      catch
      endcatch

      try
         FormatChangeCaseKatagana
         Wait 500
         Warnlog "Format/Case/Katagana is active!"
      catch
      endcatch
   end if

 printlog "  - check find&replace-dialog"
   EditSearchAndReplace
   Kontext "SuchenUndErsetzenWriter"
   if bAsianTrue = TRUE then
      if HalbNormalbreiteFormen.Exists then
         if HalbNormalbreiteFormen.IsVisible = FALSE then Warnlog "'Match half-/full-width forms' is not visible!"
      else
         Warnlog "'Match half-/full-width forms' does not exists!"
      end if
      if AehnlicheSchreibweise.Exists then
         if AehnlicheSchreibweise.IsVisible = FALSE then
            Warnlog "'Sounds like (Japanese)' is not visible!"
         else
            AehnlicheSchreibweise.Check
         end if
      else
         Warnlog "'Sounds like (Japanese)' does not exists!"
      end if
      if AehnlicheSchreibweiseOptionen.Exists then
         if AehnlicheSchreibweiseOptionen.IsVisible = FALSE then Warnlog "'Options for 'Sounds like' is not visible!"
      else
         Warnlog "'Options for 'Sounds like' does not exists!"
      end if
   else
      if HalbNormalbreiteFormen.Exists then
         if HalbNormalbreiteFormen.IsVisible then Warnlog "'Match half-/full-width forms' is visible!"
      end if
      if AehnlicheSchreibweise.Exists then
         if AehnlicheSchreibweise.IsVisible then Warnlog "'Sounds like (Japanese)' is visible!"
      end if
      if AehnlicheSchreibweiseOptionen.Exists then
         if AehnlicheSchreibweiseOptionen.IsVisible then Warnlog "'Options for 'Sounds like' is visible!"
      end if
   end if
   SuchenUndErsetzenWriter.Close

   Call hCloseDocument

end sub

' *********************************************
' **
' **
sub hTestAsianForCalc ( bAsianTrue as Boolean )
  Dim iTabCounter as Integer

 Printlog "- test in calc"
   gApplication = "CALC"
   hNewDocument

   FormatCells

   if bAsianTrue = TRUE then
      Kontext
      iTabCounter = active.GetPageCount
      if iTabCounter <> 8 then Warnlog "There are not 8 pages on the dialog, there are " + iTabCounter + "!"
   else
      Kontext
      iTabCounter = active.GetPageCount
      if iTabCounter <> 7 then Warnlog "There are not 7 pages on the dialog, there are " + iTabCounter + "!"
   end if

 printlog "  - check tab-dialog for Format/Cell"
   Kontext
   active.SetPage TabZahlen

   Kontext
   active.SetPage TabFont
   Kontext "TabFont"
   if bAsianTrue = TRUE then
      if FontWest.Exists then
         if FontWest.IsVisible = FALSE then Warnlog "'Western Font' is not visible!"
      else
         Warnlog "'Western Font' does not exists!"
      end if
      if StyleWest.Exists then
         if StyleWest.IsVisible = FALSE then Warnlog "'Western Style' is not visible!"
      else
         Warnlog "'Western Style' does not exists!"
      end if
      if SizeWest.Exists then
         if SizeWest.IsVisible = FALSE then Warnlog "'Western Size' is not visible!"
      else
         Warnlog "'Western Size' does not exists!"
      end if
      if LanguageWest.Exists then
         if LanguageWest.IsVisible = FALSE then Warnlog "'Western Language' is not visible!"
      else
         Warnlog "'Western Language' does not exists=> Bug!"
      end if
      if FontEast.Exists then
         if FontEast.IsVisible = FALSE then Warnlog "'Eastern Font' is not visible!"
      else
         Warnlog "'Eastern Font' does not exists!"
      end if
      if StyleEast.Exists then
         if StyleEast.IsVisible = FALSE then Warnlog "'Eastern Style' is not visible!"
      else
         Warnlog "'Eastern Style' does not exists!"
      end if
      if SizeEast.Exists then
         if SizeEast.IsVisible = FALSE then Warnlog "'Eastern Size' is not visible!"
      else
         Warnlog "'Eastern Size' does not exists!"
      end if
      if LanguageEast.Exists then
         if LanguageEast.IsVisible = FALSE then Warnlog "'Eastern Language' is not visible!"
      else
         Warnlog "'Eastern Language' does not exists!"
      end if
   else
      if FontEast.Exists then
         if FontEast.IsVisible then Warnlog "'Eastern Font' is visible!"
      end if
      if StyleEast.Exists then
         if StyleEast.IsVisible then Warnlog "'Eastern Style' is visible!"
      end if
      if SizeEast.Exists then
         if SizeEast.IsVisible then Warnlog "'Eastern Size' is visible!"
      end if
      if LanguageEast.Exists then
         if LanguageEast.IsVisible then Warnlog "'Eastern Language' is visible!"
      end if
      if FontWest.Exists then
         if FontWest.IsVisible then Warnlog "'Eastern Font' is visible!"
      end if
      if StyleWest.Exists then
         if StyleWest.IsVisible then Warnlog "'Eastern Style' is visible!"
      end if
      if SizeWest.Exists then
         if SizeWest.IsVisible then Warnlog "'Eastern Size' is visible!"
      end if
      if LanguageWest.Exists then
         if LanguageWest.IsVisible then Warnlog "'Eastern Language' is visible!"
      end if
   end if

   Kontext
   active.SetPage TabFontEffects
   Kontext "TabFontEffects"
   if bAsianTrue = TRUE then
      if Emphasis.Exists then
         if Emphasis.IsVisible = FALSE then
            Warnlog "'Emphasis mark' is not visible!"
         else
            Emphasis.Select 2
         end if
      else
         Warnlog "'Emphasis mark' does not exists!"
      end if
      if Position.Exists then
         if Position.IsVisible = FALSE then Warnlog "'Position' is not visible!"
      else
         Warnlog "'Position' does not exists!"
      end if
   else
      if Emphasis.Exists then
         if Emphasis.IsVisible then Warnlog "'Emphasis mark' is visible!"
      end if
      if Position.Exists then
         if Position.IsVisible then Warnlog "'Position' is visible!"
      end if
   end if

   if bAsianTrue = TRUE then
      try
         Kontext
         Active.SetPage TabAsianTypography
      catch
         Warnlog "The tabpage 'Asian Typography' does not exists!"
      endcatch
   else
      try
         Kontext
         Active.SetPage TabAsianTypography
         Warnlog "The tabpage 'Asian Typography' exists!"
      catch
      endcatch
   end if

    Kontext
    active.SetPage TabAusrichtung

    Kontext
    active.SetPage TabUmrandung

    Kontext
    active.SetPage TabHintergrund

    Kontext
    active.SetPage TabZellschutz
    Kontext "TabZellschutz"
    TabZellschutz.Cancel

 printlog "  - check find&replace-dialog"
   EditSearchAndReplace
   Kontext "SuchenUndErsetzenCalc"
   if bAsianTrue = TRUE then
      if HalbNormalbreiteFormen.Exists then
         if HalbNormalbreiteFormen.IsVisible = FALSE then Warnlog "'Match half-/full-width forms' is not visible!"
      else
         Warnlog "'Match half-/full-width forms' does not exists!"
      end if
      if AehnlicheSchreibweise.Exists then
         if AehnlicheSchreibweise.IsVisible = FALSE then
            Warnlog "'Sounds like (Japanese)' is not visible!"
         else
            AehnlicheSchreibweise.Check
         end if
      else
         Warnlog "'Sounds like (Japanese)' does not exists!"
      end if
      if AehnlicheSchreibweiseOptionen.Exists then
         if AehnlicheSchreibweiseOptionen.IsVisible = FALSE then Warnlog "'Options for 'Sounds like' is not visible!"
      else
         Warnlog "'Options for 'Sounds like' does not exists!"
      end if
   else
      if HalbNormalbreiteFormen.Exists then
         if HalbNormalbreiteFormen.IsVisible then Warnlog "'Match half-/full-width forms' is visible!"
      end if
      if AehnlicheSchreibweise.Exists then
         if AehnlicheSchreibweise.IsVisible then Warnlog "'Sounds like (Japanese)' is visible!"
      end if
      if AehnlicheSchreibweiseOptionen.Exists then
         if AehnlicheSchreibweiseOptionen.IsVisible then Warnlog "'Options for 'Sounds like' is visible!"
      end if
   end if
   SuchenUndErsetzenCalc.Close

   Call hCloseDocument

end sub

' *********************************************
' **
' **
sub hTestAsianForImpress ( bAsianTrue as Boolean )
  Dim iTabCounter as Integer

 Printlog "- test in impress"
   gApplication = "IMPRESS"
   hNewDocument

 printlog "  - check 'asian typography' at Format/Paragraph"
   FormatParagraph
   Kontext
   active.SetPage TabEinzuegeUndAbstaende

   if bAsianTrue = TRUE then
      try
         Kontext
         Active.SetPage TabAsianTypography
      catch
         Warnlog "The tabpage 'Asian Typography' does not exists!"
      endcatch
   else
      try
         Kontext
         Active.SetPage TabAsianTypography
         Warnlog "The tabpage 'Asian Typography' exists!"
      catch
      endcatch
   end if

   Kontext
   active.SetPage TabAusrichtungAbsatz

   Kontext
   active.SetPage TabTabulator
   kontext "TabTabulator"
   TabTabulator.Cancel

   hCloseDocument
end sub

' *********************************************
' **
' **
sub hTestAsianForDraw ( bAsianTrue as Boolean )
  Dim iTabCounter as Integer

 Printlog "- test in draw"
   gApplication = "DRAW"
   hNewDocument

 printlog "  - check 'asian typography' at Format/Paragraph"
   FormatParagraph
   Kontext
   active.SetPage TabEinzuegeUndAbstaende

   if bAsianTrue = TRUE then
      try
         Kontext
         Active.SetPage TabAsianTypography
      catch
         Warnlog "The tabpage 'Asian Typography' does not exists!"
      endcatch
   else
      try
         Kontext
         Active.SetPage TabAsianTypography
         Warnlog "The tabpage 'Asian Typography' exists!"
      catch
      endcatch
   end if

   Kontext
   active.SetPage TabAusrichtungAbsatz

   Kontext
   active.SetPage TabTabulator
   kontext "TabTabulator"
   TabTabulator.Cancel

   hCloseDocument
end sub

