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
'* owner : oliver.craemer@oracle.com
'*
'* short description : spreadsheet printrange test
'*
'*************************************************************************
'*
' #1 tDefinePrintRange
' #1 tAddPrintRange
' #1 tRemovePrintRangeByContext
' #1 tRemovePrintRangeByMenu
' #1 tNamedRangeToPrintRange
' #1 tEntireSheet
' #1 tPrintRangeMouseSelection
' #1 tEmptyRangeMessagebox
'*
'\************************************************************************
'TODO: three test cases to go

sub c_printrange

Printlog Chr(13) + "--------- printrange ---------"

    call tDefinePrintRange
    call tAddPrintRange
    call tRemovePrintRangeByContext
    call tRemovePrintRangeByMenu
    call tNamedRangeToPrintRange
    call tEntireSheet
    call tPrintRangeMouseSelection
    call tEmptyRangeMessagebox
   
End sub
'
'---------------------------------------------------------------------------

testcase tDefinePrintRange

    dim sRangeFile as string
    dim sLocalFile as string
    
    sRangeFile = convertpath ( gTesttoolpath & "spreadsheet/optional/input/printrange.sxc" )
    sLocalFile = convertpath ( gOfficepath & "user/work/defineprintrange." & sDefaultExtension )

    printlog " Load testdocument and save locally with current filter"
    call hFileOpen ( sRangeFile )
    if not hFileSaveAsWithFilterKill( sLocalFile , sDefaultCalcFilter ) then
        warnlog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        goto endsub
    end if

    printlog " Select area B2:E5"
    call fCalcSelectRange("B2:E5")
    printlog " Define print range by menu Format-Print Ranges-Define"
    FormatPrintRangesDefine
    printlog " Activate Page Preview (File-Page Preview)"
    FilePageView
    sleep(2)
    printlog " Check if there is only one page as output"
    Kontext "Previewbar"
    'now all navigation buttons should be deactivated
    'if this is true there's only one page to print
    'that's what we check now
    printlog "Check if all navigation buttons do not work"
    printlog "True -> Only one page to print (expected result)"
    printlog "False -> BUG"
    Kontext "Previewbar"
    try
       OL_SEITENANSICHT_NaechsteSeiteCalc
       warnlog "+ There seems to be more than one page -> BUG!?"
    catch
       printlog " + OK, 'next page' button does not work" 
    endcatch
    Kontext "Previewbar"
    try
       OL_SEITENANSICHT_VorherigeSeiteCalc
       warnlog "+ There seems to be more than one page -> BUG!?"
    catch
       printlog " + OK, 'previous page' button does not work" 
    endcatch
    Kontext "Previewbar"
    try
       OL_SEITENANSICHT_ErsteSeiteCalc
       warnlog "+ There seems to be more than one page -> BUG!?"
    catch
       printlog " + OK, 'first page' button does not work" 
    endcatch
    Kontext "Previewbar"
    try
       OL_SEITENANSICHT_LetzteSeiteCalc
       warnlog "+ There seems to be more than one page -> BUG!?"
    catch
       printlog " + OK, 'last page' button does not work" 
    endcatch
    printlog " Close page preview"
    FilePageView
    printlog " Enable View-Page Break Preview"
    ViewPageBreakPreview
    printlog " Select area A1:C10"
    call fCalcSelectRange("A1:C10")
    printlog " Call context Define Print Range"
    Kontext "DocumentCalc"
    DocumentCalc.MouseMove(5,5)
    sleep(2)
    DocumentCalc.OpenContextMenu
    sleep(2)
    'Because of the SunSearchToolbar, which is not available on OOo by default the count of
    'entries in the context could vary, therefor the desired entry must be searched.
    if hMenuFindSelect(26602, true, 3) = false then
          Warnlog "Context-Menu-entry `Define Print Range` was not found. Therefore the test ends."
          Call hCloseDocument
          Goto Endsub
    end if
    printlog "+  'Define Print Range' was selected in context menu"
    printlog " Activate Page Preview (File-Page Preview)"
    FilePageView
    sleep(2)
    '///+ Check if there is only one page as output</li>
    Kontext "Previewbar"
    'now all navigation buttons should be deactivated
    'if this is true there's only one page to print
    'that's what we check now
    printlog "Check if all navigation buttons do not work"
    printlog "True -> Only one page to print (expected result)"
    printlog "False -> BUG"
    Kontext "Previewbar"
    try
       OL_SEITENANSICHT_NaechsteSeiteCalc
       warnlog "+ There seems to be more than one page -> BUG!?"
    catch
       printlog " + OK, 'next page' button does not work" 
    endcatch
    Kontext "Previewbar"
    try
       OL_SEITENANSICHT_VorherigeSeiteCalc
       warnlog "+ There seems to be more than one page -> BUG!?"
    catch
       printlog " + OK, 'previous page' button does not work" 
    endcatch
    Kontext "Previewbar"
    try
       OL_SEITENANSICHT_ErsteSeiteCalc
       warnlog "+ There seems to be more than one page -> BUG!?"
    catch
       printlog " + OK, 'first page' button does not work" 
    endcatch
    Kontext "Previewbar"
    try
       OL_SEITENANSICHT_LetzteSeiteCalc
       warnlog "+ There seems to be more than one page -> BUG!?"
    catch
       printlog " + OK, 'last page' button does not work" 
    endcatch
    printlog " Close page preview"
    FilePageView
    printlog " At last check if the expected print range is selected"
    FormatPrintRangesEdit
    Kontext "DruckbereicheBearbeiten"
    if DruckBereich.GetText = "$A$1:$C$10" then
       printlog " Print range selection as expected"
    else
       warnlog "Expected print range is <$A$1:$C$10>, but we got: " & DruckBereich.GetText
    end if
    DruckbereicheBearbeiten.Cancel
    sleep(2)
    call hCloseDocument
