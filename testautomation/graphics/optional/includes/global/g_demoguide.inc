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
'/***********************************************************************
'*
'* owner : wolfram.garten@oracle.com
'*
'* short description : testcase to check the demo guide
'*
'************************************************************************
'*
' #1 t_Introduction         ' Description
' #1 t_Interoperability     ' Description
' #1 t_DrawingEngine        ' Description
'
'*
'\***********************************************************************
testcase t_Introduction

    gApplication   = "IMPRESS"
    
    printlog "Open new presentation"
    Call hNewDocument
    Call sSelectEmptyLayout
    
    printlog "Open the Gallery"
    ToolsGallery
    Sleep (1)
    Kontext "Gallery"
    if Gallery.NotExists(2) then
        ToolsGallery
    end if
    
    printlog "Choose an item from the gallery"
    Gallerys.Select(2)
    View.TypeKeys "<HOME><RIGHT><RIGHT>"
    View.TypeKeys "<SHIFT F10>"    'OpenContextMenu(true)
    sleep 2
    MenuSelect(MenuGetItemID(1))   'Insert
    sleep 2
    MenuSelect(MenuGetItemID(1))   'Copy
    sleep 2
    printlog "Check that we really got a copy of the object "
    kontext "DocumentImpress"
    DocumentImpress.OpenContextMenu(true)
    sleep 2
    MenuSelect 27353
    sleep 2
    Kontext "ExportierenDlg"
    if ExportierenDlg.IsVisible(5) then
            printlog "   Gallery-object correctly copied into Slide."
            ExportierenDlg.Close
        else
            warnlog "   Doesn't seem like we copied anything from the Gallery... ?"
    endif
    printlog "Delete the item"
    kontext "DocumentImpress"
    DocumentImpress.TypeKeys "<DELETE>"
    printlog "Close the Gallery"
    ToolsGallery
    printlog "Insert computergraphic from file"
    InsertGraphicsFromFile
    try
        Kontext "GrafikEinfuegenDlg"
        sleep 2
        Dateiname.settext Convertpath (gTesttoolPath + "graphics/required/input/screen_white.wmf")
        Oeffnen.click
    catch
        Warnlog "Insert graphic doesn't work"
    endcatch
    sleep 5

    printlog "Resize object"
    Kontext "DocumentImpress"
    gMouseClick (10, 10)
    sleep 1
    EditSelectAll
    sleep 1
    printlog "Format-position and size"
    ContextPositionAndSize                
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    KeepRatio.Check
    SizePosition.TypeKeys "<UP> <UP> <LEFT> <LEFT>"
    SizePosition.TypeKeys "<DOWN> <RIGHT>"
    Width.SetText CStr((CInt(Width.GetText)) * 2)
    TabPositionAndSize.OK
    
    printlog "Break object"
    Kontext "DocumentImpress"
    gMouseClick (10, 10)
    sleep 1
    EditSelectAll
    sleep 1
    hOpenContextMenu  '(when the object is marked)
    hMenuSelectNr(13) 
    printlog "Select part of object and make it black"
    DocumentImpress.TypeKeys "<SHIFT TAB>"
    FormatArea
    Kontext
    Active.SetPage TabArea
    Kontext "TabArea"
    FillOptions.Select 2
    ColourList.Select 1
    TabArea.OK
    
    printlog "Group the object"
    kontext "DocumentImpress"
    DocumentImpress.TypeKeys "<MOD1 A>"
    hOpenContextMenu  '(when the object is marked)
    hMenuSelectNr(12)

    Printlog "Add Layout"
    kontext "Tasks"
    if (NOT Tasks.exists) then
        warnlog "Tasks Panel not visible. Opening now."
        ViewTaskPane
    endif

    printlog "enable all items in the task pane"
    printlog "may be one item is not enabled due to a previous testrun"
    dim i as integer
    for i = 1 to 5
    	printlog i
        View.OpenMenu
        if ( MenuIsItemChecked(MenuGetItemID(i)) ) then
            printlog "ok"
        else
            MenuSelect(MenuGetItemId(4))
        endif
        hMenuSelectNr(0)
    next

    View.OpenMenu
    MenuSelect(MenuGetItemId(1))
    View.OpenMenu
    MenuSelect(MenuGetItemId(3))
    View.OpenMenu
    MenuSelect(MenuGetItemId(4))
    View.OpenMenu
    MenuSelect(MenuGetItemId(5))

    LayoutsPreview.TypeKeys "<HOME> <DOWN> <DOWN> <RETURN>"
    sleep (5)
    Printlog "Add background"

    Kontext "Tasks"
    printlog "Deselect all tabs from view menu in task pane except the needed."
    View.OpenMenu
    MenuSelect(MenuGetItemID(5))
    View.OpenMenu
    MenuSelect(MenuGetItemID(4))
    View.OpenMenu
    MenuSelect(MenuGetItemID(3))
    View.OpenMenu
    MenuSelect(MenuGetItemID(2))
    sleep(5)
    View.OpenMenu
    MenuSelect(MenuGetItemID(1))
    	Tasks.TypeKeys "<TAB>",true
	Tasks.TypeKeys "<MOD1 HOME>",true
    AvailableForUsePreview.TypeKeys "<HOME> <RIGHT> <RETURN>"
    sleep (5)
    Printlog "Add a transition effect"
    Kontext "Tasks"
    sleep(5)
    View.OpenMenu
    MenuSelect(MenuGetItemID(1))
    View.OpenMenu
    MenuSelect(MenuGetItemID(2))
    View.OpenMenu
    MenuSelect(MenuGetItemID(3))
    View.OpenMenu
    MenuSelect(MenuGetItemID(4))

    TransitionSound.GetItemCount  
    Sleep 1
    TransitionSound.Select 4

    Printlog "Close the document"
    Call hCloseDocument
    
