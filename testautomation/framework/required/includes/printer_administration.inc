'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: printer_administration.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:19:03 $
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
'* owner : joerg.skottke@sun.com
'*
'* short description : updatetest for the printer configuration
'*
'\******************************************************************************

testcase t_updt_spadmin

    '///<h1>Update test for the SpAdmin printer configuration tool</h1>
    '///<ul>

    '///+<li>Do not test on eComStation</li> 
    if ( gtSysName = "eComStation" ) then
        printlog( "No SpAdmin for eComStation" )
        goto endsub
    endif
    
    '///+<li>Do not test on MacOS X</li>
    if ( lcase(gPlatform) = "osx") then
        printlog( "No SPAdmin on MacOS X" )
        goto endsub
    endif
    
    '///+<li>Do not test on Windows</li>   
    if ( gPlatGroup = "w95" ) then
        printlog( "No SPAdmin on Windows" )
        goto endsub
    endif

    const CPRINTER = "tt-testprinter"
    dim irc as integer
    dim brc as boolean
   
    '///+<li>Shut down the office completely</li>
    hShutdownOffice()
   
    '///+<li>Open SpAdmin</li>
    brc = hOpenSpAdmin()
    if ( brc ) then
    
        '///+<li>wait for the spadmin to open, on failure we end the test</li>
        brc = hWaitForSpAdmin()
        if ( not brc ) then
            warnlog( "SpAdmin is not open, the test cannot continue" )
            goto endsub
        endif        
   
        '///+<li>If a printer queue was left over from prior run: Delete ist</li>
        irc = hDelPrinter( CPRINTER )
        select case irc
        case 0 : printlog( "Maybe printer was left over by prior run?" )
        case 3 : printlog( "OK, printer does not exist" )
        end select
      
        '///+<li>wait for the spadmin to open, on failure we end the test</li>
        brc = hWaitForSpAdmin()
        if ( not brc ) then
            warnlog( "SpAdmin is not open, the test cannot continue" )
            goto endsub
        endif
        
        '///+<li>Create a new fax device</li>
        call TestNewPrinter( CPRINTER )
      
        ' wait for the spadmin to open, on failure we end the test
        brc = hWaitForSpAdmin()
        if ( not brc ) then
            warnlog( "SpAdmin is not open, the test cannot continue" )
            goto endsub
        endif
        
        '///+<li>Walk through the properties of the new fax device</li>
        call TestProperties( CPRINTER )        
      
        ' wait for the spadmin to open, on failure we end the test
        brc = hWaitForSpAdmin()
        if ( not brc ) then
            warnlog( "SpAdmin is not open, the test cannot continue" )
            goto endsub
        endif
        
        '///+<li>Rename the fax device</li>
        call TestRename( CPRINTER )
        
        ' wait for the spadmin to open, on failure we end the test
        brc = hWaitForSpAdmin()
        if ( not brc ) then
            warnlog( "SpAdmin is not open, the test cannot continue" )
            goto endsub
        endif

        '///+<li>Look at the font settings</li>
        call TestFonts( CPRINTER )   
        
        ' wait for the spadmin to open, on failure we end the test
        brc = hWaitForSpAdmin()
        if ( not brc ) then
            warnlog( "SpAdmin is not open, the test cannot continue" )
            goto endsub
        endif        
         
        
        '///+<li>Delete the fax device</li>
        irc = hDelPrinter( CPRINTER )
        if ( irc <> 0 ) then
            warnlog( "We created a Fax printer but it could not be deleted" )
        endif
        
    endif
    '///</ul>
   
endcase

'*******************************************************************************

sub TestNewPrinter( cPrinter as string )

    '///<h3>Update test for creation of a new device in SpAdmin</h3>
    '///<i>Starting point: SpAdmin</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Name of the device (string)</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    

    dim iWait as integer
    
    '///+<li>Click to create a new printer</li>
    printlog( " * New Printer" )
    Kontext "SPAdmin"
    PBNewPrinter.click()
    waitslot
    
    '///+<li>Check to create a fax device</li>
    printlog( "   * select 'Connect a Fax Device'" )
    Kontext "TabPWDeviceSelector"
    call dialogtest( TabPWDeviceSelector )
    RBFax.check()
    
    '///+<li>Click the &quot;Next&quot; button</li>
    printlog( "   * next ->" )
    Kontext "SpPrinterWizard"
    PBNext.click()
    waitslot
    
    '///+<li>Keep the default settings on this page</li>
    printlog( "   * accept the default driver (first entry)" )
    Kontext "TabPWFaxDriverSelector"
    call dialogtest( TabPWFaxDriverSelector )
    
    '///+<li>Click the &quot;Next&quot; button</li>
    printlog( "   * next ->" )
    Kontext "SpPrinterWizard"
    PBNext.click()
    waitslot
    
    '///+<li>Enter &quot;(PHONE)&quot; as queue</li>
    printlog( "   * enter '(PHONE)' as queue" )
    Kontext "TabPWQueueCommand"
    call dialogtest( TabPWQueueCommand )
    CBCommand.setText( """(PHONE)""" )
    
    '///+<li>Click the &quot;Next&quot; button</li>
    printlog( "   * next ->" )
    Kontext "SpPrinterWizard"
    PBNext.click()
    waitslot
    
    '///+<li>Enter a name for the fax printer</li>
    printlog( "   * enter a name for the fax-printer" )
    Kontext "TabPWPrinterName"
    call dialogtest( TabPWPrinterName )
    EFFaxName.setText( cPrinter )
    waitslot( 2000 )
    
    '///+<li>Click the &quot;Finish&quot; button</li>
    printlog( "   * Finish" )
    for iWait = 1 to 3
        try
            Kontext "SpPrinterWizard"
            SpPrinterWizard.ok()
            iWait = 4 ' leave the loop
        catch
            printlog( "Waiting for dialog" )
            sleep( 1 )
            if ( iWait = 3 ) then
                qaerrorlog( "Could not click 'finish' on printerwizard" )
            endif
        endcatch
    next iWait
    
    printlog( "" )
    '///</ul>
      
