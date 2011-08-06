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
'/******************************************************************************
'*
'* Owner : wolfram.garten@oracle.com
'*
'* short description : Graphics Export A-tests
'*
'\******************************************************************************

testcase tEPS

    warnlog( "#i115138# - Test document freezes draw" )
    goto endsub

    dim x as integer
    dim i as integer

    const sFilter = "EPS - Encapsulated PostScript (.eps)"
    const sExt = ".eps"

    printlog "open the document"
    hFileOpen( gTesttoolPath + "graphics\required\input\graphicexport." + ExtensionString )

    if ( hCallExport ( OutputGrafikTBO , sFilter ) ) then
        Kontext "EPSOptions"
        if ( EPSOptions.Exists( 2 ) ) then
            printlog "check if all properties have the right count, and depend on each other"

            Level1.Check
            if Color.IsEnabled then warnlog " :-("
            if Greyscale.IsEnabled then warnlog " :-("
            if LZWEncoding.IsEnabled then warnlog " :-("
            if NoneCompression.IsEnabled then warnlog " :-("

            Level2.Check
            if (Color.IsEnabled <> TRUE) then warnlog " :-("
            if (Greyscale.IsEnabled <> TRUE) then warnlog " :-("
            if (LZWEncoding.IsEnabled <> TRUE) then warnlog " :-("
            if (NoneCompression.IsEnabled <> TRUE) then warnlog " :-("

            printlog "leave dialog with cancel -> there has to be no file created!"
            hCloseDialog( EPSOptions, "cancel" )

            if ( FileExists( OutputGrafikTBO+sExt ) ) then ' inspired by bug #99932 Graphic is exported though cancel is pressed
                warnlog "Dialog was canceled, but file got saved, too :-(  - i35177"
            endif
        else
            warnlog( "No export options dialog was displayed" )
        end if

        Kontext "Active"
        if Active.Exists(2) then
            Warnlog "'" + sFilter + "' has a problem"
            Active.OK
        end if
    end if

    printlog( "Save the file" )
    if ( hCallExport( OutputGrafikTBO, sFilter ) ) then

        Kontext "EPSOptions"
        if ( EPSOptions.Exists( 2 ) ) then
            hCloseDialog( EPSOptions, "ok" )
        else
            warnlog( "No export options dialog was displayed" )
        endif

        printlog( "Close file and re-insert graphics into new document" )
        hInsertGraphicsToNewFile( OutputGrafikTBO + sExt ) ' local helper function, see bottom of this file
    endif

    call hCloseDocument

endcase 'tEPS
'-------------------------------------------------------------------------
testcase tPCT

    warnlog( "#i115138# - Test document freezes draw" )
    goto endsub



    dim x as integer
    dim i as integer
    dim iWaitIndex as integer
    dim bTemp as boolean
    dim sX as string
    dim sY as string
    dim sx1 as string
    dim sX2 as string
    dim sY2 as string
    dim sDocument as string
    dim sOutputFile as string

    const FILE_WRITE_TIMEOUT = 200 ' seconds

    const sFilter = "PCT - Mac Pict (.pct;.pict)"
    const sExt = ".pct"

    printlog "open the document"
    sDocument = convertpath( gTesttoolPath + "graphics\required\input\graphicexport." + ExtensionString )
    hFileOpen( sDocument )
    printlog "----------1st: Trying export and canceling it."
    if ( hCallExport( OutputGrafikTBO , sFilter ) ) then
        Kontext "PCTOptions"
        printlog( "Export options dialog" )
        if PCTOptions.Exists(2) then
            printlog "leave dialog with cancel -> there has to be no file created!"
            hCloseDialog( PCTOptions, "cancel" )
            if ( FileExists( OutputGrafikTBO + sExt ) ) then
                warnlog( "#i35177# - dialog <PictOptions> canceled, still the file was saved" )
            endif
        else
            warnlog( "No export options dialog was displayed" )
        end if
        
        Kontext "Active"
        if Active.Exists(2) then
            Warnlog " '" + sFilter + "'  has a problem"
            Active.OK
        end if
    end if

    printlog "----------2nd: now save it really and load the file afterwards"
    if ( hCallExport( OutputGrafikTBO, sFilter ) ) then

        Kontext "PCTOptions"
        printlog( "Export options dialog" )
        if ( PCTOptions.Exists( 2 ) ) then
            hCloseDialog( PCTOptions, "ok" )
        else
            warnlog( "No export options dialog was displayed" )
        endif
        sOutputFile = OutputGrafikTBO+sExt
        if ( hWaitForOutputFile( sOutputFile ) ) then
            printlog "Close file and re-insert graphics into new document"
            hInsertGraphicsToNewFile( sOutputFile ) ' local helper function, see bottom of this file
        else
            warnlog( "File was not saved: " & sOutputFile )
        endif
    endif

    hFileReOpen( sDocument )

    printlog "check if the document is writable"
    if ( not fIsDocumentWritable ) then
        printlog( "Document is write protected, changing state" )
        if ( not fMakeDocumentWritable ) then
            warnlog "The document can't be make writeable. Test stopped."
            goto endsub
        endif
    endif

    call hCloseDocument

