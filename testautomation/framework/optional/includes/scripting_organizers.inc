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
'*  short description : Test scripting-organizers / document-attached scripts
'*
'\******************************************************************************

testcase tScriptingOrganizers( iDialog as integer )

    if ( instr( gtSysName , "Solaris" ) > 0 ) then
	qaerrorlog( "#i93502# JavaScript organizer broken" )
        goto endsub
    endif

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

    printlog( "" ) 
    printlog( "Test init: set macro security level to low" )
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_LOW )
    
    select case iDialog
    case 1 : cScriptName = cModName & ".bsh"
    case 2 : cScriptName = cModName & ".js"
    end select
      
    brc = hDeleteFile( cFilePath )
    
    printlog( "Test init: Create and prepare document" )
    brc =  hCreateDocument()
    
    call hChangeDoc()
    
    brc = hFileSaveAsKill( cFilePath )
    
    printlog( "" )
    printlog( "Test 1: ScriptOrganizer/Documents" )
    brc = hOpenScriptOrganizer( iDialog )
    if ( not brc ) then
        warnlog( "Could not open the script Organizer, aborting" )
        brc = hDestroyDocument()
        goto endsub
    endif
    
    kontext "ScriptOrganizer"
    iItemCount = hGetNodeCount( ScriptTreeList )
    
    if ( ScriptTreeList.getItemCount() <> IDOCPOS ) then
        warnlog( "Incorrect number of documents listed in treelist, aborting" )
        kontext "ScriptOrganizer"
        ScriptOrganizer.cancel()
        brc = hDestroyDocument()
        goto endsub
    endif    
    
    kontext "ScriptOrganizer"
    ScriptTreeList.select( IDOCPOS )
    
    if ( not PBCreate.isEnabled() ) then
        warnlog( "Create button should be enabled" )
    endif
    
    if ( PBEdit.isEnabled() ) then
        warnlog( "Edit button should be disabled" )
    endif
    
    if ( PBRename.isEnabled() ) then
        warnlog( "Rename button should be disabled" )
    endif
    
    if ( PBDelete.isEnabled() ) then    
        qaerrorlog( "#i50527# - Delete button should be disabled" )
    endif
    

    printlog( "" )
    printlog( "Test 2: Libraries/Scripts create/rename" )
    
    brc = hCreateScriptingObject( "initial_lib" )
    
    brc = hRenameScriptingObject( cLibName )
    
    brc = hVerifyNodeName( ScriptTreeList , cLibName )
    if ( not brc ) then
        qaerrorlog(  "#i50526# Renaming failed: " & cLibName )
    endif
    
    cLibName = ScriptTreeList.getSelText()
    
    if ( not PBCreate.isEnabled() ) then
        warnlog( "Create button should be enabled" )
    endif
    
    if ( PBEdit.isEnabled() ) then
        warnlog( "Edit button should be disabled" )
    endif
    
    if ( not PBRename.isEnabled() ) then
        warnlog( "Rename button should be enabled" )
    endif
    
    if ( not PBDelete.isEnabled() ) then
        warnlog( "Delete button should be enabled" )
    endif
    
    brc = hCreateScriptingObject( "initial_mod" )
    
    brc = hRenameScriptingObject( cModName )
    
    brc = hVerifyNodeName( ScriptTreeList , cScriptName )
    if ( not brc ) then
        qaerrorlog( "#i50526# Renaming failed: " & cScriptName )
    endif    
    
    cModName = ScriptTreeList.getSelText()
    
    if ( PBCreate.isEnabled() ) then
        warnlog( "Create button should not be enabled" )
    endif
    
    if ( not PBEdit.isEnabled() ) then
        warnlog( "Edit button should be enabled" )
    endif

    if ( not PBRename.isEnabled() ) then
        warnlog( "Rename button should be enabled" )
    endif

    if ( not PBDelete.isEnabled() ) then
        warnlog( "Delte button should be enabled" )
    endif
    
    kontext "ScriptOrganizer"
    hCloseDialog( ScriptOrganizer, "close" )
    
    printlog( "" )
    printlog( "Test 3: Script storage in documents (save/load)" )
    try
        hFileSave
    catch
        warnlog( "Document Changed status not set" )
        hChangeDoc()
        hFileSave
    endcatch
    
    brc = hDestroyDocument()   
    
    brc = hFileOpen( cFilePath )
    
    brc = hOpenScriptOrganizer( iDialog )
    
    ScriptTreeList.select( 3 )
    
    iPos = hExpandNode( ScriptTreeList , 0 )
    
    ScriptTreeList.select( 4 )
    
    iPos = hExpandNode( ScriptTreeList , 0 )
    ScriptTreeList.select( 5 )
    

    printlog( "" )
    
    printlog( "Test 4: Script removal / Document changed" )
    brc = hDeleteScript( cModName , true )
    if ( not brc ) then
        warnlog( "Bad rc: hDelteScript()" )
    endif    
    
    brc = hDeleteScript( cLibName , true )
    if ( not brc ) then
        warnlog( "Bad rc: hDelteScript()" )
    endif        
    
    brc = hDeleteScript( cFileName , false )
    if ( not brc ) then
       qaerrorlog( "#i50527# Delete enabled for document object" )
    endif
        
    kontext "ScriptOrganizer"
    hCloseDialog( ScriptOrganizer, "close" )
    
    try
        hFileSave
    catch
        warnlog( "Document Changed status expected after script deletion" )
        brc = hChangeDoc()
        hFileSave
    endcatch
    
    brc = hDestroyDocument()   
    
    printlog( "Reload the document." )
    brc = hFileOpen( cFilePath )
    if ( brc ) then
        qaerrorlog( "#i50530# Macros not completely removed from doc when deleted" )
    endif
    
    printlog( "" )
    printlog( "Test 5: Verification of script removal" )
    brc = hOpenScriptOrganizer( iDialog )
    kontext "ScriptOrganizer"
    iItemCount = hGetNodeCount( ScriptTreeList )
    if ( iItemCount <> IDOCPOS ) then
        warnlog( "Treelist not populated, the test cannot continue" )
        kontext "ScriptOrganizer"
        hCloseDialog( ScriptOrganizer, "close" )
        brc = hDestroyDocument()
        goto endsub
    endif
    
    iPos = hExpandNode( ScriptTreeList , IDOCPOS )
    if ( iPos <> 3 ) then
        warnlog( "Not all libraries/scripts have been deleted" )
    endif
            
    kontext "ScriptOrganizer"
    hCloseDialog( ScriptOrganizer, "close" )

    kontext "Navigator"
    hCloseDialog( Navigator, "close,optional" )
    
    brc = hDestroyDocument()
    
    brc = hDeleteFile( cFilePath )
    if ( not brc ) then
        warnlog( "Could not delete: " & cFilePath & " - do so manually!" )
    endif
    
    
    hSetmacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_DEFAULT )
    

endcase
