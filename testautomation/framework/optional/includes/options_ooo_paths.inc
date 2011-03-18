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
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : Options - StarOffice - path settings
'*
'\******************************************************************************


testcase tOOoPaths
    Dim lsPathOptions(50) as String
    Dim lsDialtype(50) as String
    Dim lsPaths(10,50) as String
    Dim lsEntry(50) as String
    Dim i, j, a as Integer
    Dim sTTInputPath as String
    dim iTypeOfDialog as integer
    dim sTyp as string
    dim iPaths as integer
    dim iTyp as integer
    dim iCountOfDialogs(3) as integer
    dim aPath() as string
    dim i68646 as boolean

    i68646 = FALSE
    sTTInputPath = ConvertPath ( gOfficePath + "user\work" )
    '                  Name;         which open dialog; Count of Paths
'    call GetPathList ( lsPathOptions (), lsDialtype (), lsEntry () )
    ' which open dialog: 1: File Open; Else (2): Paths dialog
    'call DumpPathList( lsPathOptions (), lsDialtype (), lsEntry () )

    ToolsOptions
    hToolsOptions("StarOffice", "Paths")

    'reset everything to default; changes made in master.inc are undone for this test.
    Typ.TypeKeys("<down><HOME>")
    iTyp = Typ.getItemCount
    if iTyp <> 8 then
        warnlog "Unexpected count of File Types: should: " + 8 + "; is: " + iTyp
    else
        printlog "Count of Path Types: " + iTyp
    endif
    for i = 1 to iTyp
        try
            standard.click
        catch
            qaerrorlog "Unexpected: No Standard"
        endcatch
        Typ.TypeKeys( "<DOWN>" )
    next i

    printlog("save settings")
    for i = 1 to iTyp
        if i <> 1 then
            Typ.TypeKeys "<Down>"
        else
            Typ.TypeKeys "<Down>"
            Typ.TypeKeys "<Home>"
        endif
        sleep 1
        sTyp = Typ.getText
        aPath() = split(Typ.getItemText(i,2),";")
        lsPathOptions(i) = sTyp

        Bearbeiten.Click
        kontext
        if active.exists(5) then
            qaerrorlog "Active: " + active.getText
            active.ok
        endif
            kontext "OeffnenDlg"
            if OeffnenDlg.Exists(5) then
                iTypeOfDialog = 1
                inc(iCountOfDialogs(1))
            else
                kontext "PfadeAuswaehlen"
                if PfadeAuswaehlen.Exists(5) then
                    iTypeOfDialog = 2
                    inc(iCountOfDialogs(2))
                else
                    ' EXIT CONDITION
                    warnlog "No kind of File Dialog is open - exiting test"
                    iTypeOfDialog = 0
                    goto endsub
                endif
            endif

            lsDialtype(i) = iTypeOfDialog
            select case iTypeOfDialog
                case 1:' printlog( " - Dialogtype: FileOpen" )
                    kontext "OeffnenDlg"
                    if OeffnenDlg.Exists(2) then
                        lsPaths(1,i) = Pfad.GetSelText
                        ' change Path
                        Pfad.SetText sTTInputPath
                        try
                            Auswaehlen.Click
                        catch
                            qaErrorlog "Catch -1-?"
                            OeffnenDlg.typekeys("<MOD+S>")
                        endcatch
                        printlog "" + i + "/1: " + right(lsPaths(1,i),10) + " : " + sTyp + " " + lsPaths(1,i)
                    else
                        warnlog "No Path dialog available"
                    end if
                case 2:' printlog( " - Dialogtype: PathDialog" )
                    kontext "PfadeAuswaehlen"
                    if PfadeAuswaehlen.Exists(2) then
                        try
                            iPaths = Pfade.GetItemCount
                        catch
                            qaErrorLog "#i68646# Path Multilistbox is invisible to TestTool application"
                            i68646 = TRUE
                            iPaths=0
                        endcatch
                        lsEntry(i) = iPaths
                        if NOT i68646 then
                        if (2 <> iPaths) then
                            warnlog "Unexpected count of paths; Should: " + 2 + "; Is: " + iPaths
                        endif
                        endif
                        for j=1 to iPaths
                            lsPaths(j,i) = Pfade.GetItemtext(j)
                        next j
                        Hinzufuegen.Click
                        kontext "OeffnenDlg"
                        if OeffnenDlg.exists(5) then
                            Pfad.SetText sTTInputPath
                            lsPaths(iPaths+1,i) = sTTInputPath
                            Sleep (1)
                            try
                                Auswaehlen.Click
                            catch
                                OeffnenDlg.typekeys( "<MOD+S>" )
                            endcatch
                            kontext
                            if active.exists(5) then
                                qaErrorLog "Active about already existsing path?: '" + active.getText + "' + '" + sTTInputPath + "'"
                                active.OK
                            endif
                        else
                            warnlog "No Path dialog available"
                        endif
                        kontext "PfadeAusWaehlen"
                        PfadeAuswaehlen.OK
                        printlog "" + i + "/"+iPaths+": " + right(lsPaths(1,i),10) + " : " + sTyp
                        printlog "" + i + "/-: " + right(lsPaths(2,i),10) + " : " + sTyp
                    else
                        warnlog "No Path dialog available"
                    endif
                case else: warnlog "Unexpected kind of Path dialog available"
            end select
            Sleep(1)
            ' double cross check of Paths in both dialogs
            if (uBound(aPath()) <> (iTypeOfDialog-1)) then
                qaErrorLog ("Sanity check failed.")
            endif
        kontext "tabpfade"
    next i
    printlog "Path changed to, or added: '" + sTTInputPath + "'"
    
    'Check if count of kinds of dialogs match
    if iCountOfDialogs(1) <> 4 then
        warnlog "Unexpected count of File Open Dialogs: should: " + 4 + "; is: " + iCountOfDialogs(1)
    else
        printlog "Count of File Open Dialogs: " + iCountOfDialogs(1)
    endif
    if iCountOfDialogs(2) <> 4 then
        warnlog "Unexpected count of File Open Dialogs: should: " + 4 + "; is: " + iCountOfDialogs(2)
    else
        printlog "Count of File Open Dialogs: " + iCountOfDialogs(2)
    endif
    
    kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    Sleep (4)
    printlog("Exit/restart StarOffice")
    ExitRestartTheOffice
    printlog( "Check changes")
    ToolsOptions
    hToolsOptions ( "StarOffice", "Paths" )

    for i = 1 to iTyp
        ' It's a treelistbox with multiselection;
        ' .select would select an additional item :-(
        ' a lonely <home> wouldn't change the selction :-<
        if i <> 1 then
            Typ.typeKeys("<down>")
        else
            Typ.typeKeys("<down>")
            Typ.typeKeys("<Home>")
        endif
        sleep(1)

        try
            Bearbeiten.Click
        catch
            warnlog "Multiselection ?"
        endcatch
        kontext
        if active.exists(5) then
            warnlog "#i72017# '"+active.getText+"'"
            active.ok
        endif
        select case lsDialtype(i)
            case 1:
                kontext "OeffnenDlg"
                if OeffnenDlg.Exists(5) then
                    sTyp = Pfad.GetSelText
                    if (sTyp <> sTTInputPath) then
                        if (sTyp <> ( sTTInputPath + gPathSigne)) then
                            Warnlog "'" + lsPathOptions(i) + "' options is not saved! => must: '" + sTTInputPath + "' is: '" + sTyp +"'"
                        endif
                    end if
                   ' reset to default
                   ' compute path's that were changed in TestStartUp (work and temp)
                   if ((Right(lsPaths(1,i),5)="work"+gPathSigne) OR (Right(lsPaths(1,i),4)="work")) then
                       Pfad.SetText(lsPaths(1,i))
                       Auswaehlen.Click
                   else
                        OeffnenDlg.Cancel
                        kontext "tabpfade"
                        if Standard.IsEnabled then
                            Standard.Click
                        else
                            warnlog "Can't reset to standard"
                        endif
                   endif
                end if
            case 2:
                kontext "PfadeAuswaehlen"
                if PfadeAuswaehlen.exists (5) then
                    try
                        iPaths = Pfade.GetItemCount
                    catch
                        printlog "failed"
                        iPaths=0
                    endcatch
                    for j=1 to iPaths 
                        if (lsPaths(j,i) <> Pfade.GetItemtext(j)) then
                            Warnlog "'" + lsPathOptions(i) + "': " + j + ". options is not saved! Path found: '" + Pfade.GetItemtext(j) + "'; expected: '" + lsPaths(j,i) +"'"
                        endif
                    next j
                    PfadeAuswaehlen.Cancel
                end if
                ' reset to default
                kontext "tabpfade"
                if Standard.IsEnabled then
                    Standard.Click
                else
                    warnlog "Can't reset to standard"
                endif
            case else: warnlog "Unexpected kind of file open dialog"
        end select
        sleep 1
        kontext "tabpfade"
    next i
    
    printlog("Reset to default ( with default-button )")
    if gPlatgroup = "unx" then
        hToolsOptions ( "StarOffice", "General" )
    endif
    Sleep (4)
    kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK()
    Sleep (4)
    printlog("Check the reset")
    ToolsOptions
    hToolsOptions ( "StarOffice", "Paths" )
    for i = 1 to iTyp
        if i <> 1 then
            Typ.TypeKeys "<Down>"
        else
            Typ.TypeKeys "<Down>"
            Typ.TypeKeys "<Home>"
        end if
        sleep(1)
        Bearbeiten.Click
        kontext
        if active.exists(5) then
        	qaerrorlog active.getText
        	active.ok
        endif
        select case lsDialtype(i)
            case 1:
                kontext "OeffnenDlg"
                if OeffnenDlg.Exists(5) then
                    sTyp = Pfad.GetSelText
                    if (sTyp <> lsPaths(1,i)) then
                        Warnlog "'" + lsPathOptions(i) + "' options is not saved! => must: '" + lsPaths(1,i) + "' is: '" + sTyp +"'"
                    endif
                    OeffnenDlg.Cancel
                end if
            case 2:
                kontext "PfadeAuswaehlen"
                if PfadeAuswaehlen.exists(5) then
                    try
                        iPaths = Pfade.GetItemCount
                    catch
                        printlog "failed"
                        iPaths = 0
                    endcatch
                    for j=1 to iPaths 
                        if (lsPaths(j,i) <> Pfade.GetItemtext(j)) then
                            Warnlog "'" + lsPathOptions(i) + "': " + j + ". options is not saved! Path found: '" + Pfade.GetItemtext(j) + "'; expected: '" + lsPaths(j,i) +"'"
                        endif
                    next j
                    PfadeAuswaehlen.Cancel
                end if
            case else: warnlog "Unexpected kind of file open dialog"
        end select
        sleep(1)
        kontext "tabpfade"
    next i
    if gPlatgroup = "unx" then
        hToolsOptions ( "StarOffice", "General" )
    endif
    Sleep (4)
    kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    printlog("Exit/restart StarOffice")
    ExitRestartTheOffice
endcase

