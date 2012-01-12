'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
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

