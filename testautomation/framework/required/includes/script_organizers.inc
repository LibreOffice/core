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
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Verify names of macros and scripts
'*
'\******************************************************************************

testcase tUpdtScripts

    printlog( "Resource test for macros and scripts / related dialogs" )

    if ( gIsoLang <> "en-US" ) then
        printlog( "No testing for languages other than en_US" )
        goto endsub
    endif

    const SCRIPTING_DIALOGS = 5
    
    const DLG_JAVASCRIPT = "JavaScript"
    const DLG_BEANSHELL  = "BeanShell"
    const DLG_PYTHON     = "Python"
    const DLG_BASIC_ORG  = "BasicOrganizer"
    const DLG_RUN_MACRO  = "RunMacro"

    ' This is the build specific part of the filename including substrings for
    ' the productname and language. 
    dim sProductString as string : sProductString = gProductName & "_" & gISOLang & "_"

    ' define platforms that have different scriptcount
    if ( lcase( gPlatGroup ) = "w95" ) then sProductString = sProductString & "win_"
    if ( lcase( gPlatform  ) = "osx" ) then sProductString = sProductString & "osx_"

    ' replace all blanks
    sProductString = hStringReplaceChar( sProductString, " ", "-" )
 
    ' This is where reference file is located, the filename is yet incomplete
    dim sInputPath as string
        sInputPath = gTesttoolPath & "framework/required/input/scripts/" & sProductString
        
    ' This is where we store the reference file if differences are found.
    dim sOutputPath as string
        sOutputPath = gOfficePath & "user/work/" & sProductString
      
    ' These contain the input and output paths (fully qualified)
    dim sFileIn as string
    dim sFileOut as string

    ' Array that contains the scriptnames for all organizers
    dim cScriptNamesList( 1000 ) as string

    dim sDialog as string
    dim iCurrentDialog as integer

    dim iDiffCount as integer
    dim max_diffcount as integer

    hInitSingleDoc()

    for iCurrentDialog = 1 to SCRIPTING_DIALOGS

        ListAllDelete( cScriptNamesList() )

        ' Build complete filename and identify dialog for logging and case selection
        select case ( iCurrentDialog )
        case 1 : sDialog = DLG_JAVASCRIPT
        case 2 : sDialog = DLG_BEANSHELL
        case 3 : sDialog = DLG_PYTHON
        case 4 : sDialog = DLG_BASIC_ORG
        case 5 : sDialog = DLG_RUN_MACRO
        end select

        printlog( "" )
        sFileIn  = convertpath( sInputPath  & sDialog & ".txt" )
        sFileOut = convertpath( sOutputPath & sDialog & ".txt" )

        printlog( "Open <" & sDialog & "> and access the treelist object" )

        ' Note: hGetAllNodeNames() is a *global* function defined in t_treelist_tools.inc
        ' DLG_BASIC_ORG and DLG_RUN_MACRO have - in addition to the treelist -
        ' a separate script list. To get those scripts a little more effort is
        ' required which is done in the *local* function hGetScriptNames()

        select case ( sDialog )
        case DLG_JAVASCRIPT: ToolsMacrosOrganizeMacrosJavaScript
            kontext "ScriptOrganizer"
            hGetAllNodeNames( ScriptTreeList, cScriptNamesList() )
            max_diffcount = 0
        case DLG_BEANSHELL: ToolsMacrosOrganizeMacrosBeanShell
            kontext "ScriptOrganizer"
            hGetAllNodeNames( ScriptTreeList, cScriptNamesList() )
            max_diffcount = 0
        case DLG_PYTHON: ToolsMacrosOrganizeMacrosPython
            kontext "ScriptOrganizer"
            hGetAllNodeNames( ScriptTreeList, cScriptNamesList() )
            max_diffcount = 0
        case DLG_BASIC_ORG: ToolsMacro_uno
            Kontext "Makro"
            hGetScriptNames( MakroAus, MakroListe, cScriptNamesList() )
            max_diffcount = 6
        case DLG_RUN_MACRO: ToolsMacrosRunMacro
            kontext "ScriptSelector"
            hGetScriptNames( LibraryTreeList, ScriptList, cScriptNamesList() )
            max_diffcount = 6
        end select
        
        printlog( "Compare to reference list, create new one if differences were found" )
        iDiffCount = abs( hManageComparisionList( sFileIn, sFileOut, cScriptNamesList() ) )

        ' Usually we should have 0 differences in the list. However, as we do not have
        ' a unique way of installing the office (Root-Installation, archives and
        ' others) we need a little tolerance here. If a number of bundled extensions
        ' are installed, we have more scripts.
        if ( iDiffCount > max_diffcount ) then warnlog( "The number of scripts has changed, please review." )

        printlog( "Close <" & sDialog & ">" )
        select case ( sDialog ) 
        case DLG_JAVASCRIPT :
            ScriptOrganizer.cancel()
        case DLG_BEANSHELL  :
            ScriptOrganizer.cancel()
        case DLG_PYTHON     :
            ScriptOrganizer.cancel()
        case DLG_BASIC_ORG  :
            Makro.close()
        case DLG_RUN_MACRO  :
            ScriptSelector.cancel()
        end select

    next iCurrentDialog
    
    hCloseDocument()

endcase

'*******************************************************************************

sub hGetScriptNames( oTreeList as object, oScriptList as object, cItemList() as string )

    ' oTreeList is the treelist object (left pane)
    ' oScriptList is the script list (right pane)
    ' cScriptList() is filled with the scriptnames of the form
    ' <module>:<script name> like "CommonLang:LoadLanguages"

    dim iCurrentTreeListItem as integer
    dim iObjectsInTreelist as integer

    dim cModuleName as string
    dim cReferenceString as string

    dim iScriptCount as integer
    dim iCurrentScript as integer
    dim cScriptList( 1000 ) as string

    ' Get the number of nodes from the modules treelist
    iObjectsInTreeList = hExpandAllNodes( oTreeList )

    ' For each item in the modules treelist get the number of scripts (right pane)
    for iCurrentTreeListItem = 1 to iObjectsInTreeList

        ' Get the name of the current treelist node, append it to the list
        ' (otherwise it does not appear if it has no scripts)
        cModuleName = hGetNodeName( oTreeList, iCurrentTreeListItem )
        hListAppend( cModuleName, cItemList() )

        ' Get the number of scripts from the right pane, reset the array in advance
        ListAllDelete( cScriptList() )
        iScriptCount = hGetListItems( oScriptList, cScriptList() )

        ' Build the string for the reference list and append it to cScriptList()
        for iCurrentScript = 1 to iScriptCount
            cReferenceString = cModuleName & ":" & cScriptList( iCurrentScript )
            hListAppend( cReferenceString, cItemList() )
        next iCurrentScript

    next iCurrentTreeListItem

end sub