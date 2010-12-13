'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
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
'* Owner : thorsten.bosbach@oracle.com
'*
'* short description : Global routines for loading, saving, printing, export
'*
'\******************************************************************************

function hGrafikEinfuegen( cFile as string ) as Boolean

    hGrafikEinfuegen() = hInsertGraphic( cFile, "Static" )
    
end function

'*******************************************************************************

function hGrafikVerknuepftEinfuegen( cFile as string ) as Boolean

    hGrafikVerknuepftEinfuegen() = hInsertGraphic( cFile, "Linked" )

end function

'*******************************************************************************

function hInsertGraphic( byval cFile as string, byval cMode as string ) as boolean

    ' Insert a graphic file to the current document either linked or static
    ' cFile: Fully qualified path to the file to be inserted
    ' cMode: Supply "Linked" to link, empty string or anything else is static

    ' In difference to the older functions (GrafikEinfuegen etc.) this function
    ' does provide a working returnvalue and does not secretly hide warnings and
    ' errors. It does not set the file type.

    ' Make hFileWait() complain about any messageboxes
    const SHOW_WARNINGS = true

    ' Find out whether we link the file or not
    dim bLinked as boolean : bLinked = false
    if ( lcase( cMode ) = "linked" ) then bLinked = true

    hInsertGraphic() = false
    cFile = convertpath( cFile )

    if ( FileExists( cFile ) ) then
        InsertGraphicsFromFile

        kontext "GrafikEinfuegenDlg"
        if ( GrafikEinfuegenDlg.exists( 3 ) ) then

            Vorschau.unCheck()
            if ( Verknuepfen.isEnabled() ) then
                if ( bLinked ) then
                    Verknuepfen.check()
                else
                    Verknuepfen.unCheck()
                endif
            endif

            DateiName.setText( cFile )
            Oeffnen.click()

            if ( hFileWait( SHOW_WARNINGS ) >= 0 ) then hInsertGraphic() = true
            
        else
            warnlog( "hInsertGraphic(): Failed to open <Insert Graphics> dialog" )
        endif
    else
        warnlog( "hInsertGraphic(): File does not exist: " & cFile )
    endif

end function

'*******************************************************************************

function hIsNamedDocLoaded( cFileName as string ) as boolean

    ' Retrieve the current filename from the document properties - which in
    ' comparision to the old approach to retrieve the name from the file save
    ' dialog - gives us the file including its extension. So we only need
    ' to compare the last characters from a full path to be relatively
    ' certain that we work with the correct file. There is some small margin of
    ' error, though.

    const CFN = "global::tools::includes::required::hIsNamedDocumentLoaded(): "
    const RC_FAILURE = -1

    dim cDocumentName as string
    dim iDocumentNameLength as integer
    dim bTemplateDocument as boolean

    hIsNamedDocLoaded() = false

    ' If the requested document is a template, we cannot use its filename to
    ' identify the document as it is named "Untitled 1" or "Untitled" in the
    ' FileSave dialog and the document properties dialog. The string is locale
    ' dependent. So we do a shortcut here, the calling tests fail at a later
    ' point if this function does not work properly
    
    select case( right( cFileName, 4 ) )
    case ".vor", ".dot", ".sti", ".pot", ".std", ".xlt", ".stc", ".ots", "xltm", "xltx"
        if ( GVERBOSE ) then printlog( CFN & "Template document filtered" )
        hIsNamedDocLoaded() = true
        exit function
    end select

    if ( hUseAsyncSlot( "FileProperties" ) <> RC_FAILURE ) then

        kontext
        active.setpage(tabdokument)

        kontext "TabDokument"
        if ( TabDokument.exists( 2 ) ) then

            cDocumentName = DokumentName.getText()
            iDocumentNameLength = len( cDocumentName )

            if ( GVERBOSE ) then
                printlog( CFN & "Expected file: " & cFileName )
                printlog( CFN & "Current file.: " & cDocumentName )
                printlog( CFN & "Comparing last " & iDocumentNameLength & " characters" )
            endif

            if ( right( cFileName, iDocumentNameLength ) = cDocumentName ) then
                if ( GVERBOSE ) then printlog( CFN & "This is the expected file" )
                hIsNamedDocLoaded() = true
            else
                if ( GVERBOSE ) then printlog( CFN & "This is *not* the expected file" )
            endif

            hCloseDialog( TabDokument, "cancel" )
        else
            warnlog( CFN & "Unable to open dialog <Document Properties>" )
        endif
    else
        warnlog( CFN & "Unable to execute slot <FileProperties>" )
    endif

