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
'*  short description : Functions to manipulate controls by accessing them as objects
'*
'\******************************************************************************

private const MSG1 = "Incorrect default setting: "

'*******************************************************************************

function cb_test( oControl as object, def_state as string, issueid as string ) as boolean

    ' Small helper that prints a warnlog if a checkbox does not have the correct
    ' default setting (checked -> def_state = TRUE). You can supply an issue id
    ' which will be printed to the log. no parameter is optional, issueid can
    ' be an empty string.
    
    ' oControl: The checkbox itself
    ' def_state: Default state (checked or unchecked)
    ' issueid:  Issue id as string "#i123456#" for QUASTe

    dim con_state as boolean : con_state = oControl.isChecked()
    dim con_name  as string  : con_name  = oControl.name()
    
    printlog( con_name & " (Checkbox)" )
    cb_test() = true

    if ( def_state ) then
    
        if ( NOT con_state ) then
            warnlog( issueid & " " & MSG1 & con_name & " should be checked" )
            cb_test() = false
        endif
        
        oControl.unCheck()
        oControl.check()
        
    else
    
        if ( con_state ) then
            warnlog( issueid & " " & MSG1 & con_name & " should be unchecked" )
            cb_test() = false
        endif
        
        oControl.check()
        oControl.unCheck()
        
    endif
    
end function

'*******************************************************************************

function lb_test( oControl as object, items as integer, preset as integer, issueid as string ) as boolean

    ' Do some basic testing on listboxes
    ' - verify that the number of entries is correct
    ' - verify that the default selection is correct
    ' - select each item and restore default
    
    ' oControl: The Listbox as object
    ' items:    Number of items in the list
    ' preset:   Item which is selected by default (usually 1), skip with 0
    ' issueid:  Issue id as string "#i123456#" for QUASTe

    dim con_name as string    : con_name   = oControl.name()
    dim con_items as integer  : con_items  = oControl.getItemCount()
    dim con_preset as integer : con_preset = oControl.getSelIndex()
    dim con_iterator as integer
    
    printlog( con_name & " (Listbox)" )
    if ( oControl.isEnabled() ) then
        lb_test() = true
        
        if ( con_items <> items ) then
            warnlog( issueid & " Incorrect itemcount in listbox: " & con_name )
            printlog( "Expected: " & items )
            printlog( "Found...: " & con_items )
            lb_test() = false
        endif
        
        if ( preset > 0 ) then
            if ( con_preset <> preset ) then
                warnlog( issueid & " Incorrect default setting (index): " & con_name )
                printlog( "Expected: " & preset )
                printlog( "Found...: " & con_preset )
                lb_test() = false
            endif
        endif
        
        for con_iterator = 1 to con_items
            oControl.select( con_iterator )
        next con_iterator
        
        if ( preset > 0 ) then oControl.select( preset )
    else
        printlog( "The control is disabled" & con_name )
        lb_test() = false
    endif

end function

'*******************************************************************************

function checkRadioButton( _file as string , sKey as string , control as object ) as boolean
    
    '///<h3>EXPERIMENTAL: Get/Set function for some control types</h3>
    '///<i>This is a function that allows to change the state/content of a
    '///+ control via a configuration file. This function is experimental
    '///+ and should not be used.<br>
    
    ' a wrapper. The routine is identical to checkCheckBox.
    
    checkRadioButton = checkCheckBox( _file , sKey , control )
    
end function

'*******************************************************************************

