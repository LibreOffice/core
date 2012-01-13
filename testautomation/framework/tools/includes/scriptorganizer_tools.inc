'encoding UTF-8  Do not remove or change this line!
'**************************************************************
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
'**************************************************************
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
