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
'*  short description : Update test for documentconverter
'*
'\******************************************************************************

testcase tUpdtWizardDocumentConverter

    printlog( "Resource test for the document converter" )
    
    dim irc as integer
    dim brc as boolean
    dim iDialog as integer
    dim iDocumentType as integer
    dim iWait as integer
    dim sSourcePathWriter as string
        sSourcePathWriter = gTesttoolpath & "framework\required\input\document_converter\"
        sSourcePathWriter = convertpath( sSourcePathWriter )
    dim sTargetFile as string
    dim sLogFile as string
    dim aFileList( 100 ) as string
    dim iCurrentFile as integer
    
    hInitSingleDoc()
    sTargetFile = hGetWorkPath() 
    sLogFile = sTargetFile & "Logfile.odt"
    sTargetFile = sTargetFile & "docconv1" & hGetSuffix( "current" )

    FileWizardDocumentConverter
    
    Kontext "DocumentConverter"
    hWaitForObject( CreateLogfile, 3000 )
    CreateLogfile.check()

    Call DialogTest ( DocumentConverter )
    
    spreadsheet.check()
    textdoc.check()
    drawing.check()
    master.check()
    
    
    ' as we selected all documenttypes, the settings page for each
    ' documenttype has to pop up
    for iDocumentType = 1 to 4
    
        ContinueButton.click()
        
        Kontext "DocumentConverter"
        if ( template.isVisible() ) then
            printlog( "Page " & 1 + iDocumentType & " is visible" )
        else
            warnlog( "Cannot access page 2, aborting test" )
            hCloseDialog( DocumentConverter, "Cancel" )
            goto endsub
        endif
        
        for iDialog = 1 to 4 
        
            select case iDialog
            case 1 : ImportFormTemplatesSearch.click()
            case 2 : SaveToTemplatesSearch.Click()
            case 3 : ImportFormDocumentSearch.Click()
            case 4 : SaveToDocumentSearch.Click()
            end select
            
            Kontext "OeffnenDLG"
            Call Dialogtest (OeffnenDlg)
            OeffnenDLG.Cancel()
            
            Kontext "DocumentConverter"
            ImportFormDocument.setText( sSourcePathWriter )
            
        next iDialog
        
    next iDocumentType
    
    Kontext "DocumentConverter"
    ContinueButton.click()
    
    Kontext "DocumentConverter"
    if ( summary.isVisible() ) then
        printlog( "Page 6 is visible" )
    else
        warnlog( "Page 6 - Summary Page is missing" )
    endif
    
    Kontext "DocumentConverter"
    ContinueButton.click()

    Kontext "DocumentConverter"
    printlog( "Page 7" )
    
    qaerrorlog( "#i54265# Show Logfile button has no HID, using accelerator instead" )
    
    iWait = 0
    do while ( DocumentConverter.exists() )
        try
            ContinueButton.click()
        catch
        endcatch
        Wait( 1 )
        iWait = iWait + 1
        if ( iWait = 20000 ) then
            warnlog( "Document not converted within 20 seconds" )
            exit do
        endif
    loop
    
    if ( getDocumentCount <> 1 ) then
        warnlog( "Exactly one - the conversion result document - should be open" )
    endif
    
    hDestroyDocument()
    hDeleteFile( sTargetFile )
    hDeleteFile( sLogFile )

endcase

