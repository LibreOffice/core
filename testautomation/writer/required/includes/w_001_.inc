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
'* short description :  Required-Testcases for the File-menu.
'*
'\***********************************************************************

sub w_001_

    printLog Chr(13) + "--------- File Menu (w_001_.inc) ----------"
    gApplication = "WRITER"

    Call tFileNewLabels
    Call tFileNewBusinessCards
    Call tFileOpen

    Call tFileClose
    Call tFileSave
    Call tFileSaveAll
    Call tFileSaveAs
    Call tFileSaveAsExport
    Call tFileSaveWithPassword
    Call tFileReload
    Call tFileVersions

end sub

'-----------------------------------------------------------

testcase tFileNewLabels

    Dim iSheetFormat as integer
    Dim iBrandIndex as integer
    Dim iType as integer
    Dim sTypes as integer
    Dim vTypes as integer
    
    ' The brands have fixed names and should not change
    dim cBrandList( 10 ) as string
        cBrandList(  1 ) = "Avery A4" 
        cBrandList(  2 ) = "Avery A4/Asia"
        cBrandList(  3 ) = "Avery Letter Size"
        cBrandlist(  4 ) = "Avery Zweckform"
        cBrandList(  5 ) = "Herlitz"
        cBrandList(  6 ) = "Herma A4"
        cBrandList(  7 ) = "Herma A5"
        cBrandList(  8 ) = "Herma endlos"
        cBrandList(  9 ) = "Leitz"
        cBrandList( 10 ) = "Sigel"

    PrintLog "Open Labels-dialog with File / New / Labels"
    if ( hCreateLabels() ) then

        Call DialogTest( TabEtiketten )
        
        PrintLog "Switch to Tabpage 'Labels'"
        PrintLog "Check number of brands and types (see #i44801# for details)"
        
        For iSheetFormat = 1 to 2

            kontext "TabEtiketten"
            Select Case( iSheetFormat )
            Case 1
                Endlos.Check
                printlog "Continous is checked"
            Case 2
                Bogen.Check
                printlog "Sheet is checked"
            end select
            
            PrintLog "There should be 10 brands in list"
            For iBrandIndex = 1 to Marke.GetItemCount
            
                Marke.Select( cBrandList( iBrandIndex ) )
                printlog "- Brand: " & Marke.GetSelText
                sTypes = Typ.GetItemCount
                
                Select Case iBrandIndex
                Case 1:
                    PrintLog "Avery/A4 should have 295 types in list if 'Sheet' is checked"
                    PrintLog "Avery/A4 should have 1 type in list if 'Continous' is checked"
                    if ( iSheetFormat = 1 ) then vTypes = 1 else vTypes = 295
                Case 2:
                    PrintLog "Avery A4/Asia should have 88 types in list if 'Sheet' is checked"
                    PrintLog "Avery A4/Asia should have 1 type in list if 'Continous' is checked"
                    if ( iSheetFormat = 1 ) then vTypes = 1 else vTypes = 88
                Case 3:
                    PrintLog "Avery Lettersize should have 251 types in list if 'Sheet' is checked"
                    PrintLog "Avery Lettersize should have 45 types in list if 'Continous' is checked"
                    if ( iSheetFormat = 1 ) then vTypes = 45 else vTypes = 251
                Case 4:
                    PrintLog "Avery Zweckform should have 333 types in list if 'Sheet' is checked"
                    PrintLog "Avery Zweckform should have 67 types in list if 'Continous' is checked"
                    if ( iSheetFormat = 1 ) then vTypes = 67 else vTypes = 333
                Case 5:
                    PrintLog "Herlitz should have 26 types in list if 'Sheet' is checked"
                    PrintLog "Herlitz should have 6 types in list if 'Continous' is checked"
                    if ( iSheetFormat = 1 ) then vTypes = 6 else vTypes = 26
                Case 6:
                    PrintLog "Herma A4 should have 492 types in list if 'Sheet' is checked"
                    PrintLog "Herma A4 should have 1 type in list if 'Continous' is checked"
                    if ( iSheetFormat = 1 ) then vTypes = 1 else vTypes = 492
                Case 7:
                    PrintLog "Herma A5 should have 18 types in list if 'Sheet' is checked"
                    PrintLog "Herma A5 should have 1 type in list if 'Continous' is checked"
                    if ( iSheetFormat = 1 ) then vTypes = 1 else vTypes = 18
                Case 8:
                    PrintLog "Herma endlos should have 1 type in list if 'Sheet' is checked"
                    PrintLog "Herma endlos should have 119 types in list if 'Continous' is checked"
                    if ( iSheetFormat = 1 ) then vTypes = 119 else vTypes = 1
                Case 9:
                    PrintLog "Leitz should have 13 types in list if 'Sheet' is checked"
                    PrintLog "Leitz should have 1 type in list if 'Continous' is checked"
                    if ( iSheetFormat = 1 ) then vTypes = 1 else vTypes = 13
                Case 10:
                    PrintLog "Sigel should have 20 types in list if 'Sheet' is checked"
                    PrintLog "Sigel should have 1 type in list if 'Continous' is checked"
                    if ( iSheetFormat = 1 ) then vTypes = 1 else vTypes = 20
                Case else: 
                    Warnlog "- There are more than 10 brands in list"
                end select
                if sTypes <> vTypes then Warnlog "-  Not " & sTypes & " types in list but " & vTypes
                
                For iType = 1 to Typ.GetItemCount
                    Typ.Select iType
                    printlog "-  Type: " & Typ.GetSelText
                next iType
            next iBrandIndex
        next iSheetFormat

        PrintLog "Switch to Tabpage 'Format'"
        Kontext
        active.SetPage TabFormatEtiketten
        wait( 100 )

        Kontext "TabFormatEtiketten"
        Call DialogTest (TabFormatEtiketten)

        PrintLog "Click 'Save'"
        FormatSpeichern.Click

        Kontext "SaveLabelFormat"
        if ( SaveLabelFormat.exists( 1 ) ) then
            
            Call DialogTest (SaveLabelFormat)
            
            PrintLog "Close dialog 'Save Label Format'"
            SaveLabelFormat.Cancel
        else
            warnlog( "Dialog <SaveLabelFormat> is not available" )
        end if
        
        Kontext "TabFormatEtiketten"
        
        PrintLog "Switch to Tabpage 'Options'"
        Kontext
        active.SetPage TabDruckerEtiketten
        
        Kontext "TabDruckerEtiketten"
        if ( TabDruckerEtiketten.exists( 1 ) ) then
            Call DialogTest (TabDruckerEtiketten)

            if ( gPlatgroup = "w95" or gPlatgroup = "wnt" ) then
                PrintLog "Click printer setup for Windows"
                Einrichten.Click

                PrintLog "Close printer-setup dialog"
                Kontext "DruckerEinrichten"
                if DruckerEinrichten.Exists( 2 ) then
                    DruckerEinrichten.Cancel
                else
                    Warnlog "No Default Printer defined for the system, set one!"
                end if
            else
                printlog( "No printer setup for UNIX like OS" )
            end if

            PrintLog "Check checkbox 'Synchronize contents'"
            Kontext "TabDruckerEtiketten"
            InhaltSynchronisieren.Check()
            
            PrintLog "Click 'New document'"
            NeuesDok.Click()

            printlog( "JSK: Unknown messagebox handled silently before refactoring" )
            Kontext "Active"
            if ( Active.Exists( 1 ) ) then
                warnlog( "Unexpected Messagebox: " & Active.getText()
                try
                    printlog( "Click <ok>" )
                    Active.Ok
                catch
                    Warnlog "Once Document has been opened, office asks to save it! BugID 89702"
                    Active.Cancel
                endcatch
            end if

            PrintLog "Check if dialog 'Synchronize Labels' is up"
            Kontext "EtikettenSynchronisieren"
            if ( EtikettenSynchronisieren.Exists( 2 ) ) then
                Call DialogTest ( EtikettenSynchronisieren )
                Synchronisieren.Click
                WaitSlot()
            else
                Warnlog " Dialog to synchronize labels not up!"
            end if
        else
            warnlog( "Tabpage <TabDruckerEtiketten> is not available" )
        end if
    else
        warnlog( "hCreateLabels() failed" )
    end if

    PrintLog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFileNewBusinessCards

    Dim iFormat as integer
    Dim iBrand as integer
    Dim iType as integer
    Dim iWait as integer
    Dim sTypes as integer
    Dim vTypes as integer
    
    ' The brands have fixed names and should not change
    dim cBrandList( 10 ) as string
        cBrandList(  1 ) = "Avery A4" 
        cBrandList(  2 ) = "Avery A4/Asia"
        cBrandList(  3 ) = "Avery Letter Size"
        cBrandlist(  4 ) = "Avery Zweckform"
        cBrandList(  5 ) = "Herlitz"
        cBrandList(  6 ) = "Herma A4"
        cBrandList(  7 ) = "Herma A5"
        cBrandList(  8 ) = "Herma endlos"
        cBrandList(  9 ) = "Leitz"
        cBrandList( 10 ) = "Sigel"
    

	PrintLog "Open Business Cards dialog with File / New / Business Cards"
	if ( hCreateBusinessCards ) then     
        PrintLog "Switch to Tabpage 'Medium'"
        Kontext
        Active.Setpage TabEtikettenMedium
        
        Kontext "TabEtikettenMedium"
        if ( TabEtikettenMedium.exists( 1 ) ) then
            Call DialogTest (TabEtikettenMedium)

            PrintLog "Check number of brands and types (see #i44801# for details)"
            For iFormat = 1 to 2
                Select Case iFormat
                Case 1
                    Endlos.Check
                    printlog "Continous is checked"
                Case 2
                    Bogen.Check
                    printlog "Sheet is checked"
                end select
                
                PrintLog "There should be 10 brands in list"
                For iBrand = 1 to Marke.GetItemCount
                    Marke.Select( cBrandList( iBrand ) )
                    printlog "- Brand: " & Marke.GetSelText
                    sTypes = Typ.GetItemCount
                    Select Case iBrand
                    Case 1:
                        PrintLog "Avery/A4 should have 295 types in list if 'Sheet' is checked"
                        PrintLog "Avery/A4 should have 1 type in list if 'Continous' is checked"
                        if ( iFormat = 1 ) then vTypes = 1 else vTypes = 295
                    Case 2:
                        PrintLog "Avery A4/Asia should have 88 types in list if 'Sheet' is checked"
                        PrintLog "Avery A4/Asia should have 1 type in list if 'Continous' is checked"
                        if ( iFormat = 1 ) then vTypes = 1 else vTypes = 88
                    Case 3:
                        PrintLog "Avery Lettersize should have 251 types in list if 'Sheet' is checked"
                        PrintLog "Avery Lettersize should have 45 types in list if 'Continous' is checked"
                        if ( iFormat = 1 ) then vTypes = 45 else vTypes = 251
                    Case 4:
                        PrintLog "Avery Zweckform should have 333 types in list if 'Sheet' is checked"
                        PrintLog "Avery Zweckform should have 67 types in list if 'Continous' is checked"
                        if ( iFormat = 1 ) then vTypes = 67 else vTypes = 333
                    Case 5:
                        PrintLog "Herlitz should have 26 types in list if 'Sheet' is checked"
                        PrintLog "Herlitz should have 6 types in list if 'Continous' is checked"
                        if ( iFormat = 1 ) then vTypes = 6 else vTypes = 26
                    Case 6:
                        PrintLog "Herma A4 should have 492 types in list if 'Sheet' is checked"
                        PrintLog "Herma A4 should have 1 type in list if 'Continous' is checked"
                        if ( iFormat = 1 ) then vTypes = 1 else vTypes = 492
                    Case 7:
                        PrintLog "Herma A5 should have 18 types in list if 'Sheet' is checked"
                        PrintLog "Herma A5 should have 1 type in list if 'Continous' is checked"
                        if ( iFormat = 1 ) then vTypes = 1 else vTypes = 18
                    Case 8:
                        PrintLog "Herma endlos should have 1 type in list if 'Sheet' is checked"
                        PrintLog "Herma endlos should have 119 types in list if 'Continous' is checked"
                        if ( iFormat = 1 ) then vTypes = 119 else vTypes = 1
                    Case 9:
                        PrintLog "Leitz should have 13 types in list if 'Sheet' is checked"
                        PrintLog "Leitz should have 1 type in list if 'Continous' is checked"
                        if ( iFormat = 1 ) then vTypes = 1 else vTypes = 13
                    Case 10:
                        PrintLog "Sigel should have 20 types in list if 'Sheet' is checked"
                        PrintLog "Sigel should have 1 type in list if 'Continous' is checked"
                        if ( iFormat = 1 ) then vTypes = 1 else vTypes = 20
                    Case else: 
                        Warnlog "- There are more than 10 brands in list"
                    end select
                    
                    if sTypes <> vTypes then Warnlog "-  Not " & sTypes & " types in list but " & vTypes
                    For iType = 1 to Typ.GetItemCount
                        Typ.Select iType
                        printlog "-  Type: " & Typ.GetSelText
                    next iType
                next iBrand
            next iFormat

            PrintLog "Switch to Tabpage 'Business Cards'"
            Kontext
            Active.Setpage TabVisitenkarten
        else
            warnlog( "Dialog <TabEtiketteMedium> not available" )
        end if

        Kontext "Active"
        if ( Active.GetRT = 304 ) then
            Warnlog( "Unexpected messagebox: " & Active.GetText() )
            Active.Ok
        end if

        Kontext "TabVisitenkarten"
        for iWait = 1 to 100
            if Inhalt.IsVisible then
                Call DialogTest (TabVisitenkarten)
                iWait = 101
            else
                wait( 100 )
            end if
        next iWait

        PrintLog "Switch to Tabpage 'Private'"
        Kontext
        Active.Setpage TabPrivat

        Kontext "TabPrivat"
        Call DialogTest (TabPrivat)

        PrintLog "Switch to Tabpage 'Business'"
        Kontext
        Active.Setpage TabGeschaeft

        Kontext "TabGeschaeft"
        Call DialogTest (TabGeschaeft)

        Kontext
        Active.Setpage TabFormatEtiketten

        PrintLog "Switch to Tabpage 'Format'"
        Kontext "TabFormatEtiketten"
        Call DialogTest (TabFormatEtiketten)

        PrintLog "Click pushbutton 'Save'"
        FormatSpeichern.Click

        PrintLog "Close upcoming dialog 'Save Label Format'"
        Kontext "SaveLabelFormat"

        Call DialogTest (SaveLabelFormat)
        SaveLabelFormat.Cancel

        Kontext "TabFormatEtiketten"
        Kontext
        Active.Setpage TabZusaetzeEtiketten

        Kontext "TabZusaetzeEtiketten"
        Call DialogTest (TabZusaetzeEtiketten)
        Einrichten.Click

        Kontext "DruckerEinrichten"
        if ( DruckerEinrichten.Exists( 1 ) ) then
            DruckerEinrichten.Cancel
        else
            Warnlog "No Default Printer configured for this system. Set one!"
        end if

        Kontext "TabZusaetzeEtiketten"
        PrintLog "Switch to Tabpage 'Options'"
        PrintLog "Check Checkbox 'Syncronize contents"
        InhaltSynchronisieren.Check

        PrintLog "Click 'new Document'"
        NeuesDok.Click

        Kontext "Active"
        if ( Active.Exists( 2 ) ) then
            try
                Active.Ok
            catch
                Active.Cancel
                Warnlog "Once Document has been opened, office asks to save it! BugID 89702"
            endcatch
        end if

        PrintLog "Check i dialog 'Syncronize Labels' is up"
        Kontext "EtikettenSynchronisieren"
        if ( EtikettenSynchronisieren.Exists( 2 ) ) then
            Call DialogTest ( EtikettenSynchronisieren )
            Synchronisieren.Click
        else
            Warnlog "Dialog to synchronize Business Cards not up!"
        end if

        PrintLog "Close active document"
        Call hCloseDocument
    else
        warnlog( "Failed to create business cards" )
    end if
    
endcase

'-----------------------------------------------------------

testcase tFileOpen

    printLog "- File / Open"

    FileOpen
    
    Kontext "OeffnenDlg"
    if ( OeffnenDlg.exists( 2 ) ) then
    
        Call DialogTest ( OeffnenDlg )
        
        PrintLog "Click 'Up one level'"
        UebergeordneterOrdner.Click
        
        WaitSlot()
        
        try
            PrintLog "Click button 'Default directory'"
            Standard.Click
            WaitSlot()
        catch
            ExceptLog
        endcatch
        
        PrintLog "Click Button 'New folder'"
        if ( NeuerOrdner.exists() ) then
            if ( NeuerOrdner.isVisible() ) then
                if ( NeuerOrdner.IsEnabled() ) then
                    NeuerOrdner.Click
                    
                    Kontext "NeuerOrdner"
                    if ( NeuerOrdner.exists( 1 ) ) then
                        PrintLog "Enter a foldername in upcoming dialog"
                        OrdnerName.SetText( "TT_Test" )
                        
                        PrintLog "Close dialog with 'Cancel'"
                        NeuerOrdner.Cancel
                    else
                        warnlog( "Dialog <NeuerOrdner> not available" )
                    end if
                else
                    Warnlog( "New Folder button disabled" )
                end if
            else
                Warnlog( "New Folder button not visible" )
            end if
        else
            Warnlog( "New Folder button missing on dialog" )
        end if
        Kontext "OeffnenDlg"
        OeffnenDlg.Cancel
    else
        warnlog( "File Open dialog is not available" )
    end if
    PrintLog "Close File-Open-dialog with 'Cancel'"
endcase

'-----------------------------------------------------------

testcase tFileClose

    PrintLog "- File/Close"

    PrintLog "Open new document"
    Call hNewDocument
    
    PrintLog "Insert a table"
    Call hTabelleEinfuegen

    PrintLog "File / Close"
    hUseAsyncSlot( "FileClose" )
    
    PrintLog( "Cancel saving the document" )
    kontext "Active"
    if ( Active.exists( 1 ) ) then
        Active.Cancel
    else
        Warnlog( "No warning for modified document" )
    end if    

    PrintLog "again File / Close"
    hUseAsyncSlot( "FileClose" )
    
    Printlog( "Confirm to save the document" )
    Kontext "Active"
    if ( Active.exists( 1 ) ) then
        Active.Yes()
    else
        warnlog( "No warning for modified document" )
    end if

    PrintLog "Click 'Cancel' in File-Save-Dialog"
    Kontext "SpeichernDlg"
    if ( SpeichernDlg.Exists( 1 ) ) then
        SpeichernDlg.Cancel
    else
        Kontext "Active"
        if Active.Exists() then
            Warnlog Active.Gettext()
            Active.Ok()
        else
            Warnlog( "File Save dialog not displayed, messagebox came up instead" )
        end if
    end if

    PrintLog "Choose File / Close again"
    hUseAsyncSlot( "FileClose" )
    
    PrintLog "On upcoming messagebox click 'No'"
    Kontext "Active"
    if ( Active.exists( 1 ) ) then
        Active.No
    else
        warnlog( "Expected messagebox missing" )
    end if
    
endcase

'-----------------------------------------------------------

testcase tFileSave

    PrintLog "- File/Save"

    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "Insert a table"
    Call hTabelleEinfuegen

    PrintLog "Choose 'File / Save'"
    FileSave
    
    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 2 ) ) then
    
        Call DialogTest ( SpeichernDlg )
        PrintLog "In File-Dialog click 'Up one level'"
        UebergeordneterOrdner.Click
        
        WaitSlot()
        
        try
            PrintLog "Click 'Default directory'"
            Standard.Click
            WaitSlot()
        catch
            ExceptLog
        endcatch

        PrintLog "Click 'Create New Folder '"
        if ( NeuerOrdner.exists() ) then
            if ( NeuerOrdner.isVisible() ) then
                if ( NeuerOrdner.IsEnabled() ) then
                    NeuerOrdner.Click
                    
                    Kontext "NeuerOrdner"
                    if ( NeuerOrdner.exists( 1 ) ) then
                    
                        PrintLog "Enter a foldername in upcoming dialog"
                        OrdnerName.SetText( "TT_Test" )

                        PrintLog "Close dialog with 'Cancel'"
                        NeuerOrdner.Cancel
                    else
                        warnlog( "Dialog <NeuerOrdner> not available" )
                    end if
                else
                    Warnlog( "New Folder button disabled" )
                end if
            else
                Warnlog( "New Folder button not visible" )
            end if
        else
            Warnlog( "New Folder button missing on dialog" )
        end if
        
        Kontext "SpeichernDlg"
        SpeichernDlg.cancel()
    else
        warnlog( "File Save dialog is not available" )
    end if

    PrintLog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFileSaveAll

    PrintLog "- File / Save All"

    PrintLog "Open new document"
    Call hNewDocument
    
    PrintLog "Insert a table in document"
    Call hTabelleEinfuegen
    
    PrintLog "Open a second document"
    Call hNewDocument
    
    PrintLog "Insert a table in second document"
    Call hTabelleEinfuegen

    PrintLog "Choose File / Save all"
    FileSaveAll
    
    PrintLog "Cancel 1. upcoming Filedialog"
    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 2 ) ) then
        SpeichernDlg.Cancel
    else
        warnlog( "First File Save dialog is not available" )
    end if
    
    PrintLog "Cancel 2. upcoming Filedialog"    
    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 2 ) ) then
        SpeichernDlg.Cancel
    else
        warnlog( "Second File Save dialog is not available" )
    end if

    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 1 ) ) then
        warnlog( "Third, unexpected File Save dialog open" )
        SpeichernDlg.cancel()
    end if

    PrintLog "Close documents"
    hFileCloseAll()
    
