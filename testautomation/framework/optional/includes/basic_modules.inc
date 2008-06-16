'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_modules.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:13 $
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
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Edit several modules and dialogs in basic ide
'*
'\******************************************************************************


testcase tMore_Modules_Dialogs

    '///<h1>Work with macros - create multiple modules</h1>
    '///<ul>

    dim brc as boolean
    
    dim iCurrentTab as integer

    dim cFile as string 
        cFile = gOfficePath & "user\work\basic.odt"
        
    const FILTER = "writer8"

    'const iTABCOUNT is the number of dialogs and modules.
    'If iTABCOUNT = 10 then 20 tabs (+ the first one!) = 21 should be created.

    const iTABCOUNT as Integer = 10
    const CMODULE = "TTMODULE"
    
    '///+<li>Delete workfile just in case it was left over by prior testrun</li>
    hDeleteFile( cFile )
    
    '///+<li>Create a new Writer document.</li>
    printlog "open a new writer-doc"
    gApplication = "WRITER"
    hCreateDocument()
    
    '///+<li>Create a new module for the new document (named TTModule)</li>
    brc = hInitBasicIDE( CMODULE )
    
    '///+<li>Insert 10 modules.</li>
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

    '///+<li>Insert 10 dialogs</li>
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

    '///+<li>Close the BasicIDE.</li>
    printlog Chr(13) + "- close the BasicIDE"
    hCloseBasicIDE()
    
    '///+<li>Save the document.</li>
    printlog Chr(13) + "- save the document"
    Call hFileSaveAsWithFilterKill( cFile , FILTER )
    
    '///+<li>Close the document</li>
    printlog Chr(13) + "- close the document"
    hDestroyDocument()
    
    '///+<li>Open the saved document</li>
    printlog Chr(13) + "- open the saved document"
    hFileOpen( cFile )

    '///+<li>Open the BASIC Organizer, select the module for the current document</li>
    ToolsMacro_uno
    kontext "Makro"
    hSelectNodeByName( MakroAus , CMODULE )

    '///+<li>Select the last module and open the BASIC IDE.</li>
    Bearbeiten.Click()
    
    '///+<li>Delete all modules and dialogs.</li>
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

    '///</ul>

endcase

