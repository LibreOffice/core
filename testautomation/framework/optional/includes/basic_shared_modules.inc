'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_shared_modules.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:13 $
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
'*  short description : Access modules below OpenOffice.org macros
'*
'\******************************************************************************

testcase tBasicSharedModules

    
    dim iObjectPosition as integer
    dim iLibraryCount as integer

    const EXPECTED_LIBRARY_COUNT = 12
    
    hInitSingleDoc()
    
    ToolsMacro_uno
    
    Kontext "Makro"
    iObjectPosition = hSelectNodeByName( MakroAus , "Depot" )
    if ( iObjectPosition = 0 ) then
        warnlog( "Could not find the specified node" )
        goto endsub
    endif
    
    hSelectTheLastNode( MakroAus )
    printlog( "Current node (Last node): " & MakroAus.getSelText() )
    
    hSelectNode( MakroAus, iObjectPosition+1 )
    printlog( "Current node (Depot/..): " & MakroAus.getSelText() 
    
    Kontext "Makro"
    Bearbeiten.click()
    
    Kontext "MacroBar"
    if ( MacroBar.exists( 2 ) ) then
        printlog( "Macro Bar is open." )
    else
        warnlog( "MacroBar is not open but we should be on the Basic IDE" )
    endif
    
    Kontext "MacroBar"
    Library.typeKeys( "<HOME>" )
    printlog( "Top entry = " & Library.getSelText() )
    
    iLibraryCount = Library.getItemCount()
    printlog( "There are " & iLibraryCount & " items in the list" )
    
    if ( iLibraryCount <> EXPECTED_LIBRARY_COUNT ) then
        warnlog( "The number of items in the library-list is incorrect (should be 12): " & iLibraryCount )
    endif
    
    Kontext "MacroBar"
    Library.select( iLibraryCount )
    printlog( "Last entry = " & Library.getSelText() )   
    
    hCloseBasicIDE()
    
    hDestroyDocument()
    
    
endcase

