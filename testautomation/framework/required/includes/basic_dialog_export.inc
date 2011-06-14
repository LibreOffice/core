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
'*  short description : Update test for the dialog export feature
'*
'\******************************************************************************

testcase tUpdtDialogExport

    printlog( "Update test for the dialog export feature" )

    dim brc as boolean
    dim cMsg as string
    
    dim cDlgName as string
        cDlgName = hGetWorkPath() & "DialogTest"
        
    const DLG_SUFFIX = ".xdl"
    
    hCreateDocument()

    printlog( "Test init: Setting up environment" )
    brc = hInitFormControls( "DialogExport" )
    if ( not brc ) then
        warnlog( "Failed to initialize BAIC IDE/Dialog editor, aborting" )
        goto endsub
    endif
    
    printlog( "Test begin" )
    kontext "DialogBar"
    if ( not DialogBar.exists() ) then
        warnlog( "DialogBar does not exist, aborting" )
        hClosebasicIDE()
        hDestroyDocument()
        goto endsub
    endif
    sleep(1)
    printlog( "Click the Export button on the DialogBar" )
    kontext "DialogBar"
    Export.click()
    
    kontext "ExportierenDlg"
    if ( not ExportierenDlg.exists( 1 ) ) then
        warnlog( "Export dialog is not open, aborting" )
        hCloseBasicIDE()
        hDestroyDocument()
        goto endsub
    endif
    
    printlog( "Export dialog is open. Good." )
    
    kontext "ExportierenDlg"
    DateiName.setText( cDlgName )
    Speichern.click()
    
    kontext "Active"
    if ( Active.exists() ) then
        cMsg = active.getText()
        cMsg = hRemoveLineBreaks( cMsg )
        printlog( "File appears to exist, overwriting: " & cMsg )
        Active.Yes()
    endif
    
    kontext "DialogBar"
    if ( not DialogBar.exists() ) then
        warnlog( "DialogBar is not visible, please check." )
    endif
    
    printlog( "Back on the Basic IDE / Dialog Editor. Good." )
        
    cDlgName = cDlgName & DLG_SUFFIX
    if ( dir( cDlgName ) = "" ) then
        warnlog( "The dialog was not saved to the expected location" )
    else
        hDeleteFile( cDlgName )
        printlog( "Dialog was exported and deleted. Good." )
    endif
    
    printlog( "Test end. Cleanup" )
    hCloseBasicIDE()
    hDestroyDocument()    

endcase

