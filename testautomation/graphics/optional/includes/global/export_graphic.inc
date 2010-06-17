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
'* short description : Graphics Export A-tests. (More durable ones)
'*
'*******************************************************************************
'*
' #1 tEPS
' #1 tGIF
' #1 tJPEG
' #1 tPBM
' #1 tPCT
' #1 tPGM
' #1 tPPM
' #1 tRAS
' #1 tTIFF
' #1 tXPM
'*
'\******************************************************************************
testcase tEPS

    dim x as integer
    dim i as integer
    dim sFilter as string
    dim sExt as string

    sFilter = "EPS - Encapsulated PostScript (.eps)"
    sExt = ".eps"

    printlog "open the document"
    hFileOpen ConvertPath ( gTesttoolPath + "graphics\required\input\graphicexport."+ExtensionString)

    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "EPSOptionen"
        if EPSOptionen.Exists (2) then
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
                                            EPSOptionen.Cancel
                                            sleep 5
                                            if ( dir(OutputGrafikTBO+sExt) = "") then ' inspired by bug #99932 Graphic is exported though cancel is pressed
                                                Printlog "Ok :-)"
                                            else
                                                warnlog "Dialog was canceled, but file got saved, too :-(  - i35177"
                                            endif
                                        else
                                            Warnlog "No '" + sFilter + "'-Option-Dialog!"
                                            i=5
                                        end if
                                        sleep 2
                                        Kontext "Active"
                                        if Active.Exists(2) then
                                            Warnlog "'" + sFilter + "' has a problem"
                                            Active.OK
                                        end if
                                    end if
                                    printlog " now save it realy and load the file afterwards"
                                    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
                                        Kontext "EPSOptionen"
                                        if EPSOptionen.Exists (2) then
                                            printlog "TextEinstellungen.select 2"
                                            EPSOptionen.OK
                                            sleep 5
                                        endif
                                        if ( dir(OutputGrafikTBO+sExt) <> "") then
                                            Printlog "Ok :-) Saved as: '" + OutputGrafikTBO+sExt + "'"
                                        else
                                            warnlog "File didn't get saved :-("
                                        endif
                                        hCloseDocument ()
                                        sleep 5
                                        hNewDocument()
                                        sleep 5
    if ( lcase( gPlatform ) <> "osx" ) then
                                        Call hGrafikEinfuegen ( OutputGrafikTBO+sExt )
    else
        qaErrorLog "#i100253# crash on MacOS X 10.4"
    endif
                                    endif

                                    call hCloseDocument

endcase 'tEPS
'-------------------------------------------------------------------------
testcase tPCT

    dim x as integer
    dim i as integer
    dim iWaitIndex as integer
    dim sFilter as string
    dim sExt as string
    dim bTemp as boolean
    dim sX as string
    dim sY as string
    dim sx1 as string
    dim sX2 as string
    dim sY2 as string
    dim sDocument as string

    sFilter = "PCT - Mac Pict (.pct;.pict)"
    sExt = ".pct"

    printlog "open the document"
    sDocument = ConvertPath ( gTesttoolPath + "graphics\required\input\graphicexport."+ExtensionString)
    hFileOpen sDocument

    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "PICTOptionen"
        if PICTOptionen.Exists (2) then
            printlog "check if all properties have the right count, and depend on each other"
            Original.Check
            if Breite.IsEnabled then warnlog " :-("
                if Hoehe.IsEnabled then warnlog " :-("
                    Groesse.Check
                    Breite.More
                    Hoehe.Less
                    printlog "leave dialog with cancel -> there has to be no file created!"
                    PICTOptionen.Cancel
                    sleep 5
                    if ( dir(OutputGrafikTBO+sExt) = "") then
                        Printlog "Ok :-)"
                    else
                        warnlog "Dialog was canceled, but file got saved, too :-(  - i35177"
                    endif
                else
                    Warnlog "No  '" + sFilter + "' -Dialog!"
                    i=5
                end if
                sleep 2
                Kontext "Active"
                if Active.Exists(2) then
                    Warnlog " '" + sFilter + "'  has a problem"
                    Active.OK
                end if
            end if
            printlog " now save it realy and load the file afterwards"
            if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
                Kontext "PICTOptionen"
                if PICTOptionen.Exists (2) then
                    Groesse.Check
                    Breite.Less
                    Hoehe.More
                    PICTOptionen.OK
                    iWaitIndex = 0
                    do while PICTOptionen.Exists AND iWaitIndex < 30
                        sleep (1)
                        iWaitIndex = iWaitIndex + 1
                    loop
                endif
                i=0
                while ((NOT fileExists(OutputGrafikTBO+sExt)) AND (i<36))
                    inc(i)
                    sleep(5)
                wend
                if ( dir(OutputGrafikTBO+sExt) <> "") then
                    Printlog "Ok :-) Saved as: '" + OutputGrafikTBO+sExt + "'"
                else
                    warnlog "File didn't get saved :-("
                endif
                hCloseDocument ()
                sleep 5
                hNewDocument()
                sleep 5
                Call hGrafikEinfuegen ( OutputGrafikTBO+sExt )
            endif
            hCloseDocument ()
            printlog " now save a SELECTION in ORIGINAL SIZE and load the file afterwards"
            hFileOpen (sDocument)
            sleep (10)

            printlog "check if the document is writable"
            if fIsDocumentWritable = false then
                printlog "make the document writable and check if it's succesfull"
                if fMakeDocumentWritable = false then
                    warnlog "The document can't be make writeable. Test stopped."
                    goto endsub
                endif
            endif

            hTypeKeys ("<escape><tab>")
            fGetSizeXY sx1, sY, TRUE
            if hCallExport (OutputGrafikTBO + "1" , sFilter, TRUE ) = TRUE then
                Kontext "PICTOptionen"
                if PICTOptionen.Exists (2) then
                    Original.Check
                    PICTOptionen.OK
                    iWaitIndex = 0
                    do while PICTOptionen.Exists AND iWaitIndex < 30
                        sleep (1)
                        iWaitIndex = iWaitIndex + 1
                    loop
                endif
                i=0
                while ((NOT fileExists(OutputGrafikTBO+ "1"+sExt)) AND (i<36))
                    inc(i)
                    sleep(5)
                wend
                if ( dir(OutputGrafikTBO + "1"+sExt) <> "") then
                    Printlog "Ok :-) Saved as: '" + OutputGrafikTBO + "1"+sExt + "'"
                else
                    warnlog "File didn't get saved :-("
                endif
                hCloseDocument ()
                sleep 5
                hNewDocument()
                sleep 5
                Call hGrafikEinfuegen ( OutputGrafikTBO + "1"+sExt )
                bTemp = FALSE
                fGetSizeXY sx1, sY, bTemp
                if (bTemp = FALSE) then
                    warnlog "Selected original size NOT OK :-("
                endif
            endif
            hCloseDocument ()
            printlog " now CREATE a rectangle, select it, save it in SIZE and load the file afterwards"
            hNewDocument()
            hRechteckErstellen ( 10, 10, 30, 40 )
            if hCallExport (OutputGrafikTBO + "2" , sFilter, TRUE ) = TRUE then
                Kontext "PICTOptionen"
                if PICTOptionen.Exists (2) then
                    Groesse.Check
                    Breite.SetText "9"
                    Hoehe.SetText "9"
                    Groesse.Check
                    printlog "Check 'Size' one more time to make the change go through"
                    sx1 = Breite.GetText
                    sY = Hoehe.GetText
                    PICTOptionen.OK
                    iWaitIndex = 0
                    do while PICTOptionen.Exists AND iWaitIndex < 30
                        sleep (1)
                        iWaitIndex = iWaitIndex + 1
                    loop
                endif
                i=0
                while ((NOT fileExists(OutputGrafikTBO+ "2"+sExt)) AND (i<36))
                    inc(i)
                    sleep(5)
                wend
                if ( dir(OutputGrafikTBO + "2"+sExt) <> "") then
                    Printlog "Ok :-) Saved as: '" + OutputGrafikTBO + "2"+sExt + "'"
                else
                    warnlog "File didn't get saved :-("
                endif
                hCloseDocument ()
                sleep 5
                hFileOpen (OutputGrafikTBO + "2"+sExt)
                kontext "DocumentDraw"
                DocumentDraw.TypeKeys ("<escape><tab>")
                ContextOriginalSize
                bTemp = FALSE
                fGetSizeXY sx1, sY, bTemp
                if (bTemp = FALSE) then
                    warnlog "Selected original size NOT OK :-("
                endif
            endif
            if hCallExport (OutputGrafikTBO + "3" , sFilter, TRUE ) = TRUE then
                Kontext "PICTOptionen"
                if PICTOptionen.Exists (2) then
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
                    PICTOptionen.Cancel
                    sleep 5
                endif
            endif

            call hCloseDocument

