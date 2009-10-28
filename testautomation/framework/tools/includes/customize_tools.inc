'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: customize_tools.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:19:05 $
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
'*  short description : Tools to ease the use of the ToolsCustomize-Dialog
'*
'\******************************************************************************

function hToolsCustomizeOpen() as boolean

    '///<h3>Open Tools/Customize</h3>
    '///<i>Starting point: Any plain document</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorstatus (boolean)</li>
    '///<ul>
    '///+<li>TRUE if the Keyboard-Tab is open</li>
    '///+<li>FALSE on any other case</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hToolsCustomizeOpen::"

    '///+<li>Open Tools/Customize using the ToolsCustomize slot</li>
    ToolsCustomize

    '///+<li>Switch to the Events Tab</li>
    hToolsCustomizeSelectTab( "Events" )
    
    '///+<li>Verify that the requested tabpage is open</li>
    Kontext TabCustomizeEvents
    if ( TabCustomizeEvents.exists() ) then
    	hToolsCustomizeOpen() = true
    	printlog( CFN & "Successfully opened ToolsCustomize" )
    else
        hToolsCustomizeOpen() = false
        printlog( CFN & "Failed to open ToolsCustomize" )
    endif
    '///+<li>Return TRUE on success</li>
    '///</ul>

end function

'*******************************************************************************

function hToolsCustomizeSelectTab( cTab as string ) as boolean

    '///<h3>Switch Tabpages on ToolsCustomize Dialog</h3>
    '///<i>Starting point: The Tools/Customize dialog</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Name of the Tab (string)</li>
    '///<ul>
    '///+<li>&quot;Keyboard&quot;</li>
    '///+<li>&quot;Menu&quot;</li>
    '///+<li>&quot;Toolbars&quot;</li>
    '///+<li>&quot;Events&quot;</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorstatus (boolean)</li>
    '///<ul>
    '///+<li>TRUE if the requested tab is open</li>
    '///+<li>FALSE on any other condition</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hToolsCustomizeSelectTab::"
    dim brc as boolean
        brc = false

    ctab = lcase( ctab )

    '///+<li>Switch to one of the four available tabpages</li>
    '///<ol>
    kontext
    active.setPage

    kontext
    
    try

        select case ctab
        '///+<li>Keyboard</li>
        case "keyboard"    : active.setPage TabTastatur 
                             kontext "TabTastatur"
                             if ( Aendern.isVisible() ) then
                                 brc = true
                             endif
        '///+<li>Menu</li>
        case "menu"        : active.setPage TabCustomizeMenu 
                             kontext "TabCustomizeMenu"
                             if ( Entries.isVisible() ) then
                                 brc = true
                             endif
        '///+<li>Toolbars</li>
        case "toolbars"    : active.setPage TabCustomizeToolbars 
                             kontext "TabCustomizeToolbars"
                             if ( ToolbarContents.isVisible() ) then
                                 brc = true
                             endif
        '///+<li>Events</li>
        case "events"      : active.setPage TabCustomizeEvents 
                             kontext "TabCustomizeEvents"
                             if ( AssignMacro.isVisible() ) then
                                 brc = true
                             endif 
        end select
        '///</ol>
        
    catch
    
        printlog( CFN & "Could not access requested tabpage" )
        brc = false
        
    endcatch

    if ( brc ) then
        printlog( CFN & "Opened Tab: " & cTab )
    else
        printlog( CFN & "Failed to open Tab: " & cTab )
    endif
    
    '///+<li>Return TRUE on success</li>
    '///</ul>
    hToolsCustomizeSelectTab() = brc
    
end function

'*******************************************************************************

