'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
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