endcase

'
'---------------------------------------------------------------------------
'
testcase tAddPrintRange

    dim sRangeFile as string
    dim sLocalFile as string
    
    sRangeFile = convertpath ( gTesttoolpath & "spreadsheet/optional/input/printrange.sxc" )
    sLocalFile = convertpath ( gOfficepath & "user/work/addprintrange" & sDefaultExtension )

    printlog " Load testdocument printrange.sxc and save locally with current filter"
    call hFileOpen ( sRangeFile )
    if ( not hFileSaveAsWithFilterKill( sLocalFile , sDefaultCalcFilter ) ) then
        warnlog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        goto endsub
    end if

    printlog " Select area H11:J15"
    ViewPageBreakPreview
    call fCalcSelectRange("H11:J15")
    printlog " Define print range by menu Format-Print Ranges-Define"
    FormatPrintRangesDefine
    printlog " Select area E41:H55"
    call fCalcSelectRange("E41:H55")
    printlog " Add print range by menu Format-Print Ranges-Add"
    FormatPrintRangesAdd
    printlog " Select area A1:F15"
    call fCalcSelectRange("A1:F20")
    printlog " Call context Add Print Range"
    Kontext "DocumentCalc"
    DocumentCalc.MouseMove(5,5)
    sleep(2)
    DocumentCalc.OpenContextMenu
    sleep(2)
    'Because of the SunSearchToolbar, which is not available on OOo by default the count of
    'entries in the context could vary, therefor the desired entry must be searched.
    if hMenuFindSelect(26651, true, 3) = false then
        Warnlog "Context-Menu-entry `Add Print Range` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    end if
    printlog "+ <Add Print Range> was selected in context menu"
    sleep(2)
    printlog " Save and Reload"
    FileSave
    sleep(5)
    FileReload
    sleep(10)
    printlog " Check if the expected print range is selected"
    FormatPrintRangesEdit
    Kontext "DruckbereicheBearbeiten"
    if DruckBereich.GetText = "$H$11:$J$15;$E$41:$H$55;$A$1:$F$20" then
       printlog "Print range selection as expected"
    else
       warnlog "Expected print range is <$H$11:$J$15;$E$41:$H$55;$A$1:$F$20>, but we got: " & DruckBereich.GetText
    end if
    DruckbereicheBearbeiten.Cancel
    sleep(1)
    printlog " Check if the expected number of pages will be print"
    FilePrint
    kontext "Active"
    if Active.exists( 2 ) then
     Active.ok
    end if
    
    Kontext "Printing"
    Kontext "TabPrintGeneral"
    PrintPages.check
    if PageRange.GetText = "1-3" then
   	   printlog "+ Pages to print as expected"
    else
   	   warnlog "Expected pages to print are <1-3>, but we got: " & PageRange.GetText
    end if
    	Kontext "Printing"
   	Printing.Cancel
    sleep(2)
    call hCloseDocument
