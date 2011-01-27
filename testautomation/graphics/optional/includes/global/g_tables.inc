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
'/******************************************************************
'*
'* owner : wolfram.garten@oracle.com
'*
'* short description : Testing of impress tables
'*
'\******************************************************************

'Variables:

dim iItemCount AS integer           'Count variable for combobox
dim iItemIndex AS integer           'Counter for/next

'-------------------------------------------------------------------------------

testcase tiInsertTableUsingMenu

qaerrorlog "#i116013# No table toolbar when inserting table via insert/table"
goto endsub

    printlog "Inserting table into doc using the menu."
    Call hNewDocument
    printlog "Switching between Draw/Impress"
    if ( gApplication = "IMPRESS" ) then
        Kontext "Document Impress"
        DocumentImpress.UseMenu
        hMenuSelectNr(4)
        hMenuSelectNr(14)
    else
        Kontext "Document Draw"
        DocumentDraw.UseMenu
        hMenuSelectNr(4)
        hMenuSelectNr(11)
    endif


    printlog "Checking if insert Table dialog comes up."
    Kontext "InsertTableImpress"
    if InsertTableImpress.Exists then
        printlog "InsertTable Dialog is up."
        InsertTableImpress.OK
    else
        warnlog "InsertTableDialog did not come up!"
    endif

    printlog "Checking for availability of column and row boxes."
    if NumberOfColumns.IsEnabled AND NumberOfRows.IsEnabled then
        printlog "Columns and rows boxes available."
    else
        warnlog "One or both spinboxes not active!"
    endif

    printlog "Checking max and min for Columns."
    NumberOfColumns.ToMax
    if NumberOfColumns.GetText = "75" then
        printlog "Max is 75 Columns."
    else
        warnlog "Max Columns cannot be reached!"
    endif

    NumberOfColumns.ToMin
    if NumberOfColumns.GetText ="1" then
        printlog "Min is 1 column."
    else
        warnlog "Min column is wrong!"
    endif

    printlog "Checking max and min for Rows."
    NumberOfRows.ToMax
    if NumberOfRows.GetText = "75" then
        printlog "Max is 75 Rows."
    else
        warnlog "Max Rows cannot be reached!"
    endif

    NumberOfRows.ToMin
    if NumberOfRows.GetText ="1" then
        printlog "Min is 1 Row."
    else
        warnlog "Min Row is wrong!"
    endif

    printlog "Inserting Table now, 5 columns ,2 rows."
    NumberOfColumns.SetText "5"
    NumberOfRows.SetText "2"
    InsertTableImpress.OK
    Kontext "DocumentImpress"

    printlog "Checking if table is inserted."
    Kontext "TableObjectbar"
    if TableObjectbar.Exists(1) then
        printlog "TableObjectBar is up, Table inserted."
    else
        warnlog " TableObjectBar not up !"
    endif
    TableObjectbar.Close

    printlog " Close document."
    Call hCloseDocument

endcase 'tiInsertTableUsingMenu

'-------------------------------------------------------------------------------

testcase tiInsertTableUsingButton

qaerrorlog "#i116013# No table toolbar when inserting table via insert/table"
goto endsub

    printlog " Inserting table using the button in standard toolbar."
    Call hNewDocument
    printlog "Open New Document."
    kontext "Standardbar"

    printlog "Checking if button is available."
    try
        Table.Click
    catch
        warnlog "Table button in standardbar is not available!"
    endcatch
    printlog "Clicking on insert table button in standardbar."

    printlog " Checking if insert Table dialog comes up."
    Kontext "InsertTableImpress"
    if InsertTableImpress.Exists then
        printlog "InsertTable Dialog is up."
    else
        warnlog "InsertTableDialog did not come up!"
    endif
    printlog "Inserting Table by clicking OK."
    InsertTableImpress.OK

    printlog "Checking if table is inserted."
    Kontext "TableObjectbar"
    if TableObjectbar.Exists(1) then
        printlog "TableObjectBar is up, Table inserted."
    else
        warnlog "TableObjectBar not up!"
    endif
    TableObjectbar.Close
    printlog " Close document."
    Call hCloseDocument

endcase 'tiInsertTableUsingButton

