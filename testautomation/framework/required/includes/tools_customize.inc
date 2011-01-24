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
'*  short description : Update Test for the Tools Customize Dialog
'*
'\******************************************************************************

testcase tUpdtCustomize( cApp as string )

    printlog( "Tools/Customize dialog" )

    if ( hCreateDocument() ) then    
        if ( hToolsCustomizeOpen() ) then
            hUpdtToolsCustomizeKeyboard()
            hUpdtToolsCustomizeMenu()
            hUpdtToolsCustomizeToolbars()
            hUpdtToolsCustomizeEvents()
            hCloseDialog( TabCustomizeEvents, "cancel" )
        else
            warnlog( "Tools/Customize dialog did not open" )
        endif
        hDestroyDocument
    else
        warnlog( "Failed to create initial document" )
    endif
    
endcase

'*******************************************************************************

function hUpdtToolsCustomizeMenu()

    const CFN = "hUpdtToolsCustomizeMenu::"

    const MENUITEM_MOVE              = 1
    const MENUITEM_ADDSUBMENU        = 1
    const MENUITEM_RENAME            = 2
    const MENUITEM_DELETE            = 3
    const MENUITEM_MODIFY            = 2
    const MENUITEM_MODIFY_LARGE_MENU = 4

    const APPLICATION_ONLY = 1
    const APPLICATION_AND_DOCUMENT    = 2
    const ITEMNAME = "tUpdtCustomize"

    dim brc as boolean

    dim iItems as integer
    dim iCurrentItem as integer
    dim iMenuListItems as integer
    dim iEntriesListItems as integer    
    
    printlog( "" )
    printlog( "Menu" )

    hToolsCustomizeSelectTab( "Menu" )
    call DialogTest( TabCustomizeMenu )
    
    printlog( CFN & "Click New..." )
    kontext "TabCustomizeMenu"
    BtnNew.click()
    
    Kontext "MenuOrganiser"
    if ( MenuOrganiser.exists( 2 ) ) then
        printlog( CFN & "MenuOrganiser" )
        call DialogTest( MenuOrganiser )

        printlog( CFN & "Down..." )
        ButtonDown.click()

        printlog( CFN & "Up..." )
        ButtonUp.click()

        printlog( CFN & "Name the new menu" )
        MenuName.setText( "tUpdtCustomize" )

        'qaerrorlog( "#i60609# Undeclared listbox in menu organiser" )
        printlog( "TODO: Access the listbox in menuorganizer" )

        printlog( CFN & "Close the menu with ok" )
        hCloseDialog( MenuOrganiser, "ok" )
    else
        warnlog( "Dialog <MenuOrganiser> did not open" )
    endif
    
    printlog( CFN & "Click the menu listbutton" )
    kontext "TabCustomizeMenu"
    hOpenMenuButton( MenuBtn ) ' MenuBtn.OpenMenu()
    
    printlog( CFN & "Select Move..." )
    hMenuSelectNr( MENUITEM_MOVE )
    
    kontext "MenuOrganiser"
    if ( MenuOrganiser.exists( 2 ) ) then
        printlog( CFN & "Click UP" )
        ButtonUp.click()

        kontext "MenuOrganiser"
        printlog( CFN & "Click DOWN" )
        ButtonDown.click()

        printlog( CFN & "Cancel the dialog" )
        hCloseDialog( MenuOrganiser, "cancel" )
    else
        warnlog( "Dialog <MenuOrganizer> did not open" )
    endif
    
    printlog( CFN & "Click the menu listbutton" )
    kontext "TabCustomizeMenu"
    hOpenMenuButton( MenuBtn ) ' MenuBtn.OpenMenu()
    
    printlog( CFN & "Select Rename..." )
    hMenuSelectNr( MENUITEM_RENAME )

    printlog( CFN & "Cancel the renaming-dialog" )
    kontext "RenameMenu"
    RenameMenu.cancel()
    
    printlog( CFN & "Click the menu listbutton" )
    kontext "TabCustomizeMenu"
    hOpenMenuButton( MenuBtn ) ' MenuBtn.OpenMenu()
    
    printlog( CFN & "Select Delete" )
    hMenuSelectNr( MENUITEM_DELETE )
    
    kontext "TabCustomizeMenu"
    hUpdtToolsCustomizeScriptSelector( 3 )
    
    printlog( CFN & "Check itemcount in SaveIn List" )
    kontext "TabCustomizeMenu"
    select case gApplication
    case "BACKGROUND"   :   brc = hToolsCustomizeTestSaveIn( APPLICATION_ONLY )
    case else           :   brc = hToolsCustomizeTestSaveIn( APPLICATION_AND_DOCUMENT )
    end select
    if ( not brc ) then warnlog( "Incorrect itemcount in listbox, see above" )
    
    printlog( CFN & "Down..." )
    BtnDown.click()
    
    printlog( CFN & "Up..." )
    BtnUp.click()
    
    printlog( CFN & "Click the Modify-Button and select item 1 (Add submenu)" )
    kontext "TabCustomizeMenu"

    if ( not hClickCommandButton( MENUITEM_ADDSUBMENU ) ) then
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    
    printlog( CFN & "Give the submenu a name, accept with OK" )
    kontext "CustomizeMenuName"
    call dialogtest( CustomizeMenuName )
    EingabeFeld.setText( ITEMNAME )
    CustomizeMenuName.ok()
    
    printlog( CFN & "Find the new submenu in the list and select it" )
    WaitSlot()
    kontext "TabCustomizeMenu"
    iEntriesListItems = Entries.getItemCount()
    for iCurrentItem = 1 to iEntriesListItems
    
        Entries.select( iCurrentItem )
        if ( Entries.getSelText() = ITEMNAME ) then
            exit for
        endif
        
    next iCurrentItem
    
    
    printlog( CFN & "Click the Modify-Button and select to rename the item" )
    kontext "TabCustomizeMenu"
    
    if ( not hClickCommandButton( 3 ) ) then
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    
    printlog( CFN & "Rename the item, accept with OK" )
    kontext "CustomizeMenuReName" 
    call dialogtest( CustomizeMenuReName )
    EingabeFeld.setText( ITEMNAME & "1" )
    CustomizeMenuReName.ok()
    
    printlog( CFN & "Verify that the name has been updated" )
    kontext "TabCustomizeMenu"
    if ( Entries.getSelText() <> ( ITEMNAME & "1" ) ) then
        warnlog( "Names do not match" )
    endif
    
    printlog( CFN & "Click the Modify-Button and delete the current item" )
    kontext "TabCustomizeMenu"

    if ( not hClickCommandButton( MENUITEM_MODIFY_LARGE_MENU ) ) then
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    
    printlog( CFN & "Click the Modify-Button and create a new group" )
    kontext "TabCustomizeMenu"

    if ( not hClickCommandButton( MENUITEM_MODIFY ) ) then
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    
    ' Note: The current index for the delete-function is at pos 2.
    printlog( CFN & "Click the Modify-Button and delete the new group" )
    kontext "TabCustomizeMenu"

    if ( not hClickCommandButton( MENUITEM_MODIFY ) ) then
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    
end function

