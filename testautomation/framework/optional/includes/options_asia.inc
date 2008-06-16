'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: options_asia.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:15 $
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
'* short description : functionality test for language/settings
'*
'\*****************************************************************

testcase func_LanguageSettings_Language
  Dim bSave as Boolean
'///func_LanguageSettings_Language : functionality test for asian settings ( activated Asian-Support )

'///Open tools / options / Language Settings / Language
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "Languages" )
'///+save the setting for 'activated' Asian Support
   bSave = Aktivieren.IsChecked
'///+check 'activated' Asian support and press OK
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

'///+check Asian-Support in Writer ( see hTestAsianForWriter )
   hTestAsianForWriter    ( TRUE )
'///+check Asian-Support in Calc ( see hTestAsianForCalc )
   hTestAsianForCalc      ( TRUE )
'///+check Asian-Support in Impress ( see hTestAsianForImpress )
   hTestAsianForImpress   ( TRUE )
'///+check Asian-Support in Draw ( see hTestAsianForDraw )
   hTestAsianForDraw      ( TRUE )

'///Open tools / options / Language Settings / Language
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "Languages" )
 Printlog Chr(13) + "asian support = FALSE"
'///+uncheck 'activated' Asian support and press OK
   Aktivieren.Uncheck

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

'///+check the unabled Asian-Support in Writer ( see hTestAsianForWriter )
   hTestAsianForWriter    ( FALSE )
'///+check the unabled Asian-Support in Calc ( see hTestAsianForCalc )
   hTestAsianForCalc      ( FALSE )
'///+check the unabled Asian-Support in Impress ( see hTestAsianForImpress )
   hTestAsianForImpress   ( FALSE )
'///+check the unabled Asian-Support in Draw ( see hTestAsianForDraw )
   hTestAsianForDraw      ( FALSE )

'///Open tools / options / Language Settings / Language
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "Languages" )
 Printlog Chr(13) + "asian support = TRUE"
'///+check 'activated' Asian support and press OK
   if Aktivieren.isEnabled then
       Aktivieren.unCheck
   else
       qaErrorLog("Asian locale setting is selected: '"+Gebietsschema.getSelText()+"'")
       printlog Aktivieren.isEnabled 
   endif

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

'///+check Asian-Support in Writer ( see hTestAsianForWriter )
   hTestAsianForWriter    ( TRUE )
'///+check Asian-Support in Calc ( see hTestAsianForCalc )
   hTestAsianForCalc      ( TRUE )
'///+check Asian-Support in Impress ( see hTestAsianForImpress )
   hTestAsianForImpress   ( TRUE )
'///+check Asian-Support in Draw ( see hTestAsianForDraw )
   hTestAsianForDraw      ( TRUE )

'///Open tools / options / Language Settings / Language
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "Languages" )
'///+set 'activated' Asian Support to the saved state and press OK
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
'///check if all changes are made when Asian support is activated or not ( in Writer )

   gApplication = "WRITER"
'///open a new writer doc
   hNewDocument
 Printlog "- test in writer"

'///open Format / Character
   FormatCharacter

'///+- if Asian Support is activated => 6 Tabpages
'///+- if Asian Support is not activated => 5 Tabpages
   if bAsianTrue = TRUE then
      Kontext
      iTabCounter = active.GetPageCount
      if iTabCounter <> 6 then Warnlog "There are not 6 pages on the dialog, there are " + iTabCounter + "!"
   else
      Kontext
      iTabCounter = active.GetPageCount
      if iTabCounter <> 5 then Warnlog "There are not 5 pages on the dialog, there are " + iTabCounter + "!"
   end if

'///select Font-Tabpage
 printlog "  - check tab-dialog for FormatCharacter"
   Kontext
   active.SetPage TabFont
   Kontext "TabFont"
'///+- if Asian Support is activated
   if bAsianTrue = TRUE then