endcase 'tPCT
'-------------------------------------------------------------------------------
testcase tPBM

    dim x as integer
    dim i as integer
    dim sFilter as string
    dim sExt as string

    sFilter = "PBM - Portable Bitmap (.pbm)"
    sExt = ".pbm"

    printlog "Open the document"
    hFileOpen (ConvertPath ( gTesttoolPath + "graphics\required\input\graphicexport."+ExtensionString ))

    printlog "Save it"
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "PBMOptionen"
        if PBMOptionen.Exists (2) then
            Ascii.Check
            PBMOptionen.OK
            sleep 5
        endif
        if ( dir(OutputGrafikTBO+sExt) <> "") then
            Printlog "Ok :-) Saved as: '" + OutputGrafikTBO+sExt + "'"
        else
            warnlog "File didn't get saved :-("
        endif
        hCloseDocument ()
        sleep 5
        hNewDocument()
        sleep 5
        Call hGrafikEinfuegen ( OutputGrafikTBO+sExt )
    endif

    call hCloseDocument

endcase 'tPBM
'-------------------------------------------------------------------------------
testcase tPGM

    dim x as integer
    dim i as integer
    dim sFilter as string
    dim sExt as string

    sFilter = "PGM - Portable Graymap (.pgm)"
    sExt = ".pgm"

    printlog "Open the document"
    hFileOpen (ConvertPath ( gTesttoolPath + "graphics\required\input\graphicexport."+ExtensionString  ))

    printlog "Save it"
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "PGMOptionen"
        if PGMOptionen.Exists (2) then
            Ascii.Check
            PGMOptionen.OK
            sleep 5
        endif
        if ( dir(OutputGrafikTBO+sExt) <> "") then
            Printlog "Ok :-) Saved as: '" + OutputGrafikTBO+sExt + "'"
        else
            warnlog "File didn't get saved :-("
        endif
        hCloseDocument ()
        sleep 5
        hNewDocument()
        sleep 5
        Call hGrafikEinfuegen ( OutputGrafikTBO+sExt )
    endif

    call hCloseDocument

endcase 'tPGM
'-------------------------------------------------------------------------------
testcase tPPM

    dim x as integer
    dim i as integer
    dim sFilter as string
    dim sExt as string

    sFilter = "PPM - Portable Pixelmap (.ppm)"
    sExt = ".ppm"

    printlog "Open the document"
    hFileOpen (ConvertPath ( gTesttoolPath + "graphics\required\input\graphicexport."+ExtensionString  ))

    printlog "Save it"
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "PPMOptionen"
        if PPMOptionen.Exists (2) then
            Ascii.Check
            PPMOptionen.OK
            sleep 5
        endif
        if ( dir(OutputGrafikTBO+sExt) <> "") then
            Printlog "Ok :-) Saved as: '" + OutputGrafikTBO+sExt + "'"
        else
            warnlog "File didn't get saved :-("
        endif
        hCloseDocument ()
        sleep 5
        hNewDocument()
        sleep 5
        Call hGrafikEinfuegen ( OutputGrafikTBO+sExt )
    endif

    call hCloseDocument

endcase 'tPPM
'------------------------------------------------------------------------------
testcase tRAS

    dim x as integer
    dim i as integer
    dim sFilter as string
    dim sExt as string

    sFilter = "RAS - Sun Raster Image (.ras)"
    sExt = ".ras"

    printlog "Open the document"
    hFileOpen (ConvertPath ( gTesttoolPath + "graphics\required\input\graphicexport."+ExtensionString ))

    printlog "Save it"
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        if ( dir(OutputGrafikTBO+sExt) <> "") then
            Printlog "Ok :-) Saved as: '" + OutputGrafikTBO+sExt + "'"
            hCloseDocument ()
            sleep 5
            hNewDocument()
            sleep 5
            Call hGrafikEinfuegen ( OutputGrafikTBO+sExt )
        else
            warnlog "File didn't get saved :-("
        endif
    endif

    call hCloseDocument

