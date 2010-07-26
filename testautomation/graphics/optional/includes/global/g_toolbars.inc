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
'* short description :
'*
'*********************************************************************
' #1 Toolboxen_Rechtecke
' #1 Toolboxen_Kreise
' #1 Toolboxen_3dObjekte
' #1 Toolboxen_Linien
' #1 Toolboxen_Kurven
'\********************************************************************


testcase Toolboxen_Rechtecke
    Printlog "- Toolbox rectangles"
    Dim i


    Printlog "- 1.: Create rectangles, apply shadow and move them around"

    for i = 1 to 4
        select case i
        case 1 : Printlog "- Create Rectangle Filled"
            sleep 1
        case 2 : Printlog "- Create Rectangle Round-Filled"
            WL_SD_RechteckRundVoll
            sleep 1
        case 3 : Printlog "- Create Quadrat Filled"
            WL_SD_QuadratVoll
            sleep 1
        case 4 : Printlog "Create Quadrat Round-Filled"
            WL_SD_QuadratRundVoll
            sleep 1
        end select

        Kontext "DocumentImpress"
        sleep 2
        Kontext
        Kontext "TabSchatten"
        sleep 1
        Kontext "DocumentImpress"


        gMouseClick 30,30
        gMouseMove 30,30,90,90

        gMouseClick 90,90
        gMouseMove 90,90,30,30

        gMouseClick 30,30
        gMouseMove 30,30,90,90

        gMouseClick 90,90
        gMouseMove 90,90,30,30

        sleep 2
    next i
endcase

'**********************************************************************
testcase Toolboxen_Kreise
    Printlog "- Toolbox Circles"

    Dim i



    Printlog "- Create circles and ellipses"
    for i = 1 to 7
    case 1 : WL_SD_Ellipsen
        sleep 1
        Kontext "DocumentImpress"
        gMouseMove 20,20,60,60
    case 2 : WL_SD_EllipseVoll
        sleep 1
        Kontext "DocumentImpress"
        gMouseMove 20,20,60,60
    case 3 : WL_SD_EllipsensegmentVoll
        sleep 1
        kontext "DocumentImpress"
        gMouseMove 20,20,40,40
        gMouseClick 20,30
        gMouseClick 20,28
    case 4 : WL_SD_EllipsenabschnittVoll
        sleep 1
        kontext "DocumentImpress"
        gMouseMove 20,20,40,40
        gMouseClick 20,30
        gMouseClick 20,28
    case 5 : WL_SD_KreisVoll
        sleep 1
        kontext "DocumentImpress"
        gMouseMove 20,20,40,40
    case 6 : WL_SD_KreissegmentVoll
        sleep 1
        kontext "DocumentImpress"
        gMouseMove 20,20,40,40
        gMouseClick 20,30
        gMouseClick 20,28
    case 7 : WL_SD_KreisabschnittVoll
        sleep 1
        kontext "DocumentImpress"
        gMouseMove 20,20,40,40
        gMouseClick 20,30
        gMouseClick 20,28
    end select

    FormatArea
    Kontext
    Kontext "TabSchatten"
    Anzeigen.Click
    TabSchatten.OK
    sleep 1
    Kontext "DocumentImpress"
    gMouseClick 30,30

    gMouseClick 90,90
    gMouseMove 90,90,30,30

    gMouseClick 30,30
    gMouseMove 30,30,90,90

    gMouseClick 90,90
    gMouseMove 90,90,30,30

    gMouseClick 30,30
    gMouseMove 30,30,90,90

    gMouseClick 90,90
    gMouseMove 90,90,30,30

    sleep 2
next i
endcase
'***********************************************************************************
testcase Toolboxen_3dObjekte
    Printlog "- Toolbox 3D objects"
    Dim i


    Printlog "- Create 3D objects"
    for i = 1 to 8
        printlog "doing now: "+i
        select case i
            sleep 1
            Kontext "DocumentImpress"
            gMouseMove 20,20,60,60
        case 2 : WL_SD_Kugel
            sleep 1
            Kontext "DocumentImpress"
            gMouseMove 20,20,60,60
        case 3 : WL_SD_Zylinder
            sleep 1
            kontext "DocumentImpress"
            gMouseMove 20,20,40,40
        case 4 : WL_SD_Kegel
            sleep 1
            kontext "DocumentImpress"
            gMouseMove 20,20,40,40
        case 5 : WL_SD_Pyramide
            sleep 1
            kontext "DocumentImpress"
            gMouseMove 20,20,40,40
        case 6 : WL_SD_Torus
            sleep 1
            kontext "DocumentImpress"
            gMouseMove 20,20,40,40
        case 7 : WL_SD_Schale
            sleep 1
            kontext "DocumentImpress"
            gMouseMove 20,20,40,40
        case 8 : WL_SD_Halbkugel
        end select
        sleep 2
        FormatArea
        sleep 2
        Kontext
        Kontext "TabSchatten"
        Anzeigen.Click
        TabSchatten.OK
        sleep 1
        Kontext "DocumentImpress"
        gMouseClick 30,30
        gMouseClick 90,90
        gMouseMove 90,90,30,30
        gMouseClick 30,30
        gMouseMove 30,30,90,90
        gMouseClick 90,90
        gMouseMove 90,90,30,30
        gMouseClick 30,30
        gMouseMove 30,30,90,90
        gMouseClick 90,90
        gMouseMove 90,90,30,30
        sleep 2
    next i
    gMouseClick 99,99
    sleep 1
    Printlog "- 3D-body created"