'///+--  Font west => exists and is visible
      if FontWest.Exists then
         if FontWest.IsVisible = FALSE then Warnlog "'Western Font' is not visible!"
      else
         Warnlog "'Western Font' does not exists!"
      end if
'///+--  Style west => exists and is visible
      if StyleWest.Exists then
         if StyleWest.IsVisible = FALSE then Warnlog "'Western Style' is not visible!"
      else
         Warnlog "'Western Style' does not exists!"
      end if
'///+--  Size west => exists and is visible
      if SizeWest.Exists then
         if SizeWest.IsVisible = FALSE then Warnlog "'Western Size' is not visible!"
      else
         Warnlog "'Western Size' does not exists!"
      end if
'///+--  Language west => exists and is visible
      if LanguageWest.Exists then
         if LanguageWest.IsVisible = FALSE then Warnlog "'Western Language' is not visible!"
      else
         Warnlog "'Western Language' does not exists=> Bug!"
      end if
'///+--  Font east => exists and is visible
      if FontEast.Exists then
         if FontEast.IsVisible = FALSE then Warnlog "'Eastern Font' is not visible!"
      else
         Warnlog "'Eastern Font' does not exists!"
      end if
'///+--  Style east => exists and is visible
      if StyleEast.Exists then
         if StyleEast.IsVisible = FALSE then Warnlog "'Eastern Style' is not visible!"
      else
         Warnlog "'Eastern Style' does not exists!"
      end if
'///+--  Size east => exists and is visible
      if SizeEast.Exists then
         if SizeEast.IsVisible = FALSE then Warnlog "'Eastern Size' is not visible!"
      else
         Warnlog "'Eastern Size' does not exists!"
      end if
'///+--  Language east => exists and is visible
      if LanguageEast.Exists then
         if LanguageEast.IsVisible = FALSE then Warnlog "'Eastern Language' is not visible!"
      else
         Warnlog "'Eastern Language' does not exists!"
      end if
   else
'///+- if Asian Support is not activated
'///+--  Font east => mustn't exists
      if FontEast.Exists then
         if FontEast.IsVisible then Warnlog "'Eastern Font' is visible!"
      end if
'///+--  Style east => mustn't exists
      if StyleEast.Exists then
         if StyleEast.IsVisible then Warnlog "'Eastern Style' is visible!"
      end if
'///+--  Size east => mustn't exists
      if SizeEast.Exists then
         if SizeEast.IsVisible then Warnlog "'Eastern Size' is visible!"
      end if
'///+--  Language east => mustn't exists
      if LanguageEast.Exists then
         if LanguageEast.IsVisible then Warnlog "'Eastern Language' is visible!"
      end if
'///+--  Font west => mustn't exists
      if FontWest.Exists then
         if FontWest.IsVisible then Warnlog "'Eastern Font' is visible!"
      end if
'///+--  Style west => mustn't exists
      if StyleWest.Exists then
         if StyleWest.IsVisible then Warnlog "'Eastern Style' is visible!"
      end if
'///+--  Size west => mustn't exists
      if SizeWest.Exists then
         if SizeWest.IsVisible then Warnlog "'Eastern Size' is visible!"
      end if
'///+--  Language west => mustn't exists
      if LanguageWest.Exists then
         if LanguageWest.IsVisible then Warnlog "'Eastern Language' is visible!"
      end if
   end if

'///select FontEffects-Tabpage
   Kontext
   active.SetPage TabFontEffects
   Kontext "TabFontEffects"
'///+- if Asian Support is activated
   if bAsianTrue = TRUE then
'///+-- Emphasis Mark => exist and is visible
      if Emphasis.Exists then
         if Emphasis.IsVisible = FALSE then
            Warnlog "'Emphasis mark' is not visible!"
         else
            Emphasis.Select 2
         end if
      else
         Warnlog "'Emphasis mark' does not exists!"
      end if
'///+-- Position for Emphasis Mark => exist and is visible
      if Position.Exists then
         if Position.IsVisible = FALSE then Warnlog "'Position' is not visible!"
      else
         Warnlog "'Position' does not exists!"
      end if
   else
