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
'/******************************************************************************
'*
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Update test for the Euroconverter
'*
'\******************************************************************************

testcase tUpdtWizardEuroconverter

    printlog( "Resource test for the euro-converter" )

    dim irc as integer
    dim iWait as integer
    dim cMsg as string 
    
    dim iKnownCurrencies as integer
    const KNOWN_CURRENCIES = 16
    const TARGET_FILE = "barfly.ods"
    
    
    dim cSourceDir as string
        cSourceDir = gTesttoolPath & "framework\required\input"
        cSourceDir = convertpath( cSourceDir )
        
    dim cTargetDir as string
        cTargetDir = hGetWorkPath()

    hInitSingleDoc()
    
    FileWizardEuroConverter
    
    kontext "AutopilotEuroKonverter"
    call DialogTest( AutopilotEuroKonverter , 1 )
    
    Kontext "AutopilotEuroKonverter"
    iKnownCurrencies = Waehrungen.getItemCount()
    if ( iKnownCurrencies <> KNOWN_CURRENCIES ) then 
        warnlog( "The number of currencies in the currencies listbox is incorrect." )
        printlog( "Found...: " & iKnownCurrencies ) 
        printlog( "Expected: " & KNOWN_CURRENCIES )
    endif
    
    if ( not Verzeichnis.isChecked() ) then
        warnlog( "Converting entire directories should be default" )
        Verzeichnis.Check()
    endif
    
    kontext "AutopilotEuroKonverter"
    QuelleSuchen.click()
    
    kontext "FilePicker"
    EFFilename.setText( cSourceDir )
    PBSelect.click()
    
    kontext "AutopilotEuroKonverter"
    ZielSuchen.click()
    
    kontext "FilePicker"
    EFFilename.setText( cTargetDir )
    PBSelect.click()
    WaitSlot()
    
    kontext "AutopilotEuroKonverter"
    Waehrungen.Select(1) 'Setting to first entry of listbox
    
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

    hClickButton( Konvertieren )
    hDestroyDocument() 
    hDeleteFile( cTargetDir & TARGET_FILE )
    
endcase

