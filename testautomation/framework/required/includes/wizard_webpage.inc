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

