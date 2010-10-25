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
'* Owner : wolfram.garten@oracle.com 
'*
'* short description :
'*
'\******************************************************************************
sub id_002

    printLog Chr(13) + "---------    id_002    ----------"

    call tiEditUndoRedo
    call tiEditRepeat
    call tiEditCutPasteCopySelectall
    call tiEditPasteSpecial
    call tiEditSearchAndReplace
    call tiEditDuplicate
    call tEditPoints
    call tiEditFields
    call tdEditDeleteSlide
    call tiEditLinks
    call tiEditImageMap
    call tiEditObjectProperties
    call tiEditObjectEdit
    call tiEditPlugIn
    call tiEditHyperlink
end sub
'-------------------------------------------------------------------------
testcase tiEditUndoRedo

    printlog " open application"
    printlog " Type a sentence into the document and do :"
    printlog " Edit / Undo and Edit / Redo"
    hNewDocument
    call hTBOtypeInDoc

    EditUndo
    WaitSlot (2000)
    EditRedo
    WaitSlot (2000)
    printlog " close application"
    Call hCloseDocument
    
endcase 'tiEditUndoRedo
'---------------------------------------------------------
testcase tiEditRepeat

    goto endsub 'Quaste, ask FHA
    printlog " open application"
    Call hNewDocument

printlog " create 3 rectangles"
    gMouseClick 50,50
    Call hRechteckErstellen ( 30, 10, 70, 30 )
    WaitSlot (1000)
    Call hRechteckErstellen ( 20, 20, 60, 40 )
    WaitSlot (1000)
    Call hRechteckErstellen ( 80, 50, 40, 20 )
    WaitSlot (1000)
    printlog " [Modify | ContextMenu]->Arrange->SendBackward"
    ContextArrangeBringBackward
    WaitSlot (2000)
    printlog " Edit->Repeat"
    try
        EditRepeat
    catch
        Warnlog " Menu entry is disabled #i26129#"
    endcatch

    printlog " close application"
    Call hCloseDocument
    
endcase 'tiEditRepeat
'---------------------------------------------------------
testcase tiEditCutPasteCopySelectall

    printlog " open application"
    printlog " Type a sentence into the dokument and do : "
    printlog " edit-> (Cut | Paste | Copy | SelectAll | Cut | Paste | DeleteContents) "
    Call hNewDocument

    call hTBOtypeInDoc

    hUseAsyncSlot( "EditCut" )
    hUseAsyncSlot( "EditPaste" )
    hUseAsyncSlot( "EditCopy" )
    hUseAsyncSlot( "EditPaste" )
    hUseAsyncSlot( "EditSelectAll" )
    hUseAsyncSlot( "EditCut" )
    hUseAsyncSlot( "EditPaste" )
    hUseAsyncSlot( "EditDeleteContents" )

    printlog " close application"
    Call hCloseDocument
    
endcase 'tiEditCutPasteCopySelectall
'---------------------------------------------------------
testcase tiEditPasteSpecial

    printlog " open application"
    Call  hNewDocument

    printlog " put some words into the clipboard "
    SetClipboard "This is a Text in the Clipboard"

    printlog " Edit->PasteSpecial"
    hUseAsyncSlot( "EditPasteSpecial" )

    Kontext "InhaltEinfuegen"
    if ( InhaltEinfuegen.exists( 2 ) ) then
        DialogTest ( InhaltEinfuegen )

        printlog " close the dialog without inserting something"
        hCloseDialog( InhaltEinfuegen, "Cancel" )
    else
        warnlog( "Dialog <InhalteEinfuegen> did not open" )
    endif

    printlog " close application"
    Call hCloseDocument
    
endcase 'tiEditPasteSpecial
'---------------------------------------------------------
testcase tiEditSearchAndReplace

    printlog " open application"
    Call  hNewDocument

    try
        printlog " Edit->Find & Replace"
        EditSearchAndReplace
        WaitSlot (1000)
        Kontext "FindAndReplace"
            DialogTest ( FindAndReplace )

            printlog " click on More Options"
            More.Click
            printlog " check 'similarity search' "
            SimilaritySearch.Check ' culprint for errors if not resetted !
            WaitSlot (1000)
            printlog " click '...' "
            SimilaritySearchFor.Click
            Kontext "Aehnlichkeitssuche"
                DialogTest (Aehnlichkeitssuche )
                Aehnlichkeitssuche.Cancel
                printlog " close dialog 'similarity search' "
        Kontext "FindAndReplace"
            printlog " UNcheck 'similarity search' "
            SimilaritySearch.UnCheck
            printlog " click on More Options"
            More.Click
            printlog " close dialog 'Find & Replace'"
            FindAndReplace.Close
    catch
        Warnlog "EditSearchAndReplace caused an error"
    endcatch
    printlog " close application"
    Call hCloseDocument
    
