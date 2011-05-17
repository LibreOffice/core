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
'* short description : Search & Replace
'*
'\***********************************************************************

sub search

   if bAsianLan = false then Call ActiveDeactivateAsianSupport(False)
   call tCheckDialog
   call tCallDialog
   call tFree_Text

   call tAlle_Suchen
   call tSelection
   call tPart_of_a_word
   call tWhole_Word
   call tMatch_Case

   call tItalic_Search

   call tItalic_Replace
   call tUnderline_Search
   call tUnderline_Replace
   call tAll_Replace

   call tReplace_Arial_with_TmsRmn
   call tBackwards_Search
   call tSeveral_Times
   call tFurther_Look_up_At_The_Beginning
   call tParagraphStyle_Search
   call tText_In_Table
   call tText_In_Frame
   call tSimilarity_Search

end sub

'---------------------------------------------------------
testcase tCheckDialog
    
    Dim Checkboxtitel
   Call hNewDocument
   EditSearchAndReplace
   Sleep 2
   Kontext "FindAndReplace"
   if FindAndReplace.NotExists then EditSearchAndReplace

  printlog "Check Find-and-Replace-Dialog"
  printlog "- Dependencies of Buttons in Search-for-Entry"
   SearchFor.SetText ""
   if Not Backwards.IsVisible then More.Click
   if SimilaritySearch.IsEnabled and SimilaritySearch.IsChecked then SimilaritySearch.Uncheck
   if SearchAll.IsEnabled then Warnlog "Without Search-for-entry Find-All is active!"
   if SearchNow.IsEnabled then Warnlog "Without Search-for-entry Find is active!"
   if ReplaceAll.IsEnabled then Warnlog "Without Search-for-entry Replace-All is active!"
   if ReplaceBtn.IsEnabled then Warnlog "Without Search-for-entry Replace is active!"
   if NoFormat.IsEnabled then Warnlog "Without selecting Formats , No-Format is active!"
   SearchFor.SetText "Hallo"
   if NOT SearchAll.IsEnabled then Warnlog "Containing Search-for-entry Find-All is inactive!"
   if NOT SearchNow.IsEnabled then Warnlog "Containing Search-for-entry Find inactive!"
   if NOT ReplaceAll.IsEnabled then Warnlog "Containing Search-for-entry Replace-All is inactive!"
   if NOT ReplaceBtn.IsEnabled then Warnlog "Containing Search-for-entry Replace is inactive!"

   Sleep 5
  printlog "- Dependencies of Format-Properties"
   Kontext "FindAndReplace"
   Formats.Click
   wait 500
   Kontext
    Active.SetPage TabFontEffects
   Kontext "TabFontEffects"
    Outline.Check
    TabFontEffects.OK
   wait 500
   kontext "FindAndReplace"
    if NOT NoFormat.IsEnabled then Warnlog "Containing Formats, No-Formats is inactive!"
    NoFormat.Click
   wait 500

  printlog "- Dependencies of Checkboxes"
   RegularExpressions.Check
   wait 500
   if WholeWordsOnly.IsEnabled then Warnlog "Regular expressions: 'Whoole words only' is active => BUG!"
   if NOT Backwards.IsEnabled then Warnlog "Regular expressions: 'Backwards' is inactive => BUG!"
   if NOT MatchCase.IsEnabled then Warnlog "Regular expressions: 'Match case' is inactive => BUG!"
   if CurrentSelectionOnly.IsEnabled then Warnlog "Regular expressions: 'Current selection only' is active => BUG!"
   if NOT SearchForStyles.IsEnabled then Warnlog "Regular expressions: 'Search for Styles' is inactive => BUG!"
   if SimilaritySearch.IsEnabled then Warnlog "Regular expressions: 'Similarity search' is active => BUG!"
   if SimilaritySearchFor.IsEnabled then Warnlog "Regular expressions: Button for 'Similarity search' - dialog is active => BUG!"
   RegularExpressions.UnCheck

   wait 500
   SearchForStyles.Check
   if WholeWordsOnly.IsEnabled then Warnlog "Search for Styles: 'Whole words only' is active => BUG!"
   if NOT Backwards.IsEnabled then Warnlog "Search for Styles: 'Backwards' is inactive => BUG!"
   if RegularExpressions.IsEnabled then Warnlog "Search for Styles: 'Regular expressions' is active => BUG!"
   if MatchCase.IsEnabled then Warnlog "Search for Styles: 'Match case' is active => BUG!"
   if CurrentSelectionOnly.IsEnabled then Warnlog "Regular expressions: 'Current selection only' is active => BUG!"
   if SimilaritySearch.IsEnabled then Warnlog "Search for Styles: 'Similarity search' is active => BUG!"
   if SimilaritySearchFor.IsEnabled then Warnlog "Search for Styles: Button for 'Similarity search' - dialog is active => BUG! "
   SearchForStyles.UnCheck

   wait 500
   SimilaritySearch.Check
   if NOT WholeWordsOnly.IsEnabled then Warnlog "Similarity Search: 'Whole words only' is active => BUG!"
   if NOT Backwards.IsEnabled then Warnlog "Similarity Search: 'Backwards' is inactive => BUG!"
   if RegularExpressions.IsEnabled then Warnlog "Similarity Search: 'Regular expressions' is active => BUG!"
   if NOT MatchCase.IsEnabled then Warnlog "Similarity Search: 'Match case' is active => BUG!"
   if CurrentSelectionOnly.IsEnabled then Warnlog "Regular expressions: 'Current selection only' is active => BUG!"
   if SearchForStyles.IsEnabled then Warnlog "Regular expressions: 'Search for Styles' is active => BUG!"
   if NOT SimilaritySearchFor.IsEnabled then Warnlog "Similarity Search: Button for 'Similarity search' - dialog is active => BUG! "
   SimilaritySearch.UnCheck
   Sleep 2
   FindAndReplace.Close

   Kontext "DocumentWriter"
   DocumentWriter.TypeKeys "hallo <Shift Home>"
   EditSearchAndReplace
   kontext "FindAndReplace"
   if NOT CurrentSelectionOnly.IsEnabled then Warnlog "WIth a selected entry, 'Current selection only' isn't active! => BUG"
   Sleep 2
   FindAndReplace.Close
   wait 500
   Call hCloseDocument
endcase

