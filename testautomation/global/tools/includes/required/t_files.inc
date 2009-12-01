'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: t_files.inc,v $
'*
'* $Revision: 1.4 $
'*
'* last change: $Author: rt $ $Date: 2008-08-28 11:59:04 $
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
'/******************************************************************************
'*
'* Owner : thorsten.bosbach@sun.com
'*
'* short description : Global routines for loading, saving, printing, export
'*
'\******************************************************************************

private const C_INFO = TRUE ' Set this to TRUE to increase verbosity of some functions
private const VERBOSE = true

function hGrafikEinfuegen  ( Grafik$ ) as Boolean
    '/// hGrafikEinfuegen hGraphicInsert
    '/// A graphic will be inserted (not linked).
    '/// <u>Input</u>: Filename with complete path
    '/// <u>Return</u>: TRUE or FALSE if the graphic could be inserted or not.

    Dim DieDatei as String
    Dim IsActive as Boolean

    DieDatei = ConvertPath ( Grafik$ )

    InsertGraphicsFromFile
    WaitSlot()

    Kontext
    If Active.Exists(1) then
        warnlog "  Hinweis: " & Active.GetText
        Active.Ok
    End If


    Kontext "GrafikEinfuegenDlg"
    if ( GrafikEinfuegenDlg.exists( 2 ) ) then

        Vorschau.Uncheck
        if gApplication <> "HTML" AND gApplication <> "HTML" then
            Verknuepfen.UnCheck
        end if

        Dateiname.SetText DieDatei
        DateiTyp.Select 1                               ' set the filter to 'all formats'
        Oeffnen.Click

        Kontext "Active"
        if Active.Exists(2) then
            Warnlog "  Hinweis: " & Active.GetText
            try
                Active.OK
            catch
                try
                    Active.Yes
                catch
                    Active.Cancel
                endcatch
            endcatch
            Kontext "GrafikEinfuegenDlg"
            if GrafikEinfuegenDlg.Exists then
                GrafikEinfuegenDlg.Cancel
            endif
        else
            hGrafikEinfuegen = IsImageLoaded
        end if
    else
        'GrafikEinfuegeDlg not open
    endif
end function
'
'-------------------------------------------------------------------------------
'
function hGrafikVerknuepftEinfuegen  ( Grafik$ ) as Boolean
    '/// hGrafikEinfuegenEinfuegen hGraphicInsertLinked
    '/// A graphic will be inserted <b>LINKED</b>
    '/// <u>Input</u>: Filename with complete path
    '/// <u>Return</u>: TRUE or FALSE if the graphic could be inserted or not.
    Dim DieDatei as String
    Dim IsActive as Boolean

    DieDatei = ConvertPath ( Grafik$ )

    InsertGraphicsFromFile
    WaitSlot()

    Kontext
    If Active.Exists(1) then
        warnlog "  Hinweis: " & Active.GetText
        Active.Ok
    End If
    Kontext "GrafikEinfuegenDlg"
    if ( GrafikEinfuegenDlg.exists( 1 ) ) then

        Vorschau.UnCheck
        if gApplication <> "HTML" AND gApplication <> "HTML" then
            Verknuepfen.Check
        end if

        Dateiname.SetText DieDatei
        DateiTyp.Select 1                               ' set the filter to 'all formats'
        Oeffnen.Click

        Kontext "Active"
        if Active.Exists(2) then
            Warnlog "  Hinweis: " + Active.GetText
            try
                Active.OK
            catch
                Active.Yes
            endcatch
            Kontext "GrafikEinfuegenDlg"
            if GrafikEinfuegenDlg.Exists then
                GrafikEinfuegenDlg.Cancel
            endif
        else
            hGrafikVerknuepftEinfuegen = IsImageLoaded
        end if
    else
        'GrafikEinfuegenDlg not open
    endif
end function
'
'-------------------------------------------------------------------------------
'
function IsItSaved as boolean
    '/// IsItSaved
    '/// Wait until document is saved.
    Dim iLoop as integer

    IsItSaved = FALSE
    sleep(3)
    for iLoop =1 to 20
        try
            'Calling slot 'IsDocSaving'
            IsItSaved = IsDocSaving
        catch
            IsItSaved = FALSE
        endcatch

        if IsItSaved = TRUE then
            exit for
        end if
        sleep(1)
    next iLoop
    sleep(2)
end function
'
'-------------------------------------------------------------------------------
'
function IsItLoaded as boolean
    '/// IsItLoaded
    '/// Wait until document is loaded
    Dim iLoop as integer

    IsItLoaded = FALSE
    sleep(3)
    for iLoop =1 to 20
        try
            'Calling slot 'IsDocLoading'
            IsItLoaded = IsDocLoading
        catch
            IsItLoaded = FALSE
        endcatch

        if IsItLoaded = TRUE then
            exit for
        end if
        sleep(1)
    next iLoop
    sleep(2)
end function
'
'-------------------------------------------------------------------------------
'
function hIsNamedDocLoaded (ShouldFile as String, optional bSilent as boolean) as Boolean
    dim sTemp as string
    '/// hIsNamedDocLoaded ///'
    '/// !fails always if a template is loaded, because you have to set a new filename in the save-dialog -> not usable in this case ! ///'
    '/// Input: name of loaded file; Output: True/False ///'
    '///+' if a doc got loaded, the filename in a 'SaveAs' Dlg is different from "" ///
    '///+' usually the file name of the loaded document with an changed extension chosen from SO ///

    if (isMissing(bSilent)) then
        bSilent = False
    endif

    hIsNamedDocLoaded  = FALSE      ' Let's  start WorstCase :-(
    FileSaveAs
    Kontext "SpeichernDlg"
    If DateiOhneExt(DateiExtract(Dateiname.GetSelText)) = DateiOhneExt(DateiExtract(ShouldFile)) Then
        hIsNamedDocLoaded = True
    Else
        sTemp = left(right(ShouldFile,2),1) ' get the 2nd last character ' vor;dot;sti;pot;std;xlt
        if (((sTemp="o") or (sTemp="t") or (sTemp="l")) and ((left(right(ShouldFile,4),1)=".") or (left(right(ShouldFile,5),1)="."))) then
            hIsNamedDocLoaded = TRUE ' exception! for linux! and MS-Office Templates
        else
            if ((Dateiname.GetSeltext = "") and not bSilent) then
                Warnlog "Default filename is empty!"+left(right(ShouldFile,3),1)
            endif
        endif
    End If
    SpeichernDlg.Cancel
end function
'
'-------------------------------------------------------------------------------
'
function IsImageLoaded
    '/// IsImageLoaded
    '/// Wait until images in document are loaded
    Dim i% : Dim Herbert as Boolean

    Sleep 3
    for i%=1 to 20
        try
            Herbert = IsDocImageLoading
        catch
            Herbert = FALSE
        endcatch
        if Herbert = TRUE then
            i%=100
        endif
        Sleep (1)
    next i%
    if i<100 then
        IsImageLoaded = FALSE
    else
        IsImageLoaded = TRUE
    end if
end function
'
'-------------------------------------------------------------------------------
'
sub hIsWebPageLoaded as boolean
    'Author: Joerg Sievers
    '/// If a document in StarOffice Writer is loaded all 9 items in the
    '///+ statusbar are visible otherwise not.
    '///+ This sub checks the state of these items.
    Dim i as integer
    Dim iGibtdenStatusraus as integer
    printlog "- global::tools::inc::tfiles.inc::hIsWebPageLoaded"
    hIsWebPageLoaded = FALSE
    for i = 1 to 20
        Kontext "DocumentWriter"
        if DocumentWriter.Exists(3) then
            'Count the items on the status bar.
            iGibtdenStatusraus = DocumentWriter.StatusGetItemCount
            'There are nine items on the status bar if the web page has been loaded.
            if iGibtdenStatusraus = 9 then
                hIsWebPageLoaded = TRUE
                exit for
            end If
        end if
        WaitSlot( 3000 )
    next i
end sub
'
'-------------------------------------------------------------------------------
'
sub hSys2IntDlg
    '/// Change from system to StarOffice-internal file-dialog (only for Win32)
    if gPlatgroup <> "unx" then
        Call hhSysToInt ( TRUE )
    endif
    gUseSysDlg = FALSE
end sub
'
'-------------------------------------------------------------------------------
'
sub hInt2SysDlg
    '/// Change from StarOffice-internal to system file-dialog (only for Win32)
    if gPlatgroup <> "unx" then
        Call hhSysToInt ( FALSE )
    endif
    gUseSysDlg = TRUE
end sub
'
'-------------------------------------------------------------------------------
'
sub hhSysToInt ( bwhats as Boolean )
    '/// subroutine for <i>hSys2IntDlg</i> and </i>hInt2SysDlg</i>
    ToolsOptions
    hToolsOptions ( "StarOffice", "General" )
    if bwhats = TRUE then
        StarOfficeDialogeBenutzen.Check
    else
        StarOfficeDialogeBenutzen.Uncheck
    end if
    Sleep 1
    Kontext "OptionenDlg"
    OptionenDlg.OK
    WaitSlot( 3000 )
end sub
'
'-------------------------------------------------------------------------------
'
function hGetUsedFilter () as string
    '/// Get used filter for loaded file.
    try
        FileSaveAs
        Kontext "SpeichernDlg"
        hGetUsedFilter = dateityp.getseltext
        SpeichernDlg.Cancel
    catch
        hGetUsedFilter = "Not possible; try/catch fail in function"
    endcatch
end function
'
'-------------------------------------------------------------------------------
'
function hFileExport (sName as string, sFilter as string) as boolean
    '/// Wrapper function to simplify the use of the export functions, just give a filename and export format.
    '///+ INPUT:<ul><li>sName: filename</li><li>sFilter: &quot;PDF&quot;: call export for PDF</li></ul>
    '///+ RETURN: execution success?
    select case sFilter
    case "PDF" : '/// use the function 'hExportAsPDF' with RECOMMENDED values /// '
        hFileExport = hExportAsPDFmulti (3, TRUE,  sName, FALSE, TRUE, 1, 1)
    end select
    kontext
    if active.exists(5) then
        try
            printlog active.getText
            active.ok
            warnLog "i26820 - errormessage about saving"
        catch
            printlog "pdf unexpected error in hFileExport()"
        endcatch
    endif
end function
'
'-------------------------------------------------------------------------------
'
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
                Export.Click
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
    if (iTypeOfCall = 1 OR iTypeOfCall = 3) then
        kontext "ExportierenDLG"
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
            ExportierenDLG.Cancel
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
function hDeleteFile( cFileOrig as string ) as boolean

    const CFN = "global::tools::includes::required::t_files.inc::hDeleteFile():"
    dim cHome as string
    cHome = gOfficePath & "user"
    cHome = convertpath( cHome )
    dim cFile as string
    cFile = convertpath( cFileOrig )

    '///<h3>Delete a file</h3>
    '///<i>In many cases it is a good idea to use this function outside the testcase.<br>
    '///+ You should always evaluate the returncode</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Filename (string)</li>
    '///<ul>
    '///+<li>The function converts the path to system specific syntax (convertpath)
    '///+ but does <b>not</b> modify the input parameter</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorstatus (boolean)</li>
    '///<ul>
    '///+<li>TRUE: File was deleted/does not exist</li>
    '///+<li>FALSE: File could not be deleted/any other error</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    ' this function tries to delete a file and does some very basic error-
    ' handling. Returns 'true' on success, only error while deleting returns
    ' 'false', if the file does not exist, it is considered to be successfully
    ' deleted.
    ' i introduced this function due to a number of cases where deleting files
    ' actually failed because of weird code or situations where the user lacks
    ' accessrights to files are not handled at all.

    '///+<li>We may never delete a file outside gOfficePath/user/work</li>
    if ( instr( cFile , cHome ) = 0 ) then
        qaerrorlog( CFN & "Trying to delete file outside (default)homedir -> forbidden" )
        qaerrorlog( CFN & "Home: " & cHome )
        qaerrorlog( CFN & "File: " & cFile )
        hDeleteFile() = false
        exit function
    endif

    cFile = convertpath( cFile )

    '///+<li>Check that the file exists</li>
    if ( FileExists( cFile ) ) then

        '///+<li>Use kill to delete</li>
        try

            kill( cFile )

            '///+<li>Verify that the file does not exist anymore</li>
            if ( FileExists( cFile ) ) then
                warnlog( CFN & "File was not deleted: " & cFile )
                hDeleteFile() = false
            else
                if ( VERBOSE ) then printlog( CFN & "File successfully deleted: " & cFile )
                hDeleteFile() = true
            endif

        catch

            '///+<li>in very rare cases 'kill' fails and can be handled this way</li>
            qaerrorlog( CFN & "Deleting file failed: " & cFile )
            hDeleteFile() = false

        endcatch

    else

        '///+<li>write some text to the log if the file does not exist</li>
        printlog( CFN & "Nothing to do." )
        hDeleteFile() = true

    endif

    '///</ul>

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
    const CFN = "global::tools::inc::t_files.inc::hFileOpen():"
    if ( C_INFO ) then printlog( CFN & "Load: " & sFile )
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
    const CFN = "global::tools::inc::t_files.inc::hFileOpenSpecial():"
    hFileOpenSpecial() = FALSE
    if ( C_INFO ) then printlog( "Load (Flag): " & sFile & " (" & cFlag & ")" )

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

    '///</ul>

end function
'
'-------------------------------------------------------------------------------
'
function hFileOpenWithFilter( cFile as string, cFilter as string )

    dim sFile as string : sFile = convertToURL( convertpath( cFile ) )
    const CFN = "global::tools::inc::t_files.inc::hFileOpenWithFilter():"
    hFileOpenWithFilter() = FALSE
    if ( C_INFO ) then printlog( CFN & "Load (Filter): " & sFile & " (" & cFilter & ")" )
    FileOpen( "URL", sFile, "FrameName", "_default", "FilterName", cFilter )
    if ( hFileWait( FALSE ) >= 0 ) then hFileOpenWithFilter() = TRUE

end function
'
'-------------------------------------------------------------------------------
'
function hFileSave() as boolean

    const CFN = "global::tools::inc::t_files.inc::hFileSave():"
    hFileSave() = FALSE
    if ( C_INFO ) then printlog( CFN & "Save file." )
    FileSave( "SynchronMode", TRUE )
    if ( hFileWait( TRUE ) >= 0 ) then hFileSave() = TRUE

end function
'
'-------------------------------------------------------------------------------
'
function hFileSaveAs( cFile as string ) as boolean

    dim sFile as string : sFile = convertToURL( convertpath( cFile ) )
    const CFN = "global::tools::inc::t_files.inc::hFileSaveAs():"
    hFileSaveAs() = FALSE
    if ( C_INFO ) then printlog( CFN & "Save: " & sFile )
    FileSaveAs( "URL", sFile, "Overwrite", FALSE )
    if ( hFileWait( TRUE ) >= 0 ) then hFileSaveAs() = TRUE

end function
'
'-------------------------------------------------------------------------------
'
function hFileSaveAsKill( cFile as string ) as boolean

    dim sFile as string : sFile = convertToURL( convertpath( cFile ) )
    const CFN = "global::tools::inc::t_files.inc::hFileSaveAsKill():"
    hFileSaveAsKill() = FALSE
    if ( C_INFO ) then printlog( CFN & "Save, replacing: " & sFile )
    FileSaveAs( "URL", sFile, "Overwrite", TRUE )
    if ( hFileWait( TRUE ) >= 0 ) then hFileSaveAsKill() = TRUE

end function
'
'-------------------------------------------------------------------------------
'
function hFileSaveAsKillWithPassword( cFile as string, cPassword as string ) as boolean

    dim sFile as string : sFile = convertToURL( convertpath( cFile ) )
    const CFN = "global::tools::inc::t_files.inc::hFileSaveAsKillWithPassword():"
    hFileSaveAsKillWithPassword() = FALSE
    if ( C_INFO ) then printlog( CFN & "Save with password, replacing: " & sFile & "::" & cPassword )
    FileSaveAs( "URL", sFile, "Overwrite", TRUE, "Password", cPassword )
    if ( hFileWait( TRUE ) >= 0 ) then hFileSaveAsKillWithPassword() = TRUE

end function
'
'-------------------------------------------------------------------------------
'
function hFileSaveAsWithFilter( cFile as string, cFilter as string ) as boolean

    dim sFile as string : sFile = convertToURL( convertpath( cFile ) )
    const CFN = "global::tools::inc::t_files.inc::hFileSaveAsWithFilter():"
    hFileSaveAsWithFilter() = FALSE
    if ( C_INFO ) then printlog( CFN & "Save with filter: " & sFile & "::" & cFilter )
    FileSaveAs( "URL", sFile, "FilterName", cFilter, "Overwrite", FALSE )
    if ( hFileWait( TRUE ) >= 0 ) then hFileSaveAsWithFilter() = TRUE

end function
'
'-------------------------------------------------------------------------------
'
function hFileSaveAsWithFilterKill( cFile as string, cFilter as string ) as boolean

    dim sFile as string : sFile = convertToURL( convertpath( cFile ) )
    const CFN = "global::tools::inc::t_files.inc::hFileSaveAsWithFilterKill():"
    hFileSaveAsWithFilterKill() = FALSE
    if ( C_INFO ) then printlog( CFN & "Save with filter, replacing: " & sFile & "::" & cFilter )
    FileSaveAs( "URL", sFile, "FilterName", cFilter, "Overwrite", TRUE )
    if ( hFileWait( TRUE ) >= 0 ) then hFileSaveAsWithFilterKill() = TRUE

end function
'
'-------------------------------------------------------------------------------
'
function hFileWait( optional bWarn as boolean ) as integer

    ' Wait max 10 seconds for the FileOpen/FileSave slot to finish
    ' Returns the time it took the slot to finish
    ' Negaitve returnvalues are given on timeout or any blocking dialog

    const CFN = "global::tools::inc::t_files.inc::hFileWait():"
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

    if ( C_INFO ) then
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

