'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: filedlg_cjk_files.inc,v $
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
'/************************************************************************
'*
'* owner : joerg.skottke@sun.com
'*
'* short description : check the internal file dialog ( extended tests )
'*
'\******************************************************************************

testcase tSaveLoadDelFilesCJK()

    if ( not hTestLocale() ) then
        warnlog( "Test requires UTF-8 locale" )
        goto endsub
    endif

    
    dim cStrangeName as string
    dim iCounter as integer ' iterator
    dim iRandom  as long    ' random number.(needs long for cjk chars)
    dim brc as boolean
    
    printlog( CHR$(13) )
    printlog( "Check if CJK-filenames are loaded/saved/deleted" )
    printlog( CHR$(13) )
    
    printlog( CHR$(13) + "Names with CJK-chars" )
    printlog( "" )
    
    ' Invoke randomizer
    call randomize()
    
    for iCounter = 1 to 2
        
        iRandom = int( 19968 + ( 20911 * rnd ) )
        printlog( "" )
        printlog( " * Using decimal char: " & iRandom )
        
        cStrangeName = hNameGen_append( iRandom )
        brc = hSaveLoadDelSuccess( cStrangeName )
        
        cStrangeName = hNameGen_lead( iRandom )
        brc = hSaveLoadDelSuccess( cStrangeName )
        
        kontext "active"
        if( active.exists() ) then
            qaerrorlog( "#i33964# Document does not exist when the last document is deleted from filepicker" )
            printlog( active.gettext() )
            active.ok()
        endif
        
        hDeleteFile( hGetWorkPath() & cStrangeName )        
        
        hCloseNavigator()
        
        
    next iCounter
    
    printlog( "Close the document" )
    brc = hDestroyDocument()
    
    
endcase