function checkCheckBox( _file as string , sKey as string , control as object , optional cBugID as string ) as boolean
    
    '///<h3>EXPERIMENTAL: Get/Set function for some control types</h3>
    '///<i>This is a function that allows to change the state/content of a
    '///+ control via a configuration file. This function is experimental
    '///+ and should not be used.<br>
    
    dim bIsChecked as boolean
    dim sIsChecked as string
    dim _sIsChecked as string
    
    if ( ismissing( cBugID ) ) then
        cBugID = "WriteMe!"
    endif
    if ( sKey = "*" ) then
        sKey = control.name()
    endif
    
    sKey = lcase( sKey )
    _sIsChecked = lcase( hGetFileData( _file , sKey ) )
    
    if ( _sIsChecked <> "disabled" ) then
        
        ' look if the control is present on the current dialog
        if ( control.exists() and _
            control.IsVisible() and _
            control.IsEnabled() ) then
            
            ' find out whether it is checked or not, create strings for errormessage
            bIsChecked = control.isChecked()
            if ( bIsChecked = true ) then
                sIsChecked = "checked"
            else
                sIsChecked = "unchecked"
            endif
            
            ' compare
            keycompare( sIsChecked , _sIsChecked , sKey , cBugID )
            checkCheckBox() = control.isChecked()
            
        else
            
            warnlog( "Control <" & sKey & "> could not be accessed." )
            
        endif
        
    else
        
        if ( control.IsEnabled() ) then
            warnlog( "The control should be disabled but it is not: " & sKey )
        endif
        
    endif
    
end function

'*******************************************************************************

function setCheckBox( _file as string , sKey as string , control as object , optional bverbose as boolean ) as boolean
    
    '///<h3>EXPERIMENTAL: Get/Set function for some control types</h3>
    '///<i>This is a function that allows to change the state/content of a
    '///+ control via a configuration file. This function is experimental
    '///+ and should not be used.<br>
    
    dim sIsChecked as string
    dim _sIsChecked as string
    
    if ( sKey = "*" ) then
        sKey = control.name()
    endif
    if ( ismissing( bverbose ) ) then
        bverbose = true
    endif
    
    sKey = lcase( sKey )
    _sIsChecked = lcase( hGetFileData( _file , sKey ) )
    
    if ( _sIsChecked <> "disabled" ) then
        
        if ( control.exists() and _
            control.IsVisible() and _
            control.IsEnabled() ) then
            
            select case _sIsChecked
                
            case "checked"
                control.check()
                setCheckBox() = true
            case "unchecked"
                control.uncheck()
                setCheckBox() = false
            case else
                warnlog( "what?" )
                
            end select
            
            if ( bverbose ) then
                printlog( "     * " & sKey )
            endif
            
        else
            
            ' warn if the control could not be used (only works in rare cases)
            warnlog( "Control <" & sKey & "> is not accessible." )
            
        endif
        
    else
        
        if ( control.IsEnabled() ) then
            warnlog( "The control should be disabled but it is not: " & sKey )
        endif
        
    endif
    
end function

'*******************************************************************************

function checkComboBox( _file as string , sKey as string , control as object ) as boolean
    
    '///<h3>EXPERIMENTAL: Get/Set function for some control types</h3>
    '///<i>This is a function that allows to change the state/content of a
    '///+ control via a configuration file. This function is experimental
    '///+ and should not be used.<br>
    
    dim controltext as string  ' current state
    dim _controltext as string ' reference
    
    if ( sKey = "*" ) then
        sKey = control.name()
    endif
    
    sKey = lcase( sKey )
    
    if ( control.exists() = true and control.IsVisible = true ) then
        
        _controltext = lcase( hGetFileData( _file , skey ) )
        controltext = control.getSelText()
        keycompare( controltext , _controltext , sKey )
        checkComboBox() = control.isEnabled()
        
    else
        
        warnlog( "Control <" & sKey & "> could not be accessed." )
        
    endif
    
end function

'*******************************************************************************

function setComboBox( _file as string , sKey as string , control as object , optional bverbose as boolean  ) as integer
    
    '///<h3>EXPERIMENTAL: Get/Set function for some control types</h3>
    '///<i>This is a function that allows to change the state/content of a
    '///+ control via a configuration file. This function is experimental
    '///+ and should not be used.<br>
    
    dim _controltext as string ' reference string retrieved from _file
    
    if ( ismissing( bverbose ) ) then
        bverbose = true
    endif
    if ( sKey = "*" ) then
        sKey = control.name()
    endif
    
    sKey = lcase( sKey )
    
    if ( control.exists() = true and control.IsVisible = true ) then
        
        _controltext = hGetFileData( _file , sKey )
        control.settext( _controltext )
        setComboBox() = _controltext
        if ( bverbose = true ) then
            printlog( "     * " & sKey )
        endif
        
    else
        
        warnlog( "Control <" & sKey & "> is not accessible." )
        
    endif
    