endcase

'-----------------------------------------------------------

testcase tFileSaveAs

    PrintLog "- File / Save As"

    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "Choose 'File / Save as'"
    FileSaveAs
    
    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 1 ) ) then
    
        Call DialogTest ( SpeichernDlg )
        
        PrintLog "Click button 'Up one level'"
        UebergeordneterOrdner.Click()
        
        WaitSlot()
        
        PrintLog "Click button 'Default Directory'"
        try
            Standard.Click
        catch
            ExceptLog
        endcatch
        
        PrintLog "Click button 'Create Folder'"
        if ( NeuerOrdner.exists() ) then
            if ( NeuerOrdner.isVisible() ) then
                if ( NeuerOrdner.IsEnabled() ) then
                    NeuerOrdner.Click
                    
                    Kontext "NeuerOrdner"
                    if ( NeuerOrdner.exists( 1 ) ) then
                    
                        Printlog( "Name the folder" )
                        OrdnerName.SetText( "TT_Test" )
                        PrintLog "Close dialog with 'Cancel'"
                        NeuerOrdner.Cancel
                    else
                        warnlog( "Dialog <NeuerOrdner> not available" )
                    end if
                else
                    Warnlog( "New Folder button disabled" )
                end if
            else
                Warnlog( "New Folder button not visible" )
            end if
        else
            Warnlog( "New Folder button missing on dialog" )
        end if
        
        Kontext "SpeichernDlg"
        SpeichernDlg.Cancel
    else
        warnlog( "File Save dialog is not available" )
    end if

    PrintLog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFileSaveAsExport

    QaErrorLog "#i93340# - Testcase outcommented due to bug with filter."
    goto endsub

    Dim xExport as string
    Dim xExportFile as string
    Dim CountDocu as integer
    PrintLog "- File / Save As / Export with MS-filter"

    Select Case gApplication
    Case "WRITER"
        xExport = Convertpath( gTesttoolpath + "writer\required\input\st1_1.sxw" )
    Case else
        goto endsub
    end select
    
    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "Open document '..\writer\required\input\st1_1.sxw"
    Call hFileOpenLocally(xExport)

    PrintLog "Select Save as .. and choose 'Microsoft Word 97/2000/XP' as export-filter"
    xExportFile = gOfficepath + "user\work\ecksport.doc"
    Call hFileSaveAsWithFilterKill (xExportFile, "MS Word 2003 XML")

    if ( hFileCloseAll() > 1 ) then
        QAErrorlog "More than 1 document had to be closed!"
    end if

    PrintLog "Close document"
    Call hCloseDocument
    