endcase 'tPCT
'-------------------------------------------------------------------------------
testcase tPBM
    warnlog( "#i115138# - Test document freezes draw" )
    goto endsub



    const sFilter = "PBM - Portable Bitmap (.pbm)"
    const sExt = ".pbm"

    printlog "Open the document"
    hFileOpen( gTesttoolPath + "graphics\required\input\graphicexport."+ExtensionString )

    printlog "Save it"
    if ( hCallExport (OutputGrafikTBO , sFilter ) ) then

        Kontext "PBMOptions"
        if ( PBMOptions.Exists( 2 ) ) then
            QualityBinary.Check
            QualityText.Check
            hCloseDialog( PBMOptions, "ok" )
        else
            warnlog( "No export options dialog was displayed" )
        endif

        printlog( "Close file and re-insert graphics into new document" )
        sleep (1)
        hInsertGraphicsToNewFile( OutputGrafikTBO + sExt ) ' local helper function, see bottom of this file

    endif

    call hCloseDocument

endcase 'tPBM
'-------------------------------------------------------------------------------
testcase tPGM
    warnlog( "#i115138# - Test document freezes draw" )
    goto endsub



    const sFilter = "PGM - Portable Graymap (.pgm)"
    const sExt = ".pgm"

    printlog "Open the document"
    hFileOpen( gTesttoolPath + "graphics\required\input\graphicexport." + ExtensionString )

    printlog "Save it"
    if ( hCallExport (OutputGrafikTBO , sFilter ) ) then

        Kontext "PGMOptions"
        if ( PGMOptions.Exists( 2 ) ) then
            QualityBinary.Check
            QualityText.Check
            hCloseDialog( PGMOptions, "ok" )
        else
            warnlog( "No export options dialog was displayed" )
        endif

        printlog( "Close file and re-insert graphics into new document" )
        hInsertGraphicsToNewFile( OutputGrafikTBO + sExt ) ' local helper function, see bottom of this file

    endif

    call hCloseDocument

endcase 'tPGM
'-------------------------------------------------------------------------------
testcase tPPM
    warnlog( "#i115138# - Test document freezes draw" )
    goto endsub



    const sFilter = "PPM - Portable Pixelmap (.ppm)"
    const sExt = ".ppm"

    printlog "Open the document"
    hFileOpen( gTesttoolPath + "graphics\required\input\graphicexport." + ExtensionString )

    printlog "Save it"
    if ( hCallExport (OutputGrafikTBO , sFilter ) ) then

        Kontext "PPMOptions"
        if ( PPMOptions.Exists( 2 ) ) then
            QualityBinary.Check
            QualityText.Check
            hCloseDialog( PPMOptions, "ok" )
        else
            warnlog( "No export options dialog was displayed" )
        endif

        printlog( "Close file and re-insert graphics into new document" )
        hInsertGraphicsToNewFile( OutputGrafikTBO + sExt ) ' local helper function, see bottom of this file

    endif

    call hCloseDocument

endcase 'tPPM
'------------------------------------------------------------------------------
testcase tRAS
    warnlog( "#i115138# - Test document freezes draw" )
    goto endsub



    const sFilter = "RAS - Sun Raster Image (.ras)"
    const sExt = ".ras"

    printlog "Open the document"
    hFileOpen( gTesttoolPath + "graphics\required\input\graphicexport." + ExtensionString )

    printlog "Save it"
    if ( hCallExport( OutputGrafikTBO, sFilter ) ) then
        printlog( "Close file and re-insert graphics into new document" )
        hInsertGraphicsToNewFile( OutputGrafikTBO + sExt ) ' local helper function, see bottom of this file
    else
        warnlog( "No export options dialog was displayed" )
    endif

    call hCloseDocument

endcase 'tRAS
'------------------------------------------------------------------------------
testcase tTIFF
    warnlog( "#i115138# - Test document freezes draw" )
    goto endsub



    dim x as integer
    dim i as integer

    const sFilter = "TIFF - Tagged Image File Format (.tif;.tiff)"
    const sExt = ".tif"

    printlog "Open the document"
    hFileOpen (ConvertPath ( gTesttoolPath + "graphics\required\input\graphicexport."+ExtensionString ))

    printlog "Save it"
    if ( hCallExport (OutputGrafikTBO , sFilter ) ) then
        printlog( "Close file and re-insert graphics into new document" )
        hInsertGraphicsToNewFile( OutputGrafikTBO + sExt ) ' local helper function, see bottom of this file
    else
        warnlog( "No export options dialog was displayed" )
    endif

    call hCloseDocument

