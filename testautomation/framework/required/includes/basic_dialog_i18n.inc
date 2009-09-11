'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_dialog_i18n.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:19:03 $
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
'*  short description : Update test for Basic IDE / Dialog i18n-feature
'*
'\******************************************************************************

testcase tUpdtBasicDialogI18n

    printlog( "BASIC IDE Dialog i18n." )

    dim brc as boolean
    
    hCloseNavigator()
    hCreateDocument()'
    
    brc = hInitFormControls( "tDialogI18n" )
    if ( not brc ) then
        warnlog( "Failed to open Basic IDE / Dialogs / ToolsCollectionBar" )
        goto endsub
    endif
    
    printlog( "Click Manage Languages on ToolsCollectionBar")
    kontext "ToolsCollectionBar" 
    if ( ToolsCollectionBar.exists() ) then
        ManageLanguage.click()
    else
        warnlog( "<ToolsCollectionBar> is not open, is the test environment dirty?" )
    endif
        
    kontext "ManageUILanguages"
    if ( ManageUILanguages.exists( 2 ) ) then
        call dialogtest( ManageUILanguages )
        
        printlog( "Add default language, choose preselection")
        kontext "ManageUILanguages"
        Add.click()
        
        kontext "SetDefaultLanguage"
        if ( SetDefaultLanguage.exists( 2 ) ) then
            call dialogtest( SetDefaultLanguage )
            
            kontext "SetDefaultLanguage"
            DefaultLanguageListbox.select( 3 )
            
            kontext "SetDefaultLanguage"
            SetDefaultLanguage.ok()
        else
            warnlog( "Dialog <SetDefaultLanguage> did not open" )
         endif  
       
        printlog( "Back on Manage UI Languages Dialog we click <Add>")
        kontext "ManageUILanguages"
        Add.click()
        
        kontext "AddUserInterface"
        if ( AddUserInterface.exists( 2 ) ) then
            call dialogtest( AddUserInterface )

            printlog( "Add just another language at random")    
            kontext "AddUserInterface"
            AddNewControl.typeKeys( "<HOME>" )     ' select first item
            AddNewControl.typeKeys( "<SPACE>" )    ' check it

            kontext "AddUserInterface"
            AddUserInterface.ok()
        else
            warnlog( "Dialog <AddUserInterface> did not open" )
        endif
        
        kontext "ManageUILanguages"
        PresentLanguages.Select (1)
        
        printlog( "Back on Manage UI Languages Dialog, delete selected language")
        kontext "ManageUILanguages"
        Delete.click()
        
        printlog( "Confirm delete")
        kontext "active"
        if ( Active.exists( 1 ) ) then
            call dialogtest( active )
            
            kontext "active"
            active.ok()
        else
            warnlog( "Confirm delete messagebox is missing" )
        endif
        
        printlog( "Leave Manage UI Languages Dialog")
        kontext "ManageUILanguages"
        ManageUILanguages.ok()
    else
        warnlog( "Manage UI languages did not open" )
    endif
    
    printlog( "Verify that the translationbar is visible")
    kontext "TranslationBar"
    if ( not TranslationBar.exists() ) then
        warnlog( "The IDE Translation Toolbar is not visible" )
    endif

    kontext "ToolsCollectionBar"
    if ( not ToolsCollectionBar.exists() ) then
        warnlog( "The ToolsCollectionBar is not visible" )
    endif

    printlog( "Cleanup" )
    ToolsCollectionBar.Close
    hClosebasicIDE()
    hDestroyDocument()

endcase

