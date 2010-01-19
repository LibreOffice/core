'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: script_organizers.inc,v $
'*
'* $Revision: 1.3 $
'*
'* last change: $Author: rt $ $Date: 2008-09-04 09:16:12 $
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
'*  owner : Joerg.Skottke@Sun.Com
'*
'*  short description : Verify names of macros and scripts
'*
'\******************************************************************************

testcase tUpdtScriptCount

    printlog( "Resource test for macros and scripts / related dialogs" )

    const SCRIPTING_DIALOGS = 5
    dim aScriptCount( SCRIPTING_DIALOGS )
        aScriptCount( 1 ) = 10
        aScriptCount( 2 ) = 14
        aScriptCount( 3 ) = 10 
        aScriptCount( 4 ) = 585
        aScriptCount( 5 ) = 601 
                
    dim iCurrentDialog as integer
    dim iCurrentScriptCount as integer
    
    
    
    do while ( getDocumentCount > 0 ) 
        hDestroyDocument()
    loop
        
    hNewDocument()

    for iCurrentDialog = 1 to SCRIPTING_DIALOGS

        printlog( "" )
        select case iCurrentDialog
        case 1: printlog( "JavaScript" )
                ToolsMacrosOrganizeMacrosJavaScript
                kontext "ScriptOrganizer"
                iCurrentScriptCount = hExpandAllNodes( ScriptTreeList )
                ScriptOrganizer.cancel()                 
        case 2: printlog( "BeanShell" )
                ToolsMacrosOrganizeMacrosBeanShell
                kontext "ScriptOrganizer"
                iCurrentScriptCount = hExpandAllNodes( ScriptTreeList )
                ScriptOrganizer.cancel()                
        case 3: printlog( "Python" )
                ToolsMacrosOrganizeMacrosPython
                kontext "ScriptOrganizer"
                iCurrentScriptCount = hExpandAllNodes( ScriptTreeList )
                ScriptOrganizer.cancel()                
        case 4: printlog( "Makro Organizer" )
                ToolsMacro_uno
                Kontext "Makro"
                hExpandAllNodes( MakroAus )
                iCurrentScriptCount = hGetScriptCount( MakroAus, MakroListe )
                Makro.close()                
        case 5: printlog( "Run Macro" )
                ToolsMacrosRunMacro
                kontext "ScriptSelector"
                hExpandAllNodes( LibraryTreeList )
                iCurrentScriptCount = hGetScriptCount( LibraryTreeList, ScriptList )
                ScriptSelector.cancel()                
        end select
        
        if ( aScriptCount( iCurrentDialog ) <> iCurrentScriptCount ) then
            warnlog( "Incorrect number of scripts for this dialog: " & iCurrentScriptCount )
        else
            printlog( "Number of scripts is ok" )
        endif
        
    next iCurrentDialog
    
    hCloseDocument()
    

endcase

'*******************************************************************************

function hGetScriptCount( oTree as object, oList as object ) as integer

    dim iTreeItem as integer
    dim iScript as integer : iScript = 0
    
    for iTreeItem = 1 to oTree.getItemCount()
        oTree.select( iTreeItem )
        iScript = iScript + oList.getItemCount()
    next iTreeItem
    hGetScriptCount() = iScript
    
end function