end function

'*******************************************************************************

function  checkEntryField( _file as string , sKey as string , control as object , optional cBugID as string ) as boolean
    
    '///<h3>EXPERIMENTAL: Get/Set function for some control types</h3>
    '///<i>This is a function that allows to change the state/content of a
    '///+ control via a configuration file. This function is experimental
    '///+ and should not be used.<br>
    
    dim controltext as string  ' current state
    dim _controltext as string ' reference
    
    if ( ismissing( cBugID ) ) then
        cBugID = "WriteMe!"
    endif
    if ( sKey = "*" ) then
        sKey = control.name()
    endif
    
    ' lowercase the name (or alias) of the control which will be tested
    sKey = lcase( sKey )
    
    ' get the expected setting for the control from the reference file
    _controltext = lcase( hGetFileData( _file , skey ) )
    
    ' if the reference file specifies that the control should be disabled
    ' the routine tries to access it - expecting to fail. In this case we write
    ' a string into the EF
    if ( _controltext <> "disabled" ) then
        
        if ( control.exists()  and _
            control.IsVisible() and _
            control.IsEnabled() ) then
            
            controltext = lcase( control.getText() )
            keycompare( controltext , _controltext , sKey , cBugID )
            checkEntryField() = control.isEnabled()
            
        else
            
            warnlog( "Control <" & sKey & "> could not be accessed." )
            
        endif
        
    else
        
        if ( control.IsEnabled() ) then
            warnlog( "The control should be disabled but it is not: " & sKey )
        endif
        
    endif
    
end function

'*******************************************************************************

function  setEntryField( _file as string , sKey as string , control as object , optional bverbose as boolean ) as string
    
    '///<h3>EXPERIMENTAL: Get/Set function for some control types</h3>
    '///<i>This is a function that allows to change the state/content of a
    '///+ control via a configuration file. This function is experimental
    '///+ and should not be used.<br>
    
    ' this is the text that will be written into the entryfield
    dim _controltext as string
    
    if ( ismissing( bverbose ) ) then
        bverbose = true
    endif
    if ( sKey = "*" ) then
        sKey = lcase(  control.name() )
    endif
    
    ' get the string from the reference file
    _controltext = lcase( hGetFileData( _file , sKey ) )
    
    if ( _controltext <> "disabled" ) then
        
        ' we can only access the control if it is present + visible
        if ( control.exists() and _
            control.IsVisible() and _
            control.IsEnabled() ) then
            
            control.setText( lcase( _controltext ) )
            setEntryField() = _controltext
            
            if ( bverbose ) then
                printlog( "     * " & sKey )
            endif
            
        else
            
            warnlog( "Control <" & sKey & "> is not accessible." )
            
        endif
        
    else
        
        if ( control.IsEnabled() ) then
            warnlog( "The control should be disabled but it is not: " & sKey )
        endif
        
    endif
    
end function

'*******************************************************************************

function checkListBox( _file as string , sKey as string , control as object ) as boolean
    
    '///<h3>EXPERIMENTAL: Get/Set function for some control types</h3>
    '///<i>This is a function that allows to change the state/content of a
    '///+ control via a configuration file. This function is experimental
    '///+ and should not be used.<br>
    
    dim controltext as string  ' current state
    dim _controltext as string ' reference
    
    if ( sKey = "*" ) then
        sKey = control.name()
    endif
    
    sKey = lcase( sKey )
    
    if ( control.exists() = true and control.IsVisible = true ) then
        
        _controltext = lcase( hGetFileData( _file , skey ) )
        controltext = control.getSelIndex()
        keycompare( controltext , _controltext , sKey )
        checkListBox() = control.isEnabled()
        
    else
        
        warnlog( "Control <" & sKey & "> could not be accessed." )
        
    endif
    
