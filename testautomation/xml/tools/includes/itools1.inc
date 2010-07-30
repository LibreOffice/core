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
'* owner : wolfram.garten@oracle.com
'*
'* short description : Internet- and XML Tools
'*
'***************************************************************************************
'*
' #1 hGetXMLRoot
'*
'\***********************************************************************************
function hGetXMLRoot ( XMLFileName as string ) as boolean
'/// <i>hGetXML</i>
'/// +checks the initial XML-tag in the StarOffice-XML-document. This function is CASE SENSITIVE!
'/// +INPUT  : <Filename as string>
'/// +RETURN : TRUE, FALSE
 Dim FileNum as Integer
 Dim xmlZeile as String

 hGetXMLRoot = FALSE
 FileNum = FreeFile

 Open XMLFileName For Input As #FileNum
   Do until EOF(#FileNum) = TRUE
      Line input #FileNum, xmlZeile
      xmlZeile = Trim ( xmlZeile )
        if xmlZeile = "<?xml version=" & CHR$(34) & "1.0" & CHR$(34) & " encoding=" & CHR$(34) & "UTF-8" & CHR$(34) & "?>" then
          hGetXMLRoot = TRUE
          Exit Function
        end if
   Loop
   Close #FileNum
end function

'-------------------------------------------------------------------------

