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
'/************************************************************************
'*
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Test scripting-organizers / document-attached scripts
'*
'\******************************************************************************

function hCreateScriptingObject( cName as string ) as boolean

    '///<h3>Create a new scripting object for the current module</h3>
    '///<i>Starting point: Script organizer is open, module selected</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Name of the module (string)</li>
    '///<ul>
    '///+<li>Any name the organizer can accept</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition</li>
    '///<ul>
    '///+<li>TRUE on success</li>
    '///+<li>FALSE on failure or invalid user input</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    

    const CFN = "hNewScriptingObject::"

    '///+<li>Verify that the &quot;Create...&quot; button is enabled</li>
    if ( PBCreate.isEnabled() ) then
             
        '///+<li>Click &quot;Create...&quot; to open the naming dialog</li> 
        hClickButton( PBCreate )
        
        '///+<li>Name the new script</li>
        Kontext "ScriptNameDlg"
        if ( ScriptNameDlg.exists( 2 ) ) then        
            
            EFObjectName.setText( cName )
            ScriptNameDlg.OK()  
            hCreateScriptingObject() = true
            
        endif

    else
        printlog( CFN & "Button is disabled" )
        hCreateScriptingObject() = false
    endif
    
    '///+<li>Verify that we are back on the script organizer</li>
    kontext "ScriptOrganizer"
    if ( not ScriptOrganizer.exists( 2 ) ) then
        warnlog( CFN & "Could not return to ScriptOrganizer" )
        hCreateScriptingObject() = false
    endif    
    
   '///</ul>

end function

'*******************************************************************************

function hRenameScriptingObject( cName as string ) as boolean

    '///<h3>Rename the selected script</h3>
    '///<i>Starting point: Script organizer is open, a script is selected</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>New name of the script</li>
    '///<ul>
    '///+<li>Any name the script organizer does accept</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition</li>
    '///<ul>
    '///+<li>TRUE on success</li>
    '///+<li>FALSE on failure or invalid user input</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    const CFN = "hRenameScriptingObject::"
    hRenameScriptingObject() = true
    
    '///+<li>Click &quot;Rename...&quot; to open the renaming dialog</li>
    hClickButton( PBRename )

    '///+<li>Name the new script</li>
    Kontext "ScriptNameDlg"
    if ( ScriptNameDlg.exists( 2 ) ) then

        EFObjectName.setText( cName )
        hCloseDialog( ScriptNameDlg, "ok" )

        '///+<li>Test for any messagebox, try to close it with OK</li>
        kontext "active"
        if ( active.exists( 1 ) ) then
            printlog( CFN & active.getText() )
            hRenameScriptingObject() = false
            active.OK()
        endif

    endif
    
    '///+<li>Verify that we are back on the script organizer</li>
    kontext "ScriptOrganizer"
    if ( not ScriptOrganizer.exists( 2 ) ) then
        warnlog( CFN & "Could not return to ScriptOrganizer" )
        hRenameScriptingObject() = false
    endif
    '///</ul>
    
end function

'*******************************************************************************

function hDeleteScript( cName as string , bSuccess as boolean ) as boolean

    '///<h3>Delete the selected script</h3>
    '///<i>Starting point: Script organizer is open, a script is selected</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Name of the script (string)</li>
    '///<ul>
    '///+<li>For debugging purpose only, the name will be used for warnlogs</li>
    '///+<li>May be an empty string (not recommended)</li>
    '///</ul>
    '///+<li>Expected result (boolean)</li>
    '///<ul>
    '///+<li>TRUE: The script is expected to be deleted without warnings/errors</li>
    '///+<li>FALSE: The script should not be deletable</li>
    '///</ul>    
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean) - see description</li>
    '///<ul>
    '///+<li>TRUE on success (expected outcome)</li>
    '///+<li>FALSE on failure</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    const CFN = "hDeleteScript::"
    
    '///+<li>Test for the possible conditions:</li>
    '///<ol>
    '///+<li>Delete is correctly enabled (success)</li>
    kontext "ScriptOrganizer"
    if ( PBDelete.isEnabled() and bSuccess ) then
    
        hClickButton( PBDelete )
        kontext "active"
        if ( Active.exists( 1 ) ) then
            printlog( CFN & active.getText() )
            active.yes()
        endif

        kontext "active"
        if ( active.exists( 1 ) ) then
            qaerrorlog( CFN & "Message: Failed to delete object: " & cName )
            printlog( CFN & active.getText() )
            active.ok()
        endif
        hDeleteScript() = true
        
    '///+<li>Delete is disabled correctly (success)</li>
    elseif ( ( not PBDelete.isEnabled() ) and ( not bSuccess ) ) then
    
        printlog( CFN & "Delete-Button disabled for non-deletable object: " & cName )
        hDeleteScript() = true
        
    '///+<li>Delete is enabled but should not be (failure)</li>
    elseif ( PBDelete.isEnabled() and ( not bSuccess ) ) then

        hClickButton( PBDelete )
        
        kontext "active"
        if ( active.exists( 1 ) ) then
            printlog( CFN & "Delete-Button enabled for non deletable object" )
            printlog( CFN & active.getText() )
            active.yes()
        endif
        
        kontext "active"
        if ( active.exists( 2 ) ) then
            printlog( CFN & "Message: Failed to delete object" )
            printlog( CFN & active.getText() )
            active.ok()
        endif
        hDeleteScript() = false
        
    '///+<li>Delete is incorrectly disabled (failure)</li>        
    elseif ( ( not PBDelete.isEnabled() ) and bSuccess ) then
    
        printlog( CFN & "Delete-Button disabled for deletable object" )
        hDeleteScript() = false
        
    else
    
        warnlog( CFN & "Unknown condition in if-statement" )
        hDeleteScript() = false
        
    endif
    '///</ol>
    '///</ul>
   
end function                
            
'*******************************************************************************

function hOpenScriptOrganizer( iDialog as integer ) as boolean

    '///<h3>Open the ScriptOrganizers and verify that they are open</h3>
    '///<i>Starting point: Any document</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Dialog-ID (integer)</li>
    '///<ul>
    '///+<li>1 = BeanShell organizer</li>
    '///+<li>2 = JavaScript organizer</li>
    '///+<li>3 = Python script organizer</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition</li>
    '///<ul>
    '///+<li>TRUE if the expected organizer is open</li>
    '///+<li>FALSE if the expected organizer is not open</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hOpenScriptOrganizer::"

    '///<li>Call the associated slot</li>
    select case iDialog
    case 1 : ToolsMacrosOrganizeMacrosBeanShell
    case 2 : ToolsMacrosOrganizeMacrosJavaScript
    case 3 : ToolsMacrosOrganizeMacrosPython
    end select

    '///<li>Verify that the dialog is open, return TRUE if yes</li>
    kontext "ScriptOrganizer"
    if ( ScriptOrganizer.exists( 5 ) ) then
        hOpenScriptOrganizer() = true
        printlog( "Script Organizer is open" )
    else
        hOpenScriptOrganizer() = false
        warnlog( CFN & "Slot failed, dialog not open" )
    endif

    '///</ul>

end function