endcase 't_Introduction

'--------------------------------------------------------------------

testcase t_Interoperability

    Dim sTemplatename as string

    gApplication   = "IMPRESS"
    
    printlog "Open PowerPoint presentation"
    hFileOpenLocally( gTesttoolPath + "graphics/required/input/dotNetOverview.ppt" )

    printlog "Save the name of the origin background (taken from last slide)"
        Kontext "DocumentImpress"
        DocumentImpress.TypeKeys "<END>"
        sleep (4)
        sTemplatename = DocumentImpress.StatusGetText (6)
        printlog "Background = " & sTemplatename     'Debugcode

    printlog "Choose a masterpage from right pane"
    
    Kontext "Tasks"
    printlog "Deselect all tabs accept the Master Pages"
    View.OpenMenu
    MenuSelect(MenuGetItemID(5)) 
    View.OpenMenu
    MenuSelect(MenuGetItemID(4))        
    View.OpenMenu
    MenuSelect(MenuGetItemID(3))
    View.OpenMenu
    MenuSelect(MenuGetItemID(2))
    printlog "needing a longer wait here for first start of the master page previews"
    wait(20000)
    Kontext "AvailableForUsePreview"
    AvailableForUsePreview.TypeKeys "<HOME> <RIGHT> <RETURN>"
    sleep (10)
    printlog "Verify that the background has changed"
    Kontext "DocumentImpress"
    printlog "Background is " & DocumentImpress.StatusGetText (6)     'Debugcode
    if DocumentImpress.StatusGetText (6) = sTemplatename then
            warnlog "The background is not changed"
        else printlog "    Background has been changed"
    endif
    
    printlog "Switch back to original background"
    Kontext "Tasks"
    Kontext "AvailableForUsePreview"
    AvailableForUsePreview.TypeKeys "<HOME> <RETURN>"
    sleep (10)
    Kontext "DocumentImpress"
    if DocumentImpress.StatusGetText (6) = sTemplatename then
            printlog "    The background has been set back to default"
        else warnlog "The background is " & DocumentImpress.StatusGetText (6) & " but should be: " & sTemplatename
    endif
    
    printlog "Resort slides"
    Kontext "DocumentImpress"
    Kontext "Slides"
    Kontext "SlidesControl"
    SlidesControl.OpenContextMenu
    sleep 1
    hMenuSelectNr (8)
    Kontext "DocumentImpress"
    DocumentImpress.TypeKeys "<HOME>", true
    Kontext "Slides"
    Kontext "SlidesControl"
    SlidesControl.OpenContextMenu
    sleep 1
    hMenuSelectNr (10)

     Printlog "    Verify that the slide was moved correctly"
     Kontext "Slides"
     sleep (5)
   
     SlidesControl.TypeKeys "<SHIFT F10>"
     if hMenuFindSelect(27268, true, 3) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
     endif
     kontext "NameDlgPage"
     if NameField.GetText <> ".NET Enterprise Federation" then
        Warnlog "Wrong slide found! Expected no 3, but found " + NameField.GetText + "."
     else
        Printlog "The slide was correctly moved"
     endif
     NameDlgPage.OK
     sleep 1
    
    printlog "Switch views"
    ViewWorkspaceOutlineView
    sleep (2)
    setclipboard("")
    try
        Kontext "DocumentImpressOutlineView"
        DocumentImpressOutlineView.TypeKeys "<Mod1 C>"
        sleep (5)
        if getclipboard() <> ".NET Enterprise Federation" then
                warnlog "Wrong selection in outlineview. Should be '.NET Enterprise', but is '" & getclipboard() +"'."
            else printlog "Switching to Outlineview was successfull"
        endif
    catch
        warnlog "Switching to Outlineview was not successfull"
    endcatch
    ViewWorkspaceNotesView
    sleep 1
    try
        Kontext "DocumentImpressNotesView"
        DocumentImpressNotesView.TypeKeys "<TAB><TAB>"
        DocumentImpressNotesView.OpenContextMenu(true)
        sleep 2
        MenuSelect(MenuGetItemID(3))   'Open Contextmenu Line
        Kontext "TabLinie"
        TabLinie.Cancel
        printlog "Switching to Notesview was successfull"
    catch
        warnlog "Switching to Notesview was not successfull"
    endcatch
    ViewWorkspaceHandoutView
    sleep 1
    try
        Kontext "DocumentImpressHandoutView"
        DocumentImpressHandoutView.TypeKeys "<TAB>"
        printlog "Switching to Handoutview was successfull"
    catch
        warnlog "Switching to Handoutview was not successfull"
    endcatch
    sleep 2
    ViewWorkspaceSlidesView
    sleep 2
    try
        Kontext "Slides"
        SlidesControl.TypeKeys "<TAB>"
        printlog "Switching to Slideview was successfull"
    catch
        warnlog "Switching to Slideview was not successfull"
    endcatch
    sleep 1
    ViewWorkspaceDrawingView
    sleep 1
    try
        Kontext "DocumentImpress"
        DocumentImpress.TypeKeys "<TAB>"
        printlog "Switching back to Drawingview was successfull"
    catch
        warnlog "Switching to Drawingview was not successfull"
    endcatch
    
    printlog "Travel through the presentation by left pane to slide 7"
    Kontext "DocumentImpress"
    DocumentImpress.TypeKeys "<HOME><PAGEDOWN><PAGEDOWN><PAGEDOWN><PAGEDOWN><PAGEDOWN><PAGEDOWN><PAGEDOWN>"
    Printlog "    Verify that the correct slide (7) is reached"
    Kontext "Slides"
    Kontext "SlidesControl"
    SlidesControl.OpenContextMenu
    if hMenuFindSelect(27268, true, 3) = false then
    Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
    Call hCloseDocument
    Goto Endsub
    endif
    kontext "NameDlgPage"
    if NameField.GetText <> "Visual Studio.NET The complete development environment  for building distributed applications  for Windows and the Web" then
            Warnlog "Wrong slide found! Expected no 7, but found " + NameField.GetText + "."
        else Printlog "    Slide 7 is reached"        
    endif
    NameDlgPage.OK
    sleep 1
    
    printlog "Autoshape support"
    Kontext "DocumentImpress"
    DocumentImpress.TypeKeys "<TAB>"
    try
        Kontext "ExtrusionObjectBar"
	printlog "Checking if ExtrusionObjectBar is already open."
        if ExtrusionObjectBar.Exists then
        	printlog "ExtrusionObjectBar Exists = " & ExtrusionObjectBar.Exists
        else
		printlog "ExtrusionObjectBar Exists = " & ExtrusionObjectBar.Exists
		printlog "Opening ExtrusionObjectBar."
        	ViewToolbarsThreeDSettings
        endif	
        ExtrusionTiltRight.Click
        printlog "    An autoshape was found and turned right"
    catch
        warnlog "Turning autoshape was not successfully"
    endcatch
    printlog "Close document"
    Call hCloseDocument
        
