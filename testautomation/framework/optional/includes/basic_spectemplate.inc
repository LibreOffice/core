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

    '///<h1>Load the spec template via http and enable macros</h1>
    '///<i>Required: Macro Security Level must be Default (Medium)</i>
    '///<ul>
    
    const cFile = "http://specs.openoffice.org/collaterals/template/2.0/OpenOffice-org-Specification-Template.ott"
    dim brc as boolean
    dim cProxy as string
    dim cPort as string
    
    dim cWorkFile as string
    
    cWorkFile = hGetWorkPath() & "SpecTemplate" & hGetSuffix( "current" )


    '///+<li>Retrieve the http-proxy from private environment</li>    
    hGetPrivateHttpServer( cProxy, cPort )

    '///+<li>Set HTTP-Proxy to access a website outside SWAN (not for OOo)</li>
    printlog( "Init: Set proxy (requires private environment to be set)" )
    hSetProxies( cProxy, cPort, "", "", "" )
    if ( WaitSlot() <> WSFinished ) then
        warnlog( "Slot not finished after 1000 msec." )
    else
        printlog( "Slot free in less than 1000 msec." )
    endif
    
    '///+<li>Open the file (using slot)</li>
    printlog( "FileOpen" )
    hFileOpen( cFile )
    
    '///+<li>Intercept a possible &quot;General Internet error&quot;</li>
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
    
    '///+<li>Handle security warning (allow execution of macros)</li>
    printlog( "Accept to execute macros" )
    brc = hAllowMacroExecution()
    if ( not brc ) then 
        warnlog( "Missing Macro execution dialog. Please check the file/security settings" )
    endif
    
    '///+<li>Wait until the document is loaded</li>
    '///+<li>Check for possible warning dialog: Could not resolve host -> Fatal error</li>
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
    
    '///+<li>Go to the end of the document, write some text</li>
    printlog( "Change the document")
    kontext "DocumentWriter"
    DocumentWriter.typeKeys( "<MOD1 END>" )
    DocumentWriter.typeKeys( "Changing the document" )
    
    '///+<li>Disable the help item from the Spectemplate-menu</li>
    printlog( "Disable Help Entry from Spec-Template Menu")
    hUseMenu()
    hMenuSelectNr( 10 )
    hMenuSelectNr( 2 )
    
    
    '///+<li>Save the file</li>
    hFileSaveAsKill( cWorkFile )
    
    '///+<li>Close the document</li>
    hDestroyDocument()
    
    '///+<li>Reload the file</li>
    hFileOpen( cWorkFile )
    
    '///+<li>Allow Macro execution</li>
    hAllowMacroExecution()
    
    '///+<li>Close the document</li>
    printlog( "Cleanup: Close the document" )
    hDestroyDocument()
    
    '///+<li>Reset proxies (delete all proxies, if changed)</li>
    printlog( "Cleanup: Delete proxy settings" )
    hSetProxies( "", "", "", "", "" )
    
    '///+<li>Remove the workfile</li>
    hDeleteFile( cWorkFile )
    '///</ul>

endcase

