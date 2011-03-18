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
'* owner : oliver.craemer@oracle.com
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