end function

'*******************************************************************************

function hExportAsPDFmulti (iTypeOfCall as integer, bExecute as boolean, sFileName as string, bAutoExtension as boolean, bOverwriteFile as boolean, iRange as integer, iConpression as integer, optional sRange as string) as boolean
    '/// Export a document to PDF with various options.
    '///+ INPUT
    '///+<ul><li>iTypeOfCall</li><ul><li>1: via  the icon in the functionbar (no options selectable, since only file dialog comes up!)</li>
    '///+<li>2: via File->Send->Document As PDF Attachment... (makes no sense either, because after the mail window comes up... (you can't handle by the testtool))</li>
    '///+<li>3 via File->Export As PDF... (RECOMMENDED)</li></ul>
    '///+<li>bExecute: Shall all dialogs left with OK? TRUE (RECOMMENDED) / FALSE</li>
    '///+<li>sFileName: Filename to use</li>
    '///+<li>bAutoExtension: Shall the file extension be chosen by StarOffice? TRUE / FALSE (RECOMMENDED)</li>
    '///+<li>bOverwriteFile: If filename already exists should it be overwritten? TRUE (RECOMMENDED) / FALSE</li>
    '///+<li>iRange:</li><ol><li>1: All (RECOMMENDED)</li>
    '///+<li>2: Pages: The range is set in the optional parameter 'sRange' at the end</li>
    '///+<li>3: Selection</li></ol>
    '///+<li>iConpression:</li><ol><li>1: JpegCompression check and ReduceImageResolution check</li>
    '///+<li>2: JpegCompression check and ReduceImageResolution uncheck</li>
    '///+<li>3: LosslessCompression check and ReduceImageResolution uncheck</li></ol>
    '///+<li>sRange: Optional parameter only used for iRange=2; takes the page range as text</li>
    '///+<li>RETURN: Any Errors? TRUE / FALSE</li></ul>
    dim sPDF as string
    dim sTemp as string
    dim iTemp as integer
    dim sTFileName as string
    dim iCount as integer

    sPDF = "PDF - Portable Document Format (.pdf)"
    hExportAsPDFmulti = TRUE ' optimistic
    iTemp = 0

    select case (iTypeOfCall)
    case 1 : '/// click the button 'Export Directly as PDF' on the Functionbar ///'
        kontext "Standardbar"
        try
            ExportAsPDF.click
        catch
            Warnlog "Button 'Export directly as PDF' not accessible!"
            hExportAsPDFmulti = FALSE
            exit function
        endcatch
    case 2 : '/// File->Send->Document As PDF Attachment... ///'
        try
            FileSendDocumentAsPDF
        catch
            Warnlog "'Export as PDF' not accessible!"
            hExportAsPDFmulti = FALSE
            exit function
        endcatch
    case 3 : '/// File->Export As PDF... ///'
        try
            FileExportAsPDF ' works in draw/impress, too but is not 'legal' in the UI :-)
        catch
            Warnlog "Button 'Export as PDF' not accessible!"
            hExportAsPDFmulti = FALSE
            exit function
        endcatch
    end select

    ' only if the type is 1 OR 3 then the File Save dialog appear
    ' just kept here for compatibility reasons - has to get removed around july 2007
    if (iTypeOfCall = 1 OR iTypeOfCall = 3) then
        kontext "ExportierenDlg"
        if ExportierenDlg.exists(5) then
            try
                Dateityp.Select sPDF
            catch
                Warnlog "Unable to select filter: '" + sPDF + "'"
                ExportierenDlg.Cancel
                hExportAsPDFmulti = FALSE
                exit function
            endcatch

            sTemp = Dateityp.GetSelText
            if (sTemp <> sPDF) then
                Warnlog "filter for PDF export is missing :-( '" + sPDF + "'"
            end if
            '/// set Textbox 'File name' ///'
            Dateiname.SetText sFileName
            'if the file has no pdf extension then add the extension
            if (lCase(right(sFileName, 4))=".pdf") then
                sTFileName = sFileName
            else
                sTFileName = sFileName + ".pdf"
            endif
            '/// click on the button 'Export...' ///'
            if (bExecute) then
                speichern.Click
                kontext "AlienWarning"
                if AlienWarning.exists(5) then
                    warnlog "#i41983# Alien Warning on export not allowed."
                    AlienWarning.OK
                endif
                if (iTypeOfCall = 1) then
                    ''Export Directly as PDF'
                endif
                iCount = 0
                ' wait until file exists for max 5 minutes
                while ((dir(sTFileName)="") AND (iCount < 30))
                    sleep 10
                    inc iCount
                wEnd
            endif
        else
            'ExportierenDlg.Cancel
        end if

        ' if file exists, there is a message...
        kontext "PDFOptions"
        if NOT PDFOptions.exists then
            Kontext
            if messagebox.exists (5) then
                if (bOverwriteFile) then
                    messagebox.Yes
                else
                    messagebox.No
                    kontext "ExportierenDlg"
                    ExportierenDlg.Cancel
                    hExportAsPDFmulti = FALSE
                end if
            end if
        end if
    end if


    ' only if the type is 2 OR 3 then the PDF option dialog appear
    if (iTypeOfCall = 2 OR iTypeOfCall = 3) then
        kontext "PDFOptions"
        select case (iRange)
        case 1 : '/// check radiobutton 'All' ///'
            try
                RangeAll.Check
            catch
                Warnlog "Radiobutton 'All' not accessible!"
                PDFOptions.Cancel
                hExportAsPDFmulti = FALSE
                exit function
            endcatch
        case 2 : '/// check radiobutton 'Pages' ///'
            try
                RangePages.Check
            catch
                Warnlog "Radiobutton 'Range' not accessible!"
                PDFOptions.Cancel
                hExportAsPDFmulti = FALSE
                exit function
            endcatch
            if isMissing(sRange) then
                Warnlog "parameter 'sRange' in function 'hExportAsPDFmulti' is not optional if 'iRange' = 2"
                hExportAsPDFmulti = FALSE
            else
                '/// set range textbox the given value ///'
                RangePagesEdit.SetText sRange
            end if
        case 3 : '/// check radiobutton 'Selection' (not enabled in math!) ///'
            try
                if (RangeSelection.IsEnabled <> TRUE)then
                    Warnlog "RangeSelection is disabled :-("
                else
                    RangeSelection.Check
                end if
            catch
                Warnlog "Radiobutton 'Selection' not accessible!"
                PDFOptions.Cancel
                hExportAsPDFmulti = FALSE
                exit function
            endcatch
        end select

        select case (iConpression)
        case 1 : '/// JpegCompression check and ReduceImageResolution check///'
            try
                JpegCompression.Check
                ReduceImageResolution.Check
            catch
                Warnlog "Radiobutton 'Reduce Image Resolution' not accessible!"
                PDFOptions.Cancel
                hExportAsPDFmulti = FALSE
                exit function
            endcatch
        case 2 : '/// JpegCompression check and ReduceImageResolution uncheck ///'
            try
                JpegCompression.Check
                ReduceImageResolution.UnCheck
            catch
                Warnlog "Radiobutton 'Jpeg Compression' not accessible!"
                PDFOptions.Cancel
                hExportAsPDFmulti = FALSE
                exit function
            endcatch
        case 3 : '/// LosslessCompression check and ReduceImageResolution uncheck ///'
            try
                LosslessCompression.Check
                ReduceImageResolution.UnCheck
            catch
                Warnlog "Radiobutton 'Lossless Compression' not accessible!"
                PDFOptions.Cancel
                hExportAsPDFmulti = FALSE
                exit function
            endcatch
        end select
        if (bExecute) then
            try
                PDFOptions.OK
            catch
                qaErrorlog "ImprovementOfSave: PDF export loops?"
            endcatch
        else
            PDFOptions.Cancel
        end if
    end if
    ' only if the type is 1 OR 3 then the File Save dialog appear
    kontext "ExportierenDLG"
    if (iTypeOfCall = 1 OR iTypeOfCall = 3) then
        if ExportierenDLG.exists(5) then
            try
                Dateityp.Select sPDF
            catch
                Warnlog "Unable to select filter: '" + sPDF + "'"
                ExportierenDLG.Cancel
                hExportAsPDFmulti = FALSE
                exit function
            endcatch

            sTemp = Dateityp.GetSelText
            if (sTemp <> sPDF) then
                Warnlog "filter for PDF export is missing :-( '" + sPDF + "'"
            end if
            '/// set Textbox 'File name' ///'
            Dateiname.SetText sFileName
            'if the file has no pdf extension then add the extension
            if (lCase(right(sFileName, 4))=".pdf") then
                sTFileName = sFileName
            else
                sTFileName = sFileName + ".pdf"
            endif
            '/// click on the button 'Export...' ///'
            if (bExecute) then
                Speichern.Click
                kontext "AlienWarning"
                if AlienWarning.exists(5) then
                    warnlog "#i41983# Alien Warning on export not allowed."
                    AlienWarning.OK
                endif
                if (iTypeOfCall = 1) then
                    ''Export Directly as PDF'
                endif
                iCount = 0
                ' wait until file exists for max 5 minutes
                while ((dir(sTFileName)="") AND (iCount < 30))
                    sleep 10
                    inc iCount
                wEnd
            endif
        else
            try
            ExportierenDLG.Cancel
            catch
            printlog "export not there"
            endcatch
        end if

        ' if file exists, there is a message...
        kontext "PDFOptions"
        if NOT PDFOptions.exists then
            Kontext
            if messagebox.exists (5) then
                if (bOverwriteFile) then
                    messagebox.Yes
                else
                    messagebox.No
                    kontext "ExportierenDLG"
                    ExportierenDLG.Cancel
                    hExportAsPDFmulti = FALSE
                end if
            end if
        end if
    end if

