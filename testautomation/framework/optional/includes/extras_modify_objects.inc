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


    dim iObjectFolder as integer
    dim iObjectFolders as integer
   
    dim iObject as integer    ' Iterator
    dim iObjectCount( 20 ) as integer
    dim iObjectSum as integer : iObjectSum = 0
    dim iObjectCountExpected as integer
   
    dim sFile as string
    dim sPath as string
        sPath = hGetWorkPath()  

    dim iReset as integer : iReset = 0

    printlog( "" )
    
    select case( lcase( cCategory ))
    case "templates" : iObjectCountExpected = TEMPLATE_COUNT
    case "samples"   : iObjectCountExpected = SAMPLE_COUNT
    end select
   
    printlog( "Open Templates and Samples dialog" )
    FileNewFromTemplate
    WaitSlot( 2000 )
    hSelectCategory( cCategory )
    iObjectFolders = FileList.getItemCount()
   
    printlog( "Count the items in the templates and samples dialog" )
    for iObject = 1 to iObjectFolders
        hSelectFileFolder( iObject , true )
        iObjectCount( iObject ) = FileList.getItemCount()
        iObjectSum = iObjectSum + iObjectCount( iObject )
        UpOneLevel.click()
    next iObject
    
    printlog( "Verify object count (en_US only)" )
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
    printlog( "Switch from preview to document info" )
    docinfo.click()
    TemplateAndDocuments.cancel()
   
    printlog( "" )
    printlog( "Starting to load/save/close/reload/close all Objects" )
    printlog( "" )
   
    for iObjectFolder = 1 to iObjectFolders
   
        for iObject = 1 to iObjectCount( iObjectFolder ) 
      
            gApplication = "WRITER"    
            
            printlog( "" )
            printlog( "Folder index = " & iObjectFolder & ", Object index = " & iObject )
            
            hFileCloseAll()
            
            if ( iReset = 10 ) then
                call exitRestartTheOffice()
                iReset = 0
            endif
          
            printlog( "Select the item we want to load" )
            FileNewFromTemplate
            WaitSlot( 2000 )
            hSelectCategory( cCategory )
            hSelectFileFolder( iObjectFolder , false )
            hSelectDocumentObject( iObject , iMode )
            
            ' treat it like a folder. If it is, skip it. Otherwise perform the test
            kontext "TemplateAndDocuments"
            if ( TemplateAndDocuments.exists() ) then
                if ( not edit.isEnabled() ) then
                    printlog( CFN & "Object is folder or #edit# is disabled, skipping" )
                    UpOneLevel.click()
                    TemplateAndDocuments.cancel()
                endif
            else
                printlog( "Loading file..." )
                printlog( "Check for Filter Options dialog" )
                if ( hFileWait( false ) = -6 ) then 
                    warnlog( "Load failure: ASCII filter dialog displayed, recovering" )
                    kontext "FilterAuswahl"
                    FilterAuswahl.cancel()
                    hDestroyDocument()
                else
                    printlog( "Build filename; save, reload and delete" )
                    sFile = sPath & cCategory & iObjectFolder & "_" & iObject
                    hCloseNavigator()
                    hFileSaveAsKill( sFile )
                    hDestroyDocument()
                    hFileOpen( sFile )
                    hHandleActivesOnLoad( 1 , 2 )
                    hCloseNavigator()
                    hDestroyDocument()
                    hDeleteFile( sFile )
                    printlog( "Test cycle done, going for next object." )
                endif
            endif
            
            iReset = iReset + 1
      
        next iObject
      
    next iObjectFolder
    
    hDestroyDocument()
   
endcase   
   

