'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_modulehide.inc,v $
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
'*  short description : Verify that hiding modules works
'*
'\******************************************************************************

testcase tBasicIdeModuleHide

    '///<h1>Hiding modules (make them invisible)</h1>
    '///<ul>
    
    const CFN = "tBasicIdeModuleHide::"

    dim rc as integer
    dim brc as boolean
    dim cDefaultTabName as string

    gApplication = "WRITER"
    call hNewDocument()
    
    '///<li>Create a new BASIC module for the current (writer) document</li>
    brc = hOpenBasicOrganizerFromDoc()
    brc = hCreateModuleForDoc()
    
    '///<li>Write a macro (one that is unique to all modules)</li>
    brc = hInsertMacro( 1 )
    if ( brc ) then
       printlog( CFN & "Macro has been written successfully" )
    else
       warnlog( CFN & "Failed to insert macro" )
    endif

    '///<li>Hide the module using the Tab-Bar's context menu</li>
    rc = hHideModule()
    if ( rc > 0 ) then
        warnlog( "Some unexpected error occurred while trying to hide the module" )
    endif

    '///<li>Verify if the module is really hidden.</li>
    try
        ' hTestMacro is expected to fail, so we jump to the catch statement
        rc = hTestMacro( 1 )
        if ( rc = 0 ) then
            warnlog( "For some reason the original module is still visible" )
        else
            warnlog( "There should not be any editingwindow visible" )
        endif
    catch
        printlog( " * unable to locate editwindow -> no module visible." )
    endcatch
    
    '///<li>Open the BASIC organizer and select the hidden module</li> 
    if ( hOpenBasicObjectOrganizer( 1 ) ) then
    
        modulliste.typekeys( "<END><RIGHT><DOWN><RIGHT><DOWN>" )
        '///<li>Click 'Edit' to open the module in the IDE</li>
        try
            bearbeiten.click()
        catch
            warnlog( "#i35097# Crash when editing last module" )
        endcatch
        
         
        '///<li>Verify that the correct module is visible</li>
        rc = hTestMacro( 1 )
        if ( rc = 1 ) then
            printlog( " * the correct macro-module is open. Good." )
        else
            warnlog( "The open macro-module is not the one that was expected" )
        endif
         
        '///<li>Close IDE and document</li>
        hCloseBasicIDE()
    
        call hCloseDocument()      
         
    else
    
        warnlog( "restarting the office to recover from errors" )
        call exitRestartTheOffice()
        
    endif
    
    '///</ul>
    
endcase
