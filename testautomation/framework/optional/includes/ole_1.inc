'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: ole_1.inc,v $
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
'* short description : global-level-1-test -> insert all OLE-Objects out of OLE-dialog into all doc-types
'*
'\******************************************************************************

testcase tOLEWriter
    
    '///Test all OLE-Objects you can insert out of menu
    Dim i% : Dim k%
    Dim sText$
    
    gApplication = "WRITER"
    
    '///'file / new / text document'
    Call hNewDocument
    
    '///'insert / object / chart' without a table
    PrintLog "- Writer :   Chart without table"
    InsertObjectChart
    sleep(4)    
    '///+ click 'create' on the Autoformat dialog for Charts
    Kontext "DocumentChart"
    if DocumentChart.Exists(5) then
        '///+ format / vhart type
        FormatChartType
        Kontext "ChartType"
        if ChartType.Exists(5) then
            '///+ press ( cancel ) in chart type dialog
            ChartType.Cancel
        else
            warnlog "Chart Type dialog did bot occour."
        end if        
        Kontext "DocumentChart"
        '///+ type ( escape )
        DocumentChart.TypeKeys "<ESCAPE>"
        '///+ delete the selected chart with keyboard ( delete )
        Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<DELETE>"
        sleep(1)        
    else
        warnlog "Chart has not been inserted."
    end if    
    sleep(4)        
    '///create a table with numbers
    PrintLog "- Writer :   Chart out of a table only with numbers"
    InsertTableWriter
    
    Kontext "TabelleEinfuegenWriter"
    Spalten.SetText "3"
    Zeilen.SetText "10"
    TabelleEinfuegenWriter.OK
    
    Kontext "DocumentWriter"
    for i%=1 to 10
        for k%=1 to 3
            sText$ = (i%+k%)*k%
            DocumentWriter.TypeKeys  sText$
            DocumentWriter.TypeKeys "<Right>"
        next k%
        DocumentWriter.TypeKeys "<Down>"
        DocumentWriter.TypeKeys "<Left>", 3
    next i%
    
    '///+if you are in the table 'insert / object / chart'
    InsertObjectChart
    hStepThroughChartWizard()
    
    '///+ delete the selected chart with keyboard ( delete )
    Kontext "DocumentChart"
    DocumentChart.typeKeys "<ESCAPE>"
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Delete>"
    Sleep 1
    DocumentWriter.TypeKeys "<Down>", 20
    DocumentWriter.TypeKeys "<Return>", 2
    Sleep 1
    
    
    '///create a new table only with characters
    PrintLog "- Writer :   Chart out of a table only with chars"
    InsertTableWriter
    
    Kontext "TabelleEinfuegenWriter"
    Spalten.SetText "3"
    Zeilen.SetText "10"
    TabelleEinfuegenWriter.OK
    
    Kontext "DocumentWriter"
    for i%=1 to 10
        for k%=1 to 3
            sText$ = (i%+k%)*k%
            DocumentWriter.TypeKeys  "Hallo" + sText$
            DocumentWriter.TypeKeys "<Right>"
        next k%
        DocumentWriter.TypeKeys "<Down>"
        DocumentWriter.TypeKeys "<Left>", 3
    next i%
    
    '///+if you are in the table 'insert / object / chart'
    InsertObjectChart
    
    hStepThroughChartWizard()
        
    '///+ delete the selected chart with keyboard ( delete )
    Kontext "DocumentChart"
    DocumentChart.typeKeys "<ESCAPE>"
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Delete>"
    Sleep 1
    DocumentWriter.TypeKeys "<Down>", 20
    DocumentWriter.TypeKeys "<Return>", 2
    Sleep 1
    
    '///insert a math object into the writer doc
    PrintLog "- Writer :   Math"
    
    '///+ insert / object / formula
    InsertObjectFormulaWriter
    gMouseClick ( 1, 1)
    
    '///+ delete the selected mathobject with keyboard ( delete )
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Delete>"
    Sleep 1
    
    '///insert a floating object into the writer doc
    PrintLog "- Writer :   floating frame"
    
    'warnlog( "#148094# - Crash when deselecting floating frame in Writer" )
    
    '///+insert / floating frame
    InsertFloatingFrame
    
    '///+insert in the dialog as name 'hello' and as contents '[Testtoolpath]\global\input\graf_inp\borabora.jpg'
    Kontext "TabEigenschaften"
    FrameName.SetText "Hallo"
    Inhalt.SetText ConvertPath ( gTesttoolPath+"global\input\graf_inp\borabora.jpg" )
    
    '///+click 'OK'
    TabEigenschaften.OK
    Sleep 2
    
    '///+ delete the selected frame with keyboard ( delete )
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Delete>"
    Sleep 1
    
    '///close the document
    hCloseDocument
    
