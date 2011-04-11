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
'/******************************************************************************
'*
'*  owner : gregor.hartmann@oracle.com
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

'*******************************************************************************

function hDocumentInfoSelectTab( cTabPage as string ) as boolean


    '///<h3>Switch between the tabpages in the document info dialog</h3>
    '///<i>The declaration of the document info dialog is not complete which
    '///+ enforces special treatment</i><br><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>

    '///+<li>The name of the tabpage to be activated (String)</li>
    '///<ul>
    '///+<li>&quot;General&quot;</li>
    '///+<li>&quot;Description&quot;</li>
    '///+<li>&quot;User&quot;</li>
    '///+<li>&quot;Internet&quot;</li>
    '///+<li>&quot;Statistics&quot;</li>
    '///+<li>The string is case insensitive</li>

    '///</ul>

    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Errorcondition (Boolean)</li>
    '///<ul>
    '///+<li>TRUE if tabpage is known and switching worked</li>
    '///+<li>FALSE on icorrect input parameter</li>
    '///+<li>FALSE on any other error</li>
    '///</ul>
    '///</ol>

    const CFN = "hDocumentInfoSelectTab::"
    printlog( CFN & "Enter with option (tabpage): " & cTabPage )
    dim brc as boolean 'a multi purpose boolean returnvalue
        brc = true

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Kontext to the dialog</li>
    Kontext

    '///+<li>Select the page to switch to, context to the new page</li>
    select case ( ucase( cTabPage ) )
    case "GENERAL"     : active.setPage TabDokument     : kontext "TabDokument"
    case "DESCRIPTION" : active.setPage TabBeschreibung : kontext "TabBeschreibung"
    case "USER"        : warnlog( "#i95523# - Cannot access controls on Custom page" )
                         brc = false
                        'active.setPage TabBenutzer     : kontext "TabBenutzer"
    case "INTERNET"    : active.setPage TabInternet     : kontext "TabInternet"
    case "STATISTICS"  : active.setPage TabStatistik    : kontext "TabStatistik"
    case default       : brc = false
    end select

    '///</ul>

    printlog( CFN & "Exit with result: " & brc )
    hDocumentInfoSelectTab() = brc

end function


