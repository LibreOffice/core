'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: options_ooo_security.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:15 $
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
'* owner : thorsten.bosbach@sun.com
'*
'* short description : Tools->Options: OpenOffice.org Security
'*
'\******************************************************************************

testcase tOOoSecurity
    '///Test the OpenOffice.org/Security-Tabpage in Tools/Options
    '///<ul>    
    dim _file as string
    
    '///<li>check defaults</li>
    if ( getDocumentCount() < 1 ) then call hNewDocument()
    _file = gTesttoolPath & "framework\optional\input\options\ooo_security_defaults.ref"
    checkSecurityPage( _file )

    '///<li>change the settings</li>    
    if ( getDocumentCount() < 1 ) then call hNewDocument()
    _file = gTesttoolPath & "framework\optional\input\options\ooo_security_changed.ref"
    changeSecurityPage( _file )
    
    '///<li>check the changes</li>
    if ( getDocumentCount() < 1 ) then call hNewDocument()
    checkSecurityPage( _file )
    
    '///<li>revert to default settings</li>
    if ( getDocumentCount() < 1 ) then call hNewDocument()
    _file = gTesttoolPath & "framework\optional\input\options\ooo_security_defaults.ref"
    changeSecurityPage( _file )
    
    '///<li>check the changes</li>
    if ( getDocumentCount() < 1 ) then call hNewDocument()
    checkSecurityPage( _file )
    
    '///</ul>
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
        
        Kontext "TabSecurity"
        if ( TabSecurity.exists() ) then
            checkCheckBox( _file , "*" , RecommendToOpenDocumentReadOnly )
            checkCheckBox( _file , "*" , RecordChanges )   
        else
            warnlog( "Could not return to Security Tabpage" )
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
    
    Options.click
    Kontext "TabSecurityOptionsAndWarnings"
    setCheckBox( _file , "*" , SavingOrSendingDocuments )
    setCheckBox( _file , "*" , SigningDocuments )
    setCheckBox( _file , "*" , PrintingDocuments )
    setCheckBox( _file , "*" , CreatingPDFfiles )
    setCheckBox( _file , "*" , RemovePersonalInformationOnSaving )
    setCheckBox( _file , "*" , RecommendPasswordProtectionOnSaving )
    TabSecurityOptionsAndWarnings.cancel
    Kontext "TabSecurity"
    setCheckBox( _file , "*" , RecommendToOpenDocumentReadOnly )
    setCheckBox( _file , "*" , RecordChanges )
    
    Kontext "OptionenDLG"
    OptionenDLG.OK
   
    sleep(2)
    call exitRestartTheOffice()
end sub


