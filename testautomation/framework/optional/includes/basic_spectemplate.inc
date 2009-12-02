'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_spectemplate.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:13 $
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
'/******************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Load the spec template via http and enable macros
'*
'\******************************************************************************

testcase tBasicSpecTemplate

    qaerrorlog( "#i89554# Macro dialog not displayed / document not loaded" )
    goto endsub

    
    const cFile = "http://specs.openoffice.org/collaterals/template/2.0/OpenOffice-org-Specification-Template.ott"
    dim brc as boolean
    dim cProxy as string
    dim cPort as string
    
    dim cWorkFile as string
    
    cWorkFile = hGetWorkPath() & "SpecTemplate" & hGetSuffix( "current" )


    hGetPrivateHttpServer( cProxy, cPort )

    printlog( "Init: Set proxy (requires private environment to be set)" )
    hSetProxies( cProxy, cPort, "", "", "" )
    if ( WaitSlot() <> WSFinished ) then
        warnlog( "Slot not finished after 1000 msec." )
    else
        printlog( "Slot free in less than 1000 msec." )
    endif
    
    printlog( "FileOpen" )
    hFileOpen( cFile )
    
    kontext "Active"
    if ( Active.exists( 2 ) ) then
        warnlog( "Unexpected errormessage: " & active.getText()
        active.ok()
        
        kontext "OeffnenDlg"
        if ( OeffnenDlg.exists( 1 ) ) then
            printlog( "Recovering - closing File Open dialog" )
            OeffnenDlg.cancel()
        endif
        
        goto endsub
    endif
    
    printlog( "Accept to execute macros" )
    brc = hAllowMacroExecution()
    if ( not brc ) then 
        warnlog( "Missing Macro execution dialog. Please check the file/security settings" )
    endif
    
    kontext "Active"
    if ( Active.exists( 5 ) ) then
        warnlog( "Fatal: Unexpected active: " & Active.getText() )
        active.ok
        
        kontext "OeffnenDlg"
        if ( OeffnenDlg.exists( 2 ) ) then
            OeffnenDlg.close()
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
    
    
    hFileSaveAsKill( cWorkFile )
    
    hDestroyDocument()
    
    hFileOpen( cWorkFile )
    
    hAllowMacroExecution()
    
    printlog( "Cleanup: Close the document" )
    hDestroyDocument()
    
    printlog( "Cleanup: Delete proxy settings" )
    hSetProxies( "", "", "", "", "" )
    
    hDeleteFile( cWorkFile )

endcase

