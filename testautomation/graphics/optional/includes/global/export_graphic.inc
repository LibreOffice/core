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
'* Owner : wolfram.garten@sun.com
'*
'* short description : Graphics Export A-tests
'*
'\******************************************************************************

testcase tEPS

    dim x as integer
    dim i as integer

    const sFilter = "EPS - Encapsulated PostScript (.eps)"
    const sExt = ".eps"

    printlog "open the document"
    hFileOpen( gTesttoolPath + "graphics\required\input\graphicexport." + ExtensionString )

    if ( hCallExport ( OutputGrafikTBO , sFilter ) ) then
        Kontext "EPSOptionen"
        if ( EPSOptionen.Exists( 2 ) ) then
            printlog "check if all properties have the right count, and depend on each other"
            ' they do not affect annything, i can check (TBO)
            ' VorschauTIF.Check
            '  InterchangeEPSI.Check
            Level1.Check
            if Farbe.IsEnabled then warnlog " :-("
            if Graustufen.IsEnabled then warnlog " :-("
            if LZWKodierung.IsEnabled then warnlog " :-("
            if Keine.IsEnabled then warnlog " :-("
            ' if (TextEinstellungen.IsEnabled <> TRUE) then warnlog " :-("
            Level2.Check
            if (Farbe.IsEnabled <> TRUE) then warnlog " :-("
            if (Graustufen.IsEnabled <> TRUE) then warnlog " :-("
            if (LZWKodierung.IsEnabled <> TRUE) then warnlog " :-("
            if (Keine.IsEnabled <> TRUE) then warnlog " :-("
            ' if (TextEinstellungen.IsEnabled <> TRUE) then warnlog " :-("
            printlog "'Color Resolution' listbox contains eight items"
            ' x = TextEinstellungen.GetItemCount
            ' if x <> 2 then warnlog "'TextEinstellungen' Count is wrong; should:2, is:" + x
            ' for i = 1 to x
            ' TextEinstellungen.Select i
            ' sleep 1
            ' Printlog " - " + i + ": '" +TextEinstellungen.GetSelText + "'"
            ' next i
            printlog "leave dialog with cancel -> there has to be no file created!"
            hCloseDialog( EPSOptionen, "cancel" )

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

        Kontext "EPSOptionen"
        if ( EPSOptionen.Exists( 2 ) ) then
            printlog "TextEinstellungen.select 2"
            hCloseDialog( EPSOptionen, "ok" )
        else
            warnlog( "No export options dialog was displayed" )
        endif

        printlog( "Save image, close file and re-insert graphics into new file" )
        hInsertGraphicsToNewFile( OutputGrafikTBO + sExt ) ' local helper function, see bottom of this file
    endif

    call hCloseDocument

