'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: g_spellcheck.bas,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: jsk $ $Date: 2008-06-20 08:03:54 $
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
'* short description : Graphics Function: Tools - Spellcheck
'*
'\******************************************************************

public glLocale (15*20) as string

sub main
    PrintLog "------------------------- g_spellcheck test -------------------------"
    Call hStatusIn ( "Graphics","g_spellcheck.bas")

    use "graphics\tools\id_tools.inc"
    use "graphics\tools\id_tools_2.inc"
    use "graphics\optional\includes\global\g_spellcheck.inc"

    hSetLocaleStrings ( gTesttoolPath + "graphics\tools\locale_1.txt" , glLocale () )

    PrintLog "-------------------------" + gApplication + "-------------------"
    Call tiToolsSpellcheckCorrect
    Call tiToolsSpellcheckError
    Call tiToolsSpellcheckCheck
    Call tToolsSpellcheckAutoSpellcheck

    gApplication = "DRAW"
    PrintLog "-------------------------" + gApplication + "-------------------"
    Call tiToolsSpellcheckCorrect
    Call tiToolsSpellcheckError
    Call tiToolsSpellcheckCheck
    Call tToolsSpellcheckAutoSpellcheck

    Call hStatusOut
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\tools\includes\optional\t_extension_manager_tools.inc"
    gApplication = "IMPRESS"
    Call GetUseFiles
end sub
