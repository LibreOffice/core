'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: topten.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:19:04 $
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
'* owner : thorsten.bosbach@sun.com
'*
'* short description : Smoke test (load/save/clipboard)
'*
'\***********************************************************************

sub topten
    gApplication = "WRITER"
    printlog( "** " + gApplication )
    call Top_ten_test
    gApplication = "CALC"
    printlog( "** " + gApplication )
    call Top_ten_test
    gApplication = "IMPRESS"
    printlog( "** " + gApplication )
    call Top_ten_test
    gApplication = "DRAW"
    printlog( "** " + gApplication )
    call Top_ten_test
    gApplication = "MATH"
    printlog( "** " + gApplication )
    call Top_ten_test
    gApplication = "HTML"
    printlog( "** " + gApplication )
    call Top_ten_test
    gApplication = "MASTERDOC"
    printlog( "** " + gApplication )
    call Top_ten_test

end sub

'*******************************************************************************

testcase Top_ten_test
    '///Top-Ten-Test
    '///<ul><li>open all document types</li>
    '///+<li>insert a text or a rectangle (Draw/Impress)</li>
    '///+<li>cut - paste / copy - paste</li>
    '///+<li>save as default-format (6.0format (XML))</li>
    '///+<li>save as 5.2-format</li>
    '///+<li>close the doc</li>
    '///+<li>load the 8.0-doc, change it, save this changes and close the doc</li>
    '///+<li>load the 5.2-doc, change it, save this changes and close the doc</li></ul>
    
    dim rootdir as string
    dim vExtension(10) as string
    dim DokName1 as String
    dim DokName2 as String
    dim sFilter1 as string
    dim sFilter2 as String
    Dim i as Integer
    dim bExport as boolean

    rootdir = gOfficePath + "user\work\"
    bExport = true
    
    ' set the filenames and their filters
    if gApplication = "HTML" then
        'hGetFilternameExtension is able to read more than one extension entry
        vExtension() = hGetFilternameExtension("writer_web_HTML")
        DokName1 = "ls_test" & "." & vExtension(0) 
        Dokname2 = Dokname1        
    else
        DokName1 = "ls_test" & hGetSuffix( "current" )
        DokName2 = "ls_test" & hGetSuffix( "569" )
    endif
    
    printlog( " * File (current): " & DokName1 )
    printlog( " * File (src569).: " & DokName2 )
    
    select case gApplication
    case "WRITER"     : sFilter1 = "writer8"
    case "CALC"       : sFilter1 = "calc8"
    case "DRAW"       : sFilter1 = "draw8"
    case "IMPRESS"    : sFilter1 = "impress8"
    case "MATH"       : sFilter1 = "math8"
    case "MASTERDOC"  : sFilter1 = "writerglobal8"
    case "HTML"       : sFilter1 = "HTML" : bExport = FALSE
    case else         : warnlog "Invalid gApplication: " & gApplication 
    end select

    printlog( "- create a new document" )
    call hNewDocument
    printlog( "- clipboard" )
    call CutCopyPaste
    printlog( "- saving => default-fileformat" )
    call hFileSaveAsKill ( rootdir & DokName1 )
    if ( gApplication  = "IMPRESS" ) then sleep( 2 )

    if bExport then
    
        ' hFileSaveAsWithFilterKill needs the API filtername. Hardcoded for speed.
        select case gApplication
        case "WRITER"     : sFilter2 = "StarWriter 5.0"
        case "CALC"       : sFilter2 = "StarCalc 5.0"
        case "DRAW"       : sFilter2 = "StarDraw 5.0"
        case "IMPRESS"    : sFilter2 = "StarImpress 5.0"
        case "MATH"       : sFilter2 = "StarMath 5.0"
        case "MASTERDOC"  : sFilter2 = "StarWriter 5.0/GlobalDocument"
        case else         : warnlog "Invalid gApplication: " & gApplication 
        end select
        
        printlog "- saving => 5.2-fileformat"
        hFileSaveAsWithFilterKill( rootdir & DokName2 , sFilter2 )

        printlog "- close"
        FileClose

        kontext "active"
        If active.exists(5) then
            warnlog "Unexpected messagebox: '" + active.getText + "'"
            Active.Yes
        else
            printlog "- no more alien warning on closing the document. Good."
        end if
    else
        FileClose

        kontext "active"
        if Active.Exists(5) then
            warnlog "Unexpected Active after saving: '" + active.gettext + "'"
            try
                Active.Yes
            catch
                Active.No
            endcatch
        end if
    end if

    printlog "- loading => default-fileformat"
    call hFileOpen (rootdir & DokName1)

    if bExport then
        printlog "  - change the doc"
        hChangeDoc()
        printlog "  - save"
        hFileSave()
    endif

    printlog "  - close"
    call hCloseDocument

    if bExport then
        printlog "- loading => 5.2-fileformat"
        call hFileOpen (rootdir & DokName2)
        if (ucase(gApplication) = "MATH" AND iSprache = 84 AND gPlatform="lin") then
            try
                kontext "DocumentMath"
                DocumentMath.MouseDown(50,50)
                DocumentMath.MouseUp(50,50)
                qaErrorLog "Use workaround focus problem for math and language vi/84"
                ' Needed by AndreSchnabel, not manual reproduceable, only with TestTool
            catch
                qaErrorLog "Workaround focus problem failed."
            endcatch
        end if
        printlog "  - change the doc"
        hChangeDoc()
        printlog "  - save"
        hFileSave()

        Kontext "AlienWarning"
        if AlienWarning.exists() then ' was 5 secs.
            printlog "- alien warning came up. Good."
            AlienWarning.OK
            hFileWait() 
        else
            if (gBuild > 8870) then '8871 is CWS alienwarning integrated SRC680m77
                warnlog "- alien warning didn't came up."
            endif
        endif

        kontext "active"
        if Active.Exists(5) then
            if (gBuild > 8870) then '8871 is CWS alienwarning integrated SRC680m77
                warnlog  "Unexpected messagebox: '" + active.gettext + "'"
            else
                printlog "  - alien warning for change to current filter. Good. '" + active.gettext + "'"
            endif
            Active.yes
        end if

        printlog "  - close"
        FileClose

        kontext "active"
        if Active.Exists(5) then
            warnlog  "Unexpected messagebox: '" + active.gettext + "'"
            Active.Yes
        else
            printlog "- no more alien warning on closing the document. Good."
        end if
    end if
    
    if ( FileExists( rootdir & DokName1 ) ) then kill( rootdir & DokName1 )
    if ( FileExists( rootdir & DokName2 ) ) then kill( rootdir & DokName2 )
    
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
        case "WRITER", "HTMLDOKUMENT", "MASTERDOC"
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
            printlog( "   copy"
            gMouseClick ( 1, 1 )
            DocumentImpress.TypeKeys sSelectAll
            DocumentImpress.TypeKeys "<Mod1 c>"
            Wait( 500 )
            printlog( "   paste" )
            gMouseClick ( 1, 1 )
            DocumentImpress.TypeKeys "<Mod1 v>"
        case "MATH"   : SchreibenInMathdok "a over b"
            printlog( "   cut" )
            EditSelectAllMath
            WaitSlot()
            EditCut
            WaitSlot()
            printlog( "   paste" )
            EditPaste
            WaitSlot()
            printlog( "   copy" )
            EditSelectAllMath
            WaitSlot()
            EditCopy
            WaitSlot()
            printlog( "   paste" )
            EditPaste
            WaitSlot()
    end select
end sub



