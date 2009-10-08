'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: graphics_import.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: obo $ $Date: 2008-07-25 08:03:02 $
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
'/******************************************************************************
'*
'* owner : joerg.skottke@sun.com
'*
'* short description : Graphic import (2nd file for the first.bas)
'*
'\******************************************************************************


testcase tQuickInsertGraphNotAsLink

    printlog( "Insert a sample of commonly used file formats into a writer document (not linked)" )
    
    if ( gUseSysDlg ) then
        warnlog( "This test needs the OpenOffice.org filepicker to be set as default" )
        goto endsub
    endif

    dim iCurrentFile as integer
    dim GraphicFile as String
    dim Filtername as String
    
    gApplication = "WRITER"    
    Call hNewDocument
    
    for iCurrentFile = 1 to 25
    
        printlog( "" )
        
        ' MODIFY and ENABLE this SAMPLE snippet to exclude one fileformat from testing
        '         if ( iCurrentFile = 7 ) AND (gtPlatform="x86") then
        '             qaErrorLog("#<issue-id># crash on <platform> on loading <image-file>")
        '             goto skipformat
        '         endif
        ' Note: Can be locale dependent!

        Kontext "GrafikEinfuegenDlg"
        if ( not GrafikEinfuegenDlg.exists() ) then
            InsertGraphicsFromFile
            Kontext "GrafikEinfuegenDlg"
        endif
        
        if ( GrafikEinfuegenDlg.Exists( 2 ) ) then
        
            ' the list of files expected to be found in the samples directory
            select case iCurrentFile
            case 1  :  GraphicFile = "desp.bmp"
            case 2  :  GraphicFile = "columbia.dxf"
            case 3  :  GraphicFile = "extrud.emf"
            case 4  :  GraphicFile = "ball.eps"
            case 5  :  GraphicFile = "airshw.met"
            case 6  :  GraphicFile = "eface.gif"
            case 7  :  GraphicFile = "purzel.gif"         ' Animated Gif
            case 8  :  GraphicFile = "borabora.jpg"
            case 9  :  GraphicFile = "grafix1.pbm"
            case 10 :  GraphicFile = "circle.pcx"
            case 11 :  GraphicFile = "papagei1.pcd"
            case 12 :  GraphicFile = "newtonme.pct"
            case 13 :  GraphicFile = "grafix2.pgm"
            case 14 :  GraphicFile = "obst.png"
            case 15 :  GraphicFile = "london.ppm"
            case 16 :  GraphicFile = "tiger.psd"
            case 17 :  GraphicFile = "grafix3.ras"
            case 18 :  GraphicFile = "mickym.sgf"
            case 19 :  GraphicFile = "faxpfeil.sgv"
            case 20 :  GraphicFile = "picture.svm"
            case 21 :  GraphicFile = "pilz.tga"
            case 22 :  GraphicFile = "stabler.tif"
            case 23 :  GraphicFile = "krokodil.wmf"
            case 24 :  GraphicFile = "grafix4.xbm"
            case 25 :  GraphicFile = "papagei2.xpm"
            end select
            
            GraphicFile = ConvertPath (gTesttoolPath & "global\input\graf_inp\" & GraphicFile)
            printlog( "Current file: " & GraphicFile )
            printlog( " * Set filter to <All formats>" )
            DateiName.SetText( GraphicFile )
            DateiTyp.Select( 1 )
            WaitSlot()
            
            printlog( " * Uncheck Preview- and Link-checkbox, load the file" )
            Vorschau.unCheck
            Verknuepfen.unCheck()
            Oeffnen.click()
            WaitSlot( 2000 )
            
            Kontext "Active"
            if ( Active.Exists() ) then
                Warnlog( "Unexpected messagebox: " & Active.GetText() )
                Warnlog( "Affected file: " & GraphicFile )
                Active.OK
            else
                try
                    EditCopy
                    WaitSlot( 2000 )
                    EditDeleteContents
                    WaitSlot( 2000 )
                catch
                    Warnlog( "Graphic file not selected/Copy or Delete is disabled: " & GraphicFile )
                    gMouseClick (90, 90)
                endcatch
            endif
            
            skipformat:
        else 
            warnlog( "Insert Graphics Dialog is not open, aborting" )
            goto endsub
        endif
        
    next iCurrentFile
    
    printlog( "" )
    printlog( "Test the PCD-Filters directly (3 samples)" )
    
    for iCurrentFile = 1 to 3
    
        printlog( "" )
        
        Kontext "GrafikEinfuegenDlg"
        if ( not GrafikEinfuegenDlg.exists() ) then
            InsertGraphicsFromFile
            Kontext "GrafikEinfuegenDlg"
        endif

        if ( GrafikEinfuegenDlg.exists( 2 ) ) then
            
            select case iCurrentFile
            case 1 : Filtername = "PCD - Kodak Photo CD (768x512) (*.pcd)"
            case 2 : Filtername = "PCD - Kodak Photo CD (384x256) (*.pcd)"
            case 3 : Filtername = "PCD - Kodak Photo CD (192x128) (*.pcd)"
            end select
            
            GraphicFile = ConvertPath( gTesttoolPath & "global\input\graf_inp\papagei1.pcd" )
            
            printlog( "Current filter: " & Filtername )
            printlog( "Current file..: " & GraphicFile )
            DateiName.SetText ( GraphicFile )
            DateiTyp.Select( Filtername )
            
            printlog( " * Uncheck Preview- and Link-checkbox, load the file" )
            Vorschau.Uncheck()
            Verknuepfen.unCheck()
            Oeffnen.click()
            WaitSlot( 2000 )
            
            Kontext "Active"
            if ( Active.Exists() )then
                Warnlog( "Unexpected messagebox: " & Active.GetText() )
                Warnlog( "Affected file..: " & GraphicFile )
                Warnlog( "Affected filter: " & Filtername )
                Active.OK()
            else
                try
                    EditCopy
                    WaitSlot( 2000 )
                    EditDeleteContents
                    WaitSlot( 2000 )
                catch
                    Warnlog( "Problems with filtertype : " & Filtername )
                    gMouseClick (90, 90)
                endcatch
            endif
        else
            warnlog( "Insert Graphics Dialog is not open. Aborting" )
            goto endsub
        endif
        
    next iCurrentFile
    
    Kontext "GrafikEinfuegenDlg"
    if GrafikEinfuegenDlg.Exists then
        GrafikEinfuegenDlg.Cancel
    endif
    
    Call hCloseDocument

endcase

'*******************************************************************************

testcase tInFilterCountCompare()

    '///<h1>Verify presence of all known graphic filters</h1>
    '///<i>Plain text description</i><br><br>
    
    if ( gUseSysDlg ) then
        warnlog( "This test needs the OpenOffice.org filepicker to be set as default" )
        goto endsub
    endif
    
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

    '///<u>Description:</u>
    printlog( "" )
    printlog( "Verify filternames listed on the Insert Graphics dialog" )
    '///<ul>

    '///+<li>Open a new document, change it</li>
    hCreateDocument()
    hChangeDoc()
    
    '///+<li>Open the graphics import dialog</li>
    InsertGraphicsFromFile
    
    '///+<li>Get the number of filters from the list</li>
    kontext "GrafikEinfuegenDlg"
    iFilterCount = DateiTyp.getItemCount()
    
    'Make list compatible with listfunctions
    caUIFilters( 0 ) = iFilterCount - 1
    
    '///+<li>Part 1: Retrieve the names of the filters, ignore the first entry (all formats)</li>
    for iCurrentFilter = 2 to iFilterCount
    
        caUIFilters( iCurrentFilter - 1 ) = DateiTyp.getItemText( iCurrentFilter )
        
    next iCurrentFilter
    
    '///+<li>Compare the list against the reference</li>
    irc = hManageComparisionList( cReferencePath, cUIFiltersPath, caUIFilters() )
    if ( irc <> 0 ) then
        warnlog( "The list of graphic format filters has changed" )
    endif
    
    '///+<li>Part 2: Verify the &lt;All formats&gt; string</li>
    printlog( "" )
    printlog( "Verify <All formats>-string" )
    cAllFormats = DateiTyp.getItemText( 1 )
    
    '///+<li>Test opening bracket (&lt;) (this is first character in string)</li>
    iBegin = instr( cAllFormats, "<" )
    if ( iBegin <> 1 ) then
        warnlog( "The opening bracket < is missing in <All formats> string" )
    endif
    
    '///+<li>Test closing bracket (&gt;)</li>
    iEnd = instr( cAllFormats, ">" )
    if ( iEnd = 0 ) then
        warnlog( "The closing bracket > is missing in <All formats> string" )
    endif
    
    '///+<li>Verify that &lt;All formats&gt; string has sane length</li>
    if ( ( iEnd - iBegin ) < 2 ) then
        warnlog( "<All formats> string is shorter than expected, please check" )
        printlog( "String is: " & cAllFormats & ", length (" & len( cAllFormats ) & ")" )
    endif
    
    '///+<li>Test opening bracket (, it is separated by one blank from &gt;</li>
    iBegin = instr( cAllFormats, "(" )
    if ( iBegin <> iEnd + 2 ) then
        warnlog( "The opening bracket ( is missing in <All formats> string" )
    endif
    
    '///+<li>Test closing bracket )</li>
    iEnd = instr( cAllFormats, ")" )
    if ( iEnd < 10 ) then
        warnlog( "The closing bracket ) is missing in <All formats> string" )
    endif   
    
    '///+<li>Verify that the list of filter-suffixes has sane length</li>
    irc = iEnd - iBegin
    if ( irc <> SUFFIX_STRING_LENGTH ) then
        warnlog( "The list of file suffixes has changed, please verify" )
        printlog( "Found: " & irc & " chars, expected " & SUFFIX_STRING_LENGTH )
    endif
    
    '///+<li>Close the dialog</li>
    kontext "GrafikEinfuegenDlg"
    GrafikEinfuegenDlg.cancel()

    '///+<li>Close the document</li>
    hDestroyDocument()
    
    '///</ul>


endcase
