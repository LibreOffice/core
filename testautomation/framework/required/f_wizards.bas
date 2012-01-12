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
'*  short description : Category 0 tests for all included wizards
'*
'\******************************************************************************

sub main

    use "framework\required\includes\wizard_agenda.inc"
    use "framework\required\includes\wizard_documentconverter.inc"
    use "framework\required\includes\wizard_euroconverter.inc"
    use "framework\required\includes\wizard_fax.inc"
    use "framework\required\includes\wizard_firsttime.inc"
    use "framework\required\includes\wizard_letter.inc"
    use "framework\required\includes\wizard_mailmerge.inc"
    use "framework\required\includes\wizard_presentation.inc"
    use "framework\required\includes\wizard_webpage.inc"
        
    hDeleteUserTemplates()
   
    call hStatusIn( "framework" , "f_wizards.bas" )
    call tUpdtWizardFirsttime()
    call tUpdtWizardLetter()
    call tUpdtWizardPresentation()
    call tUpdtWizardMailmerge()
    call tUpdtWizardAgenda()
    call tUpdtWizardDocumentConverter()
    call tUpdtWizardEuroConverter()
    call tUpdtWizardFax()
    call tUpdtWizardWebpage()
    call hStatusOut()
    
    if ( hDeleteUserTemplates() > 0 ) then printlog( "User templates left over" )

end sub

'*******************************************************************************

sub LoadIncludeFiles

    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    
    use "global\tools\includes\optional\t_filetools.inc"
    use "global\tools\includes\optional\t_docfuncs.inc"
    use "global\tools\includes\optional\t_key_tools.inc"
    use "global\tools\includes\optional\t_stringtools.inc"
    use "global\tools\includes\optional\t_accels.inc"

    use "framework\tools\includes\wizards.inc"
    use "framework\tools\includes\template_tools.inc"
    
    gApplication = "WRITER"
    call GetUseFiles()

end sub