end function
'
'-------------------------------------------------------------------------------
'
function hFileDelete( cFileOrig as string ) as boolean
    hFileDelete() = hDeleteFile( cFileOrig )
end function
'
'-------------------------------------------------------------------------------
'
function hDeleteFile( cFileOrig as string ) as boolean

    const CFN = "global::tools::includes::required::t_files.inc::hDeleteFile():"
    dim cHome as string : cHome = convertpath( gOfficePath & "user" )
    dim cFile as string : cFile = convertpath( cFileOrig )

    ' This function tries to delete a file and does some very basic error-
    ' handling. Returns 'true' on success, only error while deleting returns
    ' 'false', if the file does not exist, it is considered to be successfully
    ' deleted.
    ' I introduced this function due to a number of cases where deleting files
    ' actually failed because of weird code or situations where the user lacks
    ' accessrights to files are not handled at all.

    if ( instr( cFile , cHome ) = 0 ) then
        qaerrorlog( CFN & "Trying to delete file outside (default)homedir -> forbidden" )
        qaerrorlog( CFN & "Home: " & cHome )
        qaerrorlog( CFN & "File: " & cFile )
        hDeleteFile() = false
        exit function
    endif

    cFile = convertpath( cFile )

    if ( FileExists( cFile ) ) then
        try
            kill( cFile )

            if ( FileExists( cFile ) ) then
                warnlog( CFN & "File was not deleted: " & cFile )
                hDeleteFile() = false
            else
                if ( GVERBOSE ) then printlog( CFN & "File successfully deleted: " & cFile )
                hDeleteFile() = true
            endif
        catch
            qaerrorlog( CFN & "Deleting file failed: " & cFile )
            hDeleteFile() = false
        endcatch
    else
        printlog( CFN & "Nothing to do." )
        hDeleteFile() = true
    endif

