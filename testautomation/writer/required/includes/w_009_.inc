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

'*                                                                **
'* owner : helge.delfs@oracle.com                                    **
'*                                                                **
'* short description :   Testcases for the Help-Menu
'*
'\******************************************************************

sub w_009_

    printLog Chr(13) + "--------- Menu Help (w_009_.inc) ----------"
    gApplication = "WRITER"

    Call tHelpTip
    Call tHelpExtendedTips
    Call tHelpAboutStarOffice

end sub

'-----------------------------------------------------------

testcase tHelpTip
    PrintLog "- Help / Tip"

    printlog " Open new document"
    Call hNewDocument
    printlog " Help / Tips"
    HelpTips
    WaitSlot (2000)
    printlog " Help / Tips"
    HelpTips
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tHelpExtendedTips
    PrintLog "- Help / Extended Tips"

    printlog " Open new document"
    Call hNewDocument
    printlog " Help / Extended Help"
    HelpEntendedHelp
    WaitSlot (2000)
    printlog " Help / Extended Help"
    HelpEntendedHelp
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------

testcase tHelpAboutStarOffice
    PrintLog "- Help / About StarOffice"

    printlog " Open new document"
    Call hNewDocument
    printlog " Help / About"
    HelpAboutStarOffice
    Kontext "UeberStarWriter"
    DialogTest ( UeberStarWriter )
    printlog " In About dialog enter key <Strg+Alt+Del>"
    UeberStarWriter.TypeKeys "<Mod1 S><Mod1 D><Mod1 T>"
    printlog " Close About dialog"
    hCloseDialog( UeberStarWriter, "ok" )
    printlog " Close active document"
    Call hCloseDocument
endcase

'-----------------------------------------------------------