endcase 'tiEditSearchAndReplace
'---------------------------------------------------------
testcase tiEditDuplicate

    printlog " open application "
    Call hNewDocument
    printlog " create a rectangle"
    call hTBOtypeInDoc
    printlog " 'Edit->Select All' "
    EditSelectAll
    printlog " Edit->Duplicate "
    EditDuplicate

    Kontext "Duplizieren"
        Call DialogTest ( Duplizieren )
        printlog " cancel dialog 'duplicate'"
        Duplizieren.Cancel

    Call hCloseDocument
    printlog " close application"
    
endcase 'tiEditDuplicate
'---------------------------------------------------------
testcase tEditPoints

    printlog " open application "
    Call hNewDocument
    printlog " create a rectangle"
    call hTBOtypeInDoc
    printlog " 'Edit->Points'"
    FormatEditPoints
    printlog " 'Edit->Glue Points'"
    EditGluePoints
    printlog " close application"
    Call hCloseDocument
    
endcase 'tEditPoints
'---------------------------------------------------------
testcase tiEditFields

    printlog " open application "
    Call hNewDocument
    WaitSlot (2000)
    call sSelectEmptyLayout()
    printlog " insert something to be able to use this function"
    printlog "+ Insert->Fields->Date (fixed)"
    printlog "+ deselect object, select, editmode "
    InsertFieldsDateFix
    WaitSlot (1000)
    gMouseDoubleClick 10,10

    hTypeKeys "<ESCAPE>"
    printlog "With a Tab we catch always the Object"
    hTypeKeys "<Tab>",3
    printlog "Here we enter Edit-Mode and therefore also the right place"
    hTypeKeys "<F2>"
    printlog "Here we enter Edit-Mode and therefore also the right place"
    hTypeKeys "<Home>"

    try
        printlog " Edit->Fields"
        EditFieldsDraw
        Kontext "FeldbefehlBearbeitenDraw"
            Call DialogTest ( FeldbefehlBearbeitenDraw )
            printlog " cancel dialog 'Edit->Field' "
            FeldbefehlBearbeitenDraw.Close
    catch
        Warnlog "- Slot could not be accessed"
    endcatch

printlog " close application"
    Call hCloseDocument
    
endcase 'tiEditFields
'--------------------------------------------------------
testcase tdEditDeleteSlide

printlog " open application "
    Call hNewDocument
printlog " Insert->Slide"
    InsertSlide
    WaitSlot (2000)
    hTypekeys "<Pagedown>"
    WaitSlot (2000)
    printlog " check with navigator, if slide is inserted "
    printlog "  check state of navigator ! expected: closed "
    Kontext "Navigator"
    sleep (2)
    if Navigator.exists then
        printlog "Navigator: open :-)"
    else
        printlog "Navigator: NOT available :-( Will be opened now!"
        ViewNavigator
    end if
    WaitSlot (2000)
    Kontext "NavigatorDraw"
    if Liste.GetItemCount<>2 Then
        Warnlog "-  No slide inserted"
        Kontext "Navigator"
        Navigator.Close
        Call hCloseDocument
        goto endsub
    else
        Liste.Select 2
         printlog " Press Return to go to the selected slide"
        Liste.Typekeys "<RETURN>"  
        Kontext "Navigator"
        Navigator.Close
    end if
    WaitSlot (2000)
    printlog " Edit->Delete Slide"
    EditDeleteSlide
    WaitSlot (2000)
    printlog " close application "
    Call hCloseDocument
    