endcase

'*******************************************************************************

testcase tOLECalc


    '///<h1>Create charts within Calc as OLE objects</h1>
    '///<ul>

    dim i as Integer
    dim k as Integer
    
    const ICWAIT as Integer = 2
    dim brc as boolean
    dim iCurrentPos as integer
    dim iColumn as integer
    dim iRow as integer
    
    const CHART_OBJECT_DEFAULT_POSITION_X = 30 
    const CHART_OBJECT_DEFAULT_POSITION_Y = 30
    const OUTSIDE_CHART_OBJECT_X = 1
    const OUTSIDE_CHART_OBJECT_Y = 1    
    const CURSOR_MOVEMENT_RETRIES = 20
    const SELECT_CELLS_X = 7
    const SELECT_CELLS_Y = 7
    
    gApplication = "CALC"
    
    '///+<li>Create a new spredsheet document</li>
    brc = hCreateDocument()
    if ( not brc ) then 
        warnlog( "Failed to create new " & gApplication & " document" )
    endif
    
    '///+<li>Insert a Chart Object without specifying a data range</li>
    PrintLog "- Calc :   Chart without data"
    InsertChartCalc

    '///+<li>Step through the wizard without changing any settings, close it</li>    
    hStepThroughChartWizard()
    
    '///+<li>Switch from editing to select mode of the OLE object by typing <ESC></li>
    kontext "DocumentChart"
    DocumentChart.typeKeys( "<ESCAPE>" )
    
    '///+<li>Remove focus from object by clicking into the document</li>
    Kontext "DocumentCalc"
    gMouseClick ( OUTSIDE_CHART_OBJECT_X , OUTSIDE_CHART_OBJECT_Y )
    sleep( ICWAIT )
    
    '///+<li>Click on the chart and delete it with 'delete'</li>
    ' If the OLE object is in selection mode the Drawing Object Bar should be
    ' visible. As we do not know exactly where on the spreadsheet the object
    ' appears we move from the upper left to the lower right corner of the 
    ' document until we hit an OLE object and the Drawing Object Bar is enabled.
    ' Then <DEL> is sent to the document hopefully deleting the object.
    
    for iCurrentPos = 1 to 9
    
        kontext "DocumentCalc" 
        gMouseClick( 10 * iCurrentPos , 10 * iCurrentPos )
        
        Kontext "DrawingObjectBar"
        if ( DrawingObjectBar.exists( 1 ) ) then
        
            kontext "DocumentCalc"
            DocumentCalc.TypeKeys( "<Delete>" )
            printlog( "Found drawing object, executed <DEL> on the object" )
            exit for
            
        else
        
            qaerrorlog( "Drawing object not hit, trying again" )
        
        endif
        
    next iCurrentPos
    '</ul>

    
    '///+<li>insert a chart with data</li>
    '///<ul>

    PrintLog "- Calc :   Chart with data"
    
    '///+<li>Create a table with random numbers in the cells A1 to G7</li>
    kontext "DocumentCalc"
    DocumentCalc.TypeKeys( "<MOD1 HOME>" )
    for iColumn = 1 to SELECT_CELLS_Y
        for iRow = 1 to SELECT_CELLS_X
            hTypeKeys ( iColumn * iRow )
            hTypeKeys ( "<Return>" )
        next iRow
        DocumentCalc.TypeKeys( "<Up><Left>", CURSOR_MOVEMENT_RETRIES )
        DocumentCalc.TypeKeys( "<Right>", iColumn )
    next iColumn
    
    '///+<li>Select the range A1 to G7</li>
    DocumentCalc.TypeKeys( "<Up><Left>", CURSOR_MOVEMENT_RETRIES )
    call ZellenMarkieren ( SELECT_CELLS_X , SELECT_CELLS_Y )
    
    '///+<li>Insert / chart</li>
    InsertChartCalc
    
    '///+<li>Click on 'next' for each page in AutoFormat dialog for charts and on the last page click 'create'</li>
    hStepThroughChartWizard()
    
    '///+<li>click in the spreadsheet document to deselect the chart</li>
    Kontext "DocumentCalc"
    DocumentCalc.typeKeys("<escape>")
    sleep( ICWAIT )
    '///</ul>
    
    '///+<li>insert a chart only with text in the table</li>
    '///<ul>
    PrintLog "- Calc :   Chart for a table only with text"
    
    '///+<li>Create a range of cells from A1 to G7 containing text content</li>
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys( "<MOD1 HOME>" )
    for iColumn = 1 to SELECT_CELLS_Y
        for iRow = 1 to SELECT_CELLS_X
            hTypeKeys ( "Hallo<Return>" )
        next iRow
        DocumentCalc.TypeKeys "<Up><Left>", CURSOR_MOVEMENT_RETRIES
        DocumentCalc.TypeKeys "<Right>", iColumn
    next iColumn
    
    '///+<li>Select the range from A1 to G7</li>
    DocumentCalc.TypeKeys "<Up><Left>", CURSOR_MOVEMENT_RETRIES
    call ZellenMarkieren ( SELECT_CELLS_X , SELECT_CELLS_Y )
    
    '///+<li>Click insert / chart or go there via menu</li>
    InsertChartCalc
    
    '///+<li>Step through the Chart Wizard keeping all defaults, finish it</li>
    hStepThroughChartWizard()
    
    '///+<li>Click in the spreadsheet document to deselect the chart</li>
    Kontext "DocumentCalc"
    gMouseCLick ( OUTSIDE_CHART_OBJECT_X , OUTSIDE_CHART_OBJECT_Y )
    sleep( ICWAIT )
    '///</ul>
    
    '///+<li>Insert a math object</li>
    '///<ul>
    PrintLog( "- Calc :   Math" )
    
    '///+<li>Insert / object / formula</li>
    InsertObjectFormulaCalc
    sleep( ICWAIT )
    
    '///+<li>Click in the spreadsheet document to deselect the chart</li>
    gMouseClick ( 50, 99 )
    sleep( ICWAIT )
    '///</ul>
    
    '///+<li>insert a floating frame</li>
    '///<ul>
    PrintLog( "- Calc :   floating frame" )
    
    '///+<li>Insert a floating frame</li>
    InsertFloatingFrame
    
    '///+<li>Insert in the dialog as name 'hello' and as contents '[Testtoolpath]\global\input\graf_inp\borabora.jpg'</li>
    Kontext "TabEigenschaften"
    FrameName.SetText( "Hallo" )
    Inhalt.SetText( ConvertPath ( gTesttoolPath+"global\input\graf_inp\borabora.jpg" ) )
    
    '///+<li>Click 'OK'</li>
    TabEigenschaften.OK()
    sleep( ICWAIT )
    
    '///+<li>Click in the spreadsheet document to deselect the chart</li>
    gMouseCLick ( OUTSIDE_CHART_OBJECT_X , OUTSIDE_CHART_OBJECT_Y )
    sleep( ICWAIT )
    
    '///</ul>
    '///+<li>close the document</li>
    brc = hDestroyDocument()
    '///</ul>
    