'*******************************************************************************

function hUpdtToolsCustomizeToolbars()

    const CFN = "hUpdtToolsCustomizeToolbars::"

    const DOCUMENT_ONLY = 1
    const APPLICATION_AND_DOCUMENT = 2

    const MENUITEM_RENAME = 1
    const MENUITEM_DELETE = 2
    const MENUITEM_MODIFY = 4

    const DIALOG_DELAY = 1

    dim brc as boolean
    dim iCurrentItem as integer
        
    printlog( "" )
    printlog( "Toolbars" )

    brc = hToolsCustomizeSelectTab( "Toolbars" )
    call DialogTest( TabCustomizeToolbars )
    
    printlog( CFN & "Click New..." )
    kontext "TabCustomizeToolbars"
    BtnNew.click()
    
    kontext "NewToolbar"
    if ( NewToolbar.exists( DIALOG_DELAY ) ) then
        printlog( CFN & "NewToolbar" )
        call DialogTest( NewToolbar )
        
        select case gApplication
        case "BACKGROUND"   :   brc = hToolsCustomizeTestSaveIn( DOCUMENT_ONLY )
        case else           :   brc = hToolsCustomizeTestSaveIn( APPLICATION_AND_DOCUMENT )
        end select

        if ( not brc ) then warnlog( "Incorrect itemcount in listbox, see above" )
        
        printlog( CFN & "Name the new toolbar for further usage" )
        ToolbarName.setText( "tUpdtCustomize" )
        
        printlog( CFN & "Close NewToolbar dialog with OK" )
        hCloseDialog( NewToolbar, "ok" )
    else
        warnlog( CFN & "Could not access New Toolbar dialog" )
    endif
    
    printlog( CFN & "Rename the toolbar via Toolbar-Button" )
    kontext "TabCustomizeToolbars"
    hOpenMenuButton( MenuBtn ) ' MenuBtn.OpenMenu()
    
    printlog( "Select rename" )
    hMenuSelectNr( MENUITEM_RENAME )
    
    printlog( CFN & "Cancel the dialog" )
    kontext "RenameToolbar"
    if ( RenameToolbar.exists( DIALOG_DELAY ) ) then
        call dialogtest( RenameToolbar )
        hCloseDialog( RenameToolbar, "Cancel" )
    else
        warnlog( CFN & "Could not access Toolbar renaming dialog" )
    endif
    
    printlog( CFN & "Delete the toolbar via Toolbar-Button" )
    kontext "TabCustomizeToolbars"
    hOpenMenuButton( MenuBtn ) ' MenuBtn.OpenMenu()
    
    printlog( "Select delete" )
    hMenuSelectNr( MENUITEM_DELETE )
       
    kontext "TabCustomizeToolbars"
    hUpdtToolsCustomizeScriptSelector( 2 )
    
    kontext "TabCustomizeToolbars"
    select case gApplication
    case "BACKGROUND"   :   brc = hToolsCustomizeTestSaveIn( DOCUMENT_ONLY )
    case else           :   brc = hToolsCustomizeTestSaveIn( APPLICATION_AND_DOCUMENT )
    end select

    if ( not brc ) then warnlog( "Incorrect itemcount in listbox, see above" )
    
    printlog( CFN & "Down..." )
    if ( BtnDown.isEnabled() ) then
        BtnDown.click()
    else
        qaerrorlog( "BtnDown is not enabled" )
    endif
    
    printlog( CFN & "Up..." )
    if ( BtnUp.isEnabled() ) then
        BtnUp.click()
    else
        qaerrorlog( "BtnUp is not enabled" )
    endif

    kontext "TabCustomizeToolbars"
    printlog( CFN & "Add command" )
    AddCommands.click()
    
    kontext "ScriptSelector"
    if ( ScriptSelector.exists( DIALOG_DELAY ) ) then
        printlog( "ScriptSelector open." )
    else
        LibraryTreeList.typeKeys( "<HOME>" )
        for iCurrentItem = 1 to 5
            printlog( CFN & "LibraryTreeList: Moving down..." )
            LibraryTreeList.typeKeys( "<DOWN>" )
            if ( ScriptSelector.isEnabled() ) then exit for
        next iCurrentItem
    endif
    
    call dialogtest( ScriptSelector )
    
    printlog( CFN & "Close the Script Selector, back to ToolsCustomize dialog" )
    hCloseDialog( ScriptSelector, "cancel" )
    
    kontext "TabCustomizeToolbars"
    printlog( CFN & "Click the Modify-button" )

    if ( hClickCommandButton( 1 ) ) then

        printlog( CFN & "Command button clicked successfully" )
        kontext "CustomizeToolbarsRename"
        UseBindings
        wait( 100 )
        if ( CustomizeToolbarsRename.exists( DIALOG_DELAY ) ) then

            printlog( CFN & "Rename dialog is open" )
            call dialogtest( CustomizeToolbarsRename )
            EingabeFeld.setText( "Renamed item" )

            printlog( CFN & "Close the dialog" )
            hCloseDialog( CustomizeToolbarsRename, "ok" )
        else
            warnlog( CFN & "CustomizeToolbarsRename could not be accessed" )
        endif
    else
        warnlog( CFN & "Something went wrong when accessing the command button" )
    	exit function
    endif
    
    kontext "TabCustomizeToolbars"
    if ( hClickCommandButton( 2 ) ) then
        printlog( CFN & "Successfully clicked second item on the command button" )
    else
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    
    kontext "TabCustomizeToolbars"
    waitslot
    printlog( CFN & "Deselect separator (if this is the current object)" )
    hDeselectSeparator()

    kontext "TabCustomizeToolbars"
    if ( not hClickCommandButton( 3 ) ) then
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    
    waitslot
    kontext "TabCustomizeToolbars"
    printlog( CFN & "Click the Modify-button" )
    if ( hClickCommandButton( MENUITEM_MODIFY ) ) then
        printlog( CFN & "Successfully selected modify-item on the command button" )
    else
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    
    WaitSlot
    kontext "TabCustomizeToolbars"
    printlog( CFN & "Click the Modify-button" )
    if ( hClickCommandButton( 1 ) ) then
        printlog( CFN & "Successfully selected modify-item on the command button" )
    else
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    
    waitslot
    kontext "TabCustomizeToolbars"
    printlog( CFN & "Click the Modify-button" )
    hDeselectSeparator()

    if ( hClickCommandButton( 5 ) ) then
        printlog( CFN & "Successfully selected modify-item on the command button" )
    else
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif
    
    kontext "ChangeIcon"
    printlog( CFN & "On the Change Icon dialog: Click to import an icon" )
    if ( ChangeIcon.exists( DIALOG_DELAY ) ) then
        call dialogtest( ChangeIcon )
        import.click()
    
        printlog( CFN & "Cancel the FileOpen-dialog" )
        kontext "OeffnenDlg"
        call dialogtest( OeffnenDlg )
        hCloseDialog( OeffnenDlg, "cancel" )
        
        printlog( CFN & "Close the Change Icon dialog with <ok>" )
        kontext "ChangeIcon"
        hCloseDialog( ChangeIcon, "ok" )
    else
        warnlog( CFN & "Could not access Change Icon dialog" )
    endif
    
    waitslot
    kontext "TabCustomizeToolbars"
    printlog( CFN & "Click the Modify-button" )
    if ( hClickCommandButton( 3 ) ) then
        printlog( CFN & "Successfully selected modify-item on the command button" )
    else
        warnlog( CFN & "Something went wrong when accessing the command button" )
        exit function
    endif

