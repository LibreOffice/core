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
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : Resource test of Web Page Wizard
'*
'\************************************************************************

testcase tUpdtWizardWebpage

    printlog( "Resource test for the webpage wizard" )

    dim i as integer
    dim a as integer
    a = getDocumentCount
    FileWizardsWebPage

    Kontext "WebWizard"
    if WebWizard.exists(20) then

        printlog " 1. Introduction"
        Call DialogTest (WebWizard,1)
        IntroductionChooseSettings.getItemCount
        IntroductionChooseSettings.getSelText
        NextButton.click
        
        printlog " 2. Documents"
        Call DialogTest (WebWizard,2)
        DocumentsAdd.click

        Kontext "OeffnenDlg"
        if OeffnenDlg.exists(5) then
            Dateiname.setText ConvertPath ( gTesttoolPath + "global\input\graf_inp\borabora.jpg"
            Oeffnen.click           
        else
            warnlog "File Dialog didn't came up"
        endif
        WaitSlot( 5000 )
        
        Kontext "WebWizard"
        DocumentsWebSiteContent.getSelText
        DocumentsWebSiteContent.getItemcount
        DocumentsExportFileFormat.getItemCount
        DocumentsExportFileFormat.getSelText
        DocumentsInformationTitle.getText
        DocumentsInformationsummary.setText "A"
        DocumentsInformationAuthor.setText "B"
        NextButton.click
        
        printlog " 3. Main Layout"
        Call DialogTest (WebWizard,3)
        MainLayoutLayout2.typeKeys "<space>"
        NextButton.click
        
        printlog " 4. Layout details"
        Call DialogTest (WebWizard,4)
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
        NextButton.click
        
        printlog " 5. Style"
        Call DialogTest (WebWizard,5)
        StyleStyle.getItemCount
        StyleStyle.getSelText
        StyleBackgroundImage.click
        Kontext "BackgroundImages"
        if BackgroundImages.exists(10) then
            Call Dialogtest (BackgroundImages)
            Forward.click
            Back.click
            Other.click
            Kontext "OeffnenDlg"
            if OeffnenDlg.exists(5) then
                OeffnenDlg.cancel
            else
                warnlog "File Dialog didn't came up"
            endif
            WaitSlot( 5000 )

            Kontext "BackgroundImages"
            None.click
            cancelB.click
        else
            warnlog "Dialog Background images didn't came up"
        endif
        Kontext "WebWizard"
        StyleIconSet.click
        Kontext "IconSets"
        if IconSets.exists(5) then
            Call Dialogtest (IconSets)
            None.click
            cancelB.click
        else
            warnlog "File Dialog didn't came up"
        endif
        Kontext "WebWizard"
        NextButton.click

        printlog " 6. Web site information"
        Call DialogTest (WebWizard,6)
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
        NextButton.click
        
        printlog " 7. Preview"
        Call DialogTest (WebWizard,7)
        PreviewToALocalFolderCB.unCheck
        PreviewToALocalFolderTF.getText
        PublishToALocalFolderB.click
            Kontext "OeffnenDlg"
            if OeffnenDlg.exists(5) then
                OeffnenDlg.cancel
            else
                warnlog "File Dialog didn't came up"
            endif
            WaitSlot( 5000 )
        Kontext "WebWizard"
        PublishToAZIPArchiveCB.isChecked
        PublishToAZIPArchiveTF.getText
        PublishToAZIPArchiveB.click
            Kontext "SpeichernDlg"
            if SpeichernDlg.exists(5) then
                SpeichernDlg.cancel
            else
                warnlog "File Dialog didn't came up"
            endif
            WaitSlot( 5000 )
        Kontext "WebWizard"
        PublishToAWebServer.isChecked
        try
            PublishFTPConfigure.click
            Kontext "FTPConnection"
            if FTPConnection.exists(5) then
                Call Dialogtest (FTPConnection)
                ServerName.getText
                UserName.getText
                Password.getText
                Connect.isEnabled
                ChooseARemoteDirectory.getText
                ChooseARemoteDirectoryB.isEnabled
                CancelB.click
            endif
        catch
            printlog "no ftp"
        endcatch
        
        Kontext "WebWizard"
        if ( webwizard.exists( 10 ) ) then
            PublishSaveSettings.check()
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
            warnlog( "Cannot access webwizard" )
        endif
    else
        warnlog "Web page wizard didn't come up"
    endif
endcase