endcase

'
'---------------------------------------------------------------------------
'
testcase tRemovePrintRangeByContext

    dim sRangeFile as string
    dim sLocalFile as string
    
    sRangeFile = convertpath ( gTesttoolpath & "spreadsheet/optional/input/printranges_3defined.ods" )
    sLocalFile = convertpath ( gOfficepath & "user/work/removeprintrange." & sDefaultExtension )
    
    printlog " Open test document '.../spreadsheet/optional/input/printranges_3defined.ods'"
    call hFileOpen ( sRangeFile )
    printlog " Save file locally '.../user/work/removeprintrange.ods'"
    if ( not hFileSaveAsWithFilterKill( sLocalFile , sDefaultCalcFilter ) ) then
        warnlog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        goto endsub
    end if
    
    printlog " Invoke Format::Print Ranges::Edit"
    FormatPrintRangesEdit
    Kontext "DruckbereicheBearbeiten"
    printlog " Check if print range is '$H$11:$J$15;$E$41:$H$55;$A$1:$F$20'"
    if DruckBereich.GetText = "$H$11:$J$15;$E$41:$H$55;$A$1:$F$20" then
        printlog "+ Print range selection as expected"
    else
        warnlog "Expected print range is <$H$11:$J$15;$E$41:$H$55;$A$1:$F$20>, but we got: " & DruckBereich.GetText
    end if
    printlog " Cancel print range dialog"
    DruckbereicheBearbeiten.Cancel
    
    Kontext "DocumentCalc"
    printlog " Click in right buttom corner"
    call gMouseClick ( 95, 95 )
    sleep(2)
    printlog " Open context menu"
    DocumentCalc.OpenContextMenu
    sleep(2)
    printlog " Select 'Undo Print Range' in context menu"
    'Because of the SunSearchToolbar, which is not available on OOo by default the count of
    'entries in the context could vary, therefor the desired entry must be searched.
    if hMenuFindSelect(26603, true, 3) = false then
        warnlog "Context-Menu-entry `Undo Print Range` was not found. Therefore the test ends."
        call hCloseDocument
        goto endsub
    end if
    printlog "+ <Undo Print Range> was selected in context menu"
    
    printlog " Invoke Format::Print Ranges::Edit"
    FormatPrintRangesEdit
    Kontext "DruckbereicheBearbeiten"
    printlog " Check if no print range is defined"
    if DruckBereich.GetText = "" then
        printlog "+ No print range selected as expected"
    else
        warnlog "Expected print range is <EMPTY>, but we got: " & DruckBereich.GetText
    end if
    DruckbereicheBearbeiten.Cancel
    
    sleep(2)
    printlog " Close document"
    call hCloseDocument

endcase

