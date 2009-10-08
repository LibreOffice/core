'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: pbrowser_tools.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:19:06 $
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
'*  short description : Tools for working with the property browser for formcontrols
'*
'\******************************************************************************

private const DEBUG_ENABLE = false

function hOpenPropertyBrowser() as boolean
    
    '///<h3>Function to open the properties of a selected control</h3>
    '///<i>The function verifies that the property browser is really open and
    '///+ ensures that we are on the General tabpage</i><br><br>
    
    '///<u>Input</u>:
    '///<ol>
    
    '///+<li>Nothing</li>
    
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE on successful open of the property browser</li>
    '///+<li>FALSE on any error</li>
    '///</ul>
    
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hOpenPropertyBrowser::"
    
    '///+<li>Open the property browser (call slot)</li>
    ContextProperties
    
    '///+<li>Verify that the property browser is open</li>
    kontext "ControlPropertiesTabControl"
    if ( ControlPropertiesTabControl.exists( 2 ) ) then
        
        '///+<li>Activate General-tabpage</li>
        ControlPropertiesTabControl.setPage( TabGeneralControl )
    
        '///+<li>Verify that the General-tabpage is visible</li>
        kontext "TabGeneralControl"
        if ( TabGeneralControl.isVisible() ) then
            printlog( CFN & "ok" )
            hOpenPropertyBrowser() = true
        else
            printlog( CFN & "General-tab is not visible." )
            hOpenPropertyBrowser() = false
        endif   
    else
        printlog( CFN & "Could not open property browser" )
        hOpenPropertyBrowser() = false    
    endif
    '///</ul>
    
end function

'*******************************************************************************

function hClosePropertyBrowser() as boolean
    
    '///<h3>A function that closes the Property-Browser</h3>
    '///<i>The property browser is closed by executing the slot (the slot 
    '///+ toggles the dialog).</i><br><br>
    
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE if the Property Browser has been closed</li>
    '///+<li>FALSE if the property browser is not open</li>
    '///+<li>FALSE if the property browser could not be closed</li>
    '///</ul>
    
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hClosePropertyBrowser::"

    '///+<li>Verify that the property browser is open</li>
    kontext "ControlPropertiesTabControl"
    if ( ControlPropertiesTabControl.exists() ) then
    
        '///+<li>Execute the ContextProperties slot</li>
        ContextProperties
        
        '///+<li>Verify that the property browser is closed</li>
        if ( ControlPropertiesTabControl.exists() ) then
            printlog( CFN & "Property browser could not be closed" )
            hClosePropertyBrowser() = false
        else
            printlog( CFN & "ok" )
            hClosePropertyBrowser() = true
        endif
    else
        printlog( CFN & "Property browser is not open" )
        hClosePropertyBrowser() = false
    endif
    '///</ul>
    
end function

'*******************************************************************************

function hPBSetControlName( cControl as string ) as boolean
    
    '///<h3>Name a control, with limited errorhandling</h3>
    
    '///<i>This function was introduced due to a problem with the property-
    '///browser not being open fast enough or just refusing to accept input</i><br><br>
    
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Text to be inserted in the control &quot;NameText&quot; (string)</li>
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    
    '///+<li>Errorcondition</li>
    '///<ul>
    '///+<li>TRUE: The control name was successfully updated</li>
    '///+<li>FALSE: Control is not visible within current context</li>
    '///</ul>
    
    '///</ol>
    
    const CFN = "hPBSetControlName::"
    
    '///<u>Description</u>:
    '///<ul>
    '///+<li>Test that the control &quot;NameText&quot; exists</li>
    kontext "TabGeneralControl"
    if ( NameText.exists() ) then
        '///+<li>Set the new name</li>
        WaitSlot()
        
        printlog( CFN & "Naming control: " & cControl )
        NameText.setText( cControl )
        TabGeneralControl.typeKeys( "<RETURN>" )
        WaitSlot()
        
        printlog( CFN & "Verifying rename..." )
        if ( NameText.getText() = cControl ) then
            printlog( CFN & "Name is set ok: " & cControl )
            hPBSetControlName() = true
            exit function
        endif
        
        ' If the name cannot be set this is in 99% of the cases a timing problem.
        ' Here is a (costly) workaround.
        qaerrorlog( CFN & "Name not set correctly, retrying" )
        Wait( 300 )
        NameText.setText( cControl )
        TabGeneralControl.typeKeys( "<RETURN>" )
        Wait( 300 )

        ' Test again, leave function if good
        if ( NameText.getText() = cControl ) then
            printlog( CFN & "Name is set ok: " & cControl )
            hPBSetControlName() = true
            exit function
        endif
        
        warnlog( CFN & "Unable to set control name: " & cControl )
        hPBSetControlName() = false
            
    else
        warnlog( "Unable to name the control." )
        hPBSetControlName() = false
    endif
    '///</ul>
    
