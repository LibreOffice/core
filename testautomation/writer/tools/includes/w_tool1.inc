'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_tool1.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: vg $ $Date: 2008-08-18 12:43:01 $
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
'* short description : Put description here
'*
'************************************************************************
' **                                                                **
' #1 hInhalte                                      '*** fügt den Inhalt der Zwischenablage in allen Formaten einmal ein und versucht es zu löschen
' #1 ClipboardGibsMir(Inhalt As String) As Boolean '*** Fügt aus dem Clipboard ein vorher gewähltes Format ins Dokument ein und gibt True zurück wenn erfolgreich
' #1 hObjektmalen                                  '*** Vergrössert ein vorher selektiertes Objekt mit der Maus
' #1 hClipboarden                                  '*** Ausschneiden, Kopieren und Einfuegen von Objekten
' #1 FeldbefehlAendernUndKontrollieren             '*** Selektierten Feldbefehl ändern
' #1 hEinfuegenFloatingFrame                       '*** fügt ein Frameset ein
' #1 hEinfuegenRahmen                              '*** fügt einen Rahmen ein
' #1 hEinfuegenChart                               '*** fügt ein Chart ein
' #1 hRechteckEinfuegen                            '*** zeichnet im Draw win Rechteck und fügt es übers Clipboard ins Writerdokument ein
' #1 hLegendeEinfugen                              '*** fügt über die Objektleiste eine Legende ein
' #1 hSchreibeTestinfo                             '*** öffnet Hauptdokument und ein neues Dokument, schreibt Testnamen ins Dokument
' **                                                                **
' \*******************************************************************

sub hInhalte
   Dim Anzahl as Integer
   Dim i as Integer

   Sleep 5
   try
      EditCut
   catch
      Warnlog "Unable to execute 'Edit / Cut'!. Maybe object not selected!"
      exit sub
   endcatch
   Sleep 1
   Call gMouseClick ( 99,80 )
   Sleep 1

   EditPasteSpecialWriter
   Kontext "InhaltEinfuegen"
   Anzahl = Auswahl.GetItemCount
   for i = 1 to Anzahl
      Auswahl.Select i
      InhaltEinfuegen.OK
      Sleep 1
      try
         EditDeleteContents
      catch
         Warnlog "Unable to execute Edit / Delete / Contents ! Entry: " + i
      endcatch
      Sleep 1
      Call gMouseClick ( 99,80 )
      Sleep 1
      try
         EditPasteSpecialWriter
      catch
         warnlog "Unable to execute Edit / Paste / Special!"
      endcatch
      Kontext "InhaltEinfuegen"
   next i
   InhaltEinfuegen.Cancel
end sub

' -----------------------------------------------------------------------

sub hObjektmalen
   Kontext "DocumentWriter"
   DocumentWriter.MouseDown 35, 25
   DocumentWriter.MouseMove 70, 60
   DocumentWriter.MouseUp 70, 60
   Sleep 1
end sub

' -----------------------------------------------------------------------

sub hClipboarden
   Sleep 1
   try
      EditCut
   catch
      Warnlog "Unable to Cut object. 'Edit / Cut' is disabled!"
      exit sub
   endcatch

   Sleep 1
   Call wTypeKeys "<Escape>"
   EditPaste
   Sleep 1
   EditCopy
   Call wTypeKeys "<Escape>"
   Sleep 1
   EditPaste
   Sleep 1
   Call wTypeKeys "<Delete>"
   Sleep 2
   Call wTypeKeys "<Escape>"
   Sleep 1
end sub

' -----------------------------------------------------------------------

sub FeldbefehlAendernUndKontrollieren ( Selektion as Integer )
   Dim Merk_dir_das as String

   Call wTypeKeys "<Left>"
   EditFields
   Kontext "FeldbefehlBearbeitenDokument"
   if Formatliste.IsVisible AND Formatliste.IsEnabled then
      Formatliste.Select Selektion
      Merk_dir_das=Formatliste.GetSelText
   else
      if Zahlenformat.IsVisible AND Zahlenformat.IsEnabled then
         Zahlenformat.Select Selektion
         Merk_dir_das=Zahlenformat.GetSelText
      else
         Auswahl.Select 2
      end if
   end if
   Sleep 2
   FeldbefehlBearbeitenDokument.OK
   Sleep 1

   EditFields
   Kontext "FeldbefehlBearbeitenDokument"
   if Formatliste.IsVisible AND Formatliste.IsEnabled then
      if Formatliste.GetSelText <> Merk_dir_das then Warnlog "Changing the field has not been worked. Format " + Merk_dir_das + "   Is : Format " + Formatliste.GetSelText
   else
      if Zahlenformat.IsVisible AND Zahlenformat.IsEnabled then
         if Zahlenformat.GetSelText <> Merk_dir_das then Warnlog "Changing the field has not been worked. Format " + Merk_dir_das + "   Is : Format " + Zahlenformat.GetSelText
      else
         if Auswahl.GetSelIndex <> 2 then Warnlog "Changing the field has not been worked."
      end if
   end if
   FeldbefehlBearbeitenDokument.OK
   Call wTypeKeys "<End>"
   Sleep 1
   Call wTypeKeys "<Return>"
end sub

' -----------------------------------------------------------------------

sub hEinfuegenFloatingFrame
   InsertFloatingFrame
   Kontext "TabEigenschaften"
   FrameName.SetText "Hallo"
   Inhalt.SetText ConvertPath ( gTesttoolpath + "input\desktop\frameset.htm" )
   TabEigenschaften.OK
   Sleep 10
end sub

' -----------------------------------------------------------------------

sub hEinfuegenRahmen
   InsertFrame
   Kontext
   Active.SetPage TabType
   Kontext "TabType"
   TabType.OK
   Sleep 3
end sub

' -----------------------------------------------------------------------

sub hEinfuegenChart
   InsertObjectChart
   Kontext "AutoformatDiagramm1"
   Fertigstellen.Click
   Sleep 2
end sub

' -----------------------------------------------------------------------

sub hLegendeEinfugen
   Kontext "Toolbar"
   Zeichnen.TearOff
   Kontext "Drawbar"
   Sleep 2
   Legende.Click
   Kontext "DocumentWriter"
   DocumentWriter.MouseDown 20, 20
   DocumentWriter.MouseMove 40, 40
   DocumentWriter.MouseUp 40, 40
   Kontext "Drawbar"
   Drawbar.Close
end sub

' -----------------------------------------------------------------------

sub wWriterKickBoxAway
   ' Call this routine if you don't expect a messagebox
   ' expect on an error. Closes messagebox and gives a Warnlog
   Kontext "Active"
   if Active.Exists then
      if Active.GetRT = 304 then
         Warnlog Active.Gettext
         try
            Active.Ok
         catch
            Active.Yes
         endcatch
      end if
   end if
end sub

'----------------------------------------------------------------

sub tCheckAutotextInGroup(NumberOff as integer, NumberVariant as integer)
   Dim j as integer
   for j = 1 to NumberOff
      Liste.Select j + NumberVariant
      printlog "- " & Liste.GetSelText
      Wait 500
      try
         AutoText.Ok
         Wait 100
         Kontext "Active"    ' Messagebox bei Basic-Fehler
         if Active.Exists then
            Warnlog Active.Gettext + "Group " + NumberVariant + " Entry " + j
            Active.Ok
         end if
         wait 500
         Kontext "Eingabefeld"
         if Eingabefeld.Exists then Eingabefeld.Cancel
         wait 500
         Call wTypeKeys "<Mod1 a>"
         EditCopy
         if GetClipboard = "" then Warnlog "No autotext inserted. Variant: "+ NumberVariant + "  " + j
      catch
         Warnlog "Variant "+ NumberVariant + " " + j +" has a Problem."
      endcatch
      Call wTypeKeys "<Delete>"  ' only text
      Call wMarkObjects(TRUE)
      EditAutotext
      Kontext "Active"
      if active.Exists then
         QAErrorlog Active.Gettext
         Active.Ok
      end if
      Kontext "AutoText"
      if not Vorschau_anzeigen.IsChecked then Warnlog "Preview isn't checked anymore!"
   next j
end sub

'----------------------------------------------------------------

