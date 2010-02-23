'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
'/************************************************************************
'*
'* owner : joerg.skottke@sun.com
'*
'* short description : helper functions for SPAdmin
'*
'\******************************************************************************

function hGetPrinterPosition( cName as string, bWarn as boolean ) as integer

    '///<h3>Find a printer queue in the SpAdmin list</h3>
    ' IN:
    ' cName = Name of the queue to look for
    ' bWarn = if TRUE we warn if the queue does not exist

    const CFN = "hGetPrinterPosition::"
    
    if ( cName = "" ) then
        warnlog( CFN & "Invalid Parameter passed to function: Empty String" )
        hGetPrinterPosition() = -1
        exit function
    endif
  
    dim iCurrentQueue as integer
    dim bFound as boolean
    dim iPrinterCount as integer
        iPrintercount = LBPrinters.getItemCount()

    Kontext "SpAdmin"
    bFound = false   
                
    for iCurrentQueue = 1 to iPrinterCount
   
        wait( 200 )
      
        LBPrinters.select( iCurrentQueue )
        if ( LBPrinters.getseltext() = cName ) then
            bFound = true
            exit for
        endif
         
    next iCurrentQueue
   
    ' warn if queue was not found and we requested a warning
    if ( not bFound and bWarn ) then
        iCurrentQueue = 0
        printlog( CFN & "The specified printer queue could not be found" )
    endif
   
    ' print a message that the printer queue exists
    if ( bFound ) then
        printlog( CFN & "Printer Queue was found at pos " & iCurrentQueue )
    endif
   
    Kontext "SpAdmin"
    hGetPrinterPosition() = iCurrentQueue
                   
end function      

'*******************************************************************************

function hDelPrinter( cPrinterName as string ) as integer

    '///<h3>Delete a printer queue by its name in SpAdmin</h3>
    
    const CFN = "hDelPrinter::"

    ' delete a printer-queue from the printers-list by name. Only exact matches
    ' will be removed.
    
    ' IN:
    ' - Name of the queue
    ' OUT:
    ' -1 = Bad function call
    '  0 = Success
    '  1 = Confirmation Dialog for Delete is missing
    '  2 = Unable to press "OK" on Confirm-Delete Dialog
    '  3 = Printer queue does not exist so it was not deleted
    
    if ( cPrinterName = "" ) then
        warnlog( CFN & "Invalid Parameter passed to function: Empty String" )
        hDelPrinter() = -1
        exit function
    endif

    dim iPrinterPos as integer
    dim iErr as integer
        iErr = 1 

    Kontext "SpAdmin"
    iPrinterPos = hGetPrinterPosition( cPrinterName , true )
   
    if ( iPrinterPos > 0 ) then
   
        LBPrinters.select( iPrinterPos )
        PBRemove.click()
      
        try
            Kontext "Active"
            if ( active.exists( 2 ) ) then
                Active.Yes()
                printlog( CFN & "Printer Queue deleted" )
                iErr = 0
            else
                warnlog( CFN & "Confirm Delete Dialog is missing" )
                iErr = 1
            endif
        catch
            warnlog( CFN & "Unable to confirm printer deletion" )
            iErr = 2
        endcatch
      
    else
   
        printlog( CFN & "The printer queue does not exist" )
        iErr = 3
      
    endif
   
    Kontext "SpAdmin"
    hDelPrinter() = iErr

end function            

'*******************************************************************************

