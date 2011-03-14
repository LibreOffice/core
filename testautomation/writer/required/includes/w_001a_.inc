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
'* owner : helge.delfs@oracle.com
'*
'* short description:  Testcases for the Writer Required-test.
'*
'\***********************************************************************

sub w_001a_

    printLog Chr(13) + "--------- File Menu (w_001a_.inc) ----------"
    gApplication = "WRITER"

    Call tFileProperties

    Call tFileTemplatesOrganize
    Call tFileTemplatesAddressbookSource
    Call tFileTemplatesSave
    Call tFileTemplatesEdit

    Call tFilePagePreview
    Call tPrintCommon()
    Call tPrintGeneral()
    Call tPrintApplication()
    Call tPrintLayout()
    Call tPrintOptions()
    Call tFilePrinterSetup

    'Writer Master Document
    gApplication = "MASTERDOCUMENT"
    Call tMasterDocFilePagePreview
    gApplication = "WRITER"

end sub

'-----------------------------------------------------------

testcase tFileProperties
    PrintLog "- File / Properties"

    printlog " Open new document"
    Call hNewDocument

    printlog " File / Properties"
    FileProperties

    WaitSlot(5000)
    Kontext
    try
        active.SetPage TabDokument
    catch
        Warnlog "Dialog 'FileProperties' not up! (Bug#99828)"
        goto endsub
    endcatch

    printlog " Switch to Tabpage 'General'"
    Kontext "TabDokument"
    Call DialogTest (TabDokument)

    printlog " Switch to Tabpage 'Description'"
    Kontext
    active.SetPage TabDokumentinfo
    Kontext "TabDokumentinfo"
    Call DialogTest (TabDokumentinfo)


    Warnlog "#i95523# - FileProperties - Tabpage 'User Defined' has changed. - Adaption of testcases needed."
    '    printlog " Switch to Tabpage 'User defined'"
    '    Kontext
    '    active.SetPage TabBenutzer
    '    Kontext "TabBenutzer"
    '    Call DialogTest ( TabBenutzer )

    '    Wait 500

    '    Infofelder.Click
    '    printlog " Click 'Infofields'"
    '    Kontext "InfonamenBearbeiten"
    '    printlog " Close upcoming dialog with 'Cancel'"
    '    Call DialogTest (InfonamenBearbeiten)
    '    InfonamenBearbeiten.Cancel

    printlog " Switch to Tabpage 'Internet'"
    Kontext
    active.SetPage TabInternet
    Kontext "TabInternet"
    Call DialogTest (TabInternet)

    printlog " Switch to Tabpage 'Statistics' and close dialog"
    Kontext
    active.SetPage TabStatistik
    Kontext "TabStatistik"
    Call DialogTest (TabStatistik)
    TabStatistik.Close

    Sleep 1
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFileTemplatesOrganize
    Dim i as integer

    PrintLog " Open new document."
    Call hNewDocument
    PrintLog " File / Templates / Organize"
    FileTemplatesOrganize
    Kontext "DVVerwalten"
    For i = 1 to 50
        if DVVerwalten.Exists then
            i = 51
        else
            Sleep 5
        end if
    next i
    if Not DVVerwalten.Exists then
        Warnlog "Dialog 'Template Management' not up !"
        Call hCloseDocument
        goto endsub
    end if
    wait 500
    PrintLog " Select first entry in left list of dialog 'Template Management' (should be 'My Templates')"
    ListeLinks.Select 1
    ListeLinks.TypeKeys "<Home>"
    Sleep 1
    Befehle.Click
    wait 500
    PrintLog " Select 'Commands -> Import Template'"
    Call hMenuSelectNr(3)
    Wait 500
    PrintLog " Close Filedialog."
    if gUseSysDlg = False then
        Kontext "OeffnenDlg"
        if OeffnenDlg.Exists then
            OeffnenDlg.Cancel
        else
            Warnlog "FileOpen dialog not up!"
        end if
    end if

    PrintLog " Select 'Commands -> Printer Settings"
    Kontext "DVVerwalten"
    Sleep 1
    Befehle.Click
    wait 500
    Call hMenuSelectNr(4)

    PrintLog " Cancel 'Printer-Setup' dialog"
    Kontext "DruckerEinrichten"
    if DruckerEinrichten.Exists then
        DruckerEinrichten.Cancel
    else
        Warnlog "Printersettings dialog is not up!"
    end if

    PrintLog " Select first entry in right list"
    Kontext "DVVerwalten"
    ListeRechts.Select 1

    PrintLog " Select pushbutton 'File..'"
    if WelcheDatei.IsEnabled then
        WelcheDatei.Click
        if gUseSysDlg = False then
            PrintLog " Cancel Filedialog"
            Kontext "OeffnenDlg"
            OeffnenDlg.Cancel
        end if
    end if

    PrintLog " Select pushbutton 'Address Book'"
    Kontext "DVVerwalten"
    AddressBook.Click
    Kontext "AddressbookSource"
    PrintLog " Cancel Dialog 'Address Book-Assignment'"
    if AddressbookSource.Exists then
        AddressbookSource.Cancel
    else
        Warnlog "the Dialog ' Address Book Source' wasnt up!"
    end if

    PrintLog " Close dialog"
    Kontext "DVVerwalten"
    DVVerwalten.Close

    PrintLog " Close active document."
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFileTemplatesAddressbookSource

    PrintLog " Open new document"
    Call hNewDocument

    PrintLog " File / Templates /Address Book Source..."
    UseBindings
    FileTemplatesAddressbookSource

    PrintLog " In 'Address Book Assignement' dialog click pushbutton 'Administrate'"
    Kontext "AddressbookSource"
    if AddressbookSource.Exists( 2 ) then
        Call DialogTest (AddressbookSource)
        Administrate.Click

        PrintLog " Close 'Data Source Administration' with 'Cancel'"
        Kontext "DatabaseProperties"
        if ( DatabaseProperties.Exists( 2 ) ) then
            PrintLog " DatabaseProperties.SetPage TabConnection"
            Kontext "TabConnection"
            Call DialogTest ( TabConnection )
            Kontext "DatabaseProperties"
            DatabaseProperties.Close
            Kontext "AddressbookSource"
            PrintLog " Close dialog"
            AddressbookSource.Cancel
        else
            Kontext "AddressSourceAutopilot"
            if AddressSourceAutopilot.Exists( 1 ) then
                AddressSourceAutopilot.Cancel
            end if
            Kontext "AddressbookSource"
            AddressbookSource.Cancel
        end if
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFileTemplatesSave
    if gApplication = "MASTERDOCUMENT" then
        qaErrorLog "    tFileTemplatesSave has been disabled in Masterdocument."
        goto endsub
    end if
    printlog "- File / Templates / Save"
    Call hNewDocument
    printlog " Open new document"
    FileTemplatesSave
    printlog " File / Templates / Save"

    Kontext "DokumentVorlagen"
    Call DialogTest ( DokumentVorlagen )
    printlog " On 'Templates' dialog click 'Organizer'"

    Verwalten.Click
    Kontext "DVVerwalten"
    DVVerwalten.Close
    printlog " Close dialog 'Template Management'"

    Kontext "DokumentVorlagen"
    DokumentVorlagen.Cancel
    printlog " Close 'Templates' dialog"

    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------

