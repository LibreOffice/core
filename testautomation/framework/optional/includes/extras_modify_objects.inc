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
'* owner : gregor.hartmann@oracle.com
'*
'* short description : Perform standard operations on all samples/templates etc.
'*
'\***********************************************************************

private const MAX_FILE_COUNT = 3000 ' the max number of templates or samples

' NOTE: Usually we have only about a 100 templates but in multi-language
'       installations there are more than 2100. You should not run these     
'       tests on multi language installations. Really.

testcase tModifyObjects( iMode as integer, cCategory as string, sVer as string )

    ' This test loads/saves and reloads every template or sample shipped with
    ' the product. One of the special challenges of the test is to find out
    ' which document type a sample/template belongs to as well as to handle
    ' any possible dialogs the samples/documents may trigger.

    ' iMode = 1 means to open the document (0 = just get its name)
    ' cCategory may be "SAMPLES" or "TEMPLATES"
    ' sVER is the fileformat for saving files, typically this is "current"

    const CFN = "tModifyObjects::"

    const TEMPLATE_COUNT = 237 ' For en-US/Oracle Open Office, numbers may differ for Languages/Brands
    const SAMPLE_COUNT = 56    ' For en-US/Oracle Open Office, numbers may differ for Languages/Brands

    ' Varaibles needed to navigate on the Templates And Samples dialog
    dim iObjectFolder as integer
    dim iObjectFolders as integer

    ' Iterators and number of documents, test values
    dim iObject as integer    ' Iterator
    dim iObjectCount( 100 ) as integer
    dim iObjectSum as integer : iObjectSum = 0
    dim iObjectCountExpected as integer

    ' Where to store the files
    dim sFile as string
    dim sPath as string : sPath = hGetWorkPath()

    ' Reset counter, if the office becomes unstable you might want to use it.
    ' See further below on how to enable this feature
    dim iReset as integer : iReset = 0

    printlog( "" )

    ' Find out if we are testing samples or templates, fix the number of items
    ' allowed for each category
    select case( lcase( cCategory ) )
    case "templates" : iObjectCountExpected = TEMPLATE_COUNT
    case "samples"   : iObjectCountExpected = SAMPLE_COUNT
    end select
   
    printlog( "Open Templates and Samples dialog" )
    FileNewFromTemplate
    WaitSlot( 2000 )

    ' This is the icon on the left pane
    hSelectCategory( cCategory )

    ' The number of folders for the category on the middle pane
    iObjectFolders = FileList.getItemCount()
   
    printlog( "Count the items in the templates and samples dialog" )
    for iObject = 1 to iObjectFolders
        hSelectFileFolder( iObject , true )
        iObjectCount( iObject ) = FileList.getItemCount()
        iObjectSum = iObjectSum + iObjectCount( iObject )
        UpOneLevel.click()
    next iObject
    
    printlog( "Verify object count (en_US only)" )
    if ( gProductName = "Oracle Open Office" and gISOLang = "en-US" ) then
        if ( iObjectSum <> iObjectCountExpected ) then
            warnlog( "Number of objects has changed. OLD: " & iObjectCountExpected &_
            " NEW: " & iObjectSum )
        else
            printlog( "Object count is ok for en-US / Oracle Open Office" )
        endif
    else
        printlog( "Template count testing skipped for non-en_US/StarOffice" )
    endif
   
    Kontext "TemplateAndDocuments"
    printlog( "Switch from preview to document info, required to limit load-times" )
    ' Another good test case would be to enable the preview.
    docinfo.click()
    hCloseDialog( TemplateAndDocuments, "cancel" )
   
    printlog( "" )
    printlog( "Starting to load/save/close/reload/close all Objects" )
    printlog( "" )

    ' Iterate over the folders in the middle pane of the templates and samples
    ' dialog, open the folders and load the containing files
    ' If you want to debug the office using this test, get the Object indices
    ' from the logfiles and change the loops to only load the file in question

    ' These are the folders
    for iObjectFolder = 1 to iObjectFolders
    ' for iObjectFolder = MyFailedFolder to MyFailedFolder

        ' These are the individual files
        for iObject = 1 to iObjectCount( iObjectFolder )
        ' for iObject = MyFailedFile to MyFailedFile

            ' Preset gApplication to "Writer", this is the basis
            gApplication = "WRITER"    
            
            printlog( "" )
            printlog( "Folder index = " & iObjectFolder & ", Object index = " & iObject )

            printlog( "Close all files" )
            hFileCloseAll()

            ' every now and then the office becomes a little unstable.
            ' Enable the if()-block then, otherwise you might miss some broken
            ' documents (because the test aborts too early)
            'if ( iReset = 10 ) then
            '    call exitRestartTheOffice()
            '    iReset = 0
            'endif
          
            printlog( "Open <FileNewFromTemplate>" )
            FileNewFromTemplate
            WaitSlot( 2000 )

            printlog( "Select category: " & cCategory )
            hSelectCategory( cCategory )

            printlog( "Select a folder, do not print debug info" )
            hSelectFileFolder( iObjectFolder , false )

            printlog( "Select a document by index, iMode = 1 -> Open it" )
            hSelectDocumentObject( iObject , iMode )
            
            ' In few cases we accidentially work on a folder. So we test for the
            ' Templates and Samples dialog again, if it is closed we have a
            ' regular document, otherwise we have a folder which we then skip.
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

                ' Yes, hFileWait() knows which dialog is in the way so we can ask it
                ' directly. The ASCII filter dialog has the number -6
                if ( hFileWait( false ) = -6 ) then 
                    warnlog( "Load failure: ASCII filter dialog displayed, recovering" )
                    kontext "FilterAuswahl"
                    hCloseDialog( FilterAuswahl, "cancel" )
                    hDestroyDocument()
                else
                    printlog( "Build filename; save, reload and delete" )
                    sFile = sPath & cCategory & iObjectFolder & "_" & iObject
                    printlog( "File is: " & sFile )

                    kontext "Navigator"
                    printlog( "Close Navigator (if present)" )
                    hCloseDialog( Navigator, "close,optional" )

                    printlog( "Save file" )
                    hFileSaveAsKill( sFile )

                    printlog( "Close file" )
                    hDestroyDocument()

                    printlog( "Open file" )
                    hFileOpen( sFile )

                    printlog( "Handle possible dialogs" )
                    hHandleActivesOnLoad( 1 , 2 )

                    kontext "Navigator"
                    printlog( "Close Navigator (if present)" )
                    hCloseDialog( Navigator, "close,optional" )

                    printlog( "Close document" )
                    hDestroyDocument()

                    printlog( "Delete work document" )
                    hDeleteFile( sFile )
                endif
            endif

            iReset = iReset + 1
      
        next iObject
      
    next iObjectFolder

    printlog( "Close last remaining document(s) and exit test" )
    hFileCloseAll()
   
endcase   
   

