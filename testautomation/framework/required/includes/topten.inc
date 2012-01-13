'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************

*****

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



