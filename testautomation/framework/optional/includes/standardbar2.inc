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
'* short description : global update test (Standardbar)
'*
'\***************************************************************************

testcase tStandardBar_2

    printlog( "Extended toolbar test for the standardbar" )

    if ( gtSysName = "Solaris x86" ) then
        qaerrorlog( "#i62423# - New Database crashes office (Evolution)" )
        goto endsub
    endif
 
    dim iCurrentApplication as Integer
    dim iNewButtonItem as Integer
    dim iPresentItemCount as integer
    
    hUseImpressAutopilot( false )
    hCreateDocument()
    
    kontext "standardbar"
    try
        Neu.openMenu()
        iPresentItemCount = hMenuItemGetCount()
    catch
        warnlog( "Failed to access New-Button, skipping test" )
        goto endsub
    endcatch
        
    if ( iPresentItemCount <> 12 ) then
        warnlog( "Items missing on New-Button, Expected: 12, Found:  " & _
                 iPresentItemCount & " -> Running limited test!" )
        iPresentItemCount = 8 ' the first eight usually work (no guarantee)
    endif
                 
    for iCurrentApplication = 1 to 7
    
        printlog( "" )
        printlog ( hNumericDoctype( iCurrentApplication ) )
        hNewDocument()

        for iNewButtonItem = 1 to iPresentItemCount
        
            ' For some reason Impress needs an extra kick...
            if ( iCurrentApplication = 3 ) then
                kontext "documentimpress"
                DocumentImpress.MouseDoubleClick ( 50, 50 )
            endif

            printlog( "Click on the " & iNewButtonItem & ". item on the new-button" )

            kontext "standardbar"
            try
                Neu.openMenu()
                sleep( 1 )
                iPresentItemCount = hMenuItemGetCount()
            catch
                warnlog( "Failed to access New-Button, skipping test" )
                goto endsub
            endcatch
            
            sleep( 1 )
            hMenuselectNr ( iNewButtonItem )
            hFileWait() ' was sleep( 3 )
            
            kontext "Navigator"
            hCloseDialog( Navigator, "close,optional" )
            
            select case iNewButtonItem
            case 1  :   Kontext "DocumentWriter"
                        DocumentWriter.TypeKeys( "Hallo" )
                        hDestroyDocument()
                        printlog( "- Writer document" )
            case 2  :   Kontext "DocumentCalc"
                        DocumentCalc.TypeKeys( "Hallo" )
                        hDestroyDocument()
                        printlog( "- Spreadsheet document" )
            case 3  :   Kontext "AutoPilotPraesentation1"
                        Kontext "DocumentImpress"
                        DocumentImpress.MouseDoubleClick ( 50, 50 )
                        hDestroyDocument()
                        printlog( "- Presentation document" )
            case 4  :   Kontext "DocumentDraw"
                        DocumentDraw.MouseDoubleClick ( 50, 50 )
                        hDestroyDocument()
                        printlog( "- Drawing document" )
            case 5  :   Kontext "DatabaseWizard"
                        CancelBtn.click()
                        printlog( "- Database wizard" )
            case 6  :   Kontext "DocumentWriter"     ' HTML Document
                        DocumentWriter.TypeKeys( "Hallo" )
                        hDestroyDocument()
                        printlog( "- HTML document" )
            case 7  :   Kontext "DocumentWriter"     ' XML Form
                        DocumentWriter.TypeKeys( "Hallo" )
                        hDestroyDocument()
                        printlog( "- XML Form" )
            case 8  :   Kontext "DocumentWriter"     ' Master Document
                        DocumentWriter.TypeKeys( "Hallo" )
                        hDestroyDocument()
                        printlog( "- Master document" )
            case 9  :   SchreibenInMathdok "a over b"
                        hDestroyDocument()
                        printlog( "- Formula document" )
            case 10 :   Kontext
                        Active.SetPage( TabEtiketten )
                        kontext "tabetiketten"
                        TabEtiketten.Cancel()
                        printlog( "- Labels (Labels dialog)" )
            case 11 :   Kontext
                        Active.SetPage( TabEtikettenMedium )
                        kontext "tabetikettenmedium"
                        TabEtikettenMedium.Cancel()
                        printlog( "- Business cards (Labels dialog)" )
            case 12 :   Kontext "TemplateAndDocuments"
                        TemplateAndDocuments.cancel()
                        printlog( "- Template dialog" )
            end select

            sleep( 2 )
            
        next iNewButtonItem

        hCloseDocument()
        
    next iCurrentApplication
    
    hDestroyDocument()
    
endcase


