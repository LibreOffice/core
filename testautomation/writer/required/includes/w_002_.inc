'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
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
'/******************************************************************************
'*                                                                            **
'* owner : helge.delfs@oracle.com                                                **
'*                                                                            **
'* short description :                                                        **
'*                                                                            **
'\******************************************************************************

sub w_002_

    printLog Chr(13) + "--------- Edit Menu ( w_002_.inc ) ----------"
    gApplication = "WRITER"

    Call tEditUndoRedo
    Call tEditRepeat

    Call tEditPaste
    Call tEditPasteSpecial
    Call tEditSelectionMode

    Call tEditChanges
    Call tEditCompareDocument
    Call tEditFindAndReplace
    Call tViewNavigator
    Call tEditAutoText
    Call tEditHyperlink

    Call tEditFields
    Call tEditFieldsInputList
    Call tEditFootnote
    Call tEditIndexEntry

    Call tEditLinks
    Call tEditPlugIn
    Call tEditImageMap
    Call tEditObjectEdit

    'Writer Master Document
    gApplication = "MASTERDOCUMENT"
    Call tMasterDocEditNavigator

    gApplication "HTML"
    Call tEditFields

    gApplication = "WRITER"

end sub

'-----------------------------------------------------------

testcase tEditUndoRedo
    PrintLog "- Edit/Undo - Redo"

    PrintLog "Open new document"
    hNewDocument()

    PrintLog "Insert table"
    Call hTabelleEinfuegen

    PrintLog "Edit / Undo"
    hUseAsyncSlot( "EditUndo" )
    
    PrintLog "Undo / Redo"
    hUseAsyncSlot( "EditRedo" )
    
    PrintLog "Edit / Undo"
    hUseAsyncSlot( "EditUndo" )
    
    PrintLog "File / Close"
    hUseAsyncSlot( "FileClose" )

    Kontext "Active"
    if ( Active.Exists( 1 ) ) then
        warnlog( "No messagebox was expected upon file close" )
        printlog( Active.getText )
        Active.no()
    end if

endcase

'----------------------------------------------------

testcase tEditRepeat
    PrintLog "- Edit/Repeat"

    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "Insert table"
    Call hTabelleEinfuegen

    PrintLog "Move cursor out of table"
    Call wTypeKeys("<Down><Down>")

    PrintLog "Edit / Repeat"
    hUseAsyncSlot( "EditRepeat" )

    PrintLog "Close active document"
    Call hCloseDocument
endcase

'----------------------------------------------------

testcase tEditPaste
    PrintLog "- Edit/Paste"

    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "Insert some text in document"
    Call wTypeKeys("What a beatiful day !")

    PrintLog "Edit / Select All"
    hUseAsyncSlot( "EditSelectAll" )
    
    PrintLog "Edit / Cut"
    hUseAsyncSlot( "EditCut" )
    
    PrintLog "Edit / Paste"
    hUseAsyncSlot( "EditPaste" )
    
    PrintLog "Type <Shift Home >"
    Call wTypeKeys("<Shift Home>")
    
    PrintLog "Edit / Copy"
    hUseAsyncSlot( "EditCopy" )
    
    PrintLog "Enter 2 Line breaks"
    Call wTypeKeys("<Return>", 2)
    
    PrintLog "Edit / Paste"
    hUseAsyncSlot( "EditPaste" )
    
    PrintLog "Edit / Select All"
    hUseAsyncSlot( "EditSelectAll" )
    
    PrintLog "Edit / Cut"
    hUseAsyncSlot( "EditCut" )
    
    PrintLog "Close active document"
    Call hCloseDocument
endcase

'----------------------------------------------------

