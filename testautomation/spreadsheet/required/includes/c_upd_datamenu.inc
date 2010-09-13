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
'/***********************************************************************
'*
'* owner : oliver.craemer@oracle.com
'*
'* short description : Resource Test - Data Menu
'*
'************************************************************************
'*
' #1 tDataDefineRange
' #1 tDataSelectRange
' #1 tDataSort
' #1 tDataFilterAutoFilter
' #1 tDataFilterStandardFilter
' #1 tDataFilterAdvancedFilter
' #1 tDataFilterRemoveFilter
' #1 tDataFilterHideFilter
' #1 tDataSubtotals
' #1 tDataValidity
' #1 tDataMultipleOperations
' #1 tDataTexttoColumn
' #1 tDataConsolidate
' #1 tDataGroupandOutline
' #1 tDataDataPilot
' #1 tDataRefreshRange
'*
'\***********************************************************************

sub c_upd_datamenu

    Printlog Chr(13) + "--------- Data Menu (c_upd_datamenu.inc) ---------"
    
    call tDataDefineRange
    call tDataSelectRange
    call tDataSort
    call tDataFilterAutoFilter
    call tDataFilterStandardFilter
    call tDataFilterAdvancedFilter
    call tDataFilterRemoveFilter
    call tDataFilterHideFilter
    call tDataSubtotals
    call tDataValidity
    call tDataMultipleOperations
    call tDataTexttoColumn
    call tDataConsolidate
    call tDataGroupandOutline
    call tDataDataPilot
    call tDataRefreshRange
    

end sub

'-----------------------------------------------------------

