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
'* short description : Funcitonal-Test for Tools -Options - HTML - Cursor.
'*
'\***********************************************************************

sub wh_o_3
   Call tToolsOptionsHTMLInsert

' on view2 with ex content
   Call tToolsOptionsHTMLCursor
end sub

'------------------------------------------------------------------------------
testcase tToolsOptionsHTMLInsert

'/ Insert                     ///'
'/ ------                     ///'
'/ tToolsOptionsHTMLInsert    ///'
'/ normal                         ///'
'/ Function:  ///'
'/ Function:  not yet: Tables ///'
'/                                ///'

   dim irgendwas(8) as boolean    ' Checkbox states

'(1) = Automatisch
'
'(2) = Ueberschrift
'(3) = Wiederholen                       depends on Ueberschrift
'(4) = NichtTrennen
'(5) = Umrandung
'
'(6) = AutomatischeZahlenerkennung
'(7) = Zahlenformaterkennung             depends on AutomatischeZahlenerkennung
'(8) = AutomatischeAusrichtung           depends on AutomatischeZahlenerkennung

   call hNewDocument
    printlog "'///- Tools/Options/HTML: Insert ///"

    printlog ("'/// - save states ///")
    ToolsOptions
    hToolsOptions ("HTML","Table")

    irgendwas(2) = Ueberschrift.IsChecked
    irgendwas(3) = Wiederholen.IsChecked
    irgendwas(5) = Umrandung.IsChecked

    irgendwas(6) = AutomatischeZahlenerkennung.IsChecked
    irgendwas(7) = Zahlenformaterkennung.IsChecked
    irgendwas(8) = AutomatischeAusrichtung.IsChecked

    printlog ("'/// - all states inverting ///")

'      if irgendwas(2) Then Ueberschrift.UnCheck                Else Ueberschrift.Check
    Ueberschrift.Check
    if irgendwas(3) Then Wiederholen.UnCheck                 Else Wiederholen.Check
    if irgendwas(5) Then Umrandung.UnCheck                   Else Umrandung.Check

'      if irgendwas(6) Then AutomatischeZahlenerkennung.UnCheck Else AutomatischeZahlenerkennung.Check
    AutomatischeZahlenerkennung.Check
    if irgendwas(7) Then Zahlenformaterkennung.UnCheck       Else Zahlenformaterkennung.Check
    if irgendwas(8) Then AutomatischeAusrichtung.UnCheck     Else AutomatischeAusrichtung.Check

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("'/// - SO quit - start ///")

    call wOfficeRestart

    printlog ("'/// - checking states ///")
    ToolsOptions
    hToolsOptions ( "HTML" , "Table" )

'      If ( irgendwas(2) = Ueberschrift.IsChecked                ) Then WarnLog "Ueberschrift state changed"
      If ( Ueberschrift.IsChecked                = False ) Then WarnLog "Ueberschrift state changed"
      If ( irgendwas(3) = Wiederholen.IsChecked                 ) Then WarnLog "Wiederholen state changed"
      If ( irgendwas(5) = Umrandung.IsChecked                   ) Then WarnLog "Umrandung state changed"

'      If ( irgendwas(6) = AutomatischeZahlenerkennung.IsChecked ) Then WarnLog "AutomatischeZahlenerkennung state changed"
      If ( AutomatischeZahlenerkennung.IsChecked = False ) Then WarnLog "AutomatischeZahlenerkennung state changed"
      If ( irgendwas(7) = Zahlenformaterkennung.IsChecked       ) Then WarnLog "Zahlenformaterkennung state changed"
      If ( irgendwas(8) = AutomatischeAusrichtung.IsChecked     ) Then WarnLog "AutomatischeAusrichtung state changed"

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("'/// - all UnCheck -> o ///")
    ToolsOptions
    hToolsOptions ( "HTML" , "Table" )

    Ueberschrift.UnCheck
'      Wiederholen.UnCheck
    Umrandung.UnCheck

    AutomatischeZahlenerkennung.UnCheck
'      Zahlenformaterkennung.UnCheck
'      AutomatischeAusrichtung.UnCheck

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

'These Options also have another location

'/// 2.Location of Options: InsertTableWriter get's checked too ///'

  Printlog "'///    - check in InsertTableHTML ///"
   Call hNewDocument
   InsertTableWriter
   Kontext "TabelleEinfuegenWriter"

      If ( Ueberschrift.IsChecked          ) Then WarnLog "Ueberschrift x"
'      If ( UeberschriftWiederholen.IsChecked ) Then WarnLog "Wiederholen x"
      If ( Umrandung.IsChecked           ) Then WarnLog "Umrandung x"

   TabelleEinfuegenWriter.OK

