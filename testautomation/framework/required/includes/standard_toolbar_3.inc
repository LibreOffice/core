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
'* owner : gregor.hartmann@oracle.com
'*
'* short description : Items on the standardbar
'*
'\***************************************************************************

testcase tStandardBar_3

    printlog( "Items on the standardbar" )

    dim sFileOut as String
    dim HyperlinkTabID as integer
    dim iTabPage as integer

    gApplication = "CALC"

    printlog( "Cleanup test environment - delete files" )
    sFileOut = hGetWorkPath() & "FWK-update-functionbar" & hGetSuffix( "current" )
    ConvertPath ( sFileOut )
    hDeleteFile( sFileOut )

    printlog( "Reset the standardbar to defaults" )
    hResetStandardBar()

    printlog( "New Spreadsheet document" )
    hCreateDocument()

    Kontext "Navigator"
    if ( Navigator.Exists() ) then
        Navigator.close()
    endif

    printlog( "Insert text into a random cell, press <Enter>" )
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys( "Only for a test.<Return>" )

    Kontext "Standardbar"
    printlog( "Press the 'Save' button" )
    hWaitForObject( Speichern , 1000 )
    Speichern.click()


    Kontext "SpeichernDlg"
    printlog( "- Save the file to: " & sFileOut )
    Dateiname.SetText( sFileOut )
    Speichern.Click()

    printlog( "Close the document" )
    hDestroyDocument()
    
    printlog( "Click the 'File Open' button" )
    hUseAsyncSlot( "FileOpen" )

    Kontext "OeffnenDlg"
    Printlog( "Load test file in read-only mode" )
    if ( OeffnenDlg.exists( 2 ) ) then
        Dateiname.SetText( sFileOut )
        NurLesen.Check()
        oeffnen.click()
    else
        warnlog( "File Open dialog is missing" )
    endif
    
    wait( 2000 )

    Kontext "DocumentCalc"
    printlog( "Try to insert a chart to verify that the document is read-only (this should fail)" )
    try
        InsertChartCalc
        warnlog( "The document is not read-only" )
    catch
    endcatch

    Kontext "Standardbar"
    printlog( "Click the <Edit File> button" )
    hWaitForObject( Bearbeiten , 1000 )
    Bearbeiten.click() : WaitSlot()

    printlog( "Try to insert a chart (should work this time)" )
    InsertChartCalc
    
    Kontext "ChartWizard"
    if ( ChartWizard.exists( 2 ) ) then
        printlog( "Cancel Chart Wizard" )
        ChartWizard.cancel()
        WaitSlot()
    else
        warnlog( "Chart Wizard should be open" )
    endif

    printlog( "Select a few cells for Cut/Copy/Paste test" )
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys( "<Up>", 7 )
    DocumentCalc.TypeKeys( "<Shift Down>", 5 )

    printlog( "Cut" )
    Kontext "Standardbar"
    hWaitForObject( Ausschneiden , 1000 )
    Ausschneiden.Click()
    WaitSlot()

    Kontext "DocumentCalc"
    printlog( "Move down one cell" )
    DocumentCalc.TypeKeys( "<Down>" )

    printlog( "Paste" )
    Kontext "Standardbar"
    sleep( 3 ) ' No better solution found yet.
    Einfuegen.Click()

    Kontext "DocumentCalc"
    printlog( "Move up 7 cells, then do <shift down>" )
    DocumentCalc.TypeKeys( "<Up>", 7 )
    DocumentCalc.TypeKeys( "<Shift Down>", 5 )
    WaitSlot()

    printlog( "Copy" )
    Kontext "Standardbar"
    hWaitForObject( Kopieren , 1000 )
    Kopieren.Click()
    WaitSlot()

    Kontext "DocumentCalc"
    printlog( "Press right arrow and RETURN" )
    DocumentCalc.TypeKeys( "<Right><Return>" )
    WaitSlot()

    printlog( "Paste again" )
    Kontext "Standardbar"
    hWaitForObject( Einfuegen , 1000 )
    Einfuegen.Click()

    printlog( "Undo" )
    hWaitForObject( Undo , 2000 )
    Undo.Click()
    WaitSlot()
    
    printlog( "Redo" )
    hWaitForObject( Redo , 2000 )
    Redo.Click()
    WaitSlot()

    Kontext "Standardbar"
    printlog( "Open Navigator" )
    hWaitForObject( NavigatorButton , 1000 )
    NavigatorButton.Click()

    Kontext "Navigator"
    if ( Navigator.Exists( 1 ) ) then
        printlog( "Close Navigator" )
        Kontext "Standardbar"
        NavigatorButton.Click()
    else
        warnlog( "Navigator did not open" )
    endif

    Kontext "Navigator"
    if ( Navigator.exists() ) then
        warnlog( "The navigator should not be visible" )
    endif

    Kontext "Standardbar"
    printlog( "Click <HyperLink> to open the dialog" )
    hWaitForObject( HyperlinkDialog , 1000 )
    HyperlinkDialog.Click()

    for iTabPage = 1 to 4
        select case iTabPage
        case 1: Kontext "Hyperlink"
            if ( Hyperlink.exists() ) then HyperlinkTabID = 1
            exit for
        case 2: kontext "TabHyperlinkMailUndNews"
            if ( TabHyperlinkMailUndNews.exists() ) then HyperlinkTabID = 2
            exit for
        case 3: kontext "TabHyperlinkDokument"
            if ( TabHyperlinkDokument.exists() ) then HyperlinkTabID = 3
            exit for
        case 4: kontext "TabHyperlinkNeuesDokument"
            if ( TabHyperlinkNeuesDokument.exists() ) then HyperlinkTabID = 4
            exit for
        case default:
            HyperlinkTabID = 0
        end select
    next iTabPage

    if ( HyperlinkTabID > 0 ) then    
    
        if ( HyperlinkTabID <> 1 ) then
            qaerrorlog( "This is not the internet tabpage, did you " & _
                        "kill the userlayer before running the test?" )
        endif
        
        kontext "Standardbar"
        printlog( "Click <HyperLink> to close" )
        HyperlinkDialog.Click()
        WaitSlot()
    else
        warnlog( "#i104704# - Insert Hyperlink dialog cannot be accessed" )
        Kontext "Standardbar"
        HyperlinkDialog.Click()
    endif

    Kontext "Standardbar"
    printlog( "Gallery button" )
    hWaitForObject( GalleryButton , 1000 )
    GalleryButton.Click()
    
    kontext "Gallery"
    if ( Gallery.exists( 3 ) ) then
        printlog( "Gallery button again, to close" )
        kontext "StandardBar"
        GalleryButton.click()
        WaitSlot()
        
        kontext "Gallery" 
        if ( Gallery.exists() ) then    
            warnlog( "Gallery did not close " )
        endif
    else
        warnlog( "The Gallery did not open" )
    endif

    printlog( "Close all documents and delete files" )        
    hDestroyDocument()
    hDeleteFile( sFileOut )

endcase


