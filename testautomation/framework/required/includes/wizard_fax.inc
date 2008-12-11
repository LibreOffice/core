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

    dim iErr as integer
    dim brc as boolean
    
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
        call dialogtest2( AutopilotFax, 1 )
        hClickNextButton()

        kontext "AutopilotFax"    
        call dialogtest2( AutopilotFax, 2 )
        hClickNextButton()

        kontext "AutopilotFax"    
        call dialogtest2( AutopilotFax, 3 )
        hClickNextButton()

        kontext "AutopilotFax"    
        call dialogtest2( AutopilotFax, 4 )
        hClickNextButton()
        
        kontext "AutopilotFax"
        call dialogtest2( AutopilotFax, 5 )
        
        printlog( "Name the template for further usage" )
        TemplateName.setText( cTemplateName )
        
        hSetTemplateSavePath( cTemplatePath )
        
        hFinishWizard( 1 )
        
        kontext "StandardBar"
        hWaitForObject( Speichern, 5000 )
       
        brc = hDestroyDocument()
        if ( not brc ) then
            qaerrorlog( "#i59233# The wizard does not display the new template" )
        endif   
        
        printlog( "Delete the user-template: " & cTemplatePath )
        hDeleteFile( cTemplatePath )
        
    else
        warnlog( "Fax wizard did not open/exceeded timeout" )
    endif
        
    while( getDocumentCount() > 0 ) 
        hDestroyDocument()
    wend
    
    '///</ul>
    
endcase
