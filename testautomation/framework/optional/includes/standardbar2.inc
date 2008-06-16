'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: standardbar2.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:16 $
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

testcase tStandardBar_2

    if ( gtSysName = "Solaris x86" ) then
        qaerrorlog( "#i62423# - New Database crashes office (Evolution)" )
        goto endsub
    endif
 

    '///<h1>Update test: Create all documenttypes from all applications via 
    '///+ &quot;New-Button&quot; on Standard Bar</h1>

    dim iCurrentApplication as Integer
    dim iNewButtonItem as Integer
    dim iPresentItemCount as integer
    
    '///<u>Note</u>: The &quot;New-Button&quot; has 12 items in a FULL office
    '///+ installation. The test will not handle partial installations!
    '///<ol>
    '///+<li>Writer Document</li>
    '///+<li>Calc Document</li>
    '///+<li>Impress Document</li>
    '///+<li>Draw Document</li>
    '///+<li>Database Wizard</li>
    '///+<li>HTML Document</li>
    '///+<li>XML Form</li>
    '///+<li>Master Document</li>
    '///+<li>Math Document</li>
    '///+<li>Labels</li>
    '///+<li>Cards</li>
    '///+<li>Templates and documents</li>
    '///</ol>    
    
    '///<i>Do this for all 12 applications</i>
    '///<ul>
    
    '///+<li>Create exactly one single unmodified document<li>
    hInitSingleDoc()
    
    '///+<li>Verify that this is a full installations (12 items on &quot;New-Button&quot;)</li>
    kontext "standardbar"

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
    
        '///+<li>Create a new document</li>
        printlog( "" )
        printlog ( hNumericDoctype( iCurrentApplication ) )
        hCreateDocument()

        '///+<li>Click the &quot;New-Button&quot; - all 12 items</li>
        for iNewButtonItem = 1 to iPresentItemCount
        
            ' If Unix platform and application is Impress, get the focus of 
            ' the document in each loop.

            if ( iCurrentApplication = 3 AND gPlatgroup = "unx" ) then
                kontext "documentimpress"
                DocumentImpress.MouseDoubleClick ( 50, 50 )
            endif

            printlog "   click on the " + iNewButtonItem + ". item on the new-button"

            kontext "standardbar"
            try
                Neu.openMenu()
                sleep( 1 )
                iPresentItemCount = hMenuItemGetCount()
            catch
                warnlog( "Failed to access New-Button, skipping test" )
                goto endsub
            endcatch
            
            sleep (1)
            hMenuselectNr ( iNewButtonItem )
            sleep (3)
            
            '///+<li>Check if all applications or dialogs are opened correctly</li>
            '///+<li>Close the document or dialog</li>

            select case iNewButtonItem
            case 1  :   Kontext "DocumentWriter"
                        DocumentWriter.TypeKeys( "Hallo" )
                        hDestroyDocument()
            case 2  :   Kontext "DocumentCalc"
                        DocumentCalc.TypeKeys( "Hallo" )
                        hDestroyDocument()
            case 3  :   Kontext "AutoPilotPraesentation1"
                        if ( AutoPilotPraesentation1.exists( 2 ) ) then
                            Startwithwizard.unCheck()
                            AutopilotPraesentation1.ok()
                        endif
                        Kontext "DocumentImpress"
                        DocumentImpress.MouseDoubleClick ( 50, 50 )
                        hDestroyDocument()
            case 4  :   Kontext "DocumentDraw"
                        DocumentDraw.MouseDoubleClick ( 50, 50 )
                        hDestroyDocument()
            case 5  :   Kontext "DatabaseWizard"
                        CancelBtn.click()
            case 6  :   Kontext "DocumentWriter"     ' HTML Document
                        DocumentWriter.TypeKeys( "Hallo" )
                        hDestroyDocument()
            case 7  :   Kontext "DocumentWriter"     ' XML Form
                        DocumentWriter.TypeKeys( "Hallo" )
                        hDestroyDocument()
            case 8  :   Kontext "DocumentWriter"     ' Master Document
                        DocumentWriter.TypeKeys( "Hallo" )

                        kontext "navigator"
                        if Navigator.Exists( 5 ) then
                            Navigator.Close()
                        endif
                        hDestroyDocument()
            case 9  :   SchreibenInMathdok "a over b"
                        hDestroyDocument()
            case 10 :   Kontext
                        Active.SetPage( TabEtiketten )

                        kontext "tabetiketten"
                        TabEtiketten.Cancel()
            case 11 :   Kontext
                        Active.SetPage()

                        kontext "tabetikettenmedium"
                        TabEtikettenMedium.Cancel()
            case 12 :   Kontext "TemplateAndDocuments"
                        TemplateAndDocuments.cancel()
            end select

            sleep(2)
            
        next iNewButtonItem

        '///+<li>Close the document</li>
        hDestroyDocument()
        
    next iCurrentApplication
    '///</ul>
    '///</ul>

endcase


