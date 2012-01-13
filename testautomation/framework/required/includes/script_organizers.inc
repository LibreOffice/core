'encoding UTF-8  Do not remove or change this line!
'**************************************************************
'  
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'  
'**************************************************************
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