'---------------------------------------------------------
testcase tCallDialog
 Dim Shortcut as String
   Call hNewDocument
   '/// Open new document ///
   kontext "FindAndReplace"
   Sleep 2
   if FindAndReplace.Exists then FindAndReplace.Close
   '/// Close Search&Replace dialog if open ///
   Kontext "DocumentWriter"
   select case iSprache
      case 01 : Shortcut = "<Mod1 f>"
      case 03 : Shortcut = "<Mod1 g>"
      case 07 : Shortcut = "<Mod1 f>"
      case 31 : Shortcut = "<Mod1 f>"
      case 33 : Shortcut = "<Mod1 f>"
      case 34 : Shortcut = "<Mod1 b>"
      case 39 : Shortcut = "<Mod1 f>"
      case 46 : Shortcut = "<Mod1 f>"
      case 49 : Shortcut = "<Mod1 f>"
      case 50 : Shortcut = "<Mod1 f>"
      case 55 : Shortcut = "<Mod1 f>"
	  case 81 : Shortcut = "<Mod1 f>"
      case 82 : Shortcut = "<Mod1 f>"
      case 86 : Shortcut = "<Mod1 f>"
      case 88 : Shortcut = "<Mod1 f>"
      case else : QAErrorLog "Shortcut to call Find&Replace-Dialog has to be included!"
                  Call hCloseDocument
                  goto endsub
   end select
   '/// Open Search and Replace dialog with Shortcut depending on current language ///
   DocumentWriter.TypeKeys Shortcut
   kontext "FindAndReplace"
   if FindAndReplace.NotExists then
      Warnlog "Shortcut " + Shortcut + " didn't call the Find dialog"
   else
      printlog "- Dialog has been called with Shortcut: " + Shortcut
   end if
   Call hCloseDocument
   '/// Close active document ///
   Call hNewDocument
   '/// Open new document ///
   Kontext "Navigator"
   If Navigator.Exists then ViewNavigator
  printlog "- Check, if dialog is not present in a new document"
   '/// Check, if dialog is not present in a new document ///
   kontext "FindAndReplace"
   if FindAndReplace.Exists then
      Warnlog "Search-For-Dialog has been opened in a new document"
      EditSearchAndReplace
   end if
   Call hCloseDocument
   '/// Close active document ///
endcase

'---------------------------------------------------------
testcase tFree_Text
   Call wSetFindAndReplaceToDefault(1)
   Such_Text = "non"                               'Suchen in freiem Text
   Meldungs_Text = "Suchen in freiem Text"
   SearchFor.SetText Such_Text
   ReplaceWith.SetText Meldungs_Text
   SearchNow.Click
   call Vergleichen_und_ausgeben
   EditSearchAndReplace
   kontext "FindAndReplace"            'Suchen&Replace-Fenster "ffnen
   SearchNow.Click
   call Vergleichen_und_ausgeben
   Call hCloseDocument
endcase

'---------------------------------------------------------
testcase tSelection
   Call wSetFindAndReplaceToDefault(1)
   Such_Text = "d n"
   Meldungs_Text = "Suchen in selektiertem Text"              'Suchen in selektiertem Text
   Sleep 2
   FindAndReplace.Close
   kontext "DocumentWriter"
   DocumentWriter.TypeKeys "<Shift PageDown>"
   EditSearchAndReplace
   kontext "FindAndReplace"
   SearchFor.SetText Such_Text
   if CurrentSelectionOnly.IsVisible = False then More.Click
   if not CurrentSelectionOnly.ischecked then                        'Sollte automatisch
   Warnlog "   'Current selection only' has not been checked automatically"       'gecheckt sein, wenn
   CurrentSelectionOnly.check                                     'Text markiert ist
   else
      printlog "OK: Only in Selection is checked automatically"
   end if
   ReplaceWith.SetText Meldungs_Text
   SearchNow.Click
    call Vergleichen_und_ausgeben

   EditSearchAndReplace
   kontext "FindAndReplace"            'Suchen&Replace-Fenster "ffnen
   SearchNow.Click
   call Vergleichen_und_ausgeben
   Call hCloseDocument
endcase

'---------------------------------------------------------
testcase tPart_of_a_word
   Call wSetFindAndReplaceToDefault(1)
   Such_Text = "te"
   Meldungs_Text = "Suchen Teil eines Wortes"         'Suchen  Teil eines Wortes
   SearchFor.SetText Such_Text
   ReplaceWith.SetText Meldungs_Text
   SearchNow.Click
   call Vergleichen_und_ausgeben
   Call hCloseDocument
endcase

'---------------------------------------------------------
testcase tWhole_Word
   call wSetFindAndReplaceToDefault(1)
   Such_Text = "illa"
   Meldungs_Text = "Suchen nur ganzes Wort"        'Suchen  nur ganzes Wort
   SearchFor.SetText Such_Text
   ReplaceWith.SetText Meldungs_Text
   WholeWordsOnly.Check
   SearchNow.Click                     '"te" ist Teil eines Wortes, "Te" als
   call Vergleichen_und_ausgeben
   Call hCloseDocument
endcase

'---------------------------------------------------------
testcase tMatch_Case
   call wSetFindAndReplaceToDefault(1)
   Such_Text = "Be"                 'Exakte Suche
   Meldungs_Text = "Exakte Suche"
   ReplaceWith.SetText Meldungs_Text
   'sleep 1
   MatchCase.check
   SearchFor.SetText Such_Text
   SearchAll.Click
   call Vergleichen_und_ausgeben
   Call hCloseDocument
endcase

'---------------------------------------------------------
testcase tItalic_Search
   call wSetFindAndReplaceToDefault(1)
   Such_Text = "k"                     'Suche nach Format "kursiv"
   Meldungs_Text = "Suchen Format kursiv"
   SearchFor.SetText Such_Text
   try
    Formats.Click
    wait 500
   catch
    Warnlog "Button 'Format is disabled !"
    Call hCloseDocument
    goto endsub
   endcatch

   Kontext
   Active.Setpage TabFont
   Kontext "TabFont"
   if gAsianSup = True or gCTLSup = True then
      try
         StyleWest.Select 2
      catch
         Warnlog "Style has not been found ! -> Asian-Support: " + gAsianSup + "CTL-Support: " + gCTLSup
         Style.Select 2
      endcatch
   else
      Style.Select 2
   end if
   wait 500
   TabFont.Ok
   ' Workaround
   Kontext "FindAndReplace"
   NoFormat.Click
   wait 500
   Formats.Click
   sleep  1
   kontext
   Active.SetPage TabFont
   kontext "TabFont"
   sleep  1
   if gAsianSup = True or gCTLSup = True then
     StyleWest.Select 2
   else
     Style.Select 2
   end if
   sleep  1
   TabFont.OK
   sleep  1
   ' Workaround ends here
   Kontext "FindAndReplace"
   ReplaceWith.SetText Meldungs_Text
   sleep  1
   SearchAll.Click                     'wenn auf Format geachtet wird,
   sleep  1
   call Vergleichen_und_ausgeben             'darf nur ein "k" gefunden werden
   sleep  1
   Call hCloseDocument
endcase

