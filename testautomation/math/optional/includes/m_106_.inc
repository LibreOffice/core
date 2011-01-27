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
'* short description :
'*
'\******************************************************************

sub M_106_
   printlog Chr(13) +  "--  Tools-Menue  --"
   Call tExtrasFormelImportieren
end sub

testcase tExtrasFormelImportieren
 dim i,x as integer

'/// check if formula is loaded in same window ///'

   Call hNewDocument

  '/// check amount of available filters (6) (inc. one seperator) ///'
   ToolsImportFormula
      Kontext "OeffnenDlg"
      x = Dateityp.GetItemCount
      if x <> cFilterCountLoad then warnlog " The Filter count is wrong! Have to be ("+cFilterCountLoad+"), but are: "+x
      printlog "------------- this is the filter list: ------------"
      for i = 1 to x
         try
            Printlog " "+i+": " +  Dateityp.GetItemText (i)
          catch
            printlog "fail ------------"
          endcatch
      next i
   OeffnenDlg.Cancel
   Sleep 2

   Call hCloseDocument
endcase