endcase 'tEPS
'-------------------------------------------------------------------------
testcase tPCT

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

    if ( hCallExport( OutputGrafikTBO , sFilter ) ) then

        Kontext "PICTOptionen"
        printlog( "Export options dialog" )
        if ( PICTOptionen.Exists( 2 ) ) then

            printlog "check if all properties have the right count, and depend on each other"
            Original.Check
            if Breite.IsEnabled then warnlog " :-("
            if Hoehe.IsEnabled then warnlog " :-("
            Groesse.Check
            Breite.More
            Hoehe.Less

            printlog "leave dialog with cancel -> there has to be no file created!"
            hCloseDialog( PICTOptionen, "cancel" )

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

    printlog " now save it realy and load the file afterwards"
    if ( hCallExport( OutputGrafikTBO, sFilter ) ) then

        Kontext "PICTOptionen"
        printlog( "Export options dialog" )
        if ( PICTOptionen.Exists( 2 ) ) then
            Groesse.Check
            Breite.Less
            Hoehe.More
            hCloseDialog( PICTOptionen, "ok" )
        else
            warnlog( "No export options dialog was displayed" )
        endif

        sOutputFile = OutputGrafikTBO+sExt

        if ( hWaitForOutputFile( sOutputFile ) ) then

            printlog( "Save image, close file and re-insert graphics into new file" )
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

    hTypeKeys ("<escape><tab>")
    fGetSizeXY sx1, sY, TRUE

    if ( hCallExport( OutputGrafikTBO + "1", sFilter, TRUE ) ) then

        Kontext "PICTOptionen"
        printlog( "Export options dialog" )
        if ( PICTOptionen.Exists( 2 ) ) then
            Original.Check
            hCloseDialog( PICTOptionen, "ok" )
        else
            warnlog( "No export options dialog was displayed" )
        endif

        sOutputFile = OutputGrafikTBO + "1" + sExt
        if ( hWaitForOutputFile( sOutputFile ) ) then
            printlog( "Save image, close file and re-insert graphics into new file" )
            hInsertGraphicsToNewFile( sOutputFile ) ' local helper function, see bottom of this file
        else
            warnlog( "File was not saved: " & sOutputFile )
        endif

        bTemp = fGetSizeXY( sx1, sY, bTemp )
        if (bTemp = FALSE) then
            warnlog "Selected original size NOT OK :-("
        endif
    endif

    hFileReOpen( "" )
    hRechteckErstellen ( 10, 10, 30, 40 )

    if ( hCallExport( OutputGrafikTBO + "2" , sFilter, TRUE ) ) then

        Kontext "PICTOptionen"
        printlog( "Export options dialog" )
        if ( PICTOptionen.Exists( 2 ) ) then
            Groesse.Check
            Breite.SetText "9"
            Hoehe.SetText "9"
            Groesse.Check

            printlog "Check 'Size' one more time to make the change go through"
            sx1 = Breite.GetText
            sY = Hoehe.GetText
            hCloseDialog( PICTOptionen, "ok" )
        else
            warnlog( "No export options dialog was displayed" )
        endif

        sOutputFile = OutputGrafikTBO + "2" + sExt

        if ( not hWaitForOutputFile( sOutputFile ) ) then
            warnlog( "File was not saved: " & sOutputFile )
        endif

        printlog( "Reload file if it exists. Warn if not" )
        hFileReOpen( sOutputFile )

        kontext "DocumentDraw"
        DocumentDraw.TypeKeys ("<escape><tab>")
        ContextOriginalSize
        bTemp = fGetSizeXY( sx1, sY, bTemp )
        if ( not bTemp ) then warnlog "Selected original size NOT OK :-("
    endif

    if ( hCallExport( OutputGrafikTBO + "3", sFilter, TRUE ) ) then

        Kontext "PICTOptionen"
        printlog( "Export options dialog" )
        if ( PICTOptionen.Exists( 2 ) ) then
            Groesse.Check
            sX2 = Breite.GetText
            if (LiberalMeasurement(sx1, sX2)) <> TRUE then
                if (val(str(StrToDouble(sx1)+5)) >= StrToDouble(sX2) ) AND (val(str(StrToDouble ( sx1 )-5)) <= StrToDouble ( sX2 )) then
                    Printlog "Width was ok. Expected: " + sx1 + "' was: '" + sX2 + "'"
                else
                    warnLog "Width is different expected: '" + sx1 + "' is: '" + sX2 + "'"
                endif
            endif
            sY2 = Hoehe.GetText
            if (LiberalMeasurement(sY, sY2)) <> TRUE then
                if ( val(str(StrToDouble(sY)+5)) >= StrToDouble(sY2) ) AND (val(str(StrToDouble ( sY )-5)) <= StrToDouble ( sY2 )) then
                    Printlog "Height was ok. Expected: " + sY + "' was: '" + sY2 + "'"
                else
                    warnLog "Height is different expected: '" + sY + "' is: '" + sY2 + "'"
                endif
            endif
            hCloseDialog( PICTOptionen, "cancel" )
        else
            warnlog( "No export options dialog was displayed" )
        endif
    endif

    call hCloseDocument

endcase 'tPCT
'-------------------------------------------------------------------------------
testcase tPBM

    const sFilter = "PBM - Portable Bitmap (.pbm)"
    const sExt = ".pbm"

    printlog "Open the document"
    hFileOpen( gTesttoolPath + "graphics\required\input\graphicexport."+ExtensionString )

    printlog "Save it"
    if ( hCallExport (OutputGrafikTBO , sFilter ) ) then

        Kontext "PBMOptionen"
        if ( PBMOptionen.Exists( 2 ) ) then
            Ascii.Check
            hCloseDialog( PBMOptionen, "ok" )
        else
            warnlog( "No export options dialog was displayed" )
        endif

        printlog( "Save image, close file and re-insert graphics into new file" )
        hInsertGraphicsToNewFile( OutputGrafikTBO + sExt ) ' local helper function, see bottom of this file

    endif

    call hCloseDocument

endcase 'tPBM
'-------------------------------------------------------------------------------
testcase tPGM

    const sFilter = "PGM - Portable Graymap (.pgm)"
    const sExt = ".pgm"

    printlog "Open the document"
    hFileOpen( gTesttoolPath + "graphics\required\input\graphicexport." + ExtensionString )

    printlog "Save it"
    if ( hCallExport (OutputGrafikTBO , sFilter ) ) then

        Kontext "PGMOptionen"
        if ( PGMOptionen.Exists( 2 ) ) then
            Ascii.Check
            hCloseDialog( PGMOptionen, "ok" )
        else
            warnlog( "No export options dialog was displayed" )
        endif

        printlog( "Save image, close file and re-insert graphics into new file" )
        hInsertGraphicsToNewFile( OutputGrafikTBO + sExt ) ' local helper function, see bottom of this file

    endif

    call hCloseDocument

