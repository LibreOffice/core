'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: wizard_fax.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: rt $ $Date: 2008-09-04 09:16:59 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Update Test for Fax Wizard
'*
'\******************************************************************************

testcase tUpdtWizardFax

    '///<h1>Update test for the Fax Wizard</h1>
    '///<ul>

    dim iErr as integer
    dim brc as boolean
    
    dim cTemplateName as string
    dim cTemplatePath as string    
    
    ' Build the filename we want to save the template as.
    cTemplateName = "FWK-testtool-faxtemplate.ott"
    cTemplatePath = gOfficePath & "user\template\" & cTemplateName
    cTemplatePath = convertpath( cTemplatePath )   
    
    '///+<li>Make sure exactly one single writer document is open</li>
    do while( getDocumentCount > 0 ) 
        hCloseDocument()
    loop
    gApplication = "CALC"
    hNewDocument()
    
    '///+<li>open the faxwizard using the menu, page design opens</li>
    iErr = hOpenWizardWithMenu( "FAX" )
    if ( iErr = 1 ) then
        warnlog( "#i46909# Wizard crashes/does not open" )
        goto endsub
    endif
    
    kontext "DocumentCalc"
    hCloseDocument()

    '///+<li>Page 1</li>
    kontext "AutopilotFax"    
    call dialogtest2( AutopilotFax, 1 )
    hClickNextButton()

    '///+<li>Page 2</li>
    kontext "AutopilotFax"    
    call dialogtest2( AutopilotFax, 2 )
    hClickNextButton()

    '///+<li>Page 3</li>
    kontext "AutopilotFax"    
    call dialogtest2( AutopilotFax, 3 )
    hClickNextButton()

    '///+<li>Page 4</li>
    kontext "AutopilotFax"    
    call dialogtest2( AutopilotFax, 4 )
    hClickNextButton()
    
    '///+<li>Page 5</li>
    kontext "AutopilotFax"
    call dialogtest2( AutopilotFax, 5 )
    
    '///+<li>Name the template for further usage</li>
    printlog( "Name the template for further usage" )
    TemplateName.setText( cTemplateName )
    
    '///+<li>Save it (Click SaveAS-button)</li>
    hSetTemplateSavePath( cTemplatePath )
    
    '///+<li>click the 'finish' button</li>
    hFinishWizard( 1 )
    
    kontext "StandardBar"
    hWaitForObject( Speichern, 5000 )
   
    '///+<li>close the document</li>
    brc = hDestroyDocument()
    if ( not brc ) then
        qaerrorlog( "#i59233# The wizard does not display the new template" )
    endif   
    
    '///+<li>Delete the new template</li>
    printlog( "Delete the user-template: " & cTemplatePath )
    hDeleteFile( cTemplatePath )
    
    while( getDocumentCount() > 0 ) 
        hDestroyDocument()
    wend
    
    '///</ul>
    
endcase
