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
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Bookmarks in the help browser
'*
'\******************************************************************************

testcase tHelpBookmarks

    printlog( "Bookmarks in Help Browser")
    dim brc as boolean
    dim cStringFind as string
    dim cStringBookmark as string
    dim iItemCount as integer
    const MYBOOKMARK = "myBookMark"

    printlog( "Open Help Browser")
    brc = hOpenHelp()
    if ( not brc ) then
        warnlog( "Help not open, aborting test")
        goto endsub
    endif

    printlog( "Switch to Search Tab (Find)")
    brc = hSelectHelpTab( "find")

    printlog( "Enter <Java> as search term")
    SearchFind.setText( "java")

    printlog( "Search whole words and headers only")
    FindInHeadingsOnly.check()
    FindFullWords.check()
    FindButton.click()

    printlog( "Select first match and display the item")
    Result.select( 1  )
    cStringFind = Result.getSelText()
    DisplayFind.click()

    printlog( "Switch to the bookmarks tab")
    hSelectHelpTab( "bookmarks")

    printlog( "Click the <Add Bookmark> button")
    SetBookmarks.click()

    Kontext "AddBookmark"
    printlog( "Name the bookmark after checking its default name")
    cStringBookmark = BookmarkName.getText() 
    if ( cStringFind = cStringBookmark ) then
        printlog( "Bookmark has correct default name: " & cStringFind )
    else
        warnlog( "Incorrect default name for new bookmark")
    endif

    printlog( "Accept default name, close dialog with ok")
    AddBookmark.ok()

    printlog( "Verify that the bookmark has been added to the list")
    hSelectHelpTab( "bookmarks") 
    
    Kontext "BookmarksPage"
    printlog( "Verify that the bookmark has been added")
    iItemCount = Bookmarks.getItemCount()
    if ( iItemCount = 1 ) then
        printlog( "The correct number of bookmarks is listed (1)")
    else
        warnlog( "Incorrect number of bookmarks listed")
    endif

    printlog( "Abort the test if there is no bookmark listed at all")
    if ( iItemCount = 0 ) then
        warnlog( "Bookmark has not been created, aborting test")
        call hCloseHelp()
        goto endsub
    endif

    Kontext "BookmarksPage"
    printlog( "Verify that the bookmark name makes it to the bookmarks list")
    Bookmarks.select( 1 )
    cStringBookmark = Bookmarks.getSelText()
    if ( cStringBookmark = cStringFind ) then
        printlog( "Name in the bookmarks-list: " & cStringFind )
    else
        warnlog( "Incorrect name in bookmarks list: " & cStringBookmark )
    endif

    printlog( "Close the help")
    hCloseHelp()

    printlog( "Reopen Help")
    hOpenHelp()

    printlog( "Verify that the bookmark is still present")
    hSelectHelpTab( "bookmarks")

    printlog( "Open the context menu and rename the bookmark")
    Kontext "BookmarksPage"
    Bookmarks.select( 1 )
    brc = hUseBookmarksContextMenu( "rename") 

    printlog( "Change the name of the bookmark, accept with ok")
    Kontext "AddBookmark"
    if ( brc and AddBookmark.exists() ) then
	BookmarkName.setText( MYBOOKMARK ) 
        AddBookmark.ok() 
    else
        warnlog( "The AddBookmarks dialog is not open")
    endif
    
    printlog( "Verify that the changed name is listed in the listbox")
    hSelectHelpTab( "bookmarks")

    kontext "BookmarksPage"
    Bookmarks.select( 1 )
    cStringBookmark = Bookmarks.getSelText()
    if ( cStringBookmark = MYBOOKMARK ) then
        printlog( "The Bookmark has been renamed: " & cStringBookmark )
    else
        warnlog( "Incorrect name displayed in bookmarks list")
    endif
 
    printlog( "Close the Help, Close the Office")
    hCloseHelp()

    printlog( "Restart the office, open help, switch to Bookmarks tab")
    call ExitRestartTheOffice()
    hOpenHelp()
    hSelectHelpTab( "bookmarks")

    printlog( "Verify that the renamed bookmark still exists")
    Kontext "BookmarksPage"
    if ( Bookmarks.getItemCount() > 0 ) then
        Bookmarks.select( 1 )
        cStringBookmark = Bookmarks.getSelText()
        if ( cStringBookmark = MYBOOKMARK ) then
            printlog( "The bookmark has the correct name")
        else
            warnlog( "Incorrect name displayed in bookmarks list")
        endif

        printlog( "Delete the Bookmark (using context menu)")    
        hUseBookmarksContextMenu( "delete")

        printlog( "verify that the bookmark has been deleted (none left)")
        kontext "Bookmarks"
        if ( bookmarks.getItemCount = 0 ) then
            printlog( "Bookmark has been deleted")
        else
            warnlog( "There are bookmarks left over, please check")
        endif
    else
        warnlog( "Bookmarks are lost on program restart")
    endif

    printlog( "Close the help")
    hClosehelp()

    printlog( "Open help, switch to bookmarks tab")
    hOpenhelp()
    hSelectHelpTab( "bookmarks")

    printlog( "Verify that the bookmark has been deleted")
    kontext "BookmarksPage"
    iItemCount = Bookmarks.getItemCount()
    if ( iItemCount = 0 ) then
        printlog( "The bookmark has been deleted")
    else
        warnlog( "Bookmarks have been left over, there should be none")
    endif

    printlog( "Close help")
    hCloseHelp()
    
endcase

