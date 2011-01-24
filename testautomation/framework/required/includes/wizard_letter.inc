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
'*  owner : gregor.hartmann@oracle.com
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


