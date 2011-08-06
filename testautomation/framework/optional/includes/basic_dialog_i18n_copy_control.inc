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
'*  short description : Copy a control with i18n data
'*
'\******************************************************************************

testcase tBasicDialogI18nCopyControl


    const COMMAND_BUTTON = 1            ' id of the command button (formcontrols.inc)
    const DIALOG_NAME = "DialogExport"  ' Name of the first module
    const LANGUAGES_TO_COPY = 5         ' Only copy n languages
    const TARGET_DIALOG = "Target"      ' The name of the dialog the control is pasted to
    const DEFAULT_TIMEOUT = 2
    
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
    
    hCreateDocument()

    printlog( "Test init: Setting up environment" )
    if ( hInitFormControls( DIALOG_NAME ) ) then

        printlog( "Draw a commandbutton on the dialog" )
        hDrawControlOnDialog( COMMAND_BUTTON )

        printlog( "Open the property browser" )
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
        printlog( "Click <Manage Languages>" )
        ManageLanguage.click()

        kontext "ManageUILanguages"
        if ( ManageUILanguages.exists( DEFAULT_TIMEOUT ) ) then

            printlog( "Click <Add>" )
            hClickButton( Add )

            kontext "SetDefaultLanguage"
            if ( SetDefaultLanguage.exists( DEFAULT_TIMEOUT ) ) then
                hCloseDialog( SetDefaultLanguage, "ok" )
            else
                warnlog( "Set Default Language dialog is missing" )
            endif
        else
            warnlog( "Unable to open Manage UI Languages dialog" )
        endif

        kontext "ManageUILanguages"
        if ( ManageUILanguages.exists( DEFAULT_TIMEOUT ) ) then

            printlog( "Click <Add>" )
            hClickButton( Add )

            kontext "AddUserInterface"
            if ( AddUserInterface.exists( DEFAULT_TIMEOUT ) ) then
                for iCurrentLanguage = 1 to LANGUAGES_TO_COPY
                    AddNewControl.select( iCurrentLanguage )
                    AddNewControl.check()
                next iCurrentLanguage
                hCloseDialog( AddUserInterface, "ok" )
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
        hCloseDialog( ManageUILanguages, "close" )

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
        hUseAsyncSlot( "EditCopy" )

        printlog( "New dialog" )
        hNewDialog()

        printlog( "Paste control" )
        hUseAsyncSlot( "EditPaste" )

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

        hCloseBasicIDE()
    else
        warnlog( "Failed to initialize dialog" )
    endif
    hDestroyDocument()
    
endcase