end function


'*******************************************************************************

function hUpdtToolsCustomizeEvents()

    const CFN = "hUpdtToolsCustomizeEvents::"
    const APPLICATION_ONLY         = 1
    const APPLICATION_AND_DOCUMENT = 2
    const TAB_EVENTS               = 1

    dim brc as boolean

    printlog( "" )
    printlog( "Events" )
    
    hToolsCustomizeSelectTab( "events" )
    if ( TabCustomizeEvents.exists() ) then
        if ( TabCustomizeEvents.isVisible() ) then
            call DialogTest( TabCustomizeEvents )

            hUpdtToolsCustomizeScriptSelector( TAB_EVENTS )

            kontext "TabCustomizeEvents"
            select case gApplication
            case "BACKGROUND"   :   brc = hToolsCustomizeTestSaveIn( APPLICATION_ONLY )
            case else           :   brc = hToolsCustomizeTestSaveIn( APPLICATION_AND_DOCUMENT )
            end select

            if ( not brc ) then warnlog( "Incorrect itemcount in listbox, see above" )

        else
            warnlog( "<TabCustomizeEvents> is not visible" )
        endif
    else
        warnlog( "<TabCustomizeEvents> did not open" )
    endif

end function

'*******************************************************************************

function hUpdtToolsCustomizeKeyboard() as boolean

    const CFN = "hUpdtToolsCustomizeKeyboard::"
    const FIRST_CHANGEABLE_ITEM = 2
    
    printlog( "" )
    printlog( "Keyboard" )

    if ( gApplication = "BACKGROUND" ) then
        if ( not hToolsCustomizeSelectTab( "keyboard" ) ) then
            qaerrorlog( "#i61765# TabTastatur is missing when called from Backing Window" )
            hUpdtToolsCustomizeKeyboard() = false
            exit function
        endif
    endif

    hToolsCustomizeSelectTab( "keyboard" )

    kontext "TabTastatur"
    call DialogTest( TabTastatur )
  
    ' no verification of functionality, this is an update test!
    kontext "TabTastatur"
    StarOffice.check()
    hSelectNode( Tastatur , FIRST_CHANGEABLE_ITEM )
    hSelectTopNode( bereich )

    ' Workaround: It might still happen that the currently selected accelerator
    '             is "fixed" so we need to find another one that can be modified.
    do while ( not Aendern.isEnabled() ) 
        printlog( "Moving selection down by one, Control is not enabled" )
        Tastatur.TypeKeys( "<DOWN>" )
    loop 

    Aendern.click()
    Zuruecksetzen.click()
    hSelectNode( Tastatur , FIRST_CHANGEABLE_ITEM )

    ' Workaround: It might still happen that the currently selected accelerator
    '             is "fixed" so we need to find another one that can be modified.
    do while ( not Aendern.isEnabled() ) 
        printlog( "Moving selection down by one, Control is not enabled" )
        Tastatur.TypeKeys( "<DOWN>" )
    loop 

    Aendern.click()
    Loeschen.click()
    Speichern.click()
    
    kontext "SpeichernDlg"
    hCloseDialog( SpeichernDlg, "cancel" )
    
    kontext "TabTastatur"
    Laden.click()
    
    kontext "OeffnenDlg"
    hCloseDialog( OeffnenDlg, "cancel" )

