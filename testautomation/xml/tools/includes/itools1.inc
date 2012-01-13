'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************

*************
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

