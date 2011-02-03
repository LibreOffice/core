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
'* owner : gregor.hartmann@oracle.com
'*
'* short description : "10ner/Smoke Test" from release engineering
'*
'\******************************************************************************

testcase tSmokeTest

    if ( not hCheckForBinfilters() ) then
        goto endsub
    endif

    printlog( "Smoketest (10er-Test) from Release Engineering" )
    dim sLocation as string
    dim i,x,a as integer
    dim sTemp as string
    dim lFiles(200) as string
    dim bTemp as boolean
    dim iError, iOK as integer
    
    sLocation = "framework/first/inc/smoketest.inc::"

    'To avoid focus problems, go to backingwindow
    if getDocumentcount <> 0 then
        'After an resetApplication at least one document may be open.
        printlog "closed one window " + getDocumentCount
        call hCloseDocument()
    endif
    
    'Delete temp dir of logs
    sTemp = convertPath(gOfficePath + "user/temp/")
    printlog "Going to delete ttt.*: " + GetFileList(sTemp,"ttt.*",lFiles())
    printlog "Going to delete *.dat: " + GetFileList(sTemp,"*.dat",lFiles())
    bTemp = KillFileList(lFiles())
    if NOT bTemp then
        for i = 1 to ListCount(lFiles())
            qaErrorlog sLocation + "Can't delete file: " + i + ": '" + lFiles(i) + "'"
        next i
    endif

    'Open the test document
    call hFileOpen(convertPath(gTestToolPath + "framework/required/input/10erTest_680.sxw"))
    'Security dialog might come up
    kontext "SecurityWarning"
    if SecurityWarning.exists(5) then
        'Allow to run macros
        SecurityWarning.ok
    endif
    call sleep 1

    'Start the macro, that performs the test
    'Tools->Macros->Run Macro...
    'Check, if all three libraries are there, else close and try again...
    ToolsMacrosRunMacro
    Kontext "ScriptSelector"
    if ScriptSelector.exists(10) then
        x = LibraryTreeList.getItemCount
        'There have to show up 3 libraries, else the document isn't loaded
        if x <> 3 then
            printlog "Try again..."
            ScriptSelector.cancel
            sleep 10
            ToolsMacrosRunMacro
            Kontext "ScriptSelector"
            x = LibraryTreeList.getItemCount
        endif
        'Look for library '10erTest_680' and '10ERTEST_680'
        for i = 1 to x
            sTemp = LibraryTreeList.getItemText(i)
            'printlog "("+i+"/"+x+"): '" + sTemp + "'"
            if (instr(sTemp,"10erTest_680")>0 OR instr(sTemp,"10ERTEST_680")) then
                a = i
                printlog "Found library ("+i+"/"+x+"): '" + sTemp + "'"
            endif
        next i
        if 0 <> a then
            'Select the library and unfold it
            LibraryTreeList.select(a)
            LibraryTreeList.typeKeys("+")
    '        LibraryTreeList.select("10erTest_680") ' not implemented right now :-(
            'Go to 'Standard' and unfold
            LibraryTreeList.typeKeys("<down>")
            LibraryTreeList.typeKeys("+")
            'Go to 'Global'
            LibraryTreeList.typeKeys("<down>")

            'Select macro name 'StartTestByOption'
            x = ScriptList.getItemCount
            for i = 1 to x
                sTemp = ScriptList.getItemText(i)
                'printlog "("+i+"/"+x+"): '" + sTemp + "'"
                if "StartTestByOptions" = sTemp then
                    a = i
                    printlog "Found macro ("+i+"/"+x+"): '" + sTemp + "'"
                endif
            next i
            if (0 <> a) then
                'Select the macro
                ScriptList.select(a)
                'Start the macro by pressing the button 'run'
                ScriptSelector.ok
        '       sleep... at least 60 seconds
                call sleep(60)
                ' If macro fails for whatever reason, a message comes up
                Kontext
                if active.exists() then
                    warnlog "Macro failed " + active.getText
                    active.ok
                    call hCloseDocument()
                    goto endsub
                endif

                'Path of the log file written by the macro
                sTemp = convertPath(gOfficePath + "user/temp/log.dat")
                a = 0
                'Wait until file exists, but not longer than ten minutes
                while ((NOT fileExists(sTemp)) AND (a < 10))
                    printlog "waiting another minute until file exists - " + a
                    inc(a)
                    call sleep(60)
                wend

                'File exists
                if (a<10) then
                    printlog "Log file exists: " + sTemp
                    'Wait until test finished, but not longer than ten minutes
                    'String at end, if finished: 'FINISHED' modified 10er sxw document from TBO!
                    a = 0
                    lFiles(0)=0
                    call ListRead (lFiles(), sTemp)
                    while ((lFiles(ListCount(lFiles())) <> "FINISHED") AND (a < 10))
                        printlog "last line: '" + lFiles(ListCount(lFiles())) + "'"
                        inc(a)
                        call sleep(60)
                        lFiles(0)=0
                        call ListRead (lFiles(), sTemp)
                    wend
                    
                    'Macro finished
                    if (a<10) then
                        printlog "Test document finished."
                        'Grep file for "-> error" & "-> ok"
                        a = ListCount(lFiles())
                        for i = 1 to a
                            if inStr(lFiles(i), "-> error") > 0 then
                                if inStr("simpress Paste Object -> errorsdraw Paste Object -> error",lFiles(i)) > 0 then
                                    qaerrorlog lFiles(i)
                                else
                                    warnlog lFiles(i)
                                    inc(iError)
                                endif
                            elseif inStr(lFiles(i), "-> ok") > 0 then
                                printlog lFiles(i)
                                inc(iOK)
                            endif
                        next i
                        if (iError > 0) then
                            qaErrorlog sLocation + "Smoketest/10-er Test failed: " + iError + " times."
                        endif
                        printlog "Smoketest/10-er Test succeeded: " + iOK + " times."
                    else
                        warnlog sLocation + "Macro didn't finish in time (10 minutes)"
                    endif
                else
                    warnlog sLocation + "Macro didn't create log file in time (10 minutes)"
                endif
            else
                warnlog sLocation + "Macro not found: 'StartTestByOptions'"
            endif
        else
            warnlog sLocation + "Library not found: '10erTest_680' or '10ERTEST_680'"
        endif
    else
        warnlog sLocation + "'Run macro dialog' didn't came up"
    endif
    
    'clean up
    'Close the document, else an error about the navigator will be thrown
    if getDocumentcount > 0 then
        call hCloseDocument()
    endif
endcase

