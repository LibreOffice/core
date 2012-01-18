'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
'*
'*  short description : Access modules below OpenOffice.org macros
'*
'\******************************************************************************

testcase tBasicSharedModules

    
    dim iObjectPosition as integer
    dim iLibraryCount as integer

    const EXPECTED_LIBRARY_COUNT     = 14 ' prebundled extensions add a library
    const EXPECTED_LIBRARY_COUNT_OOO = 12
    
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

    if ( gOOo ) then
        if ( iLibraryCount <> EXPECTED_LIBRARY_COUNT_OOO ) then
            warnlog( "Incorrect itemcount in libraries list: " & iLibraryCount )
        endif
    else
        if ( iLibraryCount <> EXPECTED_LIBRARY_COUNT ) then
            warnlog( "Incorrect itemcount in libraries list: " & iLibraryCount )
        endif
    endif
    
    Kontext "MacroBar"
    Library.select( iLibraryCount )
    printlog( "Last entry = " & Library.getSelText() )   
    
    hCloseBasicIDE()
    
    hDestroyDocument()
    
    
endcase