function hGetSpadminPath() as string

    '///<h3>Retrieve the path to the SpAdmin script/binary</h3>
    const CFN = "hGetSpadminPath::"
    const C_REL_PATH = "program\spadmin"

    dim sPath as string
   
    sPath = gNetzOfficePath & C_REL_PATH
    sPath = convertpath( sPath ) 

    printlog( CFN & "Using SPAdmin from: " & sPath

    hGetSpadminPath() = sPath
   
end function

'*******************************************************************************

function hShutdownOffice() as integer

    '///<h3>Shutdown the office by closing all docs and the backing window</h3>
    const CFN = "hShutdownOffice::"

    dim iOpenDocs as integer
        iOpenDocs = getDocumentCount()
    dim iThisDoc as integer
       
    ' close all open documents (One open document to remain)
    for iThisDoc = 1 to iOpenDocs
        call hCloseDocument()
    next iThisDoc
   
    ' see how many documents are still open - should be exactly one
    iOpenDocs = getDocumentCount()
    if ( iOpenDocs <> 0 ) then
        warnlog( CFN & "No open documents expected but found: " & iOpenDocs )
    endif
   
    ' shutdown the backing window, do not test with getDocumentCount() because
    ' this would inevitably restart the office
    ' we need some additional parameter for FileExit, this is a bug
    FileExit( "SynchronMode", TRUE ) 
    
    ' wait long enough to ensure all office threads are removed from memory
    sleep( 5 )
   
    ' Print a somehow fuzzy message, we do not know for sure whether the office
    ' has been shutdown or not
    printlog( CFN & "The office should have been closed by now." )
    hShutdownOffice() = iOpenDocs
   
end function

'*******************************************************************************

function hOpenSpadmin() as boolean

    '///<h3>Execute the SpAdmin binary/Script and verify that it is open</h3>
    ' Return TRUE if hWaitForSpadmin() completes successfully

    const CFN = "hOpenSpadmin::"

    dim cSpadminPath as string
        cSpadminPath = hGetSpadminPath()
        
    dim brc as boolean
       
    ' start SPAdmin in automation mode. 
    try
        start( cSpadminPath  , "-enableautomation" )
        printlog( CFN & "SpAdmin command executed successfully" )
        brc = true
    catch
        warnlog( CFN & "Failure: SpAdmin command did not succeed" )
        brc = false
    endcatch
    
    hOpenSpAdmin() = brc 

end function

'********************************************************************************

function hWaitForSpAdmin() as boolean

    '///<h3>Wait for SpAdmin to be loaded and displayed</h3>
    const CFN = "hWaitForSpAdmin::"
   
    dim bOpen as boolean
   
    ' Wait for SpAdmin to open
    kontext "SpAdmin"
    if ( SpAdmin.exists( 10 ) ) then
        printlog( CFN & "SpAdmin is open. Good." )
        sleep( 10 )
        bOpen = true
    else
        warnlog( CFN & "SpAdmin is not open, the test cannot continue" )
        bOpen = false
    endif
   
    hWaitForSpadmin() = bOpen

end function

'*******************************************************************************

function hCreateFaxDevice( cName as string ) as boolean

    '///<h3>Open the printer creation dialog and create a fax device</h3>
    ' The function verifies that the device has been created and returns
    ' TRUE on success
    '///<ul>
    
    const CFN = "hCreateFaxDevice::"
    
    if ( cName = "" ) then
        warnlog( CFN & "Invalid Parmeter passed to function: Empty String" )
        hCreateFaxDevice() = false
        exit function
    endif

    ' quickly greates a fax device accepting all defaults
    '///+<li>Click on "New Printer"</li>
    Kontext "SpAdmin"
    PBNewPrinter.click()
   
    '///+<li>Click on "Next..."</li>
    Kontext "SpPrinterWizard"
    PBNext.click()
   
    '///+<li>Click on "Next..."</li>
    Kontext "SpPrinterWizard"
    PBNext.click()

    '///+<li>Enter "(PHONE)" as queue command</li>
    Kontext "TabPWQueueCommand"
    CBCommand.setText( """(PHONE)""" )
   
    '///+<li>Click on "Next..."</li>
    Kontext "SpPrinterWizard"
    PBNext.click()
   
    '///+<li>Enter a Fax-Printer Name</li>
    Kontext "TabPWPrinterName"
    EFFaxName.setText( cName )
   
    '///+<li>Finish the wizard by pressing OK</li>
    Kontext "SpPrinterWizard"
    SpPrinterWizard.ok()
   
    sleep( 1 )

    '///+<li>Verify that the Queue has been created in SpAdmin</li>
    Kontext "SpAdmin"
    if ( hgetPrinterPosition( cName ) <> 0 ) then
        hCreateFaxDevice() = true
        printlog( CFN & "Successfully created Fax device" )
    else
        hCreateFaxDevice() = false
        warnlog( CFN & "Failed to create a Fax device" )
    endif
    '///</ul>
   
end function      
