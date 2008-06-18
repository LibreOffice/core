'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_exp_xml1.bas,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: fredrikh $ $Date: 2008-06-18 20:00:33 $
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
'* owner : fredrik.haegg@sun.com
'*
'* short description : Checks the export to all available Filters from sxw-files
'*
'\***********************************************************************

sub main
    Dim Start
    Dim ExportFilterDatei as String, PrevMacroSetting as integer

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool6.inc"
    use "writer\optional\includes\w_export.inc"

    Start = Now()

    PrevMacroSetting = hSetMacroSecurity(1)
    Call hStatusIn ( "writer", "w_exp_xml1.bas")
    Call tExportListCompare
    Call tExportFile("sxw")
    Call hStatusOut
    Call hSetMacroSecurity(PrevMacroSetting)

    Printlog Chr(13) + "Duration : " + Wielange ( Start )
end sub

' ********************************************
' ** - Starteinstellungen aufrufen
' ********************************************
sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles
    gApplication = "Writer"
end sub
