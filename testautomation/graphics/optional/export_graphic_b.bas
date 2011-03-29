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

