'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: fileopen.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:03:20 $
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
'/******************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : CWS oxtsysint01 enables installing extensions via file open dialog
'*
'\******************************************************************************

testcase tExtensionFileOpen

    printlog( "Install an extension via File Open - new with CWS oxtsysint01" )
    
    if ( gBuild < 9305 ) then
        printlog( "No testing for builds prior to 9305"
        goto endsub
    endif

    const EXTENSION_NAME = "ShortLicense.oxt"
    dim cPath as string 
        cPath = gTesttoolPath & "extensions/optional/input/simpleLicense/" & EXTENSION_NAME
        
    hFileOpen( cPath )
    
    kontext "Active"
    if ( Active.exists() ) then
        printlog( "Extension installation message found, accepting" )
        printlog( Active.getText() )
        Active.ok()
    else
        warnlog( "Extension installation message missing" )
    endif
    
    kontext "ExtensionSoftwareLicenseAgreement"
    if ( ExtensionSoftwareLicenseAgreement.exists( 2 ) ) then
        printlog( "License found. Accepting" )
        accept.click()
    else
        warnlog ( "License dialog is missing" )
    endif

    hExtensionRemoveGUI( EXTENSION_NAME )    
    
endcase


