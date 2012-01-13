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

*****

function hCreateModuleForDoc( optional cName as string ) as boolean

    '///<h3>Create a new Basic module for the current document</h3>
    '///<i>Starting point: Basic Macro Organizer is visible and has focus</i>
    const CFN = "hCreateModuleForDoc::"
    const DEFAULT_WAIT = 2
    const RETVAL_FAILURE = 0

    if ( isMissing( cName ) ) then cName = "TTModule"

    kontext "Makro"
    if ( Makro.exists() ) then
        if ( hSelectTheLastNode( MakroAus ) > RETVAL_FAILURE ) then

            kontext "Makro"
            if ( hClickButton( Neu ) ) then

                Kontext "NeuesModul"
                if ( NeuesModul.exists( DEFAULT_WAIT ) ) then
                    Modulname.SetText( cName )
                    hCloseDialog( NeuesModul, "ok" )

                    kontext "BasicIDE"
                    if ( BasicIde.exists( DEFAULT_WAIT ) ) then
                        hCreateModuleForDoc() = true
                    else
                        printlog( CFN & "BASIC IDE is not open" )
                        hCreateModuleForDoc() = false
                    endif
                else
                    warnlog( CFN & "Dialog <NeuesModul> did not open" )
                    hCreateModuleForDoc() = false
                endif
            else
                warnlog( CFN & "Failed to click <Neu> button" )
                hCreateModuleForDoc() = false
            endif
        else
            warnlog( CFN & "Failed to select the last node" )
            hCreateModuleForDoc() = false
        endif
    else
        warnlog( CFN & "Macro organizer is not open" )
        hCreateModuleForDoc() = false
    endif

end function

'*******************************************************************************

function hOpenBasicObjectOrganizer( iTabPage as integer ) as boolean

    '///<h3>Open the Basic Library/Module/Dialog Organizer</h3>
    '///<i>Starting point: Basic IDE is open and has focus</i>
    '///<ul>

    const CFN = "hOpenBasicObjectOrganizer::"

    ' This function opens a specified tab on the BASIC Organizer from the
    ' BASIC-IDE
    ' NOTE: This is the organizer for libraries, modules and dialogs,
    '       not the one to run or assign macros!
    '///+<li>Open the context menu of the tabbar</li>
    kontext "basicide"
    Tabbar.OpenContextMenu()
    
    '///+<li>Select the last entry of the context menu (dynamic menu!)</li>
    if ( hMenuItemGetCount() = 2 ) then
        printlog( " * short context menu (no edit-window)" )
        hMenuSelectNr( 2 )
    else
        printlog( " * long context menu (edit-window/dialog visible)" )
        hMenuSelectNr( 5 )
    endif
    
    WaitSlot()

    '///+<li>On the basic object organizer switch to the Modules tab</li>
    if ( hSelectBasicObjectOrganizerTab( 1 ) ) then
        printlog( CFN & "Requested page is open" )
        hOpenBasicObjectOrganizer() = true
    else
        warnlog( CFN & "Failed to open requested Tabpage" )
        hOpenBasicObjectOrganizer() = false
    endif
    '///+<li>Return TRUE on success, FALSE on failure</li>
    '///</ul>

    
end function

'*******************************************************************************

