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
'**                                                                
'** owner : oliver.craemer@oracle.com                                  
'**                                                                
'** short description : Imports every Document from a given path (gsSourcePath variable)     
'**                                                                
'*************************************************************************
'**                                                               
' #1 tLoadAllDocuments (gsSourcePath as string )                    
' #1 tLoadSpreadsheetDocument
' #1 fCalcFileOpen                                                   
'**                                                               
'\*******************************************************************

sub tLoadAllDocuments ( gsSourcePath as string )
    Dim i as integer 
    Dim x as integer

    dim gsSourceDocument( 50 ) as string

    printlog "Source path: " & gsSourcePath
    'Get the files into a list.    
    Call GetAllFileList(gsSourcePath, "*.*" , gsSourceDocument())
    x = ListCount(gsSourceDocument())
    printlog "- " + x + " steps will be done"
    for i = 1 to x
        printlog "(" & i & "/" & x & "): " & gsSourceDocument(i)
        Call tLoadSpreadsheetDocument(gsSourceDocument(i))
        sleep(5)
    next i
end sub
'
'-------------------------------------------------------------------------------
'
testcase tLoadSpreadsheetDocument (SourceFile)

    Dim sOnlyFileName as string
    Dim sOnlyFileExtension as string
    Dim sTempFileName as String
    Dim i as integer

    sTempFileName = SourceFile
    sOnlyFileName = DateiOhneExt(DateiExtract(SourceFile)
    sOnlyFileExtension = GetExtention(sTempFileName)

    select case ( sOnlyFileName )
    case "so3tmpl", "so4tmpl", "so5tmpl" : printlog  "Test case: " & sOnlyFileName & "." & sOnlyFileExtension
    case else :              printlog  "Test case: " & sOnlyFileName & "." & sOnlyFileExtension
    end select

    'Maybe file name is empty
    if sOnlyFileName > "" then
        'Some extensions make no sense to load
        if sOnlyFileExtension <> "so" AND sOnlyFileExtension <> "tmp" AND sOnlyFileExtension <> "dbt" then
            if fCalcFileOpen(SourceFile) then
                if (hIsNamedDocLoaded (SourceFile)) then
                    printlog "-  loaded"
                    sleep(15)
                    Call hCloseDocument
                else
                    warnlog sOnlyFileName + " document has not been loaded correctly [hIsNamedDocLoaded]!"                    
                    hFileCloseAll()
                endif
            else
                warnlog sOnlyFileName + " document has not been loaded correctly [fCalcFileOpen; IsItLoaded]!"                
                hFileCloseAll()
            endif
        else
            printlog "(" + sOnlyFileName + "." + sOnlyFileExtension + ") won't be loaded because of excluded extensions .so and .tmp."
            hFileCloseAll()
        endif
    endif
endcase
'
'-------------------------------------------------------------------------------
'
function fCalcFileOpen (sDocName as string, optional bLinked as boolean) as boolean
    Dim sSourceFile as String
    'Loads the spreadsheet documents like hLoadDocument but with special handling of
    'spreadsheet import filter dialogs, like ASCII, Lotus, dBase, CSV, ... 
    sSourceFile = ConvertPath ( sDocName )

    fCalcFileOpen = TRUE
    if FileExists (sSourceFile) = FALSE then
        warnlog "fCalcFileOpen: '" + sSourceFile + "' does not exists!"
        fCalcFileOpen = FALSE
    else
        'Slot: FileOpen
        FileOpen
        sleep (1)
        Kontext "OeffnenDlg"
        Dateiname.SetText(sSourceFile)
        sleep (3)
        Oeffnen.Click
        sleep (20)

        'If .txt ASCII-filter dialog will come up.
        Kontext "AsciiFilterOptionen"
        if AsciiFilterOptionen.Exists(1) then
            printlog "- ASCII filter dialog is up!"
            AsciiFilterOptionen.Ok
        endif

        'If Lotus 1-2-3 or dBase files will be opend the import characters
        'dialog will come up
        Kontext "ExportCalc"
        if ExportCalc.Exists(1) then
            printlog "- Import character set selection dialog is up!"
            ExportCalc.Ok
        endif

        'Importing detected (e.g. extension .csv) CSV files will bring
        ' up the Text import dialog
        Kontext "TextImport"
        if TextImport.Exists(1) then
            printlog "- Text import dialog is up!"
            TextImport.OK
        endif

        Kontext "Active"
        if Active.Exists(3) then
            if IsMissing(bLinked) then
                warnlog "fCalcFileOpen: " + Active.GetText
            else
                printlog "fCalcFileOpen: " + Active.GetText                
            end if
            try
                Active.OK
            catch
                if IsMissing(bLinked) then
                    'e.g. Links won't be updated
                    Active.No
                else
                    'e.g. Links will be updated
                    Active.Yes
                end if
            endcatch
            Kontext "OeffnenDlg"
            if OeffnenDlg.Exists(1) then
                OeffnenDlg.Cancel
                fCalcFileOpen = FALSE
                Warnlog "fCalcFileOpen: The File-Open-Dialog is open!"
            end if
        end if
    end if
end function
