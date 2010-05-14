'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: g_crossfading.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:39 $
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
' #1 tCrossfading
'\*************************************************************************************

testcase tCrossfading
    dim X as integer
    dim Z as integer
    dim Y as integer
    dim I as integer
    dim CheckX as integer
    dim FirstX as integer
    dim CounterForX as integer
    dim anothercounter as integer

    gMouseClick 50,50
    sleep 1
    gMouseClick 90,90
    sleep 1
    sleep 1
    kontext "Ueberblenden"
    X = Schritte.GetText									' /// Get counter-value and save it in X ///'
    printlog "Steps before we start the test: " + X
    printlog "Schritte.GetText= " + Schritte.GetText

    Schritte.ToMin
    if Schritte.GetText <> "1" then qaerrorlog "Could not change the step-number to 1."

        for I = 2 to 10										' /// How many steps do you want? ///'
            Schritte.More 1
            if Schritte.GetText <> I then qaerrorlog "Could not change the step-number to" + I
                Ueberblenden.OK
                sleep 1
                try
                catch
                    qaerrorlog "Could not ungroup objects"
                endcatch
                sleep 1

                try
                    ContextPositionAndSize
                catch
                    qaerrorlog "Couldnt call 'ContextPositionAndSize' - No object selected?"
                endcatch
                kontext
                active.SetPage TabPositionAndSize
                sleep 1
                kontext "TabPositionAndSize"
                if TabPositionAndSize.exists (5) then
                    CounterForX=0
                    CheckX=0
                    TabPositionAndSize.OK
                else
                    qaerrorlog "Couldn't switch tab page :-( "
                endif



                if Y > anothercounter then warnlog "Too many steps, something is wrong."
                    try
                    catch
                        errolog "Could not step to the next object"
                    endcatch
                    '   Printlog "- Get position and dimensions of elements"
                    sleep 1
                    Kontext
                    Active.SetPage TabPositionAndSize
                    Kontext "TabPositionAndSize"
                    sleep 1
                    TabPositionAndSize.OK
                    Kontext "DocumentDraw"
                    if anothercounter = CounterForX then printlog "The first box is at: " + CheckX + " And we're now at " + FirstX + " = Correct steps between the objects, everything's fine."
                        printlog "Number of objects: " + anothercounter
                        printlog ""
                    endif

                    sleep 1
                    gMouseClick 90,90
                    sleep 1
                    sleep 1
                    sleep 2
                    kontext "Ueberblenden"
                next I


                for Z = 1 to X -1
                next Z
                printlog "Value has been restored to: " + Schritte.GetText
                printlog "X is now: " + X
                Attributierung.UnCheck
                Ueberblenden.OK
                sleep 1
                Kontext "DocumentDraw"
                sleep 1
                gMouseClick 90,90
                sleep 1
                Kontext "Ueberblenden"
                if Attributierung.IsChecked = TRUE then qaerrorlog "Attributierung /    was NOT unchecked."
                    sleep 1
                    Kontext "DocumentDraw"
                    sleep 1
                    gMouseClick 90,90
                    sleep 1
                    Kontext "Ueberblenden"
                    if GleicheOrientierung.IsChecked = TRUE then qaerrorlog "GleicheOrientierung /    was NOT unchecked."
                        sleep 1
                        Kontext "DocumentDraw"
                        sleep 1
                        sleep 1
                        gMouseClick 90,90
                        sleep 1
                        Kontext "Ueberblenden"
                        if Attributierung.IsChecked = FALSE then qaerrorlog "Attributierung /    was NOT checked again."
                            Ueberblenden.OK
                            sleep 1
                            Kontext "DocumentDraw"
                            sleep 1
                            gMouseClick 90,90
                            sleep 1
                            Kontext "Ueberblenden"
                            if GleicheOrientierung.IsChecked = TRUE then qaerrorlog "GleicheOrientierung /    was NOT checked again."
                                sleep 1
                                Kontext "DocumentDraw"
                                gMouseClick 90,90
                                sleep 1
                                Kontext "Ueberblenden"
                                if GleicheOrientierung.IsChecked = FALSE then qaerrorlog "GleicheOrientierung /    was NOT checked again."
                                    sleep 1


                                    sleep 1
                                    gMouseClick 90,90
                                    gMouseClick 90,90
                                    sleep 1
                                    sleep 1
                                    try
                                        sleep 2
                                        If active = "Ueberblenden" then
                                            qaerrorlog "CrossFading should NOT be selectable since more than two objects is selected."
                                            kontext "Ueberblenden"
                                        endif
                                        sleep 2
                                        Kontext "DocumentDraw"
                                    catch
                                        printlog "CrossFading could not be selected when more than two objects selected: Correct."
                                    endcatch

                                    try
                                    catch
                                        warnlog "Could not select and delete all objects."
                                    endcatch

                                    printlog "End of test"

endcase

