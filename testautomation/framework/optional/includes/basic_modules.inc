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
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Edit several modules and dialogs in basic ide
'*
'\******************************************************************************


testcase tMore_Modules_Dialogs


    dim brc as boolean
    
    dim iCurrentTab as integer

    dim cFile as string 
        cFile = gOfficePath & "user\work\basic.odt"
        
    const FILTER = "writer8"

    'const iTABCOUNT is the number of dialogs and modules.
    'If iTABCOUNT = 10 then 20 tabs (+ the first one!) = 21 should be created.

    const iTABCOUNT as Integer = 10
    const CMODULE = "TTMODULE"
    
    hDeleteFile( cFile )
    
    printlog "open a new writer-doc"
    gApplication = "WRITER"
    hCreateDocument()
    
    brc = hInitBasicIDE( CMODULE )
    
    printlog "insert 10 modules"

    for iCurrentTab = 1 to iTABCOUNT
        printlog "   - insert new module : " & iCurrentTab

        kontext "basicide"
        Tabbar.OpenContextMenu
        hMenuSelectNr(1)
        hMenuSelectNr(1)
        brc = hDeleteMacro()
        if ( brc ) then
            EditWindow.TypeKeys "'# " & ( iCurrentTab + 1 ) & ". module in this document"
        endif
    next iCurrentTab

    printlog "insert 10 dialogs"

    for iCurrentTab = 1 to iTABCOUNT
        printlog "   - insert new dialog : " & iCurrentTab
        Tabbar.OpenContextMenu
        hMenuSelectNr(1)
        hMenuSelectNr (2)

        if DialogWindow.Exists (2) <> TRUE then
            warnlog "No dialog window is shown!"
        end if

    next iCurrentTab

    printlog Chr(13) + "- close the BasicIDE"
    hCloseBasicIDE()
    
    printlog Chr(13) + "- save the document"
    Call hFileSaveAsWithFilterKill( cFile , FILTER )
    
    printlog Chr(13) + "- close the document"
    hDestroyDocument()
    
    printlog Chr(13) + "- open the saved document"
    hFileOpen( cFile )

    ToolsMacro_uno
    kontext "Makro"
    hSelectNodeByName( MakroAus , CMODULE )

    Bearbeiten.Click()
    
    printlog "delete all modules and dialogs"

    for iCurrentTab = 1 to ( 2 * iTABCOUNT + 1 )

        try
            printlog "delete  - " & iCurrentTab & "/" & ( 2 * iTABCOUNT + 1 )
            UseBindings

            kontext "basicide"
            Tabbar.OpenContextMenu
            
            hMenuSelectNr( 2 )

            kontext "active"

            if Active.Exists then
                Active.Yes
            else
                warnlog "No warning after deleting a dialog!"
            end if

        catch
            QAErrorLog "Deleting  - " & iCurrentTab & "/" & ((2 * iTABCOUNT)+1) & " failed."
        endcatch

    next iCurrentTab

    hDestroyDocument()
    hDestroyDocument()


endcase

