'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: filedlg_triggers.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: jsk $ $Date: 2008-06-20 08:03:53 $
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
'*  short description : Alternative ways to open file-dialogs / slot availablilty
'*
'\******************************************************************************

testcase tDialogTriggers

    '///<h1>Alternative ways to open file-dialogs / slot availablilty</h1>

    '///<u><pre>Synopsis</pre></u>Test that some slots for the file-save, file-save-as 
    '///+ and file-open dialogs are not available in special situations, using accelerators.<br>
    '///<u><pre>Specification document</pre></u>No specification provided<br>
    '///<u><pre>Files used</pre></u>framework/tools/input/accelerators.txt<br>framework/tools/inc/t_accels.inc<br>
    '///<u><pre>Test case specification</pre></u>
    '///<ul>
    
    dim cAcceleratorFileOpen as string
    dim cAcceleratorFileSave as string
    dim cAcceleratorFileSaveAs as string
    
    '///+<li>Make sure we are on the backing window</li>
    do while ( getDocumentCount > 0 ) 
        call hCloseDocument()
    loop
    
    '///+<li>Get the accelerators from reference file for FileOpen, FileSave
    '///+ and FileSaveAs</li>
    cAcceleratorFileOpen   = hGetAccel( "FileOpen" )
    cAcceleratorFileSave   = hGetAccel( "FileSave" )
    cAcceleratorFileSaveAs = hGetAccel( "FileSaveAs" )


    '///+<li>Actions on backing window</li>
    '///<ul>
    '///+<li>Test case 1: FileOpen should be possible from backing window (accelerator)</li>
    printlog( "Test case 1: FileOpen should be possible from backing window (accelerator)" )

    kontext "DocumentBackground"
    if ( DocumentBackground.exists( 1 ) ) then
    
        DocumentBackground.typeKeys( cAcceleratorFileOpen )
        
        kontext "OeffnenDlg"
        if ( OeffnenDlg.exists( 1 ) ) then
            printlog( "File Open dialog is open. Good." )
            OeffnenDlg.cancel()
        else
            warnlog( "File Open dialog did not open." )
        endif
    else
        warnlog( "Backing window not found" )
    endif
    
    
    
    '///+<li>Test case 2: File Save should not work from backing window (accelerator)</li>
    printlog( "Test case 2: File Save should not work from backing window (accelerator)" )

    kontext "DocumentBackground"
    if ( DocumentBackground.exists( 1 ) ) then
    
        DocumentBackground.typeKeys( cAcceleratorFileSave )
        
        kontext "SpeichernDlg"
        if ( SpeichernDlg.exists( 1 ) ) then
            warnlog( "File Save dialog should not open." )
            SpeichernDlg.cancel()
        else
            printlog( "File Open dialog is not open. Good." )
        endif
    else
        warnlog( "Backing window not found" )
    endif
    
    
    
    '///+<li>Test case 3: File Save As should not work from backing window (accelerator)</li>
    printlog( "Test case 3: File Save As should not work from backing window (accelerator)" )
    
    kontext "DocumentBackground"
    if ( DocumentBackground.exists( 1 ) ) then
    
        DocumentBackground.typeKeys( cAcceleratorFileSaveAs )
        
        kontext "SpeichernDlg"
        if ( SpeichernDlg.exists( 1 ) ) then
            warnlog( "File Save dialog should not open." )
            SpeichernDlg.cancel()
        else
            printlog( "File Open dialog is not open. Good." )
        endif
    else
        warnlog( "Backing window not found" )
    endif
    
    
    '///+<li>Test case 4: FileOpen should be possible from backing window (slot)</li>
    printlog( "Test case 4: FileOpen should be possible from backing window (slot)" )

    kontext "DocumentBackground"
    if ( DocumentBackground.exists( 1 ) ) then
    
        FileOpen
        
        kontext "OeffnenDlg"
        if ( OeffnenDlg.exists( 1 ) ) then
            printlog( "File Open dialog is open. Good." )
            OeffnenDlg.cancel()
        else
            warnlog( "File Open dialog did not open." )
        endif
    else
        warnlog( "Backing window not found" )
    endif
    
    
    
    '///+<li>Test case 5: File Save should not work from backing window (slot)</li>
    printlog( "Test case 5: File Save should not work from backing window (slot)" )

    kontext "DocumentBackground"
    if ( DocumentBackground.exists( 1 ) ) then
    
        try
            FileSave
        
            kontext "SpeichernDlg"
            if ( SpeichernDlg.exists( 1 ) ) then
                warnlog( "File Save dialog should not open." )
                SpeichernDlg.cancel()
            else
                printlog( "File Open dialog is not open. Good." )
            endif
        catch
            printlog( "Slot blocked, good" )
        endcatch

    else
        warnlog( "Backing window not found" )
    endif
    
    
    
    '///+<li>Test case 6: File Save As should not work from backing window (slot)</li>
    printlog( "Test case 6: File Save As should not work from backing window (slot)" )
    
    kontext "DocumentBackground"
    if ( DocumentBackground.exists( 1 ) ) then
    
        try
            FileSaveAs
        
            kontext "SpeichernDlg"
            if ( SpeichernDlg.exists( 1 ) ) then
                warnlog( "File Save dialog should not open." )
                SpeichernDlg.cancel()
            else
                printlog( "File Open dialog is not open. Good." )
            endif
        catch
            printlog( "Slot blocked, good" )
        endcatch

    else
        warnlog( "Backing window not found" )
    endif
    
    '///</ul>
    


    '///+<li>Actions on unmodified documents (Writer)</li>
    '///<ul>
    
    gApplication = "WRITER"
    
    '///+<li>Open a new writer document</li>
    call hNewDocument()
    
    '///+<li>Test case 7: File Save should not work from unmodified documents (accelerator)</li>
    printlog( "Test case 7: File Save should not work from unmodified documents (accelerator)" )

    kontext "DocumentWriter"
    if ( DocumentWriter.exists( 1 ) ) then
    
        DocumentWriter.typeKeys( cAcceleratorFileSave )
        
        kontext "SpeichernDlg"
        if ( SpeichernDlg.exists( 1 ) ) then
            warnlog( "File Save dialog should not open." )
            SpeichernDlg.cancel()
        else
            printlog( "File Open dialog is not open. Good." )
        endif
    else
        warnlog( "Writer document not not found" )
    endif
    
    
    
    '///+<li>Test case 8: File Save As should work from unmodified documents (accelerator)</li>
    printlog( "Test case 8: File Save As should work from unmodified documents (accelerator)" )

    kontext "DocumentWriter"
    if ( DocumentWriter.exists( 1 ) ) then
    
        DocumentWriter.typeKeys( cAcceleratorFileSaveAs )
        
        kontext "SpeichernDlg"
        if ( SpeichernDlg.exists( 1 ) ) then
            printlog( "File Save dialog is open. Good." )
            SpeichernDlg.cancel()
        else
            warnlog( "File Open dialog is not open." )
        endif
    else
        warnlog( "Writer document not not found" )
    endif


    '///+<li>Test case 9: File Save should not work from unmodified documents (slot)</li>
    printlog( "Test case 9: File Save should not work from unmodified documents (slot)" )

    kontext "DocumentWriter"
    if ( DocumentWriter.exists( 1 ) ) then
    
        try
            FileSave
        
            kontext "SpeichernDlg"
            if ( SpeichernDlg.exists( 1 ) ) then
                warnlog( "File Save dialog should not open." )
                SpeichernDlg.cancel()
            else
                printlog( "File Save dialog is not open. Good." )
            endif
        catch
            printlog( "Slot blocked, good" )
        endcatch
    else
        warnlog( "Writer document not not found" )
    endif
    
    
    
    '///+<li>Test case 10: File Save As should work from unmodified documents (slot)</li>
    printlog( "Test case 10: File Save As should work from unmodified documents (slot)" )

    kontext "DocumentWriter"
    if ( DocumentWriter.exists( 1 ) ) then
    
        try
            FileSaveAs
        
            kontext "SpeichernDlg"
            if ( SpeichernDlg.exists( 1 ) ) then
                printlog( "File Save dialog is open. Good." )
                SpeichernDlg.cancel()
            else
                warnlog( "File Save dialog should open." )
            endif
        catch
            warnlog( "Slot blocked" )
        endcatch
    else
        warnlog( "Writer document not not found" )
    endif
    
    '///+<li>Close the document</li>
    call hCloseDocument()
    '///</ul>
    '///</ul>

endcase

