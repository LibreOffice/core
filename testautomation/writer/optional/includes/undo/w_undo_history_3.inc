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
'* short description : Writer Undo-History - Test
'*
'\***********************************************************************

sub w_undo_history_3

    Call tUndoSection			' Section (Insert/Modify/Delete)
    Call tUndoHyperlink			' Hyperlink (Insert as Text/as URL button)
    Call tUndoFootnote			' Footnote (Insert/Modify/Delete)
    Call tUndoCaption			' Insert Caption
    Call tUndoAutoText			' Insert Autotext
    Call tUndoBookmark			' Bookmark (Insert/Rename/Delete)
    Call tUndoComment			' Comment (Insert/Delete)
    Call tUndoScript			' Script (Insert/Delete)
    Call tUndoIndexes			' Index/Bibliography entrys (Insert/Delete)
    Call tUndoFrames			' Frames (Insert/Rename/Delete)
    Call tUndoTableGeneral			' Tables Part 1
    Call tUndoTableOptionsMerge			' Tables Part 2
    Call tUndoTableConvertSort			' Tables Part 3
    Call tUndoTableSplitDelete			' Tables Part 4
    Call tUndoHorizontalRuler			' Horizontal ruler
    Call tUndoGraphics			' Graphics (Insert/Replace/Delete)

end sub

testcase tUndoSection

    '/// <B>Section (Insert/Modify/Delete)</B>
    printlog "Section (Insert)"
    '/// new document
    Call hNewDocument
    '/// Insert a section
    InsertSection
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    TabBereiche.Ok
    '///  string in Undo-list has to be: "Insert section")
    Call CheckUndoStringInUndoList ( 10, "InsertSection" )

    printlog "Section (Modify)"
    '/// Open navigator and select section
    Call wNavigatorAuswahl ( 7, 1, true )
    Kontext "NavigatorWriter"
    '/// Open contextmenu and choose 'Edit'
    Auswahlliste.OpenContextMenu
    wait 500
    Call hMenuSelectNr(4)

    '/// Change Background-Color
    Kontext "BereicheBearbeiten"
    Optionen.Click
    Kontext
    Active.Setpage TabHintergrund
    Kontext "TabHintergrund"
    Hintergrundfarbe.TypeKeys "<Down>", 5
    TabHintergrund.Ok

    Kontext "BereicheBearbeiten"
    BereicheBearbeiten.Ok
    '///  string in Undo-list has to be: "Modify section")
    Call CheckUndoStringInUndoList ( 10, "ModifySection" )

    printlog "Section (Delete)"
    '/// Open navigator and select section
    Call wNavigatorAuswahl ( 7, 1, true )
    Kontext "NavigatorWriter"
    '/// Open contextmenu and choose 'Edit'
    Auswahlliste.OpenContextMenu
    wait 500
    Call hMenuSelectNr(4)
    '/// In 'Edit sections' dialog choose 'remove'
    Kontext "BereicheBearbeiten"
    Aufheben.Click
    BereicheBearbeiten.Ok
    '///  string in Undo-list has to be: "Delete section")
    Call CheckUndoStringInUndoList ( 10, "DeleteSection" )

    '/// Close Document
    Call hCloseDocument

endcase