end function
'
'-------------------------------------------------------------------------------
'
function hGetWorkFile( cFileName as string ) as string
    hGetWorkFile() = hGetWorkPath() & cFileName
end function
'
'-------------------------------------------------------------------------------
'
function hGetWorkPath() as string

    ' Retrieve the user's work directory using the API (or fallback)

    dim sPath as string : sPath = ""
    dim oOfficeConnect as object
    dim oOfficeConfig as object
    dim bPathIsFromAPI as boolean

    const CFN = "global::tools::includes::required::t_files.inc:hGetWorkPath(): "

    ' Create an UNO service and ask it for the location of Work
    try
        oOfficeConnect = hGetUnoService( true )
        oOfficeConfig  = oOfficeConnect.createInstance( "com.sun.star.util.PathSettings" )
        sPath = convertFromURL( oOfficeConfig.Work )
        bPathIsFromAPI = true
    catch
        printlog( CFN & "Could not access service, connection broken?" )
        sPath = convertpath( gOfficePath & "user/work" )
        bPathIsFromAPI = false
    endcatch

    if ( GVERBOSE ) then

        printlog( CFN & "Path is: " & sPath )

        ' Check path existence
        if ( FileExists( sPath ) ) then
            printlog( CFN & "Path exists." )
        else
            warnlog( CFN & "Path does not exist." )
        endif

        ' Inform about the location from where the path was taken
        if ( bPathIsFromAPI ) then
        	printlog( CFN & "Path was taken from API" )
        else
            warnlog( CFN & "Path was taken from (hardcoded) fallback" )
        endif

    endif

    hGetWorkPath() = sPath & getPathSeparator

