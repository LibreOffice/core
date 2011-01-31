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
'* short description : Smoke test (load/save/clipboard)
'*
'\***********************************************************************

sub topten

    ' we need the binary filters (.sxw etc.) for this test but beginning with
    ' OOo 3.3 these are optional
    if ( hCheckForBinfilters() ) then
        gApplication = "WRITER"
        call Top_ten_test

        gApplication = "CALC"
        call Top_ten_test

        gApplication = "IMPRESS"
        call Top_ten_test

        gApplication = "DRAW"
        call Top_ten_test

        gApplication = "MATH"
        call Top_ten_test

        gApplication = "HTML"
        call Top_ten_test

        gApplication = "MASTERDOCUMENT"
        call Top_ten_test
    endif
end sub

'*******************************************************************************

testcase Top_ten_test

    dim sUserWorkDirectory as string
    dim sFilename_native as String
    dim sFilter_native as string
    dim sFilter_export as String
    dim bExportFile as boolean

    sUserWorkDirectory = gOfficePath & "user\work\"
    bExportFile = true
    
    printlog( "Current document type: " & gApplication )
    
    ' set the filenames and their filters. HTML is not exported
    if ( gApplication = "HTML" ) then
        sFilename_native = "ls_test.html"
        bExportFile      = FALSE      
    else
        sFilename_native = "ls_test" & hGetSuffix( "current" )
    endif
    
    printlog( "File (current): " & sFilename_native )
    
    ' Delete the workfiles, they might have been left over by prior incomplete testrun
    hDeleteFile( sUserWorkDirectory & sFilename_native )
    
    ' Set the API filternames for the current application (native XML format)
    select case gApplication
    case "WRITER"          : sFilter_native = "writer8"
    case "CALC"            : sFilter_native = "calc8"
    case "DRAW"            : sFilter_native = "draw8"
    case "IMPRESS"         : sFilter_native = "impress8"
    case "MATH"            : sFilter_native = "math8"
    case "MASTERDOCUMENT"  : sFilter_native = "writerglobal8"
    case "HTML"            : sFilter_native = "HTML"
    case else              : warnlog "Invalid gApplication: " & gApplication 
    end select

    printlog( "Create a new document" )
    call hNewDocument
    
    printlog( "Clipboard" )
    call CutCopyPaste
    
    printlog( "Save (default-fileformat): " & sFilename_native )
    call hFileSaveAsKill ( sUserWorkDirectory & sFilename_native )
    
    FileClose

    kontext "active"
    if ( Active.Exists( 1 ) ) then
        warnlog( "Unexpected Active after saving: '" & active.getText & "'" )
        try
            Active.Yes()
        catch
            Active.No()
        endcatch
    endif

    printlog( "Load (default-fileformat): " & sFilename_native )
    call hFileOpen( sUserWorkDirectory & sFilename_native )

    if ( bExportFile ) then
        printlog( "Change the document" )
        hChangeDoc()
        printlog( "Save" )
        hFileSave()
    endif

    printlog( "Close" )
    FileClose

    hDeleteFile( sUserWorkDirectory & sFilename_native )
    
endcase

'*******************************************************************************

sub CutCopyPaste
    dim sSelectAll as string

    ' In Spain Select All is CTRL+E; CTRL+A is FileOpen
    if (iSprache=34) then
        sSelectAll = "<Mod1 e>"
    else
        sSelectAll = "<Mod1 a>"
    endif

    select case gApplication
        case "WRITER", "HTML", "MASTERDOCUMENT"
            kontext "documentwriter"
            DocumentWriter.TypeKeys "This is a test.<Return>"
            printlog( "   cut" )
            DocumentWriter.TypeKeys sSelectAll
            DocumentWriter.TypeKeys "<Mod1 x>"
            Wait( 500 )
            printlog( "   paste" )
            DocumentWriter.TypeKeys "<Mod1 v>"
            Wait( 500 )
            printlog( "   copy" )
            DocumentWriter.TypeKeys sSelectAll
            DocumentWriter.TypeKeys "<Mod1 c>"
            Wait( 500 )
            printlog( "   paste" )
            DocumentWriter.TypeKeys "<Mod1 v>"
            Wait( 500 )
        case "CALC"   : Kontext "DocumentCalc"
            DocumentCalc.TypeKeys "This is a test.<Return>"
            DocumentCalc.TypeKeys "<Up>"
            printlog( "   cut" )
            DocumentCalc.TypeKeys "<Mod1 x>"
            Wait( 500 )
            printlog( "   paste" )
            DocumentCalc.TypeKeys "<Down>"
            DocumentCalc.TypeKeys "<Mod1 v>"
            Wait( 500 )
            printlog( "   copy" )
            DocumentCalc.TypeKeys "<Mod1 c>"
            Wait( 500 )
            printlog( "   paste" )
            DocumentCalc.TypeKeys "<Down>"
            DocumentCalc.TypeKeys "<Mod1 v>"
            Wait( 500 )
        case "DRAW"   : Kontext "DocumentDraw"
            hRechteckErstellen ( 30, 30, 60, 60 )
            gMouseClick ( 1, 1 )
            printlog( "   cut" )
            DocumentDraw.TypeKeys sSelectAll
            DocumentDraw.TypeKeys "<Mod1 x>"
            Wait( 500 )
            printlog( "   paste" )
            DocumentDraw.TypeKeys "<Mod1 v>"
            Wait( 500 )
            printlog( "   copy" )
            gMouseClick ( 1, 1 )
            DocumentDraw.TypeKeys sSelectAll
            DocumentDraw.TypeKeys "<Mod1 c>"
            Wait( 500 )
            printlog( "   paste" )
            gMouseClick ( 1, 1 )
            DocumentDraw.TypeKeys "<Mod1 v>"
        case "IMPRESS": Kontext "DocumentImpress"
            hRechteckErstellen ( 30, 30, 60, 60 )
            gMouseClick ( 1, 1 )
            printlog( "   cut" )
            DocumentImpress.TypeKeys sSelectAll
            DocumentImpress.TypeKeys "<Mod1 x>"
            Wait( 500 )
            printlog( "   paste" )
            DocumentImpress.TypeKeys "<Mod1 v>"
            Wait( 500 )
            printlog( "   copy" )
            gMouseClick ( 1, 1 )
            DocumentImpress.TypeKeys sSelectAll
            DocumentImpress.TypeKeys "<Mod1 c>"
            Wait( 500 )
            printlog( "   paste" )
            gMouseClick ( 1, 1 )
            DocumentImpress.TypeKeys "<Mod1 v>"
        case "MATH"   : SchreibenInMathdok "a over b"
            printlog( "   cut" )
            hUseAsyncSlot( "EditSelectAllMath" )
            hUseAsyncSlot( "EditCut" )
            printlog( "   paste" )
            hUseAsyncSlot( "EditPaste" )
            printlog( "   copy" )
            hUseAsyncSlot( "EditSelectAllMath" )
            hUseAsyncSlot( "EditCopy" )
            printlog( "   paste" )
            hUseAsyncSlot( "EditPaste" )

    end select
end sub



