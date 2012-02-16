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
'* short description : Graphics Function: Stylist
'*
'\*******************************************************************

public glLocale (15*20) as string

sub main
    PrintLog "------------------------- Stylist Test -------------------"
    Call hStatusIn ( "Graphics","g_stylist.bas" )

    use "graphics\tools\id_tools.inc"
    use "graphics\tools\id_tools_2.inc"
    use "graphics\optional\includes\global\g_stylist.inc"
    use "graphics\optional\includes\impress\i_stylist.inc"

    if hSetLocaleStrings ( gTesttoolPath + "graphics\tools\locale_1.txt" , glLocale () ) = FALSE then
        warnlog "Locales file doesn't exist graphics\tools\locale_1.txt"
    endif

    gApplication = "IMPRESS"

    PrintLog "-------------------------" + gApplication + "-------------------"
    Call tFormatStylistFlyer
    Call tFormatStylistDocuments
    Call tFormatStylistTemplatesUse
    Call tFormatStylistTemplateCreate
    if iSprache <> "82" then
        Call tFormatStylistTemplateSelect
        Call tFormatStylistTemplateNew
    else
        qaerrorlog "#i65490# - Deactivated tFormatStylistTemplateSelect, tFormatStylistTemplateNew due to bug."
    endif
    call tiFormatStylist
    call tFormatStylist

    if iSprache <> "81" then
        if iSprache <> "82" then
            Call tFormatStylistBackground
        else
            qaerrorlog "#i65534# - Deactivated tFormatStylistBackground due to bug under Korean."
        endif
    else
        qaerrorlog "Deactivated tFormatStylistBackground due to string-name-difficulties."
    endif

    gApplication = "DRAW"
    PrintLog "-------------------------" + gApplication + "-------------------"

    Call tFormatStylistFlyer
    Call tFormatStylistDocuments
    Call tFormatStylistTemplatesUse
    '    Call tFormatStylistTemplateCreate
    if iSprache <> "82" then
        Call tFormatStylistTemplateSelect
        Call tFormatStylistTemplateNew
    else
        qaerrorlog "#i65490# - Deactivated tFormatStylistTemplateSelect, tFormatStylistTemplateNew due to bug."
    endif
    call tiFormatStylist
    call tFormatStylist

    Call hStatusOut
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    gApplication = "IMPRESS"
    Call GetUseFiles
end sub
