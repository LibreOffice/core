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
'* short description : Navigator test
'*
'\***********************************************************************

sub w_navigator
    printLog Chr(13) + "---------   Navigator Functions -   ----------"

    Call tViewNavigator1                ' alle Methoden am Dockingwindow  - Navigator
    Call tViewNavigator2            ' dokument?bergreifenden Methoden am Dockingwindow
    Call tViewNavigator3            ' Funktionen testen
    Call tViewNavigator4            ' Inhaltsansicht des Navigators ?berpr?fen
    Call tViewNavigator5            ' Anspringen von Punkten im Dokument ?ber die Inhaltsansicht
    Call tViewNavigator6            ' Umbenennen von Objekten aus dem Navigator

end sub

' **********************************************************************

testcase tViewNavigator1
    Dim xPos as long : Dim yPos as long: Dim xSize% : Dim ySize%

    '/// Edit / Navigator ( all methods from Navigator as Window ) ///
    printlog "- Edit / Navigator : all methods from Navigator as Window"
    '/// New document ///
    Call hNewDocument
    '/// Call Navigator with 'F5' ///
    printlog "  - Call Navigator with F5"
    call wTypeKeys "<F5>"

    Kontext "Navigator"
    if Navigator.NotExists then
        Warnlog "No Navigator with F5"
        ViewNavigator
    end if

    '/// Move Navigator ///
    printlog "  - move Navigator"
    Navigator.Move 300, 400
    Sleep 1
    Navigator.Move 590, 790
    Sleep 1
    Navigator.Move 10, 10
    Sleep 1

    xPos = Navigator.GetPosX
    yPos = Navigator.GetPosY
    xSize% = Navigator.GetSizeX
    ySize% = Navigator.GetSizeY

    '/// Size Navigator ///
    printlog "  - size Navigator"
    try
		Navigator.Size (300, 400)
	catch
		Warnlog "Sizen Navigator doesn't work.. see #i32449"
	endcatch
    Sleep 1
    try
		Navigator.Size (xSize%, ySize%)
	catch
		Printlog "Sizen Navigator doesn't work.. see #i32449"
	endcatch

    Sleep 1

    '/// Close and reopen Navigator, size should not change ///
    printlog "  - Close Navigator and reopen, the size has to be the same"
    xPos = Navigator.GetPosX
    yPos = Navigator.GetPosY
    xSize% = Navigator.GetSizeX
    ySize% = Navigator.GetSizeY
    ViewNavigator
    Sleep 1
    ViewNavigator
    if xPos <> Navigator.GetPosX then Warnlog "X-Pos. has been changed : Should -> "+ xPos + "  Is -> "+ Navigator.GetPosX
    if yPos <> Navigator.GetPosY then Warnlog "Y-Pos. has been changed : Should -> "+ yPos + "  Is -> "+ Navigator.GetPosY
    if xSize% <> Navigator.GetSizeX then Warnlog "X-Size has been changed : Should -> "+ xSize% + "  Is -> "+ Navigator.GetSizeX
    if ySize% <> Navigator.GetSizeY then Warnlog "Y-Size has been changed : Should -> "+ ySize% + "  Is -> "+ Navigator.GetSizeY

    '/// Close Navigator ///
    try
        Navigator.Close
    catch
        Warnlog "Unable to close Navigator!"
    endcatch
    '/// Close document ///
    Call hCloseDocument
endcase

' **********************************************************************