endcase 'tPGM
'-------------------------------------------------------------------------------
testcase tPPM

    const sFilter = "PPM - Portable Pixelmap (.ppm)"
    const sExt = ".ppm"

    printlog "Open the document"
    hFileOpen( gTesttoolPath + "graphics\required\input\graphicexport." + ExtensionString )

    printlog "Save it"
    if ( hCallExport (OutputGrafikTBO , sFilter ) ) then

        Kontext "PPMOptionen"
        if ( PPMOptionen.Exists( 2 ) ) then
            Ascii.Check
            hCloseDialog( PPMOptionen, "ok" )
        else
            warnlog( "No export options dialog was displayed" )
        endif

        printlog( "Save image, close file and re-insert graphics into new file" )
        hInsertGraphicsToNewFile( OutputGrafikTBO + sExt ) ' local helper function, see bottom of this file

    endif

    call hCloseDocument

endcase 'tPPM
'------------------------------------------------------------------------------
testcase tRAS

    const sFilter = "RAS - Sun Raster Image (.ras)"
    const sExt = ".ras"

    printlog "Open the document"
    hFileOpen( gTesttoolPath + "graphics\required\input\graphicexport." + ExtensionString )

    printlog "Save it"
    if ( hCallExport( OutputGrafikTBO, sFilter ) ) then
        printlog( "Save image, close file and re-insert graphics into new file" )
        hInsertGraphicsToNewFile( OutputGrafikTBO + sExt ) ' local helper function, see bottom of this file
    else
        warnlog( "No export options dialog was displayed" )
    endif

    call hCloseDocument

endcase 'tRAS
'------------------------------------------------------------------------------
testcase tTIFF

    dim x as integer
    dim i as integer

    const sFilter = "TIFF - Tagged Image File Format (.tif;.tiff)"
    const sExt = ".tif"

    printlog "Open the document"
    hFileOpen (ConvertPath ( gTesttoolPath + "graphics\required\input\graphicexport."+ExtensionString ))

    printlog "Save it"
    if ( hCallExport (OutputGrafikTBO , sFilter ) ) then
        printlog( "Save image, close file and re-insert graphics into new file" )
        hInsertGraphicsToNewFile( OutputGrafikTBO + sExt ) ' local helper function, see bottom of this file
    else
        warnlog( "No export options dialog was displayed" )
    endif

    call hCloseDocument

endcase 'tTIFF
'------------------------------------------------------------------------------
testcase tXPM

    const sFilter = "XPM - X PixMap (.xpm)"
    const sExt = ".xpm"

    printlog "Open the document"
    hFileOpen( gTesttoolPath + "graphics\required\input\graphicexport." + ExtensionString )

    printlog "save it"
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        printlog( "Save image, close file and re-insert graphics into new file" )
        hInsertGraphicsToNewFile( OutputGrafikTBO + sExt ) ' local helper function, see bottom of this file
    else
        warnlog( "No export options dialog was displayed" )
    endif

    call hCloseDocument

endcase 'tXPM
'-------------------------------------------------------------------------------
testcase tGIF

    const sFilter = "GIF - Graphics Interchange Format (.gif)"
    const sExt = ".gif"

    printlog "open the document"
    hFileOpen( gTesttoolPath + "graphics\required\input\graphicexport." + ExtensionString )

    printlog "save it"
    if ( hCallExport( OutputGrafikTBO, sFilter ) ) then

        Kontext "GIFOptionen"
        if GIFOptionen.Exists (2) then
            Interlace.Uncheck
            TransparentSpeichern.UnCheck
            hCloseDialog( GIFOptionen, "ok" )
        else
            warnlog( "No export options dialog was displayed" )
        endif

        printlog( "Save image, close file and re-insert graphics into new file" )
        hInsertGraphicsToNewFile( OutputGrafikTBO + sExt ) ' local helper function, see bottom of this file

    endif

    call hCloseDocument

endcase 'tGIF
'-------------------------------------------------------------------------------
testcase tJPEG

    const sFilter = "JPEG - Joint Photographic Experts Group (.jpg;.jpeg;.jfif;.jif;.jpe)"
    const sExt = ".jpg"

    printlog "Open the document"
    hFileOpen( gTesttoolPath + "graphics\required\input\graphicexport." + ExtensionString )

    printlog " save it "
    if ( hCallExport (OutputGrafikTBO , sFilter ) ) then

        Kontext "JpegOptionen"
        if ( JpegOptionen.Exists( 2 ) ) then
            Echtfarben.Check
            Qualitaet.ToMin
            hCloseDialog( JpegOptionen, "ok" )
        else
            warnlog( "No export options dialog was displayed" )
        endif

        printlog( "Save image, close file and re-insert graphics into new file" )
        hInsertGraphicsToNewFile( OutputGrafikTBO + sExt ) ' local helper function, see bottom of this file
        
    endif

    call hCloseDocument

endcase 'tJPEG
'-------------------------------------------------------------------------

function hInsertGraphicsToNewFile( sOutputFile as string )

    if ( not FileExists( sOutputFile ) ) then
        warnlog( "File not saved: " & sOutputFile )
    else
        hFileReOpen( "" )
        Call hGrafikEinfuegen ( sOutputFile )
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
