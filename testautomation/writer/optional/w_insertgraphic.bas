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
'* short description : Test of Graphic function 
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
	use "writer\optional\includes\insertgraphic\w_insertgraphic1.inc"
	use "writer\optional\includes\insertgraphic\w_insertgraphic2.inc"
	use "writer\optional\includes\insertgraphic\w_insertgraphic3.inc"
	use "writer\optional\includes\insertgraphic\w_insertgraphic4.inc"
	use "writer\optional\includes\insertgraphic\w_insertgraphic5.inc"

	printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartZeit )
	printlog Chr(13) + "******* Writer - Level 1 - Test *******"
	printlog Chr(13) + "      - Test of Graphic function -"

	Call hStatusIn ( "writer", "w_insertgraphic.bas" , "Graphic" )
	Call wOptionsUndo("AutoCaption")

    'Software Configuration:
    'Getting the decimal seperator from global function
    gSeperator = GetDecimalSeperator()
    'Setting the measurement unit to centimeters.
    gMeasurementUnit = fSetMeasurementToCM()

    Call tInsertGraphic_X
    Call tInsertGraphic_1
    Call tInsertGraphic_2
    Call tInsertGraphic_3
	Call tInsertGraphic_4
	Call tInsertGraphic_5
	Call tInsertGraphic_6
	Call tInsertGraphic_7
	Call tInsertGraphic_8
	Call tInsertGraphic_9
	Call tInsertGraphic_10
	Call tInsertGraphic_11
	Call tInsertGraphic_12
	Call tInsertGraphic_13
	Call tInsertGraphic_14
	Call tInsertGraphic_15
	Call tInsertGraphic_16
	Call tInsertGraphic_17
	Call tInsertGraphic_18
	Call tInsertGraphic_19
	Call tInsertGraphic_20
	Call tInsertGraphic_21
	Call tInsertGraphic_22
	Call tInsertGraphic_23
	Call tInsertGraphic_24
	Call tInsertGraphic_25
	Call tInsertGraphic_26
	Call tInsertGraphic_27
	Call tInsertGraphic_28
	Call tInsertGraphic_29
	Call tInsertGraphic_30
	Call tInsertGraphic_31
	Call tInsertGraphic_32
	Call tInsertGraphic_33
	Call tInsertGraphic_34
	Call tInsertGraphic_35
	Call tInsertGraphic_36
	Call tInsertGraphic_37
	Call tInsertGraphic_38
	Call tInsertGraphic_39
	Call tInsertGraphic_40
	Call tInsertGraphic_41
	Call tInsertGraphic_42
	Call tInsertGraphic_43
	Call tInsertGraphic_44
	Call tInsertGraphic_45
	Call tInsertGraphic_46
	Call tInsertGraphic_47
	Call tInsertGraphic_48
	Call tInsertGraphic_49

	Call hStatusOut

	Printlog Chr(13) + "End of Level 1 Test (Insert Graphic):"
	Printlog "Duration: "+ WieLange ( StartZeit )
	Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    
    Call GetUseFiles
    gApplication = "WRITER"
end sub
