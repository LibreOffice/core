'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: help_tools.inc,v $
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
'/******************************************************************************
'*
'* owner : joerg.skottke@sun.com
'*
'* short description : Replacements for routines in t_lists.inc adds some
'*
'\******************************************************************************

function hOpenHelp() as boolean

    '///<h3>Function to open the Help Browser</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE if the help is open</li>
    '///+<li>FALSE if the help did not open</li>
    '///</ul>
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
   
    const CFN = "hOpenHelp::"

    HelpContents
    
    if ( WaitSlot( 3000 ) <> WSFinished ) then
        printlog( CFN & "Failed to open help" )
        hOpenHelp() = FALSE
    else
        kontext "StarOfficeHelp"
        if ( StarOfficeHelp.exists() ) then
            printlog( CFN & "Help was opened" )
            hOpenHelp() = TRUE
        else
            printlog( CFN & "Help was not opened" )
            hOpenHelp() = FALSE
        endif
    endif
    
end function

'*******************************************************************************

function hCloseHelp() as boolean

    '///<h3>Function to close the Help Browser</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE if the help is closed</li>
    '///+<li>FALSE if the help did not close</li>
    '///</ul>
    '///</ol>
    
    const CFN = "hCloseHelp::"

    kontext "StarOfficeHelp"
    if ( StarOfficeHelp.exists() ) then
        ''StarOfficeHelp.typeKeys( "<MOD1 F4>" )
        hUseAsyncSlot( "FileClose")
        if ( StarOfficeHelp.notExists( 3 ) ) then
            printlog( CFN & "Help was closed")
            hCloseHelp() = TRUE
        else
            printlog( CFN & "Help was not closed")
            hCloseHelp() = FALSE
        endif
    else
        printlog( CFN & "Help is not open, no action taken")
        hCloseHelp() = TRUE
    endif

end function

'******************************************************************************

function hSelectHelpTab( cTab as string ) as boolean

    '///<h3>Select a tab on the help dialog by name</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>The tab to open (string)</li>
    '///<ul>
    '///+<li>&quot;content&quot; for the contents tab</li>
    '///+<li>&quot;index&quot; for the index tab</li>
    '///+<li>&quot;find&quot; for the find tab</li>
    '///+<li>&quot;bookmarks&quot; for the bookmarks tab</li>
    '///</ul>    
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE if the requested tabpage is open</li>
    '///+<li>FALSE if the requested tabpage did not open</li>
    '///</ul>
    '///</ol>
    
    '///<u>Description</u>:    
    '///<ul>
    const CFN = "hSelectHelpTab::"

    dim brc as boolean
        brc = false

    dim cTabName as string
        cTabName = lcase( cTab )

    kontext "StarOfficeHelp"
    
    printlog( CFN & "Enter" )

    '///+<li>Select the requested Tabpage, verify that it is open</li>
    select case cTabName
    case "content"       : TabControl.setPage( ContentPage )
                           if ( SearchContent.isVisible() ) then
                               brc = true
                           endif
    case "index"         : TabControl.setPage( IndexPage )
                           if ( SearchIndex.isVisible() ) then
                               brc = true
                           endif
    case "find"          : TabControl.setPage( FindPage )
                           if ( SearchFind.isVisible() ) then
                               brc = true
                           endif
    case "bookmarks"     : TabControl.setPage( BookmarksPage )
                           if ( Bookmarks.isVisible() ) then
                               brc = true
                           endif
    case else 
        printlog( CFN & "Invalid parameter passed to function: " & cTab )
        printlog( CFN & "Valid are: content, index, find, bookmarks" )
        brc = false
    end select

    '///+<li>Print a comment to the log</li>
    if ( brc ) then
        printlog( CFN & "Exit: Selected Tabpage: " & cTab )
    endif
 
    '///+<li>Return TRUE or FALSE</li>
    hSelectHelpTab() = brc    

    '///</ul>
end function

'*******************************************************************************

function hUseBookmarksContextMenu( cAction as string ) as boolean

    '///<h3>handle the bookmarks context menu in Help Browser</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>The context item to execute (string)</li>
    '///<ul>
    '///+<li>&quot;show&quot;</li>
    '///+<li>&quot;rename&quot;</li>
    '///+<li>&quot;delete&quot;</li>
    '///</ul>    
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE on success</li>
    '///+<li>FALSE on any other error</li>
    '///</ul>
    '///</ol>
    
    '///<u>Description</u>:        
    '///<ul>
    const CFN = "hUseBookmarksContextMenu::"
    dim cSelection as string
    dim iActionPos as integer
    
    printlog( CFN & "Enter" )

    '///+<li>Verify that we are on the Bookmarks Page</li>
    Kontext "BookmarksPage"
    if ( not BookmarksPage.exists() ) then
        printlog( CFN & "BookmarksPage is not open" )
        hUseBookmarksContextMenu() = false
        exit function
    endif

    '///+<li>Find the index of the requested entry</li>
    cSelection = lcase( cAction )
    select case cSelection
    case "show"       : iActionPos = 1
    case "rename"     : iActionPos = 2
    case "delete"     : iActionPos = 3
    case else         : iActionPos = 0
    end select

    '///+<li>If action is unsuported, return false and exit the function</li>
    if ( iActionPos = 0 ) then
        printlog( CFN & "Invalid action passed to function" )
        hUseBookmarksContextMenu() = false
        exit function
    endif
    
    '///+<li>Open the context menu and select the requested index</li>
    kontext "Bookmarks"
    Bookmarks.openContextMenu()
    printlog( CFN & "Exit: Opening item: " & MenuGetItemText( iActionPos ) )
    MenuSelect( iActionPos )

    '///+<li>Return TRUE if all went well so far</li>
    hUseBookmarksContextMenu() = true
   
    '///</ul>