'
'---------------------------------------------------------------------------
'
testcase tRemovePrintRangeByMenu

    dim sRangeFile as string
    dim sLocalFile as string
    
    sRangeFile = convertpath ( gTesttoolpath & "spreadsheet/optional/input/printranges_3defined.ods" )
    sLocalFile = convertpath ( gOfficepath & "user/work/removeprintrange." & sDefaultExtension )

    printlog " Open test document '.../spreadsheet/optional/input/printranges_3defined.ods'"
    call hFileOpen ( sRangeFile )
    printlog " Save file locally '.../user/work/removeprintrange.ods'"
    if ( not hFileSaveAsWithFilterKill( sLocalFile , sDefaultCalcFilter ) ) then
        warnlog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        goto endsub
    end if
    
    printlog "Invoke Format::Print Ranges::Edit"
    FormatPrintRangesEdit
    Kontext "DruckbereicheBearbeiten"
    printlog " Check if print range is '$H$11:$J$15;$E$41:$H$55;$A$1:$F$20'"
    if DruckBereich.GetText = "$H$11:$J$15;$E$41:$H$55;$A$1:$F$20" then
        printlog "+ Print range selection as expected"
    else
        warnlog "Expected print range is <$H$11:$J$15;$E$41:$H$55;$A$1:$F$20>, but we got: " & DruckBereich.GetText
    end if
    printlog "Cancel print range dialog"
    DruckbereicheBearbeiten.Cancel
    
    Kontext "DocumentCalc"
    printlog " Invoke Format::Print Ranges::Remove"
    FormatPrintRangesRemove
    printlog " Invoke Format::Print Ranges::Edit"
    FormatPrintRangesEdit
    printlog " Check if no print range is defined"
    Kontext "DruckbereicheBearbeiten"
    if DruckBereich.GetText = "" then
        printlog "+ No print range selected as expected"
    else
        warnlog "Expected print range is <EMPTY>, but we got: " & DruckBereich.GetText
    end if
    printlog " Cancel print range dialog"
    DruckbereicheBearbeiten.Cancel
    printlog " Undo removal"
    EditUndo
    
    printlog " Invoke Format::Print Ranges::Edit"
    FormatPrintRangesEdit
    Kontext "DruckbereicheBearbeiten"
    printlog " Check if print range is '$H$11:$J$15;$E$41:$H$55;$A$1:$F$20'"
    if DruckBereich.GetText = "$H$11:$J$15;$E$41:$H$55;$A$1:$F$20" then
        printlog "+ Print range selection as expected"
    else
        warnlog "Expected print range is <$H$11:$J$15;$E$41:$H$55;$A$1:$F$20>, but we got: " & DruckBereich.GetText
    end if
    printlog " Cancel print range dialog"
    DruckbereicheBearbeiten.Cancel
    printlog " Redo removal"
    EditRedo
    printlog " Invoke Format::Print Ranges::Edit"
    FormatPrintRangesEdit
    Kontext "DruckbereicheBearbeiten"
    printlog " Check if no print range is defined"
    if DruckBereich.GetText = "" then
        printlog "+ No print range selected as expected"
    else
        warnlog "Expected print range is <EMPTY>, but we got: " & DruckBereich.GetText
    end if
    printlog " Cancel print range dialog"
    DruckbereicheBearbeiten.Cancel
    printlog " Undo removal"
    EditUndo
    printlog " File::Save"
    FileSave
    
    sleep(2)
    printlog " Close document"
    call hCloseDocument
    printlog " Re-open document"
    call hFileOpen ( sLocalFile )
    printlog " Invoke Format::Print Ranges::Edit"
    FormatPrintRangesEdit
    Kontext "DruckbereicheBearbeiten"
    printlog " Check if print range is '$H$11:$J$15;$E$41:$H$55;$A$1:$F$20'"
    if DruckBereich.GetText = "$H$11:$J$15;$E$41:$H$55;$A$1:$F$20" then
        printlog "+ Print range selection as expected"
    else
        warnlog "Expected print range is <$H$11:$J$15;$E$41:$H$55;$A$1:$F$20>, but we got: " & DruckBereich.GetText
    end if
    
    DruckbereicheBearbeiten.Cancel
    printlog " Cancel print range dialog"
    sleep(2)
    printlog " Close document"
    call hCloseDocument

endcase

