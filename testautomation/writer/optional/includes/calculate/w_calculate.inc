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
'* owner : helge.delfs@oracle.com
'*
'* short description : Put description here
'*
'\***********************************************************************

sub w_calculate

    Call tToolsCalculate1
    Call tToolsCalculate2
    Call tToolsCalculate3

end sub

testcase tToolsCalculate1
    Printlog "- Tools / calculation   base functionality formula bar"

    Printlog "   - Add"
    Call hNewDocument
    Call wTypeKeys "2+3+4+5<Shift Home>"
    ToolsCalculate
    Call wTypeKeys "<End> = "
    EditPaste
    if GetClipboardText <> "14" then Warnlog "Result of Add is not 14 but "+ GetClipboardText

    Printlog "   - Subtract"
    Call wTypeKeys "<Return>101-3-4-155<Shift Home>"
    ToolsCalculate
    Call wTypeKeys "<End> = "
    EditPaste
    if GetClipboardText <> "-61" then Warnlog "Result of Subtract is not -61 but "+ GetClipboardText

    Printlog "   - multiply"
    Call wTypeKeys "<Return>11*3*1" & DecimalSeparator & "5<Shift Home>"
    ToolsCalculate
    Call wTypeKeys "<End> = "
    EditPaste
    if GetClipboardText <> "49" & DecimalSeparator & "50" and GetClipboardText <> "49" & DecimalSeparator & "5" then Warnlog "Result of Multiply is not 49" & DecimalSeparator & "50 but " & GetClipboardText

    Printlog "   - Division"
    Call wTypeKeys "<Return>111/3/2<Shift Home>"
    ToolsCalculate
    Call wTypeKeys "<End> = "
    EditPaste
    if GetClipboardText <> "18" & DecimalSeparator & "50" and GetClipboardText <> "18" & DecimalSeparator & "5" then Warnlog "Result of Multiply is not 18" & DecimalSeparator & "50 but " & GetClipboardText

    Printlog "   - random with brackets"
    Call wTypeKeys "<Return>((12*2)/4" & DecimalSeparator & "5+3)-1" & DecimalSeparator & "5 <Shift Home>"
    
    ToolsCalculate
    Call wTypeKeys "<End> = "
    EditPaste
    if Left$(GetClipboardText,4) <> "6" & DecimalSeparator & "83" then Warnlog "Result of mixed calculation not 6" & DecimalSeparator & "83 but " & GetClipboardText

    Call hCloseDocument
endcase

' *******************************************************************

testcase tToolsCalculate2
    Printlog "- Tools / Calculate   Basic functionality"

    Printlog "   - Add"
    Call hNewDocument
    Call wTypeKeys "<F2>"
    Kontext "OL_SW_Rechenleiste"
    EingabeFeld.SetText "2+3+4+5" : EingabeFeld.TypeKeys "<Return>"
    if ErmittleErgebnis ( "14" ) = FALSE then Warnlog "Result of Add not 14 but "+ GetClipboardText

    Printlog "   - Subtract"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    EingabeFeld.SetText "101-3-4-155" : EingabeFeld.TypeKeys "<Return>"
    if ErmittleErgebnis ( "-61" ) = FALSE then Warnlog "Result of Subtract not -61 but "+ GetClipboardText

    Printlog "   - Multiply"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    EingabeFeld.SetText "11*3*1" & DecimalSeparator & "5" : EingabeFeld.TypeKeys "<Return>"
    if ErmittleErgebnis ( "49" & DecimalSeparator & "5" ) = FALSE then Warnlog "Result of Multiplication not 49" & DecimalSeparator & "5 but " & GetClipboardText

    Printlog "   - Division"
    Call wTypeKeys "<End><Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    EingabeFeld.SetText "111/3/2" : EingabeFeld.TypeKeys "<Return>"
    if ErmittleErgebnis ( "18" & DecimalSeparator & "5" ) = FALSE then Warnlog "Result of Division not 18" & DecimalSeparator & "5 but " & GetClipboardText

    Printlog "   - random with brackets"
    Call wTypeKeys "<End><Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    EingabeFeld.SetText "((12*2)/4" & DecimalSeparator & "5+3)-1" & DecimalSeparator & "5" : EingabeFeld.TypeKeys "<Return>"
    if ErmittleErgebnis ( "6" & DecimalSeparator & "83" ) = FALSE then Warnlog "Result of mixed calculation not 6" & DecimalSeparator & "83 but " & GetClipboardText

    Call hCloseDocument
endcase

' *******************************************************************

