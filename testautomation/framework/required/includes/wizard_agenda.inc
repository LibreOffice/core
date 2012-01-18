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
'*  short description : Update test for agenda wizard
'*
'\******************************************************************************

testcase tUpdtWizardAgenda

    printlog( "Resource test for the agenda wizard" )

    dim iErr as integer
    dim brc as boolean
    
    dim cTemplateName as string
    dim cTemplatePath as string    
    
    ' Build the filename we want to save the template as.
    cTemplateName = "FWK-Testtool-Template-AgendaWizard.ott"
    cTemplatePath = gOfficePath & "user\template\" & cTemplateName
    cTemplatePath = convertpath( cTemplatePath )    
    
    hInitSingleDoc()     

    FileAutopilotAgenda
    kontext "AutopilotAgenda"
    if ( AutopilotAgenda.exists( 2 ) ) then
    
        printlog( " * Page 1 : Page Design" )
        Kontext "AutopilotAgenda"
        call DialogTest( AutopilotAgenda, 1 )
        PageDesignList.select( 3 )
        hClickButton( NextButton )
        
        Kontext "AutopilotAgenda"
        call DialogTest( AutopilotAgenda, 2 )
        hClickButton( NextButton )
        
        Kontext "AutopilotAgenda"
        call DialogTest( AutopilotAgenda, 3 )
        hClickButton( NextButton )

        Kontext "AutopilotAgenda"
        call DialogTest( AutopilotAgenda, 4 )
        hClickButton( NextButton )

        Kontext "AutopilotAgenda"
        call DialogTest( AutopilotAgenda, 5 )
        hClickButton( NextButton )

        Kontext "AutopilotAgenda"
        call DialogTest( AutopilotAgenda, 6 )
        
        printlog( "   * name the template for further usage" )
        hWaitForObject( TemplateName, 3000 )
        TemplateName.setText( cTemplateName )
        hSetTemplateSavePath( cTemplatePath )    

        kontext "AutopilotAgenda"
        hClickButton( CreateButton )
        
        if ( hHandleSaveError() = 1 ) then
            kontext "AutopilotAgenda"
            hClickButton( CreateButton )
        endif    
        
        if ( not hDestroyDocument() ) then qaerrorlog( "#i59233# The wizard does not display the new template" )

        hFileCloseAll()
        hDeleteFile( cTemplatePath )
    else
        warnlog ( "The Agenda wizard did not open/timelimit exceeded" )
    endif

endcase