function hSelectBasicObjectOrganizerTab( iTabPage as integer ) as boolean

    const CFN = "hSelectBasicObjectOrganizerTab::"
    
    '///<h3>Switch between tab pages in the Basic Object Organizer</h3>
    '///<i>Starting point: Masic Object Organizer is visible and has focus</i>
    '///<ol>
    '///+<li>1 = Modules</li>
    '///+<li>2 = Dialogs</li>
    '///+<li>3 = Libraries</li>
    '///</ol>
    '///Description:
    '///<ul>
    
    '///+<li>Switch between tabpages 1, 2 or 3</li>
    select case iTabPage
    case 1 : 
        kontext
        Active.SetPage TabModule
        kontext "tabmodule"
        if ( TabModule.exists() ) then
            printlog( CFN & "Modules-Tab is open"
            hSelectBasicObjectOrganizerTab() = true
            exit function
        endif
        
    case 2 : 
        kontext
        Active.setPage TabDialogs
        kontext "tabdialogs"
        if ( TabDialogs.exists() ) then
            printlog( CFN & "Dialogs-Tab is open"
            hSelectBasicObjectOrganizerTab() = true
            exit function
        endif
    case 3 :
        kontext
        Active.setPage TabBibliotheken
        kontext "tabbibliotheken"
        if ( TabBibliotheken.exists() ) then
            printlog( CFN & "Libraries-Tab is open"
            hSelectBasicObjectOrganizerTab() = true
            exit function
        endif
    end select
    
    '///+<li>Return TRUE on success, FALSE on failure</li>
    '///</ul>
    hSelectBasicObjectOrganizerTab() = false
    
end function

'*******************************************************************************

function hDeleteLibrary( iLocation as integer, cLibName as string ) as boolean

    '///<h3>Delete a library by name via Macro Object Organizer</h3>
    '///<i>Starting point: Plain document</i>
    '///<ul>

    const CFN = "hDeleteLibrary::"
    const ITABPOS = 3

    dim iObjectCount as integer
    dim iCurrentObject as integer
    dim cCurrentObjectName as string

    '///+<li>Open Macro Organizer</li>
    ToolsMacro_uno
    
    '///+<li>Click Manage-button</li>
    Kontext "Makro"
    hClickButton( Verwalten )
    
    '///+<li>Go to the Libraries Tab on the Macro Object Organizer</li>
    hSelectBasicObjectOrganizerTab( ITABPOS )
    
    '///+<li>Select the root node in the libraries treelist</li>
    ' Needs to be specified to avoid touching the wrong library (e.g. a protected one)
    kontext "TabBibliotheken"
    Bibliothek.select( iLocation )
    iObjectCount = Bibliotheksliste.getItemCount()
    
    '///+<li>Find the requested item in the treelist</li>
    for iCurrentObject = 1 to iObjectCount
    
        Bibliotheksliste.select( iCurrentObject )
        cCurrentObjectName = Bibliotheksliste.getSelText()
        if ( cCurrentObjectName = cLibName ) then
        
            '///+<li>Click Delete</li>
            hClickButton( Loeschen )
            exit for
        endif
    
    next iCurrentObject
    
    '///+<li>Confirm to delete library</li>
    Kontext "Active"
    if ( Active.exists( 1 ) ) then
        active.yes()
    else
        warnlog( CFN & "No confirmation for delete" )
    endif
    
    '///+<li>Verify that there is one item less in the treelist (object has been deleted)</li>
    kontext "TabBibliotheken"
    if ( Bibliotheksliste.getItemCount() = ( iObjectCount - 1 ) ) then
        printlog( CFN & "Library has been deleted" )
        hDeleteLibrary() = true
    else    
        printlog( CFN & "Library has not been deleted" )
        hDeleteLibrary() = false
    endif
    
    '///+<li>Cancel Macro Object Organizer</li>
    hCloseDialog( TabBibliotheken, "cancel" )
    
    '///+<li>Cancel Basic Macro Organizer</li>
    Kontext "Makro"
    hCloseDialog( Makro, "cancel" )
    '///+<li>Return TRUE on success, FALSE on failure</li>
    '///</ul>
    
end function

'*******************************************************************************

function hNameBasicModule( cName as string ) as boolean

    '///<h3>Name a Basic Module</h3>
    '///<i>Starting point: Clicked &quot;New...&quot; button in Basic Macro organizer.<br>
    '///+ On success the kontext will be on the Basic IDE. In case of
    '///+ failure we remain on the naming dialog so that the function can be 
    '///+ triggered again.</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Name of the module (string)</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (Boolean)</li>
    '///<ul>
    '///+<li>TRUE if name is valid, module has been created</li>
    '///+<li>FALSE if name is invalid or naming dialog does not exist</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hNameBasicModule::"
    
    '///+<li>Verify that the naming dialog is open</li>
    kontext "NeuesModul"
    if ( not NeuesModul.exists() ) then
        hNameBasicModule() = false
        exit function
    endif
    
    '///+<li>Insert the name into the EntryField</li>
    Modulname.SetText( cName )
    
    '///+<li>Accept the name with OK</li>
    hCloseDialog( NeuesModul, "ok" )
    
    '///+<li>If there is no warning, the name should be valid.</li>
    kontext "BasicIDE"
    if ( BasicIde.exists( 2 ) ) then
        hNameBasicModule() = true
        printlog( CFN & "Name accepted, Basic-Ide is open" )
    else
        hNameBasicModule() = false
        '///+<li>Look for invalid name warning, close it</li>
        kontext "active"
        if ( active.exists() ) then
            printlog( CFN & "Msgbox: " & active.exists() )
            hCloseDialog( active, "ok" )
        endif
        
        '///+<li>If the name is invalid, get back to the naming-dialog</li>
        kontext "NeuesModul"
        if ( NeuesModul.exists() ) then
            printlog( CFN & "Name not accepted, focus on naming-dialog" )
        else
            warnlog( CFN & "Naming failed but we are not on the naming dialog" )
        endif
    endif
    '///</ul>
    
end function

        