'---------------------------------------------------------
testcase tItalic_Replace
   call wSetFindAndReplaceToDefault(1)
   Such_Text = "kursiv"                'Suche Format "kursiv", ersetze
   Meldungs_Text = "Ersetzte kursiv mit durchgestrichen"    'mit Format "durchgestrichen"
   SearchFor.SetText Such_Text
   wait 500
   try
    NoFormat.Click
   catch
   endcatch
   wait 500
   Formats.Click
   sleep  1
   kontext
   Active.SetPage TabFont
   kontext "TabFont"
   sleep  1
   if gAsianSup = True or gCTLSup = True then
      try
         StyleWest.Select 2
      catch
         Warnlog "Style has not been found ! -> Asian-Support: " + gAsianSup + "CTL-Support: " + gCTLSup
         Style.Select 2
      endcatch
   else
     Style.Select 2
   end if
   sleep  1
   TabFont.OK
   sleep  1
   kontext "FindAndReplace"
   NoFormat.Click
   ' Tried a workaround, because sometimes, searchstring ends with ','. Dont know why !?
   Formats.Click
   sleep  1
   kontext
   Active.SetPage TabFont
   kontext "TabFont"
   sleep  1
   if gAsianSup = True or gCTLSup = True then
     StyleWest.Select 2
   else
     Style.Select 2
   end if
   sleep  1
   TabFont.OK
   sleep  1
   kontext "FindAndReplace"


   ReplaceWith.SetText "durchgestrichen"
   sleep  1
   Formats.Click
   kontext
   sleep  1
   Active.SetPage TabFontEffects
   kontext "TabFontEffects"
   sleep  1
   StrikeThrough.Select 2
   sleep  1
   TabFontEffects.OK
   sleep  1
   kontext "FindAndReplace"
   sleep  1
   ReplaceAll.Click
   sleep  1
   kontext
   sleep  1
   Kontext "Active"
   if Active.Exists then
    if Active.GetRT = 304 then
     Active.OK                     'MSG-Box (... x ersetzt) schließen
    else
     Warnlog "Messagebox, how often replaced, not up !"
    end if
   end if
   sleep  2
   kontext "FindAndReplace"
   FindAndReplace.Close
   sleep  1
   Kontext "DocumentWriter"
   DocumentWriter.Typekeys "<MOD1 HOME>"
   EditSearchAndReplace
   kontext "FindAndReplace"
   Such_Text = "durchgestrichen"
   SearchFor.SetText Such_Text
   sleep  1
   if NoFormat.IsVisible = False then More.Click
   NoFormat.Click
   sleep  1
   ReplaceWith.SetText "durchgestrichen suchen"
   sleep  1
   NoFormat.Click
   sleep  1
   SearchAll.Click
   sleep  1
   call Vergleichen_und_ausgeben
   FormatCharacter
   Kontext
   active.SetPage TabFontEffects
   Kontext "TabFontEffects"
   if StrikeThrough.GetSelIndex <> 2 then Warnlog "Format 'Strikethrough' has not been overtaken!"
   TabFontEffects.Cancel
   Call hCloseDocument
endcase

'---------------------------------------------------------
testcase tUnderline_Search
    if gCTLSup = True then
        QAErrorlog "No Check for attributes if CTL enabled!"
        goto endsub
    end if        
    call wSetFindAndReplaceToDefault(1)
    Such_Text = "U"                     'search for Attributes underlined
    Meldungs_Text = "search Attribut Underlined"
    if gAsianSup = false then
        Select Case iSprache
            case 01: ToggleAttribute(30,"True")             'Check Attributes underlined
            case 07: ToggleAttribute(16,"True")             'Check Attributes underlined
            case 31: ToggleAttribute(12,"True")             'Check Attributes underlined
            case 33: ToggleAttribute(27,"True")
            case 34: ToggleAttribute(28,"True")
            case 39: ToggleAttribute(27,"True")
            case 46: ToggleAttribute(30,"True")            
            case 49: ToggleAttribute(28,"True")
            case 50: ToggleAttribute(15,"True")
            case 55: ToggleAttribute(29,"True")
            case 88: ToggleAttribute(13,"True")
            case else : QAErrorlog "Please adapt test for this language!"
                    Call hCloseDocument
                    goto endsub
        end select
    else
        Select Case iSprache
            case 01: ToggleAttribute(39,"True")             'Check Attributes underlined
            case 07: ToggleAttribute(22,"True")             
            case 31: ToggleAttribute(17,"True")             'Onderstrepen
            case 33: ToggleAttribute(36,"True")             ' Soulignage
            case 34: ToggleAttribute(37,"True")             ' Subrayado
            case 39: ToggleAttribute(35,"True")             ' Sottolineato
            case 46: ToggleAttribute(40,"True")             ' Understruken
            case 49: ToggleAttribute(37,"True")             ' Unterstrichen
            case 50: ToggleAttribute(24,"True")             ' Podčrtano
            case 55: ToggleAttribute(38,"True")             ' Sublinhado
            case 88: ToggleAttribute(13,"True")
            case else : QAErrorlog "Please adapt test for this language!"
                    Call hCloseDocument
                    goto endsub
        end select   
    end if
   
   SearchFor.SetText Such_Text
   ReplaceWith.SetText Meldungs_Text
   SearchAll.Click                     'if looking for the Attributes,
   call Vergleichen_und_ausgeben
   Call hCloseDocument
endcase

'---------------------------------------------------------
testcase tUnderline_Replace
    if gCTLSup = True then
        QAErrorlog "No Check for attributes if CTL enabled!"
        goto endsub
    end if        
    call wSetFindAndReplaceToDefault(1)
    Such_Text = "Unterstrichen"                  'search for Attributes underlined
    Meldungs_Text = "search underlined, replace shadow"      'replace with 'shadow'
    SearchFor.SetText Such_Text
    if gAsianSup = false then
        Select Case iSprache
            case 01: ToggleAttribute(30,"True")             'Check Attributes underlined
            case 07: ToggleAttribute(16,"True")
            case 31: ToggleAttribute(12,"True")
            case 33: ToggleAttribute(27,"True")
            case 34: ToggleAttribute(28,"True")
            case 39: ToggleAttribute(27,"True")
            case 46: ToggleAttribute(30,"True")            
            case 49: ToggleAttribute(28,"True")
            case 50: ToggleAttribute(15,"True")
            case 55: ToggleAttribute(29,"True")
            case 88: ToggleAttribute(13,"True")
            case else : QAErrorlog "Please adapt test for this language!"
                    Call hCloseDocument
                    goto endsub
        end select
    else
        Select Case iSprache
            case 01: ToggleAttribute(39,"True")             'Check Attributes underlined
            case 07: ToggleAttribute(22,"True")
            case 31: ToggleAttribute(17,"True")             ' Onderstrepen
            case 33: ToggleAttribute(36,"True")             ' Soulignage
            case 34: ToggleAttribute(37,"True")             ' Subrayado
            case 39: ToggleAttribute(35,"True")             ' Sottolineato
            case 46: ToggleAttribute(40,"True")             ' Understruken
            case 49: ToggleAttribute(37,"True")             ' Unterstrichen
            case 50: ToggleAttribute(24,"True")             ' Podčrtano
            case 55: ToggleAttribute(38,"True")             ' Sublinhado
            case 88: ToggleAttribute(13,"True")
            case else : QAErrorlog "Please adapt test for this language!"
                    Call hCloseDocument
                    goto endsub
        end select   
    end if
    ReplaceWith.SetText "Schatten"
    Formats.Click
    kontext
    Active.SetPage TabFontEffects
    kontext "TabFontEffects"
    Shadow.Check
    TabFontEffects.OK
    kontext "FindAndReplace"
    ReplaceAll.Click
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Active.OK                     'MSG-Box (... x ersetzt) schließen
        end if
    end if
    kontext "FindAndReplace"
    Such_Text = "Schatten"
    SearchFor.SetText Such_Text
    NoFormat.Click
    'ReplaceWith.SetText Meldungs_Text
    SearchAll.Click
    call Vergleichen_und_ausgeben
    FormatCharacter
    Kontext
    active.SetPage TabFontEffects
    Kontext "TabFontEffects"
    if NOT Shadow.IsChecked then Warnlog "Attribute 'Shadow' has not been overtaken!"
    TabFontEffects.Cancel
    Call hCloseDocument
