'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: g_print.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:40 $
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
'* Owner : wolfram.garten@sun.com
'*
'* short description :
'*
'**************************************************************************************
' #1 tFilePrint
'\*************************************************************************************

testcase tFilePrint
    Call hNewDocument

    printlog "Inserting testtext."
    hTextrahmenErstellen ("This is an automated print test with testtool for GRAPHICS ",90,90,80,10)
    hTextrahmenErstellen ("Version    : "+ gVersionsnummer + " / " + gLanguage,80,90,70,10)
    hTextrahmenErstellen ("Date / Time : "+  Date + " / " + Time,70,90,60,10)
    hTextrahmenErstellen ("Machine / User: " + gPCName + "   " + gUser,60,90,50,10)

    printlog "Opening print Dialog."
    FilePrint
    kontext
    if active.exists(2) then
        active.ok
        qaerrorlog "There is no printer available - please install one on your system!"
    endif
    sleep 2
    
    printlog "Opening Draw Tabpage."
    Kontext "Printing"
    PrintingTabControl.setPage( TabApplication )
	kontext "TabApplication"
   
    printlog "Checking Boxes and Buttons..."
    SlideName.Check
    DateAndTime.Check
    if (gApplication = "IMPRESS") then
    HiddenPages.Check
    endif
    OriginalColors.Check
    Grayscale.Check
    BlackAndWhite.Check
    OriginalSize.Check
    FitToPage.Check
    MultiplePages.Check
    TilePaper.Check

    printlog "Changing Tabpage and back."
    kontext "printing"
    PrintingTabControl.SetPage ( TabPrintGeneral )
    kontext "TabPrintGeneral"
    
    kontext "printing"
    PrintingTabControl.SetPage ( TabApplication )
	kontext "TabApplication"
    
    printlog "Checking state of buttons and boxes.."
     if NOT ( SlideName.isChecked ) then
        warnlog( "Slide Name is not checked." )
        endif
     if NOT ( DateAndTime.isChecked ) then
        warnlog( "Date and Time is not checked." )
    endif
    if (gApplication = "IMPRESS") then
        if NOT ( HiddenPages.isChecked ) then
            warnlog( "Hidden Pages is not checked." )
        endif
    endif
    if NOT ( BlackAndWhite.isChecked ) then
        warnlog( "Black And White is not checked." )
    endif
    if NOT ( TilePaper.isChecked ) then
        warnlog( "Tile Paper is not checked." )
    endif
    
    kontext "printing"
    Printing.Cancel()

    Call hCloseDocument
endcase

