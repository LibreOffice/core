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
'* owner : gregor.hartmann@oracle.com
'*
'* short description : global update test (Standardbar)
'*
'\***************************************************************************

function hAccessStandardBar() as boolean

    '///<h3>Verify that the Standard Bar exists</h3>
    '///<i>This function works for all gApplications</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE if StandardBar exists</li>
    '///+<li>FALSE if standardBar does not exist</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    const CFN = "hAccessStandardBar::"
    dim brc as boolean
    
    printlog( CFN & "Enter" )

    '///+<li>Open a new document</li>
    hCreateDocument()

    '///+<li>Try to access the standardbar (query .exists())</li>
    kontext "StandardBar"
    if ( StandardBar.exists() ) then
        if ( standardbar.isVisible() ) then
            printlog( CFN & "StandardBar is visible. Good." )
            brc = true
        else
            warnlog( "The StandardBar is not visible in " & gApplication )
            brc = false
        endif
    else
        warnlog( "The StandardBar does not exist in " & gApplication )
        brc = false
    endif

    '///+<li>Close the navigator</li>
    kontext "Navigator"
    hCloseDialog( Navigator, "close,optional" )
    
    '///+<li>Close the document</li>
    hDestroyDocument()
    hAccessStandardBar() = brc
    printlog( CFN & "Exit" )
    '///</ul>
    
end function

'*******************************************************************************

function hResetStandardBar() as boolean

    '///<h3>Reset the Standard Bar to default configuration</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>Always TRUE as no errorconditions are handled yet</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hResetStandardBar::"
    const MIN_STATIC_DELAY = 1
    const DEFAULT_DELAY = 2
    const MENUITEM_RESTORE = 1
    const MENUITEM_CLOSEMENU = 0
    const TOOLBAR_MENUITEM_CUSTOMIZE = 2
    const EXPECTED_RESOURCE_ID = 304

    
    printlog( CFN & "Enter" )

    '///+<li>Create a new document</li>
    hInitSingleDoc()
    printlog " - Standardbar"

    kontext "standardbar"
    if ( Standardbar.exists() ) then
    
        '///+<li>Open the context menu on Standardbar</li>
        Standardbar.OpenContextmenu
        sleep( MIN_STATIC_DELAY )
        
        '///+<li>Click on 2. entry (Cutomize Toolbar)</li>
        hMenuselectNr ( TOOLBAR_MENUITEM_CUSTOMIZE )
        sleep( MIN_STATIC_DELAY )
        
        kontext
        Active.SetPage TabCustomizeToolbars
        sleep( MIN_STATIC_DELAY )

        '///+<li>Toggle to 'Toolbars' tab page.</li>
        kontext "TabCustomizeToolbars"
        if TabCustomizeToolbars.Exists( DEFAULT_DELAY ) then
            sleep( MIN_STATIC_DELAY )

            '///+<li>Press in first section '... Toolbars' the 'toolbar'-button</li>
            MenuBtn.Click
            sleep( MIN_STATIC_DELAY )
            
            '///+<li>A drop down menu will be opened.</li>
            '///+<li>Select the first enabled menu entry which should be 'Restore...'</li>
            hMenuSelectNr( MENUITEM_RESTORE )
            sleep( MIN_STATIC_DELAY )

            kontext
            if ( active.exists( DEFAULT_DELAY ) ) then
                if ( Active.GetRT = EXPECTED_RESOURCE_ID ) then
                
                    '///+<li>Press Yes button on verification dialog.</li>
                    Active.Yes()
                end if
            else
                warnlog( "No verification comes up if the RESET button has been pressed!" )
            end if

            '///+<li>Close 'Toolbars' tab page and the dialog with OK.</li>
            kontext "tabcustomizetoolbars"
            hCloseDialog( TabCustomizeToolbars, "ok" )
        else
            warnlog( "Cutomuze Toolbar not open (from context menu)" )
            'Closing the Context menu if the dialog does not come up.
            Menuselect( MENUITEM_CLOSEMENU )
        endif
            
    else
        warnlog( CFN & "Could not access Standardbar" )
    endif

    '///+<li>Close document.</li>
    hDestroyDocument()
    hResetStandardBar() = true
    printlog( CFN & "Exit" )
    '///</ul>
    
end function

'*******************************************************************************

