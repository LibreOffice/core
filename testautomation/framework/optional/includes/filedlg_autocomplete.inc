'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: filedlg_autocomplete.inc,v $
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
'*  short description : Autocompletion feature
'*
'\******************************************************************************

testcase tAutocomplete

    '///<h1>Autocompletion feature</h1>

    '///<u><pre>Synopsis</pre></u>Test the autocompletion feature, filename completion<br>
    '///<u><pre>Specification document</pre></u>No specification document available<br>
    '///<u><pre>Files used</pre></u>
    '///+ (User-Layer)/user/work/autocomplete<br>
    '///+ (User-Layer)/user/work/autocomplete/autocomplete_a.odt</br>
    '///+ (User-Layer)/user/work/autocomplete/autocomplete_b.odt</br>
    '///+ (User-Layer)/user/work/autocomplete/autocomplete_bb.odt</br>    
    
    ' IMPORTANT NOTE: Autocompletion is asynchronous. This means that no rule 
    '                 exists when exactly it is going to strike. If you are
    '                 using a networked environment you have to make sure that
    '                 all network ressources deliver a decent performance as 
    '                 this seriously influences the speed of the autocompletion.
        
    dim cBasePath as string
        
    const WORKDIR = "autocomplete"
        
    ' These are the files we create in the users homedirectory
    dim aWorkFile( 3 ) as string
        aWorkFile( 1 ) = "autocomplete_a.odt"
        aWorkFile( 2 ) = "autocomplete_b.odt"
        aWorkFile( 3 ) = "autocomplete_bb.odt"
        
    ' These are the full paths to the files
    dim aFullPath( 3 ) as string
        
    ' These are the wildcarded filter strings
    dim aWildCard( 3 )
        aWildCard( 1 ) = "autocomplete_?.odt"
        aWildCard( 2 ) = "autocomplete_b?.odt"
        aWildCard( 3 ) = "autocomplete_*.odt"
                
        
    dim cCurrentPath as string
    dim cAutocomplete as string
    
    dim iCurrentDocument as integer
            
    dim cLeft as string
    dim iLeft as integer
        iLeft = 4
        
    dim cEFString as string
    
    dim slot
        
    '///<u><pre>Test case specification</pre></u>
    '///<ul>
    '///+<li>Init: Make sure we only have no open documents</li>
    do while( getDocumentCount > 0 )
        call hCloseDocument()
    loop
    
    '///+<li>Init: Get the current workpath</li>
    cBasePath = hGetWorkPath()
    aFullPath( 1 ) = cBasePath & WORKDIR & gPathSigne & aWorkFile( 1 )
    aFullPath( 2 ) = cBasePath & WORKDIR & gPathSigne & aWorkFile( 2 )
    aFullPath( 3 ) = cBasePath & WORKDIR & gPathSigne & aWorkFile( 3 )
    hDeleteFile( aFullPath( 1 ) )
    hDeleteFile( aFullPath( 2 ) )
    hDeleteFile( aFullPath( 3 ) )
    if ( dir( cBasePath & WORKDIR ) <> "" ) then
        rmdir( cBasePath & WORKDIR )
    endif
        
    '///+<li>Init: Create a directory called &quot;autocomplete&quot; in the user directory</li>
    mkdir( cBasePath & WORKDIR )
    if ( dir( cBasePath & WORKDIR ) = "" ) then
        warnlog( "Failed to create work directory, aborting" )
        goto endsub
    endif
    
    '///+<li>Init: Create three documents within the workdir - autocomplete_a.odt/_b.odt/_bb.odt</li>
    gApplication = "WRITER"
    for iCurrentDocument = 1 to 3
        call hNewDocument()
        kontext "DocumentWriter"
        DocumentWriter.typeKeys( aWorkFile( iCurrentDocument ) )
        hFileSaveAs( aFullPath( iCurrentDocument ) )
        FileClose
    next iCurrentDocument
    
    
    '///+<li>
    '///+<li>Open the File Open dialog</li>
    printlog( "open the File Open dialog" )
    FileOpen
    
    '///+<li>Test case 1: Workdirectory and folder autocompletion</li>
    '///<ul>
    '///+<li>Go to the work directory using the &quot;Home&quot; button</li>
    kontext "OeffnenDlg"
    Standard.click()
    
    '///+<li>Enter the first 4 characters of the testdirectory</li>
    cLeft     = left( WORKDIR, iLeft )
    cEFString = convertpath( WORKDIR & "/" )
    printlog( "Type """ & cLeft & """ into the filename entryfield" )
    DateiName.typeKeys( cLeft )
    
    '///+<li>Press the &quot;END&quot; key</li>
    printlog( "Press the <END> key and wait for two seconds" )
    DateiName.typeKeys( "<END>" )
    sleep( 1 ) ' Required, do not remove
    
    '///+<li>Verify that the &quot;autocomplete&quot; dir has been autocompleted</li>
    cAutocomplete = DateiName.getSelText()
    if ( cEFString = cAutocomplete ) then
        printlog( "Autocompletion succeeded for workdirectory" )
    else
        warnlog( "Autocompletion failed" )
        printlog( "Expected: " & cRight )
        printlog( "Found...: " & cAutocomplete )
    endif
    
    '///+<li>Press return on the FileOpen dialog</li>
    printlog( "Press <RETURN> on the file open dialog -> Open"
    kontext "OeffnenDlg"
    OeffnenDlg.typeKeys( "<RETURN>" )
    '///</ul>
    
    '///+<li>Test case 2: Filename autocompletion/selection</li>
    '///<ul>
    '///+<li>Enter the string &quot;auto&quot; into the entryfield</li>
    printlog( "Type "" auto "" into the entryfield" )
    cLeft = left( aWorkFile( 1 ), 4 )
    DateiName.typeKeys( cLeft )
    sleep( 1 )
    
    '///+<li>Verify that the filename is expanded to autocomplete_a.odt</li>
    cAutocomplete = DateiName.getSelText()
    if ( aWorkFile( 1 ) = cAutocomplete ) then
        printlog( "Autocompletion succeeded for first file" )
    else
        warnlog( "Autocompletion failed" )
        printlog( "Expected: " & cRight )
        printlog( "Found...: " & cAutocomplete )
    endif   

    '///+<li>Press the down-key in the entry field</li>
    printlog( "Press down key in entry field" )
    DateiName.typeKeys( "<DOWN>" ) 
    sleep( 1 )
    
    '///+<li>Verify that the filename is expanded to autocomplete_b.odt</li>
    cAutocomplete = DateiName.getSelText()
    if ( aWorkFile( 2 ) = cAutocomplete ) then
        printlog( "Autocompletion succeeded for second file" )
    else
        warnlog( "Autocompletion failed" )
        printlog( "Expected: " & cRight )
        printlog( "Found...: " & cAutocomplete )
    endif
    '///</ul>

    '///+<li>Test case 3: Wildcards ? and * (click &quot;Open&quot; to apply)</li>
    '///<ul>
    '///+<li>Enter autocomplete_?.odt -> 2 matches expected</li>
    printlog( "Enter autocomplete_?.odt -> 2 matches expected" )
    Dateiname.setText( aWildCard( 1 ) )
    Oeffnen.click()
    if ( DateiAuswahl.getItemCount() <> 2 ) then
        warnlog( "Incorrect number of files displayed, 2 expected" )
    else
        printlog( "Correct number of files listed in files-list" )
    endif
    
    '///+<li>Enter autocomplete_b?.odt -> 1 match expected</li>
    printlog( "Enter autocomplete_b?.odt -> 1 match expected" )
    Dateiname.setText( aWildCard( 2 ) )
    Oeffnen.click()
    if ( DateiAuswahl.getItemCount() <> 1 ) then
        warnlog( "Incorrect number of files displayed, 1 expected" )
    else
        printlog( "Correct number of files listed in files-list" )
    endif
    
    '///+<li>Enter autocomplete_*.odt -> 3 matches expected</li>
    printlog( "Enter autocomplete_*.odt -> 3 matches expected" )
    Dateiname.setText( aWildCard( 3 ) )
    Oeffnen.click()
    if ( DateiAuswahl.getItemCount() <> 3 ) then
        warnlog( "Incorrect number of files displayed, 3 expected" )
    else
        printlog( "Correct number of files listed in files-list" )
    endif
    '///</ul>
    
    
    '///+<li>Close File Open dialog</li>
    printlog( "Close File Open dialog" )
    kontext "OeffnenDlg"
    OeffnenDlg.cancel()
    
    '///+<li>Cleanup: Remove files, directories</li>
    hDeleteFile( aFullPath( 1 ) )
    hDeleteFile( aFullPath( 2 ) )
    hDeleteFile( aFullPath( 3 ) )
    rmdir( cBasePath & WORKDIR )
    '///</ul>

endcase

