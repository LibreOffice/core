'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_008_.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: vg $ $Date: 2008-08-18 12:42:00 $
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
'* owner : fredrik.haegg@sun.com
'*
'* short description :   Diverse Testcases for the office.
'*
'\******************************************************************

sub w_008printLog Chr(13) + "--------- Window Menu (w_008_.inc) ----------"
    gApplication = "WRITER"

    Call tWindowNewWindow

end sub

'-----------------------------------------------------------

testcase tWindowNewWindow
    PrintLog "- Window / New Window"
    if gApplication = "HTMLDOKUMENT" then
        printlog "Not in WriterWeb !"
        goto endsub
    end if
    Call hNewDocument
    printlog " Open new document"
    Call hTabelleEinfuegen
    printlog " Insert Table"
    WindowNewWindow
    printlog " Window / New Window"
    WaitSlot (2000)
    Call hCloseDocument
    printlog " Close active document"
endcase

'-----------------------------------------------------------