function hStandardbarItemGetCount() as integer

    '///<h3>Update test: Get the number of items in the StandardBar</h3>
    '///<i>Starting point: Any plain document</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Number of items on the toolbar (integer)</li>
    '///<ul>
    '///+<li>No errorhandling is done. Beware.</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    dim iToolbarItemsCurrent as integer
    const DEFAULT_TOOLBAR_ITEMCOUNT = 30
    const WEBLOG_PUBLISHER = 2
    
    const CFN = "hStandardbarItemGetCount::"

    gApplication = "WRITER"
    printlog( CFN & "Enter" )
    
    '///+<li>Look for additional icons on the standardbar (through extensions)</li>
    printlog( CFN & "Count all items at the Standardbar" )

    '///+<li>Open the context menu on the Standardbar</li>
    Kontext "Standardbar"
    Standardbar.OpenContextmenu()

    '///+<li>Select the 1. entry</li>
    hMenuselectNr( 1 )

    '///+<li>Count all entries ( there must be 30 entries )</li>
    iToolbarItemsCurrent = 0
    iToolbarItemsCurrent = hMenuItemGetCount()
   
    ' StarOffice/StarSuite come with a preinstalled extension beginning with build
    ' 9181. This extension is not shipped with the Solaris patches as this would
    ' violate Sun's patching policy. So StarSuite/StarOffice now have 30 or 31
    ' items on the standardbar, OOo and its spinoffs have 30.
    
    select case ( iToolbarItemsCurrent )
    case DEFAULT_TOOLBAR_ITEMCOUNT
        printlog( CFN & "Default number of items found" )
    case DEFAULT_TOOLBAR_ITEMCOUNT + WEBLOG_PUBLISHER
        printlog( CFN & "Default icons + Weblog publisher icons found" )
    case else
        warnlog( CFN & "Number of icons is unexpected: " & iToolbarItemsCurrent )
    end select

    '///+<li>close the context menu</li>
    hMenuClose()
    hStandardbarItemGetCount() = iToolbarItemsCurrent
    printlog( CFN & "Exit" )
    '///</ul>
    
end function

'*******************************************************************************

function hToggleToolbarItem( iMenuPos as integer )

    '///<h3>Update test: Toggle an icon on the Standardbar (on/off)</h3>
    '///<i>Starting point: Any plain document</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Position of the item in the menu (integer)</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hToggleToolbarItem::"
    const INVISIBLE_ITEMS_MODIFIER = 3
    dim iItemsInMenu as integer
    
    printlog( CFN & "Enter with option (Menu position): " & iMenuPos )

    '///+<li>Open the context menu of the standard bar</li>
    Kontext "Standardbar"
    Standardbar.OpenContextmenu()
    
    '///+<li>Retrieve the number of items in the menu</li>
    iItemsInMenu = hMenuItemGetCount()
    
    ' in case the toolbar does not fit into the window we have more items
    ' (those that are invisible will be listed as context menu entries),
    ' the menuitem (invisible items) is placed at position nItems - 3
    '///+<li>Take the number of items -3, select the entry</li>
    hMenuselectNr( iItemsInMenu - INVISIBLE_ITEMS_MODIFIER )
    
    '///+<li>Select the provided menu position</li>
    hMenuselectNr( iMenuPos )        
    printlog( CFN & "Exit" )
    '///</ul>

end function

'*******************************************************************************

function hStandardbarLoadUrl() as boolean

    '///<h3>Update test: Access the url-entryfield</h3>
    '///<i>Starting point: Any plain document</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    

    ' this function tries to write to the url-EntryField. This should fail,
    ' because the item is not visible. Then the item should be enabled,
    ' written to and disabled again (no use of reset toolbar here, this is
    ' done in another testcase
    
    dim iItemMenuPos as integer : iItemMenuPos = 1
    
    const ITEM_MENU_POSITION = 1
    const CFN = "hStandardbarLoadUrl::"
    
    printlog( CFN & "Enter" )

    '///+<li>Write to URL-Filed (fails: Entryfiled is not visible)</li>
    try
        Kontext "Standardbar"
        URLEingabe.settext( "Should not work" )
        warnlog( CFN & "The Load URL EntryField is visible and active" )
    catch
        printlog( CFN & "The Load URL EntryField is not visible. Good." )
    endcatch
    
    '///+<li>add the control to the standardbar</li>
    printlog( CFN & "activate load URL" )
    hToggleToolbarItem( ITEM_MENU_POSITION )
    
    '///+<li>verify its existence by entering some text</li>
    Kontext "Standardbar"
    try
        URLEingabe.settext( "Should work" )
        printlog( CFN & "The Load URL REntryFiled is visible and active. Good." )
    catch
        warnlog( CFN & "Could not write to 'load url' control" )
    endcatch
        
    '///+<li>remove the control from the standardbar</li>
    printlog( CFN & "Deactivate load URL" )
    hToggleToolbarItem( ITEM_MENU_POSITION )
    Kontext "Standardbar"
    
    '///+<li>try to modify non existing object, failure expected</li>
    try
        Kontext "Standardbar"
        URLEingabe.settext( "Should not work" )
        warnlog( CFN & "The Load URL EntryField is visible and active." )
    catch 
        printlog( CFN & "The Load URL EntryField is not visible. Good." )
    endcatch
    printlog( CFN & "Exit" )
    '///</ul>
    