testcase tUndoHyperlink

    '///<B>Hyperlink (Insert as Text/as URL button)</B>
    printlog "- Insert hyperlink as text"
    '/// new document
    Call hNewDocument
    '/// Insert / Hyperlink as Text
    InsertHyperlink
    '///  string in Undo-list has to be: "Insert Hyperlink")
      kontext "HyperlinkDialog"
        if ( HyperlinkDialog.exists( 2 ) ) then
            Call wEinfuegenHyperlink(1)
            ZielURL.SetText "www.sun.com"
            Form.Select 1
            Uebernehmen.Click()

            kontext "HyperlinkDialog"
            HyperlinkDialog.Close()
        else
            warnlog( "Unable to open <HyperlinkDialog>" )
        endif
    Call CheckUndoStringInUndoList ( 10, "InsertHyperlink" )

    Call wTypeKeys "<Return>"

    '/// Insert / Hyperlink as Button
    printlog "- Insert hyperlink as Button"
    InsertHyperlink

        kontext "HyperlinkDialog"
        if ( HyperlinkDialog.exists( 2 ) ) then
            Call wEinfuegenHyperlink(1)
            ZielURL.Settext "www.sun.com"
            Form.Select 2
            Kontext "Hyperlink"
            Uebernehmen.Click
            kontext "HyperlinkDialog"
            HyperlinkDialog.Close
        else
            warnlog( "Unable to open <HyperlinkDialog>" )
        endif

    '///  string in Undo-list has to be: "insert URL Button")
    Call CheckUndoStringInUndoList ( 10, "InsertHyperlinkButton" )

    '/// Close Document
    Call hCloseDocument

endcase


testcase tUndoFootnote

    '/// <B>Footnote (Insert/Modify/Delete)</B>
    printlog "- Footnote (Insert)"
    '/// new document
    Call hNewDocument
    '/// Insert / Footnote
    InsertFootnote
    Kontext "FussnoteEinfuegen"
    FussnoteEinfuegen.Ok
    '///  string in Undo-list has to be: "Insert footnote")
    Call CheckUndoStringInUndoList ( 10, "InsertFootnote" )

    printlog "- Footnote (Modify)"
    '/// Tools/Footnote
    ToolsFootnote
    Kontext
    Active.Setpage TabFussnoten
    Kontext "TabFussnoten"

    '/// Change any option in footnote dialog
    if Seitenende.IsChecked then
        Dokumentende.Check
    else
        Seitenende.Check
    end if
    '/// Close dialog
    TabFussnoten.Ok
    '///  string in Undo-list has to be: "Modify footnote options")
    Call CheckUndoStringInUndoList ( 10, "ModifyFootnote" )

    printlog "- Footnote (Delete)"
    '/// Point cursor directly to the footnote anchor
    Call wTypeKeys "<Mod1 Home>"
    '/// press 'delete' to remove footnote
    Call wTypeKeys "<Delete>"
    '///  string in Undo-list has to be: "Delete footnote options")
    Call CheckUndoStringInUndoList ( 10, "DeleteFootnote" )

    '/// Close document
    Call hCloseDocument
endcase


testcase tUndoCaption

    '/// <B>Insert a caption</B>
    printlog "- Insert Caption"
    '/// new document
    Call hNewDocument
    '/// Insert a graphic
    Call hGrafikEinfuegen ( gTesttoolPath & "writer\optional\input\graphics\jolink.jpg" )
    '/// Insert / Caption
    InsertCaption
    Kontext "Beschriftung"
    '/// Insert a caption more than 20 characters long
    Beschriftungstext.Settext "This caption has more than 20 characters"
    Beschriftung.Ok

    '///  string in Undo-list has to be: "Insert Caption: This capt...aracters")
    Call CheckUndoStringInUndoList ( 10, "InsertCaption" )

    '/// Close document
    Call hCloseDocument
endcase


testcase tUndoAutoText

    '/// <B>Insert autotext</B>
    printlog "- Insert autotext"
    '/// new document
    Call hNewDocument
    '/// Insert dummy text
    Call wBlindtextEinfuegen

    '///  string in Undo-list has to be: "Insert Auto Text")
    Call CheckUndoStringInUndoList ( 10, "InsertAutotext" )

    '/// Close document
    Call hCloseDocument
endcase


