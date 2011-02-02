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
'* short description : updatetest for the printer configuration
'*
'\******************************************************************************

testcase t_updt_spadmin

    printlog( "Resource test for SPAdmin (Printer administration tool) for Unix(like) OS" )

    if ( gtSysName = "eComStation" ) then
        printlog( "No SpAdmin for eComStation" )
        goto endsub
    endif
    
    if ( lcase(gPlatform) = "osx") then
        printlog( "No SPAdmin on MacOS X" )
        goto endsub
    endif
    
    if ( gPlatGroup = "w95" ) then
        printlog( "No SPAdmin on Windows" )
        goto endsub
    endif

    const CPRINTER = "tt-testprinter"
    dim irc as integer
    dim brc as boolean
   
    hShutdownOffice()
   
    brc = hOpenSpAdmin()
    if ( brc ) then
    
        brc = hWaitForSpAdmin()
        if ( not brc ) then
            warnlog( "SpAdmin is not open, the test cannot continue" )
            goto endsub
        endif        
   
        irc = hDelPrinter( CPRINTER )
        select case irc
        case 0 : printlog( "Maybe printer was left over by prior run?" )
        case 3 : printlog( "OK, printer does not exist" )
        end select
      
        brc = hWaitForSpAdmin()
        if ( not brc ) then
            warnlog( "SpAdmin is not open, the test cannot continue" )
            goto endsub
        endif
        
        call TestNewPrinter( CPRINTER )
      
        ' wait for the spadmin to open, on failure we end the test
        brc = hWaitForSpAdmin()
        if ( not brc ) then
            warnlog( "SpAdmin is not open, the test cannot continue" )
            goto endsub
        endif
        
        call TestProperties( CPRINTER )        
      
        ' wait for the spadmin to open, on failure we end the test
        brc = hWaitForSpAdmin()
        if ( not brc ) then
            warnlog( "SpAdmin is not open, the test cannot continue" )
            goto endsub
        endif
        
        call TestRename( CPRINTER )
        
        ' wait for the spadmin to open, on failure we end the test
        brc = hWaitForSpAdmin()
        if ( not brc ) then
            warnlog( "SpAdmin is not open, the test cannot continue" )
            goto endsub
        endif
        
        ' wait for the spadmin to open, on failure we end the test
        brc = hWaitForSpAdmin()
        if ( not brc ) then
            warnlog( "SpAdmin is not open, the test cannot continue" )
            goto endsub
        endif        
         
        
        irc = hDelPrinter( CPRINTER )
        if ( irc <> 0 ) then
            warnlog( "We created a Fax printer but it could not be deleted" )
        endif
        
    endif
   
endcase

'*******************************************************************************

sub TestNewPrinter( cPrinter as string )

    

    dim iWait as integer
    
    printlog( " * New Printer" )
    Kontext "SPAdmin"
    PBNewPrinter.click()
    waitslot
    
    printlog( "   * select 'Connect a Fax Device'" )
    Kontext "TabPWDeviceSelector"
    call dialogtest( TabPWDeviceSelector )
    RBFax.check()
    
    printlog( "   * next ->" )
    Kontext "SpPrinterWizard"
    PBNext.click()
    waitslot
    
    printlog( "   * accept the default driver (first entry)" )
    Kontext "TabPWFaxDriverSelector"
    call dialogtest( TabPWFaxDriverSelector )
    
    printlog( "   * next ->" )
    Kontext "SpPrinterWizard"
    PBNext.click()
    waitslot
    
    printlog( "   * enter '(PHONE)' as queue" )
    Kontext "TabPWQueueCommand"
    call dialogtest( TabPWQueueCommand )
    CBCommand.setText( """(PHONE)""" )
    
    printlog( "   * next ->" )
    Kontext "SpPrinterWizard"
    PBNext.click()
    waitslot
    
    printlog( "   * enter a name for the fax-printer" )
    Kontext "TabPWPrinterName"
    call dialogtest( TabPWPrinterName )
    EFFaxName.setText( cPrinter )
    waitslot( 2000 )
    
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
      
end sub

'*******************************************************************************

sub TestProperties( cPrinter as string )

    

    dim iPrinterPos as integer

    kontext "SpAdmin"
    iPrinterPos = hGetPrinterPosition( CPRINTER , true )
    
    LBPrinters.select( iPrinterPos )
         
    PBProperties.click()
    waitslot
    printlog( " * Properties for the new fax device" )
        
    kontext 
    active.setpage TabSPACommand
      
    kontext "TabSPACommand"
    printlog( "   * Tab: Command" ) 
    call dialogtest( TabSPACommand )
         
    kontext 
    active.setpage TabSPAPaper
      
    kontext "TabSPAPaper"
    printlog( "   * Tab: Paper" ) 
    call dialogtest( TabSPAPaper )
         
    kontext 
    active.setpage TabSPADevice
      
    kontext "TabSPADevice"
    printlog( "   * Tab: Device" ) 
    call dialogtest( TabSPADevice )
         
    kontext 
    active.setpage TabSPAFontReplacement
      
    kontext "TabSPAFontReplacement"
    printlog( "   * Tab: FontReplacement" ) 
    call dialogtest( TabSPAFontReplacement )
       
    kontext 
    active.setpage TabSPAOther
      
    kontext "TabSPAOther"
    printlog( "   * Tab: Other" ) 
    call dialogtest( TabSPAOther )
         
    printlog( "   * close Properties" )
    TabSPAOther.cancel()
    
    printlog( "" )
      
end sub

'*******************************************************************************

sub TestRename( cPrinter as string )

    

    Kontext "SpAdmin"
    LBPrinters.select( hGetPrinterPosition( CPRINTER , true ) )
    waitslot
      
    printlog( " * Open 'Rename' dialog" )
    PBRename.click()
    waitslot
      
    Kontext "SPRenamePrinterDLG"
    call dialogtest( SPRenamePrinterDLG )
    printlog( "   * Close 'Rename' dialog" )
    SPRenamePrinterDLG.cancel()
    
    printlog( "" )

end sub