endcase

'---------------------------------------------------------
testcase tAll_Replace
   call wSetFindAndReplaceToDefault(1)
   Such_Text = "x"                     'Replace alle
   Meldungs_Text = "Replace alle"
   SearchFor.SetText Such_Text
   ReplaceWith.SetText Meldungs_Text
   ReplaceWith.SetText "y"
   ReplaceAll.Click
   Kontext "Active"
   if Active.Exists then
    if Active.GetRT = 304 then
     Active.OK                     'MSG-Box (... x ersetzt) schließen
    end if
   end if
   Such_Text = "y"                     'Suchen  aller "y", um die Ersetzung
   kontext "FindAndReplace"            'zu pruefen
   SearchFor.SetText Such_Text
   ReplaceWith.SetText Meldungs_Text
   SearchAll.Click
   Such_Text = "yyyy"                  '4 mal wurde ersetzt
   call Vergleichen_und_ausgeben
   Call hCloseDocument
endcase

'---------------------------------------------------------
testcase tReplace_Arial_with_TmsRmn
   call wSetFindAndReplaceToDefault(1)
   Such_Text = "opus"                  'Suche Arial, 16 pt, ersetze
   Meldungs_Text = "Ers. Arial mit Times Roman"       'mit Times New Roman, normal 24pt
   SearchFor.SetText Such_Text

   Formats.Click
   kontext
   Active.SetPage TabFont
   kontext "TabFont"
   try
      FontWest.SetText "Arial"
   catch
      Font.SetText "Arial"
   endcatch
   sleep 2
   try
      SizeWest.SetText "16"
   catch
      Size.SetText "16"
   endcatch
   sleep 2
   TabFont.OK
   kontext "FindAndReplace"
   ReplaceWith.SetText "Ersetzungstext"
   sleep 2

   Formats.Click
   kontext
   Active.SetPage TabFont
   kontext "TabFont"
   if gAsianSup = True or gCTLSup = True then
   		try
			FontWest.SetText "Times New Roman"
		catch
        	Warnlog "Control: 'FontWest' has not been found ! -> Asian-Support: " + gAsianSup + "CTL-Support: " + gCTLSup
			Font.SetText "Times New Roman"
		endcatch
	else
		Font.SetText "Times New Roman"
	end if
   sleep 2
   if gAsianSup = True or gCTLSup = True then
     StyleWest.Select 1
   else
     Style.Select 1
   end if
   sleep 2
   try
      SizeWest.SetText "20"
   catch
      Size.SetText "20"
   endcatch
   sleep 2
   TabFont.OK
   sleep 2
   kontext "FindAndReplace"
   ReplaceAll.Click

   sleep 2
   kontext
   Kontext "Active"
   if Active.Exists then
    if Active.GetRT = 304 then
     Active.OK                     'MSG-Box (... x ersetzt) schließen
    else
     Warnlog "Messagebox, how often replaced, not up !"
    end if
   end if
   Sleep 2
   kontext "FindAndReplace"
   FindAndReplace.Close
   Kontext "DocumentWriter"
   DocumentWriter.Typekeys "<MOD1 HOME>"

   EditSearchAndReplace
   kontext "FindAndReplace"
   Such_Text = "Ersetzungstext"
   SearchFor.SetText Such_Text
   if NoFormat.IsVisible = False then More.Click
   NoFormat.Click
   ReplaceWith.SetText "Ersetzungstext suchen"
   NoFormat.Click
   SearchAll.Click
   call Vergleichen_und_ausgeben
   Call hCloseDocument
endcase

'---------------------------------------------------------
testcase tBackwards_Search
   call wSetFindAndReplaceToDefault(1)
   Sleep 2
   FindAndReplace.Close
   kontext "DocumentWriter"               'Suchen  rückwärts, dafür an
   DocumentWriter.Typekeys "<PAGEDOWN>"            'das Ende des Dokuments gehen
   DocumentWriter.Typekeys "<MOD1 END>"
   Such_Text = "no"
   Meldungs_Text = "Suchen rückwärts"
   EditSearchAndReplace
   kontext "FindAndReplace"
   if Backwards.IsVisible = False then More.Click
   Backwards.Check
   if WholeWordsOnly.ischecked then
      Warnlog "   'Match case' is checked"
      WholeWordsOnly.uncheck
   end if
   SearchFor.SetText Such_Text
   ReplaceWith.SetText Meldungs_Text
   SearchNow.Click
   call Vergleichen_und_ausgeben
   Call hCloseDocument
endcase

'---------------------------------------------------------
testcase tSeveral_Times
  dim Zaehler as integer

   call wSetFindAndReplaceToDefault(1)
   Such_Text = "m"                     'Suchen  mehrmals
   Meldungs_Text = "Suchen mehrmals"
   SearchFor.SetText Such_Text
   ReplaceWith.SetText Meldungs_Text
   for Zaehler = 1 to 10
      SearchNow.Click
   next Zaehler
   call Vergleichen_und_ausgeben
   Call hCloseDocument
endcase

'---------------------------------------------------------
testcase tAlle_Suchen
   call wSetFindAndReplaceToDefault(1)
   Such_Text = "x"                     'Suchen  alle
   Meldungs_Text = "Suchen alle"
   SearchFor.SetText Such_Text
   ReplaceWith.SetText Meldungs_Text
   SearchAll.Click
   Such_Text = "xxxx"                  'im Beispieltext kommt "x" 4 mal vor
   call Vergleichen_und_ausgeben
   Call hCloseDocument
endcase

'---------------------------------------------------------
testcase tFurther_Look_up_At_The_Beginning
   call wSetFindAndReplaceToDefault(1)
   Sleep 2
   FindAndReplace.Close             'Meldung am Dokumentende
   kontext "DocumentWriter"
   DocumentWriter.Typekeys "<MOD1 END>"            'and Ende des Dokuments gehen
   Such_Text = "o"
   Meldungs_Text = "Am Dokumentanfang weitersuchen"
   EditSearchAndReplace
   kontext "FindAndReplace"
   SearchFor.SetText Such_Text
   ReplaceWith.SetText Meldungs_Text
   SearchNow.Click
   Kontext "Active"
   if Active.Exists (1) then
      dummy_text = Active.GetText               'Auslesen der Meldung (dt/engl/frz)
      call Meldungs_Test (dummy_text)           'in der Query-Box und Schließen
      Active.Yes                 'mit JA
   end if
   call Vergleichen_und_ausgeben
   Call hCloseDocument
endcase

