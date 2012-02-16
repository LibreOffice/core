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
'* short description : Graphics Export Test - B-tests. (the usual suspects)
'*
'\************************************************************************

global OutputGrafikTBO as string
global Document as string
global ExtensionString as String

sub main
    Printlog "--------------------- Graphics Export Test ------------------- "
    Call hStatusIn ( "Graphics","export_graphic_b.bas" )

    use "graphics\tools\id_tools.inc"
    use "graphics\tools\id_tools_2.inc"
    use "graphics\optional\includes\global\export_graphic_2.inc"

    if bAsianLan then
        qaErrorLog "#i71631# - Test deactivated with Asian languages due to bug"
        Call hStatusOut
        exit sub
    endif

    Call sFileExport 'OBSERVE - This line must run before the following testcases!
    Call tPDF   ' Portable Document Format  
    Call tPDF_Creator  'test if the right creator is set in the PDF document
    Call tSVG   'Scalable Vector Graphics
    Call tSVM   'StarView Metafile                 
    Call tBMP   'Windows Bitmap                    
    Call tEMF   'Enhanced Metafile                 
    Call tPNG   'Portable Network Graphic          
    Call tMET   'OS/2 Metafile                     
    Call tWMF   'Windows Metafile                  
    Call tSWF   'Macromedia Flash (SWF) (.swf)     

    Call hStatusOut
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles
    gApplication = "IMPRESS"
end sub

