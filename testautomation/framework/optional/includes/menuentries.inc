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
'* Owner : thorsten.bosbach@oracle.com
'*
'* short description : Read menubar and first level drop down lists and compare to reference
'*
'\*****************************************************************

sub t_getMenuText
    dim k,c as integer
    dim sApplication() as string

    sApplication = array("WRITER","MASTERDOCUMENT","HTML","CALC","IMPRESS","DRAW","MATH","BASIC","DATABASE")

    ' for every application
    c = uBound(sApplication())
    for k = 0 to c
        gApplication = sApplication(k)
        printlog "********** " + gApplication
        call s_getMenuText
    next k
    gApplication = "WRITER"
end sub

testcase s_getMenuText
    dim i,j,l,m,a,b,d,e as integer
    dim sReference(400) as string
    dim sCurrent(400) as string
    dim sFileName as string
    dim sTemp as string
    dim bError as boolean
    dim iTopLevelCount as integer
    dim iOffset as integer
    
        for i=0 to uBound(sReference())
            sReference(i) = 0
            sCurrent(i) = 0
        next i
        bError = FALSE
        hNewDocument
        hUseMenu
        ' for every item in main menu bar
        a = MenuGetItemCount
        iTopLevelCount = a
        MenuSelect (0)
        for i = 1 to a
            hUseMenu
            stemp = MenuGetItemText(MenuGetItemID(i))
            'printlog sTemp
            listAppend(sCurrent(),"********** "+sTemp)
            MenuSelect(MenuGetItemID(i))
            b = MenuGetItemCount
            for j = 1 to b
                sTemp = MenuGetItemText(MenuGetItemID(j))
                'printlog "    " + sTemp
                listAppend(sCurrent(),sTemp)
            next j
            MenuSelect (0)
        next i
        hCloseDocument
        sFileName = convertPath(gTestToolPath+"framework\optional\input/menu/"+lCase(gISOLang+"_"+gApplication+".txt"))
        printlog( "Using reference file: " & sFileName )
        ' gProductName should not make a big difference.
        if fileExists(sFileName) then
            ' load to compare
            listRead(sReference(), sFileName, "utf8")
            d = listCount(sCurrent())
            m = d
            e = listCount(sReference())
            if (d <> e) then
                warnlog "The count of Menuentries differs from reference "+gApplication+". should: '" + e + "'; delta is: '" + (d - e) + "'"
                bError = TRUE
                ' always take the shorter list count
                if cBool(d - e) then
                    d = e
                endif
                e = m - e
            else
                printlog "Count of Menuentries is ok"
                e = 0
            endif
            ' compare all entries
            iOffset = 0
            m = 0
            for l=1 to d 
                ' count main menu bar items
                if inStr(sReference(l), "********** ") > 0 then
                    m = m + 1
                    ' bug in inc m :-(
                    printlog mid(sReference(l),12)
                endif
                ' complain non existing accelerator
                if (sReference(l) <> "") AND (inStr(sReference(l),"~") < 1) then
                    qaErrorLog "No accelerator (~) in item: '" + sReference(l) + "'"
                endif
                ' compare the menu entries
                if (sReference(l) <> sCurrent(l+iOffset))then
                    ' try to syncronize different count of entries
                    if (e <> 0) then
                        iOffset = iOffset + sgn(e)
                        'qaerrorlog "io: " + ioffset
                        e = e - sgn(e)
                        'qaerrorlog "e: " + e
                    endif
                    '
                    if (m = (iTopLevelCount-1)) then
                        ' tolerate the window menu
                        printlog "String differs: " + chr(13) + "current  : '"+sCurrent(l) + "'" + chr(13) + "reference: '" + sReference(l) + "'"
                    else
                        bError = TRUE
                        warnlog "String differs: " + chr(13) + "current  : '"+sCurrent(l) + "'" + chr(13) + "reference: '" + sReference(l) + "'"
                    endif
                endif
            next l
          '  for l=d-5 to d+5
          '  printlog "------"
          '  printlog sreference(l)
          '  printlog scurrent(l)
          '  next l
            ' if there is still is a difference in count
            if (e <> 0) then
                if sgn(e) then
                    warnlog "Not all reference entries are there, e.g.: '" + sReference(l+1) + "'"
                else
                    warnlog "There are too much entries, e.g.: '" + sCurrent(l+1) + "'"
                endif
            endif
            if bError then
                sFileName = convertPath(gOfficePath+"user/work/menu/"+lCase(gISOLang+"_"+gApplication+".txt"))
                listWrite(sCurrent(), sFileName, "utf8")
                printlog "Written strings from this build to: " + chr(13) + sFileName
            else
                ' OK
            endif
        else
            ' write to checkin
            warnlog "No reference found; please copy:"+lCase(gISOLang+"_"+gApplication+".txt")
            sFileName = convertPath(gOfficePath+"user/work/menu/"+lCase(gISOLang+"_"+gApplication+".txt"))
            listWrite(sCurrent(), sFileName, "utf8")
            printlog sFileName + chr(13) + "to"
            sFileName = convertPath(gTestToolPath+"framework/level1/input/menu/"+lCase(gISOLang+"_"+gApplication+".txt"))
            printlog sFilename
            'listWrite(sCurrent(), sFileName, "utf8")
        endif
    'speichern: (vendor?) sprache gISOLang applikation
endcase

