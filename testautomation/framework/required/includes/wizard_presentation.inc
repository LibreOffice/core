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
'*  short description : Update test for the presentation wizard
'*
'\******************************************************************************

testcase tUpdtWizardPresentation

    printlog( "Resource test for the presentation wizard" )

    dim irc as integer
    dim brc as boolean
    
    hInitSingleDoc()
    
    FileAutopilotPresentation
    kontext "AutopilotPraesentation1"
    if ( AutopilotPraesentation1.exists( 2 ) ) then
    
        
        printlog( "Page 1" )
        call Dialogtest( AutopilotPraesentation1 )
        hClickButton( NextButton )
        
        printlog( "Page 2" )
        kontext "AutopilotPraesentation2"
        call DialogTest( AutopilotPraesentation2 )
        hClickButton( NextButton )
        
        printlog( "Page 3" )
        kontext "AutopilotPraesentation3" 
        call DialogTest( AutopilotPraesentation3 )

        kontext "AutopilotPraesentation3" 
        hCloseDialog( AutopilotPraesentation3, "ok" )
    else
        warnlog( "Presentation wizard not open/exceeded timeout" )
    endif
    
    hFileCloseAll()

    gApplication = "WRITER"

endcase

