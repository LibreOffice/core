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
