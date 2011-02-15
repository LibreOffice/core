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
'* Owner : wolfram.garten@oracle.com
'*
'* short description: includefile for Slidecopy-testing
'*
'\******************************************************************************

testcase tiSlideCopyNewPresentation

    const MENUITEM_RENAME = 27027

    Dim value1 as string
    Dim value2 as string
    Dim value12 as string
    Dim value13 as string

    printlog "This test copies one slide from one presentation to another."
    printlog "Make new presentation"
    Call  hNewDocument
    Call sSelectEmptyLayout
     printlog "Insert three objects: Connector, Door-plate, and Smiley."
     
     printlog "First we Insert a Connector."
     kontext "Toolbar"
     sleep 1
     try
        Verbinder.TearOff
        printlog "Tearing off Connector bar.."
    catch
        warnlog "bug for GH from FHA; .tearoff doesnt tell success"
    endcatch
    kontext "Connectorsbar"
    sleep 2
    Verbinder.click
    sleep 5
    gMouseDown (10,10)
    gMouseMove (10,10,30,30)
    gMouseUp (30,30)
    kontext "Connectorsbar"
    Connectorsbar.Close
    hTypeKeys "<ESCAPE>"
    hTypeKeys "<TAB>", 1
    printlog "We rename the object via the contextmenu."
    sleep (1)
    DocumentImpress.TypeKeys "<SHIFT F10>"
    sleep (2)
    printlog "then Choose rename."
    if hMenuFindSelect( MENUITEM_RENAME, true, 14) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif
    
    kontext "NameDlgObject"
    if ( NameDlgObject.exists( 2 ) ) then
        NameField.SetText "First"
        NameDlgObject.OK
    else
        warnlog( "<NameDlgObject> failed to open" )
    endif

    kontext "DocumentImpress"
    hMouseClick DocumentImpress, 90, 90
    '----------------------------------------------------------------------------1
    printlog "Then we insert the second object: a Door-plate."

    kontext "Toolbar"
    sleep 2
    try
        StarShapes.TearOff
        printlog "Tearing off StarShapes Bar..."
    catch
        warnlog( "<StarShapes.TearOff> failed" )
        goto endsub
    endcatch

    kontext "StarShapes"
    sleep 2
    if ( StarShapes.exists( 2 ) ) then
        StarShapesDoorplate.click
    else
        warnlog( "<StarShapesDoorplate> could not be clicked" )
    endif

    sleep (5)
    gMouseDown (40,40)
    gMouseMove (40,40,60,60)
    gMouseUp (60,60)

    kontext "StarShapes"
    hCloseDialog( StarShapes, "close" )

    hTypeKeys "<ESCAPE>"
    hTypeKeys "<TAB>", 2
    printlog "And then we rename the object."
    sleep (1)
    DocumentImpress.TypeKeys "<SHIFT F10>"
    sleep 2
    if hMenuFindSelect( MENUITEM_RENAME, true, 15) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif
    kontext "NameDlgObject"
    if ( NameDlgObject.exists( 2 ) ) then
        NameField.SetText "Second"
        NameDlgObject.OK
    else
        warnlog( "<NameDlgObject> could not be accessed" )
    endif

    kontext "DocumentImpress"
    hMouseClick DocumentImpress, 90, 90
    '----------------------------------------------------------------------------2
    printlog "Then we insert the thrid object: a Smiley."
    kontext "Toolbar"
    sleep 1
    try
        SymbolShapes.TearOff
    catch
        warnlog "bug for GH from FHA; .tearoff doesnt tell success"
    endcatch
    kontext "SymbolShapes"
    sleep 1
    SymbolShapesSmiley.Click
    sleep 2
    gMouseDown (70,70)
    gMouseMove  (70,70,89,89)
    gMouseUp (89,89)
    kontext "SymbolShapes"
    SymbolShapes.Close
    hTypeKeys "<ESCAPE>"
    hTypeKeys "<TAB>", 3
    printlog "We rename the object via the Context-menu."
    DocumentImpress.TypeKeys "<SHIFT F10>"
    sleep (2)
    'Choose rename.
    if hMenuFindSelect( MENUITEM_RENAME, true, 15) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif
    kontext "NameDlgObject"
    NameField.SetText "Third"
    NameDlgObject.OK
    kontext "DocumentImpress"
    hMouseClick DocumentImpress, 90, 90
    '---------------------------------------------------------------------------3
    printlog "Copy the slide to the clipboard"
    kontext "Slides"
    SlidesControl.TypeKeys "<PAGEUP>"
    sleep (1)
    SlidesControl.TypeKeys "<SHIFT F10>"
    sleep 2
    printlog "We copy the object via the Context-menu."

    if hMenuFindSelect(5711, true, 10) = false then
        Warnlog "Context-Menu-entry `Copy` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif
    printlog "Close the presentation-window"
    Call hCloseDocument
    printlog "Open a new presentation"
    Call  hNewDocument
    printlog "Paste the slide from the clipboard!"
    kontext "Slides"
    hUseAsyncSlot( "EditPaste" )
    printlog "Delete the first slide"
    hUseAsyncSlot( "EditDeleteSlide" )
    printlog "Check if all three objects exists, and has the right values"
    kontext "DocumentImpress"
    hTypeKeys "<ESCAPE>"
    hTypeKeys "<TAB>", 1 'Select the first object.
    'Bring up the kontext-menu for the object
    sleep (1)
    DocumentImpress.TypeKeys "<SHIFT F10>"
    sleep 2
    printlog "We rename the object via the Context-menu."
    if hMenuFindSelect( MENUITEM_RENAME, true, 14) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif
    printlog "Read out what the name is, and checks if it's correct."
    kontext "NameDlgObject"
    value1 = NameField.GetText
    NameDlgObject.OK
    if value1 = "First" then
        printlog "First object was found correctly"
    else
        Warnlog "Wrong object or object-name! The name found was: " + value1
    endif
    kontext "DocumentImpress"
    hMouseClick DocumentImpress, 90, 90
    DocumentImpress.TypeKeys "<TAB TAB>" 'Select the second object.
    'Bring up the kontext-menu for the object
    sleep (1)
    DocumentImpress.TypeKeys "<SHIFT F10>"
    sleep 2
    printlog "We rename the object via the Context-menu."
    if hMenuFindSelect( MENUITEM_RENAME, true, 15) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif
    printlog "Read out what the name is, and check if it's correct."
    sleep 1
    kontext "NameDlgObject"
    value12 = NameField.GetText
    NameDlgObject.OK

    if value12 = "Second" then
        printlog "Second object was found correctly"
    else
        Warnlog "Wrong object or object-name! The name found was: " + value12
    endif

    kontext "DocumentImpress"
    hMouseClick DocumentImpress, 90, 90
    DocumentImpress.TypeKeys "<TAB TAB TAB>" 'Select the third object.
    'Bring up the kontext-menu for the object
    sleep (2)
    DocumentImpress.TypeKeys "<SHIFT F10>"
    sleep 2
    printlog "We rename the object via the Context-menu."
    if hMenuFindSelect( MENUITEM_RENAME, true, 15) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif
    printlog "Read out what the name is, and check if it's correct."
    kontext "NameDlgObject"
    value13 = NameField.GetText
    NameDlgObject.OK

    if value13 = "Third" then
        printlog "Third object was found correctly"
    else
        Warnlog "Wrong object or object-name! The name found was: " + value13
    endif

    printlog "Close the bars we opened before."
    kontext "Connectorsbar"
    if Connectorsbar.Exists then Connectorsbar.Close
    kontext "SymbolShapes"
    if SymbolShapes.Exists then SymbolShapes.Close
    kontext "StarShapes"
    if StarShapes.Exists then StarShapes.Close

    printlog "Close Presentation. End testcase"
    Call hCloseDocument