testcase tToolsCalculate3
    Dim Fehlermeldung as String

    Printlog "- Tools / Calculate   Basic functionality (formula button)"

    Call hNewDocument
    Printlog "   - Sum (6)"
    Call wTypeKeys "<F2>"
    Kontext "OL_SW_Rechenleiste"
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 1 )
    Sleep 1
    EingabeFeld.TypeKeys " (6)" : EingabeFeld.TypeKeys "<Return>"
    Sleep 1
    if ErmittleErgebnis ( "6" ) = FALSE then Warnlog "Result is not 6 but "+ GetClipboardText

    Printlog "   - Sum ( <a1:B2>|<b4:r3> )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 1 )
    Sleep 1
    EingabeFeld.TypeKeys " ( <a1:B2" : EingabeFeld.TypeKeys ">|<b4:r3" : EingabeFeld.TypeKeys "> )" : EingabeFeld.TypeKeys "<Return>"
    select case iSprache
        case 01   : Fehlermeldung = "** Expression is faulty **"
        case 03   : Fehlermeldung = "** Expressão incorrecta **"
        case 31   : Fehlermeldung = "** Foutieve expressie **"
        case 33   : Fehlermeldung = "** Expression errone **"
        case 34   : Fehlermeldung = "** Expresión errónea **"
        case 39   : Fehlermeldung = "** Errore nell'espressione **"
        case 45   : Fehlermeldung = "** Fejlagtigt udtryk **"
        case 46   : Fehlermeldung = "** Felaktigt uttryck **"
        case 48   : Fehlermeldung = "** Nieprawiedlowe wyralenie **"
        case 49   : Fehlermeldung = "** Fehlerhafter Ausdruck **"
        case else : Fehlermeldung = ""       
    end select

    if ErmittleErgebnis ( Fehlermeldung ) = FALSE then
        if Fehlermeldung > "" then
            Warnlog "Result is not " + Fehlermeldung + " but " & GetClipboardText
        else
            QAErrorlog "** Errortext for this language not defined **"
        end if
    end if
    Printlog "   - 1323,3 round 1"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    EingabeFeld.TypeKeys "1323" & DecimalSeparator & "345 "
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 2 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> 1" : EingabeFeld.TypeKeys "<Return>"
    if ErmittleErgebnis ( "1323" & DecimalSeparator & "3" ) = FALSE then Warnlog "Result is not 1323" & DecimalSeparator & "3 but " & GetClipboardText

    Printlog "   - 10 + 20 PHD ( Percent )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    EingabeFeld.TypeKeys "10 + 20 "
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 3 )
    Sleep 1
    EingabeFeld.TypeKeys "<End><Return>"
    if ErmittleErgebnis ( "10" & DecimalSeparator & "2" ) = FALSE then Warnlog "Result is not 10" & DecimalSeparator & "2 but " & GetClipboardText

    Printlog "   - SQRT 144 ( Square Root )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 4 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> 144 <Return>"
    if ErmittleErgebnis ( "12" ) = FALSE then Warnlog "Result is not 12 but "+ GetClipboardText

    Printlog "   - 4 POW 8 ( Power )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    EingabeFeld.TypeKeys "4 "
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 5 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> 8 <Return>"
    if ErmittleErgebnis ( "65536" ) = FALSE then Warnlog "Result is not 65536 but "+ GetClipboardText

    Printlog "- Tools / Calculate   Operators with formula button"
    Printlog "   - 2 EQ 2 ( equal )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    EingabeFeld.TypeKeys "2 "
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 6 )
    Sleep 1
    Call hMenuSelectNr ( 2 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> 2 <Return>"
    if ErmittleErgebnis ( "1" ) = FALSE then Warnlog "Result is not 1 but "+ GetClipboardText

    Printlog "   - 2 NEQ 2 ( not equal )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    EingabeFeld.TypeKeys "2 "
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 6 )
    Sleep 1
    Call hMenuSelectNr ( 3 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> 2 <Return>"
    if ErmittleErgebnis ( "0" ) = FALSE then Warnlog "Result is not 0 but "+ GetClipboardText

    Printlog "   - 2 LEQ 3 ( Less than or equal )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    EingabeFeld.TypeKeys "2 "
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 6 )
    Sleep 1
    Call hMenuSelectNr ( 4 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> 3 <Return>"
    if ErmittleErgebnis ( "1" ) = FALSE then Warnlog "Result is not 1 but "+ GetClipboardText

    Printlog "   - 2 GEO 2,0 ( greater than or equal )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    EingabeFeld.TypeKeys "2 "
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 6 )
    Sleep 1
    Call hMenuSelectNr ( 5 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> 2" & DecimalSeparator & "0 <Return>"
    if ErmittleErgebnis ( "1" ) = FALSE then Warnlog "Result is not 1 but "+ GetClipboardText

    Printlog "   - 2 L 2 ( less )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    EingabeFeld.TypeKeys "2 "
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 6 )
    Sleep 1
    Call hMenuSelectNr ( 6 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> 2 <Return>"
    if ErmittleErgebnis ( "0" ) = FALSE then Warnlog "Result is not 0 but "+ GetClipboardText

    Printlog "   - 2,01 g 2 ( greater )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    EingabeFeld.TypeKeys "2" & DecimalSeparator & "01 "
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 6 )
    Sleep 1
    Call hMenuSelectNr ( 7 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> 2 <Return>"
    if ErmittleErgebnis ( "1" ) = FALSE then Warnlog "Result is not 1 but "+ GetClipboardText

    Printlog "   - 0 OR 0 ( Boolean or )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    EingabeFeld.TypeKeys "0 "
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 6 )
    Sleep 1
    Call hMenuSelectNr ( 8 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> 0 <Return>"
    if ErmittleErgebnis ( "0" ) = FALSE then Warnlog "Result is not 0 but "+ GetClipboardText

    Printlog "   - 0 XOR 1 ( Boolean XOR )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    EingabeFeld.TypeKeys "0 "
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 6 )
    Sleep 1
    Call hMenuSelectNr ( 9 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> 1 <Return>"
    if ErmittleErgebnis ( "1" ) = FALSE then Warnlog "Result is not 1 but "+ GetClipboardText

    Printlog "   - 1 AND 0 ( Boolean And )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    EingabeFeld.TypeKeys "1 "
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 6 )
    Sleep 1
    Call hMenuSelectNr ( 10 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> 0 <Return>"
    if ErmittleErgebnis ( "0" ) = FALSE then Warnlog "Result is not 0 but "+ GetClipboardText

    Printlog "   - 0 NOT 0 ( Boolean Or )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    EingabeFeld.TypeKeys "0 "
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 6 )
    Sleep 1
    Call hMenuSelectNr ( 11 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> 0 <Return>"
    if ErmittleErgebnis ( "0" ) = FALSE then Warnlog "Result is not 0 but "+ GetClipboardText

    Printlog "- Tools / Calculate   Statistical function with formula button"
    Printlog "   - mean 30|40|20 ( Mean )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 7 )
    Sleep 1
    Call hMenuSelectNr ( 1 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> 30|40|20 <Return>"
    if ErmittleErgebnis ( "30" ) = FALSE then Warnlog "Result is not 30 but "+ GetClipboardText

    Printlog "   - min 1|1,01|1,001 ( Minimum )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 7 )
    Sleep 1
    Call hMenuSelectNr ( 2 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> 1|1" & DecimalSeparator & "01|1" & DecimalSeparator & "001 <Return>"
    if ErmittleErgebnis ( "1" ) = FALSE then Warnlog "Result is not 1 but "+ GetClipboardText

    Printlog "   - max 1|1,01|1,001 ( Maximum )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 7 )
    Sleep 1
    Call hMenuSelectNr ( 3 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> 1|1" & DecimalSeparator & "01|1" & DecimalSeparator & "001 <Return>"
    if ErmittleErgebnis ( "1" & DecimalSeparator & "01" ) = FALSE then Warnlog "Result is not 1" & DecimalSeparator & "01 but "+ GetClipboardText

    Printlog "- Tools / Calculate   Further functions"
    Printlog "   - sin (PI/2) ( Sine )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 8 )
    Sleep 1
    Call hMenuSelectNr ( 1 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> (PI/2) <Return>"
    if ErmittleErgebnis ( "1" ) = FALSE then Warnlog "Result is not 1 but "+ GetClipboardText

    Printlog "   - cos (PI/2) ( Cosine )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 8 )
    Sleep 1
    Call hMenuSelectNr ( 2 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> (PI/2) <Return>"
    if ErmittleErgebnis ( "0" ) = FALSE then Warnlog "Result is not 0 but "+ GetClipboardText

    Printlog "   - tan (PI/2 - 0,001) ( Tangent )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 8 )
    Sleep 1
    Call hMenuSelectNr ( 3 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> (PI/2 - 0" & DecimalSeparator & "001) <Return>"
    if ErmittleErgebnis ( "1000" ) = FALSE then Warnlog "Result is not 1000 but "+ GetClipboardText

    Printlog "   - asin 1 ( Arc-Sine )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 8 )
    Sleep 1
    Call hMenuSelectNr ( 4 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> 1 <Return>"
    if ErmittleErgebnis ( Fehlermeldung ) = FALSE then
        if Fehlermeldung > "" then
            Warnlog "Result is not " + Fehlermeldung + " but " & GetClipboardText
        else
            QAErrorlog "** Errortext for this language not defined **"
        end if
    end if
	wait 500
	
    Printlog "   - acos 0 ( Arc-Cosine )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 8 )
    Sleep 1
    Call hMenuSelectNr ( 5 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> 0 <Return>"
    if ErmittleErgebnis ( "1" & DecimalSeparator & "57" ) = FALSE then Warnlog "Result is not 1" & DecimalSeparator & "57 but "+ GetClipboardText 

    Printlog "   - asin 1 ( Arc-Tangent )"
    Call wTypeKeys "<End> <Return><F2>"
    Kontext "OL_SW_Rechenleiste"
    Formel.Click
    Sleep 1
    Call hMenuSelectNr ( 8 )
    Sleep 1
    Call hMenuSelectNr ( 6 )
    Sleep 1
    EingabeFeld.TypeKeys "<End> 1 <Return>"
    if ErmittleErgebnis ( "0" & DecimalSeparator & "79" ) = FALSE then Warnlog "Result is not 0" & DecimalSeparator & "79 but "+ GetClipboardText
    Call hCloseDocument
endcase