'/// Function: Input in tables ///'

   Kontext "DocumentWriter"

    DocumentWriter.TypeKeys "<Down>1<Tab><Left>"

    FormatParagraph

    Kontext
   Active.SetPage TabAusrichtungAbsatz
   Kontext "TabAusrichtungAbsatz"
   if NOT Links.IsChecked then Warnlog "Die automatische Ausrichtung der Zahl wurde trotzdem durchgeführt!"

    TabAusrichtungAbsatz.Cancel


   FormatNumberFormat

    Kontext "Zahlenformat"
   If Kategorie.GetSelIndex <> 11 then Warnlog "Das Zahlenformat ist nicht Text geblieben!"

    Zahlenformat.Cancel


   Call hCloseDocument

'.................

  printlog ("'/// - check if all UnChecked ///")

  ' Workaround issue i48383 Slot get's executed on backing window
  Kontext "DocumentWriter"
  DocumentWriter.typekeys ("a")

    ToolsOptions
   hToolsOptions ( "HTML" , "Table" )

      If Ueberschrift.IsChecked                Then WarnLog "Ueberschrift x"
'      If Wiederholen.IsChecked                 Then WarnLog "Wiederholen x"
      If Umrandung.IsChecked                   Then WarnLog "Umrandung x"

      If AutomatischeZahlenerkennung.IsChecked Then WarnLog "AutomatischeZahlenerkennung x"
'      If Zahlenformaterkennung.IsChecked       Then WarnLog "Zahlenformaterkennung x"
'      If AutomatischeAusrichtung.IsChecked     Then WarnLog "AutomatischeAusrichtung x"

      Printlog ("'/// - all Check -> x ///")

      Ueberschrift.Check
      Wiederholen.Check
      Umrandung.Check

      AutomatischeZahlenerkennung.Check
      Zahlenformaterkennung.Check
      AutomatischeAusrichtung.Check

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

'These Options also have another location

  Printlog "'///    - check in InsertTableWriter ///"
   hNewDocument
   InsertTableWriter
   Kontext "TabelleEinfuegenWriter"

      If ( Ueberschrift.IsChecked                = FALSE ) Then WarnLog "Ueberschrift o"
      If ( UeberschriftWiederholen.IsChecked                 = FALSE ) Then WarnLog "Wiederholen o"
      If ( Umrandung.IsChecked                   = FALSE ) Then WarnLog "Umrandung o"

   TabelleEinfuegenWriter.Ok

   Kontext "DocumentWriter"
   DocumentWriter.TypeKeys "<Down>1<Tab><Left>"
   FormatParagraph
   Kontext
   Active.SetPage TabAusrichtungAbsatz
   Kontext "TabAusrichtungAbsatz"
   if NOT Rechts.IsChecked then Warnlog "Alignment of number didn't change to: right!"
   TabAusrichtungAbsatz.Cancel

   FormatNumberFormat
   Kontext "Zahlenformat"
   If Kategorie.GetSelIndex <> 3 then Warnlog "numberformat didn't get changed!"
   Zahlenformat.Cancel

   Call hCloseDocument

'.......................

  printlog ("'/// - check if all Checked ///")

  ' Workaround issue i48383 Slot get's executed on backing window
  Kontext "DocumentWriter"
  DocumentWriter.typekeys ("a")
  
    ToolsOptions
    hToolsOptions ( "HTML" , "Table" )

      If ( Ueberschrift.IsChecked                = FALSE ) Then WarnLog "Ueberschrift o"
      If ( Wiederholen.IsChecked                 = FALSE ) Then WarnLog "Wiederholen o"
      If ( Umrandung.IsChecked                   = FALSE ) Then WarnLog "Umrandung o"

      If ( AutomatischeZahlenerkennung.IsChecked = FALSE ) Then WarnLog "AutomatischeZahlenerkennung o"
      If ( Zahlenformaterkennung.IsChecked       = FALSE ) Then WarnLog "Zahlenformaterkennung o"
      If ( AutomatischeAusrichtung.IsChecked     = FALSE ) Then WarnLog "AutomatischeAusrichtung o"

  printlog ("'/// - restore states ///")

      If ( irgendwas(2) = TRUE ) Then Ueberschrift.Check                Else Ueberschrift.UnCheck
      If ( irgendwas(3) = TRUE ) Then Wiederholen.Check                 Else _
      if Ueberschrift.IsChecked Then Wiederholen.UnCheck
      If ( irgendwas(5) = TRUE ) Then Umrandung.Check                   Else Umrandung.UnCheck

      AutomatischeZahlenerkennung.Check
    if AutomatischeZahlenerkennung.IsChecked then
        if ( irgendwas(7) = TRUE ) Then Zahlenformaterkennung.Check
        if ( irgendwas(8) = TRUE ) Then AutomatischeAusrichtung.Check
    end if
    if ( irgendwas(6) = TRUE ) Then AutomatischeAusrichtung.Check

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------------------------------------------------

