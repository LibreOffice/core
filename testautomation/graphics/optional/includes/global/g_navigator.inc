'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: g_navigator.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:40 $
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
'/************************************************************************
'*
'* Owner : wolfram.garten@sun.com
'*
'* short description :
'*
'**************************************************************************************
' #1 tdViewNavigator
' #1 tViewNavigatorZOrder
' #1 tViewNavigatorNameActivated
' #1 tViewNavigatorGroupedObjects
'\*************************************************************************************

testcase tdViewNavigator
    dim position1 as integer
    dim zaehler as integer
    dim Ueber_Text_1 as string
    dim Ueber_Text_2 as string
    dim Ueber_Text_3 as string
    dim Ueber_Text_4 as string
    dim Ueber_Text_5 as string
    dim Ueber_Text_6 as string

    Call hNewDocument

    For Zaehler = 1 to 3
        Select Case Zaehler
            Case 1:  Position1 = 20
            Case 2:  Position1 = 30
            Case 3:  Position1 = 40
        end select                     '///  create test document containing 3 slides ///'

        Call hRechteckErstellen ( Position1, Position1, 60, 60)  '/// On each page there is a rectangle at a different position, so jumping from slide to slide can be checked ///'
        ContextPositionAndSize
        kontext
        active.SetPage TabPositionAndSize
        kontext "TabPositionAndSize"
            Select Case Zaehler
                Case 1:   Ueber_Text_4 = PositionX.GetText
                Case 2:   Ueber_Text_5 = PositionX.GetText
                Case 3:   Ueber_Text_6 = PositionX.GetText
            End Select
            printlog "  CHECK: Xposition i: "+Zaehler+": "+PositionX.GetText
            TabPositionAndSize.OK
            Select Case Zaehler
                Case 1,2: InsertSlide
                    WaitSlot (2000)
                    hTypekeys "<Pagedown>"
                    WaitSlot (2000)
            End Select
    Next Zaehler

    WaitSlot (1000)
    ViewNavigator                   '/// open navigator flyer ///'
    Kontext "NavigatorDraw"
        if NavigatorDraw.exists (5) then printlog "OK"
            Erste.Click                     '/// jump to 1st slide ///'
            Ueber_Text_1 = Ueber_Text_4     '/// compare position ///'
            Ueber_Text_2 = "OK   First"
            Ueber_Text_3 = "First doesnt work"
            Call Position_Vergleichen (Ueber_Text_1,Ueber_Text_2,Ueber_Text_3)
            WaitSlot (1000)
            Kontext "NavigatorDraw"
                Naechste.Click                  '/// jump to next slide ///'
                Ueber_Text_1 = Ueber_Text_5     '/// compare position ///'
                Ueber_Text_2 = "OK   Next"
                Ueber_Text_3 = "Next doesnt work"
                Call Position_Vergleichen (Ueber_Text_1,Ueber_Text_2,Ueber_Text_3)
            Kontext "NavigatorDraw"
                Erste.Click                     '/// jump back to 1st slide ///'
                WaitSlot (1000)
                Letzte.Click                    '/// jump to last slide ///'
                Ueber_Text_1 = Ueber_Text_6     '/// compare position ///'
                Ueber_Text_2 = "OK   Last"
                Ueber_Text_3 = "Last doesnt work"
                Call Position_Vergleichen (Ueber_Text_1,Ueber_Text_2,Ueber_Text_3)
                WaitSlot (1000)
            Kontext "NavigatorDraw"
                Vorherige.Click
                Ueber_Text_1 = Ueber_Text_5
                Ueber_Text_2 = "OK   Before this"
                Ueber_Text_3 = "Before this doesnt work"
                Call Position_Vergleichen (Ueber_Text_1,Ueber_Text_2,Ueber_Text_3)

                WaitSlot (1000)
            Kontext "NavigatorDraw"
                try
                    Kontext "Navigator"
                        Navigator.Close                '/// close navigator ///'
                catch
                    hTypeKeys "<SHIFT MOD1 F5>"
                    Warnlog "- Unable to reach the Navigator, tried to use the keyboard Shortcut (MOD1 F5) to close it."
                endcatch

            '///  check state of navigator. Expected: Closed ///'
            Kontext "Navigator"
                if Navigator.exists then
                    '/// close navigator ///'
                    Navigator.Close
                    Warnlog "Navigator: should be closed at this point, but was not."
                else
                    printlog "Navigator: Not available - Good."
                end if

                WaitSlot (1000)
    '/// Close the document ///'
    Call hCloseDocument
endcase   'tdViewNavigator

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tViewNavigatorZOrder
    qaerrorlog "Not yet ready."
    goto endsub
    
    dim zPosition(4)
    dim Counter as integer
    dim MaxCounter as integer
    dim PositionName as string
    dim CheckingValue as string

    '/// New document ///'
    Call hNewDocument

    zPosition(1) = "10"
    zPosition(2) = "15"
    zPosition(3) = "20"
    zPosition(4) = "25"

    For Counter = 1 to 4
        '/// Insert Objects ///'
        Call hRechteckErstellen ((zPosition(Counter)), (zPosition(Counter)), 60, 60)
        WaitSlot (1000)
        '/// Name them according to their Z-Order ///'
            printlog "Name the Objects according to their Z-Order. (" +Counter + ")"
            hOpenContextMenu
            WaitSlot (1000)
            'Choose rename.
            if hMenuFindSelect(27027, true, 16) = false then
                Warnlog "Context-Menu-entry `Rename` was not found. Therefore the test ends."
                Call hCloseDocument
                Goto Endsub
            end if
            kontext "NameDlgObject"
                NameField.SetText "Object" + Counter
                NameDlgObject.OK
                hTypeKeys "<ESCAPE>"
    Next Counter

    '/// Check the Z-Order ///'
    printlog "check"

    '/// Open the Navigator ///'
    Kontext "NavigatorDraw"
        if (gApplication = "IMPRESS") then
            try
                NavigatorDraw.Close                '/// test if the Navigator is visible. It should not be. ///'
                warnlog "- the Navigator was open, check earlier ran tests for inconsistency."
                hTypeKeys "<SHIFT MOD1 F5>"
            catch
                hTypeKeys "<SHIFT MOD1 F5>"
            endcatch
        else
            Kontext "NavigatorDraw"
            try
                NavigatorDraw.Close                '/// test if the Navigator is visible. It should not be. ///'
                Warnlog "- the Navigator was open, check earlier ran tests for inconsistency."
                hTypeKeys "<F5>"
            catch
                hTypeKeys "<F5>"
            endcatch
        end if

