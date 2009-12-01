'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: i_us_presentation.bas,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:38 $
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
'* Owner : wolfram.garten@sun.com
'*
'* short description : Graphics User-scenario-test: 2 Users creates Presentations.
'*
'\******************************************************************
public i as integer

sub main
    Call hStatusIn ( "Graphics","i_us_presentation.bas")
    
    use "graphics\tools\id_tools.inc"
    use "graphics\tools\id_tools_2.inc"
    use "graphics\optional\includes\impress\i_us_present.inc"
    use "graphics\optional\includes\impress\i_us2_present.inc"

    PrintLog "------------ Graphics User-scenario-test: PowerUser creates a Presentation ------------"

        Call i_us_presentation1  ' User-Scenario: Pro.
        Call i_us_presentation2
        Call i_us_presentation3
        Call i_us_presentation4
        Call i_us_presentation5
        Call i_us_presentation6
        Call i_us_presentation7

        Call i_us2_pres1         ' User-Scenario: Beginner.
        Call i_us2_pres2
        
    Call hStatusOut
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    gApplication = "IMPRESS"
    Call GetUseFiles
end sub
