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
    
    hCreateDocument()

    printlog( "Test init: Setting up environment" )
    brc = hInitFormControls( DIALOG_NAME )
    if ( not brc ) then
        warnlog( "Failed to initialize BASIC IDE/Dialog editor, aborting" )
        goto endsub
    endif
    
    hDrawControlOnDialog( COMMAND_BUTTON )
    
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
    
    kontext "ToolsCollectionBar"
    ManageLanguage.click()
    
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
    
    kontext "ManageUILanguages"
    for iCurrentLanguage = 0 to LANGUAGES_TO_COPY
        PresentLanguages.select( iCurrentLanguage + 1 ) 
        cLanguageList( iCurrentLanguage ) = PresentLanguages.getSelText()
        printlog( iCurrentLanguage & ": " & cLanguageList( iCurrentLanguage ) )
    next iCurrentLanguage
    
    kontext "ManageUILanguages"
    ManageUILanguages.close()
    
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
    
    printlog( "Copy control" )
    EditCopy
        
    printlog( "New dialog" )
    Call hNewDialog()
    
    printlog( "Paste control" )
    EditPaste
    
    hOpenPropertyBrowser()
    kontext "TabGeneralControl"
    printlog( "Set coordinates to be identical with those from the source control" )
    PosX.typeKeys  (  )
    PosX.typeKeys  ( "<HOME><SHIFT END>" & iOriginalCoordinates( 3 ) & "<RETURN>" )
    PosY.typeKeys  ( "<HOME><SHIFT END>" & iOriginalCoordinates( 4 ) & "<RETURN>" )
    Height.typeKeys( "<HOME><SHIFT END>" & iOriginalCoordinates( 1 ) & "<RETURN>" )
    Width.typeKeys ( "<HOME><SHIFT END>" & iOriginalCoordinates( 2 ) & "<RETURN>" )
    hClosePropertyBrowser


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
    
    
    kontext "BasicIde"
    hDestroyDocument()
    hDestroyDocument()
    
endcase