'---------------------------------------------------------
testcase tParagraphStyle_Search
    call wSetFindAndReplaceToDefault(1)
    ' print "the base for the dialog isn't reached!???? before"
    if Not SearchForStyles.IsVisible then More.Click
    SearchForStyles.Check    
    ' <font color=#ff0000> TBO TODO : insert global LOCALE routine for text : 'Heading' </FONT>
    Select Case iSprache
        Case 01 : Such_Text = "Heading"
        Case 03 : Such_Text = "Título"
        Case 07 : Such_Text = "Заголовок"
        Case 31 : Such_Text = "Kop"
        Case 33 : Such_Text = "Titre"
        Case 34 : Such_Text = "Encabezado"
        Case 39 : Such_Text = "Intestazione"
        Case 46 : Such_Text = "Överskrift"
        Case 49 : Such_Text = "Überschrift"          'Suchen  Absatzformat "Überschrift"
        Case 50 : Such_Text = "Naslov"
        Case 55 : Such_Text = "Título Principal"
        Case 81 : Such_Text = "見出し"
        Case 86 : Such_Text = "标题"
        Case 88 : Such_Text = "標題"
        case else:  QAErrorlog "Insert heading-name (" + wGetHeadingName + ") for language: " + iSprache
                    Call hCloseDocument
                    goto endsub
    end select
    Meldungs_Text = "Suchen Absatzformat"
    Sleep 2
    FindAndReplace.Close
    kontext "DocumentWriter"
    DocumentWriter.Typekeys "<RIGHT>"
    DocumentWriter.Typekeys "<LEFT>"
    EditSearchAndReplace
    kontext "FindAndReplace"
    if SearchForStyles.IsVisible = False then More.Click
    SearchForStyles.Check
    sleep 1                       'Vorlagen muessen erst gelesen werden
    SearchForStyle.Select Such_Text
    SearchNow.Click
    Such_Text = chr(9) + "Absatzformat: Überschrift"
    call Vergleichen_und_ausgeben
    Call hCloseDocument
endcase

'---------------------------------------------------------
testcase tText_In_Table
    call wSetFindAndReplaceToDefault(1)
'warnlog "WorkAround beginn-----------------"
'   kontext "FindAndReplace"
'   Sleep 2
'   if FindAndReplace.Exists then            'Evtl. offenes Fenster schließen
'      FindAndReplace.Close
'   end if
'   kontext "DocumentWriter"
'   EditSearchAndReplace
'   kontext "FindAndReplace"           'Suchen&Replace-Fenster öffnen
'   sleep 2
'warnlog "WorkAround ending!-----------------"

 'print "the base for the dialog isn't reached!???? after!"
   Such_Text = "Tabelle"                  'Suchen Text in Tabelle
   Meldungs_Text = "Text in Tabelle"
   try
      SearchFor.SetText Such_Text
   catch
      print "Text in table not enabled"
   endcatch
   ReplaceWith.SetText Meldungs_Text
   SearchNow.Click
   call Vergleichen_und_ausgeben
   Call hCloseDocument
endcase

'---------------------------------------------------------
testcase tText_In_Frame
   call wSetFindAndReplaceToDefault(1)
   Such_Text = "Rahmen"                'Suchen  Text in Rahmen
   Meldungs_Text = "Suchen Text in Rahmen"   '
   SearchFor.SetText Such_Text
   ReplaceWith.SetText Meldungs_Text
   SearchNow.Click
   call Vergleichen_und_ausgeben
   Call hCloseDocument
endcase

'---------------------------------------------------------

testcase tSimilarity_Search
  dim Zaehler as integer
  dim Zaehler2 as integer
  dim Zeichen_Tauschen as integer
  dim Zeichen_Hinzufuegen as integer
  dim Zeichen_Entfernen as integer

   call wSetFindAndReplaceToDefault(1)
   Such_Text = "Ähnlichkeit"              'Ähnlichkeitssuche
   SearchFor.SetText Such_Text
   if SimilaritySearch.IsVisible = False then More.Click
   SimilaritySearch.check
   for Zaehler2 = 1 to 4                  'Es werden die Optionen
      for Zaehler = 1 to 3                'Tauschen, Hinzufuegen und Entfernen
         Zeichen_Tauschen = 0             '1 bis 3 mal angechewählt
         Zeichen_Hinzufuegen = 0
         Zeichen_Entfernen = 0
         if SimilaritySearch.IsVisible = False then More.Click
         SimilaritySearchFor.click
         kontext "Aehnlichkeitssuche"
         Kombinieren.check
         ZeichenTauschen.ToMin
         ZeichenHinzufuegen.ToMin
         ZeichenEntfernen.ToMin
         ZeichenTauschen.Less
         ZeichenHinzufuegen.Less
         ZeichenEntfernen.Less
         select case Zaehler2
            Case 1 : Zeichen_Tauschen = Zaehler
            Case 2 : Zeichen_Hinzufuegen = Zaehler
            Case 3 : Zeichen_Entfernen = Zaehler
            Case 4 : Zaehler = 3             'Die 3 Optionen werden gleichzeitig
                     Zeichen_Tauschen = Zaehler       'angewählt.
                     Zeichen_Hinzufuegen = Zaehler
                     Zeichen_Entfernen = Zaehler
         end select
         ZeichenTauschen.More (Zeichen_Tauschen)
         ZeichenHinzufuegen.More (Zeichen_Hinzufuegen)
         ZeichenEntfernen.More (Zeichen_Entfernen)
         Aehnlichkeitssuche.OK
         kontext "FindAndReplace"
         SearchAll.Click
         select case Zeichen_Tauschen
            Case 1 : Such_Text = "Ähnlechkiit"
                     Meldungs_Text = "Ähnlichkeitssuche Tauschen 1x"
            Case 2 : Such_Text = "ÄhnlechkiitÄhnilchkiet"
                     Meldungs_Text = "Ähnlichkeitssuche Tauschen 2x"
            Case 3 : Such_Text = "ÄhnlechkiitÄhnilchkiethänilchkiet"
                     Meldungs_Text = "Ähnlichkeitssuche Tauschen 3x"
         end select
         select case Zeichen_Hinzufuegen
            Case 1 : Such_Text = "Ähnnlichkeit"
                     Meldungs_Text = "Ähnlichkeitssuche Hinzufuegen 1x"
            Case 2 : Such_Text = "ÄhnnlichkeitÄhnnliichkeit"
                     Meldungs_Text = "Ähnlichkeitssuche Hinzufuegen 2x"
            Case 3 : Such_Text = "ÄhnnlichkeitÄhnnliichkeitÄhnliichhkeeit"
                     Meldungs_Text = "Ähnlichkeitssuche Hinzufuegen 3x"
         end select
         select case Zeichen_Entfernen
            Case 1 : Such_Text = "Ähnlichkei"
                     Meldungs_Text = "Ähnlichkeitssuche Entfernen 1x"
            Case 2 : Such_Text = "ÄhnlichkeiÄhnlichke"
                     Meldungs_Text = "Ähnlichkeitssuche Entfernen 2x"
            Case 3 : Such_Text = "ÄhnlichkeiÄhnlichkeÄhnlichk"
                     Meldungs_Text = "Ähnlichkeitssuche Entfernen 3x"
         end select
         if Zaehler2 = 4 then
            Such_Text = "ÄhnlechkiitÄhnilchkiethänilchkietÄhnnlichkeitÄhnnliichkeitÄhnliichhkeeitÄhnlichkeiÄhnlichkeÄhnlichkÄnhliikheeii"
            Meldungs_Text ="Ähnlichkeitssuche Hinzu+Entfern+Tausch, je3x"
         end if
         call Vergleichen_und_ausgeben
         kontext "DocumentWriter"
         DocumentWriter.Typekeys "<PAGEUP>",          'an den Anfang des Textes gehen
         DocumentWriter.Typekeys "<MOD1 HOME>"
         EditSearchAndReplace
         kontext "FindAndReplace"            'Suchen&Replace-Fenster öffnen
      next Zaehler
   next Zaehler2
   Call hCloseDocument
