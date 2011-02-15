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
'*  short description : Test correct handling of the default language
'*
'\******************************************************************************

testcase tBasicDialogI18nDefaults


    const MODULE_NAME = "defmod"
    const FILE_NAME   = "basic_defaults.odt"
    
    dim cWorkPath as string
        cWorkPath = hGetWorkPath() & FILE_NAME

    dim brc as boolean
    dim cMsg as string

    dim cDefLangSelect as string   ' Default language from Select Default Language Dialog
    dim cDefLangDialog as string   ' Default language from Manage UI Languages dialog
    dim cDefLangToolbar as string  ' Default language from Translation Toolbar
    dim cTmpLangString as string   ' Temporary language string for comparision
    dim cAdditionalLanguage as string
    
    dim iLangCount as integer
    
    printlog( "Preparing testenvironment" )
    printlog( "Workfile is: " & cWorkPath )

    brc = hCreateDocument()
    if ( not brc ) then
        warnlog( "Failed to create a new document, aborting" )
        goto endsub
    endif

    brc = hFileSaveAsWithFilterKill( cWorkPath , "writer8" )

    brc = hInitFormControls( MODULE_NAME )
    if ( not brc ) then
        warnlog( "Ups, bad again..." )
    endif
    
    printlog( "Dialog Editor is open, ToolsCollectionBar is visible. Good." )

    printlog( "" )
    printlog( "Starting test (check control states, add default language)" )

    kontext "TranslationBar"
    if ( TranslationBar.exists( 1 ) ) then
	warnlog( "The TranslationBar should not be exist/visible by default" )
    else
        printlog( "The TranslationBar is hidden. Good." )
    endif

    Kontext "ToolsCollectionBar"
    ManageLanguage.click()

    kontext "ManageUILanguages"
    if ( not ManageUILanguages.exists( 2 ) ) then
        warnlog( "ManageUILanguages Dialog is not open, aborting test" )
        goto TestExit
    endif

    printlog( "ManageUILanguages Dialog is open" )

    iLangCount = PresentLanguages.getItemCount()
    if ( iLangCount <> 1 ) then
        warnlog( "There should only be one single entry in the languages listbox" )
        printlog( "Currently " & iLangCount & " entries are listed" )
    endif

    if ( not add.isEnabled() ) then
        warnlog( "Add button should be enabled" )
    endif

    if ( delete.isEnabled() ) then  
        warnlog( "Delete button should be disabled" )
    endif


    if ( Default.isEnabled() ) then  
        warnlog( "Default button should be disabled" )
    endif

    
    printlog( "Select the first/only entry to see whether the button states change" )
    PresentLanguages.select( 1 )
    printlog( "Entry is: " & PresentLanguages.getSelText() )
    
    if ( not add.isEnabled() ) then
        warnlog( "Add button should be enabled" )
    endif

    if ( Delete.isEnabled() ) then  
        warnlog( "Delete button should be disabled" )
    endif

    if ( Default.isEnabled() ) then  
        warnlog( "Default button should be disabled" )
    endif

    printlog( "Add default language" )
    kontext "ManageUILanguage"
    Add.click()

    kontext "SetDefaultLanguage"
    if ( not SetDefaultLanguage.exists( 2 ) ) then
        warnlog( "Default language selection dialog did not open" )
        kontext "ManageUILanguages"
        ManageUILanguages.close()
        goto TestExit
    endif

    kontext "SetDefaultLanguage"
    cDeflangSelect = DefaultLanguageListbox.getSelText()
    iLangCount = DefaultLanguageListbox.getItemCount()
    printlog( "Default language is: " & cDeflangSelect )
    printlog( "Total languages listed: " & iLangCount )
    SetDefaultLanguage.ok()


    kontext "ManageUILanguages"
    iLangCount = PresentLanguages.getItemCount()
    if ( iLangCount <> 1 ) then
        warnlog( "There should only be one single entry in the languages listbox" )
        printlog( "Currently " & iLangCount & " entries are listed" )
    endif
    
    PresentLanguages.select( 1 )
    cDefLangDialog = PresentLanguages.getSelText()
    if ( instr( cDefLangSelect, cDefLangDialog ) <> 0 ) then
        warnlog( "The default language was not transported to the languages list" )
    else
        printlog( "Ok, the string was copied correctly" )
    endif
    printlog( "Select.: " & cDefLangSelect )
    printlog( "Dialog.: " & cDefLangDialog )
    

    if ( not add.isEnabled() ) then
        warnlog( "Add button should be enabled" )
    endif

    if ( not Delete.isEnabled() ) then  
        warnlog( "Delete button should be enabled" )
        Delete.click()
        kontext "active"
        if ( active.exists( 1 ) ) then
        	printlog( "Removal warning is displayed. Good." )
        	active.cancel()
        else
        	warnlog( "Removal warning not displayed" )
        endif
    endif

    if ( Default.isEnabled() ) then  
        warnlog( "Default button should be disabled" )
    endif
    
    kontext "ManageUILanguages"
    ManageUILanguages.close()
    
    
    
    kontext "TranslationBar"
    printlog( "Testing the TranslationBar ..." )
    if ( TranslationBar.exists() ) then
        printlog( "TranslationBar exists." )
        
        if ( TranslationBar.isVisible() ) then
            printlog( "TranslationBar is visible." )
            
            if ( currentLanguage.isEnabled() ) then
                printlog( "Languages list is enabled." )
                            
                if ( CurrentLanguage.getItemCount() = 1 ) then
                    printlog( "Exactly one item in listed in the listbox, good." )
                    
                    ' Note that we need to test for a substring as the [default] marker
                    ' is not copied from the Select Default languages dialog
                    cDefLangToolbar = CurrentLanguage.getSelText()
                    if ( instr( cDefLangToolbar, cDefLangSelect ) <> 0 ) then
                        printlog( "The correct entry is in the languages list. Good." )
                    else
                        warnlog( "The default language should be listed and selected." )
                    endif
                    printlog( "Select.: " & cDefLangSelect )
                    printlog( "Toolbar: " & cDefLangToolbar )
                    
                    if ( cDefLangToolbar <> cDefLangDialog ) then
                        warnlog( "The default language did not copy to the toolbar" )
                    else
                        printlog( "The default language was copied ok." )
                    endif
                    printlog( "Dialog.: " & cDefLangDialog )
                    printlog( "Toolbar: " & cDefLangToolbar )
                    
                else
                    warnlog( "There are too many entries in the languages list." )
                endif
            else
                warnlog( "The languages listbox is disabled." )
            endif
        else
            warnlog( "The Translation Bar is not visible." )
        endif
    else
        warnlog( "The Translation Bar does not exist." )
    endif
    
    
    printlog( "Click ManageLanguages-button on the TranslationBar" )
    kontext "TranslationBar" 
    wait( 100 )
    ManageLanguage.click()
    
    kontext "ManageUILanguages"
    if ( not ManageUILanguages.exists( 2 ) ) then
    	warnlog( "Failed to open Manage Languages dialog" )
    endif
    
    iLangCount = PresentLanguages.getItemCount()
    if ( iLangCount <> 1 ) then
        warnlog( "There should only be one single entry in the languages listbox" )
        printlog( "Currently " & iLangCount & " entries are listed" )
    endif    
    
    PresentLanguages.select( 1 )
    cTmpLangString = Presentlanguages.getSeltext()
    if ( cTmpLangString <> cDefLangDialog ) then
        warnlog( "The dialog forgot the default language" )
    else
        printlog( "The dialog remembers the default language. Good." )
    endif        
    printlog( "Expected: " & cDefLangDialog )
    printlog( "Found...: " & cTmpLangString )
    
    kontext "ManageUILanguages"
    Add.click()
    
    kontext "AddUserInterface"
    if ( not AddUserInterface.exists( 1 ) ) then    
        warnlog( "Cannot add another language, the dialog is not open" )
        kontext "ManageUILanguages"
        ManageUILanguages.close()
        goto TestExit
    endif
        
    AddNewControl.select( 1 )
    cAdditionalLanguage = AddNewControl.getSelText()
    printlog( "Adding language: " & cAdditionalLanguage )
    AddNewControl.check()
    
    AddUserInterface.ok()
    
    kontext "ManageUILanguages"
    if ( not ManageUILanguages.exists( 1 ) ) then
        warnlog( "Could not return to Manage UI Languages dialog" )
        goto TestExit
    endif
    
    kontext "ManageUILanguages"
    iLangCount = PresentLanguages.getItemCount()
    if ( iLangCount <> 2 ) then
        warnlog( "There should be exactly two languages listed in the listbox" )
        printlog( "Currently " & iLangCount & " entries are listed" )
        goto TestExit
    else
        printlog( "Two languages listed in Manage UI Languages dialog. Good." )
    endif    
    
    printlog( "Verify language in Manage UI Language dialog" )
    PresentLanguages.select( 2 )
    cTmpLangString = Presentlanguages.getSelText()
    if ( cTmpLangString = cAdditionalLanguage ) then    
        printlog( "Found correct language at pos 2. Good" )
    else
        warnlog( "The language is not listed at the expected position" )
    endif
    printlog( "Expected: " & cAdditionalLanguage )
    printlog( "Found...: " & cTmpLangString )   
    
    printlog( "Select second language and make it default" )
    PresentLanguages.select( 2 )
    Default.click() 
    cAdditionalLanguage = PresentLanguages.getSelText()

    printlog( "Close the Manage UI Languages dialog" )
    kontext "ManageUILanguages"
    ManageUILanguages.close()     
    
    printlog( "Verify that the new default is on pos 2 in the TranslationBar list" )
    kontext "TranslationBar"
    CurrentLanguage.select( 2 )
    cDefLangToolbar = CurrentLanguage.getSelText()
    if ( cDefLangToolbar = cAdditionalLanguage ) then    
        printlog( "Found correct language at pos 2. Good." )
    else
        warnlog( "The language is not listed at the expected position" )
    endif
    printlog( "Expected: " & cAdditionalLanguage )
    printlog( "Found...: " & cDefLangToolbar )      
    
    printlog( "Verify that the list on the Manage UI Language is still unsorted" )
    printlog( "The dafault language should be at pos 2" )
    ManageLanguage.click()
    kontext "ManageUILanguages"
    PresentLanguages.select( 2 )
    cTmpLangString = Presentlanguages.getSelText()
    if ( cTmpLangString = cAdditionalLanguage ) then    
        printlog( "Found correct language at pos 2. Good" )
    else
        warnlog( "The language is not listed at the expected position" )
    endif
    printlog( "Expected: " & cAdditionalLanguage )
    printlog( "Found...: " & cTmpLangString )
    
    printlog( "Delte the current default language" )
    Delete.click()
    
    printlog( "There should be a deletion warning" )
    kontext "Active"
    if ( Active.exists( 2 ) ) then
        cMsg = active.getText()
        cMsg = hRemoveLineBreaks( cMsg )
        printlog( "Message: " & cMsg )
        Active.ok()
    else
        warnlog( "Deletion Warning missing" )
    endif
    
    printlog( "Verify deletion, the remaining language must be default now" )
    kontext "ManageUILanguages"
    iLangCount = PresentLanguages.getItemCount()
    if ( iLangCount <> 1 ) then
        warnlog( "Only the default language should be listed" )
        printlog( "Currently " & iLangCount & " entries are listed" )
    endif         
    
    PresentLanguages.select( 1 )
    cTmpLangString = Presentlanguages.getSeltext()
    if ( cTmpLangString <> cDefLangDialog ) then
        warnlog( "The dialog forgot the default language" )
    else
        printlog( "The dialog sets the default language. Good." )
    endif        
    printlog( "Expected: " & cDefLangDialog )
    printlog( "Found...: " & cTmpLangString )        
    
    printlog( "verify button states" )
    
    if ( not add.isEnabled() ) then
        warnlog( "Add button should be enabled" )
    endif

    if ( not Delete.isEnabled() ) then  
        warnlog( "Delete button should be enabled" )
    endif

    if ( Default.isEnabled() ) then  
        warnlog( "Default button should be disabled" )
    endif
    
    TestExit:
    printlog( "" )
    printlog( "Test exit, cleanup" )

    kontext "ManageUILanguages"
    if ( ManageUILanguages.exists() ) then
        ManageUILanguages.close()     
    endif
    
    hCloseBasicIde()
    hCloseDocument()
    hDeleteFile( cWorkPath )

endcase


