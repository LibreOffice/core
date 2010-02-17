'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
'/************************************************************************
'*
'* owner : thorsten.bosbach@sun.com
'*
'* short description : gridcontrol
'*
'\******************************************************************************

testcase tGridcontrolLoad
    dim sLocation as string
    dim i,x,a as integer
    dim sTemp as string
    dim lFiles(200) as string
    dim bTemp as boolean
    dim iError, iOK as integer
    
    sLocation = "framework/optional/include/basic_gridcontrol.inc::"

    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_LOW )

    printlog "Open the test document"
    call hFileOpen(convertPath(gTestToolPath + "framework/optional/input/gridcontrol.odt"))
    printlog "Security dialog might come up"
    kontext "SecurityWarning"
    if SecurityWarning.exists(5) then
        printlog "Allow to run macros"
        SecurityWarning.ok
    endif
    call sleep 1

    printlog "Start the macro, that performs the test"
    printlog "Tools->Macros->Run Macro..."
    ToolsMacrosRunMacro
    Kontext "ScriptSelector"
    if ScriptSelector.exists(10) then
        x = LibraryTreeList.getItemCount
        printlog "Look for library 'gridcontrol'"
        for i = 1 to x
            sTemp = LibraryTreeList.getItemText(i)
            'printlog "("+i+"/"+x+"): '" + sTemp + "'"
            if (instr(sTemp,"gridcontrol")>0 OR instr(sTemp,"GRIDCONTROL")) then
                a = i
                printlog "Found library ("+i+"/"+x+"): '" + sTemp + "'"
            endif
        next i
        if 0 <> a then
            printlog "Select the library and unfold it"
            LibraryTreeList.select(a)
            LibraryTreeList.typeKeys("+")
            printlog "Go to 'Standard' and unfold"
            LibraryTreeList.typeKeys("<down>")
            LibraryTreeList.typeKeys("+")
            printlog "Go to 'Module1'"
            LibraryTreeList.typeKeys("<down>")

            printlog "Select macro name 'VclTestTool'"
            x = ScriptList.getItemCount
            for i = 1 to x
                sTemp = ScriptList.getItemText(i)
                'printlog "("+i+"/"+x+"): '" + sTemp + "'"
                if "VclTestTool" = sTemp then
                    a = i
                    printlog "Found macro ("+i+"/"+x+"): '" + sTemp + "'"
                endif
            next i
            if (0 <> a) then
                printlog "Select the macro"
                ScriptList.select(a)
                printlog "Start the macro by pressing the button 'run'"
                ScriptSelector.ok
                printlog "sleep... at least 60 seconds"
                call sleep(60)
                printlog "If macro fails for whatever reason, a message comes up"
                Kontext
                if active.exists() then
                    warnlog "" + active.getText
                    active.ok
                    try
                        EditCopy
                        warnlog getclipboard()
                    catch
                    endcatch
                    printlog "close basic ide"
                    call hCloseDocument()
                    printlog "close document"
                    call hCloseDocument()
                    goto endsub
                endif

            else
                warnlog sLocation + "Macro not found: 'Main'"
            endif
        else
            warnlog sLocation + "Library not found: 'gridcontrol'"
        endif
    else
        warnlog sLocation + "'Run macro dialog' didn't came up"
    endif
    
    printlog "clean up"
    printlog "Close the document, else an error about the navigator will be thrown"
    if getDocumentcount > 0 then
        call hCloseDocument()
    endif
endcase

