'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: wizard_mailmerge.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: rt $ $Date: 2008-09-04 09:17:13 $
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
'*  short description : Update test for the mailmerge wizard
'*
'\******************************************************************************

private CSV_DATABASE as string

testcase tUpdtWizardMailMerge

    dim brc as boolean
    dim irc as integer
    
    qaerrorlog( "#i54524# - MM-Wizard modifies datasource -> CVS merge conflict" )
    
    CSV_DATABASE = hGetWorkPath() & "myDatabase"
    hDeleteFile( CSV_DATABASE )
    hRemoveDatabaseConnections()

    '///<H1>Update test for Mailmerge Wizard</H1>
    '///<ul>
    
    '///+<li>Make sure exactly one single writer document is open</li>
    hInitSingleDoc()
    
    '///<li>Open the Mailmerge-Wizard</li>
    printlog( "Open the Mailmerge-Wizard" )
    ToolsMailMergeWizard
    
    '///<ol>
    '///<li>Page 1</li>
    Kontext "MailMergeWizard"
    brc = hUpdtMMWpage1()
    if ( not brc ) then
        warnlog( "MailMergeWizard is not visible, aborting test" )
        goto endsub
    endif
    
    '///<li>Page 2</li>
    Kontext "MailMergeWizard"
    brc = hClickNextButton()
    hUpdtMMWPage2()
    
    '///<li>Page 3</li>
    Kontext "MailMergeWizard"
    brc = hClickNextButton()
	hUpdtMMWpage3()
    
    '///<li>Page 4</li>
    Kontext "MailMergeWizard"
    brc = hClickNextButton()
    hUpdtMMWPage4()
    
    '///<li>Page 5</li>
    Kontext "MailMergeWizard"
    brc = hClickNextButton()
    hUpdtMMWPage5()

    '///<li>Page 6</li>
    Kontext "MailMergeWizard"
    brc = hClickNextButton()
    hUpdtMMWPage6()
    
    '///<li>Page 7</li>
    Kontext "MailMergeWizard"
    brc = hClickNextButton()
    hUpdtMMWPage7()
    
    '///<li>Page 8</li>
    Kontext "MailMergeWizard"
    brc = hClickNextButton()
    hUpdtMMWPage8()
    
    '///</ol>
    '///<li>close the wizard</li>
    printlog( "close the wizard" )
    Kontext "MailMergeWizard"
    MailMergeWizard.cancel()

    hRemoveDatabaseConnections()
    hCloseDocument()
    hCloseDocument()
    '///</ul>
    

endcase

'**********************************************************************

function hUpdtMMWpage1() as boolean

    '///<h3>Update test for Mailmerge Wizard - page 1</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorstatus (boolean)</li>
    '///<ul>
    '///+<li>Do not use</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    

    const CFN = "hUpdtMMWpage1::"
    
    '///+<li>Verify that the tabpage is open</li>
    Kontext "TabMailMergeSourceDocument"
    if ( not TabMailMergeSourceDocument.exists() ) then
        warnlog( CFN & "Page 1 could not be accessed" )
        hUpdtMMWPage1() = false
        exit function
    endif

    '///+<li>Click on Browse Document to open FileOpen</li>
    Kontext "TabMailMergeSourceDocument"
    if ( BrowseDocument.exists( 1 ) ) then
        if ( BrowseDocument.isEnabled ) then
            BrowseDocument.click()
            
            '///+<li>Cancel FileOpen dialog</li>
            Kontext "OeffnenDlg"
            if ( OeffnenDlg.exists( 1 ) ) then
                printlog( CFN & "BrowseDocument::File Open Dialog" )
                call dialogtest( OeffnenDlg )
                OeffnenDlg.cancel()
            else
                warnlog( CFN & "BrowseDocument::FileOpen not open" )
            endif
        else
            warnlog( CFN & "BrowseDocument::Button is disabled" )
       endif
    else
        warnlog( CFN & "BrowseDocument::Button does not exist" )
    endif
    
    '///+<li>Click on Browse Template to open the New...-dialog</li>
    Kontext "TabMailMergeSourceDocument"
    if ( BrowseTemplate.exists( 2 ) ) then
        if ( BrowseTemplate.isEnabled ) then
            BrowseTemplate.click()
            
            '///+<li>Cancel the New...-dialog</li>
            Kontext "Neu"
            if ( Neu.exists( 1 ) ) then
                printlog( CFN & "BrowseTemplate::New Dialog" )
                call dialogtest( Neu )
                Neu.cancel()
            else
                warnlog( CFN & "BrowseTemplate::FileOpen not open" )
            endif
        else
            warnlog( CFN & "BrowseTemplate::Button is disabled" )
        endif
    else
        warnlog( CFN & "BrowseTemplate::Button does not exist" )
    endif
        
    '///+<li>Check to create a new document</li>
    kontext "TabMailMergeSourceDocument"
    if ( CreateANewDocument.exists( 2 ) ) then
        if ( CreateANewDocument.isEnabled ) then
            printlog( CFN & "Check Create new Document" )
            CreateANewDocument.check()
        else
            warnlog( CFN & "CheckBbox is disabled" )
        endif
    else
        warnlog( CFN & "CheckBox does not exist" )
    endif

    hUpdtMMWPage1() = true
    '///</ul>

