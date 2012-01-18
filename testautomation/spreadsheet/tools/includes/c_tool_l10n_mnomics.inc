'encoding UTF-8  Do not remove or change this line!
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
'*
'* short description : Tools for Keyboard Accessibility
'*
'************************************************************************
'*
' #1 fDataPilotRowSelection
' #1 fDataPilotColumnSelection
' #1 fDataPilotDataSelection
'*
'\***********************************************************************

function fDataPilotRowSelection()

fDataPilotRowSelection = TRUE

   select case iSprache
      case 01   :        ' English (USA)
         DataPilotSelection.typekeys "<MOD2 R>"
      case 33   :        ' French
         DataPilotSelection.typekeys "<MOD2 g>"
      case 34   :        ' Spanish
         DataPilotSelection.typekeys "<MOD2 l>"
      case 39   :        ' Italian
         DataPilotSelection.typekeys "<MOD2 R>"
      case 46   :        ' Swedish
         DataPilotSelection.typekeys "<MOD2 d>"
      case 49   :        ' German
         DataPilotSelection.typekeys "<MOD2 l>"
      case 55   :        ' Portuguese
         DataPilotSelection.typekeys "<MOD2 h>"
      case 81   :        ' Japanese
         DataPilotSelection.typekeys "<MOD2 B>"
      case 82   :        ' Korean
         DataPilotSelection.typekeys "<MOD2 B>"
      case 86   :        ' Chinese (simplified)
         DataPilotSelection.typekeys "<MOD2 B>"
      case 88   :        ' Chinese (traditional)
         DataPilotSelection.typekeys "<MOD2 B>"
      case else :        ' Fallback
         QAErrorLog "Now, the test does not support for the language " +iSprache
	 fDataPilotRowSelection = FALSE
   end select

end function

'----------------------------------------------------

function fDataPilotColumnSelection()

fDataPilotColumnSelection = TRUE

   select case iSprache
      case 01   :        ' English (USA)
         DataPilotSelection.typekeys "<MOD2 C>"
      case 33   :        ' French
         DataPilotSelection.typekeys "<MOD2 C>"
      case 34   :        ' Spanish
         DataPilotSelection.typekeys "<MOD2 C>"
      case 39   :        ' Italian
         DataPilotSelection.typekeys "<MOD2 C>"
      case 46   :        ' Swedish
         DataPilotSelection.typekeys "<MOD2 m>"
      case 49   :        ' German
         DataPilotSelection.typekeys "<MOD2 S>"
      case 55   :        ' Portuguese
         DataPilotSelection.typekeys "<MOD2 C>"
      case 81   :        ' Japanese
         DataPilotSelection.typekeys "<MOD2 A>"
      case 82   :        ' Korean
         DataPilotSelection.typekeys "<MOD2 A>"
      case 86   :        ' Chinese (simplified)
         DataPilotSelection.typekeys "<MOD2 A>"
      case 88   :        ' Chinese (traditional)
         DataPilotSelection.typekeys "<MOD2 A>"
      case else :        ' Fallback
         QAErrorLog "Now, the test does not support for the language " +iSprache
	 fDataPilotColumnSelection = FALSE
   end select

end function

'----------------------------------------------------

function fDataPilotDataSelection()

fDataPilotDataSelection = TRUE

   select case iSprache
      case 01   :        ' English (USA)
         DataPilotSelection.typekeys "<MOD2 D>"
      case 33   :        ' French
         DataPilotSelection.typekeys "<MOD2 D>"
      case 34   :        ' Spanish
         DataPilotSelection.typekeys "<MOD2 t>"
      case 39   :        ' Italian
         DataPilotSelection.typekeys "<MOD2 D>"
      case 46   :        ' Swedish
         DataPilotSelection.typekeys "<MOD2 t>"
      case 49   :        ' German
         DataPilotSelection.typekeys "<MOD2 D>"
      case 55   :        ' Portuguese
         DataPilotSelection.typekeys "<MOD2 D>"
      case 81   :        ' Japanese
         DataPilotSelection.typekeys "<MOD2 C>"
      case 82   :        ' Korean
         DataPilotSelection.typekeys "<MOD2 C>"
      case 86   :        ' Chinese (simplified)
         DataPilotSelection.typekeys "<MOD2 C>"
      case 88   :        ' Chinese (traditional)
         DataPilotSelection.typekeys "<MOD2 C>"
      case else :        ' Fallback
         QAErrorLog "Now, the test does not support for the language " +iSprache
	 fDataPilotDataSelection = FALSE
   end select

end function

'----------------------------------------------------

function fDataPilotPageSelection()

fDataPilotPageSelection = TRUE

   select case iSprache
      case 01   :        ' English (USA)
         DataPilotSelection.typekeys "<MOD2 P>"
      case else :        ' Fallback
         QAErrorLog "Now, the test does not support for the language " +iSprache
	 fDataPilotPageSelection = FALSE
   end select

end function