testcase tEditPasteSpecial
    PrintLog "- Edit/Paste Special"
    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "  - External Clipboard"
    PrintLog "Enter some text in external clipboard"
    SetClipboard "Text from external clipboard!"

    PrintLog "Paste from external clipboard with Edit / Paste in Office"
    hUseAsyncSlot( "EditPaste" )

    Call wTypeKeys("<Mod1 A>")
    Wait(200)
    hUseAsyncSlot( "EditCopy" )

    if GetClipboardText <> "Text from external clipboard!" then
        Warnlog "External Clipboard not available!"
        Warnlog "Is: " & GetClipboardText
    end if
    Call wTypeKeys("<End><Return>")
    SetClipboard ""
    SetClipboard "2nd Text from external clipboard!"
    try
        PrintLog "Edit / Paste / Special"
        hUseAsyncSlot( "EditPasteSpecialWriter" )

        Kontext "InhaltEinfuegen"
        if ( InhaltEinfuegen.Exists( 2 ) ) then
            try
                Auswahl.Select 1
            catch
                Warnlog "Unable to select entry in format list !"
                InhaltEinfuegen.Close
                Call hCloseDocument
                goto endsub
            endcatch
            InhaltEinfuegen.OK
            Wait(200)
            Kontext "Active"
            if Active.Exists then
                if Active.GetRT = 304 then
                    Warnlog "Maybe Bug#109331"
                    Warnlog Active.Gettext
                    try
                        Active.Ok
                    catch
                        Active.No
                    endcatch
                end if
            else
                Call wTypeKeys("<Home><Shift End>")
                hUseAsyncSlot( "EditCopy" )
                if GetClipboardText <> "2nd Text from external clipboard!" then Warnlog "Wrong Text inserted! Is: " & GetClipboardText
            end if
        else
            Warnlog "Dialog 'Paste Special' is not up!"
        end if
    catch
        Exceptlog
    endcatch

    PrintLog "  - Internal Clipboard with StarOffice Writer-Text"
    PrintLog "Enter some text in writer and copy it"
    Call wTypeKeys("<Return>Text from internal clipboard<Shift Home>")
    hUseAsyncSlot( "EditCopy" )
    Call wTypeKeys("<End><Return>")
    hUseAsyncSlot( "EditPasteSpecialWriter" )
    
    PrintLog "Edit / Paste / Special"
    Kontext "InhaltEinfuegen"
    if ( InhaltEinfuegen.Exists( 2 ) ) then
        DialogTest ( InhaltEinfuegen )
        Auswahl.Select 1
        InhaltEinfuegen.OK
        Call wTypeKeys "<Home><Shift End>"
        hUseAsyncSlot( "EditCopy" )
        if GetClipboardText <> "Text from internal clipboard" then Warnlog "Wrong Text inserted! Is: " & GetClipboardText
    else
        Warnlog "Dialog 'Paste Special' is not up!"
    end if
    
    Call hCloseDocument
    PrintLog "Close active document"
    
endcase

'----------------------------------------------------

testcase tEditSelectionMode

    PrintLog "    - Edit - Selection Mode"
    PrintLog "    Open new writer document"
    Call hNewDocument

    PrintLog "    Try to switch to / activate the Block-selectionmode."
    if ( hUseAsyncSlot( "EditSelectionModeBlock" ) = -1 ) then
        warnlog "    Could not activate Blockmode via Edit-menu."
    endif

    PrintLog "    Try to switch to / activate the Standard-selectionmode."
    if ( hUseAsyncSlot( "EditSelectionModeStandard" ) = -1 ) then
        warnlog "    Could not activate Blockmode via Edit-menu."
    endif

    PrintLog "    Close active document."
    Call hCloseDocument

endcase 'tEditSelectionMode

'----------------------------------------------------

testcase tEditChanges

    PrintLog "- Edit/Changes"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if
    
    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "Enter some text in document"
    Call wTypeKeys("Hallo<Return><Return>")

    PrintLog "Select Edit / Changes / Record"
    hUseAsyncSlot( "EditChangesRecord" )

    PrintLog "Enter some more text in document"
    Call wTypeKeys("Aenderung 1<Return><Up><Up><End>")
    Call wTypeKeys("<Backspace>", 4)

    PrintLog "Select Edit / Changes / Record"
    hUseAsyncSlot( "EditChangesRecord" )

    PrintLog "Select Edit / Changes / Show"
    hUseAsyncSlot( "EditChangesShow" )

    PrintLog "Select Edit / Changes / Show"
    hUseAsyncSlot( "EditChangesShow" )

    PrintLog "Select Edit / Changes / Protect Trace"
    hUseAsyncSlot( "EditChangesProtectTrace" )

    PrintLog "Passwort-dialog has to come up, cancel it"
    Kontext "Passwort"
    hCloseDialog( Passwort, "close" )

    PrintLog "Edit / Changes / Accept or Reject"
    EditChangesAcceptOrReject

    PrintLog "Dialog 'Accept or reject changes' has to come up"
    Kontext "Redlining"
    if Redlining.Exists then
        PrintLog "Step through all Tabpages"
        TabControl.SetPage TabListe
        Call DialogTest ( Redlining )
        TabControl.SetPage TabFilter
        Call DialogTest ( Redlining )
        Redlining.Close
        PrintLog "Close dialog"
    else
        Warnlog "Dialog 'Redlining' not opened!"
    end if

    Call wTypeKeys("<Down><Right>")

    PrintLog "Point cursor into a changed text in document"
    PrintLog "Edit / Changes / Comment"
    if ( hUseAsyncSlot( "EditChangesComment" ) = -1 ) then
        Call wTypeKeys("<Up>")
        if ( hUseAsyncSlot( "EditChangesComment" ) = -1 ) then
            Warnlog "Unable to execute 'Edit / Changes / Comment!"
        endif
    endif
    
    Kontext "Kommentar"
    PrintLog "Dialog to comment a change has to come up"
    if ( Kommentar.Exists( 5 ) ) then
        Call DialogTest ( Kommentar )
        Kommentar.Cancel
    else
        Warnlog "Dialog 'Comment' not opened!"
        PrintLog "Close dialog 'Comment'"
    end if
    
    PrintLog "Close active document"
    Call hCloseDocument

endcase

'----------------------------------------------------