function hToolsCustomizeClose( iMode as integer  ) as boolean

    '///<h3>Close the ToolsCustomize-Dialog</h3>
    '///<i>Starting point: Tools/Customize dialog</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Closing mode (integer)</li>
    '///<ul>
    '///+<li>1: Use OK-button</li>
    '///+<li>2: Use Cancel-button</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorstatus (boolean)</li>
    '///<ul>
    '///+<li>TRUE if executing close action succeeded</li>
    '///+<li>FALSE on any other condition</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hToolsCustomizeClose::"
    dim brc as boolean : brc = true

    kontext 

    '///+<li>Switch to the Events page by default</li>
    active.setPage( TabCustomizeEvents )
    
    '///+<li>Close dialog by OK or CANCEL</li>
    '///<ol>
    select case iMode
    '///+<li>OK</li>
    case 1 : TabCustomizeEvents.OK()
    '///+<li>Cancel</li>
    case 2 : TabCustomizeEvents.Cancel()
    case else
        brc = false
    end select
    '///</ol>
    
    
    
    '///+<li>Verify that the dialog has indeed been closed</li>
    kontext "TabCustomizeEvents"
    TabCustomize.notExists( 3 )
    if ( TabCustomizeEvents.exists() ) then
        brc = false
    endif

    if ( brc ) then
        printlog( CFN & "Closed Tools/Customize" )
    else
        printlog( CFN & "Failed to close Tools/Customize" )
    endif

    '///</ul>
    hToolsCustomizeClose() = brc
end function

'*******************************************************************************

function hToolsCustomizeAddNewMenu( cName as string, bMode as boolean ) as boolean

    '///<h3>Add a new menu via Tools/Customize/Menu</h3>
    '///<i>Starting point: Tools/Customize with Menu-Tab open</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Name of the new menu (string)</li>
    '///+<li>Mode (boolean). Options:</li>
    '///<ul>
    '///+<li>TRUE = The entry will be created (OK)</li>
    '///+<li>FALSE = The entry will not be created (Cancel)</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorstatus (boolean)</li>
    '///<ul>
    '///+<li>TRUE on success</li>
    '///+<li>FALSE on failure</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hToolsCustomizeAddNewMenu::"
    dim brc as boolean
        brc = false
    
    '///+<li>Click the &quot;New...&quot; button</li>
    kontext "TabCustomizeMenu"
    BtnNew.click()
    
    '///+<li>Verify that the menu organizer exists</li>
    Kontext "MenuOrganiser"
    if ( not MenuName.exists() ) then
        printlog( CFN & "MenuOrganiser is not open" )
        exit function
    endif
    
    '///+<li>Name the new menu if we intend to create the new entry</li>
    if ( bMode ) then
        printlog( CFN & "Naming menu: " & cName )
        MenuName.setText( cName )
        MenuOrganiser.OK()
        brc = true
    else
        call DialogTest( MenuOrganiser )
        printlog( CFN & "Opened and closed MenuOrganiser" )
        MenuOrganiser.cancel()
        brc = true
    endif
    
    hToolsCustomizeAddNewMenu() = brc
    '///</ul>

end function

'*******************************************************************************

function hDeselectSeparator() as integer

    '///<h3>Make sure that we do not work on a separator item (Toolbars)</h3>
    '///<i>Starting point: Tools/Customize: Toolbars must be open</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Position of the selected item (integer)</li>
    '///<ul>
    '///+<li>Always &gt; 1</li>
    '///+<li>Always &lt; Max number of items</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    

    dim iCurrentItem as integer
    dim cString as string
    dim irc as integer
    const CFN = "hDeselectSeparator::"
    
    kontext "ToolsCustomizeToolbars"
    if ( ToolbarContents.getItemCount > 0 ) then
    
        iCurrentItem = ToolbarContents.getSelIndex()
        cString = ToolbarContents.getSelText()
        
        '///+<li>Check whether the selected item contains a number of minus-chars (---)</li>
        if ( instr( cString , "----" ) ) then
        
            '///+<li>If we are at the beginning of the list: Move up</li>
            if ( iCurrentItem = 1 ) then
                irc = 2
                ToolbarContents.select( irc )
                printlog( CFN & " Moved away from separator (up)" )
            else
            '///+<li>If we are somewhere else in the list, move down</li>
                irc = iCurrentItem - 1
                ToolbarContents.select( irc )
                printlog( CFN & " Moved away from separator (down)" )
            endif
            
        endif
    else
        irc = 0
    endif
    
    hDeselectSeparator() = irc
    '///</ul>
    
end function

'*******************************************************************************