end function

'**********************************************************************

function hUpdtMMWpage2()

    '///<h3>Update test for Mailmerge Wizard - page 2</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    const CFN = "hUpdtMMWpage2::"

    '///+<li>Verify that the tabpage is open</li>
    Kontext "TabMailMergeDocumentType"
    if ( not TabMailMergeDocumentType.exists( 2 ) ) then
        warnlog( CFN & "Page 2 could not be accessed" )
        exit function
    endif
    
    call dialogtest( TabMailMergeDocumentType )
    
    '///+<li>Check the Letter-checkbox</li>
    printlog( CFN & "Select to create a letter" )
    if ( Letter.exists() ) then
        if ( Letter.isEnabled() ) then
            Letter.check()
        else
            warnlog( CFN & "Cannot check Letter-Checkbox" )
        endif
    else
        warnlog( CFN & "Letter Checkbox does not exist" )
    endif
    '///</ul>
    
end function   

'*******************************************************************************

function hUpdtMMWpage3()

    '///<h3>Update test for Mailmerge Wizard - page 3</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    const CFN = "hUpdtMMWpage3::"
    
    dim cDatabase as string
        cDatabase = gTesttoolpath & "framework\required\input\mailmerge_data.odb"
        cDatabase = convertpath( cDatabase )
    
    '///+<li>Verify that the tabpage is open</li>
    Kontext "TabMailMergeAddressBlock"
    if ( not TabMailMergeAddressBlock.exists( 2 ) ) then
        warnlog( CFN & "Page 3 could not be accessed" )
        exit function
    endif
    
    call Dialogtest( TabMailMergeAddressBlock )
    
    '///+<li>Click &quot;different addresslist&quot; button and open all subsequent dialogs</li>
    Kontext "TabMailMergeAddressBlock"
    hSelectDifferentAddressList( cDatabase )
    
    '///+<li>Click more button</li>
    Kontext "TabMailMergeAddressBlock"
    if ( AddressBlock.exists( 2 ) ) then

        
	if( not AddressBlock.isChecked() ) then
            warnlog( CFN & "AddressBlock Checkbox not checked" )
            AddressBlock.check()
        endif

	hSelectAddressBlock()
    
        '///+<li>Click match fields button</li>
    	'///+<li>Assign any value to all of the listboxes</li>
        hUpdtMailmergeMatchFields()
    else
        warnlog( CFN & "AddressBlock-Checkbox missing, skipping dialogs" )
    endif
    '///</ul>
    
    
end function

'*******************************************************************************

function hUpdtMMWpage4()

    '///<h3>Update test for Mailmerge Wizard - page 4</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    const CFN = "hUpdtMMWpage4::"
    dim brc as boolean
   
    brc = false

    '///+<li>Verify that the tabpage is open</li>
    Kontext "TabMailMergeGreetingsPage"
    if ( not TabMailMergeGreetingsPage.exists( 2 ) ) then
        warnlog( CFN & "Page 4 could not be accessed" )
        exit function
    endif
    
    call dialogtest( TabMailMergeGreetingsPage )
    
    ' the salutation checkbox must be checked, otherwise the buttons are not 
    ' available
    '///+<li>Verify that the Checkbox for Salutation is chaecked</li>
    if ( not GreetingPersonalized.isEnabled() ) then
        warnlog( CFN & "CheckBox Salutation not in HID.LST, it should be checked" )
        exit function
    endif
    
    '///+<li>Edit Addressblock for female salutation (GreetingsbuttonFemale)</li>
    Kontext "TabMailMergeGreetingsPage"
    printlog( CFN & "GreetingButtonFemale" )
    if ( GreetingButtonFemale.isEnabled() ) then
        hNewEditAddressBlock( 3 )
    else
        warnlog( CFN & "GreetingButtonFemale is not enabled" )
    endif

    '///+<li>Edit Addressblock for female salutation (GreetingsbuttonMale)</li>
    Kontext "TabMailMergeGreetingsPage"
    printlog( CFN & "GreetingButtonMale" )
    if ( GreetingButtonMale.isEnabled() ) then
        hNewEditAddressBlock( 4 )
    else
        warnlog( CFN & "GreetingButtonMale is not enabled" )
    endif

    '///+<li>Click the &quot;Match Fields&quot;-button</li>
    Kontext "TabMailMergeGreetingsPage"
    '///</ul>