'
'---------------------------------------------------------------------------
'
testcase tNamedRangeToPrintRange

     dim sLocalFile as STRING
         sLocalFile = convertpath ( gOfficepath & "user/work/namedRange." & sDefaultExtension )
     dim sAnotherFile as STRING
         sAnotherFile = convertpath ( gOfficepath & "user/work/namedRange1." & sDefaultExtension )
     dim sLocalFileXLS as STRING
         sLocalFileXLS = convertpath ( gOfficepath & "user/work/namedRange.xls" )
     dim sFormatXLS as STRING
         sFormatXLS = "MS Excel 97"
     dim sRangeToSelect as STRING
         sRangeToSelect = "$B$3:$D$5"
     dim sRangeName as STRING
         sRangeName = "NamedRange"
     dim sNewRangeToSelect as STRING
         sNewRangeToSelect = "$C$4:$E$6"

    printlog "Use named range as print range"

    printlog " Open new document"
    call hNewDocument
    printlog " Select range '$B$3:$D$5'"
    fCalcSelectRange ( sRangeToSelect )
    printlog " Name this range 'NamedRange'"
    fCalcNameRange ( sRangeName )
    printlog " Invoke Insert::Names::Define"     
    InsertNamesDefine
    sleep 1
    Kontext "NamenFestlegen"
    printlog " Select previously defined range"
    BereichsName.typekeys "N"
    sleep 1
    printlog " Click 'More' button"
    Zusaetze.Click
    printlog " Check 'Print range'"
    Druckbereich.Check
    printlog " Click OK"
    NamenFestlegen.OK
    printlog " Invoke Format::Print Ranges::Edit"
    FormatPrintRangesEdit
    Kontext "DruckbereicheBearbeiten"
    printlog " Select 'NamedRange' as print range"
    DruckbereichListe.select ( sRangeName )
    printlog " Check if this is item no.5"
    if NOT DruckbereichListe.getSelIndex = 5 then
        warnlog "Unexpected selection for 'print range' listbox"
    end if
    printlog " Check if the according string is '$B$3:$D$5'"
    if Druckbereich.getText <> sRangeToSelect then
        warnlog "Unexpected string in 'print range' textbox"
    end if
    printlog " Click OK"
    DruckbereicheBearbeiten.OK
    printlog " Save file locally '.../user/work/namedRange.ods'"
    if ( not hFileSaveAsWithFilterKill( sLocalFile , sDefaultCalcFilter ) ) then
        warnlog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        goto endsub
    end if
    printlog " Close document"
    call hCloseDocument
    printlog " Open document"
    call hFileOpen ( sLocalFile )
    printlog " Invoke Format::Print Ranges::Edit"
    FormatPrintRangesEdit
    Kontext "DruckbereicheBearbeiten"
    printlog " Check if item no.5 is selected as print range"
    if DruckbereichListe.getSelIndex <> 5 then
        warnlog "Unexpected selection for 'print range' listbox"
    end if
    printlog " Check if 'NamedRange' is selected as print range"
    if DruckbereichListe.getSelText <> sRangeName then
        warnlog "Unexpected selection for 'print range' listbox"
    end if
    printlog " Check if the according string is '$B$3:$D$5'"
    if Druckbereich.getText <> sRangeToSelect then
        warnlog "Unexpected string in 'print range' textbox"
    end if
    printlog " Cancel print range dialog"
    DruckbereicheBearbeiten.Cancel
    printlog " Save filein Excel97/2000/XP format '.../user/work/namedRange.xls'"
    if NOT hFileSaveAsWithFilterKill ( sLocalFileXLS , sFormatXLS ) then
        warnlog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        goto endsub
    end if
    printlog " Close document"
    call hCloseDocument
    printlog " Open document"
    call hFileOpen ( sLocalFileXLS )
    printlog " Invoke Format::Print Ranges::Edit"
    FormatPrintRangesEdit
    Kontext "DruckbereicheBearbeiten"
    printlog " Check if item no.5 is selected as print range"
    if DruckbereichListe.getSelIndex <> 5 then
        warnlog "Unexpected selection for 'print range' listbox"
    end if
    printlog " Check if the according string is '$B$3:$D$5'"
    if Druckbereich.getText <> sRangeToSelect then
        warnlog "Unexpected string in 'print range' textbox"
    end if
    printlog " Cancel print range dialog"
    DruckbereicheBearbeiten.Cancel
    printlog " Invoke Insert::Names::Define" 
    InsertNamesDefine
    sleep 1
    Kontext "NamenFestlegen"
    printlog " Select entry 'NamedRange'"
    BereichsName.typekeys "N"
    sleep 1
    printlog " Assign new range, '$C$4:$E$6'"
    ZugeordnetZu.SetText ( sNewRangeToSelect )
    printlog " Click 'More'"
    Zusaetze.Click
    printlog " Check 'Print range'"
    Druckbereich.Check
    printlog " Click OK"
    NamenFestlegen.OK
    printlog " Invoke Format::Print Ranges::Edit"
    FormatPrintRangesEdit
    Kontext "DruckbereicheBearbeiten"
    printlog " Select 'NamedRange' as print range"
    DruckbereichListe.select ( sRangeName )
    printlog " Check if this is item no.6"
    if NOT DruckbereichListe.getSelIndex = 6 then
        warnlog "Unexpected selection for 'print range' listbox"
    end if
    printlog " Check if the according string is '$C$4:$E$6'"
    if Druckbereich.getText <> sNewRangeToSelect then
        warnlog "Unexpected string in 'print range' textbox"
    end if
    printlog " Click OK"
    DruckbereicheBearbeiten.OK
    printlog " Save file locally '.../user/work/namedRange1.ods'"
    if NOT hFileSaveAsWithFilterKill ( sAnotherFile , sDefaultCalcFilter ) then
        warnlog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        goto endsub
    end if
    printlog " Close document"
    call hCloseDocument
    printlog " Open document again"
    call hFileOpen ( sAnotherFile )
    printlog " Invoke Format::Print Ranges::Edit"
    FormatPrintRangesEdit
    Kontext "DruckbereicheBearbeiten"
    printlog " Check if item no.6 is selected as print range"
    if DruckbereichListe.getSelIndex <> 6 then
        warnlog "Unexpected selection for 'print range' listbox"
    end if
    printlog " Check if 'NamedRange' is selected as print range"
    if DruckbereichListe.getSelText <> sRangeName then
        warnlog "Unexpected selection for 'print range' listbox"
    end if
    printlog " Check if the according string is '$C$4:$E$6'"
    if Druckbereich.getText <> sNewRangeToSelect then
        warnlog "Unexpected string in 'print range' textbox"
    end if
    printlog " Cancel print range dialog"
    DruckbereicheBearbeiten.Cancel
    printlog " Close document"
    call hCloseDocument