testcase tUndoBookmark

    Warnlog "#i102310# - tUndoBookmark outcommented due to bug."
    goto endsub


    '/// <B>Bookmark (Insert/Rename/Delete)</B>
    printlog "- Insert Bookmark"
    '/// new document
    Call hNewDocument
    '/// Insert / Bookmark
    InsertBookmark
    Kontext "TextmarkeEinfuegen"
    Textmarken.Settext "I'm a bookmark"
    TextmarkeEinfuegen.Ok
    '///  string in Undo-list has to be: "Insert Bookmark: I'm a bookmark")
    Call CheckUndoStringInUndoList ( 10, "InsertBookmark" )

    printlog "- Rename Bookmark"
    '/// Open navigator and select bookmark
    Call wNavigatorAuswahl ( 6, 1, true )
    Kontext "NavigatorWriter"
    '/// Open contextmenu and choose 'Edit'
    Auswahlliste.OpenContextMenu
    wait 500
    Call hMenuSelectNr(4)
    wait 500
    Call hMenuSelectNr(3)
    wait 500
    Kontext "ObjekteUmbenennen"
    Objektname.Settext "a book"
    ObjekteUmbenennen.Ok
    '///  string in Undo-list has to be: "Rename Bookmark: a book")
    Call CheckUndoStringInUndoList ( 10, "RenameBookmark" )

    printlog "- Delete Bookmark"
    '/// Open navigator and select bookmark
    Call wNavigatorAuswahl ( 6, 1, true )
    Kontext "NavigatorWriter"
    '/// Open contextmenu and choose 'Edit'
    Auswahlliste.OpenContextMenu
    wait 500
    Call hMenuSelectNr(4)
    wait 500
    Call hMenuSelectNr(2)
    wait 500
    '///  string in Undo-list has to be: "Rename Bookmark: a book")
    Call CheckUndoStringInUndoList ( 10, "DeleteBookmark" )

    '/// Close document
    Call hCloseDocument
endcase


testcase tUndoComment

    printlog "New document"
    Call hNewDocument
    printlog "Insert / Comment"
    InsertNoteWriter
    printlog "String in Undo-list has to be: 'Insert Comment'"
    Call CheckUndoStringInUndoList ( 10, "InsertComment" )
    printlog "- Delete Comment"
    printlog "Bring up the Navigator, find the entry 'Comments', expand it, and select the expanded section."
    Call wNavigatorAuswahl( 11, 1 )
    kontext "Navigator"
    Navigator.TypeKeys "<DELETE>"
    '/// Close the Navigator
    ViewNavigator
    '///  string in Undo-list has to be: "Delete comment")
    Call CheckUndoStringInUndoList ( 10, "DeleteComment" )

    '/// Close document
    Call hCloseDocument
endcase


testcase tUndoScript

    '/// <B>Script (Insert/Delete)</B>
    printlog "- Insert Script"
    '/// new document
    Call hNewDocument
    '/// Insert / Script
    InsertScript
    Kontext "ScriptEinfuegen"
    ScriptEinfuegen.Ok
    '///  string in Undo-list has to be: "Insert script")
    Call CheckUndoStringInUndoList ( 10, "InsertScript" )

    '/// Point cursor directly to the script
    Call wTypeKeys "<Mod1 Home>"
    '/// press 'delete' to remove script
    Call wTypeKeys "<Delete>"
    '///  string in Undo-list has to be: "Delete script")
    Call CheckUndoStringInUndoList ( 10, "DeleteScript" )

    '/// Close document
    Call hCloseDocument
endcase


