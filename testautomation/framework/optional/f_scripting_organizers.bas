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