endcase 'tiSlideCopyNewPresentation

'-------------------------------------------------------------------------

testcase tiSlideCopyDuplicate
    Dim value1 as string
    Dim value2 as string
    Dim value12 as string
    Dim value13 as string
    dim i as integer
    printlog "Make new presentation"
     Call  hNewDocument
     Call sSelectEmptyLayout
     printlog "Insert Connector."
     kontext "Toolbar"
     sleep 1
     try
        Verbinder.TearOff  ' insert connector
    catch
        warnlog "bug for GH from FHA; .tearoff doesnt tell success"
    endcatch
    kontext "Connectorsbar"
    sleep 1
    Verbinder.click
    sleep 1
    kontext "DocumentImpress"
    gMouseDown (10,10)
    gMouseMove (10,10,30,30)
    gMouseUp (30,30)

     hTypeKeys "<ESCAPE>"
     hTypeKeys "<TAB>", 1
     sleep (1)
     
     printlog "rename object"
     DocumentImpress.TypeKeys "<SHIFT F10>"
     sleep 1
     
     printlog "Choose rename"
     if hMenuFindSelect(27027, true, 14) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif
    kontext "NameDlgObject"
    NameField.SetText "First"
    NameDlgObject.OK
    sleep 1

    kontext "DocumentImpress"
    hMouseClick DocumentImpress, 90, 90

     printlog "2 Insert Door-plate"
     kontext "Toolbar"
     sleep 1
     try
        StarShapes.TearOff
     catch
        warnlog "bug for GH from FHA; .tearoff doesnt tell success"
    endcatch
    kontext "StarShapes"
    sleep 1
    StarShapesDoorplate.click
    sleep 1
    kontext "DocumentImpress"
    gMouseDown (40,40)
    gMouseMove (40,40,60,60)
    gMouseUp (60,60)

    hTypeKeys "<ESCAPE>"
    hTypeKeys "<TAB>", 2
    sleep (1)

     printlog "Rename object"
     kontext "DocumentImpress"
     DocumentImpress.TypeKeys "<SHIFT F10>"
     sleep (1)
     printlog "Choose rename."
     if hMenuFindSelect(27027, true, 15) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif
    kontext "NameDlgObject"
    NameField.SetText "Second"
    NameDlgObject.OK
    sleep 1

    kontext "DocumentImpress"
    hMouseClick DocumentImpress, 90, 90

     printlog "Insert Smiley."
     kontext "Toolbar"
     sleep 1
     try
        SymbolShapes.TearOff  
     catch
        warnlog "bug for GH from FHA; .tearoff doesnt tell success"
    endcatch
    kontext "SymbolShapes"
    sleep 1
    SymbolShapesSmiley.Click
    sleep 1
    gMouseDown (70,70)
    gMouseMove  (70,70,89,89)
    gMouseUp (89,89)

    hTypeKeys "<ESCAPE>"
    hTypeKeys "<TAB>", 3
    sleep (1)

     printlog "rename object"
     DocumentImpress.TypeKeys "<SHIFT F10>"
     sleep (1)
     printlog "Choose rename"
     if hMenuFindSelect(27027, true, 15) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif
    kontext "NameDlgObject"
    NameField.SetText "Third"
    NameDlgObject.OK
    sleep 1

    kontext "DocumentImpress"
    hMouseClick DocumentImpress, 90, 90

     printlog "Close the bars we opened before."
     kontext "Connectorsbar"
     if Connectorsbar.Exists then Connectorsbar.Close
     kontext "SymbolShapes"
     if SymbolShapes.Exists then SymbolShapes.Close
     kontext "StarShapes"
     if StarShapes.Exists then StarShapes.Close

     printlog "Copy the slide to the clipboard"
     kontext "Slides"
     SlidesControl.TypeKeys "<PAGEUP>"
     sleep (1)
     SlidesControl.TypeKeys "<SHIFT F10>"
     sleep (1)
     printlog "Choose `Copy`"
     if hMenuFindSelect(5711, true, 10) = false then
        Warnlog "Context-Menu-entry `Copy` was not found.Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif

     printlog "Paste the content from the Clipboard"
     kontext "Slides"
     printlog "open context-menu choose 'paste'"

    EditPaste

    kontext "InsertPaste"
    if InsertPaste.Exists(1) then
        After.Check
        InsertPaste.OK
     endif
     sleep (2)
     printlog "Check if we have two slides, and that the objects exists, and has the right values "

    kontext "Slides"
    SlidesControl.TypeKeys "<PAGEUP>"

     for i = 1 to 2   
        Printlog "Checking objects the " + i + " + time."
        kontext "DocumentImpress"
        hMouseClick DocumentImpress, 90, 90
        DocumentImpress.TypeKeys "<TAB>" 
        sleep (1)
        
        DocumentImpress.TypeKeys "<SHIFT F10>"
        sleep 1
        printlog "Choose rename"
	if hMenuFindSelect(27027, true, 14) = false then
           Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
           Call hCloseDocument
           Goto Endsub
        endif

        printlog "Read out what the name is, and check if its correct."
        kontext "NameDlgObject"  
        value1 = NameField.GetText
        NameDlgObject.OK 

        if value1 = "First" then
            printlog "First object was found correctly"
        else
            Warnlog "Wrong object or object-name! The name found was: " + value1
        endif

        hTypeKeys "<ESCAPE>"
        DocumentImpress.TypeKeys "<TAB>", 2
        sleep (1)
        DocumentImpress.TypeKeys "<SHIFT F10>"

        printlog "Choose rename"
        if hMenuFindSelect(27027, true, 15) = false then
            Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
            Call hCloseDocument
            Goto Endsub
        endif

        printlog "Read out what the name is, and check if its correct."
        kontext "NameDlgObject"
        value12 = NameField.GetText
        NameDlgObject.OK

        if value12 = "Second" then
            printlog "Second object was found correctly"
        else
            Warnlog "Wrong object or object-name! The name found was: " + value12
        endif

        hTypeKeys "<ESCAPE>"
        printlog "Select the third object."
        hTypeKeys "<TAB>", 3 
        sleep (1)

        printlog "Bring up the kontext-menu for the object"
        DocumentImpress.TypeKeys "<SHIFT F10>"
        sleep 1
        printlog "Choose rename"
        if hMenuFindSelect(27027, true, 15) = false then
            Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
            Call hCloseDocument
            Goto Endsub
        endif

        printlog "Read out what the name is, and check if its correct."
        kontext "NameDlgObject"
        value13 = NameField.GetText
        NameDlgObject.OK

        if value13 = "Third" then
            printlog "Third object was found correctly"
        else
            Warnlog "Wrong object or object-name! The name found was: " + value13
        endif
        kontext "Slides"
        SlidesControl.TypeKeys "<DOWN>"
    next i

     printlog " Close Presentation. End testcase "
     Call hCloseDocument
