'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_delete_modules.inc,v $
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
'/************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : 
'*
'\******************************************************************************

testcase tBasicDialogI18nImport

    printlog( "Dialogs that show up when importing Multi locale dialogs into BASIC IDE" )
    
    dim cDialogPath as string
        cDialogPath = gTesttoolPath & "framework\required\input\basic_ide\Dialog5.xdl"
        cDialogPath = convertpath( cDialogPath )
        
    printlog( "Open BASIC-IDE" )
    hCreateDocument()
    hInitFormControls( "tDialogI18nImport" )
    
    kontext "ToolsCollectionBar"
    if ( ToolsCollectionBar.exists() ) then
        ToolsCollectionBar.close()
    endif
    
    Kontext "DialogBar"
    if ( DialogBar.exists() ) then
    
        printlog( "Click on <Import> button on <DialogBar>" )
        Import.click()
        
        kontext "OeffnenDlg"
        if ( OeffnenDlg.exists( 2 ) ) then
        
            printlog( "Enter path to test dialog" )
            DateiName.setText( cDialogPath ) 
            Oeffnen.click()
            
            printlog( "Test messagebox" )
            kontext "active"
            if ( Active.exists( 2 ) ) then
            
                printlog( "Messagebox: " & Active.getText() ) 
                if ( Active.getButtonCount <> 4 ) then
                    warnlog( "Incorrect messagebox, expected locale clash warning" )
                endif
                
                call DialogTest( Active )
                
                printlog( "Accept with yes" )
                Active.Yes()
                
                Kontext "TranslationBar"
                if ( TranslationBar.exists() ) then
                
                    printlog( "Check number of imported languages, should be 5" )
                    if ( CurrentLanguage.getItemCount <> 5 ) then
                        warnlog( "Five languages should be listed in the languages box" )
                    endif
                    
                    Kontext "DialogBar"
                    if ( DialogBar.exists() ) then
                    
                        printlog( "Click on <Import> button on <DialogBar>" )
                        Import.click()
                        
                        kontext "OeffnenDlg"
                        if ( OeffnenDlg.exists( 2 ) ) then

                            printlog( "Enter path to test dialog" )                        
                            DateiName.setText( cDialogPath )
                            Oeffnen.click()
                            
                            printlog( "Test messagebox" )
                            kontext "Active"
                            if ( Active.exists( 2 ) ) then
                            
                                printlog( "Messagebox: " & Active.getText() ) 
                                if ( Active.getButtonCount <> 3 ) then
                                    warnlog( "Incorrect messagebox, expected dialog replace warning" )
                                endif
                                
                                call DialogTest( Active )
                                
                                Printlog( "Rename the new dialog" )
                                Active.Yes()
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
                warning( "Locale clash messagebox is missing" )
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

