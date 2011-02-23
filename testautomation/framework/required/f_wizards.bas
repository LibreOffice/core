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