testcase tEditCompareDocument

    PrintLog "- Edit/Compare Document"

    dim sFile as string
        sFile = gTesttoolPath & "writer\required\input\bt.sxw"

    if ( gApplication <> "WRITER" ) then
        printlog( "Test only designed for WRITER, skipping" )
        goto endsub
    endif

    call hNewDocument()

    printlog( "Open test document")    
    EditCompareDocumentCalc

    Kontext "OeffnenDlg"
    printlog( "File Open dialog" )
    if ( OeffnenDlg.exists( 2 ) ) then

        printlog( "Load file: " & sFile )
        Dateiname.SetText ConvertPath ( sFile )
        Oeffnen.Click()

        Kontext "FilterAuswahl"
        printlog( "Check for filterselection dialog (comes up e.g. if file is broken)" )
        if Filterauswahl.Exists( 2 ) then
            warnlog( "Dialog <Filterauswahl> was not expected at this point" )
            Filterauswahl.OK()
        else
            printlog( "No dialog <Filterauswahl>, good" )
        endif
        
        Kontext "Active"
        if Active.Exists( 1 ) then
            warnlog( "Unexpected messagebox displayed:" )
            printlog( Active.getText() )
            Active.Ok()
        else
            printlog( "No unexpected messageboxes, good." )
        endif

        Kontext "Redlining"
        PrintLog "Dialog 'Accept and Reject changes' has to come up"
        if Redlining.Exists( 1 ) then
            TabControl.SetPage TabListe
            wait 200
            Kontext "Redlining"
            TabControl.SetPage TabFilter
            Kontext "Redlining"
            hCloseDialog( Redlining, "close" )
        else
            Warnlog "- Dialog 'Redlining' does not exist!"
        endif
    else
        warnlog( "Dialog <FileOpen> did not open")
    endif

    PrintLog "Close active document"
    Call hCloseDocument
    
endcase

'----------------------------------------------------

testcase tEditFindAndReplace
    PrintLog "- Edit/Find & Replace"

    PrintLog "Open new document"
    Call  hNewDocument
    EditSearchAndReplace
    PrintLog "Edit / Find and Replace"
    Kontext "FindAndReplace"
    if Attributes.IsVisible = False then
        More.Click
    end if

    Attributes.Click
    PrintLog "in 'Find&Replace' dialog click 'Attributess'"
    Kontext "Attribute"
    Call DialogTest ( Attribute )
    PrintLog "Cancel dialog 'Attributess'"
    Attribute.Cancel

    Kontext "FindAndReplace"
    Call DialogTest ( FindAndReplace )
    PrintLog "In Find&Replace dialog uncheck 'Match case'"
    SimilaritySearch.UnCheck
    PrintLog "Click 'Format'"
    Formats.Click
    PrintLog "In dialog 'Text format' step throug all tabpages"

    Kontext
    active.SetPage TabFont
    kontext "TabFont"
    Call DialogTest ( TabFont )
    Kontext
    active.SetPage TabFontEffects
    kontext "TabFontEffects"
    Call DialogTest ( TabFontEffects )

    Kontext
    active.SetPage TabEinzuegeUndAbstaende
    kontext "TabEinzuegeUndAbstaende"
    Call DialogTest ( TabEinzuegeUndAbstaende )

    Kontext
    Active.SetPage TabAusrichtungAbsatz
    Kontext "TabAusrichtungAbsatz"
    Call DialogTest ( TabAusrichtungAbsatz )

    Kontext
    active.SetPage TabTextfluss
    kontext "TabTextfluss"
    Call DialogTest ( TabTextfluss )

    Kontext
    active.SetPage TabHintergrund
    kontext "TabHintergrund"
    Call DialogTest ( TabHintergrund )
    TabHintergrund.Cancel
    PrintLog "Close 'Text Format' dialog with cancel"
    Wait 300

    Kontext "FindAndReplace"
    PrintLog "In Find&Replace' select 'Similarity Search'"
    SimilaritySearch.Check
    PrintLog "The Searchbutton should be enabled, click it"
    SimilaritySearchFor.Click
    wait 300

    Kontext "Aehnlichkeitssuche"
    DialogTest ( Aehnlichkeitssuche )
    Aehnlichkeitssuche.Cancel

    wait 300
    PrintLog "Close 'Similarity Search' button with cancel"
    Kontext "FindAndReplace"
    SimilaritySearch.Uncheck
    More.Click
    FindAndReplace.Close

    Call hCloseDocument
    PrintLog "Close active document"
endcase

'----------------------------------------------------

