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
'* short description : Impress Required (Test Part 2)
'*
'\*****************************************************************

public glLocale (15*20) as string
global ExtensionString as String

sub main
      Printlog " -------------------- Impress-Required-Test -----------------------------"
      Call hStatusIn ( "Graphics","i_updt_2.bas")

    use "graphics\tools\id_tools.inc"
    use "graphics\tools\id_tools_2.inc"
    use "graphics\required\includes\global\id_001.inc"                        'File
    use "graphics\required\includes\global\id_007.inc"                        'Kontext
    use "graphics\required\includes\global\id_008.inc"                        'Window
    use "graphics\required\includes\global\id_009.inc"                        'Help
    use "graphics\required\includes\global\id_011.inc"                        'Toolbars
    use "graphics\required\includes\impress\im_007_.inc"                   'Praesentation

   if hSetLocaleStrings ( gTesttoolPath + "graphics\tools\locale_1.txt" , glLocale () ) = FALSE then
      qaErrorLog "Locales doesn't exist in file : "+gTesttoolPath + "graphics\tools\locale_1.txt"    ' this is needed for spellchecking.
   endif

   Call tFileExportAsPDF
   Call tExportAsPDFButton
   call id_001
   Call im_007_
   call id_008
   call id_009
   call id_007
   call id_011
   call g_printing

  Call hStatusOut
end sub

'----------------------------------------------
sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   use "global\required\includes\g_001.inc"
   use "global\required\includes\g_printing.inc"
   use "global\tools\includes\optional\t_docfuncs.inc"
   use "global\tools\includes\optional\t_control_objects.inc"

   gApplication   = "IMPRESS"
    Call GetUseFiles()
end sub

