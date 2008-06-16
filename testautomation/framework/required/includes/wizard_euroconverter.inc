'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: wizard_euroconverter.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:19:04 $
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
'/******************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Update test for the Euroconverter
'*
'\******************************************************************************

testcase tUpdtWizardEuroconverter

    dim irc as integer
    dim iWait as integer
    dim cMsg as string 
    
    dim iKnownCurrencies as integer
    const KNOWN_CURRENCIES = 13
    const TARGET_FILE = "barfly.ods"
    
    
    dim cSourceDir as string
        cSourceDir = gTesttoolPath & "framework\required\input"
        cSourceDir = convertpath( cSourceDir )
        
    dim cTargetDir as string
        cTargetDir = hGetWorkPath()

    '///<h3>Update test for the Euroconverter</h3>
    '///<ul>
    
    '///+<li>Make sure exactly one single writer document is open</li>
    hInitSingleDoc()
    
    '///+<li>Trigger the Euroconverter</li>
    irc = hOpenWizardWithMenu( "EUROCONV" )
    if ( irc <> 0 ) then
        warnlog( "Failed to open the Euroconverter, aborting test" )
        hDestroyDocument()
        goto endsub
    endif
    
    kontext "AutopilotEuroKonverter"
    call dialogtest2( AutopilotEuroKonverter , 1 )
    
    '///+<li>Change the currency to anything else</li>
    Kontext "AutopilotEuroKonverter"
    iKnownCurrencies = Waehrungen.getItemCount()
    if ( iKnownCurrencies <> KNOWN_CURRENCIES ) then 
        warnlog( "The number of currencies in the currencies listbox is incorrect." )
        printlog( "Found...: " & iKnownCurrencies ) 
        printlog( "Expected: " & KNOWN_CURRENCIES )
    endif
    
    '///+<li>Check to convert an entire directory (this should be default anyway)</li>
    if ( not Verzeichnis.isChecked() ) then
        warnlog( "Converting entire directories should be default" )
        Verzeichnis.Check()
    endif
    
    '///+<li>Open the filepicker for the source directory, enter valid path</li>
    kontext "AutopilotEuroKonverter"
    QuelleSuchen.click()
    
    kontext "FilePicker"
    EFFilename.setText( cSourceDir )
    PBSelect.click()
    
    '///+<li>Open the filepicker for the target directory, enter valid path (workdir)</li>
    kontext "AutopilotEuroKonverter"
    ZielSuchen.click()
    
    kontext "FilePicker"
    EFFilename.setText( cTargetDir )
    PBSelect.click()
    
    '///+<li>Selecting a currency</li>
    kontext "AutopilotEuroKonverter"
    Waehrungen.Select(1) 'Setting to first entry of listbox
    
    '///+<li>Convert the file(s)</li>
    Konvertieren.click()
    
    iWait = 0
    do while( not Konvertieren.isEnabled() ) 
    
        iWait = iWait + 1
        printlog( "Waiting for document to get converted..." )
        
        if ( iWait = 20000 ) then
            warnlog( "Document(s) were not converted within 20 seconds, aborting" )
            exit do
        endif
        
        ' If the target file already exists we get an overwrite warning
        kontext "Active"
        if ( Active.exists() ) then
            cMsg = active.getText()
            cMsg = hRemoveLineBreaks( cMsg )
            warnlog( "Unexpected message: " & cMsg )
            active.yes()
        endif

        Wait( 1 )
        kontext "AutopilotEuroKonverter"
    
    loop
    
    kontext "AutopilotEuroKonverter"
    call DialogTest( AutopilotEuroKonverter , 2 )

    '///+<li>Click finish to close the EuroConverter</li>    
    hFinishWizard( 1 )
    
    '///+<li>Close the workdocument</li>
    hDestroyDocument() 
    
    '///+<li>Delete the workfile</li>
    hDeleteFile( cTargetDir & TARGET_FILE )
    
    '///</ul>

endcase