endcase
'******************************************************************************
testcase Toolboxen_Linien
    Printlog "- Toolbox Lines "
    Dim i
    Printlog "- Create lines and arrows"
    for i = 1 to 10
        select case i
            sleep 1
            Kontext "DocumentImpress"
            gMouseMove 20,20,60,60
        case 2 : WL_SD_LinieMitPfeilende
            sleep 1
            Kontext "DocumentImpress"
            gMouseMove 20,20,60,60
        case 3 : WL_SD_LinieMitPfeilKreisende
            sleep 1
            kontext "DocumentImpress"
            gMouseMove 20,20,40,40
        case 4 : WL_SD_LiniemitPfeilQuadratende
            sleep 1
            kontext "DocumentImpress"
            gMouseMove 20,20,40,40
        case 5 : WL_SD_Liniewinkel
            sleep 1
            kontext "DocumentImpress"
            gMouseMove 20,20,40,40
        case 6 : WL_SD_LinieMitPfeilanfang
            sleep 1
            kontext "DocumentImpress"
            gMouseMove 20,20,40,40
        case 7 : WL_SD_LinieMitKreisPfeilende
            sleep 1
            kontext "DocumentImpress"
            gMouseMove 20,20,40,40
        case 8 : WL_SD_LiniemitPfeilKreisende

        case 9 : WL_SD_Masslinie
            sleep 1
            kontext "DocumentImpress"
            gMouseMove 20,20,40,40
        case 10 : WL_SD_LinieMitPfeilenden

        end select
        Kontext "DocumentImpress"
        gMouseClick 25,25
        sleep 1
        sleep 1
        gMouseMove 80,80,25,25
        sleep 1
        gMouseMove 25,25,90,90
        sleep 1
        gMouseClick 90,90,25,25
        sleep 1
        gMouseClick 99,99
        sleep 1
    next i
    Call hCloseDocument
endcase
'********************************************************************
testcase Toolboxen_Kurven
    Printlog "- Toolbox curves"
    Dim i
    Call hNewDocument
    sleep 1
    for i = 1 to 8
        select case i
        case 1 : WL_SD_KurveVoll
            sleep 1
            gMouseMove 20,20,60,60
            gMouseClick 70,70
            gMouseMove 70,70,30,30
            gMouseDoubleClick 30,30
        case 2 : WL_SD_KurveLeer
            sleep 1
            gMouseMove 20,20,60,60
            gMouseClick 70,70
            gMouseMove 70,70,30,30
            gMouseDoubleClick 30,30
        case 3 : WL_SD_PolygonVoll
            sleep 1
            gMouseMove 20,20,60,60
            gMouseClick 70,70
            gMouseClick 10,10
            gMouseClick 90,90
            gMouseClick 50,50
            gMouseDoubleClick 50,50
        case 4 : WL_SD_Polygon
            sleep 1
            gMouseMove 20,20,60,60
            gMouseClick 70,70
            gMouseClick 10,10
            gMouseClick 90,90
            gMouseClick 50,50
            gMouseDoubleClick 50,50
        case 5 : WL_SD_PolygonWinkelVoll
            sleep 1
            gMouseMove 20,20,60,60
            gMouseClick 70,70
            gMouseClick 10,10
            gMouseClick 90,90
            gMouseClick 50,50
            gMouseDoubleClick 50,50
        case 6 : WL_SD_PolygonWinkel
            sleep 1
            gMouseMove 20,20,60,60
            gMouseClick 70,70
            gMouseClick 10,10
            gMouseClick 90,90
            gMouseClick 50,50
            gMouseDoubleClick 50,50
        case 7 : WL_SD_FreihandlinieVoll
            sleep 1
            gMouseMove 10,10,70,70
            gMouseMove 20,20,30,80
        case 8 : WL_SD_FreihandlinieLeer
            sleep 1
            gMouseMove 10,10,70,70
            gMouseMove2 70,70,20,20
            gMouseMove 20,20,30,80
        end select
        Kontext "DocumentImpress"
        sleep 1
        sleep 1
        try
        catch
            Warnlog "- Nothing in the document to cut out"
        endcatch
        sleep 1
    next i
endcase