end function


'**************************************************************************

function hUpdtMMWpage5()

    '///<h3>Update test for Mailmerge Wizard - page 5</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    const CFN = "hUpdtMMWpage5::"
    
    '///+<li>Verify that the tabpage is open</li>
    kontext "TabMailMergeLayoutPage"
    if ( not TabMailMergeLayoutPage.exists( 2 ) ) then
        warnlog( CFN & "Page 5 could not be accessed" )
        exit function
    endif
    
    '///+<li>Change nothing, go to next page directly</li>
    call DialogTest( TabMailMergeLayoutPage )
    
    printlog( CFN )
    '///</ul>
    
end function

'**************************************************************************

function hUpdtMMWpage6()

    '///<h3>Update test for Mailmerge Wizard - page 6</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    const CFN = "hUpdtMMWpage6::"
    
    printlog( CFN )
    
    '///+<li>Verify that the tabpage is open</li>
    kontext "TabMailMergePrepare"
    if ( not TabMailMergePrepare.exists( 2 ) ) then
        warnlog( CFN & "Page 6 could not be accessed" )
        exit function
    endif
    
    call DialogTest( TabMailMergePrepare )
    
    '///+<li>Click the &quot;Edit&quot;-button</li>
    qaerrorlog( "Skipping preview button" )
    '///</ul>
    
end function

'**************************************************************************

function hUpdtMMWpage7()

    '///<h3>Update test for Mailmerge Wizard - page 7</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hUpdtMMWpage7::"
    
    printlog( CFN )    
    
    '///+<li>Verify that the tabpage is open</li>
    kontext "TabMailMergePersonalize"
    hWaitForObject( EditIndividualButton, 8000 )
    'if ( not TabMailMergePersonalize.exists( 2 ) ) then
    '    warnlog( CFN & "Page 7 could not be accessed" )
    '    exit function
    'endif
    
    call DialogTest( TabMailMergePersonalize )
    
    '///+<li>Click on &quot;EditIndividual&quot;-button</li>
    qaerrorlog( "Skipping Preview-Button" )
    '///</ul>
    
end function

'**************************************************************************

function hUpdtMMWpage8()

    '///<h3>Update test for Mailmerge Wizard - page 8</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    const CFN = "hUpdtMMWpage8::"
    
    '///+<li>Verify that the tabpage is open</li>
    kontext "TabMailMergeOutputPage"
    if ( not TabMailMergeOutputPage.exists( 2 ) ) then
        warnlog( CFN & "Page 8 could not be accessed" )
        exit function
    endif
    
    printlog( CFN )
    
    call DialogTest( TabMailMergeOutputPage )
    
    '///+<li>Check Save Document, go throught subsequent dialogs (if any)</li>
    kontext "TabMailMergeOutputPage"
    hSaveStartingDocument()
    
    '///+<li>Check Save Mailmerge Document, go throught subsequent dialogs (if any)</li>
    kontext "TabMailMergeOutputPage"
    hSaveMailMergeDocument()
    
    '///+<li>Check Print, go throught subsequent dialogs (if any)</li>
    kontext "TabMailMergeOutputPage"
    hPrintMailMergeDocument()
    
    '///+<li>Check Send, go throught subsequent dialogs (if any)</li>
    kontext "TabMailMergeOutputPage"
    hSendMailMergeDocument()
    '///</ul>
    
end function

'*******************************************************************************