end function

'*******************************************************************************

function hUpdtToolsCustomizeScriptSelector( iPageButton as integer ) as boolean
    
    const CFN = "hUpdtToolsCustomizeScriptSelector::"

    dim brc as boolean :  brc = true
    const TAB_EVENTS   = 1
    const TAB_TOOLBARS = 2
    const TAB_MENU     = 3
    const DIALOG_DELAY = 2

    select case iPageButton
    case TAB_EVENTS
        kontext "TabCustomizeEvents"
        printlog( CFN & "Assign Macro... (1)" )
        AssignMacro.click()
    case TAB_TOOLBARS
        kontext "TabCustomizeToolbars"
        printlog( CFN & "Add... (2)" )
        AddCommands.click()
    case TAB_MENU
        kontext "TabCustomizeMenu"
        printlog( CFN & "Add... (3)" )
        AddCommands.click()
    case else
        warnlog( CFN & "Invalid function parameter" )
        hUpdtToolsCustomizeScriptSelector() = false
        exit function
    end select
  
    kontext "ScriptSelector"
    if ( ScriptSelector.exists( DIALOG_DELAY ) ) then
        call DialogTest( ScriptSelector )

        kontext "ScriptSelector"
        hCloseDialog( ScriptSelector, "cancel" )
    else
         warnlog( CFN & "Dialog <ScriptSelector> did not open" )
    endif
   
    select case iPageButton
    case TAB_EVENTS   : kontext "TabCustomizeEvents"
    case TAB_TOOLBARS : kontext "TabCustomizeToolbars"
    case TAB_MENU     : kontext "TabCustomizeMenu"
    end select    
    
    hUpdtToolsCustomizeScriptSelector() = brc
    