testcase tViewNavigator
    printlog "- Edit/Navigator"
    Dim ToolboxOk as boolean

    Call hNewDocument
    PrintLog "Open new document"

    if gApplication = "MASTERDOCUMENT" then
        Kontext "NavigatorGlobalDoc"
        if Not NavigatorGlobalDoc.Exists then ViewNavigator
        Kontext "NavigatorGlobalDoc"
        Call DialogTest ( NavigatorGlobalDoc )

        printlog " - Umschalten"
        GlobaldokumentToolbox.Click Umschalten
        Sleep 5
        kontext "NavigatorWriter"

        try
            Toolbox.TearOff Navigation
            Kontext "NavigationsFenster"
            NavigationsFenster.Close
        catch
            Warnlog "Unable to Tear off Navigation Toolbox!"
        endcatch

        Kontext "NavigatorWriter"
        Umschalten.Click

        Kontext "NavigatorGlobalDoc"
        printlog( "Click <Edit>" )
        if ( hClickButton( Bearbeiten ) = -1 ) then
            warnlog( "Unable to click <Edit> button, cannot switch Navigator-Mode" )
        endif

        Kontext "Navigator"
        hCloseDialog( Navigator, "close" )
        Call hCloseDocument
        PrintLog "Close active document"
        goto endsub
    end if

    Kontext "NavigatorWriter"
    if Not NavigatorWriter.Exists then ViewNavigator
    Kontext "NavigatorWriter"
    PrintLog "Open Navigator"
    Call DialogTest ( NavigatorWriter )

    printlog "  - Navigation"
    ToolboxOK = True
    try
        PrintLog "Tear Off Toolbox from Navigator"
        Toolbox.TearOff Navigation
    catch
        Warnlog "Unable to tear off Toolbox (#i32092)"
        ToolboxOk = False
    endcatch

    if ToolboxOK = True then
        Kontext "NavigationsFenster"
        wait 200
        try
            PrintLog "Close Navigator Toolbox"
            NavigationsFenster.Close
        catch
        endcatch
    end if

    Kontext "NavigatorWriter"
    wait 200
    PrintLog "  - In Navigator window click 'Previous page'"
    VorherigeSeite.Click
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.No
            catch
                Active.Ok
            endcatch
        end if
    end if
    wait 200

    Kontext "NavigatorWriter"
    PrintLog "- In Navigator window click 'Next page'"
    NaechsteSeite.Click
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.No
            catch
                Active.Ok
            endcatch
        end if
    end if
    wait 300

    printlog "  - switch Selection box off and on"
    Kontext "NavigatorWriter"
    if Auswahlliste.Exists then Auswahlbox.Click
    wait 200
    Auswahlbox.Click
    wait 200
    if Auswahlliste.NotExists then
        Warnlog "Navigator not maximized"
        Auswahlbox.Click
    end if

    printlog "  - Switch Content View on and off"
    Inhaltsansicht.Click
    wait 200
    Inhaltsansicht.Click
    wait 200

    printlog "  - Set Note"
    Merker.Click
    wait 200

    printlog "  - jump to header"
    Kopfzeile.Click
    wait 200

    printlog "  - jump to footer"
    Fusszeile.Click
    wait 200

    printlog "  - to Anchor <-> Text"
    AnkerText.Click
    wait 200

    Kontext "Navigator"
    try
        PrintLog "Close Navigator"
        Navigator.Close
    catch
        Errorlog
        ViewNavigator
    endcatch

    PrintLog "Close active document"
    Call hCloseDocument
endcase

'---------------------------------------------------

