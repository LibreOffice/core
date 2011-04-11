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
'* owner : gregor.hartmann@oracle.com
'*
'* short description : check the internal file dialog ( 1. part )
'*
'\******************************************************************************

testcase tFolder3

   
   dim FULLPATH as string
       FULLPATH = gOfficePath + "user\work\"
       
   dim FOLDERNAME_U as string
       FOLDERNAME_U = "AAABBB"
       
   dim FOLDERNAME_M as string
       FOLDERNAME_M = "AAaabbBB"
       

   FileOpen
   
   printlog( " - change to working directory" )
   Kontext "OeffnenDlg"
   Dateiname.SetText( ConvertPath ( FULLPATH ))
   Oeffnen.Click()
   
   printlog( " - foldername with upper case characters only" )
   call CreateValidDirectory( FOLDERNAME_U )
   
   printlog( " - foldername with mixed case characters" )
   call CreateValidDirectory( FOLDERNAME_M )
   
   kontext "OeffnenDlg"
   OeffnenDlg.Cancel()
   
endcase