end function
'
'-------------------------------------------------------------------------------
'
function hFileGetLocalPath( sSourcePath as string ) as string

    ' Isolate filename from path, prepend local work directory and return new file path
    hFileGetLocalPath() = hGetWorkFile( hSplitString( sSourcePath, getPathSeparator, 0 ) )
    
end function
'
'-------------------------------------------------------------------------------
'
function hFileListCopyLocal( lsSourceList() as string ) as boolean

    ' Copy a list of files from testautomation to the local work directory

    const CFN = "global::tools::includes::required::t_files.inc:hFileListCopyLocal(): "
    dim iCurrentPath as integer
    dim cTargetPath as string
    dim cSourcePath as string

    hFileListCopyLocal() = true

    for iCurrentPath = 1 to listcount( lsSourceList() )

        cSourcePath = lsSourceList( iCurrentPath )
        cTargetPath = hFileGetLocalPath( cSourcePath )

        FileCopy( cSourcePath, cTargetPath )

        ' If any one copy operation fails we
        if ( not FileExists( cTargetPath ) ) then
            hFileListCopyLocal() = false
            warnlog( CFN & "File was not copied: " & cSourcePath )
        endif

    next iCurrentPath

end function
'
'-------------------------------------------------------------------------------
'
function hFileOpenLocally( byVal sSourcePath as string ) as boolean

    ' Issue #i112208# - implement a function that copies a file from testautomation
    ' into the local work directory and opens it from there to avoid that the
    ' document gets opened in read-only mode. See details of the implementation
    ' in the issue description.

    ' Parameters
    ' 1) Path as string, URLs are untested.
    ' 2) Separator, usually "getPathSeparator"
    ' 3) Item number, 0 = last item which usually is the file name from a path.
    '    Begin counting with 1, while the function internally accesses item 0
    '    from the array.

    const CFN = "global::tools::includes::required::t_files.inc:hFileOpenLocally(): "

    dim sTargetPath as string

    sSourcePath = convertpath( sSourcePath )

    ' If the source file does not exist we quit
    if ( not FileExists( sSourcePath ) ) then
        warnlog( CFN & "Source path/file does not exist: " & sSourcePath )
        hFileOpenLocally() = false
        exit function
    endif

    ' Find out what the name of the target file is going to be
    sTargetPath = hFileGetLocalPath( sSourcePath ) ' The path + file

    ' For debugging purposes set global variable gVerbose to TRUE
    if ( gVerbose ) then
        printlog( CFN & "Copying file" )
        printlog( CFN & "From: " & sSourcePath )
        printlog( CFN & "To..: " & sTargetPath )
    endif

    ' Copy the file from anywhere to the local user directory if it does not
    ' exist. This behavior was discussed on IRC
    if ( FileExists( sTargetPath ) ) then
        printlog( CFN & "Re-using existing local copy of workfile" )
        'warnlog( CFN & "Target file exists, it has not been deleted by prior test" )
    endif

    FileCopy( sSourcePath, sTargetPath )

    ' Verify that the file has been created. return an empty string and warn
    if ( not FileExists( sTargetPath ) ) then
        warnlog( CFN & "File was not copied: " & sTargetFile )
        hFileOpenLocally() = false
        gLastWorkFile = ""
        exit function
    endif

    ' This is a hook that allows to access the filename under which the file
    ' has been saved. Due to restrictive coding guidelines we cannot return
    ' the new filename.
    gLastWorkFile = sTargetPath

    ' Finally open the file from the local work directory. It should open
    ' ready for editing
    hFileOpenLocally() = hFileOpen( gLastWorkFile )

