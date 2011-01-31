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
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Test scripting-organizers / document-stored scripts
'*
'\******************************************************************************

sub main

    use "framework\optional\includes\scripting_organizers.inc"
    
    dim iDialog as integer ' allowed: 1 (BeanShell) and/or 2 (JavaScript), 3 for python
    dim iApp as integer    ' Numeric expr. for Doc-Type: Writer, Calc, Draw...
    dim cApp as string     

    call hStatusIn( "framework" , "f_scripting_organizers.bas" )

    ' Note: For performace reasons we only test a few combinations of docs/dialogs
    for iApp = 2 to 6 step 2
    
        cApp = hNumericDocType( iApp )
        printlog( "" )
        printlog( cApp )
        
        for iDialog = 1 to 2 ' Python currently not tested, needs more adjustments
            call tScriptingOrganizers( iDialog )
        next iDialog
        
    next iApp
    
    hSetmacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_DEFAULT )

    call hStatusOut()

end sub

'-------------------------------------------------------------------------

sub LoadIncludeFiles

    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    
    use "global\tools\includes\optional\t_docfuncs.inc"
    use "global\tools\includes\optional\t_filetools.inc"
    use "global\tools\includes\optional\t_treelist_tools.inc"
    use "global\tools\includes\optional\t_control_objects.inc"
    use "global\tools\includes\optional\t_security_tools.inc"
    
    use "framework\tools\includes\scriptorganizer_tools.inc"
    
    gApplication = "WRITER"
    call GetUseFiles()

end sub

'-------------------------------------------------------------------------