testcase tEditAutotext
    Dim j as integer
    Dim i as integer
    Dim CurrentEntryNr as integer
    Dim unxAutotextCategorie as integer

    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "Edit / Autotext"
    EditAutotext
    Kontext "AutoText"
    DialogTest ( AutoText )

    Kontext "AutoText"
    Printlog "If it exists: Delete DummyAutotext."
    if wDeleteAutotext("DummyAutotext") = true then
        qaerrorlog "DummyAutotext from previous testrun found, now deleted."
    end if

    PrintLog "Click Categories"
    Kontext "Autotext"
    Bereiche.Click
    
    Kontext "BereicheBearbeitenAutoText"
    if BereicheBearbeitenAutoText.Exists( 2 ) then
        DialogTest ( BereicheBearbeitenAutoText )
        PrintLog "Create a new categorie"
        Bereich.SetText "Hallo"
        if Not Umbenennen.IsEnabled then
            if gNetzInst = False then
                Warnlog "- Not able to rename Categories, Button 'Rename' disabled!"
            end if
        end if

        if Neu.IsEnabled then
            Neu.Click
            Wait (200)

            printlog( "Click <Delete>" )
            if ( hClickButton( Loeschen ) = -1 ) then
                warnlog( "Unable to delete category, the button is disabled" )
            endif
        else
            if gNetzInst = False then
                Warnlog "- Category cannot be created!"
            end if
        end if
        BereicheBearbeitenAutoText.Cancel
    else
        Warnlog "- Dialog to edit Categories not up!"
    end if
    PrintLog "Close 'Edit Categories with 'Cancel'"

    Kontext "AutoText"
    PrintLog "In Autotext dialog click 'Path..'"
    Pfade.Click
    Kontext "PfadeAuswaehlen"
    DialogTest ( PfadeAuswaehlen )
    Kontext "PfadeAuswaehlen"
    PrintLog "On 'Select Paths' dialog click 'add'"
    Hinzufuegen.Click
    
    Kontext "OeffnenDlg"
    if OeffnenDlg.Exists( 2 ) then
        PrintLog "Cancel File-Open dialog"
        hCloseDialog( OeffnenDlg, "cancel" )
    else
        Warnlog "Dialog ' Select Paths' not up!"
    end if
    
    Kontext "PfadeAuswaehlen"
    PrintLog "Cancel dialog 'Select Paths'"
    hCloseDialog( PfadeAuswaehlen, "cancel" )

    Kontext "AutoText"
    PrintLog "We have to create an autotext first"
    PrintLog "to assure the 'Autotext'-Button enabled"
    printlog "First close all categories"
    Liste.TypeKeys "<Home>"
    for i = 1 to Liste.GetItemCount
        Liste.TypeKeys "-"
        Liste.TypeKeys "<Down>"
    next i

    unxAutotextCategorie = 1
    printlog "choose category and remember entry number"
    Liste.TypeKeys "<Home>"
    for i = 1 to Liste.GetItemCount
        if Not Menue.IsEnabled then
            Liste.TypeKeys "<Down>"
            unxAutotextCategorie = unxAutotextCategorie + 1
        end if
    next i
    Autotext.Close
    Call wTypeKeys ( "Create a dummy autotext" )
    Call wTypeKeys ( "<Shift Home>" )
    EditAutotext
    Kontext "AutoText"
    WaitSlot(3000)
    Liste.Select unxAutotextCategorie
    NameText.Settext "DummyAutotext"
    Kuerzel.Settext "dAt"
    if Menue.IsEnabled(1) then
        Menue.Click
        wait 500
        try
            Call hMenuSelectNr ( 1 )  'New
        catch
            ' This is a workaround as sometimes menu isn't opened for some reason
            Menue.Click
            wait 500
            try
                Call hMenuSelectNr ( 1 )
            catch
                Warnlog "Unable to create autotext"
                Autotext.Close
                Call hCloseDocument
                goto endsub
            endcatch
        endcatch
        Autotext.Close
        Call wTypeKeys "<Home>"
    else
        Warnlog "Unable to create autotext"
        Autotext.Close
        Call hCloseDocument
        goto endsub
    end if

    printlog "Select created autotext"
    EditAutotext
    WaitSlot (2000)
    Kontext "Autotext"
    Liste.Select unxAutotextCategorie
    Liste.TypeKeys "+"
    Liste.TypeKeys "<Down>"
    wait (500)

    if Menue.IsEnabled(1) then
        Menue.Click
        wait 300
        try
            PrintLog "Select Autotext / Rename"
            Call hMenuSelectNr ( 2 )  'Rename
            wait 500
        catch
            ' This is a workaround as sometimes menu isn't opened for some reason
            Menue.Click
            wait 500
            try
                Call hMenuSelectNr ( 2 )
                wait 300
            catch
                Warnlog "Unable to rename autotext"
                Autotext.Close
                Call hCloseDocument
                goto endsub
            endcatch
        endcatch
    else
        Warnlog "Unable to rename autotext"
        Autotext.Close
        Call hCloseDocument
        goto endsub
    end if

    Kontext "TextbausteineUmbenennen"
    if TextbausteineUmbenennen.Exists(5) then
        Call DialogTest ( TextbausteineUmbenennen )
        PrintLog "Cancel 'Rename Autotext'"
        TextbausteineUmbenennen.Cancel
        Wait (500)
    else
        Warnlog "Dialog 'Rename Autotext' not up!"
    end if

    Kontext "AutoText"
    Menue.Click
    wait (500)
    try
        PrintLog "Select Autotext / Macro"
        Call hMenuSelectNr ( 5 )
        ' if no JRE is installed a messagebox appears
        Do
            j = j + 1
            Kontext "Active"
            if Active.Exists then
                if Active.GetRT = 304 then
                    if j = 1 then Warnlog Active.Gettext
                    Active.Ok
                else
                    exit do
                end if
            else
                exit do
            end if
        Loop

        ' Dialog needs some time to be opened
        Kontext "MakroZuweisen"
        printlog( "Close <MakroZuweisen> dialog" )
        if ( MakroZuweisen.exists( 5 ) ) then
            call Dialogtest( MakroZuweisen )
            hCloseDialog( MakroZuweisen, "close" )
        else
            warnlog "AutoText-Macro - Assign Macro took longer than 5 seconds to load. Check why."
        endif
    catch
        Warnlog "Menu Autotext->'Macro' disabled !"
        Call hMenuClose
    endcatch

    Kontext "AutoText"
    Menue.Click
    wait 500
    try
        PrintLog "Select Autotext / Edit"
        Call hMenuSelectNr ( 4 )
        Sleep 1
        PrintLog "Enter some text in document and close it"
        Call wTypeKeys("Hallo")
        if GetDocumentCount = 1 then
            Warnlog "The autotext has been edited in current document!"
        else
            hUseAsyncSlot( "FileClose" )            
            Kontext "Active"
            PrintLog "A message that module has been changed should come up"
            if Active.Exists then
                Active.No
            else
                Warnlog "Though changing the text module the document could be closed without a query!"
            end if
        end if
    catch
        Warnlog "Menu Autotext->'Edit' disabled !"
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                Warnlog Active.Gettext
                try
                    Active.No
                catch
                    Active.Ok
                endcatch
            end if
        end if
    endcatch

    PrintLog "Close 'Autotext' dialog"
    Kontext "Autotext"
    if Autotext.Exists then Autotext.Close
    if wDeleteAutotext("DummyAutotext") = false then
        Warnlog "Unable to delete created autotext"
    end if
    Kontext "Autotext"
    if Autotext.Exists then Autotext.Close

    PrintLog "Close active document"
    Call hCloseDocument
