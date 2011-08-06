'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
'*
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
'*  short description : Remane files dialog
'*
'\******************************************************************************

testcase tFileRename

    dim cPath as string
        cPath = convertpath( gOfficePath & "user/work/" )
        
    dim cFile as string
        cFile = "tFileRename.odt"
        
    dim iCurrentFile as integer
    dim cCurrentFile as string
    dim iFileCount as integer
    dim bFileFound as boolean : bFileFound = false

    ' Issue #i88446
    Printlog( "Rename file dialog" )
    
    gApplication = "WRITER"
    hNewDocument()
    DocumentWriter.typeKeys( "framework/optional/filedlg_dialogtest.bas::tFileRename" )
    
    hFileSaveAsKill( cPath & cFile )
    FileClose
    FileOpen
    kontext "OeffnenDlg"
    if ( OeffnenDlg.exists( 2 ) ) then

        DateiName.setText( cPath )
        Oeffnen.click()
        iFileCount = DateiAuswahl.getItemCount()
        printlog( iFileCount & " files listed" )
        DateiAuswahl.typeKeys( "<HOME>" )

        for iCurrentFile = 1 to iFileCount

            wait( 100 )
            DateiAuswahl.typeKeys( "<SPACE>" )
            wait( 100 )
            
            cCurrentFile = DateiAuswahl.getSelText()
            printlog( " * " & cCurrentFile )
            
            if ( cCurrentFile = cFile ) then
                printlog( "File found" )
                DateiAuswahl.openContextMenu()
                hMenuSelectNr( 2 )
                DateiAuswahl.typeKeys( "<ESCAPE>" )
                kontext "OeffnenDlg"
                OeffnenDlg.cancel()
                exit for
            endif
            
            wait( 100 )
            DateiAuswahl.typeKeys( "<DOWN>" )
            wait( 100 )
            
        next iCurrentFile
    endif
    
    if ( FileExists( cPath & cFile ) ) then kill( cPath & cFile )
    

endcase

