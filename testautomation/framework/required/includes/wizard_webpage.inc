'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: wizard_webpage.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:19:04 $
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
'* owner : thorsten.bosbach@sun.com
'*
'* short description : Resource test of Web Page Wizard
'*
'\************************************************************************

testcase tUpdtWizardWebpage
    dim i as integer
    dim a as integer
    '/// call File->Wizards->WebPage ... ///'
    a = getDocumentCount
    FileWizardsWebPage

    Kontext "WebWizard"
    if WebWizard.exists(20) then
        '/// current page: 'Introduction' ///'
        printlog " 1. Introduction"
        Call Dialogtest2 (WebWizard,1)
        IntroductionChooseSettings.getItemCount
        IntroductionChooseSettings.getSelText
        '/// click button 'Next >' to get to next step ///'
        NextButton.click
        
        '/// current page: 'Documents' ///'
        printlog " 2. Documents"
        Call Dialogtest2 (WebWizard,2)
        '/// click button 'Add' to get some content to make teh other steps accessible ///'
        DocumentsAdd.click
        '/// Dialog 'Open' comes up ///'
        Kontext "OeffnenDlg"
        if OeffnenDlg.exists(5) then
            '/// select the image: global\input\graf_inp\borabora.jpg ///'
            Dateiname.setText ConvertPath ( gTesttoolPath + "global\input\graf_inp\borabora.jpg"
            '/// close dialog with 'Open' ///'
            Oeffnen.click           
        else
            warnlog "File Dialog didn't came up"
        endif
        sleep 5
        Kontext "WebWizard"
        DocumentsWebSiteContent.getSelText
        DocumentsWebSiteContent.getItemcount
        DocumentsExportFileFormat.getItemCount
        DocumentsExportFileFormat.getSelText
        DocumentsInformationTitle.getText
        DocumentsInformationsummary.setText "A"
        DocumentsInformationAuthor.setText "B"
        '/// click button 'Next >' to get to next step ///'
        NextButton.click
        
        '/// current page: 'Main Layout' ///'
        printlog " 3. Main Layout"
        Call Dialogtest2 (WebWizard,3)
        MainLayoutLayout2.typeKeys "<space>"
        '/// click button 'Next >' to get to next step ///'
        NextButton.click
        
        '/// current page: 'Layout details' ///'
        printlog " 4. Layout details"
        Call Dialogtest2 (WebWizard,4)
        LayoutDetailsFileName.isChecked
        LayoutDetailsDescription.isChecked
        LayoutDetailsAuthor.isChecked
        LayoutDetailsCreationDate.isChecked
        LayoutDetailsLastChangeDate.isChecked
        LayoutDetailsFileFormat.isChecked
        LayoutDetailsFileFormatIcon.isChecked
        LayoutDetailsNumberOfPages.isChecked
        LayoutDetailsSizeInKB.isChecked
        LayoutDetails640x480.isChecked
        LayoutDetails800x600.isChecked
        LayoutDetails1024x768.isChecked
        '/// click button 'Next >' to get to next step ///'
        NextButton.click
        
        '/// current page: 'Style' ///'
        printlog " 5. Style"
        Call Dialogtest2 (WebWizard,5)
        StyleStyle.getItemCount
        StyleStyle.getSelText
        '/// click on button Background image 'Choose...' ///'
        StyleBackgroundImage.click
        '/// Dialog 'Background images' comes up ///'
        Kontext "BackgroundImages"
        if BackgroundImages.exists(10) then
            Call Dialogtest (BackgroundImages)
            '/// click on buttons: '>', '<' and 'Other...' ///'
            Forward.click
            Back.click
            Other.click
            '/// Dialog 'Open' comes up ///'
            Kontext "OeffnenDlg"
            if OeffnenDlg.exists(5) then
                '/// Close dialog with Cancel ///'
                OeffnenDlg.cancel
            else
                warnlog "File Dialog didn't came up"
            endif
            sleep 5

            Kontext "BackgroundImages"
            '/// click button 'None' ///'
            None.click
            '/// close dialog by clicking button 'Cancel' ///'
            cancelB.click
        else
            warnlog "Dialog Background images didn't came up"
        endif
        Kontext "WebWizard"
        '/// click on button Icon set 'Choose...' ///'
        StyleIconSet.click
        '/// Dialog 'Icon sets' comes up ///'
        Kontext "IconSets"
        if IconSets.exists(5) then
            Call Dialogtest (IconSets)
            '/// click on button 'None' ///'
            None.click
            '/// close dialog by clicking button 'Cancel' ///'
            cancelB.click
        else
            warnlog "File Dialog didn't came up"
        endif
        Kontext "WebWizard"
        '/// click button 'Next >' to get to next step ///'
        NextButton.click

        '/// current page: 'Web site information' ///'
        printlog " 6. Web site information"
        Call Dialogtest2 (WebWizard,6)
        WebSiteInformationTitle.getText
        WebSiteInformationDescription.getText
        WebSiteInformationEmail.getText
        WebSiteInformationCopyrightNotice.getText
        WebSiteInformationCreated.getText
        WebSiteInformationCreated.more
        WebSiteInformationCreated.getText
        WebSiteInformationCreated.toMax
        WebSiteInformationCreated.getText
        WebSiteInformationModified.getText
        '/// click button 'Next >' to get to next step ///'
        NextButton.click
        
        '/// current page: 'Preview' ///'
        printlog " 7. Preview"
        Call Dialogtest2 (WebWizard,7)
        PreviewToALocalFolderCB.unCheck
        PreviewToALocalFolderTF.getText
        '/// click button To a local folder '...' ///'
        PublishToALocalFolderB.click
            '/// Dialog 'Local destination directory' comes up  ///'
            Kontext "OeffnenDlg"
            if OeffnenDlg.exists(5) then
                '/// Cancel dialog ///'
                OeffnenDlg.cancel
            else
                warnlog "File Dialog didn't came up"
            endif
            sleep 5
        Kontext "WebWizard"
        PublishToAZIPArchiveCB.isChecked
        PublishToAZIPArchiveTF.getText
        '/// click button To a ZIP archive '...' ///'
        PublishToAZIPArchiveB.click
            '/// Dialog 'Save as' comes up ///'
            Kontext "SpeichernDlg"
            if SpeichernDlg.exists(5) then
                '/// Cancel dialog ///'
                SpeichernDlg.cancel
            else
                warnlog "File Dialog didn't came up"
            endif
            sleep 5
        Kontext "WebWizard"
        PublishToAWebServer.isChecked
        try
            '/// click button 'Configure' ///'
            PublishFTPConfigure.click
            '/// Dialog 'FTP Connection' comes up  ///'
            Kontext "FTPConnection"
            if FTPConnection.exists(5) then
                Call Dialogtest (FTPConnection)
                ServerName.getText
                UserName.getText
                Password.getText
                Connect.isEnabled
                ChooseARemoteDirectory.getText
                ChooseARemoteDirectoryB.isEnabled
                '/// Close dialog with Cancel ///'
                CancelB.click
            endif
        catch
            printlog "no ftp"
        endcatch
        sleep 10
        Kontext "WebWizard"
        PublishSaveSettings.isChecked
        PublishSaveAs.getItemCount
        PublishSaveAs.getSelText
        CancelB.click
        i = 0
        while ((getdocumentcount > a) AND (i<30))
        	printlog "waiting for closing of wizard (max 30 sec): " + i
        	inc i
        	sleep 1
        wend
        if i > 29 then
        	qaErrorlog "Wizard performance issue; takes longer than 30 sec to close."
        endif
    else
        warnlog "Web page wizard didn't come up"
    endif
endcase