endcase

'----------------------------------------------------------------

testcase tEditHyperlink
    Printlog "- Edit / Hyperlink"

    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "Insert / Hyperlink"
    InsertHyperlink

    kontext "HyperlinkDialog"
    if ( HyperlinkDialog.exists( 2 ) ) then

        PrintLog "Insert / Hyperlink"
        Call wEinfuegenHyperlink(1)

        Kontext "TabHyperlinkInternet"
        printlog( "Check <Internet>" )
        Internet.Check()

        PrintLog "Enter URL "
        ZielUrl.Settext "http://www.sonor.de"

        Kontext "Hyperlink"
        PrintLog "Click 'Apply'"
        Uebernehmen.Click

        kontext "HyperlinkDialog"
        PrintLog "Close Hyperlink dialog"
        hClickButton( CloseButton )
    else
        warnlog( "Dialog <Hyperlink> did not open" )
        hCloseDocument()
        goto endsub
    endif

    hSetDocumentContext()

    PrintLog "Check if correct URL has been set to document"
    if ( hUseAsyncSlot( "EditCopy" ) > -1 ) then
        if GetClipBoardtext = "" then
            Warnlog "No Hyperlink or wrong hyperlink in document!" & GetClipBoardtext
        end if
    else
        warnlog( "Slot <EditCopy> could not be executed. Most likely the hyperlink was not selected" )
    endif

    PrintLog "Set cursor into link"
    Call wTypeKeys("<Left>")

    PrintLog "Edit / Hyperlink has to be enabled"
    if ( hUseAsyncSlot( "EditHyperlink" ) > -1 ) then
        kontext "HyperlinkDialog"
        hClickButton( CloseButton )
    else
        warnlog( "Slot <EditHyperlink> could not be called" )
    endif

    PrintLog "Close active document"
    Call hCloseDocument
endcase

' ----------------------------------------------------------------

testcase tEditFields

    PrintLog( "Edit/Fields: Edit Fields dialog: " & gApplication )
    dim iCurrentType as integer

    PrintLog( "Open new document" )
    Call hNewDocument()

    printlog( "Insert->Field->Other" )
    hUseAsyncSlot( "InsertFieldsOther" )

    ' Note that the tabdialog does not have a unique ID yet, so we treat it as "Active"
    kontext
    if ( Active.exists( 2 ) ) then

        printlog( "Switch to <Document> tab" )
        Active.setPage( TabDokumentFeldbefehle )

        kontext "TabDokumentFeldbefehle"
        printlog( "Select sender item from the user data, insert the field, close the dialog" )
        if ( TabDokumentFeldbefehle.isVisible() ) then

            ' Search for the sender item in the "Type" listbox by counting the
            ' number of items in the "Select" box. Only the sender item has
            ' more than 12 (usually 15) selectable options.
            ' This is done to make sure that we have an enabled "Edit..." button
            ' in the "Edit Fields" dialog called later in this test
            printlog( "Search for suitable field type" )
            for iCurrentType = 1 to FeldTyp.getItemCount()
                FeldTyp.select( iCurrentType )
                if ( Auswahl.isEnabled() and Auswahl.getItemCount() > 12 ) then
                    exit for
                endif
            next iCurrentType

            hClickButton( Einfuegen )
            hCloseDialog( TabDokumentFeldbefehle , "close" )

            PrintLog( "Move cursor infront of field (start of document)" )
            Call wTypeKeys("<Home>")

            PrintLog( "Edit / Fields" )
            hUseAsyncSlot( "EditFields" )

            Kontext "FeldbefehlBearbeitenDokument"
            if ( FeldbefehlBearbeitenDokument.exists( 2 ) ) then
                Call DialogTest ( FeldbefehlBearbeitenDokument )

                PrintLog( "In 'Edit Fields' dialog select 'Edit'" )
                hClickButton( Bearbeiten )

                Kontext "TabBenutzerdaten"
                if ( TabBenutzerdaten.exists( 2 ) ) then
                    printlog( "<TabBenutzerdaten> is open. Good." )
                    call DialogTest( TabBenutzerdaten )
                    hCloseDialog( TabBenutzerdaten, "cancel" )
                else
                    warnlog( "<TabBenutzerdaten> did not open" )
                endif
            else
                warnlog( "<TabDokumentFeldbefehle> is not visible" )
            endif

            PrintLog( "Close dialog" )
            Kontext "FeldbefehlBearbeitenDokument"
            hCloseDialog( FeldbefehlBearbeitenDokument, "cancel" )
        else
            warnlog( "<FeldbefehlBearbeitenDokument> did not open" )
        endif
    else
        warnlog( "Unable to open <InsertFieldsOther>")
    endif

    PrintLog( "Close active document" )
    Call hCloseDocument()

