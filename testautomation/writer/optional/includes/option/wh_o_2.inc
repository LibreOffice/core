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
'* short description : Functional-Tests for Tools - Options - Html - Print & Table
'*
'\***********************************************************************

sub wh_o_2

   Call tToolsOptionsHTMLPrint 'wrn:2
   Call tToolsOptionsHTMLTable
   Call tToolsOptionsHTMLTable1

end sub

'------------------------------------------------------------------------------
testcase tToolsOptionsHTMLPrint

    dim bCheckboxStatus(12) as boolean    ' Checkbox states
    dim iListBox(1) as integer         ' ListBox
    dim iTempListBox(1) as integer
    dim bRadioBut(4) as boolean         ' RadioButtons
    dim bTempRadioBut(4) as boolean

    hFileOpenLocally(  gTesttoolPath + "writer\optional\input\options\test.html" )
    
    printlog "- Tools/Options/HTML: PrintHTML "

    printlog (" - save states ")
    ToolsOptions
    Call hToolsOptions ("HTML","PRINT")

    bCheckboxStatus(1) = Grafiken.IsChecked
    bCheckboxStatus(2) = Kontrollfelder.IsChecked
    bCheckboxStatus(3) = Hintergrund.IsChecked
    bCheckboxStatus(4) = SchwarzDrucken.IsChecked

    bCheckboxStatus(5)= Prospekt.IsChecked

    bCheckboxStatus(6)= AusDruckereinstellung.IsChecked

    iListBox(1) = Fax.GetSelIndex

    bRadioBut(1) = Keine.IsChecked
    bRadioBut(2) = NurNotizen.IsChecked
    bRadioBut(3) = Dokumentende.IsChecked
    bRadioBut(4) = Seitenende.IsChecked

    printlog (" - all states inverting ")

    if bCheckboxStatus(1) Then Grafiken.UnCheck               Else Grafiken.Check
    if bCheckboxStatus(2) Then Kontrollfelder.UnCheck         Else Kontrollfelder.Check
    if bCheckboxStatus(3) Then Hintergrund.UnCheck            Else Hintergrund.Check
    if bCheckboxStatus(4) Then SchwarzDrucken.UnCheck         Else SchwarzDrucken.Check

    if bCheckboxStatus(5) Then Prospekt.UnCheck               Else Prospekt.Check

    if bCheckboxStatus(6) Then AusDruckereinstellung.UnCheck  Else AusDruckereinstellung.Check

    if ( Keine.IsEnabled) then
        if (Keine.IsChecked = True) Then
            NurNotizen.Check
        Else
            Keine.Check
        end if
    end if

    bTempRadioBut(1) = Keine.IsChecked
    bTempRadioBut(2) = NurNotizen.IsChecked
    bTempRadioBut(3) = Dokumentende.IsChecked
    bTempRadioBut(4) = Seitenende.IsChecked

    if ( Fax.GetItemCount <> iListBox(1) ) Then Fax.Select (Fax.GetItemCount) _
    Else Fax.Select(1)
    iTempListBox(1) = Fax.GetSelIndex

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
	sleep (4)
    Call hFileSaveAsKill(gOfficepath + "user\work\printtest.html")
    Call hCloseDocument

    printlog (" - SO quit - start ")

    Call wOfficeRestart

    printlog (" - checking states ")
    ToolsOptions
    Call hToolsOptions ( "HTML" , "PRINT" )

    if ( bCheckboxStatus(1) = Grafiken.IsChecked               ) Then WarnLog "#i108219#'Graphics' state changed"
    if ( bCheckboxStatus(2) = Kontrollfelder.IsChecked         ) Then WarnLog "'Controls' state changed"
    if ( bCheckboxStatus(3) = Hintergrund.IsChecked            ) Then WarnLog "'Background' state changed"
    if ( bCheckboxStatus(4) = SchwarzDrucken.IsChecked         ) Then WarnLog "'Print black' state changed"

    if ( bCheckboxStatus(5)= Prospekt.IsChecked               ) Then WarnLog "'Brochure' state changed"

    if ( bCheckboxStatus(6)= AusDruckereinstellung.IsChecked  ) Then WarnLog "'Paper tray from printer settings' state changed"

    if ( bTempRadioBut(1) <> Keine.IsChecked       ) Then WarnLog "'None' state changed (Bug#99202)"
    if ( bTempRadioBut(2) <> NurNotizen.IsChecked  ) Then WarnLog "'Notes only' state changed (Bug#99202)"
    if ( bTempRadioBut(3) <> Dokumentende.IsChecked) Then WarnLog "'End of document' state changed"
    if ( bTempRadioBut(4) <> Seitenende.IsChecked  ) Then WarnLog "'End of page' state changed"

    if ( Fax.GetSelIndex <> iTempListBox(1) ) Then warnlog "Fax state changed"

    printlog (" - all UnCheck -> o ")

    Grafiken.UnCheck
    Kontrollfelder.UnCheck
    Hintergrund.UnCheck
    SchwarzDrucken.UnCheck

    Prospekt.UnCheck

    AusDruckereinstellung.UnCheck

    if (NurNotizen.IsChecked = True) Then Dokumentende.Check Else NurNotizen.Check

    bTempRadioBut(1) = Keine.IsChecked
    bTempRadioBut(2) = NurNotizen.IsChecked
    bTempRadioBut(3) = Dokumentende.IsChecked
    bTempRadioBut(4) = Seitenende.IsChecked

    Fax.Select(1)

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog (" - check if all UnChecked ")
    ToolsOptions
    Call hToolsOptions ( "HTML" , "PRINT" )

    if Grafiken.IsChecked       Then WarnLog "'Graphics' x"
    if Kontrollfelder.IsChecked Then WarnLog "'Controls' x"
    if Hintergrund.IsChecked    Then WarnLog "'Background' x"
    if SchwarzDrucken.IsChecked Then WarnLog "'Print black' x"

    if Prospekt.IsChecked       Then WarnLog "'Brochure' x"

    if AusDruckereinstellung.IsChecked  Then WarnLog "'Paper tray from printer settings' x"

    ' Is saved with document
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    hFileOpen( gOfficePath & "user\work\printtest.html")
    ToolsOptions
    Call hToolsOptions ( "HTML" , "PRINT" )

    if ( bTempRadioBut(1) <> Keine.IsChecked       ) Then WarnLog "'None' state changed"
    if ( bTempRadioBut(2) <> NurNotizen.IsChecked  ) Then WarnLog "'Notes only' state changed"
    if ( bTempRadioBut(3) <> Dokumentende.IsChecked) Then WarnLog "'End of document' state changed"
    if ( bTempRadioBut(4) <> Seitenende.IsChecked  ) Then WarnLog "'end of page' state changed"

    if ( Fax.GetSelIndex <> 1 ) Then WarnLog "Fax is not item 1"

    Printlog (" - all Check -> x ")

    Grafiken.Check
    Kontrollfelder.Check
    Hintergrund.Check
    SchwarzDrucken.Check

    Prospekt.Check

    AusDruckereinstellung.Check

    if ( Dokumentende.IsEnabled ) then
        if (Dokumentende.IsChecked = True) Then
            Seitenende.Check
        Else
            Dokumentende.Check
        end if
    end if

    bTempRadioBut(1) = Keine.IsChecked
    bTempRadioBut(2) = NurNotizen.IsChecked
    bTempRadioBut(3) = Dokumentende.IsChecked
    bTempRadioBut(4) = Seitenende.IsChecked

    Fax.Select(Fax.GetItemCount)

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog (" - check if all Checked ")
    ToolsOptions
    Call hToolsOptions ( "HTML" , "PRINT" )

    if not Grafiken.IsChecked                Then WarnLog "'Graphics' o"
    if not Kontrollfelder.IsChecked          Then WarnLog "'Controls' o"
    if not Hintergrund.IsChecked             Then WarnLog "'Background' o"
    if not SchwarzDrucken.IsChecked          Then WarnLog "'Print black' o"

    if not Prospekt.IsChecked                Then WarnLog "'Brochure' o"

    if not AusDruckereinstellung.IsChecked   Then WarnLog "'Paper tray from printer settings' o"

    if ( bTempRadioBut(1) <> Keine.IsChecked       ) Then WarnLog "'None' state changed"
    if ( bTempRadioBut(2) <> NurNotizen.IsChecked  ) Then WarnLog "'Notes only' state changed"
    if ( bTempRadioBut(3) <> Dokumentende.IsChecked) Then WarnLog "'End of document' state changed"
    if ( bTempRadioBut(4) <> Seitenende.IsChecked  ) Then WarnLog "'End of page' state changed"

    if ( Fax.GetSelIndex <> Fax.GetItemCount ) Then WarnLog "Fax is not item (GetItemCount)"

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Cancel

    printlog (" - restore states ")
    ToolsOptions
    Call hToolsOptions ( "HTML" , "PRINT" )

    if ( bCheckboxStatus(1) =  TRUE ) Then  Grafiken.Check                Else Grafiken.UnCheck
    if ( bCheckboxStatus(2) =  TRUE ) Then  Kontrollfelder.Check          Else Kontrollfelder.UnCheck
    if ( bCheckboxStatus(3) =  TRUE ) Then  Hintergrund.Check             Else Hintergrund.UnCheck
    if ( bCheckboxStatus(4) =  TRUE ) Then  SchwarzDrucken.Check          Else SchwarzDrucken.UnCheck

    if ( bCheckboxStatus(5) = TRUE ) Then  Prospekt.Check                Else Prospekt.UnCheck

    if ( bCheckboxStatus(7) = TRUE ) Then  AusDruckereinstellung.Check   Else AusDruckereinstellung.UnCheck

    try
        Fax.Select (iListBox(1))
    catch
        warnlog "restore fax doesn't work"
    endcatch

    if ( bRadioBut(1) = TRUE ) Then Keine.Check
    if ( bRadioBut(2) = TRUE ) Then NurNotizen.Check
    if ( bRadioBut(3) = TRUE ) Then Dokumentende.Check
    if ( bRadioBut(4) = TRUE ) Then Seitenende.Check

    Fax.Select (iListBox(1))
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsHTMLTable
    '/ Table                     ///'
    '/ -----                     ///'
    '/ tToolsOptionsHTMLTable    ///'
    '/ normal                         ///'
    '/ Function:  ///'
    '/                                ///'

    dim sMetricField(4) as string      ' MetricField
    dim sTempMetricField(4) as string
    dim bRadioBut(3) as boolean    ' RadioButton states
    dim bTempRadioBut(3) as boolean

    ' sMetricField(i) and sTempMetricField(i)
    '(1) = VerschiebenZeile
    '(2) = VerschiebenSpalte
    '(3) = EinfuegenZeile
    '(4) = EinfuegenSpalte

    ' bRadioBut(i) and bTempRadioBut(i)
    '(1) = VerhaltenFix
    '(2) = FixProportional
    '(3) = Variabel

    Call hNewDocument
    printlog ("- Tools/Options/HTML: Table ")

    printlog (" - save states ")
    ToolsOptions
    Call hToolsOptions ("HTML","Table")

    sMetricField(1) = VerschiebenZeile.GetText
    sMetricField(2) = VerschiebenSpalte.GetText
    sMetricField(3) = EinfuegenZeile.GetText
    sMetricField(4) = EinfuegenSpalte.GetText

    bRadioBut(1) = VerhaltenFix.IsChecked
    bRadioBut(2) = FixProportional.IsChecked
    bRadioBut(3) = Variabel.IsChecked

    if ((bRadioBut(1) Xor bRadioBut(2) Xor bRadioBut(3)) = False) Then Warnlog (" RadioButtons have NO init state: " + bRadioBut(1) +", BugID: 83097")

    printlog (" - all states inverting ")

    VerschiebenZeile.ToMax
    VerschiebenSpalte.ToMax
    EinfuegenZeile.ToMax
    EinfuegenSpalte.ToMax
    if ( VerschiebenZeile.GetText  = sMetricField(1) ) Then VerschiebenZeile.ToMin
    if ( VerschiebenSpalte.GetText = sMetricField(2) ) Then VerschiebenSpalte.ToMin
    if ( EinfuegenZeile.GetText    = sMetricField(3) ) Then EinfuegenZeile.ToMin
    if ( EinfuegenSpalte.GetText   = sMetricField(4) ) Then EinfuegenSpalte.ToMin
    sTempMetricField(1) = VerschiebenZeile.GetText
    sTempMetricField(2) = VerschiebenSpalte.GetText
    sTempMetricField(3) = EinfuegenZeile.GetText
    sTempMetricField(4) = EinfuegenSpalte.GetText

    FixProportional.Check

    bTempRadioBut(1) = VerhaltenFix.IsChecked
    bTempRadioBut(2) = FixProportional.IsChecked
    bTempRadioBut(3) = Variabel.IsChecked

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog (" - SO quit - start ")

    Call wOfficeRestart

    printlog (" - checking states ")
    ToolsOptions
    Call hToolsOptions ( "HTML" , "Table" )

    if ( VerschiebenZeile.GetText  <> sTempMetricField(1) ) Then WarnLog "Move cells:  'Rows' state changed BugID: 82990 "
    if ( VerschiebenSpalte.GetText <> sTempMetricField(2) ) Then WarnLog "Move cells:  'Column' state changed"
    if ( EinfuegenZeile.GetText    <> sTempMetricField(3) ) Then WarnLog "Insert Cell: 'Row' state changed"
    if ( EinfuegenSpalte.GetText   <> sTempMetricField(4) ) Then WarnLog "Insert Cell: 'Column' state changed"

    if ( bTempRadioBut(1) <> VerhaltenFix.IsChecked   ) Then WarnLog "Behaviour of rows/columns : 'Fixed' state changed"
    if ( bTempRadioBut(2) <> FixProportional.IsChecked) Then WarnLog "Behaviour of rows/columns : 'Fixed, proportional' state changed"
    if ( bTempRadioBut(3) <> Variabel.IsChecked       ) Then WarnLog "Behaviour of rows/columns : 'Variable' state changed"

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog (" - all UnCheck -> o ")
    ToolsOptions
    Call hToolsOptions ( "HTML" , "Table" )

    VerschiebenZeile.ToMin
    VerschiebenSpalte.ToMin
    EinfuegenZeile.ToMin
    EinfuegenSpalte.ToMin
    sTempMetricField(1) = VerschiebenZeile.GetText
    sTempMetricField(2) = VerschiebenSpalte.GetText
    sTempMetricField(3) = EinfuegenZeile.GetText
    sTempMetricField(4) = EinfuegenSpalte.GetText
    printlog (" More test ///'")
    VerschiebenZeile.More
    VerschiebenSpalte.More
    EinfuegenZeile.More
    EinfuegenSpalte.More
    if ( sTempMetricField(1) = VerschiebenZeile.GetText  ) Then WarnLog "'Move Cells Row' :     More-Button not working"
    if ( sTempMetricField(2) = VerschiebenSpalte.GetText ) Then WarnLog "'Move Cells Column' :  More-Button not working"
    if ( sTempMetricField(3) = EinfuegenZeile.GetText    ) Then WarnLog "'Insert Cell Row'   :  More-Button not working"
    if ( sTempMetricField(4) = EinfuegenSpalte.GetText   ) Then WarnLog "'Insert Cell Column' : More-Button not working"
    sTempMetricField(1) = VerschiebenZeile.GetText
    sTempMetricField(2) = VerschiebenSpalte.GetText
    sTempMetricField(3) = EinfuegenZeile.GetText
    sTempMetricField(4) = EinfuegenSpalte.GetText

    Variabel.Check

    bTempRadioBut(1) = VerhaltenFix.IsChecked
    bTempRadioBut(2) = FixProportional.IsChecked
    bTempRadioBut(3) = Variabel.IsChecked

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog (" - check if all UnChecked ")
    ToolsOptions
    Call hToolsOptions ( "HTML" , "Table" )

    if ( VerschiebenZeile.GetText  <> sTempMetricField(1) ) Then WarnLog "Move Cells : 'Row' is not min."
    if ( VerschiebenSpalte.GetText <> sTempMetricField(2) ) Then WarnLog "Move Cells : 'Column' is not min."
    if ( EinfuegenZeile.GetText    <> sTempMetricField(3) ) Then WarnLog "Insert Cells : 'Row' is not min."
    if ( EinfuegenSpalte.GetText   <> sTempMetricField(4) ) Then WarnLog "Insert Cells : 'Column' is not min."

    if ( bTempRadioBut(1) <> VerhaltenFix.IsChecked   ) Then WarnLog "Behaviour of rows/columns : 'Fixed' state changed"
    if ( bTempRadioBut(2) <> FixProportional.IsChecked) Then WarnLog "Behaviour of rows/columns : 'Fixed, proportional' state changed"
    if ( bTempRadioBut(3) <> Variabel.IsChecked       ) Then WarnLog "Behaviour of rows/columns : 'Variable' state changed"

    Printlog (" - all Check -> x ")

    VerschiebenZeile.ToMax
    VerschiebenSpalte.ToMax
    EinfuegenZeile.ToMax
    EinfuegenSpalte.ToMax
    sTempMetricField(1) = VerschiebenZeile.GetText
    sTempMetricField(2) = VerschiebenSpalte.GetText
    sTempMetricField(3) = EinfuegenZeile.GetText
    sTempMetricField(4) = EinfuegenSpalte.GetText
    printlog (" Less test ///'")
    VerschiebenZeile.Less
    VerschiebenSpalte.Less
    EinfuegenZeile.Less
    EinfuegenSpalte.Less
    if ( sTempMetricField(1) = VerschiebenZeile.GetText  ) Then WarnLog "'Move Cells Row' :     Less-Button not working"
    if ( sTempMetricField(2) = VerschiebenSpalte.GetText ) Then WarnLog "'Move Cells Column' :  Less-Button not working"
    if ( sTempMetricField(3) = EinfuegenZeile.GetText    ) Then WarnLog "'Insert Cell Row'   :  Less-Button not working"
    if ( sTempMetricField(4) = EinfuegenSpalte.GetText   ) Then WarnLog "'Insert Cell Column' : Less-Button not working"
    sTempMetricField(1) = VerschiebenZeile.GetText
    sTempMetricField(2) = VerschiebenSpalte.GetText
    sTempMetricField(3) = EinfuegenZeile.GetText
    sTempMetricField(4) = EinfuegenSpalte.GetText

    VerhaltenFix.Check

    bTempRadioBut(1) = VerhaltenFix.IsChecked
    bTempRadioBut(2) = FixProportional.IsChecked
    bTempRadioBut(3) = Variabel.IsChecked

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog (" - check if all Checked ")
    ToolsOptions
    Call hToolsOptions ( "HTML" , "Table" )

    if ( VerschiebenZeile.GetText  <> sTempMetricField(1) ) Then WarnLog "Move Cells : 'Row' is not max."
    if ( VerschiebenSpalte.GetText <> sTempMetricField(2) ) Then WarnLog "Move Cells : 'Column' is not min."
    if ( EinfuegenZeile.GetText    <> sTempMetricField(3) ) Then WarnLog "Insert Cells : 'Row' is not max."
    if ( EinfuegenSpalte.GetText   <> sTempMetricField(4) ) Then WarnLog "Insert Cells : 'Column' is not min."

    if ( bTempRadioBut(1) <> VerhaltenFix.IsChecked   ) Then WarnLog "Behaviour of rows/columns : 'Fixed' state changed"
    if ( bTempRadioBut(2) <> FixProportional.IsChecked) Then WarnLog "Behaviour of rows/columns : 'Fixed, proportional' state changed"
    if ( bTempRadioBut(3) <> Variabel.IsChecked       ) Then WarnLog "Behaviour of rows/columns : 'Variable' state changed"

    printlog (" - restore states ")

    VerschiebenZeile.SetText  (sMetricField(1))
    VerschiebenSpalte.SetText (sMetricField(2))
    EinfuegenZeile.SetText    (sMetricField(3))
    EinfuegenSpalte.SetText   (sMetricField(4))

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsHTMLTable1

    '/// There was a bug from OpenOffice where the Office crashed ///
    '/// This testcase check for this bug ///

    Call hNewDocument
    printlog ("- Tools/Options/Text document: Table ")

    ToolsOptions
    Call hToolsOptions ("WRITER","Table")
    VerhaltenFix.Check

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase
