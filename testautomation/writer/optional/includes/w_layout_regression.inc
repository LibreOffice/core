'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_layout_regression.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: fredrikh $ $Date: 2008-06-18 14:50:47 $
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
'/**********************************************************************
'**                                                                                                   **
'** owner : fredrik.haegg@sun.com                                                **
'**                                                                                                   **
'** short description : imports every Document from given path     **
'**                                                                                                   **
'**********************************************************************
'**                                                                                                   **
' #1 tLoadAllDocuments (SourcePath as string )                            **
' #1 tLoadBugdoc                                                                           **
' #1 wwFileOpen                                                                              **
'**                                                                                                   **
'\*********************************************************************

sub tLoadAllDocuments ( SourcePath as string )
    Dim i as integer 
    Dim x as integer

    printlog "SourcePath: " & SourcePath
    Call GetAllFileList(SourcePath, "*.*" , SourceDocument())
    x = ListCount ( SourceDocument() )
    printlog "- " + (x-1) + " Documents will be loaded!"
    for i = 2 to x-1
        printlog "(" & (i-1) & "/" & (x-1) & "): " & SourceDocument(i)
        Call tLoadBugdoc(SourceDocument(i))
        Sleep 5
    next i
end sub

' ****************************************************************************************

testcase tLoadBugdoc ( SourceFile )
    Dim OnlyFileName as string
    Dim OnlyFileExtension as string
    Dim TempFileName as String
    Dim i as integer

    TempFileName = SourceFile
    OnlyFileName = DateiOhneExt(DateiExtract(SourceFile)
    OnlyFileExtension = GetExtention(TempFileName)
    printLog "- " + SourceFile + "(" + OnlyFileName + "." + OnlyFileExtension + ")"
    ' Maybe Filename is empty
    if OnlyFileName > "" then
        ' Some extensions make no sense to load
        if OnlyFileName = "49069_so8pp3" then
            QAErrorlog "This document needs too much time to load! Please test manually"
            goto endsub
        end if
        if OnlyFileExtension <> "so" and OnlyFileExtension <> "tmp" then
            Select case OnlyFileName
                Case "9539_641"
                    if gPlatform = "sol" then  
                        QAErrorlog "#133851#Word doc with extremely long paragraph needs 7 min to format on sols"
                        goto endsub
                    end if
            end select
            if wwFileOpen( SourceFile ) = TRUE then
                IsItLoaded
                printlog "-  loaded"
                if (hIsNamedDocLoaded (SourceFile)) then
                    Sleep 15
                    Call hCloseDocument
                else
                    warnlog OnlyFileName + " not loaded!"
                    printlog "second stage: " + SourceFile
                    if GetDocumentCount > 1 then
                        Do Until GetDocumentCount = 1
                            Call hCloseDocument
                        Loop
                    end if
                end if
            else
                warnlog OnlyFileName + " doc not loaded!"
                printlog "first stage: " + SourceFile
                if GetDocumentCount > 1 then
                    Do Until GetDocumentCount = 1
                        Call hCloseDocument
                    Loop
                end if
            end if
        else
            printlog "(" + OnlyFileName + "." + OnlyFileExtension + ") will not be loaded"
            if GetDocumentCount > 1 then
                Do Until GetDocumentCount = 1
                    Call hCloseDocument
                Loop
            end if
        end if
    end if
endcase

' ****************************************************************************************

function wwFileOpen ( DocName$, optional Verknuepft as Boolean  ) as boolean
    Dim SourceFile as String
    Dim Dummy as Boolean
    Dim isMis as Boolean

    SourceFile = ConvertPath ( DocName$ )

    isMis = FALSE

    Dummy = TRUE
    if hFileExists ( SourceFile ) = FALSE then
        Warnlog "hFileOpen(): '" + SourceFile + "' does not exists!"
        Dummy = FALSE
    else
        if IsMissing ( Verknuepft ) = TRUE then
            Verknuepft = FALSE
            isMis = TRUE
        end if

        FileOpen
        sleep (1)
        Kontext "OeffnenDlg"
        Dateiname.SetText SourceFile
        sleep (3)
        Oeffnen.Click      
        
        Kontext "AsciiFilterOptionen"
        if AsciiFilterOptionen.Exists then
            printlog "- Ascii Filter Dialog is up!"
            AsciiFilterOptionen.Ok
        end if

        Kontext "FilterAuswahl"
        if FilterAuswahl.Exists then
            printlog "- Filter Dialog is up!"
            FilterAuswahl.Close
        end if

        Kontext "DocumentWriter"
        try
            Do until DocumentWriter.StatusIsProgress = false
                wait 500
            Loop
        catch
        endcatch

        Kontext "Active"
        if Active.Exists(3) then
            if isMis = FALSE then
                PrintLog "hFileOpen(): " + Active.GetText
            else
                Warnlog "hFileOpen(): " + Active.GetText
            end if
            try
                Active.OK
            catch
                if Verknuepft = FALSE then
                    Active.No              ' z.B. Verknuepfungen werden nicht aktualisiert
                else
                    Active.Yes             ' z.B. Verknuepfungen werden aktualisiert
                end if
            endcatch
            Kontext "OeffnenDlg"
            if OeffnenDlg.Exists(1) then
                OeffnenDlg.Cancel
                Dummy = FALSE
                Warnlog "hFileOpen(): The File-Open-Dialog is open!"
            end if
        end if
    end if

    if Dummy = TRUE then
        wwFileOpen = True
    else
        wwFileOpen = FALSE
    end if
end function