endcase

'----------------------------------------------------------------

testcase tEditFieldsInputList
    PrintLog "- Edit / Fields / Functions"
    If gApplication = "HTML" then goto endsub

    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "Select Insert / Fields Other"
    InsertFieldsOther
    PrintLog "Goto Tabpage Functions and select 2nd entry (Input list)"
    Kontext
    Active.Setpage TabFunktionen
    Kontext "TabFunktionen"
    Sleep 1
    Feldtyp.Select 2
    if NewListEntry.IsVisible and NewListEntry.IsEnabled then
        PrintLog "Add 1 entry to list"
        if NewListEntry.IsEnabled then
            NewListEntry.Settext "MyNewListEntry"
            wait 200
            if AddListEntry.IsEnabled= True then AddListEntry.Click
        end if
        PrintLog "Close Fields dialog"
        if EntrysList.GetItemCount <> 1 then
            Warnlog "Entry has not been added to list!"
            Schliessen.Click
        else
            Einfuegen.Click
            Wait 500
            Schliessen.Click
            Call wTypeKeys "<Home>"
            try
                Call wTypeKeys "<Mod1 Shift F9>"
                wait 200
                Kontext "EditInputList"
                if EditInputList.Exists then
                    Call DialogTest ( EditInputList )
                    EditInputList.Cancel
                else
                    Warnlog "Dialog 'Edit Input list' is not up!"
                end if
            catch
                Warnlog "Unable to 'Edit / Fields'!"
            endcatch
        end if
    else
        Warnlog "'Input list' probably not found!"
    end if

    PrintLog "Close active document"
    Call hCloseDocument
endcase

'----------------------------------------------------------------

testcase tEditFootnote
    PrintLog "- Edit/Footnote"

    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "Insert / Footnote"
    InsertFootnote
    Kontext "FussnoteEinfuegen"
    FussnoteEinfuegen.OK

    Call wTypeKeys("<PageUp><Left>")

    PrintLog "Edit / Footnote"
    EditFootnote

    Kontext "FussnoteBearbeiten"
    Call DialogTest ( FussnoteBearbeiten )
    PrintLog "Close 'Footnote' dialog"
    FussnoteBearbeiten.Cancel

    PrintLog "Close active document"
    Call hCloseDocument
endcase

'----------------------------------------------------------------

testcase tEditIndexEntry
    
    PrintLog "- Edit/Index Entry"
    if gApplication = "HTML" then
        printlog "Not in WriterWeb!"
        goto endsub
    end if

    PrintLog "Open new document"
    Call hNewDocument

    PrintLog "Write down something in document"
    Call wTypeKeys("Ich werde ein Verzeichniseintrag")
    Call wTypeKeys("<Left>")
    InsertIndexesEntry
    PrintLog "Insert / Indexes / Entry"
    Kontext "VerzeichniseintragEinfuegen"
    try
        PrintLog "Close dialog"
        EintragOk.Click
    catch
        Warnlog "Index entry couldn't be inserted!(Button disabled)"
    endcatch
    EinfuegenSchliessen.Click

    Call wTypeKeys("<End><Left>")
    try
        PrintLog "Point cursor in Index entry"
        EditIndexEntry
        PrintLog "Edit / Index entry"
        Kontext "VerzeichniseintragBearbeiten"
        Call DialogTest (VerzeichniseintragBearbeiten)
        BearbeitenSchliessen.Click
    catch
        Warnlog "Not able to edit index entry!"
    endcatch

    Call hCloseDocument
    PrintLog "Close active document"
endcase

'----------------------------------------------------------------