endcase

' ----------------------------------------------------------------------------------

function wCountNumberOfAttributes() as Integer
   Dim i as integer, AttributeCount as integer
   Attribute.Click
   Kontext "Attribute"
   wait 500
   ' Count how much Attributes are in the list
   For i = 1 to 200
    try
       Auswahl.Select i
       printlog Auswahl.GetSelText
       AttributeCount = AttributeCount + 1
    catch
      wCountNumberOfAttributes = AttributeCount
      i = 202
    endcatch
   next i
   Attribute.Close
end function

' ----------------------------------------------------------------------------------

function wSelectAttributeName(aNumber as integer, AsianSupport as boolean, SetState as integer) as string
 'This function checks for the Attribute name depending on the given number (aNumber).
 'SetState is given to check or uncheck the Attribute in list
 'Returns the Attribute name if found and successfully checked in list.
 'Default for this test is given from the german office-Version (easier for me:-))
 Dim i as integer, aSelectIt as String
 Kontext "FindAndReplace"
 Attributes.Click
 Kontext "Attribute"

 Select Case aNumber
  Case 1
   Select Case iSprache
    Case 49: aSelectIt = "Absatz trennen"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 2
   Select Case iSprache
    Case 49: aSelectIt = "Absätze zusammenhalten"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 3
   Select Case iSprache
    Case 49: aSelectIt = "Abstand"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 4
   Select Case iSprache
    Case 49: aSelectIt = "Asiatische Schrift"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 5
   Select Case iSprache
    Case 49: aSelectIt = "Ausrichtung"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 6
   Select Case iSprache
    Case 49: aSelectIt = "Auszeichnung"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 7
   Select Case iSprache
    Case 49: aSelectIt = "Betonungszeichen"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 8
   Select Case iSprache
    Case 49: aSelectIt = "Blinkend"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 9
   Select Case iSprache
    Case 49: aSelectIt = "Doppelzeilig"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 10
   Select Case iSprache
    Case 49: aSelectIt = "Durchgestrichen"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 11
   Select Case iSprache
    Case 49: aSelectIt = "Einzug"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 12
   Select Case iSprache
    Case 49: aSelectIt = "Hängende Interpunktion"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 13
   Select Case iSprache
    Case 49: aSelectIt = "Hurenkinder"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 14
   Select Case iSprache
    Case 49: aSelectIt = "Kerning"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 15
   Select Case iSprache
    Case 49: aSelectIt = "Kontur"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 16
   Select Case iSprache
    Case 49: aSelectIt = "Position"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 17
   Select Case iSprache
    Case 49: aSelectIt = "Registerhaltigkeit"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 18
   Select Case iSprache
    Case 49: aSelectIt = "Relief"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 19
   Select Case iSprache
    Case 49: aSelectIt = "Rotation"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 20
   Select Case iSprache
    Case 49: aSelectIt = "Schattiert"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 21
   Select Case iSprache
    Case 49: aSelectIt = "Schrift"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 22
   Select Case iSprache
    Case 49: aSelectIt = "Schriftfarbe"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 23
   Select Case iSprache
    Case 49: aSelectIt = "Schriftgrad"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 24
   Select Case iSprache
    Case 49: aSelectIt = "Schriftgrad asiatische Schrift"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 25
   Select Case iSprache
    Case 49: aSelectIt = "Schriftstärke"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 26
   Select Case iSprache
    Case 49: aSelectIt = "Schriftstärke asiatische Schrift"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 27
   Select Case iSprache
    Case 49: aSelectIt = "Schriftstellung"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 28
   Select Case iSprache
    Case 49: aSelectIt = "Schriftstellung asiatische Schrift"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 29
   Select Case iSprache
    Case 49: aSelectIt = "Schusterjungen"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 30
   Select Case iSprache
    Case 49: aSelectIt = "Seitenvorlage"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 31
   Select Case iSprache
    Case 49: aSelectIt = "Hyphenation"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 32
   Select Case iSprache
    Case 49: aSelectIt = "Skalierung"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 33
   Select Case iSprache
    Case 49: aSelectIt = "Sprache"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 34
   Select Case iSprache
    Case 49: aSelectIt = "Sprache asiatische Schrift"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 35
   Select Case iSprache
    Case 49: aSelectIt = "Tabulatoren"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 36
   Select Case iSprache
    Case 49: aSelectIt = "Text Abstand"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 37
   Select Case iSprache
    Case 49: aSelectIt = "Unterstrichen"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 38
   Select Case iSprache
    Case 49: aSelectIt = "Verbotene Zeichen"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 39
   Select Case iSprache
    Case 49: aSelectIt = "Vertikale Textausrichtung"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 40
   Select Case iSprache
    Case 49: aSelectIt = "Wortweise"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 41
   Select Case iSprache
    Case 49: aSelectIt = "Zeichenhintergrund"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

  Case 42
   Select Case iSprache
    Case 49: aSelectIt = "Zeilenabstand"
    Case else: Warnlog "Language not supported. No test"
               exit function
   end select

 end select

 ' Searches for selected Attribute in Attributes list
  For i = 1 to 42
   try
    Auswahl.Select i
    if Ucase(Auswahl.GetSelText) = Ucase(aSelectIt) then
     Select Case SetState
      Case 1
        Auswahl.UnCheck
      Case 2
        Auswahl.Check
     end select
     wSelectAttributeName = aSelectIt
     i = 50
    end if
   catch
     i = 50
   endcatch
  next i
  'if Attributes not found
  if i = 42 then
   if AsianSupport = True then Warnlog "Unable to locate Attribute in list!"
   wSelectAttributeName = ""
  end if
  Attribute.Ok
end function


