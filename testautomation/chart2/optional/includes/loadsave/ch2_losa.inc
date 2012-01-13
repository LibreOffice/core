'encoding UTF-8  Do not remove or change this line!
'**************************************************************
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
'**************************************************************
'*
'* short description : Load file of one Spreadsheet format and save to another
'*
'\******************************************************************************

sub subLoSaFromTo

    dim sSourcePath as string
    dim lsInputFileList ( 500 ) as string
    dim iFilesInList as integer
    dim sCurrentFileInList as string
    dim iMainIndex as integer

    dim cRelativePath as string
        cRelativePath = "chart2\optional\input\" & gsDirectoryInSourcePath &"\"

    '///<u><b>Load/Save files with charts</b></u>
    printLog Chr(13) + "---------  Save with filter: " & gsFileFormat & "  ----------"
    sSourcePath = convertPath ( gtesttoolpath & cRelativePath )

    '///+<ul><li>Determine list of files to test (All files in 'chart2\loadsave\input\<gsDirectoryInSourcePath>\')</li>
    GetAllFileList( sSourcePath , "*." & gsDirectoryInSourcePath , lsInputFileList() )
    iFilesInList = ListCount( lsInputFileList() )
    '///+<li>Run testcase 'tLoadSaveFromTo' for each file in list</li></ul>
    for iMainIndex = 2 to iFilesInList

        sCurrentFileInList = lsInputFileList( iMainIndex )
        printlog "* * * * * * *"
        printlog "Loop " & ( iMainIndex - 1 ) & " of " & ( iFilesInList - 1 )
        printlog "Test document: " & sCurrentFileInList
        call tLoadSaveFromTo ( sCurrentFileInList )

    next iMainIndex

end sub

'*******************************************************************************

function hLoSaEscape() as boolean

    ' Fine grained function to send Escape-keystroke to a spreadsheet document

    const MAX_TRIES = 12
    dim iTry as integer

    for iTry = 1 to MAX_TRIES

        wait( 250 )

        Kontext "DocumentCalc"
        try
            DocumentCalc.TypeKeys( "<ESCAPE>" )
            hLoSaEscape() = true
            exit for
        catch
            if ( iTry = MAX_TRIES ) then
                warnlog( "hLoSaEscape(): Sending <ESCAPE> to document failed." )
                hLoSaEscape() = false
            endif
        endcatch

    next iTry

end function

'*******************************************************************************

function hLoSaInplaceEditing() as boolean

    ' Enter in-place editing mode

    const RC_TIMEOUT = -1

    Kontext "DocumentCalc"
    printlog( "Enable in-place editing by slot <EditObjectEdit>" )
    if ( hUseAsyncSlot( "EditObjectEdit" ) <> RC_TIMEOUT ) then

        printlog( "Clicked menuitem: Format->Charttype" )
        if ( hUseAsyncSlot( "FormatChartType" ) <> RC_TIMEOUT ) then

            Kontext "ChartType"
            if ChartType.exists(2) then

                printlog( "Chart type dialog open, Chart object was found." )
                Kontext "ChartType"
                if ( hCloseDialog( ChartType, "cancel" ) ) then
                    printlog( "Chart type dialog closed successfully" )
                    hLoSaInplaceEditing() = true
                else
                    warnlog( "hLoSaInplaceEditing(): Failed to close dialog <ChartType>" )
                    hLoSaInplaceEditing() = false
                endif
            else
                warnlog( "hLoSaInplaceEditing(): Chart type dialog not open, please check the Chart object" )
                hLoSaInplaceEditing() = false
            endif

        else
            warnlog( "hLoSaInplaceEditing(): Failed to execute slot <FormatChartType>" )
            hLoSaInplaceEditing() = false
        endif
    else
        warnlog( "hLoSaInplaceEditing(): Failed to execute slot <EditObjectEdit>" )
        hLoSaInplaceEditing() = false
    endif

end function

'*******************************************************************************

testcase tLoadSaveFromTo ( sCurrentFile as string )

    printlog( "Load/Save files containing Chart OLE objects in multiple file-formats" )

    dim sCurrentFileLength as integer
    dim iCurrentPostionInFileName as integer
    dim iLastPathSignePositon as integer : iLastPathSignePositon = 0
    dim sExtractedFileName as string
    dim sExtractedExtension as string
    dim sTemporaryFile as string
    dim sOutputFile as string
    dim iIndex as integer

    sCurrentFileLength = len ( sCurrentFile )

    'Remove path and extension from file name
    do
        iCurrentPostionInFileName = instr ( iCurrentPostionInFileName + 1 , sCurrentFile , gPathSigne )
        if iCurrentPostionInFileName = 0 then
            exit do
        endif
        iLastPathSignePositon = iCurrentPostionInFileName
    loop

    sExtractedFileName = right ( sCurrentFile , sCurrentFileLength - iLastPathSignePositon )
    sCurrentFileLength = len ( sExtractedFileName )
    sExtractedExtension = right ( sExtractedFileName , 4 )
    sExtractedFileName = left ( sExtractedFileName , sCurrentFileLength - 4 )
    sTemporaryFile = ( gOfficePath & ConvertPath("user\work\TemporaryFileToAvoidLocking" ) )
    sOutputFile = ( gOfficePath & ConvertPath("user\work\" & sExtractedFileName & "_" & gsDirectoryInSourcePath & "_to" & gsOutputFileCustomExtension  ) )

    printlog( "Open file: " & sCurrentFile )
    if ( not hFileOpen( sCurrentFile ) ) then
        warnlog( "Unable to read file" )
        goto endsub
    endif

    printlog( "Save file locally, overwriting existing" )
    if ( not hFileSaveAsKill( sTemporaryFile & sExtractedExtension ) ) then
        warnlog( "Unable to save file locally" )
        goto endsub
    endif

    printlog( "Select the first OLE object via Navigator" )
    call fSelectFirstOLE

    printlog( "Enter in-place editing mode" )
    hLoSaInplaceEditing()

    printlog( "Type <Escape> to exit in-place editing mode" )
    hLoSaEscape()

    printlog( "Save file as " & sOutputFile & " using filter: " & gsFileFormat )
    if ( not hFileSaveAsWithFilterKill ( sOutputFile , gsFileFormat ) ) then
        warnlog( "Saving the document failed using filter: " & gsFileFormat )
        goto endsub
    endif

    printlog( "Close the document" )
    hCloseDocument()

    printlog( "Reload document" )
    if ( not hFileOpen(sOutputFile) ) then
        warnlog( "Failed to reload document" )
        goto endsub
    endif

    printlog( "Select the first (and only) OLE object" )
    call fSelectFirstOLE

    printlog( "Enter in-place editing mode" )
    hLoSaInplaceEditing()

    printlog( "Type <Escape> to exit in-place editing mode" )
    hLoSaEscape()

    printlog( "Close the document" )
    call hCloseDocument()
    
endcase