sub hMalZeichnenMitSelektion ( xS, yS, xE, yE )
   Select Case gApplication
   Case "WRITER"
      Kontext "DocumentWriter"
      DocumentWriter.MouseDown ( xS, yS )
      DocumentWriter.MouseMove ( xE, yE )
      DocumentWriter.MouseUp   ( xE, yE )
      Sleep 1

   Case "HTML"
      Kontext "DocumentWriterWeb"
      DocumentWriterWeb.MouseDown ( xS, yS )
      DocumentWriterWeb.MouseMove ( xE, yE )
      DocumentWriterWeb.MouseUp   ( xE, yE )
      Sleep 1

   Case "MASTERDOCUMENT"
      Kontext "DocumentMasterDoc"
      DocumentMasterDoc.MouseDown ( xS, yS )
      DocumentMasterDoc.MouseMove ( xE, yE )
      DocumentMasterDoc.MouseUp   ( xE, yE )
      Sleep 1

   end select
end sub

'----------------------------------------------------------------

sub hMalZeichnen ( xS, yS, xE, yE )
   Select Case gApplication
   Case "WRITER"
      Kontext "DocumentWriter"
      DocumentWriter.MouseDown ( xS, yS )
      DocumentWriter.MouseMove ( xE, yE )
      DocumentWriter.MouseUp   ( xE, yE )
      Sleep 1
      if DocumentWriter.IsEnabled then Call wTypeKeys "<Escape>"

   Case "HTML"
      Kontext "DocumentWriterWeb"
      DocumentWriterWeb.MouseDown ( xS, yS )
      DocumentWriterWeb.MouseMove ( xE, yE )
      DocumentWriterWeb.MouseUp   ( xE, yE )
      Sleep 1
      if DocumentWriterWeb.IsEnabled then DocumentWriterWeb.TypeKeys "<Escape>"

   Case "MASTERDOCUMENT"
      Kontext "DocumentMasterDoc"
      DocumentMasterDoc.MouseDown ( xS, yS )
      DocumentMasterDoc.MouseMove ( xE, yE )
      DocumentMasterDoc.MouseUp   ( xE, yE )
      Sleep 1
      if DocumentMasterDoc.IsEnabled then DocumentMasterDoc.TypeKeys "<Escape>"

   end select
end sub

'----------------------------------------------------------------

sub sMenufunktionen(Fuer_Was as string)
   Dim i as integer

   Kontext "FormControls"
   Sleep 3
   for i = 1 to 20
      if ControlProperties.IsEnabled then
         ControlProperties.Click          ' Formularfunktion über Toolbox abschalten
         i=21
      else
         Sleep 1
         if i >= 20 then 
         	if gPlatform = "osx" then
         		Warnlog "#i82427#-Mac OS X (Aqua): Drawing function does not draw anything"
         	else
	         	Warnlog "Button (" + i + ")" + Fuer_Was + " is disabled "
         	end if
         end if
      end if
   next i

   Kontext
   try
      FormatControl
      Sleep 1
      Kontext "ControlPropertiesDialog"
      if ControlPropertiesDialog.Exists then
         ControlPropertiesDialog.Close
      else
         FormatControl
         Wait 500
         Kontext "ControlPropertiesDialog"
         if ControlPropertiesDialog.Exists then
            ControlPropertiesDialog.Close
         else
            Warnlog "  - Controlfieldproperties "+ Fuer_Was +" from Menu not available!"
         end if
      end if
      printlog Fuer_Was
   catch
        if gPlatform = "osx" then
            Warnlog "#i82427#-Mac OS X (Aqua): Drawing function does not draw anything"
        else
            Warnlog "  - Controlfieldproperties "+ Fuer_Was +" from Menu not available!"
        end if         
   endcatch

   Call wTypeKeys "<Escape>"
   Kontext "FormControls"
 end sub

'----------------------------------------------------------------