endcase

'*******************************************************************************

testcase tOLEDraw
    
    Dim i% : Dim k%
    Dim iMenuCount(1) as integer
    
    gApplication = "DRAW"
    
    '///file / new / drawing
    Call hNewDocument
    
    '///insert a math object
    PrintLog "- Draw :   Math"
    '///+insert / object / formula
    InsertObjectFormulaDraw
    sleep 2
    
    '///+click into the draw document once to set the focus on the math object
    gMouseClick ( 1 , 1 )
    sleep 1
    
    '///+delete the object with keyboard ( delete )
    DocumentDraw.TypeKeys "<Delete>"
    
    '///insert a chart
    PrintLog "- Draw :   Chart"
    
    '/// Special test: check that menubar switches, by comparing the count. ///'
    hUseMenu()
    iMenuCount(0) = menuGetItemCount
    MenuSelect(0)
    
    '///+insert / chart
    try
        InsertObjectChart
        Sleep ( 2 )
        
        Kontext "DocumentChart"
        DocumentChart.useMenu
        iMenuCount(1) = menuGetItemCount
        MenuSelect(0)
        if iMenuCount(0) = iMenuCount(1) then
            warnlog "Menu bar didn't change after inserting OLE object; It should be different from: " +iMenuCount(0)
        else
            printlog "Menu bar did change; from: " + iMenuCount(0) + "; to: " + iMenuCount(1)
        endif
        
        '///+click into the draw document once to set the focus on the math object
        gMouseClick ( 1 , 1 )
        Sleep ( 1 )
        
        '///+delete the object with keyboard ( delete )
        DocumentDraw.TypeKeys "<Delete>"
    catch
        Call hReopenDoc
    endcatch
    
    '///insert a spreadsheet
    PrintLog "- Draw :   Calc"
    
    '///+insert / spreadsheet
    InsertSpreadsheetDraw
    sleep 2
    
    '///+click one time into the draw document to set only the focus on the math object
    gMouseClick ( 1 , 1 )
    sleep 1
    
    '///+delete the object with keyboard ( delete )
    DocumentDraw.TypeKeys "<Delete>"
    
    '///insert a floating frame
    PrintLog "- Draw :   floating frame"
    
    '///+insert / floating frame
    try
        InsertFloatingFrame
        
        Kontext "TabEigenschaften"
        
        '///+insert in the dialog as name 'hello' and as contents '[Testtoolpath]\global\input\graf_inp\borabora.jpg'
        FrameName.SetText "Hallo"
        Inhalt.SetText ConvertPath ( gTesttoolPath+"global\input\graf_inp\borabora.jpg" )
        
        '///+click 'OK'
        TabEigenschaften.OK
        Sleep 2
        
        '///+click one time into the draw document to set only the focus on the math object
        gMouseClick ( 1 , 1 )
        Sleep 1
        
        '///+delete the object with keyboard ( delete )
        DocumentDraw.TypeKeys "<Delete>"
    catch
        Call hReopenDoc
    endcatch
    
    hCloseDocument
    
