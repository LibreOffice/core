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
        hClickNextButton()
        
        Kontext "AutopilotAgenda"
        call DialogTest( AutopilotAgenda, 2 )
        hClickNextButton()
        
        Kontext "AutopilotAgenda"
        call DialogTest( AutopilotAgenda, 3 )
        hClickNextButton()

        Kontext "AutopilotAgenda"
        call DialogTest( AutopilotAgenda, 4 )
        hClickNextButton()

        Kontext "AutopilotAgenda"
        call DialogTest( AutopilotAgenda, 5 )
        hClickNextButton()

        Kontext "AutopilotAgenda"
        call DialogTest( AutopilotAgenda, 6 )
        
        printlog( "   * name the template for further usage" )
        hWaitForObject( TemplateName, 3000 )
        TemplateName.setText( cTemplateName )
        hSetTemplateSavePath( cTemplatePath )    
            
        hFinishWizard( 1 )
        
        iErr = hHandleSaveError()
        if ( iErr = 1 ) then
            kontext "AutopilotAgenda"
            hFinishWizard( 1 )
        endif    
        
        brc = hDestroyDocument()
        if ( not brc ) then
            qaerrorlog( "#i59233# The wizard does not display the new template" )
        endif
        
        do while( getDocumentCount() > 0 )
            call hCloseDocument()
        loop
        
            
        hDeleteFile( cTemplatePath )
    else
        warnlog ( "The Agenda wizard did not open/timelimit exceeded" )
    endif

endcase

