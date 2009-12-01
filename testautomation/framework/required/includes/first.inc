'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: first.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:19:03 $
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
'* owner : joerg.skottke@sun.com
'*
'* short description : First test of basic functionality
'*
'\******************************************************************************

testcase tAllNew
    '/// Open all document types and check the default filter name.
    Dim lsList (20) as string
    
    if (gUseSysDlg = TRUE) then
        warnlog("Only check, if the documents will be opened; no " & _
        "check for the defaultfilter (system file-dialog)!")
    end if
    
    '///+<ul><li>Create a new text document (Writer)</li>
    '///+<li>Type some text</li>
    '///+<li>Check the default filter name (fDocumentCheck)</li>
    '///+<li>Close the document window without saving it</li></ul>
    printlog ""
    try
        gApplication = "WRITER"
        printlog "  - " +  gApplication
        
        hNewDocument()
        
        Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "This is a Writer-document!"
        if gUseSysDlg = FALSE then
            if fDocumentCheck (gWriterFilter) = FALSE then
                warnlog("The filter name (saving) is not correct! " & _
                "Please check if a Writer document will be opened!")
            end if
        end if
        hCloseDocument()
    catch
        Exceptlog
        ResetApplication
    endcatch
    
    '///+<ul><li>Create a new spreadsheet document</li>
    '///+<li>Type some text in a cell</li>
    '///+<li>Check the default filter name (fDocumentCheck)</li>
    '///+<li>Close the document window without saving it</li></ul>
    try
        gApplication = "CALC"
        printlog "  - " +  gApplication
        
        hNewDocument
        Kontext "DocumentCalc"
        DocumentCalc.TypeKeys "This is a Calc-document!"
        
        if gUseSysDlg = FALSE then
            if fDocumentCheck (gCalcFilter) = FALSE then
                warnlog("The filter name (saving) is not correct! " & _
                "Please check if a Calc document will be opened!")
            end if
        end if
        hCloseDocument
    catch
        Exceptlog
        ResetApplication
    endcatch
    
    '///+<ul><li>Create a new (empty) presentation</li>
    '///+<li>Insert a new slide with <i>Insert</i> / <i>Duplicate slide</i></li>
    '///+<li>Check the default filter name (fDocumentCheck)</li>
    '///+<li>Close the document window without saving it</li></ul>
    try
        gApplication = "IMPRESS"
        printlog "  - " +  gApplication
        
        hNewDocument()
        sleep(3)
        InsertDuplicateSlide
        sleep(2)
        if gUseSysDlg = FALSE then
            if fDocumentCheck (gImpressFilter) = FALSE then
                warnlog("The filter name (saving) is not correct! " & _
                "Please check if an Impress document will be opened!")
            end if
        end if
        hCloseDocument()
    catch
        Exceptlog
        ResetApplication
    endcatch
    
    '///+<ul><li>Create a new drawing document</li>
    '///+<li><i>Insert</i> / <i>slide</i></li>
    '///+<li>Check the default filter name (fDocumentCheck)</li>
    '///+<li>Close the document window without saving it</li></ul>
    try
        gApplication = "DRAW"
        printlog "  - " +  gApplication
        hNewDocument()
        InsertSlide
        if gUseSysDlg = FALSE then
            if fDocumentCheck (gDrawFilter) = FALSE then
                warnlog("The filter name (saving) is not correct! Please check if a Draw document will be opened!")
            end if
        end if
        hCloseDocument()
    catch
        Exceptlog
        ResetApplication
    endcatch
    try
        gApplication = "HTML"
        printlog "  - " +  gApplication
        '///+<ul><li>Create a new HTML document</li>
        hNewDocument()
        Kontext "DocumentWriter"
        '///+<li>Type some text</li>
        DocumentWriter.TypeKeys "This is a HTML-Document!"
        '///+<li>Check the default filter name (fDocumentCheck)</li>
        if gUseSysDlg = FALSE then
            if (fDocumentCheck (gHTMLFilter) = FALSE) then
                warnlog("The filter name (saving) is not correct! Please check if a HTML document will be opened! -> #i30867")
            end if
        end if
        '///+<li>Close the document window without saving it</li></ul>
        hCloseDocument()
    catch
        Exceptlog
        ResetApplication
    endcatch
    
    '///+<ul><li>Create a new Math document</li>
    '///+<li>Type formula: <i>a over b</i></li>
    '///+<li>Check the default filter name (fDocumentCheck)</li>
    '///+<li>Close the document window without saving it</li></ul>
    try
        gApplication = "MATH"
        printlog "  - " +  gApplication
        hNewDocument()
        Call SchreibenInMathDok("a over b")
        if gUseSysDlg = FALSE then
            if fDocumentCheck (gMathFilter) = FALSE then
                warnlog("The filter name (saving) is not correct! Please check if a Math document will be opened!")
            end if
        end if
        hCloseDocument
    catch
        Exceptlog
        ResetApplication
    endcatch
    
    '///+<ul><li>Crate a new Master document</li>
    '///+<li>Type some text</li>
    '///+<li>Check the default filter name (fDocumentCheck)</li>
    '///+<li>Close the document window without saving it</li></ul>
    
    try
        gApplication = "MASTERDOCUMENT"
        printlog "  - " +  gApplication
        hNewDocument()
        Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "This is a master document!"
        if gUseSysDlg = FALSE then
            if fDocumentCheck (gMasterDocFilter) = FALSE then
                warnlog("The filter name (saving) is not correct! Please check if a Master document will be opened!")
            end if
        end if
        Kontext "Navigator"
        if Navigator.Exists(5) then
            Navigator.Close
        end if
        hCloseDocument()
    catch
        Exceptlog
        ResetApplication
    endcatch
