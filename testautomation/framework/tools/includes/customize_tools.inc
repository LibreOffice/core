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
'/******************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Tools to ease the use of the ToolsCustomize-Dialog
'*
'\******************************************************************************

function hToolsCustomizeOpen() as boolean

    const CFN = "hToolsCustomizeOpen::"
    hToolsCustomizeOpen() = FALSE

    printlog( "Open Tools/Customize dialog" )

    ToolsCustomize
    hToolsCustomizeSelectTab( "Events" )
    
    Kontext TabCustomizeEvents
    if ( TabCustomizeEvents.exists( 2 ) ) then
        hToolsCustomizeOpen() = TRUE
    else
        warnlog( CFN & "Unable to open Tools/Customize dialog" )
    endif

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
    hToolsCustomizeClose() = FALSE

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
    if ( TabCustomizeEvents.notExists( 3 ) ) then
        hToolsCustomizeClose() = TRUE
    else
        warnlog( "Unable to close Tools/Customize dialog" )
    endif

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

