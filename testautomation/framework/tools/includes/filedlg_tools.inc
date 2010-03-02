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
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Special tasks on filedialogs
'*
'\******************************************************************************

function hFileOpenMultiSelect( iArray() as integer ) as integer


    '///<h3>Multiselect files with the fileopen dialog</h3>
    '///<i>This function uses keyboard navigation to select a number of files
    '///+ (multiselection).</i><br>
    '///<u>Starting point</u>: FileOpen dialog has context, workdirectory is 
    '///+ open<br><br>

    '///<u>Input value(s):</u><br>
    '///<ol>
    '///+<li>Array (integer)</li>
    '///<ul>
    '///+<li>if ( array( n ) = 1 ) select the file at pos n, starting at n = 1</li>
    '///+<li>Any other value: Do not select, preferably preset with 0!</li>
    '///+<li>The size of the array must less or equal the number of files in the filepicker<br>
    '///+ Array( 0 ) is ignored</li>
    '///</ul>
    '///</ol>

    '///<u>Return Value:</u><br>
    '///<ol>
    '///+<li>Number of selected files (integer)</li>
    '///<ul>
    '///+<li>= 0: any error</li>
    '///+<li>&gt; 0: Number of selected files (Sum of Array( n ) = 1)</li>
    '///</ul>
    '///</ol>

    const CFN = "hFileOpenMultiSelect::"
    dim brc as boolean 'a multi purpose boolean returnvalue
    
    dim iArraySize as integer
    dim iCurrentFile as integer
    dim cCurrentFile as string
    dim iSelectedFilesCount as integer
        iSelectedFilesCount = 0

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Get the size of the array</li>
    iArraySize = ubound( iArray() )
    
    '///+<li>Get the number of items from the filepicker</li>
    kontext "OeffnenDlg"
    iFileCount = Dateiauswahl.getItemCount()
    
    '///+<li>Verify that the array size is equal or less the number of files<br>
    '///+ Exit with rc = 0 on error</li>
    ' Note: This can be done because it is quite simply expected that we know the
    '       number of files within the workdirectory. Take one input-dir.
    if ( iFileCount < iArraySize ) then
        qaerrorlog( CFN & "Array too large. Array must be <= file count" )
        printlog( CFN & "Files present in dialog: " & iFileCount )
        printlog( CFN & "Arraysize..............: " & iArraySize )
        hFileOpenMultiSelect() = 0
        exit function
    endif

    '///+<li>Select the first object in the filelist</li>
    kontext "OeffnenDlg"
    DateiAuswahl.typeKeys( "<HOME>" )
    
    '///+<li>Run through the filelist and select all items that are marked in the array</li>
    '///<ul>
    for iCurrentFile = 1 to iArraySize

        '///+<li>Select a file with CTRL+SPACE</li>
        if ( iArray( iCurrentFile ) = 1 ) then
            DateiAuswahl.typeKeys( "<MOD1 SPACE>" )
            cCurrentFile = DateiAuswahl.getSelText() ' does this work?
            printlog( CFN & "Select: " & cCurrentFile & " at pos: " & iCurrentFile )
            iSelectedFilesCount = iSelectedFilesCount + 1
        endif
    
        '///+<li>Move one down with CTRL key pressed</li>
        DateiAuswahl.typekeys( "<MOD1 DOWN>" )
        
    next iCurrentFile
    '///</ul>
    '///</ul>
    hFileOpenMultiSelect() = 0

end function