'    * Check if the Navigator contains the 'Show Shapes' drop down menu.

    kontext "NavigatorDraw"
    Dragmodus.OpenMenu 

    MaxCounter = MenuGetItemCount
    if MaxCounter <> 0 then 
        For Counter = 1 to MaxCounter
            printlog "Entry (Dragmode): '" + (MenuGetItemText(MenuGetItemID (Counter))) + "' Found."
        Next Counter
    end if

    hMenuSelectNr (0)


    '/// Check the Options for the Shapes-Filter. ///'
    Shapesfilter.OpenMenu 
    MaxCounter = MenuGetItemCount
    For Counter = 1 to MaxCounter
        printlog "Entry (Shapesfilter): '" + (MenuGetItemText(MenuGetItemID (Counter))) + "' Found."
    Next Counter
    hMenuSelectNr (0)

    Printlog " "

    'Dokumentliste.Click

    kontext "NavigatorDraw"

    'Main part of the Navigator: the list with it's contents.

    Liste.TypeKeys "<PAGEDOWN>"
    CheckingValue = Liste.GetSelText
    printlog CheckingValue

    Liste.TypeKeys "<RIGHT>"
    Liste.TypeKeys "<DOWN>"
    CheckingValue = Liste.GetSelText
    printlog CheckingValue
    Liste.TypeKeys "<DOWN>"
    CheckingValue = Liste.GetSelText
    printlog CheckingValue
    Liste.TypeKeys "<DOWN>"
    CheckingValue = Liste.GetSelText
    printlog CheckingValue
    Liste.TypeKeys "<DOWN>"
    CheckingValue = Liste.GetSelText
    printlog CheckingValue
    printlog "ok?"


    '*NavigatorDraw         HID_SD_NAVIGATOR
    'NavigatorToolboxDraw   HID_SD_NAVIGATOR_TBX
    'Stift                  HID_SD_NAVIGATOR_TBI_PEN
    'Erste                  HID_SD_NAVIGATOR_TBI_FIRST
    'Vorherige              HID_SD_NAVIGATOR_TBI_PREV
    'Naechste               HID_SD_NAVIGATOR_TBI_NEXT
    'Letzte                 HID_SD_NAVIGATOR_TBI_LAST
    
    'Context-menu framställbar?
    'Dragmodus              HID_SD_NAVIGATOR_TBI_DRAGTYPE
    
    'Liste                  HID_SD_NAVIGATOR_TLB
    
    'Den undre scroll-delen.
    'Dokumentliste          HID_SD_NAVIGATOR_LB

'    * the drop  down menu must contain the entries 'Named shapes' and 'All Shapes'

    '/// Close the Navigator ///'
    Kontext "Navigator"
        try
            Navigator.Close
        catch
            printlog "Nope"
        endcatch

'    * check if 'Named shapes' is selected by default

'    * insert 5 shapes in the Document. the shapes should overlap slightly.
'    * there must be no entry in the navigator
'    * name 1 shape (context menu of the shape / name...)
'    * there must appear 1 entry 'under' the corresponding slide in the navigator
'    * Select All shapes in the Show shapes drop down menu
'    * there must be 5 shapes in the navigator
'    * change the order of the shapes in the navigator
'    * the z-order of the shapes must stay
'    * delete 1 shape from the slide
'    * the automatic generated names of the shapes must change and no shape name must be duplicate. The changed shape name must stay

'    * select 2 shapes and group them
'    * an entry with a small plus in front must appear in the navigator
'    * click on the plus
'    * there must be an additional tree with the 2 shapes of the group

    '/// Close the document ///'
    Call hCloseDocument
endcase 'tViewNavigatorZOrder

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tViewNavigatorNameActivated
    qaerrorlog "Not yet ready."
    goto endsub
    dim zPosition1 as integer
    dim zPosition2 as integer
    dim zPosition3 as integer
    dim zPosition4 as integer
    dim counter as integer

    '/// New document ///'
    Call hNewDocument

    '/// Insert Objects ///'
    Call hRechteckErstellen ( Position1, Position1, 60, 60) 

    '/// Name them accoirding to their Z-Order ///'
    '/// Open the Navigator ///'
    '/// Close the Navigator ///'
    '/// Close the document ///'
endcase 'tViewNavigatorNameActivated

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tViewNavigatorGroupedObjects
    qaerrorlog "Not yet ready."
    goto endsub
    dim zPosition1 as integer
    dim zPosition2 as integer
    dim zPosition3 as integer
    dim zPosition4 as integer
    dim counter as integer

    '/// New document ///'
    Call hNewDocument

    '/// Insert Objects ///'
    Call hRechteckErstellen ( Position1, Position1, 60, 60) 

    '/// Name them accoirding to their Z-Order ///'

    '/// Open the Navigator ///'
    '/// Close the Navigator ///'
    '/// Close the document ///'
endcase 'tViewNavigatorGroupedObjects

'--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
