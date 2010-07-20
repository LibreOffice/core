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
'*  owner : Joerg.Skottke@Sun.Com
'*
'*  short description : Verify names of macros and scripts
'*
'\******************************************************************************

testcase tUpdtScripts

    printlog( "Resource test for macros and scripts / related dialogs" )

    const SCRIPTING_DIALOGS = 5
    
    const DLG_JAVASCRIPT = "JavaScript"
    const DLG_BEANSHELL  = "BeanShell"
    const DLG_PYTHON     = "Python"
    const DLG_BASIC_ORG  = "BasicOrganizer"
    const DLG_RUN_MACRO  = "RunMacro"

    ' This is the build specific part of the filename including substrings for
    ' the productname, platform, and language. At a later point we add the dialog name.
    ' Replace all blanks (spaces) with "-"
    dim sDynamicPath as string
        sDynamicPath = gProductName & "_" & gPlatform & "_" & gISOLang & "_"
        hStringReplaceChar( sDynamicPath, " ", "-" )
 
    ' This is where reference file is located, the filename is yet incomplete
    dim sInputPath as string
        sInputPath = gTesttoolPath & "framework/required/input/scripts/" & sDynamicPath
        
    ' This is where we store the reference file if differences are found.
    dim sOutputPath as string
        sOutputPath = gOfficePath & "user/work/" & sDynamicPath
      
    ' These contain the input and output paths (fully qualified)
    dim sFileIn as string
    dim sFileOut as string

    ' Array that contains the scriptnames for all organizers
    dim cScriptNamesList( 1000 ) as string

    ' Holds the object of the treelist. Used to reduce the amount of code.
    dim oTreeList as object

    dim sDialog as string
    dim iCurrentDialog as integer

    hInitSingleDoc()

    for iCurrentDialog = 1 to SCRIPTING_DIALOGS

        ' Build complete filename and identify dialog for logging and case selection
        select case ( iCurrentDialog )
        case 1 : sDialog = DLG_JAVASCRIPT
        case 2 : sDialog = DLG_BEANSHELL
        case 3 : sDialog = DLG_PYTHON
        case 4 : sDialog = DLG_BASIC_ORG
        case 5 : sDialog = DLG_RUN_MACRO
        end select

        sFileIn  = convertpath( sInputPath  & sDialog & ".txt" )
        sFileOut = convertpath( sOutputPath & sDialog & ".txt" )
        printlog( "File in.: " & sFileIn  )
        printlog( "File out: " & sFileOut )

        printlog( "Open <" & sDialog & "> and access the treelist object" )

        select case ( sDialog )
        case DLG_JAVASCRIPT: ToolsMacrosOrganizeMacrosJavaScript
            kontext "ScriptOrganizer"
            oTreeList = ScriptTreelist
        case DLG_BEANSHELL: ToolsMacrosOrganizeMacrosBeanShell
            kontext "ScriptOrganizer"
            oTreeList = ScriptTreeList
        case DLG_PYTHON: ToolsMacrosOrganizeMacrosPython
            kontext "ScriptOrganizer"
            oTreeList = ScriptTreeList
        case DLG_BASIC_ORG: ToolsMacro_uno
            Kontext "Makro"
            oTreeList = MakroAus
        case DLG_RUN_MACRO: ToolsMacrosRunMacro
            kontext "ScriptSelector"
            oTreeList = LibraryTreeList
        end select
        
        printlog( "Expand all nodes and retrieve all nodenames" )
        hGetAllNodeNames( oTreeList, cScriptNamesList() )
        
        printlog( "Compare to reference list, create new one if differences were found" )
        hManageComparisionList( sFileIn, sFileOut, cScriptNamesList() )
        
        printlog( "Close " & sDialog )
        select case ( sDialog ) 
        case DLG_JAVASCRIPT : ScriptOrganizer.cancel()
        case DLG_BEANSHELL  : ScriptOrganizer.cancel()
        case DLG_PYTHON     : ScriptOrganizer.cancel()
        case DLG_BASIC_ORG  : Makro.close()
        case DLG_RUN_MACRO  : ScriptSelector.cancel()
        end select

    next iCurrentDialog
    
    hCloseDocument()

endcase

'*******************************************************************************

function hGetScriptCount( oTree as object, oList as object ) as integer

    dim iCurrentTreeItem as integer
    dim iScriptCount as integer : iScriptCount = 0
    
    for iCurrentTreeItem = 1 to oTree.getItemCount()
        oTree.select( iCurrentTreeItem )
        iScriptCount = iScriptCount + oList.getItemCount()
    next iCurrentTreeItem
    hGetScriptCount() = iScriptCount
    
end function
