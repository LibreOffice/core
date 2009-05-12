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

    '///<h3>Update test for Basic IDE / Dialog i18n-feature</h3>
    '///<i>Specification: Translation Inside Dialog IDE</i><br>
    
    dim brc as boolean
    
    '///<ul>
    '///+<li>Create a new writer document</li>
    '///+<li>Open the BASIC organiser</li>
    '///+<li>Select the document and create a new BASIC module</li>
    '///+<li>Edit the module</li>
    '///+<li>Create a new dialog</li>
    '///+<li>Open the ToolsCollectionBar</li>
    
    ' Just in case, this thing is annoying
    hCloseNavigator()
    
    '///+<li>Open a new writer document</li>
    hCreateDocument()'
    
    brc = hInitFormControls( "tDialogI18n" )
    if ( not brc ) then
        warnlog( "Failed to open Basic IDE / Dialogs / ToolsCollectionBar" )
        goto endsub
    endif
    
    '///+<li>Click the &quot;Add language&quot; button on the ToolsCollectionBar</li>
    printlog( "Click Manage Languages on ToolsCollectionBar")
    kontext "ToolsCollectionBar" 
    ManageLanguage.click()
        
    kontext "ManageUILanguages"
    call dialogtest( ManageUILanguages )
    
    '///+<li>Click the &quot;Add&quot; button, this will add a default language</li>
    printlog( "Add default language, choose preselection")
    kontext "ManageUILanguages"
    Add.click()
    
    kontext "SetDefaultLanguage"
    call dialogtest( SetDefaultLanguage )
    
    '///+<li>Select a language at random</li>
    kontext "SetDefaultLanguage"
    DefaultLanguageListbox.select( 3 )
    
    '///+<li>Leave the dialog with &quot;ok&quot;</li>
    kontext "SetDefaultLanguage"
    SetDefaultLanguage.ok()
    
    '///+<li>Click &quot;Add&quot; again</li>
    printlog( "Back on Manage UI Languages Dialog")
    kontext "ManageUILanguages"
    Add.click()
    
    kontext "AddUserInterface"
    call dialogtest( AddUserInterface )

    '///+<li>Select and check the first language in the list</li>
    printlog( "Add just another language at random")    
    kontext "AddUserInterface"
    AddNewControl.typeKeys( "<HOME>" )     ' select first item
    AddNewControl.typeKeys( "<SPACE>" )    ' check it

    '///+<li>Leave the dialog with &quot;ok&quot;</li>
    kontext "AddUserInterface"
    AddUserInterface.ok()
    
    '///+<li>Select the first language in the languages list</li>
    kontext "ManageUILanguages"
    PresentLanguages.Select (1)
    
    '///+<li>Click &quot;Delete&quot;</li>
    printlog( "Back on Manage UI Languages Dialog, delete selected language")
    kontext "ManageUILanguages"
    Delete.click()
    
    printlog( "Confirm delete")
    kontext "active"
    call dialogtest( active )
    
    '///+<li>Confirm deletion</li>
    kontext "active"
    active.ok()
    
    '///+<li>Close &quot;Manage User Interface languages&quot; dialog</li>
    printlog( "Leave Manage UI Languages Dialog")
    kontext "ManageUILanguages"
    ManageUILanguages.ok()
    
    '///+<li>Verify that the new translation toolbar is visible</li>
    printlog( "Verify that the translationbar is visible")
    kontext "TranslationBar"
    if ( not TranslationBar.exists() ) then
        warnlog( "The IDE Translation Toolbar is not visible" )
    endif
    
    '///+<li>Cleanup: Close ToolsCollectionBar</li>
    kontext "ToolsCollectionBar"
    if ( not ToolsCollectionBar.exists() ) then
        warnlog( "The ToolsCollectionBar is not visible" )
    endif
    ToolsCollectionBar.Close
    
    '///+<li>Cleanup: Close the basic IDE</li>
    hClosebasicIDE()
    
    '///+<li>Cleanup: Close the document</li>
    hDestroyDocument()

    '///</ul>

endcase

