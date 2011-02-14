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