sub sAutopilot_Groupelement
   Kontext "AutopilotGroupelement"
   if AutopilotGroupelement.Exists then
      printlog "  -Autopilot-Group-Box Page 1"
      Kontext "TabGroupBoxData"
      Call DialogTest ( TabGroupBoxData )
      Names.Settext "Optionsfeld 1"
      if AddTo.IsEnabled then
         AddTo.Click
      else
         Warnlog "Entry couldn't be overtaken in 1st Window of the Group Box-Autopilot!"
         Kontext "AutopilotGroupelement"
         CancelButton.Click
      end if
      Sleep 3
      Kontext "AutopilotGroupelement"
      if NextButton.IsEnabled then
         NextButton.Click
      else
         Warnlog "In 1st Window the 'Next'-button is disabled!"
         CancelButton.Click
      end if
      Sleep 3
      printlog "  -Autopilot-Group-Box Page 2"
      Kontext "TabDefaultFieldSelection"
      Call DialogTest ( TabDefaultFieldSelection )
      Kontext "AutopilotGroupelement"
      if NextButton.IsEnabled then
         NextButton.Click
      else
         Warnlog "In 2nd Window the 'Next'-button is disabled!!"
         AutopilotGroupelement.Cancel
      end if
      Sleep 3
      printlog "  -Autopilot-Group-Box Page 3"
      Kontext "TabDatabaseValues"
      Call DialogTest ( TabDatabaseValues )
      Kontext "AutopilotGroupelement"
      if NextButton.IsEnabled then
         NextButton.Click
      else
         Warnlog "In 3rd Window the 'Next'-button is disabled!"
         AutopilotGroupelement.Cancel
      end if
      Sleep 3
      printlog "  -Autopilot-Group-Box Page 4"
      Kontext "TabCreateOptionGroup"
      Call DialogTest ( TabCreateOptionGroup )
      Kontext "AutopilotGroupelement"
      if CreateButton.IsEnabled then
         CreateButton.Click
      else
         Warnlog "In 4th Window the 'Create'-button is disabled!"
         AutopilotGroupelement.Cancel
      end if
   else
      Warnlog "- Autopilot for Group Box didn't appear!"
   end if
   Sleep 2
end sub

'----------------------------------------------------------------

sub sAutopilot_ListboxCombobox(WhichBox as String)
   Kontext "AutoPilotListComboBox"
   if AutoPilotListComboBox.Exists then
      printlog "  -Autopilot-Listbox Page 1"
      Kontext "TabListBoxData"
      if TabListboxData.Exists then
         Call DialogTest( TabListBoxData )
         DataSource.Select 1
         if Table.GetItemCount > 0 then
            Table.Select 1
         else
            Kontext "Messagebox"
			Do
                if Messagebox.Exists then
                    Warnlog Messagebox.Gettext
                    Messagebox.Ok
                else
                    Warnlog "No table selectable!"
                    exit do
                end if
            Loop
            Kontext "AutoPilotListComboBox"
            CancelButton.Click
            exit sub
         end if
      end if
      Sleep 5
      Kontext "AutoPilotListComboBox"
      if NextButton.IsEnabled then
         NextButton.Click
         printlog "  -Autopilot-Listbox Page 2"
         Kontext "TabTableSelection"
         Call DialogTest ( TabTableSelection )
         if TableSelection.GetItemCount > 0 then
            TableSelection.Select 1
            Kontext "AutoPilotListComboBox"
            if NextButton.IsEnabled then
               NextButton.Click
               printlog "  -Autopilot-Listbox Page 3"
               Kontext "TabFieldSelection"
               Call DialogTest ( TabFieldSelection )
               ExistingFields.Select 1
               Kontext "AutoPilotListComboBox"
               if NextButton.IsEnabled then
                  NextButton.Click
                  Select Case Ucase(WhichBox)
                  Case "LISTBOX"
                     printlog "  -Autopilot-Listbox Page 4"
                     Kontext "TabFieldLink"
                     Call DialogTest ( TabFieldLink )
                  Case "COMBO"
                     Kontext "TabDatabaseField"
                     Call DialogTest ( TabDatabaseField )
                  end select
                  Kontext "AutoPilotListComboBox"
                  CancelButton.Click
               else
                  Warnlog "Next-Button on 3rd Tabpage disabled!"
                  CancelButton.Click
                  exit sub
               end if
            else
               Warnlog "Next-Button on Second Tabpage disabled!"
               CancelButton.Click
               exit sub
            end if
         else
            Warnlog "Unable to select table on second tabpage!"
            CancelButton.Click
            exit sub
         end if
      else
         Warnlog "Next-Button on first Tabpage disabled!"
         CancelButton.Click
         Exit sub
      end if
   else
      Warnlog "Autopilot is not up!"
   end if