testcase tFileTemplatesEdit
    printlog "- File / Templates / Edit"

    printlog " Open new document"
    Call hNewDocument
    printlog " File / Templates / Edit"
    FileTemplatesEdit

    if gUseSysDlg = False then
        printlog " Cancel Filedialog"
        Kontext "OeffnenDlg"
        OeffnenDlg.Cancel
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFilePagePreview
    PrintLog "- File / Page Preview"
    Dim gehtnicht as boolean
    gehtnicht = FALSE
    printlog " Open new document"
    Select Case gApplication
    Case "WRITER", "MASTERDOCUMENT" : hNewDocument()
    Case else
        printlog "Not in Writer/Web!"
        goto endsub
    end select

    printlog " Enter some text in document"
    Call wTypeKeys("Just a small change!")
    printlog " File / Page Preview"
    FilePageView
    WaitSlot (1000)
    Kontext "DocPageViewWriter"
    if DocPageViewWriter.NotExists then
        Warnlog "Page Preview is not coming up!"
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                Active.No
            end if
        end if
    else
        if gehtnicht=FALSE then
            Kontext "PreviewObjectbar"
            if PreviewObjectbar.Exists = False then Call hToolbarSelect("PagePreview", true)
            printlog " Select 'Zoom 100%' in toolbar"
            try
                Massstab.Select 4
            catch
                Warnlog "Unable to change Zoom factor in toolbar! (Bug#110498)"
            endcatch

            printlog " Click 'Print Page Preview' in toolbar"
            Kontext "PreviewObjectbar"
            DruckenSeitenansicht.Click
            Kontext "Active"
            if Active.Exists then
                QAErrorlog "No default printer!"
                Active.ok

                Kontext "Printing"
                if Printing.Exists( 2 ) then Printing.Cancel
            else
                printlog " Close 'Print' dialog with cancel"
                Kontext "Printing"
                Printing.Cancel
            end if

            printlog " Close Page Preview with button 'Close Preview' in toolbar"
            Kontext "PreviewObjectbar"
            SeitenansichtSchliessen.Click
        else
            Warnlog "Objectbar not addressable!"
        end if
    end if
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFilePrinterSetup
    PrintLog " Open new document"
    Call hNewDocument

    PrintLog " File / Printer Settings"
    FilePrintersettings
    Kontext "Active"
    WaitSlot(1000)
    if Active.Exists then
        if Active.GetRT = 304 then
            QAErrorLog "No Default Printer!"
            Active.Ok

            Kontext "DruckerEinrichten"
            if DruckerEinrichten.Exists( 2 ) then
                Call DialogTest ( DruckerEinrichten )
                DruckerEinrichten.Cancel
            end if
        end if
    else
        Kontext "DruckerEinrichten"
        Call DialogTest ( DruckerEinrichten )
        DruckerEinrichten.Cancel
        PrintLog " Cancel 'Printer Setup' dialog"
    end if
    PrintLog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tMasterDocFilePagePreview
    PrintLog "- File / Page Preview"
    Dim gehtnicht as boolean
    gehtnicht = FALSE
    gApplication   = "MASTERDOCUMENT"
    printlog " Open new document"
    Call hNewDocument

    printlog " Enter some text in document"
    Call wTypeKeys("Just a small change!")
    printlog " File / Page Preview"
    FilePageView
    WaitSlot (1000)
    Kontext "DocPageViewWriter"
    if DocPageViewWriter.NotExists then
        Warnlog "Page Preview is not coming up!"
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                Active.No
            end if
        end if
    else
        if gehtnicht=FALSE then
            Kontext "PreviewObjectbar"
            if PreviewObjectbar.Exists = False then Call hToolbarSelect("PagePreview", true)
            printlog " Select 'Zoom 100%' in toolbar"
            try
                Massstab.Select 4
            catch
                Warnlog "Unable to change Zoom factor in toolbar! (Bug#110498)"
            endcatch

            printlog " Click 'Print Page Preview' in toolbar"
            Kontext "PreviewObjectbar"
            DruckenSeitenansicht.Click
            Kontext "Active"
            if Active.Exists then
                QAErrorlog "No default printer!"
                Active.ok
                Sleep 1
                Kontext "Printing"
                if Printing.Exists then Printing.Cancel
            else
                printlog " Close 'Print' dialog with cancel"
                Kontext "Printing"
                Printing.Cancel
            end if

            printlog " Close Page Preview with button 'Close Preview' in toolbar"
            Kontext "PreviewObjectbar"
            SeitenansichtSchliessen.Click
        else
            Warnlog "Objectbar not addressable!"
        end if
    end if

    printlog " Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase
