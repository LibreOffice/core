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
'*  short description : Update test for Basic IDE / Dialog i18n-feature
'*
'\******************************************************************************

testcase tUpdtBasicDialogI18n

    printlog( "BASIC IDE Dialog i18n." )
    const MODULE_NAME = "tUpdtBasicDialogI18n"
    const DIALOG_DEFAULT_TIMEOUT = 2

    kontext "Navigator"
    hCloseDialog( Navigator, "close,optional" )
    hCreateDocument()
    
    if ( not hInitFormControls( MODULE_NAME ) ) then
        warnlog( "Failed to open Basic IDE / Dialogs / ToolsCollectionBar" )
        goto endsub
    endif
    
    printlog( "Click Manage Languages on ToolsCollectionBar")
    kontext "ToolsCollectionBar" 
    if ( ToolsCollectionBar.exists( DIALOG_DEFAULT_TIMEOUT ) ) then
        wait( 500 )
        hClickButton( ManageLanguage )
    else
        warnlog( "<ToolsCollectionBar> is not open, is the test environment dirty?" )
    endif
        
    kontext "ManageUILanguages"
    if ( ManageUILanguages.exists( DIALOG_DEFAULT_TIMEOUT ) ) then
        call dialogtest( ManageUILanguages )
        
        printlog( "Add default language, choose preselection")
        kontext "ManageUILanguages"
        hClickButton( Add )
        
        kontext "SetDefaultLanguage"
        if ( SetDefaultLanguage.exists( DIALOG_DEFAULT_TIMEOUT ) ) then
            call dialogtest( SetDefaultLanguage )
            
            kontext "SetDefaultLanguage"
            DefaultLanguageListbox.select( 3 )
            
            kontext "SetDefaultLanguage"
            hCloseDialog( SetDefaultLanguage, "ok" )
        else
            warnlog( "Dialog <SetDefaultLanguage> did not open" )
        endif  
       
        printlog( "Back on Manage UI Languages Dialog we click <Add>")
        kontext "ManageUILanguages"
        hClickButton( Add )
        
        kontext "AddUserInterface"
        if ( AddUserInterface.exists( DIALOG_DEFAULT_TIMEOUT ) ) then
            call dialogtest( AddUserInterface )

            printlog( "Add just another language at random")    
            kontext "AddUserInterface"
            AddNewControl.typeKeys( "<HOME>" )     ' select first item
            AddNewControl.typeKeys( "<SPACE>" )    ' check it

            kontext "AddUserInterface"
            hCloseDialog( AddUserInterface, "ok" )
        else
            warnlog( "Dialog <AddUserInterface> did not open" )
        endif
        
        kontext "ManageUILanguages"
        PresentLanguages.Select (1)
        
        printlog( "Back on Manage UI Languages Dialog, delete selected language")
        kontext "ManageUILanguages"
        hClickButton( Delete )

        printlog( "Confirm delete")
        kontext "active"
        if ( Active.exists( DIALOG_DEFAULT_TIMEOUT ) ) then
            call dialogtest( active )
            
            kontext "active"
            hCloseDialog( active, "ok" )
        else
            warnlog( "Confirm delete messagebox is missing" )
        endif
        
        printlog( "Leave Manage UI Languages Dialog")
        kontext "ManageUILanguages"
        hCloseDialog( ManageUILanguages, "ok" )
    else
        warnlog( "Manage UI languages did not open" )
    endif
    
    printlog( "Verify that the translationbar is visible")
    kontext "TranslationBar"
    if ( not TranslationBar.exists() ) then warnlog( "The IDE Translation Toolbar is not visible" )

    kontext "ToolsCollectionBar"
    hCloseDialog( ToolsCollectionBar, "close" )
    hClosebasicIDE()
    hDestroyDocument()

endcase