sub wCheckAttribute(aNumber as integer)
 Dim i as integer
 Kontext "FindAndReplace"
 SearchNow.Click
 if wCheckActiveMessageboxes = False then
  Warnlog "Search for Attributes has failed!"
  exit sub
 end if
 Select Case aNumber
  Case 1   ' "Absatz trennen"
    EditCopy
    if Left$(GetClipboardtext,15) <> "First Paragraph" then
     Warnlog "-  Wrong paragraph has been found!"
    end if
  Case 2    '"Absätze zusammenhalten"
    EditCopy
    if Left$(GetClipboardtext,25) <> "Keep with next paragraph1" then
     Warnlog "-  Wrong paragraph has been found!"
    end if
  Case 3    '"Abstand"
    EditCopy
    if Left$(GetClipBoardText,15) <> "Spacing between" then
     Warnlog "- Wrong paragraph has been found!"
    end if
  Case 4    '"Asiatische Schrift"
  Case 5    '"Ausrichtung"
     EditCopy
     if Left$(GetClipBoardtext,20) <> "Wow, Alignment right" then
      Warnlog "- Wrong paragraph has been found!"
     else
      printlog "-  'Alignment right' found!"
     end if
     For i = 1 to 2
      Kontext "FindAndReplace"
      SearchNow.Click
      if wCheckActiveMessageboxes = False then
       Warnlog "Search for Attributes has failed!"
       exit sub
      end if
      EditCopy
      Select Case i
       Case 1
         if Left$(GetClipBoardtext,23) <> "Wow, Alignment centered" then
          Warnlog "- Wrong paragraph has been found!"
          exit sub
         else
          printlog "-  'Alignment centered' found!"
         end if
       Case 2
         if Left$(GetClipBoardtext,24) <> "Wow, Alignment justified" then
          Warnlog "- Wrong paragraph has been found!"
         exit sub
         else
          printlog "-  'Alignment justified' found!"
         end if
      end select
     next i
  Case 6    '"Auszeichnung"
     EditCopy
     If Ucase(GetClipBoardtext) <> "CAPITALS" then
      Warnlog "Wrong effect has been found"
      exit sub
     end if
     For i = 1 to 3
      Kontext "FindAndReplace"
      SearchNow.Click
      if wCheckActiveMessageboxes = False then
       Warnlog "Search for effects has failed!"
       exit sub
      end if
      EditCopy
      Select Case i
       Case 1
        If Ucase(GetClipBoardtext) <> "LOWERCASE" then
         Warnlog "Wrong effect has been found"
         exit sub
        end if
       Case 2
        If Ucase(GetClipBoardtext) <> "TITLE" then
         Warnlog "Wrong effect has been found"
         exit sub
        end if
       Case 3
        If Ucase(GetClipBoardtext) <> "SMALL CAPITALS" then
         Warnlog "Wrong effect has been found"
         exit sub
        end if
      end select
     next i
  Case 7    '"Betonungszeichen"
  Case 8    '"Blinkend"
    EditCopy
    if Left$(GetClipBoardText,13) <> "Blinking text" then
     Warnlog "- Wrong Attribute has been found!"
    end if
  Case 9     '"Doppelzeilig"
  Case 10    '"Durchgestrichen"
    EditCopy
    if Left$(GetClipBoardText,14) <> "Strike through" then
     Warnlog "- Wrong Attribute has been found!"
    end if
    For i = 1 to 4
     Kontext "FindAndReplace"
     SearchNow.Click
     EditCopy
     if Left$(GetClipBoardText,14) <> "Strike through" then
      Warnlog "- Wrong Attribute has been found!"
     end if
    next i
  Case 11    '"Einzug"
    EditCopy
    if Left$(GetClipBoardText,6) <> "Indent" then
     Warnlog "- Wrong Attribute has been found!"
    end if
  Case 12    '"Hängende Interpunktion"
  Case 13    '"Hurenkinder"
    EditCopy
    if Left$(GetClipBoardText, 9) <> "Hurenkind" then
     Warnlog "- Wrong Attribute has been found!"
    end if
    Kontext "FindAndReplace"
    SearchNow.Click
    EditCopy
    if Left$(GetClipBoardText, 8) <> "tstststs" then
     Warnlog "- Wrong Attribute has been found!"
    end if
  Case 14    '"Kerning"
    EditCopy
    if Left$(GetClipBoardText, 8) <> "Kerning1" then
     Warnlog "- Wrong Attribute has been found!"
    end if
    Kontext "FindAndReplace"
    SearchNow.Click
    EditCopy
    if Left$(GetClipBoardText, 8) <> "Kerning2" then
     Warnlog "- Wrong Attribute has been found!"
    end if
  Case 15    '"Kontur"
    EditCopy
    if Left$(GetClipBoardText, 6) <> "Kontur" then
     Warnlog "- Wrong Attribute has been found!"
    end if
  Case 16    '"Position"
    EditCopy
    if Left$(GetClipBoardText, 4) <> "Hoch" then
     Warnlog "- Wrong Attribute has been found!"
    end if
    Kontext "FindAndReplace"
    SearchNow.Click
    EditCopy
    if Left$(GetClipBoardText, 4) <> "Tief" then
     Warnlog "- Wrong Attribute has been found!"
    end if
  Case 17    '"Registerhaltigkeit"

  Case 18    '"Relief"
  Case 19    '"Rotation"
  Case 20    '"Schattiert"
  Case 21    '"Schrift"
  Case 22    '"Schriftfarbe"
  Case 23    '"Schriftgrad"
  Case 24    '"Schriftgrad asiatische Schrift"
  Case 25    '"Schriftstärke"
  Case 26    '"Schriftstärke asiatische Schrift"
  Case 27    '"Schriftstellung"
  Case 28    '"Schriftstellung asiatische Schrift"
  Case 29    '"Schusterjungen"
  Case 30    '"Seitenvorlage"
  Case 31    '"Hyphenation"
  Case 32    '"Skalierung"
  Case 33    '"Sprache"
  Case 34    '"Sprache asiatische Schrift"
  Case 35    '"Tabulatoren"
  Case 36    '"Text Abstand"
  Case 37    '"Unterstrichen"
  Case 38    '"Verbotene Zeichen"
  Case 39    '"Vertikale Textausrichtung"
  Case 40    '"Wortweise"
  Case 41    '"Zeichenhintergrund"
  Case 42    '"Zeilenabstand"
 end select
  Kontext "FindAndReplace"
end sub

function wCheckActiveMessageboxes() as boolean
   wCheckActiveMessageboxes=True
   Kontext "Active"
   if Active.Exists then
    if Active.GetRT = 304 then
     try  ' Question if StarOffice should search at start of document
      Active.yes
      ' Exit when search has again failed
      if Active.Exists then
       if Active.GetRT = 304 then
        wCheckActiveMessageboxes = False
        Warnlog Active.Gettext
        Active.Ok
       end if
      end if
     catch
      wCheckActiveMessageboxes = False
      Warnlog Active.Gettext
      Active.Ok
     endcatch
    end if
   end if
end function

