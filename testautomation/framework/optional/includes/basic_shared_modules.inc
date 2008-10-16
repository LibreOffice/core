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

    '///<h3>Access modules below OpenOffice.org macros</h3>
    '///Covers issues #i74058, #i74120 and  #i74372
    
    dim iObjectPosition as integer
    dim iLibraryCount as integer

    const EXPECTED_LIBRARY_COUNT = 12
    
    '///<ul>
    '///+<li>Open a document to work on</li>
    hInitSingleDoc()
    
    '///+<li>Open the basic organizer</li>
    ToolsMacro_uno
    
    '///+<li>Select one of the internal macros</li>
    Kontext "Makro"
    iObjectPosition = hSelectNodeByName( MakroAus , "Depot" )
    if ( iObjectPosition = 0 ) then
        warnlog( "Could not find the specified node" )
        goto endsub
    endif
    
    '///+<li>Jump to the last node</li>
    hSelectTheLastNode( MakroAus )
    printlog( "Current node (Last node): " & MakroAus.getSelText() )
    
    '///+<li>Select the next node (should be one of the modules below Depot)</li>
    hSelectNode( MakroAus, iObjectPosition+1 )
    printlog( "Current node (Depot/..): " & MakroAus.getSelText() 
    
    '///+<li>Click &quot;Edit...&quot;</li>
    Kontext "Makro"
    Bearbeiten.click()
    
    '///+<li>Check that we are on the BASIC-IDE</li>
    Kontext "MacroBar"
    if ( MacroBar.exists( 2 ) ) then
        printlog( "Macro Bar is open." )
    else
        warnlog( "MacroBar is not open but we should be on the Basic IDE" )
    endif
    
    '///+<li>Use the Library listbox (Jump to top)</li>
    Kontext "MacroBar"
    Library.typeKeys( "<HOME>" )
    printlog( "Top entry = " & Library.getSelText() )
    
    '///+<li>Get the number of items from the Library control</li>
    iLibraryCount = Library.getItemCount()
    printlog( "There are " & iLibraryCount & " items in the list" )
    
    if ( iLibraryCount <> EXPECTED_LIBRARY_COUNT ) then
        warnlog( "The number of items in the library-list is incorrect (should be 12): " & iLibraryCount )
    endif
    
    '///+<li>Use the Library listbox (Jump to bottom)</li>
    Kontext "MacroBar"
    Library.select( iLibraryCount )
    printlog( "Last entry = " & Library.getSelText() )   
    
    '///+<li>Close the Basic-IDE</li>
    hCloseBasicIDE()
    
    '///+<li>Close the document</li>
    hDestroyDocument()
    '///</ul>
    
    
endcase