testcase tViewNavigator2
    Dim xPos% : Dim yPos% : Dim xSize% : Dim ySize%
    Dim datgeht as boolean
    if gApplication <> "WRITER" then
        goto endsub
    endif
    '/// Edit / Navigator ( document-spreading methods with Navigator as Window ) ///
    printlog "- Edit / Navigator : document-spreading methods with Navigator as Window"
    '/// New document ///
    Call hNewDocument
    printlog "  - Call Navigator in Document and check in another"
    '/// Insert some text in first document ///
	call wTypeKeys "Navigator test document 1"	

	'///  Edit / Navigator ///
    Kontext "Navigator"
    if Navigator.NotExists then ViewNavigator
    '/// New document ///
    Call hNewDocument
	'/// Insert some text in second document ///
	call wTypeKeys "Navigator test document 1"

    Kontext "Navigator"
    '/// Check if Navigator has been opened in new document ///
    if Navigator.NotExists then Warnlog "In new Document is no Navigator"

    '/// Close both documents and reopen one ///
    printlog "  - Close all documents and reopen one"
    Call hCloseDocument
    Sleep 2
    Call hCloseDocument
    Sleep 2
    Call hNewDocument
    '/// Check if Navigator has been opened with document ///
    Kontext "NavigatorWriter"
    if NavigatorWriter.NotExists then Warnlog "In new Document is no Navigator"
	call wTypeKeys "This is a navigator test!"

    '/// Dock Navigator ///
    printlog "  - dock Navigator in Document and check in another"
    Kontext "Navigator"
    Navigator.Dock
    Sleep 1
    '/// New Document ///
    Call hNewDocument
    '/// Check if Navigator is in new document docked ///
    Kontext "Navigator"
    if NOT Navigator.IsDocked then Warnlog "In new Document Navigator is not docked"

    '/// Close all documents reopen a new and check if Navigator is docked ///
    printlog "  - Close all documents reopen a new and check if Navigator is docked"
    hFileCloseAll()

    '/// New document ///
    Call hNewDocument
    hSetDocumentContext()
    try
        DocumentWriter.FadeOut
    catch
        Warnlog "#i35096#Unable to Fade out Navigator!"
    endcatch  
    try
        DocumentWriter.FadeIn
    catch
        Warnlog "#i35096#Unable to Fade out Navigator!"
    endcatch

    Kontext "Navigator"
    if NOT Navigator.IsDocked then Warnlog "In new Document Navigator is not docked"

    printlog "  - Close Navigator with <F5>"
    Navigator.Undock
    Sleep 1
    '/// Undock Navigator and close with 'F5' ///
    call wTypeKeys "<F5>"

    Kontext "Navigator"
    if NOT Navigator.NotExists then
        Warnlog "Navigator is already open"
        try
            Navigator.Close
        catch
            Warnlog "Could not close Navigator!"
        endcatch
    end if
    '/// Close document ///
    Call hCloseDocument
endcase

' **********************************************************************

