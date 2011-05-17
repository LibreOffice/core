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
'*  short description : Small helpers for Excel BASIC import modes
'*
'\******************************************************************************

function hSetExcelBasicImportMode( bImport as boolean, bEnable as boolean )

    printlog( "Changing Excel VBA import modes" )

    ToolsOptions
    hToolsOptions( "LoadSave", "VBAProperties" )
   
    if ( bImport ) then
        ExcelBasicLaden.check()
        if ( bEnable ) then
            ExecutableCode.check()
        else
            ExecutableCode.uncheck()
        endif
    else
        ExcelBasicLaden.uncheck()
    endif
    
    Kontext "OptionenDlg"
    OptionenDlg.ok()

end function

'*******************************************************************************

function hSetExcelImportModeDefault()

    printlog( "Setting Excel VBA import modes to default" )

    ToolsOptions
    hToolsOptions( "LoadSave", "VBAProperties" )
    
    ExcelBasicLaden.check()
    ExecutableCode.uncheck()
    
    Kontext "OptionenDlg"
    OptionenDlg.ok()

end function