testcase tDataDefineRange
'///<u><b>Data – Define Range</b></u>

    '/// Opening new spreadsheet document for getting defined starting environment
    printlog " Opening new spreadsheet document for getting defined starting environment"
    call hNewDocument
    '/// Open 'Define Database range' dialog by 'Data – Define Range'
    printlog " Open 'Define Database range' dialog by 'Data – Define Range'"
    DataDefineRange
    Kontext "DatenbankbereichFestlegen"
    '/// Enter name 'Testrange'
    printlog " Enter name 'Testrange'"
    BereichsName.SetText "Testrange_by_TT"
    '/// Click on 'More' button
    printlog " Click on 'More' button"
    Zusaetze.Click
    DialogTest ( DatenbankbereichFestlegen )
    '/// Click on 'Shrink' button to open rangechooser
    printlog " Click on 'Shrink' button to open rangechooser"
    BereichMin.Click
    '/// Close rangechooser by click on 'Shrink' button
    printlog " Close rangechooser by click on 'Shrink' button"
    BereichMin.Click
    '/// Click on 'Add' button
    printlog " Click on 'Add' button"
    Hinzufuegen.Click
    '/// Select 'Testrange_by_TT'
    printlog " Select 'Testrange_by_TT'"
    BereichsName.Select "Testrange_by_TT"
    '/// Click on 'Modify' button
    printlog " Click on 'Modify' button"
    Aendern.Click
    '/// Select 'Testrange_by_TT'
    printlog " Select 'Testrange_by_TT'"
    BereichsName.Select "Testrange_by_TT"
    '/// Click on 'Delete' button
    printlog " Click on 'Delete' button"
    Loeschen.Click
    '/// Close messagebox with 'Yes'
    printlog " Close messagebox with 'Yes'"
    Kontext
    if Active.exists then 
        Active.Yes
    end if
    Kontext "DatenbankbereichFestlegen"
    '/// Close 'Define Database range' dialog with 'Cancel'
    printlog " Close 'Define Database range' dialog with 'Cancel'"
    DatenbankbereichFestlegen.Cancel
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tDataSelectRange
'///<u><b>Data – Select Range</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Open 'Select Database range' dialog by 'Data – Select Range'
    printlog " Open 'Select Database range' dialog by 'Data – Select Range'"
    DataSelectRange
    Kontext "DatenbankbereichWaehlen"
    DialogTest ( DatenbankbereichWaehlen )
    '/// Close dialog with 'Cancel'
    printlog " Close dialog with 'Cancel'"
    DatenbankbereichWaehlen.Cancel
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tDataSort
'///<u><b>Data – Sort</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Open 'Sort' dialog by 'Data – Sort'
    printlog " Open 'Sort' dialog by 'Data – Sort'"
    DataSort
    '/// Switch to tabpage 'Sort Criteria'
    printlog " Switch to tabpage 'Sort Criteria'"
    Kontext
    active.SetPage TabSortierkriterien
    Kontext "TabSortierkriterien"
    Call DialogTest ( TabSortierkriterien )
    '/// Verify that combobox 'Sort by' has items
    printlog " Verify that combobox 'Sort by' has items"
    If SortierenNach.GetItemCount < 1 then
        warnlog "The Sortlist is empty"
    end if
    '/// Switch to tabpage 'Options'
    printlog " Switch to tabpage 'Options'"
    Kontext
    active.SetPage TabSortierenOptionen
    Kontext "TabSortierenOptionen"
    Call DialogTest ( TabSortierenOptionen )
    '/// Verify that the 'Language' combobox has items
    printlog " Verify that the 'Language' combobox has items"
    If Sprache.GetItemCount < 1 then
        warnlog "The Languagelist is empty"
    end if
    '/// Close 'Sort' dialog with 'Cancel
    printlog " Close 'Sort' dialog with 'Cancel"
    TabSortierenOptionen.Cancel
	printlog " Select cell B3"
	Kontext ( "DocumentCalc" )
	fCalcSelectRange ( "B3" )
	printlog " Fill cells B3:B8 with numbers 1-6"
    DocumentCalc.TypeKeys "1<RETURN>2<RETURN>3<RETURN>4<RETURN>5<RETURN>6"
	printlog " Select cell C3"
	fCalcSelectRange ( "C3" )
	printlog " Fill cells C3:C8 with numbers 7-2"
    DocumentCalc.TypeKeys "7<RETURN>6<RETURN>5<RETURN>4<RETURN>3<RETURN>2"
	printlog " Select cells B5:B6"
	fCalcSelectRange ( "B5:B6" )
	printlog " Open Data Sort"
	DataSort
	Kontext ( "SortRange" )
	printlog " SortRange dialog has opened"
	printlog " Cancel Sort Range dialog"
	SortRange.Cancel
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tDataFilterAutoFilter
'///<u><b>Data – Filter – AutoFilter</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Enter 'Value' in A1, '1' in A2, '2' in A3
    printlog " Enter 'Value' in A1, '1' in A2, '2' in A3"
    DocumentCalc.TypeKeys "Value<Return>1<Return>2<Return><Up>"
    '/// Use 'AutoFilter' by 'Data – Filter – AutoFilter'
    printlog " Use 'AutoFilter' by 'Data – Filter – AutoFilter'"
    DataFilterAutoFilter
    '/// Remove 'AutoFilter' by 'Data – Filter – AutoFilter'
    printlog " Remove 'AutoFilter' by 'Data – Filter – AutoFilter'"
    DataFilterAutoFilter
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tDataFilterStandardFilter
'///<u><b>Data – Filter – Standard Filter</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Open 'Standard Filter' dialog by 'Data – Filter – Standard Filter'
    printlog " Open 'Standard Filter' dialog by 'Data – Filter – Standard Filter'"
    DataFilterStandardFilter
    Kontext "Standardfilter"
    '/// Click on 'More' button
    printlog " Click on 'More' button"
    Zusaetze.Click
    DialogTest ( Standardfilter )
    '/// Verify that the 'Field name' combobox has items
    printlog " Verify that the 'Field name' combobox has items"
    If FeldName1.GetItemCount < 1 then
        warnlog "The list is empty"
    end if
    '/// Verify that the 'Condition' combobox has items
    printlog " Verify that the 'Condition' combobox has items"
    If Bedingung1.GetItemCount < 1 then
        warnlog "The list is empty"
    end if
    '/// Verify that the 'Value' combobox has items
    printlog " Verify that the 'Value' combobox has items"
    If Wert1.GetItemCount < 1 then
        warnlog "The list is empty"
    end if
    '/// Choose first operator
    printlog " Choose first operator"
    Verknuepfung2.Select 1
    '/// Check option 'Case sensitive'
    printlog " Check option 'Case sensitive'"
    GrossKleinschreibung.Check
    '/// Check option 'Range contains column labels'
    printlog " Check option 'Range contains column labels'"
    BereicheEnthaeltSpaltenkoepfe.Check
    '/// Check option 'Regular expressions'
    printlog " Check option 'Regular expressions'"
    RegulaererAusdruck.Check
    '/// Check option 'No duplication'
    printlog " Check option 'No duplication'"
    KeineDuplikate.Check
    '/// Check option 'Copy results to'
    printlog " Check option 'Copy results to'"
    FilterergebnisNach.Check
    '/// Click on 'Shrink' button to open rangechooser
    printlog " Click on 'Shrink' button to open rangechooser"
    DialogMin.Click
    '/// Close rangechooser by click on 'Shrink' button
    printlog " Close rangechooser by click on 'Shrink' button"
    DialogMin.Click
    '/// Check option 'Keep filter criteria'
    printlog " Check option 'Keep filter criteria'"
    Persistent.Check
    '/// Close 'Standard Filter' dialog with 'Cancel'
    printlog " Close 'Standard Filter' dialog with 'Cancel'"
    Standardfilter.Cancel
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tDataFilterAdvancedFilter
'///<u><b>Data – Filter – Advanced Filter</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Open 'Advanced Filter' dialog by 'Data – Filter – Advanced Filter'
    printlog " Open 'Advanced Filter' dialog by 'Data – Filter – Advanced Filter'"
    DataFilterAdvancedFilter
    Kontext "Spezialfilter"
    '/// Click on 'More' button
    printlog " Click on 'More' button"
    Zusaetze.Click
    DialogTest ( Spezialfilter )
    '/// Click on 'Shrink' button to open rangechooser
    printlog " Click on 'Shrink' button to open rangechooser"
    FilterkriterienMin.Click
    '/// Close rangechooser by click on 'Shrink' button
    printlog " Close rangechooser by click on 'Shrink' button"
    FilterkriterienMin.Click
    '/// Check option 'Case sensitive'
    printlog " Check option 'Case sensitive'"
    GrossKleinschreibung.Check
    '/// Check if option 'Range contains column labels' is disabled
    printlog " Check if option 'Range contains column labels' is disabled"
    If BereichEnthaeltSpaltenkoepfe.IsEnabled then
        warnlog "The option is enabled"
    end if
    '/// Check option 'Regular expressions'
    printlog " Check option 'Regular expressions'"
    RegulaererAusdruck.Check
    '/// Check option 'No duplication'
    printlog " Check option 'No duplication'"
    KeineDuplikate.Check
    '/// Check option 'Copy results to'
    printlog " Check option 'Copy results to'"
    FilterergebnisAusgeben.Check
    '/// Click on 'Shrink' button to open rangechooser
    printlog " Click on 'Shrink' button to open rangechooser"
    FilterergebnisMin.Click
    '/// Close rangechooser by click on 'Shrink' button
    printlog " Close rangechooser by click on 'Shrink' button"
    FilterergebnisMin.Click
    '/// Check option 'Keep filter criteria'
    printlog " Check option 'Keep filter criteria'"
    Persistent.Check
    '/// Close 'Advanced Filter' dialog with 'Cancel'
    printlog " Close 'Advanced Filter' dialog with 'Cancel'"
    Spezialfilter.Cancel
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tDataFilterRemoveFilter
'///<u><b>Data – Filter – Remove Filter</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Enter 'Value' in A1, '1' in A2, '2' in A3
    printlog " Enter 'Value' in A1, '1' in A2, '2' in A3"
    DocumentCalc.TypeKeys "Value<Return>1<Return>2<Return><Up>"
    '/// Open 'Standard Filter' dialog by 'Data – Filter – Standard Filter'
    printlog " Open 'Standard Filter' dialog by 'Data – Filter – Standard Filter'"
    DataFilterStandardFilter
    Kontext "Standardfilter"
    '/// Set 'Field name' to 'Value'
    printlog " Set 'Field name' to 'Value'"
    Feldname1.select 2
    '/// Set 'Condition' to first entry
    printlog " Set 'Condition' to first entry"
    Bedingung1.select 1
    '/// Set 'Value' to last entry
    printlog " Set 'Value' to last entry"
    Wert1.select 2
    '/// Close 'Standard Filter' dialog with 'OK'
    printlog " Close 'Standard Filter' dialog with 'OK'"
    Standardfilter.Ok
    '/// Remove filter by 'Data – Filter – Remove Filter'
    printlog " Remove filter by 'Data – Filter – Remove Filter'"
    DataFilterRemoveFilter
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tDataFilterHideFilter
'///<u><b>Data – Filter – Hide Filter</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Enter 'Value' in A1, '1' in A2, '2' in A3
    printlog " Enter 'Value' in A1, '1' in A2, '2' in A3"
    DocumentCalc.TypeKeys "Value<Return>1<Return>2<Return><Up>"
    '/// Use 'AutoFilter' by 'Data – Filter – AutoFilter'
    printlog " Use 'AutoFilter' by 'Data – Filter – AutoFilter'"
    DataFilterAutoFilter
    '/// Hide 'AutoFilter' by 'Data – Filter – Hide Filter'
    printlog " Hide 'AutoFilter' by 'Data – Filter – Hide Filter'"
    DataFilterHideAutoFilter
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tDataSubtotals
'///<u><b>Data – Subtotals</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Open 'Subtotals' dialog by 'Data – Subtotals'
    printlog " Open 'Subtotals' dialog by 'Data – Subtotals'"
    DataSubtotals
    Kontext "Teilergebnisse"
    '/// Switch to tabpage '1st Group'
    printlog " Switch to tabpage '1st Group'"
    Kontext
    active.SetPage TabGruppe1
    Kontext "TabGruppe1"
    Call DialogTest ( TabGruppe1 )
    '/// Verify that the 'Group by' combobox has items
    printlog " Verify that the 'Group by' combobox has items"
    If GruppierenNach.GetItemCount < 1 then
        warnlog "The list is empty"
    end if
    '/// Verify that there are functions available in the  'Use function' control
    printlog " Verify that there are functions available in the  'Use function' control"
    If Berechnungsvorschrift.GetItemCount < 1 then
        warnlog "The list is empty"
    end if
    '/// Switch to tabpage '2nd Group'
    printlog " Switch to tabpage '2nd Group'"
    Kontext
    active.SetPage TabGruppe2
    Kontext "TabGruppe2"
    Call DialogTest ( TabGruppe2 )
    '/// Verify that the 'Group by' combobox has items
    printlog " Verify that the 'Group by' combobox has items"
    If GruppierenNach.GetItemCount < 1 then
        warnlog "The list is empty"
    end if
    '/// Verify that there are functions available in the  'Use function' control
    printlog " Verify that there are functions available in the  'Use function' control"
    If Berechnungsvorschrift.GetItemCount < 1 then
        warnlog "The list is empty"
    end if
    '/// Switch to tabpage '3rd Group'
    printlog " Switch to tabpage '3rd Group'"
    Kontext
    active.SetPage TabGruppe3
    Kontext "TabGruppe3"
    Call DialogTest ( TabGruppe3 )
    '/// Verify that the 'Group by' combobox has items
    printlog " Verify that the 'Group by' combobox has items"
    If GruppierenNach.GetItemCount < 1 then
        warnlog "The list is empty"
    end if
    '/// Verify that there are functions available in the  'Use function' control
    printlog " Verify that there are functions available in the  'Use function' control"
    If Berechnungsvorschrift.GetItemCount < 1 then
        warnlog "The list is empty"
    end if
    '/// Switch to tabpage 'Options'
    printlog " Switch to tabpage 'Options'"
    Kontext
    active.SetPage TabTeilergebnisseOptionen
    Kontext "TabTeilergebnisseOptionen"
    Call DialogTest ( TabTeilergebnisseOptionen )
    '/// Check option 'Page break between groups'
    printlog " Check option 'Page break between groups'"
    NeueSeiteBeiGruppenwechsel.Check
    '/// Check option 'Case sensitive'
    printlog " Check option 'Case sensitive'"
    GrossKleinschreibungBeachten.Check
    '/// Check option 'Pre-sort area according to groups'
    printlog " Check option 'Pre-sort area according to groups'"
    BereichVorherNachGruppenSortieren.Check
    '/// Check option 'Ascending'
    printlog " Check option 'Ascending'"
    Aufsteigend.Check
    '/// Check option 'Decending'
    printlog " Check option 'Decending'"
    Absteigend.Check
    '/// Check option 'Include formats'
    printlog " Check option 'Include formats'"
    FormateEinschliessen.Check
    '/// Check option 'Custom sort order'
    printlog " Check option 'Custom sort order'"
    BenutzerdefinierteSortierreihenfolge.Check
    '/// Verify that there are sortorders available
    printlog " Verify that there are sortorders available"
    If SortierreihenfolgeAnzeige.GetItemCount < 1 then
        warnlog "The list is empty"
    end if
    '/// Close 'Subtotals' dialog with 'Cancel'
    printlog " Close 'Subtotals' dialog with 'Cancel'"
    TabTeilergebnisseOptionen.Close
    '/// Close document
    printlog " Close document"
    hCloseDocument