end sub

'----------------------------------------------------------------

sub sAutopilotTableElement
   Kontext "AutopilotTableElement"
   if AutopilotTableElement.Exists then
      Call DialogTest ( AutopilotTableElement )
      Kontext "TabListBoxData"
      if TabListboxData.Exists then
         Call DialogTest( TabListBoxData )
         DataSource.Select 1
         if Table.GetItemCount > 0 then
            Table.Select 1
         else
            Kontext "Messagebox"
			if Messagebox.Exists then
				Warnlog Messagebox.Gettext
				Messagebox.Ok
			else
				Warnlog "No table selectable!"
			end if
            Kontext "AutopilotTableElement"
            CancelButton.Click
            exit sub
         end if
      end if
	  Kontext "AutopilotTableElement"
	  CancelButton.Click
   else
      Warnlog "Autopilot Table Element is not up"
   end if
end sub

' *****************************************************
' ************* Subroutines for Fields ****************
' *****************************************************

function uTabDokument as Integer
   Dim i : Dim j : Dim k : Dim Ende : Dim NichtMachen
   Dim Zaehler
   Dim AlterWert as String : Dim Naechster as String

   printlog "  - Tabpage Document"
   try
      Kontext
      Active.SetPage TabDokumentFeldbefehle
      Kontext "TabDokumentFeldbefehle"
      for i=1 to Feldtyp.GetItemCount
         Feldtyp.Select i
         Sleep 1
         if Formatliste.IsVisible then
            if Formatliste.IsEnabled then
               if Auswahl.IsEnabled then
                  for j=1 to Auswahl.GetItemCount
                     Auswahl.Select j
                     for k=1 to Formatliste.GetItemCount
                        Formatliste.Select k
                        Zaehler=Zaehler+1
                        Einfuegen.Click
                     next k
                     k=0
                  next j
                  j=0
               else
                  for k=1 to Formatliste.GetItemCount
                     Formatliste.Select k
                     Zaehler=Zaehler+1
                     Einfuegen.Click
                  next k
                  k=0
               end if
            else
               if Auswahl.IsEnabled then
                  for j=1 to Auswahl.GetItemCount
                     Auswahl.Select j
                     Zaehler=Zaehler+1
                     Einfuegen.Click
                  next j
                  j=0
               end if
            end if
         else
            if Zahlenformat.IsEnabled then
               if Auswahl.IsEnabled then
                  for j=1 to Auswahl.GetItemCount
                     Auswahl.Select j
                     for k=1 to Zahlenformat.GetItemCount
                        Zahlenformat.Select k
                        BeseitigeTabZahlen
                        Zaehler=Zaehler+1
                        Einfuegen.Click
                     next k
                     k=0
                  next j
                  j=0
               else
                  for k=1 to Zahlenformat.GetItemCount
                     Zahlenformat.Select k
                     BeseitigeTabZahlen
                     Zaehler=Zaehler+1
                     Einfuegen.Click
                  next k
                  k=0
               end if
            else
               if Auswahl.IsEnabled then
                  for j=1 to Auswahl.GetItemCount
                     Auswahl.Select j
                     Zaehler=Zaehler+1
                     Einfuegen.Click
                  next j
                  j=0
               end if
            end if
         end if
         Sleep 1
      next i
      printlog "    here are " + Zaehler + " Fields selectable"
   catch
      Exceptlog
      Warnlog "Tabpage Document: Error in variant " + i + "  " + j + "  " + k
   endcatch
   uTabDokument = Zaehler
end function

' -----------------------------------------------------------------------

function uTabReferenzen as Integer
   Dim i : Dim Zaehler

   printlog "  - Tabpage Reference"
   try
      Zaehler = 0 : i=0
      Kontext
      Active.SetPage TabReferenzen
      Kontext "TabReferenzen"
      if NOT NameFeld.IsEnabled then Feldtyp.Select 2
      NameFeld.SetText "Testtool"
      TabReferenzen.OK
      Feldtyp.Select 1
      if NameFeld.IsEnabled then Feldtyp.Select 2
      for i=1 to Formatliste.GetItemCount
         Formatliste.Select i
         Zaehler=Zaehler+1
         if Einfuegen.IsEnabled then Einfuegen.Click
         Sleep 1
      next i
      printlog "  - here are " + Zaehler + " Fields selectable"
   catch
      Exceptlog
      Warnlog "Tabpage References: Error in Listentry Type " + i
   endcatch
   uTabReferenzen = Zaehler
