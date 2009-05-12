'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: extras_modify_objects.inc,v $
'*
'* $Revision: 1.3 $
'*
'* last change: $Author: rt $ $Date: 2008-08-28 11:40:12 $
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
'* owner : joerg.skottke@sun.com
'*
'* short description : Perform standard operations on all samples/templates etc.
'*
'\***********************************************************************

private const MAX_FILE_COUNT = 3000 ' the max number of templates or samples

' NOTE: Usually we have only about a 100 templates but in multi-language
'       installations there are more than 2100. You should not run these     
'       tests on multi language installations. Really.

testcase tModifyObjects( iMode as integer, cCategory as string, sVer as string )

    const CFN = "tModifyObjects::"
    const RESTART = 15
    const TEMPLATE_COUNT = 236 ' For en-US/StarOffice, numbers may differ for Languages/Brands
    const SAMPLE_COUNT = 60    ' For en-US/StarOffice, numbers may differ for Languages/Brands

    '///<h1>Open all Objects, save, close, reload, save and close them</h1>
    '///<i>The test should be able to handle any OOo 1.x and 2.x files</i><br>
    '///<ul>

    dim iObjectFolder as integer
    dim iObjectFolders as integer
   
    dim iObject as integer    ' Iterator
    dim iObjectCount( 20 ) as integer
    dim iObjectSum as integer : iObjectSum = 0
    dim iObjectCountExpected as integer
   
    dim sFile as string
    dim sPath as string
        sPath = hGetWorkPath()  
    dim brc as boolean
    dim crc as string
    dim irc as integer
    
    dim iReset as integer : iReset = 0

    printlog( "" )
    
    select case( lcase( cCategory ))
    case "templates" : iObjectCountExpected = TEMPLATE_COUNT
    case "samples"   : iObjectCountExpected = SAMPLE_COUNT
    end select
   
    '///+<li>Open File->New->Templates and documents</li>
    FileNewFromTemplate
   
    '///+<li>select the category in the template dialog (left pane)</li>
    hSelectCategory( cCategory )
   
    '///+<li>Get the number of folders in the root of cCategory</li>
    iObjectFolders = FileList.getItemCount()
   
    '///+<li>get the number of all objects below cCategory and its subfolders</li>
    for iObject = 1 to iObjectFolders
   
        '///+<li>select and open a folder</li>
        hSelectFileFolder( iObject , true )
      
        '///+<li>count the items in the list</li>
        iObjectCount( iObject ) = FileList.getItemCount()
        iObjectSum = iObjectSum + iObjectCount( iObject )
      
        '///+<li>move back up to the root of cCategory</li>
        UpOneLevel.click()
      
    next iObject
    
    if ( gProductName = "StarOffice" and gISOLang = "en-US" ) then
        if ( iObjectSum <> iObjectCountExpected ) then
            warnlog( "Number of objects has changed. OLD: " & iObjectCountExpected &_
            " NEW: " & iObjectSum )
        else
            printlog( "Object count is ok for en-US / StarOffice" )
        endif
    else
        printlog( "Template count testing skipped for non-en_US/StarOffice" )
    endif
   
    Kontext "TemplateAndDocuments"
    
    '///+<li>Click the document-info button, do not load the preview</li>
    docinfo.click()

    '///+<li>close the templates and samples dialog</li>
    TemplateAndDocuments.cancel()
   
    printlog( "" )
    printlog( "Starting to load/save/close/reload/close all Objects" )
    printlog( "" )
   
    '///+<li>Step through all folders and Objects</li>
    for iObjectFolder = 1 to iObjectFolders
   
        for iObject = 1 to iObjectCount( iObjectFolder ) 
      
            ' to skip a document add the rule here and jump to SkipThisObject: 
          
            gApplication = "WRITER"    
            
            printlog( "" )
            printlog( "Folder index = " & iObjectFolder & ", Object index = " & iObject )
            
            do while ( getDocumentCount > 0 ) 
                hDestroyDocument()
            loop
            
            if ( iReset = 10 ) then
                iReset = 0
                call exitRestartTheOffice()
            else
                iReset = iReset + 1
            endif
          
            '///<ol>
            FileNewFromTemplate
      
            '///+<li>Select the desired-category: Templates/Samples ...</li>
            hSelectCategory( cCategory )
      
            '///+<li>Select the current folder</li>
            hSelectFileFolder( iObjectFolder , false )
          
            '///+<li>Select the desired Object, open or edit it</li>
            crc = hSelectDocumentObject( iObject , iMode )
            
            '///+<li>If the object can be edited we open it. Skip otherwise</li>
            ' treat it like a folder
            kontext "TemplateAndDocuments"
            if ( TemplateAndDocuments.exists() ) then
                if ( not edit.isEnabled() ) then
                    crc = "Folder"
                endif
            else
                printlog( "Templates And Documents dialog is closed" )
            endif
            
            '///+<li>If object is a folder, we skip the entire test</li>
            if ( crc = "Folder" ) then
                printlog( CFN & "Object is folder or #edit# is disabled, skipping" )
                UpOneLevel.click()
                TemplateAndDocuments.cancel()
                goto SkipThisObject
            endif
            
            '///+<li>Cancel the filterdialog if present -> bug</li>
            irc = hFileWait( false )
            
            if ( irc = -6 ) then 
                warnlog( "Load failure: ASCII filter dialog displayed" )
                kontext "FilterAuswahl"
                FilterAuswahl.cancel()
            endif
            
            if ( brc ) then
                printlog( CFN & "Skipping document" )
            else
               
                '///+<li>Close the navigator if present</li>
                brc = hCloseNavigator()
          
                '///+<li>Build the filename</li>
                sFile = sPath & cCategory & iObjectFolder & "_" & iObject

                '///+<li>Save the file without suffix, use default filter, overwrite</li>
                brc = hFileSaveAsKill( sFile )
                
                '///+<li>Close the document</li>
                brc = hDestroyDocument()
                
                '///+<li>Load the file again</li>
                brc = hFileOpen( sFile )
                brc = hHandleActivesOnLoad( 1 , 2 )
               
                '///+<li>Close the navigator if present</li>
                brc = hCloseNavigator()
            
                '///+<li>Close the document</li>
                brc = hDestroyDocument()

                '///+<li>Delete the file after completion</li>
                brc = hDeleteFile( sFile )

                '///</ol>
                
            endif
            
            SkipThisObject:
      
        next iObject
      
    next iObjectFolder
    
    hDestroyDocument()
   
   '///</ul>
   
endcase   
   

