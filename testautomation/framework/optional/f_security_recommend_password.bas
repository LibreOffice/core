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
'*  short description : checkbox to recommend password protection 
'*
'\******************************************************************************

sub main

    dim iApplication as integer

    use "framework\optional\includes\security_recommend_password.inc"

    call hStatusIn( "framework" , "f_security_recommend_password.bas" )

    hSetPasswordRecommendation( true )

    printlog( "" )
    printlog( "**** OpenOffice.org 2.x format (OASIS) ****" )

    for iApplication =  1 to 6

        printlog( "" )
        printlog( hNumericDocType( iApplication ) )
        call tRecommendPassword( "current" )

    next iApplication

    hSetPasswordRecommendation( false )

    call hStatusOut()

end sub

'-------------------------------------------------------------------------

sub LoadIncludeFiles

    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    
    use "global\tools\includes\optional\t_security_tools.inc"
    use "global\tools\includes\optional\t_filetools.inc"
    use "global\tools\includes\optional\t_docfuncs.inc"
    
    gApplication = "WRITER"
    call GetUseFiles()

end sub

'-------------------------------------------------------------------------