testcase tViewNavigator3
    QAErrorlog "#i94614# - tViewNavigator3 outcommented"
    goto endsub

    Dim NavigationTearOff as boolean
	'/// Edit / Navigator : check some Navigator-Functions ///
    printlog "- Edit / Navigator : check some Navigator-Functions"
    '/// Open document ( navigator.odt ) ///
    hFileOpenLocally(  gTesttoolpath + "writer\optional\input\navigator\navigator.odt" )
    
    '/// Open Navigator ///
    Kontext "Navigator"
    if Navigator.NotExists then ViewNavigator
    Kontext "NavigatorWriter"
	'/// Enter Pagenumber '5' in Navigator
	Seitennummer.Settext "5"
    printlog "  - Navigation"
    '/// Tear Off Navigation Window ///
    NavigationTearOff = False
	try
		Navigation.TearOff
		NavigationTearOff = true
	catch
		Warnlog "Unable to tear off Navigation Toolbar! (#i32092)"
	endcatch
	if NavigationTearOff = true then
		Sleep 1
		Kontext "NavigationsFenster"
		'/// Move Navigation Window ///
		NavigationsFenster.Move 300, 400
		sleep 1
		'/// Minimize Navigation Window ///
		NavigationsFenster.Minimize
		sleep 1
		'/// Maximize Navigation Window ///
		NavigationsFenster.Maximize
		sleep 1
		Auswahl.MouseDoubleClick ( 45, 20 )
		sleep 1
		'/// Close Navigation Window ///
		NavigationsFenster.Close
	end if
    
	'/// Functions in Navigator ///
    printlog "  - next Page"
    Kontext "NavigatorWriter"

    '/// previous page ///
    printlog "  - previous page"
	VorherigeSeite.Click
	wait 200
	Call gMouseClick ( 20, 30 )
	call wTypeKeys "   <Left>"

	InsertFieldsPageNumbers
	call wTypeKeys "+0<Shift Left><Shift Left><Shift Left>"

	wait 200
	ToolsCalculate
	Sleep 1
	if GetClipboardText <> "4" then QAErrorlog "#i94614#Previous Page isn't  4 but " & GetClipboardText
    Kontext "NavigatorWriter"

    '/// Click Next page 2 times ///
    NaechsteSeite.Click
    wait 200
	NaechsteSeite.Click
	wait 200
    Call gMouseClick 20, 60
    call wTypeKeys "   <Left>"

    InsertFieldsPageNumbers
    call wTypeKeys "+0<Shift Left><Shift Left><Shift Left>"

    wait 200
    ToolsCalculate
    if GetClipboardText <> "6" then QAErrorlog "#i94614#Next Page isn't 6 but " & GetClipboardText
    Kontext "NavigatorWriter"

    printlog "  - switch Selectbox off and on"
    '/// Switsch Select box off and on ///
    if Auswahlliste.Exists then Auswahlbox.Click
    wait 500
    Auswahlbox.Click
    wait 500
    if Auswahlliste.NotExists then
        Warnlog "Navigator hasn't been sized to default size"
        Auswahlbox.Click
    end if
    Kontext "NavigatorWriter"

    printlog "  - Set Reminder"
    '/// Set Reminder ///
    Merker.Click
    wait 500

    printlog "  - jump to header"
    '/// Select Jump to header ///
    Kopfzeile.Click
    wait 500
    call wTypeKeys "<Home><Shift End>"

    EditCopy
    if GetClipboardText <> "Kopfzeile" then Warnlog "Cursorposition wrong: Not in Header"
    Kontext "NavigatorWriter"

    '/// Select Jump to Footer ///
    printlog "  - jump to footer"
    Fusszeile.Click
    wait 500
    call wTypeKeys "<Home><Shift End>"

    EditCopy
    if GetClipboardText <> "Fusszeile" then Warnlog "Cursorposition wrong: Not in Footer"
    Kontext "NavigatorWriter"

    printlog "  - to Anchor <-> Text"
    '/// Select Anchor to Text ///
    AnkerText.Click
    wait 500
    call wTypeKeys "<Right><Shift Mod1 Right><Shift Left>"

    EditCopy
    if GetClipboardText <> "sed" then Warnlog "Cursorposition wrong: Not in Footer"
    wait 500
    Kontext "NavigatorWriter"
    AnkerText.Click
    wait 200
    call wTypeKeys "<Home><Shift End>"

    EditCopy
    if GetClipboardText <> "Fussnote" then Warnlog "Cursorposition wrong: Not in Footer"

    '/// Close Navigator ///
    Kontext "Navigator"
    try
        Navigator.Close
    catch
        Warnlog "Could not close Navigator!"
    endcatch
    '/// Close document ///
    Call hCloseDocument
endcase

' **********************************************************************

