'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: wizard_agenda.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: rt $ $Date: 2008-08-01 09:48:01 $
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
'*  short description : Update test for agenda wizard
'*
'\******************************************************************************

testcase tUpdtWizardAgenda

    '///<H1>Update test for agenda wizard</H1>
    '///<ul>
    dim iErr as integer
    dim brc as boolean
    
    dim cTemplateName as string
    dim cTemplatePath as string    
    
    ' Build the filename we want to save the template as.
    cTemplateName = "FWK-Testtool-Template-AgendaWizard.ott"
    cTemplatePath = gOfficePath & "user\template\" & cTemplateName
    cTemplatePath = convertpath( cTemplatePath )    
    
    '///+<li>Make sure exactly one single writer document is open</li>
    hInitSingleDoc()     

    '///+<li>open the Agenda-wizard using the menu, page design opens</li>
    iErr = hOpenWizardWithMenu( "AGENDA" )
    if ( iErr = 1 ) then
        warnlog( "#i46909# Wizard crashes/does not open" )
        goto endsub
    endif  
    
    '///+<li>First page</li>
    printlog( " * Page 1 : Page Design" )
    Kontext "AutopilotAgenda"
    call dialogtest2( AutopilotAgenda, 1 )
    PageDesignList.select( 3 )
    hClickNextButton()
    
    '///+<li>Second page</li>
    Kontext "AutopilotAgenda"
    call dialogtest2( AutopilotAgenda, 2 )
    hClickNextButton()
    
    '///+<li>Third page</li>
    Kontext "AutopilotAgenda"
    call dialogtest2( AutopilotAgenda, 3 )
    hClickNextButton()

    '///+<li>Fourth page</li>
    Kontext "AutopilotAgenda"
    call dialogtest2( AutopilotAgenda, 4 )
    hClickNextButton()

    '///+<li>Fifth page</li>
    Kontext "AutopilotAgenda"
    call dialogtest2( AutopilotAgenda, 5 )
    hClickNextButton()

    '///+<li>sixth page</li>
    Kontext "AutopilotAgenda"
    call dialogtest2( AutopilotAgenda, 6 )
    
    '///+<li>name the template for further usage</li>
    printlog( "   * name the template for further usage" )
    hWaitForObject( TemplateName, 3000 )
    TemplateName.setText( cTemplateName )
    hSetTemplateSavePath( cTemplatePath )    
        
    '///+<li>Click Finish-Button</li>
    hFinishWizard( 1 )
    
    '///+<li>Handle possible File-Save error</li>
    iErr = hHandleSaveError()
    if ( iErr = 1 ) then
        kontext "AutopilotAgenda"
        hFinishWizard( 1 )
    endif    
    
    '///+<li>Close the new template</li>
    brc = hDestroyDocument()
    if ( not brc ) then
        qaerrorlog( "#i59233# The wizard does not display the new template" )
    endif
    
    do while( getDocumentCount() > 0 )
        call hCloseDocument()
    loop
    
        
    '///+<li>delete the new template</li>
    hDeleteFile( cTemplatePath )
    '///</ul>

endcase