end function
'
'-------------------------------------------------------------------------------
'
function hFileCloseAll() as integer

    dim iDocumentCount as integer : iDocumentCount = 0
    Do Until GetDocumentCount = 0
        Call hCloseDocument()
        iDocumentCount = iDocumentCount + 1
    Loop
    hFileCloseAll() = iDocumentCount
    
end function
'
'-------------------------------------------------------------------------------
'
function hFileOpen( cFile as string ) as boolean

    dim sFile as string : sFile = convertToURL( convertpath( cFile ) )
    hFileOpen() = FALSE
    const CFN = "global::tools::inc::t_files.inc::hFileOpen(): "
    if ( GVERBOSE ) then printlog( CFN & "Load: " & sFile )
    FileOpen( "URL", sFile, "FrameName", "_default" )
    if ( hFileWait( FALSE ) >= 0 ) then hFileOpen() = TRUE

end function
'
'-------------------------------------------------------------------------------
'
function hFileOpenSpecial( cFile as string, cFlag as string ) as boolean

    ' Open a file with some special options
    ' cFlag = "ReadOnly" opens file read-onlx
    ' cFlag = "AsTemplate" opens file as templat
    ' cFlag = <Any other string> treats string as password

    dim sFile as string : sFile = convertToURL( convertpath( cFile ) )
    const CFN = "global::tools::inc::t_files.inc::hFileOpenSpecial(): "
    hFileOpenSpecial() = FALSE
    if ( GVERBOSE ) then printlog( "Load (Flag): " & sFile & " (" & cFlag & ")" )

    select case( lcase( cFlag ) )
    case "readonly"
        printlog( CFN & "Load file read-only: " & sFile )
        FileOpen( "URL", sFile, "FrameName", "_default", "ReadOnly", TRUE)
    case "astemplate"
        printlog( CFN & "Load file as template: " & sFile )
        FileOpen( "URL", sFile, "FrameName", "_default", "AsTemplate" , TRUE )
    case else
        printlog( CFN & "Load file with password: " & sFile )
        FileOpen( "URL", sFile, "FrameName", "_default", "Password" , cFlag )
    end select

    if ( hFileWait( FALSE ) >= 0 ) then hFileOpenSpecial() = TRUE

