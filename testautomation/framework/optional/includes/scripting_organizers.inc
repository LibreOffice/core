'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: scripting_organizers.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:15 $
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
'*  short description : Test scripting-organizers / document-attached scripts
'*
'\******************************************************************************

testcase tScriptingOrganizers( iDialog as integer )

    if ( instr( gtSysName , "Solaris" ) > 0 ) then
	qaerrorlog( "#i93502# JavaScript organizer broken" )
        goto endsub
    endif

    '///<H1>Test scripting-organizers</H1>
    ' 1 = BeanShell (.bsh)
    ' 2 = JavaScript (.js)
    
    const IDOCPOS = 3

    dim cFileName as string
        cFileName = "sftest" & hGetSuffix( "current" )
    dim cCompare as string
        
    dim cFilePath as string
        cFilePath= hGetWorkPath() & cFileName
        cFilePath= convertpath( cFilePath )
        
    dim iPos as integer  ' position of the workdocument in treelist
    dim brc as boolean   ' boolean return code used to determine the status
    
    dim cLibName as string
        cLibName = "TTLib"
    dim cModName as string
        cModName = "TTMod"
    dim cScriptName as string
    
    dim iItemCount as integer

    '///<ul>
    '///+<li>Set macro security level to low</li>
    printlog( "" ) 
    printlog( "Test init: set macro security level to low" )
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_LOW )
    
    '///+<li>For both script organizers (javascript/beanshell) do</li>
    '///<ul>
    '///+<li>Create a script name of type TTMod.(.bsh|js)</li>
    select case iDialog
    case 1 : cScriptName = cModName & ".bsh"
    case 2 : cScriptName = cModName & ".js"
    end select
      
    '///+<li>Delete the workfile in case it has been left over by prior run</li>
    brc = hDeleteFile( cFilePath )
    
    '///+<li>Open a new document</li>
    printlog( "Test init: Create and prepare document" )
    brc =  hCreateDocument()
    
    '///+<li>Modify the document</li>
    call hChangeDoc()
    
    '///+<li>Save the document (Menu File->Save As...)</li>
    brc = hFileSaveAsKill( cFilePath )
    
    '///+<li>Open the ScriptOrganizer</li>
    printlog( "" )
    printlog( "Test 1: ScriptOrganizer/Documents" )
    brc = hOpenScriptOrganizer( iDialog )
    if ( not brc ) then
        warnlog( "Could not open the script Organizer, aborting" )
        brc = hDestroyDocument()
        goto endsub
    endif
    
    '///+<li>Count the objects in the collapsed treelist</li>
    kontext "ScriptOrganizer"
    iItemCount = hGetNodeCount( ScriptTreeList )
    
    '///+<li>Verify that there are exactly three items in the treelist</li>
    if ( ScriptTreeList.getItemCount() <> IDOCPOS ) then
        warnlog( "Incorrect number of documents listed in treelist, aborting" )
        kontext "ScriptOrganizer"
        ScriptOrganizer.cancel()
        brc = hDestroyDocument()
        goto endsub
    endif    
    
    '///+<li>Select the document at pos. 3</li>
    kontext "ScriptOrganizer"
    ScriptTreeList.select( IDOCPOS )
    
    '///+<li>Test status of Buttons</li>
    '///<ul>
    '///+<li>&quot;Create&quot; should be enabled</li>
    if ( not PBCreate.isEnabled() ) then
        warnlog( "Create button should be enabled" )
    endif
    
    '///+<li>&quot;Edit&quot; should be disabled</li>
    if ( PBEdit.isEnabled() ) then
        warnlog( "Edit button should be disabled" )
    endif
    
    '///+<li>&quot;Rename&quot; should be disabled</li>
    if ( PBRename.isEnabled() ) then
        warnlog( "Rename button should be disabled" )
    endif
    
    '///+<li>&quot;Delete&quot; should be disabled</li>
    if ( PBDelete.isEnabled() ) then    
        qaerrorlog( "#i50527# - Delete button should be disabled" )
    endif
    '///</ul>
    

    printlog( "" )
    printlog( "Test 2: Libraries/Scripts create/rename" )
    
    '///+<li>Create a new library</li>
    brc = hCreateScriptingObject( "initial_lib" )
    
    '///+<li>Rename the library</li>
    brc = hRenameScriptingObject( cLibName )
    
    '///+<li>Verify that renaming worked</li>
    brc = hVerifyNodeName( ScriptTreeList , cLibName )
    if ( not brc ) then
        qaerrorlog(  "#i50526# Renaming failed: " & cLibName )
    endif
    
    '///+<li>Remember name of the library, it is deleted later</li>
    cLibName = ScriptTreeList.getSelText()
    
    '///+<li>Test status of Buttons</li>
    '///<ul>
    '///+<li>&quot;Create&quot; should be enabled</li>
    if ( not PBCreate.isEnabled() ) then
        warnlog( "Create button should be enabled" )
    endif
    
    '///+<li>&quot;Edit&quot; should be disabled</li>
    if ( PBEdit.isEnabled() ) then
        warnlog( "Edit button should be disabled" )
    endif
    
    '///+<li>&quot;Rename&quot; should be enabled</li>
    if ( not PBRename.isEnabled() ) then
        warnlog( "Rename button should be enabled" )
    endif
    
    '///+<li>&quot;Delete&quot; should be enabled</li>
    if ( not PBDelete.isEnabled() ) then
        warnlog( "Delete button should be enabled" )
    endif
    '///</ul>
    
    '///+<li>Create a new script</li>
    brc = hCreateScriptingObject( "initial_mod" )
    
    '///+<li>Rename the script</li>
    brc = hRenameScriptingObject( cModName )
    
    '///+<li>Verify that renaming worked</li>
    brc = hVerifyNodeName( ScriptTreeList , cScriptName )
    if ( not brc ) then
        qaerrorlog( "#i50526# Renaming failed: " & cScriptName )
    endif    
    
    '///+<li>Remember the module name, it is deleted later</li>
    cModName = ScriptTreeList.getSelText()
    
    '///+<li>Test status of Buttons</li>
    '///<ul>
    '///+<li>&quot;Create&quot; should be disabled</li>
    if ( PBCreate.isEnabled() ) then
        warnlog( "Create button should not be enabled" )
    endif
    
    '///+<li>&quot;Edit&quot; should be enabled</li>
    if ( not PBEdit.isEnabled() ) then
        warnlog( "Edit button should be enabled" )
    endif

    '///+<li>&quot;Rename&quot; should be enabled</li>
    if ( not PBRename.isEnabled() ) then
        warnlog( "Rename button should be enabled" )
    endif

    '///+<li>&quot;Delete&quot; should be enabled</li>
    if ( not PBDelete.isEnabled() ) then
        warnlog( "Delte button should be enabled" )
    endif
    '///</ul>
    
    '///+<li>Exit the scriptorganizer (cancel)</li>
    kontext "ScriptOrganizer"
    brc = hCloseScriptOrganizer()
    
    '///+<li>Save the file</li>
    printlog( "" )
    printlog( "Test 3: Script storage in documents (save/load)" )
    try
        hFileSave
    catch
        warnlog( "Document Changed status not set" )
        hChangeDoc()
        hFileSave
    endcatch
    
    '///+<li>Close the document</li>
    brc = hDestroyDocument()   
    
    '///+<li>Reload the document.</li>
    brc = hFileOpen( cFilePath )
    
    '///+<li>Open the ScriptOrganizer</li>
    brc = hOpenScriptOrganizer( iDialog )
    
    '///+<li>Select the document</li>
    ScriptTreeList.select( 3 )
    
    '///+<li>Expand the node</li>
    iPos = hExpandNode( ScriptTreeList , 0 )
    
    '///+<li>Select the module</li>
    ScriptTreeList.select( 4 )
    
    '///+<li>Select the script</li>
    iPos = hExpandNode( ScriptTreeList , 0 )
    ScriptTreeList.select( 5 )
    

    printlog( "" )
    
    '///+<li>Delete the script (should work)</li>
    printlog( "Test 4: Script removal / Document changed" )
    brc = hDeleteScript( cModName , true )
    if ( not brc ) then
        warnlog( "Bad rc: hDelteScript()" )
    endif    
    
    '///+<li>Delete the module (should work)</li>
    brc = hDeleteScript( cLibName , true )
    if ( not brc ) then
        warnlog( "Bad rc: hDelteScript()" )
    endif        
    
    '///+<li>Delete the document (which should not work)</li>
    brc = hDeleteScript( cFileName , false )
    if ( not brc ) then
       qaerrorlog( "#i50527# Delete enabled for document object" )
    endif
        
    '///+<li>Exit all dialogs (close the script organizer)</li>
    kontext "ScriptOrganizer"
    brc = hCloseScriptOrganizer()
    
    '///+<li>Save the file via File Save button on standardbar</li>
    try
        hFileSave
    catch
        warnlog( "Document Changed status expected after script deletion" )
        brc = hChangeDoc()
        hFileSave
    endcatch
    
    '///+<li>Close the document</li>
    brc = hDestroyDocument()   
    
    '///+<li>Reload the document.</li>
    printlog( "Reload the document." )
    brc = hFileOpen( cFilePath )
    if ( brc ) then
        qaerrorlog( "#i50530# Macros not completely removed from doc when deleted" )
    endif
    
    '///+<li>Open the ScriptOrganizer, look for the library</li>
    printlog( "" )
    printlog( "Test 5: Verification of script removal" )
    brc = hOpenScriptOrganizer( iDialog )
    kontext "ScriptOrganizer"
    iItemCount = hGetNodeCount( ScriptTreeList )
    if ( iItemCount <> IDOCPOS ) then
        warnlog( "Treelist not populated, the test cannot continue" )
        brc = hCloseScriptOrganizer()
        brc = hDestroyDocument()
        goto endsub
    endif
    
    '///+<li>Verify that all libraries/scripts have been removed</li>
    iPos = hExpandNode( ScriptTreeList , IDOCPOS )
    if ( iPos <> 3 ) then
        warnlog( "Not all libraries/scripts have been deleted" )
    endif
            
    '///+<li>Test Exit: Cleanup - close all dialogs</li>
    brc = hCloseScriptOrganizer()

    '///+<li>Close the navigator if it is open</li>
    brc = hCloseNavigator()
    
    '///+<li>Close the document</li>
    brc = hDestroyDocument()
    
    '///+<li>Delete the workfile</li>
    brc = hDeleteFile( cFilePath )
    if ( not brc ) then
        warnlog( "Could not delete: " & cFilePath & " - do so manually!" )
    endif
    
    '///</ul>
    
    '///+<li>Set macro security level to default</li>
    hSetmacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_DEFAULT )
    
    '///</ul>

endcase