endcase

'-----------------------------------------------------------

testcase tFileSaveWithPassword

    Dim sFileName1 as string
    dim sFileName2 as string
    dim sFilePath as string

    Printlog "- File / Save with Password"
    Select Case gApplication
    Case "WRITER"
        sFileName1 = "password.odt"
        sFileName2 = "password2.odt"
    Case "MASTERDOCUMENT"
        sFileName1 = "password.odm"
        sFileName2 = "password2.odm"
    Case "HTML"
        printlog "Not in Writer/Web!"
        goto endsub
    end select

    PrintLog "Open new document"
    Call hNewDocument
    
    PrintLog "Enter some text into document"
    Call wTypeKeys("This document will be saved with a password!")

    sFilePath = ConvertPath ( gOfficePath + "user\work\" + sFileName1 )
    hDeleteFile( sFilePath )

    PrintLog "Choose File / Save as."
    FileSaveAs
    
    PrintLog "enter destination and check 'Password'"
    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 2 ) ) then
        if ( Passwort.exists() ) then
            if ( Passwort.isEnabled() ) then
                Passwort.Check()
            else
                warnlog( "FATAL: Password checkbox is not enabled" )
                goto endsub
            end if
        else
            warnlog( "FATAL: Password checkbox does not exist" )
            goto endsub
        end if

        PrintLog "Save the document"
        Dateiname.SetText( sFilePath )
        Speichern.Click()

        printlog( "JSK: Unknown messagebox handled silently before refactoring" )
        Kontext "Active"
        if ( Active.Exists (1) ) then 
            warnlog( "Unexpected messagebox: " & Active.getText() )
            Active.Yes
        end if

        PrintLog "Password dialog has to come up"
        Kontext "PasswordFileSave"
        if ( PasswordFileSave.Exists( 1 ) ) then
        
            PrintLog "Enter your password"
            Password.SetText( "12345" )
            
            PrintLog "Re-Enter a wrong passwort"
            PasswordConfirm.Settext( "54321" )
            PasswordFileSave.OK
        else
            Warnlog "- Password-Dialog not up !"
        end if

        Kontext "Active"
        PrintLog "A messagebox has to come up, warns of wrong confirmation"
        if ( Active.Exists( 1 ) )then
            Active.OK
        else
            Warnlog "Able to save document with wrong password"
        end if

        Kontext "PasswordFileSave"
        if ( PasswordFileSave.Exists() ) then
            PrintLog "Enter new password, and re-enter the same password"
            Password.SetText( "12345" )
            PasswordConfirm.SetText( "12345" )
            PasswordFileSave.OK
        else
            Warnlog "- Password-Dialog not up !"
        end if
    else
        warnlog( "Dialog <SpeichernDlg> not available" )
    end if


    PrintLog "Close the document"
    hUseAsyncSlot( "FileClose" )

    PrintLog "Open previous saved document"
    hFileOpen( sFilePath )

    Kontext "PasswordFileOpen"
    PrintLog "Enter a wrong password."
    if ( PasswordFileOpen.Exists() ) then
        PasswortName.SetText( "34567" )
        PasswordFileOpen.OK()
    else
        Warnlog "- Password-Dialog not up or Document couldn't be loaded!"
    end if

    PrintLog "A messagebox has to come up, warns of wrong password"
    Kontext "Active"
    if ( Active.Exists(1) ) then
        printlog( "Warning for incorrect password -> good" )
        Active.OK()
    else
        Warnlog "Able to load document with wrong password"
    end if

    Kontext "PasswordFileOpen"
    PrintLog "Cancel Password dialog (not loading the document"
    if ( PasswordFileOpen.Exists( 1 ) ) then
        PasswordFileOpen.Cancel()
    end if

    PrintLog "File / Open again your previous saved document"
    hFileOpen( sFilePath )
    
    PrintLog "Enter your password in password dialog"
    Kontext "PasswordFileOpen"
    if ( PasswordFileOpen.Exists() ) then
        PasswortName.SetText( "12345" )
        PasswordFileOpen.OK()
    else
        Warnlog "- Password-Dialog not up!"
    end if
    
    ' Rename document, the old one is now obsolete    
    sFilePath = ConvertPath ( gOfficePath + "user\work\" + sFileName2 )
    hDeleteFile( sFilePath )
    
    
    PrintLog "File / Save as ..."
    hUseAsyncSlot( "FileSaveAs" )
    
    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 2 ) ) then
    
        printlog( "Verify that password setting is persistent" )
        if ( Not Passwort.IsChecked ) then
            if ( gPlatgroup = "unx" ) then
                QAErrorlog "#i73968#Opening a password protected file looses focus after loading"
            else
                Warnlog "#i36015#Checkbox 'Save with password' is not checked in file-dialog !"
                Passwort.Check
            end if
            SpeichernDlg.Cancel
        else
            Dateiname.SetText( sFilePath )
            Speichern.Click()
            
            printlog( "JSK: Unknown messagebox handled silently before refactoring" )
            Kontext "Active"
            if ( Active.Exists( 2 ) ) then 
                printlog( "Messagebox: " & Active.getText() )
                Active.Yes()
            end if
            
            PrintLog "Click on 'Save' in file open dialog"
            Kontext "PasswordFileSave"
            if ( PasswordFileSave.Exists( 1 ) ) then
                
                Password.SetText( "54321" )
                PasswordConfirm.SetText( "54321" )
                PasswordFileSave.OK()

                PrintLog( "The password dialog should come up" )
                hUseAsyncSlot( "FileClose" )
                
                PrintLog "File / Close and open your document again"
                hFileOpen( sFilePath )

                Kontext "PasswordFileOpen"
                if ( PasswordFileOpen.exists() ) then
                    PasswortName.SetText "54321"
                    PasswordFileOpen.OK
                else
                    warnlog( "Password dialog did not open" )
                end if
            else
                Warnlog "Passworddialog is not coming up on saving document"
            end if
        end if
    else
        warnlog( "Dialog <SpeichernDlg> not available" )
    end if
    PrintLog "Close active document"
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------

