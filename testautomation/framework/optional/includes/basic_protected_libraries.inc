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
'*
'*  short description : Test protected libraries
'*
'\******************************************************************************

testcase tProtectedLibraries

    
    const CLIB = "aaTestLib"
    const CPASSWORD = "SomePassword"
    const IMACRO = 3
    
    dim irc as integer
    dim cPBAFile as string
        cPBAFile = gOfficePath & "user\basic\" & CLIB & "\Module1.pba"
        cPBAFile = convertpath( cPBAFile )
        
    dim iPBAFile as long
    dim iCurrentLib as integer
    dim cMsg as string
    
    ToolsMacro_uno
    
    printlog( "Select My Macros" )
    kontext "Makro"
    hSelectNode( MakroAus , 1 )
    
    printlog( "Organize..." )
    Verwalten.click()
    
    hSelectBasicObjectOrganizerTab( 3 )
    
    printlog( "New..." )
    kontext "TabBibliotheken"
    Neu.click()
    
    printlog( "Name the Library" )
    kontext "NeueBibliothek"
    BibliotheksName.setText( CLIB )
    NeueBibliothek.ok()
    
    printlog( "Select the new library" )
    kontext "TabBibliotheken"
    for iCurrentLib = 1 to Bibliotheksliste.getItemCount()
        Bibliotheksliste.select( iCurrentLib )
        if ( Bibliotheksliste.getSelText = CLIB ) then
            exit for
        endif
    next iCurrentLib
    
    printlog( "Set Password" )
    kontext "TabBibliotheken"
    Passwort.click()
    
    kontext "PasswdDLG"
    NewPassword.setText( CPASSWORD )
    Confirm.setText( CPASSWORD )
    PasswdDLG.ok()
    
    printlog( "Edit the library" )
    kontext "TabBibliotheken"
    Bearbeiten.click()
    
    Kontext "BasicIDE"
    hInsertMacro( IMACRO )
    
    hCloseBasicIde()
    
    printlog( "Restart the application" )
    call ExitRestartTheOffice()
    
    ToolsMacro_uno
    
    printlog( "Select the protected library" )
    Kontext "Makro"
    hSelectNode( MakroAus , 1 )
    hExpandNode( MakroAus , 0 )
    hSelectNode( MakroAus , 2 )
    hExpandNode( MakroAus , 0 )
    
    printlog( "Enter correct Password" )
    kontext "PasswordDLG"
    Password.setText( CPASSWORD )
    PasswordDLG.ok()
    
    ' Handle possible errormessage (Happens if password is correct but not accepted)
    Kontext "Active"
    if ( Active.exists() ) then
        cMsg = Active.getText()
        cMsg = hRemoveLineBreaks( cMsg )
        printlog( "Unexpected messagebox: " & cMsg )
        warnlog( "#i65955# - CWS Warnings01: Protected libraries do not accept valid password" )
        Active.ok()
        Kontext "Makro"
        Makro.cancel()
        hDeleteLibrary( 1 , CLIB )
        goto endsub
    endif
    
    Kontext "Makro"
    hSelectNode( MakroAus , 3 )
    
    printlog( "Edit Library" )
    Bearbeiten.click()
    ' warnlog( "#144701 Crash on edit of password protected module" )
    
    printlog( "Check that the Macro is the correct one" )
    Kontext "BasicIDE"
    if ( BasicIDE.exists( 3 ) ) then
    
        irc = htestMacro( IMACRO )
        if ( irc <> IMACRO ) then
            warnlog( "#i54305 - Changes to protected macros lost on office restart" )
            
            iPBAFile = hGetFileSizeAsLong( cPBAFile )
            if ( iPBAFile = 0 ) then
                warnlog( "#i50568 - .pba-file has 0-byte size" )
            else
                printlog( "Module1.pba has " & iPBAFile & " Bytes" )
            endif
        endif
    

        printlog( "Cleanup" )
        hCloseBasicIde()
    else
        warnlog( "BasicIDE did not open within 3 seconds" )
    endif
    hDeleteLibrary( 1 , CLIB )

endcase