endcase 'tTIFF
'------------------------------------------------------------------------------
testcase tSVG
    'warnlog( "#i115138# - Test document freezes draw" )
    'goto endsub

    dim x as integer
    dim i as integer

    const sFilter = "SVG - Scalable Vector Graphics (.svg)"
    const sExt = ".svg"

    printlog "Open the document"
    hFileOpen (ConvertPath ( gTesttoolPath + "graphics\required\input\graphicexport."+ExtensionString ))

    printlog "Save it"
    if ( hCallExport (OutputGrafikTBO , sFilter ) ) then
        printlog( "Close file and re-insert graphics into new document" )
        hInsertGraphicsToNewFile( OutputGrafikTBO + sExt ) ' local helper function, see bottom of this file
    else
        warnlog( "No export options dialog was displayed" )
    endif

    call hCloseDocument

endcase 'tSVG
'------------------------------------------------------------------------------
testcase tXPM
    warnlog( "#i115138# - Test document freezes draw" )
    goto endsub



    const sFilter = "XPM - X PixMap (.xpm)"
    const sExt = ".xpm"

    printlog "Open the document"
    hFileOpen( gTesttoolPath + "graphics\required\input\graphicexport." + ExtensionString )

    printlog "save it"
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        printlog( "Close file and re-insert graphics into new document" )
        hInsertGraphicsToNewFile( OutputGrafikTBO + sExt ) ' local helper function, see bottom of this file
    else
        warnlog( "No export options dialog was displayed" )
    endif

    call hCloseDocument

endcase 'tXPM
'-------------------------------------------------------------------------------
testcase tGIF
    warnlog( "#i115138# - Test document freezes draw" )
    goto endsub



    const sFilter = "GIF - Graphics Interchange Format (.gif)"
    const sExt = ".gif"

    printlog "open the document"
    hFileOpen( gTesttoolPath + "graphics\required\input\graphicexport." + ExtensionString )

    printlog "save it"
    if ( hCallExport( OutputGrafikTBO, sFilter ) ) then

        Kontext "GIFOptions"
        if GIFOptions.Exists (2) then
            Interlaced.Uncheck
            Transparency.UnCheck
            hCloseDialog( GIFOptions, "ok" )
        else
            warnlog( "No export options dialog was displayed" )
        endif

        printlog( "Close file and re-insert graphics into new document" )
        sleep (1)
        hInsertGraphicsToNewFile( OutputGrafikTBO + sExt ) ' local helper function, see bottom of this file

    endif

    call hCloseDocument

endcase 'tGIF
'-------------------------------------------------------------------------------
testcase tJPEG
    warnlog( "#i115138# - Test document freezes draw" )
    goto endsub



    const sFilter = "JPEG - Joint Photographic Experts Group (.jpg;.jpeg;.jfif;.jif;.jpe)"
    const sExt = ".jpg"

    printlog "Open the document"
    hFileOpen( gTesttoolPath + "graphics\required\input\graphicexport." + ExtensionString )

    printlog " save it "
    if ( hCallExport( OutputGrafikTBO , sFilter ) ) then

        Kontext "JPGOptions"
        if ( JPGOptions.Exists( 2 ) ) then
            Quality.ToMin
            hCloseDialog( JPGOptions, "ok" )
        else
            warnlog( "No export options dialog was displayed" )
        endif

        printlog( "Close file and re-insert graphics into new document" )
        hInsertGraphicsToNewFile( OutputGrafikTBO + sExt ) ' local helper function, see bottom of this file
        
    endif

    call hCloseDocument

endcase 'tJPEG

'*******************************************************************************

function hInsertGraphicsToNewFile( sOutputFile as string )
    sleep (1)
    if ( FileExists( sOutputFile ) ) then
        hFileReOpen( "" )
        Call hGrafikEinfuegen( sOutputFile )
    else
        warnlog( "File not saved: " & sOutputFile )
    endif

end function

'*******************************************************************************

function hFileReOpen( cFile as string ) as boolean

    dim iDocumentCount as integer : iDocumentCount = getDocumentCount
    hFileReopen() = false

    hCloseDocument()

    if ( cFile = "" ) then
        hNewDocument()
    else
        if ( FileExists( cFile ) ) then
            hFileOpen( cFile )
        else
            warnlog( "hFileReOpen(): File does not exist: " & cFile )
        endif
    endif

    if ( getDocumentCount = iDocumentCount ) then hFileReOpen() = true

end function

'*******************************************************************************

function hWaitForOutputFile( sOutputFile as string ) as boolean

    const FILE_WRITE_TIMEOUT = 30
    dim iWait as integer : iWait = 0

    for iWait = 1 to FILE_WRITE_TIMEOUT

        ' File found
        if ( FileExists( sOutputFile ) ) then
            hWaitForOutputFile() = true
            exit function
        endif

        ' Timeout
        if ( iWait = FILE_WRITE_TIMEOUT ) then
            hWaitForOutputFile() = false
            exit function
        endif

        wait( 1000 )

    next iWait

end function
