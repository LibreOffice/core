'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
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
'*************************************************************************
'*
'* short description :
'*
'\******************************************************************************

testcase tiAlwaysWithCurrentPage()
    dim i as integer

    Call hNewDocument()
    printlog "Open file 'impdraw\required\input\diashow.od?"
    hFileOpen  ConvertPath (gTesttoolPath + "graphics\required\input\diashow.odp")
    WaitSlot(10000) 'sleep 10

    printlog "Check if the document is writable"
    if fIsDocumentWritable = false then
        printlog "Make the document writable and check if it's succesfull"
        if fMakeDocumentWritable = false then
            warnlog "The document can't be make writeable. Test stopped."
            goto endsub
        endif
    endif

    printlog "Test with default option."
    printlog "Checking: 'Always with current page' - default"

    printlog "Setting Options to start with current page."
    setStartCurrentPage(true)
    printlog "Check if Navigator is open. Default: closed."
    Kontext "NavigatorDraw"
    if NavigatorDraw.exists (5) then
        warnlog "presupposition not met: navigator should be closed on loading the document!"
    else
        ViewNavigator
        WaitSlot (2000)
    end if

    printlog "Making sure after loading we are on slide one."
    i = fGetSlideNumber()
    if (i<>1) then
        warnlog "Presupposition not met. After loading the file not the first slide is up. It is number: " + i
    endif

    printlog "Press Key [Page Down] two times to get to slide number three. We are on slide one."
    hTypeKeys ("<home><PageDown><PageDown>")
    printlog "Start the slideshow with 'Slide Show->Slide Show."
    SlideShowSlideshow
    kontext "DocumentPresentation"
    printlog "Open the navigator View->Navigator."
    Kontext "NavigatorDraw"
    printlog "Opening navigator if none is there."
    if (not NavigatorDraw.exists(5)) then
        ViewNavigator
    endif
    printlog "The slideshow has to start on slide 3."
    printlog "running routine for slide 3 and 4."
    for i = 3 to 4
        Kontext "NavigatorDraw"
        printlog "Checking for navigator.."
        if (not NavigatorDraw.exists(5)) then
            ViewNavigator
        endif
        printlog "Getting number of current slide."
        printlog "Countervariable is " & i
        fGetSlideNumber(i)
        printlog "Current slide number is: " & i & "."
        printlog "Typing pagedown to get to next slide."
        DocumentPresentation.TypeKeys "<pagedown>"
        WaitSlot (5000) 'sleep (5)
    next i

    printlog "Trying to end presentation with mouseclick on black endscreen."
    try
        DocumentPresentation.mousedown (50,50,1)
        DocumentPresentation.mouseup (50,50,1)
    catch
        warnlog "TimeOut on pressing mouse button."
    endcatch

    printlog "Trying to recognize if slideshow did not end.."
    try
        gMouseClick 50,50  ' this one works to recognice the not ending !
        Printlog "- Slideshow ended at the right time"
    catch
        Warnlog "- Slideshow still running "
        kontext "DocumentPresentation"
        printlgo "Ending presentation with ESC."
        DocumentPresentation.TypeKeys "<ESCAPE>"
    endcatch
    WaitSlot (2000)    'sleep 2

    printlog "Test with changed option: start with first slide."
    setStartCurrentPage(false)
    printlog "UnChecked: Always with current page."
    printlog "press Key [Home] to get to the first slide."
    kontext "DocumentImpress"
    printlog "Deselect object."
    hTypeKeys "<ESCAPE>"
    hTypeKeys "<home>"
    printlog "Making sure navigator is open."
    Kontext "NavigatorDraw"
    if (not NavigatorDraw.exists(5)) then
        ViewNavigator
    endif
    printlog "Checking if we are on slide one."
    i = fGetSlideNumber()
    if (i<>1) then
        warnlog "Start-condition not met. Not the first slide is up. It is number: " & i & "."
    endif
    printlog "Press Key [Page Down] two times to get to slide number three."
    hTypeKeys "<PageDown> <PageDown>"
    printlog "Start the slideshow with 'Slide Show->Slide Show."
    SlideShowSlideshow
    kontext "DocumentPresentation"
    printlog "Open the navigator View->Navigator."
    Kontext "NavigatorDraw"
    printlog "Checking if navigator is open."
    if (not NavigatorDraw.exists(5)) then
        ViewNavigator
    endif
    printlog "The slideshow has to start on slide 1."
    printlog "Running routine for slide 1-4. i = 1 to 3 because we start already on slide 1."
    for i = 1 to 3
        Kontext "NavigatorDraw"
        if (not NavigatorDraw.exists(5)) then
            ViewNavigator
        endif
        printlog "Getting number of current slide."
        fGetSlideNumber(i)
        printlog "Current slide is: " & i & "."
        DocumentPresentation.TypeKeys "<pagedown>"
        WaitSlot (5000)
    next i
    fGetSlideNumber(i)
    printlog "Current slide is: " & i & "."
    DocumentPresentation.TypeKeys "<pagedown>"
    printlog "Trying to end presentation with mouseclick on black endscreen."
    try
        DocumentPresentation.mousedown (50,50,1)
        DocumentPresentation.mouseup (50,50,1)
        Printlog "- Slideshow ended at the right time"
    catch
        Warnlog "- Slideshow still running "
        kontext "DocumentPresentation"
        printlog "Ending running presentation with ESC."
        DocumentPresentation.TypeKeys "<ESCAPE>"
    endcatch

    Kontext "NavigatorDraw"
    printlog "Checking if navigator is open."
    if (not NavigatorDraw.exists(5)) then
        ViewNavigator
    endif
    printlog "Checking if we are on slide 3."
    i = fGetSlideNumber()
    if (i<>3) then
        warnlog "We are on the wrong slide! Should be 3 but it is: " & i
    endif

    printlog "Close the navigator View->Navigator."
    Kontext "NavigatorDraw"
    if NavigatorDraw.exists(5) then
        ViewNavigator
    endif

    printlog "Restore: check Always with current page."
    setStartCurrentPage(true)
    Call hCloseDocument
endcase