'///+- if Asian Support is not activated
'///+-- Emphasis Mark mustn't exist
      if Emphasis.Exists then
         if Emphasis.IsVisible then Warnlog "'Emphasis mark' is visible!"
      end if
'///+-- Position for Emphasis Mark mustn't exist
      if Position.Exists then
         if Position.IsVisible then Warnlog "'Position' is visible!"
      end if
   end if

   Kontext
   Active.SetPage TabFontPosition

'///select AsianLayout-Tabpage
'///+- if Asian Support is activated
'///+-- try to select the tabpage 'Asian Layout'
   if bAsianTrue = TRUE then
      try
         Kontext
         Active.SetPage TabAsianLayout
      catch
         Warnlog "The tabpage 'Asian Layout' does not exists!"
      endcatch
   else
'///+- if Asian Support is not activated
'///+-- the tabpage 'Asian Layout' mustn't exist
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
'///close the Tabpage dialog

'///Ruby Dialog
 printlog "  - check ruby-dialog"
   if bAsianTrue = TRUE then
'///+- if Asian Support is activated
'///+-- menu item 'Format/Ruby' exists and is enabled
      try
         FormatRuby
         Kontext "RubyDialog"
         RubyDialog.Close
      catch
         Warnlog "Ruby-Dialog can't be opened!"
      endcatch
   else
'///+- if Asian Support is not activated
'///+-- menu item 'Format/Ruby' mustn't exists and or is disabled
      try
         FormatRuby
         Kontext "RubyDialog"
         RubyDialog.Close
         Warnlog "Ruby-Dialog can be opened!"
      catch
      endcatch
   end if

'///format/change case
 printlog "  - check menu items for format/change case"
   if bAsianTrue = TRUE then
'///+- if Asian Support is activated
'///+-- menu item 'Format/ChangeCase/Half width' exists and is enabled
      try
         FormatChangeCaseHalfWidth
         Wait 500
      catch
         Warnlog "Format/Case/Half width is not active!"
      endcatch

'///+-- menu item 'Format/ChangeCase/Full width' exists and is enabled
      try
         FormatChangeCaseFullWidth
         Wait 500
      catch
         Warnlog "Format/Case/Full width is not active!"
      endcatch

'///+-- menu item 'Format/ChangeCase/Hiragana' exists and is enabled
      try
         FormatChangeCaseHiragana
         Wait 500
      catch
         Warnlog "Format/Case/Hiragana is not active!"
      endcatch

'///+-- menu item 'Format/ChangeCase/Katagana' exists and is enabled
      try
         FormatChangeCaseKatagana
         Wait 500
      catch
         Warnlog "Format/Case/Katagana is not active!"
      endcatch
   else
'///+- if Asian Support is not activated
'///+-- menu item 'Format/ChangeCase/Half width' mustn't exists and is disabled
      try
         FormatChangeCaseHalfWidth
         Wait 500
         Warnlog "Format/Case/Half width is active!"
      catch
      endcatch

'///+-- menu item 'Format/ChangeCase/full width' mustn't exists and is disabled
      try
         FormatChangeCaseFullWidth
         Wait 500
         Warnlog "Format/Case/Full width is active!"
      catch
      endcatch

'///+-- menu item 'Format/ChangeCase/Hiragana' mustn't exists and is disabled
      try
         FormatChangeCaseHiragana
         Wait 500
         Warnlog "Format/Case/Hiragana is active!"
      catch
      endcatch

'///+-- menu item 'Format/ChangeCase/Katagana' mustn't exists and is disabled
      try
         FormatChangeCaseKatagana
         Wait 500
         Warnlog "Format/Case/Katagana is active!"
      catch
      endcatch
   end if

'///Find&Replace-Dialog
 printlog "  - check find&replace-dialog"
   EditSearchAndReplace
   Kontext "SuchenUndErsetzenWriter"
'///+- if Asian Support is activated
   if bAsianTrue = TRUE then
'///+-- 'Match half-/full-width forms' exists and is visible
      if HalbNormalbreiteFormen.Exists then
         if HalbNormalbreiteFormen.IsVisible = FALSE then Warnlog "'Match half-/full-width forms' is not visible!"
      else
         Warnlog "'Match half-/full-width forms' does not exists!"
      end if
'///+-- 'Sounds like (Japanese)' exists and is visible
      if AehnlicheSchreibweise.Exists then
         if AehnlicheSchreibweise.IsVisible = FALSE then
            Warnlog "'Sounds like (Japanese)' is not visible!"
         else
            AehnlicheSchreibweise.Check
         end if
      else
         Warnlog "'Sounds like (Japanese)' does not exists!"
      end if
'///+-- 'Options for 'Sounds like' exists and is visible
      if AehnlicheSchreibweiseOptionen.Exists then
         if AehnlicheSchreibweiseOptionen.IsVisible = FALSE then Warnlog "'Options for 'Sounds like' is not visible!"
      else
         Warnlog "'Options for 'Sounds like' does not exists!"
      end if
   else
'///+- if Asian Support is not activated
'///+-- 'Match half-/full-width forms' mustn't exists
      if HalbNormalbreiteFormen.Exists then
         if HalbNormalbreiteFormen.IsVisible then Warnlog "'Match half-/full-width forms' is visible!"
      end if
'///+-- 'Sounds like (Japanese)' mustn't exists
      if AehnlicheSchreibweise.Exists then
         if AehnlicheSchreibweise.IsVisible then Warnlog "'Sounds like (Japanese)' is visible!"
      end if
'///+-- 'Options for 'Sounds like' mustn't exists
      if AehnlicheSchreibweiseOptionen.Exists then
         if AehnlicheSchreibweiseOptionen.IsVisible then Warnlog "'Options for 'Sounds like' is visible!"
      end if
   end if
'///+close the Find&Replace-Dialog
   SuchenUndErsetzenWriter.Close

'///close writer doc
   Call hCloseDocument

end sub

' *********************************************
' **
' **
sub hTestAsianForCalc ( bAsianTrue as Boolean )
'///check if all changes are made when Asian support is activated or not ( in Calc )
  Dim iTabCounter as Integer

 Printlog "- test in calc"
   gApplication = "CALC"
'///open a new calc doc
   hNewDocument

'///open Format / Cells
   FormatCells

'///+- if Asian Support is activated => 8 tabpages
'///+- if Asian Support is not activated => 7 tabpages
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
'///+- if Asian Support is activated
   if bAsianTrue = TRUE then
'///+--  Font west => exists and is visible
      if FontWest.Exists then
         if FontWest.IsVisible = FALSE then Warnlog "'Western Font' is not visible!"
      else
         Warnlog "'Western Font' does not exists!"
      end if
'///+--  Style west => exists and is visible
      if StyleWest.Exists then
         if StyleWest.IsVisible = FALSE then Warnlog "'Western Style' is not visible!"
      else
         Warnlog "'Western Style' does not exists!"
      end if
'///+--  Size west => exists and is visible
      if SizeWest.Exists then
         if SizeWest.IsVisible = FALSE then Warnlog "'Western Size' is not visible!"
      else
         Warnlog "'Western Size' does not exists!"
      end if
'///+--  Language west => exists and is visible
      if LanguageWest.Exists then
         if LanguageWest.IsVisible = FALSE then Warnlog "'Western Language' is not visible!"
      else
         Warnlog "'Western Language' does not exists=> Bug!"
      end if
'///+--  Font east => exists and is visible
      if FontEast.Exists then
         if FontEast.IsVisible = FALSE then Warnlog "'Eastern Font' is not visible!"
      else
         Warnlog "'Eastern Font' does not exists!"
      end if
'///+--  Style east => exists and is visible
      if StyleEast.Exists then
         if StyleEast.IsVisible = FALSE then Warnlog "'Eastern Style' is not visible!"
      else
         Warnlog "'Eastern Style' does not exists!"
      end if
'///+--  Size east => exists and is visible
      if SizeEast.Exists then
         if SizeEast.IsVisible = FALSE then Warnlog "'Eastern Size' is not visible!"
      else
         Warnlog "'Eastern Size' does not exists!"
      end if
'///+--  Language east => exists and is visible
      if LanguageEast.Exists then
         if LanguageEast.IsVisible = FALSE then Warnlog "'Eastern Language' is not visible!"
      else
         Warnlog "'Eastern Language' does not exists!"
      end if
   else
'///+- if Asian Support is not activated
'///+--  Font east => mustn't exists
      if FontEast.Exists then
         if FontEast.IsVisible then Warnlog "'Eastern Font' is visible!"
      end if
'///+--  Style east => mustn't exists
      if StyleEast.Exists then
         if StyleEast.IsVisible then Warnlog "'Eastern Style' is visible!"
      end if
'///+--  Size east => mustn't exists
      if SizeEast.Exists then
         if SizeEast.IsVisible then Warnlog "'Eastern Size' is visible!"
      end if
'///+--  Language east => mustn't exists
      if LanguageEast.Exists then
         if LanguageEast.IsVisible then Warnlog "'Eastern Language' is visible!"
      end if
'///+--  Font west => mustn't exists
      if FontWest.Exists then
         if FontWest.IsVisible then Warnlog "'Eastern Font' is visible!"
      end if
'///+--  Style west => mustn't exists
      if StyleWest.Exists then
         if StyleWest.IsVisible then Warnlog "'Eastern Style' is visible!"
      end if
'///+--  Size west => mustn't exists
      if SizeWest.Exists then
         if SizeWest.IsVisible then Warnlog "'Eastern Size' is visible!"
      end if
'///+--  Language west => mustn't exists
      if LanguageWest.Exists then
         if LanguageWest.IsVisible then Warnlog "'Eastern Language' is visible!"
      end if
   end if

   Kontext
   active.SetPage TabFontEffects
   Kontext "TabFontEffects"
'///+- if Asian Support is activated
   if bAsianTrue = TRUE then
'///+-- Emphasis Mark => exist and is visible
      if Emphasis.Exists then
         if Emphasis.IsVisible = FALSE then
            Warnlog "'Emphasis mark' is not visible!"
         else
            Emphasis.Select 2
         end if
      else
         Warnlog "'Emphasis mark' does not exists!"
      end if
'///+-- Position for Emphasis Mark => exist and is visible
      if Position.Exists then
         if Position.IsVisible = FALSE then Warnlog "'Position' is not visible!"
      else
         Warnlog "'Position' does not exists!"
      end if
   else
'///+- if Asian Support is not activated
'///+-- Emphasis Mark mustn't exist
      if Emphasis.Exists then
         if Emphasis.IsVisible then Warnlog "'Emphasis mark' is visible!"
      end if
'///+-- Position for Emphasis Mark mustn't exist
      if Position.Exists then
         if Position.IsVisible then Warnlog "'Position' is visible!"
      end if
   end if

'///select AsianLayout-Tabpage
'///+- if Asian Support is activated
'///+-- try to select the tabpage 'Asian Layout'
   if bAsianTrue = TRUE then
      try
         Kontext
         Active.SetPage TabAsianTypography
      catch
         Warnlog "The tabpage 'Asian Typography' does not exists!"
      endcatch
   else
'///+- if Asian Support is not activated
'///+-- the tabpage 'Asian Layout' mustn't exist
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
'///close the Tabpage dialog

 printlog "  - check find&replace-dialog"
   EditSearchAndReplace
   Kontext "SuchenUndErsetzenCalc"
'///+- if Asian Support is activated
   if bAsianTrue = TRUE then