'-------------------------------------------------------------------------------
testcase tiInsertTableUsingToolbarbutton

    printlog " Inserting table using the button in table toolbar."
    Call hNewDocument
    printlog "Clicking on button in toolbar."
    Call hToolbarSelect ("Table", true)
    Kontext "TableObjectBar"
    if TableObjectbar.Exists(1) then
        printlog "TableObjectBar is up."
    else
        warnlog "TableObjectBar not up!"
    endif
    Table.Click

    printlog " Checking if insert Table dialog comes up."
    Kontext "InsertTableImpress"
    if InsertTableImpress.Exists then
        printlog "InsertTable Dialog is up."
    else
        warnlog "InsertTableDialog did not come up!"
    endif

    printlog "Inserting Table..."
    InsertTableImpress.OK

    printlog " Checking if dialog comes up."
    Kontext "TableObjectbar"
    if TableObjectbar.Exists(1) then
        printlog "Success, Table inserted."
    else
        warnlog " Table Dialog did not came up!"
    endif
    TableObjectbar.Close

    printlog " Close document."
    Call hCloseDocument

endcase 'tiInsertTableUsingToolbarbutton

'-------------------------------------------------------------------------------
testcase tiTableObjectBar

qaerrorlog "#i116013# No table toolbar when inserting table via insert/table"
goto endsub

    printlog "Testing TableObjectBar."
    printlog "Inserting table into doc using the menu."
    Call hNewDocument
    printlog "Switching between Draw/Impress"
    if ( gApplication = "IMPRESS" ) then
        DocumentImpress.UseMenu
        Kontext "Document Impress"
        hMenuSelectNr(4)
        hMenuSelectNr(14)
    else
        Kontext "Document Draw"
        DocumentDraw.UseMenu
        hMenuSelectNr(4)
        hMenuSelectNr(11)
    endif

    Kontext "InsertTableImpress"
    printlog "Inserting table."
    if InsertTableImpress.Exists(1) then
        InsertTableImpress.OK
    else
        sleep (1)
        InsertTableImpress.OK
    endif
    printlog "Selecting table for geeting the TableObjectBar."
    printlog "Switching between Draw/Impress"
    if ( gApplication = "IMPRESS" )then
        Kontext "DocumentImpress"
        DocumentImpress.TypeKeys "<TAB>",3
    else
        Kontext "DocumentDraw"
        DocumentDraw.TypeKeys "<MOD1 A>"
    endif

    printlog " Checking Table button."
    Kontext "TableObjectBar"
    if TableObjectBar.Exists then
        printlog "Table bar visible."
    else
        warnlog "Table bar is not up!"
        ViewToolbarsTable
    endif

    printlog "Checking LinienStil."
    LinienStil.TearOff
    Kontext "TB_Umrandungsstil"
    WaitSlot(1000)
    if TB_Umrandungsstil.Exists then
        printlog "LineStyle Box is up."
        TB_Umrandungsstil.Close
    else
        warnlog "LineStyleBox is not up!"
    endif
    Kontext "TableObjectBar"

    TableObjectBar.Close
    if ( gApplication = "IMPRESS" ) then
        Kontext "DocumentImpress"
    else
        Kontext "DocumentDraw"
    endif
    Call hToolbarSelect ("Table", true)
    Kontext "TableObjectBar"

    printlog "Checking LineColorOfTheBorder."
    LineColorOfTheBorder.TearOff
    Kontext "TB_Farbe"
    WaitSlot(1000)
    if TB_Farbe.Exists then
        printlog "LineColorOfTheBorder is up."
        TB_Farbe.Close
    else
        warnlog "LineColorOfTheBorder is not up!"
    endif
    Kontext "TableObjectBar"
    TableObjectBar.Close
    if ( gApplication = "IMPRESS" ) then
        Kontext "DocumentImpress"
    else
        Kontext "DocumentDraw"
    endif
    Call hToolbarSelect ("Table", true)
    Kontext "TableObjectBar"

    printlog "Checking Borderstyle."
    Umrandung.TearOff
    Kontext "TB_Umrandung"
    if TB_Umrandung.Exists then
        printlog " BordersTB is up."
        TB_Umrandung.Close
    else
        warnlog "BordersTB is not up!"
    endif
    if ( gApplication = "IMPRESS" ) then
        Kontext "DocumentImpress"
    else
        Kontext "DocumentDraw"
    endif

    printlog "Checking merge cells."
    printlog "Inserting something into cell and select."
    if ( gApplication = "IMPRESS" ) then
        DocumentImpress.TypeKeys ("a")
        DocumentImpress.TypeKeys "<SHIFT RIGHT>"
    else
        DocumentDraw.TypeKeys ("a")
        DocumentDraw.TypeKeys "<SHIFT RIGHT>"
    endif
    Kontext "TableObjectBar"
    if TableObjectBar.NotExists(2) then ViewToolbarsTable

    printlog( "Merge cells" )
    Wait( 1000 )
    ZellenVerbinden.Click

    printlog( "Split cells" )
    Wait( 1000 )
    ZelleTeilen.Click

    Kontext "ZellenTeilen"
    if ( ZellenTeilen.exists( 2 ) ) then
        
        printlog "SplitCells dialog open."

        printlog( "Set number to maximum" )
        Anzahl.ToMax
        printlog "Maximum SplitCellCount is " & Anzahl.GetText & "."

        printlog( "Set number to minimum" )
        Anzahl.ToMin
        printlog "Minimum SplitCellCount is " & Anzahl.GetText & "."

        printlog( "<Horizontal> should be checked" )
        if ( Horizontal.IsChecked ) then
            printlog "Horizontally is clicked."
        else
            warnlog "Horizontally should be clicked by default!"
        endif

        printlog( "Distribute/Split equally" )
        GleichmaessigTeilen.Check

        printlog( "Check <Vertically>" )
        Vertikal.Check

        printlog( "Close dialog <Split Cells>" )
        hCloseDialog( ZellenTeilen, "close" )
    else
        warnlog( "Dialog <Split Cells> did not open" )
    endif
    printlog " Checking optimize button   'ToDo: button not working"
    'Kontext "TableObjectBar"
    'Optimieren.TearOff
    'OptimizeTablebar.Close

    printlog "Checking Alignment."
    Kontext "TableObjectBar"
    Top.Click
    CenterVertical.Click
    Bottom.Click
    printlog "Alignment buttons checked."

    printlog "Checking inserting and deleting of columns and rows."
    ZeileEinfuegen.Click
    printlog "Row inserted."

    Zeileloeschen.Click
    printlog "Row deleted."

    SpalteEinfuegen.Click
    printlog "Column inserted."

    Spalteloeschen.Click
    printlog "Column deleted."

    printlog "Checking table design button 'ToDo:needs some further examination here!"
    tabledesign.Click
    
    printlog "Checking for Table Design in Impress TaskBar or for Dialog in Draw."
    if ( gApplication = "IMPRESS" ) then
        kontext "Tasks"
        if (NOT Tasks.exists) then
            warnlog "Tasks Panel not visible. Opening now."
            ViewTaskPane
        else
            printlog "Task Pane visible."
        endif
    else
         kontext "TableDesign"
         if TableDesign.exists then
            printlog "Table Design Dialog is up."
            TableDesign.Close
        else
            warnlog "TableDesign Dialog for Draw is missing."
        endif
    endif  
    
    printlog "Checking Properties button."
    Kontext "TableObjectBar"
    TableProperties.Click
    Kontext
    active.SetPage TabFont
    if (NOT TabFont.Exists) then
        warnlog "Tabpage missing!"
    else
        printlog "Tabpage Font is up."
    endif
    kontext "TabFont"

    printlog "Checking Fonts Box."

    if ( Font.exists() ) then
        printlog( "Western dialog layout -> good" )
    else
        if ( FontWest.exists() ) then
            warnlog( "<FontWest> exists, asian support is enabled which this test cannot handle" )
        else
            warnlog( "Unknown error while trying to access <Font> listbox" )
        endif
        hCloseDialog( TabFont, "close" )
        hFileCloseAll()
        goto endsub
    endif

    iItemCount = Font.GetItemCount
    printlog "Fontbox has " & iItemCount & " entries."
    for iItemIndex = 1 to iItemCount
        Font.Select(iItemIndex)
    next iItemIndex

    printlog "Checking style box."
    iItemCount = Style.GetItemCount
    printlog "Stylebox has " & iItemCount & " entries."
    for iItemIndex = 1 to iItemCount
        Style.Select(iItemIndex)
    next iItemIndex

    printlog "Checking size box."
    iItemCount = Size.GetItemCount
    printlog "Sizebox has " & iItemCount & " entries."
    for iItemIndex = 1 to iItemCount
        Size.Select(iItemIndex)
    next iItemIndex

    printlog "Checking language box."
    iItemCount = Language.GetItemCount
    printlog "Language box has " & iItemCount & " entries."
    for iItemIndex = 1 to iItemCount
        Language.Select(iItemIndex)
    next iItemIndex

    '------------Next Tab-----------------

    printlog "Checking Tabpage FontEffects."
    Kontext
    active.SetPage TabFontEffects
    if (NOT TabFontEffects.Exists) then
        warnlog "Tabpage FontEffects missing!"
    else
        printlog "Tabpage FontEffects is up."
    endif
    kontext "TabFontEffects"

    printlog "Checking Underlining box."
    iItemCount = Underline.GetItemCount
    printlog "Underlining has " & iItemCount & " entries."
    for iItemIndex = 1 to iItemCount
        Underline.Select(iItemIndex)
    next iItemIndex

    printlog "Checking StrikeThrough box."
    iItemCount = StrikeThrough.GetItemCount
    printlog "StrikeThrough has " & iItemCount & " entries."
    for iItemIndex = 1 to iItemCount
        StrikeThrough.Select(iItemIndex)
    next iItemIndex

    printlog "Checking individual words."
    IndividualWords.Check
    printlog "Individual words checked."

    printlog "Checking Color box."
    iItemCount = Color.GetItemCount
    printlog "Font color has " & iItemCount & " entries."
    for iItemIndex = 1 to iItemCount
        Color.Select(iItemIndex)
    next iItemIndex

    printlog "Checking Relief."
    iItemCount = Relief.GetItemCount
    printlog "Relief has " & iItemCount & " entries."
    for iItemIndex = 1 to iItemCount
        Relief.Select(iItemIndex)
    next iItemIndex
    Relief.Select(1) 'setting back Relief, otherwise Outline is greyed out

    printlog "Checking Outline and Shadow."
    Outline.Check
    printlog "Outline checked."
    Shadow.Check
    printlog "Shadow checked."

    '------------Next Tab-----------------

    printlog "Checking Tabpage Borders."
    Kontext
    active.SetPage TabUmrandung
    if (NOT TabUmrandung.Exists) then
        warnlog "Tabpage Borders missing!"
    else
        printlog "Tabpage Borders is up."
    endif
    kontext "TabUmrandung"
    printlog "Checking Style."
    iItemCount = Stil.GetItemCount
    printlog "Style has " & iItemCount & " entries."
    for iItemIndex = 1 to iItemCount
        Stil.Select(iItemIndex)
    next iItemIndex

    printlog "Checking Color."
    iItemCount = StilFarbe.GetItemCount
    printlog "Color has " & iItemCount & " entries."
    for iItemIndex = 1 to iItemCount
        StilFarbe.Select(iItemIndex)
    next iItemIndex

    printlog "Un-Checking Synchronize."
    Synchronisieren.UnCheck
    printlog "Synchronize unchecked."

    printlog "Checking Left."
    Links.ToMax
    printlog "Maximum Left is " & Links.GetText & "."
    Links.ToMin
    printlog "Minimum Left is " & Links.GetText & "."

    printlog "Checking Right."
    Rechts.ToMax
    printlog "Maximum Right is " & Rechts.GetText & "."
    Rechts.ToMin
    printlog "Minimum Right is " & Rechts.GetText & "."

    printlog "Checking Top."
    Oben.ToMax
    printlog "Maximum Top is " & Oben.GetText & "."
    Oben.ToMin
    printlog "Minimum Top is " & Oben.GetText & "."

    printlog "Checking Bottom."
    Unten.ToMax
    printlog "Maximum Bottom is " & Unten.GetText & "."
    Unten.ToMin
    printlog "Minimum Bottom is " & Unten.GetText & "."

    '------------Next Tab-----------------

    printlog "Checking Tabpage Background."
    Kontext
    active.SetPage TabArea
    if (NOT TabArea.Exists) then
        warnlog "Tabpage Area missing!"
    else
        printlog "Tabpage Area is up."
    endif
    kontext "TabArea"

    iItemCount = FillOptions.GetItemCount
    for iItemIndex = 2 to iItemCount
        FillOptions.Select(iItemIndex)
    next iItemIndex
    printlog "Tabpage Background tested."
    TabArea.Cancel
    printlog "Format Cells dialog closed."
    printlog "Toolbar tested."

    printlog " Closing doc."
    hCloseDocument

endcase 'tiTableObjectBar

'-------------------------------------------------------------------------------
