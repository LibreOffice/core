'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: t_basic.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-13 10:27:08 $
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
'* owner : helge.delfs@sun.com
'*
'* short description : Check basic functions in testtool
'*
'***************************************************************************
' #1 CheckCommand_Join() as boolean ' Checks the function Join
' #1 CheckCommand_Split() as boolean ' Check the function Split
'\**************************************************************************


function CheckCommand_Join() as boolean
 Dim BlaBla(10) As String, Test As String
 Dim i As Integer

'/// Syntax: ///
'/// Join(list[, delimiter]) ///
'/// list (Required) ///
'/// One-dimensional array containing substrings to be joined. ///

'/// delimiter (Optional) ///
'/// String character used to separate the substrings in the ///
'/// returned string. If omitted, the space character (" ") ///
'/// is used. If delimiter is a zero-length string (""), all ///
'/// items in the list are concatenated with no delimiters. ///
'/// Checks this function and returns 'True' if it worked ///

 For i = 0 To 10
  BlaBla(i) = "This is Number" & i
 Next i

 Test = Join(BlaBla(), "|")
 if Test <> "This is Number0|This is Number1|This is Number2|This is Number3|This is Number4|This is Number5|This is Number6|This is Number7|This is Number8|This is Number9|This is Number10" then
  CheckCommand_Join = False
 else
  CheckCommand_Join = True
 endif

end function



function CheckCommand_Split() as boolean
 '/// Syntax: ///
 '/// Split(expression[, delimiter[, count]]) ///
 '/// expression (Required) ///
 '///     String expression containing substrings and delimiters. If ///
 '///     expression is a zero-length string, Split returns an empty ///
 '///     array, that is, an array with no elements and no data. ///

 '/// delimiter (Optional) ///
 '/// String character used to identify substring limits. If omit- ///
 '/// ted, the space character (" ") is assumed to be the delimi- ///
 '/// ter. If delimiter is a zero-length string, a single-element ///
 '/// array containing the entire expression string is returned. ///

 '/// count (Optional) ///
 '/// Number of substrings to be returned; -1 indicates that all  ///
 '/// substrings are returned. ///

 '/// Checks this function and returns 'True' if it worked ///

 Dim BlaBla(10) As String, Test As String
 Dim i As Integer, CheckTest() as string

 For i = 0 To 10
  BlaBla(i) = "This is Number" & i
 Next i

 Test = Join(BlaBla(), "|")
 if Test <> "This is Number0|This is Number1|This is Number2|This is Number3|This is Number4|This is Number5|This is Number6|This is Number7|This is Number8|This is Number9|This is Number10" then
  CheckCommand_Split = False
 else
   CheckTest = Split(Test,"|")
   if Ubound(CheckTest) <> 10 then
    CheckCommand_Split = False
   else
    CheckTest = Split(Test,"|",5)
    if Ubound(CheckTest) <> 4 then
     CheckCommand_Split = False
    else
     CheckCommand_Split = True
    endif
   endif
 endif

end function