function hSelectDifferentAddressList( cDatabase as string ) as boolean

    '///<h3>Update test for &quot;Select Address List&quot; dialog</h3>
    '///<i>Starting point: Page three of the Mail Merge Wizard</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Path/Name of the database (string)</li>
    '///<ul>
    '///+<li>Valid, readable database</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE if all is ok</li>
    '///+<li>FALSE on any other condition</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    const CFN = "hSelectDifferentAddressList::"
    
    dim brc as boolean
        
    dim iAddressList as integer
    dim cAddressList as string
        
    '///+<li>Click the &quot;Select Address List...&quot; button</li>        
    Kontext "TabMailMergeAddressBlock"
    SelectAddressListButton.click()

    '///+<li>Verify that the dialog is open</li>
    Kontext "SelectAddressList"
    if ( SelectAddressList.exists( 2 ) ) then
        printlog( CFN & "Select Address List Dialog" )
        call dialogtest( SelectAddressList )
    else
        warnlog( CFN & "SelectAddressList dialog is not open" )
        hSelectDifferentAddressList() = false
        exit function
    endif
	    
	'///+<li>Add the named database</li>
    Kontext "SelectAddressList"
    brc = hAddDatabase( cDatabase ) 

    
    '///+<li>Make sure we have at least two databases to choose from</li>
    Kontext "SelectAddressList"
    if ( SelectAddressList.exists( 2 ) ) then
        if ( AddressList.getItemCount() < 2 ) then
            brc = hAddDatabase( cDatabase )
        endif
        
        '///+<li>Create another address list, return</li>
        Kontext "SelectAddressList"
        brc = hEditCreateAddressList( 1 )
	
        '///+<li>Open the filter dialog, work with it and return</li>
        Kontext "SelectAddressList" 
        brc = hFilterDialog()

        '///+<li>Edit the address list, return</li>
        Kontext "SelectAddressList"
        brc = hEditCreateAddressList( 2 )
	
        '///+<li>Verify that the ok button is enabled for all address lists</li>
        Kontext "SelectAddressList"
        for iAddressList = 1 to AddressList.getItemCount()
        
            AddressList.select( iAddressList )
            cAddressList = AddressList.getText()
            printlog( CFN & "Current address list: " & cAddressList )
            
            try
                SelectAddressList.ok()
                brc = true
                exit for
            catch
                qaerrorlog( CFN & "Ok-Button disabled for <" & cAddressList & ">" )
                brc = false
            endcatch
            
        next iAddressList
        
        if ( not brc ) then
            warnlog( "#i84250# OK button not enabled for any address list." )
        endif        
    else
        warnlog( CFN & "Select Address List dialog is not open" )
        brc = false
    endif
    

    
    Kontext "TabMailMergeAddressBlock"
    hSelectDifferentAddressList() = brc
    '///</ul>

end function

'*******************************************************************************

function hWaitForBackToWizardFloat() as boolean

    '///<h3>Update test: Try to close the BackToWizard float</h3>
    '///<i>Starting point: Edit individual document from Mail Merge Wizard
    '///+ Back to wizard float must be visible</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE if all is ok</li>
    '///+<li>FALSE on any other condition</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    const CFN = "hWaitForBackToWizardFloat::"
    dim brc as boolean
    
    printlog( CFN & "Enter" )
           
    '///+<li>Try to grab the float (it is currently an "active"</li>
    Kontext "MailMergeFloat"
    if ( MailMergeFloat.exists( 20 ) ) then
        printlog( CFN & "Back to document float is open" )
        ReturnToMailMergeWizard.click()
        
        '///+<li>Verify that we are back to the wizard (might take some time)</li>
        Kontext "MailMergeWizard"
        if ( MailMergeWizard.exists( 5 ) ) then
            printlog( CFN & "Successfully returned to Mail Merge Wizard" )
            brc = true
        else
            warnlog( CFN & "Could not return to Mail Merge Wizard" )
            brc = false
        endif
    else
        warnlog( CFN & "Unable to access Mail Merge Float" )
        brc = false
    endif
    
    printlog( CFN & "Exit with result = " & brc )
    hWaitForBackToWizardFloat() = brc
    '///</ul>
        
end function

'*******************************************************************************

function hAddDatabase( cDatabase as string ) as boolean

    '///<h3>Update test for &quot;Add Database&quot; dialog</h3>
    '///<i>Starting point: Select Address List dialog</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Path to database to be added (string)</li>
    '///<ul>
    '///+<li>Database must be valid</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE if all is ok</li>
    '///+<li>FALSE on any other condition</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hAddDatabase::"
    
    dim brc as boolean
        brc = false
        
    dim iWait as integer
    
    '///+<li>Click &quot;Add...&quot;</li>
    add.click()

    '///+<li>File Open dialog opens, enter filename</li>
    Kontext "OeffnenDlg"
    if ( OeffnenDlg.exists( 2 ) ) then
        printlog( CFN & "File Open Dialog" )
        DateiName.setText( cDatabase )
        Oeffnen.click()
        brc = true
    else
        warnlog( CFN & "FileOpen is missing" )
    endif
	
    ' When a new addressdatabase is imported it might take some time before we
    ' get back to the SelectAddressList dialog
    '///+<li>Wait for the dialog to populate (might take an instant)</li>
    hWaitForSelectAddressListDialog()
    
    hAddDatabase() = brc
    '///</ul>
    
end function

'******************************************************************************

