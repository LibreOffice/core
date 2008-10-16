'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: wizard_letter.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: rt $ $Date: 2008-08-01 09:48:44 $
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
'*  short description : Update Test for Letter Wizard
'*
'\******************************************************************************

testcase tUpdtWizardLetter

    '///<h1>Update test for the letter wizard</h1>
    '///<ul>
    
    dim iErr as integer
    dim brc as boolean
    dim irc as integer
    
    ' Build the filename we want to save the template as.
    dim cTemplateName as string
        cTemplateName = "FWK-Testtool-Template-letterWizard.ott"
        
    dim cTemplatePath as string
        cTemplatePath = gOfficePath & "user\template\" & cTemplateName
        cTemplatePath = convertpath( cTemplatePath )    
        
    hDeleteUserTemplates()
        
    '///+<li>Make sure exactly one single writer document is open</li>
    hInitSingleDoc()

    '///+<li>open the letterwizard using the menu, page design opens</li>
    hOpenWizardWithMenu( "LETTER" )

    '///+<li>Page 1 </li>
    kontext "Autopilotletter"  
    if ( not autopilotletter.exists( 3 ) ) then
        warnlog( "#i87733# Wizard does not start" )
        hCloseDocument()
        goto endsub
    endif
      
    printlog( " * Page: Page Design" )
    call dialogtest2( AutopilotLetter, 1 )

    '///+<li>select to create a business letter</li>
    printlog( "   * select a business letter" )
    Business.check()
    sleep( 1 )

    '///+<li>do not accept to overwrite the default template</li>
    kontext "active"
    if ( active.exists( 1 ) ) then
        warnlog( "   * refusing to change the default template" )
        active.cancel()
    endif
    sleep( 3 )

    '///+<li>select business style (here: 2. entry in listbox)</li>
    kontext "Autopilotletter"    
    BusinessStyle.select( 2 )
    sleep( 2 )

    '///+<li>do not accept to overwrite the default template</li>
    kontext "active"
    if ( active.exists( 1 ) ) then
        warnlog( "   * refusing to change the default template" )
        active.cancel()
    endif
    sleep( 3 )

    '///+<li>check to add a letterhead</li>
    kontext "AutopilotLetter"
    LetterHead.Check()

    '///+<li>click next to get to the letterhead-page</li>
    hClickNextButton()

    '///+<li>Page 2</li>
    kontext "AutopilotLetter"
    call dialogtest2( AutopilotLetter, 2 )
    
    '///+<li>check to add a logo</li>
    printlog( "   * check to add logo" )
    Logo.check()
    
    '///+<li>change all metrics for the logo</li>
    printlog( "   * change the metrics of the logo" )
    LogoHeight.settext( "4" )
    LogoWidth.settext( "4" )
    LogoXpos.settext( "1" )
    LogoYpos.settext( "1" )
    
    '///+<li>check to add own address</li>
    printlog( "   * check to add own address" )
    ReturnAddress.check()
    
    '///+<li>change the metrics of the addressfield</li>
    printlog( "   * change the metrics of the address-field" )
    AddressHeight.settext( "4" )
    AddressWidth.settext( "4" )
    AddressXpos.settext( "1" )
    AddressYpos.settext( "1" )
    
    '///+<li>check to show the return-address in envelope window</li>
    printlog( "   * check to show return-address in envelope window" )
    ReturnAddressCompany.check()
    
    '///+<li>make sure the footer checkbox is *not* selected</li>
    printlog( "   * UNcheck to add a footer" )
    IncludeFooter.unCheck()
    
    '///+<li>The footer-height box should be disabled at this point</li>
    printlog( "   * change the height" )
    if ( FooterHeight.isEnabled() ) then
        warnlog( "FooterHeight should not be enabled in this configuration" )
    endif
    
    hClickNextButton()

    '///+<li>Page 3</li>
    kontext "AutopilotLetter"
    call dialogtest2( AutopilotLetter, 3 )    
    
    '///+<li>set letterpage norm to the second entry</li>
    printlog( "   * change letterformat to the second entry" )
    LetterPageNorm.select( 2 )
    
    '///+<li>change the salutation to the second entry</li>
    printlog( "   * change salutation to the second entry" )
    ListSalutation.select( 2 )
    
    '///+<li>change complimentary close to second entry</li>
    printlog( "   * change Complimentary Close to second entry" )
    ListGreetings.select( 2 )
    hClickNextButton()

    '///+<li>Page 4</li>
    kontext "AutopilotLetter"
    call dialogtest2( AutopilotLetter, 4 )
    hClickNextButton()

    '///+<li>Page 5</li>
    kontext "AutopilotLetter"
    call dialogtest2( AutopilotLetter, 5 )    
    
    '///+<li>insert some text into the footer-field</li>
    printlog( "   * enter some text as footer" )
    TextFooter.settext( "Some non-offending text" )
    
    '///+<li>check to include it on any second page</li>
    printlog( "   * check Include only on second ..." )
    IncludeFromPage2.check()
    hClickNextButton()

    '///+<li>Page 6</li>
    kontext "AutopilotLetter"
    call dialogtest2( AutopilotLetter, 6 )   
    
    '///+<li>name the template for further usage</li>
    printlog( "   * name the template for further usage" )
    TemplateName.setText( cTemplateName )
    hSetTemplateSavePath( cTemplatePath )
    
    '///+<li>click the 'finish' button</li>
    hFinishWizard( 1 )

    if ( gOOo ) then
        kontext "UseOfThisTemplate"
        if ( UseOfThisTemplate.exists( 2 ) ) then
            printlog( "cancelling UseOfThisTemplate-dialog for OOo" )
            UseOfThisTemplate.cancel()
        else
            qaerrorlog( "UseOfThisTemplate-dialog not present" )
        endif
    endif
    
    '///+<li>Check for File-Save error</li>
    iErr = hHandleSaveError()
    if ( iErr = 1 ) then
        kontext "AutopilotLetter"
        hFinishWizard( 1 )
    endif
    
    '///+<li>close the document</li>     
    brc = hDestroyDocument()
    if ( not brc ) then
        qaerrorlog( "#i59233# The wizard does not display the new template" )
    endif   
    
    '///+<li>Delete the new template</li>
    hDeleteFile( cTemplatePath )
    
    irc = hDeleteUserTemplates()
    if ( irc <> 0 ) then
        printlog( "Unexpectedly deleted user template(s), please check")
    endif
    
    do while( getDocumentCount() > 0 )
        call hCloseDocument()
    loop
    
    '///</ul>

endcase


