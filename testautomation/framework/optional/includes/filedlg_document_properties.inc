'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: filedlg_document_properties.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:14 $
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
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Access document properties
'*
'\******************************************************************************

testcase tFiledlgDocumentProperties()


    dim cFileName as string
        cFileName = "DigitalSignature" & hGetSuffix( "current" )
        
    dim cFilePath as string
        cFilePath = gTesttoolPath & "framework\optional\input\security"
        cFilePath = convertpath( cFilePath )
        
    dim cFileURL as string : cFileURL = cFilePath & gPathSigne & cFileName
        
    dim iFileSize as long
    dim cFileSize as string
    dim iFileSizeInfo as long
    dim cSignatureCreator as string
    
    dim cTestExpression as string
    
    dim cSelectAll as string
    dim cCopy as string

    dim brc as boolean
        
    printlog( "Using filename: " & cFileName )
    printlog( "Using filepath: " & cFilePath )
    
    if ( FileExists( cFileURL ) ) then
    	iFileSize = FileLen( cFileURL )
        printlog( "Testfile size is: " & iFileSize & " Bytes" )
    else
    	warnlog( "The workfile does not exist. aborting test" )
    	goto endsub
    endif
    
    hFileOpen( cFileURL )
    
    FileProperties
    
    brc = hDocumentInfoSelectTab( "General" )
    if ( not brc ) then
        warnlog( "Unable to switch to the requested tabpage, trying to recover" )
        kontext "TabDokument"
        TabDokument.cancel()
        hDestroyDocument()
    endif
        
    cTestExpression = hGetStringFromStaticTextField( FileLocationInfo )
    if ( cTestExpression = cFilePath ) then
    	printlog( "Path is correct" )
    else
        warnlog( "The path string is incorrect: " & cTestExpression )
    endif

    kontext "TabDokument"
    cFileSize = hGetStringFromStaticTextField( FileSizeInfo )
    iFileSizeInfo =  hConvertStringToLong( cFileSize )
    if ( iFileSize <> iFileSizeInfo ) then	
        warnlog( "Filesize does not match, please check" )
        printlog( "Found...: " & iFileSizeInfo )
        printlog( "Expected: " & iFileSize    )
    else
        printlog( "Filesize is correct" )
    endif
    
    kontext "TabDokument"
    TabDokument.cancel()
    
    hDestroyDocument()
    

endcase