testcase tViewNavigator4
    Dim i : Dim j : Dim Ende : Dim k
    Dim AlteGruppe$ : Dim AlterUnterwert$
    Ende=1000
    if gApplication <> "WRITER" then
        goto endsub
    endif
    printlog "- Edit / Navigator : check entry's in Navigator Index"
    '/// File / Open / 'gTesttoolpath + "writer\optional\input\navigator\navigator.odt"' ///
    hFileOpenLocally(  gTesttoolpath + "writer\optional\input\navigator\navigator.odt" )
    
    '/// Open Navigator through Edit / Navigator ///
    ViewNavigator
    Kontext "NavigatorWriter"
    '/// Check all entrys in Navigator and notice the name ///
    for i= 1 to 12
        Call wNavigatorAuswahl(i,0)   ' Gruppe im Navigator
        Kontext "NavigatorWriter"
        printlog "-  " + Auswahlliste.Gettext
        AlteGruppe$ = Auswahlliste.Gettext    ' Gruppe merken
        select case i
            case 1  : Ende = 7          ' Headings does not work ( Testtoolproblem )
            case 2  : Ende = 4          ' Tables
            case 3  : Ende = 1          ' text frame
            case 4  : Ende = 3          ' Graphics
            case 5  : Ende = 0          ' OLE-Objects
            case 6  : Ende = 3          ' Bookmarks
            case 7  : Ende = 0          ' Sections
            case 8  : Ende = 2          ' Hyperlinks
            case 9  : Ende = 0          ' References
            case 10 : Ende = 0          ' Indexes
            case 11 : Ende = 0          ' Notes
            case 12 : Ende = 0          ' Drawing objects
        end select

        Call wNavigatorAuswahl(i,Ende)   ' Eintrag im Navigator
        Kontext "NavigatorWriter"
        '/// Switch in Navigator to Index View ///
        AlterUnterwert$ = Auswahlliste.Gettext   ' Eintrag merken
        Inhaltsansicht.Click
        Auswahlliste.TypeKeys "<Up>", 12
        if Auswahlliste.Gettext <>AlteGruppe$ then
            if i <> 1 then Warnlog "- Wrong group with switching to Index View"
        else
            Auswahlliste.TypeKeys "<Down>",Ende + 1
            if Auswahlliste.Gettext <> AlterUnterwert$ then Warnlog "Not all Subpoints in Index View are visible"
        end if
    next i
    '/// Compare entrys in index view with entrys in Navigator///

    Kontext "Navigator"
    try
        Navigator.Close
    catch
        Warnlog "Could not close Navigator!"
    endcatch
    '/// Close Navigator ///
    Call hCloseDocument
    '/// Close document ///
endcase

' **********************************************************************