function hEditCreateAddressList( iMode as integer ) as boolean

    const CFN = "hEditCreateAddressList::"

    dim brc as boolean
        brc = false
        
    dim iWait as integer
        
    Kontext "SelectAddressList"
    select case iMode
    case 1 : create.click()
    case 2 : edit.click()
    end select

    Kontext "NewAddressList"
    if ( NewAddressList.exists( 2 ) ) then
        printlog( CFN & "New/Edit Address List Dialog is open" )
        call dialogtest( NewAddressList )
        brc = true
    else
        warnlog( CFN & "New/EditAddressList does not exist" )
    endif

    if ( not brc ) then
        hEditCreateAddressList() = brc
        exit function
    endif
        

    ' Try to use the Find-dialog, do not evaluate the returncode,
    ' the test should continue even if the dialog does not come up.
    brc = hFindDialog()
    
    ' Try to use the Customize-dialog, don't evaluate the rc either
    brc = hCustomizeAddressList()
    
    ' Return to "NewAddressList" 
    Kontext "NewAddressList"
    NewAddressList.ok()
    
    ' only save when a new list has been created, editing saves automatically
    if ( iMode = 1 ) then
        hSaveNewAddressList()
    endif
    
    hWaitForSelectAddressListDialog()
    
    Kontext "SelectAddressList"
    hEditCreateAddressList() = brc
	
end function

'*******************************************************************************

function hFindDialog() as boolean

    '///<h3>Update test for Mail Merge Wizard Find dialog</h3>
    '///<i>Starting point: Select Address List dialog</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE if all is ok</li>
    '///+<li>FALSE on any other condition</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    const CFN = "hFindDialog::"

    dim brc as boolean
        brc = false
        
    printlog( CFN )

    Kontext "NewAddressList"
    find.click()

    kontext "FindEntry"
    if ( FindEntry.exists( 2 ) ) then
        call dialogtest( FindEntry )
        printlog( CFN & "Find Entry Dialog is open" )
        FindEntry.cancel()
        brc = true
    else
        warnlog( CFN & "Dialog is not open" )
    endif
	
    Kontext "NewAddressList"
    hFindDialog() = brc
    '///</ul>
	
end function

'******************************************************************************

function hCustomizeAddressList() as boolean

    '///<h3>Update test for Mail Merge Wizard Customize Address List dialog</h3>
    '///<i>Starting point: New Address List dialog</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE on success</li>
    '///+<li>FALSE on failure</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>    


    const CFN = "hCustomizeAddressList::"

    dim brc as boolean
        brc = false
        
    printlog( CFN )

    Kontext "NewAddressList"
    if ( Customize.exists( 2 ) ) then
        if ( Customize.isEnabled() ) then
            Customize.click()

            kontext "CustomizeAddressList"
            if ( CustomizeAddressList.exists( 2 ) ) then
                printlog( CFN & "Customize Address List Dialog" )
                call dialogtest( CustomizeAddressList )
		
                kontext "CustomizeAddressList"
                hAddRenameElement( "NewElement" , 1 ) 
                
                kontext "CustomizeAddressList"
                hAddRenameElement( "Renamed" , 2 )
                
                kontext "CustomizeAddressList"
                printlog( CFN & "Delete" )
                delete.click()
                
                kontext "CustomizeAddressList"
                CustomizeAddressList.cancel()
                brc = true
            else
                warnlog( CFN & "Dialog is not open" )
            endif
        else
            warnlog( CFN & "Button is not enabled" )
        endif
    else
        warnlog( CFN & "Button does not exist" )
    endif
        
    Kontext "NewAddressList"
    hCustomizeAddressList() = brc
    '///</ul>
	
end function

'************************************************************************************

function hAddRenameElement( cElementName as string , iMode as integer ) as boolean

    const CFN = "hAddRenameElement::"
    dim brc as boolean
        brc = false

    select case iMode
    case 1 : Add.click()
    case 2 : Rename.click()
    end select
    
    Kontext "RenameElement"
    if ( RenameElement.exists( 2 ) ) then
        printlog( CFN & "Add/Rename Element Dialog" )
        call Dialogtest( RenameElement )
        
        FieldTitle.setText( cElementName )
        RenameElement.ok()
        brc = true
    else
        warnlog( CFN & "Dialog is not open" )
    endif
       	
    hAddRenameElement() = brc
	
end function

'*******************************************************************************

function hFilterDialog() as integer

    '///<h3>Update test for Mail Merge Wizard Filter dialog</h3>
    '///<i>Starting point: Select Address List dialog</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE on success</li>
    '///+<li>FALSE on failure</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>    

    const CFN = "hFilterDialog::"
    
    dim brc as boolean
        brc = false
        
    Kontext "SelectAddressList"
    if ( Filter.exists( 2 ) ) then
        if ( Filter.isEnabled() ) then
            Filter.click()
            
            kontext "FilterDialog"
            if ( FilterDialog.exists( 2 ) ) then
                printlog( CFN & "Filter Dialog" )
                call Dialogtest( FilterDialog )
                
                FilterDialog.cancel()
                brc = true
            else
                warnlog( "#i54150 " & CFN & "Filter dialog is not open" )
            endif
        else
            warnlog( CFN & "Filter Button is not enabled" )
        endif
    else
        warnlog( CFN & "Filter Button is missing" )
    endif
    
    hWaitForSelectAddressListDialog()
    hFilterDialog() = brc
    '///</ul>
  