endcase 'tdEditDeleteSlide
'---------------------------------------------------------
testcase tiEditLinks

    printlog " open application "
    Call  hNewDocument

    printlog " Insert a linked graphic : global\input\graf_inp\stabler.tif"
    InsertGraphicsFromFile
    Kontext "GrafikEinfuegenDlg"
        try
            if Link.Exists then
                Link.Check
            else
                Warnlog "- Link in Insert graphic is not working"
            end if
            Dateiname.settext Convertpath (gTesttoolPath + "global\input\graf_inp\stabler.tif")
            Oeffnen.Click
            Kontext "Messagebox"
                if Messagebox.Exists=True Then
                    Warnlog Messagebox.GetText
                    Messagebox.Ok
                end if
                InsertGraphicsFromFile
                Kontext "GrafikEinfuegenDlg"
                    Link.Check
                    printlog " Insert a linked graphic : global\input\graf_inp\desp.bmp"
                    Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\desp.bmp")
                    Oeffnen.Click
                    sleep 2
                    kontext "Messagebox"
                        if Messagebox.Exists then
                            Warnlog Messagebox.GetText
                            Messagebox.OK
                            sleep 1
                        end if
        catch
            Warnlog "Insert graphic caused errors"
        endcatch

        WaitSlot (2000)
    try
        printlog " Edit->Links "
        EditLinksDraw
        WaitSlot (2000)
        Kontext "VerknuepfungenBearbeiten"
            Call DialogTest ( VerknuepfungenBearbeiten )
            printlog " close dialog 'Edit Links' "
            VerknuepfungenBearbeiten.Close
            WaitSlot (1000)
    catch
        Warnlog "- EditLinks could not be executed, could be the graphic was not imported"
    endcatch

    printlog " close application "
    Call hCloseDocument
    
endcase 'tiEditLinks
'---------------------------------------------------------
testcase tiEditImageMap

    printlog " open application "
    Call  hNewDocument

    printlog " Edit->Image Map "
    EditImageMap
    Kontext "ImageMapEditor"
        sleep (1)
        if ImageMapEditor.Exists (2) then
            printlog "- ImageMap exists"
            DialogTest ( ImageMapEditor )
            try
                printlog " Close dialog 'Image Map Editor' "
                ImageMapEditor.Close
                Printlog "ImageMap closed using the close button"
            catch
                EditImageMap
                Printlog "ImageMap closed using menue 'edit-imagemap'"
            endcatch
        else
            warnlog "ImageMap didn't come up!"
        end if
    printlog " close application "
    Call  hCloseDocument
    
endcase 'tiEditImageMap
'---------------------------------------------------------
testcase tiEditObjectProperties

    dim i as integer
    printlog " for every object, there are several edit options: "
    printlog "+ starting with FloatingFrame->Properties "
    printlog "+ file used for input in frame: global\input\graf_inp\desp.bmp"

    printlog " open application "
    Call hNewDocument

    printlog " Insert->Floating Frame "
    InsertFloatingFrame
    WaitSlot (2000)

    Kontext "TabEigenschaften"
        FrameName.SetText "Hello"
        printlog " insert an picture into the frame (embedded into draw application"
        Inhalt.SetText ConvertPath ( gTesttoolpath + "global\input\graf_inp\desp.bmp" )
        WaitSlot (2000)
        TabEigenschaften.OK
        WaitSlot (2000)
    gMouseDoubleClick 1,1

    printlog " select frame "
    hTypekeys "<tab>",3

    printlog " Edit->Object->Properties "
    kontext
    WaitSlot (2000)
    EditObjectProperties
    WaitSlot (1000)
    Kontext "TabEigenschaften"
        DialogTest ( TabEigenschaften )
        printlog " open Contents File Dialog "
        sleep(1)
        Oeffnen.Click
        Kontext "OeffnenDlg"
            Call DialogTest ( OeffnenDlg )
            printlog " Cancel 'Select File for Floating Frame'"
            OeffnenDlg.Cancel
    Kontext "TabEigenschaften"
        TabEigenschaften.Cancel

    printlog " close application "
    Call hCloseDocument
    
endcase 'tiEditObjectProperties
'---------------------------------------------------------
testcase tiEditObjectEdit

    printlog " OLE: edit & save copy as..."
    printlog " open application "
    dim i as integer
    Call hNewDocument

    printlog " Insert->Object->OLE Object; select the first one in list, usually: 'StarOffice 6.0 Spreadsheet' "
    InsertObjectOLEObject
    WaitSlot (1000)
    Kontext "OLEObjektEinfuegen"
        ObjektTyp.Select 1
        OLEObjektEinfuegen.OK
        WaitSlot (1000)

    gMouseClick 20,1

    printlog " select object "
    hTypekeys "<tab>",3

    printlog " Edit->Object->Edit "
    EditObjectEdit
    printlog "try EditObjectEdit again, to see, if it is in edit mode !"
    WaitSlot (2000)
    try
        ContextNameObject
        warnlog " Couldn't get into edit mode!"
    catch
        printlog "Reached edit mode - ok :-)"
        printlog " exit edit mode, by clicking outside the object "
        gMouseClick 20,1
    endcatch

    printlog " select object "
    hTypekeys "<tab>",3

    printlog " Edit->Object->Save Copy As "
    EditObjectSaveCopyAs
    Kontext "SpeichernDlg"
        Call DialogTest ( SpeichernDlg )
        printlog " cancel dialog 'Save As' "
        SpeichernDlg.Cancel
        WaitSlot (2000)
        Kontext "Active"
            if Active.Exists(2) then Active.No
    printlog " close application "
    Call hCloseDocument
    
