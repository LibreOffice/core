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
'/************************************************************************
'*
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : 
'*
'\******************************************************************************

testcase tBasicDialogI18nImport

    printlog( "Dialogs that show up when importing multi locale dialogs into BASIC IDE" )
    
    dim cDialogPath as string
        cDialogPath = gTesttoolPath & "framework\required\input\basic_ide\Dialog5.xdl"
        cDialogPath = convertpath( cDialogPath )
        
    dim iNumberOfLanguages as integer
    dim iTry as integer
        
    const LANGUAGE_COUNT = 5
    const MAX_RETRIES = 5
    const WAIT_TIME = 100
    const MODULE_NAME = "tDialogI18nImport"
    const DIALOG_WAIT_TIMEOUT = 2
    const MSG_LOCALE_CLASH_WARNING_BUTTONCOUNT = 4
    const MSG_DIALOG_REPLACE_WARNING_BUTTONCOUNT = 3
        
    printlog( "Open BASIC-IDE" )
    hCreateDocument()
    hInitFormControls( MODULE_NAME )
    
    kontext "ToolsCollectionBar"
    if ( ToolsCollectionBar.exists() ) then ToolsCollectionBar.close()
    
    Kontext "DialogBar"
    if ( DialogBar.exists() ) then
    
        printlog( "Click on <Import> button on <DialogBar>" )
        Import.click()
        
        kontext "OeffnenDlg"
        if ( OeffnenDlg.exists( DIALOG_WAIT_TIMEOUT ) ) then
        
            printlog( "Enter path to test dialog" )
            DateiName.setText( cDialogPath ) 
            Oeffnen.click()
            
            printlog( "Test messagebox" )
            kontext "active"
            if ( Active.exists( DIALOG_WAIT_TIMEOUT ) ) then
            
                printlog( "Messagebox: " & Active.getText() ) 
                if ( Active.getButtonCount() <> MSG_LOCALE_CLASH_WARNING_BUTTONCOUNT ) then
                    warnlog( "Incorrect messagebox, expected locale clash warning" )
                endif
                
                call DialogTest( Active )
                
                printlog( "Accept with yes" )
                hCloseDialog( Active, "yes" )

                Kontext "TranslationBar"
                if ( TranslationBar.exists() ) then

                    for iTry = 1 to MAX_RETRIES
                        iNumberOfLanguages = CurrentLanguage.getItemCount()
                        if ( iNumberOfLanguages = LANGUAGE_COUNT ) then exit for
                        wait( WAIT_TIME )
                    next iTry
                
                    printlog( "Check number of imported languages" )
                    if ( iNumberOfLanguages <> LANGUAGE_COUNT ) then
                        warnlog( "Incorrect language count in listbox <CurrentLanguage>" )
                        printlog( "Expected: " & LANGUAGE_COUNT )
                        printlog( "Found...: " & iNumberOfLanguages )
                    endif
                    
                    Kontext "DialogBar"
                    if ( DialogBar.exists() ) then
                    
                        printlog( "Click on <Import> button on <DialogBar>" )
                        Import.click()
                        
                        kontext "OeffnenDlg"
                        if ( OeffnenDlg.exists( DIALOG_WAIT_TIMEOUT ) ) then

                            printlog( "Enter path to test dialog" )                        
                            DateiName.setText( cDialogPath )
                            Oeffnen.click()
                            
                            printlog( "Test messagebox" )
                            kontext "Active"
                            if ( Active.exists( DIALOG_WAIT_TIMEOUT ) ) then
                            
                                printlog( "Messagebox: " & Active.getText() ) 
                                if ( Active.getButtonCount() <> MSG_DIALOG_REPLACE_WARNING_BUTTONCOUNT ) then
                                    warnlog( "Incorrect messagebox, expected dialog replace warning" )
                                endif
                                
                                call DialogTest( Active )
                                
                                Printlog( "Rename the new dialog" )
                                hCloseDialog( Active, "yes" )
                            else
                                warnlog( "Overwrite warning missing" )
                            endif
                        else
                            warnlog( "<OeffnenDlg> did not open" )
                        endif
                    else
                        warnlog( "<DialogBar> is not available" )
                    endif
                else
                    warnlog( "<TranslationBar> is not available" )
                endif
            else
                warning( "Conflicting locales messagebox is missing" )
            endif
        else
            warnlog( "<OeffnenDlg> did not open" )
        endif
    else
        warnlog( "<DialogBar> is not available" )
    endif
    hCloseBasicIde()
    hDestroyDocument()
    
endcase