endcase 'tiSlidecopyDuplicate

'-----------------------------------------------------------------------

testcase tiSlideCopyPasteSpecial
    qaerrorlog "#i93377#: Paste special dialog does not come up with kontext on slide pane"
    goto endsub

    Dim value1 as string
    Dim value2 as string
    Dim value3 as string
    printlog "Copies slide, and pastes it with Paste Special"
    printlog "just to see if the office can handle it"
    printlog "Make new presentation"
    Call  hNewDocument
    printlog "Insert three objects: Connector, Door-plate, and Smiley"

    printlog "Insert Connector"
    kontext "Toolbar"
    sleep 1
    try
        Verbinder.TearOff  
    catch
        warnlog "bug for GH from FHA; .tearoff doesnt tell success"
    endcatch
    kontext "Connectorsbar"
    sleep 1
    Verbinder.click
    sleep 1
    gMouseDown (10,10)
    gMouseMove (10,10,30,30)
    gMouseUp (30,30)

    kontext "DocumentImpress"
    hMouseClick DocumentImpress, 90, 90

    printlog "Insert Door-plate"
    kontext "Toolbar"
    sleep 1
    try
        StarShapes.TearOff 
    catch
        warnlog "bug for GH from FHA; .tearoff doesnt tell success"
    endcatch
    kontext "StarShapes"
    sleep 1
    StarShapesDoorplate.click
    sleep 1
    gMouseDown (40,40)
    gMouseMove (40,40,60,60)
    gMouseUp (60,60)

    kontext "DocumentImpress"
    hMouseClick DocumentImpress, 90, 90

    printlog "Insert Smiley"
    kontext "Toolbar"
    sleep 1
    try
        SymbolShapes.TearOff
    catch
        warnlog "bug for GH from FHA; .tearoff doesnt tell success"
    endcatch
    kontext "SymbolShapes"
    sleep 1
    SymbolShapesSmiley.Click
    sleep 1
    gMouseDown (70,70)
    gMouseMove  (70,70,89,89)
    gMouseUp (89,89)

    printlog "Copy the slide to the clipboard"
    kontext "Slides"
    SlidesControl.TypeKeys "<PAGEUP>"
    SlidesControl.TypeKeys "<SHIFT F10>"
    sleep 1
    'Choose `Copy`
    if hMenuFindSelect(5711, true, 10) = false then
        Warnlog "Context-Menu-entry `Copy` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif

    printlog "Try to paste special. And check that the office still stays alive"
    EditPasteSpecial
    kontext "InhaltEinfuegen"
    InhaltEinfuegen.OK

    printlog "Close the bars we opened before"
    kontext "Connectorsbar"
    if Connectorsbar.Exists then Connectorsbar.Close
    kontext "SymbolShapes"
    if SymbolShapes.Exists then SymbolShapes.Close
    kontext "StarShapes"
    if StarShapes.Exists then StarShapes.Close

    sleep 1
    kontext "DocumentImpress"
    printlog "Close Presentation. End testcase"

    Call hCloseDocument
