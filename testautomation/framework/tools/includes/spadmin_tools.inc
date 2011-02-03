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
'* owner : gregor.hartmann@oracle.com
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
                hDelPrinter() = 0
            else
                warnlog( CFN & "Confirm Delete Dialog is missing" )
                hDelPrinter() = 1
            endif
        catch
            warnlog( CFN & "Unable to confirm printer deletion" )
            hDelPrinter() = 2
        endcatch
      
    else
   
        printlog( CFN & "The printer queue does not exist" )
        hDelPrinter() = 3
      
    endif
   
    Kontext "SpAdmin"

end function            

'*******************************************************************************

function hGetSpadminPath() as string

    hGetSpadminPath() = convertpath( gNetzOfficePath & "program\spadmin" )
   
end function

'*******************************************************************************

function hShutdownOffice()

    hFileCloseAll()
    FileExit( "SynchronMode", TRUE ) 
    Wait( 5000 )
   
end function

'*******************************************************************************

function hOpenSpadmin() as boolean

    dim cSpadminPath as string : cSpadminPath = hGetSpadminPath()
        
    try
        start( cSpadminPath  , "-enableautomation" )
        printlog( "Printer administration start-command was dispatched correctly" )
        hOpenSpAdmin() = true
    catch
        warnlog( "hOpenSpAdmin(): Failed to open printer administration tool" )
        hOpenSpAdmin() = false
    endcatch

end function

'********************************************************************************

function hWaitForSpAdmin() as boolean

    kontext "SpAdmin"
    if ( SpAdmin.exists( 10 ) ) then
        printlog( "Printer administration tool is open" )
        hWaitForSpadmin() = true
        sleep( 10 )
    else
        warnlog( "hWaitForSpAdmin(): The dialog did not open within 10 seconds, aborting" )
        hWaitForSpadmin() = false
    endif

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
