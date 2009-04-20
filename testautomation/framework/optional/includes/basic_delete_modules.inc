'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_delete_modules.inc,v $
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
'*  short description : Delete modules while BASIC is running
'*
'\******************************************************************************

testcase tDeleteModulesAtRunningBasic

    Dim i as Integer
    Dim iSel as Integer
    dim brc as boolean

    '///<h1>Work with macros - delete Modules while BASIC-IDE is running</h1>

    '///<ul>
    '///<li>open a new writer-doc</li>
    printlog "open a new writer-doc"
    gApplication = "WRITER"

    Call hNewDocument()

    '///<li>create a new module for the new document (named TTModule)</li>
    printlog "create a new module "
    brc = hOpenBasicOrganizerFromDoc()
    brc = hCreateModuleForDoc()

    '///<li>insert a short script (1. page)</li>
    printlog "insert a short script (1. page)"

    brc = hInsertMacro( 1 )

    kontext "macrobar"
    printlog "- Click button:  Step Procedure"
    '///<li>activate "Step Procedure" on Toolbar</li>
    ProcedureStep.Click
    if ( WaitSlot( 3000 ) <> WSFinished ) then
        warnlog( "Slot not finished within 1 second" )
    endif

    '///<li>check if disabled menu-items are shown and delete is not shown</li>

    kontext "basicide"
    printlog "check if disabled menu items are shown in context menu and 'delete' is not shown"
    Tabbar.OpenContextMenu

    try

        if hMenuItemGetCount = 5 then
            warnlog "Disabled entries are shown in context menu on tab bar => bugID 101972"
        end if


        if hMenuItemGetCount = 3 then
            warnlog "Delete is active in context menu for a running macro => BUG!"
        end if

    catch
        warnlog "Unable to retrieve the number of menuitems"
    endcatch

    hMenuClose
    '///<li>stop the running script</li>
    printlog "Stop the running script"

    kontext "macrobar"
    BasicStop.Click
    if ( WaitSlot <> WSFinished ) then
        warnlog( "Slot not finished within 1 second" )
    endif


    '///<li>close the BasicIDE and the document</li>
    printlog "close the documents"
    Call hCloseDocument()
    Call hCloseDocument()

    '///</ul> 

endcase

