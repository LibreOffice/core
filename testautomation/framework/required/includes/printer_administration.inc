'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
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
'*************************************************************************

*****

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