end function

'****************************************************************************

function hSelectAddressBlock() as boolean

    const CFN = "hSelectAddressBlock::"
    
    dim brc as boolean
        brc = false
        
    printlog( CFN  & "More..." )
        
    Kontext "TabMailMergeAddressBlock"
    if ( more.exists( 2 ) ) then
    
        ' A dynamic delay (function borrowed from toolbar_tools.inc)
        hWaitForObject( more , 3000 )
        
        if ( more.isEnabled() ) then
            more.click()
            
            Kontext "SelectAddressBlock"
            if ( SelectAddressBlock.exists( 2 ) ) then
                printlog( CFN & "SelectAddressBlock dialog is open" )
                call DialogTest( SelectAddressBlock )
                
                Kontext "SelectAddressBlock"
                hNewEditAddressBlock( 1 )
                
                Kontext "SelectAddressBlock"
                hNewEditAddressBlock( 2 )
                
                Kontext "SelectAddressBlock"
                SelectAddressBlock.ok()
                brc = true
            else
                warnlog( CFN & "Dialog is not open" )
            endif        
        else
            warnlog( CFN & "Button is disabled" )
        endif
    else
        warnlog( CFN & "Button does not exist" )
    endif
        
    hSelectAddressBlock() = brc
    
end function

'*******************************************************************************

function hNewEditAddressBlock( iMode as integer ) as boolean

    const CFN = "hNewEditAddressBlock::"

    select case iMode
    case 1 : NewBtn.click()
    case 2 : Customize.click()
    case 3 : GreetingButtonFemale.click()
    case 4 : GreetingButtonMale.click()
    end select
    
    Kontext "NewAddressBlock"
    if ( NewAddressBlock.exists( 2 ) ) then
        printlog( cfn & "Dialog is open" )
        NewAddressBlock.cancel()
        hNewEditAddressBlock() = true
    else
        warnlog( cfn & "Dialog is not open" )
        hNewEditAddressBlock() = false
    endif
        
end function

'*******************************************************************************

function hSaveStartingDocument() as boolean

    const CFN = "hSaveStartingDocument::"
    
    dim brc as boolean
        brc = false

    if ( RBSaveStartDoc.exists( 2 ) ) then
         if ( RBSaveStartDoc.isEnabled ) then
             RBSaveStartDoc.check()
             brc = true
         else
             warnlog( CFN & "Cannot set radiobutton" )
         endif
    else
        warnlog( CFN & "Radiobutton is missing" )
    endif
    
    ' save file
    if ( PBSaveStartDoc.exists( 2 ) ) then
        if ( PBSaveStartDoc.isEnabled ) then
            PBSaveStartDoc.click()
             
            Kontext "SpeichernDlg"
            if ( SpeichernDlg.exists( 2 ) ) then
                printlog( CFN & "Save As Dialog is open" )
                SpeichernDlg.cancel()
                brc = true
            else
                warnlog( CFN & "FileSave dialog is missing" )
            endif
        else
            warnlog( CFN & "SaveStartDoc is disabled" )
        endif
    else
        warnlog( CFN & "SaveStartDoc is missing" )
    endif            
    
    hSaveStartingDocument() = brc
    
end function

'******************************************************************************* 

function hSaveMailMergeDocument() as boolean

    const CFN = "hSaveMailMergeDocument::"
    
    dim brc as boolean
        brc = false

    if ( RBSaveMergedDoc.exists( 2 ) ) then
         if ( RBSaveMergedDoc.isEnabled() ) then
             RBSaveMergedDoc.check()
             brc = true
         else
             warnlog( CFN & "Cannot set radiobutton" )
         endif
    else
        warnlog( CFN & "Radiobutton is missing" )
    endif
    
    ' save file
    if ( PBSaveNow.exists( 2 ) ) then
        if ( PBSaveNow.isEnabled() ) then
            PBSaveNow.click()
             
            Kontext "SpeichernDlg"
            if ( SpeichernDlg.exists( 2 ) ) then
                printlog( CFN & "Save As Dialog is open" )
                SpeichernDlg.cancel()
                brc = true
            else
                warnlog( CFN & "FileSave dialog is missing" )
            endif
        else
            warnlog( CFN & "Save Now is disabled" )
        endif
    else
        warnlog( CFN & "Save Now is missing" )
    endif            
    
    hSaveMailMergeDocument() = brc
    
end function

'******************************************************************************* 

