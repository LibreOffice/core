'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
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
'/******************************************************************************
'*
'* owner : helge.delfs@oracle.com
'*
'* short description : Description of file
'*
'\******************************************************************************

function hLevelContent () as String
    ' presupposition: just stay in line with the to check content
    ' easy to enhance; on demand you can add text to the paragraph and only have to chage this function
    ' input : -
    ' output: paragraph content
    dim sTemp as string
    wTypeKeys "<end><shift home>" 'select line
    editcopy
    sTemp = GetClipboardText()
    'i don't need the first '-', so i start searching at 2. char
    hLevelContent = Right(sTemp, Len(sTemp)-InStr(2,sTemp,"-")
end function

'------------------------------------------------------------------------------

function hLevelChange(aL() as integer,i as integer,x as integer ,y as integer, sublevel as boolean ) as boolean
    ' ALSO boolean, if it schould be checked, if it got changed! else don't adjust array!!!
    ' input: level Nr., how many levels +/-
    ' return: false, if wrong paragraph

    dim itemp as integer
    dim xtemp as integer
    dim ytemp as integer
    dim stemp as integer
    dim j as integer

    if (hLevelOk (i)) then
        if (aL (i,1) + x) > 0 then  ' CHG <-> level 'stufen'
            ' has to work for +/- x
            xTemp = aL (i,1)
            aL (i,1) = aL (i,1) + x
            if (sublevel) then
                iTemp = i+1
                do while (aL (iTemp,1) > xTemp)
                    aL (iTemp,1) = aL (iTemp,1) + x
                    iTemp=iTemp + 1
                loop
            end if
        else
            'warnlog "This makes no sense; you trie to 'move to the left'; it doesn't go further! Entry: "+i
        end if
        iTemp = 1
        do while (i <> aL(itemp,2))
            inc iTemp
        loop
        printlog "CHG: found i="+i+" at: "+iTemp

        ' move entry without sublevels
        if (y < -1) OR (y > 1) then warnlog "y ! aus (-1,1) is not implemented :-( yet !"
        if (y > 0) then
            xtemp = aL( itemp+y ,1)
            aL( itemp+y ,1) = aL( itemp ,1)
            aL( itemp ,1) = xtemp
            xtemp = aL( itemp+y ,2)
            aL( itemp+y ,2) = aL( itemp ,2)
            aL( itemp ,2) = xtemp
            if (sublevel) then            ' with sublevel
                yTemp = aL( iTemp+y ,1)            ' save hor <-> level in yTemp
                sTemp = 2
                do while (aL (iTemp ,1) > yTemp)
                    xtemp = aL( itemp+sTemp ,1)
                    aL( itemp+sTemp ,1) = aL( itemp ,1)
                    aL( itemp ,1) = xtemp
                    xtemp = aL( itemp+sTemp ,2)
                    aL( itemp+sTemp ,2) = aL( itemp ,2)
                    aL( itemp ,2) = xtemp
                    inc sTemp
                loop
            end if
        else
            if (y < 0) then
                for j=-1 to y
                    xtemp = aL( itemp+j ,1)              ' hori <-> level copy
                    aL( itemp+j ,1) = aL( itemp+j+1 ,1)
                    aL( itemp+j+1 ,1) = xtemp            '      ^
                    xtemp = aL( itemp+j ,2)              ' vert | level copy
                    aL( itemp+j ,2) = aL( itemp+j+1 ,2)  '      v
                    aL( itemp+j+1 ,2) = xtemp
                next j
                if (sublevel) then            ' with sublevel
                    yTemp = aL( iTemp+y ,1)            ' save hor <-> level in yTemp
                    iTemp = itemp+1
                    do while (aL (iTemp,1) > yTemp)
                        xtemp = aL( itemp+y ,1)
                        aL( itemp+y ,1) = aL( itemp+y+1 ,1)
                        aL( itemp+y+1 ,1) = xtemp
                        xtemp = aL( itemp+y ,2)
                        aL( itemp+y ,2) = aL( itemp+y+1 ,2)
                        aL( itemp+y+1 ,2) = xtemp
                        iTemp=iTemp + 1
                    loop
                end if
            else ' 0
            end if
        end if
    end if
end function

'------------------------------------------------------------------------------

function hLevelPrint (aL()as integer) as boolean
    dim sTemp as string
    dim iTemp as integer
    dim i,j as integer
    ' how big is this array??!!! pd: it has to be in aL(0,0)
    printlog "---------------------------------------------------------------------"
    for i=1 to aL(0,0)
        sTemp = ""
        for j=1 to aL(i,1)
            sTemp = sTemp + "..."
        next j
        printlog sTemp + i + "  "+aL(i,2)+ "                   +1: "+aL(i,1)
    next i
    printlog "---------------------------------------------------------------------"
end function

'------------------------------------------------------------------------------

function hIsNumberingBullets () as Boolean
    '/// am i in a NumberingBullets area ?? ///'
    '/// Output: True / False (as Boolean)  ///'
    dim NOerror as Boolean

    hIsNumberingBullets = false
    NOerror = false

    sleep (1)
    try
        FormatNumberingBullets
        NOerror = true
    catch
        Warnlog "w_tool5.inc::hIsNumberingBullets::Slot not available"
        NOerror = false
    endcatch

    if NOerror then
        Kontext
        Active.SetPage TabBullet
        Kontext "TabBullet"
        sleep (1)
        if (remove.isEnabled) then
            hIsNumberingBullets = true
        else
            hIsNumberingBullets = false
        end if
        TabBullet.cancel
    end if
    sleep (1)
end function

'------------------------------------------------------------------------------

sub hFormatParagraphNumbering (sStyle as string,bRestart as integer,sStartW as string)

    FormatParagraph
    Kontext
    Active.SetPage TabNumerierungAbsatz
    Kontext "TabNumerierungAbsatz"

    try
        if (sStyle <> "") then
            Vorlage.Select sStyle
        else
            Vorlage.Select 2
        end if
        if (bRestart <> -1) then
            select case bRestart
            case 0: NumerierungNeuBeginnen.UnCheck
            case 1: NumerierungNeuBeginnen.Check
            case 2: NumerierungNeuBeginnen.TriState
            end select
        end if
        if (sStartW <> -1) then
            select case sStartW
            case 0: NumerierungBeginnenBei.UnCheck
            case 1: NumerierungBeginnenBei.Check
            end select
        end if
    catch
        Warnlog "Not possible; wrong style=none??"
    endcatch

    TabNumerierungAbsatz.OK

end sub

'------------------------------------------------------------------------------

function hFindInDocument (sTempSearch as string) as boolean
    dim  sTemp as string

    hFindInDocument = FALSE
    Call hFindeImDokument (sTempSearch, 1)
    EditCopy
    sTemp = right (GetClipboardText(), len(sTempSearch))
    if (sTemp <> sTempSearch) then
        Warnlog "Found: '" & sTemp & "'  and not: " & sTempSearch
        hFindInDocument = FALSE
    else
        hFindInDocument = TRUE
    end if
end function
