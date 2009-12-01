'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: extras_file_open.inc,v $
'*
'* $Revision: 1.4 $
'*
'* last change: $Author: rt $ $Date: 2008-09-04 09:15:59 $
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

testcase tOpenObjectsWithFileDialog( cCategory as string )

    const CFN = "tOpenObjectsWithFileDialog::"   


    dim lsFile ( MAX_FILE_COUNT ) as String ' list of all files below samples or templates
    dim sRootPath as String       ' rootpath, depends on cCategory
    dim sRootPathFallback as string

    dim sFilter as string
      
    dim sFileIn as String         ' file to open
    dim sFileOut as string        ' file to save, reload, delete
    dim sPathOut as string
        sPathOut = hGetWorkPath()
    dim sExtension as String      ' filter: determines whether to test the file
    
    dim iCurrentTemplate as Integer ' increment: current template from lsFile()
    dim iTemplateCount as integer
    
    dim bSkipFile as boolean      ' turns true if file is to be skipped
    dim brc as boolean            ' some multi purpose boolan returncode
    dim cOfficePath as string

    ' the path where the samples/templates are located, verify function params
    ' note that for some time the suffixes for the different filetypes are changed,
    ' so we have separate names for samples and templates.
        
    if ( FileExists( gOfficeBasisPath ) ) then
        cOfficePath = gOfficeBasisPath
    else
        cOfficePath = mid( gOfficeBasisPath, len( gNetzOfficePath ) + 1 )
    endif
    printlog( "Using: " & cOfficePath )
    
    select case cCategory
    case "SAMPLES"    : sRootPath = cOfficePath & "share\samples\" & gISOLang
                        sRootPathFallback = "/opt/openoffice.org/basis3.0/share/samples/" & gISOLang
                        sFIlter   = "*.od*"
    case "TEMPLATES"  : sRootPath = cOfficePath & "share\template\" & gISOLang
                        sRootPathFallback = "/opt/openoffice.org/basis3.0/share/template/" & gISOLang
                        sFilter   = "*.ot*"
    case default
        warnlog( CFN & "Invalid category passed to function" )
        goto endsub
    end select
   
    sRootPath = convertpath( sRootPath )
   
    if ( dir( sRootPath ) = "" ) then
        qaerrorlog( "Root Path does not exist: " & sRootPath )
        sRootPath = sRootPathFallback
        if ( dir( sRootPathFallback ) = "" ) then
            warnlog( "No usable office root path found. Aborting test" )
            goto endsub
        printlog( "Using hard coded fallback" )
        endif
    endif

    printlog( "Using path..: " & sRootPath )
    printlog( "Using filter: " & sFilter   )
    GetAllFileList ( sRootPath, sFilter, lsFile() )
    hListDelete( lsFile(), 1 )

    ' Remove .lock-files
    iTemplateCount   = listCount( lsFile() )
    iCurrentTemplate = 1
    while( iCurrentTemplate <= iTemplateCount ) 
        if ( instr( lsFile( iCurrentTemplate ), "~lock." ) > 0 ) then
            hListDelete( lsFile(), iCurrentTemplate )
            iTemplateCount = iTemplateCount - 1
        else
            iCurrentTemplate = iCurrentTemplate + 1
        endif
    wend

    ' do not test wizard related files
    iTemplateCount   = listCount( lsFile() )
    iCurrentTemplate = 1
    while( iCurrentTemplate <= iTemplateCount ) 
        if ( instr( lsFile( iCurrentTemplate ), "wizard" ) > 0 ) then
            hListDelete( lsFile(), iCurrentTemplate )
            iTemplateCount = iTemplateCount - 1
        else
            iCurrentTemplate = iCurrentTemplate + 1
        endif
    wend

    hListPrint ( lsFile() , "" , "" )
    if ( listcount( lsfile() ) = 1 ) then
        if ( gProductName = "OpenOffice.org" ) then
            printlog( gProductName & " comes with no samples, skipping" )
            goto endsub
        else
    	    warnlog( "Filelist seems to be incomplete. GetAllFileList() failed!" )
    	    goto endsub
    	endif
    endif

    iTemplateCount = listCount( lsFile() )
    for iCurrentTemplate = 1 to iTemplateCount
   
        printlog( "" )
        printlog( "Loading file " & iCurrentTemplate & " of " & iTemplateCount )
      
        ' sFileIn holds the file we currently work with
        sFileIn = lsFile( iCurrentTemplate )
        
        brc = hFileOpen( sFileIn ) 
        brc = hHandleActivesOnLoad( 0 , 2 )
        
        Kontext "FilterAuswahl"        
        if ( FilterAuswahl.exists( 1 ) ) then
            warnlog( "Loading failed, ASCII filter dialog present" )
            FilterAuswahl.cancel()
            while( getDocumentCount > 0 ) 
                hDestroyDocument()
            wend
        else
        
            brc = hCloseNavigator()
    
            ' Build the filename
            sFileOut = cCategory & "_" & iCurrentTemplate
            sFileOut = convertpath( sPathOut & sFileOut ) 
          
            brc = hFileSaveAsKill( sFileOut )
    
            ' note: this is delayed, because the dialogs take time to pop up
            '       even while the dialog is visible, we can work with the doc.
            brc = hHandleInitialDialogs()
            
            brc = hDestroyDocument()
          
            brc = hFileOpen( sFileOut )
            brc = hHandleActivesOnLoad( 0 , 2 )
            brc = hCloseNavigator()
    
            brc = hDestroyDocument()
        
        endif
  
        brc = hDeleteFile( sFileOut )
        
    next iCurrentTemplate
   
endcase


