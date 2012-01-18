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
'*  short description : Import BASIC library (flat)
'*
'\******************************************************************************

testcase tBasicLibraryImport

    printlog( "Import a BASIC library" )

    const MY_MACROS = 1
    const LIBRARIES_TAB = 3
    const STANDARD_LIBRARY = 1

    const XLB_DIALOG_FILE = "dialog.xlb"

    dim cMsg as string
    dim iCurrentLib as integer

    dim cPath as string : cPath = hGetWorkPath() & LIBRARY_NAME

    ToolsMacro_uno
    
    kontext "Makro"
    MakroAus.Select( MY_MACROS )
    
    Verwalten.click()
    
    hSelectBasicObjectOrganizerTab( LIBRARIES_TAB )
    
    printlog( "Select My macros" )
    kontext "TabBibliotheken"
    Bibliothek.select( STANDARD_LIBRARY )
    
    printlog( "Click to add a library" )
    Hinzufuegen.click()
    
    printlog( "Enter the name of the library: " & LIBRARY_NAME )
    kontext "OeffnenDlg"
    DateiName.setText( LIBRARY_NAME )
    
    printlog( "Step into the directory" )
    OeffnenDlg.typeKeys( "<RETURN>" )
    
    printlog( "Enter &quot;dialog.xlb&quot;" )
    DateiName.setText( XLB_DIALOG_FILE )
    
    printlog( "Open " & XLB_DIALOG_FILE )
    Oeffnen.click()
    
    kontext "active"
    if ( active.exists( 1 ) ) then
        if ( active.getRT() <> 373 ) then
            warnlog( "Autocompletion failed: " & Active.getText() )
            Active.ok()
            Kontext "OeffnenDlg"
            DateiName.setText( XLB_DIALOG_FILE )
            Oeffnen.click()
        endif
    endif
    
    printlog( "Confirm to append the library" )
    Kontext "AppendLibraries"
    AppendLibraries.ok()
    
    printlog( "Select the new library" )
    kontext "TabBibliotheken"
    for iCurrentLib = 1 to Bibliotheksliste.getItemCount()
        Bibliotheksliste.select( iCurrentLib )
        if ( Bibliotheksliste.getSelText = LIBRARY_NAME ) then
            exit for
        endif
    next iCurrentLib        
    
    printlog( "Delete the library, confirm with YES" )
    Kontext "TabBibliotheken"
    Loeschen.click()
    
    Kontext "Active"
    if ( Active.exists( 1 ) ) then
        active.yes()
    else 
        warnlog( "Deletion warning for libraries is missing" )
    endif
    
    printlog( "Close macro/library organizer" )
    kontext "TabBibliotheken"
    TabBibliotheken.cancel()
    
    printlog( "Close macro organizer" )
    kontext "Makro"
    Makro.cancel()
    
    hDestroyDocument()

    hDeleteFile( cPath & GetPathSeparator & XLB_DIALOG_FILE  )
    hDeleteFile( cPath & GetPathSeparator & "Module1.xba" )
    hDeleteFile( cPath & GetPathSeparator & "script.xlb"  )
    rmdir( cPath ) : printlog( "Remove directory" )
       
endcase