end function

'*******************************************************************************

function hPBGetControlName( cControl as string ) as boolean
    
    '///<h3>Verify that the expected control is open</h3>
    '///<i>Use hPBSetControlName( string ) to set the name and when you reopen it
    '///+ verify that you got the right control wit this function</i><br><br>
    
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Name of the control (string)</li>
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE: The control has the correct name</li>
    '///+<li>FALSE: Any other condition</li>
    '///</ul>
    
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    
    dim cControlName as string
    const CFN = "hPBGetControlName::"
    
    '///+<li>If &quot;NameText&quot; exists, retrieve its text</li>
    kontext "TabGeneralControl"
    if ( TabGeneralControl.exists( 1 ) ) then
        if ( TabGeneralControl.isVisible() ) then
    
            cControlName = NameText.getText()
        
            '///+<li>Verify that the name is correct</li>
            if ( cControlName = cControl ) then
                printlog( CFN & "The name of the control is correct: " & cControl )
                hPBGetControlName() = true
            else
                warnlog( CFN & "Unexpected control name:" )
                printlog( CFN & "Found....: " & cControlName )
                printlog( CFN & "Expected.: " & cControl     )
                hPBGetControlName() = false
            endif
        else
            warnlog( CFN & "Dialog present but tabpage could not be accessed (TabGeneralControl)." )
            hPBGetControlName() = false
        endif
    else
        warnlog( CFN & "Unable to get the name from the control, dialog not accessible (TabGeneralControl)." )
        hPBGetControlName() = false
    endif
    '///</ul>
    
end function

'*******************************************************************************

function hCheckPropertyPresence ( cSetting as string , iPos as integer ) as boolean
    
    '///<h3>Function to determine whether a property is available for a control or not</h3>
    '///<i>This function takes a string (provided by controlcfg.dat) and looks for an 'x' at.
    '///+ a given position.  If it is found it returns TRUE, FALSE if it is a '-'<br>
    '///+Note that this function is a terrible workaround for a missing feature: In the current
    '///+ version of the Testtool we cannot ask which controls are present on a dialog. So this
    '///+ has to be kept in a list of some sort. This is especially bad for a property browser
    '///+ test as we need to maintain such a list for 21 controls and a total of 76 (IIRC)
    '///+ possible properties whereof only a small number (eight, i think) are common for all
    '///+ controls. The test is barely maintainable, issues have been written but there is
    '///+ no solution so far.</i><br><br>
    
    '///<u>Input</u>:
    '///<ol>
    
    '///+<li>Configuration string (string)</li>
    '///<ul>
    '///+<li>The string must be taken from file &quot;controlcfg.dat&quot;</li>
    '///</ul>
    
    '///+<li>Position of the control (integer)</li>
    '///<ul>
    '///+<li>&gt; 0 and &lt; 74 (all possible control config items)</li>
    '///</ul>
    
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Status (boolean)</li>
    '///<ul>
    '///+<li>TRUE: The property should exist for the current control</li>
    '///+<li>FALSE: The property is not expected to exist for this control</li>
    '///</ul>
    
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    
    ' this function looks at a given position in the string cSetting for either
    ' a "x" or a "-".
    ' if "x" is found, the function returns true, else false.
    
    
    '///+<li>Find the requested position in the string, set TRUE if it is an &quot;x&quot;</li>
    if ( mid( cSetting , iPos , 1 ) = "x" ) then
        hCheckPropertyPresence() = true
    else
        hCheckPropertyPresence() = false
    endif
    '///</ul>
    