end function

'*******************************************************************************

function hHelpGetAboutItemCount() as integer

    '///<h3>Get the number of applications listed in the Help ListBox</h3>
    
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Number of applications listed (integer)</li>
    '///<ul>
    '///+<li>The default is 7</li>
    '///</ul>
    '///</ol
    
    '///<u>Description</u>:
    '///<ul>
    
    const I_ABOUT_ITEMS = 8
    const CFN = "hHelpGetAboutItemCount::"
    
    dim iItem as integer
    dim cItemName as string
    dim iAboutItems as integer
    dim brc as boolean
    
    printlog( CFN & "Enter" )
    
    '///+<li>Open Help</li>
    brc = hOpenHelp()
    if ( not brc ) then
        qaerrorlog( CFN & "Help not open, aborting" )
        hHelpGetAboutItemCount() = 0
        exit function
    endif
    
    hSelectHelpTab( "index" )
    Kontext "IndexPage"
    
    '///+<li>get the number of applications from the drop down box</li>
    iAboutItems = HelpAbout.getItemCount()
    if ( iAboutItems <> I_ABOUT_ITEMS ) then
        warnlog( "The list of topics is incomplete:" )
    endif
    
    '///+<li>Print the list of items to the log</li>
    printlog( CFN & "Items listed in Application Listbox (HelpAbout)" )
    for iItem = 1 to iAboutItems
        cItemName = HelpAbout.getItemText( iItem )
        printlog( "     " & cItemName )
    next iItem
    
    '///+<li>close help</li>
    hCloseHelp()
    '///</ul>    
    
    printlog( CFN & "Exit with item count = " & iAboutItems )
    hHelpGetAboutItemCount() = iAboutItems
    
end function

'*******************************************************************************

function hHelpGetSearchIndexItemCount() as integer

    '///<h3>Get the number of items in the Help Search Index</h3>
    '///<i>Starting point: Help / Index page</i><br>
    '///<i>Note: Workaround for incorrect value returned by .getItemCount()</i><br>
    
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Number of items in the list (integer)</li>
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hHelpGetSearchIndexItemCount:"
    
    dim iItem as integer
    printlog( CFN & "Enter" )
    
    '///+<li>Run through the list and count the items, break on error</li>
    for iItem = 1 to 10000
    
        try
            SearchIndex.select( iItem )
        catch
            iItem = iItem - 1
            exit for
        endcatch
            
    next iItem

    printlog( CFN & "Exit with item count = " & iItem )    
    hHelpGetSearchIndexItemCount() = iItem
    '///</ul>
    
end function

'*******************************************************************************

function hGetHelpContentHeader( iLength as integer ) as string


    '///<h3>Get the header of the content in the help browser</h3>
    '///<i>This is the first line of the content page, truncated (if desired)</i><br><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>
    '///+<li>Length of the requested string (Integer)</li>
    '///<ul>
    '///+<li>The max length of the string to be returned</li>
    '///+<li>&le; 0 = do not truncate</li>
    '///</ul>
    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Header (String)</li>
    '///<ul>
    '///+<li>A string containing the content header</li>
    '///+<li>The header is the first line of the content page</li>
    '///</ul>
    '///</ol>

    const CFN = "hGetHelpContentHeader::"
    printlog( CFN & "Enter with option (iLength): " & iLength )
    dim cHeaderString as string

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Maximize the help viewer (required to avoid cut off strings)</li>
    kontext "HelpContent"
    'StarOfficeHelp.maximize()
    
    '///+<li>Go to the top left position in the help browser</li>
    HelpContent.typeKeys( "<MOD1 HOME>" )
    
    '///+<li>Mark the entire line - note that this ends at the linebreak</li>
    HelpContent.typeKeys( "<SHIFT END>" )
    
    '///+<li>Copy the string to the clipboard</li>
    EditCopy
    
    '///+<li>Assign the string to a variable</li>
    cHeaderString = getClipboardText
    
    '///+<li>Truncate string</li>
    if ( iLength > 0 ) then
        if ( len( cHeaderString ) > iLength ) then
            cHeaderString = left( cHeaderString, iLength )
        endif
    endif
    '///</ul>

    printlog( CFN & "Exit: " & cHeaderString )
    hGetHelpContentHeader() = cHeaderString

end function
