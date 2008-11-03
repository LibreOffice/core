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
    '/// Insert all types of graphics (StarOffice can handle) into a document (not linked)
    '/// The document will be opened depending on <i>gApplication</i>
    '/// 25 files will be inserted (the path is <i><b>[TesttoolPath]</b>/global/input/graf_inp</i>) without selecting the filter name
    '///+<ul><li>desp.bmp (Windows Bitmap)</li>
    '///+<li>columbia.dxf (AutoCAD Interchange Format)</li>
    '///+<li>extrud.emf (Enhanced Metafile)</li>
    '///+<li>ball.eps (Encapsulated PostScript)</li>
    '///+<li>airshw.met (OS/2 Metafile)</li>
    '///+<li>eface.gif (GIF : Graphics Interchange Format)</li>
    '///+<li>purzel.gif (Animated Gif)</li>
    '///+<li>borabora.jpg (Joint Photographic Experts Group)</li>
    '///+<li>grafix1.pbm (Portable Bitmap)</li>
    '///+<li>circle.pcx (Zsoft Paintbrush)</li>
    '///+<li>papagei1.pcd (Kodak Photo CD (768x512) / (192x128) / (384x256))</li>
    '///+<li>newtonme.pct (Mac Pict)</li>
    '///+<li>grafix2.pgm (Portable Graymap)</li>
    '///+<li>obst.png (Portable Network Graphic)</li>
    '///+<li>london.ppm (Portable Pixelmap)</li>
    '///+<li>tiger.psd (Adobe Photoshop)</li>
    '///+<li>grafix3.ras (Sun Raster Image)</li>
    '///+<li>mickym.sgf (StarWriter Graphics Format)</li>
    '///+<li>faxpfeil.sgv (StarDraw 2.0)</li>
    '///+<li>picture.svm (StarView Metafile)</li>
    '///+<li>pilz.tga (Truevision Targa)</li>
    '///+<li>stabler.tif (Tagged Image File Format)</li>
    '///+<li>krokodil.wmf (Windows Metafile)</li>
    '///+<li>grafix4.xbm (X Bitmap)</li>
    '///+<li>papagei2.xpm (X PixMap)</li></ul>
    
    if ( gUseSysDlg ) then
        warnlog( "This test needs the OpenOffice.org filepicker to be set as default" )
        goto endsub
    endif

    Dim i : Dim StartTime
    Dim DieGrafik as String, Filtername as String
    
    StartTime = Now()
    printlog "- insert all graphic-types (not linked)"
    
    '/// Create a new document
    '/// <i>Insert</i> / <i>Graphic</i> / <i>From file</i>
    Call hNewDocument
    for i=1 to 25
        
        ' use this snippet to exclude one fileformat from testing
        '         if (i = 7) AND (gtPlatform="x86") then
        '             qaErrorLog("#i68829# crash on Solaris x86 on loading purzel.gif")
        '             goto skipformat
        '         endif
        
                 if (i = 5) then
                     warnlog("#168829# crash on loading airshw.met going to next format")
                     goto skipformat
                 endif
        
        Kontext "GrafikEinfuegenDlg"
        if GrafikEinfuegenDlg.NotExists then
            InsertGraphicsFromFile
        endif
        Sleep 2
        select case i
        case 1  :  DieGrafik = "desp.bmp"
        case 2  :  DieGrafik = "columbia.dxf"
        case 3  :  DieGrafik = "extrud.emf"
        case 4  :  DieGrafik = "ball.eps"
        case 5  :  DieGrafik = "airshw.met"
        case 6  :  DieGrafik = "eface.gif"
        case 7  :  DieGrafik = "purzel.gif"         ' Animated Gif
        case 8  :  DieGrafik = "borabora.jpg"
        case 9  :  DieGrafik = "grafix1.pbm"
        case 10 :  DieGrafik = "circle.pcx"
        case 11 :  DieGrafik = "papagei1.pcd"
        case 12 :  DieGrafik = "newtonme.pct"
        case 13 :  DieGrafik = "grafix2.pgm"
        case 14 :  DieGrafik = "obst.png"
        case 15 :  DieGrafik = "london.ppm"
        case 16 :  DieGrafik = "tiger.psd"
        case 17 :  DieGrafik = "grafix3.ras"
        case 18 :  DieGrafik = "mickym.sgf"
        case 19 :  DieGrafik = "faxpfeil.sgv"
        case 20 :  DieGrafik = "picture.svm"
        case 21 :  DieGrafik = "pilz.tga"
        case 22 :  DieGrafik = "stabler.tif"
        case 23 :  DieGrafik = "krokodil.wmf"
        case 24 :  DieGrafik = "grafix4.xbm"
        case 25 :  DieGrafik = "papagei2.xpm"
        end select
        printlog "- " + DieGrafik
        DieGrafik = ConvertPath (gTesttoolPath + "global\input\graf_inp\" + DieGrafik)
        DateiName.SetText (DieGrafik)
        ' Set filter to 'all formats'
        DateiTyp.Select(1)
        sleep(1)
        
        '/// Uncheck <i>link</i>
        Vorschau.Uncheck
        if Verknuepfen.Exists then
            if  Verknuepfen.IsEnabled then
                Verknuepfen.UnCheck
            endif
        end if
        
        '/// Insert the graphic name and click <i>Insert</i>
        Oeffnen.Click
        sleep(2)
        Kontext "Active"
        if Active.Exists then
            Warnlog Active.GetText
            Warnlog "Bei der Grafik : " + DieGrafik
            Active.OK
        else
            try
                EditCopy
                sleep 1
                EditDeleteContents
                Sleep 1
            catch
                Warnlog "Probleme bei Grafik : " + DieGrafik
                gMouseClick (90, 90)
            endcatch
        end if
        
        skipformat:
        
    next i
    
    '/// Special handling for the PCD-files
    '///+<ul><li>Insert the file <i>papagei1.pcd</i> with selected filtername:</li>
    '///+<li><ol><li>PCD - Kodak Photo CD (768x512)</li>
    '///+<li>PCD - Kodak Photo CD (384x256)</li>
    '///+<li>PCD - Kodak Photo CD (192x128)</li></ol></li></ul>
    
    printlog Chr(13) + "Test the PCD-Filters directly"
    for i=1 to 3
        Kontext "GrafikEinfuegenDlg"
        if GrafikEinfuegenDlg.NotExists then
            InsertGraphicsFromFile
        endif
        Sleep 2
        select case i
        case 1 : Filtername = "PCD - Kodak Photo CD (768x512) (*.pcd)"
        case 2 : Filtername = "PCD - Kodak Photo CD (384x256) (*.pcd)"
        case 3 : Filtername = "PCD - Kodak Photo CD (192x128) (*.pcd)"
        end select
        printlog "- " + Filtername
        DateiName.SetText (ConvertPath (gTesttoolPath + "global\input\graf_inp\papagei1.pcd")
        DateiTyp.Select Filtername
        
        Vorschau.Uncheck
        if Verknuepfen.Exists then
            if  Verknuepfen.IsEnabled then
                Verknuepfen.UnCheck
            endif
        end if
        
        Oeffnen.Click
        Sleep 2
        Kontext "Active"
        if Active.Exists then
            Warnlog Active.GetText
            Warnlog "For filter : " + Filtername
            Active.OK
        else
            try
                EditCopy
                if i=13 OR i=16 then
                    Sleep 3
                endif
                EditDeleteContents
                Sleep 1
            catch
                Warnlog "Problems with filtertype : " + Filtername
                gMouseClick (90, 90)
            endcatch
        end if
    next i
    
    '/// After inserting all files the document will be closed.
    Kontext "GrafikEinfuegenDlg"
    if GrafikEinfuegenDlg.Exists then
        GrafikEinfuegenDlg.Cancel
    endif
    Call hCloseDocument
    printlog "Dauer : " + Wielange (StartTime)
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