function hPrintMailMergeDocument() as boolean

    const CFN = "hPrintMailMergeDocument::"
    
    dim brc as boolean
        brc = false

    if ( RBPrintMerged.exists( 2 ) ) then
         if ( RBPrintMerged.isEnabled() ) then
             RBPrintMerged.check()
             brc = true
         else
             warnlog( CFN & "Cannot set radiobutton" )
         endif
    else
        warnlog( CFN & "Radiobutton is missing" )
    endif

    if ( instr( gtSYSName , "win" ) > 0 ) then
        printlog( CFN & "Skipping printer dialog on windows" )
        hPrintMailMergeDocument() = true
        exit function
    endif
    
    if ( lcase( gPlatform ) = "osx" ) then
        qaerrorlog( "#i81545# No printer settings on MacOS X" )
        hPrintMailMergeDocument() = true
        exit function
    endif
    
    ' printer settings
    if ( PBPrinterSettings.exists( 2 ) ) then
        if ( PBPrinterSettings.isEnabled() ) then
            PBPrinterSettings.click()
             
            Kontext "TabSPAPaper"
            if ( TabSPAPaper.exists() ) then
                printlog( CFN & "Printer Settings Dialog" )
                TabSPAPaper.OK
                brc = true
            else
                warnlog( "#i89114# Printer Settings dialog is missing" )
            endif
        else
            warnlog( CFN & "Printer Settings is disabled" )
        endif
    else
        warnlog( CFN & "Printer Settings is missing" )
    endif   
    
    hPrintMailMergeDocument() = brc
    
end function

'******************************************************************************* 

function hSendMailMergeDocument() as boolean

    const CFN = "hSendMailMergeDocument::"
    
    dim brc as boolean
        brc = false

    if ( RBSendAsMail.exists( 2 ) ) then
         if ( RBSendAsMail.isEnabled() ) then
             RBSendAsMail.check()
         else
             warnlog( CFN & "Cannot set radiobutton" )
         endif
    else
        warnlog( CFN & "Radiobutton is missing" )
    endif
    
    ' send copy to
    if ( PBCopyTo.exists( 2 ) ) then
        if ( PBCopyTo.isEnabled() ) then
            PBCopyTo.click()
             
            Kontext "CopyToDialog"
            if ( CopyToDialog.exists( 2 ) ) then
                printlog( CFN & "CopyToDialog is open" )
                call DialogTest( CopyToDialog )
                CopyToDialog.OK()
            else
                warnlog( CFN & "CopyToDialog dialog is missing" )
            endif
        else
            warnlog( CFN & "PBCopyTo is disabled" )
        endif
    else
        warnlog( CFN & "PBCopyTo is missing" )
    endif   
    
    kontext "TabMailMergeOutputPage"
    if ( PBSendDocuments.exists( 2 ) ) then
        if ( PBSendDocuments.isEnabled() ) then
            PBSendDocuments.click()
            Kontext "Active"
            if ( active.exists( 2 ) ) then
                printlog( CFN & "Dialog asking for EMail-account is open" )
                call Dialogtest( active )
                active.yes()
                
                kontext "TabMailMergeEmail"
                hMailMergeEmailDialog()
            else
                warnlog( CFN & "Dialog asking for EMail-account is missing" )
            endif
        else
            warnlog( CFN & "Send documents Button is disabled" )
        endif
    else
        warnlog( CFN & "Send Documents Button is missing" )
    endif
    
    hSendMailMergeDocument() = brc
    
end function

'******************************************************************************* 

function hMailMergeEmailDialog() as boolean

    const CFN = "hMailMergeEmailDialog::"
    dim brc as boolean
        brc = false
    
    if ( TabMailMergeEmail.exists( 2 ) ) then
        printlog( CFN & "EMail Account Dialog is open" )
        brc = true
    else
        warnlog( CFN & "EMail Account Dialog is not open" )
        exit function
    endif
    
    ' server authentication
    if ( ServerAuthentication.exists( 2 ) ) then
        if ( ServerAuthentication.isEnabled() ) then
            ServerAuthentication.click()
            
            Kontext "ServerAuthentication"
            if ( ServerAuthentication.exists( 2 ) ) then
                printlog( CFN & "Server Authentication dialog is open" )
                call DialogTest( ServerAuthentication )
                ServerAuthentication.cancel()
            else
                warnlog( CFN & "ServerAuthentication is not open" )
            endif
        else
            warnlog( CFN & "ServerAuthentication-Button is not enabled" )
        endif
    else
        warnlog( CFN & "ServerAuthentication-Button does not exist" )
        brc = false
    endif
    
    ' Test settings
    kontext "TabMailMergeEmail"   
    if ( TestSettings.exists( 2 ) ) then
        if ( TestSettings.isEnabled() ) then
            TestSettings.click()
            
            kontext "TestAccountSettings"
            if ( TestAccountSettings.exists( 2 ) ) then
                printlog( CFN & "Test Account Settings Dialog is open" )
                call DialogTest( TestAccountSettings )
                TestAccountSettings.cancel()
            else
                warnlog( CFN & "Test Account Settings Dialog is not open" )
            endif
        else
            warnlog( CFN & "Test Settings-button is disabled" )
        endif
    else
        warnlog( CFN & "Test Settings-button is missing" )
    endif
    
    kontext "TabMailMergeEmail"
    TabMailMergeEmail.cancel()
    
    hMailMergeEmailDialog() = brc
    
