'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: ch2_losa.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: obo $ $Date: 2008-07-22 10:36:32 $
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
'* owner : oliver.craemer@sun.com
'*
'* short description : Load file of one Spreadsheet format and save to another
'*
'************************************************************************
'*
' #1 subLoSaFromTo        ' Initial routine
' #1 tLoadSaveFromTo
'*
'\************************************************************************

sub subLoSaFromTo
    Dim sSourcePath as STRING
    Dim lsInputFileList ( 500 ) as STRING
    Dim iFilesInList as INTEGER
    Dim sCurrentFileInList as STRING
    Dim iMainIndex as INTEGER
    
    '///<u><b>Load/Save files with charts</b></u>
    printLog Chr(13) + "---------  Save with filter: " & gsFileFormat & "  ----------"
    sSourcePath = convertPath ( gtesttoolpath & "chart2\optional\input\" & gsDirectoryInSourcePath &"\" )
    '///+<ul><li>Determine list of files to test (All files in 'chart2\loadsave\input\<gsDirectoryInSourcePath>\')</li>
    Call GetAllFileList( sSourcePath , "*." & gsDirectoryInSourcePath , lsInputFileList() )
    '///+<li>Determine number of files found</li>   
    iFilesInList = cInt( lsInputFileList(0) )
    '///+<li>Run testcase 'tLoadSaveFromTo' for each file in list</li></ul>
    for iMainIndex = 2 to iFilesInList
        sCurrentFileInList = lsInputFileList( iMainIndex )
        printlog "* * * * * * *"
        printlog "Loop " & ( iMainIndex - 1 ) & " of " & ( iFilesInList - 1 )
        printlog "Test document: " & sCurrentFileInList
        call tLoadSaveFromTo ( sCurrentFileInList )
    next iMainIndex
End Sub

'*******************************************************************

testcase tLoadSaveFromTo ( sCurrentFile as STRING )
    Dim sCurrentFileLength as INTEGER
    Dim iCurrentPostionInFileName as INTEGER
    Dim iLastPathSignePositon as INTEGER
    Dim sExtractedFileName as STRING
    Dim sExtractedExtension as STRING
    Dim sTemporaryFile as STRING
    Dim sOutputFile as STRING
    Dim iIndex as INTEGER
    
    '///<u><b>Load file - SaveAs... - Close - Relaod</b></u>
    sCurrentFileLength = len ( sCurrentFile )
    iCurrentPostionInFileName = 0
    iLastPathSignePositon = 0
    'Remove path and extension from file name
    DO
        iCurrentPostionInFileName = instr ( iCurrentPostionInFileName + 1 , sCurrentFile , gPathSigne )
        if iCurrentPostionInFileName = 0 then
            Exit DO
        endif
        iLastPathSignePositon = iCurrentPostionInFileName
    LOOP
    sExtractedFileName = right ( sCurrentFile , sCurrentFileLength - iLastPathSignePositon )
    sCurrentFileLength = len ( sExtractedFileName )
    sExtractedExtension = right ( sExtractedFileName , 4 )    
    sExtractedFileName = left ( sExtractedFileName , sCurrentFileLength - 4 )
    sTemporaryFile = ( gOfficePath & ConvertPath("user\work\TemporaryFileToAvoidLocking" ) )
    sOutputFile = ( gOfficePath & ConvertPath("user\work\") &  sExtractedFileName & "_" & gsDirectoryInSourcePath & "_to" & gsOutputFileCustomExtension  )  
    '///+<ul><li>Open test document</li>
    try 
        call hFileOpen(sCurrentFile)
    catch
        warnlog "Serious Problem occured while loading the input file -> Quit"
        goto endsub
    endcatch
    
        '///+<li>Save file localy to avoid locking issues</li>
        if hFileSaveAsKill ( sTemporaryFile & sExtractedExtension ) then
            printlog ">> OK, successfully saved the test document localy!"
        else
               warnlog "Saving the testdocument localy in source format failed!"
        endif
        sleep (2)
    
        '///+<li>Check existence of chart in file using following steps</li>
        '///+<li>1. Select Chart</li>
        printlog ">> Check if chart exist after loading file"
        call fSelectFirstOLE
       
        Kontext "DocumentCalc"
        try
            '///+<li>2. Invoke 'Edit::Object::Edit' to enter inplace mode</li>
            EditObjectEdit
            sleep(2)
            '///+<li>3. Invoke Chart Type</li>
            FormatChartType
            '///+<li>4. Check if Chart Type came up</li>
            Kontext "ChartType"
            if ChartType.exists(2) then
                printlog ">> OK, found a Chart object after loading the file."
            else
                warnlog "Chart Type did not appear -> Check this out!"
            endif
            '///+<li>5. Close Chart Type</li>
            Kontext "ChartType"
            ChartType.Cancel
        catch
            warnlog "Chart does not exist in file or a serious problem occured -> Check this out!" 
        endcatch
        sleep(3)
    
        '///+<li>Hit 'ESCAPE' key to leave inplace mode</li>
        Kontext "DocumentCalc"
        try 
            DocumentCalc.TypeKeys "<ESCAPE>"            
        catch
            warnlog "CRASH???"
            goto endsub
        endcatch
    
        '///+<li>Save file using ODS format</li>
        printlog "Save file as " & sOutputFile
        try
            if hFileSaveAsWithFilter ( sOutputFile , gsFileFormat ) then
                printlog ">> OK, successfully saved the test document!"
            else
                warnlog "Saving the testdocument failed!"
            endif
        catch
            warnlog "A serious problem occured while saving file to target format -> Check this out!"
            goto endsub
        endcatch
        sleep(2)
    
        '///+<li>Close document</li>
        printlog "Closing test document"
    call hCloseDocument
    sleep(5)

    '///+<li>Open document again</li>
    printlog "Reloading saved document"
    try 
        call hFileOpen(sOutputFile)
    catch
        warnlog "Serious Problem occured while reloading the output file -> Quit"
        goto endsub
    endcatch
        sleep(2)

        '///+<li>Check existence of chart after saving to target format in file using following steps</li>
        '///+<li>1. Select Chart</li>
        printlog ">> Check if chart exist after saving file to target format"
        call fSelectFirstOLE
   
        Kontext "DocumentCalc"
        try
            '///+<li>2. Invoke 'Edit::Object::Edit' to enter inplace mode</li>
            EditObjectEdit
            '///+<li>3. Invoke Chart Type</li>
            FormatChartType
            '///+<li>4. Check if Chart Type came up</li>
            Kontext "ChartType"
            if ChartType.exists(2) then
                printlog ">> OK, found a Chart object after saving the file to target format."
            else
                warnlog "Chart Type did not appear after saving the file to target format -> Check this out!"
            endif
            '///+<li>5. Close Chart Type</li>
            Kontext "ChartType"
            ChartType.Cancel
        catch
            warnlog "Chart does not exist in file or a serious problem occured after saving the file to target format -> Check this out!" 
        endcatch
        sleep(3)

        '///+<li>Hit 'ESCAPE' key to leave inplace mode</li>
        Kontext "DocumentCalc"
        try 
            DocumentCalc.TypeKeys "<ESCAPE>"            
        catch
            warnlog "CRASH???"
            goto endsub
        endcatch

        '///+<li>Close document</li>
        printlog "Closing test document"
    call hCloseDocument
    '///+<li>END</li></ul>
endcase