testcase tFileReload

    Dim DokumentPfad$
    Dim Datei$
    Dim sFile$
    Dim WriterFilterExtension(10) as string
    Dim MasterDocFilterExtension(10) as string

    WriterFilterExtension() = hGetFilternameExtension ( "writer8" )
    MasterDocFilterExtension() = hGetFilternameExtension ( "writerglobal8" )

    PrintLog "- File / Reload"
    Select Case gApplication
    Case "WRITER"
        sFile$ = "reload." & WriterFilterExtension(0)
    Case "MASTERDOCUMENT"
        sFile$ = "reload." & MasterDocFilterExtension(0)
    Case "HTML"
        printlog "Not in Writer/Web!"
        goto endsub
    end select

    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "Enter some text, save and close document"
    Call wTypeKeys("File -> Reload!")
    Call hFileSaveAsKill ( gOfficePath + "user\work\" + sFile$ )
    FileClose
    Kontext
    if MessageBox.Exists (1) then
        WarnLog "Messagebox after saving and closing : " + MessageBox.GetText
        MessageBox.Yes
    end if
    Wait (500)

    PrintLog "Open previous saved document"
    hFileOpen(  gOfficePath + "user\work\" + sFile$ )
    PrintLog "Insert a table in document"
    Call hTabelleEinfuegen
    PrintLog "File / Reload"
    FileReload
    WaitSlot(1000)
    PrintLog "Quit messagebox with 'No'"
    Kontext "Active"
    Active.No
    Wait (500)

    PrintLog "File / Reload again and quit messagebox with yes"
    FileReload

    WaitSlot(2000)
    Kontext "Active"
    Active.Yes
    Wait (500)
    PrintLog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tFileVersions
    Dim sFile$
    Dim WriterFilterExtension(10) as string
    Dim MasterDocFilterExtension(10) as string

    WriterFilterExtension() = hGetFilternameExtension ( "writer8" )
    MasterDocFilterExtension() = hGetFilternameExtension ( "writerglobal8" )

    PrintLog "- File / Versions"
    Select Case gApplication
    Case "WRITER"
        sFile$ = "kann_weg." & WriterFilterExtension(0)
    Case "MASTERDOCUMENT"
        sFile$ = "kann_weg." & MasterDocFilterExtension(0)
    Case "HTML"
        printlog "Not in Writer/Web!"
        goto endsub
    end select

    PrintLog "Open new document"
    Call hNewDocument
    Call wTypeKeys ("File version")
    PrintLog "enter some text in documet and save it"
    Call hFileSaveAsKill(gOfficePath + "user\work\" + sFile$)
    Sleep 1

    try
        PrintLog "Choose File / Versions"
        FileVersions
    catch
        Warnlog "- File / Versions not accessible!"
        goto endsub
    endcatch

    PrintLog "In Version-dialog click 'Save New Version'"
    Kontext "Versionen"
    Call DialogTest ( Versionen )
    Speichern.Click
    PrintLog "Cancel dialog 'Insert version comment'"
    Kontext "VersionskommentarEingeben"
    Call DialogTest ( VersionskommentarEingeben )
    VersionskommentarEingeben.Cancel

    PrintLog "Close dialog"
    Kontext "Versionen"
    Versionen.Close

    PrintLog "Close documents"
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

'-----------------------------------------------------------