'///+-- 'Match half-/full-width forms' exists and is visible
      if HalbNormalbreiteFormen.Exists then
         if HalbNormalbreiteFormen.IsVisible = FALSE then Warnlog "'Match half-/full-width forms' is not visible!"
      else
         Warnlog "'Match half-/full-width forms' does not exists!"
      end if
'///+-- 'Sounds like (Japanese)' exists and is visible
      if AehnlicheSchreibweise.Exists then
         if AehnlicheSchreibweise.IsVisible = FALSE then
            Warnlog "'Sounds like (Japanese)' is not visible!"
         else
            AehnlicheSchreibweise.Check
         end if
      else
         Warnlog "'Sounds like (Japanese)' does not exists!"
      end if
'///+-- 'Options for 'Sounds like' exists and is visible
      if AehnlicheSchreibweiseOptionen.Exists then
         if AehnlicheSchreibweiseOptionen.IsVisible = FALSE then Warnlog "'Options for 'Sounds like' is not visible!"
      else
         Warnlog "'Options for 'Sounds like' does not exists!"
      end if
   else
'///+- if Asian Support is not activated
'///+-- 'Match half-/full-width forms' mustn't exists
      if HalbNormalbreiteFormen.Exists then
         if HalbNormalbreiteFormen.IsVisible then Warnlog "'Match half-/full-width forms' is visible!"
      end if
'///+-- 'Sounds like (Japanese)' mustn't exists
      if AehnlicheSchreibweise.Exists then
         if AehnlicheSchreibweise.IsVisible then Warnlog "'Sounds like (Japanese)' is visible!"
      end if
'///+-- 'Options for 'Sounds like' mustn't exists
      if AehnlicheSchreibweiseOptionen.Exists then
         if AehnlicheSchreibweiseOptionen.IsVisible then Warnlog "'Options for 'Sounds like' is visible!"
      end if
   end if
'///+close the Find&Replace-Dialog
   SuchenUndErsetzenCalc.Close

'///close calc doc
   Call hCloseDocument

end sub

' *********************************************
' **
' **
sub hTestAsianForImpress ( bAsianTrue as Boolean )
'///check if all changes are made when Asian support is activated or not ( in Impress )
  Dim iTabCounter as Integer

 Printlog "- test in impress"
   gApplication = "IMPRESS"
'///open a new impress doc
   hNewDocument

'///Format/Paragraph
 printlog "  - check 'asian typography' at Format/Paragraph"
   FormatParagraph
   Kontext
   active.SetPage TabEinzuegeUndAbstaende

'///select AsianLayout-Tabpage
'///+- if Asian Support is activated
'///+-- try to select the tabpage 'Asian Layout'
   if bAsianTrue = TRUE then
      try
         Kontext
         Active.SetPage TabAsianTypography
      catch
         Warnlog "The tabpage 'Asian Typography' does not exists!"
      endcatch
   else
'///+- if Asian Support is not activated
'///+-- the tabpage 'Asian Layout' mustn't exist
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
'///close the Tabpage dialog

'///close impress doc
   hCloseDocument
end sub

' *********************************************
' **
' **
sub hTestAsianForDraw ( bAsianTrue as Boolean )
'///check if all changes are made when Asian support is activated or not ( in Draw )
  Dim iTabCounter as Integer

 Printlog "- test in draw"
   gApplication = "DRAW"
'///open a new draw doc
   hNewDocument

'///Format/Paragraph
 printlog "  - check 'asian typography' at Format/Paragraph"
   FormatParagraph
   Kontext
   active.SetPage TabEinzuegeUndAbstaende

'///select AsianLayout-Tabpage
'///+- if Asian Support is activated
'///+-- try to select the tabpage 'Asian Layout'
   if bAsianTrue = TRUE then
      try
         Kontext
         Active.SetPage TabAsianTypography
      catch
         Warnlog "The tabpage 'Asian Typography' does not exists!"
      endcatch
   else
'///+- if Asian Support is not activated
'///+-- the tabpage 'Asian Layout' mustn't exist
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
'///close the Tabpage dialog

'///close impress doc
   hCloseDocument
end sub

