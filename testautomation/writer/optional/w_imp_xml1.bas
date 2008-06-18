'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_imp_xml1.bas,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: fredrikh $ $Date: 2008-06-18 09:11:25 $
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
'* short description : Load all files types in standard installation and save with xml-filters (6.0/7)
'*
'\***********************************************************************

sub main

    Dim Start
    Dim ImportFilterDatei as String
    Dim sCurrentFilter as string
    Dim sCurrentFilterExt as string
    Dim PrevMacroSetting as integer
    
	use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool4.inc"
	use "writer\tools\includes\w_tool6.inc"
    use "writer\loadsave\includes\w_import.inc"

    Start = Now()

    sCurrentFilter = hGetUIFiltername("StarOffice XML (Writer)")
    sCurrentFilterExt = ".sxw"
    
    PrevMacroSetting = hSetMacroSecurity(1)
    if sCurrentFilter <> "Error: No matching VALUE found for key" then    
        Call hStatusIn("writer","w_imp_xml1.bas")
        if iSprache = 46 then
            QAErrorlog "#137214#AutoSpellCheck in Swedish Text is extremly slow"
            Call wEnableAutoSpellcheck(false)
        endif        
        Call wImportWithFilter ( sCurrentFilter, sCurrentFilterExt )
        Call wImportWithoutFilter ( sCurrentFilter, sCurrentFilterExt )
        if iSprache = 46 then 
            Call wEnableAutoSpellcheck(true)
        endif        
        Call hStatusOut
    else
        Warnlog "'hGetFilter(Current)' returns an error string ! Test interrupted !"
    endif
    Call hSetMacroSecurity(PrevMacroSetting)
    printlog Chr(13) + "Duration :" + Wielange ( Start )

end sub

' ********************************************
' ** - Call Start properties
' ********************************************
sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles
    gApplication = "Writer"
end sub
