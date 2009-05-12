'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_dialog_i18n_copy_control.inc,v $
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
'*  owner : Joerg.Skottke@Sun.Com
'*
'*  short description : Copy a control with i18n data
'*
'\******************************************************************************

testcase tBasicDialogI18nCopyControl

    '///<h1>Copy a formcontrol with i18n data within BASIC IDE</h1>
    '///<u><pre>Synopsis</pre></u>Apply localization to a form control and copy 
    '///+ it within the BASIC IDE. Verify that the localized data is preserved<br>
    '///<u><pre>Specification document</pre></u><a href="http://specs.openoffice.org/appwide/dialog_ide/GUI_Translation_in_Dialog_IDE.odt">
    '///+ GUI translation in Dialog IDE</a><br>
    '///<u><pre>Files used</pre></u>None<br>
    '///<u><pre>Test case specification</pre></u>

    const COMMAND_BUTTON = 1            ' id of the command button (formcontrols.inc)
    const DIALOG_NAME = "DialogExport"  ' Name of the first module
    const LANGUAGES_TO_COPY = 5         ' Only copy n languages
    const TARGET_DIALOG = "Target"      ' The name of the dialog the control is pasted to
    
    dim cLanguageList( LANGUAGES_TO_COPY ) as string ' Note: Index 0 is used
    dim iCurrentLanguage as integer
    
    ' These are the strings used to simulate localized text
    dim cIDStrings( 5 ) as string
        cIDStrings( 0 ) = "language_a"
        cIDStrings( 1 ) = "language_b"
        cIDStrings( 2 ) = "language_c"
        cIDStrings( 3 ) = "language_d"
        cIDStrings( 4 ) = "language_e"
        cIDStrings( 5 ) = "language_f"
        
    dim cTempString as string
        
    ' These are the IDE internal coordinates of the control, used to restore
    ' identical coordinates in the source and target dialogs
    dim iOriginalCoordinates( 4 ) as integer

    ' Some multi purpose returncode                
    dim brc as boolean
    
    '///<ul>
    '///+<li>Create a new document</li>
    hCreateDocument()

    '///+<li>Open the Macro Organizer</li>
    '///+<li>Create a new module for the document</li>
    '///+<li>Open the BASIC IDE</li>
    '///+<li>Create a new Dialog via the tabbar at the bottom of the IDE</li>
    printlog( "Test init: Setting up environment" )
    brc = hInitFormControls( DIALOG_NAME )
    if ( not brc ) then
        warnlog( "Failed to initialize BASIC IDE/Dialog editor, aborting" )
        goto endsub
    endif
    
    '///+<li>Draw a Command Button</li>
    hDrawControlOnDialog( COMMAND_BUTTON )
    
    '///+<li>Retrieve the current BASIC IDE internal coordinates of the control for
    '///+ later usage</li>
    hOpenPropertyBrowser()
    kontext "TabGeneralControl"
    printlog( "Get internal coordinates for the control" )
    iOriginalCoordinates( 1 ) = height.getText()
    iOriginalCoordinates( 2 ) = width.getText()
    iOriginalCoordinates( 3 ) = PosX.getText()
    iOriginalCoordinates( 4 ) = PosY.getText()
    printlog( "Height: " & iOriginalCoordinates( 1 ) )
    printlog( "Width.: " & iOriginalCoordinates( 2 ) )
    printlog( "Pos X.: " & iOriginalCoordinates( 3 ) )
    printlog( "Pos Y.: " & iOriginalCoordinates( 4 ) )
    hClosePropertyBrowser()
    
    qaerrorlog( "#i80456# properties button not enabled when a formcontrol is inserted via keyboard" )
    
    '///+<li>Click Manage Language button on ToolsCollection Bar</li>
    kontext "ToolsCollectionBar"
    ManageLanguage.click()
    
    '///+<li>Add a default language</li>
    kontext "ManageUILanguages"
    if ( ManageUILanguages.exists( 2 ) ) then
    
        add.click()
        
        kontext "SetDefaultLanguage" 
        if ( SetDefaultLanguage.exists( 1 ) ) then
            SetDefaultLanguage.ok()
        else
            warnlog( "Set Default Language dialog is missing" )
        endif
    else
        warnlog( "Unable to open Manage UI Languages dialog" )
    endif
    
    '///+<li>Add a number of additional languages</li>
    kontext "ManageUILanguages"
    if ( ManageUILanguages.exists( 1 ) ) then
    
        add.click()
        
        kontext "AddUserInterface" 
        if ( AddUserInterface.exists( 1 ) ) then
            for iCurrentLanguage = 1 to LANGUAGES_TO_COPY
                AddNewControl.select( iCurrentLanguage )
                AddNewControl.check()
            next iCurrentLanguage
            AddUserInterface.ok()
        else
            warnlog( "Add User Interface Language dialog is missing" )
        endif
    else
        warnlog( "Unable to open Manage UI Languages dialog" )
    endif
    
    '///+<li>Store the installed languages for further usage</li>
    kontext "ManageUILanguages"
    for iCurrentLanguage = 0 to LANGUAGES_TO_COPY
        PresentLanguages.select( iCurrentLanguage + 1 ) 
        cLanguageList( iCurrentLanguage ) = PresentLanguages.getSelText()
        printlog( iCurrentLanguage & ": " & cLanguageList( iCurrentLanguage ) )
    next iCurrentLanguage
    
    '///+<li>Close the Manage UI Languages dialog</li>
    kontext "ManageUILanguages"
    ManageUILanguages.close()
    
    '///+<li>Change the property &quot;NameText&quot; for all languages</li>
    printlog( "Localizing control" )
    for iCurrentLanguage = 0 to LANGUAGES_TO_COPY
        kontext "TranslationBar"
        CurrentLanguage.select( iCurrentLanguage + 1 )
        kontext "BasicIde"
        hSelectControl( COMMAND_BUTTON )
        hOpenPropertyBrowser()
        printlog( "Setting string: " & cIDStrings( iCurrentLanguage ) )
        kontext "TabGeneralControl"
        NameText.typeKeys( cIDStrings( iCurrentLanguage ) & "<RETURN>" )
        hClosePropertyBrowser()
    next iCurrentLanguage
    
    '///+<li>Execute edit/copy on the currently selected command button</li>
    printlog( "Copy control" )
    EditCopy
        
    '///+<li>Create a second dialog</li>
    printlog( "New dialog" )
    Call hNewDialog()
    
    '///+<li>Paste the content of the clipboard (the control) to the dialog window</li>
    printlog( "Paste control" )
    EditPaste
    
    '///+<li>Apply the coordinates from the source control (the control is placed
    '///+ at random within the dialog pane, now we move it to the same coordinates
    '///+ as its source control). We wouldn't be able to handle it otherwise</li>
    hOpenPropertyBrowser()
    kontext "TabGeneralControl"
    printlog( "Set coordinates to be identical with those from the source control" )
    PosX.typeKeys  (  )
    PosX.typeKeys  ( "<HOME><SHIFT END>" & iOriginalCoordinates( 3 ) & "<RETURN>" )
    PosY.typeKeys  ( "<HOME><SHIFT END>" & iOriginalCoordinates( 4 ) & "<RETURN>" )
    Height.typeKeys( "<HOME><SHIFT END>" & iOriginalCoordinates( 1 ) & "<RETURN>" )
    Width.typeKeys ( "<HOME><SHIFT END>" & iOriginalCoordinates( 2 ) & "<RETURN>" )
    hClosePropertyBrowser


    '///+<li>Retrieve the current BASIC IDE internal coordinates of the control for
    '///+ later usage</li>
    hOpenPropertyBrowser()
    kontext "TabGeneralControl"
    printlog( "Get internal coordinates for the control" )
    iOriginalCoordinates( 1 ) = height.getText()
    iOriginalCoordinates( 2 ) = width.getText()
    iOriginalCoordinates( 3 ) = PosX.getText()
    iOriginalCoordinates( 4 ) = PosY.getText()
    printlog( "Height: " & iOriginalCoordinates( 1 ) )
    printlog( "Width.: " & iOriginalCoordinates( 2 ) )
    printlog( "Pos X.: " & iOriginalCoordinates( 3 ) )
    printlog( "Pos Y.: " & iOriginalCoordinates( 4 ) )
    hClosePropertyBrowser()

    
    '///+<li>Get the list of transferred languages from the translation bar</li>
    kontext "TranslationBar"
    for iCurrentLanguage = 0 to LANGUAGES_TO_COPY
        CurrentLanguage.select( iCurrentLanguage + 1 )
        cTempString = CurrentLanguage.getSelText()
        if ( cTempString = cLanguageList( iCurrentLanguage ) ) then
            printlog( "Language <" & cTempString & "> was successfully copied" )
        else
            warnlog( "Either a language was lost or the sort order changed" )
            printlog( "Expected: " & cLanguageList( iCurrentLanguage ) )
            printlog( "Found...: " & cTempString )
        endif
    next iCurrentLanguage
    
    '///+<li>
    '///</ul>
    
    kontext "BasicIde"
    hDestroyDocument()
    hDestroyDocument()
    
endcase

