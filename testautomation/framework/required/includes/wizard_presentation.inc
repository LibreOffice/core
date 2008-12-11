'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: wizard_presentation.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: rt $ $Date: 2008-08-01 09:48:58 $
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
'*  short description : Update test for the presentation wizard
'*
'\******************************************************************************

testcase tUpdtWizardPresentation

    '///<H1>Update test for the presentation wizard</H1>
    '///<ul>
    dim irc as integer
    dim brc as boolean
    
    '///+<li>Make sure exactly one single writer document is open</li>
    hInitSingleDoc()
    
    '///+<li>Open the presentation wizard</li>
    FileAutopilotPresentation
    kontext "AutopilotPraesentation1"
    if ( AutopilotPraesentation1.exists( 2 ) ) then
    
        
        '///+<li>Look at page 1</li>
        printlog( "Page 1" )
        call Dialogtest( AutopilotPraesentation1 )
        
        hClickNextButton()
        
        '///+<li>Look at page 2</li>
        printlog( "Page 2" )
        kontext "AutopilotPraesentation2"
        call DialogTest( AutopilotPraesentation2 )
        
        hClickNextButton()
        
        '///+<li>Look at page 3</li>
        printlog( "Page 3" )
        kontext "AutopilotPraesentation3" 
        call DialogTest( AutopilotPraesentation3 )
        
        '///+<li>Finish the wizard</li>
        hFinishWizard( 1 )
    else
        warnlog( "Presentation wizard not open/exceeded timeout" )
    endif
    
    '///+<li>Close the document(s)</li>
    do while( getDocumentCount() > 0 )
        call hCloseDocument()
    loop

    gApplication = "WRITER"
    '///</ul>

endcase