endcase

'
'---------------------------------------------------------------------------
'
testcase tEntireSheet

    dim sRangeFile as STRING
        sRangeFile = convertpath ( gTesttoolpath & "spreadsheet/optional/input/printrange.xls" )
    dim sLocalFile as STRING
        sLocalFile = convertpath ( gOfficepath & "user/work/entireSheet." & sDefaultExtension )
    dim sLocalFileXLS as STRING
        sLocalFileXLS = convertpath ( gOfficepath & "user/work/entireSheet.xls" )
    dim sFormatXLS as STRING
        sFormatXLS = "MS Excel 97" 

    printlog " Open test document '.../spreadsheet/optional/input/printrange.xls'"
    call hFileOpen ( sRangeFile )
    printlog " Save file locally '.../user/work/entireSheet.ods'"
    if NOT hFileSaveAsWithFilterKill ( sLocalFile , sDefaultCalcFilter ) then
        warnlog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        goto endsub
    end if
    printlog " Select second sheet"
    if NOT fCalcSelectSheet ( 2 ) then
        warnlog "Selecting sheet no. 2 failed -> Aborting"
        call hCloseDocument
        goto endsub
    end if
    printlog " Invoke Format::Print Ranges::Edit"
    FormatPrintRangesEdit
    Kontext "DruckbereicheBearbeiten"
    printlog " Check if item no.2 (should be 'entire sheet') is selected in print range listbox"
    if DruckbereichListe.getSelIndex = 2 then
        printlog "+ OK, print range is '- entire sheet -'"
    else
        warnlog "Unexpected print range defined -> Check this out!"
    end if
    printlog " Cancel print range dialog"
    DruckbereicheBearbeiten.Cancel
    printlog " Close document"
    call hCloseDocument
    printlog " Open document"
    call hFileOpen ( sLocalFile )
    printlog " Select second sheet"
    if NOT fCalcSelectSheet ( 2 ) then
        warnlog "Selecting sheet no. 2 failed -> Aborting"
        call hCloseDocument
        goto endsub
    end if
    printlog " Invoke Format::Print Ranges::Edit"
    FormatPrintRangesEdit
    Kontext "DruckbereicheBearbeiten"
    printlog " Check if item no.2 (should be 'entire sheet') is selected in print range listbox"
    if DruckbereichListe.getSelIndex = 2 then
        printlog "+ OK, print range is '- entire sheet -'"
    else
        warnlog "Unexpected print range defined -> Check this out!"
    end if
    printlog " Cancel print range dialog"
    DruckbereicheBearbeiten.Cancel
    printlog " Save filein Excel97/2000/XP format '.../user/work/entireSheet.xls'"
    if NOT hFileSaveAsWithFilterKill ( sLocalFileXLS , sFormatXLS ) then
        warnlog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        goto endsub
    end if
    printlog " Close document"
    call hCloseDocument
    printlog " Open document again"
    call hFileOpen ( sLocalFileXLS )
    printlog " Select second sheet"
    if NOT fCalcSelectSheet ( 2 ) then
        warnlog "Selecting sheet no. 2 failed -> Aborting"
        call hCloseDocument
        goto endsub
    end if
    printlog " Invoke Format::Print Ranges::Edit"
    FormatPrintRangesEdit
    Kontext "DruckbereicheBearbeiten"
    printlog " Check if item no.2 (should be 'entire sheet') is selected in print range listbox"
    if DruckbereichListe.getSelIndex = 2 then
        printlog "+ OK, print range is '- entire sheet -'"
    else
        warnlog "Unexpected print range defined -> Check this out!"
    end if
    printlog " Cancel print range dialog"
    DruckbereicheBearbeiten.Cancel
    printlog " Close document"
    call hCloseDocument

