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
'*  short description : Load the spec template via http and enable macros
'*
'\******************************************************************************

testcase tBasicSpecTemplate

    const cFile = "http://specs.openoffice.org/collaterals/template/2.0/OpenOffice-org-Specification-Template.ott"
    dim cWorkFile as string : cWorkFile = hGetWorkPath() & "SpecTemplate" & hGetSuffix( "current" )

    printlog( "FileOpen" )
    hFileOpen( cFile )
    
    kontext "Active"
    if ( Active.exists( 2 ) ) then
        warnlog( "Unexpected errormessage: " & active.getText()
        active.ok()
        
        kontext "OeffnenDlg"
        if ( OeffnenDlg.exists( 1 ) ) then
            printlog( "Recovering - closing File Open dialog" )
            hCloseDialog( OeffnenDlg, "cancel" )
        endif
        
        goto endsub
    endif
    
    printlog( "Accept to execute macros" )
    if ( not hAllowMacroExecution() ) then
        warnlog( "Missing Macro execution dialog. Aborting test" )
        goto endsub
    endif

    kontext "Active"
    if ( Active.exists( 10 ) ) then
        warnlog( "Fatal: Unexpected active: " & Active.getText() )
        active.ok
        
        kontext "OeffnenDlg"
        if ( OeffnenDlg.exists( 2 ) ) then
            hCloseDialog( OeffnenDlg, "close" )
            goto endsub
        endif
    endif
    
    printlog( "Change the document")
    kontext "DocumentWriter"
    DocumentWriter.typeKeys( "<MOD1 END>" )
    DocumentWriter.typeKeys( "Changing the document" )
    
    printlog( "Disable Help Entry from Spec-Template Menu")
    hUseMenu()
    hMenuSelectNr( 10 )
    hMenuSelectNr( 2 )

    ' This one is required as the document is rendering for a while and we cannot 
    ' tell when it is finished (WaitSlot does not work here)
    wait( 5000 )

    printlog( "Save the file, overwriting" )
    hFileSaveAsKill( cWorkFile )

    printlog( "Close the document" )
    hDestroyDocument()

    printlog( "Load the file again" )
    hFileOpen( cWorkFile )
    hAllowMacroExecution()

    ' This sleep here is needed after loading the document otherwise the document is closed too early during execution of the macros, which results in basic runtime error messagebox, that can not be handled.
    SLEEP(1)
    
    printlog( "Cleanup: Close the document and delete the file" )
    hDestroyDocument()
    hDeleteFile( cWorkFile )
endcase

