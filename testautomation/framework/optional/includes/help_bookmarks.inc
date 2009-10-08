'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: help_bookmarks.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:14 $
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
'*  short description : Bookmarks in the help browser
'*
'\******************************************************************************

testcase tHelpBookmarks

    '///<H1>Bookmarks in the help browser</H1>
    '///<h2>help_bookmarks.bas::tHelpBookmarks</h2>
    '///<ul>
    dim brc as boolean
    dim cStringFind as string
    dim cStringBookmark as string
    dim iItemCount as integer
    const MYBOOKMARK = "myBookMark"

    '///+<li>Open Help</li>
    brc = hOpenHelp()
    if ( not brc ) then
        warnlog( "Help not open, aborting test" )
        goto endsub
    endif

    '///+<li>Select &quot;Find&quot; tab</li>
    brc = hSelectHelpTab( "find" )

    '///+<li>Enter Search Term (Java)</li>
    SearchFind.setText( "java" )

    '///+<li>Search whole words and headers only</li>
    FindInHeadingsOnly.check()
    FindFullWords.check()
    FindButton.click()

    '///+<li>Select first match and display the item</li>
    Result.select( 1  )
    cStringFind = Result.getSelText()
    DisplayFind.click()

    '///+<li>Switch to the bookmarks tab</li>
    brc = hSelectHelpTab( "bookmarks" )

    '///+<li>Click the &quot;Add Bookmark&quot; button</li>
    printlog( " Adding bookmark" )
    SetBookmarks.click()

    '///+<li>Name the bookmark after checking its default name</li>
    Kontext "AddBookmark"

    printlog( " Verifying default name of bookmark" )
    cStringBookmark = BookmarkName.getText() 
    if ( cStringFind = cStringBookmark ) then
        printlog( " Bookmark has correct default name: " & cStringFind )
    else
        warnlog( "Incorrect default name for new bookmark" )
    endif

    printlog( " Accept default name, close dialog with ok" )
    AddBookmark.ok()

    '///+<li>Verify that the bookmark has been added to the list</li>
    hSelectHelpTab( "bookmarks" ) 
    printlog( " Verify that the bookmark has been added" )
    Kontext "BookmarksPage" 
    iItemCount = Bookmarks.getItemCount()
    if ( iItemCount = 1 ) then
        printlog( " The correct number of bookmarks is listed (1)" )
    else
        warnlog( "Incorrect number of bookmarks listed" )
    endif

    '///+<li>Abort the test if there is no bookmark listed at all</li>
    if ( iItemCount = 0 ) then
        warnlog( "Bookmark has not been created, aborting test" )
        call hCloseHelp()
        goto endsub
    endif

    '///+<li>Verify that the bookmark name makes it to the bookmarks list</li>
    Kontext "BookmarksPage"
    Bookmarks.select( 1 )
    cStringBookmark = Bookmarks.getSelText()
    if ( cStringBookmark = cStringFind ) then
        printlog( " Name in the bookmarks-list: " & cStringFind )
    else
        warnlog( "Incorrect name in bookmarks list: " & cStringBookmark )
    endif

    '///+<li>Close the help</li>
    brc = hCloseHelp()

    '///+<li>Reopen Help</li>
    brc = hOpenHelp()

    '///+<li>Verify that the bookmark is still present</li>
    brc = hSelectHelpTab( "bookmarks" )

    '///+<li>Open context menu, select &quot;Rename&quot;</li>
    printlog( " Open the context menu and rename the bookmark" )
    Kontext "BookmarksPage"
    Bookmarks.select( 1 )
    brc = hUseBookmarksContextMenu( "rename" ) 

    '///+<li>Change the name of the bookmark, accept with ok</li>
    Kontext "AddBookmark"
    if ( brc and AddBookmark.exists() ) then
	BookmarkName.setText( MYBOOKMARK ) 
        AddBookmark.ok() 
    else
        warnlog( "The AddBookmarks dialog is not open" )
    endif
    
    '///+<li>Verify that the changed name is listed in the listbox</li>
    hSelectHelpTab( "bookmarks" )
    kontext "BookmarksPage" 
    Bookmarks.select( 1 )
    cStringBookmark = Bookmarks.getSelText()
    if ( cStringBookmark = MYBOOKMARK ) then
        printlog( " The Bookmark has been renamed: " & cStringBookmark )
    else
        warnlog( "Incorrect name displayed in bookmarks list" )
    endif
 
    '///+<li>Close the Help, Close the Office</li>
    brc = hCloseHelp()

    '///+<li>Restart the office, open help, switch to Bookmarks tab</li>
    printlog( " Exit and restart the office" )
    call ExitRestartTheOffice()
    brc = hOpenHelp()
    brc = hSelectHelpTab( "bookmarks" )

    '///+<li>Verify that the renamed bookmark still exists</li>
    Kontext "BookmarksPage"
    Bookmarks.select( 1 )
    cStringBookmark = Bookmarks.getSelText()
    if ( cStringBookmark = MYBOOKMARK ) then
        printlog( " The bookmark has the correct name" )
    else
        warnlog( "Incorrect name displayed in bookmarks list" )
    endif

    '///+<li>Delete the Bookmark (using context menu)</li>    
    brc = hUseBookmarksContextMenu( "delete" )

    '///+<li>verify that the bookmark has been deleted (none left)</li>
    kontext "Bookmarks"
    if ( bookmarks.getItemCount = 0 ) then
        printlog( " Bookmark has been deleted" )
    else
        warnlog( "There are bookmarks left over, please check" )
    endif

    '///+<li>Close the help</li>
    brc = hClosehelp()

    '///+<li>Open help, switch to bookmarks tab</li>
    brc = hOpenhelp()
    brc = hSelectHelpTab( "bookmarks" )

    '///+<li>Verify that the bookmark has been deleted</li>
    kontext "BookmarksPage"
    iItemCount = Bookmarks.getItemCount()
    if ( iItemCount = 0 ) then
        printlog( " The bookmark has been deleted" )
    else
        warnlog( "Bookmarks have been left over, there should be none" )
    endif

    '///+<li>Close help</li>
    brc = hCloseHelp()
    
    '///</ul>

endcase

