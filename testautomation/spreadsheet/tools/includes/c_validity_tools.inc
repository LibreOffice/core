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
'**
'** owner : oliver.craemer@Sun.COM
'**
'** short description : Subroutines for the data validity test
'**
'************************************************************************
' **
' #0 check_message ' Check if the messages in validity test are OK
' **
'\***********************************************************************

sub check_message(MType as String, Celladdress as String, cellcontent as String, wrong as String)
'/// Sub routine for checking message 
    if MType <> "Macro" then
        Kontext "RechenleisteCalc"
        if not RechenleisteCalc.isvisible then
           ViewToolbarsFormulaBar
        end if
        Kontext "RechenleisteCalc"
        Bereich.TypeKeys "<MOD1 A>"
        Bereich.TypeKeys Celladdress
        sleep(1)
        Bereich.TypeKeys "<RETURN>"
    
        Kontext "DocumentCalc"
        DocumentCalc.TypeKeys cellcontent
        DocumentCalc.TypeKeys "<RETURN>"
        kontext "active"
        if active.exists(2) then
            warnlog "Value " & cellcontent & " is correct in cell "& Celladdress & ", so no " & MType & " message must not be appear"
            if MType = "Stop" then
                active.OK
            else
                active.Cancel
            end if
        else
            printlog "OK, no " & MType & " message appears, by inserting the correct value into " & Celladdress
        end if

        Kontext "RechenleisteCalc"
        if not RechenleisteCalc.isvisible then
            ViewToolbarsFormulaBar
        end if
        Kontext "RechenleisteCalc"
        Bereich.TypeKeys "<MOD1 A>"
        Bereich.TypeKeys Celladdress
        sleep(1)
        Bereich.TypeKeys "<RETURN>"
        Kontext "DocumentCalc"
        DocumentCalc.TypeKeys wrong
        DocumentCalc.TypeKeys "<RETURN>"
        kontext "active"
        if active.exists(2) then
            printlog "OK, value " & wrong & " is not correct in cell " & Celladdress & ", so the " & MType & " message has to be appear"
            if MType = "Stop" then
                active.OK
            else
                active.Cancel
            end if
        else
            warnlog "Value " & wrong & " is not correct for cell " & Celladdress & ", so the " & MType & " message has to be appears"
        end if
    else
        Kontext "RechenleisteCalc"
        if not RechenleisteCalc.isvisible then
            ViewToolbarsFormulaBar
        end if
        Kontext "RechenleisteCalc"
        Bereich.TypeKeys "<MOD1 A>"
        Bereich.TypeKeys Celladdress
        sleep(1)
        Bereich.TypeKeys "<RETURN>"
        Kontext "DocumentCalc"
        DocumentCalc.TypeKeys cellcontent
        DocumentCalc.TypeKeys "<RETURN>"
        DocumentCalc.TypeKeys "<UP><RIGHT>"
        FormatCells
        Kontext
        Active.setPage TabFontEffects
        Kontext "TabFontEffects"
        if FontColor.getSelIndex = 1 then
            printlog "OK, the macro detect the correct word in " & Celladdress
        else
            warnlog "The Fontcolor is " & FontColor.getSelText & " instead of automatic in cell " & Celladdress
        end if
        TabFontEffects.Cancel
           
        Kontext "RechenleisteCalc"
        if not RechenleisteCalc.isvisible then
            ViewToolbarsFormulaBar
        end if
        Kontext "RechenleisteCalc"
        Bereich.TypeKeys "<MOD1 A>"
        Bereich.TypeKeys Celladdress
        sleep(1)
        Bereich.TypeKeys "<RETURN>"
        Kontext "DocumentCalc"
        DocumentCalc.TypeKeys wrong
        DocumentCalc.TypeKeys "<RETURN>"
        DocumentCalc.TypeKeys "<UP><RIGHT>"
        FormatCells
        Kontext
        Active.setPage TabFontEffects
        Kontext "TabFontEffects"
        if FontColor.getSelIndex = 12 then
            printlog "OK, the macro detect the wrong word in " & Celladdress
        else
            warnlog "The Fontcolor is " & FontColor.getSelText & " instead of light green in cell " & Celladdress
        end if
        TabFontEffects.Cancel
    end if
end sub

