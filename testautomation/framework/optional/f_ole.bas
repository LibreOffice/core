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
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : OLE
'*
'\******************************************************************************

sub main

    use "framework\optional\includes\ole_tools.inc"
    use "framework\optional\includes\ole_1.inc"
    use "framework\optional\includes\ole_2.inc"
    use "framework\optional\includes\ole_3.inc"
    
    dim iCurrentApp as integer
    
    call hStatusIn ( "framework", "f_ole.bas" )
    
    printlog( "" )
    printlog( "Quick tests of the OLE object dialog" )
    call tOLEWriter
    call tOLECalc
    call tOLEDraw
    call tOLEImpress   
    
    printlog( "" )
    printlog( "A more exhaustive test of the OLE dialog" )
    call tCheckTheOLEObjectDialog
    
    printlog( "" )
    printlog( "Do some presets required to make the following tests run smoothly" )
    call hSetToStandardView ( "WRITER" )
    call hSetToStandardView ( "GLOBALDOC" )    
     
    printlog( "" )
    printlog( "All useful combinations of applications / OLE objects" )
    
    ' Run this one in advance. Note that this function changes gApplication
    call GetOLEDefaultNames()
    
    for iCurrentApp = 1 to 6
    
        printlog( "" )
        printlog( hNumericDoctype( iCurrentApp ) )
        printlog( "" )
        
        if ( iCurrentApp = 5 ) then
            printlog( "Skipping Math" )
        else ' Testcase names as before to keep comparability to earlier testruns
            call tWriter_as_OLE_object()
            call tCalc_as_OLE_Object()
            call tDRAW_as_OLE_Object()
            call tImpress_as_OLE_Object()
            call tMath_as_OLE_Object ()
            call tChart_as_OLE_Object ()
        endif
        
    next iCurrentApp
    
    call hStatusOut()
    
end sub

'*******************************************************************************

sub LoadIncludeFiles

    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
   
    use "global\tools\includes\optional\t_filetools.inc"
    use "global\tools\includes\optional\t_ole.inc"
    use "global\tools\includes\optional\t_docfuncs.inc"
    use "global\tools\includes\optional\t_stringtools.inc"
   
    Call GetUseFiles
    gApplication   = "BACKGROUND"
    
End Sub

