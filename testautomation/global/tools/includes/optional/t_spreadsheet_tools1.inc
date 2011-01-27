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
'/***********************************************************************
'*
'* owner : oliver.craemer@oracle.com
'*
'* short description : Global tools for spreadsheet
'*
'************************************************************************
'*
' #1 fGotoCell     ' Set active cell in a spreadhseet to a defined cell adress
' 
'*
'\***********************************************************************

function fGotoCell (sCelladdress as String) as integer
'/// The function sets the active cell in a spreadsheet to a defined cell address.
'/// Input: sCelladdress as String
'/// Output: <ul><li>0 = active cell has changed successfully</li>
'///+ <li>1 = active cell hasn't changed</li> 

    Dim sActualPlace as string

    const CFN = "qa:qatesttool:global:tools:inc:t_spreadsheet_tools1.inc:fGotoCell: "
    
    'function will return 1 if something goes wrong
    fGotoCell = 1
    Kontext "RechenleisteCalc"
    '/// If the spreadsheet <i>formula toolbar</i> isn't available make them
    '/// + available with View / Toolbars / Formula Bar 
    if NOT RechenleisteCalc.Exists(1) then    
        ViewToolbarsFormulaBar
    end if
    try
        Kontext "RechenleisteCalc"
        sActualPlace = AktiverZellbereich.GetSelText
        if UCase(sActualPlace) = UCase(sCelladdress) then
            fGotocell = 0            
            exit function
        else
            sActualPlace = ""
            '/// Type the cell address into the <i>name box</i>            
            AktiverZellbereich.SetText sCelladdress
            '///+ and press RETURN
            AktiverZellbereich.TypeKeys "<RETURN>"
            sleep(1)        
            '/// If the <i>name box<i>' address has been changed to the expected
            '///+ cell address the function was successfull.
            sActualPlace = AktiverZellbereich.GetSelText
            if UCase(sActualPlace) = UCase(sCelladdress) then
                fGotocell = 0
            else
                warnlog CFN & "The cell address has not been changed!"
            end if
        end if
    catch
        warnlog CFN & "Something unexpected happened! The cell address has not been changed!"
        fGotocell = 1
    endcatch
end function

