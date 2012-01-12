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
' *
' * short description : Presentation-Engine Optional Test
' *
' \******************************************************************

global glLocale (15*20) as string

sub main
    Call hStatusIn ( "Graphics","i_pengine.bas" )

    use "graphics\tools\id_tools.inc"
    use "graphics\tools\id_tools_2.inc"
    use "graphics\optional\includes\impress\i_pengine1.inc"
    use "graphics\optional\includes\impress\i_pengine2.inc"
    use "graphics\optional\includes\impress\options.inc"


    PrintLog "------------------------- " + gApplication + " Presentation-Engine-Optional Test -------------------"

    Call tiPengineAllShapesAndEffects
    Call tiPengine2ObjectsGetsEffects

    Call tiPengineTabEntrance
    Call tiPengineTabpageEntrance2

    if bAsianLan <> TRUE then
        Call tiPengineTabEmphasis
        Call tiPengineTabPageEmphasis2
        Call tiPengineTabExit
        Call tiPengineTabPageExit2
        Call tiPengineTabMotionPaths
        Call tiPengineTabPageMotionPaths2
        Call tiPengineAnimationEffectsOptions
    else
        qaerrorlog "Deactivated tiPengineAnimationEffectsSlideshow and tiPengineAnimationEffectsOptions, due to strange results under Asian language."
    end if

    Call tiPengineAnimationEffectsPreview
    Call tiPengineSlideTransition

    Call hStatusOut
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\tools\includes\optional\t_ui_filters.inc"
    gApplication = "IMPRESS"
    Call GetUseFiles
end sub
