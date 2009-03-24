'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: standard_toolbar_3.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:19:03 $
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
'* owner : joerg.skottke@sun.com
'*
'* short description : global update test (Standardbar)
'*
'\***************************************************************************

testcase tStandardBar_3

    dim sFileOut as String
    
    '///<h1>Update test for all icons on the StandardBar</h1>

    '///<ul>
    printlog "************************************************************"
    printlog "Check all visible icons on the Standardbar"
    printlog "************************************************************"
    
    gApplication = "CALC"

    '///+<li>Setup: Delete files from the workdirectory to ensure a clean test</li>
    sFileOut = hGetWorkPath() & "FWK-update-functionbar" & hGetSuffix( "current" )
    ConvertPath ( sFileOut )
    hDeleteFile( sFileOut )

    '///+<li>Setup: Reset the Standardbar to defaults</li>
    hResetStandardBar()
    printlog "check all entries on the 'Standardbar'"

    '///+<li>Skipping testing of URL field, it is hidden by default and tested somewhere else</li>
    printlog "Skipping URL-Field"

    '///+<li>File Save</li>
    '///<ul>

    printlog "Save File"
    gApplication = "CALC"

    '///+<li>New Spreadsheet document, ensure that the navigator is closed</li>
    printlog( "New Spreadsheet document" )
    hCreateDocument()
    Kontext "Navigator"
    if ( Navigator.Exists() ) then
        Navigator.close()
    endif

    '///+<li>Insert some text in one cell (don't forget to press a <Enter> at the end)<br>
    printlog( "Insert text into a random cell, press <Enter>" )
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys( "Only for a test.<Return>" )

    '///+ <i>Save-Button must be active -> click on the button</i></li>
    printlog( "Press the 'Save' button" )

    Kontext "Standardbar"
    hWaitForObject( Speichern , 1000 )
    Speichern.click()

    '///+<li>Set a filenname, remember it for further usage</li>
    printlog( "- Save the file to: " & sFileOut )

    Kontext "SpeichernDlg"
    Dateiname.SetText( sFileOut )
    Speichern.Click()

    '///+<li>Close the document</li>
    printlog( "Close the document" )
    hDestroyDocument()

    '///</ul>
    
    '///+<li>Edit File by pressing &quot;File Open&quot; button on the StandardBar</li>

    '///<ul>
    printlog( "Edit the file by pressing the 'File Open' button" )

    '///+<li>Click the File Open button<br>The File Open dialog should pop up</li>
    FileOpen

    Kontext "OeffnenDlg"
    if ( OeffnenDlg.exists( 2 ) ) then
    
        '///+<li>Enter the name of the file we used earlier</li>
        Dateiname.SetText( sFileOut )
        
        '///+<li>Check the &quot;Read only&quot; checkbox</li>
        NurLesen.Check()
        
        '///+<li>Click &quot;Open&quot;</li>
        oeffnen.click()
        sleep( 3 )
    else
        warnlog( "File Open dialog is missing" )
    endif

    '///+<li>Verify that the document is opened read-only<br>This can be done by 
    '///+ travelling through the menus or trying to enter some text into the document.<br>
    '///+ In this case we try to insert a chart into the spreadsheet which should fail.</li>
    Kontext "DocumentCalc"
    try
        printlog( "Trying to insert a chart" )
        InsertChartCalc
        warnlog( "The document is not read-only" )
    catch
        printlog( "Failed to insert chart -> Document is read-only. Good!" )
    endcatch
        

    '///+<li>Click on &quot;Edit File&quot;<br>This should open another copy of the
    '///+ document. The copy should open in the foreground and be editable.</li>
    Kontext "Standardbar"
    hWaitForObject( Bearbeiten , 1000 )
    Bearbeiten.click()

    '///+<li>Verify that the file can be edited by inserting a Chart</li>
    InsertChartCalc
    Kontext "ChartWizard"
    if ( ChartWizard.exists( 2 ) ) then
        printlog( "Chart Wizard is open. Good." )
        
        '///+<li>Cancel Chart Wizard</li>
        printlog( "Cancel Chart Wizard" )
        ChartWizard.cancel()
        WaitSlot
    else
        warnlog( "Chart Wizard should be open" )
    endif
    

    '///</ul>
    '///+<li>Skipping to print file directly</li>

    '///+<li>Cut / Copy / Paste</li>

    '///<ul>
    '///+<li>Select a row in the spreadsheet</li>
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys( "<Up>", 7 )
    DocumentCalc.TypeKeys( "<Shift Down>", 5 )

    '///+<li>Click on Cut</li>
    printlog( "Cut" )
    Kontext "Standardbar"
    hWaitForObject( Ausschneiden , 1000 )
    Ausschneiden.Click()
    WaitSlot()

    '///+<li>Move one cell down</li>
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys( "<Down>" )

    '///+<li>Click on Paste</li>
    printlog( "Paste" )
    Kontext "Standardbar"
    hWaitForObject( Einfuegen , 8000 )
    Einfuegen.Click()

    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys( "<Up>", 7 )
    DocumentCalc.TypeKeys( "<Shift Down>", 5 )
    waitslot

    '///+<li>Select a row in the spreadsheet at random</li>
    '///+<li>Click on Copy</li>
    printlog( "Copy" )
    Kontext "Standardbar"
    hWaitForObject( Kopieren , 1000 )
    Kopieren.Click()
    waitslot

    '///+<li>Press right-arrow and <return></li>

    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys( "<Right><Return>" )
    waitslot

    '///+<li>Click on Paste</li>
    printlog( "Paste again" )
    Kontext "Standardbar"
    hWaitForObject( Einfuegen , 1000 )
    Einfuegen.Click()


    '///</ul>

    '///+<li>Undo / Redo</li>

    '///<ul>

    '///+<li>click on Undo</li>
    printlog( "Undo" )
    hWaitForObject( Undo , 2000 )
    Undo.Click()
    WaitSlot()
    
    '///+<li>click on Redo</li>
    hWaitForObject( Redo , 2000 )
    Redo.Click()
    WaitSlot()

    '///</ul>

    '///+<li>Navigator ( on/off )</li>

    '///<ul>
    '///+<li>Click on Navigator -> on</li>
    printlog( "Open Navigator" )
    Kontext "Standardbar"
    hWaitForObject( NavigatorButton , 1000 )
    NavigatorButton.Click()

    '///+<li>check if the navigator is visible</li>

    Kontext "Navigator"
    if ( Navigator.Exists( 1 ) ) then
        printlog( "Navigator present. Good." )
        
        '///+<li>Click on Navigator -> off</li>
        printlog( "Close Navigator" )
        Kontext "Standardbar"
        NavigatorButton.Click()

        '///+<li>Verify that the navigator is closed</li>
        Kontext "Navigator"
        if ( Navigator.exists() ) then
            warnlog( "The navigator is still open" )
        else
            printlog( "Navigator is closed. Good." )
        endif
    else
        warnlog( "Navigator did not open" )
    endif

    '///</ul>
    
    '///+<li>Hyperlink dialog ( on / off )</li>
    '///<ul>

    '///+<li>Click on Hyperlink dialog  -> on</li>
    printlog( "Click <HyperLink> to open the dialog" )
    Kontext "Standardbar"
    hWaitForObject( HyperlinkDialog , 1000 )
    HyperlinkDialog.Click()

    '///+<li>check if the Hyperlink dialog is visible
    Kontext "Hyperlink"
    if ( Hyperlink.Exists( 1 ) ) then
        printlog( "Hyperlink dialog is open" )
        
        '///+<li>Click on HyperLink again to close the dialog</li>
        printlog( "Click <HyperLink> to close" )
        kontext "Standardbar"
        HyperlinkDialog.Click()
        WaitSlot()
        kontext "HyperLink"
        if ( HyperLink.exists() ) then
            warnlog( "Hyperlink dialog not closed" )
        else
            printlog( "Hyperlink dialog has been closed. Good." )
        endif
    else
        warnlog( "Hyperlink dialog is not open" )
    endif

    '///+<li>click on Hyperlink dialog  -> off
    '///</ul>

    '///+<li>Gallery ( on / off )</li>

    '///<ul>
    '///+<li>Click on the &quot;Gallery&quot; button</li>
    printlog( "Gallery button" )
    Kontext "Standardbar"
    hWaitForObject( GalleryButton , 1000 
    GalleryButton.Click()
    
    kontext "Gallery"
    if ( Gallery.exists( 3 ) ) then
        printlog( "Gallery is open. Good." )
        
        '///+<li>Close the Gallery again</li>
        printlog( "Gallery button again, to close" )
        kontext "StandardBar"
        GalleryButton.click()
        waitslot
        
        kontext "Gallery" 
        if ( Gallery.exists() ) then    
            warnlog( "Gallery did not close " )
        else
            printlog( "Gallery is closed" )
        endif
    else
        warnlog( "The Gallery did not open" )
    endif
    '///</ul>
        
    '///+<li>Cleanup: Close both documents</li>
    do while( getDocumentCount > 0 ) 
        hDestroyDocument()
    loop

    
    '///+<li>Cleanup: Delete the workfile</li>
    hDeleteFile( sFileOut )

    '///</ul>

endcase


