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

function hClosePropertyBrowser() as boolean

    const CFN = "framework::tools::includes::pbrowser_tools.inc::hClosePropertyBrowser(): "
    printlog( "Closing Property-Browser (if open)" )

    kontext "ControlPropertiesTabControl"
    if ( ControlPropertiesTabControl.exists() ) then

        hUseAsyncSlot( "ContextProperties" )

        if ( ControlPropertiesTabControl.notExists( 2 ) ) then
            hClosePropertyBrowser() = true
        else
            warnlog( CFN & "Failed to close dialog" )
            hClosePropertyBrowser() = false
        endif
    else
        printlog( "Property browser is not open, no action taken." )
        hClosePropertyBrowser() = true
    endif

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

    ' Note: A number of fallbacks are used here because the entryfield "NameText"
    ' is less than reliable. My hope is that one of the methods succeeds.
    kontext "TabGeneralControl"
    if ( NameText.exists() ) then
        '///+<li>Set the new name</li>
        hDeleteEntryFieldContent( NameText )

        ' Name the control and append some Spaces which should be truncated.
        printlog( CFN & "Naming control: " & cControl )
        NameText.setText( cControl )
        TabGeneralControl.typeKeys( "<RETURN>" )
        WaitSlot()

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
            wait( 300 )
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
        if ( GVERBOSE ) then
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

function hDeleteEntryFieldContent( oControl as object ) as boolean

    printlog( "Original name is: " & oControl.getText() )

    oControl.typeKeys( "<HOME>" )
    oControl.typeKeys( "<SHIFT END>" )
    oControl.typeKeys( "<DELETE>" )

    if ( oControl.getText() = "" ) then
        hDeleteEntryFieldContent() = true
    else
        hDeleteEntryFieldContent() = false
    endif

end function