testcase tEditLinks
    PrintLog "- Edit/Links"

    dim sFile as string
        sFile = gTesttoolPath & "writer\required\input\graphics\jolink.jpg"

    if ( gApplication = "HTML" ) then
        printlog "Not in WriterWeb!"
        goto endsub
    end if

    printlog( "Open a new document" )
    Call hNewDocument

    printlog( "Insert graphic file (linked): " & sFile )
    Call hGrafikVerknuepftEinfuegen ( sFile )

    printlog( "Edit->Links" )
    if ( hUseAsyncSlot( "EditLinksWriter" ) = -1 ) then
        Warnlog "Error on calling Edit / Links ! (disabled)"
        Call hCloseDocument
        goto endsub
    endif

    Kontext "VerknuepfungenBearbeiten"
    if ( VerknuepfungenBearbeiten.Exists( 1 ) ) then
        Call DialogTest ( VerknuepfungenBearbeiten )

        Kontext "VerknuepfungenBearbeiten"
        printlog( "Click <Change>" )
        if ( hClickButton( Aendern ) > -1 ) then
            Kontext "GrafikEinfuegenDlg"
            call DialogTest( GrafikEinfuegenDlg )
            hCloseDialog( GrafikEinfuegenDlg, "cancel" )
        else
            Warnlog "Error modifying the link!"
        endif

        Kontext "VerknuepfungenBearbeiten"
        printlog( "Click <Update>" )
        if ( hClickButton( Aktualisieren ) = -1 ) then
            Warnlog "Error on updating the link!"
        endif

        Kontext "VerknuepfungenBearbeiten"
        printlog( "Click <Release>" )
        if ( hClickButton( Loesen ) > -1 ) then
            Kontext "Active"
            printlog( "Check for confirmation message" )
            if ( Active.Exists( 1 ) ) then
                printlog( "Message displayed, good. Closing with <No>" )
                printlog( Active.getText() )
                Active.No
            else
                Warnlog( "The <Are you sure you want to remove the selected link?> message is missing" )
            endif
        else
            Warnlog( "Unable to break the link to the file" )
        endif

        Kontext "VerknuepfungenBearbeiten"
        hCloseDialog( VerknuepfungenBearbeiten, "close" )
    else
        Warnlog "Dialog Edit / Links not opened!"
    end if
    PrintLog "Close active document"
    Call hCloseDocument
endcase

'------------------------------------------------------

testcase tEditPlugIn
    PrintLog "- Edit/PlugIn"
    PrintLog "Open new document"
    Call hNewDocument
    PrintLog "Edit / Plugin"
    EditPlugIn
    WaitSlot (2000)
    PrintLog "Edit / Plugin"
    EditPlugIn
    PrintLog "Close active document"
    Call hCloseDocument
endcase

'------------------------------------------------------

testcase tEditImageMap

     PrintLog "- Edit / ImageMap"

    dim sFile as string
        sFile = gTesttoolPath & "writer\required\input\graphics\jolink.jpg"
   
    PrintLog "Open new document"
    Call hNewDocument

    printlog( "Insert graphics file: " & sFile )
    if ( hGrafikEinfuegen( sFile ) ) then

        printlog( "Edit->Imagemap" )
        EditImageMap

        Kontext "ImageMapEditor"
        if ( ImageMapEditor.exists( 1 ) ) then
            Call DialogTest (ImageMapEditor)
            hCloseDialog( ImageMapEditor, "close" )

            printlog( "Check for <Imagemap changed> messagebox (should not appear)" )
            kontext "Active"
            if ( Active.exists( 1 ) ) then
                qaerrorlog( "#i109605# - <imagemap changed> message displayed." )
                printlog( Active.getText )
                Active.no()
            else
                printlog( "No messagebox on close, good." )
            endif

        else
            warnlog( "Dialog <ImageMapEditor> did not open" )
        endif
    else
        warnlog( "Insert graphics failed" )
    end if

    PrintLog "Close active document"
    Call hCloseDocument
endcase

'------------------------------------------------------

testcase tEditObjectEdit
    PrintLog "Edit / Object / Edit"

    PrintLog "Open new document"
    Call hNewDocument

    InsertObjectChart
    Call gMouseClick (99,1)
    Call gMouseClick (99,1)
    Call gMouseClick (99,1)
    wait 200
    Call wTypeKeys("<SHIFT F4>")
    try
        EditObjectEdit
    catch
        warnlog "Edit/Object/Edit is disabled with chart added"
    endcatch

    PrintLog "Close active document"
    Call hCloseDocument
endcase

'----------------------------------------------------------------

testcase tMasterDocEditNavigator
    printlog "- View/Navigator"
    Dim ToolboxOk as boolean

    gApplication = "MASTERDOCUMENT"
    Call hNewDocument
    PrintLog "Open new document"

    Kontext "NavigatorGlobalDoc"
    if Not NavigatorGlobalDoc.Exists then ViewNavigator
    Kontext "NavigatorGlobalDoc"
    Call DialogTest ( NavigatorGlobalDoc )

    printlog " - Umschalten"
    GlobaldokumentToolbox.Click Umschalten
    Sleep 5
    kontext "NavigatorWriter"

    try
        Toolbox.TearOff Navigation
        Kontext "NavigationsFenster"
        NavigationsFenster.Close
    catch
        Warnlog "Unable to Tear off Navigation Toolbox!"
    endcatch

    Kontext "NavigatorWriter"
    Umschalten.Click
    Kontext "NavigatorGlobalDoc"
    Sleep (1)

    printlog "  - Bearbeiten"
    try
        Bearbeiten.Click
    catch
        Sleep (2)
        try
            Bearbeiten.Click
        catch
            Warnlog " Either taking more than three seconds to switch Navigator-modes, or something has gone wrong."
        endcatch
    endcatch
    wait (300)

    Kontext "Navigator"
    Navigator.Close

    PrintLog "Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'---------------------------------------------------