endcase'tiEditObjectEdit
'---------------------------------------------------------
testcase tiEditPlugIn

    printlog " open application "
    Call hNewDocument

    printlog " Insert->Object->Plugin "
    InsertObjectPlugin
    Kontext "PlugInEinfuegen"
    '    DialogTest ( PlugInEinfuegen)
        Durchsuchen.click
        Kontext "OeffnenDlg"
            '      Call DialogTest ( OeffnenDlg )
            if OeffnenDlg.exists (5) then
                OeffnenDlg.Cancel
            else
                warnlog "Open file dialog didn't come up"
            end if
            WaitSlot (5000)
    Kontext "PlugInEinfuegen"
        if PlugInEinfuegen.exists then
            printlog " insert a '.mov' file : graphics\required\input\sample.mov" 
            DateiUrl.SetText (ConvertPath ( gTesttoolpath + "graphics\required\input\sample.mov" ))
            printlog " Type something into the option field, and delete it "

            Optionen.SetText "Fiddler's Green"
            Optionen.TypeKeys "<HOME>"
            Optionen.TypeKeys "<SHIFT><END>"
            Optionen.TypeKeys "<delete>"
            PlugInEinfuegen.Ok
        else
            warnlog "Insert plugin isn't visible"
        end if
    WaitSlot (5000)
    kontext "Messagebox"
        if Messagebox.exists (5) then
            warnlog "Messagebox: " + Messagebox.gettext
            Messagebox.ok
        end if
    printlog " Edit->Plugin "
    EditPlugIn
    printlog "Editplugin works!"

    printlog " close application "
    Call hCloseDocument
    
endcase 'tiEditPlugIn
'---------------------------------------------------------
testcase tiEditHyperlink

    printlog " open application"
    hNewDocument
    Call sSelectEmptyLayout
    printlog " Insert->Hyperlink"
    InsertHyperlink

    kontext "HyperlinkDialog"
    if ( HyperlinkDialog.exists( 2 ) ) then

        Kontext "Hyperlink"
        Auswahl.MouseDown 50, 5
        Auswahl.MouseUp 50, 5
        Auswahl.typekeys "<PAGEDOWN><PAGEUP>"
        Auswahl.typekeys "<TAB>"
        sleep 1

        Kontext "TabHyperlinkInternet"
        printlog "Workaround to get rid of a Focusing-problem..."
        NameText.Typekeys "alal <RETURN>"
        NameText.Typekeys "<MOD1 A><DELETE>"
        TabHyperlinkInternet.Typekeys "<TAB>", 6
        TabHyperlinkInternet.Typekeys "<LEFT>", 3
        printlog "End of workaround"

        Internet.Check  
        ZielUrl.SetText( "http://www.liegerad-fahrer.de" )

        Uebernehmen.Click()

        kontext "HyperlinkDialog"
        HyperlinkDialog.Close()
    else
        warnlog( "Failed to open <HyperlinkDialog>" )
    endif
        printlog " presupposition to enable edit->hyperlink: <TAB><F2><STRG+a>"
        hTypeKeys "<TAB>"
        hTypeKeys "<F2>"
        hTypeKeys "<MOD1 SHIFT RIGHT>"
    EditSelectAll
    try
        printlog " Edit->Hyperlink "
        EditHyperlinkDraw

        Kontext "HyperlinkDialog"
        if ( HyperlinkDialog.Exists( 2 ) ) then
            printlog " close dialog 'Hyperlink'"
            HyperlinkDialog.Close()
        else
            Warnlog "- Hyperlinkdialog not up"
        end if
    catch
        Warnlog "- Not able to edit Hyperlink!"
    endcatch

    printlog " close application "
    Call hCloseDocument
    
endcase 'tiEditHyperlink
'---------------------------------------------------------
