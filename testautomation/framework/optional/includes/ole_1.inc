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
'* short description : global-level-1-test -> insert all OLE-Objects out of OLE-dialog into all doc-types
'*
'\******************************************************************************

testcase tOLEWriter
    
    Dim i% : Dim k%
    Dim sText$
    
    gApplication = "WRITER"
    
    Call hNewDocument
    
    PrintLog "- Writer :   Chart without table"
    hUseAsyncSlot( "InsertObjectChart" )

    Kontext "DocumentChart"
    if ( DocumentChart.Exists( 5 ) ) then
        FormatChartType
        Kontext "ChartType"
        if ( ChartType.Exists( 5 ) ) then
            ChartType.Cancel
        else
            warnlog "Chart Type dialog did bot occour."
        end if        
        Kontext "DocumentChart"
        DocumentChart.TypeKeys "<ESCAPE>"
        Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<DELETE>"
    else
        warnlog "Chart has not been inserted."
    end if    

    PrintLog "- Writer :   Chart out of a table only with numbers"
    hUseAsyncSlot( "InsertTableWriter" )
    
    Kontext "TabelleEinfuegenWriter"
    if ( TabelleEinfuegenWriter.exists( 1 ) ) then
        Spalten.SetText "3"
        Zeilen.SetText "10"
        TabelleEinfuegenWriter.OK
        TabelleEinfuegenWriter.notExists( 2 )
    else
        warnlog( "Dialog <TabelleEinfuegenWriter> did not open" )
    endif
    
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
    
    hUseAsyncSlot( "InsertObjectChart" )
    hStepThroughChartWizard()
    
    Kontext "DocumentChart"
    DocumentChart.typeKeys "<ESCAPE>"
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Delete>"
    Sleep 1
    DocumentWriter.TypeKeys "<Down>", 20
    DocumentWriter.TypeKeys "<Return>", 2
    
    PrintLog "- Writer :   Chart out of a table only with chars"
    hUseAsyncSlot( "InsertTableWriter" )
    
    Kontext "TabelleEinfuegenWriter"
    if ( TabelleEinfuegenWriter.exists( 1 ) ) then
        Spalten.SetText "3"
        Zeilen.SetText "10"
        TabelleEinfuegenWriter.OK
        TabelleEinfuegenWriter.notExists( 2 )
    else
        warnlog( "Dialog <TabelleEinfuegenWriter> did not open" )
    endif
    
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
    
    hUseAsyncSlot( "InsertObjectChart" )
    hStepThroughChartWizard()
        
    Kontext "DocumentChart"
    DocumentChart.typeKeys "<ESCAPE>"
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Delete>"
    Sleep 1
    DocumentWriter.TypeKeys "<Down>", 20
    DocumentWriter.TypeKeys "<Return>", 2
    
    PrintLog "- Writer :   Math"
    
    hUseAsyncSlot( "InsertObjectFormulaWriter" )
    gMouseClick ( 1, 1)
    
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Delete>"
    
    PrintLog "- Writer :   floating frame"
    hUseAsyncSlot( "InsertFloatingFrame" )
    
    printlog( "Properties dialog for floating frame should open" )    
    Kontext "TabEigenschaften"
    if ( TabEigenschaften.exists( 1 ) and TabEigenschaften.isVisible() ) then

        printlog( "Name the floating frame, close the dialog" )
        FrameName.SetText "Hallo"
        Inhalt.SetText ConvertPath ( gTesttoolPath+"global\input\graf_inp\borabora.jpg" )
        
        TabEigenschaften.OK
        TabEigenSchaften.notExists( 2 )
        
    endif
    
    printlog( "Close the writer document" )
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Delete>"
    Sleep 1
    
    hCloseDocument
    
endcase

'*******************************************************************************

