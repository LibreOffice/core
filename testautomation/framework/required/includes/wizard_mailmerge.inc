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
'*  short description : Update test for the mailmerge wizard
'*
'\******************************************************************************

private CSV_DATABASE as string

testcase tUpdtWizardMailMerge

    printlog( "Resource test for the mailmerge wizard" )

    dim brc as boolean
    dim irc as integer
    
    qaerrorlog( "#i54524# - MM-Wizard modifies datasource -> CVS merge conflict" )
    
    CSV_DATABASE = hGetWorkPath() & "myDatabase"
    hDeleteFile( CSV_DATABASE )
    hRemoveDatabaseConnections()

    
    hInitSingleDoc()
    
    printlog( "Open the Mailmerge-Wizard" )
    ToolsMailMergeWizard
    
    Kontext "MailMergeWizard"
    brc = hUpdtMMWpage1()
    if ( not brc ) then
        warnlog( "MailMergeWizard is not visible, aborting test" )
        goto endsub
    endif
    
    Kontext "MailMergeWizard"
    hClickButton( NextButton )
    hUpdtMMWPage2()
    
    Kontext "MailMergeWizard"
    hClickButton( NextButton )
    hUpdtMMWpage3()
    
    Kontext "MailMergeWizard"
    hClickButton( NextButton )
    hUpdtMMWPage4()
    
    Kontext "MailMergeWizard"
    hClickButton( NextButton )
    hUpdtMMWPage5()

    Kontext "MailMergeWizard"
    hClickButton( NextButton )
    hUpdtMMWPage6()
    
    Kontext "MailMergeWizard"
    hClickButton( NextButton )
    hUpdtMMWPage7()
    
    Kontext "MailMergeWizard"
    hClickNextButton()
    hUpdtMMWPage8()
    
    printlog( "close the wizard" )
    Kontext "MailMergeWizard"
    hCloseDialog( MailMergeWizard, "cancel" )

    hRemoveDatabaseConnections()
    hCloseDocument()
    hCloseDocument()
    

endcase

'**********************************************************************

function hUpdtMMWpage1() as boolean

    

    const CFN = "hUpdtMMWpage1::"
    
    Kontext "TabMailMergeSourceDocument"
    if ( not TabMailMergeSourceDocument.exists() ) then
        warnlog( CFN & "Page 1 could not be accessed" )
        hUpdtMMWPage1() = false
        exit function
    endif

    Kontext "TabMailMergeSourceDocument"
    if ( BrowseDocument.exists( 1 ) ) then
        if ( BrowseDocument.isEnabled ) then
            BrowseDocument.click()
            
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
    
    Kontext "TabMailMergeSourceDocument"
    if ( BrowseTemplate.exists( 2 ) ) then
        if ( BrowseTemplate.isEnabled ) then
            BrowseTemplate.click()
            
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

end function

'**********************************************************************

function hUpdtMMWpage2()


    const CFN = "hUpdtMMWpage2::"

    Kontext "TabMailMergeDocumentType"
    if ( not TabMailMergeDocumentType.exists( 2 ) ) then
        warnlog( CFN & "Page 2 could not be accessed" )
        exit function
    endif
    
    call dialogtest( TabMailMergeDocumentType )
    
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
    
end function   

'*******************************************************************************

function hUpdtMMWpage3()


    const CFN = "hUpdtMMWpage3::"
    
    dim cDatabase as string
        cDatabase = gTesttoolpath & "framework\required\input\mailmerge_data.odb"
        cDatabase = convertpath( cDatabase )
    
    Kontext "TabMailMergeAddressBlock"
    if ( not TabMailMergeAddressBlock.exists( 2 ) ) then
        warnlog( CFN & "Page 3 could not be accessed" )
        exit function
    endif
    
    call Dialogtest( TabMailMergeAddressBlock )
    
    Kontext "TabMailMergeAddressBlock"
    hSelectDifferentAddressList( cDatabase )
    
    Kontext "TabMailMergeAddressBlock"
    if ( AddressBlock.exists( 2 ) ) then

        
	if( not AddressBlock.isChecked() ) then
            warnlog( CFN & "AddressBlock Checkbox not checked" )
            AddressBlock.check()
        endif

	hSelectAddressBlock()
    
        hUpdtMailmergeMatchFields()
    else
        warnlog( CFN & "AddressBlock-Checkbox missing, skipping dialogs" )
    endif
    
    
end function

'*******************************************************************************

function hUpdtMMWpage4()


    const CFN = "hUpdtMMWpage4::"
    dim brc as boolean
   
    brc = false

    Kontext "TabMailMergeGreetingsPage"
    if ( not TabMailMergeGreetingsPage.exists( 2 ) ) then
        warnlog( CFN & "Page 4 could not be accessed" )
        exit function
    endif
    
    call dialogtest( TabMailMergeGreetingsPage )
    
    ' the salutation checkbox must be checked, otherwise the buttons are not 
    ' available
    if ( not GreetingPersonalized.isEnabled() ) then
        warnlog( CFN & "CheckBox Salutation not in HID.LST, it should be checked" )
        exit function
    endif
    
    Kontext "TabMailMergeGreetingsPage"
    printlog( CFN & "GreetingButtonFemale" )
    if ( GreetingButtonFemale.isEnabled() ) then
        hNewEditAddressBlock( 3 )
    else
        warnlog( CFN & "GreetingButtonFemale is not enabled" )
    endif

    Kontext "TabMailMergeGreetingsPage"
    printlog( CFN & "GreetingButtonMale" )
    if ( GreetingButtonMale.isEnabled() ) then
        hNewEditAddressBlock( 4 )
    else
        warnlog( CFN & "GreetingButtonMale is not enabled" )
    endif

    Kontext "TabMailMergeGreetingsPage"