testcase tToolsOptionsHTMLCursor

   dim irgendwas(17) as boolean    ' Checkbox states

'(10) = Absatzenden
'(11) = WeicheTrenner
'(12) = Leerzeichen
'(13) = GeschLeerzeichen
'(14) = Tabulatoren
'(15) = Umbrueche
'(16) = VersteckterText
'(17) = VersteckteAbsatze

   hNewDocument
  printlog "'///- Tools/Options/Writer: View 2 ///"

  printlog ("'/// - save states ///")
    ToolsOptions
    hToolsOptions ("HTML","FORMATTINGAIDS")

      irgendwas(10) = Absatzenden.IsChecked
      irgendwas(11) = WeicheTrenner.IsChecked
      irgendwas(12) = Leerzeichen.IsChecked
      irgendwas(13) = GeschLeerzeichen.IsChecked
      irgendwas(15) = Umbrueche.IsChecked

      printlog ("'/// - all states inverting ///")

      if irgendwas(10) Then Absatzenden.UnCheck Else Absatzenden.Check
      if irgendwas(11) Then WeicheTrenner.UnCheck Else WeicheTrenner.Check
      if irgendwas(12) Then Leerzeichen.UnCheck Else Leerzeichen.Check
      if irgendwas(13) Then GeschLeerzeichen.UnCheck Else GeschLeerzeichen.Check
      if irgendwas(15) Then Umbrueche.UnCheck Else Umbrueche.Check

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("'/// - SO quit - start ///")

    call wOfficeRestart

    printlog ("'/// - checking states ///")
    ToolsOptions
    hToolsOptions ( "HTML" , "FORMATTINGAIDS" )

      If ( irgendwas(10) = Absatzenden.IsChecked  ) Then  WarnLog "Absatzenden state changed"
      If ( irgendwas(11) = WeicheTrenner.IsChecked  ) Then  WarnLog "WeicheTrenner state changed"
      If ( irgendwas(12) = Leerzeichen.IsChecked  ) Then  WarnLog "Leerzeichen state changed"
      If ( irgendwas(13) = GeschLeerzeichen.IsChecked  ) Then  WarnLog "GeschLeerzeichen state changed (Bug#99202)"
      If ( irgendwas(15) = Umbrueche.IsChecked  ) Then  WarnLog "Umbrueche state changed (Bug#99202)"

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("'/// - all UnCheck -> o ///")
    ToolsOptions
    hToolsOptions ( "HTML" , "FORMATTINGAIDS" )

      Absatzenden.UnCheck
      WeicheTrenner.UnCheck
      Leerzeichen.UnCheck
      GeschLeerzeichen.UnCheck
      Umbrueche.UnCheck

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

  printlog ("'/// - check if all UnChecked ///")
    ToolsOptions
    hToolsOptions ( "HTML" , "FORMATTINGAIDS" )

      If Absatzenden.IsChecked Then  WarnLog "Absatzenden x"
      If WeicheTrenner.IsChecked Then  WarnLog "WeicheTrenner x"
      If Leerzeichen.IsChecked Then  WarnLog "Leerzeichen x"
      If GeschLeerzeichen.IsChecked Then  WarnLog "GeschLeerzeichen x"
      If Umbrueche.IsChecked Then  WarnLog "Umbrueche x"

      Printlog ("'/// - all Check -> x ///")

      Absatzenden.Check
      WeicheTrenner.Check
      Leerzeichen.Check
      GeschLeerzeichen.Check
      Umbrueche.Check

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

  printlog ("'/// - check if all Checked ///")
    ToolsOptions
    hToolsOptions ( "HTML" , "FORMATTINGAIDS" )

      If True <> Absatzenden.IsChecked Then  WarnLog "Absatzenden o BugID: 82942 OS"
      If True <> WeicheTrenner.IsChecked Then  WarnLog "WeicheTrenner o"
      If True <> Leerzeichen.IsChecked Then  WarnLog "Leerzeichen o"
      If True <> GeschLeerzeichen.IsChecked Then  WarnLog "GeschLeerzeichen o"
      If True <> Umbrueche.IsChecked Then  WarnLog "Umbrueche o"

  printlog ("'/// - restore states ///")

      If ( irgendwas(10) = TRUE ) Then  Absatzenden.Check Else Absatzenden.UnCheck
      If ( irgendwas(11) = TRUE ) Then  WeicheTrenner.Check Else WeicheTrenner.UnCheck
      If ( irgendwas(12) = TRUE ) Then  Leerzeichen.Check Else Leerzeichen.UnCheck
      If ( irgendwas(13) = TRUE ) Then  GeschLeerzeichen.Check Else GeschLeerzeichen.UnCheck
      If ( irgendwas(15) = TRUE ) Then  Umbrueche.Check Else Umbrueche.UnCheck

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