endcase

'-------------------------------------------------------------------------

testcase tJava
    '/// Check a HTML-page with Java crashes.
    '///+<ul><li>Enable the internal file dialog:
    '///+<ul><li><i>Tools</i></li>
    '///+<li><i>Options</i></li>
    '///+<li><i>OpenOffice.org</i></li>
    '///+<li><i>General</i></li>
    '///+<li>Check <i>Use OpenOffice.org dialogs</i></li></ul></li>
    '///+<li>Insert <b><i>TesttoolPath</i>/global/input/java/java.htm</b></li>
    '///+<li>Wait 5 seconds</li>
    '///+<li>Close the document</li></ul>
    
    Dim iTryLoadingJava as integer
    Dim cPath as string
    
    ' This test will not work if the system filedialog is used.
    if (gUseSysDlg) then
        warnlog("No test with system file-dialog!")
    else
        FileOpen
        Kontext "OeffnenDlg"
        Dateiname.settext(ConvertPath(gTestToolPath & "global\input\java\java.htm")
        Oeffnen.Click()
        sleep(5)
        for iTryLoadingJava = 1 to 10
            
            'Sometimes it needs time to bring up the Java Runtime on the system.
            'Just trying it 10 times (paused with a sleep(1)
            
            ' if no java is installed or it is disabled a messagebox will be displayed
            ' for each class file triggered by this test, so there will be two errormessages
            ' asking to enable java. The first msgbox will be handled within the loop while
            ' waiting for the document to get loaded, the second msgbox will be handled
            ' outside the loop, it comes up right after the first one.
            try
                Kontext "Messagebox"
                if ( MessageBox.exists()) then
                    if (gPlatGroup = "unx") then
                        sleep(2)
                    endif
                    warnlog(Messagebox.GetText())
                    try
                        Messagebox.Cancel()
                        sleep( 2 )
                    catch
                        Messagebox.OK() ' if "OK" works, we are in an undefined state
                        warnlog( "The dialog has been closed by OK -> BUG" )
                    endcatch
                    exit for
                end if
            catch
                sleep(1)
                printlog "... wait another second ..."
            endcatch
        next iTryLoadingJava
        
        ' the second errormessage is a bug (even if it is logical it is not ok
        ' from a user's point of view. The task will not be fixed for OOo 2.0
        kontext "Messagebox"
        if ( messagebox.exists() ) then
            try
                qaerrorlog( "#i37020# Second messagebox displayed" )
                Messagebox.cancel()
            catch
                Messagebox.OK()
                warnlog( "The dialog has been closed by OK -> BUG" )
            endcatch
        endif
        
        sleep(5)
        Call hCloseDocument()
    end if
endcase

'-------------------------------------------------------------------------

testcase tDatabaseCheck
    '/// Open bibliography Database.
    printlog ""
    try
        gApplication = "WRITER"
        hFileOpen( ConvertPath(gOfficePath & "user\database\biblio.odb" )
        '/// Check if database is open
        printlog "check if database is open"
        Kontext "DATABASE"
        if (Database.exists()) then
            printlog "Database open"
            '/// Click on the table icon and check if tables are displayed
            printlog "click on the table icon and check if tables are displayed"
            ViewTables
            WaitSlot()
            Kontext "ContainerView"
            if TableTree.exists() then
                printlog "table tree visible"
            else
                warnlog "table tree not visible"
            end if
        else
            warnlog "Database not open"
        end if
        Call hCloseDocument
    catch
        warnlog "error while open Database."
    endcatch
endcase

'-------------------------------------------------------------------------

function fDocumentCheck (SollFilter as String) as boolean
    ' Check the name of the file type in the Save dialog and validate
    ' it against a given string.
    Dim sFiltername as String
    
    FileSaveAs
    Kontext "SpeichernDlg"
    sFiltername = DateiTyp.GetItemText (1)
    if (Instr(lcase(sFiltername), lcase (SollFilter)) <> 0) then
        fDocumentCheck() = TRUE
    else
        printlog("Filter names do not match:")
        printlog("Found:    [" & sFilterName & "]")
        printlog("Expected: [" & SollFilter  & "]")
        SollFilter = sFiltername
        fDocumentCheck() = FALSE
    end if
    SpeichernDlg.Cancel()
end function

'-------------------------------------------------------------------------

testcase tHelpRegistration
    ' Check i69670 which was a showstopper in OOo 2.0.4
    ' Menu-entry "Help / Registration" is disabled
    dim i,a as integer
    
    printlog " - Check if all entries in Help Menu are enabled"
    call hNewDocument
    printlog "   open menu"
    hUseMenu()
    a = hMenuItemGetCount
    printlog "   select the last entry 'Help'"
    hMenuSelectNr(a)
    a = hMenuItemGetCount
    for i = 1 to a
        if hMenuItemIsEnabled(i) then
            printlog "("+i+"/"+a+"): Menu entry is enabled:     Help-> " + hMenuItemGetText(i)
        else
	    if (lcase(gPlatform) = "osx") then
	        warnlog "#i86247# Help->Registration is disabled on MacOS X"
	    else
                warnlog "("+i+"/"+a+"): Menu entry is not enabled: Help-> " + hMenuItemGetText(i)
	    endif
        endif
    next i
    hMenuClose()
    call hCloseDocument
endcase

'-------------------------------------------------------------------------