end function

'*******************************************************************************

function hWaitForSelectAddressListDialog() as boolean

    Kontext "SelectAddressList"
    if ( SelectAddressList.exists( 5 ) ) then
        hWaitForSelectAddressListDialog() = true
    else
        hWaitForSelectAddressListDialog() = false
    endif
    
    
end function

'*******************************************************************************

function hSaveNewAddressList()

    const CFN = "hSaveNewAddressList::"

    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 2 ) ) then
    
        DateiName.setText( CSV_DATABASE )
        Speichern.click()
        
        kontext "active"
        if ( active.exists( 2 ) ) then
            warnlog( active.getText() ) 
            active.yes()
        endif
        
        printlog( CFN & "File saved" )
    else
        warnlog( CFN & "FileSave did not come up" )
    endif
    
end function

'*******************************************************************************

function hRemoveDatabaseConnections() as integer

    const CFN = "hRemoveDatabaseConnections()::"
    const DEFAULT_DATABASE = "Bibliography"
    
    dim iConnectionCount as integer
    dim iCurrentConnection as integer
    dim cConnectionName as string
    dim bOptionsOpen as boolean : bOptionsOpen = true
    
    kontext "OptionenDlg"
    if ( not OptionenDlg.exists() ) then
        bOptionsOpen = false
        ToolsOptions
    endif
    
    hToolsOptions( "DataSources", "Databases" )
    
    kontext "TabRegisteredDatabase"
    iConnectionCount = RegisteredDatabases.getItemCount()
    
    for iCurrentConnection = iConnectionCount to 1 step -1
    
        kontext "TabRegisteredDatabase"
        if ( TabRegisteredDatabase.exists( 2 ) ) then
        
            RegisteredDatabases.select( iCurrentConnection )
            cConnectionName = RegisteredDatabases.getSelText()
            printlog( CFN & "Current database: " & cConnectionName )
        
            if ( cConnectionName <> DEFAULT_DATABASE ) then
                DeleteBtn.click()
                kontext "active"
                if ( active.exists( 2 ) ) then
                    active.yes()
                else
                    warnlog( CFN & "Confirmation dialog for delete is missing" )
                endif
            else
                printlog( CFN & "Not deleting default database connection" )
            endif
        else
            warnlog( CFN & "Failed to set context to Registered Databases Tab" )
        endif
        
    next iCurrentConnection
    
    if ( not bOptionsOpen ) then
        kontext "OptionenDlg"
        OptionenDlg.ok()
    endif
    
    hRemoveDatabaseConnections() = iConnectionCount - 1
    
end function

'*******************************************************************************

function hUpdtMailmergeMatchFields() as boolean

    '///<h3>Assign random fields on the Match Fields dialog of the Mailmerge Wizard</h3>
    '///<i>Starting point: Match fields dialog</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE on no error</li>
    '///+<li>FALSE otherwise</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hUpdtMailmergeMatchFields::"
    dim brc as boolean
    
    printlog( CFN & "Accessing Match Fields Dialog" )
    Kontext "TabMailMergeAddressBlock"
    MatchFieldsButton.click()
    
    '///+<li>Verify that the match fields dialog is open</li>
    kontext "MatchFields"
    if ( not MatchFields.exists( 2 ) ) then
        warnlog( CFN & "Unable to access Match Fields dialog" )
        brc = false
    else
    
        '///+<li>Assign randomly one of the 8 values to each of the 14 listboxes</li>
        Title.select( 2 )
        FirstName.select( 3 )
        LastName.select( 4 )
        Company.select( 5 )
        Address1.select( 6 )
        Address2.select( 7 )
        City.select( 8 )
        State.select( 2 )
        ZIP.select( 3 )
        Country.select( 4 )
        PhonePrivate.select( 1 )
        PhoneBusiness.select( 2 )
        EMail.select( 5 )
        Gender.select( 4 )
        
        '///+<li>Return to the Mail Merge Wizard</li>
        MatchFields.OK()
        printlog( CFN & "Leaving Match Fields Dialog" )
        brc = true
        
    endif
    
    hUpdtMailmergeMatchFields() = brc
    '///</ul>
        
end function


