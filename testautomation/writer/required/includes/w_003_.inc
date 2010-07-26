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
'* short description :  Testscripts connected to the View-Menu.
'*
'\******************************************************************

sub w_003_

    printLog Chr(13) + "--------- View Menu (w_003_.inc) ----------"
    gApplication = "WRITER"

    Call tViewPrintLayout
    Call tViewWebLayout
    Call tViewHTMLSource

    Call tViewToolbars

    Call tViewStatusBar
    Call tViewInputMethodStatus
    Call tViewRuler

    Call tViewTextBoundaries
    Call tViewFieldShadings
    Call tViewFields
    Call tViewNonprintingCharacters
    Call tViewHiddenParagraphs

    Call tViewDataSources

    Call tViewFullScreen
    Call tViewZoom

    'Writer Web
    gApplication = "HTML"
    Call tHtmlDocViewZoom
    Call tHtmlDocViewHTMLSource
    gApplication = "WRITER"

end sub

'------------------------------------------------------

testcase tViewZoom
    printlog " open application"
    Call hNewDocument
    printlog " type something into the document"
    hTypekeys "<RETURN>", 3
    UseBindings
    printlog " View -> Zoom"
    ViewZoom
    Kontext "Massstab"
    if ( Massstab.exists( 1 ) ) then
        DialogTest ( Massstab )
        select case uCASE(gApplication)
        case "HTML" :
            printlog " check checkbox '100%'"
            Vergroesserung100.check
            printlog " check checkbox 'Variable'"
            VergroesserungStufenlos.check
        case else :
            printlog " check checkbox 'Optimal'"
            Optimal.check
            printlog " check checkbox 'Fit height and width'"
            GanzeSeite.check
            printlog " check checkbox 'Fit Width'"
            Seitenbreite.check
            printlog " check checkbox '100%'"
            Vergroesserung100.check
            printlog " check checkbox 'Variable'"
            VergroesserungStufenlos.check
            printlog " click button MORE on 'Variable'"
            Stufenlos.more
            printlog " click button 'Automatic'"
            Automatic.check
            printlog " click button 'Single Page'"
            Singlepage.check
            printlog " click button 'Columns'"
            Columns.check
            printlog " click button MORE on 'Columns'"
            Columnsfield.more
            printlog " click button LESS on 'Columns'"
            Columnsfield.less
            printlog " Check checkbox 'Book mode'"
            Bookmode.check
            printlog " cancel dialog 'Zoom'"
        end select
        Massstab.Cancel
    else
        warnlog( "Dialog <Massstab> is not available" )
    endif
    printlog " close application"
    Call  hCloseDocument
endcase

'-----------------------------------------------------------

