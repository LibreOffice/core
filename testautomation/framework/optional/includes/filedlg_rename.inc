'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
'*
'*  OpenOffice.org - a multi-platform office productivity suite
'*
'*  $RCSfile: filedlg_rename.inc,v $
'*
'*  $Revision: 1.1 $
'*
'*  last change: $Author: jsi $ $Date: 2008-06-16 12:18:14 $
'*
'*  The Contents of this file are made available subject to
'*  the terms of GNU Lesser General Public License Version 2.1.
'*
'*
'*    GNU Lesser General Public License Version 2.1
'*    =============================================
'*    Copyright 2006 by Sun Microsystems, Inc.
'*    901 San Antonio Road, Palo Alto, CA 94303, USA
'*
'*    This library is free software; you can redistribute it and/or
'*    modify it under the terms of the GNU Lesser General Public
'*    License version 2.1, as published by the Free Software Foundation.
'*
'*    This library is distributed in the hope that it will be useful,
'*    but WITHOUT ANY WARRANTY; without even the implied warranty of
'*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
'*    Lesser General Public License for more details.
'*
'*    You should have received a copy of the GNU Lesser General Public
'*    License along with this library; if not, write to the Free Software
'*    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
'*    MA  02111-1307  USA
'*
'/******************************************************************************
'*
'*  owner : Joerg.Skottke@Sun.Com
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