endcase

'*******************************************************************************

testcase tOLEImpress
    
    Dim i% : Dim k%
    
    gApplication = "IMPRESS"
    
    '///file / new / presentation
    Call hNewDocument
    
    '///insert a math object
    PrintLog "- Impress :   Math"
    
    '///+insert / object / formula
    InsertObjectFormulaDraw
    sleep 2
    
    '///+click one time into the impress document to set only the focus on the math object
    gMouseClick ( 1 , 1 )
    sleep 1
    
    '///+delete the object with keyboard ( delete )
    DocumentImpress.TypeKeys "<Delete>"
    
    '*** Chart
    PrintLog "- Impress :   Chart"
    
    try
        '///+insert / chart
        InsertObjectChart
        sleep 2
        
        '///+click one time into the impress document to set only the focus on the math object
        gMouseClick ( 1 , 1 )
        sleep 1
        
        '///+delete the object with keyboard ( delete )
        DocumentImpress.TypeKeys "<Delete>"
    catch
        Call hReopenDoc
    endcatch
    
    '*** Calc
    PrintLog "- Impress :   Calc"
    
    '///+insert / spreadsheet
    InsertSpreadsheetDraw
    sleep 2
    
    '///+click one time into the impress document to set only the focus on the math object
    gMouseClick ( 1 , 1 )
    sleep 1
    
    '///+delete the object with keyboard ( delete )
    DocumentImpress.TypeKeys "<Delete>"
    
    '*** floating frame
    PrintLog "- Impress :   floating frame"
    
    try
        '///+insert / floating frame
        InsertFloatingFrame
        
        Kontext "TabEigenschaften"
        FrameName.SetText "Hallo"
        
        '///+insert in the dialog as name 'hello' and as contents '[Testtoolpath]\global\input\graf_inp\borabora.jpg'
        Inhalt.SetText ConvertPath ( gTesttoolPath+"global\input\graf_inp\borabora.jpg" )
        
        '///+click 'OK'
        TabEigenschaften.OK
        Sleep 2
        
        '///+click one time into the impress document to set only the focus on the math object
        gMouseClick ( 1 , 1 )
        Sleep 1
    catch
        hReopenDoc
    endcatch
    
    '///+delete the object with keyboard ( delete )
    DocumentImpress.TypeKeys "<Delete>"
    
    hCloseDocument
    
endcase