endcase 'tiSlideCopyPasteSpecial

'-----------------------------------------------------------------------

testcase tiSlideCopyInSlideSorter
    qaerrorlog "#i58418#: Pasting in slide sorter sets slide to wrong position"
    goto endsub

    Dim value1 as string
    Dim value2 as string
    Dim value3 as string
    printlog "Copies slides within the slidesorter, and makes sure they are pasted on the correct position"
    printlog "Make new presentation"
    Call  hNewDocument
    printlog "Insert an object: Connector, on the first slide"
    kontext "Toolbar"
    sleep 1
    try
        Verbinder.TearOff
    catch
        warnlog "bug for GH from FHA; .tearoff doesnt tell success"
    endcatch
    kontext "Connectorsbar"
    sleep 1
    Verbinder.click
    Verbinder.click
    Verbinder.click
    sleep 1
    kontext "DocumentImpress"
    gMouseDown (10,10)
    gMouseMove (10,10,30,30)
    gMouseUp (30,30)
    Printlog "   Inserted a Connector-object."
    hTypeKeys "<ESCAPE>"
    hTypeKeys "<TAB>", 1
    DocumentImpress.TypeKeys "<SHIFT F10>"
    if hMenuFindSelect(27027, true, 14) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif
    kontext "NameDlgObject"
    NameField.SetText "First"
    NameDlgObject.OK
    sleep 1
    Printlog "Renamed a the object to 'First'"

    sleep 3
    printlog "Rename slide to 1"
    kontext "Slides"
    SlidesControl.TypeKeys "<PAGEUP>"
    SlidesControl.TypeKeys "<SHIFT F10>"
    sleep 3
    if hMenuFindSelect(27268, true, 2) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif

    kontext "NameDlgPage"
    NameField.SetText "1"
    NameDlgPage.OK
    sleep 1
    Printlog "Renamed the Slide to '1'"

    kontext "DocumentImpress"
    hMouseClick DocumentImpress, 90, 90
    sleep 3

    printlog "Insert a new slide"
    kontext "Slides"
    SlidesControl.TypeKeys "<TAB><PAGEDOWN>"
    SlidesControl.OpenContextMenu
    sleep 3
    if hMenuFindSelect(27014, true, 1) = false then
        Warnlog "Context-Menu-entry `Insert slide` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif
    Printlog "Inserted a new slide."
    printlog "Insert an object: Door-plate, on the second slide"
    kontext "Toolbar"
    sleep 1
    try
        StarShapes.TearOff
    catch
        warnlog "bug for GH from FHA; .tearoff doesnt tell success"
    endcatch
    kontext "StarShapes"
    sleep 1
    StarShapesDoorplate.click
    sleep 1
    kontext "DocumentImpress"
    gMouseDown (40,40)
    gMouseMove (40,40,60,60)
    gMouseUp (60,60)
    Printlog "Inserted a Door-Plate-object."
    hTypeKeys "<ESCAPE>"
    hTypeKeys "<TAB>", 2
    hTypeKeys "<SHIFT F10>"
    if hMenuFindSelect(27027, true, 15) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif

    kontext "NameDlgObject"
    NameField.SetText "Second"
    NameDlgObject.OK
    sleep 1
    Printlog "Renamed a the object to 'Second'"

    kontext "DocumentImpress"
    hMouseClick DocumentImpress, 90, 90
    sleep 2
    printlog "Rename slide to 2"
    kontext "Slides"
    SlidesControl.OpenContextMenu
    sleep 3
    if hMenuFindSelect(27268, true, 3) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif
    kontext "NameDlgPage"
    NameField.SetText "2"
    NameDlgPage.OK
    sleep 1
    Printlog "   Renamed the slide '2'"

    printlog "Insert a new slide"
    kontext "Slides"
    Slidescontrol.TypeKeys "<PAGEDOWN>", 2
    Slidescontrol.TypeKeys "<SHIFT F10>"
    sleep 3
    if hMenuFindSelect(27014, true, 1) = false then
        Warnlog "Context-Menu-entry `Insert slide` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif
    Printlog "Inserted a new slide."
    printlog "Insert an object: Smiley, on the second slide"
    kontext "Toolbar"
    sleep 1
    try
        SymbolShapes.TearOff
    catch
        warnlog "bug for GH from FHA; .tearoff doesnt tell success"
    endcatch
    kontext "SymbolShapes"
    sleep 1
    SymbolShapesSmiley.Click
    sleep 1
    gMouseDown (70,70)
    gMouseMove  (70,70,89,89)
    gMouseUp (89,89)
    Printlog "   Inserted a Smiley-object."
    hTypeKeys "<ESCAPE>"
    hTypeKeys "<TAB>", 2
    hTypeKeys "<SHIFT F10>"
    if hMenuFindSelect(27027, true, 15) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif
    kontext "NameDlgObject"
    NameField.SetText "Third"
    NameDlgObject.OK
    Printlog "   Renamed a the object to 'Third'"
    printlog "Rename slide to 3"
    kontext "Slides"
    Slidescontrol.TypeKeys "<PAGEDOWN>", 2
    SlidesControl.OpenContextMenu
    sleep 3
    if hMenuFindSelect(27268, true, 3) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif

    kontext "NameDlgPage"
    NameField.SetText "3"
    NameDlgPage.OK
    sleep 1
    Printlog "   Renamed the slide '3'"

    printlog "Close the bars we opened before."
    kontext "Connectorsbar"
    if Connectorsbar.Exists then Connectorsbar.Close
    kontext "SymbolShapes"
    if SymbolShapes.Exists then SymbolShapes.Close
    kontext "StarShapes"
    if StarShapes.Exists then StarShapes.Close

    printlog "Change to Slidesorter"
    kontext "DocumentImpress"
    ViewSlideSorter

    printlog "Now we have changed view to the slidesorter"

    kontext "Slides"
    Printlog "Changed view to 'SlideSorter'"
    printlog "Make sure the last slide is selcted"
    SlidesControl.TypeKeys "<TAB>"
    SlidesControl.TypeKeys "<PAGEDOWN>", 3

    printlog "Cut the last slide"
    SlidesControl.TypeKeys "<MOD1 X>"

    printlog "Make sure the first slide is selected"
    SlidesControl.TypeKeys "<TAB>"
    SlidesControl.TypeKeys "<PAGEUP>", 2
    sleep 1

    printlog "Paste the Slide"
    kontext "Slides"
    SlidesControl.TypeKeys "<MOD1 V>"

    printlog "choose Before as specification for where"
    Kontext "InsertPaste"
    if InsertPaste.Exists(3) then
        Before.Check
        InsertPaste.OK
        Printlog "   Pasted slide 'Before'"
    else
        warnlog "Problem when copying/pasting slide."
    endif

    printlog "Check where the slide ended up"
    kontext "Slides"
    SlidesControl.TypeKeys "<TAB>"
    SlidesControl.TypeKeys "<PAGEUP>", 3 'to get to the first slide
    SlidesControl.TypeKeys "<SHIFT F10>"
    sleep 3
    if hMenuFindSelect(27268, true, 3) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif

    kontext "NameDlgPage"
    if NameField.GetText <> "3" then
        Warnlog "Wrong slide found! Expected nr 3, but found " + NameField.GetText + "."
    else
        Printlog "   Slide nr 3 was found correctly."
    endif
    NameDlgPage.OK
    sleep 1

    kontext "Slides"
    printlog "Choose the slide in the middle (named 1)"
    SlidesControl.TypeKeys "<TAB>"
    printlog "to get to the last slide"
    SlidesControl.TypeKeys "<PAGEDOWN>", 3
    printlog "to go back one step"
    SlidesControl.TypeKeys "<PAGEUP>" 
    SlidesControl.OpenContextMenu (true)
    printlog "Check the name of that slide (via Rename)"
    if hMenuFindSelect(27268, true, 3) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif

    kontext "NameDlgPage"
    if NameField.GetText <> "1" then
        Warnlog "Wrong slide found! Expected nr 1, but found " + NameField.GetText + "."
    else
        Printlog "   Slide nr 1 was found correctly in the middle position."
    endif
    NameDlgPage.OK
    sleep 1
    kontext "Slides"

    printlog "Cut the slide"
    SlidesControl.TypeKeys "<SHIFT F10>"
    sleep 3
    if hMenuFindSelect(5710, true, 10) = false then
        Warnlog "Context-Menu-entry `Cut` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif

    printlog "Make sure we have selected the last slide"
    SlidesControl.TypeKeys "<TAB>"
    SlidesControl.TypeKeys "<PAGEDOWN>", 2

    printlog "Place the mousepointer to the left of the first slide"
    sleep 1
    kontext "Slides"
    SlidesControl.MouseMove 2, 3
    SlidesControl.OpenContextMenu true
    sleep 3

    printlog "Paste the Slide"
    if hMenuFindSelect(5712, true, 12) = false then
        Warnlog "Context-Menu-entry `Paste` were not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif
    sleep 1

    printlog "Choose After as specification for where"
    Kontext "InsertPaste"
    if InsertPaste.Exists(3) then
        After.Check
        InsertPaste.OK
        Printlog "   Pasted slide 'After'"
    else
        warnlog "   Problem when copying/pasting slide."
    endif

    printlog "Check where the slide ended up. Should have showed up at the last position"
    kontext "Slides"
    SlidesControl.TypeKeys "<TAB>"
    printlog "to get to the last slide"
    SlidesControl.TypeKeys "<PAGEDOWN>", 3
    printlog "open context menu"
    SlidesControl.TypeKeys "<SHIFT F10>" 
    sleep 3
    printlog "Rename"
    if hMenuFindSelect(27268, true, 3) = false then
        Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
        Call hCloseDocument
        Goto Endsub
    endif

    kontext "NameDlgPage"
    if NameField.GetText <> "1" then
        Warnlog "Wrong slide found! Expected nr 1, but found " + NameField.GetText + "."
    else
        Printlog "   Slide nr 1 was found correctly."
    endif
    NameDlgPage.OK
    sleep 1

    printlog "Close Presentation. End testcase"
    Call hCloseDocument
endcase 'tiSlideCopyInSlideSorter

'------------------------------------------------------------------------