end function

' -----------------------------------------------------------------------

function uTabFunktionen as Integer
   Dim i : Dim j : Dim k : Dim Ende : Dim NichtMachen
   Dim Zaehler, x as integer
   Dim weiter as Boolean
   Dim AlterWert as String : Dim Naechster as String

   printlog "  - Tabpage Function"
   try
      Zaehler = 0 : i=0 : j=0 : k= 0
      Kontext
      Active.SetPage TabFunktionen
      Kontext "TabFunktionen"
      for i=1 to Feldtyp.GetItemCount
         Feldtyp.Select i
         weiter = TRUE
         if Formatliste.IsVisible AND Formatliste.IsEnabled then
            for j=1 to FormatListe.GetItemCount
               FormatListe.Select j
               Platzhalter.SetText "Ein"
               Hinweis.SetText "Test"
               Zaehler=Zaehler+1
               if Einfuegen.IsEnabled then Einfuegen.Click
               weiter = FALSE
            next j
            j=0
         else
            if Dann.IsVisible AND Dann.IsEnabled then
               Bedingung.SetText "Hallo"
               Dann.SetText "Ein"
               Sonst.SetText "Test"
               Zaehler=Zaehler+1
               if Einfuegen.IsEnabled then Einfuegen.Click
               weiter = FALSE
            end if

            if weiter = TRUE then
               if ( Hinweis.IsVisible AND Hinweis.IsEnabled ) AND NOT Namefeld.IsEnabled then
                  Hinweis.SetText "Testtool"
                  Zaehler=Zaehler+1
                  Einfuegen.Click
                  Kontext "Eingabefeld"
                  if Eingabefeld.Exists then
                     Eingabefeld.OK
                  else
                     Warnlog "The textfield is missing in Variant " + i + " " + j + " " + k + " !"
                  end if
                  Kontext "TabFunktionen"
                  weiter = FALSE
               end if
            end if

            if weiter = TRUE then
               if MakroButton.IsEnabled then
                    MakroButton.Click
                    ' if no JRE is installed a messagebox appears
                    Do
                        x = x + 1
                        Kontext "Active"
                        if Active.Exists then
                            if Active.GetRT = 304 then
                                if x = 1 then Warnlog Active.Gettext    
                                Active.Ok
                            else
                                exit do
                            end if                    
                        else
                            exit do
                        end if
                    Loop                              
                  Kontext "Makro"
                  if Ausfuehren.IsEnabled then
                     Ausfuehren.Click
                  else
                     Makro.Cancel
                  end if
                  Kontext "TabFunktionen"
                  Zaehler=Zaehler+1
                  if Einfuegen.IsEnabled then Einfuegen.Click
                  weiter = FALSE
               end if
            end if

            if weiter = TRUE then
               if Bedingung.IsEnabled AND TextEinfuegen.IsEnabled then
                  Bedingung.SetText "Ein"
                  TextEinfuegen.SetText "Test"
                  Zaehler=Zaehler+1
                   if Einfuegen.IsEnabled then Einfuegen.Click
                   weiter = FALSE
               end if
            end if

            if weiter = TRUE then
               if Bedingung.IsEnabled AND NOT Wert.IsEnabled then
                  Bedingung.SetText "Ein Test"
                  Zaehler=Zaehler+1
                   if Einfuegen.IsEnabled then Einfuegen.Click
                   weiter = FALSE
               end if
            end if
         end if
         Sleep 1
      next i
      printlog "  - here are " + Zaehler + " Fields selectable"
   catch
      Exceptlog
      Warnlog "Tabpage Functions: Error in entry type " + i + " "  + j
   endcatch
   uTabFunktionen = Zaehler
end function

' -----------------------------------------------------------------------

function uTabDokumentinfo as Integer
   Dim i : Dim j : Dim k : Dim Ende : Dim NichtMachen
   Dim Zaehler
   Dim AlterWert as String : Dim Naechster as String

   printlog "  - Tabpage DocInformation"
   try
      Zaehler = 0 : i=0 : j=0 : k= 0
      Kontext
      Active.SetPage TabDokumentInfoFeldbefehle
      Kontext "TabDokumentInfoFeldbefehle"

      for i=1 to 20
         AlterWert = Feldtyp.GetText
         Feldtyp.TypeKeys "<Down>"
         Naechster = Feldtyp.GetText
         Feldtyp.TypeKeys "<Up>"
         Feldtyp.TypeKeys "<Add>"
         Feldtyp.TypeKeys "<Down>"
         if Feldtyp.GetText <> Naechster then NichtMachen=i
         Feldtyp.TypeKeys "<Up>"
         Feldtyp.TypeKeys "<Down>"
         if Feldtyp.GetText = AlterWert then
            Ende = i
            i=21
         end if
         Sleep 1
      next i
      Feldtyp.TypeKeys "<Up>", Ende+1

      i=0
      for i=1 to Ende         ' Alle Feldtypen
         Sleep 1
         if Auswahl.IsEnabled then
            for j=1 to Auswahl.GetItemCount
               Auswahl.Select j
               if Zahlenformat.IsEnabled then
                  for k=1 to Zahlenformat.GetItemCount
                     Zahlenformat.Select k
                     Zaehler = Zaehler + 1
                     Kontext "TabZahlen"
                     if TabZahlen.Exists then TabZahlen.Cancel
                     Kontext "TabDokumentinfoFeldbefehle"
                     if Einfuegen.IsEnabled then Einfuegen.Click
                  next k
                  k=0
               else
                  Zaehler = Zaehler + 1
                  if Einfuegen.IsEnabled then Einfuegen.Click
               end if
            next j
            j=0
         else
            if Zahlenformat.IsEnabled then
               for k=1 to Zahlenformat.GetItemCount
                  Zahlenformat.Select k
                  Zaehler = Zaehler + 1
                  Kontext "TabZahlen"
                  if TabZahlen.Exists then TabZahlen.Cancel
                  Kontext "TabDokumentinfoFeldbefehle"
                  if Einfuegen.IsEnabled then Einfuegen.Click
               next k
               k=0
            else
               Zaehler = Zaehler + 1
               if Einfuegen.IsEnabled then Einfuegen.Click
            end if
         end if
         if i = NichtMachen-1 then
            Feldtyp.TypeKeys "<Down><Down>"
         else
            Feldtyp.TypeKeys "<Down>"
         end if
         Sleep 1
      next i
      printlog "  - here are " + Zaehler + " Fields selectable"
   catch
      Exceptlog
      Warnlog "Tabpage Variables: Error on type entry " + i + " " + j + " " + k
   endcatch
   uTabDokumentinfo = Zaehler
end function

' -----------------------------------------------------------------------

