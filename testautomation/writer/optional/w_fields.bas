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
'* owner :  helge.delfs@oracle.com
'*
'* short description : Test of content and update reliability of the fields 
'*
'\*******************************************************************

global gSeperator as String
global gMeasurementUnit as String

sub main

    Dim StartZeit
    StartZeit = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool3.inc"
    use "writer\tools\includes\w_tool7.inc"   
    use "writer\optional\includes\tools\tools1.inc"
    use "writer\optional\includes\tools\tools2.inc"
    use "writer\optional\includes\fields\w_fields1.inc"
    use "writer\optional\includes\fields\w_fields2.inc"
    use "writer\optional\includes\fields\w_fields3.inc"
    use "writer\optional\includes\fields\w_fields4.inc"
    use "writer\optional\includes\fields\w_fields5.inc"
    use "writer\optional\includes\fields\w_fields6.inc"   
    use "writer\optional\includes\fields\w_fields7.inc"   

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartZeit )
    printlog Chr(13) + "******* Writer - Fields - Test *******"

    Call hStatusIn ( "writer", "w_fields.bas" , "Fields" )

    'Software Configuration:
    'Getting the decimal seperator from global function
    gSeperator = GetDecimalSeperator()
    'Setting the measurement unit to centimeters.
    gMeasurementUnit = fSetMeasurementToCM()

    printlog Chr(13) + " --- Test Fields - General Test ---"
    Call w_fields1
    Call w_fields2
    printlog Chr(13) + " --- Test Fields - Reference ---"
    Call w_fields3
    printlog Chr(13) + " --- Test Fields - Doc information ---"
    Call w_fields4
    printlog Chr(13) + " --- Test Fields - Variables ---"
    Call w_fields5                  
    printlog Chr(13) + " --- Test Fields - Functions ---"
    Call w_fields6
    printlog Chr(13) + " --- Test Fields - L10N ---"
    Call w_fields7

    Call hStatusOut

    Printlog Chr(13) + "End of Level 1 Test - Fields"
    Printlog "Duration: "+ WieLange ( StartZeit )
    Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"  

   Call GetUseFiles
   gApplication = "WRITER"
end Sub
