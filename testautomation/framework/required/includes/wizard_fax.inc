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
'*  short description : Update Test for Fax Wizard
'*
'\******************************************************************************

testcase tUpdtWizardFax

    printlog( "Resource test for the fax wizard" )

    dim cTemplateName as string
    dim cTemplatePath as string    
    
    ' Build the filename we want to save the template as.
    cTemplateName = "FWK-testtool-faxtemplate.ott"
    cTemplatePath = gOfficePath & "user\template\" & cTemplateName
    cTemplatePath = convertpath( cTemplatePath )  
    
    FileAutopilotFax

    kontext "AutopilotFax"
    if ( AutopilotFax.exists( 2 ) ) then
    
        kontext "AutopilotFax"    
        call DialogTest( AutopilotFax, 1 )
        hClickButton( NextButton )

        kontext "AutopilotFax"    
        call DialogTest( AutopilotFax, 2 )
        hClickButton( NextButton )

        kontext "AutopilotFax"    
        call DialogTest( AutopilotFax, 3 )
        hClickButton( NextButton )

        kontext "AutopilotFax"    
        call DialogTest( AutopilotFax, 4 )
        hClickButton( NextButton )
        
        kontext "AutopilotFax"
        call DialogTest( AutopilotFax, 5 )
        
        printlog( "Name the template for further usage" )
        TemplateName.setText( cTemplateName )
        
        hSetTemplateSavePath( cTemplatePath )

        kontext "AutopilotFax"
        hClickButton( FinishButton )
        
        kontext "StandardBar"
        hWaitForObject( Speichern, 5000 )
       
        if ( not hDestroyDocument() ) then qaerrorlog( "#i59233# The wizard does not display the new template" )
        
        printlog( "Delete the user-template: " & cTemplatePath )
        hDeleteFile( cTemplatePath )
        
    else
        warnlog( "Fax wizard did not open/exceeded timeout" )
    endif
        
    hFileCloseAll()
    
    
endcase