'---------------------------------------------------------
'            Unterprogramme
'---------------------------------------------------------
'---------------------------------------------------------
sub wSetFindAndReplaceToDefault(WhichDocument as integer)
    '/// <font color=#ff0000> TODO: translate this test ///'
    '/// file "input\\writer\\such_u_e.sxw" will be used <FONT> ///
    Dim i as integer
    Select Case WhichDocument
        Case 0
            Call hNewDocument  ' empty Document
        Case 1
            hFileOpenLocally(  gTesttoolPath +"writer\optional\input\search\such_u_e.sxw")
            
        case 2
            hFileOpenLocally(  gTesttoolPath +"writer\optional\input\search\attrib.sxw")
            
    end select
    kontext "FindAndReplace"
    Sleep 2
    if FindAndReplace.Exists then          'Evtl. offenes Fenster schließen
        FindAndReplace.Close
    end if
    kontext "DocumentWriter"
    DocumentWriter.Typekeys "<PAGEUP>",          'an den Anfang des Textes gehen
    DocumentWriter.Typekeys "<MOD1 HOME>"
    EditSearchAndReplace
    kontext "FindAndReplace"            'Suchen&Replace-Fenster öffnen
    if Not SearchForStyles.IsVisible then More.Click
    if SearchForStyles.IsEnabled then SearchForStyles.Uncheck
    try
        SearchFor.SetText ""
    catch
        warnlog "in 'grundzustand' writing a text to search for is not possible :-("
    endcatch
    if SimilaritySearch.IsEnabled then SimilaritySearch.Uncheck
    ' Column 1
    WholeWordsOnly.Uncheck
    Backwards.Uncheck
    RegularExpressions.Uncheck
    ' Column 2
    MatchCase.Uncheck
    if CurrentSelectionOnly.IsEnabled then CurrentSelectionOnly.Uncheck
    'Column 3
    Attributes.Click
    Kontext "Attribute"
    ' List has 32 Attributes
    For i = 1 to 32
        Auswahl.Select i
        Auswahl.Uncheck
    Next i
    Attribute.OK
    kontext "FindAndReplace"
    if NoFormat.IsEnabled then NoFormat.Click
    Kontext "FindAndReplace"
end sub

'---------------------------------------------------------
sub Vergleichen_und_ausgeben
                              'ht Such_Text mit dem,
    dim i as integer           
    dim Boxen_RT as integer    
    
    kontext
    for i = 1 to 2                      'Close all Message-, Info-, and Query-Boxes
      if active.exists (1) then           
         Boxen_RT = Active.GetRT
         select case Boxen_RT
            Case 304, 305 : Active.OK
            Case 308 : Active.Yes
         end select
    end if
    next i
    kontext "FindAndReplace"
    Sleep 2
    try
        FindAndReplace.Close
    catch
        Warnlog "Dialog is not present. Should be open!"
    endcatch
    
    wait 500
    Kontext "DocumentWriter"
    try
        BearbeitenKopieren               
    catch                     
        DocumentWriter.Typekeys "<PAGEUP>"
        DocumentWriter.Typekeys "<MOD1 END>"
        DocumentWriter.Typekeys "<SHIFT LEFT>", 15
        EditCopy
    endcatch
    wait 500
    dummy_Text = GetClipboardText
    if dummy_Text = Such_Text then
        Printlog "OK   ",Meldungs_Text
    else
        warnlog "'",Such_Text,"' was not found, but '", Dummy_Text, "' " '"With '" + Meldungs_Text+ "'
    end if
    Sleep 2
end sub

'---------------------------------------------------------  'abhängig von der Sprache wird die
sub Meldungs_Test
                     'Meldung "...bis zum Dokumentende
    dim Text_In_Meldung as string, sProduct as string

    if gOOO = true then
        sProduct = "OpenOffice.org"
    else
        sProduct = "Oracle Open Office"
    end if
    
    select case iSprache
        Case 01 : Text_In_Meldung = sProduct & " Writer has searched to the end of the document. Do you want to continue at the beginning?"
        Case 07 : Text_In_Meldung = sProduct & " Writer осуществил поиск к концу документа. Продолжить поиск с начала документа?"
        Case 03 : Text_In_Meldung = "O " & sProduct & " Writer procurou até ao fim do documento. Deseja continuar a procura no início do documento?"
        Case 31 : Text_In_Meldung = sProduct & " Writer heeft tot aan het einde van het document gezocht. Wilt u doorgaan met zoeken vanaf het begin?"
        Case 33 : Text_In_Meldung = sProduct & " Writer a atteint la fin du document. Souhaitez-vous reprendre la recherche au début du document ?"
        Case 34 : Text_In_Meldung = sProduct & " Writer ha buscado hasta el final del documento. ¿Desea continuar la búsqueda desde el principio?"
        Case 39 : Text_In_Meldung = sProduct & " Writer ha cercato fino alla fine del documento. Volete continuare dall'inizio del documento?"
        Case 46 : Text_In_Meldung = sProduct & " Writer har sökt till slutet av dokument. Vill du fortsätta sökningen i början av dokumentet?"
        Case 49 : Text_In_Meldung = sProduct & " Writer hat bis zum Ende des Dokumentes gesucht. Möchten Sie die Suche am Dokumentanfang fortsetzen?"
        Case 50 : Text_In_Meldung = sProduct & " Writer je preiskal do konca dokumenta. Ali želite nadaljevati na začetku?"
		Case 55 : Text_In_Meldung = sProduct & " Writer pesquisou até o final do documento. Deseja continuar do início?"
        Case 81 : Text_In_Meldung = sProduct & " Writer は、文書末まで検索しました。文書の始めに戻って続行しますか。"
        Case 82 : Text_In_Meldung = sProduct & " Writer가 문서의 끝까지 검색했습니다. 문서의 시작부분에서 검색을 계속하겠습니까?"
        Case 88 : Text_In_Meldung = sProduct & " Writer 已經搜尋至文件的結束位置。要從文件的起始位置繼續搜尋？"
    end select

    if Dummy_text = Text_In_Meldung then
        printlog "OK   Correct Message at end of document"
    else
        if Text_In_Meldung > "" then
            Warnlog "   No correct message at end of document"
            printlog Dummy_text
        else
            QAErrorlog "Please adapt test for this language!"
            printlog Dummy_text
        end if
    end if
end sub


function ToggleAttribute(sNumber as integer, sCheck as boolean)
    Kontext "FindAndReplace"
    Attributes.Click
    Kontext "Attribute"
    Auswahl.Select sNumber             'Check Attributes underlined
    wait 500
    if sCheck = "True" then
        Auswahl.Check
    else
        Auswahl.Uncheck
    end if
    wait 500
    Attribute.OK
    Kontext "FindAndReplace"
end function


function wGetHeadingName() as string
   ' This function gets the name 'Heading' from Stylist
   ' Needed this function for additional languages !
   FormatStylist
   Kontext "Stylist"
   if Stylist.NotExists then FormatStylist
   Absatzvorlagen.Click

   ' "Heading" should always be the 9th entry
   Gruppenliste.Select 5 ' Automatic
   wait 500
   Vorlagenliste.Select 9 ' Heading
   wGetHeadingName = Vorlagenliste.GetSeltext
   Call SetClipboard (wGetHeadingName)
   printlog "- Name of 'heading' for this language has been pasted to clipboard!"
   Stylist.Close
end function

'---------------------------------------------------------
sub   LoadIncludeFiles
   use "inc\master.inc"
   use "inc\gvariabl.inc"
   gApplication = "WRITER"
   Call GetUseFiles
end sub
