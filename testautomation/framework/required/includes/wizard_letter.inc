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
'*
'*  short description : Update Test for Letter Wizard
'*
'\******************************************************************************

testcase tUpdtWizardLetter

    printlog( "Resource test for the letter-wizard" )

    ' Build the filename we want to save the template as.
    dim cTemplateName as string
        cTemplateName = "FWK-Testtool-Template-letterWizard.ott"
        
    dim cTemplatePath as string
        cTemplatePath = gOfficePath & "user\template\" & cTemplateName
        cTemplatePath = convertpath( cTemplatePath )    
        
    hDeleteUserTemplates()
        
    hInitSingleDoc()

    FileAutopilotLetter
    kontext "AutopilotLetter"
    if ( AutopilotLetter.exists( 2 ) ) then

        if ( not autopilotletter.exists( 3 ) ) then
            warnlog( "#i87733# Wizard does not start" )
            hCloseDocument()
            goto endsub
        endif
          
        printlog( " * Page: Page Design" )
        call DialogTest( AutopilotLetter, 1 )

        printlog( "   * select a business letter" )
        Business.check()
        WaitSlot()

        kontext "active"
        if ( active.exists( 1 ) ) then
            warnlog( "   * refusing to change the default template" )
            active.cancel()
        endif
        WaitSlot( 2000 )

        kontext "Autopilotletter"    
        BusinessStyle.select( 2 )
        WaitSlot( 2000 )

        kontext "active"
        if ( active.exists( 1 ) ) then
            warnlog( "   * refusing to change the default template" )
            active.cancel()
        endif
        WaitSlot( 3000 )

        kontext "AutopilotLetter"
        LetterHead.Check()

        hClickButton( NextButton )

        kontext "AutopilotLetter"
        call DialogTest( AutopilotLetter, 2 )
        
        printlog( "   * check to add logo" )
        Logo.check()
        
        printlog( "   * change the metrics of the logo" )
        LogoHeight.settext( "4" )
        LogoWidth.settext( "4" )
        LogoXpos.settext( "1" )
        LogoYpos.settext( "1" )
        
        printlog( "   * check to add own address" )
        ReturnAddress.check()
        
        printlog( "   * change the metrics of the address-field" )
        AddressHeight.settext( "4" )
        AddressWidth.settext( "4" )
        AddressXpos.settext( "1" )
        AddressYpos.settext( "1" )
        
        printlog( "   * check to show return-address in envelope window" )
        ReturnAddressCompany.check()
        
        printlog( "   * UNcheck to add a footer" )
        IncludeFooter.unCheck()
        
        printlog( "   * change the height" )
        if ( FooterHeight.isEnabled() ) then
            warnlog( "FooterHeight should not be enabled in this configuration" )
        endif
        
        hClickButton( NextButton )

        kontext "AutopilotLetter"
        call DialogTest( AutopilotLetter, 3 )    
        
        printlog( "   * change letterformat to the second entry" )
        LetterPageNorm.select( 2 )
        
        printlog( "   * change salutation to the second entry" )
        ListSalutation.select( 2 )
        
        printlog( "   * change Complimentary Close to second entry" )
        ListGreetings.select( 2 )
        hClickButton( NextButton )

        kontext "AutopilotLetter"
        call DialogTest( AutopilotLetter, 4 )
        hClickButton( NextButton )

        kontext "AutopilotLetter"
        call DialogTest( AutopilotLetter, 5 )    
        
        printlog( "   * enter some text as footer" )
        TextFooter.settext( "Some non-offending text" )
        
        printlog( "   * check Include only on second ..." )
        IncludeFromPage2.check()
        hClickButton( NextButton )

        kontext "AutopilotLetter"
        call DialogTest( AutopilotLetter, 6 )   
        
        printlog( "   * name the template for further usage" )
        TemplateName.setText( cTemplateName )
        hSetTemplateSavePath( cTemplatePath )

        kontext "AutopilotLetter"
        hClickButton( FinishButton )

        if ( gOOo ) then
            kontext "UseOfThisTemplate"
            if ( UseOfThisTemplate.exists( 2 ) ) then
                printlog( "cancelling UseOfThisTemplate-dialog for OOo" )
                UseOfThisTemplate.cancel()
            else
                qaerrorlog( "UseOfThisTemplate-dialog not present" )
            endif
        endif
        
        if ( hHandleSaveError() = 1 ) then
            kontext "AutopilotLetter"
            hClickButton( FinishButton )
        endif
        
        if ( not hDestroyDocument() ) then qaerrorlog( "#i59233# The wizard does not display the new template" )
    else
        warnlog( "Autopilot Letter not open/exceeded timeout" )
    endif
    
    hDeleteFile( cTemplatePath )
    
    if ( hDeleteUserTemplates() <> 0 ) then
        printlog( "Unexpectedly deleted user template(s), please check")
    endif
    
    hFileCloseAll()

endcase


