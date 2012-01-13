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

*********
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


