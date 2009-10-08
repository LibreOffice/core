'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: g_group.inc,v $
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
'*********************************************************************
' #1 tiGruppierung
'\********************************************************************

testcase tiGruppierung
    Dim PosX 		 'Variable fuer PositionX

    sleep 3
    Kontext "DocumentImpress"
    sleep 2
    sleep 1
    sleep 1
    sleep 1
    Printlog "- Created 3 rectangles for group testing"
    if (gApplication = "DRAW") then
        hOpenContextMenu
        sleep(2)
        hMenuSelectNr(12) ' Select "Group"
        sleep(2)
    else
    endif
    sleep 1
    Printlog "- Get position and dimensions of elements"
    sleep 1
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    sleep 1
    PosX=PositionX.GetText
    TabPositionAndSize.OK
    sleep 1
    Kontext "DocumentImpress"
    gMouseClick 35,35
    sleep 1
    sleep 3
    try
        sleep 1
        Warnlog "- Entering the group, no object within the group should have been selected"
    catch
        Printlog "- entered group, nothing selected"
    endcatch
    gMouseClick (35,35)
    gMouseMove (30,30,60,60)
    sleep 1
    Printlog "- Exit group"
    '  sleep 1
    '  hMenuSelectNr (13)
    ' would be better to call the slot TBO!
    ContextPositionAndSize
    sleep 1
    Kontext
    Active.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    sleep 1
    if PosX = PositionX.GetText then
        Warnlog "- No change in position for X axis, even we changed position of 1 object within the group"
        TabPositionAndSize.OK
    else
        Printlog "- Moving within the group works"
        TabPositionAndSize.OK
    end if
endcase
