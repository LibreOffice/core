/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/

'*************************************************************************
'
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
'*************************************************************************

'*
'* owner : helge.delfs@oracle.com
'*
'* short description :   Diverse Testcases for the office.
'*
'\******************************************************************

sub w_008_

    printLog Chr(13) + "--------- Window Menu (w_008_.inc) ----------"
    gApplication = "WRITER"

    Call tWindowNewWindow

end sub

'-----------------------------------------------------------

testcase tWindowNewWindow
    PrintLog "- Window / New Window"
    if gApplication = "HTML" then
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