testcase tViewNavigator5
    QaErrorLog "#i94614# - tViewNavigator5 outcommented"
    goto endsub
    
    if gApplication <> "WRITER" then
        goto endsub
    endif

    Dim j as integer
    Dim Dummy as String
    Dim Bildname as string
    Dim Abbild as String

    printlog "- Edit / Navigator : Check Jumpmark from Index in Navigator"
    '/// File / Open / 'gTesttoolpath + "writer\optional\input\navigator\navigator.odt"'  ///
    hFileOpenLocally(  gTesttoolpath + "writer\optional\input\navigator\navigator.odt" )
    
    '/// Open Navigator ///
    Kontext "NavigatorWriter"
    if Not NavigatorWriter.Exists then
        ViewNavigator
        Kontext "NavigatorWriter"
    end if

    printlog "  - Header"
    '/// In Navigator switch to Index view ///
	Call wNavigatorAuswahl(1,0)
    Kontext "NavigatorWriter"
	wait 500
	Inhaltsansicht.Click
    '/// In Navigator select 'Headings' and jump to headers in document ///
	wait 500
    Auswahlliste.TypeKeys "<Home>"
	Auswahlliste.TypeKeys "<+>"
    if Auswahlliste.GetItemCount = 8 then
        for j=2 to 6
            Auswahlliste.Select j
            Auswahlliste.TypeKeys "<return>"
            Sleep 1
            call wTypeKeys "<Home><Shift End>"

            EditCopy
            Sleep 1
            '/// Check if the correct header has been selected ///
            if GetClipboardText <> "Heading " & j - 1 then Warnlog "Wrong Jumppoint: Should -> Heading " & j - 1 & "  Is ->  " & GetClipboardText
            Kontext "NavigatorWriter"
        next j
        for j=2 to 3
            Auswahlliste.TypeKeys "<down>"
            Dummy = Auswahlliste.GetText
            Auswahlliste.TypeKeys "<return>"
            wait 200
            call wTypeKeys "<Home><Shift End>"

            EditCopy
            wait 100
            Kontext "NavigatorWriter"
        next j
    else
        Warnlog "There are not 7 but " & Auswahlliste.GetItemCount & " headings in navigator list"
    end if
    '/// Select in Navigator entry 'Tables' and jump via Navigator to tables in document ///
    printlog "  - Tables"
	Call wNavigatorAuswahl(2,0)
	Kontext "NavigatorWriter"
	wait 500
	Inhaltsansicht.Click
	wait 500
	Auswahlliste.TypeKeys "<Home>"
    wait 500
    for j=1 to 4
        Auswahlliste.TypeKeys "<down><return>"
        Sleep 2
        call wTypeKeys "<Home>"
        call wTypeKeys "<Mod1 A>"

        EditCopy
        '/// Check if correct tables has been selected ///
        if GetClipboardText <> "Tabelle" + j then Warnlog "Wrong Cursorposition: Should -> Tabelle" + j + "  Is ->  " + GetClipboardText
        Kontext "NavigatorWriter"
    next j

    '/// Select in Navigator entry 'Text frames' and jump via Navigator to text frames in document ///
    printlog "  - Text frames"
    Call wNavigatorAuswahl(3,0)
	Kontext "NavigatorWriter"
	wait 500
	Inhaltsansicht.Click
	wait 500
    Auswahlliste.TypeKeys "<Home>"
    Auswahlliste.TypeKeys "<Down><Return>"
    Sleep 2
    call wTypeKeys "<Home><Shift End>"

    '/// Check if correct text frames have been selected ///
    try
        EditCopy
        if Left$(GetClipboardText,6) <> "Figura" then Warnlog "Wrong Cursorposition: Should -> Figura" + "  Is ->  "+ Left(GetClipboardText,6)
    catch
        Printlog "Unable to go into the Text frame with testtool"
    endcatch
    Kontext "NavigatorWriter"

    '/// Select in Navigator entry 'Graphics' and jump via Navigator to graphics in document ///
    printlog "  - Graphics"
	Call wNavigatorAuswahl(4,0)
    Kontext "NavigatorWriter"
	wait 500	
	Inhaltsansicht.Click
	wait 500
	Auswahlliste.TypeKeys "<Home>"
    '/// Check if correct graphic have been selected ///
    for j=1 to 3
        Auswahlliste.TypeKeys "<down><return>"
        Sleep 2
        FormatGraphics
        Kontext
        Active.SetPage TabZusaetze
        Kontext "TabZusaetze"
        Bildname = ObjektName.GetText
        select case j
            case 1 : if Bildname <> "Der Baer" then Warnlog "Wrong Graphicname: Not -> 'Der B�r'   But -> "+ Bildname
            case 2 : if Bildname <> "Star Division" then Warnlog "Wrong Graphicname: Not -> 'Star Division'   But -> "+ Bildname
            case 3 : if Bildname <> "Der Hase" then Warnlog "Wrong Graphicname: Not -> 'Der Hase'   But -> "+ Bildname
        end select
        TabZusaetze.Cancel
        Kontext "NavigatorWriter"
    next j

    '/// Select in Navigator entry 'Bookmarks' and jump via Navigator to references in document ///
    printlog "  - Bookmarks"
	Call wNavigatorAuswahl(6,0)
    Kontext "NavigatorWriter"
	wait 500
	Inhaltsansicht.Click
	wait 500
    Auswahlliste.TypeKeys "<Home>"
    for j=1 to 3
        Auswahlliste.TypeKeys "<down><return>"
        Sleep 2
        call wTypeKeys "<Shift Mod1 Right>"

        wait 200
        EditCopy
        '/// Check if correct reference have been selected ///
        select case j
            case 1 : if GetClipboardText <> "rerum" then Warnlog "Wrong Bookmark: Should -> 'rerum'   Is -> "+ GetClipboardText
            case 2 : if GetClipboardText <> "non " then Warnlog "Wrong Bookmark: Should -> 'non'   Is -> "+ GetClipboardText
            case 3 : if GetClipboardText <> "invenietur" then Warnlog "Wrong Bookmark: Should -> 'invenietur'   Is -> "+ GetClipboardText
        end select
        Kontext "NavigatorWriter"
    next j
    wait 500

    '/// Select in Navigator entry 'Hyperlinks' and jump via Navigator to hyperlinks in document ///
    printlog "  - Hyperlinks"
	Call wNavigatorAuswahl(8,0)
    Kontext "NavigatorWriter"
	wait 500
	Inhaltsansicht.Click
	wait 500
    Auswahlliste.TypeKeys "<Home>"
    wait 500
    for j=1 to 2
        Auswahlliste.TypeKeys "<down><return>"
        Sleep 2
        wait 200
        EditCopy
        '/// Check if correct hyperlinks have been selected ///
        select case j
            case 1 : if GetClipboardText <> "offensantes" then Warnlog "Wrong Hyperlink: Should -> 'offensantes'   Is -> "+ GetClipboardText
            case 2 : if GetClipboardText <> "decreverint" then Warnlog "Wrong Hyperlink: Should -> 'decreverint'   Is -> "+ GetClipboardText
        end select
        Kontext "NavigatorWriter"
    next j
    Inhaltsansicht.Click

    '/// Close Navigator ///
    Kontext "Navigator"
    try
        Navigator.Close
    catch
        Warnlog "Could not close Navigator!"
    endcatch
    '/// Close document ///
    Call hCloseDocument
