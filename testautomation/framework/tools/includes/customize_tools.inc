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
'*  short description : Tools to ease the use of the ToolsCustomize-Dialog
'*
'\******************************************************************************

function hToolsCustomizeOpen() as boolean

    printlog( "Open Tools/Customize dialog" )
    ToolsCustomize
    hToolsCustomizeOpen() = hToolsCustomizeSelectTab( "Events" )
    
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
    hToolsCustomizeSelectTab() = true
    ctab = lcase( ctab )

    '///+<li>Switch to one of the four available tabpages</li>
    '///<ol>

    printlog( "Open Tools/Customize: <" & ctab & ">" )
    kontext
    
    try
        select case ctab
        '///+<li>Keyboard</li>
        case "keyboard"    : 
            active.setPage TabTastatur
            WaitSlot
            kontext "TabTastatur"
            if ( not Aendern.isVisible() ) then
                printlog( CFN & "Keyboard tabpage is not available" )
                hToolsCustomizeSelectTab() = false
            endif
        '///+<li>Menu</li>
        case "menu"        : 
            active.setPage TabCustomizeMenu
            WaitSlot
            kontext "TabCustomizeMenu"
            if ( not Entries.isVisible() ) then
                printlog( CFN & "Menu tabpage is not available" )
                hToolsCustomizeSelectTab() = false
            endif
        '///+<li>Toolbars</li>
        case "toolbars"    : 
            active.setPage TabCustomizeToolbars
            WaitSlot
            kontext "TabCustomizeToolbars"
            if ( not ToolbarContents.isVisible() ) then
                printlog( CFN & "Toolbar tabpage is not available" )
                hToolsCustomizeSelectTab() = false
            endif
        '///+<li>Events</li>
        case "events"      : 
            active.setPage TabCustomizeEvents
            WaitSlot
            kontext "TabCustomizeEvents"
            if ( not AssignMacro.isVisible() ) then
                printlog( CFN & "Assign Macro tabpage is not available" )
                hToolsCustomizeSelectTab() = false
            endif
        end select
        '///</ol>
    catch
        printlog( CFN & "Could not access requested tabpage" )
        hToolsCustomizeSelectTab() = false
    endcatch
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

