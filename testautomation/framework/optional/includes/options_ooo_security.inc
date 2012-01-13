'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************

*****

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