endcase 't_Interoperability
'--------------------------------------------------------------------
testcase t_DrawingEngine

    Dim sGroupPosition as string
    Dim sObjectPosition as string
    
    gApplication   = "DRAW"
    
    printlog "Open a drawing"
    hFileOpen (Convertpath (gTesttoolPath + "graphics/required/input/Blueprint.odg"))

    Call fMakeDocumentWritable

    printlog "Select group"
    Kontext "DocumentDraw"
    DocumentDraw.TypeKeys "<TAB><TAB>"
    sleep (5)
    printlog "Save position of group"
    ContextPositionAndSize                'Format-position and size
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    sGroupPosition = PositionY.GetText
    printlog "    - GroupPosition is " & sGroupPosition
    TabPositionAndSize.OK
    printlog "Enter group"
    Kontext "DocumentDraw"
    FormatEditGroupDraw
    sleep 1
    
    printlog "Select and move single object"
    printlog "    Select object"
    Kontext "DocumentDraw"
    DocumentDraw.TypeKeys "<TAB><TAB>"
    sleep (5)
    printlog "    Verify that the wanted object is selected"
    DocumentDraw.TypeKeys "<SHIFT F10>"         'Open contextmenu
    MenuSelect(MenuGetItemID(15))               'Select NAME from context
    kontext "NameDlgObject"
    if NameField.GetText <> "Chair" then
            Warnlog "Wrong object selected!"
        else Printlog "The correct object is selected"
    endif
    NameDlgObject.OK
    sleep 1
    printlog "Save position of group"
    ContextPositionAndSize            
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    sObjectPosition = PositionY.GetText
    printlog "ObjectPosition is " & sObjectPosition
    TabPositionAndSize.OK
    printlog "Move object"
    Kontext "DocumentDraw"
    DocumentDraw.TypeKeys "<UP><UP><UP>"
    sleep (5)
    printlog "Verify that the object was moved"
    ContextPositionAndSize                
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    if PositionY.GetText <> sObjectPosition then
            printlog "Single object was moved" 
        else warnlog "The object has not been moved"
    endif
    TabPositionAndSize.OK
    printlog "Exit group"
    FormatExitGroupDraw
    Printlog "Verify that the group is still on position"
    ContextPositionAndSize                'Format-position and size
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    if PositionY.GetText = sGroupPosition then
            printlog "Group is still on position"
        else warnlog "Group has moved from " & sGroupPosition & " to " & PositionY.GetText
    endif
    TabPositionAndSize.OK
    
    printlog "Close document"
    Call hCloseDocument

endcase 't_DrawingEngine