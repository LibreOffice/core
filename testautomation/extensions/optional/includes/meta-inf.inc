'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: meta-inf.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: jsk $ $Date: 2008-06-19 09:02:10 $
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
'*  short description : Case sensitive META-INF in packages
'*
'\******************************************************************************

testcase tLowerCaseMetaInf

    '///<h3>Case sensitive META-INF in packages</h3>
    '///<ul>
    
    dim cExtensionName as string
        cExtensionName = "lowercasemetainf.oxt"
    
    dim cExtensionPath as string
        cExtensionPath = gTesttoolPath & "extensions\optional\input\errors\"
        cExtensionPath = cExtensionPath & cExtensionName
        cExtensionPath = convertpath( cExtensionPath )

    dim irc as integer
        
    '///+<li>Create a new document</li>
    hCreateDocument()
    
    '///+<li>Add the problematic extension</li>
    irc = hExtensionAddGUI( cExtensionPath, "" )

    ' the extension comes with a german license only and will trigger
    ' a localized license warning on any other language
    if ( irc = 4 ) then
        if ( iSprache <> 49 ) then

            printlog( "This test only works with german ui-language" )
            kontext "active"
            if ( active.exists() ) then
                active.ok()
               
                kontext "PackageManager"
                PackageManager.ok()
            else
                warnlog( "Unexpected active" )
            endif
        else
            kontext "active"
            if ( active.exists() ) then
                warnlog( "Unexpected active" ) 
            endif
        endif
    endif

    '///+<li>Remove the problematic extension</li>
    if ( irc = 0 ) then
        hExtensionRemoveGUI( cExtensionName )
    endif
    
    '///+<li>Close the document</li>
    hDestroyDocument()
    
    '///</ul>

endcase