function uTabVariablen as Integer
   Dim i : Dim j : Dim k : Dim Ende : Dim NichtMachen
   Dim Zaehler
   Dim AlterWert as String : Dim Naechster as String
   printlog "  - Tabpage Variables"

   try
      Zaehler = 0 : i=0 : j=0 : k=0
      Kontext
      Active.SetPage TabVariablen
      Kontext "TabVariablen"

      for i=1 to Feldtyp.GetItemCount
         Feldtyp.Select i
         if i=9 then
            Feldtyp.TypeKeys "<Up>"
            Feldtyp.TypeKeys "<Down>"
         end if
         if NameText.IsEnabled then NameText.SetText "Hallo"
         if Wert.IsEnabled then Wert.SetText "189882"

         if Auswahl.IsEnabled AND ( Formatliste.IsEnabled OR Zahlenformat.IsEnabled )then
            for j=1 to Auswahl.GetItemCount
               Auswahl.Select j
               if FormatListe.IsVisible AND FormatListe.IsEnabled then
                  for k=1 to Formatliste.GetItemCount
                     Formatliste.Select k
                     Zaehler = Zaehler + 1
                     if Einfuegen.IsEnabled then Einfuegen.Click
                  next k
                  k=0
               else
                  if Zahlenformat.IsVisible AND Zahlenformat.IsEnabled then
                     for k=1 to Zahlenformat.GetItemCount
                        Zahlenformat.Select k
                        Zaehler = Zaehler + 1
                        Kontext "TabZahlen"
                        if TabZahlen.Exists then TabZahlen.OK
                        Kontext "TabVariablen"
                        if Einfuegen.IsEnabled then Einfuegen.Click
                        Kontext "Eingabefeld"
                        if Eingabefeld.Exists then Eingabefeld.OK
                        Kontext "TabVariablen"
                     next k
                     k=0
                  end if
               end if
            next j
            j=0
         else
            if FormatListe.IsVisible AND FormatListe.IsEnabled then
               for k=1 to Formatliste.GetItemCount
                  Formatliste.Select k
                  Zaehler =  Zaehler + 1
                  if Einfuegen.IsEnabled then Einfuegen.Click
                     Kontext "TabZahlen"
                     if TabZahlen.Exists then TabZahlen.OK
                     Kontext "TabVariablen"
               next k
               k=0
            else
               if Zahlenformat.IsVisible AND Zahlenformat.IsEnabled then
                  for k=1 to Zahlenformat.GetItemCount
                     Zahlenformat.Select k
                     Zaehler =  Zaehler + 1
                     Kontext "TabZahlen"
                     if TabZahlen.Exists then TabZahlen.OK
                     Kontext "TabVariablen"
                     if Einfuegen.IsEnabled then Einfuegen.Click
                     Kontext "Eingabefeld"
                     if Eingabefeld.Exists then Eingabefeld.OK
                     Kontext "TabVariablen"
                  next k
                  k=0
               end if
            end if
         end if
         Sleep 1
      next i
      i=0
      printlog "  - here are " + Zaehler + " Fields selectable"
   catch
      Exceptlog
      Warnlog "Tabpage Variables: Error on entry type" + i + " " + j + " " + k
   endcatch
   uTabVariablen = Zaehler
end function

' -----------------------------------------------------------------------

function uTabDatenbank as Integer
   Dim i : Dim j : Dim k : Dim Ende : Dim NichtMachen
   Dim Zaehler
   Dim AlterWert as String : Dim Naechster as String

   printlog "  - Tabpage Database"
   try
      Zaehler = 0 : i=0 : j=0 : k= 0
      Kontext
      Active.SetPage TabDatenbank
      Kontext "TabDatenbank"

      Datenbankauswahl.TypeKeys "<Up>", 5
      for i=1 to Feldtyp.GetItemCount
         j=0 : k=0
         Feldtyp.Select i
         if Satznummer.IsEnabled then Satznummer.SetText "Na_mal_sehen"
         for j=1 to 20
            AlterWert = Datenbankauswahl.GetText
            if j=1 then
               Datenbankauswahl.TypeKeys "<Add>"
               Datenbankauswahl.TypeKeys "<Down>"
               Datenbankauswahl.TypeKeys "<Add>"
               Datenbankauswahl.TypeKeys "<Down>"
            else
               Datenbankauswahl.TypeKeys "<Down>"
            end if
            if Datenbankauswahl.GetText = AlterWert then
               if j=1 then printlog "Address book contains no data"
               j=21
            else
               if Selbstdefiniert.IsEnabled AND Selbstdefiniert.IsChecked then
                  for k=1 to Formatliste.GetItemCount
                     Formatliste.Select k
                     Zaehler = Zaehler + 1
                     if Einfuegen.IsEnabled then Einfuegen.Click
                  next k
               else
                  Zaehler = Zaehler + 1
                  if Einfuegen.IsEnabled then Einfuegen.Click
               end if
            end if
         next j
         Datenbankauswahl.TypeKeys "<up>", 20
         Sleep 1
      next i

      printlog "  - here are " + Zaehler + " Fields selectable"
   catch
      Exceptlog
      Warnlog "Tabpage Database: Error on field entry " + i + " " + j + " " + k
   endcatch
   uTabDatenbank = Zaehler
end function

' -----------------------------------------------------------------------

sub BeseitigeTabZahlen
   Kontext "TabZahlen"
   if TabZahlen.Exists then TabZahlen.Cancel
   Kontext "TabDokumentFeldbefehle"
end sub