endcase

'
'---------------------------------------------------------------------------
'
testcase tPrintRangeMouseSelection

    dim sLocalFile as STRING
        sLocalFile = convertpath ( gOfficepath & "user/work/mouseSelection." & sDefaultExtension )
    dim sSelectedRange as STRING
    dim sSelectedRows as STRING
    dim sSelectedColumns as STRING
    
    printlog " Open new document"
    call hNewDocument
    printlog " Invoke Format::Print Ranges::Edit"
    FormatPrintRangesEdit
    printlog " Check if all settings are in initial state"
    Kontext "DruckbereicheBearbeiten"
    if NOT DruckbereichListe.getSelIndex = 1 then
        warnlog "Unexpected selection for 'print range' listbox"
    end if
    if NOT WiederholungszeileListe.getSelIndex = 1 then
        warnlog "Unexpected selection for 'rows to repeat' listbox"
    end if
    if NOT WiederholungsspalteListe.getSelIndex = 1 then
        warnlog "Unexpected selection for 'columns to repeat' listbox"
    end if
    if Druckbereich.getText <> "" then
        warnlog "Unexpected string in 'print range' textbox"
    end if
    if Wiederholungszeile.getText <> "" then
        warnlog "Unexpected string in 'rows to repeat' textbox"
    end if
    if Wiederholungsspalte.getText <> "" then
        warnlog "Unexpected string in 'columns to repeat' textbox"
    end if
    sleep(1)
    printlog " Click print range button to shrink dialog"
    DruckbereichButton.typekeys "<RETURN>"
    sleep(1)
    Kontext "DocumentCalc"
    printlog " Select print range using mouse"
    DocumentCalc.MouseDown ( 20 , 20 )
    DocumentCalc.MouseMove ( 60 , 60 )
    DocumentCalc.MouseUp ( 60 , 60 )
    sleep(1)
    Kontext "DruckbereicheBearbeiten"
    printlog " Click print range button to unshrink dialog again"
    DruckbereichButton.typekeys "<RETURN>"
    printlog " Rember to selection for later comparisson"
    sleep(1)
    sSelectedRange = Druckbereich.getText
    printlog " Click 'rows to repeat' button to shrink dialog"
    WiederholungszeileButton.typekeys "<RETURN>"
    sleep(1)
    Kontext "DocumentCalc"
    printlog " Select 'rows to repeat' using mouse"
    DocumentCalc.MouseDown ( 10 , 10 )
    DocumentCalc.MouseMove ( 15 , 30 )
    DocumentCalc.MouseUp ( 15 , 30 )
    sleep(1)
    Kontext "DruckbereicheBearbeiten"
    printlog " Click 'rows to repeat' button to unshrink dialog again"
    WiederholungszeileButton.typekeys "<RETURN>"
    sleep(1)
    printlog " Remember to selection for later comparisson"
    sSelectedRows = Wiederholungszeile.getText
    printlog " Click 'columns to repeat' button to shrink dialog"
    WiederholungsspalteButton.typekeys "<RETURN>"
    sleep(1)
    Kontext "DocumentCalc"
    printlog " Select 'columns to repeat' using mouse"
    DocumentCalc.MouseDown ( 10 , 10 )
    DocumentCalc.MouseMove ( 50 , 15 )
    DocumentCalc.MouseUp ( 50 , 15 )
    sleep(1)
    Kontext "DruckbereicheBearbeiten"
    printlog " Click 'columns to repeat' button to unshrink dialog again"
    WiederholungsspalteButton.typekeys "<RETURN>"
    sleep(1)
    printlog " Rember to selection for later comparisson"
    sSelectedColumns = Wiederholungsspalte.getText
    sleep(1)
    printlog " Click OK"
    DruckbereicheBearbeiten.OK
    sleep(1)
    printlog " Save file locally '.../user/work/mouseSelection.ods'"
    if not hFileSaveAsWithFilterKill( sLocalFile , sDefaultCalcFilter ) then
        warnlog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        goto endsub
    end if
    sleep(2)
    printlog " Close document"
    call hCloseDocument
    printlog " Open document again"
    call hFileOpen ( sLocalFile )
    printlog " Invoke Format::Print Ranges::Edit"
    FormatPrintRangesEdit
    Kontext "DruckbereicheBearbeiten"
    printlog " Check persistance of all settings"
    if NOT DruckbereichListe.getSelIndex = 3 then
        warnlog "Unexpected selection for 'print range' listbox: Item No. " & DruckbereichListe.getSelIndex
    end if
    if NOT WiederholungszeileListe.getSelIndex = 2 then
        warnlog "Unexpected selection for 'rows to repeat' listbox: Item No. " & WiederholungszeileListe.getSelIndex
    end if
    if NOT WiederholungsspalteListe.getSelIndex = 2 then
        warnlog "Unexpected selection for 'columns to repeat' listbox: Item No. " & WiederholungsspalteListe.getSelIndex
    end if
    if Druckbereich.getText <> sSelectedRange then
        warnlog "Unexpected selection in 'print range' textbox"
        qaErrorLog "Found:  " & Druckbereich.getText
        qaErrorLog "Should: " & sSelectedRange
    end if
    if Wiederholungszeile.getText <> sSelectedRows then
        warnlog "Unexpected selection in 'rows to repeat' textbox"
        qaErrorLog "Found:  " & Wiederholungszeile.getText
        qaErrorLog "Should: " & sSelectedRows
    end if
    if Wiederholungsspalte.getText <> sSelectedColumns then
        warnlog "Unexpected selection in 'columns to repeat' textbox"
        qaErrorLog "Found:  " & Wiederholungsspalte.getText
        qaErrorLog "Should: " & sSelectedColumns
    end if
    printlog " Cancel print range dialog"
    DruckbereicheBearbeiten.Cancel
    printlog " Close document"
    call hCloseDocument

