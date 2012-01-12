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
'* short description : Graphics Export Test - A-tests. (More durable ones)
'*
'\************************************************************************

global OutputGrafikTBO as string
global Document as string
global ExtensionString as String

sub main
    Printlog "--------------------- Graphics Export Test ------------------- "
    Call hStatusIn ( "Graphics","export_graphic_a.bas" )

    use "graphics\tools\id_tools.inc"
    use "graphics\tools\id_tools_2.inc"
    use "graphics\optional\includes\global\export_graphic.inc"

    if bAsianLan then
        qaErrorLog "#i71631# - Test deactivated with Asian languages due to bug"
        Call hStatusOut
        exit sub
    endif

    Call sFileExport 'OBSERVE - This line must run before the following testcases!
    Call tEPS   'Encapsulated PostScript        
    Call tPCT   'Mac Pict                       
    Call tPBM   'Portable Bitmap                
    Call tPGM   'Portable Graymap               
    Call tPPM   'Portable Pixelmap              
    Call tRAS   'Sun Raster Image               
    Call tTIFF  'Tagged Image File Format       
    Call tXPM   'X PixMap                       
    Call tGIF   'Graphics Interchange Format    
    Call tJPEG  'Joint Photographic Experts Group
    Call tSVG   'Scalable Vector Graphics

    Call hStatusOut
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles
    gApplication = "IMPRESS"
end sub

