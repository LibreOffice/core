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
'* Owner : wolfram.garten@oracle.com
'*
'* short description :
'*
'\*******************************************************************

testcase tFormatStylistBackground
    printlog " special test for BUG # 96364 Background style looses functionality over PPT Ex/import"
    dim sFilter (50) as string
    dim sFileList (30) as string
    dim i as integer
    dim x as integer
    dim y as integer
    dim iColor as integer
    dim sFileName as string
    dim Exlist(20) as string

    sFilter (0) = 0

    if bAsianLan then 
       QaErrorLog "This testcase does not support Asian languages. Test ends."
       goto endsub
    end if
    
    if (glLocale(5) = "") then
        qaErrorLog("Language dependant string 'Background' is missing; get it from the stylist and insert it into the locale-file mentioned in the .bas file!")
        goto endsub
    endif

    printlog " if not exists : gOfficePath + '\\user\\impress\\optional\\', create it"
    if app.dir (ConvertPath ( gOfficePath + "user/work/impress") ) = "" then
        app.mkdir ConvertPath ( gOfficePath + "user/work/impress")
    endif
    if app.dir (ConvertPath ( gOfficePath + "user/work/impress/optional")) = "" then
        app.mkdir ConvertPath ( gOfficePath + "user/work/impress/optional")
    endif

    printlog " open application"
    Call  hNewDocument

    iColor = 10
    printlog " change background color via stylist"
    if (iColor <> fGetSetPageBackground (iColor,0)) then
        warnlog "Stylist background was not changed as expected :-( "+iColor
    endif
    printlog " check if it is the same in Format -> Page"
    if fGetSetPageBackground (0,1) <> iColor then
        warnlog " First stage error! Difference between stylist and menu entry :-( "+iColor
    endif

    printlog " save this in 3 formats:"
    printlog "+  - StarImpress 5.0"
    printlog "+  - StarOffice 6.0 Presentation"
    printlog "+  - Microsoft PowerPoint 97/2000/XP"
    FileSaveAs
    sleep 2
    Kontext "SpeichernDlg"
    for i = 0 to 2
        if i=0 then x = Dateityp.GetItemCount
        if (i) then  ' set border, whenm start from beginning/end
            y = ((i-1)*2)+1     ' set filter from beginning
        else
            y = x-3     ' set filter from end
        endif
        ListAppend(sFilter(), Dateityp.GetItemText (y))
    next i
    SpeichernDlg.Cancel

    sFileName = convertpath( gOfficePath + "user/work/impress/optional")
    GetFileList ( sFileName, "isty_*.*", Exlist() )
    KillFileList ( Exlist() )

    for i = 1 to (ListCount(sFilter()))
     sleep 1
      printlog "Going to save: '"+sFileName+"isty_"+(i)+"'..."
     sleep 1
      hFileSaveAsWithFilterKill (sFileName+"isty_"+(i), "impress8" )
      printlog " saved with filter ("+i+"/3): "+ sFilter(i)
    next i

    printlog "close impress"
    FileClose
    sleep 1
    kontext 
    if active.exists (5) then active.yes
    sleep 10

    printlog " open just saved files"
    sFileName = ConvertPath ( gOfficePath + "user/work/impress/optional/" )
    GetFileList ( sFileName, "isty_*", sFileList() )

    x = ListCount ( sFileList() )
    for i = 1 to x
     printlog "("+i+"/"+x+"): "+sFileList(i)
      hFileOpen ( sFileList(i) )
      Sleep 5
      If hIsNamedDocLoaded (sFileList(i)) Then
         printlog "used filter: " + hGetUsedFilter()
      else
         warnlog "document didn't get loaded"
      endif
      sleep 5
      printlog " check if stylist, menue and prediction are as expected"
     iColor = 10
      if ((fGetSetPageBackground (0,0) <> iColor) OR (fGetSetPageBackground (0,1) <> iColor)) then
         warnlog "Background was not as expected on loading :-( "+iColor
      endif
      iColor = 20
      printlog " change background in stylist"
      if (fGetSetPageBackground (iColor,0) <> iColor) then
         warnlog "Stylist background was not changed as expected :-( "+iColor
      endif
      sleep 1
      printlog " check if background is same in format menu"
      if (fGetSetPageBackground (0,1) <> iColor) then
         warnlog "Format menu background is wrong :-( "+iColor
      endif

      hCloseDocument
      sleep 2
    next i
endcase 'tFormatStylistBackground

'------------------------------------------------------------------------------