end function
'
'-------------------------------------------------------------------------------
'
function hFileOpenWithFilter( cFile as string, cFilter as string )

    dim sFile as string : sFile = convertToURL( convertpath( cFile ) )
    const CFN = "global::tools::inc::t_files.inc::hFileOpenWithFilter(): "
    hFileOpenWithFilter() = FALSE
    if ( GVERBOSE ) then printlog( CFN & "Load (Filter): " & sFile & " (" & cFilter & ")" )
    FileOpen( "URL", sFile, "FrameName", "_default", "FilterName", cFilter )
    if ( hFileWait( FALSE ) >= 0 ) then hFileOpenWithFilter() = TRUE

end function
'
'-------------------------------------------------------------------------------
'
function hFileSave() as boolean

    const CFN = "global::tools::inc::t_files.inc::hFileSave(): "
    hFileSave() = FALSE
    if ( GVERBOSE ) then printlog( CFN & "Save file." )
    FileSave( "SynchronMode", TRUE )
    if ( hFileWait( TRUE ) >= 0 ) then hFileSave() = TRUE

end function
'
'-------------------------------------------------------------------------------
'
function hFileSaveAs( cFile as string ) as boolean

    dim sFile as string : sFile = convertToURL( convertpath( cFile ) )
    const CFN = "global::tools::inc::t_files.inc::hFileSaveAs(): "
    hFileSaveAs() = FALSE
    if ( GVERBOSE ) then printlog( CFN & "Save: " & sFile )
    FileSaveAs( "URL", sFile, "Overwrite", FALSE )
    if ( hFileWait( TRUE ) >= 0 ) then hFileSaveAs() = TRUE

end function
'
'-------------------------------------------------------------------------------
'
function hFileSaveAsKill( cFile as string ) as boolean

    dim sFile as string : sFile = convertToURL( convertpath( cFile ) )
    const CFN = "global::tools::inc::t_files.inc::hFileSaveAsKill(): "
    hFileSaveAsKill() = FALSE
    if ( GVERBOSE ) then printlog( CFN & "Save, replacing: " & sFile )
    FileSaveAs( "URL", sFile, "Overwrite", TRUE )
    if ( hFileWait( TRUE ) >= 0 ) then hFileSaveAsKill() = TRUE

end function
'
'-------------------------------------------------------------------------------
'
function hFileSaveAsKillWithPassword( cFile as string, cPassword as string ) as boolean

    dim sFile as string : sFile = convertToURL( convertpath( cFile ) )
    const CFN = "global::tools::inc::t_files.inc::hFileSaveAsKillWithPassword(): "
    hFileSaveAsKillWithPassword() = FALSE
    if ( GVERBOSE ) then printlog( CFN & "Save with password, replacing: " & sFile & "::" & cPassword )
    FileSaveAs( "URL", sFile, "Overwrite", TRUE, "Password", cPassword )
    if ( hFileWait( TRUE ) >= 0 ) then hFileSaveAsKillWithPassword() = TRUE

end function
'
'-------------------------------------------------------------------------------
'
function hFileSaveAsWithFilter( cFile as string, cFilter as string ) as boolean

    dim sFile as string : sFile = convertToURL( convertpath( cFile ) )
    const CFN = "global::tools::inc::t_files.inc::hFileSaveAsWithFilter(): "
    hFileSaveAsWithFilter() = FALSE
    if ( GVERBOSE ) then printlog( CFN & "Save with filter: " & sFile & "::" & cFilter )
    FileSaveAs( "URL", sFile, "FilterName", cFilter, "Overwrite", FALSE )
    if ( hFileWait( TRUE ) >= 0 ) then hFileSaveAsWithFilter() = TRUE

end function
'
'-------------------------------------------------------------------------------
'
function hFileSaveAsWithFilterKill( cFile as string, cFilter as string ) as boolean

    dim sFile as string : sFile = convertToURL( convertpath( cFile ) )
    const CFN = "global::tools::inc::t_files.inc::hFileSaveAsWithFilterKill(): "
    hFileSaveAsWithFilterKill() = FALSE
    if ( GVERBOSE ) then printlog( CFN & "Save with filter, replacing: " & sFile & "::" & cFilter )
    FileSaveAs( "URL", sFile, "FilterName", cFilter, "Overwrite", TRUE )
    if ( hFileWait( TRUE ) >= 0 ) then hFileSaveAsWithFilterKill() = TRUE