testcase tOLECalc



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
    
    brc = hCreateDocument()
    if ( not brc ) then 
        warnlog( "Failed to create new " & gApplication & " document" )
    endif
    
    PrintLog "- Calc :   Chart without data"
    InsertChartCalc

    hStepThroughChartWizard()
    
    kontext "DocumentChart"
    DocumentChart.typeKeys( "<ESCAPE>" )
    
    Kontext "DocumentCalc"
    gMouseClick ( OUTSIDE_CHART_OBJECT_X , OUTSIDE_CHART_OBJECT_Y )
    sleep( ICWAIT )
    
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

    

    PrintLog "- Calc :   Chart with data"
    
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
    
    DocumentCalc.TypeKeys( "<Up><Left>", CURSOR_MOVEMENT_RETRIES )
    call ZellenMarkieren ( SELECT_CELLS_X , SELECT_CELLS_Y )
    
    InsertChartCalc
    
    hStepThroughChartWizard()
    
    Kontext "DocumentCalc"
    DocumentCalc.typeKeys("<escape>")
    sleep( ICWAIT )
    
    PrintLog "- Calc :   Chart for a table only with text"
    
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys( "<MOD1 HOME>" )
    for iColumn = 1 to SELECT_CELLS_Y
        for iRow = 1 to SELECT_CELLS_X
            hTypeKeys ( "Hallo<Return>" )
        next iRow
        DocumentCalc.TypeKeys "<Up><Left>", CURSOR_MOVEMENT_RETRIES
        DocumentCalc.TypeKeys "<Right>", iColumn
    next iColumn
    
    DocumentCalc.TypeKeys "<Up><Left>", CURSOR_MOVEMENT_RETRIES
    call ZellenMarkieren ( SELECT_CELLS_X , SELECT_CELLS_Y )
    
    InsertChartCalc
    
    hStepThroughChartWizard()
    
    Kontext "DocumentCalc"
    gMouseCLick ( OUTSIDE_CHART_OBJECT_X , OUTSIDE_CHART_OBJECT_Y )
    sleep( ICWAIT )
    
    PrintLog( "- Calc :   Math" )
    
    InsertObjectFormulaCalc
    sleep( ICWAIT )
    
    gMouseClick ( 50, 99 )
    sleep( ICWAIT )
    
    PrintLog( "- Calc :   floating frame" )
    
    InsertFloatingFrame
    
    Kontext "TabEigenschaften"
    FrameName.SetText( "Hallo" )
    Inhalt.SetText( ConvertPath ( gTesttoolPath+"global\input\graf_inp\borabora.jpg" ) )
    
    TabEigenschaften.OK()
    sleep( ICWAIT )
    
    gMouseCLick ( OUTSIDE_CHART_OBJECT_X , OUTSIDE_CHART_OBJECT_Y )
    sleep( ICWAIT )
    
    brc = hDestroyDocument()
    
endcase

'*******************************************************************************

testcase tOLEDraw
    
    Dim i% : Dim k%
    Dim iMenuCount(1) as integer
    
    gApplication = "DRAW"
    
    Call hNewDocument
    
    PrintLog "- Draw :   Math"
    InsertObjectFormulaDraw
    sleep 2
    
    gMouseClick ( 1 , 1 )
    sleep 1
    
    DocumentDraw.TypeKeys "<Delete>"
    
    PrintLog "- Draw :   Chart"
    
    hUseMenu()
    iMenuCount(0) = menuGetItemCount
    MenuSelect(0)
    
    try
        hUseAsyncSlot( "InsertObjectChart" )
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
        
        gMouseClick ( 1 , 1 )
        Sleep ( 1 )
        
        DocumentDraw.TypeKeys "<Delete>"
    catch
        Call hReopenDoc
    endcatch
    
    PrintLog "- Draw :   Calc"
    
    InsertSpreadsheetDraw
    sleep 2
    
    gMouseClick ( 1 , 1 )
    sleep 1
    
    DocumentDraw.TypeKeys "<Delete>"
    
    PrintLog "- Draw :   floating frame"
    
    try
        InsertFloatingFrame
        
        Kontext "TabEigenschaften"
        
        FrameName.SetText "Hallo"
        Inhalt.SetText ConvertPath ( gTesttoolPath+"global\input\graf_inp\borabora.jpg" )
        
        TabEigenschaften.OK
        Sleep 2
        
        gMouseClick ( 1 , 1 )
        Sleep 1
        
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
    
    Call hNewDocument
    
    PrintLog "- Impress :   Math"
    
    InsertObjectFormulaDraw
    sleep 2
    
    gMouseClick ( 1 , 1 )
    sleep 1
    
    DocumentImpress.TypeKeys "<Delete>"
    
    '*** Chart
    PrintLog "- Impress :   Chart"
    
    try
        hUseAsyncSlot( "InsertObjectChart" )
        sleep 2
        
        gMouseClick ( 1 , 1 )
        sleep 1
        
        DocumentImpress.TypeKeys "<Delete>"
    catch
        Call hReopenDoc
    endcatch
    
    '*** Calc
    PrintLog "- Impress :   Calc"
    
    InsertSpreadsheetDraw
    sleep 2
    
    gMouseClick ( 1 , 1 )
    sleep 1
    
    DocumentImpress.TypeKeys "<Delete>"
    
    '*** floating frame
    PrintLog "- Impress :   floating frame"
    
    try
        InsertFloatingFrame
        
        Kontext "TabEigenschaften"
        FrameName.SetText "Hallo"
        
        Inhalt.SetText ConvertPath ( gTesttoolPath+"global\input\graf_inp\borabora.jpg" )
        
        TabEigenschaften.OK
        Sleep 2
        
        gMouseClick ( 1 , 1 )
        Sleep 1
    catch
        hReopenDoc
    endcatch
    
    DocumentImpress.TypeKeys "<Delete>"
    
    hCloseDocument
    
endcase