testcase tViewDataSources
    PrintLog "- View / Data Sources"
    printlog " Open new document"
    Call hNewDocument
    printlog " View / Data Sources"
    ViewCurrentDatabaseWriter

    Kontext "DatabaseBeamer"
    if ( DatabaseBeamer.Exists( 5 ) ) then
        printlog " Close database beamer with : View Data Sources"
        ViewCurrentDatabaseWriter
        
        Kontext "DatabaseBeamer"
        if ( DatabaseBeamer.Exists() ) then
            Warnlog "Could not close Databasebeamer with same slot!"
            ViewBeamer
            
            Kontext "DatabaseBeamer"
            if DatabaseBeamer.Exists then 
                Warnlog "Could not close Databasebeamer with Slot->View Beamer!"
            endif
        else
            printlog( "Closed Database Beamer dialog" )
        endif
    else
        Warnlog "Database Beamer has not been opened!"
    endif

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tViewToolbars
    PrintLog "- View / Toolbars"
    Dim i as integer

    printlog " Close all documents until only backing window is opened"
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop

    printlog " Open a new writer document"
    Call hNewDocument

    printlog "- Toolbar 'Align'"
    printlog "-  open"
    Call hToolbarSelect("Align", true)
    printlog "-  close"
    Call hToolbarSelect("Align", false)
    printlog "-  reset"
    Call hToolbarSelect("Align", true, true)

    printlog "- Toolbar 'Bullets And Numbering'"
    printlog "-  open"
    Call hToolbarSelect("BulletsAndNumbering", true)
    printlog "-  close"
    Call hToolbarSelect("BulletsAndNumbering", false)
    printlog "-  reset"
    Call hToolbarSelect("BulletsAndNumbering", true, true)

    printlog "- Toolbar 'Drawing'"
    printlog "-  open"
    Call hToolbarSelect("Drawing", true)
    printlog "-  close"
    Call hToolbarSelect("Drawing", false)
    printlog "-  reset"
    Call hToolbarSelect("Drawing", true, true)

    printlog "- Toolbar 'Drawing Object Properties'"
    printlog "-  open"
    Call hToolbarSelect("DrawingObjectProperties", true)
    printlog "-  close"
    Call hToolbarSelect("DrawingObjectProperties", false)
    printlog "-  reset"
    Call hToolbarSelect("DrawingObjectProperties", true, true)

    printlog "- Toolbar 'Form Controls'"
    printlog "-  open"
    Call hToolbarSelect("FormControls", true)
    printlog "-  close"
    Call hToolbarSelect("FormControls", false)
    printlog "-  reset"
    Call hToolbarSelect("FormControls", true, true)

    printlog "- Toolbar 'Form Design'"
    printlog "-  open"
    Call hToolbarSelect("FormDesign", true)
    printlog "-  close"
    Call hToolbarSelect("FormDesign", false)
    printlog "-  reset"
    Call hToolbarSelect("FormDesign", true, true)

    printlog "- Toolbar 'Form Navigation'"
    printlog "-  open"
    Call hToolbarSelect("FormNavigation", true)
    printlog "-  close"
    Call hToolbarSelect("FormNavigation", false)
    printlog "-  reset"
    Call hToolbarSelect("FormNavigation", true, true)

    ' The following toolbar is disabled because it will be found in all documents
    ' therefore they can't be reactivated once deactivated
    printlog "- Toolbar 'Formatting'"
    printlog "-  open"
    Call hToolbarSelect("Formatting", true)
    printlog "-  close"
    Call hToolbarSelect("Formatting", false)
    printlog "-  reset"
    Call hToolbarSelect("Formatting", true, true)

    printlog "- Toolbar 'Frame'"
    printlog "-  open"
    Call hToolbarSelect("Frame", true)
    printlog "-  close"
    Call hToolbarSelect("Frame", false)
    printlog "-  reset"
    Call hToolbarSelect("Frame", true, true)

    printlog "- Toolbar 'Insert'"
    printlog "-  open"
    Call hToolbarSelect("Insert", true)
    printlog "-  close"
    Call hToolbarSelect("Insert", false)
    printlog "-  reset"
    Call hToolbarSelect("Insert", true, true)

    printlog "- Toolbar 'Insert Object'"
    printlog "-  open"
    Call hToolbarSelect("InsertObject", true)
    printlog "-  close"
    Call hToolbarSelect("InsertObject", false)
    printlog "-  reset"
    Call hToolbarSelect("InsertObject", true, true)

    printlog "- Toolbar 'Media Playback'"
    printlog "-  open"
    Call hToolbarSelect("MediaPlayback", true)
    printlog "-  close"
    Call hToolbarSelect("MediaPlayback", false)
    printlog "-  reset"
    Call hToolbarSelect("MediaPlayback", true, true)

    printlog "- Toolbar 'OLE Object'"
    printlog "-  open"
    Call hToolbarSelect("OLEObject", true)
    printlog "-  close"
    Call hToolbarSelect("OLEObject", false)
    printlog "-  reset"
    Call hToolbarSelect("OLEObject", true, true)

    printlog "- Toolbar 'Page Preview'"
    printlog "-  open"
    Call hToolbarSelect("PagePreview", true)
    printlog "-  close"
    Call hToolbarSelect("PagePreview", false)
    printlog "-  reset"
    Call hToolbarSelect("PagePreview", true, true)

    printlog "- Toolbar 'Picture'"
    printlog "-  open"
    Call hToolbarSelect("Picture", true)
    printlog "-  close"
    Call hToolbarSelect("Picture", false)
    printlog "-  reset"
    Call hToolbarSelect("Picture", true, true)

    ' The following toolbar is disabled because it will be found in all documents
    ' therefore they can't be reactivated once deactivated
    printlog "- Toolbar 'Standard'"
    printlog "-  open"
    Call hToolbarSelect("Standard", true)
    printlog "-  close"
    Call hToolbarSelect("Standard", false)
    printlog "-  reset"
    Call hToolbarSelect("Standard", true, true)

    printlog "- Toolbar 'Standard (Viewing Mode)'"
    printlog "-  open"
    Call hToolbarSelect("StandardViewingMode", true)
    printlog "-  close"
    Call hToolbarSelect("StandardViewingMode", false)
    printlog "-  reset"
    Call hToolbarSelect("StandardViewingMode", true, true)

    printlog "- Toolbar 'Table'"
    printlog "-  open"
    Call hToolbarSelect("Table", true)
    printlog "-  close"
    Call hToolbarSelect("Table", false)
    printlog "-  reset"
    Call hToolbarSelect("Table", true, true)

    printlog "- Toolbar 'Text Object'"
    printlog "-  open"
    Call hToolbarSelect("TextObject", true)
    printlog "-  close"
    Call hToolbarSelect("TextObject", false)
    printlog "-  reset"
    Call hToolbarSelect("TextObject", true, true)

    printlog "- Toolbar 'Tools'"
    printlog "-  open"
    Call hToolbarSelect("Tools", true)
    printlog "-  close"
    Call hToolbarSelect("Tools", false)
    printlog "-  reset"
    Call hToolbarSelect("Tools", true, true)

    printlog "- Toolbar 'Hyperlink Bar'"
    printlog "-  open"
    Call hToolbarSelect("HyperlinkBar", true)
    printlog "-  close"
    Call hToolbarSelect("HyperlinkBar", false)
    printlog "-  reset"
    Call hToolbarSelect("HyperlinkBar", true, true)

    printlog "- Toolbar 'Formula'"
    printlog "-  open"
    Call hToolbarSelect("MATH", true)
    OL_SW_Rechenleiste.TypeKeys "<Enter>"

    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tViewTextBoundaries
    PrintLog "- View / Text Boundaries"
    printlog " Open new document"
    Call hNewDocument
    printlog " View / Text Boundaries"
    ViewTextBoundaries
    WaitSlot (2000)
    printlog " View / Text Boundaries"
    ViewTextBoundaries
    printlog " Close active document"
    Call hCloseDocument
