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
'* owner : gregor.hartmann@oracle.com
'*
'* short description : First test of basic functionality
'*
'\******************************************************************************

testcase tAllNew

    printlog( "Open all available document types" )
    
    Dim lsList (20) as string
    
    if ( gUseSysDlg ) then
        warnlog("Only check, if the documents will be opened; no " & _
        "check for the defaultfilter (system file-dialog)!")
    end if
    
    printlog ""
    try
        gApplication = "WRITER"
        printlog "  - " & gApplication
        
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
    
    try
        gApplication = "CALC"
        printlog "  - " & gApplication
        
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
    
    try
        gApplication = "IMPRESS"
        printlog "  - " & gApplication
        
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
    
    try
        gApplication = "DRAW"
        printlog "  - " & gApplication
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
        printlog "  - " & gApplication
        hNewDocument()
        Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "This is a HTML-Document!"
        if gUseSysDlg = FALSE then
            if (fDocumentCheck (gHTMLFilter) = FALSE) then
                warnlog("The filter name (saving) is not correct! Please check if a HTML document will be opened! -> #i30867")
            end if
        end if
        hCloseDocument()
    catch
        Exceptlog
        ResetApplication
    endcatch
    
    try
        gApplication = "MATH"
        printlog "  - " & gApplication
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
    
    
    try
        gApplication = "MASTERDOCUMENT"
        printlog "  - " & gApplication
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

'*******************************************************************************

testcase tJava

    printlog( "Load a Java applet. It should *not* be executed, applets are disabled by default" )
    
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
        hFileWait( false ) ' Wait but do not warn.
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
                if ( MessageBox.exists( 1 )) then
                    warnlog(Messagebox.GetText())
                    try
                        Messagebox.Cancel()
                        MessageBox.notExists( 2 ) ' wait till it's gone, timing related
                    catch
                        Messagebox.OK() ' if "OK" works, we are in an undefined state
                        warnlog( "The dialog has been closed by OK -> BUG" )
                    endcatch
                    exit for
                end if
            catch
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
        
        messageBox.notExists( 5 )
        Call hCloseDocument()
    end if
endcase

'*******************************************************************************

testcase tDatabaseCheck

    printlog( "Minimal test for Database functionality" )
    
    try
        gApplication = "WRITER"
        hFileOpen( ConvertPath(gOfficePath & "user\database\biblio.odb" )
        printlog "check if database is open"
        Kontext "DATABASE"
        if (Database.exists()) then
            printlog "Database open"
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

'*******************************************************************************

function fDocumentCheck( SollFilter as String ) as boolean
    ' Check the name of the file type in the Save dialog and validate
    ' it against a given string.
    Dim sFiltername as String
    
    FileSaveAs

    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 1 ) ) then
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
    else
        warnlog( "Dialog <File Save As> did not open" )
        fDocumentCheck() = FALSE
    endif
end function

'*******************************************************************************

testcase tInFilterCountCompare()

    printlog( "Check filterlist (filterstring) for graphics files" )

    const FILE_NAME = "graphic_filters.txt"
    const SUFFIX_STRING_LENGTH = 178

'   List of filter suffixes for verification in case of error
'   *.bmp;*.dxf;*.emf;*.eps;*.gif;*.jpg;*.jpeg;*.jfif;*.jif;*.met;*.pbm;*.pcd;
'   *.pct;*.pcx;*.pgm;*.png;*.ppm;*.psd;*.ras;*.sgf;*.sgv;*.svm;*.tga;*.tif;
'   *.tiff;*.wmf;*.xbm;*.xpm;*.pict

    dim caUIFilters( 30 ) as string

    dim cReferencePath as string
        cReferencePath = gTesttoolPath & "global/input/filters/"
        cReferencePath = convertpath ( cReferencePath & FILE_NAME )

    dim cUIFiltersPath as string
        cUIFiltersPath = hGetWorkPath() & FILE_NAME

    dim iCurrentFilter as integer
    dim iFilterCount as integer

    dim cAllFormats as string

    dim irc as integer
    dim iBegin as integer
    dim iEnd as integer

    printlog( "" )
    printlog( "Verify filternames listed on the Insert Graphics dialog" )

    hCreateDocument()
    hChangeDoc()

    InsertGraphicsFromFile

    kontext "GrafikEinfuegenDlg"
    if ( GrafikEinfuegenDlg.exists( 1 ) ) then
        iFilterCount = DateiTyp.getItemCount()

        'Make list compatible with listfunctions
        caUIFilters( 0 ) = iFilterCount - 1

        for iCurrentFilter = 2 to iFilterCount

            caUIFilters( iCurrentFilter - 1 ) = DateiTyp.getItemText( iCurrentFilter )

        next iCurrentFilter

        irc = hManageComparisionList( cReferencePath, cUIFiltersPath, caUIFilters() )
        hListResultEvaluation( irc , 0 )

        printlog( "" )
        printlog( "Verify <All formats>-string" )
        cAllFormats = DateiTyp.getItemText( 1 )

        iBegin = instr( cAllFormats, "<" )
        if ( iBegin <> 1 ) then
            warnlog( "The opening bracket < is missing in <All formats> string" )
        endif

        iEnd = instr( cAllFormats, ">" )
        if ( iEnd = 0 ) then
            warnlog( "The closing bracket > is missing in <All formats> string" )
        endif

        if ( ( iEnd - iBegin ) < 2 ) then
            warnlog( "<All formats> string is shorter than expected, please check" )
            printlog( "String is: " & cAllFormats & ", length (" & len( cAllFormats ) & ")" )
        endif

        iBegin = instr( cAllFormats, "(" )
        if ( iBegin <> iEnd + 2 ) then
            warnlog( "The opening bracket ( is missing in <All formats> string" )
        endif

        iEnd = instr( cAllFormats, ")" )
        if ( iEnd < 10 ) then
            warnlog( "The closing bracket ) is missing in <All formats> string" )
        endif

        irc = iEnd - iBegin
        if ( irc <> SUFFIX_STRING_LENGTH ) then
            warnlog( "The list of file suffixes has changed, please verify" )
            printlog( "Found: " & irc & " chars, expected " & SUFFIX_STRING_LENGTH )
        endif

        kontext "GrafikEinfuegenDlg"
        GrafikEinfuegenDlg.cancel()
    else
        warnlog( "<Insert Graphics> dialog did not open" )
    endif

    hDestroyDocument()

endcase