endcase

' **********************************************************************

testcase tViewNavigator6
    Dim j as integer
    Dim datIsderAlte(4) as string

    if gApplication <> "WRITER" then
        goto endsub
    endif
    
    printlog "- Edit / Navigator : Rename Objects"
    '/// File / Open / 'gTesttoolpath + "writer\optional\input\navigator\st1_1.sxw" ' ///
    hFileOpenLocally(  gTesttoolpath + "writer\optional\input\navigator\st1_1.sxw" )
    
    Kontext "Navigator"
    '/// Open Navigator ///
    if Navigator.NotExists then ViewNavigator
    Kontext "NavigatorWriter"
    Call wNavigatorAuswahl(1,0)

    printlog "- rename Objects"
    '/// Rename Objects in Navigator through context menue (4 Objects) ///

    Kontext "NavigatorWriter"
    for j= 1 to 4
        Auswahlliste.Select j + 1
        printlog "-  Now "+ Auswahlliste.GetItemText(j+1) +" is selected!"
        Auswahlliste.TypeKeys "+"
        Auswahlliste.Select j + 2

        datIsderAlte(j) = Auswahlliste.GetitemText(j+2)

        Auswahlliste.OpenContextMenu
        Call hMenuSelectNr(4)  ' Object nmenu
        Sleep 2
        Call hMenuSelectNr(3)  ' Rename entry
        Sleep 2

        Kontext "ObjekteUmbenennen"
        if Not ObjekteUmbenennen.Exists then
            Warnlog "- Dialog to rename object isn't up!"
        else
            Objektname.Settext "RenamingObjects" + j
            Sleep 1
            try
                ObjekteUmbenennen.Ok
            catch
                Warnlog "- Object could not be renamed!"
                ObjekteUmbenennen.Cancel
            endcatch
        end if

        '/// Check in Navigator if Object has been renamed ///
        Sleep 1
        Kontext "NavigatorWriter"
        if Auswahlliste.GetItemText(j+2) = datIsderAlte(j) then Warnlog "- Entry " + datIsderAlte(j) + " wasn't renamed in Navigator!"

        Auswahlliste.Select j+1
        Auswahlliste.TypeKeys "-"
    next j
    wait 500
    Call gMouseDoubleClick(99,50)
    '/// Save document///
    Call hFileSaveAsWithFilterKill (gOfficePath + "user\work\uvobj.odt", "writer8")
    '/// Close document and reopen previous saved document ///
    Call hCloseDocument
    Sleep 2

    printlog "- Saving and Loading the document, check settings"
    hFileOpen( gOfficePath & "user\work\uvobj.odt")
    '/// Navigator has to be opened with document ///
    Kontext "Navigator"
    if Navigator.NotExists then
        Warnlog "- The Navigator should have been opened!"
        ViewNavigator
    end if
    Kontext "NavigatorWriter"

    For j= 1 to 12
        Auswahlliste.Select j
        Auswahlliste.TypeKeys "-"
    next j

    '/// Check table names in Navigator ///
    printlog  "-  Table"
    Auswahlliste.Select 2
    Auswahlliste.TypeKeys "+"
    Auswahlliste.Select 3
    Auswahlliste.OpenContextMenu
    Sleep 1
    try
        Call hMenuSelectNr(4)
        Sleep 1
        Call hMenuSelectNr(1)
        Sleep 4
    catch
        Warnlog "Unable to open Context-menu!"
        FormatTable
    endcatch
    Kontext

    Kontext
    Active.Setpage TabTabelle
    Kontext "TabTabelle"
    if Tabellenname.Gettext<>"RenamingObjects1" then Warnlog "- Name of Table in Properties is wrong!"
    Sleep 1
    TabTabelle.Cancel
    Kontext "NavigatorWriter"
    Auswahlliste.Select 2
    Auswahlliste.TypeKeys "-"

    '/// Check Text frames names in Navigator ///
    printlog  "-  Text Frames"
    Auswahlliste.Select 3
    Auswahlliste.TypeKeys "+"
    Auswahlliste.Select 4
    Auswahlliste.OpenContextMenu
    Sleep 1
    Call hMenuSelectNr(4)
    Sleep 1
    Call hMenuSelectNr(1)
    Sleep 2
    Kontext
    try
        Active.Setpage TabZusaetze
    catch
        FormatFrame
        Sleep 2
        Kontext
        Active.Setpage TabZusaetze
    endcatch
    Kontext "TabZusaetze"
    if Objektname.Gettext<>"RenamingObjects2" then Warnlog "- Name of textframe in Properties is wrong!"
    Sleep 1
    TabZusaetze.Cancel
    Kontext "NavigatorWriter"
    Auswahlliste.Select 3
    Auswahlliste.TypeKeys "-"

    '/// Check graphic names in Navigator ///
    printlog  "-  Graphic"
    Auswahlliste.Select 4
    Auswahlliste.TypeKeys "+"
    Auswahlliste.Select 5
    Auswahlliste.OpenContextMenu
    Call hMenuSelectNr(4)
    Sleep 1
    Call hMenuSelectNr(1)
    Sleep 2
    Kontext
    try
        Active.Setpage TabZusaetze
    catch
        FormatGraphics
        Kontext
        Active.Setpage TabZusaetze
    endcatch
    Kontext "TabZusaetze"
    if Objektname.Gettext<>"RenamingObjects3" then Warnlog "- Name of graphic in Properties is wrong!"
    Sleep 1
    TabZusaetze.Cancel
    Kontext "NavigatorWriter"
    Auswahlliste.Select 4
    Auswahlliste.TypeKeys "-"

	'/// Check OLE-Objects names in Navigator ///
	printlog  "-  Ole-Object"
	Auswahlliste.Select 5
	Auswahlliste.TypeKeys "+"
	Auswahlliste.Select 6
	Auswahlliste.OpenContextMenu
	try
		Call hMenuSelectNr(4)
		Sleep 1
		Call hMenuSelectNr(1)
		Sleep 5
	catch
		Warnlog "Unable to open context menu!"
		FormatFrame
	endcatch
	Kontext
	try
		Active.Setpage TabZusaetze
	catch
		FormatFrame
		Kontext
		Active.Setpage TabZusaetze
	endcatch

	Kontext "TabZusaetze"
	if Objektname.Gettext<>"RenamingObjects4" then Warnlog "- Name of OLE-Oblect is wrong!"
	Sleep 1
	TabZusaetze.Cancel
    Kontext
    '/// Close Navigator ///
    ViewNavigator
    Sleep 1
    '/// Close document ///
    Call hCloseDocument
endcase
