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
'* short description : (long duration! > 6h) Testing all Gallery items
'*
'\***********************************************************************

dim iSaveSetting as Integer
sub main
    call hStatusIn ( "Graphics", "gallery2.bas" )

    use "graphics\tools\id_tools.inc"
    use "graphics\tools\id_tools_2.inc"
    use "graphics\optional\includes\global\gallery2.inc"

    printlog "------------------------------- T h e m e s -----------------------------"
    call tSettingsToCM
    call tCheckAllGalleryGraphicFiles
    call tLoad100GalleryGraphicFiles
    call tInsertGalleryObjects
    call tResetSettings

    call hStatusOut
end sub

'-------------------------------------------------------------------------

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    gApplication = "WRITER"
    call GetUseFiles
end sub

'-------------------------------------------------------------------------


