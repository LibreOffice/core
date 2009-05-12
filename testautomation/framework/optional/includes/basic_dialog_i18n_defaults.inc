'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_dialog_i18n_defaults.inc,v $
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
'*  short description : Test correct handling of the default language
'*
'\******************************************************************************

testcase tBasicDialogI18nDefaults

    '///<h1>Test correct handling of the default language</h1>
    '///<i>This test verifies that - if the user adds localization to the dialogs - 
    '///+ these are handled in a sensible way. This means that a) the first 
    '///+ language automatically becomes default, b) the languages really make it
    '///+ from the language selection to the Manage UI Languages dialog and the
    '///+ the listbox of the translation toolbar in the BASIC IDE. Switching 
    '///+ languages is tested, as well as the states of the three action buttons on
    '///+ the Manage UI Languages dialog.</i><br>

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
    
    '///<ul>
    printlog( "Preparing testenvironment" )
    printlog( "Workfile is: " & cWorkPath )

    '///+<li>Create a new document</li>
    brc = hCreateDocument()
    if ( not brc ) then
        warnlog( "Failed to create a new document, aborting" )
        goto endsub
    endif

    '///+<li>Save the file, overwrite existing if found</li>
    brc = hFileSaveAsWithFilterKill( cWorkPath , "writer8" )

    '///+<li>Open the basic organizer</li>
    '///+<li>Create a new module for the current document</li>
    '///+<li>Click to edit the module</li>
    '///+<li>Create a dialog, switch to it and ensure that the ToolsCollectionBar is open</li>
    brc = hInitFormControls( MODULE_NAME )
    if ( not brc ) then
        warnlog( "Ups, bad again..." )
    endif
    
    printlog( "Dialog Editor is open, ToolsCollectionBar is visible. Good." )

    printlog( "" )
    printlog( "Starting test (check control states, add default language)" )

    '///+<li>Verify that the Translation Bar is hidden by default"</li>
    kontext "TranslationBar"
    if ( TranslationBar.exists( 1 ) ) then
	warnlog( "The TranslationBar should not be exist/visible by default" )
    else
        printlog( "The TranslationBar is hidden. Good." )
    endif

    Kontext "ToolsCollectionBar"
    ManageLanguage.click()

    '///+<li>On the now open Language dialog test the default states of the controls</li>
    kontext "ManageUILanguages"
    if ( not ManageUILanguages.exists( 2 ) ) then
        warnlog( "ManageUILanguages Dialog is not open, aborting test" )
        goto TestExit
    endif

    printlog( "ManageUILanguages Dialog is open" )

    '///<ul>
    '///+<li>There should be no languages listed yet but a hint on how to continue</li>
    iLangCount = PresentLanguages.getItemCount()
    if ( iLangCount <> 1 ) then
        warnlog( "There should only be one single entry in the languages listbox" )
        printlog( "Currently " & iLangCount & " entries are listed" )
    endif

    '///+<li>&quot;Add...&quot; should be enabled</li>
    if ( not add.isEnabled() ) then
        warnlog( "Add button should be enabled" )
    endif

    '///+<li>&quot;Delete&quot; should be disabled</li>
    if ( delete.isEnabled() ) then  
        warnlog( "Delete button should be disabled" )
    endif


    '///+<li>&quot;Default&quot; should be disabled</li>
    if ( Default.isEnabled() ) then  
        warnlog( "Default button should be disabled" )
    endif

    '///</ul>
    
    '///+<li>Select the first and only entry in the list, it should not change the button states</li>
    printlog( "Select the first/only entry to see whether the button states change" )
    PresentLanguages.select( 1 )
    printlog( "Entry is: " & PresentLanguages.getSelText() )
    '///<ul>
    
    '///+<li>&quot;Add...&quot; should be enabled</li>
    if ( not add.isEnabled() ) then
        warnlog( "Add button should be enabled" )
    endif

    '///+<li>&quot;Delete&quot; should be disabled</li>
    if ( Delete.isEnabled() ) then  
        warnlog( "Delete button should be disabled" )
    endif

    '///+<li>&quot;Default&quot; should be disabled</li>
    if ( Default.isEnabled() ) then  
        warnlog( "Default button should be disabled" )
    endif
    '///</ul>

    '///+<li>Click &quot;Add...&quot; to add a default language</li>
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


    '///+<li>Test the language string and the states of the buttons on the dialog</li>
    kontext "ManageUILanguages"
    '///<ul>
    '///+<li>There should be exactly one language listed</li>
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
    

    '///+<li>&quot;Add...&quot; should be enabled</li>
    if ( not add.isEnabled() ) then
        warnlog( "Add button should be enabled" )
    endif

    '///+<li>&quot;Delete&quot; should be enabled</li>
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

    '///+<li>&quot;Default&quot; should be disabled</li>
    if ( Default.isEnabled() ) then  
        warnlog( "Default button should be disabled" )
    endif
    '///</ul>
    
    '///+<li>Close the Manage UI Languages Dialog</li>
    kontext "ManageUILanguages"
    ManageUILanguages.close()
    
    
    
    '///+<li>Test the Translation Bar</li>
    '///<ul>
    kontext "TranslationBar"
    printlog( "Testing the TranslationBar ..." )
    if ( TranslationBar.exists() ) then
        printlog( "TranslationBar exists." )
        
        '///+<li>Verify that the translation bar is visible</li>
        if ( TranslationBar.isVisible() ) then
            printlog( "TranslationBar is visible." )
            
            '///+<li>Verify that the languages listbox is enabled</li>
            if ( currentLanguage.isEnabled() ) then
                printlog( "Languages list is enabled." )
                            
                '///+<li>Verify that the default language is the only item in the listbox</li>
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
    '///</ul> 
    
    
    '///+<li>Reopen ManageUILanguages dialog - this time by clicking the icon on the TranslationBar</li>
    printlog( "Click ManageLanguages-button on the TranslationBar" )
    kontext "TranslationBar" 
    wait( 100 )
    ManageLanguage.click()
    
    '///+<li>The Manage UI Languages dialog should pop open</li>
    kontext "ManageUILanguages"
    if ( not ManageUILanguages.exists( 2 ) ) then
    	warnlog( "Failed to open Manage Languages dialog" )
    endif
    
    '///+<li>Verify that exactly one language is listed</li>
    iLangCount = PresentLanguages.getItemCount()
    if ( iLangCount <> 1 ) then
        warnlog( "There should only be one single entry in the languages listbox" )
        printlog( "Currently " & iLangCount & " entries are listed" )
    endif    
    
    '///+<li>Verify that it is marked as the default language</li>
    PresentLanguages.select( 1 )
    cTmpLangString = Presentlanguages.getSeltext()
    if ( cTmpLangString <> cDefLangDialog ) then
        warnlog( "The dialog forgot the default language" )
    else
        printlog( "The dialog remembers the default language. Good." )
    endif        
    printlog( "Expected: " & cDefLangDialog )
    printlog( "Found...: " & cTmpLangString )
    
    '///+<li>Add another language, click on &quot;Add...&quot;</li>
    kontext "ManageUILanguages"
    Add.click()
    
    kontext "AddUserInterface"
    if ( not AddUserInterface.exists( 1 ) ) then    
        warnlog( "Cannot add another language, the dialog is not open" )
        kontext "ManageUILanguages"
        ManageUILanguages.close()
        goto TestExit
    endif
        
    '///+<li>Select the first language from the list (Check the Checkbox)</li>
    AddNewControl.select( 1 )
    cAdditionalLanguage = AddNewControl.getSelText()
    printlog( "Adding language: " & cAdditionalLanguage )
    AddNewControl.check()
    
    '///+<li>Click OK to add the language</li>
    AddUserInterface.ok()
    
    kontext "ManageUILanguages"
    if ( not ManageUILanguages.exists( 1 ) ) then
        warnlog( "Could not return to Manage UI Languages dialog" )
        goto TestExit
    endif
    
    '///+<li>Verify that the language shows up in the Manage UI Languages dialog" )
    kontext "ManageUILanguages"
    iLangCount = PresentLanguages.getItemCount()
    if ( iLangCount <> 2 ) then
        warnlog( "There should be exactly two languages listed in the listbox" )
        printlog( "Currently " & iLangCount & " entries are listed" )
        goto TestExit
    else
        printlog( "Two languages listed in Manage UI Languages dialog. Good." )
    endif    
    
    '///+<li>Verify that additional language is at position 2 (default is first)</li>
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
    
    '///+<li>Switch the default to the newly added language</li>
    printlog( "Select second language and make it default" )
    PresentLanguages.select( 2 )
    Default.click() 
    cAdditionalLanguage = PresentLanguages.getSelText()

   '///+<li>Close the Manage UI Languages dialog</li>
    printlog( "Close the Manage UI Languages dialog" )
    kontext "ManageUILanguages"
    ManageUILanguages.close()     
    
    '///+<li>Verify that the change in default makes it to the translation bar</li>
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
    
    '///+<li>The new default should be at pos 2 of the Manage UI Language dialog</li>
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
    
    '///+<li>Delete the default language</li>
    printlog( "Delte the current default language" )
    Delete.click()
    
    '///+<li>Handle the warning</li>
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
    
    '///+<li>Verify that there is only one item left</li>
    printlog( "Verify deletion, the remaining language must be default now" )
    kontext "ManageUILanguages"
    iLangCount = PresentLanguages.getItemCount()
    if ( iLangCount <> 1 ) then
        warnlog( "Only the default language should be listed" )
        printlog( "Currently " & iLangCount & " entries are listed" )
    endif         
    
    '///+<li>Verify that this is the first language and that it is default</li>    
    PresentLanguages.select( 1 )
    cTmpLangString = Presentlanguages.getSeltext()
    if ( cTmpLangString <> cDefLangDialog ) then
        warnlog( "The dialog forgot the default language" )
    else
        printlog( "The dialog sets the default language. Good." )
    endif        
    printlog( "Expected: " & cDefLangDialog )
    printlog( "Found...: " & cTmpLangString )        
    
    '///+<li>Select the first and only entry in the list, it should not change the button states</li>
    printlog( "verify button states" )
    '///<ul>
    
    '///+<li>&quot;Add...&quot; should be enabled</li>
    if ( not add.isEnabled() ) then
        warnlog( "Add button should be enabled" )
    endif

    '///+<li>&quot;Delete&quot; should be disabled</li>
    if ( not Delete.isEnabled() ) then  
        warnlog( "Delete button should be enabled" )
    endif

    '///+<li>&quot;Default&quot; should be disabled</li>
    if ( Default.isEnabled() ) then  
        warnlog( "Default button should be disabled" )
    endif
    '///</ul>    
    
    '///+<li>Test exit, cleanup</li>
    '///<ul>
    TestExit:
    printlog( "" )
    printlog( "Test exit, cleanup" )

    '///+<li>Close Manage UI Languages dialog" )
    kontext "ManageUILanguages"
    if ( ManageUILanguages.exists() ) then
        ManageUILanguages.close()     
    endif
    
    '///+<li>Close the BASIC IDE</li>
    hCloseBasicIde()
    '///+<li>Close the document</li>
    hCloseDocument()
    '///+<li>Delete the workfile</li>
    hDeleteFile( cWorkPath )
    '///</ul>
    '///</ul>

endcase