end function


'**************************************************************************

function hUpdtMMWpage5()


    const CFN = "hUpdtMMWpage5::"
    
    kontext "TabMailMergeLayoutPage"
    if ( not TabMailMergeLayoutPage.exists( 2 ) ) then
        warnlog( CFN & "Page 5 could not be accessed" )
        exit function
    endif
    
    call DialogTest( TabMailMergeLayoutPage )
    
    printlog( CFN )
    
end function

'**************************************************************************

function hUpdtMMWpage6()


    const CFN = "hUpdtMMWpage6::"
    
    printlog( CFN )
    
    kontext "TabMailMergePrepare"
    if ( not TabMailMergePrepare.exists( 2 ) ) then
        warnlog( CFN & "Page 6 could not be accessed" )
        exit function
    endif
    
    call DialogTest( TabMailMergePrepare )
    
    qaerrorlog( "Skipping preview button" )
    
end function

'**************************************************************************

function hUpdtMMWpage7()

    
    const CFN = "hUpdtMMWpage7::"
    
    printlog( CFN )    
    
    kontext "TabMailMergePersonalize"
    hWaitForObject( EditIndividualButton, 8000 )
    'if ( not TabMailMergePersonalize.exists( 2 ) ) then
    '    warnlog( CFN & "Page 7 could not be accessed" )
    '    exit function
    'endif
    
    call DialogTest( TabMailMergePersonalize )
    
    qaerrorlog( "Skipping Preview-Button" )
    
end function

'**************************************************************************

function hUpdtMMWpage8()


    const CFN = "hUpdtMMWpage8::"
    
    kontext "TabMailMergeOutputPage"
    if ( not TabMailMergeOutputPage.exists( 2 ) ) then
        warnlog( CFN & "Page 8 could not be accessed" )
        exit function
    endif
    
    printlog( CFN )
    
    call DialogTest( TabMailMergeOutputPage )
    
    kontext "TabMailMergeOutputPage"
    hSaveStartingDocument()
    
    kontext "TabMailMergeOutputPage"
    hSaveMailMergeDocument()
    
    kontext "TabMailMergeOutputPage"
    hPrintMailMergeDocument()
    
    kontext "TabMailMergeOutputPage"
    hSendMailMergeDocument()
    
end function

'*******************************************************************************

function hSelectDifferentAddressList( cDatabase as string ) as boolean


    const CFN = "hSelectDifferentAddressList::"
    
    dim brc as boolean
        
    dim iAddressList as integer
    dim cAddressList as string
        
    Kontext "TabMailMergeAddressBlock"
    SelectAddressListButton.click()

    Kontext "SelectAddressList"
    if ( SelectAddressList.exists( 2 ) ) then
        printlog( CFN & "Select Address List Dialog" )
        call dialogtest( SelectAddressList )
    else
        warnlog( CFN & "SelectAddressList dialog is not open" )
        hSelectDifferentAddressList() = false
        exit function
    endif
	    
    Kontext "SelectAddressList"
    brc = hAddDatabase( cDatabase ) 

    
    Kontext "SelectAddressList"
    if ( SelectAddressList.exists( 2 ) ) then
        if ( AddressList.getItemCount() < 2 ) then
            brc = hAddDatabase( cDatabase )
        endif
        
        Kontext "SelectAddressList"
        brc = hEditCreateAddressList( 1 )
	
        Kontext "SelectAddressList" 
        brc = hFilterDialog()

        Kontext "SelectAddressList"
        brc = hEditCreateAddressList( 2 )
	
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

end function

'*******************************************************************************

function hWaitForBackToWizardFloat() as boolean


    const CFN = "hWaitForBackToWizardFloat::"
    dim brc as boolean
    
    printlog( CFN & "Enter" )
           
    Kontext "MailMergeFloat"
    if ( MailMergeFloat.exists( 20 ) ) then
        printlog( CFN & "Back to document float is open" )
        ReturnToMailMergeWizard.click()
        
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
        
end function

'*******************************************************************************

function hAddDatabase( cDatabase as string ) as boolean

    
    const CFN = "hAddDatabase::"
    
    dim brc as boolean
        brc = false
        
    dim iWait as integer
    
    add.click()

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
    hWaitForSelectAddressListDialog()
    
    hAddDatabase() = brc
    
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
	
end function

'******************************************************************************

function hCustomizeAddressList() as boolean



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

    
    const CFN = "hUpdtMailmergeMatchFields::"
    dim brc as boolean
    
    printlog( CFN & "Accessing Match Fields Dialog" )
    Kontext "TabMailMergeAddressBlock"
    MatchFieldsButton.click()
    
    kontext "MatchFields"
    if ( not MatchFields.exists( 2 ) ) then
        warnlog( CFN & "Unable to access Match Fields dialog" )
        brc = false
    else
    
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
        
        MatchFields.OK()
        printlog( CFN & "Leaving Match Fields Dialog" )
        brc = true
        
    endif
    
    hUpdtMailmergeMatchFields() = brc
        
end function


