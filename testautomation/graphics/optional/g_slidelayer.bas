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
'* short description : Graphics Function: Slide and Layer
'*
'\******************************************************************

'Variables:
global ExtensionString as String

sub main
    PrintLog "------------------------- g_slidelayer test -------------------------"
    Call hStatusIn ( "Graphics","g_slidelayer.bas")

    use "graphics\tools\id_tools.inc"
    use "graphics\tools\id_tools_2.inc"
    use "graphics\optional\includes\global\g_slidelayer.inc"
    use "graphics\optional\includes\impress\i_slidelayer.inc"

    PrintLog "-------------------------" + gApplication + "-------------------"
    Call tInsertSlide
    Call tInsertDuplicateSlide
    Call tiInsertExpandSlide  ' only in impress
    Call tiInsertSummarySlide  ' only in impress
    Call tiFormatSeitenlayout  ' only in impress
    Call t114174  ' only in impress
    Call t111862

    gApplication = "DRAW"
    PrintLog "-------------------------" + gApplication + "-------------------"
    Call tInsertSlide
    Call tInsertDuplicateSlide
    Call tInsertLayer ' in impress not here, instead Edit->Layer->Insert

    Call hStatusOut
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    gApplication = "IMPRESS"
    Call GetUseFiles
end sub