end sub

'*******************************************************************************

sub TestProperties( cPrinter as string )

    '///<h3>Update test for the device properties in SpAdmin</h3>
    '///<i>Starting point: SpAdmin</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Name of the printer (string)</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    

    dim iPrinterPos as integer

    '///+<li>Find the printer in the list</li>
    kontext "SpAdmin"
    iPrinterPos = hGetPrinterPosition( CPRINTER , true )
    
    '///+<li>Select it</li>
    LBPrinters.select( iPrinterPos )
         
    '///+<li>Click the properties-button</li>
    PBProperties.click()
    waitslot
    printlog( " * Properties for the new fax device" )
        
    '///<ol>
    '///+<li>Command-Page</li>
    kontext 
    active.setpage TabSPACommand
      
    kontext "TabSPACommand"
    printlog( "   * Tab: Command" ) 
    call dialogtest( TabSPACommand )
         
    '///+<li>Paper-Page</li>
    kontext 
    active.setpage TabSPAPaper
      
    kontext "TabSPAPaper"
    printlog( "   * Tab: Paper" ) 
    call dialogtest( TabSPAPaper )
         
    '///+<li>Device-Page</li>
    kontext 
    active.setpage TabSPADevice
      
    kontext "TabSPADevice"
    printlog( "   * Tab: Device" ) 
    call dialogtest( TabSPADevice )
         
    '///+<li>Fontreplacement-Page</li>
    kontext 
    active.setpage TabSPAFontReplacement
      
    kontext "TabSPAFontReplacement"
    printlog( "   * Tab: FontReplacement" ) 
    call dialogtest( TabSPAFontReplacement )
       
    '///+<li>Other-Page</li>
    kontext 
    active.setpage TabSPAOther
      
    kontext "TabSPAOther"
    printlog( "   * Tab: Other" ) 
    call dialogtest( TabSPAOther )
         
    '///</ol>
    '///+<li>Cancel dialog</li>
    printlog( "   * close Properties" )
    TabSPAOther.cancel()
    
    printlog( "" )
    '///</ul>
      
end sub

'*******************************************************************************

sub TestRename( cPrinter as string )

    '///<h3>Update test for the rename dialog in SpAdmin</h3>
    '///<i>Starting point: SpAdmin</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Name of the printer queue to rename (string)</li>
    '///<ul>
    '///+<li>The named queue must exist</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    

    '///+<li>Find the requested printer queue</li>
    Kontext "SpAdmin"
    LBPrinters.select( hGetPrinterPosition( CPRINTER , true ) )
    waitslot
      
    '///+<li>Click to rename the queue</li>
    printlog( " * Open 'Rename' dialog" )
    PBRename.click()
    waitslot
      
    '///+<li>Cancel the rename dialog</li>
    Kontext "SPRenamePrinterDLG"
    call dialogtest( SPRenamePrinterDLG )
    printlog( "   * Close 'Rename' dialog" )
    SPRenamePrinterDLG.cancel()
    
    printlog( "" )
    '///</ul>

end sub

'*******************************************************************************

sub TestFonts( cPrinter as string )

    '///<h3>Update test for SpAdmin printer font replacement</h3>
    '///<i>Starting point: SpAdmin</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Name of the printer queue (string)</li>
    '///<ul>
    '///+<li>The queue must exist</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    '///+<li>Press the Fonts-button - the fonts dialog opens</li>
    printlog( " * Open 'Fonts...' dialog" )
    
    Kontext "SpAdmin"
    if ( PBFonts.isEnabled() ) then
        PBFonts.click()
        waitslot
          
        printlog( "   * Test the font-dialog" )
        Kontext "SPFontNameDLG"
        call dialogtest( SPFontNameDLG )
          
        '///+<li>Click to add fonts - import dialog opens</li>
        printlog( "   * Press 'Add ...'" )
        PBAdd.click()
        waitslot
          
        '///+<li>Cancel the font-import dialog</li>
        printlog( "   * Test the import-dialog" )
        kontext "SPFontImportDLG"
        call dialogtest( SPFontImportDLG )
          
        printlog( "   * close import dialog" )
        SPFontImportDLG.cancel()
          
        '///+<li>Close the Font-dialog with ok</li>
        Kontext "SPFontNameDLG"
        printlog( "   * close fonts-dialog" )
        SPFontNameDLG.ok()
        waitslot
    else
        printlog( "Fonts... not available, skipping." )
    endif
        
    '///</ul>

end sub