endcase 'tRAS
'------------------------------------------------------------------------------
testcase tTIFF

    dim x as integer
    dim i as integer
    dim sFilter as string
    dim sExt as string

    sFilter = "TIFF - Tagged Image File Format (.tif;.tiff)"
    sExt = ".tif"

    printlog "Open the document"
    hFileOpen (ConvertPath ( gTesttoolPath + "graphics\required\input\graphicexport."+ExtensionString ))

    printlog "Save it"
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        if ( dir(OutputGrafikTBO+sExt) <> "") then
            Printlog "Ok :-) Saved as: '" + OutputGrafikTBO+sExt + "'"
            hCloseDocument ()
            sleep 5
            hNewDocument()
            sleep 5
            Call hGrafikEinfuegen ( OutputGrafikTBO+sExt )
        else
            warnlog "File didn't get saved :-("
        endif
    endif

    call hCloseDocument

endcase 'tTIFF
'------------------------------------------------------------------------------
testcase tXPM

    dim x as integer
    dim i as integer
    dim sFilter as string
    dim sExt as string

    sFilter = "XPM - X PixMap (.xpm)"
    sExt = ".xpm"

    printlog "Open the document"
    hFileOpen (ConvertPath ( gTesttoolPath + "graphics\required\input\graphicexport."+ExtensionString ))

    printlog "save it"
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        if ( dir(OutputGrafikTBO+sExt) <> "") then
            Printlog "Ok :-) Saved as: '" + OutputGrafikTBO+sExt + "'"
            hCloseDocument ()
            sleep 5
            hNewDocument()
            sleep 5
            Call hGrafikEinfuegen ( OutputGrafikTBO+sExt )
        else
            warnlog "File didn't get saved :-("
        endif
    endif

    call hCloseDocument

endcase 'tXPM
'-------------------------------------------------------------------------------
testcase tGIF

    dim x as integer
    dim i as integer
    dim sFilter as string
    dim sExt as string

    sFilter = "GIF - Graphics Interchange Format (.gif)"
    sExt = ".gif"

    printlog "open the document"
    hFileOpen (ConvertPath ( gTesttoolPath + "graphics\required\input\graphicexport."+ExtensionString)

    printlog "save it"
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "GIFOptionen"
        if GIFOptionen.Exists (2) then
            Interlace.Uncheck
            TransparentSpeichern.UnCheck
            GIFOptionen.OK
            sleep 5
        endif
        if ( dir(OutputGrafikTBO+sExt) <> "") then
            Printlog "Ok :-) Saved as: '" + OutputGrafikTBO+sExt + "'"
        else
            warnlog "File didn't get saved :-("
        endif
        hCloseDocument ()
        sleep 5
        hNewDocument()
        sleep 5
        Call hGrafikEinfuegen ( OutputGrafikTBO+sExt )
    endif

    call hCloseDocument

endcase 'tGIF
'-------------------------------------------------------------------------------
testcase tJPEG

    dim x as integer
    dim i as integer
    dim sFilter as string
    dim sExt as string

    sFilter = "JPEG - Joint Photographic Experts Group (.jpg;.jpeg;.jfif;.jif;.jpe)"
    sExt = ".jpg"

    printlog "Open the document"
    hFileOpen (ConvertPath ( gTesttoolPath + "graphics\required\input\graphicexport."+ExtensionString)

    printlog " save it "
    if hCallExport (OutputGrafikTBO , sFilter ) = TRUE then
        Kontext "JpegOptionen"
        if JpegOptionen.Exists (2) then
            Echtfarben.Check
            Qualitaet.ToMin
            JpegOptionen.OK
            sleep 5
        endif
        if ( dir(OutputGrafikTBO+sExt) <> "") then
            Printlog "Ok :-) Saved as: '" + OutputGrafikTBO+sExt + "'"
        else
            warnlog "File didn't get saved :-("
        endif
        hCloseDocument ()
        sleep 5
        hNewDocument()
        sleep 5
        Call hGrafikEinfuegen ( OutputGrafikTBO+sExt )
    endif

    call hCloseDocument

endcase 'tJPEG
'-------------------------------------------------------------------------
