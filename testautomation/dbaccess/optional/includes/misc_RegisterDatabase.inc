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

'encoding UTF-8  Do not remove or change this line!
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
'* owner : marc.neumann@oracle.com
'*
'* short description : Register database
'*
'\***********************************************************************

sub misc_RegisterDatabase

	printlog "------------------- misc_RegisterDatabase.inc ------------------------"
       
    call tRegisterDatabase
    
end sub
'-------------------------------------------------------------------------
testcase tRegisterDatabase

    ' if the cws dba30 is integrated use revision 1.5 of this file

    call fCreateDbaseDatasource(gOfficePath + "user/work/TT_RegisterTest.odb",gOfficePath + "user/work")
    call fRegisterDatabaseFile(gOfficePath + "user/work/TT_RegisterTest.odb","TT_Register")
    
    call hNewDocument
       
    ViewCurrentDatabase
    
    if ( not fSelectDatasourceInBeamer("TT_Register") ) then
        warnlog "the datasource registration doesn't work. The database is not displayed in the beamer"
    endif
    
    call fDeRegisterDatabaseFile("TT_Register")
    
    call hCloseDocument
    
endcase


