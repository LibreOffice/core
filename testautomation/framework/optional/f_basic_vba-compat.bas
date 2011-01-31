'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
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
'/******************************************************************************
'*
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Test VBA import options and compatibility switches
'*
'\******************************************************************************

sub main

    use "framework\optional\includes\basic_vba-compat_import_nothing.inc"
    use "framework\optional\includes\basic_vba-compat_import_disabled.inc"
    use "framework\optional\includes\basic_vba-compat_import_enabled.inc"
    use "framework\optional\includes\basic_vba-compat_thisworkbook.inc"
    use "framework\optional\includes\basic_vba-compat_application-union.inc"
    use "framework\optional\includes\basic_vba-compat_xlsm-xlsb.inc"
    
    call hStatusIn( "framework" , "f_basic_vba-compat.bas" )
    call tBasicVBACompatImportNothing()
    call tBasicVBACompatImportDisabled()
    call tBasicVBACompatImportEnabled()
    call tBasicVBACompatThisWorkBook()
    call tBasicVBACompatApplicationUnion()
    call tBasicVBACompatXLSM_XLSB()
    call hStatusOut()

end sub

'*******************************************************************************

sub LoadIncludeFiles

    use "global\system\includes\gvariabl.inc"
    use "global\system\includes\master.inc"

    use "global\tools\includes\optional\t_basic_organizer_tools.inc"
    use "global\tools\includes\optional\t_treelist_tools.inc"
    use "global\tools\includes\optional\t_stringtools.inc"
    use "global\tools\includes\optional\t_security_tools.inc"
    use "global\tools\includes\optional\t_macro_tools.inc"
    use "global\tools\includes\optional\t_docfuncs.inc"
    use "global\tools\includes\optional\t_user_info.inc"

    use "framework\optional\includes\basic_vba_compat_tools.inc"
    
    gApplication = "WRITER"
    call GetUseFiles()

end sub