end function

'*******************************************************************************

function hToolsCustomizeTestSaveIn( iItems as integer ) as boolean
    
    const CFN = "hToolsCustomizeTestSaveIn::"
    dim iPresentItems as integer

    iPresentItems = SaveIn.getItemCount()
    
    if ( iPresentItems <> iItems ) then
        printlog( CFN & "Incorrect itemcount in SaveIn: " )
        printlog( CFN & "Expected: " & iItems )
        printlog( CFN & "Found...: " & iPresentItems )
        hToolsCustomizeTestSaveIn() = false
    else
        printlog( CFN & "Correct itemcount in SaveIn: " & iItems )
        hToolsCustomizeTestSaveIn() = true
    endif

end function

'*******************************************************************************

function hOpenMenuButton( oControl as object ) as integer

    ' This function is very evil.
    ' It was written to hopefully workaround the infamous menubutton which is a button 
    ' that - when clicked - opens a menu. This feature is shaky and depends on 
    ' a lot of factors. There is a simple working implemantation in t_menu.inc 
    ' which has the drawback of being absolutely slow, costing the tools_customize
    ' test a lot of time waiting for the control (30% of testtime spent waiting).
    ' This approach tries to be dynamic: Machines that can do it the fast way
    ' use it automatically, machines that are too slow get two retries with the 
    ' slower approach.
    ' The menubutton is one of the last remaining places where none of the
    ' speed optmizations and enhancements apply, so neither WaitSlot() nor 
    ' synchronous slot execution help here.

    dim iClick as integer
    dim bUseSlowMethod as boolean : bUseSlowMethod = false
    
    const CFN = "hOpenMenuButton: "
    const RC_FAILURE = -1
    const MAX_RETRIES = 3
    const DEFAULT_DELAY = 1000
    
    hOpenMenuButton() = RC_FAILURE
    
    for iClick = 1 to MAX_RETRIES
    
        if ( bUseSlowMethod ) then 
            wait( DEFAULT_DELAY )
            oControl.OpenMenu()
            wait( DEFAULT_DELAY * 3 )
        else
            oControl.click()
        endif
            
        try
            hOpenMenuButton() = MenuGetItemCount
            printlog( CFN & "Success on " & iClick & ". attempt" )
            exit function
        catch
            qaerrorlog( CFN & "#i96753 - Failed to retrieve itemcount from Menu-/Command-button" )
            bUseSlowMethod = true
        endcatch
        
    next iClick
    
end function

'*******************************************************************************

function hClickCommandButton( iItemToClick as integer ) as boolean

    const CFN = "hClickCommandButton::"
    printlog( CFN & "Enter" )
    
    dim brc as boolean 'a multi purpose boolean returnvalue
    dim iMenuItems as integer

    iMenuItems = hOpenMenuButton( Command )
    
    ' exit on error
    if ( iMenuItems < 0 ) then
        warnlog( CFN & "Menu apparently not open, giving up." )
    	hClickCommandButton() = false
    	exit function
    endif        
    
    hMenuSelectNr( iItemToClick ) 
    wait( 500 )
    ' check for the rename dialog (menu)
    kontext "CustomizeMenuReName"
    if ( CustomizeMenuReName.exists() ) then
    	printlog( CFN & "Opened dialog: Rename Menu" )
    	hClickCommandButton() = true
    	exit function
    endif
    
    ' check for the rename dialog (toolbar)    
    kontext "CustomizeToolbarsRename"
    if ( CustomizeToolbarsRename.exists() ) then
    	printlog( CFN & "Opened dialog: Rename Toolbar" )
    	hClickCommandButton() = true
    	exit function
    endif

    printlog( CFN & "Exit" )
    hClickCommandButton() = true

end function
