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
'* owner : oliver.craemer@oracle.com
'*
'* short description : calc named range level 2 test
'*
'*************************************************************************
'*
' #1 c_namedrange ' Initial routine
' #1 tAssignRangeName
' #1 tCheckErrorInvalidRangeName
' #1 tCheckErrorInvalidRangeSelected
'*
'\************************************************************************
'TODO: add some old features

sub cNamedRange

Printlog Chr(13) + "--------- named range ---------"

   call tAssignRangeName
   call tCheckErrorInvalidRangeName
   call tCheckErrorInvalidRangeSelected
End sub
'
'---------------------------------------------------------------------------

testcase tAssignRangeName

     dim sInputFile as string
         sInputFile = convertpath(gTesttoolPath & "spreadsheet/optional/input/namedrange.sxc")
     dim sOutputFile as String
         sOutputFile = convertpath(gOfficePath & "user/work/assignrangename." & sDefaultExtension)

     
     printlog "Assign range names in name box"

     printlog " Load testdocument and save locally with current filter"
     call hFileOpen (sInputFile)
     if NOT hFileSaveAsWithFilterKill (sOutputFile , "calc8") then
         warnlog "Saving test document localy failed -> Aborting"
         call hCloseDocument
         goto endsub
     end if
  
     printlog " Select range C2:F10"
     call fCalcSelectRange("C2:F10")
     printlog " Assign name 'FirstRange'"
     call fCalcNameRange("FirstRange")
     printlog " Select range X31996:AB32005"
     call fCalcSelectRange("X31996:AB32005")
     printlog " Assign name 'SecondRange'"
     call fCalcNameRange("SecondRange")
     printlog " Select range IT65533:IU65535"
     call fCalcSelectRange("IT65533:IU65535")
     printlog " Assign name 'ThördRänge' (Special Characters ;-)"
     call fCalcNameRange("ThördRänge")
     printlog " Select cell J333"
     call fCalcSelectRange("J333")
     printlog " Assign name 'OneCell'"
     call fCalcNameRange("OneCell")
     printlog " Save changes and reload"
     FileSave
     call hCloseDocument
     call hFileOpen (sOutputFile)
     printlog " Select range C2:F10"
     call fCalcSelectRange("C2:F10")
     printlog " Check for formula bar visibility"
     Kontext "RechenleisteCalc"
     if not RechenleisteCalc.isvisible then
        warnlog "Formula bar was expected to be visible"
        ViewToolbarsFormulaBar
     end if
     sleep(2)
     printlog " Check name box for 'FirstRange'"
     Kontext "RechenleisteCalc"
     if Bereich.GetSelText = "FirstRange" then 
        printlog "Correct name assigned for 'C2:F10'"
     else
        warnlog "Name for 'C2:F10' is " & Bereich.GetSelText & " instead of 'FirstRange'"
     end if
     printlog " Select range X31996:AB32005"
     call fCalcSelectRange("X31996:AB32005")
     printlog " Check name box for 'SecondRange'"
     Kontext "RechenleisteCalc"
     if Bereich.GetSelText = "SecondRange" then 
        printlog "Correct name assigned for 'X31996:AB32005'"
     else
        warnlog "Name for 'X31996:AB32005' is " & Bereich.GetSelText & " instead of 'SecondRange'"
     end if
     printlog " Select range IT65533:IU65535"
     call fCalcSelectRange("IT65533:IU65535")
     printlog " Check name box for 'ThördRänge'"
     Kontext "RechenleisteCalc"
     if Bereich.GetSelText = "ThördRänge" then 
        printlog "Correct name assigned for 'IT65533:IU65535'"
     else
        warnlog "Name for 'IT65533:IU65535' is " & Bereich.GetSelText & " instead of 'ThördRänge'"
     end if
     printlog " Select cell J333"
     call fCalcSelectRange("J333")
     printlog " Check name box for 'OneCell'"
     Kontext "RechenleisteCalc"
     if Bereich.GetSelText = "OneCell" then 
        printlog "Correct name assigned for 'J333'"
     else
        warnlog "Name for 'J333' is " & Bereich.GetSelText & " instead of 'OneCell'"
     end if
     printlog " Close document and finish test</ul>"
     sleep(2)
     call hCloseDocument

  endcase
'
'---------------------------------------------------------------------------
'
testcase tCheckErrorInvalidRangeName

    printlog "Check for error messages when assigning invalid range name"
    printlog " New calc document"
    call hNewDocument
    printlog " Select range C2:F10"
    call fCalcSelectRange("C2:F10")
    printlog " Try to assign name '%FirstRange'"
    printlog " and check for failture"
    if fCalcNameRange("%FirstRange") then
        warnlog "OOPS, assigning invalid range name '%FirstRange' should fail!"
    else
        printlog "Invalid name was not accepted"
    end if
    printlog " Close document and finish test</ul>"
  call hCloseDocument
endcase
'
'---------------------------------------------------------------------------
'
testcase tCheckErrorInvalidRangeSelected

    printlog "Check for error messages when assigning invalid range name"
    printlog " New calc document"
    call hNewDocument
    printlog " Select range C2:F10"
    call fCalcSelectRange("C2:D3")
    printlog " Change calc selection mode"
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys ("<SHIFT F8>")
    printlog " Add random cell to selection"
    Kontext "DocumentCalc"
    call gMouseClick (90,90)
    printlog " Try to assign valid name 'FirstRange'"
    printlog " and check for failture"
    if fCalcNameRange("FirstRange") then
        warnlog "OOPS, assigning invalid range selected with 'FirstRange' should fail!"
    else
        printlog "Invalid selection was not accepted"
    end if
    Kontext "DocumentCalc"
    printlog " Reset calc selection mode"
    DocumentCalc.TypeKeys ("<SHIFT F8>")
    printlog " Close document and finish test"
    call hCloseDocument
endcase

