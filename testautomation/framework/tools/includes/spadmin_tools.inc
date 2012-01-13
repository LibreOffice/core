'encoding UTF-8  Do not remove or change this line!
'**************************************************************
'  
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'  
'**************************************************************
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
