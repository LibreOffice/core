'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
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
'/************************************************************************
'*
'* owner : marc.neumann@oracle.com
'*
'* short description : Address Book
'*
'\***********************************************************************

'* TODO: add testcase for issue i61611 

' Information for this test under
' http://wiki.services.openoffice.org/wiki/Database_Automatic_Testing


sub main
    printlog "---------------------------------------------------------------------"
    printlog "-----        D B A C C E S S   A D D R E S S   B O O K          -----"       
    printlog "---------------------------------------------------------------------"

    use  "dbaccess/optional/includes/db_Mozilla.inc"
    use  "dbaccess/optional/includes/db_Windows.inc"

    call hStatusIn ("dbaccess" , "dba_db_Addressbook.bas" )
    printlog "------------- Mozilla Address Book -----------------"   
    
    call db_Mozilla    
    
    printlog "--------------- Windows Address Book ------------------"
    
    call db_Windows
            
    call hStatusOut

end sub


sub LoadIncludeFiles      
   use "dbaccess/tools/dbinit.inc"
   Call sDBInit
   Call GetUseFiles
   gApplication   = "WRITER"
end sub