end function
'
'-------------------------------------------------------------------------------
'
function hFileWait( optional bWarn as boolean ) as integer

    ' Wait max 10 seconds for the FileOpen/FileSave slot to finish
    ' Returns the time it took the slot to finish
    ' Negative returnvalues are given on timeout or any blocking dialog

    const CFN = "global::tools::inc::t_files.inc::hFileWait(): "
    const MAX_WAIT = 10
    dim iWait as integer : iWait = 0
    dim bWarnlog as boolean

    if ( IsMissing( bWarn ) ) then
        bWarnlog = TRUE
    else
        bWarnlog = bWarn
    endif

    do while( WaitSlot() <> WSFinished )

        iWait = iWait + 1

        ' This is the timeout
        if ( iWait = MAX_WAIT ) then
            iWait = -1
            exit do
        endif

        ' LOAD: Security warning prevents slot from finishing
        kontext "SecurityWarning"
        if ( SecurityWarning.exists() ) then
            iWait = -2
            exit do
        endif

        ' LOAD: Password dialog
        kontext "PasswordFileOpen"
        if ( PasswordFileOpen.exists() ) then
            iWait = -3
            exit do
        endif

        ' SAVE: Password dialog
        kontext "PasswordDlg"
        if ( PasswordDlg.exists() ) then
            iWait = -4
            exit do
        endif
        
        ' LOAD: Master Password Dialog
        kontext "MasterPasswordEnter"
        if ( MasterPasswordEnter.exists() ) then
            iWait = -10
            warnlog( CFN & "Master Password dialog displayed. Tests are likely to fail" )
            exit do
        endif

        ' LOAD: Filter selection dialog
        Kontext "Filterauswahl"
        if ( FilterAuswahl.exists() ) then
            iWait = -5
            exit do
        endif

        ' LOAD: ASCII Filter Options dialog
        kontext "ASCIIFilterOptionen"
        if AsciiFilterOptionen.Exists() then
            iWait = -6
            exit do
        end if

        ' LOAD/SAVE: Some other dialogs that may interrupt file I/O
        kontext "Active"
        if ( Active.exists() ) then
            printlog( Active.getText() )

            ' This is most likely a warning
            if ( Active.getButtonCount() = 1 ) then
                iWait = -7
                exit do
            endif

            ' This is probably the update links dialog
            if ( Active.getButtonCount() = 2 ) then
                iWait = -8
                exit do
            endif

            ' This is probably the styles not matching dialog
            if ( Active.getButtonCount() = 3 ) then
                iWait = -9
                exit do
            endif

        endif

    loop

    if ( GVERBOSE ) then
        select case iWait
        case -1   : printlog( CFN & "Timeout reached, rc = " & iWait )
        case -2   : printlog( CFN & "Macro security warning displayed, rc = " & iWait )
        case -3   : printlog( CFN & "Password dialog (load) displayed, rc = " & iWait )
        case -4   : printlog( CFN & "Password dialog (save) displayed, rc = " & iWait )
        case -5   : printlog( CFN & "Filter Selection dialog displayed, rc = " & iWait )
        case -6   : printlog( CFN & "ASCII filter options dialog displayed, rc = " & iWait )
        case -7   : printlog( CFN & "A dialog box is in the way (1 button), rc = " & iWait )
        case -8   : printlog( CFN & "A dialog box is in the way (2 buttons), rc = " & iWait )
        case -9   : printlog( CFN & "A dialog box is in the way (3 buttons), rc = " & iWait )
        case -10  : printlog( CFN & "Master Password Dialog displayed, rc = " & iWait )
        case else : printlog( CFN & "File I/O completed in " & iWait & " seconds" )
        end select
    endif

    if ( bWarnlog and ( iWait < 0 ) ) then
        warnlog( CFN & "The messagebox was not expected at this point" )
    endif

    hFileWait() = iWait

end function