testcase tUndoIndexes

    '/// <B>Indexes (Insert/Delete)</B>
    printlog "- Insert Index entry"
    '/// new document
    Call hNewDocument
    '/// Type a word
    Call wTypeKeys "IndexEntry"
    Call wTypeKeys "<Shift Home>"
    '/// Insert / Indexes and Tables / Entry
    InsertIndexesEntry
    Kontext "VerzeichnisEintragEinfuegen"
    EintragOk.Click
    VerzeichnisEintragEinfuegen.Close
    '///  string in Undo-list has to be: "Insert index entry")
    Call CheckUndoStringInUndoList ( 10, "InsertIndexEntry" )

    printlog "delete index entry with 'Backspace'"
    Call wTypeKeys ( "<Backspace>"
    '///  string in Undo-list has to be: "Delete index entry")
    Call CheckUndoStringInUndoList ( 10, "DeleteIndexEntry" )

    '/// Type a word
    Call wTypeKeys "BibliographyEntry"
    Call wTypeKeys "<Shift Home>"
    printlog "- Insert Bibliography entry"
    printlog "Insert / Index / Bibliography entry"
    InsertIndexesBibliographyEntry
    Kontext "LiteratureintragEinfuegen"
    EintragEinfuegen.Click
    LiteratureintragEinfuegen.Close
    '///  string in Undo-list has to be: "Insert bibliography entry")
    Call CheckUndoStringInUndoList ( 10, "InsertBibliographyEntry" )

    Printlog "delete bibliography entry with 'Backspace'"
    Call wTypeKeys ( "<Backspace>"
    '///  string in Undo-list has to be: "Delete bibliography entry")
    Call CheckUndoStringInUndoList ( 10, "DeleteBibliographyEntry" )

    '/// Close document
    Call hCloseDocument
endcase


testcase tUndoFrames

    warnlog( "#111909# - Cannot access <rename> item in context menu" )
    goto endsub

    '/// <B>Frames (Insert/Rename/Delete)</B>
    printlog "- Insert Frame"
    '/// new document
    Call hNewDocument
    '/// Insert / Frame
    InsertFrame
    Kontext
    Active.Setpage TabZusaetze
    Kontext "TabZusaetze"
    ObjektName.Settext "MyFrame"
    TabZusaetze.Ok
    '///  string in Undo-list has to be: "Insert frame: MyFrame")
    Call CheckUndoStringInUndoList ( 10, "InsertFrame" )

    printlog "- Rename Frame"
    '/// Open navigator and select frame
    Call wNavigatorAuswahl ( 3, 1, true )
    Kontext "NavigatorWriter"
    '/// Open contextmenu and choose 'Edit'
    Auswahlliste.OpenContextMenu
    Call hMenuSelectNr(4)
    Call hMenuSelectNr(3)
    Kontext "ObjekteUmbenennen"
    Objektname.Settext "MyRenamedFrame"
    ObjekteUmbenennen.Ok
    '///  string in Undo-list has to be: "Rename frame: 'MyFrame' -> 'MyRenamedFrame'")
    Call CheckUndoStringInUndoList ( 10, "RenameFrame" )
    printlog "- Delete Frame"
    '/// Type 'Del' to delete the frame
    Call wTypeKeys "<Delete>"
    '///  string in Undo-list has to be: "Delete frame: 'MyRenamedFrame'")
    Call CheckUndoStringInUndoList ( 10, "DeleteFrame" )

    '/// Close document
    Call hCloseDocument
endcase


testcase tUndoTableGeneral
    Dim sTableName as string

    '/// <B>Tables Part 1</B>
    printlog "- insert table"
    '/// new document
    Call hNewDocument
    '/// Insert a table
    InsertTableWriter
    Kontext "TabelleEinfuegenWriter"
    sTableName = "MyTable"
    Tabellenname.Settext sTableName
    TabelleEinfuegenWriter.Ok
    '///  string in Undo-list has to be: "Insert table")
    Call CheckUndoStringInUndoList ( 10, "InsertTable" )
    printlog "- Insert a row"
    '/// <B>Insert a row</B>
    Call hToolbarSelect("Table", true)
    ZeileEinfuegen.Click
    '///  string in Undo-list has to be: "Insert row")
    Call CheckUndoStringInUndoList ( 10, "InsertRow" )
    printlog "- Insert a column"
    '/// <B>Insert a column</B>
    Call hToolbarSelect("Table", true)
    SpalteEinfuegen.Click
    '///  string in Undo-list has to be: "Insert column")
    Call CheckUndoStringInUndoList ( 10, "InsertColumn" )
    printlog "- Delete a row"
    '/// <B>Delete a row</B>
    Call hToolbarSelect("Table", true)
    ZeileLoeschen.Click
    '///  string in Undo-list has to be: "Delete row")
    Call CheckUndoStringInUndoList ( 10, "DeleteRow" )
    printlog "- Delete a column"
    '/// <B>Delete a column</B>
    Call hToolbarSelect("Table", true)
    SpalteLoeschen.Click
    '///  string in Undo-list has to be: "Delete column")
    Call CheckUndoStringInUndoList ( 10, "DeleteColumn" )
    printlog "- format a cell"
    '/// <B>Format cell (enable number recognition form that)</B>
    ToolsOptions
    Call hToolsOptions ("WRITER","Table")
    if Not AutomatischeZahlenerkennung.IsChecked then AutomatischeZahlenerkennung.Check
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Cancel

    '/// type '1' in a cell
    Call wTypeKeys ( "1" )
    Call wTypeKeys ( "<Right>" )
    '///  string in Undo-list has to be: "Format cell")
    Call CheckUndoStringInUndoList ( 10, "FormatCell" )

    ToolsOptions
    Call hToolsOptions ("WRITER","Table")
    if AutomatischeZahlenerkennung.IsChecked then AutomatischeZahlenerkennung.UnCheck
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Cancel

    '/// CloseDocument
    Call hCloseDocument
endcase


testcase tUndoTableOptionsMerge

    '/// <B>Tables Part 2</B>
    printlog "- apply table attributes"
    '/// new document
    Call hNewDocument
    '/// Insert a table
    InsertTableWriter
    Kontext "TabelleEinfuegenWriter"
    TabelleEinfuegenWriter.Ok

    printlog "- apply attributes"
    '/// Format / Table
    FormatTable
    '/// change some table options
    Kontext
    Active.Setpage TabTextflussTabelle
    Kontext "TabTextflussTabelle"
    if Umbruch.IsChecked then
        Umbruch.Uncheck
    else
        Umbruch.Check
    end if
    TabTextflussTabelle.Ok
    '///  string in Undo-list has to be: "Apply table attributes")
    Call CheckUndoStringInUndoList ( 10, "ApplyTableAttributes" )

    printlog "- merge tables"
    '/// <B>Merge tables</B>
    Call wTypeKeys ( "<Down>" ,2 )
    '/// Insert a second table
    InsertTableWriter
    Kontext "TabelleEinfuegenWriter"
    TabelleEinfuegenWriter.Ok
    '/// Format / Merge  / Tables
    FormatMergeTables
    '///  string in Undo-list has to be: "Merge table")
    Call CheckUndoStringInUndoList ( 10, "MergeTable" )

    '/// close Document
    Call hCloseDocument
endcase


testcase tUndoTableConvertSort

    '/// <B>Tables Part 3</B>
    printlog "- Convert to text"
    '/// new document
    Call hNewDocument
    '/// Insert text "This<Return>is a<Return>table to text<Return>text"
    Call wTypeKeys ("This<Return>is a<Return>table to text<Return>text")
    '/// select all
    Call wTypeKeys ( "<Mod1 A>" )
    '/// Format / Text <-> Table
    ToolsTextInTable
    Kontext "TextInTabelleUmwandeln"
    TextInTabelleUmwandeln.OK
    '///  string in Undo-list has to be: "Convert table -> text")
    Call CheckUndoStringInUndoList ( 10, "ConvertTable" )

    printlog "- Sort table"
    '/// <B>Sort table</B>
    Call wTypeKeys ( "<Mod1 A>", 2 )
    '/// Select all -> Tools / Sort
    ToolsSort
    Kontext "Sortieren"
    Sortieren.Ok
    '///  string in Undo-list has to be: "Sort table")
    Call CheckUndoStringInUndoList ( 10, "SortTable" )

    '/// Close Document
    Call hCloseDocument
endcase


testcase tUndoTableSplitDelete

    '/// <B>Tables Part 4</B>
    printlog "- Split tables"
    '/// new document
    Call hNewDocument
    '/// Insert 2 tables
    InsertTableWriter
    Kontext  "TabelleEinfuegenWriter"
    TabelleEinfuegenWriter.Ok
    Call wTypeKeys ( "<Down>", 3 )
    InsertTableWriter
    Kontext  "TabelleEinfuegenWriter"
    TabellenName.Settext "AnotherTable"
    TabelleEinfuegenWriter.Ok
    '/// MergeTables
    FormatMergeTables
    wait 500
    '/// Split tables
    FormatSplitTable
    Kontext "TabelleAuftrennen"
    TabelleAuftrennen.Ok
    '///  string in Undo-list has to be: "Split table")
    Call CheckUndoStringInUndoList ( 10, "SplitTable" )

    '/// <B>Delete table</B>
    printlog "- Delete table"
    '/// Open navigator and select table
    Call wNavigatorAuswahl ( 2, 1, true )
    Kontext "NavigatorWriter"
    '/// Open contextmenu and choose 'Delete'
    Auswahlliste.OpenContextMenu
    Call hMenuSelectNr(4)
    Call hMenuSelectNr(2)
    '///  string in Undo-list has to be: "Delete table: 'AnotherTable'")
    Call CheckUndoStringInUndoList ( 10, "DeleteTable" )

    '/// Close document
    Call hCloseDocument
endcase


testcase tUndoHorizontalRuler

    '/// <B>HorizontalRuler</B>
    printlog "- Horizontal ruler"
    '/// new document
    Call hNewDocument
    '/// Insert / Horizontal Line
    InsertHorizontalLine
    '/// Select one line
    Kontext "HorizontaleLinieEinfuegen"
    Auswahl.TypeKeys "<Down>", 4
    HorizontaleLinieEinfuegen.Ok
    '///  string in Undo-list has to be: "Insert horizontal ruler")
    Call CheckUndoStringInUndoList ( 10, "InsertRuler" )

    '/// close document
    Call hCloseDocument
endcase


testcase tUndoGraphics

    '/// <B>Graphics (Insert/Delete/Replace)</B>
    printlog "- Insert graphics from file"
    '/// new document
    Call hNewDocument
    '/// Insert a graphic from file
    Call hGrafikEinfuegen ( gTesttoolPath & "writer\optional\input\graphics\jolink.jpg" )
    '///  string in Undo-list has to be: "Insert graphics")
    Call CheckUndoStringInUndoList ( 10, "InsertGraphic" )

    '/// <B>Delete graphic</B>
    printlog "- delete graphic"
    '/// press 'delete' to remove graphic
    Call wTypeKeys "<Delete>"
    '///  string in Undo-list has to be: "Delete graphics")
    Call CheckUndoStringInUndoList ( 10, "DeleteGraphic" )

    '/// <B>Insert graphic from gallery</B>
    printlog "- insert graphic from gallery"
    Kontext "Gallery"
    if Gallery.NotExists(2) then ToolsGallery
    Sleep (2)

    View.OpenContextMenu
    Call hMenuSelectNr(1)
    Call hMenuSelectNr(1)
    '///  string in Undo-list has to be: "Insert graphics")
    Call CheckUndoStringInUndoList ( 10, "InsertGraphic" )
    Sleep (1)

    '/// <B>Replace graphic</B>
    printlog "- replace graphic"
    '/// Insert another graphics from gallery
    Kontext "Gallery"
    View.OpenContextMenu
    Call hMenuSelectNr(1)
    Call hMenuSelectNr(1)
    '///  string in Undo-list has to be: "Replace graphics")
    Call CheckUndoStringInUndoList ( 10, "ReplaceGraphic" )

    '/// Close gallery
    Kontext "Gallery"
    if Gallery.Exists then ToolsGallery

    '/// Close document
    Call hCloseDocument
endcase