endcase

'------------------------------------------------------

testcase tViewFieldShadings
    PrintLog "- View / Field Shadings"
    printlog " Open new document"
    Call hNewDocument
    printlog " View / Field Shadings"
    ViewFieldShadings
    WaitSlot (2000)
    printlog " View / Field Shadings"
    ViewFieldShadings
    printlog " Close active document"
    Call hCloseDocument
endcase

'------------------------------------------------------

testcase tViewFields
    PrintLog "- View / Fields"
    printlog " Open new document"
    Call hNewDocument
    printlog " View / Fields"
    ViewFields
    WaitSlot (2000)
    printlog " View / Fields"
    ViewFields
    printlog " Close active document"
    Call hCloseDocument
endcase

'------------------------------------------------------

testcase tViewNonprintingCharacters
    PrintLog "- View / Nonprinting Characters"
    printlog " Open new document"
    Call hNewDocument
    printlog " View / Nonprinting characters"
    ViewNonprintingCharacters
    WaitSlot (2000)
    printlog " View / Nonprinting characters"
    ViewNonprintingCharacters
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tViewHiddenParagraphs
    printLog "- View / Hidden Paragraphs"
    printlog " Open new document"
    Call hNewDocument
    printlog " View / Hidden paragraphs "
    ViewHiddenParagraphs
    WaitSlot(2000)
    printlog " View / Hidden paragraphs "
    ViewHiddenParagraphs
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tViewWebLayout
    PrintLog "- View / WebLayout"
    printlog " Open new document"
    Call hNewDocument
    printlog " View / Web Layout "
    ViewOnlineLayout
    WaitSlot (2000)
    printlog " View / Online Layout "
    ViewOnlineLayout
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tViewFullScreen
    PrintLog "- View / Full Screen"
    printlog " Open new document"
    Call hNewDocument
    printlog " View / Full Screen "
    ViewFullScreen
    WaitSlot (2000)
    printlog " View / Full Screen "
    ViewFullScreen
    WaitSlot (2000)
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tViewPrintLayout
    PrintLog "- View / Print Layout"
    printlog " ONLY HTML-Document"
    printlog " Open new document"
    Call hNewDocument
    WaitSlot (2000)
    printlog " View / Print Layout "
    ViewPrintLayout
    WaitSlot (2000)
    printlog " View / Print Layout "
    ViewPrintLayout
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tViewStatusBar
    PrintLog "- View / Status Bar"
    printlog " Open new document"
    Call hNewDocument
    printlog " View / Statusbar "
    ViewToolbarsStatusbar
    WaitSlot (2000)
    printlog " View / Print Layout "
    ViewToolbarsStatusbar
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tViewRuler
    PrintLog "- View / Ruler"

    printlog " Open new document"
    Call hNewDocument
    printlog " View / Ruler "
    ViewRuler
    WaitSlot (2000)
    printlog " View / Ruler "
    ViewRuler
    WaitSlot (2000)
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tViewInputMethodStatus
    PrintLog "- View / Input Method Status (only Solaris)"
    if ( Ucase(gPlatform) <> "SOLARIS" ) then 
        printlog( "No testing for IME status for OS <> Solaris" )
        goto endsub
    endif

    PrintLog " Open new document"
    Call hNewDocument
    PrintLog " View / View / Input Method Status"
    try
        ViewInputMethodStatus
    catch
        Warnlog "Menuentry 'Input Method Status' is disabled!"
        Call hCloseDocument
        goto endsub
    endcatch
    WaitSlot (2000)
    PrintLog " View / View / Input Method Status"
    ViewInputMethodStatus
    PrintLog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tViewHTMLSource
    PrintLog "- View / HTML Source (only in HTML-Document)"
    if ( gApplication <> "HTML" ) then
        goto endsub
    endif
    
    printlog "Open new document"
    Call hNewDocument
    printlog "View / HTML-Source"
    AnsichtHTMLQuelltext

    printlog "Save-As dialog has to come up"
    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 1 ) ) then
        Dateiname.Settext Convertpath(gofficepath + "user\work\sourcecode.html")
        printlog "Save sourcecode"
        Speichern.Click()
        Kontext "Active"
        If Active.Exists then
            if Active.GetRT = 304 then
                try
                    Active.Yes
                catch
                    Warnlog Active.Gettext
                    Active.Ok
                endcatch
            end if
        end if
    else
        warnlog( "Dialog <SpeichernDlg> is not available" )
    endif

    Kontext "DocumentHTMLSourceView"
    DocumentHTMLSourceView.TypeKeys "Check for bug: #116402"
    wait 500
    printlog "View / HTML-Source"
    AnsichtHTMLQuelltext
    WaitSlot (1000)
    printlog "Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tHtmlDocViewZoom
    gApplication = "HTML"
    printlog " open application"
    Call hNewDocument
    printlog " type something into the document"
    hTypekeys "<RETURN>", 3

    printlog " View -> Zoom"
    ViewZoom
    Kontext "Massstab"
    if ( Massstab.exists( 1 ) ) then
        DialogTest ( Massstab )
        printlog " check checkbox '100%'"
        Vergroesserung100.check
        printlog " check checkbox 'Variable'"
        VergroesserungStufenlos.check
        Massstab.Cancel
    else
        warnlog( "Dialog <Massstab> is not available" )
    endif
    printlog " close application"
    Call  hCloseDocument
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------

testcase tHtmlDocViewHTMLSource
    PrintLog "- View / HTML Source (only in HTML-Document)"
    gApplication = "HTML"
    printlog "Open new document"
    Call hNewDocument
    printlog "View / HTML-Source"
    AnsichtHTMLQuelltext
    WaitSlot (1000)
    printlog "Save-As dialog has to come up"
    printlog "Save sourcecode"
    Kontext "SpeichernDlg"
    Dateiname.Settext Convertpath(gofficepath + "user\work\sourcecode.html")
    Speichern.Click
    Kontext "Active"
    If Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.Yes
            catch
                Warnlog Active.Gettext
                Active.Ok
            endcatch
        end if
    end if
    wait 500
    Kontext "DocumentHTMLSourceView"
    DocumentHTMLSourceView.TypeKeys "Check for bug: #116402"
    wait 500
    printlog "View / HTML-Source"
    AnsichtHTMLQuelltext
    WaitSlot (1000)
    printlog "Close active document"
    Call hCloseDocument
    gApplication = "WRITER"
endcase

'-----------------------------------------------------------
