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
'**************************************************************************************
' #1 tInsertSlide
' #1 tInsertDuplicateSlide
' #1 tInsertLayer
'\*************************************************************************************

testcase tInsertSlide
    Call  hNewDocument
    InsertSlide
    sleep 2
    hTypekeys "<Pagedown>"
    sleep 2
    try
        EditDeleteSlide
        PrintLog "Slide 2 deleted"
    catch
        sleep 2
        WarnLog "No slide was inserted"
    endcatch
    hCloseDocument
endcase

testcase tInsertDuplicateSlide
    Call  hNewDocument
    sleep 2
    hRechteckErstellen ( 10, 10, 50, 50 )
    sleep 2
    InsertDuplicateSlide
    try
        EditSelectAll
        sleep 2
        hTypeKeys "<DELETE>"
        PrintLog "Slide duplicated"
    catch
        Warnlog " Slide not duplicated"
    endcatch
    try
        EditDeleteSlide
    catch
        WarnLog "Error when deleting. Slide is not duplicated ??"
    endcatch
    hCloseDocument
endcase

testcase tInsertLayer
    Call hNewDocument
    sleep 2
    ViewLayer
    InsertLayer
    Kontext "EbeneEinfuegenDlg"
    EbeneEinfuegenDlg.OK
    sleep 2
    try
        EditDeleteLayer
        Kontext "Active"
        Active.Yes
    catch
        WarnLog "There is no avaiable additional layer to delete"
    endcatch
    hCloseDocument
endcase