endcase

'-----------------------------------------------------------

testcase tDataValidity
'///<u><b>Data – Validity</b></u>

    '/// Open new Spreadshet document
    printlog " Open new Spreadshet document"
    call hNewDocument
    '/// Open 'Validity' dialog by 'Data – Validity'
    printlog " Open 'Validity' dialog by 'Data – Validity'"
    DataValidity
    '/// Switch to tabpage 'Criteria'
    printlog " Switch to tabpage 'Criteria'"
    Kontext
    Active.SetPage TabWerte
    Kontext "TabWerte"
    Call DialogTest ( TabWerte,1 )
    '/// Choose 8th entry (Text length) in 'Allow' combobox
    printlog " Choose 8th entry (Text length) in 'Allow' combobox"
    Zulassen.select (8)
    Call DialogTest ( TabWerte,2 )
    '/// Choose last entry (not between) in 'Data' combobox
    printlog " Choose last entry (not between) in 'Data' combobox"
    Daten.select (8)
    Call DialogTest ( TabWerte,3 )
    '/// Choose 7th entry (List) in 'Allow' combobox
    printlog " Choose 7th entry (List) in 'Allow' combobox"
    Zulassen.select (7)
    Call DialogTest ( TabWerte,4 )
    '/// Choose 6th entry (Cell range) in 'Allow' combobox
    printlog " Choose 6th entry (Cell range) in 'Allow' combobox"
    Zulassen.select (6)
    Call DialogTest ( TabWerte,5 )
    printlog " Press Shrink button"
    SourceShrink.Click
    Kontext "TabWerte"
    printlog " Close rangechooser"
    SourceShrink.Click
    Kontext "TabWerte"
    '/// Switch to tabpage 'Input Help'
    printlog " Switch to tabpage 'Input Help'"
    Kontext
    Active.SetPage TabEingabehilfe
    Kontext "TabEingabehilfe"
    Call DialogTest ( TabEingabehilfe )
    '/// Switch to tabpage 'Error Alert'
    printlog " Switch to tabpage 'Error Alert'"
    Kontext
    Active.SetPage TabFehlermeldung
    Kontext "TabFehlermeldung"
    Call DialogTest ( TabFehlermeldung )
    '/// Choose last entry (Macro) in 'Action' combobox
    printlog " Choose last entry (Macro) in 'Action' combobox"
    Aktion.Select (4)
    '/// Click on 'Browse' button
    printlog " Click on 'Browse' button"
    Durchsuchen.Click
    Kontext "ScriptSelector"
    '/// Close 'Macro Selector' with 'Cancel'
    printlog " Close 'Macro Selector' with 'Cancel'"
    ScriptSelector.Cancel
    Kontext "TabFehlermeldung"
    '/// Close 'Validity' dialog with 'Cancel'
    printlog " Close 'Validity' dialog with 'Cancel'"
    TabFehlermeldung.Cancel
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tDataMultipleOperations
'///<u><b>Data – Multiple Operations</b></u>

    '/// Open new Spreadsheet ocument
    printlog " Open new Spreadsheet ocument"
    Call hNewDocument
    '/// Select cells A1:D4
    printlog " Select cells A1:D4"
    Call fCalcSelectRange ("A1:D4")
    '/// Open 'Multiple operations' dialog by 'Data – Multiple Operations'
    printlog " Open 'Multiple operations' dialog by 'Data – Multiple Operations'"
    DataMultipleOperations
    Kontext "Mehrfachoperationen"
    DialogTest ( Mehrfachoperationen )
    '/// Click on 'Shrink' button for Formulas' to open rangechooser
    printlog " Click on 'Shrink' button for Formulas' to open rangechooser"
    Vorgaben1.Click
    '/// Close rangechooser by click on 'Shrink' button
    printlog " Close rangechooser by click on 'Shrink' button"
    Vorgaben1.Click
    '/// Click on 'Shrink' button for 'Row input cell' to open rangechooser
    printlog " Click on 'Shrink' button for 'Row input cell' to open rangechooser"
    Vorgaben2.Click
    '/// Close rangechooser by click on 'Shrink' button
    printlog " Close rangechooser by click on 'Shrink' button"
    Vorgaben2.Click
    '/// Click on 'Shrink' button for 'Column Input Cell' to open rangechooser
    printlog " Click on 'Shrink' button for 'Column Input Cell' to open rangechooser"
    Vorgaben3.Click
    '/// Close rangechooser by click on 'Shrink' button
    printlog " Close rangechooser by click on 'Shrink' button"
    Vorgaben3.Click
    '/// Close 'Multiple operations' dialog with 'Cancel'
    printlog " Close 'Multiple operations' dialog with 'Cancel'"
    Mehrfachoperationen.Cancel
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tDataTexttoColumn
'///<u><b>Data – Text to Column</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// In cell A1 enter string  'First Second'
    printlog " In cell A1 enter string  'First Second'"
    Call fCalcSelectRange ("A1")
    DocumentCalc.TypeKeys "First Second<Return>"
    '/// Select cell A1
    printlog " Select cell A1"
    Call fCalcSelectRange ("A1")
    '/// Open 'Text to Column' dialog by 'Data – Text to Columns'
    printlog " Open 'Text to Column' dialog by 'Data – Text to Columns'"
    DataTextToColumns
    Kontext "TextImport"
    '/// Close 'Text to Column' dialog
    printlog " Close 'Text to Column' dialog"
    TextImport.Cancel
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tDataConsolidate
'///<u><b>Data – Consolidate</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Open 'Consolidate' dialog by 'Data – Consolidate'
    printlog " Open 'Consolidate' dialog by 'Data – Consolidate'"
    DataConsolidate
    '/// Click on 'More' button
    printlog " Click on 'More' button"
    Kontext "Konsolidieren"
    Zusaetze.Click
    DialogTest ( Konsolidieren )
    '/// Verify that the 'Function' combobox has items
    printlog " Verify that the 'Function' combobox has items"
    If Berechnungsvorschrift.GetItemCount < 1 then
        warnlog "The list is empty"
    end if
    '/// Click on 'Shrink' button for 'Source data range' to open rangechooser
    printlog " Click on 'Shrink' button for 'Source data range' to open rangechooser"
    ButtonQuelldaten.Click
    '/// Close rangechooser by click on 'Shrink' button
    printlog " Close rangechooser by click on 'Shrink' button"
    ButtonQuelldaten.Click
    '/// Click on 'Shrink' button for 'Copy results to' to open rangechooser
    printlog " Click on 'Shrink' button for 'Copy results to' to open rangechooser"
    ButtonAusgeben.Click
    '/// Close rangechooser by click on 'Shrink' button
    printlog " Close rangechooser by click on 'Shrink' button"
    ButtonAusgeben.Click
    '/// Close 'Consolidate' dialog with 'Cancel'
    printlog " Close 'Consolidate' dialog with 'Cancel'"
    Konsolidieren.Cancel
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tDataGroupandOutline
'///<u><b>Data – Group and Outline</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Select cells A1:D9
    printlog " Select cells A1:D9"
    Call fCalcSelectRange ("A1:D9")
    '/// Open 'Group' dialog by 'Data – Group and Outline – Group'
    printlog " Open 'Group' dialog by 'Data – Group and Outline – Group'"
    DataOutlineGroup
    Kontext "GruppierungAktivieren"
    DialogTest ( GruppierungAktivieren )
    '/// Check option 'Rows'
    printlog " Check option 'Rows'"
    Zeilen.Check
    '/// Close dialog with 'OK'
    printlog " Close dialog with 'OK'"
    GruppierungAktivieren.OK
    '/// Open 'Group' dialog by 'Data – Group and Outline – Group'
    printlog " Open 'Group' dialog by 'Data – Group and Outline – Group'"
    DataOutlineGroup
    Kontext "GruppierungAktivieren"
    '/// Check option 'Rows'
    printlog " Check option 'Rows'"
    Spalten.Check
    '/// Close dialog with 'OK'
    printlog " Close dialog with 'OK'"
    GruppierungAktivieren.OK
    '/// Hide details by 'Data – Group and Outline – Hide details'
    printlog " Hide details by 'Data – Group and Outline – Hide details'"
    DataOutlineHideDetails
    '/// Show details by 'Data – Group and Outline – Show details'
    printlog " Show details by 'Data – Group and Outline – Show details'"
    DataOutlineShowDetails
    '/// Open 'Ungroup' dialog ' Data – Group and Outline – Ungroup'
    printlog " Open 'Ungroup' dialog by ' Data – Group and Outline – Ungroup'"
    DataOutlineUngroup
    Kontext "GruppierungAufheben"
    DialogTest ( GruppierungAufheben )
    '/// Check option 'Rows'
    printlog " Check option 'Rows'"
    Spalten.Check
    '/// Close dialog with 'OK'
    printlog " Close dialog with 'OK'"
    GruppierungAufheben.OK
    '/// Ungroup data by 'Data – Group and Outline – Ungroup'
    printlog " Ungroup data by 'Data – Group and Outline – Ungroup'"
    DataOutlineUngroup
    '/// Enter '1' in A1, '2' in A2, '3' in A3
    printlog " Enter '1' in A1, '2' in A2, '3' in A3"
    Call fCalcSelectRange ("A1")
    DocumentCalc.TypeKeys "1<Return>2<Return>3<Return>"
    '/// Select cell A4 and click on Autosum button on Formularbar
    printlog " Select cell A4 and click on Autosum button on Formularbar"
    Call fCalcSelectRange ("A4")
    Kontext "RechenleisteCalc"
    Summe.Click
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "<Return>"
    '/// Select cells A1:A4
    printlog " Select cells A1:A4"
    Call fCalcSelectRange ("A1:A4")
    '/// Use 'AutoOutline' by 'Data – Group and Outline – AutoOutline'
    printlog " Use 'AutoOutline' by 'Data – Group and Outline – AutoOutline'"
    DataOutlineAutoOutline
    '/// Remove outline by 'Data – Group and Outline – Remove'
    printlog " Remove outline by 'Data – Group and Outline – Remove'"
    DataOutlineRemove
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tDataDataPilot
'///<u><b>Data – DataPilot</b></u>
    
    printlog " Close and restart the Office"
    ExitRestartTheOffice
    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Enter string 'Month' in A1, 'Value' in B1
    printlog " Enter string 'Month' in A1, 'Value' in B1"
    Call fCalcSelectRange ("A1")
    DocumentCalc.TypeKeys "Month<Right>Value<Return>"
    '/// Select A1:B1
    printlog " Select A1:B1"
    Call fCalcSelectRange ("A1:B1")
    '/// Start 'DataPilot' by 'Data – DataPilot'
    printlog " Start 'DataPilot' by 'Data – DataPilot'"
    DataDataPilotStart
    Kontext "QuelleAuswaehlen"
    DialogTest ( QuelleAuswaehlen )
    '/// Choose 'Current Selection' on 'Select Source' dialog
    printlog " Choose 'Current Selection' on 'Select Source' dialog"
    AktuelleSelektion.Check
    '/// Click 'OK' button
    printlog " Click 'OK' button"
    QuelleAuswaehlen.OK
    '/// On 'DataPilot' dialog click on 'More' button
    printlog " On 'DataPilot' dialog click on 'More' button"
    Kontext "Datenpilot"
    Zusaetze.Open
    DialogTest ( Datenpilot )
	'/// Verify that the 'Selection from' editfield is correct
	printlog "Verify that the 'Selection from' editfield has content"
    printlog "The Range is" & DataPilotRange.Gettext
    '/// Click on 'Shrink' button for 'Selection from' to open rangechooser
    printlog " Click on 'Shrink' button for 'Selection from' to open rangechooser"
    DataPilotRangeShrink.Click
    '/// Close rangechooser by click on 'Maximize' button
    printlog " Close rangechooser by click on 'Maximize' button"
    DataPilotRangeShrink.Click
    '/// Verify that the 'Results to' combobox has items
    printlog " Verify that the 'Results to' combobox has items"
    If AusgabeAb.GetItemCount < 1 then
        warnlog "The list is empty"
    end if
    '/// Click on 'Shrink' button for 'Source data range' to open rangechooser
    printlog " Click on 'Shrink' button for 'Source data range' to open rangechooser"
    Minimieren.Click
    '/// Close rangechooser by click on 'Shrink' button
    printlog " Close rangechooser by click on 'Shrink' button"
    Minimieren.Click
    '/// Check option 'Ignore empty rows'
    printlog " Check option 'Ignore empty rows'"
    LeerzeilenIgnorieren.Check
    '/// Check option 'Total columns'
    printlog " Check option 'Total columns'"
    GesamtSpalten.Check
    '/// Check option 'Add filter'
    printlog " Check option 'Add filter'"
    AddFilter.Check
    '/// Check option 'Identify categories'
    printlog " Check option 'Identify categories'"
    KategorieErkennung.Check
    '/// Check option 'Total rows'
    printlog " Check option 'Total rows'"
    GesamtZeilen.Check
    '/// Check option 'Enable drill to details'
    printlog " Check option 'Enable drill to details'"
    EnableDrillToDetails.Check
    '/// Close 'DataPilot' dialog with 'Cancel'
    printlog " Close 'DataPilot' dialog with 'Cancel'"
    DatenPilot.Cancel

    '/// Start datapilot by 'Data – DataPilot'
    printlog " Start datapilot by 'Data – DataPilot'"
    DataDataPilotStart
    Kontext "QuelleAuswaehlen"
    '/// Choose 'Data source registered in...' on 'Select Source' dialog
    printlog " Choose 'Data source registered in...' on 'Select Source' dialog"
    AngemeldeteDatenQuelle.Check
    '/// Click 'OK' button
    printlog " Click 'OK' button"
    QuelleAuswaehlen.OK
    Kontext "DatenquelleAuswaehlen"
    Call DialogTest ( DatenquelleAuswaehlen )
    '/// Count the entries in the 'Database' combobox
    printlog " Count the entries in the 'Database' combobox"
    Datenbank.GetItemCount
    '/// Count the entries in the 'Data source' combobox
    printlog " Count the entries in the 'Data source' combobox"
    Datenquelle.GetItemCount
    '/// Count the entries in the 'Type' combobox
    printlog " Count the entries in the 'Type' combobox"
    Art.GetItemCount
    '/// Close 'Select Data Source' dialog with 'Cancel'
    printlog " Close 'Select Data Source' dialog with 'Cancel'"
    DatenquelleAuswaehlen.Cancel
    '/// Close document
    printlog " Close document"
    call hCloseDocument
    
    '/// Open testdocument 'gTesttoolPath + spreadsheet\required\input\datapilot.ods'
    printlog " Open testdocument 'gTesttoolPath + spreadsheet\required\input\datapilot.ods'"
    hFileOpenLocally( gTesttoolPath & "spreadsheet\required\input\datapilot.ods" )
    '/// Select cell C34
    printlog " Select cell C34"
    Call fCalcSelectRange ( "C34" )
    '/// Open 'DataPilot' dialog by 'Data – Datapilot – Start'
    printlog " Open 'DataPilot' dialog by 'Data – Datapilot – Start'"
    DataDataPilotStart
    sleep 1
    Kontext "Datenpilot"
    '/// Select 'Products' in 'Page Fields' by 5 <TAB>
    printlog " Select 'Products' in 'Page Fields' by 5 <TAB>"
    Datenpilot.TypeKeys "<TAB><TAB><TAB><TAB><TAB>",TRUE
    '/// Click on 'Options' button
    printlog " Click on 'Options' button"
    If Optionen.IsEnabled Then
        Optionen.Click
    else
        warnlog "Option button is not enabled. Aborting"
        Datenpilot.Cancel
        Call hCloseDocument
        goto endsub
    end if
    Kontext "Datenfeld"
    Call DialogTest ( Datenfeld )
    '/// On 'Data Field' dialog choose option 'User-defined'
    printlog " On 'Data Field' dialog choose option 'User-defined'"
    Benutzerdefiniert.Check
    '/// Verify that there are functions in the combobox
    printlog " Verify that there are functions in the combobox"
    If Funktionen.GetItemCount < 1 then
        warnlog "There are no functions available"
    end if
    '/// Check option 'Show items without data'
    printlog " Check option 'Show items without data'"
    OhneDatenAnzeigen.Check
    '/// Click on 'Options' button to open 'Data Filed Options' dialog
    printlog " Click on 'Options' button to open 'Data Filed Options' dialog"
    Options.Click
    Kontext "DataFieldsOptionsDlg"
    Call DialogTest ( DataFieldsOptionsDlg )
    '/// Choose option 'Ascending'
    printlog " Choose option 'Ascending'"
    Ascending.Check
    '/// Verify that there are items in the 'Sort by' combobox
    printlog " Verify that there are items in the 'Sort by' combobox"
    If Sortby.GetItemCount < 1 then
        warnlog "The list is empty"
    end if
    '/// Check option 'Show'
    printlog " Check option 'Show'"
    Show.Check
    '/// Increase 'Show Items'
    printlog " Increase 'Show Items'"
    ShowItems.More
    '/// Count items in combobox 'From'
    printlog " Count items in combobox 'From'"
    ShowFrom.GetItemCount
    '/// Count items in combobox 'Using field'
    printlog " Count items in combobox 'Using field'"
    UsingField.GetItemCount
    '/// Close 'Data Filed Options' dialog with 'Cancel'
    printlog " Close 'Data Filed Options' dialog with 'Cancel'"
    DataFieldsOptionsDlg.Cancel
    Kontext "Datenfeld"
    '/// Close 'Data Field' dialog with 'Cancel'
    printlog " Close 'Data Field' dialog with 'Cancel'"
    Datenfeld.Cancel
    sleep 2
    Kontext "Datenpilot"
	printlog " Close 'DataPilot' dialog with 'Cancel'"
    Datenpilot.Cancel
	printlog " Reopen 'DataPilot' dialog by 'Data – Datapilot – Start'"
    DataDataPilotStart
    sleep 1
    Kontext "Datenpilot"
    '/// Select item (Sum – 2001) in 'Data Fields' by 8 <TAB>
    printlog " Select item (Sum – 2001) in 'Data Fields' by 8 <TAB>"
    Datenpilot.TypeKeys "<TAB><TAB><TAB><TAB><TAB><TAB><TAB><TAB>",TRUE
    Sleep 1
    '/// Click on 'Options' button to open 'Data Filed' dialog
    printlog " Click on 'Options' button to open 'Data Filed' dialog"
    If Optionen.IsEnabled Then
        Optionen.Click
    else
        warnlog "Option button is not enabled. Aborting"
        Datenpilot.Cancel
        Call hCloseDocument
        goto endsub
    end if
	sleep 1
    Kontext "DataFieldsFunctionDlg"
    '/// Click on 'More' button
    printlog " Click on 'More' button"
    More.Click
    Call DialogTest ( DataFieldsFunctionDlg )
    '/// Verify that the 'Function' list has items
    printlog " Verify that the 'Function' list has items"
    If FunctionList.GetItemCount < 1 then
        warnlog "The list is empty"
    end if
    '/// Choose second entry in 'Type' combobox to enable next both comboboxes
    printlog " Choose second entry in 'Type' combobox to enable next both comboboxes"
    ValueType.Select 2
    '/// Verify that the 'Base field' combobox has items
    printlog " Verify that the 'Base field' combobox has items"
    If BaseField.GetItemCount < 1 then
        warnlog "The list is empty"
    end if
    '/// Verify that the 'Base item' combobox has items
    printlog " Verify that the 'Base item' combobox has items"
    If BaseItem.GetItemCount < 1 then
        warnlog "The list is empty"
    end if
    '/// Close 'Data Field' dialog with 'Cancel'
    printlog " Close 'Data Field' dialog with 'Cancel'"
    DataFieldsFunctionDlg.Cancel
    Kontext "Datenpilot"
    '/// Close 'DataPilot' dialog with 'Cancel'
    printlog " Close 'DataPilot' dialog with 'Cancel'"
    Datenpilot.Cancel
    
    '/// Refresh 'DataPilot' by 'Data – DataPilot – Refresh'
    printlog " Refresh 'DataPilot' by 'Data – DataPilot – Refresh'"
    DataDataPilotRefresh
    
    '/// Delete 'DataPilot' by 'Data – DataPilot – Delete'
    printlog " Delete 'DataPilot' by 'Data – DataPilot – Delete'"
    DataDataPilotDelete

    '/// Close document without saving
    printlog " Close document without saving"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tDataRefreshRange
'///<u><b>Data – Refresh Range</b></u>

    '/// Load testdocument Database_Refresh
    printlog " Load testdocument Database_Refresh"
    hFileOpenLocally( gTesttoolPath & "spreadsheet\required\input\Database_Refresh.ods" )
    '/// Refresh Database by 'Data – Refresh Range'
    printlog " Refresh Database by 'Data – Refresh Range'"
    DataRefreshRange
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

