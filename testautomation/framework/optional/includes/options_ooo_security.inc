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
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : Tools->Options: OpenOffice.org Security
'*
'\******************************************************************************

testcase tOOoSecurity
    dim _file as string
    
    if ( getDocumentCount() < 1 ) then call hNewDocument()
    _file = gTesttoolPath & "framework\optional\input\options\ooo_security_defaults.ref"
    checkSecurityPage( _file )

    if ( getDocumentCount() < 1 ) then call hNewDocument()
    _file = gTesttoolPath & "framework\optional\input\options\ooo_security_changed.ref"
    changeSecurityPage( _file )
    
    if ( getDocumentCount() < 1 ) then call hNewDocument()
    checkSecurityPage( _file )
    
    if ( getDocumentCount() < 1 ) then call hNewDocument()
    _file = gTesttoolPath & "framework\optional\input\options\ooo_security_defaults.ref"
    changeSecurityPage( _file )
    
    if ( getDocumentCount() < 1 ) then call hNewDocument()
    checkSecurityPage( _file )
    
    kontext "ExtrasOptionenDlg"
    if ( ExtrasOptionenDlg.exists() ) then
        ExtrasOptionenDlg.ok()
    else
        warnlog( "options dialog not available" 
    endif
    if ( getDocumentCount() > 0 ) then call hCloseDocument()

endcase

'*******************************************************************************

sub checkSecurityPage( _file as string )
    _file = convertpath( _file )

    printlog( " * Testing current settings against a reference list." )
    printlog( " * Using settings from file: " & _file )

    ToolsOptions
    call hToolsOptions("StarOffice", "Security")
    
    kontext "TabSecurity"
    if ( TabSecurity.exists() ) then
       
        Options.click
        
        Kontext "TabSecurityOptionsAndWarnings"
        if ( TabSecurityOptionsAndWarnings.exists( 2 ) ) then
            checkCheckBox( _file , "*" , SavingOrSendingDocuments )
            checkCheckBox( _file , "*" , SigningDocuments )
            checkCheckBox( _file , "*" , PrintingDocuments )
            checkCheckBox( _file , "*" , CreatingPDFfiles )
            checkCheckBox( _file , "*" , RemovePersonalInformationOnSaving )
            checkCheckBox( _file , "*" , RecommendPasswordProtectionOnSaving )
            TabSecurityOptionsAndWarnings.cancel
        else
            warnlog( "Failed to open Security options" )
        endif
        
    else
        warnlog( "Security Tabpage not available" )
    endif
end sub

'*******************************************************************************

sub changeSecurityPage( _file as string )
    _file = convertpath( _file )
    
    printlog( " * Changing current settings according to the reference list." )
    printlog( " * Using settings from file: " & _file )

    kontext "TabSecurity"
    if ( TabSecurity.exists() ) then
    
    Options.click
    Kontext "TabSecurityOptionsAndWarnings"
    setCheckBox( _file , "*" , SavingOrSendingDocuments )
    setCheckBox( _file , "*" , SigningDocuments )
    setCheckBox( _file , "*" , PrintingDocuments )
    setCheckBox( _file , "*" , CreatingPDFfiles )
    setCheckBox( _file , "*" , RemovePersonalInformationOnSaving )
    setCheckBox( _file , "*" , RecommendPasswordProtectionOnSaving )
    TabSecurityOptionsAndWarnings.cancel
    
    Kontext "OptionenDLG"
    OptionenDLG.OK
    else
        warnlog( "Security Tabpage not available" )
    endif
   
    sleep(2)
    call exitRestartTheOffice()
end sub