endcase

'
'---------------------------------------------------------------------------
'
testcase tEmptyRangeMessagebox

    printlog "If the printrange or sheet is empty there should be a messagebox which says that there is nothing to print"
    printlog " Open new document"
    call hNewDocument
    
    printlog " Print empty document by pressing standardbar button"
    kontext "Standardbar"
    Drucken.click
    
    printlog " Check that there is a messagebox"
    kontext
    if Active.Exists(2) then
        if Active.GetRT = 304 then
            printlog "+ The messagebox " & Active.GetText & " has appeared"
            Active.OK
        else
            warnlog "Unexpected objekt: " & Active.GetText & " found!"
            try
                Active.OK
            catch
                Active.Yes
            endcatch
        end if
    else
        warnlog "There is no messagebox (#i106423#)"
        kontext "DocumentCalc"
        call hCloseDocument
        goto endsub
    end if
    
    Kontext "Printing"
    if Printing.Exists( 2 ) then
    	Printing.Cancel
    end if
    sleep 2

    printlog " Enter 1 in A1"
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys "1 <RETURN>"

    printlog " Select cell A5"
    if fGotoCell ( "A5" ) <> 0 then
            warnlog "Could not select cell I2"
            hCloseDocument
            goto endsub
    end if
    
    printlog " Define print range by menu FORMAT / PRINT RANGES / DEFINE"
    kontext "DocumentCalc"
    FormatPrintRangesDefine

    printlog " Print empty range by pressing standardbar button"
    kontext "Standardbar"
    Drucken.click
    
    printlog " Check that there is a messagebox"
    kontext
    if Active.Exists(2) then
        if Active.GetRT = 304 then
            printlog "+ The messagebox " & Active.GetText & " has appeared"
            Active.OK
        else
            warnlog "Unexpected objekt: " & Active.GetText & " found!"
            try
                Active.OK
            catch
                Active.Yes
            endcatch
        end if
    else
        warnlog "There is no messagebox"
        kontext "DocumentCalc"
        call hCloseDocument
        goto endsub
    end if

    kontext "Printing"
    if Printing.Exists( 2 ) then
    	Printing.Cancel
    end if    
    printlog "Close document"
    call hCloseDocument

endcase