end function

'*******************************************************************************

function setListBox( _file as string , sKey as string , control as object , optional bverbose as boolean  ) as integer
    
    '///<h3>EXPERIMENTAL: Get/Set function for some control types</h3>
    '///<i>This is a function that allows to change the state/content of a
    '///+ control via a configuration file. This function is experimental
    '///+ and should not be used.<br>
    
    dim _controltext as string ' reference
    
    ' if name of the control is '*' get the name from the control itself
    if ( ismissing( bverbose ) ) then
        bverbose = true
    endif
    if ( sKey = "*" ) then
        sKey = control.name()
    endif
    
    ' change the key to lowercase
    sKey = lcase( sKey )
    
    if ( control.exists() = true and control.IsVisible = true ) then
        
        _controltext = hGetFileData( _file , sKey )
        control.select( val( _controltext ) )
        setListBox() = control.getselindex()
        if ( bverbose = true ) then
            printlog( "     * " & sKey )
        endif
        
    else
        
        warnlog( "Control <" & sKey & "> is not accessible." )
        
    endif
    
end function

'*******************************************************************************

function checkListItem( _file as string , _iIndex as integer , sKey as string  , control as object ) as boolean
    
    '///<h3>EXPERIMENTAL: Get/Set function for some control types</h3>
    '///<i>This is a function that allows to change the state/content of a
    '///+ control via a configuration file. This function is experimental
    '///+ and should not be used.<br>
    
    dim controltext as string  ' current state
    dim _controltext as string ' reference
    
    sKey = lcase( sKey )
    
    
    if ( control.exists() = true and control.IsVisible() = true ) then
        
        control.select( _iIndex )
        controltext = control.getSelText()
        _controltext = hGetFileData( _file , skey )
        
        
        keycompare( controltext , _controltext , sKey )
        
        checkListItem() = control.isEnabled()
        
    else
        
        printlog( "Control <" & sKey & "> could not be found." )
        
    endif
    
    
end function

'*******************************************************************************

function hSetControlValue( oObject as object, cValue as string ) as integer
    
    '///<h3>Enter some text into a EntryField/TextField</h3>
    '///<i>This extends .setText() to provide a returnvalue</i><br>
    '///<i>The function runs silent as long as no errors occur</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Control Object (Object)</li>
    '///<ul>
    '///+<li>The object must exist</li>
    '///+<li>The object must be enabled</li>
    '///+<li>The object must be visible</li>
    '///</ul>
    '///+<li>String (string)</li>
    '///<ul>
    '///+<li>Any string, including empty strings (=delete)</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Status of the control (integer)</li>
    '///<ul>
    '///+<li>0 = EntryField was updated correctly</li>
    '///+<li>1 = Object does not exist</li>
    '///+<li>2 = Object it not visible</li>
    '///+<li>3 = Object is disabled</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    dim irc as integer
    const CFN = "hSetControlValue::"
    
    '///+<li>Verify that the control exists</li>
    if ( oObject.exists() ) then
        
        '///+<li>Verify that the object is visible</li>
        if ( oObject.isVisible() ) then
            
            '///+<li>Verify that the control is enabled</li>
            if ( oObject.isEnabled() ) then
                oObject.setText( cValue )
                irc = 0
            else
                irc = 3 ' control disabled
                printlog( CFN & "Control is disabled: ID: " & oObject )
            endif
        else
            irc = 2 ' control not visible
            printlog( CFN & "Control is not visible: ID: " & oObject )
        endif
    else
        irc = 1 ' control does not exist
        printlog( CFN & "Control does not exist: ID: " & oObject )
    endif
    
    hSetControlValue() = irc
    '///</ul>
    
end function