end function

'*******************************************************************************

function hSetPBTabPage( iPage as integer ) as boolean
    
    '///<h3>A small helper to switch between tabpages in the property-browser</h3>
    '///<u>Input</u>:
    '///<ol>
    
    '///+<li>Page-ID (integer)</li>
    '///<ul>
    '///+<li>1 = General page</li>
    '///+<li>2 = Events page</li>
    '///</ul>
    
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE on success</li>
    '///+<li>FALSE on any error</li>
    '///</ul>
    
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hSetPBTabPage::"
    printlog( CFN & "Enter with option: " & iPage )
    
    '///+<li>Switch to the requested page</li>
    kontext "ControlPropertiesTabControl"
    if ( not ControlPropertiesTabControl.exists( 3 ) ) then
        printlog( CFN & "Exit: Control Properties Dialog is not open, aborting" )
        hSetPBTabPage() = false
        exit function
    else
        if ( DEBUG_ENABLE ) then
            printlog( CFN & "Control Properties Dialog is open" )
        endif
    endif
    
    select case iPage
    case 1 
    
        printlog( CFN & "Switching to control properties tabpage" )
        kontext "ControlPropertiesTabControl"
        ControlPropertiesTabControl.setPage TabGeneralControl
        
        kontext "TabGeneralControl"
        if ( nametext.exists( 5 ) ) then
            printlog( CFN & "Exit: Control properties are open (true)" )
            hSetPBTabPage() = true
            exit function
        else
            printlog( CFN & "Exit: Failed to open Control Properties (false)" )
            hSetPBTabPage() = false
            exit function
        endif
            
    case 2
    
        printlog( CFN & "Switching to event assignment tabpage" )
        kontext "ControlPropertiesTabControl"
        ControlPropertiesTabControl.setPage TabEventsControl
        
        kontext "TabEventsControl"
        if ( PBFocusGained.exists( 5 ) ) then
            printlog( CFN & "Exit: Events page is open (true)" )
            hSetPBTabPage() = true
            exit function
        else
            printlog( CFN & "Exit: Failed to open events-page (false)" )
            hSetPBTabPage() = false
            exit function
        endif
        
    case else
    
        printlog( CFN & "Invalid parameter passed to function: " & iPage )
        hSerPBTabPage() = false
        exit function
            
    end select
    '///</ul>
    
end function

'*******************************************************************************

function hSetLabelName( sLabelName as string ) as boolean
    
    '///<h3>Name a control, with limited errorhandling</h3>
    
    '///<i>This function was introduced due to a problem with the property-
    '///browser not being open fast enough or just refusing to accept input</i><br><br>
    
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Text to be inserted in the control &quot;NameText&quot; (string)</li>
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    
    '///+<li>Errorcondition</li>
    '///<ul>
    '///+<li>TRUE: The control name was successfully updated</li>
    '///+<li>FALSE: Control is not visible within current context</li>
    '///</ul>
    
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    '///+<li>Test that the control &quot;NameText&quot; exists</li>
    kontext "TabGeneralControl"
    if ( Label.exists() ) then
        '///+<li>Set the new name</li>
        Label.setText( sLabelName )
        TabGeneralControl.TypeKeys ("<RETURN>" , true)
        hSetLabelName() = true
    else
        warnlog( "Unable to name the control." )
        hSetLabelName() = false
    endif
    '///</ul>
    
end function

