'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: scriptorganizer_tools.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: jsk $ $Date: 2008-06-20 08:03:54 $
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
'*  owner : joerg.skottke@sun.com
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
    dim brc as boolean

    '///+<li>Verify the function parameter</li>
    if ( cName = "" ) then
        warnlog( CFN & "Empty Name for scripting object passed to function" )
        hCreateScriptingObject() = false
        exit function
    endif
    
    '///+<li>Verify that the &quot;Create...&quot; button exists</li>
    Kontext "ScriptOrganizer"
    if ( not PBCreate.exists( 2 ) ) then
        warnlog( CFN & "Create-button does not exist" )
        hCreateScriptingObject() = false
        exit function        
    endif    
    
    '///+<li>Verify that the &quot;Create...&quot; button is enabled</li>
    if ( PBCreate.isEnabled() ) then
             
        '///+<li>Click &quot;Create...&quot; to open the naming dialog</li> 
        PBCreate.click()
        
        '///+<li>Name the new script</li>
        Kontext "ScriptNameDlg"
        if ( ScriptNameDlg.exists( 2 ) ) then        
            
            EFObjectName.setText( cName )
            ScriptNameDlg.OK()  
            brc = true
            
        endif

    else
        printlog( CFN & "Button is disabled" )
        brc = false
    endif
    
    '///+<li>Verify that we are back on the script organizer</li>
    kontext "ScriptOrganizer"
    if ( not ScriptOrganizer.exists( 2 ) ) then
        warnlog( CFN & "Could not return to ScriptOrganizer" )
        brc = false
    endif    
    
   hCreateScriptingObject() = brc
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

    use "global\tools\includes\optional\t_stringtools.inc"    

    const CFN = "hRenameScriptingObject::"
    dim cMessage as string
    dim brc as boolean
    
    brc = true
    
    '///+<li>Verify the function parameter</li>
    if ( cName = "" ) then
        warnlog( CFN & "Empty Name for scripting object passed to function" )
        hRenameScriptingObject() = false
        exit function
    endif    
    
    '///+<li>Verify that the &quot;Rename...&quot; button exists</li>
    Kontext "ScriptOrganizer"
    if ( not PBRename.exists( 2 ) ) then
        warnlog( CFN & "Rename button does not exist, aborting" )
        hRenameScriptingObject() = false
        exit function
    endif
    
    '///+<li>Verify that the &quot;Rename...&quot; button is enabled</li>
    if ( PBRename.isEnabled() ) then
    
        '///+<li>Click &quot;Rename...&quot; to open the renaming dialog</li>
        PBRename.click()
        
        '///+<li>Name the new script</li>
        Kontext "ScriptNameDlg"
        if ( ScriptNameDlg.exists( 2 ) ) then
        
            EFObjectName.setText( cName )
            ScriptNameDlg.OK()
            
            '///+<li>Test for any messagebox, try to close it with OK</li>
            kontext "active"
            if ( active.exists( 1 ) ) then
                cMessage = hRemoveLineBreaks( active.getText )
                printlog( CFN & cMessage )
                brc = false
                active.OK()
            endif
            
        endif
        
    endif
    
    '///+<li>Verify that we are back on the script organizer</li>
    kontext "ScriptOrganizer"
    if ( not ScriptOrganizer.exists( 2 ) ) then
        warnlog( CFN & "Could not return to ScriptOrganizer" )
        brc = false
    endif
    
    hRenameScriptingObject() = brc
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

    use "global\tools\includes\optional\t_stringtools.inc"
    
    const CFN = "hDeleteScript::"
    dim iPos as integer
    dim cMessage as string
    
    '///+<li>Test for the possible conditions:</li>
    '///<ol>
    '///+<li>Delete is correctly enabled (success)</li>
    kontext "ScriptOrganizer"
    if ( PBDelete.isEnabled() and bSuccess ) then
    
        PBDelete.click()
        kontext "active"
        cMessage = active.getText()
        cMessage = hRemoveLineBreaks( cMessage )
        printlog( CFN & cMessage ) 
        active.yes()

        kontext "active"
        if ( active.exists( 1 ) ) then
            cMessage = active.getText()
            cMessage = hRemoveLineBreaks( cMessage )
            qaerrorlog( CFN & "Message: Failed to delete object: " & cName )
            printlog( CFN & cMessage )
            active.ok()
        endif
        hDeleteScript() = true
        
    '///+<li>Delete is disabled correctly (success)</li>
    elseif ( ( not PBDelete.isEnabled() ) and ( not bSuccess ) ) then
    
        printlog( CFN & "Delete-Button disabled for non-deletable object: " & cName )
        hDeleteScript() = true
        
    '///+<li>Delete is enabled but should not be (failure)</li>
    elseif ( PBDelete.isEnabled() and ( not bSuccess ) ) then
    
        PBDelete.click()
        
        kontext "active"
        if ( active.exists( 1 ) ) then
            cMessage = active.getText()
            cMessage = hRemoveLineBreaks( cMessage )
            printlog( CFN & "Delete-Button enabled for non deletable object" )
            printlog( CFN & cMessage )
            active.yes()
        endif
        
        kontext "active"
        if ( active.exists( 2 ) ) then
            cMessage = active.getText()
            cMessage = hRemoveLineBreaks( cMessage )
            printlog( CFN & "Message: Failed to delete object" )
            printlog( CFN & cMessage )
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
        printlog( CFN & "Dialog is open" )
    else
        hopenScriptOrganizer() = false
        warnlog( CFN & "Slot failed, dialog not open" )
    endif

    '///</ul>

end function
    
'*******************************************************************************

function hCloseScriptOrganizer() as boolean

    '///<h3>Close a Script Organizer (Cancel)</h3>
    '///<i>Starting point: Any open Script Organizer</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE if the Script Organizer was closed</li>
    '///+<li>FALSE if the Script Organizer is still open</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    

    const CFN = "hCloseScriptOrganizer::" 
    
    hCloseScriptOrganizer() = false
    
    '///+<li>Try to close the script </li>
    kontext "ScriptOrganizer"
    if ( ScriptOrganizer.exists( 5 ) ) then
        ScriptOrganizer.cancel()
        hCloseScriptOrganizer() = true
    endif
        
    '///</ul>
    
end function

'*******************************************************************************

function hOpenRunMacroDialog() as boolean

    '///<h3>Open the &quot;Run Macro&quot; dialog</h3>
    '///<i>Starting point: Any document</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition</li>
    '///<ul>
    '///+<li>TRUE if dialog is open</li>
    '///+<li>FALSE if the Script Selector does not exist</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    

    '///+<li>Execute the ToolsMacrosRunMacro slot</li>
    ToolsMacrosRunMacro

    '///+<li>Verify that the dialog is present</li>
    kontext "ScriptSelector"
    if ( ScriptSelector.exists( 2 ) ) then
        hOpenRunMacroDialog() = true
    else
        hOpenRunMacroDialog() = false
    endif
    '///</ul>

end function