end function

'*******************************************************************************

function hStandardbarNewDialog()
    
    '///<h3>Update test: New from Template via StandardBar</h3>
    '///<i>Starting point: Plain document</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hStandardbarNewDialog::"
    const ITEM_MENU_POSITION = 3
    const TEMPLATE_DIALOG_MAX_RETRIES = 3
    const RC_TIMEOUT = -1

    dim iTry as integer

    printlog( CFN & "Enter" )

    '///+<li>Click on &quot;New from Template&quot;-button (fails, it is not visible)</li>
    try
        NeuDialog.Click()
        warnlog( CFN & "The 'New from Template'-button should not be visible." )
    catch
        printlog( CFN & "The control is not present or active." )
    endcatch
    
    '///+<li>add the control to the standardbar</li>
    printlog( CFN & "activate New from Template button" )
    hToggleToolbarItem( ITEM_MENU_POSITION )

    '///+<li>click the button</li>
    for iTry = 1 to TEMPLATE_DIALOG_MAX_RETRIES

        Kontext "Standardbar"
        hClickButton( NeuDialog )

        '///+<li>handle possible dialogs (there should never be one)</li>
        kontext "Active"
        if ( active.exists() ) then
            warnlog( CFN & "Unexpected active" )
            printlog( CFN & active.gettext() )
            if ( hCloseDialog( Active, "ok" ) = RC_TIMEOUT ) then
                warnlog( CFN & "Unknown dialog blocks test, now crashing" )
            endif
        endif

        '///+<li>close the templates and samples dialog</li>
        printlog( CFN & "Close templates and samples (cancel)" )
        Kontext "TemplateAndDocuments"
        if ( TemplateAndDocuments.Exists( 5 ) ) then
            hCloseDialog( TemplateAndDocuments, "cancel" )
            exit for
        else
            if ( iTry = TEMPLATE_DIALOG_MAX_RETRIES ) then
                warnlog( CFN & "The 'Template and Documents'-dialog was not activated" )
            endif
        endif

    next iTry


    '///+<li>finally remove the button from the toolbar</li>
    printlog( CFN & "Deactivate New from Template button" )
    hToggleToolbarItem( ITEM_MENU_POSITION )
    Kontext "Standardbar"
    '///</ul>
    
end function

'*******************************************************************************

function hStandardbarSaveAs()

    '///<h3>Update test: SaveAs button on StandardBar</h3>
    '///<i>Starting point: Any plain document</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>


    dim bBreakOnError as boolean

    const ITEM_MENU_POSITION = 6
    const CFN = "hStandardbarSaveAs::"

    printlog( CFN & "Enter" )

    '///+<li>Click on SaveAs (fails, item is not visible)</li>
    try
        SpeichernAls.Click()
    catch
        printlog( CFN & "The control is not present, good." )
    endcatch
    
    kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 1 ) ) then
        warnlog( "#i46363# (?)The 'SaveAs'-button should be invisible." )
        hCloseDialog( SpeichernDlg, "cancel" )
    else
        
        '///+<li>Add the control to the standardbar</li>
        printlog( CFN & "Activate 'Save As' button" )
        hToggleToolbarItem( ITEM_MENU_POSITION )
        
        '///+<li>Click the button</li>
        Kontext "Standardbar"
        hWaitForObject( SpeichernAls , 5000 )
        SpeichernAls.click()    
    
        '///+<li>Close the save-as dialog</li>
        printlog( CFN & "Close Save As (cancel)" )
        Kontext "SpeichernDlg"
        if ( SpeichernDlg.Exists( 2 ) ) then
            hCloseDialog( SpeichernDlg, "cancel" )
        else
            qaerrorlog( "Retrying" )
            Kontext "Standardbar"
            SpeichernAls.click()
            Kontext "SpeichernDlg"
            if ( SpeichernDlg.exists( 1 ) ) then
                printlog( "FileSaveAs dialog is open" )
                hCloseDialog( SpeichernDlg, "cancel" )
            else
                warnlog( CFN & "The 'Save As'-dialog was not activated" )
            endif
        endif
       
        '///+<li>Remove the button from the toolbar</li>
        printlog( CFN & "Deactivate 'Save as' button" )
        hToggleToolbarItem( ITEM_MENU_POSITION )
        
    endif
    
    Kontext "Standardbar"
    printlog( CFN & "Exit" )
    '///</ul>
    
end function




